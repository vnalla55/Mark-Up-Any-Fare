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

#include <map>
#include "DataModel/Common/Types.h"
#include "DataModel/Services/ServiceFeeSecurity.h"
#include "ServiceInterfaces/ServiceFeeSecurityService.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include "DataModel/Services/SubCache.h"

namespace tax
{

class ServiceFeeSecurityServiceServer : public ServiceFeeSecurityService
{
public:
  ServiceFeeSecurityServiceServer(const boost::ptr_vector<ServiceFeeSecurity>&);
  virtual ~ServiceFeeSecurityServiceServer()
  {
  }

  std::shared_ptr<const ServiceFeeSecurityItems>
  getServiceFeeSecurity(const type::Vendor& vendor, const type::Index& itemNo) const final;

  void add(const ServiceFeeSecurity& sfs)
  {
    SubCache<ServiceFeeSecurityItem>::key_type key(sfs.vendor, sfs.itemNo);
    _services.addAll(key, sfs.entries.begin(), sfs.entries.end());
  };

private:
  SubCache<ServiceFeeSecurityItem> _services;
};

} // namespace tax
