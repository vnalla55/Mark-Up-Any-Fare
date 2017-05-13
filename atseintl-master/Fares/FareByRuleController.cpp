//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#include "Fares/FareByRuleController.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/DiagMonitor.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/Logger.h"
#include "Common/Memory/GlobalManager.h"
#include "Common/PaxTypeUtil.h"
#include "Common/Rec2Selector.h"
#include "Common/RoutingUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/BaseFareRule.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "DBAccess/Record2Types.h"
#include "Diagnostic/Diag208Collector.h"
#include "Diagnostic/Diag225Collector.h"
#include "Diagnostic/Diag325Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/AvailabilityChecker.h"
#include "Fares/FareByRuleAppValidator.h"
#include "Fares/FareByRuleOverrideShallowScanner.h"
#include "Fares/FareByRuleProcessingInfo.h"
#include "Fares/FareByRuleValidator.h"
#include "Fares/FareCollectorOrchestrator.h"
#include "Fares/FareTypeMatcher.h"
#include "Fares/IndustryFareController.h"
#include "Fares/NegFareSecurity.h"
#include "Routing/RoutingConsts.h"
#include "Rules/FareMarketDataAccess.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <bitset>
#include <string>
#include <utility>
#include <vector>

namespace tse
{
const char* FareByRuleController::BLANK_CURRENCY("***");
const char* FareByRuleController::MILEAGE_ROUTING("0000");

FALLBACK_DECL(fallbackCat35If1Prevalidation);
FALLBACK_DECL(cat25baseFarePrevalidation);
FALLBACK_DECL(fallbackFootNoteR2Optimization);
FALLBACK_DECL(skipBkcValidationForFlownFM);
FALLBACK_DECL(shallowScanShoppingHist);
FALLBACK_DECL(shallowScanHistorical);

static Logger
logger("atseintl.Fares.FareByRuleController");

FareByRuleController::FareByRuleController(PricingTrx& trx,
                                           const FareCollectorOrchestrator* fco,
                                           Itin& itin,
                                           FareMarket& fareMarket)
  : FareController(trx, itin, fareMarket),
    _creator(trx, itin, fareMarket),
    _diag325Requested(_trx.diagnostic().diagnosticType() == Diagnostic325),
    _diagWithRuleNumber(!_trx.diagnostic().diagParamMapItem(Diagnostic::RULE_NUMBER).empty()),
    _fco(fco)
{
  _diagWithR3Cat25 =
      _diag325Requested && _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "REC3";
  _diagWithT989 =
      _diag325Requested && _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "T989";
  _calcMoney.configForCat25();

  _fallbackCat25baseFarePrevalidation = fallback::cat25baseFarePrevalidation(&trx);

  if (_fallbackCat25baseFarePrevalidation)
    _fareMarket.setCanValidateFBRBaseFares(false);
  else
  {
    // Do base fare validation only for unique faremarkets and only for
    // non exchange, non altdates MIP and Pricing. Skip for SFR and reprice trx;
    // Historical is also skipped due to existing bug in rule controller logic
    // that needs to be in advance.
    _fareMarket.setCanValidateFBRBaseFares(!_fareMarket.hasDuplicates() &&
         !trx.dataHandle().isHistorical() &&
         trx.getTrxType() != PricingTrx::REPRICING_TRX &&
         trx.excTrxType() == PricingTrx::NOT_EXC_TRX &&
         !trx.isAltDates() && !trx.getRequest()->isSFR() &&
           (trx.getTrxType() == PricingTrx::MIP_TRX ||
            (trx.getTrxType() == PricingTrx::PRICING_TRX && !_fareMarket.isCmdPricing())));
  }
}

// overrides to external services
const PaxTypeInfo*
FareByRuleController::getPaxTypeInfo(const BaseFareRule& baseFareRule) const
{
  return _trx.dataHandle().getPaxType(baseFareRule.basepsgType(), baseFareRule.vendor());
}

void
FareByRuleController::getPricingCurrency(const NationCode& nation, CurrencyCode& currency) const
{
  CurrencyUtil::getPricingCurrency(nation, currency, _itin.travelDate());
}

bool
FareByRuleController::matchNationCurrency(const NationCode& nation, const CurrencyCode& currency)
    const
{
  return CurrencyUtil::matchNationCurrency(nation, currency, _itin.travelDate());
}

const FareByRuleItemInfo*
FareByRuleController::getFareByRuleItem(const VendorCode& vendor, int itemNo) const
{
  return _trx.dataHandle().getFareByRuleItem(vendor, itemNo);
}

const std::vector<const BaseFareRule*>&
FareByRuleController::getBaseFareRule(const VendorCode& vendor, int itemNo) const
{
  return _trx.dataHandle().getBaseFareRule(vendor, itemNo, _itin.travelDate());
}

const Loc*
FareByRuleController::getLoc(const LocCode& locCode) const
{
  return _trx.dataHandle().getLoc(locCode, _itin.travelDate());
}

const LocCode
FareByRuleController::getMultiTransportCity(const LocCode& locCode) const
{
  return _trx.dataHandle().getMultiTransportCity(locCode);
}

const std::vector<tse::FareByRuleApp*>&
FareByRuleController::getFareByRuleApp(const std::string& corpId,
                                       const AccountCode& accountCode,
                                       const TktDesignator& tktDesignator,
                                       std::vector<PaxTypeCode>& paxTypes) const
{
  return _trx.dataHandle().getFareByRuleApp(_fareMarket.governingCarrier(),
                                            corpId,
                                            accountCode,
                                            tktDesignator,
                                            _itin.travelDate(),
                                            paxTypes);
}

const std::vector<const IndustryFareAppl*>*
FareByRuleController::getIndustryFareAppl(Indicator selectionType) const
{
  return &_trx.dataHandle().getIndustryFareAppl(
      selectionType, _fareMarket.governingCarrier(), _itin.travelDate());
}

const CarrierPreference*
FareByRuleController::getCarrierPreference(const CarrierCode& carrier) const
{
  return _trx.dataHandle().getCarrierPreference(carrier, _itin.travelDate());
}

void
FareByRuleController::putIntoPTF(PaxTypeFare& ptf, FareInfo& fareInfo)
{
  _calcMoney.putIntoPTF(ptf, fareInfo);
}

bool
FareByRuleController::fareByRuleAppValidate(FareByRuleAppValidator& validator,
                                            FareByRuleProcessingInfo& fbrProcessingInfo,
                                            std::map<std::string, bool>& ruleTariffMap) const
{
  return validator.isValid(fbrProcessingInfo, ruleTariffMap);
}

bool
FareByRuleController::getFareByRuleCtrlInfo(FareByRuleApp& fbrApp,
                                            FareByRuleCtrlInfoVec& fbrCtrlInfoVec,
                                            DiagManager& diagManager)
{
  return RuleUtil::getFareByRuleCtrlInfo(_trx, fbrApp, _fareMarket, fbrCtrlInfoVec, diagManager);
}

FareInfo*
FareByRuleController::createFareInfo(const MoneyAmount& fareAmt,
                                     const CurrencyCode& currency,
                                     const DateTime& effDate,
                                     const DateTime& expDate)
{
  return _creator.createFareInfo(fareAmt, currency, _fmOrigCurr, _fmDestCurr, effDate, expDate);
}

void
FareByRuleController::createFBRPaxTypeFareRuleData(PaxTypeFare& ptf, PaxTypeFare* baseFare)
{
  _creator.createFBRPaxTypeFareRuleData(ptf, _baseFareInfoBkcAvailMap, _isMinMaxFare, baseFare);
}

uint16_t
FareByRuleController::getTPM(const Loc& market1,
                             const Loc& market2,
                             const GlobalDirection& glbDir,
                             const DateTime& tvlDate) const
{
  return LocUtil::getTPM(market1, market2, glbDir, tvlDate, _trx.dataHandle());
}

void
FareByRuleController::publishedFaresStep(FareMarket& targetMarket) const
{
  if (isFdTrx())
    _fco->publishedFaresStepFareDisplay(*_fdTrx, _itin, targetMarket, true);
  else
    _fco->publishedFaresStep(_trx, _itin, targetMarket, true);
}

// end of overides

// ----------------------------------------------------------------------------
// @function bool FareByRuleController::process
//
// Description:  This function processes Fare by Rule for the fare market.
//               The process starts from Record 8 - Fare by Rule Application.
//               Examine all Record 8s that match the passenger and itinerary
//               information. Then retrieve and validate Record 2 Cat 25 and
//               Record 3 Cat 25 to create Fare by Rule fares for the fare market.
//
// @return bool
//        -false, if any error occurs.
//        -true otherwise.
// ----------------------------------------------------------------------------
bool
FareByRuleController::process()
{
  LOG4CXX_DEBUG(logger, "process() started.");
  const DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();

  if (UNLIKELY((diagType == Diagnostic208 || diagType == Diagnostic225 || _diag325Requested) &&
               !_trx.diagnostic().shouldDisplay(_fareMarket)))
  {
    LOG4CXX_DEBUG(logger, "process() completed.");
    return true;
  }

  DiagManager diag(_trx, diagType);

  if (UNLIKELY(diagType == Diagnostic208 || diagType == Diagnostic225))
  {
    diag << _fareMarket;
  }

  if (!Memory::changesFallback)
  {
    Memory::GlobalManager::instance()->checkTrxMemoryLimits(_trx);
  }
  else
  {
    TrxUtil::checkTrxMemoryAborted(_trx, 1, 1, 1);
  }

  _spanishDiscountApplied = LocUtil::isSpain(*(_trx.getRequest()->ticketingAgent()->agentLocation())) &&
                            LocUtil::isWholeTravelInSpain(_trx.travelSeg());

  if (UNLIKELY(_spanishDiscountApplied &&
               _trx.getOptions()->getSpanishLargeFamilyDiscountLevel() !=
                   SLFUtil::DiscountLevel::NO_DISCOUNT))
  {
    _spanishLargeFamilyDiscountApplied = true;
    _largeFamilyDiscountPercent = SLFUtil::getDiscountPercent(*_trx.getOptions());
  }

  std::vector<FareByRuleApp*> filteredFBRApp;
  const std::vector<FareByRuleApp*>& fbrAppList = getFareByRuleApp(filteredFBRApp);

  if (UNLIKELY(diagType == Diagnostic325))
  {
    Diag325Collector* diag325 = dynamic_cast<Diag325Collector*>(&diag.collector());
    diag325->writeHeader(_fareMarket,
                         *_fareMarket.paxTypeCortege().front().actualPaxType().front());
  }

  std::map<std::string, bool> ruleTariffMap;

  for (const auto fareByRuleApp : fbrAppList)
    processFareByRuleApp(fareByRuleApp, diag, ruleTariffMap);

  updateFareMarket();
  LOG4CXX_DEBUG(logger, "process() completed.");
  return true;
}

void
FareByRuleController::updateFareMarket()
{
  std::vector<PaxTypeFare*>& allFares = _fareMarket.allPaxTypeFare();

  for (const auto paxTypeFare : _fbrPaxTypeFares)
  {
    allFares.push_back(paxTypeFare);
    addFareToPaxTypeBucket(*paxTypeFare);
  }

  _fbrPaxTypeFares.clear();
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleController::processRule
//
// Description:  This function processes Record 2 Category 25 stringing.
//
// @param  fbrApp        A reference to the FareByRuleApp object
// @param  crInfo        A reference to the FareByRuleCtrlInfo object
//
// @return bool
//        -false, if any error occurs.
//        -true otherwise.
// ----------------------------------------------------------------------------
bool
FareByRuleController::processRule(FareByRuleProcessingInfo& fbrProcessingInfo,
                                  const FareByRuleCtrlInfo& crInfo,
                                  bool isLocationSwapped)
{
  DiagManager& diag = *fbrProcessingInfo.diagManager();
  // Save SOFTPASS condition during cat25 fare creation when:
  //  - any qualifier category
  //    or
  //  - categotyRuleItemInfo has a directionality != BLANK        &&
  //                                              != LOC1_TO_LOC2 &&
  //                                              != LOC2_TO_LOC1
  //    or
  //  - Cat25 was created by "ELSE" relation
  bool isSoftPassFor25 = false;
  const std::vector<CategoryRuleItemInfoSet*>& ruleItemInfoSets = crInfo.categoryRuleItemInfoSet();
  std::vector<CategoryRuleItemInfoSet*>::const_iterator m = ruleItemInfoSets.begin();
  std::vector<CategoryRuleItemInfoSet*>::const_iterator n = ruleItemInfoSets.end();

  if (UNLIKELY(fbrProcessingInfo.fbrApp()->carrier() == INDUSTRY_CARRIER && !_multiAppls && !_indAppls))
  {
    return false;
  }

  // During Cat 25 fare creation, there is no fare yet.
  // So we need dummy PaxTypeFare for some rules validation.
  //
  // QT: moved here outside of the loop.
  FareInfo fareInfo;
  fareInfo.carrier() = fbrProcessingInfo.fbrApp()->carrier();
  fareInfo.vendor() = fbrProcessingInfo.fbrApp()->vendor();
  fareInfo.vendorFWS() = fbrProcessingInfo.fbrApp()->vendorFWS();

  bool reverseDirection = false;

  if (_fareMarket.boardMultiCity() < _fareMarket.offMultiCity())
  {
    fareInfo.market1() = _fareMarket.boardMultiCity();
    fareInfo.market2() = _fareMarket.offMultiCity();
  }
  else
  {
    fareInfo.market1() = _fareMarket.offMultiCity();
    fareInfo.market2() = _fareMarket.boardMultiCity();
    reverseDirection = true;
  }

  TariffCrossRefInfo tariffCrossRefInfo;
  tariffCrossRefInfo.tariffCat() = fbrProcessingInfo.tcrInfo()->tariffCat();
  Fare fare;
  fare.initialize(Fare::FS_PublishedFare, &fareInfo, _fareMarket, &tariffCrossRefInfo);
  if (reverseDirection)
    fare.status().set(Fare::FS_ReversedDirection);

  FareClassAppSegInfo fareClassAppSegInfo;
  fareClassAppSegInfo._paxType = fbrProcessingInfo.fbrApp()->primePaxType();
  fareClassAppSegInfo._minAge = 0;
  fareClassAppSegInfo._maxAge = 0;
  PaxTypeFare ptFare;
  ptFare.initializeFlexFareValidationStatus(_trx);
  ptFare.fareClassAppSegInfo() = &fareClassAppSegInfo;
  PaxType* actualPaxType = const_cast<PaxType*>(PaxTypeUtil::isAnActualPaxInTrx(
      _trx, fbrProcessingInfo.fbrApp()->carrier(), fbrProcessingInfo.fbrApp()->primePaxType()));

  ptFare.setFare(&fare);
  ptFare.fareMarket() = &_fareMarket;
  ptFare.actualPaxType() = actualPaxType;

  // To prevent crash in Eligibility when Record 8 primepaxtype does not exist in the Pricing entry
  if (ptFare.actualPaxType() == nullptr)
  {
    return false;
  }

  ptFare.status().set(PaxTypeFare::PTF_FareByRule);

  std::vector<CarrierCode> inputValCxrList = _fareMarket.validatingCarriers();

  for (; m != n; ++m)
  {
    CategoryRuleItemInfoSet* ruleItemInfoSet = *m;
    isSoftPassFor25 = false;
    _qMatchCorpID = false;
    _qMatchNationFR = false;
    _qEtktWarning = false;
    _qNewAcctCode.clear();

    if (UNLIKELY(ruleItemInfoSet == nullptr))
    {
      LOG4CXX_DEBUG(logger, "FBRC::processRule() - Skipping null CategoryRuleItemInfoSet pointer");
      continue;
    }

    // Special logic for Fare Display transactions
    if (UNLIKELY(isFdTrx()))
    {
      // Initialize temporary FareDisplayInfo object
      if (!FareDisplayUtil::initFareDisplayInfo(_fdTrx, ptFare))
      {
        LOG4CXX_DEBUG(logger, "FBRC::processRule() - Unable to init FareDisplayInfo");
        return false;
      }
    }

    std::vector<CategoryRuleItemInfo>* segQual = nullptr;
    _trx.dataHandle().get(segQual);
    std::vector<CategoryRuleItemInfo>::iterator o =
        (*ruleItemInfoSet).begin();
    std::vector<CategoryRuleItemInfo>::iterator p = (*ruleItemInfoSet).end();
    processRuleItemInfoSet(*ruleItemInfoSet, *segQual, o, p);
    bool qualifierPasses = true;
    std::vector<CarrierCode> validValCxrList;

    if (!segQual->empty())
    {
      isSoftPassFor25 = true; // the current set has a qualified
      VendorCode& vendor = fbrProcessingInfo.fbrApp()->vendor();

      // Validate the if conditions
      if (!validateQualifiers(
              ptFare, *segQual, vendor, inputValCxrList, validValCxrList, &crInfo,
              (diag.isActive() ? &diag.collector() : nullptr)))
      {
        qualifierPasses = false;
      }
      fbrProcessingInfo.validatingCarriers() = validValCxrList;

      if (ptFare.matchedCorpID())
      {
        _qMatchCorpID = true;
        _qNewAcctCode = ptFare.matchedAccCode();
      }

      if (UNLIKELY(ptFare.fare()->isNationFRInCat15()))
      {
        _qMatchNationFR = true;
      }

      if (UNLIKELY(ptFare.fare()->isWarningEtktInCat15())) // is populated in Cat15 only for the pricing entry
      {
        _qEtktWarning = true;
      }

      if (_trx.isShopping() && (!ptFare.qualifyFltAppRuleDataMap().empty()))
      {
        _qualifyFltAppRuleDataMap = ptFare.qualifyFltAppRuleDataMap();
      }
    }

    o = (*ruleItemInfoSet).begin();

    // Now loop through all the records before the if and build
    // Cat 25 fares
    for (; o != p; o++)
    {
      Record3ReturnTypes retCode = SKIP;

      if (checkQualifierPassed(&(*o), diag, qualifierPasses))
      {
        if (UNLIKELY(!processCategoryRuleItemInfo(&(*o),
                                                  ruleItemInfoSet,
                                                  segQual,
                                                  fbrProcessingInfo,
                                                  crInfo,
                                                  ptFare,
                                                  isLocationSwapped,
                                                  diag,
                                                  isSoftPassFor25,
                                                  retCode)))
        {
          break;
        }
      }

      diagDisplayRelation(&(*o), diag, retCode);

      if (UNLIKELY(retCode == STOP && !isFdTrx() && !isSoftPassFor25))
        return true;
    }
  }

  return true;
}

// return true if we want to process other CategoryRuleItemInfo, otherwise false
bool
FareByRuleController::processCategoryRuleItemInfo(CategoryRuleItemInfo* catRuleItemInfo,
                                                  CategoryRuleItemInfoSet* ruleItemInfoSet,
                                                  std::vector<CategoryRuleItemInfo>* segQual,
                                                  FareByRuleProcessingInfo& fbrProcessingInfo,
                                                  const FareByRuleCtrlInfo& crInfo,
                                                  PaxTypeFare& dummyPtFare,
                                                  bool isLocationSwapped,
                                                  DiagManager& diag,
                                                  bool& isSoftPassFor25,
                                                  Record3ReturnTypes& retCode)
{
  Diagnostic& trxDiag = _trx.diagnostic();
  FareByRuleValidator fbrValidator;

  // We want to process the then's, or's and else's
  if (UNLIKELY((catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::IF) ||
      (catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::AND)))
  {
    LOG4CXX_DEBUG(logger, "FBRC::process() - Skipping unapplicable CategoryRuleItemInfo pointer");
    return false;
  }

  if (trxDiag.diagnosticType() == Diagnostic325)
  {
    Diag325Collector* diag325 = dynamic_cast<Diag325Collector*>(&diag.collector());
    diag325->diag325Collector(catRuleItemInfo, Diag325Collector::R2_PASS);
  }

  if ((catRuleItemInfo->directionality() != RuleConst::ALWAYS_APPLIES &&
       catRuleItemInfo->directionality() != RuleConst::FROM_LOC1_TO_LOC2 &&
       catRuleItemInfo->directionality() != RuleConst::TO_LOC1_FROM_LOC2) ||
      catRuleItemInfo->inOutInd() != RuleConst::ALWAYS_APPLIES ||
      catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::ELSE ||
      fbrProcessingInfo.fbrApp()->directionality() != RuleConst::ALWAYS_APPLIES)
  {
    isSoftPassFor25 = true;
  }

  bool directionCode = false;

  if (UNLIKELY(isFdTrx()))
  {
    directionCode = isDirectionPassForFD(*catRuleItemInfo, isLocationSwapped);
  }
  else
  {
    directionCode = isDirectionPass(*catRuleItemInfo, isLocationSwapped);
  }

  if (directionCode == false)
  {
    if (trxDiag.diagnosticType() == Diagnostic325)
    {
      Diag325Collector* diag325 = dynamic_cast<Diag325Collector*>(&diag.collector());
      diag325->diag325Collector(catRuleItemInfo, Diag325Collector::R2_FAIL_DIR);
    }

    return true;
  }

  const FareByRuleItemInfo* fbrItemInfo =
      getFareByRuleItem(crInfo.vendorCode(), catRuleItemInfo->itemNo());

  if (!fbrItemInfo)
  {
    LOG4CXX_ERROR(RuleUtil::_dataErrLogger,
                  "VENDOR=" << crInfo.vendorCode() << " CAT 25 RECORD3 ITEM "
                            << catRuleItemInfo->itemNo() << " MISSING");
  }

  fbrProcessingInfo.fbrItemInfo() = fbrItemInfo;
  fbrProcessingInfo.fbrCtrlInfo() = &crInfo;
  fbrProcessingInfo.isSoftPassFor25() = isSoftPassFor25;
  fbrProcessingInfo.isResidenceFiledInR3Cat25() = false;
  _creator.initCreationData(
      &fbrProcessingInfo, segQual, catRuleItemInfo, ruleItemInfoSet, isLocationSwapped);
  retCode = fbrValidator.validate(fbrProcessingInfo);

  if (PASS == retCode)
  {
    processFare(fbrProcessingInfo, dummyPtFare);
  }

  return true;
}

bool
FareByRuleController::findOtherFares(const FareByRuleItemInfo& fbrItemInfo,
                                     const std::vector<PaxTypeFare*>*& otherFares,
                                     bool& sorted)
{
  FareMarket* targetMarket;

  if (carriersMatch(fbrItemInfo.carrier(), _fareMarket.governingCarrier()))
  {
    targetMarket = &_fareMarket;
    sorted = false;
  }
  else
  {
    targetMarket = cloneFareMarket(fbrItemInfo.carrier());
    publishedFaresStep(*targetMarket);
    sorted = true;
  }

  otherFares = findFaresFromPaxType(*targetMarket);
  // true if something in vector
  return (otherFares != nullptr && otherFares->size() != 0);
}

//----------------------------------------------------------------------------
// @function void FareByRuleController::processFare
//----------------------------------------------------------------------------
void
FareByRuleController::processFare(FareByRuleProcessingInfo& fbrProcessingInfo,
                                  PaxTypeFare& dummyPtFare)
{
  const FareByRuleItemInfo& fbrItemInfo = *fbrProcessingInfo.fbrItemInfo();
  _displayFareHdr = true;

  if (UNLIKELY((fbrItemInfo.fareInd() == FareByRuleItemInfo::SPECIFIED ||
                fbrItemInfo.fareInd() == FareByRuleItemInfo::SPECIFIED_K ||
                fbrItemInfo.fareInd() == FareByRuleItemInfo::SPECIFIED_E ||
                fbrItemInfo.fareInd() == FareByRuleItemInfo::SPECIFIED_F) &&
                fbrItemInfo.specifiedCur1() == BLANK_CURRENCY && fbrItemInfo.specifiedFareAmt1() == 0.0 &&
                fbrItemInfo.specifiedFareAmt2() == 0.0))
  {
    // Handle special case for Curr "***" with Fare Amount 0.0
    processSpecifiedFareWithBlankCurrency(fbrProcessingInfo);
  }
  else
  {
    switch (fbrItemInfo.fareInd())
    {
    case 'K':
      processSpecifiedFare_K(fbrProcessingInfo);
      break;

    case 'E':
      processSpecifiedFare_EF(fbrProcessingInfo, getMileage(_fareMarket, _trx));
      break;

    case 'F':
      processSpecifiedFare_EF(fbrProcessingInfo,
                              getTPM(*_fareMarket.origin(),
                                     *_fareMarket.destination(),
                                     _fareMarket.getGlobalDirection(),
                                     _itin.travelDate()));
      break;

    case 'S':
      processSpecifiedFare(fbrProcessingInfo);
      break;

    default:
      if (!fallback::fallbackCat35If1Prevalidation(&_trx) &&
          !runCat35If1PreValidation(fbrProcessingInfo, dummyPtFare))
        return;

      bool runForHistorical = !fallback::shallowScanHistorical(&_trx) &&
                              _trx.getTrxType() != PricingTrx::MIP_TRX &&
                              _trx.isRexBaseTrx() &&
                              static_cast<RexBaseTrx&>(_trx).trxPhase() ==
                                                                RexPricingTrx::PRICE_NEWITIN_PHASE;

      bool runForShopHist = !fallback::shallowScanShoppingHist(&_trx) &&
                            _trx.getTrxType() == PricingTrx::MIP_TRX &&
                            _trx.isExchangeTrx() &&
                            static_cast<RexBaseTrx&>(_trx).trxPhase() ==
                                                                RexPricingTrx::PRICE_NEWITIN_PHASE;

      bool runShallowScanner = ((_trx.getTrxType() == PricingTrx::MIP_TRX ||
                                _trx.getTrxType() == PricingTrx::IS_TRX) &&
                               _trx.isNotExchangeTrx()) || runForHistorical || runForShopHist;

      if (runShallowScanner)
      {
        FareByRuleOverrideShallowScanner fbrOverrideShallowScanner(
            _trx,
            _fareMarket,
            _itin,
            *fbrProcessingInfo.fbrApp(),
            *fbrProcessingInfo.fbrCtrlInfo(),
            fbrItemInfo,
            dummyPtFare,
            fbrProcessingInfo.tcrInfo()->tariffCat());
        if (!fbrOverrideShallowScanner.isValid())
          return;
      }

      processCalculatedFare(fbrProcessingInfo);
      break;
    }
  }
}

void
FareByRuleController::processSpecifiedFareWithBlankCurrency(
    FareByRuleProcessingInfo& fbrProcessingInfo)
{
  MoneyAmount fareAmount = fbrProcessingInfo.fbrItemInfo()->specifiedFareAmt1();

  if (!_fmCurrency.empty())
  {
    // Only create one fare with FM pricing currency
    if (!createSpecifiedFare(fbrProcessingInfo, fareAmount, _fmCurrency))
    {
      LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - createSpecifiedFare failed");
    }
  }
  else
  {
    // Create one fare with FM origin/dest pricing currency
    if (!createSpecifiedFare(fbrProcessingInfo, fareAmount, _fmOrigCurr) ||
        !createSpecifiedFare(fbrProcessingInfo, fareAmount, _fmDestCurr))
    {
      LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - createSpecifiedFare failed");
    }
  }
}

void
FareByRuleController::processSpecifiedFare_K(FareByRuleProcessingInfo& fbrProcessingInfo)
{
  MoneyAmount fareAmount = fbrProcessingInfo.fbrItemInfo()->specifiedFareAmt1();

  if (UNLIKELY(!createSpecifiedFare(
                   fbrProcessingInfo, fareAmount, fbrProcessingInfo.fbrItemInfo()->specifiedCur1())))
  {
    LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - createSpecifiedFare_K failed");
  }
}

void
FareByRuleController::processSpecifiedFare_EF(FareByRuleProcessingInfo& fbrProcessingInfo,
                                              uint16_t mileage)
{
  MoneyAmount fareAmount = 0;
  const FareByRuleItemInfo& fbrItemInfo = *fbrProcessingInfo.fbrItemInfo();
  CurrencyCode specifiedCur1 = fbrItemInfo.specifiedCur1();
  CurrencyCode specifiedCur2 = fbrItemInfo.specifiedCur2();

  if (specifiedCur2.empty())
  {
    fareAmount = fbrItemInfo.specifiedFareAmt1();
    fareAmount = calculateFareAmtPerMileage(fareAmount, mileage);

    if (!createSpecifiedFare(fbrProcessingInfo, fareAmount, specifiedCur1))
    {
      LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - createSpecifiedFare_E/F failed");
    }

    return;
  }

  if (!_fmCurrency.empty())
  {
    if (_fmCurrency == specifiedCur1)
    {
      fareAmount = fbrItemInfo.specifiedFareAmt1();
    }
    else if (_fmCurrency == specifiedCur2)
    {
      fareAmount = fbrItemInfo.specifiedFareAmt2();
    }
    else
    {
      LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - specified currency not matched");
      return;
    }

    fareAmount = calculateFareAmtPerMileage(fareAmount, mileage);

    if (!createSpecifiedFare(fbrProcessingInfo, fareAmount, _fmCurrency))
    {
      LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - createSpecifiedFare_E/F failed");
    }
  }
  else
  {
    // Create one fare with FM origin pricing currency
    if ((!_fmOrigCurr.empty() && _fmOrigCurr == specifiedCur1) || _fmOrigCurr == specifiedCur2)
    {
      if (_fmOrigCurr == specifiedCur1)
      {
        fareAmount = fbrItemInfo.specifiedFareAmt1();
      }
      else
      {
        fareAmount = fbrItemInfo.specifiedFareAmt2();
      }

      fareAmount = calculateFareAmtPerMileage(fareAmount, mileage);

      if (!createSpecifiedFare(fbrProcessingInfo, fareAmount, _fmOrigCurr))
      {
        LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - createSpecifiedFare_E/F failed");
      }
    }

    // Create one fare with FM dest pricing currency
    if ((!_fmDestCurr.empty() && _fmDestCurr != _fmOrigCurr && (_fmDestCurr == specifiedCur1)) ||
        (_fmDestCurr == specifiedCur2))
    {
      if (_fmDestCurr == specifiedCur1)
      {
        fareAmount = fbrItemInfo.specifiedFareAmt1();
      }
      else
      {
        fareAmount = fbrItemInfo.specifiedFareAmt2();
      }

      fareAmount = calculateFareAmtPerMileage(fareAmount, mileage);

      if (!createSpecifiedFare(fbrProcessingInfo, fareAmount, _fmDestCurr))
      {
        LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - createSpecifiedFare_E/F failed");
      }
    }
  }
}

void
FareByRuleController::processSpecifiedFare(FareByRuleProcessingInfo& fbrProcessingInfo)
{
  const FareByRuleItemInfo& fbrItemInfo = *fbrProcessingInfo.fbrItemInfo();
  FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();
  MoneyAmount fareAmount = 0;
  CurrencyCode specifiedCur1 = fbrItemInfo.specifiedCur1();
  CurrencyCode specifiedCur2 = fbrItemInfo.specifiedCur2();
  bool matchesRecord3 = fareMatchesRecord3(fbrItemInfo, fbrApp);
  MoneyAmount minFareAmount = 0;
  MoneyAmount maxFareAmount = 0;
  // Process Fare Indicator 'S'
  bool matchSpecifiedCur1 = false;

  if (LIKELY(!specifiedCur1.empty()))
  {
    if (matchNationCurrency(_fareMarket.origin()->nation(), specifiedCur1) ||
        matchNationCurrency(_fareMarket.destination()->nation(), specifiedCur1))
    {
      matchSpecifiedCur1 = true;
    }
  }

  if (matchSpecifiedCur1)
  {
    fareAmount = fbrItemInfo.specifiedFareAmt1();

    if (UNLIKELY(!matchesRecord3))
    {
      const std::vector<PaxTypeFare*>* otherFares = nullptr;
      bool sorted = false;

      if (!findOtherFares(fbrItemInfo, otherFares, sorted))
      {
        LOG4CXX_DEBUG(logger, "FBRC::processSpecifiedFare() - no otherFares for currency1");
        return;
      }

      minFareAmount = findMinAndMaxFares(
          otherFares, sorted, fbrItemInfo, specifiedCur1, fbrItemInfo.ruleTariff());
    }

    if (UNLIKELY(!ensureMinMaxRange(
                     matchesRecord3, fareAmount, minFareAmount, maxFareAmount, specifiedCur1, fbrItemInfo)))
    {
      LOG4CXX_DEBUG(logger, "FBRC::processSpecifiedFare() - ensureMinMaxRange failed");
      return;
    }
    else
    {
      if (UNLIKELY(!createSpecifiedFare(fbrProcessingInfo, fareAmount, specifiedCur1)))
      {
        LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - createSpecifiedFare failed");
        return;
      }
    }
  }

  bool matchSpecifiedCur2 = false;

  if (!specifiedCur2.empty())
  {
    if (matchNationCurrency(_fareMarket.origin()->nation(), specifiedCur2) ||
        matchNationCurrency(_fareMarket.destination()->nation(), specifiedCur2))
    {
      matchSpecifiedCur2 = true;
    }
  }

  if (matchSpecifiedCur2)
  {
    const std::vector<PaxTypeFare*>* otherFares = nullptr;
    bool sorted = false;
    fareAmount = fbrItemInfo.specifiedFareAmt2();

    if (!matchesRecord3)
    {
      if (!findOtherFares(fbrItemInfo, otherFares, sorted))
      {
        LOG4CXX_DEBUG(logger, "FBRC::processSpecifiedFare() - no otherFares for currency2");
        return;
      }

      minFareAmount = findMinAndMaxFares(
          otherFares, sorted, fbrItemInfo, specifiedCur2, fbrItemInfo.ruleTariff());
    }

    if (!ensureMinMaxRange(
            matchesRecord3, fareAmount, minFareAmount, maxFareAmount, specifiedCur2, fbrItemInfo))
    {
      LOG4CXX_DEBUG(logger, "FBRC::processSpecifiedFare() - ensureMinMaxRange failed");
      return;
    }

    if (!createSpecifiedFare(fbrProcessingInfo, fareAmount, specifiedCur2))
    {
      LOG4CXX_WARN(logger, "FBRC::processSpecifiedFare() - createSpecifiedFare failed");
      return;
    }
  }
}

bool
FareByRuleController::calculateFareAmt(PaxTypeFare& paxTypeFare,
                                       const FareByRuleItemInfo& fbrItemInfo,
                                       FareByRuleProcessingInfo& fbrProcessingInfo)
{
  _calcMoney.getFromPTF(paxTypeFare, true);

  if (UNLIKELY((fbrItemInfo.specifiedCur1() == FareByRuleController::BLANK_CURRENCY) &&
      (fbrItemInfo.specifiedFareAmt1() == 0) && (fbrItemInfo.specifiedFareAmt2() == 0)))
  {
    _calcMoney.setFareAmount(0);
    return true;
  }

  bool unknownFareInd = false;

  switch (fbrItemInfo.fareInd())
  {
  case CALCULATED:
    if (UNLIKELY(_spanishLargeFamilyDiscountApplied &&
        (fbrProcessingInfo.isResidenceFiledInR8() ||
         fbrProcessingInfo.isResidenceFiledInR3Cat25())))
      _calcMoney.doPercent(fbrProcessingInfo.combinedPercent());
    else
      _calcMoney.doPercent(fbrItemInfo.percent());
    break;

  case CREATE_RT_FROM_OW:
  case SELECT_HIGHEST:
  case SELECT_LOWEST:
    _calcMoney.doPercent(fbrItemInfo.percent());
    break;

  case ADD_SPECIFIED_TO_CALCULATED:
    if (!matchSpecifiedCurrency(paxTypeFare, fbrItemInfo))
      return false;

    _calcMoney.doPercent(fbrItemInfo.percent());
    _calcMoney.doAdd(fbrItemInfo.specifiedFareAmt1(),
                     fbrItemInfo.specifiedCur1(),
                     fbrItemInfo.specifiedFareAmt2(),
                     fbrItemInfo.specifiedCur2());
    break;

  case SUBTRACT_SPECIFIED_FROM_CALCULATED:
    if (!matchSpecifiedCurrency(paxTypeFare, fbrItemInfo))
      return false;

    _calcMoney.doPercent(fbrItemInfo.percent());
    _calcMoney.doMinus(fbrItemInfo.specifiedFareAmt1(),
                       fbrItemInfo.specifiedCur1(),
                       fbrItemInfo.specifiedFareAmt2(),
                       fbrItemInfo.specifiedCur2());

    if (_calcMoney.value() < 0.0)
    {
      _calcMoney.value() = 0.0; // Raise the resulting fare amount to be zero
      _calcMoney.nucValue() = 0.0;
    }

    break;

  case ADD_SPECIFIED_TO_BASE_CALC_PERCENTAGE:
    if (!matchSpecifiedCurrency(paxTypeFare, fbrItemInfo))
      return false;

    _calcMoney.doAdd(fbrItemInfo.specifiedFareAmt1(),
                     fbrItemInfo.specifiedCur1(),
                     fbrItemInfo.specifiedFareAmt2(),
                     fbrItemInfo.specifiedCur2());
    _calcMoney.doPercent(fbrItemInfo.percent());

    break;

  case SUBTRACT_SPECIFIED_FROM_BASE_CALC_PERCENTAGE:
    if (!matchSpecifiedCurrency(paxTypeFare, fbrItemInfo))
      return false;

    _calcMoney.doMinus(fbrItemInfo.specifiedFareAmt1(),
                       fbrItemInfo.specifiedCur1(),
                       fbrItemInfo.specifiedFareAmt2(),
                       fbrItemInfo.specifiedCur2());

    if (UNLIKELY(_calcMoney.value() < 0.0))
    {
      _calcMoney.value() = 0.0; // Raise the resulting fare amount to be zero
      _calcMoney.nucValue() = 0.0;
    }

    if (_spanishLargeFamilyDiscountApplied &&
        (fbrProcessingInfo.isResidenceFiledInR8() || fbrProcessingInfo.isResidenceFiledInR3Cat25()))
      _calcMoney.doPercent(fbrProcessingInfo.combinedPercent());
    else
      _calcMoney.doPercent(fbrItemInfo.percent());

    break;

  default:
    unknownFareInd = true;
  }

  if (UNLIKELY(unknownFareInd))
  {
    LOG4CXX_WARN(logger, "FBRC::calculateFareAmt() - fareInd is an improper value");
    return false;
  }

  if (UNLIKELY(_calcMoney.fareAmount() < 0))
  {
    LOG4CXX_WARN(logger, "FBRC::calculateFareAmt() - create negative fare amount");
    return false;
  }

  return true;
}

bool
FareByRuleController::fareMatchesRecord3(const FareByRuleItemInfo& fbrItemInfo,
                                         const FareByRuleApp& fbrApp)
{
  return (fbrItemInfo.carrier().empty() || fbrItemInfo.carrier() == fbrApp.carrier()) &&
         (fbrItemInfo.ruleTariff() == 0 || fbrItemInfo.ruleTariff() == -1) &&
         fbrItemInfo.baseFareClass().empty() && fbrItemInfo.baseFareType().empty();
}

bool
FareByRuleController::ensureMinMaxRange(bool useRecord3,
                                        MoneyAmount& fareAmt,
                                        MoneyAmount minFareAmount,
                                        MoneyAmount maxFareAmount,
                                        const CurrencyCode& currency,
                                        const FareByRuleItemInfo& fbrItemInfo)
{
  _isMinMaxFare = false;

  switch (fbrItemInfo.fareInd())
  {
  case SELECT_HIGHEST:
    return ensureMinMaxRangeHighest(
        useRecord3, fareAmt, minFareAmount, maxFareAmount, currency, fbrItemInfo);

  case SELECT_LOWEST:
    return ensureMinMaxRangeLowest(
        useRecord3, fareAmt, minFareAmount, maxFareAmount, currency, fbrItemInfo);

  default:
    return ensureMinMaxRangeOther(
        useRecord3, fareAmt, minFareAmount, maxFareAmount, currency, fbrItemInfo);
  }

  return true;
}

bool
FareByRuleController::ensureMinMaxRangeHighest(bool useRecord3,
                                               MoneyAmount& fareAmt,
                                               MoneyAmount minFareAmount,
                                               MoneyAmount maxFareAmount,
                                               const CurrencyCode& currency,
                                               const FareByRuleItemInfo& fbrItemInfo)
{
  if (LIKELY(useRecord3))
  {
    if (UNLIKELY(fbrItemInfo.specifiedCur1().empty() && fbrItemInfo.cur1().empty()))
    {
      return true;
    }

    if (UNLIKELY(!fbrItemInfo.cur1().empty()))
    {
      if (fbrItemInfo.cur1() == currency)
      {
        minFareAmount = fbrItemInfo.minFareAmt1();
      }
      else if (fbrItemInfo.cur2() == currency)
      {
        minFareAmount = fbrItemInfo.minFareAmt2();
      }
      else
      {
        return false;
      }
    }
  }

  if (LIKELY(!fbrItemInfo.specifiedCur1().empty()))
  {
    if (fbrItemInfo.specifiedCur1() == currency)
    {
      if (LIKELY(fbrItemInfo.specifiedFareAmt1() > minFareAmount))
      {
        // Select the highest comparison amount
        minFareAmount = fbrItemInfo.specifiedFareAmt1();
      }
    }
    else if (fbrItemInfo.specifiedCur2() == currency)
    {
      if (fbrItemInfo.specifiedFareAmt2() > minFareAmount)
      {
        // Select the highest comparison amount
        minFareAmount = fbrItemInfo.specifiedFareAmt2();
      }
    }
    else
    {
      return false;
    }
  }

  if (minFareAmount != 0 && fareAmt < minFareAmount)
  {
    // Raise the calculated amount to the minimum fare amount
    fareAmt = minFareAmount;
    _isMinMaxFare = true;
  }

  return true;
}

bool
FareByRuleController::ensureMinMaxRangeLowest(bool useRecord3,
                                              MoneyAmount& fareAmt,
                                              MoneyAmount minFareAmount,
                                              MoneyAmount maxFareAmount,
                                              const CurrencyCode& currency,
                                              const FareByRuleItemInfo& fbrItemInfo)
{
  if (useRecord3)
  {
    if (fbrItemInfo.specifiedCur1().empty() && fbrItemInfo.cur1().empty())
    {
      return true;
    }

    if (!fbrItemInfo.cur1().empty())
    {
      if (fbrItemInfo.cur1() == currency)
      {
        maxFareAmount = fbrItemInfo.maxFareAmt1();
      }
      else if (fbrItemInfo.cur2() == currency)
      {
        maxFareAmount = fbrItemInfo.maxFareAmt2();
      }
      else
      {
        return false;
      }
    }
  }

  bool zeroAmtFound = false;

  if (!fbrItemInfo.specifiedCur1().empty())
  {
    if (fbrItemInfo.specifiedCur1() == currency)
    {
      if (maxFareAmount == 0 || maxFareAmount > fbrItemInfo.specifiedFareAmt1())
      {
        // Select the lowest comparison amount
        maxFareAmount = fbrItemInfo.specifiedFareAmt1();

        if (fbrItemInfo.specifiedFareAmt1() == 0)
        {
          zeroAmtFound = true;
        }
      }
    }
    else if (fbrItemInfo.specifiedCur2() == currency)
    {
      if (maxFareAmount == 0 || maxFareAmount > fbrItemInfo.specifiedFareAmt2())
      {
        // Select the lowest comparison amount
        maxFareAmount = fbrItemInfo.specifiedFareAmt2();

        if (fbrItemInfo.specifiedFareAmt2() == 0)
        {
          zeroAmtFound = true;
        }
      }
    }
    else
    {
      return false;
    }
  }

  if ((maxFareAmount != 0 && fareAmt > maxFareAmount) || zeroAmtFound)
  {
    // Lower the calculated amount to the maximum fare amount
    fareAmt = maxFareAmount;
    _isMinMaxFare = true;
  }

  return true;
}

bool
FareByRuleController::ensureMinMaxRangeOther(bool useRecord3,
                                             MoneyAmount& fareAmt,
                                             MoneyAmount minFareAmount,
                                             MoneyAmount maxFareAmount,
                                             const CurrencyCode& currency,
                                             const FareByRuleItemInfo& fbrItemInfo)
{
  if (LIKELY(useRecord3))
  {
    if (fbrItemInfo.minFareAmt1() == 0 && fbrItemInfo.maxFareAmt1() == 0)
    {
      return true;
    }

    if (fbrItemInfo.cur1() == currency)
    {
      minFareAmount = fbrItemInfo.minFareAmt1();
      maxFareAmount = fbrItemInfo.maxFareAmt1();
    }
    else if (fbrItemInfo.cur2() == currency)
    {
      minFareAmount = fbrItemInfo.minFareAmt2();
      maxFareAmount = fbrItemInfo.maxFareAmt2();
    }
    else
    {
      return false;
    }
  }

  if ((minFareAmount != 0 && fareAmt < minFareAmount) ||
      (maxFareAmount != 0 && fareAmt > maxFareAmount))
  {
    return false;
  }

  return true;
}

bool
FareByRuleController::matchSpecifiedCurrency(PaxTypeFare& baseFare,
                                             const FareByRuleItemInfo& fbrItemInfo) const
{
  return baseFare.currency() == fbrItemInfo.specifiedCur1() ||
         baseFare.currency() == fbrItemInfo.specifiedCur2();
}

bool
FareByRuleController::createSpecifiedFare(FareByRuleProcessingInfo& fbrProcessingInfo,
                                          MoneyAmount& fareAmt,
                                          const CurrencyCode& currency)
{
  const FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();
  const FareByRuleCtrlInfo& fbrCtrlInfo = *fbrProcessingInfo.fbrCtrlInfo();
  DateTime effDate =
      fbrApp.effDate() < fbrCtrlInfo.effDate() ? fbrCtrlInfo.effDate() : fbrApp.effDate();
  DateTime expDate = fbrApp.expireDate() > fbrCtrlInfo.expireDate() ? fbrCtrlInfo.expireDate()
                                                                    : fbrApp.expireDate();
  TariffCrossRefInfo* tcrInfo = _creator.createTariffCrossRefInfo(effDate, expDate);
  FareInfo* fareInfo = createFareInfo(fareAmt, currency, effDate, expDate);
  Fare* fare = _creator.createFare(
      _fareMarket.boardMultiCity() > _fareMarket.offMultiCity(), fareInfo, tcrInfo);
  FareClassAppInfo* fcAppInfo = _creator.createFareClassAppInfo(effDate, expDate);
  FareClassAppSegInfo* fcAppSegInfo = _creator.createFareClassAppSegInfo();
  // add it to fareClassApp
  fcAppInfo->_segs.push_back(fcAppSegInfo);
  //------------------------------------------------------------
  // Create PaxTypeFare for specified Fare By rule
  //------------------------------------------------------------
  PaxTypeFare* fbrPtFare;
  _trx.dataHandle().get(fbrPtFare);
  PaxType* paxType = const_cast<PaxType*>(
      PaxTypeUtil::isAnActualPaxInTrx(_trx, fcAppInfo->_carrier, fcAppSegInfo->_paxType));
  fbrPtFare->initialize(fare, paxType, &_fareMarket, _trx);
  fbrPtFare->status().set(PaxTypeFare::PTF_FareByRule);
  fbrPtFare->fareClassAppInfo() = fcAppInfo;
  fbrPtFare->fareClassAppSegInfo() = fcAppSegInfo;

  if (UNLIKELY(!resolveFareTypeMatrix(*fbrPtFare, *fcAppInfo)))
  {
    return false;
  }

  // must match with multilateral or industry table to use fbrAppl
  if (UNLIKELY(fbrApp.carrier() == INDUSTRY_CARRIER))
  {
    IndustryFareController indCtrl(_trx, _itin, _fareMarket);
    IndustryFare* indFare = nullptr;

    if (_multiAppls)
    {
      indFare = indCtrl.matchFare(*fbrPtFare, *_multiAppls);

      if (indFare)
      {
        fareInfo->carrier() = _fareMarket.governingCarrier();
      }
    }

    if (_indAppls && indFare == nullptr)
    {
      indFare = indCtrl.matchFare(*fbrPtFare, *_indAppls);
    }

    if (indFare == nullptr)
      return false;

    fbrPtFare->initialize(indFare, paxType, &_fareMarket, _trx);
  } // endif - YY validation

  if (UNLIKELY(_trx.awardRequest() &&
               (fbrProcessingInfo.fbrItemInfo()->fareInd() == FareByRuleItemInfo::SPECIFIED ||
                fbrProcessingInfo.fbrItemInfo()->fareInd() == FareByRuleItemInfo::SPECIFIED_K)))
  {
    fbrPtFare->mileage() = (int)fbrPtFare->fareAmount();
    fareInfo->fareAmount() = 0;
    fareAmt = 0;
  }

  _calcMoney.setRT(fbrPtFare->isRoundTrip());
  _calcMoney.setCurrency(currency);
  _calcMoney.setFareAmount(fareAmt);
  putIntoPTF(*fbrPtFare, *fareInfo);
  //------------------------------------------------------------
  // Create paxTypeFareRuleData for specified Fare By rule
  //------------------------------------------------------------
  _creator.createFBRPaxTypeFareRuleData(*fbrPtFare, _baseFareInfoBkcAvailMap, _isMinMaxFare);

  if (UNLIKELY(!finalizeFareCreation(fbrPtFare, fbrProcessingInfo)))
    return false;

  setPtfFlags(*fbrPtFare, fbrApp);
  return true;
}

void
FareByRuleController::setPtfFlags(PaxTypeFare& ptf, const FareByRuleApp& fbrApp) const
{
  if (!fbrApp.accountCode().empty() || _qMatchCorpID)
  {
    ptf.setMatchedCorpID();
    if (_qMatchCorpID)
      ptf.matchedAccCode() = _qNewAcctCode;
  }

  if (UNLIKELY(_qMatchNationFR))
  {
    ptf.fare()->setNationFRInCat15();
  }

  if (UNLIKELY(_qEtktWarning))
  {
    ptf.fare()->setWarningEtktInCat15();
  }

  if (_trx.isShopping() && (!_qualifyFltAppRuleDataMap.empty()))
  {
    ptf.qualifyFltAppRuleDataMap() = _qualifyFltAppRuleDataMap;
  }
}

void
FareByRuleController::processCalculatedFare(FareByRuleProcessingInfo& fbrProcessingInfo)
{
  // this rule determines the carrier pref for all
  setCarrierPref(fbrProcessingInfo.fbrCtrlInfo()->carrierCode());
  _baseFareInfoBkcAvailMap.clear();
  int baseTableItemNo = fbrProcessingInfo.fbrItemInfo()->baseTableItemNo();
  const std::vector<const BaseFareRule*>& baseFareRules =
      getBaseFareRule(fbrProcessingInfo.fbrCtrlInfo()->vendorCode(), baseTableItemNo); // table 989

  if (UNLIKELY(baseFareRules.empty()))
    return;

  if (UNLIKELY(_diagWithR3Cat25 || _diagWithT989))
  {
    if (fbrProcessingInfo.diagManager())
      printT989Header(*fbrProcessingInfo.diagManager(), baseTableItemNo);
    else
    {
      DiagManager diag(_trx, Diagnostic325);
      printT989Header(diag, baseTableItemNo);
    }
  }

  if (_fareMarket.canValidateFBRBaseFares())
    _baseFareValidator.setCat25R3(fbrProcessingInfo.fbrItemInfo());

  std::vector<PaxTypeFare*> processFares;
  selectCalculatedFares(processFares, baseFareRules, fbrProcessingInfo);

  if (UNLIKELY(_diagWithT989 && _failBaseFareList.size()))
  {
    if (fbrProcessingInfo.diagManager())
    {
      DiagManager& diag = *fbrProcessingInfo.diagManager();
      Diag325Collector* diag325 = dynamic_cast<Diag325Collector*>(&diag.collector());
      diag325->displayFailBaseFareList(_failBaseFareList);
    }
    else
    {
      DiagManager diag(_trx, Diagnostic325);
      Diag325Collector* diag325 = dynamic_cast<Diag325Collector*>(&diag.collector());
      diag325->displayFailBaseFareList(_failBaseFareList);
    }
  }

  _failBaseFareList.clear();
  createCalculatedFares(processFares, fbrProcessingInfo);
}

void
FareByRuleController::printT989Header(DiagManager& diag, int fbrItemInfoNo) const
{
  diag << "BASE FARE TABLE 989: ";
  diag << std::setw(10) << fbrItemInfoNo;
  diag << '\n';
}

void
FareByRuleController::selectCalculatedFares(std::vector<PaxTypeFare*>& processFares,
                                            const std::vector<const BaseFareRule*>& baseFareRules,
                                            FareByRuleProcessingInfo& fbrProcessingInfo)
{
  std::vector<PaxTypeFare*> discardFares;
  FareTypeMatcher fareTypeMatch(_trx);

  for (const auto curBaseFareRule : baseFareRules)
  {
    // Display all Table 989 items for Diag 325 with Rule qualifier Q/*325/RUxxxx
    if (UNLIKELY(_diagWithR3Cat25 || _diagWithT989))
    {
      if (fbrProcessingInfo.diagManager())
      {
        DiagManager& diag = *fbrProcessingInfo.diagManager();
        diag << *curBaseFareRule;
      }
      else
      {
        DiagManager diag(_trx, Diagnostic325);
        diag << *curBaseFareRule;
      }
    }

    _matchedBkc1Avail = false;
    _passedBkc1Avail = false;
    _matchedBkc2Avail = false;
    _passedBkc2Avail = false;

    // Table 989 Alternate Market is out of scope,
    // so market1 and market2 must be blank or match the current fare market

    if (UNLIKELY(!marketsMatch((*curBaseFareRule).market1(), (*curBaseFareRule).market2())))
    {
      continue;
    }
    PTFRange range(determinePTFRange(*curBaseFareRule));
    if (range.first == range.second)
    {
      continue;
    }
    RoutingUtil::PaxTypeFareMap* ptfMap(nullptr);
    if (UNLIKELY(FareByRuleController::MILEAGE_ROUTING == curBaseFareRule->baseRouting()))
    {
      ptfMap = createPaxTypeFareMap(range);
      if (nullptr == ptfMap)
      {
        LOG4CXX_WARN(logger, "FBRC: Did not create/instantiate PaxTypeFareMap");
        continue;
      }
      if (!RoutingUtil::getRoutingType(*ptfMap, _trx))
      {
        LOG4CXX_WARN(logger, "FBRC: There was en error getting the Routing Type");
        continue;
      }
    }
    setAllowedVendors(curBaseFareRule->vendor());

    for (std::vector<PaxTypeFare*>::const_iterator it(range.first); it != range.second; ++it)
    {
      PaxTypeFare* currPtf(*it);

      if (_fareMarket.canValidateFBRBaseFares())
      {
        if (!_baseFareValidator.isFareValidInContext(*currPtf))
        {
          baseFareFailedValidation(*currPtf);
          continue;
        }
      }

      if (!matchFareToRule(*currPtf, *curBaseFareRule, ptfMap) ||
          !matchFareToFbrItem(*currPtf, fbrProcessingInfo, fareTypeMatch))
      {
        continue;
      }

      if ('N' == curBaseFareRule->baseFareAppl())
      {
        discardFares.push_back(currPtf);
      }
      else
      {
        bool notInDis(find(discardFares.begin(), discardFares.end(), currPtf) ==
                      discardFares.end());
        bool notInProc(find(processFares.begin(), processFares.end(), currPtf) ==
                       processFares.end());
        if (notInDis && notInProc)
        {
          processFares.push_back(currPtf);
        }
      }
    }
    if (_matchedBkc1Avail || _matchedBkc2Avail)
    {
      displayBkcAvail(fbrProcessingInfo, *curBaseFareRule);
    }
  } // Next Table 989 item
}

void
FareByRuleController::createCalculatedFares(const std::vector<PaxTypeFare*>& processFares,
                                            FareByRuleProcessingInfo& fbrProcessingInfo)
{
  const FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();
  const FareByRuleItemInfo& fbrItemInfo = *fbrProcessingInfo.fbrItemInfo();
  bool matchesRecord3 = fareMatchesRecord3(fbrItemInfo, fbrApp);

  if (UNLIKELY(_spanishLargeFamilyDiscountApplied &&
      (fbrProcessingInfo.isResidenceFiledInR8() ||
      fbrProcessingInfo.isResidenceFiledInR3Cat25())))
    setCombinedPercent(fbrProcessingInfo);

  _calcMoney._applyNonIATARounding = CurrencyRoundingUtil::applyNonIATARounding(
      _trx, fbrApp.vendor(), fbrApp.carrier(), fbrApp.ruleNo());

  for (const auto curProcessPTFare : processFares)
  {
    if (_fareMarket.canValidateFBRBaseFares())
    {
      switch (_baseFareValidator.getFareStatusInContext(*curProcessPTFare))
      {
        case FAIL:
        {
          baseFareFailedValidation(*curProcessPTFare);
          continue;
        }

        case NOTPROCESSED:
          if (!_baseFareValidator.validate(_trx, _itin, *curProcessPTFare))
          {
            baseFareFailedValidation(*curProcessPTFare);
            continue;
          }
          break;

        default: // PASS
          break;
      }
    }

    CurrencyCode currency = curProcessPTFare->currency();

    if (!calculateFareAmt(*curProcessPTFare, fbrItemInfo, fbrProcessingInfo))
    {
      continue; // Process next base fare
    }

    MoneyAmount minFareAmount = 0;
    MoneyAmount maxFareAmount = 0;

    if (UNLIKELY(!matchesRecord3))
    {
      const std::vector<PaxTypeFare*>* otherFares = nullptr;
      bool sorted = false;
      findOtherFares(fbrItemInfo, otherFares, sorted);
      TariffNumber ruleTariff = fbrItemInfo.ruleTariff();

      if (fbrItemInfo.ruleTariff() == 0)
      {
        ruleTariff = curProcessPTFare->tcrRuleTariff();
      }

      minFareAmount = findMinAndMaxFares(otherFares, sorted, fbrItemInfo, currency, ruleTariff);

      if (fbrItemInfo.fareInd() == SELECT_LOWEST)
      {
        maxFareAmount = minFareAmount;
        minFareAmount = 0;
      }
    }

    MoneyAmount fareAmount = _calcMoney.fareAmount();
    MoneyAmount currentFareAmount = fareAmount;

    if (!ensureMinMaxRange(
            matchesRecord3, fareAmount, minFareAmount, maxFareAmount, currency, fbrItemInfo))
    {
      continue; // Process next base fare
    }

    if ((fbrItemInfo.fareInd() == SELECT_HIGHEST || fbrItemInfo.fareInd() == SELECT_LOWEST) &&
        fareAmount != currentFareAmount)
    {
      _calcMoney.setFareAmount(fareAmount);
    }

    PaxTypeFare* calculatedFare;
    _trx.dataHandle().get(calculatedFare);

    if (UNLIKELY(!createCalculatedFare(curProcessPTFare, *calculatedFare, fbrProcessingInfo)))
    {
      continue; // Process next base fare
    }

    finalizeFareCreation(calculatedFare, fbrProcessingInfo);
  }
}

bool
FareByRuleController::finalizeFareCreation(PaxTypeFare* ptFare,
                                           FareByRuleProcessingInfo& fbrProcessingInfo)
{
  // Check for FareDisplay transaction
  if (UNLIKELY(isFdTrx()))
  {
    // Create new FareDisplayInfo object and clone
    if (!FareDisplayUtil::initFareDisplayInfo(_fdTrx, *ptFare))
    {
      LOG4CXX_DEBUG(logger, "FBRC::finalizeFareCreation() - Unable to init FareDisplayInfo");
      return false;
    }
    // Check if inhibit indicator is 'D' - for Display Only
    bool isDisplayOnly =
        fbrProcessingInfo.fbrCtrlInfo()->inhibit() == RuleConst::FARE_FOR_DISPLAY_ONLY ||
        ptFare->fbrApp().inhibit() == RuleConst::FARE_FOR_DISPLAY_ONLY;
    FareDisplayUtil::setDisplayOnly(_trx, *ptFare, isDisplayOnly, _isMinMaxFare);
  }
  // populate validating carriers vector in cat25 fare, if the qualified 15 passed one
  if(!fbrProcessingInfo.validatingCarriers().empty())
    ptFare->validatingCarriers() = fbrProcessingInfo.validatingCarriers();

  prevalidatePaxTypeFare(_trx, _itin, *ptFare);

  if (ptFare->isValid())
  {
    if(analyzeValidatingCarriers(_trx, ptFare))
    {
      _fbrPaxTypeFares.push_back(ptFare);

      if (fbrProcessingInfo.isSoftPassFor25())
        ptFare->setCategorySoftPassed(RuleConst::FARE_BY_RULE, true);
    }
  }

  if (UNLIKELY(_diag325Requested))
  {
    if (!_trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE).empty() &&
        _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE) != ptFare->fareClass())
      return true;

    if (fbrProcessingInfo.diagManager())
    {
      DiagManager& diag = *fbrProcessingInfo.diagManager();
      printFareHeader(diag, ptFare, fbrProcessingInfo);
      diag << *ptFare;
    }
    else
    {
      DiagManager diag(_trx, Diagnostic325);
      printFareHeader(diag, ptFare, fbrProcessingInfo);
      diag << *ptFare;
    }
  }

