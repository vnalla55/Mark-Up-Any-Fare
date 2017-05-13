#pragma once

// FIXME: <memory> should be included by Singleton.h
#include "Common/Singleton.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Routing/MileageRoute.h"
#include "Routing/RoutingConsts.h"

#include <memory>
#include <vector>

namespace tse
{

class TpdPsr;
class DataHandle;
class MileageRouteItem;

/**
 * @class PSRRetriever
 *
 * A singleton - stateless class to retrieve PSR from TpdPsr table.
 */
class PSRRetriever
{
public:
  /**
   * @function retrieve
   *
   * Concrete implementation of retrieval from TpdPsr table.
   *
   * @param MileageRouteItem& - mileage route item that provides data needed to match and retrieve
   *data
   *				 from db
   * @param DataHandle& - data handle through which the data will be actually retrieved
   * @param Indicator - 'P' for PSR
   * @return bool - true if retrieval was successful, false otherwise
   */

  const std::vector<TpdPsr*>& getpsrData(MileageRoute& mileageRoute) const;

private:
  /**
   * Construction, destruction and copying prohibited for a singleton.
   */
  PSRRetriever() {}
  PSRRetriever(const PSRRetriever&);
  PSRRetriever& operator=(const PSRRetriever&);
  /**
   * Protected destructor not really necessary, Should it cause any problem,
   * can be safely made back public (and friendship of unique_ptr removed).
   */
  ~PSRRetriever() {}
  friend class tse::Singleton<PSRRetriever>;
  friend class std::unique_ptr<PSRRetriever>;

  const std::vector<TpdPsr*>& getPSRData(DataHandle& dataHandle,
                                         Indicator application,
                                         const CarrierCode& carrier,
                                         Indicator area1,
                                         Indicator area2,
                                         const DateTime& ticketingDT,
                                         const DateTime& travelDT) const;
};

} // namespace tse

