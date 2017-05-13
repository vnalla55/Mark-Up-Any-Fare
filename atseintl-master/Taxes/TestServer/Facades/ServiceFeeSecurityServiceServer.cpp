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

#include "TestServer/Facades/ServiceFeeSecurityServiceServer.h"

namespace tax
{
ServiceFeeSecurityServiceServer::ServiceFeeSecurityServiceServer(
    const boost::ptr_vector<ServiceFeeSecurity>& data)
{
  for (const ServiceFeeSecurity& sfs : data)
  {
    _services.add(sfs);
  }
}

std::shared_ptr<const ServiceFeeSecurityItems>
ServiceFeeSecurityServiceServer::getServiceFeeSecurity(const type::Vendor& vendor,
                                                       const type::Index& itemNo) const
{
  return std::shared_ptr<const ServiceFeeSecurityItems>(
      new ServiceFeeSecurityItems(_services.get(vendor, itemNo)));
}

} // namespace tax
