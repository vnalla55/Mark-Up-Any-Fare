// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Common/NotTimingOutDetector.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/Thread/TimerTaskExecutor.h"
#include "Common/TrxCounter.h"
#include "DataModel/BaseTrx.h"
#include "Util/FlatMap.h"
#include "Util/Vector.h"

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <time.h>

namespace tse
{
namespace NotTimingOutDetector
{
namespace
{
ConfigurableValue<uint32_t>
intervalCfg("TSE_SERVER", "NOT_TIMING_OUT_DETECTION_INTERVAL");

Logger
loggerNotTimingOut("atseintl.NotTimingOutRequest");

struct NotTimingOut
{
  NotTimingOut() : created(time(nullptr)) {}
  time_t created;
};

class Task : public TimerTask, public TrxCounter::Observer
{
public:
  Task() : TimerTask(TimerTask::REPEATING, std::chrono::seconds(60)), _interval(0) {}
  virtual ~Task() {}

  virtual void trxCreated(BaseTrx& trx) override;
  virtual void trxDeleted(BaseTrx& trx) override;

  virtual void run() override;

  int32_t interval() const { return _interval; }
  void setInterval(int32_t v) { _interval = v; }

protected:
  virtual void onCancel() override;

private:
  typedef FlatMap<BaseTrx*, NotTimingOut> MetaData;
  MetaData _metaData;
  boost::mutex _mutex;

  int32_t _interval;

  bool isTrxNotTimingOut(const NotTimingOut& nto, time_t now);
  void logNotTimingOutTrx(const BaseTrx& trx);
} _detectorTask;
}

void
Task::trxCreated(BaseTrx& trx)
{
  boost::lock_guard<boost::mutex> lock(_mutex);
  _metaData[&trx];
}

void
Task::trxDeleted(BaseTrx& trx)
{
  boost::lock_guard<boost::mutex> lock(_mutex);

  const MetaData::iterator it = _metaData.find(&trx);
  if (it == _metaData.end())
    return;

  if (isTrxNotTimingOut(it->second, time(nullptr)))
    logNotTimingOutTrx(trx);

  _metaData.erase(it);
}

void
Task::run()
{
  boost::lock_guard<boost::mutex> lock(_mutex);

  const time_t currentTime = time(nullptr);
  Vector<BaseTrx*> toRemove;

  for (MetaData::value_type& pair : _metaData)
  {
    BaseTrx& trx = *pair.first;
    NotTimingOut& nto = pair.second;

    if (!isTrxNotTimingOut(nto, currentTime))
      continue;

    toRemove.push_back(&trx);
    logNotTimingOutTrx(trx);
  }

  for (BaseTrx* trx : toRemove)
  {
    _metaData.erase(trx);
  }
}

void
Task::onCancel()
{
  boost::lock_guard<boost::mutex> lock(_mutex);
  _metaData.clear();
}

bool
Task::isTrxNotTimingOut(const NotTimingOut& nto, time_t now)
{
  return ((now - nto.created) > _interval);
}

void
Task::logNotTimingOutTrx(const BaseTrx& trx)
{
  LOG4CXX_INFO(loggerNotTimingOut, trx.rawRequest());
}

void
start()
{
  const int32_t interval = intervalCfg.getValue();

  if (!interval)
    return;

  _detectorTask.setInterval(interval);
  TimerTaskExecutor::instance().scheduleNow(_detectorTask);
  TrxCounter::registerObserver(_detectorTask);
}

void
stop()
{
  TrxCounter::unregisterObserver(_detectorTask);
  TimerTaskExecutor::instance().cancel(_detectorTask);
}
}
}
