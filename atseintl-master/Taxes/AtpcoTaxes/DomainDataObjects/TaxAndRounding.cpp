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

#include "DomainDataObjects/TaxAndRounding.h"
#include "Common/RoundingUtil.h"

namespace tax
{
type::MoneyAmount
TaxAndRounding::getRoundedPercentageTax(const type::MoneyAmount& amount,
                                        const type::MoneyAmount& totalTaxPercentage)
{
  type::MoneyAmount result(0, 1);
  result = amount * _value/(1 + totalTaxPercentage);
  if (!_roundingInfo.standardRounding)
    result = doRound(result, _roundingInfo.unit, _roundingInfo.dir);
  return result;
}

type::MoneyAmount
TaxAndRounding::getRoundedFlatTax()
{
  type::MoneyAmount result = _value;

  if (!_roundingInfo.standardRounding)
    result = doRound(result, _roundingInfo.unit, _roundingInfo.dir);

  return result;
}

type::MoneyAmount
TaxAndRounding::doRound(const type::MoneyAmount& amount,
                        const type::MoneyAmount& unit,
                        const type::TaxRoundingDir& dir) const
{
  if (dir == type::TaxRoundingDir::NoRounding)
    return amount;

  return tax::doRound(amount, unit, dir, _doTruncation);
}
} //end of tax namespace
