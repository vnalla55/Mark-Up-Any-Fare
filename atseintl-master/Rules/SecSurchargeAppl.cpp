//-------------------------------------------------------------------
//
//  File:        SecSurchargeAppl.cpp
//  Created:     Oct 21, 2004
//  Authors:     Simon Li
//
//  Description:
//          This class is for Sector Surchages validation, diagnostic 416.
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
//------------------------------------------------------------------

#include "Rules/SecSurchargeAppl.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/Nation.h"
#include "DBAccess/SectorSurcharge.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{
static Logger
logger("atseintl.Rules.SecSurchargeAppl");

const std::string SecSurchargeAppl::CRS_USER_UNMATCH = "CRS USER";
const std::string SecSurchargeAppl::MULTIHOST_USER_UNMATCH = "MULTIHOST USER";
const std::string SecSurchargeAppl::SALES_LOC_UNMATCH = "SALES LOCATION";
const std::string SecSurchargeAppl::TICKET_LOC_UNMATCH = "TKTING LOCATION";
const std::string SecSurchargeAppl::PAX_TYPE_EXCEPT = "PSG TYPE EXCEPTION";
const std::string SecSurchargeAppl::SURCHARGE_TYPE = "SURCHARGE TYPE";
const std::string SecSurchargeAppl::TRAVEL_DATE = "TRAVEL DATE";
const std::string SecSurchargeAppl::CAT12_SURCHARGE = "CAT12 SURCH EXIST";
const std::string SecSurchargeAppl::LOC_NOT_MATCHED = "LOC NOT MATCHED";
const std::string SecSurchargeAppl::DIRECTION_NOT_MATCHED = "NOT IN DIRECTION";
const std::string SecSurchargeAppl::DATE_NOT_MATCHED = "DATE NOT MATCHED";
const std::string SecSurchargeAppl::MEMORY_ERROR = "MEMORY ERROR";
const std::string SecSurchargeAppl::TICKET_CARRIER_EXCEPTION = "TKTING CARRIER EXCEPTION";
const std::string SecSurchargeAppl::LOG_FAIL_MSG = " Leaving SecSurchargeAppl::validate() - FAIL";

const std::string SecSurchargeAppl::SURCHARGE_DESC = "SECTOR SURCHARGE";
const std::string SecSurchargeAppl::EXCLUDE_SURCHARGE_SECTOR = "EXCLUDE SECTOR SURCHARGE";

//----------------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::process()
//
//  process()     check if sector surcharge applies and update transaction
//                 surcharge if it does.
//
//  @param PricingTrx&           - Pricing transaction
//  @param FareUsage&            - FareUsage that sector surcharge will be
//                               checked and processed on
//
//  @return  Record3ReturnTypes
//                  SKIP         surcharge does not apply
//----------------------------------------------------------------------------
void
SecSurchargeAppl::process(PricingTrx& trx,
                          const CarrierCode& validatingCxr,
                          FareUsage& fareUsage,
                          SecSurchRuleMap& ssrMap)
{
  LOG4CXX_INFO(logger, " Entered SecSurchargeAppl::process() for FareUsage");

  std::vector<TravelSeg*>::const_iterator tvlSegI = fareUsage.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = fareUsage.travelSeg().end();

  //-------------------------------------------------------
  // Loop travel segments of fareUsage, get all surcharges
  // that apply.
  //-------------------------------------------------------

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    //-----------------------------------------------------
    // We should look up sector surcharge information
    // with marketing carrier of travel sector as key;
    //-----------------------------------------------------
    const AirSeg* airSeg = dynamic_cast<AirSeg*>(*tvlSegI);

    if (airSeg != nullptr)
    {
      const CarrierCode& carrier = airSeg->marketingCarrierCode();
      if (ssrMap.find(carrier) == ssrMap.end())
      {
        const std::vector<SectorSurcharge*>& secSurchList =
            trx.dataHandle().getSectorSurcharge(carrier, trx.ticketingDate());

        ssrMap[carrier] = &secSurchList;
      }
      const SecSurchRuleList* surchList = ssrMap[carrier];
      if (UNLIKELY(surchList == nullptr))
        continue;

      //-------------------------------------------------------
      // For each surcharge rule, check if the surcharge apply
      //-------------------------------------------------------
      SecSurchRuleList::const_iterator surchInfoI = surchList->begin();
      SecSurchRuleList::const_iterator surchInfoEndI = surchList->end();

      for (; surchInfoI != surchInfoEndI; surchInfoI++)
        validate(trx, validatingCxr, **surchInfoI, **tvlSegI, fareUsage);
    }
  }

  LOG4CXX_INFO(logger, " Leaving SecSurchargeAppl::process()");
  return;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::validate()