  return true;
}

bool
FareByRuleController::analyzeValidatingCarriers(PricingTrx& trx, PaxTypeFare* ptFare)
{
   if (!trx.isValidatingCxrGsaApplicable())
     return true;

   FBRPaxTypeFareRuleData* fbrPTF = ptFare->getFbrRuleData(RuleConst::FARE_BY_RULE);
   if(fbrPTF == nullptr)
      return true;

   // Populate or not PTF validating carrier vector depends on override tag for the
   // calculated fares only
   const FareByRuleItemInfo* fbrItemInfo =
              dynamic_cast<const FareByRuleItemInfo*>(fbrPTF->ruleItemInfo());

   if (fbrItemInfo != nullptr && !fbrPTF->isSpecifiedFare())
   {
      Indicator i = fbrItemInfo->ovrdcat15();

      // override tag is 'X', keep cat25 validating carriers vector as is
      // 'B' or ' '
      if ((i == 'B' || i == ' ') && fbrPTF->baseFare() &&
          !fbrPTF->baseFare()->validatingCarriers().empty())
      {
         PaxTypeFare* bFare = fbrPTF->baseFare();
         if(!ptFare->validatingCarriers().empty())
         {
            std::vector<CarrierCode> validatingCxrs;
            for (CarrierCode cxr : bFare->validatingCarriers())
            {
              if (find(ptFare->validatingCarriers().begin(),
                       ptFare->validatingCarriers().end(),
                       cxr) != ptFare->validatingCarriers().end())
               {
                  validatingCxrs.push_back(cxr);
               }
            }
            if(validatingCxrs.empty())
               return false;
            ptFare->validatingCarriers().swap(validatingCxrs);
         }
         else
           ptFare->validatingCarriers() = bFare->validatingCarriers();
      }
   }
   return true;
}

