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
#include "DataModel/RequestResponse/InputCalculationRestrictionTax.h"
#include "DomainDataObjects/CalculationRestrictionTax.h"
#include "Factories/CalculationRestrictionTaxFactory.h"

namespace tax
{

CalculationRestrictionTax
CalculationRestrictionTaxFactory::createFromInput(
    const InputCalculationRestrictionTax& inputCalculationRestrictionTax)
{
  CalculationRestrictionTax result;
  result.nationCode() = inputCalculationRestrictionTax.nationCode();
  result.taxCode() = inputCalculationRestrictionTax.taxCode();
  result.taxType() = inputCalculationRestrictionTax.taxType();
  result.sabreTaxCode() = inputCalculationRestrictionTax.sabreTaxCode();
  return result;
}

} // namespace tax
