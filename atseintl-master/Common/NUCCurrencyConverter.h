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
class CurrencyCollectionResults;
class CurrencyConversionCache;
class CurrencyConversionRequest;
class DataHandle;
class FarePath;
class FareUsage;
class Itin;
class Logger;
class PricingTrx;

class NUCCurrencyConverter : CurrencyConverter
{
  friend class NUCCurrencyConverterConvertTest;
  friend class NUCCurrencyConverterTest;

public:
  NUCCurrencyConverter();
  virtual ~NUCCurrencyConverter() {}

  virtual bool convert(CurrencyConversionRequest& request,
                       CurrencyCollectionResults* results,
                       CurrencyConversionCache* cache = nullptr);

  bool convertBaseFare(PricingTrx& trx,
                       const FarePath& farePath,
                       MoneyAmount nucAmount,
                       MoneyAmount& convertedAmount,
                       CurrencyNoDec& convertedAmtNoDec,
                       CurrencyCode& convertedCurrencyCode,
                       ExchRate& roeRate,
                       CurrencyNoDec& roeRateNoDec,
                       DateTime& nucEffectiveDate,
                       DateTime& nucDiscontinueDate,
                       bool useInternationalRounding = true,
                       bool roundBaseFare = true);

  bool roundFare(Money& target,
                 RoundingRule& roundingRule,
                 RoundingFactor& roundingFactor,
                 const DateTime& ticketingDate,
                 DataHandle& dataHandle,
                 bool useInternationalRounding = false);

  const static double USE_INTERNATIONAL_ROUNDING;
  const static double NO_ROUNDING;

  void checkDomesticRounding(PricingTrx& trx,
                             const Itin* itin,
                             const CurrencyCode& currencyCode,
                             bool& useInternationalRounding);

  const DateTime& defineConversionDate(PricingTrx& trx, const FarePath& fp);

protected:

  bool hasArunkSegments(const FareUsage* fareUsage, LocCode& originLoc);
  CurrencyCode checkBaseFareCurrency(const Itin* itin, const FarePath& farePath);

  static const std::string MISSING_NUC_RATE_FOR;

private:
  static Logger _logger;
};

} // tse

