#include "Diagnostic/Diag450Collector.h"

#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/RoutingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RoutingInfo.h"
#include "Routing/TravelRoute.h"

#include <iomanip>

namespace tse
{
void
Diag450Collector::displayRoutingValidationResults(PricingTrx& trx,
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

  Diag450Collector& diag = dynamic_cast<Diag450Collector&>(*this);

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

    // Select each Routing for Display from the RoutingInfos Map
    RoutingInfos::const_iterator rInfosItr;
    for (rInfosItr = routingInfos->begin(); rInfosItr != routingInfos->end(); ++rInfosItr)
    {
      const RoutingInfo& rtgInfo = *(*rInfosItr).second;
      const Routing* routing = rtgInfo.routing();

      if (TrxUtil::isFullMapRoutingActivated(trx))
      {
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

      // Separate Routings with a row of asteriks
      if (rInfosItr != routingInfos->begin())
      {
        *this << "    **********************************************************" << std::endl;
      }

      // Display ATP  WH  0084 TRF-  99  ROUTING VALID
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

      // Display Routing Restrictions and Pass/Fail Status
      diag.displayRestrictions(rtgInfo);
      diag.displayMileageMessages(rtgInfo);
      displayPSRs(rtgInfo);
      if (diag.displayMapMessages(rtgInfo))
      {
        diag.displayMissingCity(tvlRoute.travelRoute(), rtgInfo, false);
      }

      if (!TrxUtil::isFullMapRoutingActivated(trx))
      {
        diag.displayTerminalPointMessage(tvlRoute, rtgInfo);
      }

      diag.displayDRVInfos(tvlRoute, rtgInfo);
    } // Process Each RoutingInfo which corresponds to a specific VCTR
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag450Collector
//
// Description:  Display the results of Routing Validation
//
// @param
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag450Collector::buildHeader()
{
  if (!_active)
  {
    return;
  }

  *this << "\n**************** ROUTING VALIDATION RESULTS *******************\n";
}
}
