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

#include "Common/Memory/Config.h"
#include "Common/Memory/GlobalManager.h"
#include "Common/Memory/OutOfMemoryException.h"

#include "Common/Memory/test/MockTrxManager.h"
#include "Common/Memory/test/MockMonitor.h"
#include "Common/Thread/test/MockTimerTaskExecutor.h"

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

namespace tse
{
namespace Memory
{

namespace
{
constexpr size_t GIGABYTE = 1024L * 1024 * 1024;
}

using namespace ::testing;

class GlobalManagerTest : public Test
{
public:
  void captureTask(TimerTask& task) { _tasks.push_back(&task); }
  void abandonTask(TimerTask& /*task*/) { _tasks.pop_back(); }
  void SetUp()
  {
    _memH(new TestConfigInitializer);
    _logger = _memH(new TestLogger("atseintl.Common.Memory.GlobalManager"));

    _monitorUpdatePeriod = 0;
    MockMonitor::createInstance();
    _monitorUpdatePeriod = 1000;
    MockTimerTaskExecutor::setInstance(new MockTimerTaskExecutor);
  }
  void TearDown()
  {
    MockTimerTaskExecutor::destroyInstance();
    MockMonitor::destroyInstance();
    _memH.clear();
    _tasks.clear();
  }

protected:
  void setUpThresholds(size_t normal = GIGABYTE, size_t critical = GIGABYTE)
  {
    _globalThreshold = normal;
    _criticalThreshold = critical;
    _criticalFreeRssWatermark = 0;
    _softFreeRssWatermark = 0;
    _manager = _memH(new GlobalManager);
    _child1 = _memH(new MockTrxManager);
    _child2 = _memH(new MockTrxManager);
  }
  void setUpWatermarks(size_t soft = 9 * GIGABYTE, size_t critical = 6 * GIGABYTE)
  {
    _globalThreshold = 0;
    _criticalThreshold = 0;
    _criticalFreeRssWatermark = critical;
    _softFreeRssWatermark = soft;
    _manager = _memH(new GlobalManager);
    _child1 = nullptr;
    _child2 = nullptr;
  }

  void checkMemoryWatermarks(bool soft, bool crit)
  {
    _manager->getUpdatedAvailableMemorySize();
    for (TimerTask* task : _tasks)
    {
      if (task)
      {
        task->performTask();
      }
    }
    ASSERT_EQ((bool) soft, _manager->isOutOfMemory());
    ASSERT_EQ((bool) crit, _manager->isCriticalCondition());
  }

  TestMemHandle _memH;
  TestLogger* _logger;
  GlobalManager* _manager;
  MockTrxManager* _child1;
  MockTrxManager* _child2;

private:
  std::vector<TimerTask*> _tasks;
};

TEST_F(GlobalManagerTest, testRegisterUnregisterImpl)
{
  setUpThresholds();
  _child1->setTotalMemory(1337);
  _manager->registerManager(_child1);

  EXPECT_EQ(_manager, _child1->parent());
  EXPECT_EQ(1337, _manager->getTotalMemory());

  EXPECT_TRUE(_manager->unregisterManager(_child1));

  EXPECT_EQ(nullptr, _child1->parent());
  EXPECT_EQ(0, _manager->getTotalMemory());
}

TEST_F(GlobalManagerTest, testDestructorUnregistersAll_children)
{
  setUpThresholds();
  {
    GlobalManager manager;
    manager.registerManager(_child1);
    manager.registerManager(_child2);
  }

  EXPECT_EQ(nullptr, _child1->parent());
  EXPECT_EQ(nullptr, _child2->parent());
}

TEST_F(GlobalManagerTest, testSetOutOfMemoryThresholdNotReached)
{
  InSequence is;
  setUpThresholds(2000);

  _child1->setTotalMemory(1337);
  _child2->setTotalMemory(42);

  _manager->registerManager(_child1);
  _manager->registerManager(_child2);

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(0);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(0);

  _manager->setOutOfMemory();
}

TEST_F(GlobalManagerTest, testSetOutOfMemoryThresholdNotReachedAfterUpdate)
{
  InSequence is;
  setUpThresholds(2000);

  _child1->setTotalMemory(1337);
  _child2->setTotalMemory(42);

  _manager->registerManager(_child1);
  _manager->registerManager(_child2);

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1)
      .WillOnce(Invoke([&]() { _child1->setTotalMemory(1337); }));
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(0);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(0);

