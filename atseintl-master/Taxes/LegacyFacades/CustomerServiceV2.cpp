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

#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/CustomerServiceV2.h"


namespace tse {

CustomerServiceV2::CustomerServiceV2(DataHandle& data)
  : _dataHandle(data)
{
}

tax::CustomerPtr
CustomerServiceV2::getCustomer(const tax::type::PseudoCityCode& pcc) const
{
  auto customers = _dataHandle.getCustomer(toTsePseudoCityCode(pcc));
  if (customers.empty())
    return tax::CustomerPtr();

  tse::Customer* customer = customers.front();

  return std::make_shared<tax::CustomerPtr::element_type>(
      customer->pricingApplTag8()=='Y' ? true : false,
      customer->pricingApplTag9()=='Y' ? true : false,
      customer->pricingApplTag10()=='Y' ? true : false,
      pcc);
}

} /* namespace tse */
