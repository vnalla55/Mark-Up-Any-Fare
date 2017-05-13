// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/RoundingUtil.h"

namespace tax
{

type::MoneyAmount
doRound(type::MoneyAmount amount,
        const type::MoneyAmount& unit,
        const type::TaxRoundingDir& dir,
        bool doTruncation)
{
  if (unit == 0)
    return amount;

  if (doTruncation)
  {
    type::MoneyAmount truncUnit = (unit >= 1) ? type::MoneyAmount(1, 10) : unit/10;
    amount = doRound(amount, truncUnit, type::TaxRoundingDir::RoundDown, false);
  }

  type::MoneyAmount scale = unit;
  scale *= (amount >= 0) ? 1 : -1;
  type::MoneyAmount scaled = amount / scale;
  if (dir == type::TaxRoundingDir::RoundUp)
    scaled = ((scaled.numerator() - 1) / scaled.denominator()) + 1;
  else if (dir == type::TaxRoundingDir::RoundDown)
    scaled = scaled.numerator() / scaled.denominator();
  else // dir == type::TaxRoundingDir::Nearest
  {
    int64_t base = scaled.numerator() / scaled.denominator();
    int64_t remainder = scaled.numerator() % scaled.denominator();
    if (2 * remainder >= scaled.denominator())
      scaled = base + 1;
    else
      scaled = base;
  }
  type::MoneyAmount result(scaled * scale);
  return result;
}

} // end of tax namespace


