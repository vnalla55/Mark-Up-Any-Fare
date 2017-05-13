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

class CalculationRestrictionTax
{
public:
  type::Nation& nationCode() { return _nationCode; }
  const type::Nation& nationCode() const { return _nationCode; }

  type::TaxCode& taxCode() { return _taxCode; }
  const type::TaxCode& taxCode() const { return _taxCode; }

  type::TaxType& taxType() { return _taxType; }
  const type::TaxType& taxType() const { return _taxType; }

  type::SabreTaxCode& sabreTaxCode() { return _sabreTaxCode; }
  const type::SabreTaxCode& sabreTaxCode() const { return _sabreTaxCode; }

private:
  type::Nation _nationCode;
  type::TaxCode _taxCode;
  type::TaxType _taxType;
  type::SabreTaxCode _sabreTaxCode;
};

} // namespace tax

