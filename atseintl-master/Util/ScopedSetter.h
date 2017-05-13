#pragma once

#include <utility>

namespace tse
{
template <class T>
class ScopedSetter
{
public:
  template <class U>
  ScopedSetter(T& target, U&& newValue) : _target(target)
  {
    _oldValue = std::move(target);
    _target = std::forward<U>(newValue);
  }

  template <class U>
  ScopedSetter(T& target, std::initializer_list<U> newValue) : _target(target)
  {
    _oldValue = std::move(target);
    _target = newValue;
  }

  ScopedSetter(const ScopedSetter&) = delete;
  ScopedSetter& operator=(const ScopedSetter&) = delete;

  ~ScopedSetter() { _target = std::move(_oldValue); }

private:
  T& _target;
  T _oldValue;
};

}
