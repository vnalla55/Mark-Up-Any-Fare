#pragma once

namespace tse
{

class MileageRoute;
class MileageRouteItem;
class DataHandle;

/**
 * @class MPMCollectorMV
 *
 * MPM collector for Mileage Validation.
 */
class MPMCollectorMV
{
public:
  virtual ~MPMCollectorMV();

  /**
   * @function collectMileage
   *
   * Algorithm of MPM retrieval for whole mileage route for mileage validation.
   * Retrieves MPM for global origin and destination.
   * Applies MPM reduction if appropriate.
   *
   * @param MileageRoute& - mileage route for which MPM is to be collected
   * @return bool - true if retrieval was successful, false otherwise
   */
  bool collectMileage(MileageRoute& mileageRoute) const;

private:
  /**
   * Auxiliary methods to make unit tests independent of the retrievers.
   */
  virtual bool getMPM(MileageRouteItem&, DataHandle&) const;
};

} // namespace tse

