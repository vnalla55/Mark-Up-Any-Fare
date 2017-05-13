/*
 * TsePoolAllocator.h
 *
 *  Created on: Apr 16, 2012
 *      Author: Grzegorz Cholewiak
 */

#ifndef TSEPOOLALLOCATOR_H_
#define TSEPOOLALLOCATOR_H_

#include <boost/pool/pool.hpp>

#include <map>
#include <memory>
#include "Util/BranchPrediction.h"

namespace tse
{
class PoolFactory
{
public:
  typedef boost::pool<boost::default_user_allocator_malloc_free> Pool;
  PoolFactory() {}
  ~PoolFactory() { purge(); }

  Pool& getPool(size_t s)
  {
    PoolMap::iterator i = _pools.find(s);
    Pool* p = nullptr;
    if (i != _pools.end())
    {
      p = i->second;
    }
    else
    {
      p = new Pool(s);
      _pools.insert(PoolMap::value_type(s, p));
    }
    return *p;
  }
  template <typename T>
  Pool& getPool()
  {
    return getPool(sizeof(T));
  }
  void purge()
  {
    for (auto& elem : _pools)
    {
      delete elem.second;
    }
    _pools.clear();
  }

protected:
  typedef std::map<size_t, Pool*> PoolMap;
  PoolMap _pools;

private:
  PoolFactory(const PoolFactory&);
  PoolFactory& operator=(const PoolFactory&);
};

// This allocator does not have all the properties required by the C++ standard library.
// It does not provide a default constructor (see 20.1.5 in the standard). It doesn't fulfill the
// additional
// requirement that all instances of a given allocator type should be interchangeable and compare
// equal
// (see 20.1.5.4). Only instances using the same boost::pool are interchangeable.
// It is known to work with GNU implementation std::map, std::tr1::unordered_map and std::vector.
// Most probably
// works with all std and std::tr1 containers but wasn't tested. I don't recommend using it for
// anything else
// than maps, sets and lists. For std::vector and std::deque you will get a better performance with
// std::alloc.
// It doesn't work with std::string.

template <class T>
class TsePoolAllocator;
template <>
class TsePoolAllocator<void>
{
public:
  typedef void* pointer;
  typedef const void* const_pointer;
  typedef void value_type;
  template <class U>
  struct rebind
  {
    typedef TsePoolAllocator<U> other;
  };
};

template <typename T>
class TsePoolAllocator : public std::allocator<T>
{
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T value_type;
  template <class U>
  struct rebind
  {
    typedef TsePoolAllocator<U> other;
  };
  template <class U>
  friend class TsePoolAllocator;

  TsePoolAllocator(const TsePoolAllocator& a) throw() : _poolFactory(a._poolFactory), _pool(a._pool)
  {
  }

  TsePoolAllocator(PoolFactory& p) : _poolFactory(p), _pool(p.getPool(sizeof(T))) {}

  template <class U>
  TsePoolAllocator(const TsePoolAllocator<U>& a) throw()
    : _poolFactory(a._poolFactory), _pool(_poolFactory.getPool(sizeof(T)))
  {
  }

  ~TsePoolAllocator() throw() {}

  pointer allocate(size_type n, TsePoolAllocator<void>::const_pointer hint = nullptr)
  {
    return (T*)((n == 1) ? _pool.malloc() : _pool.ordered_malloc(n));
  }
  void deallocate(pointer p, size_type n)
  {
    if (LIKELY(n == 1))
      _pool.free(p);
    else
      _pool.ordered_free(p, n);
  }

  void construct(pointer p, const T& val) { ::new ((void*)p) T(val); }

  void destroy(pointer p) { p->~T(); }

  bool operator==(const TsePoolAllocator& rhs) const { return &this->_pool == &rhs._pool; }
  bool operator!=(const TsePoolAllocator& rhs) const { return &this->_pool != &rhs._pool; }

protected:
  PoolFactory& _poolFactory;
  PoolFactory::Pool& _pool;

private:
  TsePoolAllocator() throw();
  TsePoolAllocator& operator=(const TsePoolAllocator&);
};
}
#endif /* TSEPOOLALLOCATOR_H_ */
