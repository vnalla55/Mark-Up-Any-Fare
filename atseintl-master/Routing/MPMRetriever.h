#pragma once

namespace tse
{

class MileageRouteItem;
class DataHandle;

/**
 * @class MPMRetriever
 *
 * MPM retriever for individual market.
 */
class MPMRetriever
{
public:
  virtual ~MPMRetriever();

  /**
   * @function retrieve
   *
   * Algorithm of MPM retrieval.
   * Calls Mileage, MultiTransport, MileageSubstitution and
   * AdditionalMileage retrievers if necessary.
   *
   * @param MileageRouteItem& - mileage route item for which MPM is to be retrieved
   * @param DataHandle& - data handle which will be passed to more specialized retrievers
   * @return bool - true if retrieval was successful, false otherwise
   */
  bool retrieve(MileageRouteItem&, DataHandle&) const;

private:
  /**
   * Auxiliary methods to make unit tests independent of the retrievers.
   */
  virtual bool getMileage(MileageRouteItem&, DataHandle&) const;
  virtual bool getMileageSubstitution(MileageRouteItem&, DataHandle&) const;
  virtual bool getAdditionalMileage(MileageRouteItem&, DataHandle&) const;
};

} // namespace tse

