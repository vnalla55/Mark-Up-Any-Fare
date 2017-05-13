//-------------------------------------------------------------------
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------

#include "RexPricing/RefundSolutionValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/FlownStatusCheck.h"
#include "Common/Logger.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RefundProcessInfo.h"
#include "DBAccess/FareTypeTable.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag689Collector.h"
#include "RexPricing/EnhancedRefundDiscountApplier.h"
#include "RexPricing/PenaltyCalculator.h"
#include "RexPricing/RefundDiscountApplier.h"

#include <algorithm>

namespace tse
{
FIXEDFALLBACK_DECL(enhancedRefundDiscountApplierRefactor)

static Logger
logger("atseintl.RefundPricing.RefundSolutionValidator");

const Indicator RefundSolutionValidator::SAME_FARE_TYPE = 'T';
const Indicator RefundSolutionValidator::SAME_FARE_CLASS = 'C';
const Indicator RefundSolutionValidator::CHANGE_TO_FARE_BREAKS_NOT_ALLOWED = ' ';
const Indicator RefundSolutionValidator::RULE_TARIFF_NO_RESTRICTION = ' ';
const Indicator RefundSolutionValidator::RULE_TARIFF_EXACT_TARIFF = 'X';
const Indicator RefundSolutionValidator::SAME_RBD_OR_CABIN = 'S';

namespace
{

typedef std::vector<RefundPermutation*>::const_iterator PermIt;
typedef std::vector<RefundProcessInfo*>::const_iterator InfoIt;
typedef std::vector<PricingUnit*>::const_iterator PuIt;
typedef std::vector<FareUsage*>::const_iterator FuIt;
}

RefundSolutionValidator::RefundSolutionValidator(RefundPricingTrx& trx, FarePath& fp)
  : _trx(trx), _farePath(fp), _dc(nullptr)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic689)
  {
    DCFactory* factory = DCFactory::instance();
    _dc = dynamic_cast<Diag689Collector*>(factory->create(_trx));
    if (_dc != nullptr)
    {
      _dc->enable(Diagnostic689);

      if (!_dc->isActive())
      {
        _dc = nullptr;
      }
      else
      {
        _dc->initialize();
        _dc->filterByFarePath(_farePath);
        if (!_dc->filterPassed())
        {
          _dc = nullptr;
        }
      }
    }
  }
};

RefundSolutionValidator::~RefundSolutionValidator()
{
  if (_dc != nullptr)
  {
    _dc->flushMsg();
  }
}

bool
RefundSolutionValidator::process()
{
  if (_dc)
    _dc->print(_farePath);

  const RefundDiscountApplier& discountApplier =
      (fallback::fixed::enhancedRefundDiscountApplierRefactor()
           ? *EnhancedRefundDiscountApplier::create(_trx.dataHandle(), _trx.getExchangePaxType())
           : *RefundDiscountApplier::create(_trx.dataHandle(), _trx.getExchangePaxType()));

  PenaltyCalculator penaltyCalc(_trx, discountApplier);

  for (PermIt perm = _trx.permutations().begin(); perm != _trx.permutations().end(); ++perm)
  {
    printPermutation(**perm);
    bool result = process(**perm, penaltyCalc);
    printValidationResult((**perm).number(), result);

    if (result)
      setWinnerPermutation(**perm);
  }

  return _farePath.lowestFee33Perm();
}

void
RefundSolutionValidator::setWinnerPermutation(const RefundPermutation& permutation)
{
  if (!_farePath.lowestFee33Perm() ||
      _farePath.lowestFee33Perm()->overallPenalty(_trx) > permutation.overallPenalty(_trx))
  {
    _farePath.setLowestFee33Perm(&permutation);
    _farePath.setRexChangeFee(permutation.overallPenalty(_trx));
  }
}

