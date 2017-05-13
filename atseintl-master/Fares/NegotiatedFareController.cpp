//-------------------------------------------------------------------
//
//  File:        NegotiatedFareController.cpp
//  Created:     Aug 2, 2004
//  Authors:     Nakamon Thamsiriboon/Jeff Hoffman/Lipika Bardalai
//
//  Description: Negotiated fare factory
//
//  Updates:
//
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
//-------------------------------------------------------------------
//

#include "Fares/NegotiatedFareController.h"

#include "Common/Assert.h"
#include "Common/CurrencyConversionCache.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/DiagMonitor.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/CorpId.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareProperties.h"
#include "DBAccess/FareRetailerCalcInfo.h"
#include "DBAccess/FareRetailerResultingFareAttrInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MarkupControl.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/PrintOption.h"
#include "DBAccess/Record2Types.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag335Collector.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Fares/DiscountedFareController.h"
#include "Fares/FareUtil.h"
#include "Fares/NegFareCalc.h"
#include "Fares/NegFareSecurity.h"
#include "Fares/NegotiatedPtcHierarchy.h"
#include "Rules/FareMarketDataAccess.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <memory>
#include <sstream>


namespace tse
{
const Indicator NegotiatedFareController::BLANK = ' '; // for the rule directionality
const Indicator NegotiatedFareController::LOC1_TO_LOC2 = '1'; // for the rule directionality
const Indicator NegotiatedFareController::LOC2_TO_LOC1 = '2'; // for the rule directionality
const Indicator NegotiatedFareController::DECLINED = 'D';
const Indicator NegotiatedFareController::PENDING = 'P';
const MoneyAmount NegotiatedFareController::INVALID_AMT = -1.0;

FALLBACK_DECL(fallbackProcessTypeLAndT);
FALLBACK_DECL(fallbackFRRCat25AmtCheckFix);
FALLBACK_DECL(fallbackFRRCat25Responsive);
FALLBACK_DECL(cat35_psgTypeMatching);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(fallbackFRRRedistFix);

namespace
{
Logger
logger("atseintl.Fares.NegotiatedFareController");
}

NegotiatedFareController::NegotiatedFareController(PricingTrx& trx,
                                                   Itin& itin,
                                                   FareMarket& fareMarket)
  : FareController(trx, itin, fareMarket),
    _dc(DCFactory::instance()->create(_trx)),
    _dcDisplayRelation(_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) ==
                       "RELATION"),
    _fareRetailerRuleValidator(trx),
    _ccf(new CurrencyConversionFacade),
    _cache(trx.dataHandle())
{
  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic335))
  {
    _dc->enable(Diagnostic335);
  }
  else
  {
    _dc->deActivate();
  }

  _isJalAxessUser = _trx.getRequest()->ticketingAgent()->axessUser();
  _isJalAxessC35Request =
      (_trx.getRequest()->isWpNettRequested() || _trx.getRequest()->isWpSelRequested());
  _isJalAxessWPNETT = _trx.getRequest()->isWpNettRequested();
  // we may need to bypass creating Cat35 fares because of pricing option,
  // although we still need to go through the loop to kill cat35 base fares.
  _needByPassNegFares = (_trx.getOptions() && _trx.getOptions()->fareX());

  // load PTC hierarchy if needed
  static bool hierarchyLoaded = (NegotiatedPtcHierarchy::loadPtcHierarchy());
  SUPPRESS_UNUSED_WARNING(hierarchyLoaded);

  LOG4CXX_DEBUG(logger, "hierarchyLoaded:" << hierarchyLoaded);

  _itinIndex = RexPricingTrx::isRexTrxAndNewItin(_trx)
                   ? static_cast<RexPricingTrx*>(&_trx)->getItinPos(&_itin)
                   : 0;
  _isHierarchyApplied = false;
  _negPtfBucketContainer = std::make_shared<NegPTFBucketContainer<NegPaxTypeFareDataComparator>>();

  _trx.dataHandle().get(_ruleDataFareRetailer);
  _ruleDataFareRetailer->sourcePseudoCity() = "";
  _ruleDataFareRetailer->fareRetailerRuleId() = 0;
  _ruleDataFareRetailer->frrSeqNo() = 0;
  _ruleDataFareRetailer->fareRetailerCode() = "";
}

bool
NegotiatedFareController::process()
{
  TSELatencyData metrics(_trx, "FCO NEGFARECNTRLR");

  LOG4CXX_DEBUG(logger, "NFC::process() - begin");

  setRexCreateFaresForPrevAgent();

  getFareRetailerRuleLookupInfo();

  *_dc << _fareMarket;

  // get diagnostic parameters
  const std::string& diagRule = _trx.diagnostic().diagParamMapItem(Diagnostic::RULE_NUMBER);
  const std::string& diagFareClass =
      _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);

  // Get all PaxTypeFares from each bucket
  std::vector<PaxTypeFare*>& allFares = _fareMarket.allPaxTypeFare();
  std::vector<PaxTypeFare*>::iterator fareIter = allFares.begin();
  std::vector<PaxTypeFare*>::iterator fareEnd = allFares.end();

  static const std::string passSecurityMsg("PASS SECURITY");

  for (; fareIter != fareEnd; ++fareIter)
  {
    _passSecurity = false;
    PaxTypeFare& ptFare = *(*fareIter);

    bool sfaTag = false;
    std::vector<FareClassCode> fareClasses;
    sfaTag = sfaDiagRequested(_trx, fareClasses);
    if (sfaTag)
    {
      uint16_t fareNum = 0;
      for (; fareNum != fareClasses.size(); ++fareNum)
      {
        if (ptFare.fareClass() == fareClasses[fareNum])
          break;
      }
      if (fareNum >= fareClasses.size())
        continue;
      if (ptFare.fareClass() != fareClasses[fareNum])
        continue;
    }

    if (UNLIKELY(_dc->diagnosticType() == Diagnostic335 &&
                  ((!diagRule.empty() && diagRule != ptFare.ruleNumber()) ||
                   (!diagFareClass.empty() && diagFareClass != ptFare.fareClass()))))
    {
      continue;
    }

    if (checkFareJalAxess(ptFare) && checkFareRexTrx(ptFare) && checkFareTariffCarrier(ptFare))
    {
      if (LIKELY(!_needByPassNegFares))
      {
        processFare(ptFare);

        if (UNLIKELY(_passSecurity))
        {
          doDiag335Collector(passSecurityMsg, true);
        }
        else
        {
          doDiag335Collector(std::string(),
                             DiagnosticUtil::isvalidForCarrierDiagReq(_trx, ptFare) ||
                                 _dcDisplayRelation);
        }
      }
      invalidateBaseFare(ptFare);
    }
  }

  if (LIKELY(!_needByPassNegFares))
  {
    updateFareMarket();
  }

  if (_dc->diagnosticType() == Diagnostic335)
    _dc->flushMsg();

  return true;
}

