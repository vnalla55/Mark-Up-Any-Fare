#pragma once

#include "ServiceInterfaces/CurrencyService.h"

namespace tax
{
class CurrencyServiceMock : public CurrencyService
{
public:
  static CurrencyService* create();

  type::MoneyAmount
  convertTo(const type::CurrencyCode& /* targetCurrency */, const type::Money& /* source */,
            CalcDetails* /* calcDetails */) const override;

  type::BSRValue getBSR(const type::CurrencyCode& /* fromCurrency */,
                        const type::CurrencyCode& /* toCurrency */) const override;

  type::CurDecimals getCurrencyDecimals(const type::CurrencyCode&) const;

  type::CurrencyCode getNationCurrency(const type::CurrencyCode& taxCurrency,
                                       const type::Nation& /*nation*/) const override;

  void setConverted(const type::MoneyAmount& converted);

private:
  type::MoneyAmount _converted;
};

} // namespace tax