bool
RefundSolutionValidator::process(RefundPermutation& permutation, PenaltyCalculator& penaltyCalc)
{
  if (hasDiagAndFilterPassed())
    *_dc << "VALIDATING FARE BREAK LIMITATIONS: ";

  if (executeValidation(permutation, FARE_BREAKS))
  {
    if (hasDiagAndFilterPassed())
      *_dc << "PASSED\n"
           << "VALIDATING FARE RESTRICTIONS: ";

    if (repriceIndicatorValidation(permutation) && executeValidation(permutation, RULE_TARIFF) &&
        executeValidation(permutation, RULE) && executeValidation(permutation, FARE_CLASS_FAMILY) &&
        executeValidation(permutation, FARE_TYPE) && executeValidation(permutation, SAME_FARE) &&
        executeValidation(permutation, NORMAL_SPECIAL) && executeValidation(permutation, OWRT) &&
        executeValidation(permutation, FARE_AMOUNT) && executeValidation(permutation, SAME_RBD))
    {
      penaltyCalc.calculate(permutation);

      if (hasDiagAndFilterPassed())
      {
        *_dc << "PASSED\n";
        _dc->printPenalty(_trx.exchangeItin().front()->farePath().front()->pricingUnit(),
                          permutation,
                          _farePath.getTotalNUCAmount());
      }
      return true;
    }
  }

  return false;
}

bool
RefundSolutionValidator::hasDiagAndFilterPassed() const
{
  return _dc && _dc->filterPassed();
}

void
RefundSolutionValidator::printValidationResult(unsigned number, bool result) const
{
  if (hasDiagAndFilterPassed())
  {

    *_dc << "PERMUTATION " << number << ": ";
    result ? *_dc << "PASSED\n" : *_dc << "FAILED\n";
  }
}

void
RefundSolutionValidator::printPermutation(const RefundPermutation& permutation)
{
  if (_dc)
  {
    _dc->print(permutation);
    InfoIt processInfo = permutation.processInfos().begin(),
           processInfoEnd = permutation.processInfos().end();

    for (; processInfo != processInfoEnd; ++processInfo)
      _dc->printMapping(**processInfo, getRelatedFUs((**processInfo).fareCompInfo()));
  }
}

bool
RefundSolutionValidator::executeValidation(const RefundPermutation& permutation,
                                           const Validator& byteValidator)
{
  InfoIt processInfo = permutation.processInfos().begin(),
         processInfoEnd = permutation.processInfos().end();
  for (; processInfo != processInfoEnd; ++processInfo)
  {
    const CacheKey key((**processInfo).fareCompInfo(), (**processInfo).record3(), byteValidator);
    bool cachedValidationResult;

    if (checkCache(key, cachedValidationResult))
    {
      if (cachedValidationResult)
        continue;

      return false;
    }

    if (!executeValidation(
            **processInfo, getRelatedFUs((**processInfo).fareCompInfo()), byteValidator))
    {
      _validationCache.insert(std::make_pair(key, false));
      return false;
    }

    _validationCache.insert(std::make_pair(key, true));
  }

  return true;
}

RefundSolutionValidator::ValidationMethod
RefundSolutionValidator::getValidationMethod(const Validator& byteValidator)
{
  switch (byteValidator)
  {
  case FARE_BREAKS:
    return &RefundSolutionValidator::checkFareBreaks;
  case RULE_TARIFF:
    return &RefundSolutionValidator::checkRuleTariff;
  case RULE:
    return &RefundSolutionValidator::checkRule;
  case FARE_CLASS_FAMILY:
    return &RefundSolutionValidator::checkFareClassCode;
  case FARE_TYPE:
    return &RefundSolutionValidator::checkFareType;
  case SAME_FARE:
    return &RefundSolutionValidator::validateSameFareIndicator;
  case NORMAL_SPECIAL:
    return &RefundSolutionValidator::checkFareNormalSpecial;
  case OWRT:
    return &RefundSolutionValidator::checkOWRT;
  case FARE_AMOUNT:
    return &RefundSolutionValidator::checkFareAmount;
  case SAME_RBD:
    return &RefundSolutionValidator::checkSameRBD;
  default:
    TSE_ASSERT(!"RefundSolutionValidator::getValidationMethod: INVALID INPUT");
    return nullptr; // never reached
  }
}

