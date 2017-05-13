//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class Money;
class PricingTrx;
class ErrorResponseException;
class CurrencyConverter;
class CurrencyCollectionResults;
class NUCCurrencyConverter;
class BSRCurrencyConverter;
class CurrencyConversionCache;

namespace
{
using ConversionType = CurrencyConversionRequest::ApplicationType;
}

class CurrencyConversionFacade
{
public:
  virtual ~CurrencyConversionFacade() = default;

  virtual bool convert(Money& target,
                       const Money& source,
                       const PricingTrx& trx,
                       bool useInternationalRounding = false,
                       ConversionType applType = CurrencyConversionRequest::OTHER,
                       bool reciprocalRate = false,
                       CurrencyCollectionResults* results = nullptr);

  bool convertWithCurrentDate(Money& target, const Money& source, PricingTrx& trx);

  virtual bool convertCalc(Money& target,
                           const Money& source,
                           PricingTrx& trx,
                           bool useInternationalRounding = false,
                           ConversionType applType = CurrencyConversionRequest::OTHER,
                           bool reciprocalRate = false,
                           CurrencyCollectionResults* results = nullptr,
                           CurrencyConversionCache* cache = nullptr,
                           bool roundFare = true);

  bool convert(Money& target,
               const Money& localCurrency,
               const PricingTrx& trx,
               const CurrencyCode& calculationCurrency,
               MoneyAmount& convertedAmount,
               bool useInternationalRounding = false,
               ConversionType applType = CurrencyConversionRequest::OTHER,
               bool reciprocalRate = false);

  bool round(Money& target,
             PricingTrx& trx,
             bool useInternationalRounding = false);

  bool roundCB(Money& target,
               PricingTrx& trx,
               bool useInternationalRounding);

  bool roundFare() const { return _roundFare; }
  void setRoundFare(bool roundFare) { _roundFare = roundFare; }

  bool useSecondRoeDate() const { return _useSecondRoeDate; }
  void setUseSecondRoeDate(bool useSecondRoeDate) { _useSecondRoeDate = useSecondRoeDate; }

  const DateTime& defineConversionDate(const PricingTrx& trx) const;

  virtual NUCCurrencyConverter& nucConverter() { return _nucConverter; }
  virtual const NUCCurrencyConverter& nucConverter() const { return _nucConverter; }

private:
  NUCCurrencyConverter _nucConverter;
  BSRCurrencyConverter _bsrConverter;

  bool _roundFare = true;
  bool _useSecondRoeDate = false;
};
} // tse
