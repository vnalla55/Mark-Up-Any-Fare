// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <vector>
#include <mutex>
#include <thread>

namespace tse
{
class ThreadFactory
{
public:
  template <typename Func>
  void spawn(Func func)
  {
    std::unique_lock<std::mutex> lock(mutex);
    size_t i = threads.size();
    threads.emplace_back(std::thread([this, i, func]()
    {
      try
      {
        func();
      }
      catch (...)
      {
        std::unique_lock<std::mutex> lock(mutex);
        didExcept = true;
      }

      {
        std::unique_lock<std::mutex> lock(mutex);
        threads[i].second = true;
        queue.notify_all();
      }
    }), false);
  }

  void join()
  {
    const auto till = std::chrono::system_clock::now() + std::chrono::seconds(10);

    bool result;
    {
      std::unique_lock<std::mutex> lock(mutex);
      result = queue.wait_until(lock, till, [this]()
      {
        return std::all_of(threads.begin(), threads.end(), [](std::pair<std::thread, bool>& p)
                                                           { return p.second; });
      });
    }

    ASSERT_TRUE(result && "Some of the threads are blocked.");

    for (auto& p : threads)
      p.first.join();

    ASSERT_FALSE(didExcept && "Some of the threads did throw exceptions.");

    threads.clear();
  }

private:
  std::mutex mutex;
  std::condition_variable queue;
  std::vector<std::pair<std::thread, bool>> threads;
  bool didExcept = false;
};
}
