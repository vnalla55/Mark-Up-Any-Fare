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
#include "DataModel/Services/PassengerTypeCode.h"
#include "DataModel/Services/SubCache.h"
#include "ServiceInterfaces/PassengerTypesService.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{

class PassengerTypesServiceServer : public PassengerTypesService
{
public:
  PassengerTypesServiceServer(const boost::ptr_vector<PassengerTypeCode>& cacheData);

  virtual ~PassengerTypesServiceServer()
  {
  }

  std::shared_ptr<const PassengerTypeCodeItems>
  getPassengerTypeCode(const type::Vendor& vendor, const type::Index& itemNo) const final;

  void add(const type::Vendor& vendor,
           const type::Index& itemNo,
           const PassengerTypeCodeItems& items);

private:
  tax::SubCache<PassengerTypeCodeItem> _passengerTypes;
};

} // namespace tax
