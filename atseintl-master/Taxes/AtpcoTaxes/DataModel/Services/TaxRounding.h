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

#include "DataModel/Common/Types.h"

namespace tax
{

class TaxRounding
{
public:
  TaxRounding()
    : _nation(UninitializedCode)
    , _unit(0)
    , _decimals(0)
    , _dir(type::TaxRoundingDefaultDir::Blank)
    {}

  ~TaxRounding() {}

  type::Nation& nation() { return _nation; }

  const type::Nation& nation() const { return _nation; }

  type::MoneyAmount& taxRoundingUnit() { return _unit; }

  type::MoneyAmount taxRoundingUnit() const { return _unit; }

  type::CurDecimals& taxUnitDecimals() { return _decimals; }

  const type::CurDecimals& taxUnitDecimals() const { return _decimals; }

  type::TaxRoundingDefaultDir& taxRoundingDir() { return _dir; }

  type::TaxRoundingDefaultDir taxRoundingDir() const { return _dir; }

private:
  type::Nation _nation;
  type::MoneyAmount _unit;
  type::CurDecimals _decimals;
  type::TaxRoundingDefaultDir _dir;
};
}
