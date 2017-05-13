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
#pragma once

#include "Common/Memory/Manager.h"

#include <mutex>

namespace tse
{
namespace Memory
{
class CompositeManager : public Manager
{
protected:
  typedef std::recursive_mutex Mutex;

public:
  // Make sure the manager is idle when calling these functions.
  void registerManager(Manager* manager);
  bool unregisterManager(Manager* manager);
  bool archiveManager(Manager* manager);

protected:
  // These are called with mutex locked.
  virtual void registerManagerImpl(Manager* manager);
  virtual bool unregisterManagerImpl(Manager* manager);

  mutable Mutex _mutex;
};
}
}
