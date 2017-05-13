#include "Diagnostic/RoutingDiagCollector.h"
#include "Common/RoutingUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "DBAccess/TPMExclusion.h"
#include "Diagnostic/DiagCollector.h"
#include "Routing/RoutingConsts.h"
#include "Routing/TravelRoute.h"
#include "Util/FlatSet.h"

#include <iomanip>
#include <vector>

namespace tse
{

void
RoutingDiagCollector::displayNoRestrictionsMessage()
{
  ((std::ostringstream&)*this) << "    NO RESTRICTIONS" << std::endl;
}

void
RoutingDiagCollector::displayNoFaresMessage()
{
  ((DiagCollector&)*this) << "    NO FARES EXIST IN THIS MARKET\n";
}

void
RoutingDiagCollector::displayTerminalPointMessage(const TravelRoute& tvlRoute,
                                                  const RoutingInfo& rtgInfo)
{
  if (rtgInfo.mapInfo() != nullptr && rtgInfo.mapInfo()->processed())
  {
    if (tvlRoute.terminalPoints())
    {
      *this << "    ROUTING VALIDATION APPLIED TO TERMINAL ON/OFF POINTS ONLY" << std::endl;
    }
  }

  else if (rtgInfo.rtgAddonMapInfo() != nullptr && rtgInfo.rtgAddonMapInfo()->processed())
  {
    *this << "    ROUTING VALIDATION APPLIED TO TERMINAL ON/OFF POINTS ONLY" << std::endl;
  }
}

// ----------------------------------------------------------------------------
//
// Display   ATP   WH  0519 TRF-0099  ROUTING VALID
//
//           MPM   WH  5530 TRF-0001  ROUTING VALID
//
// ----------------------------------------------------------------------------
void
RoutingDiagCollector::displayRoutingStatusDepreciated(const TravelRoute& tvlRoute,
                                                      const RoutingInfo& rtgInfo)
{
  const Routing* routing = rtgInfo.routing();

  globalDirectionToStr(_gd, tvlRoute.globalDir());

  if (rtgInfo.routingStatus())
  {
    _routingStatus = "ROUTING VALID";
  }
  else
  {
    _routingStatus = "ROUTING NOT VALID";
  }

  setRoutingTypes(rtgInfo);

  if (_constructedRouting)
  {
    displayConstructedRoutingDepreciated(tvlRoute, rtgInfo);
  }

  else
  {
    //------------------------------------------------
    // Display Mileage Routing
    //------------------------------------------------
    if (rtgInfo.mileageInfo() != nullptr)
    {
      *this << "    MPM   " << _gd << "  " << std::setw(4)
            << rtgInfo.mileageInfo()->totalApplicableMPM() << " TRF-" << std::setw(4)
            << rtgInfo.routingTariff() << "  " << std::setw(7) << std::setfill(' ')
            << rtgInfo.tcrRoutingTariffCode() << "  " << _routingStatus << std::endl;

      if (rtgInfo.mileageInfo()->surchargeAmt() != 0)
      {
        if (rtgInfo.mileageInfo()->surchargeAmt() == EXCEEDS_MPM)
        {
          *this << "    MILEAGE EXCEEDS MPM  -  TOTAL TPM "
                << rtgInfo.mileageInfo()->totalApplicableTPM() << std::endl;
        }
        else
        {
          *this << "    MILEAGE SURCHARGE APPLIES - " << rtgInfo.mileageInfo()->surchargeAmt()
                << "M TOTAL TPM " << rtgInfo.mileageInfo()->totalApplicableTPM() << std::endl;
        }
      }
    }

    else
    //------------------------------------------------
    // Display Map Routing
    //------------------------------------------------
    {
      if (routing != nullptr)
      {
        *this << "    " << routing->vendor() << "   " << _gd << "  " << std::setw(4)
              << routing->routing() << " TRF-" << std::setw(4) << routing->routingTariff() << "  "
              << std::setw(7) << std::setfill(' ') << rtgInfo.tcrRoutingTariffCode() << "  "
              << _routingStatus << std::endl;
      }
      else
      {
        *this << "    UNABLE TO RETRIEVE ROUTING " << rtgInfo.rtgKey().vendor() << '/'
              << rtgInfo.rtgKey().carrier() << '/' << rtgInfo.rtgKey().routingTariff() << '/'
              << rtgInfo.rtgKey().routingNumber() << std::endl;
      }
    }
  }
}

void
RoutingDiagCollector::displayRoutingStatus(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo)
{
  const Routing* routing = rtgInfo.routing();

  globalDirectionToStr(_gd, tvlRoute.globalDir());

  if (rtgInfo.routingStatus())
  {
    _routingStatus = "ROUTING VALID";
  }
  else
  {
    _routingStatus = "ROUTING NOT VALID";
  }

  setRoutingTypes(rtgInfo);

  if (_constructedRouting)
  {
    displayConstructedRouting(tvlRoute, rtgInfo);
  }
  else
  {
    // Display Mileage Routing
    if (rtgInfo.mileageInfo() != nullptr)
    {
      *this << "    MPM   " << _gd << "  " << std::setw(4)
            << rtgInfo.mileageInfo()->totalApplicableMPM() << " TRF-" << std::setw(4)
            << rtgInfo.routingTariff() << "  " << std::setw(7) << std::setfill(' ')
            << rtgInfo.tcrRoutingTariffCode() << "  " << _routingStatus << std::endl;

      if (rtgInfo.mileageInfo()->surchargeAmt() != 0)
      {
        if (rtgInfo.mileageInfo()->surchargeAmt() == EXCEEDS_MPM)
        {
          *this << "    MILEAGE EXCEEDS MPM  -  TOTAL TPM "
                << rtgInfo.mileageInfo()->totalApplicableTPM() << std::endl;
        }
        else
        {
          *this << "    MILEAGE SURCHARGE APPLIES - " << rtgInfo.mileageInfo()->surchargeAmt()
                << "M TOTAL TPM " << rtgInfo.mileageInfo()->totalApplicableTPM() << std::endl;
        }
      }
    }
    else
    // Display Map Routing
    {
      if (routing != nullptr)
      {
        *this << "    " << routing->vendor() << "   " << _gd << "  " << std::setw(4)
              << routing->routing() << " TRF-" << std::setw(4) << routing->routingTariff() << "  "
              << std::setw(7) << std::setfill(' ') << rtgInfo.tcrRoutingTariffCode() << "  "
              << _routingStatus << std::endl;
      }
      else
      {
        *this << "    UNABLE TO RETRIEVE ROUTING " << rtgInfo.rtgKey().vendor() << '/'
              << rtgInfo.rtgKey().carrier() << '/' << rtgInfo.rtgKey().routingTariff() << '/'
              << rtgInfo.rtgKey().routingNumber() << std::endl;
      }
    }
  }
}

// ----------------------------------------------------------------------------
//  Display Missing City or Carrier Message when Map Validation Fails
//
//  * DFW-AA-CHI NOT FOUND ON ROUTE MAP
//
// ----------------------------------------------------------------------------
void
RoutingDiagCollector::displayMissingCity(std::vector<TravelRoute::CityCarrier> cityCarrierVec,
                                         const RoutingInfo& rtgInfo,
                                         bool postDRVDisplay)
{
  if (rtgInfo.mapInfo() == nullptr || cityCarrierVec.empty())
  {
    return;
  }

  int16_t cityIndex = 0;
  if (postDRVDisplay)
  {
    cityIndex = rtgInfo.mapInfo()->postDRVmissingCityIndex();
  }
  else
  {
    cityIndex = rtgInfo.mapInfo()->missingCityIndex();
  }

  int16_t cityCxrVecSize = cityCarrierVec.size();
  if (cityIndex > cityCxrVecSize || cityIndex < 0)
  {
    return;
  }

  LocCode missingCityOrig = cityCarrierVec[cityIndex].boardCity().loc();
  LocCode missingCityDest = cityCarrierVec[cityIndex].offCity().loc();
  CarrierCode missingCarrier = cityCarrierVec[cityIndex].carrier();

  if (missingCarrier == SURFACE_CARRIER)
  {
    *this << "    * " << missingCityDest << " NOT FOUND ON ROUTE MAP" << std::endl;
  }

  else
  {
    *this << "    * " << missingCityOrig << "-" << missingCarrier << "-" << missingCityDest
          << " NOT FOUND ON ROUTE MAP" << std::endl;
  }
}

//----------------------------------------------------------------------------
// Display Routing Restrictions
//----------------------------------------------------------------------------
void
RoutingDiagCollector::displayRestrictions(const RoutingInfo& rtgInfo)
{
  bool messageDisplayed = false;

  // Display Routing Restrictions and Pass/Fail Status
  if (rtgInfo.restrictions() == nullptr)
  {
    displayNoRestrictionsMessage();
    return;
  }

  std::map<std::string, std::vector<const RoutingRestriction*>> nationPairMap;

  if (isRtw())
  {
    for (const auto& ri : *rtgInfo.restrictions())
    {
      const RoutingRestriction* restriction = ri.first;

      if (restriction->restriction() == RTW_ROUTING_RESTRICTION_12)
        RoutingController::groupRestriction12(
            *static_cast<PricingTrx*>(_trx), restriction, nationPairMap);
    }
  }

  for (const auto& ri : *rtgInfo.restrictions())
  {
    const RoutingRestriction* restriction = ri.first;
    const RestrictionInfo& restInfo = ri.second;
    if (restInfo.base() || restInfo.origAddOn() || restInfo.destAddOn())
    {
      continue;
    }
    else
    {
      if (!(messageDisplayed) && (rtgInfo.origAddOnRouting() || rtgInfo.destAddOnRouting()))
      {
        *this << "    THESE RESTRICTIONS APPLY TO THE ENTIRE ROUTE OF TRAVEL" << std::endl;
        messageDisplayed = true;
      }

      if (restriction->restriction() == RTW_ROUTING_RESTRICTION_12)
      {
        for (const auto& restr12 : nationPairMap)
        {
          if (restr12.second.front() == restriction)
          {
            displayRestrictionHeader(*restr12.second.front(), restInfo);
            FlatSet<std::pair<Indicator, LocCode>> cityGroup1, cityGroup2;
            RoutingController::createCityGroups(restr12.second, cityGroup1, cityGroup2);
            displayRestriction12(cityGroup1, cityGroup2);
          }
        }
      }
      else
        displayRestriction(*restriction, restInfo);
    }
  }

  // Display Blank Line following restrictions
  *this << " " << std::endl;
}

//-----------------------------------------------------
// Display Routing Restrictions applicable to the Base
//-----------------------------------------------------
void
RoutingDiagCollector::displayBaseRestrictions(const RoutingInfo& rtgInfo)
{
  bool cityCarriersDisplayed = false;
  CarrierCode cxr = "  ";

  // Display Routing Restrictions and Pass/Fail Status
  if (rtgInfo.restrictions() == nullptr)
  {
    return;
  }

  const RestrictionInfos* resInfos = rtgInfo.restrictions();
  RestrictionInfos::const_iterator resInfosItr;

  for (resInfosItr = resInfos->begin(); resInfosItr != resInfos->end(); ++resInfosItr)
  {
    const RoutingRestriction* restriction = (*resInfosItr).first;
    const RestrictionInfo& restInfo = (*resInfosItr).second;

    if (!restInfo.base())
    {
      continue;
    }
    else
    {
      if (!cityCarriersDisplayed)
      {
        *this << "    ";
        displayCityCarriers(cxr, rtgInfo.restBaseCityCarrier());
        cityCarriersDisplayed = true;
      }
      displayConstructedRestriction(*restriction, restInfo.valid());
    }
  }
}

//-----------------------------------------------------
// Display Restrictions applicable to the Origin Addon
//-----------------------------------------------------
void
RoutingDiagCollector::displayOrigAddonRestrictions(const RoutingInfo& rtgInfo)
{
  bool cityCarriersDisplayed = false;
  CarrierCode cxr = "  ";

  // Display Routing Restrictions and Pass/Fail Status
  if (rtgInfo.restrictions() == nullptr)
  {
    return;
  }

  const RestrictionInfos* resInfos = rtgInfo.restrictions();
  RestrictionInfos::const_iterator resInfosItr;

  for (resInfosItr = resInfos->begin(); resInfosItr != resInfos->end(); ++resInfosItr)
  {
    const RoutingRestriction* restriction = (*resInfosItr).first;
    const RestrictionInfo& restInfo = (*resInfosItr).second;

    if (!restInfo.origAddOn())
    {
      continue;
    }
    else
    {
      if (!cityCarriersDisplayed)
      {
        *this << "    ";
        displayCityCarriers(cxr, rtgInfo.rtgAddon1CityCarrier());
        cityCarriersDisplayed = true;
      }
      displayConstructedRestriction(*restriction, restInfo.valid());
    }
  }
}

//------------------------------------------------------------------
// Display Routing Restrictions applicable to the destination addon
//------------------------------------------------------------------
void
RoutingDiagCollector::displayDestAddonRestrictions(const RoutingInfo& rtgInfo)
{
  bool cityCarriersDisplayed = false;
  CarrierCode cxr = "  ";

  // Display Routing Restrictions and Pass/Fail Status
  if (rtgInfo.restrictions() == nullptr)
  {
    return;
  }

  const RestrictionInfos* resInfos = rtgInfo.restrictions();
  RestrictionInfos::const_iterator resInfosItr;

  for (resInfosItr = resInfos->begin(); resInfosItr != resInfos->end(); ++resInfosItr)
  {
    const RoutingRestriction* restriction = (*resInfosItr).first;
    const RestrictionInfo& restInfo = (*resInfosItr).second;

    if (!restInfo.destAddOn())
    {
      continue;
    }
    else
    {
      if (!cityCarriersDisplayed)
      {
        *this << "    ";
        displayCityCarriers(cxr, rtgInfo.rtgAddon2CityCarrier());
        cityCarriersDisplayed = true;
      }
      displayConstructedRestriction(*restriction, restInfo.valid());
    }
  }
}

//----------------------------------------------------------------------------
// Indicate whether the Mileage Routing Passed, Failed, or was not Validated
//----------------------------------------------------------------------------
void
RoutingDiagCollector::displayMileageMessages(const RoutingInfo& rtgInfo)
{
  if (rtgInfo.mileageInfo() == nullptr)
  {
    return;
  }

  std::vector<Market>::const_iterator s =
      rtgInfo.mileageInfo()->surfaceSectorExemptCities().begin();
  std::vector<Market>::const_iterator end =
      rtgInfo.mileageInfo()->surfaceSectorExemptCities().end();

  while (s < end)
  {
    *this << "    SURFACE SECTOR MILEAGE EXEMPT BETWEEN " << (*s).first << " AND " << (*s).second
          << std::endl;
    s++;
  }

  if (rtgInfo.mileageInfo()->southAtlanticTPMExclusion())
  {
    if (rtgInfo.mileageInfo()->tpmExclusion())
    {
      *this << "    TPM EXCLUSION APPLIED ";
      displayMarketVector(rtgInfo.mileageInfo()->southAtlanticTPDCities());
      *this << "\n    RECORD " << rtgInfo.mileageInfo()->tpmExclusion()->carrier() << " / SEQ NO "
            << rtgInfo.mileageInfo()->tpmExclusion()->seqNo()
            << "      MATCHED - CHECK DIAGNOSTIC 452";
    }
    else
    {
      *this << "    SOUTH ATLANTIC TPM EXCLUSION APPLIED ";
      displayMarketVector(rtgInfo.mileageInfo()->southAtlanticTPDCities());
    }

    *this << std::endl;
  }

  if (rtgInfo.mileageInfo()->tpd() != 0)
  {
    *this << "    TICKETED POINT DEDUCTION " << rtgInfo.mileageInfo()->tpd() << " APPLIED"
          << std::endl;
  }

  if (rtgInfo.mileageInfo()->mileageEqualizationApplies())
  {
    *this << "    MILEAGE EQUALIZATION APPLIED. SURCHG REDUCED FROM "
          << rtgInfo.mileageInfo()->equalizationSurcharges().first << " TO "
          << rtgInfo.mileageInfo()->equalizationSurcharges().second << std::endl;
  }

  if (rtgInfo.restrictions() == nullptr)
  {
    if (rtgInfo.mileageInfo()->processed() && rtgInfo.mileageInfo()->valid())
    {
      *this << "    MILEAGE VALIDATION PASSED" << std::endl;
    }
    else if (rtgInfo.mileageInfo()->processed() && !rtgInfo.mileageInfo()->valid())
    {
      *this << "    MILEAGE VALIDATION FAILED" << std::endl;
    }
  }
}

void
RoutingDiagCollector::displayMarketVector(const std::vector<Market>& markets)
{
  std::vector<Market>::const_iterator t = markets.begin();
  std::vector<Market>::const_iterator end = markets.end();

  *this << (*t).first;

  while (t < end)
  {
    *this << "-" << (*t).second;
    t++;
  }
}

//----------------------------------------------------------------------------
// Display Travel Routes for constructed routings
//----------------------------------------------------------------------------
bool
RoutingDiagCollector::displayMapMessages(const RoutingInfo& rtgInfo)
{
  bool mapValidationFailed = false;

  if (!rtgInfo.mapInfo() == 0)
  {
    const Routing* origAddOnRtg = rtgInfo.origAddOnRouting();
    const Routing* destAddOnRtg = rtgInfo.destAddOnRouting();
    const Routing* routing = rtgInfo.routing();

    if (origAddOnRtg == nullptr || origAddOnRtg->routing() != MILEAGE_ROUTING)
    {
      if (destAddOnRtg == nullptr || destAddOnRtg->routing() != MILEAGE_ROUTING)
      {
        if (routing != nullptr && routing->routing() != MILEAGE_ROUTING &&
            !routing->rmaps().empty())
        {
          //---------------------------------------------------------------
          // Display this message only for a base routing or constructed
          // routing with no mileage addons.
          //---------------------------------------------------------------
          mapValidationFailed = displayMapResults(rtgInfo.mapInfo(), rtgInfo);
        }
      }
    }
  }

  else
  {
    *this << "    NO ROUTE MAP EXISTS" << std::endl;
  }

  return mapValidationFailed;
}

//----------------------------------------------------------------------------
// Display Travel Routes for constructed routings
// Return true only if map validation failed
//----------------------------------------------------------------------------
bool
RoutingDiagCollector::displayMapResults(const MapInfo* mapInfo, const RoutingInfo& rtgInfo)
{
  if (mapInfo != nullptr)
  {
    if (mapInfo->processed() && mapInfo->valid())
    {
      //------------------------------------------------------------
      // If DRVInfos exist, initial map validation must have failed
      //------------------------------------------------------------
      if (!mapInfo->drvInfos().empty())
      {
        *this << "    ROUTE MAP EXISTS - MAP VALIDATION FAILED" << std::endl;
        return true;
      }
      else
      {
        *this << "    ROUTE MAP EXISTS - MAP VALIDATION PASSED" << std::endl;
      }
    }
    else if (mapInfo->processed() && !(mapInfo->valid()))
    {
      *this << "    ROUTE MAP EXISTS - MAP VALIDATION FAILED" << std::endl;
      return true;
    }
    else if (!mapInfo->processed())
    {
      if (rtgInfo.mileageInfo() && rtgInfo.mileageInfo()->processed() &&
          !(rtgInfo.mileageInfo()->valid()))
      {
        *this << "    ROUTE MAP EXISTS BUT WAS NOT VALIDATED DUE TO MILEAGE" << std::endl;
      }
      else
      {
        *this << "    ROUTE MAP EXISTS BUT WAS NOT VALIDATED DUE TO RESTS" << std::endl;
      }
    }
  }

  return false; // Map Validation passed or was not processed
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void RoutingDiagCollector::displayTravelRoute from a vector
//                                      of CityCarriers
//
// AA    MEM-AA-CHI-AA-MIA
//
// Description:  Display the CityCarriers contained in the TravelRoute
//
// @param  TravelRoute passenger's route of travel
//
// </PRE>
// ----------------------------------------------------------------------------
void
RoutingDiagCollector::displayCityCarriers(const CarrierCode& carrier,
                                          std::vector<TravelRoute::CityCarrier> cityCarrierVec)
{
  bool hiddenCityFound = false;
  int16_t charCount = 9;

  std::vector<TravelRoute::CityCarrier>::const_iterator tri = cityCarrierVec.begin();
  std::vector<TravelRoute::CityCarrier>::const_iterator trEnd = cityCarrierVec.end();

  uint16_t lineLength = 59;

  if (tri != trEnd)
  {
    // Display Governing Carrier
    *this << std::setw(2) << carrier << "  ";

    // Display Board City
    *this << std::setw(3) << (*tri).boardCity().loc();
    for (; tri != trEnd; ++tri)
    {
      // Display Carrier or Surface Indicator
      if ((charCount + 4) >= lineLength)
      {
        *this << "\n    "; // Start a new line indented
        charCount = 4;
      }
      charCount += 4;
      if ((*tri).carrier() == SURFACE_CARRIER)
      {
        *this << " // ";
      }
      else
      {
        *this << "-" << std::setw(2) << (*tri).carrier() << "-";
      }

      // Display Off City
      if ((charCount + 4) >= lineLength)
      {
        *this << "\n    "; // Start a new line indented
        charCount = 4;
      }
      if ((*tri).offCity().isHiddenCity())
      {
        *this << "*";
        hiddenCityFound = true;
        charCount += 1;
      }
      *this << std::setw(3) << (*tri).offCity().loc();
      charCount += 3;
    }

    // Display Trailer Message for Flight Tracking
    if (hiddenCityFound)
    {
      *this << "\n      * INTERMEDIATE NON-TICKETED POINTS TRACKED ON ROUTING";
    }

    *this << std::endl;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void RoutingDiagCollector::displayConstructedRestriction
//
// Description:  Display Routing Restrictions applicable to portions of a
//               constructed routing.
//
// @param  Restricton
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
RoutingDiagCollector::displayConstructedRestriction(const RoutingRestriction& restriction,
                                                    bool restrictionStatus)
{
  RestrictionNumber restrictionNumber = restriction.restriction();
  std::string status;
  if (restrictionStatus)
  {
    status = "RESTRICTION PASSED";
  }
  else
  {
    status = "RESTRICTION FAILED";
  }

  *this << "        RESTRICTION " << restrictionNumber << "                " << status << std::endl;

  const char* ptr = restrictionNumber.c_str();
  int16_t restNo = atoi(ptr);

  switch (restNo)
  {
  case 3:
  {
    *this << "    ";
    displayRestriction3(restriction);
    break;
  }
  case 17:
  {
    *this << "    ";
    displayRestriction17(restriction);
    break;
  }
  default:
    break;
  }
}

void
RoutingDiagCollector::displayRestrictionHeader(const RoutingRestriction& restriction,
                                               const RestrictionInfo& restrictionStatus)
{
  std::string status;
  if (!restrictionStatus.processed())
  {
    status = "NOT PROCESSED";
  }
  else if (restrictionStatus.valid())
  {
    status = "RESTRICTION PASSED";
  }
  else
  {
    status = "RESTRICTION FAILED";
  }

  *this << "    RESTRICTION " << restriction.restriction() << "                    " << status
        << std::endl;
}

void
RoutingDiagCollector::displayRestriction(const RoutingRestriction& restriction,
                                         const RestrictionInfo& restrictionStatus)
{
  displayRestrictionHeader(restriction, restrictionStatus);

  const char* ptr = restriction.restriction().c_str();
  int16_t restNo = atoi(ptr);

  switch (restNo)
  {
  case 1:
    displayRestriction1(restriction);
    break;
  case 2:
    displayRestriction2(restriction);
    break;
  case 3:
    displayRestriction3(restriction);
    break;
  case 4:
    displayRestriction4(restriction);
    break;
  case 5:
    displayRestriction5(restriction);
    break;
  case 6:
    displayRestriction6(restriction);
    break;
  case 7:
    displayRestriction7(restriction);
    break;
  case 8:
    displayRestriction8(restriction);
    break;
  case 9:
    displayRestriction9(restriction);
    break;
  case 10:
    displayRestriction10(restriction);
    break;
  case 11:
    displayRestriction11(restriction);
    break;
  case 12:
    break;
  case 13:
    displayRestriction13(restriction);
    break;
  case 14:
    displayRestriction14(restriction);
    break;
  case 15:
    displayRestriction15(restriction);
    break;
  case 16:
    displayRestriction16(restriction);
    break;
  case 17:
    displayRestriction17(restriction);
    break;
  case 18:
    displayRestriction18(restriction);
    break;
  case 19:
    displayRestriction19(restriction);
    break;
  case 21:
    displayRestriction21(restriction);
    break;
  default:
    break;
  }
}

//--------------------------------------------------------
//  RESTRICTION 1
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CITY
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST NOT BE VIA CITY
//--------------------------------------------------------
void
RoutingDiagCollector::displayRestriction1(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    TRAVEL BETWEEN " << restriction.market1() << " AND " << restriction.market2();

  if (restriction.negViaAppl() == REQUIRED)
  {
    *this << " MUST BE VIA " << restriction.viaMarket() << std::endl;
  }
  else // Assume NOT PERMITTED
  {
    *this << " MUST NOT BE VIA " << restriction.viaMarket() << std::endl;
  }
}

//--------------------------------------------------------
//  RESTRICTION 2
//  TRAVEL MUST BE VIA CITY
//  TRAVEL MUST NOT BE VIA CITY
//--------------------------------------------------------
void
RoutingDiagCollector::displayRestriction2(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    TRAVEL";

  if ((restriction.negViaAppl() == PERMITTED) || (restriction.negViaAppl() == REQUIRED))
  {
    *this << " MUST BE VIA " << restriction.viaMarket() << std::endl;
  }
  else // Assume NOT PERMITTED
  {
    *this << " MUST NOT BE VIA " << restriction.viaMarket() << std::endl;
  }
}

//--------------------------------------------------------
//  RESTRICTION 3
//  TRAVEL MUST BE NONSTOP
//  TRAVEL MUST NOT BE NONSTOP
//  TRAVEL MUST BE DIRECT
//  TRAVEL MUST NOT BE DIRECT
//  TRAVEL MUST BE NONSTOP OR DIRECT
//  TRAVEL MUST NOT BE NONSTOP OR DIRECT
//--------------------------------------------------------
void
RoutingDiagCollector::displayRestriction3(const RoutingRestriction& restriction)
{
  if (isRtw(false))
    return;
  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    TRAVEL";

  if ((restriction.negViaAppl() == PERMITTED) || (restriction.negViaAppl() == REQUIRED))
  {
    *this << " MUST BE ";
  }
  else // Assume NOT PERMITTED
  {
    *this << " MUST NOT BE ";
  }
  if (restriction.nonStopDirectInd() == NONSTOP)
  {
    *this << "NONSTOP" << std::endl;
  }
  else
  {
    if (restriction.nonStopDirectInd() == DIRECT)
    {
      *this << "DIRECT" << std::endl;
    }
    else
    {
      *this << "NONSTOP OR DIRECT" << std::endl;
    }
  }
}

//--------------------------------------------------------
//  RESTRICTION 4
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE NONSTOP
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST NOT BE NONSTOP
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE DIRECT
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST NOT BE DIRECT
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE NONSTOP OR DIRECT
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST NOT BE NONSTOP OR DIRECT
//--------------------------------------------------------
void
RoutingDiagCollector::displayRestriction4(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    TRAVEL BETWEEN " << restriction.market1() << " AND " << restriction.market2();

  if ((restriction.negViaAppl() == PERMITTED) || (restriction.negViaAppl() == REQUIRED))
  {
    *this << " MUST BE ";
  }
  else // Assume NOT PERMITTED
  {
    *this << " MUST NOT BE ";
  }
  if (restriction.nonStopDirectInd() == NONSTOP)
  {
    *this << "NONSTOP" << std::endl;
  }
  else
  {
    if (restriction.nonStopDirectInd() == DIRECT)
    {
      *this << "DIRECT" << std::endl;
    }
    else
    {
      *this << "NONSTOP OR DIRECT" << std::endl;
    }
  }
}

//--------------------------------------------------------
//  RESTRICTION 5
//  TRAVEL TO/FROM CITY1 MUST BE VIA CITY3
//  TRAVEL TO/FROM CITY1 MUST NOT BE VIA CITY3
//--------------------------------------------------------
void
RoutingDiagCollector::displayRestriction5(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    TRAVEL TO/FROM " << restriction.market1();

  if ((restriction.negViaAppl() == PERMITTED) || (restriction.negViaAppl() == REQUIRED))
  {
    *this << " MUST BE VIA " << restriction.viaMarket() << std::endl;
  }
  else // Assume NOT PERMITTED
  {
    *this << " MUST NOT BE VIA " << restriction.viaMarket() << std::endl;
  }
}

//--------------------------------------------------------
//  RESTRICTION 6
//  TRAVEL TO/FROM CITY1 MUST BE NONSTOP
//  TRAVEL TO/FROM CITY1 MUST NOT BE NONSTOP
//  TRAVEL TO/FROM CITY1 MUST BE DIRECT
//  TRAVEL TO/FROM CITY1 MUST NOT BE DIRECT
//  TRAVEL TO/FROM CITY1 MUST BE NONSTOP OR DIRECT
//  TRAVEL TO/FROM CITY1 MUST NOT BE NONSTOP OR DIRECT
//--------------------------------------------------------
void
RoutingDiagCollector::displayRestriction6(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    TRAVEL TO/FROM " << restriction.market1();

  if ((restriction.negViaAppl() == PERMITTED) || (restriction.negViaAppl() == REQUIRED))
  {
    *this << " MUST BE ";
  }
  else // Assume NOT PERMITTED
  {
    *this << " MUST NOT BE ";
  }

  if (restriction.nonStopDirectInd() == NONSTOP)
  {
    *this << "NONSTOP" << std::endl;
  }
  else
  {
    if (restriction.nonStopDirectInd() == DIRECT)
    {
      *this << "DIRECT" << std::endl;
    }
    else
    {
      *this << "NONSTOP OR DIRECT" << std::endl;
    }
  }
}

//----------------------------------------------------------
//  RESTRICTION 7
//  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 REQUIRED
//  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 PERMITTED
//  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 NOT PERMITTED
//----------------------------------------------------------
void
RoutingDiagCollector::displayRestriction7(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    BETWEEN " << restriction.market1() << " AND " << restriction.market2();
  *this << " STOPOVER IN " << restriction.viaMarket();

  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << " PERMITTED" << std::endl;
  }
  else if (restriction.negViaAppl() == REQUIRED)
  {
    *this << " REQUIRED" << std::endl;
  }
  else
  {
    *this << " NOT PERMITTED" << std::endl;
  }
}

bool
RoutingDiagCollector::isRtw()
{
  PricingTrx* trx = dynamic_cast<PricingTrx*>(_trx);

  return trx && trx->getOptions() && trx->getOptions()->isRtw();
}

bool
RoutingDiagCollector::isRtw(bool expectRtw)
{
  bool ret = isRtw();

  if (ret != expectRtw)
  {
    *this << "    RESTRICTION IGNORED ";
    if (!ret)
      *this << "- NON-";
    *this << "RTW/CT" << std::endl;
  }

  return ret;
}

void
RoutingDiagCollector::displayRestriction8(const RoutingRestriction& restriction)
{
  if (isRtw(true))
    *this << "    MPM: " << restriction.mpm() << std::endl;
}

void
RoutingDiagCollector::displayRestriction9(const RoutingRestriction& /*restriction*/)
{
  if (isRtw(true))
    *this << "    TRAVEL IS NOT PERMITTED VIA THE FARE ORIGIN" << std::endl;
}

void
RoutingDiagCollector::displayRestriction10(const RoutingRestriction& /*restriction*/)
{
  if (isRtw(true))
    *this << "    TRAVEL CANNOT CONTAIN MORE THAN ONE COUPON BETWEEN\n"
          << "    THE SAME POINTS IN THE SAME DIRECTION" << std::endl;
}

//-------------------------------------------------------------------
//  RESTRICTION 11
//  BETWEEN CITY1 AND CITY2 AIR SECTOR REQUIRED
//  BETWEEN CITY1 AND CITY2 AIR SECTOR PERMITTED
//  BETWEEN CITY1 AND CITY2 AIR SECTOR NOT PERMITTED
//  BETWEEN CITY1 AND CITY2 SURFACE SECTOR REQUIRED
//  BETWEEN CITY1 AND CITY2 SURFACE SECTOR PERMITTED
//  BETWEEN CITY1 AND CITY2 SURFACE SECTOR NOT PERMITTED
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction11(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    BETWEEN " << restriction.market1() << " AND " << restriction.market2();

  if (restriction.airSurfaceInd() == AIR)
  {
    *this << " AIR SECTOR";
  }
  else if (restriction.airSurfaceInd() == SURFACE)
  {
    *this << " SURFACE SECTOR";
  }
  else
  {
    *this << " AIR OR SURFACE SECTOR";
  }

  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << " PERMITTED" << std::endl;
  }
  else if (restriction.negViaAppl() == REQUIRED)
  {
    *this << " REQUIRED" << std::endl;
  }
  else
  {
    *this << " NOT PERMITTED" << std::endl;
  }
}

//-------------------------------------------------------------------
//  RESTRICTION 12
//  ONLY ONE SINGLE COUPON TRAVEL IS PERMITTED BETWEEN CITY1 and CITY2
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction12(const RoutingRestriction& restriction)
{
  if (isRtw(true))
    *this << "    ONLY ONE INSTANCE OF NONSTOP SINGLE COUPON TRAVEL \n"
             "    IS PERMITTED BETWEEN " << restriction.market1() << " AND "
          << restriction.market2() << std::endl;
}

namespace
{
class TruncateLineByCities
{
  static const int CAPACITY = 15;
  int _pos;
  bool _first;
  RoutingDiagCollector& _parent;

public:
  TruncateLineByCities(int pos, RoutingDiagCollector& parent)
    : _pos(pos), _first(true), _parent(parent)
  {
  }

  void reset() { _first = true; }

  void print(const LocCode& loc)
  {
    if (!_first)
      _parent << '/';
    if (++_pos > CAPACITY)
    {
      _parent << "\n    ";
      _pos = 1;
    }
    _parent << loc;
    _first = false;
  }
};
}

void
RoutingDiagCollector::displayRestriction12(const FlatSet<std::pair<Indicator, LocCode>>& cityGroup1,
                                           const FlatSet<std::pair<Indicator, LocCode>>& cityGroup2)
{
  if (isRtw(true))
  {
    *this << "    ONLY ONE INSTANCE OF NONSTOP SINGLE COUPON TRAVEL \n"
             "    IS PERMITTED BETWEEN ";

    TruncateLineByCities tlbc(6, *this);

    for (const auto& city : cityGroup1)
    {
      tlbc.print(city.second);
    }
    tlbc.reset();
    tlbc.print(" AND ");
    tlbc.reset();
    for (const auto& city : cityGroup2)
    {
      tlbc.print(city.second);
    }
    *this << std::endl;
  }
}

//-------------------------------------------------------------------
//  RESTRICTION 13
//  BETWEEN CTY AND CTY AIR SECTOR REQUIRED AT ADDL EXP
//  BETWEEN CTY AND CTY AIR SECTOR PERMITTED AT ADDL EXP
//  BETWEEN CTY AND CTY AIR SECTOR NOT PERMITTED AT ADDL EXP
//  BETWEEN CTY AND CTY SURFACE SECTOR REQUIRED AT ADDL EXP
//  BETWEEN CTY AND CTY SURFACE SECTOR PERMITTED AT ADDL EXP
//  BETWEEN CTY AND CTY SURFACE SECTOR NOT PERMITTED AT ADDL EXP
//  BETWEEN CTY AND CTY AIR OR SURFACE REQUIRED AT ADDL EXP
//  BETWEEN CTY AND CTY AIR OR SURFACE PERMITTED AT ADDL EXP
//  BETWEEN CTY AND CTY AIR OR SURFACE NOT PERMITTED AT ADDL EXP
//  123456789012345678901234567890123456789012345678901234567890123
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction13(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == REQUIRED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS REQUIRED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == NOT_PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR - NOT PERMITTED" << std::endl;
    return;
  }

  *this << "    BETWEEN " << restriction.market1() << " AND " << restriction.market2();

  if (restriction.airSurfaceInd() == AIR)
  {
    *this << " AIR SECTOR";
  }
  else if (restriction.airSurfaceInd() == SURFACE)
  {
    *this << " SURFACE SECTOR";
  }
  else
  {
    *this << " AIR OR SURFACE";
  }

  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << " PERMITTED";
  }
  else if (restriction.negViaAppl() == REQUIRED)
  {
    *this << " REQUIRED";
  }
  else if (restriction.negViaAppl() == NOT_PERMITTED)
  {
    *this << " NOT PERMITTED";
  }
  // Default to Permitted
  else
  {
    *this << " PERMITTED";
  }

