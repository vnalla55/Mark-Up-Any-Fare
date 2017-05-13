#pragma once

#include "Common/Thread/TseCallableTask.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Util/BranchPrediction.h"

namespace tse
{

class TseRunnableWrapper
{
  struct Subtask
  {
    Subtask(TseCallableTask* runnable = 0,
            TseRunnableExecutor* runnableExecutor = 0,
            bool destroyRunnable = false)
      : _runnable(runnable)
      , _runnableExecutor(runnableExecutor)
      , _destroyRunnable(destroyRunnable)
    {
    }

    inline void run()
    {
      if (_runnable != 0 && _runnableExecutor != 0)
      {
        if (LIKELY(!_runnableExecutor->isCanceled()))
        {
          try
          {
            _runnable->run();
          }
          catch (const ErrorResponseException& e)
          {
            _runnableExecutor->recordFirstException(e);
          }
          catch (const std::exception& e)
          {
            _runnableExecutor->recordFirstException(e);
          }
          catch (...)
          {
            _runnableExecutor->recordFirstException();
          }
        }
        _runnableExecutor->decrementTaskCount();
        if (_destroyRunnable)
        {
          delete _runnable;
        }
      }
    }

    operator bool () const
    {
      return _runnable != 0;
    }

    TseCallableTask* _runnable;
    TseRunnableExecutor* _runnableExecutor;
    bool _destroyRunnable;
  };

  Subtask _subtasks[2];

 public:

  TseRunnableWrapper(TseCallableTask* runnable = 0,
                     TseRunnableExecutor* runnableExecutor = 0,
                     bool destroyRunnable = false)
  {
    _subtasks[0]._runnable = runnable;
    _subtasks[0]._runnableExecutor = runnableExecutor;
    _subtasks[0]._destroyRunnable = destroyRunnable;
  }

  inline void run()
  {
    _subtasks[0].run();
    _subtasks[1].run();
  }

  bool mergeTasks(TseRunnableWrapper& other)
  {
    if (canMerge() && other.canMerge())
    {
      _subtasks[1] = other._subtasks[0];
      return true;
    }
    return false;
  }

  bool canMerge() const
  {
    return _subtasks[0] && !_subtasks[1];
  }

  bool empty() const
  {
    return 0 == _subtasks[0]._runnable;
  }
};

}// tse
