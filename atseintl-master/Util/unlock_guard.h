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
#pragma once

namespace tse
{
template <typename Mutex>
class unlock_guard
{
public:
  explicit unlock_guard(Mutex& mutex) : _mutex(mutex)
  {
    _mutex.unlock();
  }

  ~unlock_guard()
  {
    _mutex.lock();
  }

  unlock_guard(const unlock_guard&) = delete;
  void operator=(const unlock_guard&) = delete;

private:
  Mutex& _mutex;
};
}
