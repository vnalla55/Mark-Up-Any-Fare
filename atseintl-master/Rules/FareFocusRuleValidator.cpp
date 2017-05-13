//-------------------------------------------------------------------
//  Copyright Sabre 2014
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

#include "Rules/FareFocusRuleValidator.h"

#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/FareFocusLookupInfo.h"
#include "DBAccess/FareFocusRuleInfo.h"
#include "DBAccess/ZoneInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag240Collector.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{

FIXEDFALLBACK_DECL(fallbackFFRaddFocusCodeInFFR);

const Indicator FareFocusRuleValidator::DirectionalityFrom = 'F';
const Indicator FareFocusRuleValidator::DirectionalityBoth = 'B';

FareFocusRuleValidator::FareFocusRuleValidator(PricingTrx& trx, FareMarket& fareMarket)
  : _trx(trx),
    _fareMarket(fareMarket),
    _diag(nullptr),
    _diagInfo(false)
  , _adjustedTicketDate(trx.dataHandle().ticketDate())

{
}

FareFocusRuleValidator::~FareFocusRuleValidator() {}

void
FareFocusRuleValidator::process()
{
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
      static_cast<const RexPricingTrx&>(_trx).trxPhase() == RexPricingTrx::REPRICE_EXCITIN_PHASE))
    return;

  RuleUtil::setAdjustedTicketDateTime(_trx, _adjustedTicketDate);

  createDiagnostic();
  if (!isFareFocusApplicable())
  {
    endDiag();
    return;
  }
  if (UNLIKELY(!matchFMForDiagnostic()))
  {
    endDiag();
    return;
  }
  const std::vector<CustomerSecurityHandshakeInfo*>& customerSH = getCustomerSecurityHandshake();
  if (customerSH.empty())
  {
    printDiagSecurityHShakeNotFound();
    endDiag();
    return;
  }

  std::vector<const FareFocusRuleInfo*>  ffRulesV;
  getFareFocusRules(customerSH, ffRulesV);
  if (ffRulesV.empty())
  {
    printDiagFareFocusRulesNotFound();
    endDiag();
    return;
  }
  printFareMarketHeader();

  std::vector<PaxTypeFare*>::iterator i = _fareMarket.allPaxTypeFare().begin();
  while (i != _fareMarket.allPaxTypeFare().end())
  {
    bool eraseFare = false;
    PaxTypeFare* ptf = *i;

    if (!ptf->isValid())
    {
      ++i;
      continue;
    }

    if (!matchPTFareForDiagnostic(*ptf))
    {
      ++i;
      continue;
    }

    printPaxTypeFare(*ptf);
    if (isFareFocusRuleMatched(*ptf, ffRulesV))
    {
      ptf->setMatchFareFocusRule(true);
      if (_trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX)
        ptf->invalidateFare(PaxTypeFare::FD_Matched_Fare_Focus);
      else if (_trx.getTrxType() == PricingTrx::FF_TRX)
        eraseFare = true;
    }

    if (eraseFare)
      i = _fareMarket.allPaxTypeFare().erase(i);
    else
      ++i;
  }

  endDiag();
  return;
}

bool 
FareFocusRuleValidator::isFareFocusRuleMatched(const PaxTypeFare& ptf, const std::vector<const FareFocusRuleInfo*>& ffRulesV) const
{
  StatusFFRuleValidation rc = PASS_FF;
  for (const FareFocusRuleInfo* ffri : ffRulesV)
  {
    rc = PASS_FF;
    if (!matchVendor(ffri->vendor(), ptf.vendor()))
      rc = FAIL_FF_VENDOR;
    else if (!matchRule(ffri->ruleCode(), ptf.ruleNumber()))
      rc = FAIL_FF_RULE;
    else if (!matchRuleTariff(ffri->ruleTariff(),ptf.tcrRuleTariff()))
      rc = FAIL_FF_RULE_TARIFF;
    else if (!matchPublicInd(ffri->publicPrivateIndicator(), ptf.tcrTariffCat()))
      rc = FAIL_FF_PUBLIC_IND;
    else if (!matchFareType(ffri->fareType(),ptf.fcaFareType()))
      rc = FAIL_FF_FARE_TYPE;
    else if (!matchCarrier(ffri->carrier(), ptf))
      rc = FAIL_FF_CARRIER;
    else if (!matchOWRT(ffri->owrt(), ptf.owrt()))
      rc = FAIL_FF_OWRT;
    else if (!RuleUtil::matchCat35Type(ffri->displayType(), ptf.fcaDisplayCatType()))
      rc = FAIL_FF_DCT;
    else if (!RuleUtil::matchBookingCode(_trx, ffri->bookingCodeItemNo(), ptf, _adjustedTicketDate))
      rc = FAIL_FF_BOOKING_CODE;
    else if (!matchFareClass(ffri->fareClassItemNo(), ptf))
      rc = FAIL_FF_FARE_CLASS;
    else if (!matchTravelRangeX5(_trx, ffri->travelDayTimeApplItemNo(), ptf, _adjustedTicketDate))
      rc = FAIL_FF_MATCH_TRAVEL_DATE_RANGE_X5;
    else if (!matchGeo(*ffri, ptf))
      rc = FAIL_FF_GEO;
    else if (!matchDirectionality(*ffri, ptf))
      rc = FAIL_FF_DIRECTIONALITY;

    printFareFocusRuleProcess(ffri, rc);
    if (rc == PASS_FF)
      break;
  }

  if (rc == PASS_FF)
    return true;

  printFareFocusRuleNoneFound();
  return false;
}

