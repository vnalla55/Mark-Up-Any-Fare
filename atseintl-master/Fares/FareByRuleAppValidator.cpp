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
#include "Fares/FareByRuleAppValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/MarkupSecFilter.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag208Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/FareByRuleProcessingInfo.h"
#include "Rules/RuleUtil.h"
#include "Rules/SalesRestrictionRule.h"

#include <iostream>
#include <vector>

namespace tse
{
FALLBACK_DECL(dffOaFareCreation);
using namespace std;
const char FareByRuleAppValidator::FROM_LOC_1 = 'F';
const char FareByRuleAppValidator::TO_LOC_1 = 'T';
const char FareByRuleAppValidator::TEST_RULE = 'X';
const char FareByRuleAppValidator::TEST_RULE_1 = '1';
const char FareByRuleAppValidator::TEST_RULE_S = 'S';

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleAppValidator::isValid
//
// Description:  This function validates Record 8 - Fare By Rule Application
//               against the passenger/itinerary information.
//
// @param  trx           Transaction info
// @param  fareMarket    A reference to the FareMarket object.
//
// @return bool
//        -true, if Record 8 is matched.
//        -false otherwise.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
FareByRuleAppValidator::isValid(FareByRuleProcessingInfo& fbrProcessingInfo,
                                std::map<std::string, bool>& ruleTariffMap)
{
  FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();
  PricingTrx& trx = *fbrProcessingInfo.trx();
  FareMarket& fareMarket = *fbrProcessingInfo.fareMarket();

  if (UNLIKELY(fbrProcessingInfo.isSpanishDiscountApplied() &&
      PaxTypeUtil::isSpanishPaxType(fbrApp.primePaxType())))
  {
    fbrProcessingInfo.isSpanishPassengerType() = true;
  }

  //----------------------------------------------------------------------------
  // validate passenger status
  //----------------------------------------------------------------------------
  if (!matchPassengerStatus(fbrApp, trx, fbrProcessingInfo))
  {
    doDiag208Collector(fbrApp, trx, Diag208Collector::FAIL_PAX_STATUS, fareMarket);
    return false;
  }

  //----------------------------------------------------------------------------
  // validate global indicator
  //----------------------------------------------------------------------------
  if (isUsCaRuleTariff(fbrApp))
  {
    fbrProcessingInfo.isUsCaRuleTariff() = true;
  }

  if (!fbrProcessingInfo.isUsCaRuleTariff() && fbrApp.globalDir() != GlobalDirection::ZZ &&
      fareMarket.getGlobalDirection() != GlobalDirection::ZZ &&
      fbrApp.globalDir() != fareMarket.getGlobalDirection() &&
      !fbrProcessingInfo.isUsCaRuleTariff())
  {
    doDiag208Collector(fbrApp, trx, Diag208Collector::FAIL_GLOBAL, fareMarket);
    return false;
  }

  //----------------------------------------------------------------------------
  // validate directional indicator and fare geographic scope
  //----------------------------------------------------------------------------
  if (!matchLocation(trx, fbrApp, fareMarket, fbrProcessingInfo))
  {
    doDiag208Collector(fbrApp, trx, Diag208Collector::FAIL_GEO, fareMarket);
    return false;
  }

  //----------------------------------------------------------------------------
  // validate wholly within location
  //----------------------------------------------------------------------------
  if (UNLIKELY(fbrApp.whollyWithinLoc().locType() !=
               BLANK && // TODO: ensure that itinIndex is set correctly in case of Fare Led MIP
               !matchWhollyWithin(fbrApp, fareMarket)))
  {
    doDiag208Collector(fbrApp, trx, Diag208Collector::FAIL_LOC3, fareMarket);
    return false;
  }

  //----------------------------------------------------------------------------
  // validate carrier restrictions
  //    sameCarrier or need to match table986, or no restriction
  //----------------------------------------------------------------------------
  if (!(trx.getTrxType() == PricingTrx::MIP_TRX || trx.getTrxType() == PricingTrx::IS_TRX ||
        trx.getTrxType() == PricingTrx::FF_TRX || trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
  {
    if (fbrApp.sameCarrier() != BLANK)
    {
      if (!RuleUtil::useSameCarrier(trx.travelSeg()))
      {
        doDiag208Collector(fbrApp, trx, Diag208Collector::FAIL_SAME_CARRIER, fareMarket);
        return false;
      }
    }
    else if (fbrApp.carrierFltTblItemNo() != 0)
    {
      if (!RuleUtil::useCxrInCxrFltTbl(
              trx.travelSeg(), fbrApp.vendor(), fbrApp.carrierFltTblItemNo(), trx.ticketingDate()))
      {
        doDiag208Collector(fbrApp, trx, Diag208Collector::FAIL_CXRFLT_TBL, fareMarket);
        return false;
      }
    }
  }

  //----------------------------------------------------------------------------
  // validate rule tariff in Tariff Cross Reference table
  //----------------------------------------------------------------------------
  if (!matchTariffXRef(fbrProcessingInfo, ruleTariffMap))
  {
    return false;
  }

  //----------------------------------------------------------------------------
  // check upfront security
  //----------------------------------------------------------------------------
  TariffCrossRefInfo& tcrInfo = *fbrProcessingInfo.tcrInfo();

  bool isFdTrx = fbrProcessingInfo.isFdTrx();

  if (tcrInfo.tariffCat() == RuleConst::PRIVATE_TARIFF &&
      !checkUpFrontSecurity(fbrApp, trx, fareMarket, tcrInfo, isFdTrx, fbrProcessingInfo))
  {
    doDiag208Collector(fbrApp, trx, Diag208Collector::FAIL_SECURITY, fareMarket);
    return false;
  }
  // Remove isVendorPCC with dffOaFareCreation fallback
  fbrProcessingInfo.isFbrAppVendorPCC() = fallback::dffOaFareCreation(&trx)
                                              ? isVendorPCC(fbrApp.vendor(), trx)
                                              : RuleUtil::isVendorPCC(fbrApp.vendor(), trx);
  doDiag208Collector(fbrApp, trx, Diag208Collector::R8_PASS, fareMarket);

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleAppValidator::matchTariffXRef
//
// Description:  To match the ruletariff and ruleno in Record 8 against the
//               ruletariff and ruletariffcode in the TariffCrossRef table.
//               Record 8 does not apply if the fare by rule ruletariff is
//               inhibit in TariffCrossRef or the locations of fare being created
//               are not in the fare by rule zoneno in TariffCrossRef.
//               The tariffCat in TariffCrossRef will define whether the
//               fare by rule being created is a private or public fare.
//
// @param  fbrApp        A reference to the FareByRuleApp object

// @param  trx           Transaction info
// @param  fareMarket    A reference to the FareMarket object.
//
// @return bool
//        -true, if Record 8 is matched.
//        -false otherwise.
//
// </PRE>

// ----------------------------------------------------------------------------
bool
FareByRuleAppValidator::matchTariffXRef(FareByRuleProcessingInfo& fbrProcessingInfo,
                                        std::map<std::string, bool>& ruleTariffMap) const
{
  FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();
  PricingTrx& trx = *fbrProcessingInfo.trx();
  Itin& itin = *fbrProcessingInfo.itin();
  FareMarket& fareMarket = *fbrProcessingInfo.fareMarket();

  GeoTravelType geoTravelType = fareMarket.geoTravelType();
  RecordScope recordScope = DOMESTIC;

  if (geoTravelType == GeoTravelType::International || geoTravelType == GeoTravelType::ForeignDomestic)
  {
    recordScope = INTERNATIONAL;
  }

  const vector<TariffCrossRefInfo*>& tcrList = trx.dataHandle().getTariffXRefByRuleTariff(
      fbrApp.vendor(), fbrApp.carrier(), recordScope, fbrApp.ruleTariff(), itin.travelDate());

  if (tcrList.size() == 0)
  {
    doDiag208Collector(
        fbrApp, trx, Diag208Collector::TRXREF_NOT_FOUND, *(fbrProcessingInfo.fareMarket()));
    return false;
  }

  vector<TariffCrossRefInfo*>::const_iterator tcr = tcrList.begin();

  //------------------------------------------------------------------------
  // The ruletariff public/private indicator and its Geo location are in
  // the TariffCrossRef table.
  // For Private Non-Carrier Tariffs, we must apply the logic to match the
  // Rule in Record 8 and the RuleTariffCode in TariffCrossRef table.
  // For EX: Rule 'XX21' (770) will match to RuleTariffCode 'FXX----' (770)
  //         Rule '4353' (770) will match to RuleTariffCode 'F43----' (770)
  // For other Cat 25 rule tariffs, do not apply that special logic.
  // For EX: Rule 'XX09' (191) will match to ruletariff 191, ruletariffcode could be 'FBRNAPV'.
  //         Rule 'NA71' (191) will match to ruletariff 191, ruletariffcode could be 'FBRNAPV'.
  //------------------------------------------------------------------------

  // For test rules such as 'XX20'
  if (UNLIKELY(fbrApp.ruleNo()[0] == TEST_RULE_1 && fbrApp.ruleNo()[1] == TEST_RULE_S))
  {
    // new logic for test tariff
    tcr = std::find_if(tcrList.begin(), tcrList.end(), IsTestRuleTariff());
  }
  else if (fbrApp.ruleNo()[0] == TEST_RULE && fbrApp.ruleNo()[1] == TEST_RULE)
  {
    // should be removed in future
    for (; tcr != tcrList.end(); tcr++)
    {
      // match Rule Tariff Code
      if (fbrApp.ruleNo()[0] == (*tcr)->ruleTariffCode()[1] &&
          fbrApp.ruleNo()[1] == (*tcr)->ruleTariffCode()[2])
      {
        break;
      }
    }
  }

  // For real rules such as 'XXBR' (no ruletariffcode FXX---- filed)
  if (UNLIKELY(tcr == tcrList.end()))
  {
    tcr = tcrList.begin();
  }

  if ((trx.getOptions()->isPublishedFares() && (*tcr)->tariffCat() == RuleConst::PRIVATE_TARIFF) ||
      (trx.getOptions()->isPrivateFares() && (*tcr)->tariffCat() != RuleConst::PRIVATE_TARIFF))
  {
    doDiag208Collector(
        fbrApp, trx, Diag208Collector::FAIL_PL_PV_TARIFF, *(fbrProcessingInfo.fareMarket()));
    return false;
  }

  //------------------------------------------------------------------------
  // Check the inhibit indicator
  //------------------------------------------------------------------------
  // retrieve a vector of TariffCrossRef candidates
  const Indicator tariffInhibit =
      trx.dataHandle().getTariffInhibit((*tcr)->vendor(),
                                        ((*tcr)->crossRefType() == INTERNATIONAL ? 'I' : 'D'),
                                        (*tcr)->carrier(),
                                        (*tcr)->fareTariff(),
                                        (*tcr)->ruleTariffCode());

  if (UNLIKELY(tariffInhibit == RuleConst::INHIBIT_FAIL))
  {
    doDiag208Collector(
        fbrApp, trx, Diag208Collector::TARIFF_INHIBIT, *(fbrProcessingInfo.fareMarket()));
    return false;
  }

  fbrProcessingInfo.tcrInfo() = *tcr;

  char buffer[32] = "";
  sprintf(buffer, "%s%s%d", fbrApp.vendor().c_str(), fbrApp.carrier().c_str(), fbrApp.ruleTariff());
  std::string rec8RuleTariff(buffer);
  map<std::string, bool>::iterator ruleTariffIter = ruleTariffMap.find(rec8RuleTariff);

  if (ruleTariffIter != ruleTariffMap.end())
  {
    // bool isRuleTariffValid = ruleTariffMap[fbrApp.ruleTariff()];
    bool isRuleTariffValid = ruleTariffIter->second;

    if (!isRuleTariffValid)
    {
      doDiag208Collector(
          fbrApp, trx, Diag208Collector::FAIL_TRXREF, *(fbrProcessingInfo.fareMarket()));
    }
    return isRuleTariffValid;
  }

  //------------------------------------------------------------------------
  // Check geo of rule tariff
  //------------------------------------------------------------------------
  if ((*tcr)->zoneNo().empty() || !(LocUtil::isInZone(*(fareMarket.origin()),
                                                      *(fareMarket.destination()),
                                                      (*tcr)->zoneVendor(),
                                                      (*tcr)->zoneNo(),
                                                      (*tcr)->zoneType())))
  {
    ruleTariffMap.insert(pair<std::string, bool>(rec8RuleTariff, false));
    doDiag208Collector(
        fbrApp, trx, Diag208Collector::FAIL_TRXREF, *(fbrProcessingInfo.fareMarket()));
    return false;
  }

  ruleTariffMap.insert(pair<std::string, bool>(rec8RuleTariff, true));

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleApp::matchPassengerStatus
//
// Description:  To match the passenger information against the passenger
//               status information. If the information matches and the
//               application tag (negPaxStatusInd) is set to 'N', then the
//               Record 8 does not apply. If the information matches and the
//               application tag is blank, the Record 8 does apply.
//
// @param  fbrApp        A pointer to the FareByRuleApp object
// @param  options       Options info
//
// @return bool
//        -true, if Record 8 is matched.
//        -false otherwise.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
FareByRuleAppValidator::matchPassengerStatus(FareByRuleApp& fbrApp,
                                             PricingTrx& trx,
                                             FareByRuleProcessingInfo& fbrProcessingInfo) const
{
  Indicator paxInd = fbrApp.paxInd();

  if (paxInd == BLANK)
    return true;

  StateCode stateCode("");
  fbrProcessingInfo.stateCodeFromFirstRequested(stateCode);

  GeoTravelType geoTvlType = fbrProcessingInfo.fareMarket()->geoTravelType();
  
  if (LocUtil::matchPaxStatus(fbrApp.paxLoc(),
                              fbrApp.vendor(),
                              paxInd,
                              fbrApp.negPaxStatusInd(),
                              stateCode,
                              trx,
                              geoTvlType))
  {
    if (fbrProcessingInfo.isSpanishPassengerType() &&
        paxInd == LocUtil::PAX_RESIDENCY &&
        fbrApp.paxLoc().locType() == LOCTYPE_CITY &&
        LocUtil::isCityInSpanishOverseaIslands(fbrApp.paxLoc().loc()))
    {
      fbrProcessingInfo.isResidenceFiledInR8() = true;
    }
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleAppValidator::matchLocation
//
// Description:  To match the LOC1 and LOC2 against the actual origin and
//               destination of the fare being created.
//
// @param  fbrApp        A reference to the FareByRuleApp object
// @param  fareMarket    A reference to the FareMarket object
// @param  fbrProcessingInfo    A reference to the fbrProcessingInfo object
//
// @return bool
//        -true, if Record 8 is matched.
//        -false otherwise.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
FareByRuleAppValidator::matchLocation(PricingTrx& trx,
                                      FareByRuleApp& fbrApp,
                                      FareMarket& fareMarket,
                                      FareByRuleProcessingInfo& fbrProcessingInfo) const
{
  //----------------------------------------------------------------------------
  // validate directional indicator and fare geographic scope (LOC1/LOC2)
  //----------------------------------------------------------------------------
  fbrProcessingInfo.isR8LocationSwapped() = false;

  GeoTravelType geoTvlType = fareMarket.geoTravelType();

  if (fbrApp.fareLoc1().locType() == RuleConst::ANY_LOCATION_TYPE &&
      fbrApp.loc1zoneItemNo() == RuleConst::NOT_APPLICABLE_ZONE &&
      fbrApp.fareLoc2().locType() == RuleConst::ANY_LOCATION_TYPE &&
      fbrApp.loc2zoneItemNo() == RuleConst::NOT_APPLICABLE_ZONE)
  {
    return true;
  }
  // match the original direction
  else if (matchGeo(trx, fbrApp, *(fareMarket.origin()), *(fareMarket.destination()), geoTvlType))
  {
    return true;
  }
  else if (matchGeo(trx, fbrApp, *(fareMarket.destination()), *(fareMarket.origin()), geoTvlType))
  {
    fbrProcessingInfo.isR8LocationSwapped() = true;
    return true;
  }

  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleAppValidator::matchGeo
//
// Description:  To match the specific geo location in loc1 and loc2
//               or the user defined zones against the locations of
//               fare being created.
//
// @param  fbrApp        A reference to the FareByRuleApp object
// @param  loc1          An origin or destination of fare being created
// @param  loc2          A destination or origin of fare being created
//
// @return bool
//        -true, if Record 8 is matched.
//        -false otherwise.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
FareByRuleAppValidator::matchGeo(PricingTrx& trx,
                                 FareByRuleApp& fbrApp,
                                 const Loc& loc1,
                                 const Loc& loc2,
                                 GeoTravelType geoTvlType) const
{
  bool retval = true;
  VendorCode vendor = fbrApp.vendor();

  //-------------------------------------------------
  // compare input location with record 8 Loc1 data
  //-------------------------------------------------

  const tse::DateTime& ticketDate = trx.getRequest()->ticketingDT();

  if (fbrApp.fareLoc1().locType() != RuleConst::ANY_LOCATION_TYPE)
  {
    retval = LocUtil::isInLoc(loc1,
                              fbrApp.fareLoc1().locType(),
                              fbrApp.fareLoc1().loc(),
                              vendor,
                              RESERVED,
                              LocUtil::OTHER,
                              geoTvlType,
                              EMPTY_STRING(),
                              ticketDate);
  }
  else if (LIKELY(fbrApp.loc1zoneItemNo() != RuleConst::NOT_APPLICABLE_ZONE))
  {
    retval = LocUtil::isInLoc(loc1,
                              LOCTYPE_ZONE,
                              fbrApp.loc1zoneItemNo(),
                              vendor,
                              USER_DEFINED,
                              LocUtil::OTHER,
                              geoTvlType,
                              EMPTY_STRING(),
                              ticketDate);
  }

  if (retval)
  {
    //-------------------------------------------------
    // compare input location with record 8 Loc2 data
    //-------------------------------------------------
    if (fbrApp.fareLoc2().locType() != RuleConst::ANY_LOCATION_TYPE)
    {
      retval = LocUtil::isInLoc(loc2,
                                fbrApp.fareLoc2().locType(),
                                fbrApp.fareLoc2().loc(),
                                vendor,
                                RESERVED,
                                LocUtil::OTHER,
                                geoTvlType);
    }

    else if (fbrApp.loc2zoneItemNo() != RuleConst::NOT_APPLICABLE_ZONE)
    {
      retval = LocUtil::isInLoc(
          loc2, LOCTYPE_ZONE, fbrApp.loc2zoneItemNo(), vendor, USER_DEFINED, LocUtil::OTHER, geoTvlType);
    }
  }

  return retval;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleAppValidator::matchWhollyWithin
//
// Description:  To match all travel within the fare component being created
//               must be within the location identified by LOC3.
//
// @param  fbrApp        A pointer to the FareByRuleApp object
// @param  fareMarket    A pointer to the FareMarket object
//
// @return bool
//        -true, if Record 8 is matched.
//        -false otherwise.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
FareByRuleAppValidator::matchWhollyWithin(FareByRuleApp& fbrApp, FareMarket& fareMarket) const
{
  LocTypeCode whollyWithinLocType = fbrApp.whollyWithinLoc().locType();

  if (UNLIKELY(whollyWithinLocType == LOCTYPE_CITY))
  {
    return false;
  }

  LocCode whollyWithinLoc = fbrApp.whollyWithinLoc().loc();
  VendorCode vendor = fbrApp.vendor();
  GeoTravelType geoTvlType = fareMarket.geoTravelType();

  std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
  std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();

  for (; iterTvl != iterTvlEnd; iterTvl++)
  {
    if (!(LocUtil::isInLoc(*(*iterTvl)->origin(),
                           whollyWithinLocType,
                           whollyWithinLoc,
                           vendor,
                           RESERVED,
                           LocUtil::OTHER,
                           geoTvlType)))
    {
      return false;
    }

    if (!(LocUtil::isInLoc(*(*iterTvl)->destination(),
                           whollyWithinLocType,
                           whollyWithinLoc,
                           vendor,
                           RESERVED,
                           LocUtil::OTHER,
                           geoTvlType)))
    {
      return false;
    }

    const std::vector<const Loc*>& hiddenStops = (*iterTvl)->hiddenStops();
    std::vector<const Loc*>::const_iterator stopIter = hiddenStops.begin();
    for (; stopIter != hiddenStops.end(); stopIter++)
    {
      if (!(LocUtil::isInLoc(**stopIter,
                             whollyWithinLocType,
                             whollyWithinLoc,
                             vendor,
                             RESERVED,
                             LocUtil::OTHER,
                             geoTvlType)))
      {
        return false;
      }
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleAppValidator::checkUpFrontSecurity
//
// Description:  To perform Up-Front Security Check for Record 8 which the
//               tariff is private.
//
//               If any Category 35 exists, validate the user's location
//               against the security data. If the user's location is not
//               found in Category 35 data, fails this Record 8. Otherwise,
//               softpass the Record 8 and continue to validate on other fields.
//
//               If no Category 35 is found or Category 35 data does exist but
//               it does not belong to Sabre, check for Category 15. If Category 15
//               data exists, validate the user's location against the security data.
//               If the user's location is not found in Category 15 data, fails
//               this Record 8. Otherwise, softpass the Record 8 and continue to
//               validate on other fields.
//
//               If no Category 35 and Category 15 for this private tariff, fails
//               this Record 8.
//
//               Category 35 security data will always take precedence over
//               Category 15 security data. If Category 35 exists, validate the
//               Category 35 security information only and not the Category 15
//               security data.
//
// @param  trx           Transaction info
// @param  fareMarket    A reference to the FareMarket object.
//
// @return bool
//        -true, if Record 8 has passed up-front security check
//        -false otherwise.
//
// </PRE>
// ----------------------------------------------------------------------------
bool
FareByRuleAppValidator::checkUpFrontSecurity(FareByRuleApp& fbrApp,
                                             PricingTrx& trx,
                                             FareMarket& fareMarket,
                                             TariffCrossRefInfo& tcrInfo,
                                             bool isFdTrx,
                                             FareByRuleProcessingInfo& fbrProcessingInfo) const
{
  //----------------------------------------
  // Cat 35 Up-Front Security Check
  //----------------------------------------
  const std::vector<GeneralFareRuleInfo*>& gfr35List =
      trx.dataHandle().getGeneralFareRule(fbrApp.vendor(),
                                          fbrApp.carrier(),
                                          fbrApp.ruleTariff(),
                                          fbrApp.ruleNo(),
                                          RuleConst::NEGOTIATED_RULE,
                                          fareMarket.travelDate());
  // only check if has a neg fare rule
  if (!gfr35List.empty() && !trx.dataHandle().isHistorical())
  {
    const std::vector<tse::MarkupSecFilter*>& lstMSF = trx.dataHandle().getMarkupSecFilter(
        fbrApp.vendor(), fbrApp.carrier(), fbrApp.ruleTariff(), fbrApp.ruleNo());
    std::vector<tse::MarkupSecFilter*>::const_iterator filtRec;

    if (LIKELY(TrxUtil::isMarkupSecFilterEnhancementEnabled(trx)))
    {
      if (validateMarkUpSecFilterEnhanced(lstMSF, fbrProcessingInfo))
      {
        return true;
      }
    } // end isMarkupSecFilterEnhancementEnabled
    // if no filter recs, do cat15 check
    // otherwise return based on match
    else if (!lstMSF.empty())
    {
      if (validateMarkUpSecFilter(lstMSF, fbrProcessingInfo))
      {
        return true;
      }
    } // end if - non-empty MSF
  } // endif - has Cat35 rule

  //---------------------------------------------------------------------------
  // Per ATPCO, private Cat 25 fare may use only Cat 15 security of base fare.
  // Then we can not do up-front Cat 15 security check at Record 8 level
  // because we may kill Record 8 which its VCTR does not have Cat 35 or stand
  // alone Cat 15 but the Cat 15 override tag value in Record 3 Cat 25 is 'B'
  // for base fare only.
  //---------------------------------------------------------------------------
  if (trx.dataHandle().getVendorType(fbrApp.vendor()) == PUBLISHED)
  {
    return true;
  }

  //-----------------------------------------------
  // Cat 15 Up-Front Security Check
  // for Record 8 vendor codes other than ATP/SITA
  //-----------------------------------------------

  // lint --e{527}
  const std::vector<GeneralFareRuleInfo*>& gfrList =
      trx.dataHandle().getGeneralFareRule(fbrApp.vendor(),
                                          fbrApp.carrier(),
                                          fbrApp.ruleTariff(),
                                          fbrApp.ruleNo(),
                                          RuleConst::SALE_RESTRICTIONS_RULE,
                                          fareMarket.travelDate());

  if (LIKELY(!gfrList.empty()))
  {
    // During Cat 25 fare creation, there is no fare yet.
    // So we need dummy PaxTypeFare for Cat 15 rule validation.
    FareInfo fareInfo;

    fareInfo.carrier() = fbrApp.carrier();
    fareInfo.vendor() = fbrApp.vendor();

    TariffCrossRefInfo tariffCrossRefInfo;
    tariffCrossRefInfo.tariffCat() = tcrInfo.tariffCat();

    Fare fare;
    fare.initialize(Fare::FS_PublishedFare, &fareInfo, fareMarket, &tariffCrossRefInfo);

    PaxTypeFare paxTypeFare;
    paxTypeFare.initializeFlexFareValidationStatus(trx);
    paxTypeFare.setFare(&fare);
    paxTypeFare.fareMarket() = &fareMarket;

    vector<GeneralFareRuleInfo*>::const_iterator bIt = gfrList.begin();

    while (bIt != gfrList.end())
    {
      const vector<CategoryRuleItemInfoSet*>& ruleItemInfoSets = (*bIt)->categoryRuleItemInfoSet();
      vector<CategoryRuleItemInfoSet*>::const_iterator m = ruleItemInfoSets.begin();
      vector<CategoryRuleItemInfoSet*>::const_iterator n = ruleItemInfoSets.end();

      for (; m != n; ++m)
      {
        CategoryRuleItemInfoSet* ruleItemInfoSet = *m;

        if (UNLIKELY(ruleItemInfoSet == nullptr))
        {
          continue;
        }

        std::vector<CategoryRuleItemInfo>::iterator p =
            ruleItemInfoSet->begin();
        std::vector<CategoryRuleItemInfo>::iterator q =
            ruleItemInfoSet->end();

        for (; p != q; p++)
        {
          CategoryRuleItemInfo* catRuleItemInfo = &(*p);

          if (UNLIKELY(catRuleItemInfo == nullptr))
          {
            continue;
          }
          if (LIKELY(catRuleItemInfo->itemcat() == RuleConst::SALE_RESTRICTIONS_RULE))
          {
            const SalesRestriction* salesRestriction =
                trx.dataHandle().getSalesRestriction(fbrApp.vendor(), catRuleItemInfo->itemNo());

            if (UNLIKELY(salesRestriction == nullptr))
            {
              continue;
            }

            bool failedSabre = false;
            SalesRestrictionRule salesRestrictionRule;
            if (tse::checkSecurityRestriction(salesRestrictionRule, trx,
                                                              nullptr,
                                                              paxTypeFare,
                                                              **bIt,
                                                              catRuleItemInfo,
                                                              salesRestriction,
                                                              failedSabre,
                                                              isFdTrx,
                                                              true))
            {
              return true;
            }
          }
        } // Loop thru all Record 3 Cat 15s
      } // Loop thru all sets

      ++bIt;
    } // Loop thru all Record 2 Cat 15s
  }

  return false;
}

bool
FareByRuleAppValidator::validateMarkUpSecFilter(const std::vector<MarkupSecFilter*>& lstMSF,
                                                FareByRuleProcessingInfo& fbrProcessingInfo) const
{
  FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();
  PricingTrx& trx = *fbrProcessingInfo.trx();
  const GeoTravelType geoTravelType = fbrProcessingInfo.fareMarket()->geoTravelType();
  const Agent* agent = trx.getRequest()->ticketingAgent();
  const bool isAgent = !agent->tvlAgencyPCC().empty();
  bool agentOrAirlineFound = false;

  std::vector<MarkupSecFilter*>::const_iterator filtRec = lstMSF.begin();
  const std::vector<MarkupSecFilter*>::const_iterator filtRecEnd = lstMSF.end();
  for (; filtRec != filtRecEnd; ++filtRec)
  {
    // check PCC or CRS based on who is making request
    if (isAgent)
    {
      if (!(*filtRec)->carrierCrs().empty() &&
          !isMatchCarrierCrs((*filtRec)->carrierCrs(), trx, agent))
      {
        continue;
      }
      agentOrAirlineFound = true; // True when item with pseudoCityInd=Y is found

      if ((*filtRec)->pseudoCityType() == RuleConst::TRAVEL_AGENCY)
      {
        if ((*filtRec)->pseudoCity() != agent->tvlAgencyPCC())
          continue;
      }
      else if ((*filtRec)->pseudoCityType() == RuleConst::HOME_TRAVEL_AGENCY)
      {
        if ((*filtRec)->pseudoCity() != agent->mainTvlAgencyPCC() &&
            (*filtRec)->pseudoCity() != agent->tvlAgencyPCC())
          continue;
      }
      else if ((*filtRec)->pseudoCityType() == RuleConst::IATA_TVL_AGENCY_NO)
      {
        if ((*filtRec)->iataTvlAgencyNo() != agent->tvlAgencyIATA())
          continue;
      }
      else if ((*filtRec)->pseudoCityType() == RuleConst::HOME_IATA_TVL_AGENCY_NO)
      {
        if ((*filtRec)->iataTvlAgencyNo() != agent->homeAgencyIATA() &&
            (*filtRec)->iataTvlAgencyNo() != agent->tvlAgencyIATA())
          continue;
      }
    }
    else
    { // isAirline
      agentOrAirlineFound = true; // Always true for A/L request

      if ((*filtRec)->pseudoCityInd() == YES)
      {
        continue;
      }

      if (!(*filtRec)->carrierCrs().empty() &&
          !isMatchCarrierCrs((*filtRec)->carrierCrs(), trx, agent))
      {
        continue;
      }
    } // endif pcc or crs check

    if (!LocUtil::isInLoc(*agent->agentLocation(),
                          (*filtRec)->loc().locType(),
                          (*filtRec)->loc().loc(),
                          fbrApp.vendor(),
                          RESERVED,
                          LocUtil::OTHER,
                          geoTravelType))
    {
      continue;
    }

    return true; // passed all checks
  } // endfor - all filt recs

  if (agentOrAirlineFound)
  {
    // Fail Cat 35 MSF - The indicator will be checked in FareByRuleValidator
    fbrProcessingInfo.failCat35MSF() = true;
    return true; // looped thru all vector w/o match
  }
  return false;
}

bool
FareByRuleAppValidator::validateMarkUpSecFilterEnhanced(const std::vector<MarkupSecFilter*>& lstMSF,
                                                        FareByRuleProcessingInfo& fbrProcessingInfo)
    const
{
  FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();
  PricingTrx& trx = *fbrProcessingInfo.trx();
  const GeoTravelType geoTravelType = fbrProcessingInfo.fareMarket()->geoTravelType();
  const Agent* agent = trx.getRequest()->ticketingAgent();
  const bool isAgent = !agent->tvlAgencyPCC().empty();

  std::vector<MarkupSecFilter*>::const_iterator filtRec = lstMSF.begin();
  const std::vector<MarkupSecFilter*>::const_iterator filtRecEnd = lstMSF.end();
  for (; filtRec != filtRecEnd; ++filtRec)
  {
    // check PCC or CRS based on who is making request
    if (isAgent)
    {
      if (!(*filtRec)->carrierCrs().empty() &&
          !isMatchCarrierCrs((*filtRec)->carrierCrs(), trx, agent))
      {
        continue;
      }

      if ((*filtRec)->pseudoCityType() == RuleConst::TRAVEL_AGENCY)
      {
        if ((*filtRec)->pseudoCity() != agent->tvlAgencyPCC())
          continue;
      }
      else if ((*filtRec)->pseudoCityType() == RuleConst::HOME_TRAVEL_AGENCY)
      {
        if ((*filtRec)->pseudoCity() != agent->mainTvlAgencyPCC() &&
            (*filtRec)->pseudoCity() != agent->tvlAgencyPCC())
          continue;
      }
      else if ((*filtRec)->pseudoCityType() == RuleConst::IATA_TVL_AGENCY_NO)
      {
        if ((*filtRec)->iataTvlAgencyNo() != agent->tvlAgencyIATA())
          continue;
      }
      else if ((*filtRec)->pseudoCityType() == RuleConst::HOME_IATA_TVL_AGENCY_NO)
      {
        if ((*filtRec)->iataTvlAgencyNo() != agent->homeAgencyIATA() &&
            (*filtRec)->iataTvlAgencyNo() != agent->tvlAgencyIATA())
          continue;
      }
      else if ((*filtRec)->pseudoCityType() == RuleConst::CRS_CRX_DEPT_CODE)
      {
        if ((*filtRec)->carrierCrs() == RuleConst::SABRE1S)
          continue;
      }
    }
    else
    { // isAirline

      if ((*filtRec)->pseudoCityInd() == YES)
      {
        continue;
      }

      if (!(*filtRec)->carrierCrs().empty() &&
          !isMatchCarrierCrs((*filtRec)->carrierCrs(), trx, agent))
      {
        continue;
      }
    } // endif pcc or crs check

    if (!LocUtil::isInLoc(*agent->agentLocation(),
                          (*filtRec)->loc().locType(),
                          (*filtRec)->loc().loc(),
                          fbrApp.vendor(),
                          RESERVED,
                          LocUtil::OTHER,
                          geoTravelType))
    {
      continue;
    }

    return true; // passed all checks
  } // endfor - all filt recs

  if (RuleUtil::isFrrNetCustomer(trx))
  {
    return true;
  }
  else
  {
    fbrProcessingInfo.failCat35MSF() = true;
    return false;
  }
}

bool
FareByRuleAppValidator::isMatchCarrierCrs(CarrierCode& carrierCrs,
                                          PricingTrx& trx,
                                          const Agent* agent) const
{
  if (carrierCrs[0] == '1')
  {
    if ((carrierCrs == RuleConst::SABRE1S) ||
        (carrierCrs == RuleConst::SABRE1B && agent->abacusUser()) ||
        (carrierCrs == RuleConst::SABRE1J && agent->axessUser()) ||
        (carrierCrs == RuleConst::SABRE1F && agent->infiniUser()))
    {
      return true;
    }
  }
  else if (carrierCrs == trx.billing()->partitionID())
  {
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleApp::doDiag208Collector
//
// Description:  To activate the Diag208Collector with Record 8 object and
//               reason code for the failed Record 8.
//
// @param  fbrApp        A pointer to the FareByRuleApp object
// @param  trx           Transaction info
// @param  i             A failed reason code. Value 0 if passes.
//
// </PRE>
// ----------------------------------------------------------------------------

void
FareByRuleAppValidator::doDiag208Collector(FareByRuleApp& fbrApp,
                                           PricingTrx& trx,
                                           const char* failCode,
                                           const FareMarket& fareMarket) const
{
  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic208))
  {
    if (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED" &&
        failCode != Diag208Collector::R8_PASS)
      return;

    if (_diagManager)
    {
      Diag208Collector* diag = dynamic_cast<Diag208Collector*>(&_diagManager->collector());
      diag->diag208Collector(fbrApp, trx, failCode, fareMarket);
    }
    else
    {
      DCFactory* factory = DCFactory::instance();
      Diag208Collector* diag = dynamic_cast<Diag208Collector*>(factory->create(trx));
      diag->enable(Diagnostic208);

      diag->diag208Collector(fbrApp, trx, failCode, fareMarket);

      diag->flushMsg();
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleApp::isUsCaRuleTariff
//
// </PRE>
// ----------------------------------------------------------------------------
bool
FareByRuleAppValidator::isUsCaRuleTariff(FareByRuleApp& fbrApp) const
{
  if (fbrApp.ruleTariff() == 191 || fbrApp.ruleTariff() == 770 || fbrApp.ruleTariff() == 873 ||
      fbrApp.ruleTariff() == 874)
  {
    return true;
  }
  return false;
}

bool
FareByRuleAppValidator::isVendorPCC(const VendorCode& vendor, PricingTrx& trx) const
{
  if ((vendor == ATPCO_VENDOR_CODE) || (vendor == SITA_VENDOR_CODE) ||
      (vendor == SMF_ABACUS_CARRIER_VENDOR_CODE) || (vendor == SMF_CARRIER_VENDOR_CODE))
  {
    return false;
  }
  else
  {
    return (trx.dataHandle().getVendorType(vendor) == RuleConst::SMF_VENDOR);
  }
}

bool
IsTestRuleTariff::
operator()(TariffCrossRefInfo* tcrInfo)
{
  return ((tcrInfo && tcrInfo->ruleTariffCode()[1] == FareByRuleAppValidator::TEST_RULE &&
           tcrInfo->ruleTariffCode()[2] == FareByRuleAppValidator::TEST_RULE) &&
          (tcrInfo->ruleTariff() == 850 || tcrInfo->ruleTariff() == 851 ||
           tcrInfo->ruleTariff() == 852 || tcrInfo->ruleTariff() == 853 ||
           tcrInfo->ruleTariff() == 854 || tcrInfo->ruleTariff() == 855 ||
           tcrInfo->ruleTariff() == 856 || tcrInfo->ruleTariff() == 857));
}

}