  _child1->setTotalMemory(3000);
}

TEST_F(GlobalManagerTest, testSetOutOfMemoryTresholdReached)
{
  InSequence is;
  setUpThresholds(2000);

  _child1->setTotalMemory(1337);
  _child2->setTotalMemory(42);

  _manager->registerManager(_child1);
  _manager->registerManager(_child2);

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(1);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(0);

  _child1->setTotalMemory(3000);

  // The next ones won't work until total memory < threshold.
  EXPECT_TRUE(_manager->isOutOfMemory());

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);
  _manager->setOutOfMemory();

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);
  _child1->setTotalMemory(4000);

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);
  _child1->setTotalMemory(3000);

  // To prevent the destructor from calling setOutOfMemory.
  _child1->setTotalMemory(0);
}

TEST_F(GlobalManagerTest, testSetOutOfMemoryTresholdReachedTwice)
{
  InSequence is;
  setUpThresholds(2000);

  _child1->setTotalMemory(1337);
  _child2->setTotalMemory(42);

  _manager->registerManager(_child1);
  _manager->registerManager(_child2);

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(1);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(0);

  _child1->setTotalMemory(3000);

  EXPECT_TRUE(_manager->isOutOfMemory());

  _child1->setTotalMemory(1000);

  // The second _child will be called now.
  EXPECT_FALSE(_manager->isOutOfMemory());

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(0);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(1);

  _child2->setTotalMemory(2042);

  EXPECT_TRUE(_manager->isOutOfMemory());

  // To prevent the destructor from calling setOutOfMemory.
  _child2->setTotalMemory(0);
}

TEST_F(GlobalManagerTest, testSetOutOfMemoryCriticalReached)
{
  InSequence is;
  setUpThresholds(2000, 4000);

  _child1->setTotalMemory(337);
  _child2->setTotalMemory(1000);

  _manager->registerManager(_child1);
  _manager->registerManager(_child2);

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(1);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(0);

  _child1->setTotalMemory(3000);
  _child1->restoreOutOfMemoryFlag();

  EXPECT_TRUE(_manager->isOutOfMemory());
  EXPECT_FALSE(_manager->isCriticalCondition());

  // Stop all _children.
  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(1);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(1);

  _child2->setTotalMemory(1001);

  EXPECT_TRUE(_manager->isOutOfMemory());
  EXPECT_TRUE(_manager->isCriticalCondition());

  // Don't stop all _children twice.
  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  _child2->setTotalMemory(1002);

  EXPECT_TRUE(_manager->isOutOfMemory());
  EXPECT_TRUE(_manager->isCriticalCondition());

  // To prevent the destructor from calling setOutOfMemory.
  _child1->setTotalMemory(0);

  EXPECT_FALSE(_manager->isOutOfMemory());
  EXPECT_FALSE(_manager->isCriticalCondition());
}

TEST_F(GlobalManagerTest, testSetOutOfMemoryCriticalReachedTwice)
{
  InSequence is;
  setUpThresholds(2000, 4000);

  _child1->setTotalMemory(337);
  _child2->setTotalMemory(1000);

  _manager->registerManager(_child1);
  _manager->registerManager(_child2);

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(1);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(0);

  _child1->setTotalMemory(3000);
  _child1->restoreOutOfMemoryFlag();

  EXPECT_TRUE(_manager->isOutOfMemory());
  EXPECT_FALSE(_manager->isCriticalCondition());

  // Stop all _children.
  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(1);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(1);

  _child2->setTotalMemory(1001);

  EXPECT_TRUE(_manager->isOutOfMemory());
  EXPECT_TRUE(_manager->isCriticalCondition());

  _child1->setTotalMemory(0);
  _child1->restoreOutOfMemoryFlag();
  _child2->restoreOutOfMemoryFlag();

  EXPECT_FALSE(_manager->isOutOfMemory());
  EXPECT_FALSE(_manager->isCriticalCondition());

  // Stop all _children once again.
  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  EXPECT_CALL(*_child1, setOutOfMemory()).Times(1);
  EXPECT_CALL(*_child2, setOutOfMemory()).Times(1);

  _child1->setTotalMemory(4000);

  EXPECT_TRUE(_manager->isOutOfMemory());
  EXPECT_TRUE(_manager->isCriticalCondition());

  // To prevent the destructor from calling setOutOfMemory.
  _child1->setTotalMemory(0);

  EXPECT_FALSE(_manager->isOutOfMemory());
  EXPECT_FALSE(_manager->isCriticalCondition());
}

TEST_F(GlobalManagerTest, testUpdateTotalMemory)
{
  setUpThresholds();

  _child1->setTotalMemory(1337);
  _child2->setTotalMemory(42);

  _manager->registerManager(_child1);
  _manager->registerManager(_child2);

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);

  _manager->updateTotalMemory();
}