  *this << " AT ADDL EXP" << std::endl;
}

//-------------------------------------------------------------------
//  RESTRICTION 14
//  BETWEEN CITY1 AND CITY2 NO LOCAL TRAFFIC PERMITTED
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction14(const RoutingRestriction& restriction)
{
  *this << "    BETWEEN " << restriction.market1() << " AND " << restriction.market2();
  *this << " NO LOCAL TRAFFIC PERMITTED" << std::endl;
}

//-------------------------------------------------------------------
//  RESTRICTION 15
//  BAGGAGE MUST BE CHECKED THROUGH TO DESTINATION ONLY
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction15(const RoutingRestriction& restriction)
{
  *this << "    BAGGAGE MUST BE CHECKED THROUGH TO DESTINATION ONLY" << std::endl;
}

//-------------------------------------------------------------------
//  RESTRICTION 16
//  MAXIMUM PERMITTED MILEAGE APPLIES
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction16(const RoutingRestriction& restriction)
{
  if (!isRtw(false))
  {
    *this << "    MAXIMUM PERMITTED MILEAGE "
          << "APPLIES" << std::endl;
  }
}

//-------------------------------------------------------------------
//  RESTRICTION 17
//  CARRIER LISTING ONLY
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction17(const RoutingRestriction& restriction)
{
  if (restriction.viaCarrier().empty())
  {
    *this << "    CARRIER LISTING ONLY" << std::endl;
  }
  else
  {
    *this << "    CARRIER LISTING ONLY - " << restriction.viaCarrier() << std::endl;
  }
}

