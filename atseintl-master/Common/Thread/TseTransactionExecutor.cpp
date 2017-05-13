#include "Common/Thread/TseTransactionExecutor.h"

#include "Common/Thread/ThreadPoolInfo.h"
#include "Common/Thread/TseCallableTask.h"
#include "Common/Thread/TseScopedExecutor.h"
#include "Common/Thread/TseThreadPool.h"

namespace tse
{

TseTransactionExecutor::TseTransactionExecutor()
  : TseRunnableExecutor(TseThreadingConst::TRANSACTION_TASK)
  , _threshold(ThreadPoolInfo::getTransactionThreshold())
{
  const ThreadPoolInfo& poolInfo(ThreadPoolInfo::get(TseThreadingConst::THROTTLING_TASK));
  if (poolInfo._size > 0)
  {
    _throttlingExecutor.reset(new TseScopedExecutor(poolInfo._taskId, poolInfo._size));
  }
}

void TseTransactionExecutor::execute(TseCallableTask* runnable)
{
  if (nullptr == _threads)
  {
    TseRunnableExecutor::execute(runnable);
    return;
  }
  bool throttled(false);
  {
    boost::lock_guard<boost::mutex> lock(_mutex);
    if (_threshold != 0 && _count > _threshold)
    {
      throttled = true;
      runnable->setThrottled();
      if (_throttlingExecutor)
      {
        _throttlingExecutor->execute(runnable);
        TseThreadPool::updateTrxConcurrency(_count);
        return;
      }
    }
    ++_count;
  }
  TseRunnableWrapper task(runnable, this);
  _threads->enqueue(task, throttled);
  TseThreadPool::updateTrxConcurrency(_count);
}

}// tse
