//-------------------------------------------------------------------
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <map>

namespace tse
{
class PaxTypeFare;

struct PenaltyFee
{
  PenaltyFee()
    : penaltyAmount(0),
      penaltyCurrency(""),
      penaltyFromPercent(0),
      penaltyFromPercentCurrency(""),
      penaltyAmt1InEquivCurrency(0),
      penaltyAmt2InEquivCurrency(0),
      penaltyFromPercentInEquivCurrency(0),
      penaltyAmountInEquivCurrency(0),
      applicableDiscount(0)
  {
  }

  MoneyAmount penaltyAmount;
  CurrencyCode penaltyCurrency;
  MoneyAmount penaltyFromPercent;
  CurrencyCode penaltyFromPercentCurrency;
  MoneyAmount penaltyAmt1InEquivCurrency;
  MoneyAmount penaltyAmt2InEquivCurrency;
  MoneyAmount penaltyFromPercentInEquivCurrency;
  MoneyAmount penaltyAmountInEquivCurrency;
  MoneyAmount applicableDiscount;
};

struct ReissueCharges
{
  ReissueCharges()
    : changeFee(0),
      changeFeeCurrency(""),
      changeFeeInEquivCurrency(0),
      changeFeeInCalculationCurrency(0),
      minAmtApplied(false)
  {
  }

  MoneyAmount changeFee;
  CurrencyCode changeFeeCurrency;
  MoneyAmount changeFeeInEquivCurrency;
  MoneyAmount changeFeeInCalculationCurrency;
  bool minAmtApplied;
  std::map<const PaxTypeFare*, PenaltyFee*> penaltyFees;
};
} // tse namespace

