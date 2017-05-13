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

#include "Common/CalcDetails.h"
#include "DataModel/Common/Types.h"

namespace tax
{

class CurrencyService
{
public:
  CurrencyService(void) {}
  virtual ~CurrencyService(void) {}

  virtual tax::type::MoneyAmount convertTo(const tax::type::CurrencyCode& targetCurrency,
                                           const tax::type::Money& source,
                                           CalcDetails* calcDetails = nullptr) const = 0;

  virtual type::BSRValue
  getBSR(const type::CurrencyCode& fromCurrency, const type::CurrencyCode& toCurrency) const = 0;

  virtual type::CurDecimals getCurrencyDecimals(const type::CurrencyCode&) const = 0;

  virtual tax::type::CurrencyCode getNationCurrency(const tax::type::CurrencyCode& taxCurrency,
                                                    const tax::type::Nation& nation) const = 0;
};

} // namespace tax
