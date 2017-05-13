#pragma once

#include "Common/Thread/TSEFastMutex.h"
#include "Common/Thread/TSEReadWriteLock.h"

class TSEGuard
{
  TSEFastMutex& _m;

public:
  explicit TSEGuard(TSEFastMutex& m) : _m(m) { _m.acquire(); }

  ~TSEGuard() { _m.release(); }
};

template<typename Lock = TSEReadWriteLock>
class TSEReadGuard
{
  Lock& _m;
  int _index;

public:
  explicit TSEReadGuard(Lock& m) : _m(m) { _index = _m.acquireRead(); }

  ~TSEReadGuard() { _m.releaseRead(_index); }

  void release() { _m.releaseRead(_index); }

  void acquire() { _index = _m.acquireRead(); }
};

template <typename Lock = TSEReadWriteLock>
class TSEWriteGuard
{
  Lock& _m;

public:
  explicit TSEWriteGuard(Lock& m) : _m(m) { _m.acquireWrite(); }

  ~TSEWriteGuard() { _m.releaseWrite(); }
};

