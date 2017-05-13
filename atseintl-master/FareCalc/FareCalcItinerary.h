// -------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
#pragma once

#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TSELatencyData.h"

namespace tse
{
class FareCalcCollector;
class FareCalcConfig;
class PricingTrx;
class Itin;

class FareCalcItinerary : public TseCallableTrxTask
{

public:
  FareCalcItinerary();

  virtual ~FareCalcItinerary();

  void initialize(PricingTrx& trx,
                  Itin& itin,
                  FareCalcCollector* fcCollector,
                  const FareCalcConfig* fcConfig);

  virtual void accumulator();

  void performTask() override { accumulator(); }

private:
  PricingTrx* _trx;
  Itin* _itin;
  FareCalcCollector* _fcCollector;
  const FareCalcConfig* _fcConfig;
};
}
