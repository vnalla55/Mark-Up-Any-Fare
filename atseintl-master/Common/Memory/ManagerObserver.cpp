//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/Memory/ManagerObserver.h"

#include "Common/Thread/TimerTaskExecutor.h"
#include "DataModel/BaseTrx.h"

namespace tse
{
namespace Memory
{
ManagerObserver::ManagerObserver()
{
  TrxCounter::registerObserver(*this);
}

ManagerObserver::~ManagerObserver()
{
  TrxCounter::unregisterObserver(*this);
}

void
ManagerObserver::trxCreated(BaseTrx& trx)
{
  if (!trx.getMemoryManager())
    return;

  _memoryManager.registerManager(trx.getMemoryManager());
  const uint16_t id = trx.getMemoryManager()->getId();
  TimerTaskExecutor::instance().scheduleNow(_memoryManager.getUpdateTotalMemoryTask(id));
}

void
ManagerObserver::trxDeleted(BaseTrx& trx)
{
  if (!trx.getMemoryManager())
    return;

  TimerTaskExecutor::instance().cancel(
      _memoryManager.getUpdateTotalMemoryTask(trx.getMemoryManager()->getId()));
}
}
}
