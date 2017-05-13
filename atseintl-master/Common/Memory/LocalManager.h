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

#include "Common/Memory/CompositeManager.h"

#include <vector>

namespace tse
{
namespace Memory
{
class LocalManager : public CompositeManager
{
public:
  LocalManager() {}
  LocalManager(size_t initialCapacity) { _managers.reserve(initialCapacity); }
  virtual ~LocalManager();

  virtual void setOutOfMemory() override;
  virtual void updateTotalMemory() override;

protected:
  virtual void registerManagerImpl(Manager* manager) override final;
  virtual bool unregisterManagerImpl(Manager* manager) override final;

private:
  std::vector<Manager*> _managers;
};
}
}
