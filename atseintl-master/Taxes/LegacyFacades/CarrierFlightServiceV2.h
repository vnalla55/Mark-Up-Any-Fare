// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/ChildCache.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxCarrierFlightDAO.h"
#include "DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/CarrierFlightService.h"
#include "Taxes/LegacyFacades/ApplicationCache.h"
#include "Taxes/LegacyFacades/TransactionCache.h"

#include <memory>

namespace tse
{
class DataHandle;

class CarrierFlightServiceV2 : public tax::CarrierFlightService
{
public:
  typedef tax::CarrierFlight Value;
  typedef std::shared_ptr<const Value> SharedConstValue;
  typedef std::pair<tax::type::Vendor, tax::type::Index> Key;

  explicit CarrierFlightServiceV2(const DateTime& ticketingDate);
  ~CarrierFlightServiceV2() {}

  std::shared_ptr<const tax::CarrierFlight>
  getCarrierFlight(const tax::type::Vendor& vendor, const tax::type::Index& itemNo) const final;

private:
  CarrierFlightServiceV2(const CarrierFlightServiceV2&);
  CarrierFlightServiceV2& operator=(const CarrierFlightServiceV2&);

  mutable TransactionCache<Key, Value> _cache;
  const bool _isHistorical;
  typename ApplicationCache<Key, Value>::UpdateFunction _applicationCacheUpdatFunction;

  class RegularService : public ChildCache<TaxCarrierFlightKey>
  {
  public:
    static RegularService& instance();
    void keyRemoved(const TaxCarrierFlightKey& key) override;
    void cacheCleared() override;
    SharedConstValue
    get(const Key&, typename ApplicationCache<Key, Value>::UpdateFunction function);

    ~RegularService();
  private:
    RegularService();
    RegularService(const RegularService&);
    RegularService& operator=(const RegularService&);

    ApplicationCache<Key, Value> _cache;
  };
};
}
