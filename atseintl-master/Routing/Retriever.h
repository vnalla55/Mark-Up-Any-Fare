#pragma once

#include "Common/Singleton.h"

namespace tse
{

class MileageRouteItem;
class DataHandle;
class GDPrompt;

/**
 * @class Retriever
 *
 * A singleton - stateless generic mileage retriever for individual market.
 * To be specialized for TPM or MPM.
 * An interface class to help concrete classes depend on abstraction,
 * not other concrete classes.
 */
template <typename RetrieverImpl>
class Retriever
{
public:
  /**
   * @function retrieve
   *
   * Simply delegates the call to specific retriever implementation.
   *
   * @param MileageRouteItem& - mileage route item for which data is to be retrieved
   * @param DataHandle& - data handle which will be passed to more specialized retrievers
   * @return bool - true if retrieval was successful, false otherwise
   */
  bool retrieve(MileageRouteItem& mileageRouteItem, DataHandle& dataHandle) const
  {
    return _retrieverImpl.retrieve(mileageRouteItem, dataHandle);
  }
  bool
  retrieve(MileageRouteItem& mileageRouteItem, DataHandle& dataHandle, GDPrompt*& gdPrompt) const
  {
    return _retrieverImpl.retrieve(mileageRouteItem, dataHandle, gdPrompt);
  }

private:
  /**
   * Specific retriever implementation.
   */
  RetrieverImpl _retrieverImpl;

  /**
   * Construction and copying prohibited for a singleton.
   */
  Retriever() {}
  Retriever(const Retriever&);
  Retriever& operator=(const Retriever&);
  /**
   * Private destructor not really necessary, Should it cause any problem,
   * can be safely made back public (and friendship of unique_ptr removed).
   */
  ~Retriever() {}
  friend class tse::Singleton<Retriever<RetrieverImpl> >;
  friend class std::unique_ptr<Retriever<RetrieverImpl>>;
};

} // namespace tse

