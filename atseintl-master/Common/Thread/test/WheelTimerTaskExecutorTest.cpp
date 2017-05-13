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

#include "Common/Thread/WheelTimerTaskExecutor.h"
#include "Common/Thread/test/MockTimerTask.h"

#include <atomic>

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"

namespace tse
{
using namespace ::testing;

class WheelTimerTaskExecutorTest : public Test
{
protected:
  WheelTimerTaskExecutor executor{true};

  void createThread() { executor.createThread(); }
};

TEST_F(WheelTimerTaskExecutorTest, testConstructorDestructor)
{
}

TEST_F(WheelTimerTaskExecutorTest, testThreadStartStop)
{
  createThread();
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleNowCancelOne_oneShot)
{
  std::atomic<int> times(0);
  MockTimerTask task(TimerTask::ONE_SHOT, std::chrono::milliseconds(5));
  EXPECT_CALL(task, onCancel()).Times(1);
  EXPECT_CALL(task, run()).Times(Exactly(1)).WillRepeatedly(Invoke([&](){ ++times; }));

  createThread();

  executor.scheduleNow(task);

  while (times < 1)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleAfterCancelMany_oneShot)
{
  std::atomic<int> times(0);
  MockTimerTask task1(TimerTask::ONE_SHOT, std::chrono::milliseconds(5));
  MockTimerTask task2(TimerTask::ONE_SHOT, std::chrono::milliseconds(5));
  MockTimerTask task3(TimerTask::ONE_SHOT, std::chrono::milliseconds(5));
  EXPECT_CALL(task1, onCancel()).Times(1);
  EXPECT_CALL(task2, onCancel()).Times(1);
  EXPECT_CALL(task3, onCancel()).Times(1);
  EXPECT_CALL(task1, run()).Times(Exactly(1)).WillRepeatedly(Invoke([&](){ ++times; }));
  EXPECT_CALL(task2, run()).Times(Exactly(1)).WillRepeatedly(Invoke([&](){ ++times; }));
  EXPECT_CALL(task3, run()).Times(Exactly(1)).WillRepeatedly(Invoke([&](){ ++times; }));

  createThread();

  executor.scheduleAfter(std::chrono::milliseconds(10), task1);
  executor.scheduleAfter(std::chrono::milliseconds(20), task2);
  executor.scheduleAfter(std::chrono::milliseconds(30), task3);

  while (times < 3)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task1);
  executor.cancelAndWait(task2);
  executor.cancelAndWait(task3);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleAtCancelMany_oneShot)
{
  std::atomic<int> times(0);
  MockTimerTask task1(TimerTask::ONE_SHOT, std::chrono::milliseconds(5));
  MockTimerTask task2(TimerTask::ONE_SHOT, std::chrono::milliseconds(5));
  MockTimerTask task3(TimerTask::ONE_SHOT, std::chrono::milliseconds(5));
  EXPECT_CALL(task1, onCancel()).Times(1);
  EXPECT_CALL(task2, onCancel()).Times(1);
  EXPECT_CALL(task3, onCancel()).Times(1);
  EXPECT_CALL(task1, run()).Times(Exactly(1)).WillRepeatedly(Invoke([&](){ ++times; }));
  EXPECT_CALL(task2, run()).Times(Exactly(1)).WillRepeatedly(Invoke([&](){ ++times; }));
  EXPECT_CALL(task3, run()).Times(Exactly(1)).WillRepeatedly(Invoke([&](){ ++times; }));

  createThread();

  executor.scheduleAt(TimerTask::Clock::now() + std::chrono::milliseconds(40), task1);
  executor.scheduleAt(TimerTask::Clock::now() + std::chrono::milliseconds(50), task2);
  executor.scheduleAt(TimerTask::Clock::now() + std::chrono::milliseconds(60), task3);

  while (times < 3)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task1);
  executor.cancelAndWait(task2);
  executor.cancelAndWait(task3);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleNowCancelOne_repeating)
{
  std::atomic<int> times(0);
  MockTimerTask task(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(0));
  EXPECT_CALL(task, onCancel()).Times(1);
  EXPECT_CALL(task, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times; }));

  createThread();

  executor.scheduleNow(task);

  while (times < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleNowCancelMany_repeating)
{
  std::atomic<int> times1(0), times2(0), times3(0);
  MockTimerTask task1(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(0));
  MockTimerTask task2(
      TimerTask::REPEATING, std::chrono::milliseconds(20), std::chrono::milliseconds(0));
  MockTimerTask task3(
      TimerTask::REPEATING, std::chrono::milliseconds(30), std::chrono::milliseconds(0));
  EXPECT_CALL(task1, onCancel()).Times(1);
  EXPECT_CALL(task2, onCancel()).Times(1);
  EXPECT_CALL(task3, onCancel()).Times(1);
  EXPECT_CALL(task1, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times1; }));
  EXPECT_CALL(task2, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times2; }));
  EXPECT_CALL(task3, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times3; }));

  createThread();

  executor.scheduleNow(task1);
  executor.scheduleNow(task2);
  executor.scheduleNow(task3);

  while (times1 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  while (times2 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  while (times3 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task1);
  executor.cancelAndWait(task2);
  executor.cancelAndWait(task3);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleAfterCancelMany_repeating)
{
  std::atomic<int> times1(0), times2(0), times3(0);
  MockTimerTask task1(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(0));
  MockTimerTask task2(
      TimerTask::REPEATING, std::chrono::milliseconds(20), std::chrono::milliseconds(0));
  MockTimerTask task3(
      TimerTask::REPEATING, std::chrono::milliseconds(30), std::chrono::milliseconds(0));
  EXPECT_CALL(task1, onCancel()).Times(1);
  EXPECT_CALL(task2, onCancel()).Times(1);
  EXPECT_CALL(task3, onCancel()).Times(1);
  EXPECT_CALL(task1, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times1; }));
  EXPECT_CALL(task2, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times2; }));
  EXPECT_CALL(task3, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times3; }));

  createThread();

  executor.scheduleAfter(std::chrono::milliseconds(400), task1);
  executor.scheduleAfter(std::chrono::milliseconds(500), task2);
  executor.scheduleAfter(std::chrono::milliseconds(600), task3);

  while (times1 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  while (times2 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  while (times3 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task1);
  executor.cancelAndWait(task2);
  executor.cancelAndWait(task3);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleCancelOne_WaitInsinde_repeating)
{
  std::atomic<int> times(0);
  MockTimerTask task(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(0));
  EXPECT_CALL(task, onCancel()).Times(1);
  EXPECT_CALL(task, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&]()
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ++times;
  }));

  createThread();

  executor.scheduleNow(task);

  while (times < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleCancelMany_WaitInsinde_repeating)
{
  std::atomic<int> times1(0), times2(0), times3(0);
  MockTimerTask task1(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(0));
  MockTimerTask task2(
      TimerTask::REPEATING, std::chrono::milliseconds(20), std::chrono::milliseconds(0));
  MockTimerTask task3(
      TimerTask::REPEATING, std::chrono::milliseconds(30), std::chrono::milliseconds(0));
  EXPECT_CALL(task1, onCancel()).Times(1);
  EXPECT_CALL(task2, onCancel()).Times(1);
  EXPECT_CALL(task3, onCancel()).Times(1);
  EXPECT_CALL(task1, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&]()
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ++times1;
  }));
  EXPECT_CALL(task2, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&]()
  {
    std::this_thread::sleep_for(std::chrono::microseconds(15));
    ++times2;
  }));
  EXPECT_CALL(task3, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&]()
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ++times3;
  }));

  createThread();

  executor.scheduleNow(task1);
  executor.scheduleNow(task2);
  executor.scheduleNow(task3);

  while (times1 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  while (times2 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  while (times3 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task1);
  executor.cancelAndWait(task2);
  executor.cancelAndWait(task3);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleCancelOne_ManyTimes_repeating)
{
  std::atomic<int> times(0);
  MockTimerTask task(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(0));
  EXPECT_CALL(task, onCancel()).Times(1);
  EXPECT_CALL(task, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times; }));

  createThread();

  executor.scheduleNow(task);

  while (times < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancel(task);
  executor.cancelAndWait(task);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleCancelOne_ManyTimes2_repeating)
{
  std::atomic<int> times(0);
  MockTimerTask task(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(0));
  EXPECT_CALL(task, onCancel()).Times(1);
  EXPECT_CALL(task, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times; }));

  createThread();

  executor.scheduleNow(task);

  while (times < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task);
  executor.cancelAndWait(task);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleCancelMany_Mixed_repeating)
{
  std::atomic<int> times1(0), times2(0), times3(0);
  MockTimerTask task1(
      TimerTask::REPEATING, std::chrono::milliseconds(50), std::chrono::milliseconds(0));
  MockTimerTask task2(
      TimerTask::REPEATING, std::chrono::milliseconds(20), std::chrono::milliseconds(0));
  MockTimerTask task3(
      TimerTask::REPEATING, std::chrono::milliseconds(30), std::chrono::milliseconds(0));
  EXPECT_CALL(task1, onCancel()).Times(1);
  EXPECT_CALL(task2, onCancel()).Times(1);
  EXPECT_CALL(task3, onCancel()).Times(1);
  EXPECT_CALL(task1, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times1; }));
  EXPECT_CALL(task2, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times2; }));
  EXPECT_CALL(task3, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times3; }));

  createThread();

  executor.scheduleNow(task1);
  executor.scheduleNow(task2);

  while (times2 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.scheduleNow(task3);
  executor.cancelAndWait(task2);

  while (times3 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task3);

  while (times1 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task1);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleCancelMany_Granularity_repeating)
{
  std::atomic<int> times1(0), times2(0), times3(0);
  MockTimerTask task1(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(10));
  MockTimerTask task2(
      TimerTask::REPEATING, std::chrono::milliseconds(20), std::chrono::milliseconds(10));
  MockTimerTask task3(
      TimerTask::REPEATING, std::chrono::milliseconds(30), std::chrono::milliseconds(10));
  EXPECT_CALL(task1, onCancel()).Times(1);
  EXPECT_CALL(task2, onCancel()).Times(1);
  EXPECT_CALL(task3, onCancel()).Times(1);
  EXPECT_CALL(task1, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times1; }));
  EXPECT_CALL(task2, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times2; }));
  EXPECT_CALL(task3, run()).Times(AtLeast(10)).WillRepeatedly(Invoke([&](){ ++times3; }));

  createThread();

  executor.scheduleNow(task1);
  executor.scheduleNow(task2);
  executor.scheduleNow(task3);

  while (times1 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  while (times2 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  while (times3 < 10)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task1);
  executor.cancelAndWait(task2);
  executor.cancelAndWait(task3);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleCancelSelf_repeating)
{
  std::atomic<int> times(0);
  std::atomic<bool> cancelled(false);
  MockTimerTask task(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(0));
  EXPECT_CALL(task, onCancel()).Times(1).WillRepeatedly(Invoke([&]() { cancelled = true; }));
  EXPECT_CALL(task, run()).Times(Exactly(10)).WillRepeatedly(Invoke([&]()
  {
    ++times;
    if (times == 10)
      executor.cancel(task);
  }));

  createThread();

  executor.scheduleNow(task);

  while (!cancelled)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task);
}