//-------------------------------------------------------------------
//  RESTRICTION 18
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CARRIER
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST NOT BE VIA CARRIER
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction18(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    TRAVEL BETWEEN " << restriction.market1() << " AND " << restriction.market2();

  if ((restriction.negViaAppl() == PERMITTED) || (restriction.negViaAppl() == REQUIRED))
  {
    *this << " MUST BE VIA " << restriction.viaCarrier() << std::endl;
  }
  else // Assume NOT PERMITTED
  {
    *this << " MUST NOT BE VIA " << restriction.viaCarrier() << std::endl;
  }
}

//-------------------------------------------------------------------
//  RESTRICTION 19
//  TRAVEL TO/FROM CITY1 MUST BE VIA CARRIER
//  TRAVEL TO/FROM CITY1 MUST NOT BE VIA CARRIER
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction19(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    TRAVEL TO/FROM " << restriction.market1();

  if ((restriction.negViaAppl() == PERMITTED) || (restriction.negViaAppl() == REQUIRED))
  {
    ((std::ostringstream&)*this) << " MUST BE VIA " << restriction.viaCarrier() << std::endl;
  }
  else // Assume NOT PERMITTED
  {
    *this << " MUST NOT BE VIA " << restriction.viaCarrier() << std::endl;
  }
}