bool
NegotiatedFareController::getFareRetailerRuleLookupInfo()
{
  if (!_trx.getRequest() || !_trx.getRequest()->ticketingAgent() ||
      _trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
    return false;

  _fareRetailerRuleValidator.getFRRLookupAllSources(_frrlVecTypeC, &_fareMarket, "RN", 'N', true);
  _fareRetailerRuleValidator.getFRRLookupAllSources(_frrlVecTypeLT, &_fareMarket, "RN", 'R', false);
  if (!fallback::fallbackFRRCat25Responsive(&_trx))
    _fareRetailerRuleValidator.getFRRLookupAllSources(_frrlVecTypeCFbr, &_fareMarket, "RN", 'D', false);

  return true;
}

void
NegotiatedFareController::updateFareMarket()
{
  std::vector<PaxTypeFare*>& allFares = _fareMarket.allPaxTypeFare();

  // Copy created fares to fare market
  if (!_negPaxTypeFares.empty())
  {
    std::vector<PaxTypeFare*>::iterator negFareIter = _negPaxTypeFares.begin();
    std::vector<PaxTypeFare*>::iterator negFareEnd = _negPaxTypeFares.end();

    for (; negFareIter != negFareEnd; ++negFareIter)
    {
      PaxTypeFare* ptFare = *negFareIter;

      allFares.push_back(ptFare);
      addFareToPaxTypeBucket(*ptFare);
    }
    _negPaxTypeFares.clear();
  }
}

bool
NegotiatedFareController::checkFareJalAxess(PaxTypeFare& ptFare) const
{
  bool ret = true;

  // JAL/AXESS user for Pricing/Shopping entries:
  //    - do not process non-Cat35 fares on WPSEL/WPNETT entries for JAL/AXESS user.
  //    - do not process Cat35 fares on WP/WPNC/WPNI... entries.
  //    - process WPNET as non-JAL/AXESS users
  if (UNLIKELY(_isJalAxessUser && !isFdTrx() && !_trx.getOptions()->isCat35Net() &&
                ptFare.isValid()))
  {
    bool checkFcaDisplayCatType = isSellingOrNet(ptFare.fcaDisplayCatType());

    if ((_isJalAxessC35Request && !checkFcaDisplayCatType) ||
        (!_isJalAxessC35Request && checkFcaDisplayCatType))
    {
      // kill Cat35 fare.
      //  setup "failedByJalAxess" indicator for diagnostic..
      ptFare.setCategoryValid(RuleConst::NEGOTIATED_RULE, false);
      ptFare.failedByJalAxessRequest() = true;

      ret = false;
    }
  }

  return ret;
}

bool
NegotiatedFareController::isSellingOrNet(const Indicator fcaDisplayCatType)
{
  return fcaDisplayCatType == RuleConst::SELLING_FARE ||
         fcaDisplayCatType == RuleConst::NET_SUBMIT_FARE ||
         fcaDisplayCatType == RuleConst::NET_SUBMIT_FARE_UPD;
}

bool
NegotiatedFareController::checkFare(PaxTypeFare& ptFare) const
{
  // return true if all conditions are met
  return checkFareJalAxess(ptFare) && checkFareRexTrx(ptFare) && checkFareTariffCarrier(ptFare) &&
         checkFareNeg(ptFare) && checkFareCat25WithCat35(ptFare);
}

bool
NegotiatedFareController::checkFareRexTrx(const PaxTypeFare& ptFare) const
{
  return ptFare.isValid() || (RexPricingTrx::isRexTrxAndNewItin(_trx) &&
                              static_cast<RexPricingTrx*>(&_trx)->needRetrieveKeepFare(_itinIndex));
}

bool
NegotiatedFareController::checkFareTariffCarrier(const PaxTypeFare& ptFare)
{
  bool ret = true;

  // The Record 1 and Record 3 Cat 25 Display Category Type values applicable for
  // Negotiated Fares are L, C and T. These codes only apply for private tariffs.
  if (ptFare.tcrTariffCat() != RuleConst::PRIVATE_TARIFF ||
      !isSellingOrNet(ptFare.fcaDisplayCatType()) || ptFare.carrier() == INDUSTRY_CARRIER)
  {
    ret = false;
  }

  return ret;
}

bool
NegotiatedFareController::checkFareNeg(PaxTypeFare& ptFare) const
{
  // For Pricing, always fail this ptFare.
  // For FD, reuse published C/T fares for net ptf enties
  // For FD, clone L fares (like Pricing) to get proper paxType (from cat1?)
  if (!isFdTrx() || ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE)
    ptFare.setCategoryValid(RuleConst::NEGOTIATED_RULE, false);

  // For Axess users fail C type "parent" fare for FD
  if (isFdTrx() && _isJalAxessUser && ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD)
    ptFare.setCategoryValid(RuleConst::NEGOTIATED_RULE, false);

  // now the cat35 base fare is killed, check if we need to bypass the
  // negotiated fare creation
  return !_needByPassNegFares;
}

void
NegotiatedFareController::invalidateBaseFare(PaxTypeFare& ptFare) const
{
  // For Pricing, always fail this ptFare.
  // For FD, reuse published C/T fares for net ptf enties
  // For FD, clone L fares (like Pricing) to get proper paxType (from cat1?)
  if (LIKELY(!isFdTrx() || ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE))
    ptFare.setCategoryValid(RuleConst::NEGOTIATED_RULE, false);

  // For Axess users fail C type "parent" fare for FD
  if (UNLIKELY(isFdTrx() && _isJalAxessUser &&
                ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))
    ptFare.setCategoryValid(RuleConst::NEGOTIATED_RULE, false);

  if (ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD && UNLIKELY(isFdTrx()))
  {
    const NegPaxTypeFareRuleData* negPtfRule = ptFare.getNegRuleData();
    if (negPtfRule != nullptr && negPtfRule->fareRetailerRuleId())
    {
      if (!_trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
          _trx.getRequest()->ticketingAgent()->tvlAgencyPCC() != negPtfRule->sourcePseudoCity())
        ptFare.invalidateFare(PaxTypeFare::FD_Cat35_ViewNetIndicator);
    }
  }
}

bool
NegotiatedFareController::checkFareCat25WithCat35(const PaxTypeFare& ptFare) const
{
  // For Airlines, allow Cat 25 fare with Cat 35 to ticket
  //
  // _trx.getOptions()->cat35NotAllowed() is built from P59 (CAT35_CODNOTALLOWED)
  // P59 value 'A' is for TX0AI1,#TX0T35
  // P59 value 'T' is for TX0SP8,#TX0C5X
  return !_trx.getOptions() || _trx.getOptions()->cat35NotAllowed() != 'A' || ptFare.isFareByRule();
}

void
NegotiatedFareController::processFare(PaxTypeFare& ptFare)
{
  setCat35Indicator(ptFare);

  *_dc << ptFare;

  _frrcVecTypeC.clear();
  _frrcVecTypeLT.clear();
  _frrcVecTypeCFbr.clear();

  if (LIKELY(checkNotNetFareForAxess(ptFare)))
  {
    // Get general fare rule info for current pax type fare
    GeneralFareRuleInfoVec gfrInfoVec;
    getGeneralFareRuleInfo(ptFare, gfrInfoVec);

    // Check if there are no rules available
    if (checkGeneralFareRuleInfo(ptFare, gfrInfoVec))
    {
      GeneralFareRuleInfoVec::iterator iter = gfrInfoVec.begin();
      GeneralFareRuleInfoVec::iterator iterEnd = gfrInfoVec.end();
      bool isHistorical = _trx.dataHandle().isHistorical();
      DateTime& ticketDate = _trx.ticketingDate();

      // looping through General Fare Rule vector
      for (; iter != iterEnd; ++iter)
      {
        _ruleInfo = (*iter).first;
        const bool isLocationSwapped = (*iter).second;
        _isLocationSwapped = isLocationSwapped;

        *_dc << *_ruleInfo;

        // Check, if current rule can be applied
        if (UNLIKELY(_ruleInfo->applInd() == RuleConst::STRING_DOES_NOT_APPLY))
        {
          doDiag335Collector(Diag335Collector::R2_NOT_APPL, false);
          continue;
        }

        // Process current rule
        processCurrentRule(ptFare, isLocationSwapped);

        if (LIKELY(!isHistorical || isFdTrx() ||
                    ticketDate.date() > _ruleInfo->createDate().date()))
          break;
      }
    }
  }
}

void
NegotiatedFareController::processCurrentRule(PaxTypeFare& ptFare, bool isLocationSwapped)
{
  try
  {
    processRule(ptFare, isLocationSwapped);

    if (UNLIKELY(_rexCreateFaresForPrevAgent))
    {
      _processPrevAgentForRex = true;
      processRule(ptFare, isLocationSwapped);
      _processPrevAgentForRex = false;
    }
  }
  catch (ErrorResponseException& ex) // handle exceptions
  {
    LOG4CXX_INFO(logger,
                 "NFC::process() - Process Rule exception catch: " << ex.what()); // print to logger
    ptFare.setCategoryValid(RuleConst::NEGOTIATED_RULE, false); // fail fare

    std::ostringstream oss;
    oss << "FARE FAILED-" << ex.what() << std::endl; // print to diagnostic
    doDiag335Collector(oss.str(), false);
    _processPrevAgentForRex = false; // always switch off this flag
  }
}

bool
NegotiatedFareController::checkNotNetFareForAxess(const PaxTypeFare& ptFare)
{
  bool ret = true;

  if (UNLIKELY(_isJalAxessUser && !_isJalAxessWPNETT && // not net for Axess
                !isFdTrx() &&
                ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))
  {
    std::string msg = "FAIL DISPLAY TYPE-C FOR AXESS\n";
    doDiag335Collector(msg, false);
    ret = false;
  }
  return ret;
}

void
NegotiatedFareController::getGeneralFareRuleInfo(const PaxTypeFare& paxTypeFare,
                                                 GeneralFareRuleInfoVec& gfrInfoVec) const
{
  RuleUtil::getGeneralFareRuleInfo(_trx, paxTypeFare, RuleConst::NEGOTIATED_RULE, gfrInfoVec);
}

bool
NegotiatedFareController::checkGeneralFareRuleInfo(PaxTypeFare& paxTypeFare,
                                                   GeneralFareRuleInfoVec& gfrInfoVec)
{
  bool ret = true;

  if (gfrInfoVec.empty())
  {
    // Skip fare, if so
    LOG4CXX_DEBUG(logger, "NFC::process() - Skipping null GeneralFareRuleInfo pointer");

    doDiag335Collector(Diag335Collector::R2_NOT_FOUND, false);

    // For FD, invalidate fare with incomplete data status
    if (UNLIKELY(isFdTrx()))
    {
      paxTypeFare.invalidateFare(PaxTypeFare::FD_Cat35_Incomplete_Data);
    }
    ret = false;
  }

  return ret;
}

//-------------------------------------------------------------------
// processRule()
//-------------------------------------------------------------------
void
NegotiatedFareController::processRule(PaxTypeFare& ptFare, bool isLocationSwapped)
{
  // these store data for the cheapest fare from set(s)
  // only one fare is created for:
  //    - a contigous block of sets without a condition ('IF')
  //    - a set with a condition
  validatingCxr() = CarrierCode();
  _negPtfBucketContainer.reset(new NegPTFBucketContainer<NegPaxTypeFareDataComparator>());

  std::vector<CarrierCode> valCxrs = ptFare.validatingCarriers().empty()
                                         ? ptFare.fareMarket()->validatingCarriers()
                                         : ptFare.validatingCarriers();

  if (isFdTrx() || valCxrs.empty() || (valCxrs.size() == 1 && valCxrs[0] == validatingCxr()))
  {
    processRulePerCxr(ptFare, isLocationSwapped);
  }
  else
  {
    for (CarrierCode cxr : valCxrs)
    {
      validatingCxr() = cxr;
      processRulePerCxr(ptFare, isLocationSwapped);
    }
  }
  selectNegFares();

  if (UNLIKELY(isFdTrx() && _invalidAgency))
  {
    ptFare.invalidateFare(PaxTypeFare::FD_Cat35_Invalid_Agency);
  }
}

//-------------------------------------------------------------------
// processRule()
//-------------------------------------------------------------------
void
NegotiatedFareController::processRulePerCxr(PaxTypeFare& ptFare, bool isLocationSwapped)
{
  _invalidAgency = true;
  MoneyAmount amt, amtNuc;

  NegPaxTypeFareRuleData* ruleData;
  _ruleDataFareRetailer->sourcePseudoCity() = "";
  _ruleDataFareRetailer->fareRetailerRuleId() = 0;
  _ruleDataFareRetailer->cat25Responsive() = false;
  _ruleDataFareRetailer->frrSeqNo() = 0;
  _ruleDataFareRetailer->fareRetailerCode() = "";
  // init 'em
  resetMinAmt(amt, amtNuc, ruleData);

  const std::vector<CategoryRuleItemInfoSet*>& ruleItemInfoSets =
      _ruleInfo->categoryRuleItemInfoSet();

  LOG4CXX_DEBUG(logger, "NFC::processRule() - begin");

  _farePropPrintOptRetrieved = false;
  _fareProperties = nullptr;
  _valueCodeAlgorithm = nullptr;
  _printOption = nullptr;

  _nationFranceFare = false;
  _nationFranceLocalInd = false;
  _accCodeMatch = false;
  _accCodeValue.clear();

  _fareRetailerSuccessCount = 0;
  _markupSuccessCount = 0;

  bool origMatchedCorpID = ptFare.matchedCorpID(); // save the original matchedCorpID ind.

  uint32_t matchId = 0;
  _r2SubSetNum = 0;

  for (CategoryRuleItemInfoSet* ruleItemInfoSet : ruleItemInfoSets)
  {
    _r2SubSetNum++;
    _accCodeMUCMatch = false;
    _accCodeMUCValue.clear();

    ptFare.setMatchedCorpID(origMatchedCorpID); // restore the original MatchedCorpID ind.

    if (UNLIKELY(ruleItemInfoSet == nullptr))
    {
      LOG4CXX_DEBUG(logger, "NFC::processRule() - Skipping null CategoryRuleItemInfoSet pointer");
      continue;
    }

    std::vector<CategoryRuleItemInfo>* segQual = nullptr;
    _trx.dataHandle().get(segQual);
    // lint --e{413}

    std::vector<CategoryRuleItemInfo>::iterator categoryRuleItemInfoIter;
    categoryRuleItemInfoIter = (*ruleItemInfoSet).begin();

    std::vector<CategoryRuleItemInfo>::iterator categoryRuleItemInfoEnd;
    categoryRuleItemInfoEnd = (*ruleItemInfoSet).end();

    processRuleItemInfoSet(
        *ruleItemInfoSet, *segQual, categoryRuleItemInfoIter, categoryRuleItemInfoEnd);

    // Validate the if conditions (when needed)
    if (!segQual->empty())
    {
      // a set with 'IF' triggers making a fare from the previous set(s)
      if (createFare(ptFare, ruleData, isLocationSwapped, amt, amtNuc, true))
      {
        resetMinAmt(amt, amtNuc, ruleData);
      }

      bool rc = (isCat15Qualified(*segQual) && !valCarrier().empty())
                    ? validateQualifiers(
                          ptFare, *segQual, _ruleInfo->vendorCode(), valCarrier(), _ruleInfo, _dc)
                    : validateQualifiers(ptFare, *segQual, _ruleInfo->vendorCode(), _ruleInfo, _dc);

      if (UNLIKELY((_lastCreatedPtf) && (_trx.isFlexFare())))
      {
        std::vector<CategoryRuleItemInfo>::iterator i = segQual->begin();
        std::vector<CategoryRuleItemInfo>::iterator j = segQual->end();

        for (; i != j; ++i)
        {
          CategoryRuleItemInfo* catRuleItemInfo = &(*i);

          if ((catRuleItemInfo) && (catRuleItemInfo->itemcat() == RuleConst::ELIGIBILITY_RULE))
          {
            _lastCreatedPtf->getMutableFlexFaresValidationStatus()
                ->updateAttribute<flexFares::CORP_IDS>(
                    ptFare.getMutableFlexFaresValidationStatus());
            _lastCreatedPtf->getMutableFlexFaresValidationStatus()
                ->updateAttribute<flexFares::ACC_CODES>(
                    ptFare.getMutableFlexFaresValidationStatus());
            break;
          }
        }
      }
      if (!rc)

      {
        if (_dcDisplayRelation)
          diagDisplayRemainingRelation((*ruleItemInfoSet), categoryRuleItemInfoEnd);

        continue; // Skip this rule set it's invalid
      }
    }

    // Now loop through all the records before the if and build
    // negotiated fares
    categoryRuleItemInfoIter = (*ruleItemInfoSet).begin();

    for (; categoryRuleItemInfoIter != categoryRuleItemInfoEnd; categoryRuleItemInfoIter++)
    {
      _accCodeMUCMatch = false;
      _accCodeMUCValue.clear();

      _catRuleItemInfo = &(*categoryRuleItemInfoIter);

      if (UNLIKELY(!processCategoryRuleItemInfo(ptFare, isLocationSwapped)))
      {
        continue;
      }

      _negFareRest =
          _trx.dataHandle().getNegFareRest(_ruleInfo->vendorCode(), _catRuleItemInfo->itemNo());

      _negFareRestExt = nullptr;
      _negFareRestExtRetrieved = false;
      _negFareRestExtSeq.clear();

      if (!processNegFareRest(ptFare))
      {
        continue;
      }

      if (_frrlVecTypeC.size() && (ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))
        _fareRetailerRuleValidator.validateFRR(
          ptFare, &_fareMarket, 'N', _frrlVecTypeC, _frrcVecTypeC);

      if (_frrlVecTypeLT.size() && ((ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE) ||
                                    (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE)))
        _fareRetailerRuleValidator.validateFRR(
          ptFare, &_fareMarket, 'R', _frrlVecTypeLT, _frrcVecTypeLT);

      if (!fallback::fallbackFRRCat25Responsive(&_trx))
      {
        if (_frrlVecTypeCFbr.size() &&
            (ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD) &&
             isPtfValidForCat25Responsive(ptFare))
        {
          _fareRetailerRuleValidator.validateFRR(
            ptFare, &_fareMarket, 'D', _frrlVecTypeCFbr, _frrcVecTypeCFbr);
        }
      }

      _ruleItemInfoSet = ruleItemInfoSet;
      _segQual = segQual;
      if (!getValidCalc(ptFare, ruleData, amt, amtNuc))
      {
        LOG4CXX_DEBUG(logger, "NFC::processRule() - no positive match for security/calc");
        continue;
      }

      if (_isNewMinAmt)
      {
        ruleData->categoryRuleItemInfoSet() = ruleItemInfoSet;
        ruleData->categoryRuleItemInfo() = _catRuleItemInfo;
        ruleData->categoryRuleItemInfoVec() = segQual;
        ruleData->ruleItemInfo() = _negFareRest;

        if (LIKELY(!isFdTrx() && !_trx.getOptions()->isCat35Net()))
        {
          ptFare.fareDisplayCat35Type() = _fareDisplayMinCat35Type;
        }

        _isNewMinAmt = false;
      }

      // always make a fare from a string with Directionality 3/4 or InOutInd is not blank
      // always make a fare from base fare with empty or ADT in R1B
      bool dirApplied = checkDirectionalityAndPaxType(ptFare.fcasPaxType());
      if ((matchId == 0 && !_isHierarchyApplied) || dirApplied)
      {
        if (createFare(ptFare, ruleData, isLocationSwapped, amt, amtNuc, true))
        {
          ++matchId;
          resetMinAmt(amt, amtNuc, ruleData);
        }
      }
    } // endfor - rule item

    // always make a fare from a set with a condition
    if (!segQual->empty())
    {
      if (createFare(ptFare, ruleData, isLocationSwapped, amt, amtNuc, true))
      {
        resetMinAmt(amt, amtNuc, ruleData);
      }
    }
  } // endfor - rule set

  // last fare (without segQual) hasn't been created yet, so do it
  if (createFare(ptFare, ruleData, isLocationSwapped, amt, amtNuc, true))
  {
    resetMinAmt(amt, amtNuc, ruleData);
  }

  ptFare.setMatchedCorpID(origMatchedCorpID); // restore the original MatchedCorpID ind.
}

bool
NegotiatedFareController::processCategoryRuleItemInfo(const PaxTypeFare& ptFare,
                                                      bool isLocationSwapped) const
{
  bool ret = true;

  // Check if is NULL
  if (UNLIKELY(_catRuleItemInfo == nullptr))
  {
    LOG4CXX_DEBUG(logger, "NFC::processRule() - Skipping null CategoryRuleItemInfo pointer");
    ret = false;
  }
  // We want to process the then's, or's and else's
  else if (UNLIKELY((_catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::IF) ||
                     (_catRuleItemInfo->relationalInd() == CategoryRuleItemInfo::AND)))
  {
    LOG4CXX_DEBUG(logger,
                  "NFC::processRule() - Skipping unapplicable CategoryRuleItemInfo pointer");
    ret = false;
  }
  else
  {
    *_dc << *_catRuleItemInfo;

    // directioanlity
    Record3ReturnTypes directionCode;

    // Check for Fare Display transaction
    if (UNLIKELY(isFdTrx()))
    {
      directionCode =
          CategoryRuleItem::isDirectionPassForFD(ptFare, _catRuleItemInfo, isLocationSwapped);
    }
    else
    {
      directionCode =
          CategoryRuleItem::isDirectionPass(_trx, ptFare, _catRuleItemInfo, isLocationSwapped);
    }

    if (UNLIKELY(directionCode == FAIL))
    {
      std::string msg = "FAIL DIRECTIONALITY\n";
      doDiag335Collector(msg, false);
      ret = false;
    }
  }

  return ret;
}

bool
NegotiatedFareController::processNegFareRest(PaxTypeFare& ptFare)
{
  bool ret = true;
  // Check if is NULL
  if (UNLIKELY(_negFareRest == nullptr))
  {
    LOG4CXX_DEBUG(logger, "NFC::processRule() - Skipping null negFareRest pointer");
    ret = false;
  }
  // Get the first one and then call validate
  else if (validate(ptFare) != PASS)
  {
    LOG4CXX_DEBUG(logger, "NFC::processRule() - validate() returned false");
    ret = false;
  }
  // When no Table 979 link, DIS CAT TYPE must be value L or C
  else if (_negFareRest->negFareCalcTblItemNo() == 0 &&
           ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE)
  {
    std::string msg = "T979:DOES NOT EXIST - DCT MUST BE L OR C\n";
    doDiag335Collector(msg, false);
    ret = false;
  }
  // For WPNETT entry, fail fare type L with no Table 979
  else if (UNLIKELY(getAgent() && getAgent()->axessUser() && // only for Axess
                     _trx.getRequest()->isWpNettRequested() &&
                     _negFareRest->negFareCalcTblItemNo() == 0 &&
                     ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE))
  {
    std::string msg = "FAIL DISPLAY TYPE-L WITH NO TABLE 979 FOR WPNETT\n";
    doDiag335Collector(msg, false);
    ret = false;
  }
  else if (UNLIKELY(_negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_2 &&
                     (TrxUtil::isRexOrCsoTrx(_trx) ||
                      (_trx.getOptions() && _trx.getOptions()->isNetRemitFareRestricted()))))
  {
    std::string msg = "METHOD TYPE 2 FARE NOT VALID FOR EXCHANGE\n";
    doDiag335Collector(msg, false);
    ret = false;
  }

  return ret;
}

bool
NegotiatedFareController::checkDirectionalityAndPaxType(const PaxTypeCode& paxType) const
{
  bool res =
      // ckeck directionality
      _catRuleItemInfo->directionality() == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2 ||
      _catRuleItemInfo->directionality() == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1 ||
      _catRuleItemInfo->inOutInd() != RuleConst::ALWAYS_APPLIES ||
      // ckeck Pax type
      paxType.empty() || paxType == ADULT;

  _rpData.isDir3 = _catRuleItemInfo->directionality() == RuleConst::ORIGIN_FROM_LOC1_TO_LOC2;
  _rpData.isDir4 = _catRuleItemInfo->directionality() == RuleConst::ORIGIN_FROM_LOC2_TO_LOC1;
  _rpData.inOutInd = _catRuleItemInfo->inOutInd();
  _rpData.isDirectionalityApplied = (_rpData.isDir3 || _rpData.isDir4 ||
                                     _catRuleItemInfo->inOutInd() != RuleConst::ALWAYS_APPLIES);

  return res;
}

//-------------------------------------------------------------------
// <PRE>
//
// @function  NegotiatedFareController::getValidCalc
//
// Description:  This method scans all relavent security/calc records and
//               returns the one that passes.  Also fills Calc object
//               with Markup info (if needed).  Also does restribution.
//
// @param  vend     Rule vendor code
// @param  itemSec  Table 983 item number
// @param  itemCalc Table 979 item number
// @param  ptFare   Base published or Cat 35 fare with DIS CAT TYPE L/C/T
// @param  rec2SeqNum  Sequence number from Record 2 (used for redist)
// @param  amt      Minimum fare found
// @param  view     viewNetInd associated with fare found (only applies if 980 used)
// @param  curUsed  Currency used
//
// @return true - found security record & other needed data (calc, 980, ...)
//
// </PRE>
//-------------------------------------------------------------------
bool
NegotiatedFareController::getValidCalc(PaxTypeFare& ptFare,
                                       NegPaxTypeFareRuleData* ruleData,
                                       MoneyAmount& amt,
                                       MoneyAmount& amtNuc)
{
  int32_t seqNegMatch = 0;
  bool isPosMatch = false;
  bool is979queried = false;
  NegFareCalc calcObj;
  Money base(ptFare.fare()->originalFareAmount(), ptFare.currency());
  _isFDFareCreatedForFareRetailer = false;
  _isFDCat25Responsive = false;

  // process 983 Security Positive match for regular/markup calculation info from 979/980
  if (!processSecurityMatch(
          isPosMatch, is979queried, amt, amtNuc, base, ruleData, ptFare, seqNegMatch, calcObj))
  {
    return false;
  }
  // Acess Type C WPNETT: skip redistributions
  if (LIKELY(!isJalAxessTypeC(ptFare)))
  {
    // process redistributions from 980
    processRedistributions(ptFare, ruleData, amt, amtNuc, base, calcObj, is979queried, seqNegMatch);
  }

  return true;
}

//-------------------------------------------------------------------
// <PRE>
//
// @function  NegotiatedFareController::get979
//
// Description:  This method scans all relavent fare calculation information in Table 979
//               and returns the sequence that passes the match data (Directional Indicator,
//               LOC1 and LOC2).
//
//               Once a sequence has been matched, do not continue processing to any other
//               Table 979 sequences.
//
//               If the fare does not match any sequences in Table 979,
//                   If DIS CAT TYPE is 'T'        - fail Record 3 Cat 35
//                   If DIS CAT TYPE is 'L' or 'C' - continue processing Cat 35
//
//               If Table 979 data does not exist, DIS CAT TYPE must be L or C.
//               This validation was performed up-front.
//
// @param  ptFare   Base published or Cat 25 fare with DIS CAT TYPE L/C/T
// @param  isUpd    fare security 983 UPDATE indicator
// @param  calcObj  fare calculation 979 object
//
// @return true - found fare calculation 979 record
//
// </PRE>
//-------------------------------------------------------------------
bool
NegotiatedFareController::get979(PaxTypeFare& ptFare, bool isUpd, NegFareCalc& calcObj, bool isCat25Responsive)
{
  bool ret = true;
  bool haveCalc = false;
  Indicator fareCalcIndicator;

  // caller ensures ==0 is OK scenario
  if (_negFareRest->negFareCalcTblItemNo() != 0)
  {
    std::vector<NegFareCalcInfo*>::const_iterator calcRec;
    const std::vector<NegFareCalcInfo*>& calcList = getNegFareCalc();

    for (calcRec = calcList.begin(); calcRec != calcList.end(); calcRec++)
    {
      if (*calcRec == nullptr) // invalid input
      {
        LOG4CXX_INFO(logger, "NFC::get979() - blank fare calc record");
        return false; // no sense to continue
      }
      if (!isCat25Responsive)
        calcObj.load(*calcRec);

      if (calcObj.isMatchFareLoc(_trx, ptFare))
      {
        haveCalc = true; // found element
        fareCalcIndicator = (*calcRec)->fareInd();
        *_dc << **calcRec;
        break;
      }
    } // endfor - 979 recs

    if ((!haveCalc && ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE) ||
        !calcObj.isValidCat(ptFare.fcaDisplayCatType(), isUpd))
    {
      ret = false;
    }

    if (isCat25Responsive && haveCalc && ret)
    {
      if (fareCalcIndicator == RuleConst::NF_RANGE_PERCENT   || // P
          fareCalcIndicator == RuleConst::NF_RANGE_SPECIFIED || // R
          fareCalcIndicator == RuleConst::NF_ADD_RANGE       || // N
          fareCalcIndicator == RuleConst::NF_MINUS_RANGE)       // T
      {
        ret = false;
      }
    }

  }
  else if (isCat25Responsive)
  {
    ret = false;
  }

  return ret;
}

bool
NegotiatedFareController::processFrrv(const PaxTypeFare& ptFare)
{
  if ((ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD) && !_frrcVecTypeC.empty())
    return true;

  if (((ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE) ||
       (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE)) &&
      !_frrcVecTypeLT.empty())
    return true;

  if (!fallback::fallbackFRRCat25Responsive(&_trx))
  {
      if ((ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD) &&
          !_frrcVecTypeCFbr.empty())
        return true;
  }

  return false;
}

//-------------------------------------------------------------------
// <PRE>
//
// @function  NegotiatedFareController::getOneCalc
//
// Description:  This method get one matching fare calc record (979)
//               with markup info.
//
// </PRE>
//-------------------------------------------------------------------
bool
NegotiatedFareController::getOneCalc(PaxTypeFare& ptFare,
                                     bool isUpd,
                                     long secondarySellerId,
                                     NegFareCalc& calcObj,
                                     NegPaxTypeFareRuleData* ruleData,
                                     Money& base,
                                     MoneyAmount& amt,
                                     MoneyAmount& amtNuc,
                                     NegFareSecurityInfo* secRec)
{
  if (UNLIKELY(!get979(ptFare, isUpd, calcObj)))
  {
    std::string msg = "979 RECORD NOT FOUND\n";
    doDiag335Collector(msg, false);
    return false;
  }

  bool rc = false;

  if (processFrrv(ptFare))
  {
    if (_fareRetailerSuccessCount > 0 ||
        (_fareRetailerSuccessCount == 0 && _markupSuccessCount == 0))
    {
      rc = processFareRetailerRuleValidation(
          ptFare, ruleData, amt, amtNuc, base, calcObj, false, true, secRec);
      if (rc)
      {
        _fareRetailerSuccessCount++;
        return true;
      }
      return false;
    }
  }
  if (!isUpd) // markup from 980
    return true;

  if (_markupSuccessCount > 0 || (_fareRetailerSuccessCount == 0 && _markupSuccessCount == 0))
  {
    rc = processMarkupForUpdate(ptFare, isUpd, secondarySellerId, calcObj, true);
    if (rc == true)
    {
      _markupSuccessCount++;
      return true;
    }
    else
    {
      if (getAgent()->tvlAgencyPCC() != getAgent()->mainTvlAgencyPCC() &&
          processMarkupForUpdate(ptFare, isUpd, secondarySellerId, calcObj, false))
      {
        _markupSuccessCount++;
        return true;
      }
    }
  }

  if (_markupSuccessCount > 0)
  {
    std::string msg = "MATCHING MARKUP RECORD NOT FOUND\n";
    doDiag335Collector(msg, false);
  }
  return false;
}

bool
NegotiatedFareController::processMarkupForUpdate(
    PaxTypeFare& ptFare, bool isUpd, long secondarySellerId, NegFareCalc& calcObj, bool isTvlPcc)
{
  bool ret = false;
  std::vector<MarkupCalculate*>::const_iterator muCalcRec;
  std::vector<MarkupControl*>::const_iterator markupRec;
  const std::vector<MarkupControl*>& markupCtrl = getMarkupByPcc(ptFare, isTvlPcc);
  bool ticketOrExchange = TrxUtil::isExchangeOrTicketing(_trx);

  for (markupRec = markupCtrl.begin(); markupRec != markupCtrl.end(); markupRec++)
  {
    if (!matchMarkupRec(**markupRec, 'U', isTvlPcc, secondarySellerId))
      continue;

    if (!checkMarkupControl(**markupRec, ticketOrExchange) || !matchedCorpIDInPricing(ptFare))
    {
      continue;
    }

    for (muCalcRec = (*markupRec)->calcs().begin(); muCalcRec != (*markupRec)->calcs().end();
         muCalcRec++)
    {
      calcObj.load(_ruleInfo->vendorCode(), *muCalcRec, (*markupRec)->viewNetInd(), "");
      if (calcObj.isMatchFareLoc(_trx, ptFare) && calcObj.isMatchMarkupCalc(*muCalcRec, ptFare))
      {
        if ((*muCalcRec)->netSellingInd() != S_TYPE)
        {
          std::string msg = "MARKUP HAS INVALID SELLING IND\n";
          doDiag335Collector(msg, false);
        }
        else // found correct one
        {
          std::ostringstream oss;
          oss << "MARKUP ";
          doDiag335Collector(oss.str(), false);
          *_dc << **muCalcRec;
          ret = true;
        }
        return ret;
      }
    } // endfor - calc recs
  } // endfor - markup recs

  return ret;
}

bool
NegotiatedFareController::isFareRetailerRuleForUpdate(const FareRetailerRuleInfo* frri)
{
  return (frri->sourcePseudoCity() == frri->ownerPseudoCity() &&
          frri->sourcePseudoCity() == getAgent()->tvlAgencyPCC());
}

bool
NegotiatedFareController::isContextValid(const FareRetailerRuleContext& context)
{
  if (context._t983 && context._frrfai)
    return true;

  if (_dc->isActive())
  {
    Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);
    std::ostringstream oss;
    oss << "  FRR FAIL PERMISSIONS    PCC: " << context._sourcePcc
        << " ID: " << (context._frri)->fareRetailerRuleId() << "\n"
        << " BECAUSE OF MISSING DATA\n";

    if (context._frrfai == nullptr)
      oss << " FRR RFA ITEM NO "
          << (context._frri)->resultingFareAttrItemNo() << " NOT FOUND\n";

    if (context._t983 == nullptr)
      oss << " FRR SECURITY ITEM NO "
          << (context._frri)->securityItemNo() << " NOT FOUND\n";

    dc335->displayMessage(oss.str(), false);
  }

  return false;
}

