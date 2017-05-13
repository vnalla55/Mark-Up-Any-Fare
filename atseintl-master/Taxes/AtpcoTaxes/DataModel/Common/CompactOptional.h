// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <limits>

// class CompactOptional represent optional objects (like boost::optional, except that
// the no-value state is encoded as one of the states in T. It can be used out-of the box
// for scalar types. For user defined types you have to specialize template CompactOptionalTraits
// with two funcitons: singularValue and isSingularValue in order to teach the framework
// about the special value that will represent the no-value state.
namespace tax
{

template <typename T>
struct CompactOptionalTraits
{
  static
  T singularValue()
  {
    static_assert(std::numeric_limits<T>::is_specialized,
                  "To use CompactOptional with this type T specialize "
                  "class CompactOptionalTraits");
    return std::numeric_limits<T>::max();
  }

  static
  bool isSingularValue(const T& v)
  {
    return v == singularValue();
  }
};

template <typename T>
class CompactOptional
{
public:
  CompactOptional() : _val(CompactOptionalTraits<T>::singularValue()) {}
  explicit CompactOptional(T v) : _val(v) {}
  CompactOptional& operator=(T v) { _val = v; return *this; }
  T value() const { return _val; }
  bool has_value() const { return !CompactOptionalTraits<T>::isSingularValue(_val); }

  friend bool operator==(CompactOptional lhs, CompactOptional rhs) { return lhs._val == rhs._val; }
  friend bool operator<(CompactOptional lhs, CompactOptional rhs) { return lhs._val < rhs._val; }

private:
  T _val;
};

} // namespace tax

