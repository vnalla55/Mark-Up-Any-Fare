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
#include <stdexcept>
#include "CurrencyServiceServer.h"

namespace tax
{
CurrencyServiceServer::CurrencyConversions&
CurrencyServiceServer::currencyConversions()
{
  return _currencyConversions;
}

CurrencyServiceServer::Currencies&
CurrencyServiceServer::currencies()
{
  return _currencies;
}

tax::type::MoneyAmount
CurrencyServiceServer::convertTo(const tax::type::CurrencyCode& targetCurrency,
                                 const tax::type::Money& source,
                                 CalcDetails* /* calcDetails = nullptr */) const
{
  if (source._currency == targetCurrency)
  {
    return source._amount;
  }

  for (const CurrencyConversion& conversion : _currencyConversions)
  {
    if (conversion.fromCurrency == source._currency && conversion.toCurrency == targetCurrency)
    {
      return source._amount * conversion.bsr;
    }
  }

  throw std::runtime_error("Cannot find BSR rate");
}

type::BSRValue
CurrencyServiceServer::getBSR(const type::CurrencyCode& fromCurrency,
                              const type::CurrencyCode& toCurrency) const
{
  if (fromCurrency == toCurrency)
  {
    return type::BSRValue(1);
  }

  for (const CurrencyConversion& conversion : _currencyConversions)
  {
    if (conversion.fromCurrency == fromCurrency && conversion.toCurrency == toCurrency)
    {
      return conversion.bsr;
    }
  }

  return type::BSRValue(0);
}

type::CurDecimals
CurrencyServiceServer::getCurrencyDecimals(const type::CurrencyCode& requestedCurrency) const
{
  for (const Currency& currency : _currencies)
  {
    if (currency.currencyCode == requestedCurrency)
      return currency.currencyDecimals;
  }
  return type::CurDecimals(2u);
}


tax::type::CurrencyCode
CurrencyServiceServer::getNationCurrency(const tax::type::CurrencyCode& taxCurrency,
                                         const tax::type::Nation& /*nation*/) const
{
  if (!taxCurrency.empty())
    return taxCurrency;

  // Need to add info about nation currencies - just for test server, for now return USD

  return tax::type::CurrencyCode("USD");
}
}