bool
FareByRuleController::invalidDCT(const PaxTypeFare& ptFare, const FareByRuleItemInfo& fbrItemInfo)
    const
{
  if (UNLIKELY(_trx.getRequest()->ticketingAgent()->axessUser() && checkDisplayType(ptFare)))
  {
    return true;
  }
  else if (UNLIKELY(_trx.getRequest()->ticketingAgent()->axessUser() && !isFdTrx() &&
           fbrItemInfo.resultDisplaycatType() == BLANK))
  {
    // Do not create Cat 25 fare without Cat 35 security for WPNETT/WPSEL
    if ((_trx.getRequest()->isWpNettRequested() || _trx.getRequest()->isWpSelRequested()) &&
        ptFare.fcaDisplayCatType() != RuleConst::SELLING_FARE)
    {
      return true;
    }
    // Do not create Cat 25 fare with Cat 35 security for WP
    else if (!(_trx.getRequest()->isWpNettRequested() || _trx.getRequest()->isWpSelRequested()) &&
             ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE)
    {
      return true;
    }
  }

  return false;
}

bool
FareByRuleController::matchFareToFbrItem(const PaxTypeFare& ptFare,
                                         const FareByRuleProcessingInfo& fbrProcessingInfo,
                                         const FareTypeMatcher& ftMatcher)
{
  const FareByRuleItemInfo& fbrItemInfo = *fbrProcessingInfo.fbrItemInfo();

  if (UNLIKELY(ptFare.isFareByRule()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_FARE_BY_RULE_FARE);
    return false;
  }

  if (UNLIKELY(ptFare.isDiscounted()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_DISCOUNTED_FARE);
    return false;
  }

  if (UNLIKELY(!isVendorAllowed(ptFare.vendor()) && !matchFbrPref(ptFare.vendor())))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_VENDOR_CROSS_REF_CARRIER_PREF_FBR);
    return false;
  }

  if (UNLIKELY(invalidDCT(ptFare, fbrItemInfo)))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_RESULTING_DISPLAY_CATEGORY);
    return false;
  }

  if (UNLIKELY(!ptFare.canBeFBRBaseFare()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_VALID_FOR_FBR_BASE_FARE);
    return false;
  }

  const FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();

  if (UNLIKELY(!matchBaseFareVendor(ptFare.vendor(), fbrApp.vendor())))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_BASE_FARE_VENDOR_MATCH);
    return false;
  }

  if (UNLIKELY(!matchBaseFareSecurity(ptFare, fbrProcessingInfo)))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_BASE_FARE_SECURITY);
    return false;
  }

  if (UNLIKELY(ptFare.fare()->isIndustry()))
  {
    const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(ptFare.fare());

    if ((indFare != nullptr) && (!indFare->validForPricing()))
    {
      addToFailBaseFareList(ptFare, Diag325Collector::FAIL_INVALID_INDUSTRY_FARE);
      return false;
    }
  }

  // check for the NL ( normal fare) secondary action code in the pricing entry
  if (UNLIKELY(_trx.getOptions()->isNormalFare() && fbrItemInfo.resultpricingcatType() == ' '))
  {
    if (!ptFare.isNormal())
    {
      addToFailBaseFareList(ptFare, Diag325Collector::FAIL_RESULTING_PRICING_CATEGORY);
      return false;
    }
  }

  // check for WPT/{NL,EX,IT} entry
  if (UNLIKELY(_trx.getOptions()->isFareFamilyType() && !ftMatcher(fbrItemInfo.resultFareType1(), &ptFare)))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_RESULTING_FARE_TYPE);
    return false;
  }

  if (UNLIKELY((fbrItemInfo.resultglobalDir() == GlobalDirection::XX ||
                 fbrItemInfo.resultglobalDir() == GlobalDirection::ZZ) &&
                !ptFare.isGlobalDirectionValid()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_RESULTING_GLOBAL);
    return false;
  }

  if (!marketsMatch(ptFare.market1(), ptFare.market2()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_BETWEEN_AND_CITIES);
    return false;
  }

  return true;
}

