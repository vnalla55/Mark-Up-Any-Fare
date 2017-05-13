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

#include "CarrierApplicationServiceServer.h"

#include <sstream>

namespace tax
{
std::shared_ptr<const CarrierApplication>
CarrierApplicationServiceServer::getCarrierApplication(const type::Vendor& vendor,
                                                       const type::Index& itemNo) const
{
  for(const CarrierApplication & ca : _carrierApplications)
  {
    if (ca.vendor == vendor && ca.itemNo == itemNo)
    {
      return std::shared_ptr<const CarrierApplication>(new CarrierApplication(ca));
    }
  }
  return std::shared_ptr<const CarrierApplication>();
}

} // namespace tax