bool
NegotiatedFareController::processFareRetailerRuleValidation(PaxTypeFare& ptFare,
                                                            NegPaxTypeFareRuleData* ruleData,
                                                            MoneyAmount& amt,
                                                            MoneyAmount& amtNuc,
                                                            Money& base,
                                                            NegFareCalc& calcObj,
                                                            bool is979queried,
                                                            bool isForUpdate,
                                                            NegFareSecurityInfo* secRec)
{
  bool ret = false;
  bool displayPassedOnly = _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASS";

  MoneyAmount newAmt, newAmtNuc;
  // TODO_FR 2: since we are not checking secondary seller ID against the PCCs in table 983,
  // there's no way to set _invalidAgency to false here.

  if (ptFare.fcaDisplayCatType() != RuleConst::NET_SUBMIT_FARE_UPD)
  {
    // process for types "L" and "T"
    return processFareRetailerRuleLandT(ptFare, ruleData, amt, amtNuc, base, calcObj, secRec);
  }

  if (!fallback::fallbackFRRCat25Responsive(&_trx))
  {
    // process for type C cat25 responsive
    if (processFareRetailerRuleTypeCFbrResponsive(ptFare, ruleData, amt, amtNuc, base, calcObj,
                                                  is979queried, isForUpdate, secRec))
      return true;
  }

  get979(ptFare, false, calcObj);

  const std::vector<NegFareSecurityInfo*>& secList = getNegFareSecurity();
  std::vector<FareRetailerRuleContext>& frrcV = _frrcVecTypeC;

  for (FareRetailerRuleContext& context : frrcV)
  {
    if ((context._sourcePcc != getAgent()->tvlAgencyPCC() &&
         context._sourcePcc != getAgent()->mainTvlAgencyPCC()) ||
        !secRec)
    {
      Agent* sourcePcc = createAgentSourcePcc(context._sourcePcc);
      if (!processSourcePccSecurityMatch(secList, sourcePcc, context))
        continue;
    }
    else
    {
      context._t983 = secRec;
    }

    if (!isContextValid(context))
      continue;

    if (!processPermissionIndicators(context, secRec, ruleData, ptFare.fcaDisplayCatType()))
    {
      if (_dc->isActive())
      {
        Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);
        std::ostringstream oss;
        oss << "  FRR FAIL PERMISSIONS    PCC: " << context._sourcePcc
            << " ID: " << (context._frri)->fareRetailerRuleId() << "\n";
        oss << "    T983 URST: " << context._t983->updateInd() << context._t983->redistributeInd()
            << context._t983->sellInd() << context._t983->ticketInd()
            << "    FRR URST: " << (context._frrfai)->updateInd()
            << (context._frrfai)->redistributeInd() << (context._frrfai)->sellInd()
            << (context._frrfai)->ticketInd() << "\n";
        dc335->displayMessage(oss.str(), false);
      }
      continue;
    }

    // redistribution check
    if (!isFareRetailerRuleForUpdate(context._frri))
    {
      calcObj.clearWholeSale();

      for (FareRetailerCalcInfo* calcInfo : context._frciV)
      {
        for (const FareRetailerCalcDetailInfo* calcDetail : calcInfo->details())
        {
          if (calcDetail->calculationTypeCd() == "WS")
          {
            calcObj.loadFRWholeSale(*calcDetail);
            break;
          }
        }
      }
    }

    for (FareRetailerCalcInfo* calcInfo : context._frciV)
    {
      for (const FareRetailerCalcDetailInfo* calcDetail : calcInfo->details())
      {
        if (calcDetail->calculationTypeCd() == "SL")
        {
          calcObj.loadFR(*calcDetail,
                         *(context._frri),
                         context._frrfai->viewNetInd(),
                         context._frrfai->sellInd(),
                         (isFareRetailerRuleForUpdate(context._frri))
                             ? "" : context._frri->sourcePseudoCity());
          calcObj.getPrice(_trx, ptFare, *this, newAmt, newAmtNuc);

          prepareNewSellingAmt(ptFare, newAmt);
          bool rc = false;
          rc = keepMinAmt(amt,
                          newAmt,
                          amtNuc,
                          newAmtNuc,
                          ruleData,
                          calcObj,
                          ptFare,
                          ruleData->ticketInd(),
                          *_negFareRest,
                          ptFare.fareDisplayCat35Type(),
                          true);

          if (_dc->isActive() && (_dc->diagnosticType() == Diagnostic335))
          {
            if (!displayPassedOnly || rc)
            {
              Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);

              std::ostringstream oss;
              oss << " FARE RETAIL "
                  << ((isFareRetailerRuleForUpdate(context._frri)) ? "UPDATE" : "REDIST") << "\n";
              oss << "  IS MIN AMT: " << (rc ? "Y" : "N") << "\n";
              oss << "  PCC: " << context._sourcePcc
                  << "\n"
                     "  ID : " << (context._frri)->fareRetailerRuleId()
                  << "\n"
                     "  SEQ: " << (context._frri)->ruleSeqNo() << "\n"
                                                                  "  AMT: " << std::setw(8)
                  << Money(newAmt, ptFare.currency()) << "\n";
              oss << "  FRR RESULT PERMISSIONS  URST: " << ruleData->updateInd()
                  << ruleData->redistributeInd() << ruleData->sellInd() << ruleData->ticketInd()
                  << "\n";
              dc335->displayMessage(oss.str(), false); // false means append but don't display yet
            }
          }

          if (rc)
          {
            keepFareRetailerMinAmtInfo(ruleData,
                                       context._sourcePcc,
                                       (context._frri)->fareRetailerRuleId(),
                                       (context._frri)->ruleSeqNo(),
                                       (context._frri)->fareRetailerCode());

            getRedistWholeSaleCalcPrice(calcObj, ptFare, ruleData, base);
            ret = true;
          }

          if (isFdTrx() && !isFareRetailerRuleForUpdate(context._frri)) // For redistribution only
          {
            NegFareSecurityInfo* dummySecRec;
            _trx.dataHandle().get(dummySecRec);
            dummySecRec->updateInd() = ruleData->updateInd();
            dummySecRec->redistributeInd() = ruleData->redistributeInd();
            dummySecRec->sellInd() = ruleData->sellInd();
            dummySecRec->ticketInd() = ruleData->ticketInd();

            // dummySecRec->localeType() = (context._frri)->ownerPseudoCity();
            // TODO_FR 4: (context._frri)->ownerPseudoCity() is not of type Indicator

            createFaresForFD(ptFare,
                             newAmt,
                             newAmtNuc,
                             base,
                             calcObj,
                             *dummySecRec,
                             context._frrfai->ticketInd());

            _isFDFareCreatedForFareRetailer = true;
          }
          else if (!_trx.getOptions()->isCat35Net() &&
                   _fareDisplayMinCat35Type == RuleConst::NET_FARE)
          {
            _fareDisplayMinCat35Type = RuleConst::SELLING_MARKUP_FARE;
          }
          break; // stop at the first match within the same PCC
        }
      }
    }
  }

  return ret;
}

bool
NegotiatedFareController::processFareRetailerRuleLandT(PaxTypeFare& ptFare,
                                                       NegPaxTypeFareRuleData* ruleData,
                                                       MoneyAmount& amt,
                                                       MoneyAmount& amtNuc,
                                                       Money& base,
                                                       NegFareCalc& calcObj,
                                                       NegFareSecurityInfo* secRec)
{
  if (fallback::fallbackProcessTypeLAndT(&_trx))
    return true;

  MoneyAmount newAmt, newAmtNuc;
  bool displayPassedOnly = _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASS";
  bool table979Exists = get979(ptFare, false, calcObj);

  if (ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE) // type T
  {
    if (!table979Exists || (_negFareRest->negFareCalcTblItemNo() == 0))
      return false;
  }

  if (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE) // type L
  {
    if ((_negFareRest->negFareCalcTblItemNo() != 0) && !table979Exists)
      return false;
  }

  const std::vector<NegFareSecurityInfo*>& secList = getNegFareSecurity();
  std::vector<FareRetailerRuleContext>& frrcV = _frrcVecTypeLT;

  for (FareRetailerRuleContext& context : frrcV)
  {
    if ((context._sourcePcc != getAgent()->tvlAgencyPCC() &&
         context._sourcePcc != getAgent()->mainTvlAgencyPCC()) ||
        !secRec)
    {
      Agent* sourcePcc = createAgentSourcePcc(context._sourcePcc);
      if (!processSourcePccSecurityMatch(secList, sourcePcc, context))
        continue;
    }
    else
    {
      context._t983 = secRec;
    }

    if (!isContextValid(context))
      continue;

    if (!processPermissionIndicators(context, secRec, ruleData, ptFare.fcaDisplayCatType()))
    {
      if (_dc->isActive())
      {
        Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);
        std::ostringstream oss;
        oss << "  FRR FAIL PERMISSIONS    PCC: " << context._sourcePcc
            << " ID: " << (context._frri)->fareRetailerRuleId() << "\n";
        oss << "    T983 URST: " << context._t983->updateInd() << context._t983->redistributeInd()
            << context._t983->sellInd() << context._t983->ticketInd()
            << "    FRR URST: " << (context._frrfai)->updateInd()
            << (context._frrfai)->redistributeInd() << (context._frrfai)->sellInd()
            << (context._frrfai)->ticketInd() << "\n";
        dc335->displayMessage(oss.str(), false);
      }
      continue;
    }

    bool typeLNoTable979 = false;

    if ((ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE) && // type L
        (!table979Exists || (_negFareRest->negFareCalcTblItemNo() == 0)))
    {
      typeLNoTable979 = true;
    }

    if (!typeLNoTable979)
      calcObj.getPrice(_trx, ptFare, *this, newAmt, newAmtNuc);

    prepareNewSellingAmt(ptFare, newAmt);
    bool rc = keepMinAmt(amt,
                         newAmt,
                         amtNuc,
                         newAmtNuc,
                         ruleData,
                         calcObj,
                         ptFare,
                         ruleData->ticketInd(),
                         *_negFareRest,
                         ptFare.fareDisplayCat35Type(),
                         true);

    if (_dc->isActive() && (_dc->diagnosticType() == Diagnostic335))
    {
      if (!displayPassedOnly || rc)
      {
        Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);

        std::ostringstream oss;
        oss << " FARE RETAIL L/T "
            << ((isFareRetailerRuleForUpdate(context._frri)) ? "UPDATE" : "REDIST") << "\n";
        oss << "  IS MIN AMT: " << (rc ? "Y" : "N") << "\n";
        oss << "  PCC: " << context._sourcePcc << "\n"
                                                  "  ID : " << (context._frri)->fareRetailerRuleId()
            << "\n"
               "  SEQ: " << (context._frri)->ruleSeqNo() << "\n"
                                                            "  AMT: " << std::setw(8)
            << Money(newAmt, ptFare.currency()) << "\n";
        oss << "  FRR RESULT PERMISSIONS  URST: " << ruleData->updateInd()
            << ruleData->redistributeInd() << ruleData->sellInd() << ruleData->ticketInd() << "\n";
        dc335->displayMessage(oss.str(), false); // false means append but don't display yet
      }
    }

    if (rc)
    {
      keepFareRetailerMinAmtInfo(ruleData,
                                 context._sourcePcc,
                                 (context._frri)->fareRetailerRuleId(),
                                 (context._frri)->ruleSeqNo(),
                                 (context._frri)->fareRetailerCode());
    }

    if (isFdTrx()) //  isFareRetailerRuleForUpdate(context._frri) ???
    {
      NegFareSecurityInfo* dummySecRec;
      _trx.dataHandle().get(dummySecRec);
      dummySecRec->updateInd() = ruleData->updateInd();
      dummySecRec->redistributeInd() = ruleData->redistributeInd();
      dummySecRec->sellInd() = ruleData->sellInd();
      dummySecRec->ticketInd() = ruleData->ticketInd();

      createFaresForFD(
          ptFare, newAmt, newAmtNuc, base, calcObj, *dummySecRec, context._frrfai->ticketInd());
    }
  }
  return true;
}

bool
NegotiatedFareController::processFareRetailerRuleTypeCFbrResponsive(PaxTypeFare& ptFare,
                                                                    NegPaxTypeFareRuleData* ruleData,
                                                                    MoneyAmount& amt,
                                                                    MoneyAmount& amtNuc,
                                                                    Money& base,
                                                                    NegFareCalc& calcObjReserved,
                                                                    bool is979queried,
                                                                    bool isForUpdate,
                                                                    NegFareSecurityInfo* secRec)
{
  // calcObjReserved is reserved for future use
  bool ret = false;
  bool displayPassedOnly = _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASS";
  NegFareCalc calcObj;

  MoneyAmount newAmt, newAmtNuc;

  // Req 5: The CAT35 data within the CAT25 rule will either have no Fare Creator Table 979,
  // or will have a Fare Creator Table which contains data using the following fare
  // calculation indicators: P,R,N,T
  if (get979(ptFare, false, calcObj, true))
  {
    // It has a table 979 and the fare cal indicators are either
    // A, C, M, S, or Z. So, do not process further and return
    return false;
  }

  // Req 6: The "URST" permissions in the Security Table 983
  // must be filed with "Update" permitted (value Y)
  if (secRec && secRec->updateInd() != YES)
  {
    if (_dc->isActive())
    {
        Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);
        std::ostringstream oss;
        oss << " FRR FAIL PERMISSIONS C25 MAIN PCC: " << getAgent()->tvlAgencyPCC()
            << "\n";
        oss << "    T983 URST: " << secRec->updateInd() <<"\n";
        dc335->displayMessage(oss.str(), false);
    }
    return false;
  }

  const std::vector<NegFareSecurityInfo*>& secList = getNegFareSecurity();
  std::vector<FareRetailerRuleContext>& frrcV = _frrcVecTypeCFbr;

  const FBRPaxTypeFareRuleData* fbr = ptFare.getFbrRuleData(RuleConst::FARE_BY_RULE);

  if (UNLIKELY(fbr == nullptr))
    return false;  // this is not c25

  PaxTypeFare* baseFare = fbr->baseFare();
  if (UNLIKELY(baseFare == nullptr))
    return false;

  for (FareRetailerRuleContext& context : frrcV)
  {
    if ((context._sourcePcc != getAgent()->tvlAgencyPCC() &&
         context._sourcePcc != getAgent()->mainTvlAgencyPCC()) ||
        !secRec)
    {
      Agent* sourcePcc = createAgentSourcePcc(context._sourcePcc);
      if (!processSourcePccSecurityMatch(secList, sourcePcc, context))
        continue;
    }
    else
    {
      context._t983 = secRec;
    }

    if (!processPermissionIndicators(context, secRec, ruleData, ptFare.fcaDisplayCatType()) ||
        (context._t983 && context._t983->updateInd() != 'Y'))
    {
      if (_dc->isActive())
      {
        Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);
        std::ostringstream oss;
        oss << "  FRR FAIL PERMISSIONS C25  PCC: " << context._sourcePcc
            << " ID: " << (context._frri)->fareRetailerRuleId() << "\n";
        oss << "    T983 URST: "
            << ruleData->updateInd()
            << ruleData->redistributeInd()
            << ruleData->sellInd()
            << ruleData->ticketInd()
            <<"\n";
        dc335->displayMessage(oss.str(), false);
      }
      continue;
    }

    for (FareRetailerCalcInfo* calcInfo : context._frciV)
    {
      for (const FareRetailerCalcDetailInfo* calcDetail : calcInfo->details())
      {
        if (calcDetail->calculationTypeCd() == "SL")
        {
          calcObj.loadFR(*calcDetail,
                         *(context._frri),
                         context._frrfai->viewNetInd(),
                         context._frrfai->sellInd(),
                         (isFareRetailerRuleForUpdate(context._frri))
                             ? "" : context._frri->sourcePseudoCity());
          calcObj.getPrice(_trx, *baseFare, *this, newAmt, newAmtNuc);

          if (ptFare.isDiscounted())
          {
            // no calcObj.loadFR() needed here
            calcObj.getPriceFRdiscounted(_trx,
                                         *baseFare,
                                         *this,
                                         newAmt,
                                         newAmtNuc,
                                         ptFare.isRoundTrip(),
                                         ptFare.discountInfo());
          }

          if ((newAmtNuc - ptFare.nucFareAmount() < EPSILON) ||
              (newAmt - ptFare.fareAmount() < EPSILON))
          {
            if (_dc->isActive())
            {
              Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);
              std::ostringstream oss;
              oss << "  FRR FAIL CALCULATION C25  PCC: " << context._sourcePcc
                  << " ID: " << (context._frri)->fareRetailerRuleId() << "\n";
              oss << "    NEW AMT : " << newAmt << "\n";
              oss << "    NEW NUC : " << newAmtNuc << "\n";
              oss << "    PTF AMT : " << ptFare.fareAmount() << "\n";
              oss << "    PTF NUC : " << ptFare.nucFareAmount() << "\n";
              dc335->displayMessage(oss.str(), false);
            }
            if (!fallback::fallbackFRRCat25AmtCheckFix(&_trx))
              continue;
          }

          prepareNewSellingAmt(ptFare, newAmt);
          bool rc = false;
          rc = keepMinAmt(amt,
                          newAmt,
                          amtNuc,
                          newAmtNuc,
                          ruleData,
                          calcObj,
                          ptFare,
                          ruleData->ticketInd(),
                          *_negFareRest,
                          ptFare.fareDisplayCat35Type(),
                          true);

          if (_dc->isActive() && (_dc->diagnosticType() == Diagnostic335))
          {
            if (!displayPassedOnly || rc)
            {
              Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);

              std::ostringstream oss;
              oss << " FARE RETAIL "
                  << ((isFareRetailerRuleForUpdate(context._frri)) ? "UPDATE" : "REDIST") << "\n";
              oss << "  IS MIN AMT: " << (rc ? "Y" : "N") << "\n";
              oss << "  PCC: " << context._sourcePcc
                  << "\n"
                     "  ID : " << (context._frri)->fareRetailerRuleId()
                  << "\n"
                     "  SEQ: " << (context._frri)->ruleSeqNo() << "\n"
                                                                  "  AMT: " << std::setw(8)
                  << Money(newAmt, ptFare.currency()) << "\n";
              oss << "  FRR RESULT PERMISSIONS  URST: " << ruleData->updateInd()
                  << ruleData->redistributeInd() << ruleData->sellInd() << ruleData->ticketInd()
                  << "\n";
              dc335->displayMessage(oss.str(), false); // false means append but don't display yet
            }
          }

          if (rc)
          {
            _isFDCat25Responsive = true;
            keepFareRetailerMinAmtInfo(ruleData,
                                       context._sourcePcc,
                                       (context._frri)->fareRetailerRuleId(),
                                       (context._frri)->ruleSeqNo(),
                                       (context._frri)->fareRetailerCode(),
                                       true);

            // getRedistWholeSaleCalcPrice(calcObj, ptFare, ruleData, base);
            ret = true;
          }

          // if (isFdTrx() && !isFareRetailerRuleForUpdate(context._frri)) // For redistribution only
          if (isFdTrx())
          {
            NegFareSecurityInfo* dummySecRec;
            _trx.dataHandle().get(dummySecRec);
            dummySecRec->updateInd() = ruleData->updateInd();
            dummySecRec->redistributeInd() = ruleData->redistributeInd();
            dummySecRec->sellInd() = ruleData->sellInd();
            dummySecRec->ticketInd() = ruleData->ticketInd();

            // dummySecRec->localeType() = (context._frri)->ownerPseudoCity();
            // TODO_FR 4: (context._frri)->ownerPseudoCity() is not of type Indicator

            createFaresForFD(ptFare,
                             newAmt,
                             newAmtNuc,
                             base,
                             calcObj,
                             *dummySecRec,
                             context._frrfai->ticketInd());

            _isFDFareCreatedForFareRetailer = true;
          }
          else if (!_trx.getOptions()->isCat35Net() &&
                   _fareDisplayMinCat35Type == RuleConst::NET_FARE)
          {
            _fareDisplayMinCat35Type = RuleConst::SELLING_MARKUP_FARE;
          }
          break; // stop at the first match within the same PCC
        }
      }
    }
  }

  return ret;
}