//-------------------------------------------------------------------
//  RESTRICTION 21
//  WHEN ORIGIN IS CTY AND DEST IS CTY TRAVEL MUST BE VIA CITY3
//  WHEN ORIGIN IS CTY AND DEST IS CTY TRAVEL MUST NOT BE VIA CITY3
//  123456789012345678901234567890123456789012345678901234567890123
//-------------------------------------------------------------------
void
RoutingDiagCollector::displayRestriction21(const RoutingRestriction& restriction)
{
  if (restriction.negViaAppl() == PERMITTED)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED" << std::endl;
    return;
  }

  if (restriction.negViaAppl() == BLANK)
  {
    *this << "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK" << std::endl;
    return;
  }

  *this << "    WHEN ORIGIN IS " << restriction.market1();
  *this << " AND DEST IS " << restriction.market2();
  *this << " TRAVEL";

  if ((restriction.negViaAppl() == PERMITTED) || (restriction.negViaAppl() == REQUIRED))
  {
    *this << " MUST BE VIA " << restriction.viaMarket() << std::endl;
  }
  else // Assume NOT PERMITTED
  {
    *this << " MUST NOT BE VIA " << restriction.viaMarket() << std::endl;
  }
}

// ----------------------------------------------------------------------------
// When the routing is constructed, set an indicator, and determine each type
// of addon component involved:
//
// Map
// Mileage
// SITA Mileage (No Routing)  Needed ????
// Restriction  (No Map)
//
// ----------------------------------------------------------------------------
void
RoutingDiagCollector::setRoutingTypes(const RoutingInfo& rtgInfo)
{
  _constructedRouting = false;
  _noOrigAddon = false;
  _origAddonMileage = false;
  _origAddonMap = false;
  _origAddonRestriction = false;
  _baseMileage = false;
  _baseMap = false;
  _baseRestriction = false;
  _noDestAddon = false;
  _destAddonMileage = false;
  _destAddonMap = false;
  _destAddonRestriction = false;

  const Routing* routing = rtgInfo.routing();
  const Routing* origAddOnRtg = rtgInfo.origAddOnRouting();
  const Routing* destAddOnRtg = rtgInfo.destAddOnRouting();

  //-----------------------------------------------------------------
  // Set type of Base Routing:  Mileage, Map, or Restriction
  //-----------------------------------------------------------------
  if (routing)
  {
    if (routing->routing() == MILEAGE_ROUTING)
    {
      _baseMileage = true;
    }
    else if (routing->rmaps().empty())
    {
      _baseRestriction = true;
    }
    else
    {
      _baseMap = true;
    }
  }
  else
  {
    _baseMileage = true;
  }

  if (origAddOnRtg || destAddOnRtg)
  {
    _constructedRouting = true;
  }

  //-----------------------------------------------------------------
  // Set type of origin Addon Routing:  Mileage, Map, or Restriction
  //-----------------------------------------------------------------
  if (origAddOnRtg)
  {
    if (origAddOnRtg->routing() == MILEAGE_ROUTING)
    {
      _origAddonMileage = true;
    }
    else if (origAddOnRtg->rmaps().empty())
    {
      _origAddonRestriction = true;
    }
    else
    {
      _origAddonMap = true;
    }
  }
  else if ((!rtgInfo.origAddOnInterior().empty()) || (!rtgInfo.origAddOnGateway().empty()))
  {
    _constructedRouting = true;
    if (rtgInfo.rtgKey().addOnRouting1().empty() ||
        rtgInfo.rtgKey().addOnRouting1() == MILEAGE_ROUTING)
    {
      _origAddonMileage = true;
    }
  }
  else
  {
    _noOrigAddon = true;
  }

  //----------------------------------------------------------------------
  // Set type of destination Addon Routing:  Mileage, Map, or Restriction
  //----------------------------------------------------------------------
  if (destAddOnRtg)
  {
    if (destAddOnRtg->routing() == MILEAGE_ROUTING)
    {
      _destAddonMileage = true;
    }
    else if (destAddOnRtg->rmaps().empty())
    {
      _destAddonRestriction = true;
    }
    else
    {
      _destAddonMap = true;
    }
  }
  else if ((!rtgInfo.destAddOnGateway().empty()) || (!rtgInfo.destAddOnInterior().empty()))
  {
    _constructedRouting = true;
    if (rtgInfo.rtgKey().addOnRouting2().empty() ||
        rtgInfo.rtgKey().addOnRouting2() == MILEAGE_ROUTING)
    {
      _destAddonMileage = true;
    }
  }
  else
  {
    _noDestAddon = true;
  }

  return;
}

