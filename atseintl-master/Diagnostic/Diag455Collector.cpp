//----------------------------------------------------------------------------
//  File:        Diag455Collector.cpp
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
//----------------------------------------------------------------------------

#include "Diagnostic/Diag455Collector.h"

#include "Common/RoutingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Routing.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RoutingInfo.h"
#include "Routing/TravelRoute.h"

#include <iomanip>

namespace tse
{
void
Diag455Collector::buildHeader()
{
  ((DiagCollector&)*this) << "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag455Collector
//
// Description:  Display the route map and Restrictions.
//
// @param  RouteStringVec  vector of route strings extracted from the map.
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag455Collector::displayRouteMapAndRestrictions(PricingTrx& trx,
                                                 const TravelRoute& tvlRoute,
                                                 const RoutingInfos* routingInfos)
{
  if (!_active)
  {
    return;
  }

  std::map<std::string, std::string>::const_iterator e = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::const_iterator i =
      trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
  if (i != e && tvlRoute.mileageTravelRoute().size() > 0)
  {
    LocCode boardCity = i->second.substr(0, 3);
    LocCode offCity = i->second.substr(3, 3);

    if ((tvlRoute.mileageTravelRoute().front()->origAirport() != boardCity &&
         tvlRoute.mileageTravelRoute().front()->boardMultiCity() != boardCity) ||
        (tvlRoute.mileageTravelRoute().back()->destAirport() != offCity &&
         tvlRoute.mileageTravelRoute().back()->offMultiCity() != offCity))
    {
      return;
    }
  }

  Diag455Collector& diag = dynamic_cast<Diag455Collector&>(*this);
  std::string status, gd;
  TariffNumber tariffNumber = 0;
  TariffCode tariffCode;

  VendorCode vendor;
  RoutingNumber localRtg;
  TariffNumber trf1 = 0;
  TariffNumber trf2 = 0;

  if (routingInfos == nullptr || routingInfos->empty())
  {
    diag.buildHeader();
    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    diag.displayNoFaresMessage();
  }

  else
  {
    diag.buildHeader();
    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    bool firstPass = true;

    //-----------------------------------------------------------
    // Select Each Routing for Display from the RoutingInfos Map
    //-----------------------------------------------------------

    std::map<std::string, std::string>::const_iterator it =
        trx.diagnostic().diagParamMap().find(Diagnostic::RULE_NUMBER);
    bool isRoutingRule = (it != trx.diagnostic().diagParamMap().end() && it->second.size() == 4);

    RoutingInfos::const_iterator rInfosItr;
    for (rInfosItr = routingInfos->begin(); rInfosItr != routingInfos->end(); ++rInfosItr)
    {
      const RoutingInfo& rtgInfo = *(*rInfosItr).second;
      const Routing* routing = rtgInfo.routing();

      if (TrxUtil::isFullMapRoutingActivated(trx))
      {
        if (isRoutingRule && routing && routing->routing() != it->second)
          continue;

        if (RoutingUtil::isTicketedPointOnly(routing, tvlRoute.flightTrackingCxr()))
        {
          if (tvlRoute.unticketedPointInd() == TKTPTS_ANY)
          {
            continue;
          }
        }
        else if (tvlRoute.unticketedPointInd() == TKTPTS_TKTONLY)
        {
          continue;
        }
      }

      diag.displayLine(firstPass);

      // Display  ATP  AT  0165 TRF-   1   ROUTING VALID
      if (TrxUtil::isFullMapRoutingActivated(trx) && routing &&
          (routing->entryExitPointInd() != GETTERMPTFROMCRXPREF))
      {
        diag.displayRoutingStatus(tvlRoute, rtgInfo);
      }
      else
      {
        diag.displayRoutingStatusDepreciated(tvlRoute, rtgInfo);
      }

      if (TrxUtil::isFullMapRoutingActivated(trx))
      {
        diag.displayMapDirectionalityInfo(rtgInfo);
        diag.displayEntryExitPointInfo(rtgInfo);
        diag.displayUnticketedPointInfo(rtgInfo);
      }

      // to remove after finish Full-Map Routing project testing
      if (trx.diagnostic().diagParamMap().find("TT") != trx.diagnostic().diagParamMap().end() &&
          routing && routing->entryExitPointInd() != GETTERMPTFROMCRXPREF)
      {
        const CarrierPreference* pref =
            trx.dataHandle().getCarrierPreference(routing->carrier(), trx.travelDate());
        if ((routing->entryExitPointInd() == ENTRYEXITONLY &&
             pref->applyrtevaltoterminalpt() == 'N') ||
            (routing->entryExitPointInd() != ENTRYEXITONLY &&
             pref->applyrtevaltoterminalpt() == 'Y'))
          diag << "BYTE 61 DIFFERS FROM CARRIER PREF SETTINGS\n";
      }

      firstPass = false;
      diag.displayRestrictions(rtgInfo);
      diag.displayMileageMessages(rtgInfo);

      // Call display route strings only if a map exists AND it has been processed !!
      if (rtgInfo.mapInfo() != nullptr && rtgInfo.mapInfo()->processed())
      {
        diag.displayMileageMessages(rtgInfo);
        diag.displayMapMessages(rtgInfo);
        diag.displayHeader2(rtgInfo);

        //---------------------------
        // Display Route Map Strings
        //---------------------------
        const RoutingMapStrings* rMapStrings = rtgInfo.mapInfo()->routeStrings();
        if (rMapStrings != nullptr && !rMapStrings->empty())
        {
          bool rc = diag.displayRouteStrings(rMapStrings);
          if (!rc)
          {
            diag.displayNoRouteStrings(rtgInfo);
          }
        }
      } // Process Each Route Map

      //-----------------------------------------------------------------------------
      // Display Local Routes validated for DRV Flight Tracking Carriers
      //-----------------------------------------------------------------------------
      if (rtgInfo.mapInfo() != nullptr && (!rtgInfo.mapInfo()->drvInfos().empty()))
      {
        const DRVInfos drvInfos = rtgInfo.mapInfo()->drvInfos();
        DRVInfos::const_iterator drvItr = drvInfos.begin();
        DRVInfos::const_iterator drvEnd = drvInfos.end();

        for (; drvItr != drvEnd; drvItr++)
        {
          if ((*drvItr)->flightStopMarket())
          {
            if (!localRtg.empty())
            {
              if (vendor == (*drvItr)->vendor() && localRtg == (*drvItr)->routingNumber() &&
                  trf1 == (*drvItr)->routingTariff1() && trf2 == (*drvItr)->routingTariff2())
              {
                continue;
              }
            }

            diag.displayLine(firstPass);
            diag.displayCityCarriers((*drvItr)->localGovCxr(), (*drvItr)->localCityCarrier());

            // Display Routing Status for Local Route
            if ((*drvItr)->drvInfoStatus())
            {
              status = "LOCAL ROUTING VALID";
            }
            else
            {
              status = "LOCAL ROUTING NOT VALID";
            }

            globalDirectionToStr(gd, (*drvItr)->global());

            if (routing->routingTariff() == (*drvItr)->routingTariff1())
            {
              tariffNumber = (*drvItr)->routingTariff1();
              tariffCode = (*drvItr)->tariffCode1();
            }
            else
            {
              tariffNumber = (*drvItr)->routingTariff2();
              tariffCode = (*drvItr)->tariffCode2();
            }

            diag << "    " << (*drvItr)->vendor() << "  " << gd << "   ";
            diag << std::setw(4) << (*drvItr)->routing() << "  TRF-";
            diag << std::setw(4) << tariffNumber << "  ";
            diag.setf(std::ios::left, std::ios::adjustfield);
            diag << std::setw(7) << tariffCode;
            diag << " " << status << std::endl;

            diag.displayRestrictions(**drvItr);

            if ((*drvItr)->mapInfo() != nullptr && (*drvItr)->mapInfo()->processed())
            {
              diag.displayMileageMessages(**drvItr);
              diag.displayMapMessages(**drvItr);
              diag.displayHeader2(**drvItr);
              const RoutingMapStrings* rMapStrings = (*drvItr)->mapInfo()->routeStrings();
              if (rMapStrings != nullptr && !rMapStrings->empty())
              {
                bool rc = diag.displayRouteStrings(rMapStrings);
                if (!rc)
                {
                  diag.displayNoRouteStrings(**drvItr);
                }
              }
            }

            vendor = (*drvItr)->vendor();
            localRtg = (*drvItr)->routingNumber();
            trf1 = (*drvItr)->routingTariff1();
            trf2 = (*drvItr)->routingTariff2();
          }
        }
      }

      //-----------------------------------------------------------------------------
      // Check for a second bunch of route strings for a possible
      //          Rtg Addon + Mileage Base + Rtg Addon
      //-----------------------------------------------------------------------------
      if (rtgInfo.rtgAddonMapInfo() != nullptr && rtgInfo.rtgAddonMapInfo()->processed())
      {
        diag.displayRtgAddonHeader(rtgInfo);

        //---------------------------
        // Display Route Map Strings
        //---------------------------
        const RoutingMapStrings* rMapStrings = rtgInfo.rtgAddonMapInfo()->routeStrings();
        if (rMapStrings != nullptr && !rMapStrings->empty())
        {
          bool rc = diag.displayRouteStrings(rMapStrings);
          if (!rc)
          {
            diag.displayNoRouteStrings(rtgInfo);
          }
        }
      } // Process Each Route Map
    } // Process Each RoutingInfo which corresponds to a specific VCTR
  }
}

bool
Diag455Collector::displayRouteStrings(const RoutingMapStrings* rMapStrings)
{

  if (rMapStrings != nullptr && !rMapStrings->empty())
  {
    uint16_t strSize; // Size of RouteMapString left to display
    uint16_t lineLength = 58; // Maximum line size for display
    uint16_t start = 0; // Start at position 0 for each RouteMapString

    //-----------------------------------------------
    // Iterate through the vector of RouteMapStrings
    //-----------------------------------------------

    RoutingMapStrings::const_iterator rMapStringsItr = rMapStrings->begin();
    RoutingMapStrings::const_iterator end = rMapStrings->end();

    RoutingMapStrings::const_iterator end2 = rMapStrings->end();
    --end2; // Move to the last element
    const std::string& rLastString = *(end2); // This is the last string that has the complete path

    for (; rMapStringsItr != end; ++rMapStringsItr)
    {
      const std::string& rString = *rMapStringsItr; // Get a RouteMapString
      strSize = rString.size(); // Get the size of this routeMapString
      start = 0; // Reset start position to 0 for each Map String

      // If do not display the complete path again since it will be displayed in the end
      if (rString == rLastString && rMapStringsItr != end2)
        continue;

      if (strSize > lineLength)
      {
        breakString(rString); // Break long string into multiple lines
      }

      else
      {
        //-------------------------------------------------------------------------------
        // Display a single RouteMapString with length less than or equal to line length
        //-------------------------------------------------------------------------------
        *this << "    " << rString.substr(start, strSize) << std::endl;
      }
    } // for each RouteMapString

    *this << " " << std::endl;
  } // if RouteMapStrings are Empty

  else
  {
    return false; // RouteMapStrings are Empty
  }

  return true;
}

// ---------------------------------------------------
// Break a long route map string into multiple lines
//----------------------------------------------------
void
Diag455Collector::breakString(const std::string& rString)
{
  uint16_t totalStrSize = rString.size(); // Size of RouteMapString
  uint16_t strSize = totalStrSize; // Size of RouteMapString left to display
  uint16_t lineLength = 58; // Maximum line size for display
  uint16_t totalCharsDisplayed = 0; // Total Number of chars displayed for RouteMapString
  uint16_t charsDisplayedThisLine = 0; // Number of chars moved on a single line
  uint16_t start = 0; // Start at position 0 for each RouteMapString
  uint16_t searchStart = lineLength + 1; // Search string in reverse
  std::string::size_type idx; // Size type index for find()

  while (totalCharsDisplayed < totalStrSize)
  {
    if (strSize < lineLength) // If remainder of chars is now less than line length
    {
      *this << "    " << rString.substr(start, strSize) << std::endl;
      totalCharsDisplayed = totalCharsDisplayed + strSize; // Add remaining string size
    }

    else
    {
      idx = searchString(rString, searchStart); // Search String for the last slash or dash
      if (idx == std::string::npos)
      {
        *this << "ERROR IN ROUTE MAP STRING - UNABLE TO DISPLAY" << std::endl;
        totalCharsDisplayed = strSize; // Exit loop
      }
      else
      {
        charsDisplayedThisLine = idx - start; // Get number of chars to display on this line
        if (charsDisplayedThisLine < lineLength)
        {
          charsDisplayedThisLine++; // Include found slash or dash on this line
        }

        *this << "    " << rString.substr(start, charsDisplayedThisLine) << std::endl;

        totalCharsDisplayed = totalCharsDisplayed + charsDisplayedThisLine;
        strSize = strSize - charsDisplayedThisLine; // Calculate size of string left to display
        start = totalCharsDisplayed; // resume string display following the slash or dash
        searchStart = start + (lineLength);
      }
    }
  } // while (charsDisplayed < strSize)  Loop to display a big RouteMapString
}

// ----------------------------------------------------------------------------
// Search a Route Map String for the last Slash or Dash within a range of
// characters.
//
// Return the index value to the last slash or dash, whichever is last in the
// line, or return npos if the slash or dash cannot be found in the string.

// ----------------------------------------------------------------------------
std::string::size_type
Diag455Collector::searchString(const std::string& rString, uint16_t searchStart)
{
  std::string::size_type slash; // Size type index for rfind()
  std::string::size_type dash; // Size type index for rfind()
  std::string::size_type idx; // Size type index for rfind()

  slash = rString.rfind("/", searchStart, 1); // Search for the last / in string

  dash = rString.rfind("-", searchStart, 1); // Search for the last - in string

  if (slash == std::string::npos)
  {
    idx = dash; // Dash could be a value or npos
  }
  else if (dash == std::string::npos)
  {
    idx = slash; // Slash could be a value or npos
  }
  else if (slash > dash)
  {
    idx = slash; // Slash is a value
  }
  else
  {
    idx = dash; // Dash is a value
  }

  return idx;
}

// ----------------------------------------------------------------------------
void
Diag455Collector::displayHeader2(const RoutingInfo& rtgInfo)
{
  const Routing* origAddOnRtg = rtgInfo.origAddOnRouting();
  const Routing* destAddOnRtg = rtgInfo.destAddOnRouting();
  const Routing* routing = rtgInfo.routing();

  if (origAddOnRtg == nullptr && destAddOnRtg == nullptr)
  {
    *this << " " << std::endl;
    *this << "    ROUTE MAP DISPLAY" << std::endl;
    *this << "    ***********************************************************" << std::endl;
    return;
  }

  if (routing == nullptr || routing->routing() == MILEAGE_ROUTING || routing->rmaps().empty())
  {
    if (origAddOnRtg != nullptr && origAddOnRtg->routing() != MILEAGE_ROUTING &&
        !origAddOnRtg->rmaps().empty())
    {
      *this << " " << std::endl;
      *this << "    ROUTE MAP DISPLAY       ORIGIN ROUTING ADDON " << origAddOnRtg->routing()
            << std::endl;
      *this << "    ***********************************************************" << std::endl;
    }
  }

  else if (routing != nullptr && routing->routing() != MILEAGE_ROUTING && !routing->rmaps().empty() &&
           ((origAddOnRtg != nullptr && origAddOnRtg->routing() != MILEAGE_ROUTING) ||
            (destAddOnRtg != nullptr && destAddOnRtg->routing() != MILEAGE_ROUTING)))
  {
    *this << " " << std::endl;
    *this << "    CONSTRUCTED ROUTE MAP DISPLAY" << std::endl;
    *this << "    ***********************************************************" << std::endl;
  }

  else
  {
    *this << " " << std::endl;
    *this << "    ROUTE MAP DISPLAY" << std::endl;
    *this << "    ***********************************************************" << std::endl;
  }
}

void
Diag455Collector::displayRtgAddonHeader(const RoutingInfo& rtgInfo)
{
  const Routing* destAddOnRtg = rtgInfo.destAddOnRouting();
  const Routing* routing = rtgInfo.routing();

  if (destAddOnRtg != nullptr && destAddOnRtg->routing() != MILEAGE_ROUTING)
  {
    if (routing == nullptr || routing->routing() == MILEAGE_ROUTING || routing->rmaps().empty())
    {
      *this << " " << std::endl;
      *this << "    ROUTE MAP DISPLAY       DESTINATION ROUTING ADDON " << destAddOnRtg->routing()
            << std::endl;
      *this << "    ***********************************************************" << std::endl;
    }
    else
    {
      *this << " " << std::endl;
      *this << "    ROUTE MAP DISPLAY" << std::endl;
      *this << "    ************************************************************" << std::endl;
    }
  }
}

// ----------------------------------------------------------------------------
void
Diag455Collector::displayNoRouteStrings(const RoutingInfo& rtgInfo)
{

  const RoutingMapStrings* rMapStrings = rtgInfo.mapInfo()->routeStrings();

  if (rMapStrings == nullptr)
  {
    *this << " " << std::endl;

    *this << "    UNABLE TO DISPLAY ROUTE STRINGS" << std::endl;
    *this << "    **********************************************" << std::endl;
  }

  else if (rMapStrings->empty())
  {
    *this << " " << std::endl;

    *this << "    NO ROUTE STRINGS RETURNED FROM MAP VALIDATION" << std::endl;
    *this << "    **********************************************" << std::endl;
  }
}

// Separate each routing with a row of asteriks
void
Diag455Collector::displayLine(bool firstPass)
{
  if (!firstPass)
  {
    *this << "    **********************************************" << std::endl;
  }
}
}
