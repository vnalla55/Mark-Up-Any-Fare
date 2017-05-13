//----------------------------------------------------------------
//
//  File:		FareCalcController
//  Authors:
//
//  Copyright Sabre 2004
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

#pragma once

#include "DataModel/AltPricingTrx.h"
#include "FareCalc/FareCalcController.h"

namespace tse
{
class FareCalcCollector;

class AltFareCalcController : public FareCalcController
{
public:
  AltFareCalcController(AltPricingTrx& trx) : FareCalcController(trx) {}

private:
  virtual FareCalcCollector* createFareCalcCollector() override;
};

} // tse namespace

