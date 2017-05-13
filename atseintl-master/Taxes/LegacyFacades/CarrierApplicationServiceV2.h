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

#include "DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/CarrierApplicationService.h"
#include "Taxes/LegacyFacades/TransactionCache.h"

#include <memory>

namespace tax
{
class CarrierApplication;
}
namespace tse
{
class DateTime;

class CarrierApplicationServiceV2 : public tax::CarrierApplicationService
{
public:
  typedef std::pair<tax::type::Vendor, tax::type::Index> Key;
  typedef tax::CarrierApplication Value;
  typedef std::shared_ptr<const Value> SharedConstValue;

  explicit CarrierApplicationServiceV2(const DateTime& ticketingDate);
  ~CarrierApplicationServiceV2() {}

  SharedConstValue
  getCarrierApplication(const tax::type::Vendor& vendor, const tax::type::Index& itemNo) const final;

private:
  CarrierApplicationServiceV2(const CarrierApplicationServiceV2&);
  CarrierApplicationServiceV2& operator=(const CarrierApplicationServiceV2&);

  mutable TransactionCache<Key, Value> _cache;
};
}