bool
FareFocusRuleValidator::matchGeo(const FareFocusRuleInfo& ffri,
                                 const PaxTypeFare& paxTypeFare) const
{
  // mandatory fields
  if (!validGeoType(ffri))
    return false;
  const LocCode& origMarket = paxTypeFare.fare()->market1();
  const LocCode& destMarket = paxTypeFare.fare()->market2();
  if ((RuleUtil::isInLoc(_trx, origMarket,ffri.loc1().locType(), ffri.loc1().loc(), paxTypeFare) &&
       RuleUtil::isInLoc(_trx, destMarket,ffri.loc2().locType(), ffri.loc2().loc(), paxTypeFare))
      ||
      (RuleUtil::isInLoc(_trx, destMarket,ffri.loc1().locType(), ffri.loc1().loc(), paxTypeFare) &&
       RuleUtil::isInLoc(_trx, origMarket,ffri.loc2().locType(), ffri.loc2().loc(), paxTypeFare)))
    return true;
  return false;
}

bool
FareFocusRuleValidator::validGeoType(const FareFocusRuleInfo& ffri) const
{
  if (ffri.loc1().isNull() ||
      ffri.loc2().isNull())
    return false;

  // rule is valid for locType=A,N,Z,C only
  const LocTypeCode& lType1 = ffri.loc1().locType();
  const LocTypeCode& lType2 = ffri.loc2().locType();

  return ((lType1 == LOCTYPE_AREA || lType1 == LOCTYPE_NATION ||
           lType1 == LOCTYPE_ZONE || lType1 == LOCTYPE_CITY || lType1 == GROUP_LOCATION)
          &&
          (lType2 == LOCTYPE_AREA || lType2 == LOCTYPE_NATION ||
           lType2 == LOCTYPE_ZONE || lType2 == LOCTYPE_CITY || lType2 == GROUP_LOCATION));
}

bool
FareFocusRuleValidator::matchDirectionality(const FareFocusRuleInfo& ffri,
                                            const PaxTypeFare& paxTypeFare) const
{
  GeoTravelType geoTravelType = GeoTravelType::International;
  if (paxTypeFare.fareMarket() != nullptr)
    geoTravelType = paxTypeFare.fareMarket()->geoTravelType();

  if (geoTravelType == GeoTravelType::Transborder || geoTravelType == GeoTravelType::Domestic)
  {
    if (ffri.directionality() == DirectionalityBoth)
      return true;
    if (ffri.loc1().locType() == LOCTYPE_CITY || ffri.loc2().locType() == LOCTYPE_CITY) // DirectionalityFrom
    {
      if (paxTypeFare.directionality() == BOTH || isValidFareDirectionality(paxTypeFare, ffri))
        return true;
      return false;
    }
    return true;
  }
  else if (geoTravelType == GeoTravelType::International || geoTravelType == GeoTravelType::ForeignDomestic)
  {
    if (ffri.directionality() == DirectionalityBoth)
      return true;
    return isValidFareDirectionality(paxTypeFare, ffri);  //DirectionalityFrom
  }
  return false; // just in case..
}

