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

#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/Common/Types.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "ServiceInterfaces/ServiceBaggageService.h"

#include <memory>

namespace tax
{
class ServiceBaggageServiceServer : public ServiceBaggageService
{
public:
  ServiceBaggageServiceServer() {}
  virtual ~ServiceBaggageServiceServer() {}

  std::shared_ptr<const ServiceBaggage>
  getServiceBaggage(const type::Vendor& vendor, const type::Index& itemNo) const final;

  boost::ptr_vector<ServiceBaggage>& serviceBaggage() { return _serviceBaggage; }

  const boost::ptr_vector<ServiceBaggage>& serviceBaggage() const { return _serviceBaggage; };

private:
  boost::ptr_vector<ServiceBaggage> _serviceBaggage;
};
}