TEST_F(GlobalManagerTest, testUpdateTotalMemorySingle)
{
  InSequence is;
  setUpThresholds();

  _child1->setTotalMemory(1337);
  _child2->setTotalMemory(42);

  _manager->registerManager(_child1);
  _manager->registerManager(_child2);

  EXPECT_CALL(*_child1, updateTotalMemory()).Times(1);
  _manager->updateTotalMemory(_child1->getId());

  EXPECT_CALL(*_child2, updateTotalMemory()).Times(1);
  _manager->updateTotalMemory(_child2->getId());
}

TEST_F(GlobalManagerTest, testGetUpdatedAvailableMemorySize_AboveWatermarks)
{
  setUpWatermarks();

  EXPECT_CALL(*MockMonitor::instance(), getUpdatedAvailableMemorySize())
      // [01] soft watermark reached
      .WillOnce(Return(_softFreeRssWatermark)); // getUpdatedAvailableMemorySize
  EXPECT_CALL(MockTimerTaskExecutor::instance(), scheduleNow(_)).Times(Exactly(0));
  EXPECT_CALL(MockTimerTaskExecutor::instance(), cancel(_)).Times(Exactly(0));

  // [01] soft watermark reached
  checkMemoryWatermarks(false, false);
}

TEST_F(GlobalManagerTest, testGetUpdatedAvailableMemorySize_WatermarksDisabledAndNoMemoryLeft)
{
  setUpWatermarks(0, 0);

  EXPECT_CALL(*MockMonitor::instance(), getUpdatedAvailableMemorySize())
      // [01] soft watermark reached
      .WillOnce(Return(_softFreeRssWatermark))    // getUpdatedAvailableMemorySize
      // [02] no more memory
      .WillOnce(Return(0))                        // getUpdatedAvailableMemorySize
      // [03] above soft watermark
      .WillOnce(Return(_softFreeRssWatermark+1)); // getUpdatedAvailableMemorySize
  EXPECT_CALL(MockTimerTaskExecutor::instance(), scheduleNow(_)).Times(Exactly(0));
  EXPECT_CALL(MockTimerTaskExecutor::instance(), cancel(_)).Times(Exactly(0));

  // [01] soft watermark reached
  checkMemoryWatermarks(false, false);
  // [02] no more memory
  checkMemoryWatermarks(false, false);
  // [03] above soft watermark
  checkMemoryWatermarks(false, false);
}


TEST_F(GlobalManagerTest, testGetUpdatedAvailableMemorySize_SoftWatermark)
{
  setUpWatermarks();

  EXPECT_CALL(MockTimerTaskExecutor::instance(), scheduleNow(_))
      .WillOnce(Invoke(this, &GlobalManagerTest::captureTask));
  EXPECT_CALL(MockTimerTaskExecutor::instance(), cancel(_))
      .WillOnce(Invoke(this, &GlobalManagerTest::abandonTask));
  EXPECT_CALL(*MockMonitor::instance(), getUpdatedAvailableMemorySize())
      // [01] soft watermark reached
      .WillOnce(Return(_softFreeRssWatermark))    // getUpdatedAvailableMemorySize
      // [02] below soft watermark but above critical watermark
      .WillOnce(Return(_softFreeRssWatermark-1))  // getUpdatedAvailableMemorySize; shedule
      .WillOnce(Return(_softFreeRssWatermark-1))  // performTask
      // [03] soft watermark reached
      .WillOnce(Return(_softFreeRssWatermark))    // getUpdatedAvailableMemorySize
      .WillOnce(Return(_softFreeRssWatermark))    // performTask
      // [04] above soft watermark
      .WillOnce(Return(_softFreeRssWatermark+1))  // getUpdatedAvailableMemorySize
      .WillOnce(Return(_softFreeRssWatermark+1))  // performTask; cancel
      // [05] above soft watermark
      .WillOnce(Return(_softFreeRssWatermark+1)); // getUpdatedAvailableMemorySize

  // [01] soft watermark reached
  checkMemoryWatermarks(false, false);
  // [02] below soft watermark but above critical watermark
  checkMemoryWatermarks(true, false);
  // [03] soft watermark reached
  checkMemoryWatermarks(true, false);
  // [04] above soft watermark
  checkMemoryWatermarks(false, false);
  // [05] above soft watermark
  checkMemoryWatermarks(false, false);
}

