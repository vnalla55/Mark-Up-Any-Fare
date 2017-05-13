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
#include "Rules/CurrencyOfSaleApplicator.h"
#include "Rules/CurrencyOfSaleRule.h"

namespace tax
{

CurrencyOfSaleApplicator::CurrencyOfSaleApplicator(const CurrencyOfSaleRule& rule,
                                                   const type::CurrencyCode& currencyOfSale,
                                                   const type::CurrencyCode& paymentCurrency,
                                                   const type::FormOfPayment& formOfPayment)
  : BusinessRuleApplicator(&rule),
    _currencyOfSale(currencyOfSale),
    _paymentCurrency(paymentCurrency),
    _formOfPayment(formOfPayment)
{
}

CurrencyOfSaleApplicator::~CurrencyOfSaleApplicator() {}

bool
CurrencyOfSaleApplicator::apply(PaymentDetail& /*paymentDetail*/) const
{
  return (_currencyOfSale == _paymentCurrency) || isMilesFormOfPayment();
}

bool
CurrencyOfSaleApplicator::isMilesFormOfPayment() const
{
  return (type::CurrencyCode("ZZZ") == _currencyOfSale) &&
         (type::FormOfPayment(type::FormOfPayment::Miles) == _formOfPayment);
}

} // namespace tax
