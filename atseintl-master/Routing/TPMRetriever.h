#pragma once

namespace tse
{

class MileageRouteItem;
class DataHandle;

/**
 * @class TPMRetriever
 *
 * TPM retriever for individual market.
 */
class TPMRetriever
{
public:
  virtual ~TPMRetriever();

  /**
   * @function retrieve
   *
   * Algorithm of TPM retrieval.
   * Calls Mileage, MultiTransport, MileageSubstitution retrievers and
   * TPMConstructor if necessary.
   *
   * @param MileageRouteItem& - mileage route item for which TPM is to be retrieved
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
  virtual bool getConstructed(MileageRouteItem&, DataHandle&) const;
};

} // namespace tse

