//-------------------------------------------------------------------
//
//  File:        AltFareCalcController.cpp
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

#include "FareCalc/AltFareCalcController.h"

#include "FareCalc/AltFareCalcCollector.h"

namespace tse
{
FareCalcCollector*
AltFareCalcController::createFareCalcCollector()
{
  AltFareCalcCollector* fcCollector = nullptr;
  _trx.dataHandle().get(fcCollector);

  return fcCollector;
}
} // tse namespace
