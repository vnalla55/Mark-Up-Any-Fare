#pragma once

#include "Util/Time.h"

#include <boost/atomic.hpp>

//TODO it's unnecesserily slow (seq_cst operations), needs optimization.

//-----------------------------------------------------------------------------
/// ticket_mutex class is an alternative mutex for situations where we have
/// as many writers as readers
//-----------------------------------------------------------------------------
class ticket_mutex
{
  typedef uint16_t underlying_type;
  typedef boost::atomic<underlying_type> value_type;

public:
  ticket_mutex() : _current(0), _waiting(0), _readers(0), _upgrade(0) {}

  void lock(void)
  {
    while (true)
    {
      const underlying_type ticket = get_ticket();

      wait_for_ticket(ticket);

      if (_upgrade.load() == 0)
      {
        wait_for_readers_to_leave();
        return;
      }
      _current.fetch_add(1);
    }
  }

  void unlock(void) { _current.fetch_add(1); }

  void lock_shared(void)
  {
    const underlying_type ticket = get_ticket();

    wait_for_ticket(ticket);

    _readers.fetch_add(1);
    _current.fetch_add(1);
  }

  void unlock_shared(void) { _readers.fetch_sub(1); }

  bool try_lock(void)
  {
    const underlying_type ticket = get_ticket();

    wait_for_ticket(ticket);

    if (_upgrade.load() == 0)
    {
      wait_for_readers_to_leave();
      return true;
    }
    _current.fetch_add(1);
    return false;
  }

  bool try_lock_shared(void)
  {
    // TODO - same as lock_shared
    const underlying_type ticket = get_ticket();

    wait_for_ticket(ticket);

    _readers.fetch_add(1);
    _current.fetch_add(1);
    return true;
  }

  void lock_upgrade(void)
  {
    while (true)
    {
      const underlying_type ticket = get_ticket();

      wait_for_ticket(ticket);

      if (_upgrade.load() == 0)
      {
        _upgrade.store(1);
        _current.fetch_add(1);
        return;
      }
      _current.fetch_add(1);
    }
  }

  void unlock_upgrade(void) { _upgrade.store(0); }

  void unlock_upgrade_and_lock(void)
  {
    const underlying_type ticket = get_ticket();

    wait_for_ticket(ticket);

    wait_for_readers_to_leave();
    _upgrade.store(0);
  }

  void unlock_and_lock_upgrade(void)
  {
    _upgrade.store(1);
    _current.fetch_add(1);
  }

  void unlock_and_lock_shared(void)
  {
    _readers.fetch_add(1);
    _current.fetch_add(1);
  }

  void unlock_upgrade_and_lock_shared(void)
  {
    _readers.fetch_add(1);
    _upgrade.store(0);
  }

private:
  inline const underlying_type get_ticket(void) { return _waiting.fetch_add(1); }

  inline void wait_for_ticket(const underlying_type& ticket)
  {
    while (ticket != _current.load())
      tse::Time::sleepFor(std::chrono::microseconds(wait_time));
  }

  inline void wait_for_readers_to_leave(void)
  {
    while (_readers.load() != 0)
      tse::Time::sleepFor(std::chrono::microseconds(wait_time));
  }

  value_type _current;
  value_type _waiting;
  value_type _readers;
  value_type _upgrade;

  static const underlying_type wait_time = 5; ///> Sleep duration in microsecons between mutex spins
};

