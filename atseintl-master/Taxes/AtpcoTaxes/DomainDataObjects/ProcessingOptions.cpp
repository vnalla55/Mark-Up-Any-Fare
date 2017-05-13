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
#include "DomainDataObjects/ProcessingOptions.h"
#include "Util/BranchPrediction.h"

namespace tax
{

bool
ProcessingOptions::isAllowed(const TaxName& taxName) const
{
  if (LIKELY(_calculationRestrictions.empty()))
    return true;

  for (CalculationRestriction restriction : _calculationRestrictions)
  {
    if (!restriction.isAllowed(taxName))
      return false;
  }

  return true;
}

type::CalcRestriction
ProcessingOptions::isExempted(const type::SabreTaxCode& taxCode) const
{
  for (CalculationRestriction restriction : _calculationRestrictions)
  {
    if (restriction.isExempted(taxCode))
      return restriction.restrictionType();
  }

  return type::CalcRestriction::Blank;
}

std::ostream&
ProcessingOptions::print(std::ostream& out, int /*indentLevel  = 0 */, char /*indentChar = ' ' */)
    const
{
  return out;
}
} // namespace tax
