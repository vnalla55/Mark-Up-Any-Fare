#include "ZThreads/zthread/CountingSemaphore.h"

#include "test/include/CppUnitHelperMacros.h"

#include <boost/thread/thread.hpp>
#include <queue>

#include <cstdlib>

namespace tse
{

namespace
{

const int QUEUE_SIZE = 10000;

struct Queue
{
public:
  Queue()
    : _emptyCount(QUEUE_SIZE + 1),
      _mutex(1)
  {
  }

  void push(int value)
  {
    _emptyCount.wait();

    _mutex.wait();
    _queue.push(value);
    _mutex.post();

    _fillCount.post();
  }

  int pop()
  {
    _fillCount.wait();

    _mutex.wait();
    int value = _queue.front();
    _queue.pop();
    _mutex.post();

    _emptyCount.post();
    return value;
  }

private:
  ZThread::CountingSemaphore _fillCount;
  ZThread::CountingSemaphore _emptyCount;
  ZThread::CountingSemaphore _mutex;
  std::queue<int> _queue;
};

void consumerThread(Queue& queue, int& result)
{
  for (int i = 0; i < QUEUE_SIZE; ++i)
  {
    result += queue.pop();
  }
}

void signalThread(ZThread::CountingSemaphore& cs)
{
  boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
  cs.post();
}

void waitThread(ZThread::CountingSemaphore& cs)
{
  cs.wait();
}

} // unnamed namespace

class CountingSemaphoreTest:
  public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CountingSemaphoreTest);
    CPPUNIT_TEST(testCountingSemaphoreCtor1);
    CPPUNIT_TEST(testCountingSemaphoreCtor2);
    CPPUNIT_TEST(testCountingSemaphoreQueue);
    CPPUNIT_TEST(testCountingSemaphoreWait);
    CPPUNIT_TEST(testCountingSemaphoreAcquire);
    CPPUNIT_TEST(testCountingSemaphoreTryAcquire);
    CPPUNIT_TEST(testCountingSemaphorePost);
    CPPUNIT_TEST(testCountingSemaphoreRelease);
  CPPUNIT_TEST_SUITE_END();

  void testCountingSemaphoreCtor1()
  {
    ZThread::CountingSemaphore cs1;
    CPPUNIT_ASSERT_EQUAL(0, cs1.count());

    ZThread::CountingSemaphore cs2(10);
    CPPUNIT_ASSERT_EQUAL(10, cs2.count());
  }

  void testCountingSemaphoreCtor2()
  {
    ZThread::CountingSemaphore cs(5);
    boost::thread_group threads;
    for (unsigned i = 0; i < 6; ++i)
    {
      threads.add_thread(new boost::thread(&waitThread, boost::ref(cs)));
    }
    cs.post();
    threads.join_all();
    CPPUNIT_ASSERT_EQUAL(0, cs.count());
  }

  void testCountingSemaphoreWait()
  {
    ZThread::CountingSemaphore cs(10);
    cs.wait();
    CPPUNIT_ASSERT_EQUAL(9, cs.count());
    cs.wait();
    CPPUNIT_ASSERT_EQUAL(8, cs.count());
  }

  void testCountingSemaphoreAcquire()
  {
    ZThread::CountingSemaphore cs(5);
    cs.wait();
    CPPUNIT_ASSERT_EQUAL(4, cs.count());
    cs.wait();
    CPPUNIT_ASSERT_EQUAL(3, cs.count());
  }

  void testCountingSemaphoreTryAcquire()
  {
    ZThread::CountingSemaphore cs;
    bool result = cs.tryAcquire(2);
    CPPUNIT_ASSERT_EQUAL(false, result);

    boost::thread thread(&signalThread, boost::ref(cs));

    result = cs.tryAcquire(1000);
    CPPUNIT_ASSERT_EQUAL(true, result);

    thread.join();
  }

  void testCountingSemaphorePost()
  {
    ZThread::CountingSemaphore cs;
    cs.post();
    CPPUNIT_ASSERT_EQUAL(1, cs.count());
    cs.post();
    CPPUNIT_ASSERT_EQUAL(2, cs.count());
  }

  void testCountingSemaphoreRelease()
  {
    ZThread::CountingSemaphore cs(1);
    cs.release();
    CPPUNIT_ASSERT_EQUAL(2, cs.count());
    cs.release();
    CPPUNIT_ASSERT_EQUAL(3, cs.count());
    cs.release();
    CPPUNIT_ASSERT_EQUAL(4, cs.count());
  }

  void testCountingSemaphoreQueue()
  {
    Queue queue;
    int testResult = 0;
    int threadResult = 0;
    boost::thread thread(&consumerThread, boost::ref(queue), boost::ref(threadResult));
    for (int i = 0; i < QUEUE_SIZE; ++i)
    {
      int value = std::rand() % 100;
      queue.push(value);
      testResult += value;
    }

    thread.join();

    CPPUNIT_ASSERT_EQUAL(testResult, threadResult);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CountingSemaphoreTest);

} // namespace tse
