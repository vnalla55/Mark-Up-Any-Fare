#pragma once

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/FareDisplayTax.h"
#include "Common/Money.h"

namespace tse
{
namespace FareCalc
{

template <class T>
class AccumulateAmount
{
public:
  AccumulateAmount(
      PricingTrx& trx,
      MoneyAmount& total,
      MoneyAmount& supplementalFeeAmount,
      const CurrencyCode& baseCurrency,
      const MoneyAmount& (T::*amount)() const = &T::taxAmount,
      const CurrencyCode& (T::*currency)() const = &T::taxCurrencyCode,
      CurrencyConversionRequest::ApplicationType applType = CurrencyConversionRequest::OTHER)
    : _trx(trx),
      _total(total),
      _supplementalFeeAmount(supplementalFeeAmount),
      _baseCurrency(baseCurrency),
      _amount(amount),
      _currency(currency),
      _applType(applType)
  {
  }

  AccumulateAmount(const AccumulateAmount& rhs)
    : _trx(rhs._trx),
      _total(rhs._total),
      _supplementalFeeAmount(rhs._supplementalFeeAmount),
      _baseCurrency(rhs._baseCurrency),
      _amount(rhs._amount),
      _currency(rhs._currency),
      _applType(rhs._applType)
  {
  }

  void operator()(const T* t)
  {

    if (UNLIKELY(t->taxOnChangeFee()))
      return;

    const CurrencyCode& fromCurrency = (t->*_currency)();
    MoneyAmount toBeAdded;

    if (LIKELY(fromCurrency == _baseCurrency || _baseCurrency.empty()))
    {
      toBeAdded = (t->*_amount)();
    }
    else
    {
      const Money source((t->*_amount)(), fromCurrency);
      Money target(_baseCurrency);

      if (!_ccFacade.convert(target, source, _trx, false, _applType))
        toBeAdded = (t->*_amount)();
      else
        toBeAdded = target.value();
    }

    _total += toBeAdded;

    if (UNLIKELY(FareDisplayTax::isSupplementalFee(t->specialProcessNo())))
      _supplementalFeeAmount += toBeAdded;
  }

private:
  PricingTrx& _trx;
  MoneyAmount& _total;
  MoneyAmount& _supplementalFeeAmount;
  const CurrencyCode& _baseCurrency;

  const MoneyAmount& (T::*_amount)() const;
  const CurrencyCode& (T::*_currency)() const;

  CurrencyConversionFacade _ccFacade;
  CurrencyConversionRequest::ApplicationType _applType;
};
}
}
