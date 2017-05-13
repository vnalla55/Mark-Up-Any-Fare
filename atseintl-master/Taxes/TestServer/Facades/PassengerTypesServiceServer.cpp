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
#include <stdexcept>
#include "TestServer/Facades/PassengerTypesServiceServer.h"

namespace tax
{


PassengerTypesServiceServer::PassengerTypesServiceServer(const boost::ptr_vector<PassengerTypeCode>& cacheData)
{
  for(const PassengerTypeCode& rec : cacheData)
  {
    _passengerTypes.add(rec);
  }
}

std::shared_ptr<const PassengerTypeCodeItems>
PassengerTypesServiceServer::getPassengerTypeCode(const type::Vendor& vendor,
                                                  const type::Index& itemNo) const
{
  return std::shared_ptr<const PassengerTypeCodeItems>(
      new PassengerTypeCodeItems(_passengerTypes.get(vendor, itemNo)));
}

} // namespace tax