const std::vector<MarkupControl*>&
NegotiatedFareController::getMarkupBySecondSellerId(const PaxTypeFare& ptFare,
                                                    uint32_t rec2SeqNum,
                                                    long secondarySellerId) const
{
  return _trx.dataHandle().getMarkupBySecondSellerId(getAgent()->tvlAgencyPCC(),
                                                     getAgent()->mainTvlAgencyPCC(),
                                                     _ruleInfo->vendorCode(),
                                                     ptFare.carrier(),
                                                     ptFare.tcrRuleTariff(),
                                                     ptFare.ruleNumber(),
                                                     rec2SeqNum,
                                                     secondarySellerId);
}

bool
NegotiatedFareController::checkMarkupControl(const MarkupControl& markupRec, bool ticketOrExchange)
{
  bool ret = true;

  if (markupRec.status() == PENDING) // always fail
  {
    ret = false;
  }
  else
  {
    const Indicator& tagToCheck = ticketOrExchange ? markupRec.tktTag() : markupRec.sellTag();
    if (tagToCheck != YES)
    {
      ret = false;
    }
  }

  return ret;
}

bool
NegotiatedFareController::matchedCorpIDInPricing(const PaxTypeFare& ptFare) const
{
  // There is no AccountCode from MUC (no redistribution)
  // at this point.
  // Do it only for the pricing/shopping.
  return isFdTrx() || !_trx.getOptions()->forceCorpFares() || ptFare.matchedCorpID();
}

bool
NegotiatedFareController::keepMinAmt(MoneyAmount& saveAmt,
                                     MoneyAmount newAmt,
                                     MoneyAmount& saveAmtNuc,
                                     MoneyAmount newAmtNuc,
                                     NegPaxTypeFareRuleData* ruleData,
                                     NegFareCalc& calcObj,
                                     const PaxTypeFare& ptFare,
                                     Indicator ticketingInd,
                                     const NegFareRest& negFareRest,
                                     Indicator fareDisplayCat35,
                                     bool isFareRetailer)
{
  bool rc = false;
  // check if new amount shall be stored
  if (shallKeepMinAmt(saveAmt, newAmt, *ruleData, ptFare, negFareRest))
  {
    if (!isFareRetailer ||
        ((newAmt < saveAmt && newAmtNuc < saveAmtNuc) || saveAmt == INVALID_AMT) || isFdTrx())
    {
      // update data from parameters
      saveAmt = newAmt;
      saveAmtNuc = newAmtNuc;
      ruleData->cat35Level() = calcObj.viewNetInd();
      ruleData->calculatedNegCurrency() = calcObj.curSide();
      ruleData->tktIndicator() = ticketingInd;
      ruleData->isT979FareIndInRange() = calcObj.isT979FareIndInRange();
      // abacus user
      if ((getAgent()->abacusUser() || _trx.getRequest()->ticketingAgent()->infiniUser() ||
           _trx.getTrxType() == PricingTrx::MIP_TRX) &&
          _trx.dataHandle().getVendorType(ptFare.vendor()) != PUBLISHED)
      {
        ruleData->creatorPCC() = calcObj.creatorPCC();
      }

      // axess user
      if (UNLIKELY(getAgent()->axessUser() && (_trx.getRequest()->isWpNettRequested() ||
                                                _trx.getRequest()->isWpSelRequested())))
      {
        ruleData->axessCat35Fare() = true;
      }

      // update class members
      _isNewMinAmt = true;
      _nationFranceFare = _nationFranceLocalInd;
      _accCodeMatch = _accCodeMUCMatch;
      _accCodeValue = _accCodeMUCValue;
      _fareDisplayMinCat35Type = fareDisplayCat35;
      rc = true;
    }
  }

  return rc;
}

void
NegotiatedFareController::keepFareRetailerMinAmtInfo(NegPaxTypeFareRuleData* ruleData,
                                                     const PseudoCityCode& pcc,
                                                     const uint64_t& ruleId,
                                                     const uint64_t& seqNo,
                                                     const FareRetailerCode& rc,
                                                     bool isCat25Responsive)
{
  ruleData->fareRetailerRuleId() = ruleId;
  ruleData->frrSeqNo() = seqNo;
  ruleData->sourcePseudoCity() = pcc;
  _ruleDataFareRetailer->fareRetailerRuleId() = ruleId;
  _ruleDataFareRetailer->frrSeqNo() = seqNo;
  _ruleDataFareRetailer->sourcePseudoCity() = pcc;
  if (isCat25Responsive)
  {
    ruleData->cat25Responsive() = true;
    _ruleDataFareRetailer->cat25Responsive() = true;
  }

  if (!fallback::fallbackFRRProcessingRetailerCode(&_trx))
  {
    _ruleDataFareRetailer->fareRetailerCode() = rc;
    ruleData->fareRetailerCode() = rc;
  }
}

//-------------------------------------------------------------------
// <PRE>
//
// @function  NegotiatedFareController::shallKeepMinAmt
//
// Description:  This method specifies if new min amount shall be kept.
//
// </PRE>/
//-------------------------------------------------------------------
bool
NegotiatedFareController::shallKeepMinAmt(const MoneyAmount& saveAmt,
                                          const MoneyAmount& newAmt,
                                          const NegPaxTypeFareRuleData& ruleData,
                                          const PaxTypeFare& ptFare,
                                          const NegFareRest& negFareRest)
{
  if (LIKELY(newAmt >= 0.0))
  {
    _isHierarchyApplied = (NegotiatedPtcHierarchy::isNegGroup(ptFare.fcasPaxType()) ||
                           NegotiatedPtcHierarchy::isJcbGroup(ptFare.fcasPaxType()) ||
                           NegotiatedPtcHierarchy::isPfaGroup(ptFare.fcasPaxType()));

    if (saveAmt == INVALID_AMT)
      return true;
    else if (_isHierarchyApplied)
      return NegotiatedPtcHierarchy::findLowerHierarchy(ruleData, negFareRest, ptFare);
    else
      return true;
  }
  return false;
}

//-------------------------------------------------------------------
// <PRE>
//
// @function  NegotiatedFareController::resetMinAmt
//
// Description: make room to store the data for the cheapest fare from ruleset(s)
//              also init fields known for all fares
//
// </PRE>/
//
//-------------------------------------------------------------------
void
NegotiatedFareController::resetMinAmt(MoneyAmount& saveAmt,
                                      MoneyAmount& saveAmtNuc,
                                      NegPaxTypeFareRuleData*& ruleData)
{
  saveAmt = INVALID_AMT;
  saveAmtNuc = INVALID_AMT;
  _isNewMinAmt = false;
  _isHierarchyApplied = false;

  _negFareRest = nullptr;
  ruleData = nullptr;

  _trx.dataHandle().get(ruleData);
  ruleData->categoryRuleInfo() = _ruleInfo;

  _nationFranceFare = false;
  _accCodeMatch = false;
  _accCodeValue.clear();
  _fareDisplayMinCat35Type = ' ';
  _rpData.reset();
}

//-------------------------------------------------------------------
// <PRE>
//
// @function  NegotiatedFareController::isSoftPass
//
// Description: determine if softpass bit needs to be set
//
// </PRE>/
//
//-------------------------------------------------------------------
bool
NegotiatedFareController::isSoftPass(NegPaxTypeFareRuleData* ruleData)
{
  // having a seqQual (IF condition) is always softPass for all items in the ruleset
  if (LIKELY(ruleData->categoryRuleItemInfoVec() != nullptr))
    return true;

  // or maybe this particular item in the ruleset is softPass
  const CategoryRuleItemInfo* catRuleItemInfo = ruleData->categoryRuleItemInfo();

  if ((catRuleItemInfo->directionality() != NegotiatedFareController::BLANK &&
       catRuleItemInfo->directionality() != NegotiatedFareController::LOC1_TO_LOC2 &&
       catRuleItemInfo->directionality() != NegotiatedFareController::LOC2_TO_LOC1) ||
      (catRuleItemInfo->inOutInd() != NegotiatedFareController::BLANK))
  {
    return true;
  }
  return false;
}

//-------------------------------------------------------------------
// <PRE>
//
// @function Record3ReturnTypes NegotiatedFareController::validate
//
// Description:  This method matches Record 3 Categaory 35.
//
// @param  ptFare   Base published or Cat 25 fare with DIS CAT TYPE L/C/T
// @param  negFareRest  A reference to the NegFareRest object
//
// @return pass - ptFare is validated
//         fail - ptFare fails this rule category restrictions
//         skip - skip this rule category
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
NegotiatedFareController::validate(PaxTypeFare& ptFare)
{
  Record3ReturnTypes ret = PASS;
  bool cat35LtypeEnabled(false);

  // Validate passenger type code
  if (!validatePaxType(ptFare, _negFareRest->psgType()))
  {
    doDiag335Collector(Diag335Collector::FAIL_PTC);
    ret = FAIL;
  }
  // Validate Date Override Table
  else if (!validateDateOverrideTable(ptFare, _negFareRest->overrideDateTblItemNo()))
  {
    doDiag335Collector(Diag335Collector::FAIL_TBL994);
    ret = FAIL;
  }
  // Validate Unavailable Tag and Text
  else if (UNLIKELY(_negFareRest->unavailTag() == RuleApplicationBase::dataUnavailable))
  {
    doDiag335Collector(Diag335Collector::FAIL_UNAVLTAG);
    ret = FAIL;
  }
  else if (UNLIKELY(_negFareRest->unavailTag() == RuleApplicationBase::textOnly))
  {
    doDiag335Collector(Diag335Collector::FAIL_TEXTONLY);
    ret = SKIP;
  }
  else if (!validateAllCarriers(ptFare))
  {
    doDiag335Collector(Diag335Collector::FAIL_CARRIER_RESRT);
    ret = FAIL;
  }
  // Validate IT/BT, Net Remit ticketing data for the Abacus User
  else if (UNLIKELY(!(cat35LtypeEnabled = TrxUtil::cat35LtypeEnabled(_trx)) &&
                     getAgent()->abacusUser() && !oldValidateCat35TktData(ptFare)))
  {
    ret = FAIL;
  }
  // ABACUS Cat35 Type L Automated pricing project
  else if (cat35LtypeEnabled && getAgent()->abacusUser() && !validateCat35TktData(ptFare))
  {
    ret = FAIL;
  } // Validate IT/BT, Net Remit ticketing data for the INFINI User
  else if (UNLIKELY(!cat35LtypeEnabled && getAgent()->infiniUser() &&
                     !oldValidateCat35TktData(ptFare)))
  {
    ret = FAIL;
  } // INFINI Cat35 Type L Automated pricing project
  else if (cat35LtypeEnabled && getAgent()->infiniUser() && !validateCat35TktData(ptFare))
  {
    ret = FAIL;
  } // Validate IT/BT, Net Remit ticketing data for the INFINI User

  // Validate Byte 101 value B/*
  else if ((_negFareRest->tktFareDataInd1() == RuleConst::NR_VALUE_B ||
            _negFareRest->tktFareDataInd1() == RuleConst::NR_VALUE_N) &&
           !TrxUtil::optimusNetRemitEnabled(_trx))
  {
    doDiag335Collector(Diag335Collector::FAIL_TKT_FARE_DATA_IND);
    ret = FAIL;
  }
  if (ret == PASS)
    doDiag335Collector(Diag335Collector::R3_PASS);
  return ret;
}

bool
NegotiatedFareController::validatePaxType(const PaxTypeFare& ptFare, const PaxTypeCode& paxTypeCode)
    const
{
  if (fallback::cat35_psgTypeMatching(&_trx))
  {
    if (paxTypeCode.empty() || (paxTypeCode == ADULT))
    {
      return true;
    }
  }
  else
  {
    if (paxTypeCode.empty())
    {
      return true;
    }

    // cat35 ADT
    if (paxTypeCode == ADULT)
    {
      // rec1/8 other
      if (ptFare.fcasPaxType() != ADULT && !ptFare.fcasPaxType().empty())
      {
        return false;
      }
      else
      {
        return true;
      }
    }
    else
    {
      if (ptFare.fcasPaxType().empty() || ptFare.fcasPaxType() == ADULT)
      {
        return false;
      }
    }
  }

  bool result = true;
  if (fallback::cat35_psgTypeMatching(&_trx) &&
      (ptFare.fcasPaxType().empty() || ptFare.fcasPaxType() == ADULT))
  {
    // Loop thru all requested pax types. Check if this Record 3 Cat 35 can be applied
    if (PaxTypeUtil::isAnActualPaxInTrx(_trx, ptFare.carrier(), paxTypeCode) == nullptr)
    {
      result = false;
    }
  }
  else
  {
    if (ptFare.fcasPaxType() != paxTypeCode)
    {
      if (NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(ptFare, paxTypeCode))
      {
        // Loop thru all requested pax types. Check if this Record 3 Cat 35 can be applied
        if ((PaxTypeUtil::isAnActualPaxInTrx(_trx, ptFare.carrier(), paxTypeCode) == nullptr) ||
            NegotiatedPtcHierarchy::matchLowerHierarchy(ptFare, ptFare.fcasPaxType(), paxTypeCode))
        {
          result = false;
        }
      }
      else
      {
        result = false;
      }
    }
  }
  return result;
}

