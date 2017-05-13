#include "Common/Thread/TseThreadPool.h"

#include "Common/Logger.h"
#include "Common/Thread/ThreadPoolInfo.h"
#include "Common/Thread/TseCallableTask.h"
#include "Common/Thread/TseRunnableExecutor.h"

namespace tse
{

namespace
{

Logger logger("atseintl.Common.TseThreadPool");

boost::atomic<unsigned> numberThreads(0); 
const unsigned numberThreadsMax(400);
boost::atomic<unsigned> activeTrxTasks(0);
boost::atomic<unsigned> concurrencySamples(0);
boost::atomic<double> concurrency(0);
boost::atomic<unsigned> boosterEffSize(0);

double getTrxConcurrency()
{
  unsigned tasks(activeTrxTasks);
  unsigned samples(concurrencySamples);
  if (0 == samples)
  {
    return concurrency;
  }
  double result(static_cast<double>(tasks) / samples);
  concurrency = result;
  concurrencySamples = 0;
  activeTrxTasks = 0;
  return result;
}

unsigned
getEffectiveMaxSize(unsigned maxSize)
{
  static std::vector<double> sizes;
  static const bool initSizes(ThreadPoolInfo::getBoosterSizes(sizes));
  SUPPRESS_UNUSED_WARNING(initSizes);
  LOG4CXX_DEBUG(logger, "initSizes=" << initSizes);
  static const unsigned minBoosterSize(ThreadPoolInfo::getBoosterMinSize());
  double concurrency(getTrxConcurrency());
  double calculatedEffSize(-1.);
  unsigned n(static_cast<unsigned>(concurrency + .5) / 10);
  if (n + 1 < sizes.size())
  {
    calculatedEffSize = sizes[n] + (sizes[n + 1] - sizes[n]) * (concurrency - 10 * n) / 10;
  }
  if (calculatedEffSize >= 0)
  {
    unsigned effMaxSize(static_cast<unsigned>(calculatedEffSize + .5));
    effMaxSize = std::min(effMaxSize, maxSize);
    effMaxSize = std::max(effMaxSize, minBoosterSize);
    LOG4CXX_INFO(logger, "concurrency=" << concurrency << ",effMaxSize=" << effMaxSize);
    return effMaxSize;
  }
  unsigned effMaxSize(maxSize);
  static double slope(ThreadPoolInfo::getBoosterSizeSlope());
  effMaxSize = std::min(static_cast<unsigned>(slope * concurrency / 10 + .5), maxSize);
  effMaxSize = std::max(effMaxSize, minBoosterSize);
  LOG4CXX_INFO(logger, "concurrency=" << concurrency << ",effMaxSize=" << effMaxSize);
  return effMaxSize;
}

boost::mutex registrationMutex;

}// namespace

std::shared_ptr<BoosterThreadPool> BoosterThreadPool::_instance;
TseThreadPool::PoolMap TseThreadPool::_poolMap;

TseThreadPool::Thread::Thread(TseThreadPool& pool,
                              unsigned id)
  : _pool(pool)
  , _thread(&Thread::run, this)
  , _id(id)
{
  ++numberThreads;
  if (numberThreads > numberThreadsMax)
  {
    LOG4CXX_WARN(logger, __FUNCTION__ << "numberThreads=" << numberThreads);
  }
}

TseThreadPool::Thread::~Thread()
{
  try
  {
    if (_thread.joinable())
    {
      _thread.join();
    }
  }
  catch (...)
  {
  }
  --numberThreads;
}

void TseThreadPool::Thread::run()
{
  for (;;)
  {
    TseRunnableWrapper task(_pool.getNext(_id));
    if (task.empty())
    {
      break;
    }
    task.run();
    --_pool._numberActiveThreads;
  }
  --_pool._numberActiveThreads;
  _pool.killThread(_id);
}

TseThreadPool::TseThreadPool(TseThreadingConst::TaskId taskId,
                             size_t maxSize)
  : _taskId(taskId)
  , _maxSize(maxSize)
  , _booster(maxSize > 0 ? ThreadPoolFactory::getBoosterPool(taskId) : 0)
  , _boosterThreshold(_booster ? ThreadPoolInfo::getBoosterThreshold(taskId) : 0)
  , _shouldConsumeBacklog(ThreadPoolInfo::getBoosterConsumeBacklog(taskId))
  , _shouldConsumeBacklog2(ThreadPoolInfo::getBoosterConsumeBacklog2(taskId))
  , _shouldCombineTasks(ThreadPoolInfo::getBoosterCombineTasks(taskId))
  , _joining(false)
  , _numberActiveThreads(0)
  , _numberCombinedTasks(0)
{
  if (_booster)
  {
    _booster->registerPool(this);
  }
}

TseThreadPool::~TseThreadPool()
{
  if (_booster)
  {
    _booster->unregisterPool(this);
  }
  {
    boost::lock_guard<boost::mutex> lock(_mutex);
    _joining = true;
  }
  _condition.notify_all();
}

void TseThreadPool::enqueue(const TseRunnableWrapper& task,
                            bool front)
{
  bool notifyPool(false);
  {
    boost::lock_guard<boost::mutex> lock(_mutex);
    if ((0 == _maxSize || _pool.size() < _maxSize)
        && _pool.size() <= _numberActiveThreads + _tasks.size())
    {
      std::shared_ptr<Thread> thread(new Thread(*this));
      _pool.push_back(thread);
    }
    if (front)
    {
      _tasks.push_front(task);
    }
    else
    {
      _tasks.push_back(task);
    }
    if (_booster
        && _numberActiveThreads + _tasks.size() >= _pool.size() + _boosterThreshold)
    {
      _booster->runImmediately(*this);
    }
    notifyPool = !_tasks.empty();
  }
  if (notifyPool)
  {
    _condition.notify_one();
  }
}

size_t TseThreadPool::size() const
{
  boost::lock_guard<boost::mutex> lock(_mutex);
  return _pool.size();
}

size_t TseThreadPool::getQueueSize() const
{
  boost::lock_guard<boost::mutex> lock(_mutex);
  return _tasks.size();
}

TseRunnableWrapper TseThreadPool::getNext(unsigned)
{
  TseRunnableWrapper task;
  boost::unique_lock<boost::mutex> lock(_mutex);
  while (!_joining && _tasks.empty())
  {
    _condition.wait(lock);
  }
  if (!_tasks.empty())
  {
    task = _tasks.front();
    _tasks.pop_front();
  }
  ++_numberActiveThreads;
  return task;
}

void TseThreadPool::killThread(unsigned id)
{
}

unsigned TseThreadPool::getConcurrency()
{
  return static_cast<unsigned>(concurrency + .5);
}

bool TseThreadPool::combineTasks(TseRunnableWrapper& task,
                                 Tasks& source,
                                 TseThreadingConst::TaskId taskId)
{
  unsigned threshold(ThreadPoolInfo::getBoosterCombineThreshold(taskId));
  if (source.size() > threshold
      && task.mergeTasks(source.back()))
  {
    source.pop_back();
    ++_numberCombinedTasks;
    return true;
  }
  return false;
}

bool TseThreadPool::canUseBooster() const
{
  return !_tasks.empty()
         && _tasks.size() + _numberActiveThreads >= _pool.size() + _boosterThreshold;
}

void TseThreadPool::getFirstInQueue(TseRunnableWrapper& task)
{
  task = _tasks.front();
  _tasks.pop_front();
  if (_shouldCombineTasks)
  {
    combineTasks(task, _tasks, _taskId);
  }
}

void TseThreadPool::updateTrxConcurrency(unsigned activeTasks)
{
  ++concurrencySamples;
  activeTrxTasks += activeTasks;
}

BoosterThreadPool::BoosterThreadPool(size_t maxSize)
  : TseThreadPool(TseThreadingConst::BOOSTER_TASK, maxSize)
{
  _pool.resize(maxSize);
  boost::unique_lock<boost::mutex> lock(_mutex);
  static const unsigned startBoosterSize(10);
  boosterEffSize = maxSize > startBoosterSize ? startBoosterSize : maxSize;
  for (unsigned i = 0; i < boosterEffSize; ++i)
  {
    _pool[i].reset(new Thread(*this, i));
  }
}

BoosterThreadPool::~BoosterThreadPool()
{
}

void BoosterThreadPool::registerPool(TseThreadPool* pool)
{
  boost::lock_guard<boost::mutex> lock(registrationMutex);
  _registeredPools.push_back(pool);
}

void BoosterThreadPool::unregisterPool(TseThreadPool* pool)
{
  boost::lock_guard<boost::mutex> lock(registrationMutex);
  for (size_t i = 0; i < _registeredPools.size(); ++i)
  {
    if (_registeredPools[i] == pool)
    {
      _registeredPools.erase(_registeredPools.begin() + i);
      return;
    }
  }
}

void BoosterThreadPool::adjust()
{
  static const unsigned adjustmentPeriod(ThreadPoolInfo::getBoosterAdjustmentPeriod());
  if (0 == adjustmentPeriod)
  {
    return;
  }
  static unsigned boosterCounter(0);
  if (++boosterCounter > adjustmentPeriod)
  {
    boosterCounter = 0;
    unsigned newSize(getEffectiveMaxSize(_maxSize));
    if (newSize >= boosterEffSize)
    {
      boosterEffSize = newSize;
    }
    else
    {
      int idleThreads(boosterEffSize - _numberActiveThreads - _tasks.size());
      if (idleThreads > 0)
      {
        int reduction(boosterEffSize - newSize);
        boosterEffSize -= std::min(idleThreads, reduction);
      }
    }
    for (unsigned i = 0; i < boosterEffSize && i < _pool.size(); ++i)
    {
      if (!_pool[i])
      {
        _pool[i].reset(new Thread(*this, i));
      }
    }
  }
}

TseRunnableWrapper BoosterThreadPool::getNext(unsigned id)
{
  TseRunnableWrapper task;
  boost::unique_lock<boost::mutex> lock(registrationMutex);
  time_t timer(std::time(0));
  while (!_joining)
  {
    {
      boost::unique_lock<boost::mutex> lock(_mutex);
      if (id < boosterEffSize && !_tasks.empty())
      {
        task = _tasks.front();
        _tasks.pop_front();
        ++_numberActiveThreads;
        return task;
      }
    }
    static unsigned startIdx(0);
    ++startIdx;
    for (unsigned i = 0; i < _registeredPools.size(); ++i)
    {
      unsigned k((startIdx + i) % _registeredPools.size());
      TseThreadPool* pool(_registeredPools[k]);
      if (pool->shouldConsumeBacklog2())
      {
        boost::lock_guard<boost::mutex> poolLock(pool->getMutex());
        if (pool->canUseBooster())
        {
          boost::unique_lock<boost::mutex> boosterLock(_mutex);
          if (id >= boosterEffSize)
          {
            break;
          }
          if (boosterEffSize > _numberActiveThreads + _tasks.size())
          {
            pool->getFirstInQueue(task);
            ++_numberActiveThreads;
            return task;
          }
        }
      }
    }
    static const int seconds(ThreadPoolInfo::getIdleThreadTimeout());
    if (std::time(0) - timer > seconds)
    {
      boost::unique_lock<boost::mutex> lock(_mutex);
      if (id >= boosterEffSize)
      {
        break;
      }
    }
    _condition.wait(lock);
  }
  ++_numberActiveThreads;
  return task;
}

void BoosterThreadPool::killThread(unsigned id)
{
  boost::unique_lock<boost::mutex> lock(_mutex);
  if (id < _pool.size())
  {
    _pool[id].reset();
  }
}

unsigned BoosterThreadPool::getEffPoolSize() const
{
  return boosterEffSize;
}

void BoosterThreadPool::runImmediately(TseThreadPool& pool)
{
  bool notify(false);
  boost::lock_guard<boost::mutex> lock(_mutex);
  adjust();
  Tasks& poolTasks(pool.getTasks());
  while (!poolTasks.empty()
         && boosterEffSize > _numberActiveThreads + _tasks.size())
  {
    notify = true; 
    _tasks.push_back(poolTasks.front());
    poolTasks.pop_front();
    if (pool.shouldCombineTasks())
    {
      combineTasks(_tasks.back(), poolTasks, pool.getTaskId());
    }
    if (!pool.shouldConsumeBacklog())
    {
      break;
    }
  }
  if (notify)
  {
    _condition.notify_all();
  }
}

}// tse