TEST_F(WheelTimerTaskExecutorTest, testScheduleCancelSelfMany_repeating)
{
  std::atomic<int> times1(0), times2(0), times3(0);
  std::atomic<int> cancelled(0);
  MockTimerTask task1(
      TimerTask::REPEATING, std::chrono::milliseconds(10), std::chrono::milliseconds(0));
  MockTimerTask task2(
      TimerTask::REPEATING, std::chrono::milliseconds(20), std::chrono::milliseconds(0));
  MockTimerTask task3(
      TimerTask::REPEATING, std::chrono::milliseconds(30), std::chrono::milliseconds(0));
  EXPECT_CALL(task1, onCancel()).Times(1).WillRepeatedly(Invoke([&]() { ++cancelled; }));
  EXPECT_CALL(task2, onCancel()).Times(1).WillRepeatedly(Invoke([&]() { ++cancelled; }));
  EXPECT_CALL(task3, onCancel()).Times(1).WillRepeatedly(Invoke([&]() { ++cancelled; }));
  EXPECT_CALL(task1, run()).Times(Exactly(10)).WillRepeatedly(Invoke([&]()
  {
    ++times1;
    if (times1 == 10)
      executor.cancel(task1);
  }));
  EXPECT_CALL(task2, run()).Times(Exactly(10)).WillRepeatedly(Invoke([&]()
  {
    ++times2;
    if (times2 == 10)
      executor.cancel(task2);
  }));
  EXPECT_CALL(task3, run()).Times(Exactly(10)).WillRepeatedly(Invoke([&]()
  {
    ++times3;
    if (times3 == 10)
      executor.cancel(task3);
  }));

  createThread();

  executor.scheduleNow(task1);
  executor.scheduleNow(task2);
  executor.scheduleNow(task3);

  while (cancelled < 3)
    std::this_thread::sleep_for(std::chrono::microseconds(100));

  executor.cancelAndWait(task1);
  executor.cancelAndWait(task2);
  executor.cancelAndWait(task3);
}
}
