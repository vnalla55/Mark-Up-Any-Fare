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

#include "Rules/CurrencyOfSaleRule.h"
#include "Rules/CurrencyOfSaleApplicator.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

CurrencyOfSaleRule::CurrencyOfSaleRule(const type::CurrencyCode& currencyOfSale)
  : _currencyOfSale(currencyOfSale)
{
}

CurrencyOfSaleRule::~CurrencyOfSaleRule() {}

CurrencyOfSaleRule::ApplicatorType
CurrencyOfSaleRule::createApplicator(const type::Index& /*itinIndex*/,
                                     const Request& request,
                                     Services& /*services*/,
                                     RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this,
                        _currencyOfSale,
                        request.ticketingOptions().paymentCurrency(),
                        request.ticketingOptions().formOfPayment());
}

std::string
CurrencyOfSaleRule::getDescription(Services&) const
{
  return "SALE CURRENCY MUST MATCH TO " + _currencyOfSale.asString();
}
}
