#pragma once

#include "Common/Thread/TseRunnableExecutor.h"

namespace tse
{

class PricingUnitRequesterData
{
  friend class PricingUnitRequester;

  enum Status
  {
    S_READY,
    S_RUNNING
  };

public:
  ~PricingUnitRequesterData() { wait(); }

  PricingUnitRequesterData() : _status(S_READY) {}

private:
  void wait()
  {
    if (_status == S_RUNNING)
    {
      _executor.wait(false);
      _status = S_READY;
    }
  }

  Status status() const { return _status; }
  void setStatus(Status status) { _status = status; }

  TseRunnableExecutor& executor() { return _executor; }

private:
  PricingUnitRequesterData(const PricingUnitRequesterData&) = delete;
  void operator=(const PricingUnitRequesterData&) = delete;

private:
  Status _status;
  TseRunnableExecutor _executor{TseThreadingConst::PRICINGUNITPQ_TASK};
};

} // namespace tse

