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

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/Singleton.h"
#include "Taxes/AtpcoTaxes/Common/MoneyUtil.h"
#include "Taxes/AtpcoTaxes/Rules/MathUtils.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/CurrencyServiceV2.h"

namespace tse
{

namespace
{
Logger logger("atseintl.AtpcoTaxes.CurrencyServiceV2");

class BSRCurrencyConverterV2 : public BSRCurrencyConverter
{
public:
  BSRCurrencyConverterV2() {}

  bool bsrRate(const tax::type::CurrencyCode& bsrPrimeCurrency,
               const tax::type::CurrencyCode& bsrCurrency,
               const tse::DateTime& ticketDate,
               tse::ExchRate& rate,
               CurrencyNoDec& rateNoDec,
               Indicator& rateType)
  {
    return BSRCurrencyConverter::bsrRate(
        bsrPrimeCurrency.asString(), bsrCurrency.asString(), ticketDate, rate, rateNoDec, rateType);
  }
};

void fillCalcDetails(const BSRCollectionResults& bsrResults, tax::CalcDetails& calcDetails)
{
  calcDetails.exchangeRate1 = tax::doubleToAmount(bsrResults.taxReciprocalRate1());
  calcDetails.exchangeRate1NoDec = bsrResults.taxReciprocalRate1NoDec();

  calcDetails.intermediateUnroundedAmount = tax::doubleToAmount(bsrResults.intermediateUnroundedAmount());
  calcDetails.intermediateAmount = tax::doubleToAmount(bsrResults.intermediateAmount());
  calcDetails.intermediateCurrency = toTaxCurrencyCode(bsrResults.intermediateCurrency());
  calcDetails.intermediateNoDec = bsrResults.intermediateNoDec();

  calcDetails.exchangeRate2 = tax::doubleToAmount(bsrResults.taxReciprocalRate2());
  calcDetails.exchangeRate2NoDec = bsrResults.taxReciprocalRate2NoDec();
}

} // namespace

CurrencyServiceV2::CurrencyServiceV2(PricingTrx& trx, const DateTime& ticketDate)
  : _trx(trx),
    _conversionFacade(&Singleton<CurrencyConversionFacade>::instance()),
    _ticketDate(ticketDate)
{
}

CurrencyServiceV2::~CurrencyServiceV2() {}

tax::type::MoneyAmount
CurrencyServiceV2::convertTo(const tax::type::CurrencyCode& targetCurrency,
                           const tax::type::Money& source,
                           tax::CalcDetails* calcDetails /* = nullptr*/) const
{
  if (targetCurrency == source._currency)
    return source._amount;

  tse::Money tseTarget(targetCurrency.asString());
  tse::Money tseSource(tax::amountToDouble(source._amount), source._currency.asString());
  BSRCollectionResults bsrResults;

  if (!_conversionFacade->convert(
          tseTarget, tseSource, _trx, false, CurrencyConversionRequest::TAXES, false, &bsrResults))
  {
    LOG4CXX_ERROR(logger, "Cannot convert currency. CurrencyServiceV2::convertTo()");
    throw std::runtime_error("Cannot convert currency. CurrencyServiceV2::convertTo()");
  }

  if (calcDetails)
    fillCalcDetails(bsrResults, *calcDetails);

  return tax::doubleToAmount(tseTarget.value());
}

tax::type::BSRValue
CurrencyServiceV2::getBSR(const tax::type::CurrencyCode& fromCurrency,
                        const tax::type::CurrencyCode& toCurrency) const
{
  ExchRate rate = 0;
  CurrencyNoDec rateNoDec = 0;
  Indicator rateType = 'B';

  BSRCurrencyConverterV2 converter;
  if (converter.bsrRate(toCurrency, fromCurrency, _ticketDate, rate, rateNoDec, rateType))
  {
    return tax::MathUtils::adjustDecimal(tax::doubleToAmount(rate), rateNoDec);
  }
  else
  {
    LOG4CXX_WARN(logger, "Cannot get BSR. CurrencyServiceV2::getBSR(). Will show 0.");
  }

  return tax::type::BSRValue(0);
}

tax::type::MoneyAmount
CurrencyServiceV2::convert(const tse::MoneyAmount amount,
                           const tse::CurrencyCode& baseFareCurrency,
                           const tse::CurrencyCode& calculationCurrency,
                           const tse::CurrencyCode& paymentCurrency,
                           ConversionType applType /* = CurrencyConversionRequest::OTHER */,
                           bool useInternationalRounding /* = false */) const
{
  tse::MoneyAmount convertedAmount = amount;

  if (calculationCurrency != baseFareCurrency)
  {
    tse::Money targetMoneyOrigination(baseFareCurrency);
    targetMoneyOrigination.value() = 0;

    const tse::Money sourceMoneyCalculation(amount, calculationCurrency);
    if (UNLIKELY(!_conversionFacade->convert(targetMoneyOrigination,
                                    sourceMoneyCalculation,
                                    _trx,
                                    useInternationalRounding,
                                    applType)))
    {
      throw std::runtime_error("Cannot convert currency. CurrencyConversionFacade() failed.");
    }
    convertedAmount = targetMoneyOrigination.value();
  }

  tse::Money targetMoney(paymentCurrency);
  targetMoney.value() = 0;

  if (baseFareCurrency != paymentCurrency)
  {
    const tse::Money sourceMoney(convertedAmount, baseFareCurrency);
    if (UNLIKELY(!_conversionFacade->convert(targetMoney, sourceMoney, _trx)))
    {
      throw std::runtime_error("Cannot convert currency. CurrencyConversionFacade() failed.");
    }
    convertedAmount = targetMoney.value();
  }

  return tax::doubleToAmount(convertedAmount);
}

tax::type::CurDecimals
CurrencyServiceV2::getCurrencyDecimals(const tax::type::CurrencyCode& currency) const
{
  return static_cast<tax::type::CurDecimals>(Money(currency.asString()).noDec(_ticketDate));
}


tax::type::CurrencyCode
CurrencyServiceV2::getNationCurrency(const tax::type::CurrencyCode& taxCurrency,
                                     const tax::type::Nation& nation) const
{
  if (!taxCurrency.empty())
    return taxCurrency;

  if (nation.empty())
    return tax::type::CurrencyCode();

  tse::NationCode nationCode(nation.rawArray(), nation.length());
  tse::CurrencyCode nationCurrency;
  tse::CurrencyUtil::getNationCurrency(nationCode, nationCurrency, _ticketDate);

  return toTaxCurrencyCode(nationCurrency);
}
} // namespace tse
