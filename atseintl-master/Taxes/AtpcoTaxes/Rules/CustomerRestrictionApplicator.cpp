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

#include "Rules/CustomerRestrictionApplicator.h"
#include "Rules/CustomerRestrictionRule.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/CustomerService.h"

namespace tax
{

CustomerRestrictionApplicator::CustomerRestrictionApplicator(const CustomerRestrictionRule& parent,
    const CustomerService& customer, const type::PseudoCityCode& pcc)
  : BusinessRuleApplicator(&parent)
  , _customerRestrictionRule(parent)
  , _customerService(customer)
  , _pcc(pcc)
{
}

bool
CustomerRestrictionApplicator::apply(PaymentDetail& paymentDetail) const
{
  CustomerPtr ptrCustomer = _customerService.getCustomer(_pcc);
  if (!ptrCustomer)
    return true;

  paymentDetail.applicatorFailMessage() = "CUSTOMER ";
  paymentDetail.applicatorFailMessage() += _pcc.asString();
  paymentDetail.applicatorFailMessage() += " HAS TO EXEMPT THIS TAX";

  if (_customerRestrictionRule.carrierCode() == "G3")
    return !ptrCustomer->_exemptDuG3;
  else if (_customerRestrictionRule.carrierCode() == "JJ")
    return !ptrCustomer->_exemptDuJJ;
  else if (_customerRestrictionRule.carrierCode() == "T4")
    return !ptrCustomer->_exemptDuT4;

  return true;
}

} /* namespace tax */
