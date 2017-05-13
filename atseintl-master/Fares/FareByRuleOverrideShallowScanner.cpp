#include "Fares/FareByRuleOverrideShallowScanner.h"

#include "Common/Logger.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "Fares/FareByRuleOverrideCategoryMatcher.h"
#include "Rules/Config.h"
#include "Rules/RuleUtil.h"
#include "Rules/RuleValidationChancelor.h"
#include "Util/BranchPrediction.h"

namespace tse
{
namespace
{
ConfigurableCategories
fbrShallowScanCategories("FBR_SHALLOW_SCAN");
}

static Logger
logger("atseintl.Fares.FareByRuleOverrideShallowScanner");

FareByRuleOverrideShallowScanner::FareByRuleOverrideShallowScanner(
    PricingTrx& trx,
    const FareMarket& fareMarket,
    Itin& itin,
    const FareByRuleApp& fbrApp,
    const FareByRuleCtrlInfo& fbrCtrlInfo,
    const FareByRuleItemInfo& fbrItemInfo,
    PaxTypeFare& dummyPtFare,
    TariffCategory tarrifCategory)
  : _trx(trx),
    _fareMarket(fareMarket),
    _itin(itin),
    _fbrApp(fbrApp),
    _fbrCtrlInfo(fbrCtrlInfo),
    _fbrItemInfo(fbrItemInfo),
    _dummyPtFare(dummyPtFare),
    _tarrifCategory(tarrifCategory),
    _fareMarketDataAccess(trx, &itin, dummyPtFare),
    _skipSecurity((_fbrItemInfo.resultDisplaycatType() == RuleConst::SELLING_FARE) ||
                  (_fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE) ||
                  (_fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE_UPD) ||
                  (_fbrItemInfo.resultDisplaycatType() == BLANK && fareMarket.hasCat35Fare())),
    _trInfoWrapper(nullptr),
    _diag(_trx, _trx.diagnostic().diagnosticType()),
    _isDiagActive((_trx.diagnostic().diagnosticType() == Diagnostic225) &&
                  (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SHALLOWSCAN"))
{
  _fbrShallowScanCategories = fbrShallowScanCategories.read();

  _categoryRuleItemSet.setCategoryPhase(NormalValidation);
  if (UNLIKELY(trx.getTrxType() == PricingTrx::MIP_TRX && trx.isFlexFare()))
  {
    std::shared_ptr<RuleValidationChancelor> chancelor(new RuleValidationChancelor());
    std::shared_ptr<CategoryValidationObserver> resultObserver;

    chancelor->updateContextType(RuleValidationContext::FARE_MARKET);
    chancelor->updateContext(_fareMarketDataAccess);
    chancelor->setPolicy(ELIGIBILITY_RULE, new FlexFaresValidationPolicyNoEligibility());
    chancelor->setPolicy(ADVANCE_RESERVATION_RULE,
                         new FlexFaresValidationPolicyNoAdvancePurchase());
    chancelor->setPolicy(MINIMUM_STAY_RULE, new FlexFaresValidationPolicyNoMinMax());
    chancelor->setPolicy(MAXIMUM_STAY_RULE, new FlexFaresValidationPolicyNoMinMax());
    chancelor->setPolicy(PENALTIES_RULE, new FlexFaresValidationPolicyNoPenalties());
    resultObserver = std::shared_ptr<CategoryValidationObserver>(
        new CategoryValidationObserver(chancelor->getMutableMonitor()));
    _categoryRuleItemSet.categoryRuleItem().ruleItem().setChancelor(chancelor);
  }

  _rpData.soInfoWrapper(&_soInfoWrapper);
  _rpData.trInfoWrapper(&_trInfoWrapper);
}

bool
FareByRuleOverrideShallowScanner::isValid()
{
  if (UNLIKELY(_isDiagActive))
  {
    _diag << "---------------------------------------------------------------\n";
    _diag << "SHALLOW SCAN RESULTS \n";
    _diag << "  CAT25 REC3: ";
    _diag << _fbrItemInfo.itemNo();
    _diag << "\n";
  }

  for (std::vector<uint16_t>::const_iterator it = _fbrShallowScanCategories.begin();
       it != _fbrShallowScanCategories.end();
       ++it)
  {
    bool checkFBRCategory = false;
    bool checkBaseFareCategory = false;
    RuleUtil::determineRuleChecks(*it, _fbrItemInfo, checkFBRCategory, checkBaseFareCategory);

    if (checkFBRCategory && !performShallowScan(*it))
    {
      LOG4CXX_DEBUG(logger,
                    "discarding cat25 rec3 itemNo "
                        << _fbrItemInfo.itemNo() << ", failed cat " << *it << "\n"
                        << ", vendor: " << _fbrApp.vendor() << "\n"
                        << ", tariff: " << _fbrApp.ruleTariff() << "\n"
                        << ", carrier:" << _fbrApp.carrier() << "\n"
                        << ", rule: " << _fbrApp.ruleNo() << "\n"
                        << ", skip security: " << (_skipSecurity ? "T" : "F") << "\n"
                        << ", display category type: " << _fbrItemInfo.resultDisplaycatType());

      if (UNLIKELY(_isDiagActive))
        _diag << "  SHALLOW SCAN FAILED\n";
      return false;
    }
  }
  LOG4CXX_DEBUG(logger, "passed cat25 rec3 itemNo " << _fbrItemInfo.itemNo());

  if (UNLIKELY(_isDiagActive))
    _diag << "  SHALLOW SCAN PASSED\n";
  return true;
}

bool
FareByRuleOverrideShallowScanner::performShallowScan(uint16_t categoryNumber)
{
  bool unconditionalInSeq = false;
  FareByRuleOverrideCategoryMatcher matcher(_trx, _fbrItemInfo, _dummyPtFare);

  const std::vector<GeneralFareRuleInfo*>& allRec2 = getAllPossiblyMatchingRec2s(categoryNumber);

  if (categoryNumber == 15 && allRec2.empty())
    return true;

  for (const auto fri : allRec2)
  {
    Rec2Wrapper* matchedRec2 = matcher.tryMatchRec2(*fri);
    if (matchedRec2)
    {
      if (UNLIKELY(_isDiagActive))
      {
        _diag << "  MATCHED CAT ";
        _diag << categoryNumber;
        _diag << " REC2 SEQNO ";
        _diag << matchedRec2->getRec2().sequenceNumber();
        _diag << (matchedRec2->isConditional() ? " COND." : " UNCOND.");
      }
      if (validate(*matchedRec2)) // there can be a valid fbr fare from this c25 rec3,
      {
        if (UNLIKELY(_isDiagActive))
          _diag << " :PASSED\n";
        return true; // continue with fare creation
      }

      if (UNLIKELY(_isDiagActive))
        _diag << " :FAILED\n";

      if (!matchedRec2->isConditional())
      {
        unconditionalInSeq = true;
        break;
      }
    }
  }

  if (!unconditionalInSeq)
    return passSystemAssumption(categoryNumber);

  return false; // no possible fbr fare from this c25 rec3
}

const std::vector<GeneralFareRuleInfo*>&
FareByRuleOverrideShallowScanner::getAllPossiblyMatchingRec2s(uint16_t categoryNumber)
{
  return _trx.dataHandle().getGeneralFareRule(_fbrApp.vendor(),
                                              _fbrApp.carrier(),
                                              _fbrApp.ruleTariff(),
                                              _fbrApp.ruleNo(),
                                              categoryNumber,
                                              _fareMarket.travelDate());
}

bool
FareByRuleOverrideShallowScanner::validate(const Rec2Wrapper& rec2wrapper)
{
  Record3ReturnTypes ret =
      _categoryRuleItemSet.process(_trx,
                                   _itin,
                                   rec2wrapper.getRec2(),
                                   _dummyPtFare,
                                   rec2wrapper.getRec2().categoryRuleItemInfoSet(),
                                   _rpData,
                                   rec2wrapper.isLocationSwapped(),
                                   true,
                                   _skipSecurity,
                                   _fareMarketDataAccess);

  return ret != FAIL;
}

bool
FareByRuleOverrideShallowScanner::passSystemAssumption(uint16_t categoryNumber)
{
  bool retVal =
      !((categoryNumber == 35 && _skipSecurity && _tarrifCategory == RuleConst::PRIVATE_TARIFF) ||
        (categoryNumber == 15 && !_skipSecurity && _tarrifCategory == RuleConst::PRIVATE_TARIFF));

  if (UNLIKELY(_isDiagActive))
  {
    _diag << "  CAT ";
    _diag << categoryNumber;
    _diag << " SYSTEM ASSUMPTION ";
    _diag << (retVal ? "PASSED\n" : "FAILED\n");
  }
  return retVal;
}
}
