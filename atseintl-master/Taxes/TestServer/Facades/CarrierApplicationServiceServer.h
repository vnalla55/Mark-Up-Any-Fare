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

#include "DataModel/Services/CarrierApplication.h"
#include "ServiceInterfaces/CarrierApplicationService.h"
#include "DataModel/Common/Types.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{
class CarrierApplicationServiceServer : public CarrierApplicationService
{
public:
  CarrierApplicationServiceServer()
  {
  }
  ~CarrierApplicationServiceServer()
  {
  }

  std::shared_ptr<const CarrierApplication>
  getCarrierApplication(const type::Vendor& vendor, const type::Index& itemNo) const final;

  boost::ptr_vector<CarrierApplication>& carrierApplications()
  {
    return _carrierApplications;
  }

  const boost::ptr_vector<CarrierApplication>& carrierApplications() const
  {
    return _carrierApplications;
  };

private:
  boost::ptr_vector<CarrierApplication> _carrierApplications;
};
}
