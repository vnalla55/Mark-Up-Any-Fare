// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "DataModel/Common/Types.h"

#include <boost/optional.hpp>

namespace tax
{

struct CalcDetails
{
  type::MoneyAmount taxBeforeRounding{0};
  type::MoneyAmount taxWithMarkupBeforeRounding{0};

  type::MoneyAmount roundingUnit{0};
  type::TaxRoundingDir roundingDir{type::TaxRoundingDir::Blank};
  type::MoneyAmount currencyUnit{0};

  type::MoneyAmount exchangeRate1{0};
  type::CurDecimals exchangeRate1NoDec{0};

  type::CurrencyCode intermediateCurrency{UninitializedCode};
  type::CurDecimals intermediateNoDec{0};
  type::MoneyAmount intermediateUnroundedAmount{0};
  type::MoneyAmount intermediateAmount{0};

  type::MoneyAmount exchangeRate2{0};
  type::CurDecimals exchangeRate2NoDec{0};
};

} // namespace tax
