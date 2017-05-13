#pragma once

#include "Common/Singleton.h"
#include "Common/TseConsts.h"
#include "Routing/MileageDataRetriever.h"

namespace tse
{

class DataHandle;
class DateTime;
class MileageRouteItem;
class SurfaceSectorExempt;

/**
 * @class SurfaceSectorExemptRetriever
 *
 * A singleton - stateless class to retrieve data from SurfaceSectorExempt table.
 */
class SurfaceSectorExemptRetriever : public MileageDataRetriever
{
public:
  /**
   * @function retrieve
   *
   * Concrete implementation of retrieval from SurfaceSectorExempt table.
   *
   * @param MileageRouteItem& - mileage route item that provides data needed to match and retrieve
   *data
   *				 from db, and to be updated with the retrieved data
   * @param DataHandle& - data handle through which the data will be actually retrieved
   * @param Indicator - ignored
   * @return bool - true if retrieval was successful, false otherwise
   */
  virtual bool retrieve(MileageRouteItem&, DataHandle&, Indicator mileageType = TPM) const override;

protected:
  /**
   * Construction, destruction and copying prohibited for a singleton.
   */
  SurfaceSectorExemptRetriever() {}
  SurfaceSectorExemptRetriever(const SurfaceSectorExemptRetriever&);
  SurfaceSectorExemptRetriever& operator=(const SurfaceSectorExemptRetriever);
  /**
   * Protected destructor not really necessary, Should it cause any problem,
   * can be safely made back public (and friendship of unique_ptr removed).
   */
  ~SurfaceSectorExemptRetriever() {}
  friend class tse::Singleton<SurfaceSectorExemptRetriever>;
  friend class std::unique_ptr<SurfaceSectorExemptRetriever>;

private:
  /**
   * An auxiliary method to make unit tests independent of DataHandle.
   */
  virtual const SurfaceSectorExempt*
  getData(DataHandle&, const LocCode&, const LocCode&, const DateTime&) const;
};

} // namespace tse

