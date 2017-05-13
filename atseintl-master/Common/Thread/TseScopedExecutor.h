#pragma once

#include "Common/Thread/TseRunnableExecutor.h"

namespace tse
{

class TseScopedExecutor : public TseRunnableExecutor
{
 public:
   TseScopedExecutor(TseThreadingConst::TaskId taskId, size_t maxActiveSize)
     : TseRunnableExecutor(taskId, maxActiveSize)
   {
   }
};

}// tse

