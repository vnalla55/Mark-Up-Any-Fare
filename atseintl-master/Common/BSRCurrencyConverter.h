//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/CurrencyConverter.h"


namespace tse
{
class Money;
class PricingTrx;

class BSRCurrencyConverter : tse::CurrencyConverter
{
public:
  static constexpr int BSR_RATE_NOT_FOUND = 0;
  static constexpr int CONVERSION_SUCCEEDED = 1;
  static constexpr int CONVERSION_FAILED = -1;

  enum ConversionRule
  { DIVIDE,
    MULTIPLY };
  enum ConversionStep
  { FIRST_STEP,
    SECOND_STEP };

  friend class BSRCurrencyConverterTest;

  BSRCurrencyConverter();

  virtual bool
  convert(CurrencyConversionRequest& request, CurrencyCollectionResults* results) override;

  bool hasIndirectEquivAmtOverride(PricingTrx& trx);

protected:

  int applyDirectConversion(tse::Money& target,
                            const tse::Money& source,
                            const tse::CurrencyCode& bsrPrimeCurrency,
                            const tse::CurrencyCode& bsrCurrency,
                            ConversionRule conversionRule,
                            CurrencyConversionRequest& request,
                            ExchRate& rate,
                            CurrencyNoDec& rateNoDec,
                            Indicator& rateType,
                            RoundingRule& rule,
                            RoundingFactor& factor,
                            MoneyAmount& unRoundedAmount,
                            CurrencyNoDec& roundingFactorNoDec,
                            bool rateFound = false,
                            double overRideRate = 0);

  bool bsrRate(const tse::CurrencyCode& bsrPrimeCurrency,
               const tse::CurrencyCode& bsrCurrency,
               const tse::DateTime& ticketDate,
               tse::ExchRate& rate,
               CurrencyNoDec& rateNoDec,
               Indicator& rateType);

  bool applyExchRate(tse::Money& target,
                     const tse::Money& source,
                     ExchRate& exchangeRate,
                     ConversionRule conversionRule);

  bool determinePricingCurrency(CurrencyConversionRequest& conversionRequest,
                                tse::CurrencyCode& salesCurrencyCode,
                                tse::CurrencyCode& conversionCurrency,
                                bool& hasOverride);

  const tse::NationCode& getPointOfSaleNation(CurrencyConversionRequest& conversionRequest);
  const tse::NationCode& getPointOfSaleNation(const PricingTrx& trx);

  bool applyRounding(const tse::CurrencyCode& salesCurrencyCode,
                     tse::Money& target,
                     const tse::DateTime& ticketDate,
                     RoundingRule& rule,
                     RoundingFactor& factor,
                     CurrencyNoDec& roundingFactorNoDec);

public:
  bool applyRoundingRule(tse::Money& target, const tse::DateTime& ticketDate, RoundingRule rule);
};
}

