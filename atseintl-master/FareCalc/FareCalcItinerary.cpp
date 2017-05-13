// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "FareCalc/FareCalcItinerary.h"

#include "Common/FareCalcUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/OBFeesUtils.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag853Collector.h"
#include "Diagnostic/Diag854Collector.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalcController.h"
#include "FareCalc/FareCalculation.h"

#include <iostream>
#include <memory>

namespace tse
{
namespace
{
Logger
logger("atseintl.Taxes.FareCalcItinerary");
}

//---------------------------------------------------------------------------
// class constructor
//---------------------------------------------------------------------------

FareCalcItinerary::FareCalcItinerary() : _trx(nullptr), _itin(nullptr), _fcCollector(nullptr), _fcConfig(nullptr) {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

FareCalcItinerary::~FareCalcItinerary() {}

//---------------------------------------------------------------------------
// class initialize
//---------------------------------------------------------------------------

void
FareCalcItinerary::initialize(PricingTrx& trx,
                              Itin& itin,
                              FareCalcCollector* fcCollector,
                              const FareCalcConfig* fcConfig)
{
  TseCallableTrxTask::trx(&trx);
  desc("FARE CALC ITIN TASK");
  _trx = &trx;
  _itin = &itin;
  _fcCollector = fcCollector;
  _fcConfig = fcConfig;
}

// ----------------------------------------------------------------------------
//
// bool FareCalcItinerary::accumulater
//
// Description:  Main controller for all forms of Tax services Shopping / Pricing
//
// ----------------------------------------------------------------------------

void
FareCalcItinerary::accumulator()
{
  if (_fcCollector == nullptr || !_fcCollector->initialize(*_trx, _itin, _fcConfig))
  {
    LOG4CXX_WARN(logger, "Unable to build the Calculation Totals");
    return;
  }

  FareCalculation* fareCalculation = _fcCollector->createFareCalculation(_trx, _fcConfig);
  if (_trx->isSingleMatch())
    OBFeesUtils::checkLimitOBFees(*_trx, *_fcCollector);
  fareCalculation->process();
}
} //tse
