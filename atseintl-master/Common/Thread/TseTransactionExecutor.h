#pragma once

#include "Common/Thread/TseRunnableExecutor.h"

#include <memory>

namespace tse
{

class TseTransactionExecutor : public TseRunnableExecutor
{
public:
  TseTransactionExecutor();

  virtual void execute(TseCallableTask* task);
private:
  size_t _threshold;
  std::unique_ptr<TseRunnableExecutor> _throttlingExecutor;
};
}// tse