bool
RefundSolutionValidator::executeValidation(const RefundProcessInfo& processInfo,
                                           const std::vector<FareUsage*>& mappedFus,
                                           const Validator& byteValidator)
{
  FuIt fu = mappedFus.begin(), fuEnd = mappedFus.end();
  for (; fu != fuEnd; ++fu)
  {
    RefundSolutionValidator::ValidationMethod method = getValidationMethod(byteValidator);

    if (!(this->*(method))(**fu, processInfo))
      return false;
  }
  return true;
}

bool
RefundSolutionValidator::checkCache(const CacheKey& key, bool& cachedValidationResult) const
{
  std::map<const CacheKey, const bool>::const_iterator found = _validationCache.find(key);

  if (found != _validationCache.end() && !hasDiagAndFilterPassed())
  {
    cachedValidationResult = found->second;
    return true;
  }

  return false;
}

bool
RefundSolutionValidator::originChanged(const FareUsage& fareUsage,
                                       const RefundProcessInfo& processInfo) const
{
  return fareUsage.paxTypeFare()->fareMarket()->boardMultiCity() !=
         processInfo.fareCompInfo().fareMarket()->boardMultiCity();
}

bool
RefundSolutionValidator::destinationChanged(const FareUsage& fareUsage,
                                            const RefundProcessInfo& processInfo) const
{
  return fareUsage.paxTypeFare()->fareMarket()->offMultiCity() !=
         processInfo.fareCompInfo().fareMarket()->offMultiCity();
}

bool
RefundSolutionValidator::checkFareBreaks(const FareUsage& fareUsage,
                                         const RefundProcessInfo& processInfo)
{
  FlownStatusCheck flownCheck(processInfo.fareCompInfo());

  if (flownCheck.isTotallyFlown() &&
      processInfo.record3().fareBreakpoints() == CHANGE_TO_FARE_BREAKS_NOT_ALLOWED)
  {
    if (originChanged(fareUsage, processInfo))
    {
      printFareBreaksFail(fareUsage, processInfo, "ORIGIN");
      return false;
    }
    if (destinationChanged(fareUsage, processInfo))
    {
      printFareBreaksFail(fareUsage, processInfo, "DESTINATION");
      return false;
    }
  }

  if (flownCheck.isPartiallyFlown() && originChanged(fareUsage, processInfo))
  {
    printFareBreaksFail(fareUsage, processInfo, "ORIGIN");
    return false;
  }

  return true;
}

void
RefundSolutionValidator::printFareBreaksFail(const FareUsage& fareUsage,
                                             const RefundProcessInfo& processInfo,
                                             const std::string& failPoint)
{
  if (hasDiagAndFilterPassed())
  {
    printCommonFail(*fareUsage.paxTypeFare(), processInfo, "FARE BREAKS");
    *_dc << failPoint << " FARE BREAK POINT CHANGED\n";
  }
}

bool
RefundSolutionValidator::checkRuleTariff(const FareUsage& fareUsage,
                                         const RefundProcessInfo& processInfo)
{
  if (fareUsage.paxTypeFare()->vendor() != ATPCO_VENDOR_CODE)
  {
    if (hasDiagAndFilterPassed())
      printCommonFail(*fareUsage.paxTypeFare(), processInfo, "TARIFF NUMBER: NOT ATPCO FARE");

    return false;
  }

  if (!processInfo.record3().ruleTariff())
    return checkAnyRuleTariff(*fareUsage.paxTypeFare(), processInfo);

  return checkSameRuleTariff(*fareUsage.paxTypeFare(), processInfo);
}

bool
RefundSolutionValidator::checkAnyRuleTariff(const PaxTypeFare& ptf,
                                            const RefundProcessInfo& processInfo)
{
  switch (processInfo.record3().ruleTariffInd())
  {
  case RULE_TARIFF_NO_RESTRICTION:
    if (processInfo.paxTypeFare().tcrTariffCat() < ptf.tcrTariffCat())
    {
      printAnyRuleTariffFail(ptf, processInfo, "NOT PUBLIC TARIFF");
      return false;
    }
    break;

  case RULE_TARIFF_EXACT_TARIFF:
    if (processInfo.paxTypeFare().tcrTariffCat() != ptf.tcrTariffCat())
    {
      printAnyRuleTariffFail(ptf, processInfo, "DIFFERENT FARE TARIFF");
      return false;
    }
  }

  return true;
}

