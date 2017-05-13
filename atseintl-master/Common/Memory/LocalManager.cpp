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

#include "Common/Memory/LocalManager.h"

#include "Common/Memory/OutOfMemoryException.h"
#include "Util/BranchPrediction.h"

#include <algorithm>

namespace tse
{
namespace Memory
{
LocalManager::~LocalManager()
{
  while (const size_t s = _managers.size())
    unregisterManager(_managers[s - 1]);
}

void
LocalManager::registerManagerImpl(Manager* manager)
{
  if (UNLIKELY(isOutOfMemory()))
    throw OutOfMemoryException();

  _managers.push_back(manager);
}

bool
LocalManager::unregisterManagerImpl(Manager* manager)
{
  const auto it = std::find(_managers.begin(), _managers.end(), manager);
  if (UNLIKELY(it == _managers.end()))
    return false;

  _managers.erase(it);
  return true;
}

void
LocalManager::setOutOfMemory()
{
  if (LIKELY(!setOutOfMemoryFlag()))
    return;

  std::unique_lock<Mutex> guard(_mutex);

  for (Manager* manager : _managers)
    manager->setOutOfMemory();
}

void
LocalManager::updateTotalMemory()
{
  std::unique_lock<Mutex> guard(_mutex);

  for (Manager* manager : _managers)
    manager->updateTotalMemory();
}
}
}