bool
FareFocusRuleValidator::isValidFareDirectionality(const PaxTypeFare& paxTypeFare,
                                                  const FareFocusRuleInfo& ffri) const
{
  const LocCode& origMarket = paxTypeFare.fare()->market1(); // not swapped origin
  const LocCode& destMarket = paxTypeFare.fare()->market2(); // not swapped dest
  Directionality dir = paxTypeFare.directionality();         // TO/FROM 

  if (paxTypeFare.isReversed())
  {
    if (dir == FROM)
      dir = TO;
    else
      dir = FROM;
  }
  if (dir == FROM)
    return RuleUtil::isInLoc(_trx, origMarket,ffri.loc1().locType(), ffri.loc1().loc(), paxTypeFare);
  else
    return RuleUtil::isInLoc(_trx, destMarket,ffri.loc1().locType(), ffri.loc1().loc(), paxTypeFare);
}

void
FareFocusRuleValidator::printFareFocusRuleProcess(const FareFocusRuleInfo* ffri,
                                                  StatusFFRuleValidation rc) const
{
  if (UNLIKELY(_diag))
  {
    if (_trx.diagnostic().diagParamMapItem(Diagnostic::FF_RULE_ID).empty()
        ||
        isDiagRuleNumberMatch(ffri))
    {
      if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO")
      {
        _diag->printFareFocusRuleInfo(ffri);
        _diag->printFareFocusRuleStatus(rc);
      }
      else if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SHORT")
      {
        if (rc == PASS_FF)
          _diag->printFareFocusLookup(ffri->fareFocusRuleId(), rc);
      }
      else
      {
        _diag->printFareFocusLookup(ffri->fareFocusRuleId(), rc);
      }
    }
  }
}

bool
FareFocusRuleValidator::isDiagRuleNumberMatch(const FareFocusRuleInfo* ffri) const
{
  const std::string& diagID = _trx.diagnostic().diagParamMapItem(Diagnostic::FF_RULE_ID);
  uint64_t diagRuleNumber = std::atoi(diagID.c_str());
  if (diagRuleNumber == ffri->fareFocusRuleId())
    return true;
  return false;
}

bool
FareFocusRuleValidator::matchTravelRangeX5(PricingTrx& trx,
                                           uint64_t dayTimeApplItemNo,
                                           const PaxTypeFare& ptf,
                                           DateTime adjustedTicketDate) const
{
  if (fallback::fixed::fallbackFFRaddFocusCodeInFFR())
	return true;

  if (dayTimeApplItemNo == 0)
    return true;

  const FareFocusDaytimeApplInfo* ffdtInfo = nullptr;
  ffdtInfo = trx.dataHandle().getFareFocusDaytimeAppl(dayTimeApplItemNo,adjustedTicketDate);
  if (ffdtInfo == nullptr)
    return false;

  return RuleUtil::matchTravelRangeX5(_trx, ptf, adjustedTicketDate, ffdtInfo);
}

void
FareFocusRuleValidator::getFareFocusRules(const std::vector<CustomerSecurityHandshakeInfo*>& cSH,
                                          std::vector<const FareFocusRuleInfo*>& ffriV)
{
  StatusFFRuleValidation rc = PASS_FF;
  const FareFocusLookupInfo* ffLU = _trx.dataHandle().getFareFocusLookup(_trx.getRequest()->ticketingAgent()->tvlAgencyPCC());
  if (!ffLU || ffLU->fareFocusRuleIds().empty())
    rc = FAIL_FF_LOOKUP_EMPTY;
  else
  {
    for (uint64_t fareFocusRuleId : ffLU->fareFocusRuleIds())
    {
      rc = PASS_FF;
      const FareFocusRuleInfo* ffri = _trx.dataHandle().getFareFocusRule(fareFocusRuleId, _adjustedTicketDate);
      if (ffri && ffri->securityItemNo())
      {
        const FareFocusSecurityInfo* ffS = _trx.dataHandle().getFareFocusSecurity(ffri->securityItemNo(), _adjustedTicketDate);
        if (ffS)
        {
          if (validateSecurityHandshake(*ffri, cSH))
          {
            ffriV.push_back(ffri);
            std::vector<std::string> result;
            uint64_t itemNo = ffri->fareClassItemNo();
            if (itemNo != 0)
              getFareClassMatchExpressions(itemNo, result);
            if (!result.empty())
              _matchExpressions[itemNo] = result;
          }
          else 
            rc = FAIL_FF_RULES_SECURITY_HANDSHAKE;
        }
        else
          rc = FAIL_FF_SECURITY;
        printFareFocusLookup(fareFocusRuleId, rc);
      }
    }
    if (ffriV.empty())
      rc = FAIL_FF_RULES_EMPTY;
  }
  if (rc == FAIL_FF_RULES_EMPTY || rc ==  FAIL_FF_LOOKUP_EMPTY)
    printFareFocusNoData(rc);
  return;
}

