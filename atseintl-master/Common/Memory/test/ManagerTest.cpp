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

#include "Common/Memory/CompositeManager.h"
#include "Common/Memory/Manager.h"
#include "Common/Memory/test/MockCompositeManager.h"
#include "Common/Memory/test/MockManager.h"

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"

namespace tse
{
namespace Memory
{
using namespace ::testing;

class ManagerTest : public Test
{
};

TEST_F(ManagerTest, testConstructorDestructor)
{
  MockManager manager;

  EXPECT_EQ(nullptr, manager.parent());
  EXPECT_EQ(std::numeric_limits<size_t>::max(), manager.threshold());
  EXPECT_FALSE(manager.isOutOfMemory());
  EXPECT_EQ(0, manager.getTotalMemory());
}

TEST_F(ManagerTest, testSetOutOfMemory)
{
  MockManager manager;

  manager.Manager::setOutOfMemory();
  EXPECT_TRUE(manager.isOutOfMemory());
}

TEST_F(ManagerTest, testSetRestoreOutOfMemoryFlag)
{
  MockManager manager;

  EXPECT_TRUE(manager.setOutOfMemoryFlag());
  EXPECT_TRUE(manager.isOutOfMemory());

  EXPECT_FALSE(manager.setOutOfMemoryFlag());
  EXPECT_TRUE(manager.isOutOfMemory());

  manager.restoreOutOfMemoryFlag();
  EXPECT_FALSE(manager.isOutOfMemory());

  EXPECT_TRUE(manager.setOutOfMemoryFlag());
  EXPECT_TRUE(manager.isOutOfMemory());
}

TEST_F(ManagerTest, testSetTotalMemoryGetter)
{
  MockManager manager(MockManager::IGNORE_NOTIFY_TOTAL_MEMORY_CHANGED);

  EXPECT_EQ(1337, manager.setTotalMemory(1337));
  EXPECT_EQ(1337, manager.getTotalMemory());
}

TEST_F(ManagerTest, testSetTotalMemoryCallsTotalMemoryChanged)
{
  InSequence is;

  MockManager manager;
  manager.setNotificationThreshold(0);
  EXPECT_CALL(manager, notifyTotalMemoryChanged(1337, 1337)).Times(1);

  EXPECT_EQ(1337, manager.setTotalMemory(1337));
  EXPECT_EQ(1337, manager.getTotalMemory());

  EXPECT_CALL(manager, notifyTotalMemoryChanged(42 - 1337, 42)).Times(1);

  EXPECT_EQ(42, manager.setTotalMemory(42));
  EXPECT_EQ(42, manager.getTotalMemory());

  EXPECT_EQ(42, manager.setTotalMemory(42));
  EXPECT_EQ(42, manager.getTotalMemory());
}

TEST_F(ManagerTest, testChangeTotalMemoryGetter)
{
  MockManager manager(MockManager::IGNORE_NOTIFY_TOTAL_MEMORY_CHANGED);

  EXPECT_EQ(1337, manager.changeTotalMemory(1337));
  EXPECT_EQ(1337, manager.getTotalMemory());

  EXPECT_EQ(0, manager.changeTotalMemory(-1337));
  EXPECT_EQ(0, manager.getTotalMemory());
}

TEST_F(ManagerTest, testChangeTotalMemoryCallsTotalMemoryChanged)
{
  InSequence is;

  MockManager manager;
  manager.setNotificationThreshold(0);
  EXPECT_CALL(manager, notifyTotalMemoryChanged(1337, 1337)).Times(1);

  EXPECT_EQ(1337, manager.changeTotalMemory(1337));
  EXPECT_EQ(1337, manager.getTotalMemory());

  EXPECT_CALL(manager, notifyTotalMemoryChanged(42 - 1337, 42)).Times(1);

  EXPECT_EQ(42, manager.changeTotalMemory(42 - 1337));
  EXPECT_EQ(42, manager.getTotalMemory());
}

TEST_F(ManagerTest, testTotalMemoryChangedPropagatesToParent)
{
  InSequence is;

  MockManager manager;
  MockCompositeManager parent;
  parent.setNotificationThreshold(0);
  manager.setParent(&parent);

  EXPECT_CALL(parent, notifyTotalMemoryChanged(50, 50)).Times(1);

  parent.setTotalMemory(50);

  EXPECT_CALL(parent, notifyTotalMemoryChanged(1000, 1050)).Times(1);

  manager.totalMemoryChanged(1000, 1000);

  EXPECT_CALL(parent, notifyTotalMemoryChanged(-1000, 50)).Times(1);

  manager.totalMemoryChanged(-1000, 0);

  manager.setParent(nullptr);
}

TEST_F(ManagerTest, testTotalMemoryChangedIsNotCalledWhenBelowNotificationThreshold)
{
  InSequence is;

  MockManager manager;
  manager.setNotificationThreshold(1000);
  EXPECT_CALL(manager, notifyTotalMemoryChanged(1337, 1337)).Times(1);

  EXPECT_EQ(1337, manager.setTotalMemory(1337));
  EXPECT_EQ(1337, manager.getTotalMemory());

  EXPECT_EQ(42, manager.setTotalMemory(42));
  EXPECT_EQ(42, manager.getTotalMemory());

  EXPECT_CALL(manager, notifyTotalMemoryChanged(1000 - 42, 1000)).Times(1);

  EXPECT_EQ(1000, manager.setTotalMemory(1000));
  EXPECT_EQ(1000, manager.getTotalMemory());

  EXPECT_EQ(1000, manager.setTotalMemory(1000));
  EXPECT_EQ(1000, manager.getTotalMemory());
}

TEST_F(ManagerTest, testDefaultNotifyTotalMemoryChangedChecksThreshold)
{
  MockManager manager;
  manager.setThreshold(1000);

  manager.defaultTotalMemoryChanged(1000, 1000);

  EXPECT_CALL(manager, setOutOfMemory()).Times(2);

  manager.defaultTotalMemoryChanged(1, 1001);
  manager.defaultTotalMemoryChanged(-10, 991);
  manager.defaultTotalMemoryChanged(1000, 1991);
}

TEST_F(ManagerTest, testUpdateNotificationThresholdCopiesThreshold)
{
  MockManager manager;
  manager.setThreshold(1000);
  manager.setNotificationThreshold(0);

  EXPECT_EQ(0, manager.notificationThreshold());

  manager.defaultUpdateNotificationThreshold();

  EXPECT_EQ(1000, manager.notificationThreshold());
}

TEST_F(ManagerTest, testSetThresholdCallsUpdateNotificationThreshold)
{
  InSequence is;

  MockManager manager(MockManager::DONT_FORWARD_UPDATE_NOTIFICATION_THRESHOLD);

  EXPECT_CALL(manager, updateNotificationThreshold()).Times(1);

  manager.setThreshold(1000);

  EXPECT_CALL(manager, updateNotificationThreshold()).Times(1);

  manager.setThreshold(0);
}
}
}
