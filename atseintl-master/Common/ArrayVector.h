#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include "Util/BranchPrediction.h"

// a vector-like container which is designed to be used for some
// special cases, in which it is more efficient.
//
// This container contains an additional restriction compared to
// vector that it can only be used to store 'Plain Old Data' (POD)
// types -- mainly pointers and integers, though also structs,
// and any data type which doesn't have a non-trivial constructor
// or destructor.
//
// Swapping containers of this type is also an expensive O(n) operation,
// unlike vector.
//
// The semantics and time complexity of operations are otherwise
// equivalent to vector.
//
// This container makes use of a 'small vector optimization' -- it
// contains an array of size 'ArraySize' which it stores its data in.
// It will only make dynamic memory allocations once the size of
// the data exceeds ArraySize. This makes it particularly optimal
// to use for small arrays.
//
// As a result of the use of the small vector optimization, objects
// of this type can be large, depending on the type of the object
// and the ArraySize specified.

namespace tse
{

template <typename T, size_t ArraySize = 8>
class ArrayVector
{
public:
  typedef T value_type;
  typedef T* pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T* iterator;
  typedef const T* const_iterator;

  ArrayVector() : _beg(_array), _end(_beg), _endStorage(_array + ArraySize) {}

  explicit ArrayVector(size_type n) : _beg(_array), _end(_beg), _endStorage(_array + ArraySize)
  {
    resize(n);
  }

  ArrayVector(size_type n, const T& t) : _beg(_array), _end(_beg), _endStorage(_array + ArraySize)
  {
    if (UNLIKELY(n > ArraySize))
    {
      _beg = new T[n];
      _endStorage = _beg + n;
    }

    _end = _beg + n;
    std::fill(_beg, _end, t);
  }

  ArrayVector(const ArrayVector& v) : _beg(_array), _end(_beg), _endStorage(_array + ArraySize)
  {
    reserve(v.size());
    std::copy(v.begin(), v.end(), _beg);
    _end = _beg + v.size();
  }

  ArrayVector& operator=(const ArrayVector& v)
  {
    if (this != &v)
    {
      reserve(v.size());
      std::copy(v.begin(), v.end(), _beg);
      _end = _beg + v.size();
    }

    return *this;
  }

  ~ArrayVector()
  {
    if (UNLIKELY(_beg != _array))
    {
      delete[] _beg;
    }
  }

  void resize(size_type n)
  {
    reserve(n);
    if (n > size())
    {
      std::fill(_end, _beg + n, T());
    }

    _end = _beg + n;
  }

  void reserve(size_type n)
  {
    if (UNLIKELY(n > capacity()))
    {
      const size_type newBufSize = capacity() * 2 > n ? capacity() * 2 : n;
      T* newBuf = new T[newBufSize];
      std::copy(_beg, _end, newBuf);
      destroy_buffer();
      _end = newBuf + size();
      _beg = newBuf;
      _endStorage = newBuf + newBufSize;
    }
  }

  reference front() { return *_beg; }
  reference back() { return *(_end - 1); }

  const_reference front() const { return *_beg; }
  const_reference back() const { return *(_end - 1); }

  iterator begin() { return _beg; }
  iterator end() { return _end; }

  const_iterator begin() const { return _beg; }
  const_iterator end() const { return _end; }

  void push_back(const T& t)
  {
    reserve(size() + 1);
    *_end = t;
    ++_end;
  }

  void pop_back() { --_end; }

  size_type size() const { return _end - _beg; }
  bool empty() const { return _beg == _end; }
  size_type capacity() const { return _endStorage - _beg; }

  void clear()
  {
    destroy_buffer();
    _beg = _array;
    _end = _beg;
    _endStorage = _array + ArraySize;
  }

  void swap(ArrayVector& v)
  {
    const ArrayVector tmp = v;
    v = *this;
    *this = tmp;
  }

  iterator insert(iterator pos, const T& t)
  {
    const size_t index = pos - _beg;
    insert(pos, &t, (&t) + 1);
    return &_beg[index];
  }

  template <typename FwdIterator>
  void insert(iterator pos, FwdIterator beg, FwdIterator end)
  {
    const size_t dist = std::distance(beg, end);
    const size_t index = pos - _beg;
    const size_t shuffleSize = size() - index;
    resize(size() + dist);

    for (size_t n = 0; n != shuffleSize; ++n)
    {
      *(_end - n - 1) = *(_end - n - 1 - dist);
    }

    std::copy(beg, end, _beg + index);
  }

  T& operator[](size_type index) { return _beg[index]; }

  const T& operator[](size_type index) const { return _beg[index]; }

  // not implemented
  // iterator erase(iterator pos);
  // not implemented
  // iterator erase(iterator first, iterator last);

private:
  void destroy_buffer()
  {
    if (_beg != _array)
    {
      delete[] _beg;
    }
  }

  T _array[ArraySize];
  T* _beg;
  T* _end;
  T* _endStorage;
};

template <typename T, size_t S>
bool operator==(const ArrayVector<T, S>& a, const ArrayVector<T, S>& b)
{
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

// not implemented
// template<typename T, size_t S>
// bool operator<(const ArrayVector<T,S>& a,
//               const ArrayVector<T,S>& b)

} // end namespace tse

