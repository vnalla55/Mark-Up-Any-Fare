#pragma once

#include "Common/Singleton.h"
#include "Common/TseConsts.h"
#include "Routing/MileageDataRetriever.h"

namespace tse
{

class DataHandle;
class DateTime;
class MileageRouteItem;
class Loc;

/**
 * @class MultiTransportRetriever
 *
 * A singleton - stateless class to retrieve data from MultiTransport table.
 */
class MultiTransportRetriever : public MileageDataRetriever
{
public:
  /**
   * @function retrieve
   *
   * Concrete implementation of retrieval from MultiTransport table.
   *
   * @param MileageRouteItem& - mileage route item that provides data needed to match and retrieve
   *data
   *				 from db, and to be updated with alternate locations
   * @param DataHandle& - data handle through which the data will be actually retrieved
   * @param Indicator - ignored
   * @return bool - true if retrieval was successful, false otherwise
   */
  virtual bool retrieve(MileageRouteItem&, DataHandle&, Indicator mileageType = TPM) const override;
  virtual LocCode retrieve(DataHandle&, const LocCode&, const DateTime&) const;

protected:
  /**
   * Construction, destruction and copying prohibited for a singleton.
   */
  MultiTransportRetriever() {}
  MultiTransportRetriever(const MultiTransportRetriever&);
  MultiTransportRetriever& operator=(const MultiTransportRetriever&);
  /**
   * Protected destructor not really necessary, Should it cause any problem,
   * can be safely made back public (and friendship of unique_ptr removed).
   */
  ~MultiTransportRetriever() {}
  friend class tse::Singleton<MultiTransportRetriever>;
  friend class std::unique_ptr<MultiTransportRetriever>;

private:
  /**
   * An auxiliary method to make unit tests independent of DataHandle.
   */
  virtual const LocCode* getData(DataHandle&,
                                 const LocCode&,
                                 const CarrierCode&,
                                 GeoTravelType geoTvlType,
                                 const DateTime& tvlDate) const;
  virtual const Loc* getLoc(DataHandle&, const LocCode&, const DateTime&) const;
};

} // namespace tse

