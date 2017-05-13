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

#include "Common/Memory/TrxManager.h"

#include <gmock/gmock.h>

namespace tse
{
namespace Memory
{
class MockTrxManager : public TrxManager
{
public:
  MockTrxManager() : TrxManager(nullptr) {}

  using Manager::setOutOfMemoryFlag;
  using Manager::restoreOutOfMemoryFlag;
  using Manager::setTotalMemory;
  using Manager::changeTotalMemory;

  using CompositeManager::_mutex;

  MOCK_METHOD0(setOutOfMemory, void());
  MOCK_METHOD0(updateTotalMemory, void());
};
}
}