//
//  validate()     check if sector surcharge applies and update transaction
//                 surcharge if it does.
//
//  @param PricingTrx&           - Pricing transaction
//  @param SectorSurcharge       - Sector surcharge rule data
//  @param FareUsage&            - FareUsage that sector surcharge will be
//                               checked and processed on
//
//  @return  Record3ReturnTypes
//                  SKIP         surcharge does not apply
//                  PASS         surcharge applied
//                  FAIL         severe error
//
// </PRE>
//----------------------------------------------------------------------------
Record3ReturnTypes
SecSurchargeAppl::validate(PricingTrx& trx,
                           const CarrierCode& validatingCxr,
                           const SectorSurcharge& surchInfo,
                           const TravelSeg& tvlSeg,
                           FareUsage& fareUsage)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;

  diag.enable(Diagnostic416);
  if (diag.isActive())
  {
    diag.printLine();
    diag << "* SECTOR SURCHARGE DIAGNOSTIC\n";
    diag.printLine();
    diag << *fareUsage.paxTypeFare() << std::endl;
    const std::vector<TravelSeg*>& tvlSegs = fareUsage.travelSeg();
    diag << " FARE COMPONENT SEGS: " << tvlSegs << std::endl;
    diag << " SECTOR: " << tvlSeg.origAirport() << tvlSeg.destAirport() << std::endl;
    displayRuleToDiag(surchInfo, diag);
  }

  const PricingRequest& request = *trx.getRequest();

  const PaxTypeFare& ptFare = *fareUsage.paxTypeFare();

  //-----------------------------------------------------------------
  // Inhouse sector Surchrges / General rule surcharge not applicable
  //-----------------------------------------------------------------

  PaxTypeFareRuleData* ptFrData = ptFare.paxTypeFareRuleData(RuleConst::SURCHARGE_RULE);

  if (ptFrData != nullptr && ptFrData->categoryRuleInfo() != nullptr)
  {
    const GeneralFareRuleInfo* i =
        dynamic_cast<const GeneralFareRuleInfo*>(ptFrData->categoryRuleInfo());
    //  "1"  =  Do not apply surcharges from the sector-surcharge table
    //  "2"  =  Do not apply either carrier filed general rule surcharges or surcharges from the
    // sector-surcharge table

    if (UNLIKELY(('1' == i->generalRuleAppl()) || ('2' == i->generalRuleAppl())))
    {
      return diagAndRtn(SKIP, diag, factory, EXCLUDE_SURCHARGE_SECTOR);
    }
  }
  //-------------------------------------------------------
  // CRS User Application or Multi-Host User Application
  //-------------------------------------------------------
  if (UNLIKELY(surchInfo.userApplType() == tse::CRS_USER_APPL))
  {
    if (surchInfo.userAppl() != request.ticketingAgent()->cxrCode())
    {
      return diagAndRtn(SKIP, diag, factory, CRS_USER_UNMATCH);
    }
  }
  else if (UNLIKELY(surchInfo.userApplType() == tse::MULTIHOST_USER_APPL))
  {
    if (surchInfo.userAppl() != ((const PricingTrx&)trx).billing()->partitionID())
    {
      return diagAndRtn(SKIP, diag, factory, MULTIHOST_USER_UNMATCH);
    }
  }

  //=======================================================
  // Sales Loc Restriction
  //=======================================================
  if (UNLIKELY(!surchInfo.posLoc().loc().empty()))
  {
    Loc agentLoc = *request.ticketingAgent()->agentLocation();
    if (!request.salePointOverride().empty())
    {
      agentLoc.loc() = request.salePointOverride();
    }

    if (LocUtil::isInLoc(agentLoc,
                         surchInfo.posLoc().locType(),
                         surchInfo.posLoc().loc(),
                         EMPTY_VENDOR,
                         RESERVED,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()) == false)
    {
      return diagAndRtn(SKIP, diag, factory, SALES_LOC_UNMATCH);
    }
  }

  //=======================================================
  // Ticketing Loc Restriction
  //=======================================================
  if (UNLIKELY(request.ticketingDT().isValid() && !surchInfo.poiLoc().loc().empty()))
  {
    Loc agentLoc = *request.ticketingAgent()->agentLocation();
    if (!request.ticketPointOverride().empty())
    {
      agentLoc.loc() = request.ticketPointOverride();
    }

    if (LocUtil::isInLoc(agentLoc,
                         surchInfo.poiLoc().locType(),
                         surchInfo.poiLoc().loc(),
                         EMPTY_VENDOR,
                         RESERVED,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()) == false)
    {
      return diagAndRtn(SKIP, diag, factory, TICKET_LOC_UNMATCH);
    }
  }

  //=======================================================
  // Ticking carrier
  //=======================================================
  if (UNLIKELY(!surchInfo.tktgCxrs().empty()))
  {
    bool foundMatch = false;

    if (validatingCxr.c_str()[0] != 0)
    {
      std::vector<CarrierCode>::const_iterator tktCarrierI = surchInfo.tktgCxrs().begin();
      std::vector<CarrierCode>::const_iterator tktCarrierEndI = surchInfo.tktgCxrs().end();

      for (; tktCarrierI != tktCarrierEndI; tktCarrierI++)
      {
        if (validatingCxr == *tktCarrierI)
        {
          foundMatch = true;
          break;
        }
      }
    }
    else
    {
      foundMatch = true;
    }

    if (surchInfo.exceptTktgCarrier() == tse::YES)
    {
      if (foundMatch)
      {
        return diagAndRtn(SKIP, diag, factory, TICKET_CARRIER_EXCEPTION);
      }
    }
    else
    {
      // inclusive
      if (!foundMatch)
      {
        return diagAndRtn(SKIP, diag, factory, TICKET_CARRIER_EXCEPTION);
      }
    }
  }

  //=======================================================
  // Excluded Passenger Type(s)
  //=======================================================
  if (UNLIKELY(surchInfo.exclPsgType() == tse::YES && matchExclPsgType(trx, fareUsage, surchInfo)))
  {
    return diagAndRtn(SKIP, diag, factory, PAX_TYPE_EXCEPT);
  }

  //=======================================================
  // Surcharge Type
  //=======================================================
  if (UNLIKELY(surchInfo.surchargeType() == RuleConst::BLANK))
  {
    return diagAndRtn(SKIP, diag, factory, SURCHARGE_TYPE);
  }

  //=======================================================
  // Sector Location(s)
  //=======================================================
  if (UNLIKELY(surchInfo.surchargeAppl() != NOT_APPLY))
  {
    if (!matchDirection(trx, tvlSeg, fareUsage, surchInfo))
      return diagAndRtn(SKIP, diag, factory, DIRECTION_NOT_MATCHED);
  }
  if (!matchLocations(trx, tvlSeg, surchInfo, fareUsage))
  {
    return diagAndRtn(SKIP, diag, factory, LOC_NOT_MATCHED);
  }

  //=======================================================
  // Travel Date/Time, Day Of Week
  //=======================================================
  if (UNLIKELY(!matchTvlDate(tvlSeg, surchInfo) || !matchDOWandTime(tvlSeg, surchInfo)))
  {
    return diagAndRtn(SKIP, diag, factory, DATE_NOT_MATCHED);
  }

  //=======================================================
  // get Surcharge detail
  //=======================================================
  SurchargeData* surchargeData = nullptr;
  if (LIKELY(trx.getTrxType() == PricingTrx::MIP_TRX || dynamic_cast<ShoppingTrx*>(&trx)))
  {
    trx.dataHandle().get(surchargeData);
  }
  else
  {
    surchargeData = trx.constructSurchargeData();
  }
  if (UNLIKELY(surchargeData == nullptr))
  {
    return diagAndRtn(FAIL, diag, factory, MEMORY_ERROR);
  }

  calcSurcharge(trx, fareUsage, surchInfo, *surchargeData, tvlSeg);

  //------------------------------------------------------
  // We do not add surcharge of same type if there is
  // already one by cat12 and same amount
  //------------------------------------------------------
  if (findCat12Surcharge(fareUsage, tvlSeg, *surchargeData))
  {
    return diagAndRtn(SKIP, diag, factory, CAT12_SURCHARGE);
  }

  //------------------------------------------------------
  // Sector Surcharge apply
  // Diagnostic this travel segment, and process surcharge
  //------------------------------------------------------
  if (UNLIKELY(diag.isActive()))
  {
    diag << "SEC SURCHARGE APPLY - " << tvlSeg.departureDT().toIsoExtendedString() << " "
         << tvlSeg.origAirport() << " " << tvlSeg.destAirport() << std::endl;
  }
  /*
      //------------------------------------------------------
      // set up a surcharge for the paper ticket
      //------------------------------------------------------
      if (surchInfo.surchargeType() == RuleConst::OTHER       &&
          fareUsage.paxTypeFare()->isElectronicTktRequired())
      {
          if (trx.getRequest()->isElectronicTicket())
          {
              fareUsage.paperTktSurchargeMayApply();
          }
          else
          {
              fareUsage.paperTktSurchargeIncluded();
          }
      }
   */
  LOG4CXX_DEBUG(logger,
                "\nFU: " << &fareUsage << ", Surcharge: " << surchargeData->carrier() << ", "
                         << surchargeData->surchargeType() << ", " << surchargeData->brdAirport()
                         << ", " << surchargeData->offAirport() << ", "
                         << surchargeData->amountNuc() << " : " << surchargeData->surchargeDesc());

  fareUsage.surchargeData().push_back(surchargeData);

  if (UNLIKELY(diag.isActive()))
  {
    diag << " SURCHARGE NUC AMT:" << surchargeData->amountNuc()
         << " SURCHARGE AMT:" << surchargeData->amountSelected() << " "
         << surchargeData->currSelected() << std::endl;
  }

  return diagAndRtn(PASS, diag, factory, "SURCHARGE APPLIED");
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::matchTvlDate()
//
//  matchTvlDate()     check if travel date of a travel segment meets
//                     the rule data requirement
//
//  @param SectorSurcharge&   -  Sector surcharge rule data
//  @param TravelSeg&         -  Travel Segment of that travel date is check
//                            against
//
//  @return  bool
//          true            - passed requirement
//          false           - failed requirement (surcharge won't apply)
//
// </PRE>
//-------------------------------------------------------------------
bool
SecSurchargeAppl::matchTvlDate(const TravelSeg& tvlSeg, const SectorSurcharge& surchInfo)
{
  if (UNLIKELY(surchInfo.firstTvlDate().isValid() &&
      tvlSeg.departureDT().date() < surchInfo.firstTvlDate().date()))
  {
    return false;
  }

  if (UNLIKELY(surchInfo.lastTvlDate().isValid() &&
      tvlSeg.departureDT().date() > surchInfo.lastTvlDate().date()))
  {
    return false;
  }

  return true;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::matchDOWandTime()
//
//  matchDOWandTime()   check Day of Week and Time of Day
//
//  @param TravelSeg&           -  Travel Segment that day of week and time
//                                 of day of travel date is check against
//  @param SectorSurcharge&     -  Rule data
//
//  @return  bool
//          true            - passed requirement
//          false           - failed requirement (surcharge won't apply)
//
// </PRE>
//-------------------------------------------------------------------
bool
SecSurchargeAppl::matchDOWandTime(const TravelSeg& tvlSeg, const SectorSurcharge& surchInfo)
{
  const DateTime& tvlDT = tvlSeg.departureDT();

  if (UNLIKELY(!surchInfo.dow().empty()))
  {
    if (tvlSeg.segmentType() == Open && !tvlDT.isValid())
    {
      // Open segment with no date
      return false;
    }

    int dowSeg = tvlDT.dayOfWeek(); // 0...6
    bool dowMatched = false;

    std::string::const_iterator dowI = surchInfo.dow().begin();
    std::string::const_iterator dowEndI = surchInfo.dow().end();

    for (; dowI != dowEndI; dowI++)
    {
      int dowRule = (*dowI) - '0'; // 1---7  Note: 7=sunday!!!
      if (dowRule == 7)
      {
        dowRule = 0;
      }
      if (dowSeg == dowRule)
      {
        dowMatched = true;
        break;
      }
    }

    if (!dowMatched)
    {
      return false;
    }
  } // dow not empty

  // check a time of day, requirement is that departure
  // need to be in the time period defined in rule
  const int32_t tvlStartTOD = tvlSeg.departureDT().totalMinutes() + 1;
  // range 1~1440,  same as startTime, stopTime

  const bool checkStartTime = isValidTOD(surchInfo.startTime());
  const bool checkStopTime = isValidTOD(surchInfo.stopTime());

  if (LIKELY(!checkStartTime && !checkStopTime))
  {
    return true; // done check
  }

  if (tvlSeg.segmentType() == Open)
  {
    // Open segment with no date/time or with date but no time
    if (surchInfo.startTime() == RuleApplicationBase::LOWEND_TIMEOFDAY &&
        surchInfo.stopTime() == RuleApplicationBase::HIGHEND_TIMEOFDAY)
    {
      // explicitly covers whole day
      return true;
    }
    else
    {
      return false; // system assumption, no sector surcharge
    }
  }

  if (checkStartTime && checkStopTime && (surchInfo.startTime() > surchInfo.stopTime()))
  {
    // Validate time across midnight
    // example, 11:00 PM till 7:00 AM
    if (((tvlStartTOD >= surchInfo.startTime()) || (tvlStartTOD <= surchInfo.stopTime())))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // time not across midnight
  if (checkStartTime)
  {
    if (tvlStartTOD < surchInfo.startTime())
    {
      return false;
    }
  }

  if (checkStopTime)
  {
    if (tvlStartTOD > surchInfo.stopTime())
    {
      return false;
    }
  }

  return true;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::matchExclPsgType()
//
//  matchExclPsgType()     by verifying PaxType of fareUsage to see if
//                         this fareUsage should be excluded from surcharge
//
//  @param PricingTrx&          - Pricing transaction
//  @param FareUsage&           - Fare Usage
//  @param SectorSurcharge&     - Rule Data
//
//  @return  bool
//          true           found paxType in excluded paxType vector
//          false          did not find paxType in excluded paxType vector
//
// </PRE>
//-------------------------------------------------------------------
bool
SecSurchargeAppl::matchExclPsgType(PricingTrx& trx,
                                   const FareUsage& fareUsage,
                                   const SectorSurcharge& surchInfo)
{
  const PaxTypeCode& psgPaxType = fareUsage.paxTypeFare()->fcasPaxType();
  //    const VendorCode& vendor = fareUsage.paxTypeFare()->actualPaxType()->vendorCode();
  const VendorCode& vendor = fareUsage.paxTypeFare()->vendor();

  if ((surchInfo.psgTypeChild() == tse::YES && PaxTypeUtil::isChild(trx, psgPaxType, vendor)) ||
      (surchInfo.psgTypeInfant() == tse::YES && PaxTypeUtil::isInfant(trx, psgPaxType, vendor)))
  {
    return true;
  }

  std::vector<PaxTypeCode>::const_iterator paxTypeI = surchInfo.psgTypes().begin();
  const std::vector<PaxTypeCode>::const_iterator paxTypeEndI = surchInfo.psgTypes().end();

  for (; paxTypeI != paxTypeEndI; paxTypeI++)
  {
    if ((psgPaxType == *paxTypeI) ||
        (*paxTypeI == ADULT && PaxTypeUtil::isAdult(trx, psgPaxType, vendor)))
    {
      return true;
    }
  }

  // Did not find match
  return false;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::matchLocations()
//
// matchLocations()    Check a travel segment matches requirement of
//                     location(s) defined in Sector Surcharge Rule
//
// When surchargeAppl is set, the surcharge apply when sector is in direction
//    of Fare Component; otherwise,
// Depending on directional, the condition below must be meet:
//    1. directional: FROM
//      Match the board point of the sector to the location
//      field Loc 1/Loc 1 Type, off point of the sector to
//      Loc 2/Loc 2 Type, if either of them are valid
//    2. directional: BETWEEN
//      Match the board point or off point to Loc 1/Loc 1 Type,
//      off point or board point to Loc 2 and Loc 2 Type if loc2
//      is valid; (Both direction will have surcharge)
//    3. directional: WITHIN
//      Match the board point or off point to inLoc of Loc 1/Loc 1 Type;
//
//  @param TravelSeg*         Travel Segment to check on
//  @param FareUsage&         For fare component information
//  @param SectorSurcharge&   Rule data
//
//  @return
//            true          Match
//            false         Not match, surcharge not apply
//
// </PRE>
//-------------------------------------------------------------------
bool
SecSurchargeAppl::matchLocations(PricingTrx& trx,
                                 const TravelSeg& tvlSeg,
                                 const SectorSurcharge& surchInfo,
                                 const FareUsage& fareUsage)
{
  const Indicator direction = surchInfo.directionalInd();

  if (LIKELY(direction == FROM))
  {
    if (UNLIKELY(fareUsage.isInbound() && (surchInfo.surchargeAppl() == 'Y')))
    {
      if (((LOCTYPE_NONE == surchInfo.loc2().locType()) ||
           LocUtil::isInLoc(*tvlSeg.origin(),
                            surchInfo.loc2().locType(),
                            surchInfo.loc2().loc(),
                            EMPTY_VENDOR,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT())) &&
          ((LOCTYPE_NONE == surchInfo.loc1().locType()) ||
           LocUtil::isInLoc(*tvlSeg.destination(),
                            surchInfo.loc1().locType(),
                            surchInfo.loc1().loc(),
                            EMPTY_VENDOR,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT())))
      {
        return true;
      }
    }
    else if (((LOCTYPE_NONE == surchInfo.loc1().locType()) ||
              LocUtil::isInLoc(*tvlSeg.origin(),
                               surchInfo.loc1().locType(),
                               surchInfo.loc1().loc(),
                               EMPTY_VENDOR,
                               RESERVED,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               trx.getRequest()->ticketingDT())) &&
             ((LOCTYPE_NONE == surchInfo.loc2().locType()) ||
              LocUtil::isInLoc(*tvlSeg.destination(),
                               surchInfo.loc2().locType(),
                               surchInfo.loc2().loc(),
                               EMPTY_VENDOR,
                               RESERVED,
                               LocUtil::OTHER,
                               GeoTravelType::International,
                               EMPTY_STRING(),
                               trx.getRequest()->ticketingDT())))
    {
      return true;
    }
  }
  else if (direction == BETWEEN)
  {
    if ((LocUtil::isInLoc(*tvlSeg.origin(),
                          surchInfo.loc1().locType(),
                          surchInfo.loc1().loc(),
                          EMPTY_VENDOR,
                          RESERVED,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT()) &&
         ((LOCTYPE_NONE == surchInfo.loc2().locType()) ||
          LocUtil::isInLoc(*tvlSeg.destination(),
                           surchInfo.loc2().locType(),
                           surchInfo.loc2().loc(),
                           EMPTY_VENDOR,
                           RESERVED,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()))) ||
        (LocUtil::isInLoc(*tvlSeg.destination(),
                          surchInfo.loc1().locType(),
                          surchInfo.loc1().loc(),
                          EMPTY_VENDOR,
                          RESERVED,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT()) &&
         ((LOCTYPE_NONE == surchInfo.loc2().locType()) ||
          LocUtil::isInLoc(*tvlSeg.origin(),
                           surchInfo.loc2().locType(),
                           surchInfo.loc2().loc(),
                           EMPTY_VENDOR,
                           RESERVED,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()))))
    {
      return true;
    }
  }
  else if (direction == WITHIN)
  {
    if (LocUtil::isInLoc(*tvlSeg.origin(),
                         surchInfo.loc1().locType(),
                         surchInfo.loc1().loc(),
                         EMPTY_VENDOR,
                         RESERVED,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()) &&
        LocUtil::isInLoc(*tvlSeg.destination(),
                         surchInfo.loc1().locType(),
                         surchInfo.loc1().loc(),
                         EMPTY_VENDOR,
                         RESERVED,
                         LocUtil::OTHER,
                         GeoTravelType::International,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
    {
      return true;
    }
  }

  return false;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::matchDirection()
//
// matchDirection()    Check a travel segment matches direction of
//                     fare component
//
//  @return
//            true          Match
//            false         Not match, surcharge not apply
//
// </PRE>
//-------------------------------------------------------------------
bool
SecSurchargeAppl::matchDirection(PricingTrx& trx,
                                 const TravelSeg& tvlSeg,
                                 const FareUsage& fareUsage,
                                 const SectorSurcharge& surchInfo)
{
  const Indicator direction = surchInfo.directionalInd();

  if (direction == FROM)
  {
    if (fareUsage.isInbound() && (surchInfo.surchargeAppl() == 'Y') &&
        RuleUtil::isInDirection(
            trx, tvlSeg, fareUsage.travelSeg(), surchInfo.loc2(), surchInfo.loc1()))
    {
      return true;
    }
    else if (RuleUtil::isInDirection(
                 trx, tvlSeg, fareUsage.travelSeg(), surchInfo.loc1(), surchInfo.loc2()))
    {
      return true;
    }
  }
  else if (direction == BETWEEN)
  {
    if ((RuleUtil::isInDirection(
            trx, tvlSeg, fareUsage.travelSeg(), surchInfo.loc1(), surchInfo.loc2())) ||
        (RuleUtil::isInDirection(
            trx, tvlSeg, fareUsage.travelSeg(), surchInfo.loc2(), surchInfo.loc1())))
    {
      return true;
    }

    return false;
  }
  else if (direction == WITHIN)
  {
    if (RuleUtil::isWithinLoc(trx, tvlSeg, surchInfo.loc1()))
    {
      return true;
    }

    return false;
  }
  return false;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::calcSurcharge()
//
// calcSurcharge()     process and fill in surcharge information into
//                    a SurchargeData structure
//
//  @param PricingTrx&          - Pricing transaction
//  @param FareUsage&           - Fare Usage
//  @param SectorSurcharge&     - Surcharge Rule Data
//  @param SurchargeData*       - Surcharge data (OUT)
//  @param TravelSeg&           - Travel Seg that matched the rule requirement
//
//  @return void
//
// </PRE>
//-------------------------------------------------------------------
void
SecSurchargeAppl::calcSurcharge(PricingTrx& trx,
                                const FareUsage& fareUsage,
                                const SectorSurcharge& surchInfo,
                                SurchargeData& surchargeData,
                                const TravelSeg& tvlSeg)
{
  MoneyAmount fareAmount = 0;
  MoneyAmount surAmount = 0;
  CurrencyNoDec surCurNoDec = 0;
  MoneyAmount surAmountNuc = 0;
  CurrencyCode surCurrency;

  MoneyAmount surAmountNuc1 = 0;
  MoneyAmount surAmountNuc2 = 0;

  if (UNLIKELY(surchInfo.surchargeCur1().empty() && surchInfo.surchargeCur2().empty()))
  {
    // check a percent
    surCurrency = fareUsage.paxTypeFare()->currency();
    surCurNoDec = fareUsage.paxTypeFare()->numDecimal();
    surAmount =
        ((fareUsage.paxTypeFare()->originalFareAmount()) * (surchInfo.surchargePercent() / 100.0f));
    fareAmount = fareUsage.paxTypeFare()->originalFareAmount();

    Money surAmt(fareAmount, surCurrency);
    Money nuc("NUC");

    CurrencyConversionRequest request(nuc,
                                      surAmt,
                                      trx.getRequest()->ticketingDT(),
                                      *(trx.getRequest()),
                                      trx.dataHandle(),
                                      fareUsage.paxTypeFare()->fare()->isInternational());

    NUCCurrencyConverter ncc;

    if (ncc.convert(request, nullptr))
    {
      surAmountNuc = nuc.value();
      CurrencyUtil::truncateNUCAmount(surAmountNuc);

      surAmountNuc = (surAmountNuc * surchInfo.surchargePercent()) / 100.0f;
      CurrencyUtil::truncateNUCAmount(surAmountNuc);
    }
  }
  else
  {
    // match the surcharge currency
    if (surchInfo.surchargeCur1() == fareUsage.paxTypeFare()->currency())
    {
      surCurrency = surchInfo.surchargeCur1();
      surCurNoDec = fareUsage.paxTypeFare()->numDecimal();
      surAmount = surchInfo.surchargeAmt1();
    }
    else if (surchInfo.surchargeCur2() == fareUsage.paxTypeFare()->currency())
    {
      surCurrency = surchInfo.surchargeCur2();
      surCurNoDec = fareUsage.paxTypeFare()->numDecimal();
      surAmount = surchInfo.surchargeAmt2();
    }
    else
    {
      if (LIKELY(!surchInfo.surchargeCur1().empty()))
      {
        Money surAmt1(surchInfo.surchargeAmt1(), surchInfo.surchargeCur1());
        Money nuc1("NUC");

        CurrencyConversionRequest request(nuc1,
                                          surAmt1,
                                          trx.getRequest()->ticketingDT(),
                                          *(trx.getRequest()),
                                          trx.dataHandle(),
                                          fareUsage.paxTypeFare()->fare()->isInternational());

        NUCCurrencyConverter ncc;

        if (LIKELY(ncc.convert(request, nullptr)))
        {
          surAmountNuc1 = nuc1.value();
          CurrencyUtil::truncateNUCAmount(surAmountNuc1);
        }
      }
      if (!surchInfo.surchargeCur2().empty())
      {
        Money surAmt2(surchInfo.surchargeAmt2(), surchInfo.surchargeCur2());
        Money nuc2("NUC");

        CurrencyConversionRequest request(nuc2,
                                          surAmt2,
                                          trx.getRequest()->ticketingDT(),
                                          *(trx.getRequest()),
                                          trx.dataHandle(),
                                          fareUsage.paxTypeFare()->fare()->isInternational());

        NUCCurrencyConverter ncc;

        if (ncc.convert(request, nullptr))
        {
          surAmountNuc2 = nuc2.value();
          CurrencyUtil::truncateNUCAmount(surAmountNuc2);
        }
      }
      if (LIKELY(surAmountNuc1 < surAmountNuc2 || surchInfo.surchargeCur2().empty()))
      {
        surAmountNuc = surAmountNuc1;
        surCurrency = surchInfo.surchargeCur1();
        surCurNoDec = surchInfo.surchargeNoDec1();
        surAmount = surchInfo.surchargeAmt1();
      }
      else
      {
        surAmountNuc = surAmountNuc2;
        surCurrency = surchInfo.surchargeCur2();
        surCurNoDec = surchInfo.surchargeNoDec2();
        surAmount = surchInfo.surchargeAmt2();
      }
    }
  }
  if (surAmountNuc == 0 && surAmount != 0 && !surCurrency.empty())
  {
    Money surAmt(surAmount, surCurrency);
    Money nuc("NUC");

    CurrencyConversionRequest request(nuc,
                                      surAmt,
                                      trx.getRequest()->ticketingDT(),
                                      *(trx.getRequest()),
                                      trx.dataHandle(),
                                      fareUsage.paxTypeFare()->fare()->isInternational());

    NUCCurrencyConverter ncc;

    if (LIKELY(ncc.convert(request, nullptr)))
    {
      surAmountNuc = nuc.value();
      CurrencyUtil::truncateNUCAmount(surAmountNuc);
    }
  }
  CarrierCode cc = fareUsage.paxTypeFare()->carrier();
  if (cc == INDUSTRY_CARRIER)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(&tvlSeg);

    if (airSeg != nullptr)
    {
      surchargeData.carrier() = airSeg->carrier(); // marketingCarrier
    }
  }
  else
  {
    surchargeData.carrier() = cc;
  }
  surchargeData.itinItemCount() = 1;
  surchargeData.surchargeAppl() = surchInfo.surchargeAppl(); // not same
  // meaning, we save here because it is not used anywhere.
  surchargeData.surchargeType() = surchInfo.surchargeType();
  surchargeData.travelPortion() = SecSurchargeAppl::NOT_APPLY;
  surchargeData.amountSelected() = surAmount;
  surchargeData.currSelected() = surCurrency;
  surchargeData.currNoDecSelected() = surCurNoDec;
  surchargeData.amountNuc() = surAmountNuc;
  surchargeData.brdAirport() = tvlSeg.origAirport();
  surchargeData.offAirport() = tvlSeg.destAirport();
  surchargeData.geoTblNo() = RuleConst::NOT_APPL_GEOTBL_N0;
  surchargeData.locKey1() = surchInfo.loc1();
  surchargeData.locKey2() = surchInfo.loc2();
  surchargeData.geoTblNoBtw() = RuleConst::NOT_APPL_GEOTBL_N0;
  surchargeData.geoTblNoAnd() = RuleConst::NOT_APPL_GEOTBL_N0;
  surchargeData.surchargeDesc() = SecSurchargeAppl::SURCHARGE_DESC;
  surchargeData.fcBrdCity() = tvlSeg.boardMultiCity();
  surchargeData.fcOffCity() = tvlSeg.offMultiCity();
}

bool
SecSurchargeAppl::findCat12Surcharge(const FareUsage& fareUsage,
                                     const TravelSeg& tvlSeg,
                                     const SurchargeData& surchDataSec)
{
  const MoneyAmount& amount = surchDataSec.amountNuc();

  std::vector<SurchargeData*>::const_iterator surchDataI = fareUsage.surchargeData().begin();
  std::vector<SurchargeData*>::const_iterator surchDataEndI = fareUsage.surchargeData().end();

  for (; surchDataI != surchDataEndI; surchDataI++)
  {
    const SurchargeData& surchargeData = **surchDataI;

    // This code is causing sector surchare being charge twice for FareX - QT
    //
    // if (surchargeData.surchargeDesc() == SecSurchargeAppl::SURCHARGE_DESC)
    // {
    //     // Sector surcharge, not Cat12 automated surcharge
    //     continue;
    // }

    if ( //(surchargeData.surchargeType() == surchDataSec.surchargeType())
            //   &&
            (surchargeData.brdAirport() == tvlSeg.origAirport()) &&
            (surchargeData.offAirport() == tvlSeg.destAirport()) &&
            (surchargeData.carrier() == surchDataSec.carrier()))
    {
      // float/double equality check
      // 1 penny difference is seen as equal
      const double diff = amount - surchargeData.amountNuc();

      if (fabs(diff) <= (0.01 + EPSILON))
        return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::diagAndRtn()
//
//  diagAndRtn()    writes final message to logger and diag collector
//                  before exiting validation
//
//  @param const Record3ReturnTypes   - validation result
//  @param DiagCollector&
//  @param DCFactory*                 - factory that will reclaim memory used
//                                     by DiagCollector
//  @param const std::string&         - Diagnostic message explaining the
//                                     validation result
//
//  @return  Record3ReturnTypes
//               same value of Record3ReturnTypes passed in
//
// </PRE>
//----------------------------------------------------------------------------
Record3ReturnTypes
SecSurchargeAppl::diagAndRtn(const Record3ReturnTypes result,
                             DiagCollector& diag,
                             DCFactory* factory,
                             const std::string& diagMsg)
{
  if (result == SKIP)
  {
    diag << " SEC SURCHARGE: SKIP - ";
  }
  else if (LIKELY(result == PASS))
  {
    diag << " SEC SURCHARGE: PASS - ";
  }
  else
  {
    diag << " SEC SURCHARGE: FAIL - ";
  }
  diag << diagMsg << std::endl << " " << std::endl << " " << std::endl;
  diag.flushMsg();

  return result;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @MethodName    SecSurchargeAppl::displayRuleToDiag()
//
//  displayRuleToDiag()    analyze rule detail to diag collector
//
//  @param const SectorSurcharge&     - Rule data
//  @param DiagCollector&             - Diagnostic collector
//
//  @return  Record3ReturnTypes
//
// </PRE>
//----------------------------------------------------------------------------
void
SecSurchargeAppl::displayRuleToDiag(const SectorSurcharge& surchInfo, DiagCollector& diag)
{
  diag << "SECTOR SURCHARGE RULE DATA:" << std::endl;
  diag << " CARRIER: " << surchInfo.carrier() << " VERSION DATE: ";
  if (surchInfo.versionDate().isInfinity())
  {
    diag << "INFINITY";
  }
  else
  {
    diag << surchInfo.versionDate();
  }

  diag << " SEQ NO: " << surchInfo.seqNo() << std::endl;

  diag << " SURCHARGE APPLY IF" << std::endl;
  if (surchInfo.exclPsgType() == tse::YES)
  {
    diag << "  ALL PSG TYPE EXCEPT: ";
    if (surchInfo.psgTypeChild() == tse::YES)
    {
      diag << "CNN ";
    }
    if (surchInfo.psgTypeInfant() == tse::YES)
    {
      diag << "INF ";
    }

    std::vector<PaxTypeCode>::const_iterator paxTypeI = surchInfo.psgTypes().begin();
    const std::vector<PaxTypeCode>::const_iterator paxTypeEndI = surchInfo.psgTypes().end();

    for (; paxTypeI != paxTypeEndI; paxTypeI++)
    {
      diag << (*paxTypeI) << " ";
    }
    diag << std::endl;
  }

  if (!surchInfo.tktgCxrs().empty())
  {
    if (surchInfo.exceptTktgCarrier() == tse::YES)
    {
      diag << "  ALL CARRIERS EXCEPT: ";
    }
    else
    {
      diag << "  CARRIER: ";
    }

    std::vector<CarrierCode>::const_iterator tktCarrierI = surchInfo.tktgCxrs().begin();
    const std::vector<CarrierCode>::const_iterator tktCarrierEndI = surchInfo.tktgCxrs().end();

    for (; tktCarrierI != tktCarrierEndI; tktCarrierI++)
    {
      diag << (*tktCarrierI) << " ";
    }
    diag << std::endl;
  }

  diag << "  TVL";
  if (surchInfo.firstTvlDate().isValid())
  {
    diag << " DURING: " << surchInfo.firstTvlDate().dateToIsoExtendedString()
         << " TO: " << surchInfo.lastTvlDate().dateToIsoExtendedString();
  }
  const int startTime = surchInfo.startTime();
  if (isValidTOD(startTime))
  {
    diag << " FROM: " << std::setw(2) << std::setfill('0') << startTime / 60 << ":" << std::setw(2)
         << std::setfill('0') << startTime % 60;
  }
  const int stopTime = surchInfo.stopTime();
  if (isValidTOD(stopTime))
  {
    diag << " TILL: " << std::setw(2) << std::setfill('0') << stopTime / 60 << ":" << std::setw(2)
         << std::setfill('0') << stopTime % 60;
  }
  diag << std::endl;

  if (!surchInfo.dow().empty())
  {
    std::string::const_iterator dowI = surchInfo.dow().begin();
    std::string::const_iterator dowEndI = surchInfo.dow().end();

    diag << " ON DAY OF WEEK: ";
    for (; dowI != dowEndI; dowI++)
    {
      uint32_t dowNum = *dowI - '0';
      if (dowNum == 7)
      {
        dowNum = 0;
      }
      diag << " " << tse::WEEKDAYS_UPPER_CASE[dowNum];
    }
    diag << std::endl;
  }

  const Indicator direction = surchInfo.directionalInd();

  if (direction == FROM)
    diag << "  FROM ";
  else if (direction == BETWEEN)
    diag << "  BETWEEN ";
  else if (direction == WITHIN)
    diag << "  WITHIN ";

  if (!surchInfo.loc1().loc().empty())
  {
    diag << surchInfo.loc1().locType() << " " << surchInfo.loc1().loc();
  }
  if (!surchInfo.loc2().loc().empty())
  {
    if (direction == FROM)
      diag << "  TO  ";
    else if (direction == BETWEEN)
      diag << "  AND  ";
    diag << surchInfo.loc2().locType() << " " << surchInfo.loc2().loc();
  }
  diag << std::endl;

  if (NOT_APPLY != surchInfo.surchargeAppl())
  {
    diag << "  APPLIES IN THE DIRECTION OF THE FARE COMPONENT" << std::endl;
  }

  if (LOCTYPE_NONE != surchInfo.posLoc().locType())
  {
    diag << "  TICKET SOLD AT: " << surchInfo.posLoc().locType() << " " << surchInfo.posLoc().loc()
         << std::endl;
  }
  if (LOCTYPE_NONE != surchInfo.poiLoc().locType())
  {
    diag << "  TICKET ISSUED AT: " << surchInfo.poiLoc().locType() << " "
         << surchInfo.poiLoc().loc() << std::endl;
  }

  diag << " SURCHARGE DETAIL :" << std::endl;
  diag << "  TYPE: " << surchInfo.surchargeType() << "  CUR1: " << surchInfo.surchargeCur1()
       << "  AMT1: " << surchInfo.surchargeAmt1() << "  CUR2: " << surchInfo.surchargeCur2()
       << "  AMT2: " << surchInfo.surchargeAmt2() << "  PERCENT: " << surchInfo.surchargePercent()
       << std::endl;

  diag << " " << std::endl;
}
}
