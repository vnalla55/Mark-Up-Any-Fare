#pragma once

#include "Common/Singleton.h"

namespace tse
{

class MileageRoute;

/**
 * @class Collector
 *
 * A singleton - stateless generic mileage collector for whole mileage route.
 * To be specialized for TPM or MPM, WN or MV.
 * An interface class to help concrete classes depend on abstraction,
 * not other concrete classes.
 */
template <typename CollectorImpl>
class Collector
{
public:
  /**
   * @function collectMileage
   *
   * Simply delegates the call to specific collector implementation.
   *
   * @param MileageRoute& - mileage route for which data is to be retrieved
   * @return bool - true if retrieval was successful, false otherwise
   */
  bool collectMileage(MileageRoute& mileageRoute) const
  {
    return _collectorImpl.collectMileage(mileageRoute);
  }

private:
  /**
   * Specific collector implementation.
   */
  CollectorImpl _collectorImpl;

  /**
   * Construction and copying prohibited for a singleton.
   */
  Collector() {}
  Collector(const Collector&);
  Collector& operator=(const Collector&);
  /**
   * Private destructor not really necessary, Should it cause any problem,
   * can be safely made back public (and friendship of unique_ptr removed).
   */
  ~Collector() {}
  friend class tse::Singleton<Collector<CollectorImpl> >;
  friend class std::unique_ptr<Collector<CollectorImpl>>;
};

} // namespace tse

