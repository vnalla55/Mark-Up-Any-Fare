// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "test/CurrencyServiceMock.h"

namespace tax
{
CurrencyService*
CurrencyServiceMock::create()
{
  return new CurrencyServiceMock;
}

type::MoneyAmount
CurrencyServiceMock::convertTo(const type::CurrencyCode& /* targetCurrency */,
                               const type::Money& /* source */,
                               CalcDetails* /* calcDetails */) const
{
  return _converted;
}

type::BSRValue
CurrencyServiceMock::getBSR(const type::CurrencyCode& /* fromCurrency */,
                            const type::CurrencyCode& /* toCurrency */) const
{
  return type::BSRValue(0);
}

type::CurDecimals
CurrencyServiceMock::getCurrencyDecimals(const type::CurrencyCode&) const
{
  return 2;
}

type::CurrencyCode
CurrencyServiceMock::getNationCurrency(const type::CurrencyCode& /*taxCurrency*/,
                                       const type::Nation& /*nation*/) const
{
  return type::CurrencyCode("USD");
}

void
CurrencyServiceMock::setConverted(const type::MoneyAmount& converted)
{
  _converted = converted;
}

} // namespace tax
