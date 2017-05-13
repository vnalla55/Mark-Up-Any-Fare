#pragma once

#include "Util/BranchPrediction.h"

#include <stack>

#include <boost/pool/object_pool.hpp>

namespace tse
{
template <typename T>
class TseObjectPool
{
public:
  TseObjectPool() {}
  TseObjectPool(const TseObjectPool&) = delete;
  TseObjectPool& operator=(const TseObjectPool&) = delete;

  template <typename... Args>
  T* construct(Args&&... args)
  {
    if (_reusable.empty())
      return _pool.construct(std::forward<Args>(args)...);

    T* p = _reusable.top();
    _reusable.pop();

    p->~T();

    try
    {
      p = new (p) T(std::forward<Args>(args)...);
    }
    catch (...)
    {
      _pool.free(p);
      throw;
    }

    return p;
  }

  void destroy(T* p)
  {
    if (LIKELY(p))
      _reusable.push(p);
  }

private:
  boost::object_pool<T> _pool;
  std::stack<T*> _reusable;
};
}
