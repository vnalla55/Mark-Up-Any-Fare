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
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class CurrencyCollectionResults;
class CurrencyConversionRequest;
class ErrorResponseException;
class FareDisplayTrx;
class Money;
class PricingTrx;

class CurrencyRoundingUtil
{
public:
  enum ApplicationType
  { TAXES,
    FARES,
    SURCHARGES,
    OTHER };

  CurrencyRoundingUtil();

  bool round(tse::MoneyAmount& roundedAmount, const CurrencyCode& currency, PricingTrx& trx);

  bool round(tse::Money& target, PricingTrx& trx);

  bool round(tse::Money& target, PricingTrx& trx, bool useInternationalRounding);

  bool isValid(tse::Money& amount);

  static bool applyNonIATARounding(const PricingTrx& trx,
                                   const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const RuleNumber& ruleNumber);

  MoneyAmount roundMoneyAmount(const MoneyAmount amount,
                               const CurrencyCode& displayCurrency,
                               const CurrencyCode& paxTypeFareCurrency,
                               FareDisplayTrx& trx);

private:
  CurrencyConverter _ccConverter;
};
}

