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
#include "DataModel/Services/ServiceFeeSecurity.h"

#include <memory>

namespace tax
{

class ServiceFeeSecurityService
{
public:
  ServiceFeeSecurityService() {}
  virtual ~ServiceFeeSecurityService() {}

  virtual std::shared_ptr<const ServiceFeeSecurityItems>
  getServiceFeeSecurity(const type::Vendor& vendor, const type::Index& itemNo) const = 0;
};

} // namespace tax