bool
RefundSolutionValidator::checkSameRuleTariff(const PaxTypeFare& ptf,
                                             const RefundProcessInfo& processInfo)
{
  if (processInfo.record3().ruleTariff() != ptf.tcrRuleTariff() &&
      processInfo.paxTypeFare().tcrRuleTariff() != ptf.tcrRuleTariff() &&
      processInfo.paxTypeFare().fareClassAppInfo()->_ruleTariff !=
          ptf.fareClassAppInfo()->_ruleTariff)
  {
    printSameRuleTariffFail(ptf, processInfo, "DIFFERENT TARIFF NUMBER");
    return false;
  }

  return true;
}

void
RefundSolutionValidator::printAnyRuleTariffFail(const PaxTypeFare& ptf,
                                                const RefundProcessInfo& processInfo,
                                                const std::string& message)
{
  if (hasDiagAndFilterPassed())
  {
    printCommonFail(ptf, processInfo, "TARIFF NUMBER");
    *_dc << "EXC FARE TARIFF: ";
    printTariffCat(processInfo.paxTypeFare().tcrTariffCat());
    *_dc << "   REPRICE FARE TARIFF: ";
    printTariffCat(ptf.tcrTariffCat());
    *_dc << "\n   " << message << "\n";
  }
}

void
RefundSolutionValidator::printTariffCat(const int& tariffCat)
{
  if (tariffCat)
    *_dc << "PRIVATE";
  else
    *_dc << "PUBLIC";
}

void
RefundSolutionValidator::printSameRuleTariffFail(const PaxTypeFare& ptf,
                                                 const RefundProcessInfo& processInfo,
                                                 const std::string& message)
{
  if (hasDiagAndFilterPassed())
  {
    printCommonFail(ptf, processInfo, "TARIFF NUMBER");

    *_dc << "EXC TARIFF NUMBER: " << processInfo.paxTypeFare().tcrRuleTariff();
    *_dc << "   REPRICE TARIFF NUMBER: " << ptf.tcrRuleTariff();

    *_dc << "\n   " << message << "\n";
  }
}

void
RefundSolutionValidator::printCommonFail(const PaxTypeFare& ptf,
                                         const RefundProcessInfo& processInfo,
                                         const std::string& message)
{
  *_dc << "FAILED\n FAILED: " << message << "\n";
  _dc->printHeader(processInfo);
  _dc->printMapping(ptf);
}

const std::vector<FareUsage*>&
RefundSolutionValidator::getRelatedFUs(const FareCompInfo& fareCompInfo)
{
  if (_relatedFUsMap.find(&fareCompInfo) == _relatedFUsMap.end())
  {
    std::set<FareMarket*> allFMs;
    std::map<FareMarket*, FareUsage*> allFMsFUs;

    std::vector<PricingUnit*>::const_iterator puIter = _farePath.pricingUnit().begin();

    for (; puIter != _farePath.pricingUnit().end(); ++puIter)
    {
      FuIt fuIter = (*puIter)->fareUsage().begin();
      for (; fuIter != (*puIter)->fareUsage().end(); ++fuIter)
      {
        allFMs.insert((*fuIter)->paxTypeFare()->fareMarket());
        allFMsFUs.insert(std::make_pair((*fuIter)->paxTypeFare()->fareMarket(), *fuIter));
      }
    }
    std::vector<FareMarket*> relatedFMs;

    std::set_intersection(allFMs.begin(),
                          allFMs.end(),
                          fareCompInfo.getMappedFCs().begin(),
                          fareCompInfo.getMappedFCs().end(),
                          std::back_inserter(relatedFMs));

    std::vector<FareMarket*>::const_iterator relatedFMsIter = relatedFMs.begin();
    std::vector<FareUsage*>& fareUsages = _relatedFUsMap[&fareCompInfo];

    for (; relatedFMsIter != relatedFMs.end(); ++relatedFMsIter)
      fareUsages.push_back(allFMsFUs[*relatedFMsIter]);
  }
  return _relatedFUsMap[&fareCompInfo];
}

