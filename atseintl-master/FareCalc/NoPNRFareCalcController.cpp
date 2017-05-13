//-------------------------------------------------------------------
//
//  File:        NoPNRFareCalcController.cpp
//  Created:
//  Authors:
//
//  Description:
//
//
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "FareCalc/NoPNRFareCalcController.h"

#include "FareCalc/NoPNRFareCalcCollector.h"

namespace tse
{
FareCalcCollector*
NoPNRFareCalcController::createFareCalcCollector()
{
  NoPNRFareCalcCollector* fcCollector = nullptr;
  _trx.dataHandle().get(fcCollector);

  return fcCollector;
}
} // tse namespace
