#pragma once

struct MonitorStateNotification
{
  static volatile long& empty()
  {
    static volatile long _empty(0);
    return _empty;
  }
  static volatile long& watching()
  {
    static volatile long _watching(0);
    return _watching;
  }

  MonitorStateNotification()
  {
    reset();
    watch();
  }
  ~MonitorStateNotification() { unwatch(); }

  static void reset()
  {
    __sync_synchronize();
    empty() = 0;
  }

  static void mark()
  {
    if (watching())
    {
      __sync_bool_compare_and_swap(&empty(), 0, 1);
    }
  }

  void watch() { __sync_bool_compare_and_swap(&watching(), 0, 1); }

  void unwatch() { __sync_bool_compare_and_swap(&watching(), 1, 0); }
};

