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

#include "DataModel/Services/Customer.h"

#include <memory>

namespace tax
{

typedef std::shared_ptr<const Customer> CustomerPtr;

class CustomerService
{
public:
  CustomerService() {}
  virtual ~CustomerService() {}

  virtual CustomerPtr getCustomer(const type::PseudoCityCode& pcc) const = 0;
};
}
