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

#include <boost/bind.hpp>

#include "DBAccess/DataHandle.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/CarrierFlight.h"
#include "Taxes/LegacyFacades/CarrierFlightServiceV2.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "Taxes/LegacyFacades/ConvertCode.h"

namespace tse
{
namespace
{
typedef CarrierFlightServiceV2::Key Key;
typedef CarrierFlightServiceV2::Value Value;
typedef CarrierFlightServiceV2::SharedConstValue SharedConstValue;

SharedConstValue
readData(const DateTime& ticketingDate, const Key& key)
{
  DataHandle dataHandle(ticketingDate);
  std::unique_ptr<tax::CarrierFlight> result;

  const tse::TaxCarrierFlightInfo* v2cf =
      dataHandle.getTaxCarrierFlight(toTseVendorCode(key.first), static_cast<int>(key.second));
  if (v2cf)
  {
    result.reset(new tax::CarrierFlight());
    DaoDataFormatConverter::convert(*v2cf, *result);
  }

  return SharedConstValue(std::move(result));
}
}

CarrierFlightServiceV2::CarrierFlightServiceV2(const DateTime& ticketingDate)
  : _cache(boost::bind(&readData, ticketingDate, _1)),
    _isHistorical(DataHandle(ticketingDate).isHistorical())
{
  if (!_isHistorical)
    _applicationCacheUpdatFunction = boost::bind(&readData, ticketingDate, _1);
}

std::shared_ptr<const tax::CarrierFlight>
CarrierFlightServiceV2::getCarrierFlight(const tax::type::Vendor& vendor,
                                         const tax::type::Index& itemNo) const
{
  if (_isHistorical)
    return _cache.get(std::make_pair(vendor, itemNo));

  return RegularService::instance().get(std::make_pair(vendor, itemNo),
                                        _applicationCacheUpdatFunction);
}

// Single Service
CarrierFlightServiceV2::RegularService&
CarrierFlightServiceV2::RegularService::instance()
{
  static RegularService instance;
  return instance;
}

CarrierFlightServiceV2::RegularService::RegularService()
{
  TaxCarrierFlightDAO::instance().addListener(*this);
}

CarrierFlightServiceV2::RegularService::~RegularService()
{
  TaxCarrierFlightDAO::instance().removeListener(*this);
}

void
CarrierFlightServiceV2::RegularService::keyRemoved(const TaxCarrierFlightKey& key)
{
  Key internalKey = std::make_pair(toTaxVendorCode(key._a), key._b);
  _cache.deleteKey(internalKey);
}

void
CarrierFlightServiceV2::RegularService::cacheCleared()
{
  _cache.clear();
}

CarrierFlightServiceV2::SharedConstValue
CarrierFlightServiceV2::RegularService::get(
    const Key& key, typename ApplicationCache<Key, Value>::UpdateFunction function)
{
  return _cache.get(key, function);
}
}
