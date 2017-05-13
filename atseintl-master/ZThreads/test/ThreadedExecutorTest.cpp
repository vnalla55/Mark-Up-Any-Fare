#include "ZThreads/zthread/ThreadedExecutor.h"
#include "ZThreads/zthread/Exceptions.h"

#include "ThreadTestUtil.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class ThreadedExecutorTest:
  public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ThreadedExecutorTest);
    CPPUNIT_TEST(testThreadedExecutorExecute1);
    CPPUNIT_TEST(testThreadedExecutorExecute2);
    CPPUNIT_TEST(testThreadedExecutorExecute3);
    CPPUNIT_TEST(testThreadedExecutorExecute4);
    CPPUNIT_TEST(testThreadedExecutorExecute5);
    CPPUNIT_TEST(testThreadExecutorTimedWait);
    CPPUNIT_TEST(testThreadExecutorInterrupt);
    CPPUNIT_TEST(testThreadExecutorCancel);
  CPPUNIT_TEST_SUITE_END();

  void testThreadedExecutorExecute1()
  {
    ZThread::ThreadedExecutor threadedExecutor;
    unsigned long value = 0;
    ZThread::Task task(new FibTask(42, value));
    threadedExecutor.execute(task);
    threadedExecutor.wait();
    CPPUNIT_ASSERT_EQUAL(267914296UL, value);
  }

  void testThreadedExecutorExecute2()
  {
    ThreadValue<unsigned long> value;
    {
      ZThread::Task task(new FibTask2(45, value));
      ZThread::ThreadedExecutor threadedExecutor;
      threadedExecutor.execute(task);
    }

    CPPUNIT_ASSERT_EQUAL(1134903170UL, value.get());
  }

  void testThreadedExecutorExecute3()
  {
    const unsigned N = 5;
    unsigned long values[N] = {0};
    ZThread::ThreadedExecutor threadedExecutor;
    for (unsigned i = 0; i < N; ++i)
    {
      ZThread::Task task(new FibTask(30 + i, values[i]));
      threadedExecutor.execute(task);
    }

    threadedExecutor.wait();

    unsigned long results[N] = {832040, 1346269, 2178309, 3524578, 5702887};
    for (unsigned i = 0; i < N; ++i)
    {
      CPPUNIT_ASSERT_EQUAL(results[i], values[i]);
    }
  }

  void testThreadedExecutorExecute4()
  {
    const unsigned N = 5;
    ThreadValue<unsigned long> values[N];
    {
      ZThread::ThreadedExecutor threadedExecutor;
      for (unsigned i = 0; i < N; ++i)
      {
        ZThread::Task task(new FibTask2(34 - i, values[i]));
        threadedExecutor.execute(task);
      }
    }

    unsigned long results[N] = {5702887, 3524578, 2178309, 1346269, 832040};
    for (unsigned i = 0; i < N; ++i)
    {
      CPPUNIT_ASSERT_EQUAL(results[i], values[i].get());
    }
  }

  void testThreadedExecutorExecute5()
  {
    const int N = 3000;

    boost::atomic<int> counter(0);
    ZThread::ThreadedExecutor threadedExecutor;
    for (int i = 0; i < N; ++i)
    {
      ZThread::Task task(new IncrementTask(counter));
      threadedExecutor.execute(task);
    }

    threadedExecutor.wait();
    CPPUNIT_ASSERT_EQUAL(N, counter.load(boost::memory_order_acquire));
  }

  void testThreadExecutorTimedWait()
  {
    ThreadValue<int> value;
    ZThread::ThreadedExecutor threadedExecutor;
    threadedExecutor.execute(ZThread::Task(new WaitTask(value)));
    bool result = threadedExecutor.wait(25);
    CPPUNIT_ASSERT_EQUAL(false, result);

    value.set(42);
    result = threadedExecutor.wait(25);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testThreadExecutorInterrupt()
  {
    ThreadValue<int> start;
    ThreadValue<int> value;
    bool interrupted = false;
    ZThread::ThreadedExecutor threadedExecutor;
    threadedExecutor.execute(ZThread::Task(new InterruptTask(start, value, interrupted)));
    (void)start.get();
    threadedExecutor.interrupt();
    threadedExecutor.wait();
    CPPUNIT_ASSERT_EQUAL(true, interrupted);
  }

  void testThreadExecutorCancel()
  {
    ZThread::ThreadedExecutor threadedExecutor;
    CPPUNIT_ASSERT_EQUAL(false, threadedExecutor.isCanceled());
    threadedExecutor.cancel();
    CPPUNIT_ASSERT_EQUAL(true, threadedExecutor.isCanceled());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ThreadedExecutorTest);

} // namespace tse
