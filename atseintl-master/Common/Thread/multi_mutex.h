#pragma once

#include <cassert>

#include <pthread.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
/// multi_mutex class is a representation of an array of mutexes
/// this is similar to TSEReadWriteLock, but is customizable and
/// boost compatible;
///
/// muliti_mutex supports only unique and shared locking
///
/// the mutex type must be boost compatible, i.e. provide methods
/// lock(), unlock(), lock_shared() and unlock_shared()
///
/// template MAX parameter denotes the maximum number of mutexes
/// inside multi_mutex; the idea behind it is that the actual number
/// of mutexes may be configurable and we want to avoid dynamic
/// allocation of mutex array
///
/// Note that multi_mutex works only with mutex with size less than 64 bytes
/// so it will not work with boost::shared_mutex and similar
/// If one provides bigger mutex, the code will not compile
//-----------------------------------------------------------------------------
template <typename mutex, uint32_t MAX>
class multi_mutex
{
  struct inner_mutex
  {
    mutex _mutex;
    char _pad[64 - sizeof(_mutex)];
  };

  typedef uint32_t id_type;

public:
  multi_mutex(const id_type count = MAX) : _count(count) { assert(_count < max_count); }

  //-----------------------------------------------------------------------------
  /// Sets every inner mutex in unique lock state
  //-----------------------------------------------------------------------------
  void lock(void)
  {
    for (id_type i = 0; i != _count; ++i)
      _m[i]._mutex.lock();
  }

  //-----------------------------------------------------------------------------
  /// Unlocks every inner mutex
  //-----------------------------------------------------------------------------
  void unlock(void)
  {
    for (id_type i = 0; i != _count; ++i)
      _m[i]._mutex.unlock();
  }

  //-----------------------------------------------------------------------------
  /// Locks appropriate inner mutex in shared lock state
  //-----------------------------------------------------------------------------
  void lock_shared(void) { _m[get_index()]._mutex.lock_shared(); }

  //-----------------------------------------------------------------------------
  /// Unlocks appropriate inner mutex shared lock state
  //-----------------------------------------------------------------------------
  void unlock_shared(void) { _m[get_index()]._mutex.unlock_shared(); }

private:
  static const id_type max_count = MAX;
  inner_mutex _m[max_count];
  const id_type _count;

  id_type get_index() const { return static_cast<id_type>(pthread_self() % _count); }
};