TEST_F(GlobalManagerTest, testGetUpdatedAvailableMemorySize_CriticalWatermark)
{
  setUpWatermarks();

  EXPECT_CALL(MockTimerTaskExecutor::instance(), scheduleNow(_))
      .WillOnce(Invoke(this, &GlobalManagerTest::captureTask));
  EXPECT_CALL(MockTimerTaskExecutor::instance(), cancel(_))
      .WillOnce(Invoke(this, &GlobalManagerTest::abandonTask));
  EXPECT_CALL(*MockMonitor::instance(), getUpdatedAvailableMemorySize())
      // [01] soft watermark reached
      .WillOnce(Return(_softFreeRssWatermark))       // getUpdatedAvailableMemorySize
      // [02] below soft watermark but above critical watermark
      .WillOnce(Return(_softFreeRssWatermark-1))     // getUpdatedAvailableMemorySize; shedule
      .WillOnce(Return(_softFreeRssWatermark-1))     // performTask
      // [03] critical watermark reached
      .WillOnce(Return(_criticalFreeRssWatermark))   // getUpdatedAvailableMemorySize
      .WillOnce(Return(_criticalFreeRssWatermark))   // performTask
      // [04] below critical watermark
      .WillOnce(Return(_criticalFreeRssWatermark-1)) // getUpdatedAvailableMemorySize
      .WillOnce(Return(_criticalFreeRssWatermark-1)) // performTask
      // [05] 0 reached
      .WillOnce(Return(0))                           // getUpdatedAvailableMemorySize
      .WillOnce(Return(0))                           // performTask
      // [06] critical watermark reached
      .WillOnce(Return(_criticalFreeRssWatermark))   // getUpdatedAvailableMemorySize
      .WillOnce(Return(_criticalFreeRssWatermark))   // performTask
      // [07] above critical watermark but below soft watermark
      .WillOnce(Return(_criticalFreeRssWatermark+1)) // getUpdatedAvailableMemorySize
      .WillOnce(Return(_criticalFreeRssWatermark+1)) // performTask
      // [08] soft watermark reached
      .WillOnce(Return(_softFreeRssWatermark))       // getUpdatedAvailableMemorySize
      .WillOnce(Return(_softFreeRssWatermark))       // performTask
      // [09] above soft watermark
      .WillOnce(Return(_softFreeRssWatermark+1))     // getUpdatedAvailableMemorySize
      .WillOnce(Return(_softFreeRssWatermark+1))     // performTask; cancel
      // [10] above soft watermark
      .WillOnce(Return(_softFreeRssWatermark+1));    // getUpdatedAvailableMemorySize

  // [01] soft watermark reached
  checkMemoryWatermarks(false, false);
  // [02] below soft watermark and above critical watermark
  checkMemoryWatermarks(true, false);
  // [03] reached critical watermark
  checkMemoryWatermarks(true, false);
  // [04] below critical watermark
  checkMemoryWatermarks(true, true);
  // [05] reached 0
  checkMemoryWatermarks(true, true);
  // [06] reached critical watermark
  checkMemoryWatermarks(true, true);
  // [07] above critical watermark but below soft watermark
  checkMemoryWatermarks(true, false);
  // [08] soft watermark reached
  checkMemoryWatermarks(true, false);
  // [09] above soft watermark
  checkMemoryWatermarks(false, false);
  // [10] above soft watermark
  checkMemoryWatermarks(false, false);
}

TEST_F(GlobalManagerTest, testGetUpdatedAvailableMemorySize_CriticalWatermark_Without_Soft)
{
  setUpWatermarks();

  EXPECT_CALL(MockTimerTaskExecutor::instance(), scheduleNow(_))
      .WillOnce(Invoke(this, &GlobalManagerTest::captureTask));
  EXPECT_CALL(MockTimerTaskExecutor::instance(), cancel(_))
      .WillOnce(Invoke(this, &GlobalManagerTest::abandonTask));
  EXPECT_CALL(*MockMonitor::instance(), getUpdatedAvailableMemorySize())
      // [01] soft watermark reached
      .WillOnce(Return(_softFreeRssWatermark))       // getUpdatedAvailableMemorySize
      // [02] below critical watermark
      .WillOnce(Return(_criticalFreeRssWatermark-1)) // getUpdatedAvailableMemorySize
      .WillOnce(Return(_criticalFreeRssWatermark-1)) // performTask
      // [03] critical watermark reached
      .WillOnce(Return(_criticalFreeRssWatermark))   // getUpdatedAvailableMemorySize
      .WillOnce(Return(_criticalFreeRssWatermark))   // performTask
      // [04] above soft watermark
      .WillOnce(Return(_softFreeRssWatermark+1))     // getUpdatedAvailableMemorySize
      .WillOnce(Return(_softFreeRssWatermark+1))     // performTask; cancel
      // [05] above soft watermark
      .WillOnce(Return(_softFreeRssWatermark+1));    // getUpdatedAvailableMemorySize

  // [01] soft watermark reached
  checkMemoryWatermarks(false, false);
  // [02] below critical watermark
  checkMemoryWatermarks(true, true);
  // [03] reached critical watermark
  checkMemoryWatermarks(true, true);
  // [04] above soft watermark
  checkMemoryWatermarks(false, false);
  // [05] above soft watermark
  checkMemoryWatermarks(false, false);
}
}
}