// ----------------------------------------------------------------------------
//
// Display the Constructed Routing components
//
// ----------------------------------------------------------------------------
void
RoutingDiagCollector::displayConstructedRoutingDepreciated(const TravelRoute& tvlRoute,
                                                           const RoutingInfo& rtgInfo)
{
  //--------------------------------------------------------------------
  // Display Constructed Routing message when at least 2 of the Routing
  // components have maps.
  //--------------------------------------------------------------------
  *this << "    CONSTRUCTED ROUTING               " << _routingStatus << std::endl;

  if ((_origAddonMap || _destAddonMap) && _baseMap)
  {
    if (tvlRoute.terminalPoints())
    {
      *this << "    ROUTING CONSTRUCTED USING ENTRY/EXIT POINTS" << std::endl;
    }

    else
    {
      *this << "    ROUTING CONSTRUCTED USING COMMON POINTS" << std::endl;
    }
  }

  //------------------------------------------------------------------------
  // Display Mileage message when any of the Routing Components are Mileage.
  //------------------------------------------------------------------------
  if (_origAddonMileage || _destAddonMileage || _baseMileage)
  {
    if (rtgInfo.mileageInfo() != nullptr && rtgInfo.mileageInfo()->totalApplicableMPM() != 0)
    {
      *this << "    MPM " << _gd << "  " << std::setw(4)
            << rtgInfo.mileageInfo()->totalApplicableMPM() << " APPLIES TO ENTIRE ROUTE OF TRAVEL"
            << std::endl;
    }
  }

  //-------------------------------------
  // Display Mileage Surcharges Message
  //-------------------------------------
  if (rtgInfo.mileageInfo() != nullptr && rtgInfo.mileageInfo()->surchargeAmt() != 0)
  {
    if (rtgInfo.mileageInfo()->surchargeAmt() == EXCEEDS_MPM)
    {
      *this << "    MILEAGE EXCEEDS MPM  -  TOTAL TPM "
            << rtgInfo.mileageInfo()->totalApplicableTPM() << std::endl;
    }
    else
    {
      *this << "    MILEAGE SURCHARGE APPLIES - " << rtgInfo.mileageInfo()->surchargeAmt()
            << "M TOTAL TPM " << rtgInfo.mileageInfo()->totalApplicableTPM() << std::endl;
    }
  }

  displayOrigAddon(rtgInfo);

  displayBase(tvlRoute, rtgInfo);

  displayDestAddon(tvlRoute, rtgInfo);

  return;
}

