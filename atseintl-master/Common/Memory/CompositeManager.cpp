//------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Common/Memory/CompositeManager.h"

#include "Util/BranchPrediction.h"

#include <cassert>

namespace tse
{
namespace Memory
{
void
CompositeManager::registerManager(Manager* manager)
{
  {
    std::unique_lock<Mutex> guard(_mutex);
    registerManagerImpl(manager);

    assert(!manager->parent());
    manager->setParent(this);
  }

  if (const size_t memory = manager->getTotalMemory())
    changeTotalMemory(memory);
}

bool
CompositeManager::unregisterManager(Manager* manager)
{
  if (UNLIKELY(!archiveManager(manager)))
    return false;

  if (const size_t memory = manager->getTotalMemory())
    changeTotalMemory(-memory);

  return true;
}

bool
CompositeManager::archiveManager(Manager* manager)
{
  std::unique_lock<Mutex> guard(_mutex);

  if (UNLIKELY(!unregisterManagerImpl(manager)))
    return false;

  assert(manager->parent() == this);
  manager->setParent(nullptr);

  return true;
}

void
CompositeManager::registerManagerImpl(Manager* manager)
{
}

bool
CompositeManager::unregisterManagerImpl(Manager* manager)
{
  return false;
}
}
}
