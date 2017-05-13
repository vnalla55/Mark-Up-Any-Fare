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
#include "Common/Memory/test/MockMonitor.h"
#include "Common/Thread/test/MockTimerTask.h"
#include "Common/Thread/test/MockTimerTaskExecutor.h"

#include <chrono>
#include <thread>
#include <gtest/gtest.h>

namespace tse
{
namespace Memory
{

using namespace ::testing;

class MonitorTest : public ::testing::Test
{
public:
  void mySchedule(TimerTask& /*task*/) { ++_callShedule; }

  void myCancel(TimerTask& /*task*/) { ++_callCancel; }

  void expectTimerTaskExecutor()
  {
    EXPECT_CALL(MockTimerTaskExecutor::instance(), scheduleNow(_))
        .WillRepeatedly(Invoke(this, &MonitorTest::mySchedule));
    EXPECT_CALL(MockTimerTaskExecutor::instance(), cancelAndWait(_))
        .WillRepeatedly(Invoke(this, &MonitorTest::myCancel));
  }

  void dontExpectTimerTaskExecutor()
  {
    EXPECT_CALL(MockTimerTaskExecutor::instance(), scheduleNow(_)).Times(Exactly(0));
    EXPECT_CALL(MockTimerTaskExecutor::instance(), cancelAndWait(_)).Times(Exactly(0));
  }

protected:
  void SetUp()
  {
    _monitorUpdatePeriod = PERIOD;
    _monitorUpdatePeriodGranularity = PERIOD/5;
    MockTimerTaskExecutor::setInstance(new MockTimerTaskExecutor);
  }

  void TearDown()
  {
    MockTimerTaskExecutor::destroyInstance();
  }

  const uint32_t PERIOD {100};
  int8_t _callShedule {0};
  int8_t _callCancel {0};
};

TEST_F(MonitorTest, testDestroyInstance_CallWithoutCreatingFirst)
{
  dontExpectTimerTaskExecutor();

  ASSERT_EQ(nullptr, MockMonitor::instance());

  MockMonitor::destroyInstance();
  ASSERT_EQ(nullptr, MockMonitor::instance());
}

TEST_F(MonitorTest, testInstance)
{
  _monitorUpdatePeriod = 0;
  dontExpectTimerTaskExecutor();

  MockMonitor::createInstance();
  ASSERT_NE(nullptr, MockMonitor::instance());

  MockMonitor::destroyInstance();
  ASSERT_EQ(nullptr, MockMonitor::instance());
}

TEST_F(MonitorTest, testCreateInstance_DoNotSheduleTask)
{
  _monitorUpdatePeriod = 0;
  dontExpectTimerTaskExecutor();

  MockMonitor::createInstance();
  MockMonitor::destroyInstance();
  ASSERT_EQ(0, _callShedule);
  ASSERT_EQ(0, _callCancel);

}

TEST_F(MonitorTest, testCreateInstance_SheduleTask)
{
  expectTimerTaskExecutor();

  MockMonitor::createInstance();
  MockMonitor::destroyInstance();
  ASSERT_EQ(1, _callShedule);
  ASSERT_EQ(1, _callCancel);
}

TEST_F(MonitorTest, testCreateInstance_CallTwiceWithoutDestroyingPreviousOne)
{
  MockMonitor* previous = nullptr;
  expectTimerTaskExecutor();

  MockMonitor::createInstance();
  previous = MockMonitor::instance();
  ASSERT_EQ(1, _callShedule);
  ASSERT_EQ(0, _callCancel);

  MockMonitor::createInstance();
  ASSERT_EQ(2, _callShedule);
  ASSERT_EQ(1, _callCancel);
  ASSERT_NE(previous, MockMonitor::instance());

  MockMonitor::destroyInstance();
  ASSERT_EQ(2, _callShedule);
  ASSERT_EQ(2, _callCancel);
}

TEST_F(MonitorTest, testCreateInstance_CallTwiceSheduleThanDoNotSheduleTask)
{
  expectTimerTaskExecutor();

  MockMonitor::createInstance();
  MockMonitor::destroyInstance();
  ASSERT_EQ(1, _callShedule);
  ASSERT_EQ(1, _callCancel);

  _monitorUpdatePeriod = 0;
  MockMonitor::createInstance();
  MockMonitor::destroyInstance();
  ASSERT_EQ(1, _callShedule);
  ASSERT_EQ(1, _callCancel);
}

TEST_F(MonitorTest, testCreateInstance_CallTwiceDoNotSheduleThanSheduleTask)
{
  _monitorUpdatePeriod = 0;
  expectTimerTaskExecutor();

  MockMonitor::createInstance();
  ASSERT_EQ(0, _callShedule);
  ASSERT_EQ(0, _callCancel);

  _monitorUpdatePeriod = PERIOD;
  MockMonitor::createInstance();
  MockMonitor::destroyInstance();
  ASSERT_EQ(1, _callShedule);
  ASSERT_EQ(1, _callCancel);
}

}
}
