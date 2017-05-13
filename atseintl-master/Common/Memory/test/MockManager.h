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

#include "Common/Memory/Manager.h"

#include <gmock/gmock.h>

namespace tse
{
namespace Memory
{
class MockManager : public Manager
{
public:
  static constexpr int IGNORE_NOTIFY_TOTAL_MEMORY_CHANGED = 1 << 0;
  static constexpr int DONT_FORWARD_UPDATE_NOTIFICATION_THRESHOLD = 1 << 1;

  MockManager(int flags = 0)
  {
    using ::testing::_;
    using ::testing::AnyNumber;
    using ::testing::Invoke;

    if (flags & IGNORE_NOTIFY_TOTAL_MEMORY_CHANGED)
      EXPECT_CALL(*this, notifyTotalMemoryChanged(_, _)).Times(AnyNumber());

    if (!(flags & DONT_FORWARD_UPDATE_NOTIFICATION_THRESHOLD))
      EXPECT_CALL(*this, updateNotificationThreshold()).Times(AnyNumber()).WillRepeatedly(Invoke(
          [this]() { defaultUpdateNotificationThreshold(); }));
  }

  using Manager::notificationThreshold;
  using Manager::setNotificationThreshold;
  using Manager::setOutOfMemoryFlag;
  using Manager::restoreOutOfMemoryFlag;
  using Manager::setTotalMemory;
  using Manager::changeTotalMemory;
  using Manager::totalMemoryChanged;

  void defaultUpdateNotificationThreshold()
  {
    Manager::updateNotificationThreshold();
  }
  void defaultTotalMemoryChanged(ptrdiff_t by, size_t to)
  {
    Manager::notifyTotalMemoryChanged(by, to);
  }

  MOCK_METHOD0(setOutOfMemory, void());
  MOCK_METHOD0(updateNotificationThreshold, void());
  MOCK_METHOD0(updateTotalMemory, void());
  MOCK_METHOD2(notifyTotalMemoryChanged, void(ptrdiff_t by, size_t to));
};
}
}
