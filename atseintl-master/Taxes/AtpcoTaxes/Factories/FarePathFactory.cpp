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
#include "DataModel/RequestResponse/InputFarePath.h"
#include "DomainDataObjects/FarePath.h"
#include "Factories/FactoryUtils.h"
#include "Factories/FareUsageFactory.h"
#include "Factories/FarePathFactory.h"

namespace tax
{

FarePath
FarePathFactory::createFromInput(const InputFarePath& inputFarePath)
{
  FarePath result;
  result.totalAmount() = inputFarePath._totalAmount;
  result.totalMarkupAmount() = inputFarePath._totalMarkupAmount;
  result.totalAmountBeforeDiscount() = inputFarePath._totalAmountBeforeDiscount != 0
                                        ? inputFarePath._totalAmountBeforeDiscount
                                        : inputFarePath._totalAmount;
  result.validatingCarrier() = inputFarePath._validatingCarrier;
  result.outputPtc() = inputFarePath._outputPtc;

  create<FareUsageFactory>(inputFarePath._fareUsages, result.fareUsages());
  return result;
}

} // namespace tax
