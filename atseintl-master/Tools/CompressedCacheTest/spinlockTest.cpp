//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#include <boost/noncopyable.hpp>

#include <atomic>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

extern int64_t getCurrentMillis();

namespace
{

class spinlock
{
private:
  typedef enum {LOCKED, UNLOCKED} LockState;
  std::atomic<LockState> _state;
public:
  spinlock()
    : _state(UNLOCKED)
  {
  }
  void lock()
  {
    while (_state.exchange(LOCKED) == LOCKED)
    {
    }
  }
  void unlock()
  {
    _state.store(UNLOCKED);
  }
};

class spinguard : boost::noncopyable
{
  spinlock &_spinlock;
 public:
  spinguard(spinlock &spinlock)
    : _spinlock(spinlock)
  {
    _spinlock.lock();
  }
  ~spinguard()
  {
    _spinlock.unlock();
  }
};

spinlock _spinlock;
std::mutex _mutex;

namespace
{
bool initializeDummy()
{
  static int i(0);
  std::cout << "initialization1:" << i << std::endl;
  std::ostringstream os;
  for (int i = 0; i < 1000000; ++i)
  {
    os << i;
  }
  std::string str(os.str());
  os << str;
  std::cout << "initialization2:" << i++ << std::endl;
  return true;
}
}

struct Worker
{
  Worker(int i,
         std::vector<int>& results)
    : _i(i)
    , _results(results)
  {
  }
  void run ()
  {
    spinguard guard(_spinlock);
    std::cout << __FUNCTION__ << ' ' << _i << std::endl;
  }
  void run1 ()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    std::cout  << __FUNCTION__ << ' ' << _i << std::endl;
    static bool initialized(initializeDummy());
  }
private:
  int _i;
  std::vector<int>& _results;
};
}

void testSpinlock()
{
  std::cout << __FUNCTION__ << std::endl;
  const size_t numThreads(100);
  std::thread threads[numThreads];
  std::vector<int> results(numThreads, -1);
  uint64_t start(getCurrentMillis());
  for (size_t i = 0; i < numThreads; ++i)
  {
    Worker worker(i, results);
    threads[i] = std::thread(std::bind(&Worker::run, worker));
  }
  for (size_t i = 0; i < numThreads; ++i)
  {
    threads[i].join();
  }
  for (size_t i = 0; i < numThreads; ++i)
  {
    Worker worker(i, results);
    threads[i] = std::thread(std::bind(&Worker::run1, worker));
  }
  for (size_t i = 0; i < numThreads; ++i)
  {
    threads[i].join();
  }
}
