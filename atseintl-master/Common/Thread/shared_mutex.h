#ifndef SHARED_MUTEX_H
#define SHARED_MUTEX_H

#ifndef NO_ATOMIC_SUPPORT

// Use mutex type based on lock free atomic operations

#include "Common/Thread/atomic.h"
#include "Util/Time.h"

namespace boostex
{
//-----------------------------------------------------------------------------
/// shared_mutex class is boost::shared_mutex replacement built on top
/// of std::atomic interface (std::atomic or GCC atomic operations extension
/// implementation). It was developed to mitigate pthread mutex size (312B) in
/// Itinerary Cache.
///
/// \sa boostex::mutex
/// \sa boostex::atomic
//-----------------------------------------------------------------------------
class shared_mutex
{
  typedef uint16_t underlying_type;
  typedef atomic<underlying_type> value_type;

public:
  //-----------------------------------------------------------------------------
  /// Default constructor
  //-----------------------------------------------------------------------------
  shared_mutex() : _value(empty_mask) {}

  //-----------------------------------------------------------------------------
  /// sets mutex in unique lock state
  ///
  /// \post _value == write_mask
  //-----------------------------------------------------------------------------
  void lock(void)
  {
    exchange_mask(empty_mask, write_mask);
    wait_for_readers_to_leave();
  }

  //-----------------------------------------------------------------------------
  /// Unlocks unique lock state
  ///
  /// \pre _value = write_mask
  /// \post _value == 0
  //-----------------------------------------------------------------------------
  void unlock(void) { exchange_values(write_mask, empty_mask); }

  //-----------------------------------------------------------------------------
  /// Locks mutex in shared lock state
  //-----------------------------------------------------------------------------
  void lock_shared(void) { exchange_mask(empty_mask, read_increment, write_mask); }

  //-----------------------------------------------------------------------------
  /// Unlocks shared lock state
  //-----------------------------------------------------------------------------
  void unlock_shared(void) { _value.fetch_sub(read_increment); }

  //-----------------------------------------------------------------------------
  /// Tries to lock mutex in unique lock state
  ///
  /// \pre (_value & exclusive_mask) == 0
  /// \post _value == write_mask if no other thread has exclusive ownership
  /// \return true if lock acquired
  //-----------------------------------------------------------------------------
  bool try_lock(void)
  {
    if (!try_exchange_mask(empty_mask, write_mask))
      return false;
    wait_for_readers_to_leave();
    return true;
  }

  //-----------------------------------------------------------------------------
  /// Tries to lock mutex in upgrade lock state
  ///
  /// \return true if lock acquired
  //-----------------------------------------------------------------------------
  bool try_lock_upgrade(void) { return try_exchange_mask(empty_mask, upgrade_mask, write_mask); }

  //-----------------------------------------------------------------------------
  /// Tries to lock mutex in shared lock state
  ///
  /// \return true if lock acquired
  //-----------------------------------------------------------------------------
  bool try_lock_shared(void) { return try_exchange_mask(empty_mask, read_increment, write_mask); }

  //-----------------------------------------------------------------------------
  /// Locks mutex in upgrade lock state
  ///
  /// \post (_value & exclusive_mask) == upgrade_mask
  //-----------------------------------------------------------------------------
  void lock_upgrade(void) { exchange_mask(empty_mask, upgrade_mask); }

  //-----------------------------------------------------------------------------
  /// Unlocks upgrade lock state
  ///
  /// \pre (_value & exclusive_mask) == upgrade_mask
  /// \post (_value & exclusive_mask) == 0
  //-----------------------------------------------------------------------------
  void unlock_upgrade(void) { exchange_mask(upgrade_mask, empty_mask); }

  //-----------------------------------------------------------------------------
  /// Changes upgrade lock state to unique lock state
  ///
  /// \pre (_value & exclusive_mask) == upgrade_mask
  /// \post _value == write_mask
  //-----------------------------------------------------------------------------
  void unlock_upgrade_and_lock(void)
  {
    exchange_mask(upgrade_mask, write_mask);
    wait_for_readers_to_leave();
  }

  //-----------------------------------------------------------------------------
  /// Changes unique lock state to upgrade lock state
  ///
  /// \pre _value == write_mask
  /// \post (_value & exclusive_mask) == upgrade_mask
  //-----------------------------------------------------------------------------
  void unlock_and_lock_upgrade(void) { exchange_values(write_mask, upgrade_mask); }

