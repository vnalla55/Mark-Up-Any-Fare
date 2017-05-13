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

#include "ServiceInterfaces/ServiceBaggageService.h"
#include "Taxes/LegacyFacades/TransactionCache.h"

#include <memory>

namespace tax
{
class ServiceBaggage;
}

namespace tse
{

class ServiceBaggageServiceV2 : public tax::ServiceBaggageService
{
public:
  typedef std::pair<tax::type::Vendor, tax::type::Index> Key;
  typedef tax::ServiceBaggage Value;
  typedef std::shared_ptr<const Value> SharedConstValue;

  ServiceBaggageServiceV2(const DateTime& ticketingDate);
  virtual ~ServiceBaggageServiceV2() {}

  SharedConstValue
  getServiceBaggage(const tax::type::Vendor& vendor, const tax::type::Index& itemNo) const final;

private:
  ServiceBaggageServiceV2(const ServiceBaggageServiceV2&);
  ServiceBaggageServiceV2& operator=(const ServiceBaggageServiceV2&);

  mutable TransactionCache<Key, Value> _cache;
};
}