void
FareFocusRuleValidator::printFareFocusNoData(const StatusFFRuleValidation rc) const
{
  if (UNLIKELY(_diag))
  {
    if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "LOOKUP")
      _diag->printFareFocusLookup(rc);
  }
}

void
FareFocusRuleValidator::printFareFocusLookup(const uint64_t fareFocusRuleId, const StatusFFRuleValidation rc) const
{
  if (UNLIKELY(_diag))
  {
    if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "LOOKUP")
      _diag->printFareFocusLookup(fareFocusRuleId, rc);
  }
}

bool
FareFocusRuleValidator::validateSecurityHandshake(const FareFocusRuleInfo& ffri,
                                                 const std::vector<CustomerSecurityHandshakeInfo*>& cSH) const
{
  for (CustomerSecurityHandshakeInfo* cSHInfo : cSH)
   {
     if (cSHInfo && cSHInfo->securityTargetPCC() == ffri.sourcePCC())
       return true;
   }
   return false;
}

void
FareFocusRuleValidator::printDiagFareFocusRulesNotFound() const
{
  if (UNLIKELY(_diag))
    _diag->printDiagFareFocusRulesNotFound();
}

void
FareFocusRuleValidator::printFareFocusRuleNoneFound() const
{
  if (UNLIKELY(_diag))
    if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SHORT")
      _diag->printFareFocusRuleNoneFound();
}

const std::vector<CustomerSecurityHandshakeInfo*>&
FareFocusRuleValidator::getCustomerSecurityHandshake() const
{
  const DateTime localTimeDT = DateTime::localTime();
  Code<8> productCD = "FF";
  return _trx.dataHandle().getCustomerSecurityHandshake(_trx.getRequest()->ticketingAgent()->tvlAgencyPCC(),
                                                        productCD,
                                                        localTimeDT);
}

void
FareFocusRuleValidator::printDiagSecurityHShakeNotFound() const
{
  if (UNLIKELY(_diag))
     _diag->printDiagSecurityHShakeNotFound();
}

void
FareFocusRuleValidator::printPaxTypeFare(const PaxTypeFare& paxTypeFare)
{
  if (UNLIKELY(_diag))
    _diag->printPaxTypeFare(_trx, paxTypeFare);
}

void
FareFocusRuleValidator::createDiagnostic()
{
  if (LIKELY(!_trx.diagnostic().isActive() || _trx.diagnostic().diagnosticType() != Diagnostic240))
    return;

  if (!_diag)
  {
    DiagCollector* diagCollector = DCFactory::instance()->create(_trx);
    _diag = dynamic_cast<Diag240Collector*>(diagCollector);
    if (_diag == nullptr)
      return;
    _diag->enable(Diagnostic240);
  }
}
void
FareFocusRuleValidator::printFareMarketHeader()
{
  if (UNLIKELY(_diag))
  {
    _diag->printHeader();
    _diag->printFareMarketHeaderFFR(_trx, _fareMarket);
  }
}

bool
FareFocusRuleValidator::matchFMForDiagnostic()
{
  if (UNLIKELY(_diag))
  {
    const std::string& diagFM = _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_MARKET);
    if (!diagFM.empty())
    {
      LocCode boardCity = diagFM.substr(0, 3);
      LocCode offCity = diagFM.substr(3, 3);
      if (((_fareMarket.origin()->loc() != boardCity) && (_fareMarket.boardMultiCity() != boardCity)) ||
          ((_fareMarket.destination()->loc() != offCity) && (_fareMarket.offMultiCity() != offCity)))
        return false;
    }
  }
  return true;
}

void
FareFocusRuleValidator::endDiag() const
{
  if (LIKELY(!_diag))
    return;
  _diag->flushMsg();
}

bool
FareFocusRuleValidator::printDiagAndReturn(const std::string& msg) const
{
  if (UNLIKELY(_diag))
    *_diag << " \nNO FARE FOCUS PROCESSING " << msg << "\n \n";

  return false;
}

bool
FareFocusRuleValidator::isFareFocusApplicable() const
{
  if (UNLIKELY(_trx.getOptions()->isExcludeFareFocusRule()))
    return printDiagAndReturn("- XFF IS REQUESTED");

  // JAL/Axess or airline partitions are not eligible 
  if (UNLIKELY(_trx.getRequest()->ticketingAgent()->axessUser()) )
    return printDiagAndReturn("FOR JAL/AXESS");
 
  if (!_trx.billing()->partitionID().empty() && 
      _trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
      (_trx.billing()->aaaCity().size() < 4))
  {
    return printDiagAndReturn("FOR AIRLINE PARTITIONS");
  }

  // Round the world trx is not eligible
  if (UNLIKELY(_trx.getOptions()->isRtw()))
    return printDiagAndReturn("FOR ROUND THE WORLD TRANSACTIONS");

  return true;
}

