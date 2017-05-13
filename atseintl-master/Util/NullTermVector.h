// ----------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
//  Description:
//
//    Null terminated vector with static capacity.
//    It's designed to reduce number of allocations and memory usage
//    when max number of elements is very small.
//
// ----------------------------------------------------------------

#pragma once

#include "Util/BranchPrediction.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace tse
{
template <class T>
struct NullValuePolicy
{
  static T getNull() { return T(); }
  static bool isNull(const T& v) { return v == getNull(); }
};

template <class T>
struct EmptyContainerPolicy : public NullValuePolicy<T>
{
  static bool isNull(const T& cont) { return cont.empty(); }
};

template <class T, size_t Capacity, template <class> class PolicyTmpl = NullValuePolicy>
class NullTermVector : private std::array<T, Capacity>
{
  typedef std::array<T, Capacity> Base;
  typedef PolicyTmpl<T> Policy;

public:
  typedef typename Base::value_type value_type;
  typedef typename Base::size_type size_type;
  typedef typename Base::difference_type difference_type;
  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::reverse_iterator reverse_iterator;
  typedef typename Base::const_reverse_iterator const_reverse_iterator;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;

  NullTermVector() { clear(); }
  NullTermVector(const NullTermVector& v) : Base(v) {}
  explicit NullTermVector(const_reference v) { fill(v); }
  NullTermVector(size_type num, const_reference v) { assign(num, v); }
  explicit NullTermVector(const std::vector<T>& v) { assign(v); }

  template <class Iterator>
  NullTermVector(Iterator b, Iterator e)
  {
    assign(b, e);
  }

  iterator begin() { return Base::begin(); }
  const_iterator begin() const { return Base::begin(); }
  const_iterator cbegin() const { return begin(); }

  iterator end() { return std::find_if(Base::begin(), Base::end(), Policy::isNull); }
  const_iterator end() const { return std::find_if(Base::begin(), Base::end(), Policy::isNull); }
  const_iterator cend() const { return end(); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  const_reverse_iterator crbegin() const { return rbegin(); }

  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  const_reverse_iterator crend() const { return rend(); }

  using Base::operator=;
  NullTermVector& operator=(const std::vector<T>& v)
  {
    assign(v);
    return *this;
  }
  using Base::at;
  using Base::operator[];

  reference front() { return *begin(); }
  const_reference front() const { return *begin(); }
  reference back() { return *(end() - 1); }
  const_reference back() const { return *(end() - 1); }

  bool empty() const { return Policy::isNull(front()); }
  size_type size() const { return end() - begin(); }
  size_type max_size() const { return Base::max_size(); }
  size_type capacity() const { return Capacity; }

  void fill(const_reference v) { std::fill(Base::begin(), Base::end(), v); }
  void clear() { fill(Policy::getNull()); }
  void assign(size_type num, const_reference v) { assignValImpl(num, v); }
  void assign(const std::vector<T>& v) { assign(v.begin(), v.end()); }

  void resize(size_type size, const_reference v)
  {
    if (UNLIKELY(size > Capacity))
      throw std::out_of_range("tse::NullTermVector::resize");

    const size_type prevEnd = end() - begin();
    if (prevEnd < size)
      std::fill_n(begin() + prevEnd, size - prevEnd, v);

    if (size != Capacity)
      (*this)[size] = Policy::getNull();
  }

  void shrink(size_type size)
  {
    if (UNLIKELY(size > Capacity))
      throw std::out_of_range("tse::NullTermVector::shrink");

    const size_type prevEnd = end() - begin();
    if (UNLIKELY(prevEnd < size))
      throw std::out_of_range("tse::NullTermVector::shrink");

    if (size != Capacity)
      (*this)[size] = Policy::getNull();
  }

  void push_back(const_reference v)
  {
    const auto end = this->end();
    if (UNLIKELY(end - begin() == Capacity))
      throw std::out_of_range("tse::NullTermVector::push_back");

    *end = v;
  }

  void push_back(value_type&& v)
  {
    const auto end = this->end();
    if (UNLIKELY(end - begin() == Capacity))
      throw std::out_of_range("tse::NullTermVector::push_back");

    *end = std::move(v);
  }

  template <class Iterator>
  void assign(Iterator b, Iterator e)
  {
    typedef typename std::is_integral<Iterator>::type IsInt;
    assignRangeImpl(b, e, IsInt());
  }

  // comparison operators
  friend bool operator==(const NullTermVector& l, const NullTermVector& r)
  {
    const_iterator lb = l.begin();
    const_iterator le = l.end();
    return le - lb == r.size() && std::equal(lb, le, r.begin());
  }

  friend bool operator<(const NullTermVector& l, NullTermVector& r)
  {
    return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
  }

  friend bool operator!=(const NullTermVector& l, NullTermVector& r) { return !(l == r); }
  friend bool operator>(const NullTermVector& l, NullTermVector& r) { return r < l; }
  friend bool operator<=(const NullTermVector& l, NullTermVector& r) { return !(l > r); }
  friend bool operator>=(const NullTermVector& l, NullTermVector& r) { return r <= l; }

  // cast operators
  operator std::vector<T>() const { return std::vector<T>(begin(), end()); }

private:
  template <class Iterator>
  void assignRangeImpl(Iterator b, Iterator e, std::false_type)
  {
    if (UNLIKELY(std::distance(b, e) > Capacity))
      throw std::out_of_range("tse::NullTermVector::assignRangeImpl");
    const iterator it = std::copy(b, e, Base::begin());
    std::fill(it, Base::end(), Policy::getNull());
  }

  template <class Iterator>
  void assignRangeImpl(Iterator b, Iterator e, std::true_type)
  {
    assignValImpl(b, e);
  }

  void assignValImpl(size_type num, const_reference v)
  {
    if (UNLIKELY(num > Capacity))
      throw std::out_of_range("tse::NullTermVector::assignValImpl");
    std::fill_n(Base::begin(), num, v);
    std::fill(Base::begin() + num, Base::end(), Policy::getNull());
  }
};
}

