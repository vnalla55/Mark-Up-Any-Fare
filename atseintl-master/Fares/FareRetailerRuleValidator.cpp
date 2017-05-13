//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Common/CarrierUtil.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/EligibilityInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareFocusAccountCdInfo.h"
#include "DBAccess/FareFocusDisplayCatTypeInfo.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/FareFocusCarrierInfo.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/FareFocusLocationPairInfo.h"
#include "DBAccess/FareFocusSecurityInfo.h"
#include "DBAccess/FareFocusSecurityDetailInfo.h"
#include "DBAccess/FareFocusRuleCodeInfo.h"
#include "DBAccess/FareRetailerResultingFareAttrInfo.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag868Collector.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Rules/RuleUtil.h"
#include "Fares/FareRetailerRuleValidator.h"
#include "DBAccess/FareFocusPsgTypeInfo.h"

namespace tse
{

FALLBACK_DECL(fallbackFRRCat25Responsive);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(fallbackFRRmatchTravelRangeX5);
FALLBACK_DECL(fallbackFRRFixProcessingRetailerCodeR9);

Logger
FareRetailerRuleValidator::_logger("atseintl.Fares.FareRetailerRuleValidator");

FareRetailerRuleValidator::FareRetailerRuleValidator(PricingTrx& trx)
  : _trx(trx),
    _adjustedTicketDate(trx.dataHandle().ticketDate()),
    _diag(nullptr)
{
  RuleUtil::setAdjustedTicketDateTime(trx, _adjustedTicketDate);

  createDiagnostic();
}

FareRetailerRuleValidator::~FareRetailerRuleValidator() {}

const Agent*
FareRetailerRuleValidator::getAgent() const
{
  return (_trx.getRequest()->ticketingAgent()); // not rex trx
}

void
FareRetailerRuleValidator::getFRRLookupAllSources(
  std::vector<const FareRetailerRuleLookupInfo*>& frrlV,
  const FareMarket* fm,
  const Code<8>& productCD,
  const Indicator applicationType,
  const bool printFMHdr)
{
  if (UNLIKELY(!matchFMForDiagnostic(fm)))
    return;

  matchApplicationForDiagnostic(applicationType);

  if (printFMHdr)
    printFareMarketHeader(fm);

  getFRRLookupSources(frrlV, getAgent()->tvlAgencyPCC(), productCD, applicationType);

  if (getAgent()->mainTvlAgencyPCC() != getAgent()->tvlAgencyPCC())
  {
    std::vector<const FareRetailerRuleLookupInfo*> homeFrrlV;
    getFRRLookupSources(homeFrrlV, getAgent()->mainTvlAgencyPCC(), productCD, applicationType);
    if (!homeFrrlV.empty())
      mergeFrrls(frrlV, homeFrrlV);
  }

  if (_diag && _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "LOOKUP")
    for (auto f : frrlV)
      _diag->printFareRetailerRuleLookupInfo(f);

  endDiag();
}

namespace
{
  bool compareFRRL(const FareRetailerRuleLookupId& f1, const FareRetailerRuleLookupId& f2)
  {
    return f1._ruleSeqNo < f2._ruleSeqNo;
  }