//--------------------------------------------------------------------------
// Validate Date Override Table
// TODO: refactor to const!!!
//--------------------------------------------------------------------------
bool
NegotiatedFareController::validateDateOverrideTable(const PaxTypeFare& ptFare,
                                                    uint32_t overrideDateTblItemNo) const
{
  if (overrideDateTblItemNo)
  {
    DateTime travelDate;
    RuleUtil::getFirstValidTvlDT(
        travelDate, ptFare.fareMarket()->travelSeg(), true); // TODO: refactor to const!!!

    if (UNLIKELY(travelDate.isOpenDate()))
    {
      // use first travelSeg departure date(+1 logic), setby PricingModelMap
      travelDate = ptFare.fareMarket()->travelSeg().front()->departureDT();
    }
    DateTime bookingDate;
    RuleUtil::getLatestBookingDate(_trx, bookingDate, ptFare); // TODO: refactor to const!!!

    DiagMonitor diagMonitor(_trx, Diagnostic335);
    if (!(RuleUtil::validateDateOverrideRuleItem(_trx,
                                                 overrideDateTblItemNo,
                                                 ptFare.vendor(),
                                                 travelDate,
                                                 _trx.getRequest()->ticketingDT(),
                                                 bookingDate,
                                                 &diagMonitor.diag(),
                                                 Diagnostic335)))
    {
      return false;
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int NegotiatedFareController::validateCat35TktData
//
// Description:  To check IT/BT, Net Remit ticketing data in Record 3 Cat 35 (Abacus user).
//
// @param  negFareRest   A reference to the NegFareRest object
// @param  ptFare        A reference to the PaxTypeFare object
//
// @return true     - process succeeded

//         false    - process failed
// </PRE>
// ----------------------------------------------------------------------------
bool
NegotiatedFareController::oldValidateCat35TktData(const PaxTypeFare& ptFare) const
{
  const Indicator fareType = ptFare.fcaDisplayCatType();
  bool ret = true;

  if (isFdTrx())
  {
    //    Fare/RuleDisplay:
    //      type = L - pass rule. Create fare.
    //      type = C: no selling fare -> pass only for FQ NET.
    //                selling fare amount, bad commission -> pass rule. Create Fare.
    //      type = T: no T979 ->  fail rule/fare.
    if (fareType == RuleConst::NET_SUBMIT_FARE && _negFareRest->negFareCalcTblItemNo() == 0)
    {
      ret = false;
    }
  }
  else
  {
    // pricing/tkt/shoping
    const Indicator methodType = _negFareRest->netRemitMethod();
    const bool anyTicketedFareData = isAnyTicketedFareData();
    const bool validCommisions = isValidCommissions();

    if (!oldValidateCat35TktDataL(fareType, methodType, anyTicketedFareData) ||
        !validateCat35TktDataT(fareType, methodType, validCommisions) ||
        !validateCat35TktDataC(fareType, methodType, validCommisions) ||
        !validateCat35TktDataCT(fareType, methodType, anyTicketedFareData) ||
        // Check recurring segments data
        !validateCat35SegData(anyTicketedFareData) ||
        !validateCat35TktDataKorea(methodType, TrxUtil::isExchangeOrTicketing(_trx), *getAgent()))
    {
      ret = false;
    }
  }
  return ret;
}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function int NegotiatedFareController::validateCat35TktData
//
// Description:  This method will replace alidateCat35TktData - ABACUS Cat35 Type L Autopricing
// project
//
// @param  negFareRest   A reference to the NegFareRest object
// @param  ptFare        A reference to the PaxTypeFare object
//
// @return true     - process succeeded

//         false    - process failed
// </PRE>
// ----------------------------------------------------------------------------
bool
NegotiatedFareController::validateCat35TktData(const PaxTypeFare& ptFare) const
{
  const Indicator fareType = ptFare.fcaDisplayCatType();
  bool ret = true;

  if (UNLIKELY(isFdTrx()))
  {
    //    Fare/RuleDisplay:
    //      type = L - pass rule. Create fare.
    //      type = C: no selling fare -> pass only for FQ NET.
    //                selling fare amount, bad commission -> pass rule. Create Fare.
    //      type = T: no T979 ->  fail rule/fare.
    if (fareType == RuleConst::NET_SUBMIT_FARE && _negFareRest->negFareCalcTblItemNo() == 0)
    {
      ret = false;
    }
  }
  else
  {
    // pricing/tkt/shoping
    const Indicator methodType = _negFareRest->netRemitMethod();
    const bool anyTicketedFareData = isAnyTicketedFareData();
    const bool validCommisions = isValidCommissions();

    if (!validateCat35TktDataL(fareType, methodType, validCommisions) ||
        !validateCat35TktDataT(fareType, methodType, validCommisions) ||
        !validateCat35TktDataC(fareType, methodType, validCommisions) ||
        !validateCat35TktDataLCT(fareType, methodType, anyTicketedFareData) ||
        // Check recurring segments data
        !validateCat35SegData(anyTicketedFareData) ||
        !validateCat35TktDataKorea(methodType, TrxUtil::isExchangeOrTicketing(_trx), *getAgent()))
    {
      ret = false;
    }
  }
  return ret;
}
bool
NegotiatedFareController::oldValidateCat35TktDataL(const Indicator& fareType,
                                                   const Indicator& methodType,
                                                   bool anyTicketedFareData) const
{
  bool ret = true;
  //   fare type ='L':
  //      - method type is a blank
  //        and any ticketed fare data was filed ->fail rule and continue next rule/fare.
  //      - method type is not blank             ->fail rule and continue next rule/fare.
  if (fareType == RuleConst::SELLING_FARE &&
      ((methodType == RuleConst::NRR_METHOD_BLANK && anyTicketedFareData) ||
       (methodType != RuleConst::NRR_METHOD_BLANK)))
  {
    doDiag335Collector(Diag335Collector::FAIL_TKT_NETREMIT_SELLING);
    ret = false;
  }
  return ret;
}
bool
NegotiatedFareController::validateCat35TktDataL(const Indicator& fareType,
                                                const Indicator& methodType,
                                                bool validCommisions) const
{
  bool ret = true;
  //   fare type ='L':
  //    if- method type is not equal a blank(may be ->Net Remit)
  //      - and there is T979
  //      - and commission contain data in both (percent and amount) fields
  //    { do ->fail rule}
  if (fareType == RuleConst::SELLING_FARE && methodType != RuleConst::NRR_METHOD_BLANK)
  {
    if (UNLIKELY(!validCommisions))
    {
      doDiag335Collector(Diag335Collector::FAIL_COMM_NETREMIT_TYPE_L);
      ret = false;
    }
    else if (_negFareRest->negFareCalcTblItemNo() == 0 &&
             (!getAgent()->infiniUser() || methodType != RuleConst::NRR_METHOD_1))
    {
      doDiag335Collector(Diag335Collector::FAIL_TKT_NETREMIT_SELLING);
      ret = false;
    }
  }
  return ret;
}
bool
NegotiatedFareController::validateCat35TktDataT(const Indicator& fareType,
                                                const Indicator& methodType,
                                                bool validCommisions) const
{
  bool ret = true;
  // fare type ='T':
  //    if- method type is not equal a blank(may be ->Net Remit)
  //      - and there is T979
  //      - and commission contain data in both (percent and amount) fields
  //    { do ->fail rule}
  if (UNLIKELY(fareType == RuleConst::NET_SUBMIT_FARE &&
                (methodType != RuleConst::NRR_METHOD_BLANK && !validCommisions &&
                 _negFareRest->negFareCalcTblItemNo())))
  {
    doDiag335Collector(Diag335Collector::FAIL_COMM_NETREMIT_TYPE_T);
    ret = false;
  }
  return ret;
}

bool
NegotiatedFareController::validateCat35TktDataC(const Indicator& fareType,
                                                const Indicator& methodType,
                                                bool validCommisions) const
{
  bool ret = true;
  // fare type ='C':
  //    if- method type is not equal a blank(may be ->Net Remit)
  //      - and commission contain data in both (percent and amount) fields
  //    { do ->fail rule}
  if (UNLIKELY(fareType == RuleConst::NET_SUBMIT_FARE_UPD &&
                methodType != RuleConst::NRR_METHOD_BLANK && !validCommisions))
  {
    doDiag335Collector(Diag335Collector::FAIL_COMM_NETREMIT_TYPE_C);
    ret = false;
  }
  return ret;
}

bool
NegotiatedFareController::validateCat35TktDataCT(const Indicator& fareType,
                                                 const Indicator& methodType,
                                                 bool anyTicketedFareData) const
{
  bool ret = true;
  //   fare type ='T' or 'C':
  //    if- method type is not equal a blank(Net Remit) and
  //        there is no ticketed fare data ->fail rule and continue next rule/fare..
  //    { do ->fail rule}
  //    if- method type is not equal a blank(Net Remit) and tktFareDataInd1() = blank and
  //        there is ticketed fare data ->fail rule and continue next rule/fare..
  //    { do ->fail rule}
  if (fareType == RuleConst::NET_SUBMIT_FARE || fareType == RuleConst::NET_SUBMIT_FARE_UPD)
  {
    if (methodType == RuleConst::NRR_METHOD_BLANK)
    {
      if (anyTicketedFareData)
      {
        ret = false;
      } // otherwise, do normal cat35
    }
    else if (_negFareRest->tktFareDataInd1() == ' ' && anyTicketedFareData)
    {
      ret = false;
    }
    else if (methodType == RuleConst::NRR_METHOD_2 // method = 2
             ||
             (getAgent()->infiniUser() && methodType == RuleConst::NRR_METHOD_1))
    {
      // Abacus Enhancement - Allow fare with display type T with
      // no Ticketed Fare Data to be priced and stored in TFR.
      if (fareType == RuleConst::NET_SUBMIT_FARE_UPD && !anyTicketedFareData)
      {
        ret = false;
      }
    }
    else if (methodType != RuleConst::NRR_METHOD_3)
    {
      ret = false;
    }

    if (!ret)
    {
      doDiag335Collector(Diag335Collector::FAIL_TKT_NETREMIT_TYPE_C_T);
    }
  }
  return ret;
}

bool
NegotiatedFareController::validateCat35TktDataLCT(const Indicator& fareType,
                                                  const Indicator& methodType,
                                                  bool anyTicketedFareData) const
{
  bool ret = true;
  //   fare type ='T' or 'C' or 'L':
  //    if- method type is not equal a blank(Net Remit) and
  //        there is no ticketed fare data ->fail rule and continue next rule/fare..
  //    { do ->fail rule}
  //    if- method type is not equal a blank(Net Remit) and tktFareDataInd1() = blank and
  //        there is ticketed fare data ->fail rule and continue next rule/fare..
  //    { do ->fail rule}
  if (LIKELY(fareType == RuleConst::NET_SUBMIT_FARE ||
              fareType == RuleConst::NET_SUBMIT_FARE_UPD || fareType == RuleConst::SELLING_FARE))
  {
    if (methodType == RuleConst::NRR_METHOD_BLANK)
    {
      if (anyTicketedFareData)
      {
        ret = false;
      } // otherwise, do normal cat35
    }
    else if (UNLIKELY(_negFareRest->tktFareDataInd1() == ' ' && anyTicketedFareData))
    {
      ret = false;
    }
    else if (!(getAgent()->infiniUser() && methodType == RuleConst::NRR_METHOD_1) &&
             methodType != RuleConst::NRR_METHOD_2 && methodType != RuleConst::NRR_METHOD_3)
    {
      ret = false;
    }

    if (!ret)
    {
      doDiag335Collector(Diag335Collector::FAIL_TKT_NETREMIT_TYPE_L_C_T);
    }
  }
  return ret;
}

bool
NegotiatedFareController::validateCat35TktDataKorea(const Indicator& methodType,
                                                    bool ticketOrExchange,
                                                    const Agent& agent) const
{
  bool ret = true;
  // Check user location
  if (UNLIKELY(methodType == RuleConst::NRR_METHOD_3 && ticketOrExchange &&
                !LocUtil::isKorea(*(agent.agentLocation()))))
  {
    doDiag335Collector(Diag335Collector::FAIL_LOC_OUT_SIDE_KOREA);
    ret = false;
  }
  return ret;
}

bool
NegotiatedFareController::validateCat35SegData(bool anyTicketedFareData) const
{
  bool ret = true;
  if (UNLIKELY(isRecurringSegData())) // otherwise return true
  {
    if (!anyTicketedFareData) // check recurring segments data
    {
      doDiag335Collector(Diag335Collector::FAIL_REC_SEG_1_2_DATA);
      ret = false;
    }
    else if (!checkBetwAndCities()) // check if any city is empty
    {
      doDiag335Collector(Diag335Collector::FAIL_REC_SEG_BTW_AND);
      ret = false;
    }
    else if (_negFareRest->tktFareDataInd1() != 'N') // check fare ind
    {
      doDiag335Collector(Diag335Collector::FAIL_REC_SEG_TKT_FARE_IND);
      ret = false;
    }
    // Recurring segments data is valid only for the 'SNOW_MAN' itinerary
    else if (_itin.tripCharacteristics().isSet(Itin::OneWay) || _itin.travelSeg().size() != 4 ||
             _fareMarket.travelSeg().size() != 2)
    {
      doDiag335Collector(Diag335Collector::FAIL_REC_SEG_ITIN);
      ret = false;
    }
  }
  return ret;
}

bool
NegotiatedFareController::checkBetwAndCities() const
{
  return !_negFareRest->betwCity1().empty() && !_negFareRest->andCity1().empty() &&
         !_negFareRest->betwCity2().empty() && !_negFareRest->andCity2().empty();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int NegotiatedFareController::isRecurringSegData
//
// Description:  To check Tkt Fare data in bytes (101-136) (second segment)
//
// @param  negFareRest   A reference to the NegFareRest object
//
// @return true     - Yes, there is a Tkt Fare data.
//         false    - There is no Tkt Fare data.
// </PRE>
// ----------------------------------------------------------------------------
bool
NegotiatedFareController::isRecurringSegData() const
{
  bool ret = true; // default value

  if (LIKELY(_negFareRest->tktFareDataInd2() == ' ' && _negFareRest->owrt2() == ' ' &&
              _negFareRest->seasonType2() == ' ' && _negFareRest->dowType2() == ' ' &&
              _negFareRest->globalDir2() == GlobalDirection::ZZ &&
              _negFareRest->carrier21().empty() && _negFareRest->rule2().empty() &&
              _negFareRest->fareClass2().empty() && _negFareRest->fareType2().empty() &&
              _negFareRest->betwCity2().empty() && _negFareRest->andCity2().empty() &&
              _negFareRest->ruleTariff2() == 0))
  {
    ret = false; // No recurring segmens data
  }
  return ret;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int NegotiatedFareController::isValidCommissions
//
// Description:  To check a commissions cat35 data
//
// @param  negFareRest   A reference to the NegFareRest object
//
// @return true     - commissions is correct.
//         false    - bad commissions data.
// </PRE>
// ----------------------------------------------------------------------------
bool
NegotiatedFareController::isValidCommissions() const
{
  if (UNLIKELY(_negFareRest->netRemitMethod() == RuleConst::NRR_METHOD_1 &&
                _trx.getRequest()->ticketingAgent()->infiniUser()))
    return true;

  if (_negFareRest->commPercent() != RuleConst::PERCENT_NO_APPL)
  {
    // if there is any commissions amount (currency)  ---> bad data
    if (UNLIKELY(!_negFareRest->cur1().empty() || !_negFareRest->cur2().empty()))
    {
      return false;
    }
  }
  // otherwise, commissions amount or there is no commissions...
  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int NegotiatedFareController::isAnyTicketedFareData
//
// Description:  To check Tkt Fare data in bytes (101-136)
//
// @param  negFareRest   A reference to the NegFareRest object
//
// @return true     - Yes, there is a Tkt Fare data.
//         false    - There is no Tkt Fare data.
// </PRE>
// ----------------------------------------------------------------------------
bool
NegotiatedFareController::isAnyTicketedFareData() const
{
  bool ret = true; // default value

  if (_negFareRest->tktFareDataInd1() == ' ' && _negFareRest->owrt1() == ' ' &&
      _negFareRest->seasonType1() == ' ' && _negFareRest->dowType1() == ' ' &&
      _negFareRest->globalDir1() == GlobalDirection::ZZ && _negFareRest->carrier11().empty() &&
      _negFareRest->rule1().empty() && _negFareRest->fareClass1().empty() &&
      _negFareRest->fareType1().empty() && _negFareRest->betwCity1().empty() &&
      _negFareRest->andCity1().empty() && _negFareRest->ruleTariff1() == 0)
  {
    ret = false; // No tkt data in rule
  }
  return ret;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int NegotiatedFareController::doDiag335Collector
//
// Description:  To display Record 3 Cat 35.
//
// @param  negFareRest   A reference to the NegFareRest object
// @param  failCode      A failed reason code. Value R3_PASS if passes.
//
// </PRE>
// ----------------------------------------------------------------------------
void
NegotiatedFareController::doDiag335Collector(const char* failCode) const
{
  if (_dc->isActive())
  {
    Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);
    dc335->displayFailCode(*_negFareRest, failCode);
    if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "TKT")
    {
      // to display a Ticketing Net Remit data
      dc335->doNetRemitTkt(*_negFareRest, getAgent()->axessUser(), isRecurringSegData());
    }
  }
}

//-------------------------------------------------------------------
// <PRE>
//
// @function Record3ReturnTypes NegotiatedFareController::createFare
//
// Description:  This method create new Cat 35 pax type fare and
//               save it in the _negPaxTypeFares vector.
//
// @param  ptFare Base published or Cat 25 fare with DIS CAT TYPE L/C/T
// @param  fareAmount      Selling or Net fare amount
// @param  ruleInfo        Record 2 Cat 35
// @param  ruleItemInfoSet Record 2 Cat 35 set
// @param  segQual         Cat 35 qualifiers
// @param  catRuleItemInfo Record 2 Cat 35 segment
// @param  negFareRest     Record 3 Cat 35
//
// </PRE>
//-------------------------------------------------------------------
bool
NegotiatedFareController::createFare(PaxTypeFare& ptFare,
                                     NegPaxTypeFareRuleData* ruleData,
                                     bool isLocationSwapped,
                                     MoneyAmount fareAmount,
                                     MoneyAmount fareAmountNuc,
                                     bool pricingOption,
                                     const Indicator* fareDisplayCat35)
{
  // prevents from being executed when invoked with Pricing option set
  // for a Fare Display transaction
  if ((!pricingOption || !isFdTrx()) &&
      // not just to skip when fare not found, also
      // prevents ruleset logic from creating duplicates
      fareAmount >= 0.0)
  {
    const NegFareRest* negFareRest = dynamic_cast<const NegFareRest*>(ruleData->ruleItemInfo());
    PaxTypeFare* newPTFare = ptFare.clone(_trx.dataHandle());

    FareInfo* fareInfo =
        createFareInfoAndSetFlags(ptFare, *newPTFare, *ruleData, *negFareRest, fareDisplayCat35);

    // SPR#110258
    // Remove cat35 from map to force allocation of new PaxTypeFareAllRuleData object while setting
    // rule data (only needed for FD)
    if (UNLIKELY(isFdTrx()))
    {
      (*newPTFare->paxTypeFareRuleDataMap())[35].store(nullptr, std::memory_order_relaxed);
      newPTFare->resetCat35ViewNetIndicator(); // resets(FD_Cat35_ViewNetIndicator)
    }

    // other ruleData fields set by caller
    newPTFare->setRuleData(RuleConst::NEGOTIATED_RULE, _trx.dataHandle(), ruleData);
    ruleData->isLocationSwapped() = isLocationSwapped; // Record 2 Cat 35 isLocationSwapped

    setAmounts(ptFare,
               *newPTFare,
               *ruleData,
               *negFareRest,
               *fareInfo,
               fareAmount,
               fareAmountNuc,
               fareDisplayCat35);
    printMatchedFare(*ruleData, *newPTFare);

    ruleData->baseFare() = &ptFare;

    if (LIKELY(isSoftPass(ruleData)))
    {
      newPTFare->setCategorySoftPassed(RuleConst::NEGOTIATED_RULE, true);
    }

    // If faredisplay trx, create a FareDisplayInfo in the new paxtypefare.
    // Clone FareDisplayInfo object from the base paxtypefare so that
    // the new paxtypefare we do not share this with the base paxtypefare
    if (UNLIKELY(isFdTrx()))
    {
      if (!FareDisplayUtil::initFareDisplayInfo(_fdTrx, *newPTFare))
      {
        LOG4CXX_ERROR(logger,
                      "NegotiatedFareController::createFares - Unable to init FareDisplayInfo");
      }
      else
      {
        (ptFare.fareDisplayInfo())->clone(newPTFare->fareDisplayInfo(), newPTFare);
      }
    }

    if (TrxUtil::optimusNetRemitEnabled(_trx) &&
        _trx.dataHandle().getVendorType(newPTFare->vendor()) == RuleConst::SMF_VENDOR)
    {
      if (!_negFareRestExtRetrieved)
      {
        _negFareRestExt = _trx.dataHandle().getNegFareRestExt(
            ruleData->categoryRuleInfo()->vendorCode(), ruleData->categoryRuleItemInfo()->itemNo());
        _negFareRestExtRetrieved = true;
        if (_negFareRestExt && _negFareRestExt->tktFareDataSegExistInd() == 'Y')
        {
          _negFareRestExtSeq =
              _trx.dataHandle().getNegFareRestExtSeq(ruleData->categoryRuleInfo()->vendorCode(),
                                                     ruleData->categoryRuleItemInfo()->itemNo());
        }
      }
      ruleData->negFareRestExt() = _negFareRestExt;
      ruleData->negFareRestExtSeq() = _negFareRestExtSeq;

      if (!_farePropPrintOptRetrieved)
      {
        _fareProperties = _trx.dataHandle().getFareProperties(newPTFare->vendor(),
                                                              newPTFare->carrier(),
                                                              newPTFare->tcrRuleTariff(),
                                                              newPTFare->ruleNumber());
        _farePropPrintOptRetrieved = true;
        if (_fareProperties)
        {
          _valueCodeAlgorithm =
              _trx.dataHandle().getValueCodeAlgorithm(newPTFare->vendor(),
                                                      newPTFare->carrier(),
                                                      _fareProperties->valueCodeAlgorithmName(),
                                                      newPTFare->fareMarket()->travelDate());
        }

        if (newPTFare->vendor() == SMF_ABACUS_CARRIER_VENDOR_CODE && _negFareRest &&
            _negFareRest->netRemitMethod() != RuleConst::NRR_METHOD_BLANK &&
            _trx.getRequest()->ticketingAgent())
        {
          const std::string fareSource =
              newPTFare->carrier() + _trx.getRequest()->ticketingAgent()->agentCity();
          _printOption = _trx.dataHandle().getPrintOption(
              newPTFare->vendor(), fareSource, newPTFare->fareMarket()->travelDate());
        }
      }
      ruleData->fareProperties() = _fareProperties;
      ruleData->valueCodeAlgorithm() = _valueCodeAlgorithm;
      ruleData->printOption() = _printOption;
    }

    _rpData.catRuleItemInfo = _catRuleItemInfo;
    _rpData.ptf = newPTFare;
    _lastCreatedPtf = newPTFare;

    if (LIKELY(ruleData && ruleData->ruleItemInfo()))
      _rpData.itemNo = ruleData->ruleItemInfo()->itemNo();

    if (LIKELY(_negFareRest))
      _rpData.psgType = _negFareRest->psgType();

    if (LIKELY(_segQual))
      _rpData.isQualifierPresent = _segQual->size() > 0;

    _rpData.r2SubSetNum = _r2SubSetNum;
    _rpData.isFqTrx = isFdTrx();

    // Is fare created from invoke or from processRule?
    _rpData.isPricingOption = pricingOption;

    newPTFare->validatingCarriers().clear();

    if (!validatingCxr().empty())
    {
      newPTFare->validatingCarriers().push_back(validatingCxr());
      _rpData.validatingCxr = validatingCxr();
    }

    _negPtfBucketContainer->insert(_rpData);

    return true;
  }
  return false;
}

FareInfo*
NegotiatedFareController::createFareInfoAndSetFlags(PaxTypeFare& ptFare,
                                                    PaxTypeFare& newPTFare,
                                                    NegPaxTypeFareRuleData& ruleData,
                                                    const NegFareRest& negFareRest,
                                                    const Indicator* fareDisplayCat35)
{
  if (UNLIKELY(_rexCreateFaresForPrevAgent && _processPrevAgentForRex))
  {
    ruleData.rexCat35FareUsingPrevAgent() = true;
    newPTFare.setCategoryValid(RuleConst::NEGOTIATED_RULE, false);
  }

  newPTFare.status().set(PaxTypeFare::PTF_Negotiated);
  newPTFare.fareClassAppInfo() = ptFare.fareClassAppInfo();

  if (UNLIKELY(fareDisplayCat35 != nullptr)) // set the indicator to PaxTypeFare here if available
    newPTFare.fareDisplayCat35Type() = *fareDisplayCat35;

  fillSegInfo(ptFare, newPTFare, negFareRest);

  Fare* newFare = ptFare.fare()->clone(_trx.dataHandle()); // create fare

  FareInfo* fareInfo = ptFare.fare()->fareInfo()->clone(_trx.dataHandle()); // create fare info
  newFare->setFareInfo(fareInfo);

  newPTFare.setFare(newFare);

  if (UNLIKELY(_nationFranceFare)) // nation France
    newPTFare.setNationFRInCat35();

  if (UNLIKELY(_accCodeMatch))
  {
    newPTFare.setMatchedCorpID(); // corpID

    if (!_accCodeValue.empty()) // Acc COde
      newPTFare.matchedAccCode() = _accCodeValue;
  }

  return fareInfo;
}

bool
NegotiatedFareController::checkIfNeedNewFareAmt(PaxTypeFare& ptFare,
                                                PaxTypeFare& newPTFare,
                                                const Indicator* fareDisplayCat35)
{
  bool needNewFareAmt = ptFare.fcaDisplayCatType() != RuleConst::SELLING_FARE;

  if (UNLIKELY(isFdTrx()))
  {
    newPTFare.fareDisplayStatus().clear(PaxTypeFare::FD_Cat35_Failed_Rule);

    // amount needed for FD's net/redist version of L fare
    if (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE &&
        *fareDisplayCat35 != RuleConst::SELLING_CARRIER_FARE)
    {
      needNewFareAmt = true;
    }

    // FD reuses pub fares for this situation
    // uncomment if clone C/T pub fares for 'N' FQ entries
    // amount not needed for net version of C/T fare
    if (ptFare.fcaDisplayCatType() != RuleConst::SELLING_FARE &&
        *fareDisplayCat35 == RuleConst::NET_FARE)
    {
      needNewFareAmt = false;
    }
  }

  // Type C for Jal/Axess selling amt = net amt
  if (UNLIKELY(isJalAxessTypeC(ptFare)))
  {
    needNewFareAmt = false;
  }

  return needNewFareAmt;
}

void
NegotiatedFareController::setAmounts(PaxTypeFare& ptFare,
                                     PaxTypeFare& newPTFare,
                                     NegPaxTypeFareRuleData& ruleData,
                                     const NegFareRest& negFareRest,
                                     FareInfo& fareInfo,
                                     MoneyAmount fareAmount,
                                     MoneyAmount fareAmountNuc,
                                     const Indicator* fareDisplayCat35)
{
  // fill net amount in rule data
  if (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE)
  {
    if (negFareRest.negFareCalcTblItemNo() != 0)
    {
      ruleData.netAmount() = fareAmount;
      ruleData.nucNetAmount() = fareAmountNuc;
    }
  }
  else // copy values from old fare
  {
    ruleData.netAmount() = ptFare.fareAmount();
    ruleData.nucNetAmount() = ptFare.nucFareAmount();
  }

  if (checkIfNeedNewFareAmt(ptFare, newPTFare, fareDisplayCat35))
  {
    // set fare info amounts
    fareInfo.originalFareAmount() = (ptFare.isRoundTrip()) ? fareAmount * 2.0 : fareAmount;
    fareInfo.fareAmount() = fareAmount;
    // set fare amounts
    newPTFare.nucFareAmount() = fareAmountNuc;

    if (_trx.isExchangeTrx() && _trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    {
      const RexBaseTrx& rexTrx = static_cast<const RexBaseTrx&>(_trx);

      if (!rexTrx.newItinSecondROEConversionDate().isEmptyDate())
      {
        _calcMoney.setupConverionDateForHistorical();
        newPTFare.rexSecondNucFareAmount() = CurrencyUtil::getMoneyAmountInNUC(
            fareAmount, ptFare.currency(), rexTrx.newItinSecondROEConversionDate(), _trx);
      }
    }

    if (fabs(ptFare.nucFareAmount() - fareAmountNuc) > EPSILON) // the amount is different
    {
      newPTFare.nucOriginalFareAmount() =
          (ptFare.isRoundTrip()) ? fareAmountNuc * 2.0 : fareAmountNuc;
    }
  }
}

void
NegotiatedFareController::printMatchedFare(const NegPaxTypeFareRuleData& ruleData,
                                           const PaxTypeFare& newPTFare,
                                           const bool forceDisplay)
{
  std::ostringstream oss;
  oss << "CAT 35 FARE MATCHED ON R3-" << ruleData.ruleItemInfo()->itemNo();
  if (!valCarrier().empty())
    oss << "     VALIDATING CXR - " << valCarrier();
  oss << "\n";

  if (UNLIKELY(!(ruleData.creatorPCC().empty())))
    oss << "CREATOR PCC-" << ruleData.creatorPCC() << "\n";

  if (UNLIKELY(getAgent()->cwtUser()))
    oss << "NATION FRANCE INDICATOR- " << (_nationFranceFare ? "TRUE" : "FALSE") << "\n";
  oss << "SELLING AMT-" << std::setw(8) << Money(newPTFare.fareAmount(), newPTFare.currency());
  oss << " NET AMT-" << std::setw(8) << Money(ruleData.netAmount(), newPTFare.currency());
  oss << " LVL-" << ruleData.cat35Level() << "\n";
  doDiag335Collector(oss.str(), forceDisplay);
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void NegotiatedFareController::fillSegInfo
//
// Description: overwrite FareClassAppSegInfo with cat35 data (if present)
//              This cat35 data has precedence over rec1, cat19 and cat25 data
//              only clones if data is changing
// @param  ptFare       PaxType Fare to access FareClassAppSegInfo for reading rec1/19/25
// @param  newPTFare    PaxType Fare to access FareClassAppSegInfo for writing
// @param  negFareRest  A reference to the NegFareRest object for reading cat35 data
// </PRE>
// ----------------------------------------------------------------------------
void
NegotiatedFareController::fillSegInfo(const PaxTypeFare& ptFare,
                                      PaxTypeFare& newPTFare,
                                      const NegFareRest& negFareRest) const
{
  FareClassAppSegInfo* fcasInfo = ptFare.fareClassAppSegInfo()->clone(_trx.dataHandle());
  newPTFare.fareClassAppSegInfo() = fcasInfo;

  if ((ptFare.fcasPaxType().empty() || ptFare.fcasPaxType() == ADULT) &&
      !negFareRest.psgType().empty() && negFareRest.psgType() != ADULT)
  {
    fcasInfo->_paxType = negFareRest.psgType();
    PaxType* paxType = const_cast<PaxType*>(
        PaxTypeUtil::isAnActualPaxInTrx(_trx, newPTFare.carrier(), fcasInfo->_paxType));

    if (LIKELY(paxType))
      newPTFare.actualPaxType() = paxType;
  }
}

void
NegotiatedFareController::processRedistributions(PaxTypeFare& ptFare,
                                                 NegPaxTypeFareRuleData* ruleData,
                                                 MoneyAmount& amt,
                                                 MoneyAmount& amtNuc,
                                                 Money& base,
                                                 NegFareCalc& calcObj,
                                                 bool is979queried,
                                                 int32_t seqNegMatch)
{
  std::set<long> secIds;
  getSecIdsForRedistribution(seqNegMatch, secIds);

  bool rc = false;

  if (_markupSuccessCount > 0 || (_fareRetailerSuccessCount == 0 && _markupSuccessCount == 0))
  {
    rc = processMarkupForRedistribution(
        ptFare, ruleData, amt, amtNuc, base, calcObj, is979queried, seqNegMatch, secIds, true);
    if (rc == true)
    {
      _markupSuccessCount++;
      return;
    }

    if (getAgent()->tvlAgencyPCC() != getAgent()->mainTvlAgencyPCC())
    {
      rc = processMarkupForRedistribution(ptFare,
                                          ruleData,
                                          amt,
                                          amtNuc,
                                          base,
                                          calcObj,
                                          is979queried,
                                          seqNegMatch,
                                          secIds,
                                          false); // Match with Home PCC
    }
    if (rc == true)
    {
      _markupSuccessCount++;
      return;
    }
  }
  return;
}

bool
NegotiatedFareController::processMarkupForRedistribution(PaxTypeFare& ptFare,
                                                         NegPaxTypeFareRuleData* ruleData,
                                                         MoneyAmount& amt,
                                                         MoneyAmount& amtNuc,
                                                         Money& base,
                                                         NegFareCalc& calcObj,
                                                         bool is979queried,
                                                         int32_t seqNegMatch,
                                                         std::set<long>& secIds,
                                                         bool isTvlPcc)
{
  bool ret = false;
  MoneyAmount newAmt, newAmtNuc;

  /////////////////////////////////
  // redistributions from 980
  /////////////////////////////////
  const std::vector<MarkupControl*>& markupCtrl = getMarkupByPcc(ptFare, isTvlPcc);

  std::vector<MarkupControl*>::const_iterator markupRec;
  std::vector<MarkupCalculate*>::const_iterator muCalcRec;

  bool ticketOrExchange = TrxUtil::isExchangeOrTicketing(_trx);

  for (markupRec = markupCtrl.begin(); markupRec != markupCtrl.end(); markupRec++)
  {
    if (!matchMarkupRec(**markupRec, 'R', isTvlPcc, 0))
      continue;

    if (secIds.find((*markupRec)->secondarySellerId()) == secIds.end())
      // no NegFareSecurity with redistributeInd = Y and the same secondarySellerId was found
      continue;

    _invalidAgency = false;

    if (!NegotiatedFareController::ifMarkupValidForRedistribution(**markupRec, ticketOrExchange))
      continue;

    // If it is multi account code / corporate id scenario
    if (_trx.getRequest()->isMultiAccCorpId())
    {
      // Is account code populated in MUC?
      if (!matchMultiCorpIdAccCodeMUC(**markupRec, ptFare))
        continue;
    }
    else if (_trx.isFlexFare())
    {
      // Is account code populated in MUC?
      if (!matchMultiCorpIdAccCodeMUCFlexFare(**markupRec, ptFare))
        continue;
    }
    else
    {
      if (!matchCorpIdAccCodeMUC(**markupRec, ptFare))
        continue;
    }

    for (muCalcRec = (*markupRec)->calcs().begin(); muCalcRec != (*markupRec)->calcs().end();
         muCalcRec++)
    {
      // get carrier range if missing
      if (!is979queried)
      {
        get979(ptFare, false, calcObj);
      }

      // note that this call preserves range info from 979
      calcObj.load(_ruleInfo->vendorCode(),
                   *muCalcRec,
                   (*markupRec)->viewNetInd(),
                   (*markupRec)->creatorPseudoCity());

      if (calcObj.isMatchFareLoc(_trx, ptFare) && calcObj.isMatchMarkupCalc(*muCalcRec, ptFare))
      {
        std::ostringstream oss;
        oss << "REDIST ";
        doDiag335Collector(oss.str(), false);
        *_dc << **muCalcRec;
        oss << "REDISTRIBUTION ACCOUNT CODE: "
            << ((*markupRec)->accountCode().empty() ? "BLANK" : (*markupRec)->accountCode())
            << "\n";
        doDiag335Collector(oss.str(), false);

        calcObj.getPrice(_trx, ptFare, *this, newAmt, newAmtNuc);

        NegFareSecurityInfo* dummySecRec;
        _trx.dataHandle().get(dummySecRec);
        dummySecRec->updateInd() = (*markupRec)->updateTag();
        dummySecRec->redistributeInd() = (*markupRec)->redistributeTag();
        dummySecRec->sellInd() = (*markupRec)->sellTag();
        dummySecRec->ticketInd() = (*markupRec)->tktTag();
        dummySecRec->localeType() = (*markupRec)->ownerPseudoCityType();

        prepareNewSellingAmt(ptFare, newAmt);
        keepMinAmt(amt,
                   newAmt,
                   amtNuc,
                   newAmtNuc,
                   ruleData,
                   calcObj,
                   ptFare,
                   (*markupRec)->tktTag(),
                   *_negFareRest,
                   ptFare.fareDisplayCat35Type());

        if (isFdTrx())
        {
          createFaresForFD(
              ptFare, newAmt, newAmtNuc, base, calcObj, *dummySecRec, (*markupRec)->tktTag());
        }
        else if (!_trx.getOptions()->isCat35Net() &&
                 _fareDisplayMinCat35Type == RuleConst::NET_FARE)
        {
          _fareDisplayMinCat35Type = RuleConst::SELLING_MARKUP_FARE;
        }
        ret = true;
        break; // stop at the first match within the same sending PCC
      }
    } // endfor - calc/sec recs
  } // endfor - redist recs

  return ret;
}

void
NegotiatedFareController::getSecIdsForRedistribution(int32_t seqNegMatch, std::set<long>& ret) const
{
  const std::vector<NegFareSecurityInfo*>& secList = getNegFareSecurity();
  std::vector<NegFareSecurityInfo*>::const_iterator i = secList.begin();
  std::vector<NegFareSecurityInfo*>::const_iterator ie = secList.end();
  for (; i != ie; ++i)
  {
    if ((*i)->redistributeInd() != 'Y')
      continue;
    if (UNLIKELY(seqNegMatch != 0 && (*i)->seqNo() >= seqNegMatch))
      continue;
    // store element
    ret.insert((*i)->secondarySellerId());
  }
}

bool
NegotiatedFareController::ifMarkupValidForRedistribution(const MarkupControl& markupCntrl,
                                                         bool ticketOrExchange)
{
  bool ret = false;

  if (markupCntrl.status() != DECLINED && markupCntrl.status() != PENDING)
  {
    // choose tag for checking
    const Indicator& tagToCheck = ticketOrExchange ? markupCntrl.tktTag() : markupCntrl.sellTag();

    if (tagToCheck == YES)
    {
      ret = true;
    }
  }

  return ret;
}

bool
NegotiatedFareController::matchMultiCorpIdAccCodeMUC(const MarkupControl& markupRec,
                                                     const PaxTypeFare& paxTypeFare)
{
  bool ret = true;

  if (!markupRec.accountCode().empty())
  {
    bool accCodeMUCNoMatch = false;
    bool corpIdMUCNoMatch = false;

    // Try to match requested corporate IDs with MUC
    if (!_trx.getRequest()->corpIdVec().empty())
    {
      // If not match, mark as not matched.
      if (!matchMUCAccCode(markupRec, _trx.getRequest()->corpIdVec()) && // try corp ID
          !checkMultiCorpIdMatrix(markupRec, paxTypeFare)) // and multi corp ID
      {
        corpIdMUCNoMatch = true;
      }
    }
    else
    {
      corpIdMUCNoMatch = true;
    }

    // Try to match requested account code with MUC but only when
    // CorpID did'nt match or CorpIdVec is empty.
    if (corpIdMUCNoMatch && !_trx.getRequest()->accCodeVec().empty())
    {
      if (!matchMUCAccCode(markupRec, _trx.getRequest()->accCodeVec()))
      {
        accCodeMUCNoMatch = true;
      }
    }
    else
    {
      accCodeMUCNoMatch = true;
    }

    if (corpIdMUCNoMatch && accCodeMUCNoMatch)
    {
      ret = false; // skip this MUC record.
    }
    else
    {
      _accCodeMUCValue = markupRec.accountCode().c_str();
      _accCodeMUCMatch = true;
    }
  }
  else // MUC without account code.
  {
    ret = matchEmptyCorpIdAccCodeMUC(paxTypeFare);
  }

  return ret;
}

bool
NegotiatedFareController::matchCorpIdAccCodeMUC(const MarkupControl& markupRec,
                                                const PaxTypeFare& paxTypeFare)
{
  bool ret = true;

  if (!markupRec.accountCode().empty()) // Is Account_code populated in MUC?
  {
    // Check the AccoutCode in MUC.
    //  accountCode can be populated only during the redistribution.
    if (!_trx.getRequest()->corporateID().empty()) //  Corp_ID in Request ?
    {
      if (markupRec.accountCode() != _trx.getRequest()->corporateID()) // Corp_ID matched ?
      {
        if (!checkCorpIdMatrix(markupRec, paxTypeFare))
        {
          ret = false;
        }
      }
    }
    else if (!_trx.getRequest()->accountCode().empty()) // Account code in request ?
    {
      if (markupRec.accountCode() != _trx.getRequest()->accountCode()) // Account code matched ?
      {
        ret = false;
      }
    }
    else // There is no Acc_Code in request.
    {
      ret = false; // skip this MUC record
    }

    if (ret)
    {
      _accCodeMUCMatch = true;
      _accCodeMUCValue = markupRec.accountCode().c_str();
    }
  }
  else // MUC was redistributed without accoutCode (blank)
  {
    ret = matchEmptyCorpIdAccCodeMUC(paxTypeFare);
  }

  return ret;
}

bool
NegotiatedFareController::matchEmptyCorpIdAccCodeMUC(const PaxTypeFare& paxTypeFare)
{
  bool ret = true;

  _accCodeMUCMatch = false;
  _accCodeMUCValue.clear();

  if (!isFdTrx() && // Do it only for the pricing/shopping.
      _trx.getOptions()->forceCorpFares() &&
      !paxTypeFare.matchedCorpID())
  {
    ret = false;
  }

  return ret;
}

void
NegotiatedFareController::prepareNewSellingAmt(const PaxTypeFare& paxTypeFare, MoneyAmount& retAmt)
    const
{
  if (paxTypeFare.fcaDisplayCatType() == RuleConst::SELLING_FARE && retAmt < 0.0)
  {
    if (isFdTrx())
    {
      retAmt = paxTypeFare.fareAmount();
    }
    else
    {
      retAmt = 0.0;
    }
  }
}

void
NegotiatedFareController::createFaresForFD(PaxTypeFare& ptFare,
                                           MoneyAmount newAmt,
                                           MoneyAmount newAmtNuc,
                                           Money& base,
                                           NegFareCalc& calcObj,
                                           NegFareSecurityInfo& secRec,
                                           const Indicator& ticketInd)
{
  MoneyAmount wholesaleAmt, wholeSaleNucAmt;
  Money nucBase(ptFare.nucOriginalFareAmount(), NUC);

  // create final fare (both 979 and MuCtrl applied)
  Indicator fdFinalCat35Type = RuleConst::SELLING_MARKUP_FARE;
  if (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE)
  {
    fdFinalCat35Type = RuleConst::NET_FARE;

    // pub L fares need copy ( may have new paxtype )
    invokeCreateFare(ptFare,
                     ptFare.fareAmount(),
                     ptFare.nucFareAmount(),
                     calcObj,
                     &secRec,
                     RuleConst::SELLING_MARKUP_FARE,
                     ticketInd);
  }

  // if selling fare was created, now create net fare
  invokeCreateFare(ptFare, newAmt, newAmtNuc, calcObj, &secRec, fdFinalCat35Type, ticketInd);

  // create 'middleman' fare
  if (!calcObj.getWholesalerFareAmt(base,
                                    nucBase,
                                    wholesaleAmt,
                                    wholeSaleNucAmt,
                                    *this,
                                    _itin.useInternationalRounding(),
                                    ptFare.isRoundTrip()))
  {
    if (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE)
    {
      wholesaleAmt = newAmt;
      wholeSaleNucAmt = newAmtNuc;
    }
    else
    {
      wholesaleAmt = ptFare.fareAmount();
      wholeSaleNucAmt = ptFare.nucFareAmount();
    }
  }

  if ((ptFare.fcaDisplayCatType() != RuleConst::NET_SUBMIT_FARE_UPD) ||
      (!_ruleDataFareRetailer->fareRetailerRuleId() ||
       (!_trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
        _trx.getRequest()->ticketingAgent()->tvlAgencyPCC() !=
            _ruleDataFareRetailer->sourcePseudoCity())))
  {
    if ((wholesaleAmt != 0.0) && (wholeSaleNucAmt == 0.0))
    {
      Money native(ptFare.currency());
      Money nuc("NUC");

      native.value() = wholesaleAmt;
      _ccf->convertCalc(nuc,
                        native,
                        _trx,
                        _itin.useInternationalRounding(),
                        CurrencyConversionRequest::OTHER,
                        false,
                        nullptr,
                        &_cache);
      wholeSaleNucAmt = nuc.value();
    }

    invokeCreateFare(ptFare,
                     wholesaleAmt,
                     wholeSaleNucAmt,
                     calcObj,
                     &secRec,
                     RuleConst::REDISTRIBUTED_FARE,
                     ticketInd);
  }

  if (calcObj.viewNetInd() != 'B' && // don't show this one
      ptFare.fcaDisplayCatType() != RuleConst::SELLING_FARE)
  {
    ptFare.invalidateFare(PaxTypeFare::FD_Cat35_ViewNetIndicator);
  }
}

bool
NegotiatedFareController::processSecurityMatch(bool& isPosMatch,
                                               bool& is979queried,
                                               MoneyAmount& amt,
                                               MoneyAmount& amtNuc,
                                               Money& base,
                                               NegPaxTypeFareRuleData* ruleData,
                                               PaxTypeFare& ptFare,
                                               int32_t& seqNegMatch,
                                               NegFareCalc& calcObj)
{
  const std::vector<NegFareSecurityInfo*>& secList = getNegFareSecurity();
  bool ret = true;
  bool matchT983 = false;
  if (secList.empty())
  {
    LOG4CXX_DEBUG(logger, "NFC::getValidSecurity() - missing security records");

    static const std::string msg("T983:NO SABRE1S OR MISSING TABLE\n");
    doDiag335Collector(msg, false);
    ret = false;
  }
  else
  {
    bool ticketOrExchange = TrxUtil::isExchangeOrTicketing(_trx);
    std::vector<NegFareSecurityInfo*>::const_iterator secRec;

    for (secRec = secList.begin(); secRec != secList.end(); ++secRec)
    {
      if (UNLIKELY(*secRec == nullptr))
      {
        LOG4CXX_DEBUG(logger, "NFC::getValidSecurity() - blank security record");
        continue;
      }

      const NegFareSecurity secObj(*secRec);

      if (!secObj.isMatch(_trx, getAgent())) // Match Who/Where fields
        continue;

      matchT983 = true;

      if (!processCalcObj(isPosMatch,
                          is979queried,
                          seqNegMatch,
                          ptFare,
                          calcObj,
                          *secRec,
                          ruleData,
                          base,
                          amt,
                          amtNuc,
                          secObj,
                          ticketOrExchange) &&
          secRec == secList.begin())
      {
        // If negative matched the lowest numbered Table 983 sequence,
        // fail this Record 3 Cat 35, continue processing next OR table or next set.
        ret = false;
      }

      // If negative matched NOT the lowest numbered Table 983 sequence,
      // Continue processing Table 980 data in all LOWER numbered Table 983 sequences
      // with Redistribute (byte 41) value Y.
      break;
    } // endfor - all SecRec 983

    // If no matched Table 983 process FRR
    if (processFrrv(ptFare) && !matchT983)
    {
      if (_fareRetailerSuccessCount > 0 ||
          (_fareRetailerSuccessCount == 0 && _markupSuccessCount == 0))
      {
        ret = processFareRetailerRuleValidation(
            ptFare, ruleData, amt, amtNuc, base, calcObj, is979queried, false, 0);
        if (ret)
        {
          _fareRetailerSuccessCount++;
          return ret;
        }
        return false;
      }
    }
    if (!isPosMatch && ret)
    {
      static const std::string msg("T983:NO MATCH\n");
      doDiag335Collector(msg, false);
    }
    else if (ret && _dc->diagnosticType() == Diagnostic335)
    {
      _passSecurity = true;
    }
  }
  return ret;
}

bool
NegotiatedFareController::createFaresFromCalcObj(PaxTypeFare& ptFare,
                                                 NegFareCalc& calcObj,
                                                 NegFareSecurityInfo* secRec,
                                                 NegPaxTypeFareRuleData* ruleData,
                                                 Money& base,
                                                 MoneyAmount& amt,
                                                 MoneyAmount& amtNuc,
                                                 Indicator ticketingInd)
{
  bool donotIgnoreCTypeFare = false;
  MoneyAmount newAmt, newAmtNuc;

  // get calc info from 979 (carrier) and/or 980 (agent)
  if (!getOneCalc(ptFare,
                  secRec->updateInd() == YES,
                  secRec->secondarySellerId(),
                  calcObj,
                  ruleData,
                  base,
                  amt,
                  amtNuc,
                  secRec))
  {
    // donot ignore C Type Updatable fares that donot have calculation info yet
    if (isFdTrx() && secRec->updateInd() == YES &&
        ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD)
    {
      donotIgnoreCTypeFare = true;
    }
    else
    {
      return false;
    }
  }

  if (donotIgnoreCTypeFare) // create NET for C type
  {
    calcObj.makeDummyCalcInfo(base);
    invokeCreateFare(ptFare,
                     base.value(),
                     ptFare.nucOriginalFareAmount(),
                     calcObj,
                     secRec,
                     RuleConst::NET_FARE,
                     ticketingInd);
  }
  else if (_isFDCat25Responsive==false)
  {
    Indicator fdFinalCat35Type = RuleConst::SELLING_MARKUP_FARE;
    if (fallback::fallbackProcessTypeLAndT(&_trx) || _isFDFareCreatedForFareRetailer == false)
    {
      calcObj.getPrice(_trx, ptFare, *this, newAmt, newAmtNuc);

      if (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE)
      {
        fdFinalCat35Type = RuleConst::SELLING_CARRIER_FARE;
        if (newAmt < 0.0) // is invalid
        {
          newAmt = 0.0;
        }
        if (isFdTrx()) // create NET for FD
        {
          invokeCreateFare(
              ptFare, newAmt, newAmtNuc, calcObj, secRec, RuleConst::NET_FARE, ticketingInd);
        }
      }
    }

    if (_isFDFareCreatedForFareRetailer == false)
    {
      saveCandidateFares(amt,
                         newAmt,
                         amtNuc,
                         newAmtNuc,
                         ruleData,
                         calcObj,
                         secRec,
                         ptFare,
                         fdFinalCat35Type,
                         ticketingInd);
    }
  }

  return true;
}

bool
NegotiatedFareController::processCalcObj(bool& isPosMatch,
                                         bool& is979queried,
                                         int32_t& seqNegMatch,
                                         PaxTypeFare& ptFare,
                                         NegFareCalc& calcObj,
                                         NegFareSecurityInfo* secRec,
                                         NegPaxTypeFareRuleData* ruleData,
                                         Money& base,
                                         MoneyAmount& amt,
                                         MoneyAmount& amtNuc,
                                         const NegFareSecurity& secObj,
                                         Indicator ticketOrExchange)
{
  bool ret = true;
  // 'Positive' matched Table 983.
  // Do not continue next Table 983 sequences. But continue processing Table 980 data
  // in all Table 983 sequences with Redistribute (byte 41) value Y.
  if (secObj.isPos())
  {
    isPosMatch = true;
    if (checkTktAuthorityAndUpdFlag(secObj, secRec, ticketOrExchange, ptFare))
    {
      // Acess Type C WPNETT - always ignore 979, create a dummy
      if (UNLIKELY(isJalAxessTypeC(ptFare)))
      {
        calcObj.makeDummyCalcInfo(base);
        saveCandidateFares(amt,
                           base.value(),
                           amtNuc,
                           ptFare.nucFareAmount(),
                           ruleData,
                           calcObj,
                           secRec,
                           ptFare,
                           RuleConst::SELLING_CARRIER_FARE,
                           secObj.ticketInd());
      }
      else
      {
        // simple pass-thru or need calc
        if (_negFareRest->negFareCalcTblItemNo() == 0 && secRec->updateInd() != YES)
        {
          // no 979 info and no updates to existing fare amount
          // save using dummy calcObj
          calcObj.makeDummyCalcInfo(base);
          saveCandidateFares(amt,
                             base.value(),
                             amtNuc,
                             ptFare.nucFareAmount(),
                             ruleData,
                             calcObj,
                             secRec,
                             ptFare,
                             RuleConst::SELLING_CARRIER_FARE,
                             secObj.ticketInd());
          is979queried = true;
        }
        else if (createFaresFromCalcObj(
                     ptFare, calcObj, secRec, ruleData, base, amt, amtNuc, secObj.ticketInd()))
        {
          is979queried = true;
        } // endif - need calcinfo
      }
    }
  }
  else
  {
    seqNegMatch = secRec->seqNo();
    std::ostringstream oss;
    oss << "T983:SEQ NBR" << std::setw(8);
    oss << seqNegMatch << " NEG MATCH\n";
    doDiag335Collector(oss.str(), false);
    // 'Negative' matched Table 983.
    ret = false;
  }
  return ret;
}

bool
NegotiatedFareController::checkTktAuthorityAndUpdFlag(const NegFareSecurity& secObj,
                                                      NegFareSecurityInfo* secRec,
                                                      Indicator isTkt,
                                                      PaxTypeFare& ptf)
{
  bool ret = true;
  _invalidAgency = false;

  if ((!checkTktAuthority(secObj, secRec, isTkt) &&
       // FD still shows fares that cannot price or ticket
       // RD*35 command diplays details based on T983 flags
       !isFdTrx()) ||
      !checkUpdFlag(secRec, ptf))
  {
    ret = false;
  }
  else
  {
    *_dc << *secRec;
    setNationFrance(secRec->loc1(), secRec->loc2());
  }
  return ret;
}

void
NegotiatedFareController::setNationFrance(const LocKey& lc1, const LocKey& lc2)
{
  if (UNLIKELY((lc1.locType() == LOCTYPE_NATION && lc1.loc() == NATION_FRANCE) ||
                (lc2.locType() == LOCTYPE_NATION && lc2.loc() == NATION_FRANCE)))
  {
    _nationFranceLocalInd = true;
  }
  else
  {
    _nationFranceLocalInd = false;
  }
}

void
NegotiatedFareController::invokeCreateFare(PaxTypeFare& ptFare,
                                           MoneyAmount fareAmount,
                                           MoneyAmount nucFareAmount,
                                           NegFareCalc& calcObj,
                                           NegFareSecurityInfo* secRec,
                                           Indicator fareDisplayCat35,
                                           Indicator ticketingInd)
{
  bool save_nationFranceFare = false;
  bool save_accCodeMatch = false;
  std::string save_accCodeValue;

  // set up rule data info for this fare creation
  NegPaxTypeFareRuleData* fareDisplayRuleData = nullptr;
  _trx.dataHandle().get(fareDisplayRuleData); // lint !e530

  fareDisplayRuleData->categoryRuleInfo() = _ruleInfo;

  fareDisplayRuleData->cat35Level() = calcObj.viewNetInd();
  fareDisplayRuleData->calculatedNegCurrency() = calcObj.curSide();

  fareDisplayRuleData->creatorPCC() = calcObj.creatorPCC();
  fareDisplayRuleData->securityRec() = secRec;

  if (processFrrv(ptFare))
  {
    fareDisplayRuleData->fareRetailerRuleId() = _ruleDataFareRetailer->fareRetailerRuleId();
    fareDisplayRuleData->frrSeqNo() = _ruleDataFareRetailer->frrSeqNo();
    fareDisplayRuleData->sourcePseudoCity() = _ruleDataFareRetailer->sourcePseudoCity();
    fareDisplayRuleData->updateInd() = secRec->updateInd();
    fareDisplayRuleData->redistributeInd() = secRec->redistributeInd();
    fareDisplayRuleData->sellInd() = secRec->sellInd();
    fareDisplayRuleData->ticketInd() = secRec->ticketInd();
    fareDisplayRuleData->cat25Responsive() = _ruleDataFareRetailer->cat25Responsive();
    if (!fallback::fallbackFRRProcessingRetailerCode(&_trx))
      fareDisplayRuleData->fareRetailerCode() = _ruleDataFareRetailer->fareRetailerCode();
  }

  // save data from calcObj into fareDisplayRuleData
  calcObj.getSellingInfo(fareDisplayRuleData->calcInd(),
                         fareDisplayRuleData->ruleAmt(),
                         fareDisplayRuleData->noDecAmt(),
                         fareDisplayRuleData->percent(),
                         fareDisplayRuleData->noDecPercent());

  fareDisplayRuleData->categoryRuleItemInfoSet() = _ruleItemInfoSet;
  fareDisplayRuleData->categoryRuleItemInfo() = _catRuleItemInfo;
  fareDisplayRuleData->categoryRuleItemInfoVec() = _segQual;
  fareDisplayRuleData->ruleItemInfo() = _negFareRest;
  // Selling NET fares will have baseFare set as they donot have to go
  // through createFare logic
  fareDisplayRuleData->baseFare() = &ptFare;

  fareDisplayRuleData->tktIndicator() = ticketingInd;

  if (ptFare.fcaDisplayCatType() != RuleConst::SELLING_FARE && // not L type fares
      ptFare.paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE) == nullptr)
  {
    ptFare.setRuleData(RuleConst::NEGOTIATED_RULE, _trx.dataHandle(), fareDisplayRuleData);
  }

  // save pricing indicators
  save_nationFranceFare = _nationFranceFare;
  _nationFranceFare = _nationFranceLocalInd;
  save_accCodeMatch = _accCodeMatch;
  _accCodeMatch = _accCodeMUCMatch;
  save_accCodeValue = _accCodeValue;
  _accCodeValue = _accCodeMUCValue;

  createFare(ptFare,
             fareDisplayRuleData,
             _isLocationSwapped,
             fareAmount,
             nucFareAmount,
             false,
             &fareDisplayCat35);

  // restore pricing indicator
  _nationFranceFare = save_nationFranceFare;
  _accCodeMatch = save_accCodeMatch;
  _accCodeValue = save_accCodeValue;
}

void
NegotiatedFareController::saveCandidateFares(MoneyAmount& amt,
                                             MoneyAmount newAmt,
                                             MoneyAmount& amtNuc,
                                             MoneyAmount newAmtNuc,
                                             NegPaxTypeFareRuleData* ruleData,
                                             NegFareCalc& calcObj,
                                             NegFareSecurityInfo* secRec,
                                             PaxTypeFare& ptFare,
                                             Indicator fareDisplayCat35,
                                             Indicator ticketingInd)
{
  // Do different things for Pricing and FareDisplay here
  if (UNLIKELY(isFdTrx()))
  {
    // For FareDisplay - try to create a new PaxTypeFare
    invokeCreateFare(ptFare, newAmt, newAmtNuc, calcObj, secRec, fareDisplayCat35, ticketingInd);
  }
  else
  {
    // For Pricing - keep track of the minimum amount
    keepMinAmt(amt,
               newAmt,
               amtNuc,
               newAmtNuc,
               ruleData,
               calcObj,
               ptFare,
               ticketingInd,
               *_negFareRest,
               fareDisplayCat35);
  }
}

void
NegotiatedFareController::setCat35Indicator(PaxTypeFare& ptFare)
{
  if (ptFare.fcaDisplayCatType() == RuleConst::SELLING_FARE)
  {
    ptFare.fareDisplayCat35Type() = RuleConst::SELLING_CARRIER_FARE;
  }
  else if (LIKELY(ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
                   ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))
  {
    ptFare.fareDisplayCat35Type() = RuleConst::NET_FARE;
  }
}

bool
NegotiatedFareController::checkTktAuthority(const NegFareSecurity& secObj,
                                            NegFareSecurityInfo* secRec,
                                            Indicator isTkt)
{
  bool ret = true;

  if (UNLIKELY(!secObj.isMatchWhat(isTkt)))
  {
    std::ostringstream oss;
    oss << "T983:SEQ NBR " << std::setw(8);
    oss << secRec->seqNo() << "NOT AUTHORIZED TO SELL OR TKT\n";
    doDiag335Collector(oss.str(), false);
    ret = false;
  }
  return ret;
}

bool
NegotiatedFareController::checkCorpIdMatrix(const MarkupControl& muc,
                                            const PaxTypeFare& paxTypeFare) const
{
  const std::vector<tse::CorpId*>& corpIds =
      _trx.dataHandle().getCorpId(_trx.getRequest()->corporateID(),
                                  paxTypeFare.carrier(),
                                  paxTypeFare.fareMarket()->travelDate());

  if (corpIds.empty())
  {
    const std::vector<tse::CorpId*>& corpIdsOfBlankCxr = _trx.dataHandle().getCorpId(
        _trx.getRequest()->corporateID(), "", paxTypeFare.fareMarket()->travelDate());
    return (matchCorpIdMatrix(muc, paxTypeFare, corpIdsOfBlankCxr));
  }
  else
  {
    return (matchCorpIdMatrix(muc, paxTypeFare, corpIds));
  }
}

bool
NegotiatedFareController::matchCorpIdMatrix(const MarkupControl& muc,
                                            const PaxTypeFare& paxTypeFare,
                                            const std::vector<tse::CorpId*>& corpIds) const
{
  std::vector<CorpId*>::const_iterator i = corpIds.begin();
  for (; i != corpIds.end(); ++i)
  {
    CorpId& corpId = **i;
    if (corpId.accountCode().empty()) // corpID not specified
      continue;
    if (!corpId.accountCode().empty() && // corpID specified but not matched
        corpId.accountCode() != muc.accountCode())
      continue;
    if (corpId.vendor() != paxTypeFare.vendor())
      continue;
    if (corpId.ruleTariff() != -1 && corpId.ruleTariff() != paxTypeFare.tcrRuleTariff())
      continue;
    if (!corpId.rule().empty() && corpId.rule() != paxTypeFare.ruleNumber())
      continue;

    return true; // matched - don't continue
  }
  return false; // none of the elemnt matched
}

bool
NegotiatedFareController::checkUpdFlag(const NegFareSecurityInfo* secRec, PaxTypeFare& ptf)
{
  // When Update is value 'Y', DIS CAT TYPE must be C.
  std::ostringstream oss;
  if (secRec->updateInd() == YES && ptf.fcaDisplayCatType() != RuleConst::NET_SUBMIT_FARE_UPD)
  {
    if (isFdTrx())
      ptf.invalidateFare(PaxTypeFare::FD_Cat35_Failed_Rule);

    oss << "T983:SEQ NBR " << std::setw(8);
    oss << secRec->seqNo() << " UPDATE IS Y - DCT MUST BE C\n";
    doDiag335Collector(oss.str(), false);

    return false;
  }

  // When DIS CAT TYPE is C with Update value 'N', must have T979
  if (ptf.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD && secRec->updateInd() != YES &&
      _negFareRest->negFareCalcTblItemNo() == 0)
  {
    if (isFdTrx())
      ptf.invalidateFare(PaxTypeFare::FD_Cat35_Failed_Rule);
    oss << "T983:SEQ NBR" << std::setw(8);
    oss << secRec->seqNo() << "DISP TYPE C WITH UPD-N MUST HAVE T979\n";
    doDiag335Collector(oss.str(), false);
    return false;
  }
  return true;
}

const std::vector<NegFareSecurityInfo*>&
NegotiatedFareController::getNegFareSecurity() const
{
  return _trx.dataHandle().getNegFareSecurity(_ruleInfo->vendorCode(),
                                              _negFareRest->negFareSecurityTblItemNo());
}

const std::vector<MarkupControl*>&
NegotiatedFareController::getMarkupBySecurityItemNo(const PaxTypeFare& ptFare) const
{
  return _trx.dataHandle().getMarkupBySecurityItemNo(getAgent()->tvlAgencyPCC(),
                                                     getAgent()->mainTvlAgencyPCC(),
                                                     _ruleInfo->vendorCode(),
                                                     ptFare.carrier(),
                                                     ptFare.tcrRuleTariff(),
                                                     ptFare.ruleNumber(),
                                                     _ruleInfo->sequenceNumber(),
                                                     DateTime::emptyDate());
}

const std::vector<MarkupControl*>&
NegotiatedFareController::getMarkupByPcc(const PaxTypeFare& ptFare, bool isTvlPcc) const
{
  return _trx.dataHandle().getMarkupByPcc(isTvlPcc ? getAgent()->tvlAgencyPCC()
                                                   : getAgent()->mainTvlAgencyPCC(),
                                          _ruleInfo->vendorCode(),
                                          ptFare.carrier(),
                                          ptFare.tcrRuleTariff(),
                                          ptFare.ruleNumber(),
                                          _ruleInfo->sequenceNumber(),
                                          DateTime::emptyDate());
}

bool
NegotiatedFareController::matchMarkupRec(const MarkupControl& markupRec,
                                         Indicator markupType,
                                         bool isTvlPcc,
                                         long secondarySellerId)
{
  if (markupRec.markupType() != markupType)
    return false;
  if (isTvlPcc)
  {
    if (!(markupRec.ownerPseudoCityType() == 'T' || markupRec.ownerPseudoCityType() == 'U'))
      return false;
  }
  else
  {
    if (markupRec.ownerPseudoCityType() != 'U')
      return false;
  }
  if (markupRec.markupType() == 'U' && markupRec.secondarySellerId() != secondarySellerId)
    return false;

  return true;
}

const std::vector<NegFareCalcInfo*>&
NegotiatedFareController::getNegFareCalc() const
{
  return _trx.dataHandle().getNegFareCalc(_ruleInfo->vendorCode(),
                                          _negFareRest->negFareCalcTblItemNo());
}

bool
NegotiatedFareController::matchMUCAccCode(const MarkupControl& markupRec,
                                          const std::vector<std::string>& accCodes) const
{
  std::vector<std::string>::const_iterator currIter = accCodes.begin();
  std::vector<std::string>::const_iterator endIter = accCodes.end();

  while (currIter != endIter)
  {
    if ((*currIter) == markupRec.accountCode()) // we have to match MUC accountCode
    { // with at least one corpID from request.
      return true;
    }

    ++currIter;
  }

  return false;
}

bool
NegotiatedFareController::checkMultiCorpIdMatrix(const MarkupControl& markupRec,
                                                 const PaxTypeFare& paxTypeFare)
{
  std::vector<std::string>::iterator currIter = _trx.getRequest()->corpIdVec().begin();
  std::vector<std::string>::iterator endIter = _trx.getRequest()->corpIdVec().end();

  while (currIter != endIter)
  {
    const std::vector<tse::CorpId*>& corpIds = _trx.dataHandle().getCorpId(
        *currIter, paxTypeFare.carrier(), paxTypeFare.fareMarket()->travelDate());

    if (corpIds.empty())
    {
      // try blank cxr
      const std::vector<tse::CorpId*>& corpIdsOfBlankCxr =
          _trx.dataHandle().getCorpId(*currIter, "", paxTypeFare.fareMarket()->travelDate());

      if (matchCorpIdMatrix(markupRec, paxTypeFare, corpIdsOfBlankCxr))
        return true;
    }
    else
    {
      if (matchCorpIdMatrix(markupRec, paxTypeFare, corpIds))
        return true;
    }

    ++currIter;
  }

  return false; // not matched
}

bool
NegotiatedFareController::isJalAxessTypeC(PaxTypeFare& ptFare)
{
  bool ret;

  if (UNLIKELY(isFdTrx()))
    ret = false;
  else
    ret = (_isJalAxessUser && ptFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD);

  return ret;
}

const Agent*
NegotiatedFareController::getAgent() const
{
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX && _rexCreateFaresForPrevAgent &&
                _processPrevAgentForRex))
  {
    return ((static_cast<const RexPricingRequest&>(*(_trx.getRequest()))).prevTicketIssueAgent());
  }

  return (_trx.getRequest()->ticketingAgent()); // not rex trx
}

void
NegotiatedFareController::setRexCreateFaresForPrevAgent()
{
  _rexCreateFaresForPrevAgent = false;
  if (LIKELY(!RexPricingTrx::isRexTrxAndNewItin(_trx) ||
              !static_cast<RexPricingTrx*>(&_trx)->needRetrieveKeepFare(_itinIndex) ||
              (static_cast<const RexPricingRequest&>(*(_trx.getRequest())))
                      .prevTicketIssueAgent() == nullptr))
  {
    return; // this is not rex trx with new itin
  }

  RexPricingTrx& rexPricingTrx = static_cast<RexPricingTrx&>(_trx);
  RexPricingTrx::NewItinKeepFareMap::const_iterator keepFareMapToNewFmIter =
      rexPricingTrx.newItinKeepFares(_itinIndex).begin();
  RexPricingTrx::NewItinKeepFareMap::const_iterator keepFareMapToNewFmIterEnd =
      rexPricingTrx.newItinKeepFares(_itinIndex).end();

  for (; keepFareMapToNewFmIter != keepFareMapToNewFmIterEnd; ++keepFareMapToNewFmIter)
  {
    const PaxTypeFare& excItinFare = *(keepFareMapToNewFmIter->first);
    const FareMarket& newItinFm = *(keepFareMapToNewFmIter->second);
    if (excItinFare.isNegotiated() &&
        rexPricingTrx.ticketingDate() == excItinFare.retrievalDate() &&
        newItinFm.origin()->loc() == _fareMarket.origin()->loc() &&
        newItinFm.destination()->loc() == _fareMarket.destination()->loc())
    {
      _rexCreateFaresForPrevAgent = true; // found
      break;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int NegotiatedFareController::doDiag335Collector
//
// Description:  To display messages.
//
// @param  Message    to be moved to fare item
// @param  Flag       to indicate Pass/Fail Secutity.
//
// </PRE>
// ----------------------------------------------------------------------------
void
NegotiatedFareController::doDiag335Collector(const std::string& msg, bool showDiag) const
{
  if (_dc->isActive())
  {
    if (_dc->diagnosticType() != Diagnostic335)
    {
      *_dc << msg;
    }
    else
    {
      Diag335Collector* dc335 = static_cast<Diag335Collector*>(_dc);
      dc335->displayMessage(msg, showDiag);
    }
  }
}

void
NegotiatedFareController::diagDisplayRemainingRelation(
    const std::vector<CategoryRuleItemInfo>& seg,
    std::vector<CategoryRuleItemInfo>::const_iterator endIt)
{
  for (std::vector<CategoryRuleItemInfo>::const_iterator it = seg.begin(); it != endIt; ++it)
  {
    (*_dc) << (*it);
  }
  doDiag335Collector(Diag335Collector::R2_FAIL_IF, false);
}

void
NegotiatedFareController::addPaxTypeFare(const NegPaxTypeFareData& data, std::ostringstream& oss)
{
  if (UNLIKELY(!data.ptf))
    return;

  _negPaxTypeFares.push_back(data.ptf);
  if (_dc->isActive())
  {
    oss.str("");
    oss << "CAT 35 FARE CREATED ON R3-" << data.itemNo << "\n";

    if (processFrrv(*(data.ptf)))
    {
      const NegPaxTypeFareRuleData* negPtfRule = data.ptf->getNegRuleData();
      if (negPtfRule != nullptr && negPtfRule->fareRetailerRuleId())
      {
        oss << " FARE RETAILER RULE DATA:\n"
            << "  ID NUMBER : " << negPtfRule->fareRetailerRuleId() << "\n"
            << "  SQ NUMBER : " << negPtfRule->frrSeqNo() << "\n"
            << "  SOURCE PCC: " << negPtfRule->sourcePseudoCity() << "\n";
      }
    }
    doDiag335Collector(oss.str(), false);
  }
}

/*
 * It filters duplicate fares and fares that are created from invoke but has no
 * corresponding fare from processRule. The later scenario is only possible
 * in case of FQ context
 */
void
NegotiatedFareController::selectNegFares()
{
  std::ostringstream oss;

  bool addAllFares = false;
  // for duplicaed fare markets we want to add all fares
  // and remove duplicates at the end of FCO
  if (!_trx.isValidatingCxrGsaApplicable())
  {
    addAllFares = _fareMarket.hasDuplicates();
  }

  if (addAllFares)
  {
    for (NegPTFBucket& bucket : *_negPtfBucketContainer)
    {
      for (NegPTFBucket::R3NegFares& r3NegFares : bucket)
      {
        for (NegPaxTypeFareData& negPtfData : r3NegFares)
        {
          addPaxTypeFare(negPtfData, oss);
        }
      }
    }

    if (!_negPtfBucketContainer->empty())
      _fareMarket.addNegFaresBuckets(_negPtfBucketContainer);

    return;
  }

  for (NegPTFBucket& bucket : *_negPtfBucketContainer)
  {
    for (NegPaxTypeFareData& negPtfData : *bucket.begin())
    {
      addPaxTypeFare(negPtfData, oss);
    }
  }
}

bool
NegotiatedFareController::matchMultiCorpIdAccCodeMUCFlexFare(const MarkupControl& markupRec,
                                                             const PaxTypeFare& paxTypeFare)
{
  bool ret = true;

  if (!markupRec.accountCode().empty())
  {
    bool accCodeCorpIdMUCMatched = false;

    flexFares::GroupsData::const_iterator itr = _trx.getRequest()->getFlexFaresGroupsData().begin();
    flexFares::GroupsData::const_iterator itrE = _trx.getRequest()->getFlexFaresGroupsData().end();
    flexFares::GroupId groupId = 0;

    while ((itr != itrE) && (!accCodeCorpIdMUCMatched))
    {
      groupId = itr->first;

      if (!_trx.getRequest()->getFlexFaresGroupsData().getCorpIds(groupId).empty())
      {
        if (matchMUCAccCodeFlexFare(
                markupRec, _trx.getRequest()->getFlexFaresGroupsData().getCorpIds(groupId)) ||
            checkMultiCorpIdMatrixFlexFare(
                markupRec,
                paxTypeFare,
                _trx.getRequest()->getFlexFaresGroupsData().getCorpIds(groupId)))
        {
          accCodeCorpIdMUCMatched = true;
          break;
        }
      }

      if (!_trx.getRequest()->getFlexFaresGroupsData().getAccCodes(groupId).empty())
      {
        if (matchMUCAccCodeFlexFare(
                markupRec, _trx.getRequest()->getFlexFaresGroupsData().getAccCodes(groupId)))
        {
          accCodeCorpIdMUCMatched = true;
        }
      }
      itr++;
    }

    if (!accCodeCorpIdMUCMatched)
    {
      ret = false; // skip this MUC record.
    }
    else
    {
      _accCodeMUCValue = markupRec.accountCode().c_str();
      _accCodeMUCMatch = true;
    }
  }
  else // MUC without account code.
  {
    ret = matchEmptyCorpIdAccCodeMUC(paxTypeFare);
  }

  return ret;
}

bool
NegotiatedFareController::matchMUCAccCodeFlexFare(const MarkupControl& markupRec,
                                                  const std::set<std::string>& accCodes) const
{
  std::set<std::string>::const_iterator currIter = accCodes.begin();
  std::set<std::string>::const_iterator endIter = accCodes.end();

  while (currIter != endIter)
  {
    if ((*currIter) == markupRec.accountCode()) // we have to match MUC accountCode
    { // with at least one corpID from request.
      return true;
    }
    ++currIter;
  }
  return false;
}

bool
NegotiatedFareController::checkMultiCorpIdMatrixFlexFare(const MarkupControl& markupRec,
                                                         const PaxTypeFare& paxTypeFare,
                                                         const std::set<std::string>& corpIdSet)
{
  std::set<std::string>::const_iterator currIter = corpIdSet.begin();
  std::set<std::string>::const_iterator endIter = corpIdSet.end();

  while (currIter != endIter)
  {
    const std::vector<tse::CorpId*>& corpIds = _trx.dataHandle().getCorpId(
        *currIter, paxTypeFare.carrier(), paxTypeFare.fareMarket()->travelDate());

    if (corpIds.empty())
    {
      // try blank cxr
      const std::vector<tse::CorpId*>& corpIdsOfBlankCxr =
          _trx.dataHandle().getCorpId(*currIter, "", paxTypeFare.fareMarket()->travelDate());

      if (matchCorpIdMatrix(markupRec, paxTypeFare, corpIdsOfBlankCxr))
        return true;
    }
    else
    {
      if (matchCorpIdMatrix(markupRec, paxTypeFare, corpIds))
        return true;
    }

    ++currIter;
  }
  return false; // not matched
}
bool
NegotiatedFareController::validateAllCarriers(PaxTypeFare& ptFare)
{
  bool ret = true;
  bool shoppingTrx(_trx.isShopping());

  if (_trx.isValidatingCxrGsaApplicable() && !valCarrier().empty())
  {
    if (!shoppingTrx)
    {
      if (!FareUtil::isNegFareCarrierValid(
              _negFareRest->carrier(), _negFareRest->tktAppl(), valCarrier(), isFdTrx()))
      {
        ret = false;
      }
    }
  }
  // Validate carrier for non shopping trx
  // Shopping does not have itin yet, check validating carrier later in shoppingPQ
  else if (!shoppingTrx && !ptFare.fareMarket()->hasDuplicates() &&
           !FareUtil::isNegFareCarrierValid(
               _negFareRest->carrier(), _negFareRest->tktAppl(), _itin, isFdTrx()))
  {
    ret = false;
  }
  return ret;
}

Agent*
NegotiatedFareController::createAgentSourcePcc(const PseudoCityCode& sourcePcc)
{
  std::vector<Customer*> custList = _trx.dataHandle().getCustomer(sourcePcc);
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

  return agent;
}
bool
NegotiatedFareController::processSourcePccSecurityMatch(
    const std::vector<NegFareSecurityInfo*>& secList,
    const Agent* sourcePcc,
    FareRetailerRuleContext& context)
{
  if (secList.empty())
  {
    LOG4CXX_DEBUG(logger, "NFC::getValidSecurity() - missing security records");
    static const std::string msg("SOURCE PCC T983: MISSING TABLE\n");
    doDiag335Collector(msg, false);
    return false;
  }
  std::vector<NegFareSecurityInfo*>::const_iterator secRec;
  for (secRec = secList.begin(); secRec != secList.end(); ++secRec)
  {
    if (UNLIKELY(*secRec == nullptr))
    {
      LOG4CXX_DEBUG(logger, "NFC::getValidSecurity() - blank security record");
      continue;
    }
    const NegFareSecurity secObj(*secRec);

    if (!secObj.isMatch(_trx, sourcePcc))
    {
      continue;
    }
    else
    {
      if ((*secRec)->sellInd() != 'Y' || !secObj.isPos())
      {
        if (_dc->isActive())
        {
          std::ostringstream oss;
          oss << "  SOURCE PCC -" << sourcePcc->tvlAgencyPCC() << " FAILED IN TABLE T983 "
              << "\n";
          doDiag335Collector(oss.str(), false);
        }
        return false;
      }
      context._t983 = *secRec;
      return true;
    }
  }
  if (_dc->isActive())
  {
    std::ostringstream oss;
    oss << "  SOURCE PCC -" << sourcePcc->tvlAgencyPCC() << " MISSING IN TABLE T983 "
        << "\n";
    doDiag335Collector(oss.str(), false);
  }
  return false;
}

bool
NegotiatedFareController::processPermissionIndicatorsOld(const FareRetailerRuleContext& context,
                                                         const NegFareSecurityInfo* secRec,
                                                         NegPaxTypeFareRuleData* ruleData,
                                                         const Indicator& catType)
{
  if (context._t983 == nullptr)
    return false;

  if (TrxUtil::isExchangeOrTicketing(_trx))
  {
    if (context._t983->ticketInd() == 'N')
      return false;
    if ((context._frrfai)->ticketInd() == 'N')
      return false;
  }
  else
  {
    // if Cat35 T983 from SourcePccc URST=YYNY ATSE will never FQ/Price/Shop the fare.
    if (context._t983->sellInd() == 'N')
      return false; // CASE 7 - 9

    // Since S = N in AR, do not process this CAT35
    if ((context._frrfai)->sellInd() == 'N')
      return false; // CASE 15 - 17
  }

  bool sourceIsTVL = (context._sourcePcc == getAgent()->tvlAgencyPCC());
  bool sourceIsMain = (context._sourcePcc == getAgent()->mainTvlAgencyPCC());

  if ((!sourceIsTVL && !sourceIsMain) &&
      context._t983->redistributeInd() == 'N') // Fail to process this fare in requested pcc,
    return false; // since redistribution is not permitted and,
  // is not branch of source pcc. CASE 6

  ruleData->updateInd() = context._t983->updateInd();
  ruleData->redistributeInd() = context._t983->redistributeInd();
  ruleData->sellInd() = context._t983->sellInd();
  ruleData->ticketInd() = context._t983->ticketInd();

  if (!sourceIsTVL && !sourceIsMain)
  {
    if (context._t983->updateInd() == 'Y' && (context._frrfai)->updateInd() == 'N') // CASE 14 20
    {
      ruleData->updateInd() = (context._frrfai)->updateInd();
    }
    if (context._t983->ticketInd() == 'Y' && (context._frrfai)->ticketInd() == 'N')
    {
      ruleData->ticketInd() = (context._frrfai)->ticketInd();
    }
    return true;
  }

  if (sourceIsTVL || sourceIsMain)
  {
    if (context._t983->ticketInd() == 'Y' &&
        (context._frrfai)->ticketInd() == 'N') // CASE  13   18-19
    {
      ruleData->ticketInd() = (context._frrfai)->ticketInd();
    }
    if (context._t983->updateInd() == 'Y' && (context._frrfai)->updateInd() == 'N')
    {
      ruleData->updateInd() = (context._frrfai)->updateInd();
    }
    return true;
  }

  return true;
}

bool
NegotiatedFareController::processPermissionIndicators(const FareRetailerRuleContext& context,
                                                      const NegFareSecurityInfo* secRec,
                                                      NegPaxTypeFareRuleData* ruleData,
                                                      const Indicator& catType)
{
  if (fallback::fallbackFRRRedistFix(&_trx))
    return processPermissionIndicatorsOld(context, secRec, ruleData, catType);

  if (context._t983 == nullptr)
    return false;

  if (TrxUtil::isExchangeOrTicketing(_trx))
  {
    if ((context._t983->ticketInd() == 'N') || (context._frrfai->ticketInd() == 'N'))
      return false;
  }

  if ((context._t983->sellInd() == 'N') || (context._frrfai->sellInd() == 'N'))
    return false; // CASE 7-9 & 15-17

  bool sourceIsItself = (context._sourcePcc == getAgent()->tvlAgencyPCC()) ||
                        (context._sourcePcc == getAgent()->mainTvlAgencyPCC());

  if (!sourceIsItself && (context._t983->redistributeInd() == 'N'))
  {
    /* CASE 6: Fail to process this fare in requested pcc, since redistribution
       is not permitted and it is not branch of source pcc. */
    return false;
  }

  ruleData->updateInd() = context._t983->updateInd();
  ruleData->redistributeInd() = context._t983->redistributeInd();
  ruleData->sellInd() = context._t983->sellInd();
  ruleData->ticketInd() = context._t983->ticketInd();

  if ((context._t983->updateInd() == 'Y') && (context._frrfai->updateInd() == 'N')) // CASE 14 20
    ruleData->updateInd() = 'N';

  if ((context._t983->ticketInd() == 'Y') && (context._frrfai->ticketInd() == 'N')) // CASE 13 18-19
    ruleData->ticketInd() = 'N';

  if (((context._t983->redistributeInd() == 'Y') && (context._frrfai->redistributeInd() == 'N')) &&
      !sourceIsItself)
    ruleData->redistributeInd() = 'N';

  return true;
}

void
NegotiatedFareController::getRedistWholeSaleCalcPrice(NegFareCalc& calcObj,
                                                      PaxTypeFare& ptFare,
                                                      NegPaxTypeFareRuleData* negPaxTypeFare,
                                                      const Money& base)
{
  Money nucBase(ptFare.nucOriginalFareAmount(), NUC);

  if (!_ruleDataFareRetailer->fareRetailerRuleId() ||
      (!_trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
       _trx.getRequest()->ticketingAgent()->tvlAgencyPCC() !=
           _ruleDataFareRetailer->sourcePseudoCity()))
  {
    if (!calcObj.getWholesalerFareAmt(base,
                                      nucBase,
                                      negPaxTypeFare->wholeSaleNetAmount(),
                                      negPaxTypeFare->wholeSaleNucNetAmount(),
                                      *this,
                                      _itin.useInternationalRounding(),
                                      ptFare.isRoundTrip()))
    {
      negPaxTypeFare->wholeSaleNetAmount() = ptFare.fareAmount();
      negPaxTypeFare->wholeSaleNucNetAmount() = ptFare.nucFareAmount();
    }
    else
    {
      if (negPaxTypeFare->wholeSaleNetAmount() != 0.0 &&
          negPaxTypeFare->wholeSaleNucNetAmount() == 0.0)
      {
        Money native(ptFare.currency());
        Money nuc("NUC");

        native.value() = negPaxTypeFare->wholeSaleNetAmount();
        _ccf->convertCalc(nuc,
                          native,
                          _trx,
                          _itin.useInternationalRounding(),
                          CurrencyConversionRequest::OTHER,
                          false,
                          nullptr,
                          &_cache);
        negPaxTypeFare->wholeSaleNucNetAmount() = nuc.value();
      }
    }
  }
}

bool
NegotiatedFareController::isPtfValidForCat25Responsive(const PaxTypeFare& ptFare)
{
  if (ptFare.isFareByRule())
  {
    const FBRPaxTypeFareRuleData* fbrPTFare = ptFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (fbrPTFare && !fbrPTFare->isSpecifiedFare())
    {
      //this is Cat25 calculated
      if (!ptFare.isDiscounted())
        return true;
      else
      {
        Indicator ind = ptFare.discountInfo().farecalcInd();
        if(ind == DiscountedFareController::CALCULATED ||
           ind == DiscountedFareController::ADD_CALCULATED_TO_SPECIFIED ||
           ind == DiscountedFareController::SUBTRACT_SPECIFIED_FROM_CALCULATED)
         return true;
      }
    }
  }

  return false;
}

} // namespace
