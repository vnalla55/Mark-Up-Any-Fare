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

#include "Common/Memory/Monitor.h"
#include <gmock/gmock.h>

namespace tse
{
namespace Memory
{

class MockMonitor : public MemoryMonitor
{
public:
  static void createInstance()
  {
    _instance = std::unique_ptr<MemoryMonitor>(new MockMonitor());
  }

  static MockMonitor* instance()
  {
    return static_cast<MockMonitor*>(_instance.get());
  }

  MOCK_METHOD0(getUpdatedVirtualMemorySize, size_t());
  MOCK_METHOD0(getUpdatedResidentMemorySize, size_t());
  MOCK_METHOD0(getUpdatedAvailableMemorySize, size_t());
  MOCK_METHOD0(updateMonitor, void());
};

}
}