std::string
RefundSolutionValidator::getFareBasis(const PaxTypeFare& paxTypeFare) const
{
  return paxTypeFare.createFareBasis(_trx);
}

bool
RefundSolutionValidator::validateSameFareIndicator(const FareUsage& fareUsage,
                                                   const RefundProcessInfo& refundProcessInfo)
{
  switch (refundProcessInfo.record3().sameFareInd())
  {
  case BLANK:
    return true;
  case SAME_FARE_TYPE:
  {
    if (refundProcessInfo.paxTypeFare().fcaFareType() == fareUsage.paxTypeFare()->fcaFareType())
      return true;
    else
    {
      if (hasDiagAndFilterPassed())
      {
        printCommonFail(*fareUsage.paxTypeFare(), refundProcessInfo, "SAME FARE IND");
        *_dc << " EXC FARE TYPE: " << refundProcessInfo.paxTypeFare().fcaFareType() << "\n";
        *_dc << " REPRICE FARE TYPE: " << fareUsage.paxTypeFare()->fcaFareType() << "\n";
      }
      return false;
    }
  }
  break;
  case SAME_FARE_CLASS:
  {
    std::string excFareBasis = getFareBasis(refundProcessInfo.paxTypeFare());
    std::string repriceFareBasis = getFareBasis(*fareUsage.paxTypeFare());
    if (excFareBasis == repriceFareBasis)
      return true;
    else
    {
      if (hasDiagAndFilterPassed())
      {
        printCommonFail(*fareUsage.paxTypeFare(), refundProcessInfo, "SAME FARE IND");
        *_dc << " EXC FARE CLASS: " << excFareBasis << "\n";
        *_dc << " REPRICE FARE CLASS: " << repriceFareBasis << "\n";
      }
      return false;
    }
  }
  break;
  default:
    break;
  }

  if (hasDiagAndFilterPassed())
    *_dc << " FAILED\n *** INVALID SAME FARE INDICATOR *** \n";

  return false;
}

namespace
{

FareMarket::FareRetrievalFlags
toRetrievalFlag(Indicator ind)
{
  if (ind == RefundPermutation::HISTORICAL_TICKET_BASED)
    return FareMarket::RetrievHistorical;
  if (ind == RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED)
    return FareMarket::RetrievTvlCommence;
  return FareMarket::RetrievNone;
}
}

bool
RefundSolutionValidator::repriceIndicatorValidation(const RefundPermutation& perm) const
{
  FareMarket::FareRetrievalFlags perm_flag = toRetrievalFlag(perm.repriceIndicator());

  const std::vector<PricingUnit*>& pus = _farePath.pricingUnit();
  for (const auto pu : pus)
  {
    const std::vector<FareUsage*>& fus = pu->fareUsage();
    for (const auto fu : fus)
    {
      if (fu->paxTypeFare()->retrievalInfo()->_flag != perm_flag)
      {
        if (hasDiagAndFilterPassed())
          *_dc << " FAILED\nFAILED: REPRICE IND\n";
        return false;
      }
    }
  }

  return true;
}

bool
RefundSolutionValidator::checkRule(const FareUsage& fareUsage,
                                   const RefundProcessInfo& refundProcessInfo)
{
  if (!refundProcessInfo.record3().rule().empty() &&
      !matchWildcardRuleNumber(refundProcessInfo.record3().rule(),
                               fareUsage.paxTypeFare()->ruleNumber()))
  {
    if (hasDiagAndFilterPassed())
      *_dc << " FAILED\nFAILED: RULE NUMBER\n";

    return false;
  }

  return true;
}

bool
RefundSolutionValidator::checkFareNormalSpecial(const FareUsage& fareUsage,
                                                const RefundProcessInfo& processInfo)
{
  if (CommonSolutionValidator::checkFareNormalSpecial(processInfo.record3().nmlSpecialInd(),
                                                      *fareUsage.paxTypeFare()))
    return true;
  if (hasDiagAndFilterPassed())
    printCommonFail(*fareUsage.paxTypeFare(), processInfo, "NORMAL SPECIAL");
  return false;
}

