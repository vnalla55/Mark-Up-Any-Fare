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
#include "Common/Memory/test/MockCompositeManager.h"
#include "Common/Memory/test/MockManager.h"

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"

namespace tse
{
namespace Memory
{
using namespace ::testing;

class CompositeManagerTest : public Test
{
};

TEST_F(CompositeManagerTest, testRegisterManagerImplCalled)
{
  MockCompositeManager manager;
  MockManager child;

  EXPECT_CALL(manager, registerManagerImpl(&child)).Times(1);

  manager.registerManager(&child);

  EXPECT_EQ(&manager, child.parent());

  child.setParent(nullptr);
}

TEST_F(CompositeManagerTest, testRegisterManagerAccountsForMemory)
{
  MockCompositeManager manager;
  manager.setNotificationThreshold(0);
  MockManager child(MockManager::IGNORE_NOTIFY_TOTAL_MEMORY_CHANGED);
  child.setTotalMemory(1337);

  EXPECT_CALL(manager, registerManagerImpl(&child)).Times(1);
  EXPECT_CALL(manager, notifyTotalMemoryChanged(1337, 1337)).Times(1);

  manager.registerManager(&child);

  child.setParent(nullptr);
}

TEST_F(CompositeManagerTest, testUnregisterManagerImplCalled)
{
  InSequence is;

  MockCompositeManager manager;
  MockManager child;

  EXPECT_CALL(manager, registerManagerImpl(&child)).Times(1);

  manager.registerManager(&child);

  EXPECT_CALL(manager, unregisterManagerImpl(&child)).Times(1).WillOnce(Return(true));

  EXPECT_TRUE(manager.unregisterManager(&child));

  EXPECT_EQ(nullptr, child.parent());
}

TEST_F(CompositeManagerTest, testUnregisterManagerImplReturnedFalse)
{
  MockCompositeManager manager;
  MockManager child(MockManager::IGNORE_NOTIFY_TOTAL_MEMORY_CHANGED);
  child.setTotalMemory(1337);

  EXPECT_CALL(manager, unregisterManagerImpl(&child)).Times(1).WillOnce(Return(false));

  EXPECT_FALSE(manager.unregisterManager(&child));

  EXPECT_EQ(nullptr, child.parent());
}

TEST_F(CompositeManagerTest, testUnregisterManagerAccountsForMemory)
{
  InSequence is;

  MockCompositeManager manager;
  manager.setNotificationThreshold(0);
  MockManager child(MockManager::IGNORE_NOTIFY_TOTAL_MEMORY_CHANGED);
  child.setTotalMemory(1337);

  EXPECT_CALL(manager, registerManagerImpl(&child)).Times(1);
  EXPECT_CALL(manager, notifyTotalMemoryChanged(1337, 1337)).Times(1);

  manager.registerManager(&child);

  EXPECT_CALL(manager, unregisterManagerImpl(&child)).Times(1).WillOnce(Return(true));
  EXPECT_CALL(manager, notifyTotalMemoryChanged(-1337, 0)).Times(1);

  EXPECT_TRUE(manager.unregisterManager(&child));
}

TEST_F(CompositeManagerTest, testArchiveManagerImplCalled)
{
  InSequence is;

  MockCompositeManager manager;
  MockManager child;

  EXPECT_CALL(manager, registerManagerImpl(&child)).Times(1);

  manager.registerManager(&child);

  EXPECT_CALL(manager, unregisterManagerImpl(&child)).Times(1).WillOnce(Return(true));

  EXPECT_TRUE(manager.archiveManager(&child));

  EXPECT_EQ(nullptr, child.parent());
}

TEST_F(CompositeManagerTest, testArchiveManagerImplReturnedFalse)
{
  MockCompositeManager manager;
  MockManager child(MockManager::IGNORE_NOTIFY_TOTAL_MEMORY_CHANGED);
  child.setTotalMemory(1337);

  EXPECT_CALL(manager, unregisterManagerImpl(&child)).Times(1).WillOnce(Return(false));

  EXPECT_FALSE(manager.archiveManager(&child));

  EXPECT_EQ(nullptr, child.parent());
}

TEST_F(CompositeManagerTest, testArchiveManagerStoresMemory)
{
  InSequence is;

  MockCompositeManager manager;
  manager.setNotificationThreshold(0);
  MockManager child(MockManager::IGNORE_NOTIFY_TOTAL_MEMORY_CHANGED);
  child.setTotalMemory(1337);

  EXPECT_CALL(manager, registerManagerImpl(&child)).Times(1);
  EXPECT_CALL(manager, notifyTotalMemoryChanged(1337, 1337)).Times(1);

  manager.registerManager(&child);

  EXPECT_CALL(manager, unregisterManagerImpl(&child)).Times(1).WillOnce(Return(true));

  EXPECT_TRUE(manager.archiveManager(&child));
}

TEST_F(CompositeManagerTest, testChildDestructorUnregisters)
{
  InSequence is;

  MockCompositeManager manager;
  MockManager child;

  EXPECT_CALL(manager, registerManagerImpl(&child)).Times(1);

  manager.registerManager(&child);

  EXPECT_CALL(manager, unregisterManagerImpl(&child)).Times(1).WillOnce(Return(true));
}
}
}
