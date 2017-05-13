#pragma once

namespace tse
{

class MileageRoute;
class MileageRouteItem;
class DataHandle;
class GDPrompt;

/**
 * @class TPMCollectorWN
 *
 * TPM collector for Mileage Service (WN).
 */
class TPMCollectorWN
{
public:
  virtual ~TPMCollectorWN();

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
  virtual bool getTPM(MileageRouteItem&, DataHandle&, GDPrompt*&) const;
  virtual bool getSurfaceSector(MileageRouteItem&, DataHandle&) const;
};

} // namespace tse

