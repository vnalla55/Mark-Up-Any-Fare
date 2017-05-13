#pragma once

#include "Common/Singleton.h"
#include "Common/TseConsts.h"
#include "DBAccess/TariffMileageAddon.h"
#include "Routing/MileageDataRetriever.h"

namespace tse
{

class DataHandle;
class MileageRouteItem;

/**
 * @class AdditionalMileageRetriever
 *
 * A singleton - stateless class to retrieve data from AdditionalMileage table.
 */
class AdditionalMileageRetriever : public MileageDataRetriever
{
public:
  /**
   * @function retrieve
   *
   * Concrete implementation of retrieval from AdditionalMileage table.
   *
   * @param MileageRouteItem& - mileage route item that provides data needed to match and retrieve
   *data
   *				 from db, and to be updated with the retrieved data
   * @param DataHandle& - data handle through which the data will be actually retrieved
   * @param Indicator - ignored
   * @return bool - true if retrieval was successful, false otherwise
   */
  virtual bool retrieve(MileageRouteItem&, DataHandle&, Indicator mileageType = TPM) const override;

private:
  virtual const TariffMileageAddon*
  getData(DataHandle&, const LocCode&, CarrierCode&, GlobalDirection&, const DateTime&) const;
  virtual bool getMileage(DataHandle&, MileageRouteItem&) const;

protected:
  /**
   * Construction, destruction and copying prohibited for a singleton.
   */
  AdditionalMileageRetriever() {}
  AdditionalMileageRetriever(const AdditionalMileageRetriever&);
  AdditionalMileageRetriever& operator=(const AdditionalMileageRetriever&);
  /**
   * Protected destructor not really necessary, Should it cause any problem,
   * can be safely made back public (and friendship of unique_ptr removed).
   */
  ~AdditionalMileageRetriever() {}
  friend class tse::Singleton<AdditionalMileageRetriever>;
  friend class std::unique_ptr<AdditionalMileageRetriever>;
};

} // namespace tse

