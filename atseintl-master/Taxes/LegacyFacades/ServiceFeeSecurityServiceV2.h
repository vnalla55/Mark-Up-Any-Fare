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

#include "Common/DateTime.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/ServiceFeeSecurity.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/ServiceFeeSecurityService.h"
#include "Taxes/LegacyFacades/TransactionCache.h"

#include <memory>

namespace tse
{

class DateTime;

class ServiceFeeSecurityServiceV2 : public tax::ServiceFeeSecurityService
{
public:
  typedef std::pair<tax::type::Vendor, tax::type::Index> Key;
  typedef tax::ServiceFeeSecurityItems Value;
  typedef std::shared_ptr<const Value> SharedConstValue;

  ServiceFeeSecurityServiceV2(const DateTime&);

  SharedConstValue
  getServiceFeeSecurity(const tax::type::Vendor& vendor, const tax::type::Index& itemNo) const final;

private:
  mutable TransactionCache<Key, Value> _cache;
};

} // namespace tax
