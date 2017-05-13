// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Common/Memory/CompositeManager.h"

#include <gmock/gmock.h>

namespace tse
{
namespace Memory
{
class MockCompositeManager : public CompositeManager
{
public:
  using Manager::setNotificationThreshold;
  using Manager::setOutOfMemoryFlag;
  using Manager::restoreOutOfMemoryFlag;
  using Manager::setTotalMemory;
  using Manager::changeTotalMemory;

  using CompositeManager::_mutex;

  MOCK_METHOD0(setOutOfMemory, void());
  MOCK_METHOD0(updateTotalMemory, void());
  MOCK_METHOD2(notifyTotalMemoryChanged, void(ptrdiff_t by, size_t to));

  MOCK_METHOD1(registerManagerImpl, void(Manager* manager));
  MOCK_METHOD1(unregisterManagerImpl, bool(Manager* manager));
};
}
}
