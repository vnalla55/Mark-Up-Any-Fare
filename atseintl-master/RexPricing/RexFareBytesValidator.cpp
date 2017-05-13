//-------------------------------------------------------------------
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "RexPricing/RexFareBytesValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/Vendor.h"
#include "DataModel/ExcItin.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag602Collector.h"
#include "Diagnostic/Diag689Collector.h"

#include <boost/assign/list_of.hpp>

#include <algorithm>

namespace tse
{
FALLBACK_DECL(rexFareTypeTbl)
FALLBACK_DECL(excPrivPubNoAtpFares)

const RexFareBytesValidator::CheckersVector RexFareBytesValidator::_fareByteCheckers =
    boost::assign::list_of(std::make_pair(&RexFareBytesValidator::checkFareRules, RULE_INDICATOR))(
        std::make_pair(&RexFareBytesValidator::checkFareCxrApplTbl, CARRIER_APPLICATION_TBL))(
        std::make_pair(&RexFareBytesValidator::checkFareTrfNumber, TARIFF_NUMBER))(
        std::make_pair(&RexFareBytesValidator::checkExcludePrivateIndicator, EXCLUDE_PRIVATE))(
        std::make_pair(&RexFareBytesValidator::checkFareClassCode, FARE_CLASS_CODE))(
        std::make_pair(&RexFareBytesValidator::checkFareTypeCode, FARE_TYPE_CODE))(
        std::make_pair(&RexFareBytesValidator::checkFareTypeTable, FARE_TYPE_TABLE))(
        std::make_pair(&RexFareBytesValidator::checkFareAmount, FARE_AMOUNT))(
        std::make_pair(&RexFareBytesValidator::checkFareNormalSpecial, NORMAL_SPECIAL))(
        std::make_pair(&RexFareBytesValidator::checkOWRT, OWRT_INDICATOR))(
        std::make_pair(&RexFareBytesValidator::checkSameInd, SAME_INDICATOR));

RexFareBytesValidator::RexFareBytesValidator(RexPricingTrx& trx, const FarePath* excFarePath)
  : _trx(trx), _allRepricePTFs(nullptr), _dc(nullptr), _itinIndex(0),
    _mapper(nullptr), _changeVendorForDFFFares(checkIfOverrideNeeded(excFarePath))
{
}

RexFareBytesValidator::RexFareBytesValidator(RexPricingTrx& trx,
                                             const std::vector<const PaxTypeFare*>& allRepricePTFs,
                                             const uint16_t& itinIndex,
                                             const GenericRexMapper* grm,
                                             const FarePath* excFarePath)
  : _trx(trx), _allRepricePTFs(&allRepricePTFs), _dc(nullptr), _itinIndex(itinIndex),
    _mapper(grm), _changeVendorForDFFFares(checkIfOverrideNeeded(excFarePath))
{
}

namespace
{
class NotSameOwnerFare : public std::unary_function<const PaxTypeFare*, bool>
{
  const CarrierCode& _excPtfCxr;

public:
  NotSameOwnerFare(const PaxTypeFare& excPtf)
    : _excPtfCxr(excPtf.carrier() == INDUSTRY_CARRIER ? excPtf.fareMarket()->governingCarrier()
                                                      : excPtf.carrier())
  {
  }