PTFRange
FareByRuleController::determinePTFRange(const BaseFareRule& baseFareRule)
{
  const CarrierCode& carrier(baseFareRule.carrier());
  const PaxTypeCode& basePsgType(baseFareRule.basepsgType());
  if (LIKELY(carriersMatch(carrier, _fareMarket.governingCarrier())))
  {
    return findFaresFromPaxType(_fareMarket, baseFareRule);
  }
  else
  {
    std::map<const CarrierCode, FareMarket*>::const_iterator altFmIt(
        _carrierToAltFareMarketMap.find(carrier));

    if (altFmIt != _carrierToAltFareMarketMap.end())
    {
      return findFaresFromPaxType(*(*altFmIt).second, baseFareRule);
    }
    FareMarket* newFareMarket = cloneFareMarket(carrier);
    newFareMarket->origin() = getLoc(_fareMarket.origin()->loc());
    newFareMarket->destination() = getLoc(_fareMarket.destination()->loc());
    std::vector<PaxTypeBucket>& paxTypeCortege(_fareMarket.paxTypeCortege());

    if (!findPaxType(basePsgType, paxTypeCortege))
    {
      changePaxType(baseFareRule, *newFareMarket);
    }
    publishedFaresStep(*newFareMarket);
    PTFRange otherFares(findFaresFromPaxType(*newFareMarket, baseFareRule));

    _carrierToAltFareMarketMap[carrier] = newFareMarket;

    if (otherFares.first == otherFares.second)
    {
      return PTFRange();
    }
    else
    {
      return otherFares;
    }
  }
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleController::matchFareToRule
//
// Description:
//
// @return true, if the PaxTypeFare matches the rules specified in the
//               BaseFareRule object
//         false, otherwise
// ----------------------------------------------------------------------------
bool
FareByRuleController::matchFareToRule(PaxTypeFare& ptFare,
                                      const BaseFareRule& baseFareRule,
                                      RoutingUtil::PaxTypeFareMap* routingPTFareMap)
{
  return matchBaseFareFareRecord(ptFare, baseFareRule) && matchPaxType(ptFare, baseFareRule) &&
         matchBaseFareRecord1(ptFare, baseFareRule) &&
         matchRouting(ptFare, baseFareRule, routingPTFareMap) &&
         matchFootnoteAndBkgCode(ptFare, baseFareRule) &&
         matchMinMaxFareAmount(ptFare, baseFareRule);
}

bool
FareByRuleController::matchBaseFareFareRecord(const PaxTypeFare& ptFare,
                                              const BaseFareRule& baseFareRule)
{
  if (baseFareRule.baseRuleTariff() == 0)
  {
    if (UNLIKELY(ptFare.tcrTariffCat() == 1))
    {
      addToFailBaseFareList(ptFare, Diag325Collector::FAIL_PRIVATE_FARE);
      return false;
    }
  }
  else if (UNLIKELY(baseFareRule.baseRuleTariff() != ptFare.tcrRuleTariff()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_RULE_TARIFF);
    return false;
  }

  if (!baseFareRule.baseRuleNo().empty() && baseFareRule.baseRuleNo() != ptFare.ruleNumber())
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_RULE_NUMBER);
    return false;
  }

  if (!baseFareRule.carrier().empty() && baseFareRule.carrier() != ptFare.carrier())
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_CARRIER_CODE);
    return false;
  }

  if (baseFareRule.baseowrt() != FareByRuleController::BLANK &&
      baseFareRule.baseowrt() != ptFare.owrt())
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_OW_RT);
    return false;
  }

  if (baseFareRule.baseglobalDir() != GlobalDirection::ZZ &&
      baseFareRule.baseglobalDir() != ptFare.globalDirection())
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_GLOBAL_DIRECTION);
    return false;
  }

  return true;
}

