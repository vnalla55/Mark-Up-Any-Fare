#pragma once

namespace tse
{

class MileageRoute;
class MileageRouteItem;
class DataHandle;

/**
 * @class TPMCollectorWNfromItin
 *
 * TPM collector for Mileage Service (WN).
 */
class TPMCollectorWNfromItin
{
public:
  virtual ~TPMCollectorWNfromItin();

  /**
   * @function collectMileage
   *
   * Algorithm of TPM retrieval for whole mileage route for WN.
   * Retrieves TPM for each item in the route.
   * Applies SurfaceSectorExempt (only for items different from first and last) and
   * SouthAtlanticExclulsion if applicable.
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
};

} // namespace tse

