#pragma once

#include <memory>

namespace ZThread
{

template <typename T>
class CountedPtr
{
  template<class Y> friend class CountedPtr;

public:
  CountedPtr()
  {
  }

  explicit
  CountedPtr(T* t)
    : _ptr(t)
  {
  }

  CountedPtr(const CountedPtr& other)
    : _ptr(other._ptr)
  {
  }

  template <class Y>
  CountedPtr(const CountedPtr<Y>& other)
    : _ptr(other._ptr)
  {
  }

  void operator=(const CountedPtr& other)
  {
    _ptr = other._ptr;
  }

  template <class Y>
  void operator=(const CountedPtr<Y>& other)
  {
    _ptr = other._ptr;
  }

  void reset(T* t = 0)
  {
    _ptr.reset(t);
  }

  T& operator*() { return *_ptr; }
  const T& operator*() const { return *_ptr; }

  T* operator->() const { return _ptr.get(); }
  T* get() const { return _ptr.get(); }

  bool operator!() const { return !_ptr; }

  static void unspecified_bool(T*) {}
  typedef void (*unspecified_bool_type)(T*);
  operator unspecified_bool_type() const { return _ptr.get() ? unspecified_bool : 0; }

  bool operator==(const CountedPtr& other) const
  {
    return _ptr == other._ptr;
  }

  bool operator!=(const CountedPtr& other) const
  {
    return _ptr != other._ptr;
  }

private:
  std::shared_ptr<T> _ptr;
};

} // namespace ZThread

