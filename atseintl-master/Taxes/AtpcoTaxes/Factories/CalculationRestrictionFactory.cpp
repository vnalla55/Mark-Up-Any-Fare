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
#include "DataModel/RequestResponse/InputCalculationRestriction.h"
#include "DomainDataObjects/CalculationRestriction.h"
#include "Factories/CalculationRestrictionTaxFactory.h"
#include "Factories/CalculationRestrictionFactory.h"
#include "Factories/FactoryUtils.h"

namespace tax
{

CalculationRestriction
CalculationRestrictionFactory::createFromInput(
    const InputCalculationRestriction& inputCalculationRestriction)
{
  CalculationRestriction result;
  result.restrictionType() = inputCalculationRestriction.restrictionType();

  create<CalculationRestrictionTaxFactory>(inputCalculationRestriction.calculationRestrictionTax(),
                                           result.calculationRestrictionTax());
  return result;
}

} // namespace tax
