//
// Copyright Sabre 2012
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#pragma once

#include "Common/BSRCollectionResults.h"
#include "Common/Money.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class PricingTrx;
class FarePath;
class CalcTotals;
class FareCalcConfig;

namespace FareCalc
{

class FareAmountsConverter
{
  friend class FareAmountsConverterTest;

public:
  FareAmountsConverter(PricingTrx*,
                       const FarePath*,
                       const FareCalcConfig*,
                       CalcTotals*,
                       CurrencyCode& lastBaseFareCurrencyCode,
                       CurrencyNoDec& lastConvertedBaseFareNoDec);

  void convertAmounts();

  Money convertConstructionToEquivalent(MoneyAmount amt) const;

  Money convertConstructionToBaseFare(const Money& src, bool round) const;
  Money convertBaseFareToEquivalent(const Money& src, bool round = false) const;

  Money roundUp(Money money) const;

private:
  std::pair<Money, BSRCollectionResults>
  convertBaseFareToEquivalentImpl(const Money& src, bool round) const;

  void processConvertedBaseFare();

  void convertBaseFare() const;
  void convertBaseFareImpl(const MoneyAmount totalNUCAmount,
                           const CurrencyCode& calculationCurrency,
                           CurrencyNoDec& calcCurrencyNoDec,
                           MoneyAmount& convertedBaseFare,
                           CurrencyNoDec& convertedBaseFareNoDec,
                           CurrencyCode& convertedBaseFareCurrencyCode,
                           ExchRate& roeRate,
                           CurrencyNoDec& roeRateNoDec,
                           DateTime& effectiveDate,
                           DateTime& discontinueDate,
                           bool round) const;

  void processConsolidatorPlusUp();
  void processEquivalentFare();
  void processFclNoDec();
  void convertNetChargeAmounts();
  const DateTime& defineConversionDate(PricingTrx& trx, const FarePath& fp) const;

  PricingTrx* _trx;
  const FarePath* _farePath;
  const FareCalcConfig* _fcConfig;
  CalcTotals* _calcTotals;

  CurrencyCode& _lastBaseFareCurrencyCode;
  CurrencyNoDec& _lastConvertedBaseFareNoDec;
};

} // namespace FareCalc
} // namespace tse

