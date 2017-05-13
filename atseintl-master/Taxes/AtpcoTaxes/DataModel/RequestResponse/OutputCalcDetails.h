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
#pragma once
#include <boost/optional.hpp>
#include "DataModel/Common/Types.h"

namespace tax
{

struct OutputCalcDetails
{
  boost::optional<type::MoneyAmount> roundingUnit;
  boost::optional<type::TaxRoundingDir> roundingDir;
  boost::optional<type::BSRValue> taxCurToPaymentCurBSR;

  type::MoneyAmount exchangeRate1 = 0;
  type::CurDecimals exchangeRate1NoDec = 0;

  type::CurrencyCode intermediateCurrency{UninitializedCode};
  type::CurDecimals intermediateNoDec = 0;
  type::MoneyAmount intermediateUnroundedAmount = 0;
  type::MoneyAmount intermediateAmount = 0;

  type::MoneyAmount exchangeRate2 = 0;
  type::CurDecimals exchangeRate2NoDec = 0;
};

inline bool operator==(const OutputCalcDetails& lhs, const OutputCalcDetails& rhs)
{
  return lhs.roundingUnit == rhs.roundingUnit &&
         lhs.roundingDir == rhs.roundingDir &&
         lhs.taxCurToPaymentCurBSR == rhs.taxCurToPaymentCurBSR &&
         lhs.exchangeRate1 == rhs.exchangeRate1 &&
         lhs.exchangeRate1NoDec == rhs.exchangeRate1NoDec &&
         lhs.intermediateCurrency == rhs.intermediateCurrency &&
         lhs.intermediateNoDec == rhs.intermediateNoDec &&
         lhs.intermediateUnroundedAmount == rhs.intermediateUnroundedAmount &&
         lhs.intermediateAmount == rhs.intermediateAmount &&
         lhs.exchangeRate2 == rhs.exchangeRate2 &&
         lhs.exchangeRate2NoDec == rhs.exchangeRate2NoDec;
}

}