bool
FareByRuleController::matchPaxType(const PaxTypeFare& ptFare, const BaseFareRule& baseFareRule)
{
  if (UNLIKELY(ptFare.isFareClassAppSegMissing()))
  {
    const FareClassAppInfo* fcaInfo = ptFare.fareClassAppInfo();

    // iterate through this vector of FareClassAppSegInfo records
    // and find the one that matches passanger type
    if (matchFareClassAppInfoToPaxType(*fcaInfo, baseFareRule) == fcaInfo->_segs.end())
    {
      addToFailBaseFareList(ptFare, Diag325Collector::FAIL_PASSENGER_TYPE_CODE);
      return false;
    }
  }
  else
  {
    if (baseFareRule.basepsgType() == ADULT)
    {
      if (!(ptFare.fcasPaxType() == ADULT || ptFare.fcasPaxType().empty()))
      {
        addToFailBaseFareList(ptFare, Diag325Collector::FAIL_PASSENGER_TYPE_CODE);
        return false;
      }
    }
    else if (baseFareRule.basepsgType() != ptFare.fcasPaxType())
    {
      addToFailBaseFareList(ptFare, Diag325Collector::FAIL_PASSENGER_TYPE_CODE);
      return false;
    }
  }

  return true;
}

FareClassAppSegInfoList::const_iterator
FareByRuleController::matchFareClassAppInfoToPaxType(const FareClassAppInfo& fcAppInfo,
                                                     const BaseFareRule& baseFareRule)
{
  FareClassAppSegInfoList::const_iterator fcas = fcAppInfo._segs.begin();

  for (; fcas != fcAppInfo._segs.end(); fcas++)
  {
    if (baseFareRule.basepsgType() == ADULT)
    {
      if ((*fcas)->_paxType == ADULT || (*fcas)->_paxType.empty())
      {
        break;
      }
    }
    else if (baseFareRule.basepsgType() == (*fcas)->_paxType)
    {
      break;
    }
  }

  return fcas;
}

bool
FareByRuleController::matchBaseFareRecord1(const PaxTypeFare& ptFare,
                                           const BaseFareRule& baseFareRule)
{
  if (!(RuleUtil::matchFareClass(baseFareRule.baseFareClass().c_str(), ptFare.fareClass().c_str())))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_FARE_CLASS);
    return false;
  }

  if (!RuleUtil::matchFareType(baseFareRule.baseFareType().c_str(), ptFare.fcaFareType()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_FARE_TYPE_CODE);
    return false;
  }

  if (!RuleUtil::matchSeasons(baseFareRule.baseseasonType(), ptFare.fcaSeasonType()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_SEASON_CODE);
    return false;
  }

  if (!RuleUtil::matchDayOfWeek(baseFareRule.basedowType(), ptFare.fcaDowType()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_DAY_CODE);
    return false;
  }

  if (UNLIKELY(baseFareRule.basepricingcatType() != FareByRuleController::BLANK &&
               baseFareRule.basepricingcatType() != ptFare.fcaPricingCatType()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_PRICING_CATEGORY_CODE);
    return false;
  }

  return true;
}

bool
FareByRuleController::matchRouting(PaxTypeFare& ptFare,
                                   const BaseFareRule& baseFareRule,
                                   RoutingUtil::PaxTypeFareMap* routingPTFareMap)
{
  if (UNLIKELY(baseFareRule.baseRouting() == FareByRuleController::MILEAGE_ROUTING))
  {
    if (routingPTFareMap == nullptr)
    {
      addToFailBaseFareList(ptFare, Diag325Collector::FAIL_MILEAGE_ROUTING);
      return false;
    }

    RoutingUtil::PaxTypeFareMapItr routingPTFare = routingPTFareMap->find(&ptFare);

    if (routingPTFare != routingPTFareMap->end())
    {
      if (routingPTFare->second != MILEAGE_FARE)
      {
        addToFailBaseFareList(ptFare, Diag325Collector::FAIL_MILEAGE_ROUTING);
        return false;
      }
    }
  }
  else
  {
    if (!baseFareRule.baseRouting().empty() && baseFareRule.baseRouting() != ptFare.routingNumber())
    {
      addToFailBaseFareList(ptFare, Diag325Collector::FAIL_ROUTING_NUMBER);
      return false;
    }

    if (UNLIKELY(!baseFareRule.baseRouting().empty() &&
                 ptFare.hasConstructedRouting() && ptFare.fare()->constructedFareInfo()))
    {
      if ((ptFare.fare()->constructedFareInfo()->constructionType() ==
               ConstructedFareInfo::SINGLE_ORIGIN &&
           ptFare.fare()->constructedFareInfo()->origAddonRouting() != ptFare.routingNumber()) ||
          (ptFare.fare()->constructedFareInfo()->constructionType() ==
               ConstructedFareInfo::SINGLE_DESTINATION &&
           ptFare.fare()->constructedFareInfo()->destAddonRouting() != ptFare.routingNumber()) ||
          (ptFare.fare()->constructedFareInfo()->constructionType() ==
               ConstructedFareInfo::DOUBLE_ENDED &&
           ptFare.fare()->constructedFareInfo()->origAddonRouting() != ptFare.routingNumber() &&
           ptFare.fare()->constructedFareInfo()->destAddonRouting() != ptFare.routingNumber()))
      {
        addToFailBaseFareList(ptFare, Diag325Collector::FAIL_ROUTING_NUMBER);
        return false;
      }
    }
  }
  return true;
}

bool
FareByRuleController::matchFootnoteAndBkgCode(PaxTypeFare& ptFare, const BaseFareRule& baseFareRule)
{
  if (!RuleUtil::matchFareFootnote(baseFareRule.basefootNote1(),
                                   baseFareRule.basefootNote2(),
                                   ptFare.footNote1(),
                                   ptFare.footNote2()))
  {
    addToFailBaseFareList(ptFare, Diag325Collector::FAIL_FOOT_NOTE);
    return false;
  }

  if (!baseFareRule.bookingCode1().empty())
  {
    if (!matchBookingCode(baseFareRule.bookingCode1(), baseFareRule.bookingCode2(), ptFare))
    {
      addToFailBaseFareList(ptFare, Diag325Collector::FAIL_BOOKING_CODE);
      return false;
    }
  }

  return true;
}

bool
FareByRuleController::matchMinMaxFareAmount(const PaxTypeFare& ptFare,
                                            const BaseFareRule& baseFareRule)
{
  if ((baseFareRule.baseminFare1() != 0) || (baseFareRule.baseMaxFare1() != 0))
  {
    if (baseFareRule.baseCur1() == ptFare.currency())
    {
      if ((baseFareRule.baseminFare1() != 0) &&
          (baseFareRule.baseminFare1() > ptFare.originalFareAmount()))
      {
        addToFailBaseFareList(ptFare, Diag325Collector::FAIL_MIN_MAX_FARE_RANGE);
        return false;
      }

      if ((baseFareRule.baseMaxFare1() != 0) &&
          (baseFareRule.baseMaxFare1() < ptFare.originalFareAmount()))
      {
        addToFailBaseFareList(ptFare, Diag325Collector::FAIL_MIN_MAX_FARE_RANGE);
        return false;
      }
    }
    else if (baseFareRule.baseCur2() == ptFare.currency())
    {
      if ((baseFareRule.baseminFare2() != 0) &&
          (baseFareRule.baseminFare2() > ptFare.originalFareAmount()))
      {
        addToFailBaseFareList(ptFare, Diag325Collector::FAIL_MIN_MAX_FARE_RANGE);
        return false;
      }

      if ((baseFareRule.baseMaxFare2() != 0) &&
          (baseFareRule.baseMaxFare2() < ptFare.originalFareAmount()))
      {
        addToFailBaseFareList(ptFare, Diag325Collector::FAIL_MIN_MAX_FARE_RANGE);
        return false;
      }
    }
    else
    {
      addToFailBaseFareList(ptFare, Diag325Collector::FAIL_MIN_MAX_FARE_RANGE);
      return false;
    }
  }

  return true;
}

