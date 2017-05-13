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

#include "DataModel/Common/SafeEnums.h"
#include "DomainDataObjects/CalculationRestrictionTax.h"

#include <vector>

namespace tax
{

class TaxName;

class CalculationRestriction
{
public:
  CalculationRestriction();

  type::CalcRestriction& restrictionType() { return _restrictionType; }

  const type::CalcRestriction& restrictionType() const { return _restrictionType; }

  std::vector<CalculationRestrictionTax>& calculationRestrictionTax()
  {
    return _calculationRestrictionTax;
  };

  const std::vector<CalculationRestrictionTax>& calculationRestrictionTax() const
  {
    return _calculationRestrictionTax;
  };

  bool isAllowed(const TaxName& taxName) const;
  bool isExempted(const type::SabreTaxCode& taxCode) const;

private:
  bool taxNameMatchesRestriction(const TaxName& taxName, const CalculationRestrictionTax& restriction) const;

  std::vector<CalculationRestrictionTax> _calculationRestrictionTax;
  type::CalcRestriction _restrictionType;
};

} // namespace tax