void
RoutingDiagCollector::displayConstructedRouting(const TravelRoute& tvlRoute,
                                                const RoutingInfo& rtgInfo)
{
  // Display Constructed Routing message when at least 2 of the Routing components have maps.
  *this << "    CONSTRUCTED ROUTING               " << _routingStatus << std::endl;

  if ((_origAddonMap || _destAddonMap) && _baseMap && rtgInfo.routing())
  {
    if (rtgInfo.routing()->entryExitPointInd() == ENTRYEXITONLY)
      *this << "    ROUTING CONSTRUCTED USING ENTRY/EXIT POINTS" << std::endl;
    else
      *this << "    ROUTING CONSTRUCTED USING COMMON POINTS" << std::endl;
  }

  // Display Mileage message when any of the Routing Components are Mileage.
  if (_origAddonMileage || _destAddonMileage || _baseMileage)
  {
    if (rtgInfo.mileageInfo() != nullptr && rtgInfo.mileageInfo()->totalApplicableMPM() != 0)
    {
      *this << "    MPM " << _gd << "  " << std::setw(4)
            << rtgInfo.mileageInfo()->totalApplicableMPM() << " APPLIES TO ENTIRE ROUTE OF TRAVEL"
            << std::endl;
    }
  }

  // Display Mileage Surcharges Message
  if (rtgInfo.mileageInfo() != nullptr && rtgInfo.mileageInfo()->surchargeAmt() != 0)
  {
    if (rtgInfo.mileageInfo()->surchargeAmt() == EXCEEDS_MPM)
    {
      *this << "    MILEAGE EXCEEDS MPM  -  TOTAL TPM "
            << rtgInfo.mileageInfo()->totalApplicableTPM() << std::endl;
    }
    else
    {
      *this << "    MILEAGE SURCHARGE APPLIES - " << rtgInfo.mileageInfo()->surchargeAmt()
            << "M TOTAL TPM " << rtgInfo.mileageInfo()->totalApplicableTPM() << std::endl;
    }
  }

  displayOrigAddon(rtgInfo);
  displayBase(tvlRoute, rtgInfo);
  displayDestAddon(tvlRoute, rtgInfo);

  return;
}

// -------------------------------------------
//
// Display the Origin Addon Routing Component
//
// -------------------------------------------
void
RoutingDiagCollector::displayOrigAddon(const RoutingInfo& rtgInfo)
{
  const Routing* origAddOnRtg = rtgInfo.origAddOnRouting();
  CarrierCode cxr = "  ";
  bool mapValidationFailed = false;

  if (_origAddonMap || _origAddonMileage || _origAddonRestriction)
  {
    if (_origAddonMileage)
    {
      *this << "      " << rtgInfo.origAddOnInterior() << "-" << rtgInfo.origAddOnGateway()
            << " ORIGIN MILEAGE ADDON" << std::endl;
    }
    else
    {
      //------------------------
      // MAP - MILEAGE OR
      //       RESTRICTION ONLY
      //------------------------
      if (_origAddonMap && (_baseMileage || _baseRestriction))
      {
        if (!rtgInfo.rtgAddon1CityCarrier().empty())
        {
          *this << "  ";
          displayCityCarriers(cxr, rtgInfo.rtgAddon1CityCarrier());
        }
      }
      else
      {
        //------------------------
        // MAP - MAP - MILEAGE OR
        //             RESTRICTION
        //------------------------
        if (_origAddonMap && _baseMap && (_destAddonMileage || _destAddonRestriction))
        {
          if (!rtgInfo.rtgBaseCityCarrier().empty())
          {
            *this << "  ";
            displayCityCarriers(cxr, rtgInfo.rtgBaseCityCarrier());
          }
        }
      }

      *this << "      " << rtgInfo.origAddOnInterior() << "-" << rtgInfo.origAddOnGateway()
            << " ORIGIN ROUTING ADDON" << std::endl;
    }

    //--------------------
    // ATPCO Mileage Addon
    //--------------------
    if (origAddOnRtg && origAddOnRtg->vendor() != Vendor::SITA)
    {
      *this << "        " << origAddOnRtg->vendor() << "     " << std::setw(4)
            << origAddOnRtg->routing();
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << " TRF-" << std::setw(4) << origAddOnRtg->routingTariff() << " ";
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << std::setw(7) << rtgInfo.tcrAddonTariff1Code() << std::endl;
    }

    //--------------------
    // SITA Mileage Addon
    //--------------------
    else if (rtgInfo.mileageInfo() != nullptr)
    {
      *this << "         MPM       0000 ";
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << "TRF-" << std::setw(4) << rtgInfo.routingTariff();
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << "  " << std::setw(7) << std::setfill(' ') << rtgInfo.tcrRoutingTariffCode()
            << std::endl;
    }

    //---------------------------------------------------------------------
    // Display map validation results for mileage base with routing addons
    //
    // MAP - MILEAGE OR
    //       RESTRICTION
    //---------------------------------------------------------------------
    if (_origAddonMap && (_baseMileage || _baseRestriction))
    {
      if (!rtgInfo.rtgAddon1CityCarrier().empty())
      {
        *this << "    ";
        mapValidationFailed = displayMapResults(rtgInfo.mapInfo(), rtgInfo);
        if (mapValidationFailed)
        {
          *this << "    ";
          displayMissingCity(rtgInfo.rtgAddon1CityCarrier(), rtgInfo, false);
        }
      }
    }

    //---------------------------------------------------
    // Display restrictions applicable only to the addon
    //---------------------------------------------------
    displayOrigAddonRestrictions(rtgInfo);
  }
  else
  {
    if (origAddOnRtg == nullptr && !rtgInfo.rtgKey().addOnRouting1().empty() &&
        rtgInfo.rtgKey().addOnRouting1() != MILEAGE_ROUTING)
    {
      *this << "    UNABLE TO RETRIEVE ROUTING " << rtgInfo.rtgKey().vendor() << '/'
            << rtgInfo.rtgKey().carrier() << '/' << rtgInfo.rtgKey().routingTariff() << '/'
            << rtgInfo.rtgKey().addOnRouting1() << std::endl;
    }
  }
}