  bool operator()(const PaxTypeFare* newPtf) const
  {
    return (newPtf->carrier() == INDUSTRY_CARRIER ? newPtf->fareMarket()->governingCarrier()
                                                  : newPtf->carrier()) != _excPtfCxr;
  }
};

class T988DoNotExist : public std::unary_function<const ProcessTagInfo*, bool>
{
public:
  bool operator()(const ProcessTagInfo* pti) const { return !pti->reissueSequence()->orig(); }
};
}

bool
RexFareBytesValidator::validate(FareBytesData& data, PaxTypeFare& ptf)
{
  if (std::find_if(data.processTags().begin(), data.processTags().end(), T988DoNotExist()) !=
      data.processTags().end())
    return true;

  RepriceFareValidationResult result = allPtisFailed(data, ptf);
  if (result != REPRICE_PASS)
  {
    _trx.fareFailFareBytes() = true;
    printDiag(result, ptf, data.processTags());
    return false;
  }

  return true;
}

bool
RexFareBytesValidator::anyFarePassed(const ProcessTagInfo& pti,
                                     std::vector<const PaxTypeFare*> ptfs)
{
  for (const PaxTypeFare* newPtf : ptfs)
    if (validate(pti, *newPtf))
      return true;

  return false;
}

bool
RexFareBytesValidator::allPtisFailed(const std::set<ProcessTagInfo*>& ptis,
                                     const PaxTypeFare& newPtf,
                                     FareByteChecker checkFareByte)
{
  for (const ProcessTagInfo* pti : ptis)
    if (checkFareByte(this, *pti, newPtf))
      return false;

  return true;
}

RepriceFareValidationResult
RexFareBytesValidator::allPtisFailed(FareBytesData& data, const PaxTypeFare& newPtf)
{
  std::vector<bool> applyValidation = boost::assign::list_of(data.fareRulesApply())(true)(
      data.fareTrfNumberApply())(data.excludePrivateApply())(data.fareClassCodeApply())(true)(true)(
      data.fareAmountApply())(data.fareNormalSpecialApply())(data.owrtApply())(data.sameApply());

  std::vector<bool>::const_iterator apply = applyValidation.begin();

  for (const CheckerResultPair& checkerResult : RexFareBytesValidator::_fareByteCheckers)
  {
    if (*apply && allPtisFailed(data.processTags(), newPtf, checkerResult.first))
      return checkerResult.second;

    ++apply;
  }

  return REPRICE_PASS;
}

bool
RexFareBytesValidator::validate(const ProcessTagInfo& pti)
{
  if (_trx.exchangeItin().front()->geoTravelType() != GeoTravelType::International)
  {
    const PaxTypeFare* const mappedPtf = _mapper->getCityMappedPtf(pti);

    if (mappedPtf)
      return validate(pti, *mappedPtf);

    std::vector<const PaxTypeFare*> sameOwnerFares;
    NotSameOwnerFare notSameOwnerFare(*pti.paxTypeFare());
    std::remove_copy_if(_allRepricePTFs->begin(),
                        _allRepricePTFs->end(),
                        std::back_inserter(sameOwnerFares),
                        notSameOwnerFare);

    return anyFarePassed(pti, sameOwnerFares.empty() ? *_allRepricePTFs : sameOwnerFares);
  }

  if (!_mapper->hasInternationalComponent() && pti.fareMarket()->geoTravelType() == GeoTravelType::International)
    return anyFarePassed(pti, *_allRepricePTFs);

  const PaxTypeFare* const mappedPtf = _mapper->getInternationalyMappedPtf(pti);
  return !mappedPtf || validate(pti, *mappedPtf);
}

bool
RexFareBytesValidator::validate(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  RepriceFareValidationResult result = REPRICE_PASS;

  for (const CheckerResultPair& checkerResult : RexFareBytesValidator::_fareByteCheckers)
    if (!checkerResult.first(this, pti, ptf))
    {
      result = checkerResult.second;
      break;
    }

  updateCaches(pti, result);

  if (result != REPRICE_PASS && hasDiagAndFilterPassed())
    _dc->printResultInformation(result, pti, &ptf);

  return result == REPRICE_PASS;
}

bool
RexFareBytesValidator::checkFareRules(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  switch (pti.reissueSequence()->ruleInd())
  {
  case FareRestrictions::NOT_APPLICABLE:
    return true;
  case FareRestrictions::RULE_INDICATOR_RULE_NUMBER:
    return matchWildcardRuleNumber(pti.reissueSequence()->ruleNo(), ptf.ruleNumber());
  case FareRestrictions::RULE_INDICATOR_SAME_RULE:
    return ptf.ruleNumber() == pti.paxTypeFare()->ruleNumber();
  case FareRestrictions::RULE_INDICATOR_RULE_NOT_THE_SAME:
    return ptf.ruleNumber() != pti.paxTypeFare()->ruleNumber();
  }
  return false;
}

bool
RexFareBytesValidator::checkFareCxrApplTbl(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  const CarrierCode& carrier = (ptf.carrier() == INDUSTRY_CARRIER)
                                   ? ptf.fareMarket()->governingCarrier()
                                   : ptf.carrier(); // new

  const CarrierCode& excCarrier = (pti.paxTypeFare()->carrier() == INDUSTRY_CARRIER)
                                      ? pti.paxTypeFare()->fareMarket()->governingCarrier()
                                      : pti.paxTypeFare()->carrier(); // old

  const uint32_t cxrApplTblItemNo = pti.reissueSequence()->fareCxrApplTblItemNo();

  if (carrier == excCarrier && !cxrApplTblItemNo)
      return true;

  if (pti.isOverriden())
  {
    const CarrierCode& overridingExcCarrier =
        (pti.overridingPTF()->carrier() == INDUSTRY_CARRIER)
            ? pti.overridingPTF()->fareMarket()->governingCarrier()
            : pti.overridingPTF()->carrier(); // old overriden;

    if (carrier == overridingExcCarrier && !cxrApplTblItemNo)
        return true;
  }

  static const Indicator CARRIER_ALLOWED = ' ';

  if (cxrApplTblItemNo)
  {
    const std::vector<CarrierApplicationInfo*>& cxrAppl =
        getCarrierApplication(pti.reissueSequence()->vendor(), cxrApplTblItemNo);

    std::vector<CarrierApplicationInfo*>::const_iterator i = cxrAppl.begin();
    std::vector<CarrierApplicationInfo*>::const_iterator ie = cxrAppl.end();
    int score = 0;
    for (; i != ie; i++)
    {
      if ((*i)->carrier() == DOLLAR_CARRIER)
        score = 1;
      else if ((*i)->carrier() == carrier)
      {
        if ((*i)->applInd() == CARRIER_ALLOWED)
          score += 2;
        else
          score -= 2;

        break;
      }
    }
    return score > 0;
  }

  return false;
}

bool
RexFareBytesValidator::checkFareTrfNumber(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  const TariffNumber requiredTrfNumber = pti.reissueSequence()->ruleTariffNo();
  return !requiredTrfNumber || requiredTrfNumber == ptf.tcrRuleTariff() ||
         requiredTrfNumber == ptf.fareClassAppInfo()->_ruleTariff;
}

bool
RexFareBytesValidator::checkFareClassCode(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  return CommonSolutionValidator::checkFareClassCode(
      pti.reissueSequence()->fareTypeInd(), pti.reissueSequence()->fareClass(), ptf);
}

bool
RexFareBytesValidator::checkFareTypeCode(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  return RuleUtil::matchFareType(pti.reissueSequence()->fareType(), ptf.fcaFareType());
}

bool
RexFareBytesValidator::checkFareTypeTable(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  if (pti.reissueSequence()->fareTypeTblItemNo())
  {
    VendorCode vendor = getVendorCodeForFareTypeTbl(pti, _changeVendorForDFFFares);
    const DateTime& applicationDate = ptf.fareMarket()->retrievalDate();
    const std::vector<FareTypeTable*>& fareTypes =
                                _trx.getFareTypeTables(vendor,
                                                       pti.reissueSequence()->fareTypeTblItemNo(),
                                                       applicationDate);
    if (fareTypes.empty() ||
        !CommonSolutionValidator::checkFareTypeTable(fareTypes, ptf.fcaFareType()))
      return false;
  }

  return true;
}

bool
RexFareBytesValidator::checkFareAmount(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  if (pti.processTag() == REISSUE_DOWN_TO_LOWER_FARE)
    return true;

  bool status = false;
  FareAmountComparisonResults results;

  switch (pti.reissueSequence()->fareAmtInd())
  {
  case FareRestrictions::NOT_APPLICABLE:
    status = true;
    break;
  case FareRestrictions::HIGHER_OR_EQUAL_AMOUNT:
    results = getFaresAmountForComparison(pti, ptf, -EPSILON);
    status = std::get<0>(results) > std::get<1>(results) - EPSILON;
    break;
  case FareRestrictions::HIGHER_AMOUNT:
    results = getFaresAmountForComparison(pti, ptf, EPSILON);
    status = std::get<0>(results) > std::get<1>(results) + EPSILON;
    break;
  }

  if (hasDiagAndFilterPassed())
    _dc->printNewFareEqualOrHigherValidation(pti.itemNo(),
                                             pti.seqNo(),
                                             pti.reissueSequence()->fareAmtInd(),
                                             status,
                                             std::get<0>(results),
                                             std::get<1>(results),
                                             std::get<2>(results));

  return status;
}

RexFareBytesValidator::FareAmountComparisonResults
RexFareBytesValidator::getFaresAmountForComparison(const ProcessTagInfo& pti,
                                                   const PaxTypeFare& ptf,
                                                   double signedEpsilon)
{
  FareAmountComparisonResults results{ptf.nucFareAmount(), pti.paxTypeFare()->nucFareAmount(), NUC};
  if (_trx.applyReissueExchange())
  {
    if (ptf.currency() == pti.paxTypeFare()->currency())
    {
      results = std::make_tuple(ptf.fareAmount(), pti.paxTypeFare()->fareAmount(), ptf.currency());
    }
    else if (ptf.nucFareAmount() < pti.paxTypeFare()->nucFareAmount() + signedEpsilon &&
             ptf.rexSecondNucFareAmount() > EPSILON)
    {
      results =
          std::make_tuple(ptf.rexSecondNucFareAmount(), pti.paxTypeFare()->nucFareAmount(), NUC);
    }
  }
  return results;
}

bool
RexFareBytesValidator::checkSameInd(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  switch (pti.reissueSequence()->sameInd())
  {
  case FareRestrictions::SAME_IND_FARE_TYPE:
    return pti.paxTypeFare()->fcaFareType() == ptf.fcaFareType();
  case FareRestrictions::SAME_IND_FARE_CLASS:
    return pti.paxTypeFare()->fareClass() == ptf.fareClass();
  default: // BLANK
    return true;
  }
}

bool
RexFareBytesValidator::checkFareNormalSpecial(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  return CommonSolutionValidator::checkFareNormalSpecial(pti.reissueSequence()->normalspecialInd(),
                                                         ptf);
}

bool
RexFareBytesValidator::checkExcludePrivateIndicator(const ProcessTagInfo& pti,
                                                    const PaxTypeFare& ptf)
{
  const ReissueSequenceW* reissueSeq = pti.reissueSequence();
  const Indicator& excludePrivateIndicator = reissueSeq->excludePrivate();

  if (reissueSeq->ruleTariffNo())
    return true;

  if (fallback::excPrivPubNoAtpFares(&_trx) && ptf.vendor() != ATPCO_VENDOR_CODE)
    return true;

  if (excludePrivateIndicator == BLANK)
  {
    return true;
  }

  return pti.paxTypeFare()->tcrTariffCat() == ptf.tcrTariffCat();
}

bool
RexFareBytesValidator::checkOWRT(const ProcessTagInfo& pti, const PaxTypeFare& ptf)
{
  return CommonSolutionValidator::checkOWRT(pti.reissueSequence()->owrt(), ptf);
}

bool
RexFareBytesValidator::validate(const ProcessTagPermutation& permutation)
{
  if (hasDiagAndFilterPassed())
    *_dc << "VALIDATING FARE RESTRICTIONS\n" << _mapper->internationalDiag().str();

  std::vector<ProcessTagInfo*>::const_iterator ptIter = permutation.processTags().begin();
  std::vector<ProcessTagInfo*>::const_iterator ptIterEnd = permutation.processTags().end();
  for (; ptIter != ptIterEnd; ++ptIter)
  {
    if (!(**ptIter).reissueSequence()->orig())
      continue;

    RepriceFareValidationResult result;
    if (checkCaches(**ptIter, result))
    {
      if (result == REPRICE_PASS)
        continue;

      if (hasDiagAndFilterPassed())
        _dc->printResultInformation(
            result, **ptIter, _mapper->getInternationalyMappedPtf(**ptIter));

      return false;
    }

    if (!validate(**ptIter))
      return false;
  }

  return true;
}

void
RexFareBytesValidator::updateCaches(const ProcessTagInfo& pti, RepriceFareValidationResult result)
{
  const PaxTypeFare* mappedPtf = _mapper->getInternationalyMappedPtf(pti);

  _faresCheckCache.insert(std::make_pair(&pti, result));
  if (mappedPtf)
  {
    FareCompInfo::RepriceFareValidationResultCache& cache =
        pti.fareCompInfo()->repriceValidationCache(_itinIndex);

    FareCompInfo::RepriceFareValidationResultCacheKey key(
        pti.reissueSequence()->overridingWhenExists(), mappedPtf);
    cache.insert(std::make_pair(key, result));
  }
}

bool
RexFareBytesValidator::checkCaches(const ProcessTagInfo& pti,
                                   RepriceFareValidationResult& cachedResult)
{
  // those caches will be using each other results, depending what is already inside
  const PaxTypeFare* mappedPtf = _mapper->getInternationalyMappedPtf(pti);

  std::map<const ProcessTagInfo*, RepriceFareValidationResult>::const_iterator found =
      _faresCheckCache.find(&pti);

  if (found != _faresCheckCache.end())
  {
    cachedResult = found->second;

    if (!mappedPtf)
      return true;
  }

  if (mappedPtf)
  {
    FareCompInfo::RepriceFareValidationResultCache& cache =
        pti.fareCompInfo()->repriceValidationCache(_itinIndex);

    FareCompInfo::RepriceFareValidationResultCacheKey key(
        pti.reissueSequence()->overridingWhenExists(), mappedPtf);
    FareCompInfo::RepriceFareValidationResultCache::iterator foundCacheItem = cache.find(key);
    if (foundCacheItem != cache.end())
    {
      if (found == _faresCheckCache.end())
      {
        cachedResult = foundCacheItem->second;
        _faresCheckCache.insert(std::make_pair(&pti, cachedResult));
      }

      return true;
    }
    else if (found != _faresCheckCache.end())
    {
      cache.insert(std::make_pair(key, cachedResult));
      return true;
    }
  }

  return false;
}

const std::vector<CarrierApplicationInfo*>&
RexFareBytesValidator::getCarrierApplication(const VendorCode& vendor, int itemNo)
{
  return _trx.dataHandle().getCarrierApplication(vendor, itemNo);
}

void
RexFareBytesValidator::printDiag(RepriceFareValidationResult result,
                                 PaxTypeFare& ptf,
                                 std::set<ProcessTagInfo*>& ptiList)
{
  if (_trx.diagnostic().diagnosticType() != Diagnostic602)
    return;

  DCFactory* factory = DCFactory::instance();
  Diag602Collector* dc = dynamic_cast<Diag602Collector*>(factory->create(_trx));
  if (!dc)
    return;

  dc->enable(Diagnostic602);
  dc->initializeFilter(_trx, ptf);

  *dc << ptf;

  std::set<ProcessTagInfo*>::iterator i = ptiList.begin(), ie = ptiList.end();
  for (; i != ie; i++)
    dc->printPtiInfo(result, **i);

  *dc << result;

  dc->flushMsg();
}

bool
RexFareBytesValidator::hasDiagAndFilterPassed() const
{
  return _dc && _dc->filterPassed();
}

bool
RexFareBytesValidator::checkIfOverrideNeeded(const FarePath* farePath) const
{
  if (farePath)
  {
    for (PricingUnit* pu : farePath->pricingUnit())
    {
      for (FareUsage* fu : pu->fareUsage())
      {
        if (!fu->paxTypeFare()->vendor().equalToConst("ATP") &&
            !fu->paxTypeFare()->vendor().equalToConst("SITA"))
        {
          return true;
        }
      }
    }
  }
  return false;
}
VendorCode
RexFareBytesValidator::getVendorCodeForFareTypeTbl(const ProcessTagInfo& pti,
                                                   bool changeVendorForDFFFares)
{
  //override must be done because client cant change this value in DB
  if(fallback::rexFareTypeTbl(pti.paxTypeFare()->fareMarket()->rexBaseTrx()))
  {
    return (changeVendorForDFFFares ?  Vendor::SABRE :
                                        pti.paxTypeFare()->vendor());
  }
  else if (!changeVendorForDFFFares)
  {
    return pti.paxTypeFare()->vendor();
  }
  else
  {
    if (pti.paxTypeFare()->isFareByRule())
    {
      if (pti.paxTypeFare()->fareByRuleInfo().ovrdcat31() == 'B')
        return pti.paxTypeFare()->baseFare()->vendor();
      else
        return pti.paxTypeFare()->vendor();
    }
  }
  return Vendor::SABRE;
}

}