bool
FareFocusRuleValidator::matchPTFareForDiagnostic(const PaxTypeFare& paxTypeFare)
{
  if (UNLIKELY(_diag))
  {
    const std::string& diagFC = _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
    const std::string& diagRU = _trx.diagnostic().diagParamMapItem(Diagnostic::RULE_NUMBER);
    if ((diagFC.empty() ||
         RuleUtil::matchFareClass(diagFC.c_str(), paxTypeFare.fareClass().c_str()))
        &&
       (diagRU.empty() || diagRU == paxTypeFare.ruleNumber()) )
    {
      return true;
    }
    return false; // skip PTF for diagnostic
  }
  return true;
}

bool
FareFocusRuleValidator::matchVendor(const VendorCode& vendorRule, const VendorCode& vendorFare) const
{
  // mandatory field
  return vendorRule == vendorFare;
}

bool
FareFocusRuleValidator::matchOWRT(const Indicator owrtFromFareFocusRule, const Indicator owrtFromFare) const
{
  if (owrtFromFareFocusRule == ' ')
    return true;
  return owrtFromFareFocusRule == owrtFromFare;
}

bool
FareFocusRuleValidator::matchRule(const RuleNumber& ruleNumberRule, const RuleNumber& ruleNumberFare) const
{
  if (ruleNumberRule.empty())
    return true;
  return ruleNumberRule == ruleNumberFare;
}

bool
FareFocusRuleValidator::matchRuleTariff(const TariffNumber& tariffNumberRule,
                                        const TariffNumber& tariffNumberFare) const
{
  if (tariffNumberRule == 0)
    return true;
  return tariffNumberRule == tariffNumberFare;
}

bool
FareFocusRuleValidator::matchFareType(const FareTypeAbbrevC& fareTypeRule,
                                      const FareType& fareTypeFare) const
{
  if (fareTypeRule.empty())
    return true;

  return RuleUtil::matchGenericFareType(fareTypeRule.c_str(), fareTypeFare.c_str());
}

bool
FareFocusRuleValidator::matchCarrier(const CarrierCode& carrierRule,
                                     const PaxTypeFare& ptf) const
{
  // mandatory field
  CarrierCode carrierFare = ptf.carrier();
  if (ptf.fare()->isIndustry())
    carrierFare = INDUSTRY_CARRIER;
  return carrierRule == carrierFare;
}

bool
FareFocusRuleValidator::matchPublicInd(const Indicator publicIndRule,
                                       const TariffCategory publicIndFare) const
{
  if (publicIndRule == ' ')
    return true;
  return publicIndFare != RuleConst::PRIVATE_TARIFF;
}

bool
FareFocusRuleValidator::matchFareClass(const uint64_t& fareClassItemNo,
                                       const PaxTypeFare& ptf) const
{
  if (fareClassItemNo == 0)
    return true;

  MatchExpressionMap::const_iterator it;
  it = _matchExpressions.find(fareClassItemNo);
  if(it == _matchExpressions.end() || it->second.empty())
    return false;

  std::string fareBasis = RuleUtil::getFareBasis(_trx, &ptf);

  for (const std::string& matchExpression : it->second)
    if (RuleUtil::matchFareClassExpression(matchExpression.c_str(), fareBasis.c_str()))
      return true;

  return false;
}

bool
FareFocusRuleValidator::getFareClassMatchExpressions(const uint64_t& fareClassItemNo,
                                                     std::vector<std::string>& result) const
{
  const std::vector<FareFocusFareClassInfo*>& ffFareClassInfoVec =
    _trx.dataHandle().getFareFocusFareClass(fareClassItemNo, _adjustedTicketDate);

  if (ffFareClassInfoVec.empty())
    return false;

  for (const FareFocusFareClassInfo* ffClassInfo : ffFareClassInfoVec)
  {
    if (ffClassInfo == nullptr)
      return false;

    const std::vector<FareClassCodeC>& fareClassVec = ffClassInfo->fareClass();
    for (const FareClassCodeC& fareClassCode : fareClassVec)
    {
      if (RuleUtil::validateMatchExpression(fareClassCode))
        result.push_back(fareClassCode);
    }
  }

  return true;
}

} //tse