inline FareByRuleController::BkcMatchResult
FareByRuleController::findPrimeBookingCode(const std::vector<BookingCode>& primeBkcVec,
                                           bool ignoreBkcAvail,
                                           BookingCode& bkc)
{
  if (!bkc.empty())
  {
    if (bkc[1] == ASTERISK)
      bkc = bkc[0];
    else
      ignoreBkcAvail = true;

    if (std::find(primeBkcVec.begin(), primeBkcVec.end(), bkc) != primeBkcVec.end())
      return ignoreBkcAvail ? BKC_PASSED : BKC_NEED_AVAIL;
  }
  return BKC_FAILED;
}

inline bool
FareByRuleController::checkBookingCodeAvail(BookingCode bkc,
                                            bool& matchedBkcAvail,
                                            bool& passedBkcAvail)
{
  if (!matchedBkcAvail)
  {
    const bool softPassAvail = _trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX ||
                               (_trx.getTrxType() == PricingTrx::MIP_TRX &&
                                (_fareMarket.hasDuplicates() || _fareMarket.isSimilarItinMarket()));

    matchedBkcAvail = true;
    passedBkcAvail = softPassAvail || checkAvailability(bkc);
  }
  return passedBkcAvail;
}

bool
FareByRuleController::matchBookingCode(BookingCode bookingCode1,
                                       BookingCode bookingCode2,
                                       PaxTypeFare& ptFare)
{
  std::vector<BookingCode> primeBkcVec;
  ptFare.getPrimeBookingCode(primeBkcVec);

  bool skipBkcValidationInHistorical = false;
  const RexBaseTrx* rexBaseTrx = dynamic_cast<const RexBaseTrx*>(&_trx);
  skipBkcValidationInHistorical =
      (rexBaseTrx && rexBaseTrx->trxPhase() == RexBaseTrx::REPRICE_EXCITIN_PHASE);

  //during fallback removal, please remove FareMarket::isFullyFlownFareMarket()
  bool flownFM = fallback::skipBkcValidationForFlownFM(&_trx) ?
                            ptFare.fareMarket()->isFullyFlownFareMarket() :
                            !ptFare.fareMarket()->travelSeg().front()->unflown();

  skipBkcValidationInHistorical = skipBkcValidationInHistorical || flownFM;

  const bool ignoreBkcAvail = _trx.getTrxType() == PricingTrx::IS_TRX ||
                              _trx.getRequest()->isWpas() || skipBkcValidationInHistorical;

  BkcMatchResult bkcResult1 = findPrimeBookingCode(primeBkcVec, ignoreBkcAvail, bookingCode1);
  BkcMatchResult bkcResult2 = findPrimeBookingCode(primeBkcVec, ignoreBkcAvail, bookingCode2);

  if (bkcResult1 == BKC_PASSED || bkcResult2 == BKC_PASSED)
    return true;

  bool passedAvail = false;

  if (bkcResult1 == BKC_NEED_AVAIL)
  {
    addBaseFareInfoBkcAvailMap(&ptFare, bookingCode1);
    passedAvail =
        passedAvail || checkBookingCodeAvail(bookingCode1, _matchedBkc1Avail, _passedBkc1Avail);
  }

  if (UNLIKELY(bkcResult2 == BKC_NEED_AVAIL))
  {
    addBaseFareInfoBkcAvailMap(&ptFare, bookingCode2);
    passedAvail =
        passedAvail || checkBookingCodeAvail(bookingCode2, _matchedBkc2Avail, _passedBkc2Avail);
  }

  return passedAvail;
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleController::checkAvailability
//
// Description:
//
// @return true, if the booking code is available on all flights for all
//               carriers within the fare component being priced.
//         false, otherwise
// ----------------------------------------------------------------------------
bool
FareByRuleController::checkAvailability(const BookingCode& bookingCode)
{
  uint16_t numSeatsRequired = PaxTypeUtil::totalNumSeats(_trx);
  return AvailabilityChecker().checkAvailability(
      numSeatsRequired, bookingCode, _fareMarket.classOfServiceVec(), _fareMarket.travelSeg());
}

bool
FareByRuleController::checkAvail(const BookingCode& bookingCode,
                                 const std::vector<ClassOfService*>& cosVec,
                                 uint16_t numSeatsRequired)
{
  return std::any_of(cosVec.cbegin(),
                     cosVec.cend(),
                     [=](const ClassOfService* const cs)
                     { return cs->isAvailable(bookingCode, numSeatsRequired); });
}

void
FareByRuleController::displayBkcAvail(FareByRuleProcessingInfo& fbrProcessingInfo,
                                      const BaseFareRule& baseFareRule)
{
  if (UNLIKELY(_trx.getTrxType() == PricingTrx::IS_TRX || _trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
    return;

  if (UNLIKELY(_diag325Requested && _diagWithRuleNumber))
  {
    if (fbrProcessingInfo.diagManager())
    {
      displayBkcAvail(*fbrProcessingInfo.diagManager(), baseFareRule);
    }
    else
    {
      DiagManager diag(_trx, Diagnostic325);
      displayBkcAvail(diag, baseFareRule);
    }
  }
}

void
FareByRuleController::displayBkcAvail(DiagManager& diag, const BaseFareRule& baseFareRule)
{
  displayBkcAvail(diag, _matchedBkc1Avail, _passedBkc1Avail, baseFareRule.bookingCode1()[0]);
  displayBkcAvail(diag, _matchedBkc2Avail, _passedBkc2Avail, baseFareRule.bookingCode2()[0]);
  diag << "///////////////////////////////////////////////////////////////";
  diag << '\n';
}

void
FareByRuleController::displayBkcAvail(DiagManager& diag,
                                      bool matched,
                                      bool passed,
                                      Indicator bkgCode)
{
  if (matched)
  {
    diag << "**BASE FARE PRIME RBD " << bkgCode << " AVAILABILTY REQUIREMENT EXISTS - ";
    diag << (passed ? "PASS**" : "FAIL**") << '\n';
  }
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleController::createCalculatedFare
//
// Description:
//
// @return true, if the PaxTypeFare was initialized successfully
//         false, otherwise
// ----------------------------------------------------------------------------
bool
FareByRuleController::createCalculatedFare(PaxTypeFare* baseFare,
                                           PaxTypeFare& calculatedFare,
                                           const FareByRuleProcessingInfo& fbrProcessingInfo)
{
  const FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();
  const FareByRuleCtrlInfo& fbrCtrlInfo = *fbrProcessingInfo.fbrCtrlInfo();
  DateTime effDate =
      fbrApp.effDate() < fbrCtrlInfo.effDate() ? fbrCtrlInfo.effDate() : fbrApp.effDate();
  DateTime expDate = fbrApp.expireDate() > fbrCtrlInfo.expireDate() ? fbrCtrlInfo.expireDate()
                                                                    : fbrApp.expireDate();
  TariffCrossRefInfo* tcrInfo = _creator.createTariffCrossRefInfo(effDate, expDate, baseFare);
  FareInfo* fareInfo = _creator.createFareInfo(effDate, expDate, baseFare);
  Fare* fare = _creator.createFare(
      baseFare->isReversed(), fareInfo, tcrInfo, baseFare->fare()->constructedFareInfo());
  FareClassAppSegInfo* fcAppSegInfo =
      _creator.cloneFareClassAppSegInfo(*baseFare->fareClassAppSegInfo());
  FareClassAppInfo* fcAppInfo = _creator.cloneFareClassAppInfo(
      effDate, expDate, *baseFare->fareClassAppInfo(), fareInfo->fareClass());
  fcAppInfo->_segs.push_back(fcAppSegInfo);
  //------------------------------------------------------------
  // Create PaxTypeFare for calculated Fare By rule
  //------------------------------------------------------------
  PaxType* paxType = const_cast<PaxType*>(
      PaxTypeUtil::isAnActualPaxInTrx(_trx, fcAppInfo->_carrier, fcAppSegInfo->_paxType));
  calculatedFare.initialize(fare, paxType, &_fareMarket, _trx);
  calculatedFare.status().set(PaxTypeFare::PTF_FareByRule);
  calculatedFare.fareClassAppInfo() = fcAppInfo;
  calculatedFare.fareClassAppSegInfo() = fcAppSegInfo;

  if (UNLIKELY(!resolveFareTypeMatrix(calculatedFare, *fcAppInfo)))
  {
    return false;
  }

  if (UNLIKELY(fcAppInfo->_unavailTag == RuleApplicationBase::dataUnavailable &&
      commandPricingFare(fareInfo->fareClass())))
  //    _fareMarket.fareBasisCode() == fareInfo->fareClass() &&
  //    _fareMarket.fbcUsage() == COMMAND_PRICE_FBC)
  {
    calculatedFare.cpFailedStatus().set(PaxTypeFare::PTFF_R1UNAVAIL);
    fcAppInfo->_unavailTag = BLANK;
  }

  _calcMoney.setRT(calculatedFare.isRoundTrip());
  putIntoPTF(calculatedFare, *fareInfo);
  //------------------------------------------------------------
  // Create FBRPaxTypeFareRuleData for calculated Fare By rule
  //------------------------------------------------------------
  createFBRPaxTypeFareRuleData(calculatedFare, baseFare);
  setPtfFlags(calculatedFare, fbrApp);
  return true;
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleController::findFaresFromPaxType
//
// Description:  This method was modified from the original version to always
//               return fares from allPaxtypeFare
//
// @return const std::vector<PaxTypeFare*>*, a pointer to the vector of
//         PaxTypeFares if found vector of PaxTypeBucket's
//         0, otherwise
// ----------------------------------------------------------------------------
const std::vector<PaxTypeFare*>*
FareByRuleController::findFaresFromPaxType(const FareMarket& fareMarket)
{
  return &(fareMarket.allPaxTypeFare());
}

PTFRange
FareByRuleController::findFaresFromPaxType(FareMarket& fareMarket, const BaseFareRule& baseFareRule)
{
  return fareMarket.getPTFRange(baseFareRule.baseRuleTariff(), _dataHandle);
}

bool
FareByRuleController::findPaxType(const PaxTypeCode& paxTypeCode,
                                  const std::vector<PaxTypeBucket>& paxTypeCortege) const
{
  return std::any_of(paxTypeCortege.begin(),
                     paxTypeCortege.end(),
                     [paxTypeCode](const PaxTypeBucket& ptc)
                     { return ptc.isPaxTypeInActualPaxType(paxTypeCode); });
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleController::marketsMatch
//
// Description:  This method determines if the two markets match.
//               the first pair of location codes are the ones
//               specified in the rule and if blank, match any
//               of the second pair.
//
// @return bool - true if the two pairs of locations match.
//                false otherwise
// ----------------------------------------------------------------------------
bool
FareByRuleController::marketsMatch(const LocCode& market1, const LocCode& market2) const
{
  // I broke this out into separate if statements for readability
  if (market1 == EMPTY_STRING() && market2 == EMPTY_STRING())
  {
    return true;
  }

  LocCode origin = _fareMarket.origin()->loc();
  LocCode destination = _fareMarket.destination()->loc();
  LocCode boardMultiCity = _fareMarket.boardMultiCity();
  LocCode offMultiCity = _fareMarket.offMultiCity();

  if ((market1 == origin || market1 == boardMultiCity) &&
      (market2 == destination || market2 == offMultiCity))
  {
    return true;
  }

  if ((market1 == destination || market1 == offMultiCity) &&
      (market2 == origin || market2 == boardMultiCity))
  {
    return true;
  }

  if (UNLIKELY(_trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
  {
    LocCode fareMultiCity1 = getMultiTransportCity(market1);
    LocCode fareMultiCity2 = getMultiTransportCity(market2);

    if (fareMultiCity1.empty())
      fareMultiCity1 = market1;

    if (fareMultiCity2.empty())
      fareMultiCity2 = market2;

    if ((fareMultiCity1 == origin && fareMultiCity2 == destination) ||
        (fareMultiCity1 == destination && fareMultiCity2 == origin))
    {
      return true;
    }
  }

  return false;
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleController::carriersMatch
//
// Description:  This method determines if two CarrierCodes.
//               The first carrier is the one specified in
//               the rule and if blank, matches any code
//
// @return bool - true if the two carrier codes match.
//                false otherwise
// ----------------------------------------------------------------------------
bool
FareByRuleController::carriersMatch(const CarrierCode& carrier1, const CarrierCode& carrier2)
{
  return carrier1 == EMPTY_STRING() || carrier1 == INDUSTRY_CARRIER || carrier1 == carrier2;
}

// ----------------------------------------------------------------------------
// @function bool FareByRuleController::isDirectionPass
//
// Description:  This method matches the directionality in Record 2 Category 25
//               segment. The directionality value 3 and 4 and the outbound/inbound
//               indicator can not be validated at this time.
//
// @return bool - true if pass or softpass directionality for the current RuleItem
//                false, otherwise
// ----------------------------------------------------------------------------
bool
FareByRuleController::isDirectionPass(const CategoryRuleItemInfo& rule, bool isLocationSwapped)
    const
{
  // check the outbound/inbound direction
  if (UNLIKELY(rule.inOutInd() != RuleConst::ALWAYS_APPLIES))
  {
    return true;
  }

  // check the GEO_location directionality
  Indicator directionality = rule.directionality();

  if (directionality != RuleConst::ALWAYS_APPLIES)
  {
    if (directionality == RuleConst::FROM_LOC1_TO_LOC2 &&
        isLocationSwapped) // directionality was swapped on Loc1-Loc2 from Rule
    {
      return false;
    }
    else if (directionality == RuleConst::TO_LOC1_FROM_LOC2 &&
             !isLocationSwapped) // directionality was not swapped on Loc1-Loc2 from Rule
    {
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
// isDirectionPassForFD()
// return PASS - pass directionality for the current RuleItem

//        FAIL - fail directionality for the current RuleItem
//
//----------------------------------------------------------------------------
bool
FareByRuleController::isDirectionPassForFD(const CategoryRuleItemInfo& rule, bool isLocationSwapped)
    const
{
  // check the inbound direction
  if (rule.inOutInd() != RuleConst::ALWAYS_APPLIES)
  {
    if (rule.inOutInd() == RuleConst::FARE_MARKET_INBOUND)
    {
      return false;
    }
  }

  // check the GEO_location directionality
  Indicator directionality = rule.directionality();

  if (directionality == RuleConst::ALWAYS_APPLIES)
  {
    return true;
  }

  if (directionality == RuleConst::FROM_LOC1_TO_LOC2 ||
      directionality == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2)
  {
    if (isLocationSwapped) // directionality was swapped on Loc1-Loc2 from Rule
    {
      return false;
    }
  }
  else if (directionality == RuleConst::TO_LOC1_FROM_LOC2 ||
           directionality == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1)
  {
    if (!isLocationSwapped) // directionality was not swapped on Loc1-Loc2 from Rule
    {
      return false;
    }
  }

  return true;
}

FareMarket*
FareByRuleController::cloneFareMarket(const CarrierCode& newCarrier) const
{
  FareMarket* newFareMarket = nullptr;
  _trx.dataHandle().get(newFareMarket);
  newFareMarket->origin() = _fareMarket.origin();
  newFareMarket->destination() = _fareMarket.destination();
  newFareMarket->boardMultiCity() = _fareMarket.boardMultiCity();
  newFareMarket->offMultiCity() = _fareMarket.offMultiCity();
  newFareMarket->setGlobalDirection(_fareMarket.getGlobalDirection());
  newFareMarket->geoTravelType() = _fareMarket.geoTravelType();
  newFareMarket->governingCarrierPref() = _fareMarket.governingCarrierPref();
  newFareMarket->travelBoundary() = _fareMarket.travelBoundary();
  newFareMarket->governingCarrier() = newCarrier;
  newFareMarket->travelDate() = _fareMarket.travelDate();
  newFareMarket->ruleApplicationDate() = _fareMarket.ruleApplicationDate();
  newFareMarket->paxTypeCortege() = _fareMarket.paxTypeCortege();
  newFareMarket->fareCompInfo() = _fareMarket.fareCompInfo();
  newFareMarket->travelSeg() = _fareMarket.travelSeg();
  newFareMarket->getApplicableSOPs() = _fareMarket.getApplicableSOPs();

  return newFareMarket;
}

// ----------------------------------------------------------------------------
// @function void FareByRuleController::findMinAndMaxFares
//
// Description: When the rule tariff number, carrier code, fare class or fare type
//              (bytes 121-136) is/are populated and the fare retrieval process
//              completed that resolves to multiple fares, select the highest
//              amount for use in the comparison.
// ----------------------------------------------------------------------------
MoneyAmount
FareByRuleController::findMinAndMaxFares(const std::vector<PaxTypeFare*>* otherFares,
                                         bool sorted,
                                         const FareByRuleItemInfo& fbrItemInfo,
                                         const CurrencyCode& currency,
                                         const TariffNumber& baseRuleTariff)
{
  MoneyAmount highestAmount = 0;
  MoneyAmount minFareRange;
  MoneyAmount maxFareRange;

  if (fbrItemInfo.minFareAmt1() == 0 && fbrItemInfo.maxFareAmt1() == 0)
  {
    minFareRange = 0;
    maxFareRange = 0;
  }
  else if (fbrItemInfo.cur1() == currency)
  {
    minFareRange = fbrItemInfo.minFareAmt1();
    maxFareRange = fbrItemInfo.maxFareAmt1();
  }
  else if (fbrItemInfo.cur2() == currency)
  {
    minFareRange = fbrItemInfo.minFareAmt2();
    maxFareRange = fbrItemInfo.maxFareAmt2();
  }
  else
  {
    return highestAmount;
  }

  setAllowedVendors(fbrItemInfo.vendor());

  if (sorted)
  {
    std::vector<PaxTypeFare*>::const_reverse_iterator fare = otherFares->rbegin();

    for (; fare != otherFares->rend(); fare++)
    {
      MoneyAmount curFareAmt = (*fare)->originalFareAmount();

      if (isMinAndMaxFare(
              **fare, minFareRange, maxFareRange, fbrItemInfo, currency, baseRuleTariff))
      {
        highestAmount = curFareAmt;
        break;
      }
    }
  }
  else
  {
    for (const auto fare : *otherFares)
    {
      MoneyAmount curFareAmt = fare->originalFareAmount();

      if (isMinAndMaxFare(
              *fare, minFareRange, maxFareRange, fbrItemInfo, currency, baseRuleTariff) &&
          curFareAmt > highestAmount)
      {
        highestAmount = curFareAmt;
      }
    }
  }

  return highestAmount;
}

bool
FareByRuleController::isMinAndMaxFare(const PaxTypeFare& ptf,
                                      MoneyAmount minFareRange,
                                      MoneyAmount maxFareRange,
                                      const FareByRuleItemInfo& fbrItemInfo,
                                      const CurrencyCode& currency,
                                      const TariffNumber& baseRuleTariff) const
{
  if (ptf.isFareByRule() || ptf.isDiscounted() ||
      (!isVendorAllowed(ptf.vendor()) && !matchFbrPref(ptf.vendor())))
  {
    return false;
  }

  MoneyAmount curFareAmt = ptf.originalFareAmount();

  if (ptf.currency() != currency)
  {
    return false;
  }

  if (minFareRange != 0 && curFareAmt < minFareRange)
  {
    return false;
  }

  if (maxFareRange != 0 && curFareAmt > maxFareRange)
  {
    return false;
  }

  if (!fbrItemInfo.baseFareClass().empty() &&
      !RuleUtil::matchFareClass(fbrItemInfo.baseFareClass().c_str(), ptf.fareClass().c_str()))
  {
    return false;
  }

  if (!fbrItemInfo.baseFareType().empty() &&
      !RuleUtil::matchFareType(fbrItemInfo.baseFareType(), ptf.fcaFareType()))
  {
    return false;
  }

  if (baseRuleTariff == 0)
  {
    if (ptf.tcrTariffCat() == 1)
    {
      return false;
    }
  }
  else
  {
    if (ptf.tcrRuleTariff() != baseRuleTariff)
    {
      return false;
    }
  }

  return true;
}

void
FareByRuleController::changePaxType(const BaseFareRule& baseFareRule, FareMarket& fareMarket) const
{
  fareMarket.paxTypeCortege().clear();
  fareMarket.allPaxTypeFare().clear();
  PaxTypeBucket* paxTypeCortege;
  _trx.dataHandle().get(paxTypeCortege);
  PaxType* paxType;
  _trx.dataHandle().get(paxType);
  paxType->paxType() = baseFareRule.basepsgType();
  paxType->paxTypeInfo() = getPaxTypeInfo(baseFareRule);
  paxTypeCortege->requestedPaxType() = paxType;
  paxTypeCortege->actualPaxType().push_back(paxType);
  fareMarket.paxTypeCortege().push_back(*paxTypeCortege);
}

RoutingUtil::PaxTypeFareMap*
FareByRuleController::createPaxTypeFareMap(const PTFRange& range) const
{
  RoutingUtil::PaxTypeFareMap* ptfMap;
  _trx.dataHandle().get(ptfMap);

  if (ptfMap == nullptr)
  {
    return nullptr;
  }

  RoutingFareType* rfType;
  _trx.dataHandle().get(rfType);

  if (rfType == nullptr)
  {
    return nullptr;
  }

  std::vector<PaxTypeFare*>::const_iterator it(range.first);

  for (; it != range.second; ++it)
  {
    ptfMap->insert(RoutingUtil::PaxTypeFareMap::value_type(*it, *rfType));
  }

  return ptfMap;
}

const std::vector<FareByRuleApp*>&
FareByRuleController::getFareByRuleApp(std::vector<FareByRuleApp*>& filteredFBRApp) const
{
  std::set<PaxTypeCode> paxTypeSet;

  for (const auto& ptc : _fareMarket.paxTypeCortege())
  {
    LOG4CXX_DEBUG(logger, "PTC: reqPaxType: " << ptc.requestedPaxType()->paxType());

    for (const auto pt : ptc.actualPaxType())
    {
      LOG4CXX_DEBUG(logger, "\tpaxType: " << pt->paxType());
      paxTypeSet.insert(pt->paxType());
    }
  }

  TktDesignator tktDesignator;

  if (UNLIKELY(isFdTrx()))
  {
    tktDesignator = _fdTrx->getRequest()->ticketDesignator();
  }
  else
  {
    if (UNLIKELY(_trx.getRequest()->isTktDesignatorEntry() && _fareMarket.travelSeg().size() > 0))
    {
      std::vector<TravelSeg*>::iterator iterTvl = _fareMarket.travelSeg().begin();
      int16_t segOrder = (*iterTvl)->segmentOrder();
      tktDesignator = _trx.getRequest()->tktDesignator(segOrder);
    }
  }

  std::vector<PaxTypeCode> paxTypes(paxTypeSet.begin(), paxTypeSet.end());
  const std::vector<FareByRuleApp*>& allFBRApp = getAllFareByRuleApp(tktDesignator, paxTypes);
  const std::vector<FareByRuleApp*>& fbrApp = filterByVendorCxrForCat31(allFBRApp, filteredFBRApp);

  if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
  {
    LOG4CXX_DEBUG(logger, "fbrApp count: " << fbrApp.size());

    for (const auto i : fbrApp)
    {
      LOG4CXX_DEBUG(
          logger, "\t" << i->carrier() << ", " << i->primePaxType() << ", ruleNo: " << i->ruleNo());
    }
  }

  return fbrApp;
}

const std::vector<FareByRuleApp*>&
FareByRuleController::getAllFareByRuleApp(const TktDesignator& tktDesignator,
                                          std::vector<PaxTypeCode>& paxTypes) const
{
  if (_trx.getRequest()->isMultiAccCorpId())
  {
    return getFareByRuleAppForMultiAC(
        tktDesignator, paxTypes, _trx.getRequest()->corpIdVec(), _trx.getRequest()->accCodeVec());
  }
  else if (UNLIKELY(_trx.isFlexFare()))
  {
    flexFares::GroupsData::const_iterator itr = _trx.getRequest()->getFlexFaresGroupsData().begin();
    flexFares::GroupsData::const_iterator itrE = _trx.getRequest()->getFlexFaresGroupsData().end();
    std::vector<std::string> corpIdVec;
    std::vector<std::string> accCodeVec;
    flexFares::GroupId groupId = 0;

    while (itr != itrE)
    {
      groupId = itr->first;
      std::copy(_trx.getRequest()->getFlexFaresGroupsData().getCorpIds(groupId).begin(),
                _trx.getRequest()->getFlexFaresGroupsData().getCorpIds(groupId).end(),
                std::back_inserter(corpIdVec));
      std::copy(_trx.getRequest()->getFlexFaresGroupsData().getAccCodes(groupId).begin(),
                _trx.getRequest()->getFlexFaresGroupsData().getAccCodes(groupId).end(),
                std::back_inserter(accCodeVec));
      itr++;
    }
    std::vector<FareByRuleApp*>& fareByRuleApps =
        getFareByRuleAppForMultiAC(tktDesignator, paxTypes, corpIdVec, accCodeVec);

    if (_trx.getFlexFaresTotalAttrs().matchEmptyAccCode())
    {
      const std::vector<FareByRuleApp*>& fbrAppEmptyAccCode =
          getFareByRuleApp("", "", tktDesignator, paxTypes);
      fareByRuleApps.insert(
          fareByRuleApps.end(), fbrAppEmptyAccCode.begin(), fbrAppEmptyAccCode.end());
    }
    return fareByRuleApps;
  }

  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX))
  {
    RexPricingRequest* rexRequest = static_cast<RexPricingRequest*>(_trx.getRequest());

    if (!rexRequest->newAndExcAccCode().empty() || !rexRequest->newAndExcCorpId().empty())
      return getFareByRuleAppForMultiAC(
          tktDesignator, paxTypes, rexRequest->newAndExcCorpId(), rexRequest->newAndExcAccCode());
  }

  return getFareByRuleApp(_trx.getRequest()->corporateID(),
                          _trx.getRequest()->accountCode().c_str(),
                          tktDesignator,
                          paxTypes);
}

void
FareByRuleController::processFareByRuleApp(FareByRuleApp* fbrApp,
                                           DiagManager& diag,
                                           std::map<std::string, bool>& ruleTariffMap)
{
  if (UNLIKELY(!fbrApp))
    return;

  Diagnostic& trxDiag = _trx.diagnostic();

  if (UNLIKELY(!isValidForDiagnostic(fbrApp)))
    return;

  if(_trx.excTrxType() == PricingTrx::NOT_EXC_TRX)
  {
     if(_trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
        RuleUtil::isVendorPCC(fbrApp->vendor(), _trx))
       return;
  }

  FareByRuleProcessingInfo fbrProcessingInfo(
      &_trx, &_itin, &_fareMarket, fbrApp, nullptr, nullptr, &diag, false, isFdTrx(), false, false, _spanishDiscountApplied,
      false, false);

  FareByRuleAppValidator fbrAppValidator(&diag);

  if (!fareByRuleAppValidate(fbrAppValidator, fbrProcessingInfo, ruleTariffMap))
  {
    return;
  }

  if (UNLIKELY(!_isIndApplQueried && fbrApp->carrier() == INDUSTRY_CARRIER))
  {
    _multiAppls = getIndustryFareAppl(MULTILATERAL);
    _indAppls = getIndustryFareAppl(INDUSTRY);
  }

  Diag325Collector* diag325 = nullptr;

  if (trxDiag.diagnosticType() == Diagnostic325)
  {
    diag325 = dynamic_cast<Diag325Collector*>(&diag.collector());
    *diag325 << *fbrApp;
  }

  bool isLocationSwapped = false;
  FareByRuleCtrlInfoVec fbrCtrlInfoVec; // move below when removing fallback!
  const FareByRuleCtrlInfo* fbrCtrlInfo = nullptr;

  // dont't be confused by fallback name, this is just cosmetic move that's why I've done it
  // under fallback for footnote - all will be cleared with fallback removal
  if (fallback::fallbackFootNoteR2Optimization(&_trx))
    getFareByRuleCtrlInfo(*fbrApp, fbrCtrlInfoVec, diag);
  else
    fbrCtrlInfoVec = Rec2Selector::getFareByRuleCtrlInfo(_trx, *fbrApp, _fareMarket);

  if (fbrCtrlInfoVec.empty())
  {
    if (trxDiag.diagnosticType() == Diagnostic325)
    {
      diag325->diag325Collector(fbrCtrlInfo, Diag325Collector::R2_NOT_FOUND);
    }

    return;
  }

  bool isHistorical = _trx.dataHandle().isHistorical();
  DateTime& ticketDate = _trx.ticketingDate();

  // looping through Fare By Rule Control vector
  for (const auto& fareByRuleCtrlInfo : fbrCtrlInfoVec)
  {
    fbrCtrlInfo = fareByRuleCtrlInfo.first;

    isLocationSwapped = fareByRuleCtrlInfo.second;

    if (trxDiag.diagnosticType() == Diagnostic225)
    {
      Diag225Collector* diag225 = dynamic_cast<Diag225Collector*>(&diag.collector());
      diag225->diag225Collector(fbrCtrlInfo, Diag225Collector::R2_PASS, _fareMarket);
    }

    if (fbrCtrlInfo->applInd() == FareByRuleController::NOT_APPLICABLE_X)
    {
      if (trxDiag.diagnosticType() == Diagnostic325)
      {
        diag325->diag325Collector(fbrCtrlInfo, Diag325Collector::R2_NOT_APPL);
      }

      if (LIKELY(!isHistorical || isFdTrx()))
        return;
    }

    if (trxDiag.diagnosticType() == Diagnostic325)
    {
      diag325->diag325Collector(fbrCtrlInfo, Diag325Collector::R2_PASS);
    }

    if (!processRule(fbrProcessingInfo, *fbrCtrlInfo, isLocationSwapped))
    {
      LOG4CXX_WARN(logger, " *** Rec 2 Cat 25 Process Rule Return False *** ");
    }

    if (LIKELY(!isHistorical || isFdTrx() || ticketDate.date() > fbrCtrlInfo->createDate().date()))
    {
      break;
    }
  }
}

bool
FareByRuleController::isValidForDiagnostic(const FareByRuleApp* fbrApp) const
{
  Diagnostic& trxDiag = _trx.diagnostic();

  if (UNLIKELY(trxDiag.diagnosticType() == Diagnostic208 || trxDiag.diagnosticType() == Diagnostic225 ||
      _diag325Requested))
  {
    if (_diagWithRuleNumber &&
        trxDiag.diagParamMapItem(Diagnostic::RULE_NUMBER) != fbrApp->ruleNo())
    {
      return false;
    }
  }

  return true;
}

// find carrier pref rec and store pointer to it
//    - gets derfault rec if no exact match
//    - Rec3 (ruledata) determines carrier to use
//    - skip db read if still working with the same carrier
void
FareByRuleController::setCarrierPref(const CarrierCode& ruleCarrier)
{
  if (_carrierPref && _carrierPref->carrier() == ruleCarrier)
    return;

  // new one needed
  _carrierPref = getCarrierPreference(ruleCarrier);
}

// return true if carrier pref allows this vendor's fare to be a base fare
//    - also false if no carrier pref previously set
//    - independent of VendorXRef check
bool
FareByRuleController::matchFbrPref(const VendorCode& fareVendor) const
{
  if (UNLIKELY(!_carrierPref))
    return false;

  if (LIKELY(isVendorATP(fareVendor) || isVendorSITA(fareVendor) || isVendorSMFA(fareVendor) ||
      isVendorSMFC(fareVendor)))
  {
    return std::find(_carrierPref->fbrPrefs().begin(),
                     _carrierPref->fbrPrefs().end(),
                     fareVendor) != _carrierPref->fbrPrefs().end();
  }

  VendorCode ruleVendor = fareVendor;

  if (_trx.dataHandle().getVendorType(fareVendor) == RuleConst::SMF_VENDOR)
  {
    ruleVendor = Vendor::SABRE;
  }

  return std::find(_carrierPref->fbrPrefs().begin(), _carrierPref->fbrPrefs().end(), ruleVendor) !=
         _carrierPref->fbrPrefs().end();
}

void
FareByRuleController::initFareMarketCurrency()
{
  getPricingCurrency(_fareMarket.origin()->nation(), _fmOrigCurr);
  getPricingCurrency(_fareMarket.destination()->nation(), _fmDestCurr);

  switch (_fareMarket.direction())
  {
  case FMDirection::OUTBOUND:
    _fmCurrency = _fmOrigCurr;
    break;

  case FMDirection::INBOUND:
    _fmCurrency = _fmDestCurr;
    break;

  case FMDirection::UNKNOWN:
  default:
    break;
  }
}

uint16_t
FareByRuleController::getMileage(FareMarket& fareMarket, PricingTrx& trx)
{
  const DateTime& travelDate = fareMarket.travelDate();
  GlobalDirection gd;

  uint16_t mileage = 0;

  for (const auto travelSeg : fareMarket.travelSeg())
  {
    const Loc& orig = *travelSeg->origin();
    const Loc& dest = *travelSeg->destination();
    GlobalDirectionFinderV2Adapter::getGlobalDirection(
        &trx, fareMarket.travelDate(), *travelSeg, gd);

    mileage += LocUtil::getTPM(orig, dest, gd, travelDate, trx.dataHandle());
  }

  return mileage;
}

MoneyAmount
FareByRuleController::calculateFareAmtPerMileage(MoneyAmount& fareAmount, uint16_t mileage)
{
  fareAmount = (fareAmount * (mileage / 100.0));
  return fareAmount;
}

void
FareByRuleController::addBaseFareInfoBkcAvailMap(PaxTypeFare* ptFare, BookingCode bookingCode)
{
  std::map<PaxTypeFare*, std::set<BookingCode> >::iterator baseFareIter =
      _baseFareInfoBkcAvailMap.find(ptFare);

  if (baseFareIter == _baseFareInfoBkcAvailMap.end())
  {
    std::set<BookingCode> bkcs;
    bkcs.insert(bookingCode);
    _baseFareInfoBkcAvailMap.insert(std::pair<PaxTypeFare*, std::set<BookingCode> >(ptFare, bkcs));
  }
  else
  {
    baseFareIter->second.insert(bookingCode);
  }
}

bool
FareByRuleController::checkDisplayType(const PaxTypeFare& ptf) const
{
  return (ptf.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
          ptf.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD);
}

namespace
{
template <typename T>
inline void operator+=(std::vector<T*>& left, const std::vector<T*>& right)
{
  left.insert(left.end(), right.begin(), right.end());
}
}

std::vector<FareByRuleApp*>&
FareByRuleController::getFareByRuleAppForMultiAC(const TktDesignator& tktDesignator,
                                                 std::vector<PaxTypeCode>& paxTypes,
                                                 const std::vector<std::string>& corpIdVec,
                                                 const std::vector<std::string>& accCodeVec) const
{
  std::vector<FareByRuleApp*> fbra;

  for (const auto& iC : corpIdVec)
    fbra += getFareByRuleApp(iC, "", tktDesignator, paxTypes);

  for (const auto& iA : accCodeVec)
    fbra += getFareByRuleApp("", iA.c_str(), tktDesignator, paxTypes);

  std::sort(fbra.begin(), fbra.end());
  std::vector<FareByRuleApp*>* fbra_ptr;
  _trx.dataHandle().get(fbra_ptr);
  fbra_ptr->assign(fbra.begin(), std::unique(fbra.begin(), fbra.end()));
  return *fbra_ptr;
}

bool
FareByRuleController::matchBaseFareVendor(const VendorCode& baseFareVendor,
                                          const VendorCode& fbrAppVendor) const
{
  if (LIKELY(isVendorATP(baseFareVendor) || isVendorSITA(baseFareVendor) || isVendorATP(fbrAppVendor) ||
      isVendorSITA(fbrAppVendor)))
  {
    return true;
  }

  if (isVendorSMFA(baseFareVendor)) // SMFA
  {
    if (isVendorSMFC(fbrAppVendor))
    {
      return false;
    }
  }
  else if (isVendorSMFC(baseFareVendor)) // SMFC
  {
    if (isVendorSMFA(fbrAppVendor))
    {
      return false;
    }
  }
  else
  {
    if (baseFareVendor != fbrAppVendor) // PCC
    {
      return false;
    }
  }

  return true;
}

bool
FareByRuleController::matchBaseFareSecurity(const PaxTypeFare& ptFare,
                                            const FareByRuleProcessingInfo& fbrProcessingInfo)
{
  if (UNLIKELY(fbrProcessingInfo.isFbrAppVendorPCC() &&
               (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
                ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
                ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD) &&
                (isVendorATP(ptFare.vendor()) || isVendorSITA(ptFare.vendor()) ||
                isVendorSMFA(ptFare.vendor()) || isVendorSMFC(ptFare.vendor()))))
  {
    // validate T983
    GeneralFareRuleInfo* cat35r2 = getCat35Rec2(ptFare);

    if (!cat35r2)
      return false;

    const Agent* agent = _trx.getRequest()->ticketingAgent();

    if (!agent)
      return false;

    const VendorCode r8PccVendorCode = fbrProcessingInfo.fbrApp()->vendor();

    if (r8PccVendorCode != agent->tvlAgencyPCC())
    {
      agent = getRecord8Agent(r8PccVendorCode);

      if (!agent)
        return false;
    }

    bool ticketOrExchange = TrxUtil::isExchangeOrTicketing(_trx);
    for (CategoryRuleItemInfoSet* ruleItemInfoSet : cat35r2->categoryRuleItemInfoSet())
    {
      if (!ruleItemInfoSet)
        continue;

      for (const CategoryRuleItemInfo& catRuleItemInfo : (*ruleItemInfoSet))
      {
        if (catRuleItemInfo.itemcat() != RuleConst::NEGOTIATED_RULE)
          continue;

        const NegFareRest* cat35r3 =
            _trx.dataHandle().getNegFareRest(cat35r2->vendorCode(), catRuleItemInfo.itemNo());

        if (!cat35r3)
          continue;

        const std::vector<NegFareSecurityInfo*>& t983s = _trx.dataHandle().getNegFareSecurity(
            cat35r2->vendorCode(), cat35r3->negFareSecurityTblItemNo());
        for (NegFareSecurityInfo* t983 : t983s)
        {
          const NegFareSecurity secObj(t983);

          if (secObj.isMatch(_trx, agent))
          {
            if (secObj.isPos() && secObj.isMatchWhat(ticketOrExchange))
            {
              return true;
            }
            else if (!secObj.isPos())
            {
              return false;
            }
          }
        }
      }
    }
    return false;
  }

  return true;
}

Agent*
FareByRuleController::getRecord8Agent(const VendorCode& r8PccVendorCode)
{
  std::map<const VendorCode, Agent*>::const_iterator r8AgentMapIter =
      _r8AgentMap.find(r8PccVendorCode);

  if (r8AgentMapIter != _r8AgentMap.end())
    return r8AgentMapIter->second;

  std::vector<Customer*> custList = _trx.dataHandle().getCustomer(r8PccVendorCode);

  if (custList.empty())
    return nullptr;

  Agent* agent = nullptr;
  _trx.dataHandle().get(agent);

  if (!agent)
    return nullptr;

  agent->agentTJR() = custList.front();
  agent->agentCity() = agent->agentTJR()->aaCity();
  agent->agentLocation() = _trx.dataHandle().getLoc(agent->agentCity(), time(nullptr));
  agent->tvlAgencyPCC() = agent->agentTJR()->pseudoCity();
  agent->mainTvlAgencyPCC() = agent->agentTJR()->snapHomeAgencyPcc().empty()
                                  ? agent->agentTJR()->homePseudoCity()
                                  : agent->agentTJR()->snapHomeAgencyPcc();
  agent->tvlAgencyIATA() = agent->agentTJR()->arcNo();
  agent->homeAgencyIATA() = agent->agentTJR()->homeArcNo();
  _r8AgentMap.insert(std::make_pair(r8PccVendorCode, agent));
  return agent;
}

bool
FareByRuleController::isVendorATP(const VendorCode& vendor) const
{
  return (vendor == ATPCO_VENDOR_CODE);
}

bool
FareByRuleController::isVendorSITA(const VendorCode& vendor) const
{
  return (vendor == SITA_VENDOR_CODE);
}

bool
FareByRuleController::isVendorSMFA(const VendorCode& vendor) const
{
  return (vendor == SMF_ABACUS_CARRIER_VENDOR_CODE);
}

bool
FareByRuleController::isVendorSMFC(const VendorCode& vendor) const
{
  return (vendor == SMF_CARRIER_VENDOR_CODE);
}

GeneralFareRuleInfo*
FareByRuleController::getCat35Rec2(const PaxTypeFare& ptFare) const
{
  bool isLocSwapped(false);
  return RuleUtil::getGeneralFareRuleInfo(
      _trx, ptFare, RuleConst::NEGOTIATED_RULE, isLocSwapped, nullptr, nullptr);
}

void
FareByRuleController::addToFailBaseFareList(const PaxTypeFare& ptFare, const char* msg)
{
  if (UNLIKELY(_diagWithT989))
  {
    DiagManager diag(_trx, Diagnostic325);
    Diag325Collector* diag325 = static_cast<Diag325Collector*>(&diag.collector());
    std::string str = diag325->formatFailBaseFareMessage(ptFare, msg);

    if (str.length())
    {
      _failBaseFareList.push_back(str);
    }
  }
}

void
FareByRuleController::baseFareFailedValidation(const PaxTypeFare& ptFare)
{
  if (UNLIKELY(_diagWithT989))
  {
    uint16_t failedCat = ptFare.getFailedCatNum(_baseFareValidator.categorySequence());

    if (!failedCat)
      failedCat = RuleConst::SALE_RESTRICTIONS_RULE;

    std::string message = "RULE " + std::to_string(failedCat) + " VALIDATION";

    addToFailBaseFareList(ptFare, message.c_str());
  }
}

bool
FareByRuleController::checkQualifierPassed(const CategoryRuleItemInfo* const catRuleItemInfo,
                                           DiagManager& diag,
                                           bool qualifierPasses)
{
  if (UNLIKELY(catRuleItemInfo == nullptr))
  {
    LOG4CXX_DEBUG(logger, "FBRC::processRule() - Skipping null CategoryRuleItemInfo pointer");
    return false;
  }

  Diag325Collector* diag325 = nullptr;

  if (_trx.diagnostic().diagnosticType() == Diagnostic325)
    diag325 = dynamic_cast<Diag325Collector*>(&diag.collector());

  if (!qualifierPasses)
  {
    if (UNLIKELY(diag325))
      diag325->diag325Collector(catRuleItemInfo, Diag325Collector::R2_FAIL_IF);

    return false;
  }

  return true;
}

void
FareByRuleController::diagDisplayRelation(const CategoryRuleItemInfo* const catRuleItemInfo,
                                          DiagManager& diag,
                                          Record3ReturnTypes retCode)
{
  if (UNLIKELY(catRuleItemInfo == nullptr))
    return;

  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic225))
    diag.collector().displayRelation(catRuleItemInfo, retCode);
}

void
FareByRuleController::printFareHeader(DiagManager& diag,
                                      PaxTypeFare* ptFare,
                                      FareByRuleProcessingInfo& fbrProcessingInfo)
{
  if (_displayFareHdr)
  {
    if (ptFare->isSpecifiedFare())
    {
      diag << "RESULTING CAT25 INFORMATION:\n";
      diag << "CAT25\n";
      diag << "F/C           AMT CUR";
    }
    else
    {
      if (_spanishLargeFamilyDiscountApplied &&
          (fbrProcessingInfo.fbrItemInfo()->fareInd() == CALCULATED ||
           fbrProcessingInfo.fbrItemInfo()->fareInd() ==
               SUBTRACT_SPECIFIED_FROM_BASE_CALC_PERCENTAGE) &&
          (fbrProcessingInfo.isResidenceFiledInR8() ||
           fbrProcessingInfo.isResidenceFiledInR3Cat25()))
      {
        diag << "COMBINED SPANISH LARGE FAMILY DISCOUNT PCT ";
        diag << fbrProcessingInfo.combinedPercent() << " APPLIED\n";
      }
      diag << "RESULTING CAT25 INFORMATION:\n";
      diag << "CAT25                 BASE     F   D O I\n";
      diag << "F/C           AMT CUR F/C      T   T R O TRF RULE      AMT CUR";
    }

    diag << "\n";
    _displayFareHdr = false;
  }
}

void
FareByRuleController::setCombinedPercent(FareByRuleProcessingInfo& fbrProcessingInfo)
{
  const FareByRuleItemInfo& fbrItemInfo = *fbrProcessingInfo.fbrItemInfo();

  if (fbrItemInfo.fareInd() == CALCULATED ||
      fbrItemInfo.fareInd() == SUBTRACT_SPECIFIED_FROM_BASE_CALC_PERCENTAGE)
  {
    MoneyAmount combinedPercent = fbrItemInfo.percent() - _largeFamilyDiscountPercent;
    if (combinedPercent < 0.0)
      combinedPercent = 0.0;
    fbrProcessingInfo.combinedPercent() = combinedPercent;
  }
}

bool
FareByRuleController::runCat35If1PreValidation(FareByRuleProcessingInfo& fbrProcessingInfo,
                                               PaxTypeFare& dummyPtFare)
{
  const FareByRuleItemInfo& fbrItemInfo = *(fbrProcessingInfo.fbrItemInfo());
  FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();

  if ((fbrItemInfo.resultDisplaycatType() == RuleConst::SELLING_FARE ||
       fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE ||
       fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE_UPD) &&
       fbrApp.primePaxType() == ADULT &&
       fbrApp.accountCode().empty())
  {
    if (fbrProcessingInfo.isCat35If1PreValidationProcessed()) // Cat 35 IF 1 prevalidated for this VCTR
    {
      if (!fbrProcessingInfo.isCat35If1PreValidationPassed()) // Cat 35 IF 1 failed
      {
        displayCat35If1PreValidation(fbrProcessingInfo);
        return false; // Fail this Rec 3 Cat 25
      }
      return true; // continue to process Rec 3 Cat 25
    }

    fbrProcessingInfo.isCat35If1PreValidationProcessed() = true;
    if (prevalidateCat35If1(dummyPtFare,
                            fbrApp.vendor(),
                            fbrApp.carrier(),
                            fbrApp.ruleTariff(),
                            fbrApp.ruleNo()))
      fbrProcessingInfo.isCat35If1PreValidationPassed() = true;
    else
    {
      displayCat35If1PreValidation(fbrProcessingInfo);
      return false;
    }
  }
  return true; // continue to process Rec 3 Cat 25
}

void
FareByRuleController::displayCat35If1PreValidation(FareByRuleProcessingInfo& fbrProcessingInfo) const
{
  if (UNLIKELY(_diag325Requested))
  {
    if (fbrProcessingInfo.diagManager())
    {
      *fbrProcessingInfo.diagManager() << "***FAIL - CAT35 IF1 PREVALIDATION***" << '\n';
    }
    else
    {
      DiagManager diag(_trx, Diagnostic325);
      diag << "***FAIL - CAT35 IF1 PREVALIDATION***" << '\n';
    }
  }
}

}
