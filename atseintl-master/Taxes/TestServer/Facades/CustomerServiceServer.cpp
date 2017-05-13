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

#include "CustomerServiceServer.h"

namespace tax {

CustomerServiceServer::CustomerServiceServer(const boost::ptr_vector<Customer>& customers)
  : _customers(customers)
{
}

CustomerServiceServer::~CustomerServiceServer()
{
}

CustomerPtr
CustomerServiceServer::getCustomer(const type::PseudoCityCode& pcc) const
{
  for (const auto& customer : _customers)
    if (pcc == customer._pcc)
      return std::make_shared<CustomerPtr::element_type>(customer);

  return CustomerPtr();
}

} /* namespace tax */
