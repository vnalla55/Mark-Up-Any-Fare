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

#include "Taxes/AtpcoTaxes/ServiceInterfaces/CustomerService.h"

namespace tse
{

class DataHandle;

class CustomerServiceV2 : public tax::CustomerService
{
public:
  CustomerServiceV2(DataHandle& data);

  tax::CustomerPtr getCustomer(const tax::type::PseudoCityCode& pcc) const override;

private:
  CustomerServiceV2(const CustomerServiceV2&) = delete;
  CustomerServiceV2& operator=(const CustomerServiceV2&) = delete;

  DataHandle& _dataHandle;
};

} /* namespace tse */


