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

#include "ServiceInterfaces/PassengerTypesService.h"
#include "Taxes/LegacyFacades/TransactionCache.h"

#include <memory>

namespace tse
{

class PassengerTypesServiceV2 : public tax::PassengerTypesService
{
public:
  typedef std::pair<tax::type::Vendor, tax::type::Index> Key;
  typedef tax::PassengerTypeCodeItems Value;
  typedef std::shared_ptr<const Value> SharedConstValue;

  PassengerTypesServiceV2(const DateTime& ticketingDate);
  virtual ~PassengerTypesServiceV2() {}

  SharedConstValue
  getPassengerTypeCode(const tax::type::Vendor& vendor, const tax::type::Index& itemNo) const final;

private:
  PassengerTypesServiceV2(const PassengerTypesServiceV2&);
  PassengerTypesServiceV2& operator=(const PassengerTypesServiceV2&);

  mutable TransactionCache<Key, Value> _cache;
};
}
