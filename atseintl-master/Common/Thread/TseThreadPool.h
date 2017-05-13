#pragma once

#include "Common/Thread/TseRunnableWrapper.h"
#include "Common/Thread/TseThreadingConst.h"

#include <boost/atomic.hpp>
#include <boost/container/deque.hpp>
#include <boost/thread.hpp>
#include <limits>
#include "Util/BranchPrediction.h"

namespace tse
{

class BoosterThreadPool;

class TseThreadPool : boost::noncopyable
{
 public:

  class Thread : boost::noncopyable
  {
   public:
    Thread(TseThreadPool& pool,
           unsigned id = std::numeric_limits<unsigned>::max());

    ~Thread();

    void run();

   private:
    TseThreadPool& _pool;
    boost::thread _thread;
    const unsigned _id;
  };

  typedef boost::container::deque<TseRunnableWrapper> Tasks;
  typedef std::vector<std::shared_ptr<Thread>> Pool;

  TseThreadPool(TseThreadingConst::TaskId taskId,
                size_t maxSize);
  virtual ~TseThreadPool();

  virtual TseRunnableWrapper getNext(unsigned id);
  virtual void killThread(unsigned id);

  void enqueue(const TseRunnableWrapper& task,
               bool front = false);
  bool canUseBooster() const;
  void getFirstInQueue(TseRunnableWrapper& task);
  size_t size() const;
  size_t getQueueSize() const;
  unsigned getNumberActiveThreads() const { return _numberActiveThreads; }
  const BoosterThreadPool* getBooster() const { return _booster; }
  boost::mutex& getMutex() { return _mutex; }
  Tasks& getTasks() { return _tasks; }
  TseThreadingConst::TaskId getTaskId() const { return _taskId; }
  bool shouldConsumeBacklog() const { return _shouldConsumeBacklog; }
  bool shouldConsumeBacklog2() const { return _shouldConsumeBacklog2; }
  bool shouldCombineTasks() const { return _shouldCombineTasks; }
  size_t getNumberCombinedTasks() const { return _numberCombinedTasks; }
  static void updateTrxConcurrency(unsigned activeTrxTasks);
  static unsigned getConcurrency();

  typedef std::map<TseThreadingConst::TaskId, std::shared_ptr<TseThreadPool>> PoolMap;
  static PoolMap _poolMap;

 protected:
  bool combineTasks(TseRunnableWrapper& task,
                    Tasks& source,
                    TseThreadingConst::TaskId taskId);

  const TseThreadingConst::TaskId _taskId;
  const unsigned _maxSize;
  BoosterThreadPool* _booster;
  const unsigned _boosterThreshold;
  mutable boost::mutex _mutex;
  boost::condition_variable _condition;
  const bool _shouldConsumeBacklog;
  const bool _shouldConsumeBacklog2;
  const bool _shouldCombineTasks;
  bool _joining;
  Tasks _tasks;
  boost::atomic<unsigned> _numberActiveThreads;
  boost::atomic<size_t> _numberCombinedTasks;
  Pool _pool;
};

class BoosterThreadPool : public TseThreadPool
{
 public:
  explicit BoosterThreadPool(size_t maxSize);
  virtual ~BoosterThreadPool() override;

  virtual TseRunnableWrapper getNext(unsigned id) override;
  virtual void killThread(unsigned id) override;

  unsigned getEffPoolSize() const;
  void runImmediately(TseThreadPool& pool);
  void registerPool(TseThreadPool* pool);
  void unregisterPool(TseThreadPool* pool);

  static std::shared_ptr<BoosterThreadPool> _instance;

 private:
  void adjust();

  std::vector<TseThreadPool*> _registeredPools;
};

}// tse
