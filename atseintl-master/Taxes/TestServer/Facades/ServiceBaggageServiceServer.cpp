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

#include "ServiceBaggageServiceServer.h"
#include <sstream>

namespace tax
{
typedef std::shared_ptr<const ServiceBaggage> SharedConstValue;

SharedConstValue
ServiceBaggageServiceServer::getServiceBaggage(const type::Vendor& vendor,
                                               const type::Index& itemNo) const
{
  for(const ServiceBaggage & sb : _serviceBaggage)
  {
    if (sb.vendor == vendor && sb.itemNo == itemNo)
    {
      return SharedConstValue(new ServiceBaggage(sb));
    }
  }
  return SharedConstValue();
}

} // namespace tax