  //-----------------------------------------------------------------------------
  /// Changes unique lock state to shared lock state
  ///
  /// \pre _value == write_mask
  /// \post (_value & exclusive_mask) == 0
  //-----------------------------------------------------------------------------
  void unlock_and_lock_shared(void) { exchange_values(write_mask, read_increment); }

  //-----------------------------------------------------------------------------
  /// Changes upgrade lock state to shared lock state
  ///
  /// \pre (_value & exclusive_mask) == upgrade_mask
  /// \post (_value & exclusive_mask) == 0
  //-----------------------------------------------------------------------------
  void unlock_upgrade_and_lock_shared(void) { exchange_mask(upgrade_mask, read_increment); }

private:
  //-----------------------------------------------------------------------------
  /// Waits till _value is set to exclusive unique locked state
  //-----------------------------------------------------------------------------
  void wait_for_readers_to_leave(void)
  {
    unsigned char counter = 0;
    while (true)
    {
      underlying_type value = _value.load();
      if (value == write_mask)
        break;
      if (++counter == 0)
        tse::Time::sleepFor(std::chrono::microseconds(wait_time));
    }
  }

  //-----------------------------------------------------------------------------
  /// Makes sure that _value is set from expected_value to new_value
  ///
  /// \param expected_value Expected to be replaced
  /// \param new_value New value
  //-----------------------------------------------------------------------------
  void exchange_values(underlying_type expected_value, underlying_type new_value)
  {
    unsigned char counter = 0;
    underlying_type expected = expected_value;
    while (!_value.compare_exchange_strong(expected, new_value))
    {
      expected = expected_value;
      if (++counter == 0)
        tse::Time::sleepFor(std::chrono::microseconds(wait_time));
    }
  }

  //-----------------------------------------------------------------------------
  /// Tries to change exclusive mask from one state to new one.
  ///
  /// \param expected_mask Expected binary mask of exclusive states
  /// \param new_mask Mask that needs to be set
  /// \param mask Masks _value to get exlusive bit mask
  /// \return true if successfull
  //-----------------------------------------------------------------------------
  bool try_exchange_mask(underlying_type expected_mask,
                         underlying_type new_mask,
                         underlying_type mask = exclusive_mask)
  {
    underlying_type old_value = _value.load();
    if ((old_value & mask) == expected_mask)
    {
      underlying_type new_value = ((old_value ^ expected_mask) + new_mask);
      if (_value.compare_exchange_strong(old_value, new_value))
        return true;
    }
    return false;
  }

  //-----------------------------------------------------------------------------
  /// Changes exclusive mask from one state to new one.
  ///
  /// \param expected_mask Expected binary mask of exclusive states
  /// \param new_mask Mask that needs to be set
  /// \param mask Masks _value to get exlusive bit mask
  //-----------------------------------------------------------------------------
  void exchange_mask(underlying_type expected_mask,
                     underlying_type new_mask,
                     underlying_type mask = exclusive_mask)
  {
    unsigned char counter = 0;
    while (!try_exchange_mask(expected_mask, new_mask, mask))
      if (++counter == 0)
        tse::Time::sleepFor(std::chrono::microseconds(wait_time));
  }

  value_type _value; ///> Mutex state
  static const underlying_type empty_mask =
      0; ///> Used to indicate that mutex is not exclusive locked
  static const underlying_type write_mask = 1; ///> Used to indicate that mutex is unique locked
  static const underlying_type upgrade_mask = 2; ///> Used to indicate that mutex is upgrade locked
  static const underlying_type exclusive_mask = 3; ///> Used for exclusive state masking

  static const underlying_type read_increment =
      4; ///> Used to indicate that mutex is in shared state

  static const underlying_type wait_time = 5; ///> Uleep duration in microsecons between mutex spins
};

} // namespace boostex

#else

// In case of no atomic operations support use simple boost::mutex

#warning "Using unoptimized boost::shared_mutex"
#include <boost/thread/upgrade_mutex.hpp>

namespace boostex
{
using boost::shared_mutex;

} // namespace boostex

#endif

#endif // MUTEX_H
