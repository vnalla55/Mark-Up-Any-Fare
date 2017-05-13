#include "Routing/MileageEqualization.h"

#include "Common/Logger.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Routing/Collector.h"
#include "Routing/MileageRoute.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MileageUtil.h"
#include "Routing/MPMCollectorMV.h"
#include "Routing/Retriever.h"
#include "Routing/RoutingConsts.h"
#include "Routing/TicketedPointDeduction.h"
#include "Routing/TPMRetriever.h"

namespace tse
{
static Logger
logger("atseintl.Routing.MileageEqualization");

MileageEqualization::MileageEqualization() : _tpd(Singleton<TicketedPointDeduction>::instance())
{
}

bool
MileageEqualization::apply(MileageRoute& route) const
{
  route.mileageEqualizationApplies() = false;

  if (route.globalDirection() != GlobalDirection::WH || route.mileageRouteItems().empty())
    return false;

  Target target;
  if (!prepareTarget(route, target))
    return false;

  LOG4CXX_DEBUG(logger, "Criteria matched, now applying");
  return applyEqualization(route, target);
}

bool
MileageEqualization::prepareTarget(const MileageRoute& route, Target& target) const
{
  const Loc* boardPt = route.mileageRouteItems().front().origCityOrAirport();
  const Loc* offPt = route.mileageRouteItems().back().destCityOrAirport();

  if (boardPt->loc() == RIO_DE_JANEIRO || boardPt->loc() == SAO_PAULO)
  {
    target.index = 0;
    target.isReplacingCityInDest = false;
    target.replacedCity = replaceCity(boardPt->loc());

    if (!matchCriteria(route, *offPt, target.replacedCity))
      return false;
  }
  else if (offPt->loc() == RIO_DE_JANEIRO || offPt->loc() == SAO_PAULO)
  {
    target.index = route.mileageRouteItems().size() - 1;
    target.isReplacingCityInDest = true;
    target.replacedCity = replaceCity(offPt->loc());

    if (!matchCriteria(route, *boardPt, target.replacedCity))
      return false;
  }
  else
  {
    LOG4CXX_INFO(logger, "Not applied: no RIO/SAO on origin/dest");
    return false;
  }

  return true;
}

bool
MileageEqualization::matchCriteria(const MileageRoute& route,
                                   const Loc& oppositeLoc,
                                   const LocCode& replacedCity) const
{
  if (oppositeLoc.area() != IATA_AREA1)
  {
    LOG4CXX_INFO(logger, "Not applied: travel not wholly in Area 1");
    return false;
  }

  if (!validateTicketedPoints(route, replacedCity))
  {
    LOG4CXX_INFO(logger, "Not applied: " << replacedCity << " found as intermediate point");
    return false;
  }

  return true;
}

bool
MileageEqualization::validateTicketedPoints(const MileageRoute& route,
                                            const LocCode& replacedCity) const
{
  for (const MileageRouteItem& item : route.mileageRouteItems())
  {
    if (item.origCityOrAirport()->loc() == replacedCity)
      return false;
    if (item.destCityOrAirport()->loc() == replacedCity)
      return false;
  }

  return true;
}

bool
MileageEqualization::applyEqualization(MileageRoute& route, const Target& target) const
{
  MileageRoute substitutedRoute;
  fillSubstitutedRoute(substitutedRoute, route, target);

  MileageRouteItem& substitutedItem = substitutedRoute.mileageRouteItems()[target.index];

  LOG4CXX_DEBUG(logger,
                "Before. TPM: " << substitutedRoute.mileageRouteTPM()
                                << " MPM: " << substitutedRoute.mileageRouteMPM());

  if (!updateTPM(substitutedRoute, substitutedItem))
    return false;

  if (!updateMPM(substitutedRoute))
    return false;

  applyTPD(substitutedRoute);

  LOG4CXX_DEBUG(logger,
                "After.  TPM: " << substitutedRoute.mileageRouteTPM()
                                << " MPM: " << substitutedRoute.mileageRouteMPM());

  const uint16_t newEMS =
      MileageUtil::getEMS(substitutedRoute.mileageRouteTPM(), substitutedRoute.mileageRouteMPM());
  if (newEMS >= route.ems())
  {
    LOG4CXX_DEBUG(logger, "Before: " << route.ems() << "M, now: " << newEMS << "M");
    LOG4CXX_INFO(logger, "Not applied: surcharge did not decrease");
    return false;
  }

  LOG4CXX_INFO(logger, "Applied. before: " << route.ems() << "M, now: " << newEMS << "M");

  MileageRouteItem& item = route.mileageRouteItems()[target.index];

  MileageRouteItem& last = route.mileageRouteItems().back();
  MileageRouteItem& substitutedLast = substitutedRoute.mileageRouteItems().back();

  route.mileageEqualizationApplies() = true;
  route.ems() = newEMS;

  item.tpm() = substitutedItem.tpm();
  last.mpm() = substitutedLast.mpm();
  last.tpd() = substitutedLast.tpd();

  route.mileageRouteTPM() = substitutedRoute.mileageRouteTPM();
  route.mileageRouteMPM() = substitutedRoute.mileageRouteMPM();
  route.tpd() = substitutedRoute.tpd();

  return true;
}

void
MileageEqualization::fillSubstitutedRoute(MileageRoute& substitutedRoute,
                                          const MileageRoute& route,
                                          const Target& target) const
{
  substitutedRoute.partialInitialize(route);
  substitutedRoute.mileageRouteItems() = route.mileageRouteItems();
  substitutedRoute.mileageRouteTPM() = route.mileageRouteTPM();
  substitutedRoute.mileageRouteMPM() = route.mileageRouteMPM();
  substitutedRoute.tpd() = route.tpd();

  MileageRouteItem& substitutedTarget = substitutedRoute.mileageRouteItems()[target.index];

  Loc*& substitutedCity =
      (target.isReplacingCityInDest ? substitutedTarget.city2() : substitutedTarget.city1());
  Loc*& substitutedMultiTransportAirport =
      (target.isReplacingCityInDest ? substitutedTarget.multiTransportDestination()
                                    : substitutedTarget.multiTransportOrigin());

  // Change the target city in substituted route.
  route.dataHandle()->get(substitutedCity);
  *substitutedCity =
      *route.dataHandle()->getLoc(target.replacedCity, substitutedTarget.travelDate());

  // Erase the multi transport loc.
  substitutedMultiTransportAirport = nullptr;
}

bool
MileageEqualization::updateTPM(MileageRoute& route, MileageRouteItem& item) const
{
  const Retriever<TPMRetriever>& tpmRetriever(tse::Singleton<Retriever<TPMRetriever> >::instance());

  const uint16_t oldTPM = item.tpm();

  if (!tpmRetriever.retrieve(item, *route.dataHandle()))
  {
    LOG4CXX_INFO(logger, "Not applied: could not retrieve TPM");
    return false;
  }

  // Update the route TPM.
  route.mileageRouteTPM() = uint16_t(route.mileageRouteTPM() - oldTPM + item.tpm());

  return true;
}

bool
MileageEqualization::updateMPM(MileageRoute& route) const
{
  const Collector<MPMCollectorMV>& mpmCollector(
      tse::Singleton<Collector<MPMCollectorMV> >::instance());

  if (!mpmCollector.collectMileage(route))
  {
    LOG4CXX_INFO(logger, "Not applied: could not retrieve MPM");
    return false;
  }

  // Update last item MPM.
  route.mileageRouteItems().back().mpm() = route.mileageRouteMPM();

  return true;
}

void
MileageEqualization::applyTPD(MileageRoute& route) const
{
  const uint16_t oldTPD = route.tpd();

  // Clear the old TPD value.
  route.tpd() = 0;
  route.mileageRouteItems().back().tpd() = 0;

  _tpd.apply(route);

  // Update the route TPM.
  route.mileageRouteTPM() = uint16_t(route.mileageRouteTPM() + oldTPD - route.tpd());
}

LocCode
MileageEqualization::replaceCity(const LocCode& city) const
{
  if (city == RIO_DE_JANEIRO)
    return SAO_PAULO;
  else
    return RIO_DE_JANEIRO;
}
}
