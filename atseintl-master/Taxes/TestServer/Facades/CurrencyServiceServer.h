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
#pragma once

#include "AtpcoTaxes/DataModel/Common/Types.h"
#include "AtpcoTaxes/DataModel/Services/Currency.h"
#include "AtpcoTaxes/DataModel/Services/CurrencyConversion.h"
#include "AtpcoTaxes/ServiceInterfaces/CurrencyService.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{
class CurrencyServiceServer : public CurrencyService
{
public:
  typedef boost::ptr_vector<CurrencyConversion> CurrencyConversions;
  typedef boost::ptr_vector<Currency> Currencies;

  CurrencyServiceServer(void) {}
  virtual ~CurrencyServiceServer(void) {}

  CurrencyConversions& currencyConversions();
  Currencies& currencies();

  tax::type::MoneyAmount
  convertTo(const tax::type::CurrencyCode& targetCurrency, const tax::type::Money& source,
            CalcDetails* calcDetails = nullptr) const;

  type::BSRValue
  getBSR(const type::CurrencyCode& fromCurrency, const type::CurrencyCode& toCurrency) const;

  type::CurDecimals getCurrencyDecimals(const type::CurrencyCode&) const;

  tax::type::CurrencyCode getNationCurrency(const tax::type::CurrencyCode& taxCurrency,
                                            const tax::type::Nation& nation) const;

private:
  CurrencyConversions _currencyConversions;
  Currencies _currencies;
};

} // namespace tax
