//----------------------------------------------------------------------------
//
//  Description: Common Metrics Latency class for ATSE shopping/pricing.
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/MetricsUtil.h"

#include <functional>

namespace tse
{

class Trx;

class TSELatencyData
{
public:
  using Callback = std::function<void(double wallTime, double userTime, double sysTime)>;

  TSELatencyData(Trx& trx,
                 const char* description,
                 Callback callback,
                 bool isTaskToplevelData = false);
  TSELatencyData(Trx& trx, const char* description, bool isTaskToplevelData = false);
  TSELatencyData(Trx& trx, MetricsUtil::MetricsFactor factor, bool isTaskToplevelData = false);

  virtual ~TSELatencyData();

protected:
  void collect();

  Trx& _trx;
  const char* _description = nullptr;
  std::size_t _items = 0;
  std::size_t _startMem = 0;
  bool _isTaskToplevelData = false;
  long _userTime = 0;
  long _systemTime = 0;

  MetricsTimer _timer;
  Callback _callback;
};

} // end tse namespace

