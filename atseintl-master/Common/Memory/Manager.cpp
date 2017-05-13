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

#include "Common/Memory/Manager.h"

#include "Common/Memory/CompositeManager.h"
#include "Util/BranchPrediction.h"

namespace tse
{
namespace Memory
{
Manager::~Manager()
{
  if (parent())
    parent()->unregisterManager(this);
}

void
Manager::setOutOfMemory()
{
  _outOfMemory.store(true, std::memory_order_release);
}

void
Manager::updateTotalMemory()
{
}

void
Manager::totalMemoryChanged(ptrdiff_t by, size_t to)
{
  if (LIKELY(parent()))
    parent()->changeTotalMemory(by);
  if (UNLIKELY(to >= _notificationThreshold))
    notifyTotalMemoryChanged(by, to);
}

void
Manager::notifyTotalMemoryChanged(ptrdiff_t by, size_t to)
{
  if (UNLIKELY(to > _threshold))
    setOutOfMemory();
}
}
}
