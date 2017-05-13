#include "Common/Thread/TseCallableTrxTask.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Assert.h"
#include "Common/DCFactoryBase.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DataModel/PricingTrx.h"

#include <sstream>

namespace
{
tse::Logger
_logger("atseintl.Common.TseCallableTrxTask");
}

namespace tse
{

const TseCallableTrxTask::SequenceID TseCallableTrxTask::SequenceID::EMPTY;

namespace
{
__thread Trx* curTrx = nullptr;

class CurThreadTseCallableTrxTask
{
public:
  CurThreadTseCallableTrxTask(TseCallableTrxTask* setTask)
  {
    TSE_ASSERT(setTask != nullptr);
    _parentTask = _curTask;
    _curTask = setTask;
  }
  ~CurThreadTseCallableTrxTask()
  {
    TSE_ASSERT(_curTask != nullptr);
    _curTask = _parentTask;
  }
  static TseCallableTrxTask* get() { return _curTask; }

private:
  // yeah, TseCallableTrxTask::run can be called again in the same thread (implicit recursion)
  TseCallableTrxTask* _parentTask;

  static __thread TseCallableTrxTask* _curTask;
};

__thread TseCallableTrxTask* CurThreadTseCallableTrxTask::_curTask = nullptr;

} // anon ns

TseCallableTrxTask::TseCallableTrxTask()
  : _desc(nullptr),
    _trx(nullptr),
    _parentThreadID(pthread_self()),
    _parentTask(nullptr),
    _taskSeqNum(-1), // means uninitialized
    _childTaskNextSeqNum(0) // doesn't mean anything until _taskSeqNum will have been initialized
{
  initTaskSeqNumFromParent();
}

TseCallableTrxTask::TseCallableTrxTask(const TseCallableTrxTask& task)
  : TseCallableTask(task),
    _desc(task._desc),
    _trx(task._trx),
    _parentThreadID(task._parentThreadID),

    // will use real parent to get task seq. num initialized
    _parentTask(nullptr),
    _taskSeqNum(-1),
    _childTaskNextSeqNum(0)
{
  initTaskSeqNumFromParent();

  // or try to initialize self as a top-level task
  bool isInitialized = !(_taskSeqNum < 0);
  if (!isInitialized && _trx)
  {
    initTaskSeqNum(_trx->advanceTrxTaskHighLevelSeqNum());
  }
}

void
TseCallableTrxTask::initTaskSeqNumFromParent()
{
  _parentTask = CurThreadTseCallableTrxTask::get();

  if (_parentTask)
    initTaskSeqNum(_parentTask->advanceNextChildTaskSeqNum());
}

void
TseCallableTrxTask::trx(PricingTrx* trx)
{
  _trx = trx;

  bool isInitialized = !(_taskSeqNum < 0);
  if (!isInitialized) // has been not initialized yet
  {
    initTaskSeqNum(trx->advanceTrxTaskHighLevelSeqNum());
  }
}

TseCallableTrxTask::CurrentTrxSetter::CurrentTrxSetter(Trx* trx, bool activated)
  : _activated(activated)
{
  if (_activated)
  {
    curTrx = trx;
  }
}

TseCallableTrxTask::CurrentTrxSetter::~CurrentTrxSetter()
{
  if (_activated)
  {
    curTrx = nullptr;
  }
}

Trx*
TseCallableTrxTask::currentTrx()
{
  return curTrx;
}

const TseCallableTrxTask*
TseCallableTrxTask::currentTask()
{
  return CurThreadTseCallableTrxTask::get();
}

TseCallableTrxTask::SequenceID
TseCallableTrxTask::getSequenceID() const
{
  if (UNLIKELY(_taskSeqNum < 0))
    return SequenceID();

  std::vector<InternalSequenceNum> sequence;
  for (const TseCallableTrxTask* task = this; task != nullptr; task = task->_parentTask)
  {
    bool isInitialized = !(task->_taskSeqNum < 0);
    if (UNLIKELY(!isInitialized))
    {
      LOG4CXX_WARN(_logger, "SequenceID contains uninitialized token");
    }
    sequence.push_back(task->_taskSeqNum);
  }

  // Parent id shall go first to guarantee proper sequence sort order
  return SequenceID(sequence.rbegin(), sequence.rend());
}

void
TseCallableTrxTask::run()
{
  CurThreadTseCallableTrxTask set(this);

  const bool isChildThread = pthread_self() != _parentThreadID;
  const CurrentTrxSetter trxSetter(_trx, isChildThread);

  // make this task do all its allocations in a transaction block
  // if it's a child thread (a transaction block should already
  // be set if it's not a child thread)
  const MallocContext allocatorContext(isChildThread);

  TSELatencyData d(*_trx, _desc, isChildThread);
  LOG4CXX_DEBUG(_logger, "Performing \"" << _desc << "\" sequenceID=" << getSequenceID());

  // Cause a new diagcollector to be created for this thread
  pthread_t currentThreadID = pthread_self();
  bool createdDiagCollector = false;
  DCFactoryBase* factory = DCFactoryBase::baseInstance();

  if (factory && !pthread_equal(currentThreadID, _parentThreadID))
  {
    createdDiagCollector = factory->createThreadDiag(*_trx, currentThreadID, _parentThreadID);
    if (UNLIKELY(!createdDiagCollector))
    {
      LOG4CXX_DEBUG(_logger, "Unable to create DiagCollector");
    }
  }

  // NOTE:  The Try-Catch block is here to ensure that we correctly reset the diagnostics
  try
  {
    // Do the actual work for this task object
    performTask();

    // Remove the diagcollector that was created for this thread.
    if (createdDiagCollector && !pthread_equal(currentThreadID, _parentThreadID))
    {
      if (UNLIKELY(!factory->endThreadDiag(*_trx, currentThreadID, _parentThreadID)))
      {
        LOG4CXX_DEBUG(_logger, "Unable to remove DiagCollector");
      }
    }
  }
  catch (...)
  {
    // Remove the diagcollector that was created for this thread.
    if (createdDiagCollector && !pthread_equal(currentThreadID, _parentThreadID))
    {
      if (!factory->endThreadDiag(*_trx, currentThreadID, _parentThreadID))
      {
        LOG4CXX_DEBUG(_logger, "Unable to remove DiagCollector");
      }
    }
    throw;
  }
}

int
TseCallableTrxTask::getCurrentTrxId()
{
  int trxId(-1);
  Trx* trx(currentTrx());
  if (trx)
  {
    trxId = int(trx->getBaseIntId());
  }
  return trxId;
}

DataHandle* TseCallableTrxTask::getCurrentDataHandle()
{
  Trx* trx(currentTrx());
  if (trx)
  {
    return &trx->dataHandle();
  }
  return nullptr;
}

std::ostream& operator<<(std::ostream& os, const TseCallableTrxTask::SequenceID& seq)
{
  if (seq.empty())
    return (os << "<empty>");

  const char* sep = "";
  for (const auto& elem : seq)
  {
    os << sep << elem;
    sep = ":";
  }
  return os;
}

} // ns tse
