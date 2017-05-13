#pragma once

#include "Routing/MileageRouteItem.h"

namespace tse
{

class MileageRoute;
class DataHandle;

/**
 * @class MPMCollectorWN
 *
 * MPM collector for Mileage Service (WN).
 */
class MPMCollectorWN
{
public:
  virtual ~MPMCollectorWN();

  /**
   * @function collectMileage
   *
   * Algorithm of MPM retrieval for whole mileage route for WN.
   * Retrieves MPM for each item in the route, building current market of
   * global origin and subsequent destinations.
   * Applies MPM reduction if appropriate for incremental sub-routes, with
   * subsequent items added.
   *
   * @param MileageRoute& - mileage route for which MPM is to be collected
   * @return bool - true if retrieval was successful, false otherwise
   */
  bool collectMileage(MileageRoute& mileageRoute) const;

private:
  bool
  isGDPossible(MileageRouteItems::const_iterator, MileageRouteItems::const_iterator, bool&) const;
  bool isSouthAmerica(const Loc*) const;
  bool isSouthPacific(const Loc*) const;
  /**
   * Auxiliary methods to make unit tests independent of the retrievers.
   */
  virtual bool getMPM(MileageRouteItem&, DataHandle&) const;
  virtual bool getPSR(MileageRoute& route) const;
};

} // namespace tse