bool
RefundSolutionValidator::checkOWRT(const FareUsage& fareUsage, const RefundProcessInfo& processInfo)
{
  if (CommonSolutionValidator::checkOWRT(processInfo.record3().owrt(), *fareUsage.paxTypeFare()))
    return true;

  if (hasDiagAndFilterPassed())
    printCommonFail(*fareUsage.paxTypeFare(), processInfo, "OWRT");
  return false;
}

bool
RefundSolutionValidator::checkFareType(const FareUsage& fareUsage,
                                       const RefundProcessInfo& processInfo)
{
  if (!processInfo.record3().fareTypeTblItemNo())
    return true;

  const std::vector<FareTypeTable*>& fareTypes =
      _trx.getFareTypeTables(processInfo.paxTypeFare().vendor(),
                             processInfo.record3().fareTypeTblItemNo(),
                             processInfo.fareMarket().ruleApplicationDate());

  if (fareTypes.empty())
  {
    if (hasDiagAndFilterPassed())
      printCommonFail(*fareUsage.paxTypeFare(), processInfo, "NO DATA IN FARE TYPE TBL");

    return false;
  }

  if (checkFareTypeTable(fareTypes, fareUsage.paxTypeFare()->fcaFareType()))
    return true;

  if (hasDiagAndFilterPassed())
    printCommonFail(*fareUsage.paxTypeFare(), processInfo, "FARE TYPE");

  return false;
}

bool
RefundSolutionValidator::checkFareClassCode(const FareUsage& fareUsage,
                                            const RefundProcessInfo& processInfo)
{
  if (CommonSolutionValidator::checkFareClassCode(processInfo.record3().fareClassInd(),
                                                  processInfo.record3().fareClass(),
                                                  *fareUsage.paxTypeFare()))
    return true;

  if (hasDiagAndFilterPassed())
    printCommonFail(*fareUsage.paxTypeFare(), processInfo, "FARE CLASS/FAMILY");
  return false;
}

bool
RefundSolutionValidator::checkFareAmount(const FareUsage& fareUsage,
                                         const RefundProcessInfo& processInfo)
{
  if (fareUsage.paxTypeFare()->fareMarket()->travelSeg().back()->unflown() ||
      processInfo.paxTypeFare().fareMarket()->travelSeg().back()->unflown())
    return true;

  if (fareUsage.paxTypeFare()->fareMarket()->travelSeg().front()->segmentOrder() !=
          processInfo.paxTypeFare().fareMarket()->travelSeg().front()->segmentOrder() ||
      fareUsage.paxTypeFare()->fareMarket()->travelSeg().back()->segmentOrder() !=
          processInfo.paxTypeFare().fareMarket()->travelSeg().back()->segmentOrder())
    return true;

  bool result = false;
  switch (processInfo.record3().fareAmountInd())
  {
  case FareRestrictions::NOT_APPLICABLE:
    result = fareUsage.paxTypeFare()->nucFareAmount() >
             processInfo.paxTypeFare().nucFareAmount() - EPSILON;
    break;
  case FareRestrictions::HIGHER_OR_EQUAL_AMOUNT:
    result = fareUsage.paxTypeFare()->nucFareAmount() >
             processInfo.paxTypeFare().nucFareAmount() + EPSILON;
    break;
  }
  if (result)
    return true;

  if (hasDiagAndFilterPassed())
    printCommonFail(*fareUsage.paxTypeFare(), processInfo, "FARE AMOUNT");
  return false;
}

bool
RefundSolutionValidator::checkSameRBD(const FareUsage& fareUsage,
                                      const RefundProcessInfo& processInfo)
{
  if (processInfo.record3().bookingCodeInd() == SAME_RBD_OR_CABIN &&
      fareUsage.paxTypeFare()->bookingCodeStatus().isSet(PaxTypeFare::BKS_SAME_FAIL))
  {
    if (hasDiagAndFilterPassed())
      printCommonFail(*fareUsage.paxTypeFare(), processInfo, "SAME RBD");
    return false;
  }
  return true;
}
}
