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
#include "DataModel/RequestResponse/InputFare.h"
#include "DomainDataObjects/Fare.h"
#include "Factories/FareFactory.h"

namespace tax
{

Fare
FareFactory::createFromInput(const InputFare& inputFare)
{
  Fare result;
  result.basis() = inputFare._basis;
  result.type() = inputFare._type;
  result.oneWayRoundTrip() = inputFare._oneWayRoundTrip;
  result.directionality() = inputFare._directionality;
  result.amount() = inputFare._amount;
  result.markupAmount() = inputFare._markupAmount;
  result.sellAmount() = inputFare._sellAmount;
  result.isNetRemitAvailable() = inputFare._isNetRemitAvailable;
  result.rule() = inputFare._rule;
  result.tariff() = Tariff{inputFare._tariff, inputFare._tariffInd};
  result.outputPtc() = inputFare._outputPtc;
  return result;
}

} // namespace tax
