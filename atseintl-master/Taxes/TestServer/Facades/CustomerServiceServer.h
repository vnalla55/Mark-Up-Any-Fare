// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "AtpcoTaxes/ServiceInterfaces/CustomerService.h"

namespace tax {

class CustomerServiceServer: public CustomerService
{
public:
  CustomerServiceServer(const boost::ptr_vector<Customer>& customers);
  virtual ~CustomerServiceServer();

  CustomerPtr getCustomer(const type::PseudoCityCode& pcc) const override;

private:
  const boost::ptr_vector<Customer>& _customers;
};

} /* namespace tax */

