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
#include "DataModel/RequestResponse/InputOptionalService.h"
#include "DomainDataObjects/OptionalService.h"
#include "Factories/OptionalServiceFactory.h"

namespace tax
{

OptionalService
OptionalServiceFactory::createFromInput(const InputOptionalService& inputOptionalService)
{
  OptionalService result;
  result.index() = inputOptionalService._id;
  result.amount() = inputOptionalService._amount;
  result.feeAmountInSellingCurrencyPlusTaxes() = inputOptionalService._feeAmountInSellingCurrencyPlusTaxes;
  result.subCode() = inputOptionalService._subCode;
  result.serviceGroup() = inputOptionalService._serviceGroup;
  result.serviceSubGroup() = inputOptionalService._serviceSubGroup;
  result.type() = inputOptionalService._type;
  result.ownerCarrier() = inputOptionalService._ownerCarrier;
  result.pointOfDeliveryLoc() = inputOptionalService._pointOfDeliveryLoc;
  result.outputPtc() = inputOptionalService._outputPtc;
  result.taxInclInd() = inputOptionalService._taxInclInd;
  result.setQuantity(inputOptionalService._quantity);
  return result;
}

} // namespace tax