// -------------------------------------------
//
// Display the Base Routing Component
//
// -------------------------------------------
void
RoutingDiagCollector::displayBase(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo)
{
  const Routing* routing = rtgInfo.routing();
  CarrierCode cxr = "  ";

  if (routing)
  {
    if (_baseMileage)
    {
      *this << "      " << rtgInfo.market1() << "-" << rtgInfo.market2()
            << " SPECIFIED BASE MILEAGE" << std::endl;

      *this << "        " << routing->vendor() << " " << _gd << "  " << std::setw(4)
            << routing->routing();
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << " TRF-" << std::setw(4) << routing->routingTariff() << " ";
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << std::setw(7) << rtgInfo.tcrRoutingTariffCode() << std::endl;
    }
    else
    {
      if (_noDestAddon || _destAddonMap || _noOrigAddon)
      {
        if (!rtgInfo.rtgBaseCityCarrier().empty())
        {
          *this << "  ";
          displayCityCarriers(cxr, rtgInfo.rtgBaseCityCarrier());
        }
      }

      *this << "      " << rtgInfo.market1() << "-" << rtgInfo.market2()
            << " SPECIFIED BASE ROUTING" << std::endl;

      *this << "        " << routing->vendor() << " " << _gd << "  " << std::setw(4)
            << routing->routing();

      this->setf(std::ios::left, std::ios::adjustfield);
      *this << " TRF-" << std::setw(4) << routing->routingTariff() << " ";
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << std::setw(7) << rtgInfo.tcrRoutingTariffCode() << std::endl;

      //---------------------------------------------------------------------
      // Display map validation results for routing base with mileage addons
      //---------------------------------------------------------------------
      if (RoutingUtil::processBaseTravelRoute(rtgInfo))
      {
        if (_noDestAddon || _destAddonMileage || _destAddonRestriction)
        {
          if (!routing->rmaps().empty())
          {
            *this << "    ";
            bool mapValidationFailed = displayMapResults(rtgInfo.mapInfo(), rtgInfo);
            if (mapValidationFailed)
            {
              *this << "    ";
              displayMissingCity(tvlRoute.travelRoute(), rtgInfo, false);
            }
          }
          else
          {
            *this << "        ROUTE MAP DOES NOT EXIST" << std::endl;
          }
        }
      }
    }

    //---------------------------------------------------
    // Display restrictions applicable only to the addon
    //---------------------------------------------------
    displayBaseRestrictions(rtgInfo);
  }

  //--------------------
  // SITA Mileage Addon
  //--------------------
  else if (rtgInfo.mileageInfo() != nullptr)
  {
    *this << "      " << rtgInfo.market1() << "-" << rtgInfo.market2() << " SPECIFIED BASE MILEAGE"
          << std::endl;

    *this << "         MPM   " << _gd << "  " << std::setw(4) << "0000";
    this->setf(std::ios::left, std::ios::adjustfield);
    *this << " TRF-" << std::setw(4) << rtgInfo.routingTariff();
    this->setf(std::ios::left, std::ios::adjustfield);
    *this << "  " << std::setw(7) << std::setfill(' ') << rtgInfo.tcrRoutingTariffCode()
          << std::endl;
  }
}

// ------------------------------------------------
//
// Display the Destination Addon Routing Component
//
// ------------------------------------------------
void
RoutingDiagCollector::displayDestAddon(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo)
{
  CarrierCode cxr = "  ";
  const Routing* destAddOnRtg = rtgInfo.destAddOnRouting();

  if (_destAddonMap || _destAddonMileage || _destAddonRestriction)
  {
    if (_destAddonMileage)
    {
      *this << "      " << rtgInfo.destAddOnGateway() << "-" << rtgInfo.destAddOnInterior()
            << " DESTINATION MILEAGE ADDON" << std::endl;
    }
    else
    {
      if (!rtgInfo.rtgAddon2CityCarrier().empty() && (_baseMileage || _baseRestriction))
      {
        *this << "  ";
        displayCityCarriers(cxr, rtgInfo.rtgAddon2CityCarrier());
      }
      *this << "      " << rtgInfo.destAddOnGateway() << "-" << rtgInfo.destAddOnInterior()
            << " DESTINATION ROUTING ADDON" << std::endl;
    }

    //--------------------
    // ATPCO Mileage Addon
    //--------------------
    if (destAddOnRtg && destAddOnRtg->vendor() != Vendor::SITA)
    {
      *this << "        " << destAddOnRtg->vendor() << "     " << std::setw(4)
            << destAddOnRtg->routing();
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << " TRF-" << std::setw(4) << destAddOnRtg->routingTariff();
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << " " << std::setw(7) << rtgInfo.tcrAddonTariff2Code() << std::endl;
    }

    //--------------------
    // SITA Mileage Addon
    //--------------------
    else if (rtgInfo.mileageInfo() != nullptr)
    {
      *this << "         MPM       0000";
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << " TRF-" << std::setw(4) << rtgInfo.routingTariff();
      this->setf(std::ios::left, std::ios::adjustfield);
      *this << std::setw(7) << std::setfill(' ') << rtgInfo.tcrRoutingTariffCode() << std::endl;
    }

    //---------------------------------------------------------------------
    // Display map validation results for:
    //
    // MPM Base with Routing Addons
    // Routing base with no map (Restrictions only)
    //---------------------------------------------------------------------
    if (_destAddonMap && (_baseMileage || _baseRestriction))
    {
      if (!rtgInfo.rtgAddon2CityCarrier().empty())
      {
        *this << "    ";
        bool mapValidationFailed = displayMapResults(rtgInfo.rtgAddonMapInfo(), rtgInfo);
        if (mapValidationFailed)
        {
          *this << "    ";
          displayMissingCity(rtgInfo.rtgAddon2CityCarrier(), rtgInfo, false);
        }
      }
    }

    //---------------------------------------------------------------------
    // Display map validation results for:
    //
    // MPM - RTG Base - RTG
    //
    // OR
    //
    // RTG - MPM Base - RTG
    //       Restriction
    //---------------------------------------------------------------------
    if ((_origAddonMileage || _origAddonRestriction) && _destAddonMap && _baseMap)
    // if (_destAddonMap && (_origAddonMileage || _baseMileage || _baseRestriction))
    {
      // if (_baseMap)
      //{
      *this << "    ";
      bool mapValidationFailed = displayMapResults(rtgInfo.mapInfo(), rtgInfo);
      if (mapValidationFailed)
      {
        *this << "    ";
        displayMissingCity(tvlRoute.travelRoute(), rtgInfo, false);
      }
      //}
      // else
      //{
      //  * this << "        ROUTE MAP DOES NOT EXIST" << std::endl;
      //}
    }

    //---------------------------------------------------
    // Display restrictions applicable only to the addon
    //---------------------------------------------------
    displayDestAddonRestrictions(rtgInfo);
  }
  else
  {
    if (destAddOnRtg == nullptr && !rtgInfo.rtgKey().addOnRouting2().empty() &&
        rtgInfo.rtgKey().addOnRouting2() != MILEAGE_ROUTING)
    {
      *this << "    UNABLE TO RETRIEVE ROUTING " << rtgInfo.rtgKey().vendor() << '/'
            << rtgInfo.rtgKey().carrier() << '/' << rtgInfo.rtgKey().routingTariff() << '/'
            << rtgInfo.rtgKey().addOnRouting2() << std::endl;
    }
  }
}

void
RoutingDiagCollector::displayMapDirectionalityInfo(const RoutingInfo& rtgInfo)
{
  if (!rtgInfo.routing() || rtgInfo.routing()->routing() == MILEAGE_ROUTING ||
      rtgInfo.routing()->rmaps().empty())
    return;

  switch (rtgInfo.routing()->directionalInd())
  {
  case MAPDIR_L2R:
    *this << "    MAP CONSTRUCTED LEFT TO RIGHT" << std::endl;
    break;
  case MAPDIR_BOTH:
    *this << "    MAP CONSTRUCTED LEFT TO RIGHT AND RIGHT TO LEFT" << std::endl;
    break;
  case MAPDIR_IGNOREIND:
  default:
    break;
  }
}

void
RoutingDiagCollector::displayUnticketedPointInfo(const RoutingInfo& rtgInfo)
{
  if (!rtgInfo.routing() || rtgInfo.routing()->routing() == MILEAGE_ROUTING ||
      rtgInfo.routing()->rmaps().empty())
    return;

  switch (rtgInfo.routing()->unticketedPointInd())
  {
  case TKTPTS_TKTONLY:
    *this << "    VALIDATION APPLIED TO TICKETED POINTS" << std::endl;
    break;
  case TKTPTS_ANY:
    *this << "    VALIDATION APPLIED TO TICKETED/UNTICKETED POINTS" << std::endl;
    break;
  case IGNORE_TKTPTSIND:
  default:
    break;
  }
}

void
RoutingDiagCollector::displayEntryExitPointInfo(const RoutingInfo& rtgInfo)
{
  if (!rtgInfo.routing() || rtgInfo.routing()->routing() == MILEAGE_ROUTING ||
      rtgInfo.routing()->rmaps().empty())
    return;

  switch (rtgInfo.routing()->entryExitPointInd())
  {
  case ENTRYEXITONLY:
    *this << "    VALIDATION APPLIED TO ENTRY/EXIT POINTS" << std::endl;
    break;
  case ANYPOINT:
    *this << "    VALIDATION APPLIED TO ENTRY/EXIT/INTERMEDIATE POINTS" << std::endl;
    break;
  case GETTERMPTFROMCRXPREF:
  default:
    break;
  }
}

