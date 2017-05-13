#pragma once

namespace tse
{

class MileageRouteItem;
class DataHandle;
class GDPrompt;

/**
 * @class TPMRetrieverWN
 *
 * TPM retriever for individual market.
 */
class TPMRetrieverWN
{
public:
  virtual ~TPMRetrieverWN();

  /**
   * @function retrieve
   *
   * Algorithm of TPM retrieval for WN.
   * Calls Mileage, MultiTransport, MileageSubstitution retrievers and
   * TPMConstructor if necessary.
   * Determines GlobalDirection at the same time as mileage because in WN
   * GlobalDirection and TPM retrieval are tightly coupled.
   *
   * @param MileageRouteItem& - mileage route item for which TPM is to be retrieved
   * @param DataHandle& - data handle which will be passed to more specialized retrievers
   * @return bool - true if retrieval was successful, false otherwise
   */
  bool retrieve(MileageRouteItem&, DataHandle&, GDPrompt*&) const;

private:
  bool tpmGlobalDirection(MileageRouteItem&, DataHandle&, GDPrompt*&) const;
  bool mpmBasedGD(MileageRouteItem&, DataHandle&, GDPrompt*&) const;
  /**
   * Auxiliary methods to make unit tests independent of the retrievers.
   */
  virtual bool getMileage(MileageRouteItem&, DataHandle&) const;
  virtual bool getMileageSubstitution(MileageRouteItem&, DataHandle&) const;
  virtual bool getConstructed(MileageRouteItem&, DataHandle&) const;
};

} // namespace tse