  bool sameFRRL(const FareRetailerRuleLookupId& f1, const FareRetailerRuleLookupId& f2)
  {
    return f1._ruleSeqNo == f2._ruleSeqNo;
  }
}

FareRetailerRuleLookupInfo*
FareRetailerRuleValidator::mergeRules(const FareRetailerRuleLookupInfo* f1,
                                      const FareRetailerRuleLookupInfo* f2) const
{
  FareRetailerRuleLookupInfo* result(nullptr);
  _trx.dataHandle().get(result);

  *result = *f1;
  size_t ruleSize = result->fareRetailerRuleLookupIds().size();

  result->fareRetailerRuleLookupIds().insert(result->fareRetailerRuleLookupIds().end(),
                                             f2->fareRetailerRuleLookupIds().begin(),
                                             f2->fareRetailerRuleLookupIds().end());

  std::inplace_merge(result->fareRetailerRuleLookupIds().begin(),
                     result->fareRetailerRuleLookupIds().begin() + ruleSize,
                     result->fareRetailerRuleLookupIds().end(),
                     compareFRRL);

  auto last = std::unique(result->fareRetailerRuleLookupIds().begin(),
                          result->fareRetailerRuleLookupIds().end(),
                          sameFRRL);

  result->fareRetailerRuleLookupIds().erase(last, result->fareRetailerRuleLookupIds().end());

  return result;
}

void
FareRetailerRuleValidator::mergeFrrls(
  std::vector<const FareRetailerRuleLookupInfo*>& frrlV1,
  const std::vector<const FareRetailerRuleLookupInfo*>& frrlV2) const
{
  if (frrlV1.empty())
  {
    frrlV1 = std::move(frrlV2);
    return;
  }

  std::vector<const FareRetailerRuleLookupInfo*> newFrrlV;
  std::set<const FareRetailerRuleLookupInfo*> matchesInSecondList;

  for (const FareRetailerRuleLookupInfo* f1: frrlV1)
  {
    bool matchedInFirstList = false;
    for (const FareRetailerRuleLookupInfo* f2 : frrlV2)
    {
      if (f1->sourcePcc() == f2->sourcePcc())
      {
        matchedInFirstList = true;
        matchesInSecondList.insert(f2);

        FareRetailerRuleLookupInfo* merged = mergeRules(f1, f2);
        newFrrlV.push_back(merged);

        break;
      }
    }

    if (!matchedInFirstList)
      newFrrlV.push_back(f1);
  }

  // add the ones from the second list that were not matched
  for (const FareRetailerRuleLookupInfo* f2 : frrlV2)
    if (matchesInSecondList.count(f2) == 0)
      newFrrlV.push_back(f2);

  frrlV1 = std::move(newFrrlV);
}

void
FareRetailerRuleValidator::getFRRLookupSources(
  std::vector<const FareRetailerRuleLookupInfo*>& frrlV,
  const PseudoCityCode& pcc,
  const Code<8>& productCD,
  const Indicator applicationType)
{
  const std::vector<CustomerSecurityHandshakeInfo*>& customerSH =
    getCustomerSecurityHandshake(productCD, pcc);

  if (customerSH.empty())
  {
    if (_diag && _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "LOOKUP")
      printDiagSecurityHShakeNotFound(pcc);

    return;
  }

  for (CustomerSecurityHandshakeInfo* cSHInfo : customerSH)
  {
    if (cSHInfo)
    {
      const FareRetailerRuleLookupInfo* f =
        getFareRetailerRuleLookup(applicationType, cSHInfo->securityTargetPCC(), pcc);
      if (f)
        frrlV.push_back(f);
    }
  }
}

const FareRetailerRuleLookupInfo*
FareRetailerRuleValidator::getFareRetailerRuleLookup(Indicator applicationType,
                                                     const PseudoCityCode& sourcePcc,
                                                     const PseudoCityCode& pcc) const
{
  return _trx.dataHandle().getFareRetailerRuleLookup(applicationType, sourcePcc, pcc);
}

const std::vector<CustomerSecurityHandshakeInfo*>&
FareRetailerRuleValidator::getCustomerSecurityHandshake(const Code<8> productCD,
                                                        const PseudoCityCode& pcc ) const
{
  const DateTime localTimeDT = DateTime::localTime();
  return _trx.dataHandle().getCustomerSecurityHandshake(pcc,
                                                        productCD,
                                                        localTimeDT);
}

const FareRetailerRuleInfo*
FareRetailerRuleValidator::getFareRetailerRule(uint64_t fareRetailerRuleId) const
{
  return _trx.dataHandle().getFareRetailerRule(fareRetailerRuleId, _adjustedTicketDate);
}

const FareRetailerResultingFareAttrInfo*
FareRetailerRuleValidator::getFareRetailerResultingFareAttr(uint64_t resultingFareAttrItemNo) const
{
  return _trx.dataHandle().getFareRetailerResultingFareAttr(resultingFareAttrItemNo,
                                                            _adjustedTicketDate);
}

const std::vector<FareRetailerCalcInfo*>&
FareRetailerRuleValidator::getFareRetailerCalc(uint64_t fareRetailerCalcItemNo) const
{
  return _trx.dataHandle().getFareRetailerCalc(fareRetailerCalcItemNo, _adjustedTicketDate);
}

const FareFocusSecurityInfo*
FareRetailerRuleValidator::getFareFocusSecurity(uint64_t securityItemNo) const
{
  return _trx.dataHandle().getFareFocusSecurity(securityItemNo, _adjustedTicketDate);
}

const std::vector<FareFocusRuleCodeInfo*>&
FareRetailerRuleValidator::getFareFocusRuleCode(uint64_t ruleCdItemNo) const
{
  return _trx.dataHandle().getFareFocusRuleCode(ruleCdItemNo, _adjustedTicketDate);
}

const std::vector<FareFocusFareClassInfo*>&
FareRetailerRuleValidator::getFareFocusFareClass(uint64_t fareClassItemNo) const
{
  return _trx.dataHandle().getFareFocusFareClass(fareClassItemNo, _adjustedTicketDate);
}

bool
FareRetailerRuleValidator::isFareRetailerRuleMatch(const PaxTypeFare& ptf,
                                                   const FareRetailerRuleInfo* frri,
                                                   const FareFocusSecurityInfo* ffsi) const
{
  StatusFRRuleValidation rc = PASS_FR;

  if (!matchSecurity(ffsi))
    rc = FAIL_FR_SECURITY;
  else if (!matchAccountCode(frri->accountCdItemNo(), ptf))
    rc = FAIL_FR_ACCOUNTCD;
  else if (!matchPassengerTypeCode(frri->psgTypeItemNo(), ptf))
    rc = FAIL_FR_PASSENGERTYPECODE;
  else if (!matchVendor(frri->vendor(), ptf.vendor()))
    rc = FAIL_FR_VENDOR;
  else if (!matchRuleTariff(frri->ruleTariff(), ptf.tcrRuleTariff()))
    rc = FAIL_FR_RULE_TARIFF;
  else if (!matchRule(*frri, ptf.ruleNumber()))
    rc = FAIL_FR_RULE;
  else if (!matchCarriers(*frri, ptf))
    rc = FAIL_FR_CARRIER;
  else if (!matchFareType(frri->fareType(), ptf.fcaFareType()))
    rc = FAIL_FR_FARE_TYPE;
  else if (!matchPublicPrivateInd(frri->publicPrivateInd(), ptf.tcrTariffCat()))
    rc = FAIL_FR_PUBLIC_PRIVATE;
  else if (!RuleUtil::matchBookingCode(_trx, frri->bookingCdItemNo(), ptf, _adjustedTicketDate))
    rc = FAIL_FR_BOOKING_CODE;
  else if (!fallback::fallbackFRRmatchTravelRangeX5(&_trx) &&
           !matchTravelRangeX5(_trx, frri->travelDayTimeApplItemNo(), ptf, _adjustedTicketDate))
    rc = FAIL_FR_MATCH_TRAVEL_DATE_RANGE_X5;
  else if (!matchGeo(frri->loc1(), frri->loc2(), ptf, false))
    rc = FAIL_FR_GEO;
  else if (!matchDirectionality(*frri, ptf))
    rc = FAIL_FR_DIRECTIONALITY;
  else if (matchExcludeGeo(frri->locationPairExcludeItemNo(), ptf))
    rc = FAIL_FR_EXCLUDE_GEO;
  else if (!matchFareClass(frri->fareClassItemNo(), ptf))
    rc = FAIL_FR_FARE_CLASS;
  else if (matchFareClass(frri->fareClassExcludeItemNo(), ptf, true))
    rc = FAIL_FR_EXCLUDE_FARE_CLASS;
  else if (!RuleUtil::matchOWRT(frri->owrt(), ptf.owrt()))
    rc = FAIL_FR_MATCH_OWRT;
  else if (!RuleUtil::matchCat35Type(frri->displayCatType(), ptf.fcaDisplayCatType()))
    rc = FAIL_FR_CAT35_DISPLAY_TYPE;
  else if (matchExcludeFareDisplayCatType(frri->displayCatTypeExcludeItemNo(), ptf))
    rc = FAIL_FR_EXCLUDE_DCT;
  else if (!matchRetailerCode(frri->fareRetailerCode()))
    rc = FAIL_FR_MATCH_RETAILERCODE;

  printFareRetailerRule(rc, frri, ffsi);

  endDiag();
  if (rc == PASS_FR)
    return true;

  return false;
}

AccountCode
FareRetailerRuleValidator::getAccountCode(const PaxTypeFare& paxTypeFare) const
{
  // for Cat 25, check record 8
  if (paxTypeFare.isFareByRule() && !paxTypeFare.fbrApp().accountCode().empty())
    return paxTypeFare.fbrApp().accountCode();

  // check category 1
  PaxTypeFareRuleData* ruleData = paxTypeFare.paxTypeFareRuleData(1);
  if (ruleData != nullptr)
  {
    const RuleItemInfo* ruleItemInfo = ruleData->ruleItemInfo();
    if (ruleItemInfo != nullptr)
    {
      const EligibilityInfo* eligibility = dynamic_cast<const EligibilityInfo*>(ruleItemInfo);
      if (!eligibility->acctCode().empty())
        return eligibility->acctCode();
    }
  }

  if (!paxTypeFare.matchedAccCode().empty()) // cat 1 AccCode
    return paxTypeFare.matchedAccCode().c_str();

  return "";
}

bool
FareRetailerRuleValidator::matchExcludeFareDisplayCatType(const uint64_t& displayCatTypeitemNo,
                                                   const PaxTypeFare& ptf) const
{
  if (displayCatTypeitemNo == 0)
    return false;

  const FareFocusDisplayCatTypeInfo* ffDCT = getFareFocusDisplayCatType(displayCatTypeitemNo);

  if (ffDCT != nullptr)
  {
    const std::vector<Indicator>& dctVec = ffDCT->displayCatType();
    for (const Indicator& dctFRR : dctVec)
    {
      if ( dctFRR == ptf.fcaDisplayCatType())
        return true;
    }
  }
  return false;
}


bool
FareRetailerRuleValidator::matchPassengerTypeCode(const uint64_t& psgTypeItemNo,
                                            const PaxTypeFare& paxTypeFare) const
{
  if (psgTypeItemNo == 0)
    return true;

  const FareFocusPsgTypeInfo* ffpti = getFareFocusPsgType(psgTypeItemNo);

  if (ffpti == nullptr)
    return false;

  const PaxTypeCode& paxTypeCodeFromPTF = paxTypeFare.fcasPaxType();

  const std::vector<PaxTypeCode>& ptcV = ffpti->psgType();

  bool isAdult =
      PaxTypeUtil::isAdult(_trx, paxTypeCodeFromPTF, paxTypeFare.fareClassAppInfo()->_vendor);
  bool isChild =
      PaxTypeUtil::isChild(_trx, paxTypeCodeFromPTF, paxTypeFare.fareClassAppInfo()->_vendor);
  bool isInfant =
      PaxTypeUtil::isInfant(_trx,paxTypeCodeFromPTF, paxTypeFare.fareClassAppInfo()->_vendor);

  return std::find_if(ptcV.begin(), ptcV.end(), [&](const auto& item)
                       {return (paxTypeCodeFromPTF == item) ||
                               (paxTypeCodeFromPTF.empty() && (item == ADULT || item.equalToConst("*A"))) ||
                               (isAdult && item.equalToConst("*A")) ||
                               (isChild && item.equalToConst("*C")) ||
                               (isInfant && item.equalToConst("*I")) ;}) != ptcV.end();
}

bool
FareRetailerRuleValidator::matchAccountCode(const uint64_t& accountCdItemNo,
                                            const PaxTypeFare& paxTypeFare) const
{
  if (accountCdItemNo == 0)
    return true;

  AccountCode accountCode = getAccountCode(paxTypeFare);

  if (_diag && ( _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL"))
    _diag->printPaxTypeFareAccountCode(accountCode);

  const std::vector<FareFocusAccountCdDetailInfo*>* ffacdiV =
    getFareFocusAccountCdDetailInfo(accountCdItemNo);

  if (ffacdiV == nullptr || ffacdiV->empty())
    return true;

  for (FareFocusAccountCdDetailInfo* ffacdi : *ffacdiV)
    if (ffacdi->accountCd() == accountCode)
      return true;

  return false;
}

bool
FareRetailerRuleValidator::matchPublicPrivateInd(const Indicator publicPrivateIndrule, 
                                                 const TariffCategory publicIndFare) const 

{
  if (publicPrivateIndrule  == ' ')
     return true;
  else if (publicPrivateIndrule  == 'P')
  {
    return publicIndFare == RuleConst::PUBLIC_TARIFF;
  }
  else if (publicPrivateIndrule  == 'V')
  {
    return publicIndFare == RuleConst::PRIVATE_TARIFF;
  }

  return false;
}


const std::vector<FareFocusAccountCdDetailInfo*>*
FareRetailerRuleValidator::getFareFocusAccountCdDetailInfo(uint64_t accountCdItemNo) const
{
  const FareFocusAccountCdInfo* fareFocusAccountCdInfo =
    _trx.dataHandle().getFareFocusAccountCd(accountCdItemNo, _adjustedTicketDate);

  if (fareFocusAccountCdInfo)
    return &(fareFocusAccountCdInfo->details());

  return nullptr;
}

const FareFocusPsgTypeInfo*
FareRetailerRuleValidator::getFareFocusPsgType(uint64_t psgTypeItemNo) const
{
  return _trx.dataHandle().getFareFocusPsgType(psgTypeItemNo, _adjustedTicketDate);
}

const std::vector<FareFocusCarrierInfo*>&
FareRetailerRuleValidator::getFareFocusCarrier(uint64_t carrierItemNo) const
{
  return _trx.dataHandle().getFareFocusCarrier(carrierItemNo, _adjustedTicketDate);
}

const FareFocusDisplayCatTypeInfo*
FareRetailerRuleValidator::getFareFocusDisplayCatType(uint64_t displayCatTypeitemNo) const
{
  return _trx.dataHandle().getFareFocusDisplayCatType(displayCatTypeitemNo, _adjustedTicketDate);
}

bool
FareRetailerRuleValidator::matchRule(const FareRetailerRuleInfo& frri,
                                     const RuleNumber& ruleNumberFare) const
{
  if (frri.ruleCdItemNo() == 0)
    return true;

  const std::vector<FareFocusRuleCodeInfo*>& ffrciV = getFareFocusRuleCode(frri.ruleCdItemNo());

  if (ffrciV.empty())
    return true;

  for (RuleNumber rn : ffrciV.front()->ruleCd())
    if (rn == ruleNumberFare)
      return true;

  return false;
}

bool
FareRetailerRuleValidator::matchCarriers(const FareRetailerRuleInfo& frri,
                                         const PaxTypeFare& ptf) const
{
  CarrierCode carrierFare = ptf.carrier();
  if (ptf.fare()->isIndustry())
    carrierFare = INDUSTRY_CARRIER;

  const std::vector<FareFocusCarrierInfo*> ruleCarriers = getFareFocusCarrier(frri.carrierItemNo());
  if (ruleCarriers.empty())
    return true;

  for (CarrierCode carrierRule : ruleCarriers.front()->carrier())
  {
    if (!CarrierUtil::isAllianceCode(carrierRule))
    {
      if (carrierRule == carrierFare)
        return true;
    }
    else // rule carrier code is alliance code like *N
    {
      const std::vector<AirlineAllianceCarrierInfo*>& aaci =
        _trx.dataHandle().getAirlineAllianceCarrier(carrierFare);

      if (!aaci.empty() && (aaci.front()->genericAllianceCode() == carrierRule))
        return true;
    }
  }

  return false;
}

bool
FareRetailerRuleValidator::matchRetailerCode(const FareRetailerCode& retailerCode) const
{
  if (fallback::fallbackFRRProcessingRetailerCode(&_trx))
  {
     if (!retailerCode.empty() && !fallback::fallbackFRRFixProcessingRetailerCodeR9(&_trx))
       return false;

     return true;
  }

  PricingRequest* request = _trx.getRequest();

  if (request != nullptr)
    return request->isMatch(retailerCode);

  return false;
}

bool
FareRetailerRuleValidator::matchDirectionality(const FareRetailerRuleInfo& frri,
                                               const PaxTypeFare& paxTypeFare) const
{
  if ((frri.directionality() == RuleConst::BLANK) ||
      (frri.directionality() == DirectionalityBoth) ||
      frri.loc1().isNull())
    return true;

  GeoTravelType geoTravelType = GeoTravelType::International;
  if (paxTypeFare.fareMarket() != nullptr)
    geoTravelType = paxTypeFare.fareMarket()->geoTravelType();

  if (geoTravelType == GeoTravelType::Transborder || geoTravelType == GeoTravelType::Domestic)
  {
    if ((frri.loc1().locType() == LOCTYPE_CITY) ||
        (!frri.loc2().isNull() && (frri.loc2().locType() == LOCTYPE_CITY)))
    {
      if (paxTypeFare.directionality() == BOTH || isValidFareDirectionality(paxTypeFare, frri))
        return true;

      return false;
    }

    return true;
  }

  if (geoTravelType == GeoTravelType::International || geoTravelType == GeoTravelType::ForeignDomestic)
    return isValidFareDirectionality(paxTypeFare, frri);  //DirectionalityFrom

  return false; // just in case..
}

bool
FareRetailerRuleValidator::isValidFareDirectionality(const PaxTypeFare& paxTypeFare,
                                                     const FareRetailerRuleInfo& frri) const
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
    return matchLocation(frri.loc1(), origMarket, paxTypeFare);

  return matchLocation(frri.loc1(), destMarket, paxTypeFare);
}

bool
FareRetailerRuleValidator::matchGeo(const LocKey& loc1,
                                    const LocKey& loc2,
                                    const PaxTypeFare& ptf,
                                    bool matchingExclude) const
{
  if (loc1.isNull() && loc2.isNull())
    return (matchingExclude ? false : true);

  if (!loc1.isNull() && !validGeoType(loc1.locType()))
    return (matchingExclude ? true : false);

  if (!loc2.isNull() && !validGeoType(loc2.locType()))
    return (matchingExclude ? true : false);

  const LocCode& orig = ptf.fare()->market1();
  const LocCode& dest = ptf.fare()->market2();

  if (matchLocation(loc1, orig, ptf) && matchLocation(loc2, dest, ptf))
    return true;

  if (matchLocation(loc2, orig, ptf) && matchLocation(loc1, dest, ptf))
    return true;

  return false;
}

bool
FareRetailerRuleValidator::matchExcludeGeo(const uint64_t& locationPairExcludeItemNo,
                                           const PaxTypeFare& ptf) const
{
  if (locationPairExcludeItemNo == 0)
    return false;

  const std::vector<FareFocusLocationPairDetailInfo*>* locPairs =
    getExcludeLocations(locationPairExcludeItemNo);

  if (locPairs == nullptr)
    return false;

  for (auto fflpdi : *locPairs)
    if (matchGeo(fflpdi->loc1(), fflpdi->loc2(), ptf, true))
      return true;

  return false;
}

const std::vector<FareFocusLocationPairDetailInfo*>*
FareRetailerRuleValidator::getExcludeLocations(const uint64_t& locationPairExcludeItemNo) const
{
  const FareFocusLocationPairInfo* fareFocusLocationPairInfo =
    _trx.dataHandle().getFareFocusLocationPair(locationPairExcludeItemNo, _adjustedTicketDate);

  if (fareFocusLocationPairInfo)
    return &(fareFocusLocationPairInfo->details());

  return nullptr;
}

bool
FareRetailerRuleValidator::matchLocation(const LocKey& loc,
                                         const LocCode& market,
                                         const PaxTypeFare& ptf) const
{
  if (loc.isNull())
    return true;

  return RuleUtil::isInLoc(_trx, market, loc.locType(), loc.loc(), ptf);
}

bool
FareRetailerRuleValidator::validGeoType(const LocTypeCode lType) const
{
   // rule is valid for locType=A,N,Z,C only
   return lType == LOCTYPE_AREA || lType == LOCTYPE_NATION ||
          lType == LOCTYPE_ZONE || lType == LOCTYPE_CITY || lType == GROUP_LOCATION;
}

bool
FareRetailerRuleValidator::matchFareType(const FareTypeAbbrevC& fareTypeRule,
                                         const FareType& fareTypeFare) const
{
  if (fareTypeRule.empty())
    return true;

  return RuleUtil::matchGenericFareType(fareTypeRule.c_str(), fareTypeFare.c_str());
}

bool
FareRetailerRuleValidator::matchVendor(const VendorCode& vendorRule,
                                       const VendorCode& vendorFare) const
{
  if (vendorRule == vendorFare)
    return true;

  if ((vendorRule == DFF_VENDOR_CODE) &&
      (_trx.dataHandle().getVendorType(vendorFare) == RuleConst::SMF_VENDOR))
    return true;

  return false;
}

bool
FareRetailerRuleValidator::matchRuleTariff(const TariffNumber& tariffNumberRule,
                                           const TariffNumber& tariffNumberFare) const
{
  if (tariffNumberRule == 0)
    return true;
  return tariffNumberRule == tariffNumberFare;
}

bool
FareRetailerRuleValidator::validateFRR(const PaxTypeFare& ptFare,
                                       const FareMarket* fm,
                                       const Indicator applicationType,
                                       const std::vector<const FareRetailerRuleLookupInfo*>& frrlV,
                                       std::vector<FareRetailerRuleContext>& frrcV)
{
  if (_diag && UNLIKELY(!matchFMForDiagnostic(fm)))
    return false;

  matchApplicationForDiagnostic(applicationType);

  const std::string& diagRule = _trx.diagnostic().diagParamMapItem(Diagnostic::RULE_NUMBER);
  const std::string& diagFareClass =
    _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);

  if (UNLIKELY( _diag && _diag->diagnosticType() == Diagnostic868 &&
                ((!diagRule.empty() && diagRule != ptFare.ruleNumber()) ||
                (!diagFareClass.empty() && diagFareClass != ptFare.fareClass()))))
    return false;

  if (_diag)
    _diag->printPaxTypeFare(_trx, ptFare);

  for (const FareRetailerRuleLookupInfo* frrli : frrlV)
  {
    if (_diag && (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "INFO" &&
        _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "SHORT"))
      _diag->printFareRetailerRuleLookupHeader(frrli);

    for (const FareRetailerRuleLookupId& frrlId : frrli->fareRetailerRuleLookupIds())
    {
      const FareRetailerRuleInfo* frri = getFareRetailerRule(frrlId._fareRetailerRuleId);
      if (frri &&
          isFareRetailerRuleMatch(ptFare, frri, getFareFocusSecurity(frri->securityItemNo())))
      {
        frrcV.emplace_back(
          frrli->sourcePcc(),
          frri,
          getFareRetailerCalc(frri->fareRetailerCalcItemNo()),
          getFareRetailerResultingFareAttr(frri->resultingFareAttrItemNo()),
          getFareFocusSecurity(frri->securityItemNo()));

        break;
      }
    }
  }

  if (_diag && frrcV.empty() && _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SHORT")
  {
    StatusFRRuleValidation rc = FAIL_FR_ALL;
    _diag->displayStatus(_trx, rc);
    endDiag();
 }

  return !frrcV.empty();
}

bool
FareRetailerRuleValidator::matchSecurity(const FareFocusSecurityInfo* ffsi) const
{
  if (!ffsi)
    return false;

  const PseudoCityCode& branchPCC = getAgent()->tvlAgencyPCC();
  const PseudoCityCode& homePCC = getAgent()->mainTvlAgencyPCC();

  for (FareFocusSecurityDetailInfo* ffsdi : ffsi->details())
  {
    if (ffsdi->pseudoCityType() == RuleConst::TRAVEL_AGENCY)       //type=T
    {
      if (ffsdi->pseudoCity() == branchPCC)
        return true;
    }
    else if (ffsdi->pseudoCityType() == RuleConst::HOME_TRAVEL_AGENCY)  //type=U
    {
      if ((ffsdi->pseudoCity() == homePCC) || (ffsdi->pseudoCity() == branchPCC))
        return true;
    }
  }

  return false;
}

void
FareRetailerRuleValidator::matchApplicationForDiagnostic(const Indicator applicationType)
{
  if (!_diag)
    return;

  const std::string& diagParam = _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_RETAILER_TYPE);
  if (diagParam.empty())
    return;

  bool matchApp = ((diagParam == "NET") && (applicationType == 'N')) ||
                  ((diagParam == "SEL") && (applicationType == 'R')) ||
                  ((diagParam == "ADJ") && (applicationType == 'S'));

  if (!fallback::fallbackFRRCat25Responsive(&_trx))
  {
     matchApp = ((diagParam == "NET") && ((applicationType == 'N') || (applicationType == 'D'))) ||
                ((diagParam == "SEL") && (applicationType == 'R')) ||
                ((diagParam == "ADJ") && (applicationType == 'S'));
  }

  if (!matchApp)
    _diag->deActivate();
  else
    _diag->activate();
}

void
FareRetailerRuleValidator::createDiagnostic()
{
  if (!_trx.diagnostic().isActive() || _trx.diagnostic().diagnosticType() != Diagnostic868)
    return;

  if (!_diag)
  {
    DiagCollector* diagCollector = DCFactory::instance()->create(_trx);
    _diag = dynamic_cast<Diag868Collector*>(diagCollector);
    if (_diag == nullptr)
      return;
    _diag->enable(Diagnostic868);
  }
}

void
FareRetailerRuleValidator::endDiag() const
{
  if (!_diag)
    return;
  _diag->flushMsg();
}

void
FareRetailerRuleValidator::printFareMarketHeader(const FareMarket* fm)
{
  if (_diag && fm)
  {
    _diag->printHeader();
    _diag->printFareMarketHeaderFRR(_trx, *fm);

    _diag->flushMsg();
  }
}

void
FareRetailerRuleValidator::printDiagSecurityHShakeNotFound(const PseudoCityCode& pcc) const
{
  if (_diag)
     _diag->printDiagSecurityHShakeNotFound(pcc);
}

void
FareRetailerRuleValidator::printFareRetailerRule(
   StatusFRRuleValidation rc,
   const FareRetailerRuleInfo* frri,
   const FareFocusSecurityInfo* ffsi) const
{
  if (_diag && _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SHORT"  && rc != PASS_FR)
  {
    return;
  }

  if (_diag && ( _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO" ||
      _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL"))
  {
    bool ddAll = _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL";
    if ( _trx.diagnostic().diagParamMapItem(Diagnostic::IDENTIFICATION).empty())
    {
      _diag->printFareRetailerRuleInfo(_trx, frri, ffsi, ddAll);
      _diag->printFareRetailerRuleStatus(_trx, rc);
    }
    else if (std::to_string(frri->fareRetailerRuleId()) == _trx.diagnostic().diagParamMapItem(Diagnostic::IDENTIFICATION))
    {
      _diag->printFareRetailerRuleInfo(_trx, frri, ffsi, ddAll);
      _diag->printFareRetailerRuleStatus(_trx, rc);
    }
  }
  else if (_diag)
  {
    _diag->printFareRetailerRuleLookupInfo(_trx, frri, rc);
  }
}

bool
FareRetailerRuleValidator::matchFareClass(const uint64_t& fareClassItemNo,
                                          const PaxTypeFare& ptf,
                                          bool matchingExclude /* = false */) const
{
  if (fareClassItemNo == 0)
    return (matchingExclude ? false : true);

  const std::vector<FareFocusFareClassInfo*>& ffFareClassInfoVec =
    getFareFocusFareClass(fareClassItemNo);

  std::string fareBasis = RuleUtil::getFareBasis(_trx, &ptf);

  for (const FareFocusFareClassInfo* ffClassInfo : ffFareClassInfoVec)
  {
    if (!ffClassInfo)
      return (matchingExclude ? true : false);

    for (const FareClassCodeC& fareClassCode : ffClassInfo->fareClass())
    {
      if (RuleUtil::validateMatchExpression(fareClassCode) &&
          RuleUtil::matchFareClassExpression(fareClassCode.c_str(), fareBasis.c_str()))
        return true;
    }
  }

  return false;
}

bool
FareRetailerRuleValidator::matchTravelRangeX5(PricingTrx& trx,
                             uint64_t dayTimeApplItemNo,
                             const PaxTypeFare& ptf,
                             DateTime adjustedTicketDate) const
{
   if (dayTimeApplItemNo == 0)
     return true;

   const FareFocusDaytimeApplInfo* ffdtInfo = nullptr;
   ffdtInfo = trx.dataHandle().getFareFocusDaytimeAppl(dayTimeApplItemNo,adjustedTicketDate);
   if (ffdtInfo == nullptr)
     return false;

   return RuleUtil::matchTravelRangeX5(_trx, ptf, adjustedTicketDate, ffdtInfo);
}

bool
FareRetailerRuleValidator::matchFMForDiagnostic(const FareMarket* fm) const
{
  if (!fm)
    return true;

  if (LIKELY(!_trx.diagnostic().isActive() || _trx.diagnostic().diagnosticType() != Diagnostic868))
    return true;

  const std::string& diagFM = _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_MARKET);
  if (diagFM.empty())
    return true;

  LocCode boardCity = diagFM.substr(0, 3);
  LocCode offCity = diagFM.substr(3, 3);
  if (((fm->origin()->loc() != boardCity) && (fm->boardMultiCity() != boardCity)) ||
      ((fm->destination()->loc() != offCity) && (fm->offMultiCity() != offCity)))
    return false;

  return true;
}

} //tse

