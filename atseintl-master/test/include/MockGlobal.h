#ifndef MOCK_GLOBAL_H
#define MOCK_GLOBAL_H

#include "Common/Global.h"

namespace tse
{
class MockGlobal : public Global
{
public:
  static void setMetricsMan(tse::MetricsMan* metricsMan) { _metricsMan = metricsMan; }

  static void clear()
  {
    setMetricsMan(0);
    _configMan = 0;
  }

  static void setAllowHistorical(bool allowHistorical) { _allowHistorical = allowHistorical; }

  static void setStartTime() { _startTime = time(NULL); }

protected:
  MockGlobal(const MockGlobal& rhs);
  MockGlobal& operator=(const MockGlobal& rhs);
};

} // namespace tse

#endif
