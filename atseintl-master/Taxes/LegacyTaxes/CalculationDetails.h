// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2012
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#ifndef CALCULATIONDETAILS_H
#define CALCULATIONDETAILS_H

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/BSRCollectionResults.h"
#include <vector>
#include <algorithm>

namespace tse

{
class TaxItem;
struct CalculationDetails
{
public:
  CalculationDetails()
    : baseFareSumAmount(0.0),
      fareInCalculationCurrencyAmount(0.0),
      fareInBaseFareCurrencyAmount(0.0),
      fareInPaymentCurrencyAmount(0.0),
      isInternationalRounded(false),
      internationalRoundedAmount(0.0),
      isTaxOnTax(false),
      taxableTaxSumAmount(0.0),
      taxToAdjustAmount(0.0),
      earlyPlusUpAmount(0.0),
      plusedUpAmount(0.0),
      bookletAdjustedAmount(0.0),
      minTaxAdjustedAmount(0.0),
      adjustedTaxAmount(0.0),
      taxRangeAmount(0.0),
      roundingUnit(0.0),
      roundingNoDec(0),
      roundingRule(RoundingRule::EMPTY),
      isSpecialRounded(false) {};

  //Many of those members are not used anymore
  //Probably to remove/refactore
  MoneyAmount baseFareSumAmount;
  MoneyAmount fareInCalculationCurrencyAmount;
  MoneyAmount fareInBaseFareCurrencyAmount;
  MoneyAmount fareInPaymentCurrencyAmount;
  bool isInternationalRounded;
  MoneyAmount internationalRoundedAmount;
  CurrencyCode calculationCurrency;
  CurrencyCode baseFareCurrency;
  BSRCollectionResults calculationToBaseFareResults;
  BSRCollectionResults baseFareToPaymentResults;

  std::vector<std::pair<TaxCode, MoneyAmount> > taxableTaxes;
  std::vector<TaxItem*> taxableTaxItems;
  bool isTaxOnTax;
  MoneyAmount taxableTaxSumAmount;
  MoneyAmount taxToAdjustAmount;
  MoneyAmount earlyPlusUpAmount;

  MoneyAmount plusedUpAmount;
  MoneyAmount bookletAdjustedAmount;
  MoneyAmount minTaxAdjustedAmount;
  MoneyAmount adjustedTaxAmount;

  MoneyAmount taxRangeAmount;

  RoundingFactor roundingUnit;
  CurrencyNoDec roundingNoDec;
  RoundingRule roundingRule;
  bool isSpecialRounded;
};
}
#endif // CALCULATIONDETAILS_H
