#pragma once

namespace tse
{

class MileageRoute;
class MileageRouteItem;
class DataHandle;

/**
 * @class TPMCollectorMV
 *
 * TPM collector for Mileage Validation
 */
class TPMCollectorMV
{
public:
  virtual ~TPMCollectorMV();

  /**
   * @function collectMileage
   *
   * Algorithm of TPM retrieval for whole mileage route for mileage validation.
   * Retrieves TPM for each item in the route.
   * Applies SurfaceSectorExempt (only for surface segments) and
   * SouthAtlanticExclusion if appropriate.
   * Calls TPMRetriever, SurfaceSectorExempt and SouthAtlanticExclusion.
   *
   * @param MileageRoute& - mileage route for which TPM is to be collected
   * @return bool - true if retrieval was successful, false otherwise
   */
  bool collectMileage(MileageRoute& mileageRoute) const;

private:
  /**
   * Auxiliary methods to make unit tests independent of the retrievers.
   */
  virtual bool getTPM(MileageRouteItem&, DataHandle&) const;
  virtual bool getSurfaceSector(MileageRouteItem&, DataHandle&) const;
  virtual bool getSouthAtlantic(MileageRoute&) const;
};

} // namespace tse

