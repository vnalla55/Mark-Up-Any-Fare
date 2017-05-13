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

#include "Common/Memory/LocalManager.h"
#include "Common/Memory/OutOfMemoryException.h"
#include "Common/Memory/test/MockManager.h"

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"

namespace tse
{
namespace Memory
{
using namespace ::testing;

class LocalManagerTest : public Test
{
};

TEST_F(LocalManagerTest, testRegisterUnregisterImpl)
{
  LocalManager manager;
  MockManager child(true);
  child.setTotalMemory(1337);

  manager.registerManager(&child);

  EXPECT_EQ(&manager, child.parent());
  EXPECT_EQ(1337, manager.getTotalMemory());

  EXPECT_TRUE(manager.unregisterManager(&child));

  EXPECT_EQ(nullptr, child.parent());
  EXPECT_EQ(0, manager.getTotalMemory());
}

TEST_F(LocalManagerTest, testDestructorUnregistersAllChildren)
{
  MockManager child1(true), child2(true);

  {
    LocalManager manager;

    manager.registerManager(&child1);
    manager.registerManager(&child2);
  }

  EXPECT_EQ(nullptr, child1.parent());
  EXPECT_EQ(nullptr, child2.parent());
}

TEST_F(LocalManagerTest, testSetOutOfMemory)
{
  LocalManager manager;
  MockManager child1(true), child2(true);
  child1.setTotalMemory(1337);
  child2.setTotalMemory(42);

  manager.registerManager(&child1);
  manager.registerManager(&child2);

  EXPECT_CALL(child1, setOutOfMemory()).Times(1);
  EXPECT_CALL(child2, setOutOfMemory()).Times(1);

  manager.setOutOfMemory();

  // The second one won't propagate to children.
  manager.setOutOfMemory();
}

TEST_F(LocalManagerTest, testUpdateTotalMemory)
{
  LocalManager manager;
  MockManager child1(true), child2(true);
  child1.setTotalMemory(1337);
  child2.setTotalMemory(42);

  manager.registerManager(&child1);
  manager.registerManager(&child2);

  EXPECT_CALL(child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(child2, updateTotalMemory()).Times(1);

  manager.updateTotalMemory();
}

TEST_F(LocalManagerTest, testSetOutOfMemoryBlocksRegisters)
{
  LocalManager manager;
  MockManager child(true);

  manager.setOutOfMemory();

  EXPECT_THROW(manager.registerManager(&child), OutOfMemoryException);
}
}
}
