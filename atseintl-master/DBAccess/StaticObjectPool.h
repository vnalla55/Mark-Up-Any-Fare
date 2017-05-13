//----------------------------------------------------------------------------
//
//  File:           StaticObjectPool.h
//  Description:    TSE interface to Boost singleton pool
//
//  Copyright Sabre 2009
//
//  The copyright to the computer program(s) herein is the property of Sabre.
//  The program(s) may be used and/or copied only with the written permission
//  of Sabre or in accordance with the terms and conditions stipulated in the
//  agreement/contract under which the program(s) have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include <boost/pool/pool.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <map>
#include <memory>

namespace tse
{

struct StaticPoolStatistics
{
public:
  StaticPoolStatistics(size_t itemSize, size_t* num, size_t* max, size_t* mem)
    : _itemSize(itemSize), _numActivePtr(num), _maxActivePtr(max), _memoryPtr(mem)
  {
  }
  size_t numActive() { return (_numActivePtr != nullptr ? *_numActivePtr : 0); }
  size_t maxActive() { return (_maxActivePtr != nullptr ? *_maxActivePtr : 0); }
  size_t memory() { return (_memoryPtr != nullptr ? *_memoryPtr : 0); }
  size_t itemSize() { return _itemSize; }

protected:
  size_t _itemSize;
  size_t* _numActivePtr;
  size_t* _maxActivePtr;
  size_t* _memoryPtr;
};

class StaticPoolRegistry
{
public:
  static StaticPoolRegistry& instance()
  {
    if (_instance == nullptr)
    {
      static boost::mutex _mutex;
      boost::lock_guard<boost::mutex> g(_mutex);
      if (_instance == nullptr)
        _instance = new StaticPoolRegistry();
    }
    return *_instance;
  }
  void addEntry(std::string name, StaticPoolStatistics* pool)
  {
    static boost::mutex _mutex;
    boost::lock_guard<boost::mutex> g(_mutex);
    _registry[name] = pool;
  }
  template <typename T>
  void forEach(T& t)
  {
    for_each(_registry.begin(), _registry.end(), t);
  }
  StaticPoolStatistics* getStatistics(std::string id)
  {
    std::map<std::string, StaticPoolStatistics*>::iterator item = _registry.find(id);
    return (item == _registry.end()) ? nullptr : item->second;
  }

protected:
  static StaticPoolRegistry* _instance;
  std::map<std::string, StaticPoolStatistics*> _registry;
};

template <class T>
struct StaticPoolAllocator
{
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  static size_t _memory;
  static StaticPoolStatistics _statistics;
  static char* malloc(const size_type bytes)
  {
    if (_memory == 0)
    {
      StaticPoolRegistry& registry = StaticPoolRegistry::instance();
      registry.addEntry(typeid(T).name(), &_statistics);
    }
    _memory += bytes;
    return new (std::nothrow) char[bytes];
  }
  static void free(char* const block) { delete[] block; }
  static size_t memory() { return _memory; }
};

template <class T>
class StaticObjectPool
{
private:
  typedef boost::singleton_pool<void, sizeof(T), StaticPoolAllocator<T> > pool;

  static size_t _numActive;
  static size_t _maxActive;

  class Deleter
  {
  public:
    void operator()(T* object) const { StaticObjectPool::put(object); }
  };
  friend class Deleter;
  friend struct StaticPoolAllocator<T>;

public:
  typedef T value_type;

  typedef typename std::shared_ptr<T> pointer_type;

  static size_t numActive() { return _numActive; }

  static size_t maxActive() { return _maxActive; }

  static size_t memory() { return StaticPoolAllocator<T>::memory(); }

  static std::shared_ptr<T> get() { return std::shared_ptr<T>(unsafe_get(), Deleter()); }

  static T* allocate()
  {
    void* const ptr = pool::malloc();
    if (ptr == nullptr)
      throw std::bad_alloc();

    ++_numActive;
    if (_numActive > _maxActive)
      _maxActive = _numActive;
    return static_cast<T*>(ptr);
  }

  static void deallocate(T* ptr)
  {
    --_numActive;
    pool::free(ptr);
  }

  /**
   * A family of methods:
   *  static T* unsafe_create(A0 const &a0, ..., An const &an);
   *
   */
  static T* unsafe_create()
  {
    T* ptr = allocate();
    try { new (ptr) T; }
    catch (...)
    {
      deallocate(ptr);
      throw;
    }
    return static_cast<T*>(ptr);
  }

#define METHOD(__z, N, __d)                                                                        \
  template <BOOST_PP_ENUM_PARAMS(N, typename A)>                                                   \
  static T* unsafe_create(BOOST_PP_ENUM_BINARY_PARAMS(N, A, &a))                                   \
  {                                                                                                \
    T* ptr = allocate();                                                                           \
    try { new (ptr) T(BOOST_PP_ENUM_PARAMS(N, a)); }                                               \
    catch (...)                                                                                    \
    {                                                                                              \
      deallocate(ptr);                                                                             \
      throw;                                                                                       \
    }                                                                                              \
    return static_cast<T*>(ptr);                                                                   \
  }

  BOOST_PP_REPEAT_FROM_TO(1, 10, METHOD, BOOST_PP_EMPTY())
#undef METHOD

  static T* unsafe_get()
  {
    void* const ptr = pool::malloc();
    if (ptr == nullptr)
    {
      throw std::bad_alloc();
    }

    try { new (ptr) T; }
    catch (...)
    {
      pool::free(ptr);
      throw;
    }

    ++_numActive;
    if ((int)_numActive > (int)_maxActive)
      _maxActive = _numActive;
    return static_cast<T*>(ptr);
  }

  // static T* unsafe_create(

  /**
   *  return an object back to the pool.
   *  The object's destructor is called. It does <em>not</em> return
   *  the object's memory back to the operating system or the C++ runtime.
   *  It remains under control of the pool.
   */
  static void put(T* object)
  {
    --_numActive;
    object->~T();
    pool::free(object);
  }

  static void put(std::shared_ptr<T>& ptr) { ptr.reset(); }
  static size_t get_next_size() { return pool::details::PODptr::get_next_size(); }
};

template <class T>
size_t StaticObjectPool<T>::_numActive = 0;
template <class T>
size_t StaticObjectPool<T>::_maxActive = 0;
template <class T>
size_t StaticPoolAllocator<T>::_memory = 0;
template <class T>
StaticPoolStatistics
StaticPoolAllocator<T>::_statistics(sizeof(T),
                                    &StaticObjectPool<T>::_numActive,
                                    &StaticObjectPool<T>::_maxActive,
                                    &StaticPoolAllocator<T>::_memory);

} // namespace tse