void
RoutingDiagCollector::displayPSRs(const RoutingInfo& rtgInfo)
{
  if (!rtgInfo.mileageInfo() == 0)
  {
    if (rtgInfo.mileageInfo()->psrApplies())
    {
      *this << "    " << rtgInfo.mileageInfo()->psrGovCxr()
            << " PERMISSIBLE SPECIFIED ROUTING APPLIES" << std::endl;

      if (!rtgInfo.mileageInfo()->psrGeoLocs().empty())
      {
        *this << "    ";
      }

      uint16_t lineLength = 59;
      int16_t charCount = 4;

      for (const TpdPsrViaGeoLoc& geoLoc : rtgInfo.mileageInfo()->psrGeoLocs())
      {
        if ((charCount + 8) > lineLength)
        {
          *this << std::endl;
          *this << "    ";
          charCount = 4;
        }

        if (geoLoc.relationalInd() == VIAGEOLOCREL_AND)
        {
          *this << " AND ";
          charCount += 4;
        }
        else if (geoLoc.relationalInd() == VIAGEOLOCREL_OR)
        {
          *this << " OR ";
          charCount += 4;
        }
        else if (geoLoc.relationalInd() == VIAGEOLOCREL_ANDOR)
        {
          *this << " AND/OR ";
          charCount += 8;
        }

        if ((charCount + 15) > lineLength)
        {
          *this << std::endl;
          *this << "    ";
          charCount = 4;
        }

        if (geoLoc.loc().locType() == LOCTYPE_ZONE)
        {
          *this << "ZONE ";
          charCount += 5;
        }
        else if (geoLoc.loc().locType() == LOCTYPE_AREA)
        {
          *this << "AREA ";
          charCount += 5;
        }
        else if (geoLoc.loc().locType() == LOCTYPE_SUBAREA)
        {
          *this << "SUB AREA ";
          charCount += 9;
        }
        else if (geoLoc.loc().locType() == LOCTYPE_STATE)
        {
          *this << "STATE/PROVINCE ";
          charCount += 15;
        }

        if ((charCount + geoLoc.loc().loc().size()) > lineLength)
        {
          *this << std::endl;
          *this << "    ";
          charCount = 4;
        }

        *this << geoLoc.loc().loc();
      }

      *this << std::endl;
      *this << "    NO MPM CHECK DUE TO PERMISSIBLE SPECIFIED ROUTING" << std::endl;
    }
  }
}

// ----------------------------------------------------------------------------
//  Display Domestic Route Validation Info !!!
// ----------------------------------------------------------------------------
void
RoutingDiagCollector::displayDRVInfos(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo)
{
  RoutingDiagCollector& diag = *this;

  //---------------------------------------------------------------------
  // If DRVInfos exist, display the status of the DRVIndicator
  //---------------------------------------------------------------------
  if (rtgInfo.mapInfo() != nullptr && (!rtgInfo.mapInfo()->drvInfos().empty()))
  {
    const Routing* routing = rtgInfo.routing();

    if (routing->domRtgvalInd() == PASS_ANY_ONLINE_POINT)
      diag << "    DRV INDICATOR - PASS ANY ONLINE POINT" << std::endl;
    else
      diag << "    DRV INDICATOR - PERFORM DOMESTIC ROUTE VALIDATION" << std::endl;

    //----------------------------------------------------------------
    // Iterate through the vector of drvInfos and display the results
    //----------------------------------------------------------------
    for (DRVInfo* drv : rtgInfo.mapInfo()->drvInfos())
    {
      //----------------------------------------------------------
      // Check all the reasons DRV might not have been performed.
      //---------------------------------------------------------
      if (drv->noFareMarket())
      {
        diag << "    NO DRV-UNABLE TO LOCATE MATCHING FARE MARKET" << std::endl;
        diag << "    SHOPPING-IS ASSUMPTION IS VALID IN DRVCONTROLLER" << std::endl;
      }

      else if (drv->getRoutingFailed())
      {
        diag << "    NO DRV-UNABLE TO RETRIEVE LOCAL ROUTING" << std::endl;
      }

      else if (drv->missingCityIndexInvalid())
      {
        diag << "    NO DRV-MISSING CITY INDEX IS INVALID" << std::endl;
      }

      else if (drv->noPaxTypeFares())
      {
        diag << "    NO DRV-NO FARES IN LOCAL MARKET" << std::endl;
      }

      else if (drv->missingCityOrigDest())
      {
        diag << "    NO DRV-MISSING CITY ORIG/DEST OF LOCAL TRAVEL" << std::endl;
      }

      else if (drv->notSameCarrier())
      {
        diag << "    NO DRV-LOCAL TRAVEL NOT ON SAME CARRIER" << std::endl;
      }

      else if (drv->notSameCountry())
      {
        diag << "    NO DRV-LOCAL TRAVEL NOT IN SAME COUNTRY" << std::endl;
      }

      else if (drv->notQualifiedForDRV())
      {
        diag << "    NO DRV-MISSING CITY IS NOT IN ORIG/DEST COUNTRY" << std::endl;
      }

      else
      {
        //----------------------------------
        // Display the highest oneway fare
        //----------------------------------
        diag << "    HIGHEST FARE SELECTED FOR DRV" << std::endl;

        diag.setf(std::ios::left, std::ios::adjustfield);
        diag << "    " << std::setw(15) << drv->fareClass() << " ";
        diag << std::setw(9) << Money(drv->fareAmount(), drv->currency());
        diag << std::endl;

        //--------------------------------
        // Display the Local Travel Route
        //--------------------------------
        diag.displayCityCarriers(drv->localGovCxr(), drv->localCityCarrier());

        //---------------------------------------------------------------------
        // Display the Routing Header - ATP  AT  0005 TRF-0099   ROUTING VALID
        //---------------------------------------------------------------------
        std::string status;
        if (drv->drvInfoStatus())
          status = "LOCAL ROUTING VALID";
        else
          status = "LOCAL ROUTING NOT VALID";

        std::string gd;
        globalDirectionToStr(gd, drv->global());

        diag << "    " << drv->vendor() << "  " << std::setw(2) << gd << "   ";
        diag << std::setw(4) << drv->routingNumber() << "  TRF-";
        diag << std::setw(4) << drv->routingTariff1() << "  ";
        diag.setf(std::ios::left, std::ios::adjustfield);
        diag << std::setw(7) << drv->tariffCode1();
        diag << " " << status << std::endl;

        diag.displayMileageMessages(*drv);
        if (diag.displayMapResults(drv->mapInfo(), *drv))
          diag.displayMissingCity(drv->localCityCarrier(), *drv, false);

        if (drv->restrictions() != nullptr)
          diag.displayRestrictions(*drv);

        //-----------------------------------------------------
        // Display the International TravelRoute if applicable
        //-----------------------------------------------------
        if (!drv->intlCityCarrier().empty())
        {
          diag << "    CONTINUATION OF INTERNATIONAL MAP VALIDATION" << std::endl;
          diag.displayCityCarriers(tvlRoute.govCxr(), drv->intlCityCarrier());

          //----------------------------------------------------------
          // Display Routing Status for International Route after DRV
          //----------------------------------------------------------
          if (rtgInfo.mapInfo()->valid())
            status = "ROUTING VALID";
          else
            status = "ROUTING NOT VALID";

          globalDirectionToStr(gd, tvlRoute.globalDir());

          diag << "    " << routing->vendor() << "  " << std::setw(2) << gd << "   " << std::setw(4)
               << routing->routing() << "  TRF-" << std::setw(4) << routing->routingTariff() << "  "
               << std::setw(7) << std::setfill(' ') << rtgInfo.tcrRoutingTariffCode() << " "
               << status << std::endl;

          diag.displayMileageMessages(rtgInfo);

          if (rtgInfo.mapInfo()->valid())
          {
            *this << "    ROUTE MAP EXISTS - MAP VALIDATION PASSED" << std::endl;
          }
          else
          {
            *this << "    ROUTE MAP EXISTS - MAP VALIDATION FAILED" << std::endl;
            diag.displayMissingCity(drv->intlCityCarrier(), rtgInfo, true);
          }
        }
      }
    }
  }

  else
  {
    if ((rtgInfo.mapInfo()) && !rtgInfo.mapInfo()->valid())
    {
      //------------------------------------------
      // Display Messages when DRV does not Apply
      //------------------------------------------
      if (tvlRoute.doNotApplyDRV())
      {
        diag << "    DRV APPLIES ONLY TO POINTS IN THE U.S. AND CANADA" << std::endl;
      }

      else if (rtgInfo.routing() != nullptr)
      {
        if (rtgInfo.routing()->domRtgvalInd() == NO_DOMESTIC_ROUTE_VALIDATION)
        {
          diag << "    DRV INDICATOR - DO NOT PERFORM DOMESTIC ROUTE VALIDATION" << std::endl;
        }
      }
    }
  }
}
} // tse
