//-------------------------------------------------------------------
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

#ifndef SCHEUDLE_COUNT_TASK_H
#define SCHEDULE_COUNT_TASK_H

#include "Common/Thread/TseCallableTrxTask.h"
#include "DSS/FlightCountMgr.h"

#include <vector>

namespace tse
{

class FareDisplayTrx;
class FlightCount;

class ScheduleCountTask : public TseCallableTrxTask
{

public:
  ScheduleCountTask(FareDisplayTrx& trx, FlightCountMgr& mgr, std::vector<FlightCount*>& fltCounts)
    : _fdtrx(trx), _mgr(mgr), _fltCounts(fltCounts)
  {
    desc("SCHEDULE TASK");
  }

  virtual ~ScheduleCountTask() {}

  void performTask() override { _mgr.getFlightCount(_fdtrx, _fltCounts); }

  FareDisplayTrx& _fdtrx;
  FlightCountMgr& _mgr;
  std::vector<FlightCount*>& _fltCounts;
};
} // tse namespace

#endif

