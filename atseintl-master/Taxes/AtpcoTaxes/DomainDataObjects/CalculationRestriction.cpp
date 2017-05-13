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
#include "Common/TaxName.h"
#include "DomainDataObjects/CalculationRestriction.h"
#include "Factories/MakeSabreCode.h"

#include <set>

namespace tax
{

CalculationRestriction::CalculationRestriction() : _restrictionType(type::CalcRestriction::Blank) {}

bool
CalculationRestriction::taxNameMatchesRestriction(const TaxName& taxName, const CalculationRestrictionTax& restriction) const
{
  return (taxName.nation() == restriction.nationCode() &&
          (restriction.taxCode().empty() || taxName.taxCode() == restriction.taxCode()) &&
          (restriction.taxType().empty() || taxName.taxType() == restriction.taxType()));
}

bool
CalculationRestriction::isAllowed(const TaxName& taxName) const
{
  if (_restrictionType != type::CalcRestriction::CalculateOnlySpecifiedTaxes)
    return true;

  if (_calculationRestrictionTax.empty())
    return false;

  for (const CalculationRestrictionTax& restriction : _calculationRestrictionTax)
  {
    if (taxNameMatchesRestriction(taxName, restriction))
      return true;
  }
  return false;
}

bool
CalculationRestriction::isExempted(const type::SabreTaxCode& taxCode) const
{
  static const std::set<std::string> feeSet{"AY", "XA", "XY", "YC", "ZZ"};

  if (_restrictionType == type::CalcRestriction::ExemptAllTaxesAndFees)
    return true;

  if (_restrictionType == type::CalcRestriction::ExemptAllTaxes)
  {
    if (feeSet.count(taxCode.substr(0,2)) == 1)
      return false;

    return true;
  }

  if (_restrictionType == type::CalcRestriction::ExemptSpecifiedTaxes)
  {
    // This will not work when data is fed from TaxRq XML request, because in that case we
    // do not populate or compute the value of sabreTaxCode(); not a big problem, TaxRq's
    // should only come from Bulk Charger, which shouldn't use this feature
    for (const CalculationRestrictionTax& restriction : _calculationRestrictionTax)
    {
      if (restriction.sabreTaxCode().length() > taxCode.length())
        return false;

      if (restriction.sabreTaxCode().length() == 2 && restriction.sabreTaxCode() == taxCode.substr(0, 2))
        return true; // the wildcard case: "US" matches to "US1", "US2", etc.

      if (restriction.sabreTaxCode() == taxCode)
        return true;
    }

    return false;
  }

  return false;
}

} // namespace tax
