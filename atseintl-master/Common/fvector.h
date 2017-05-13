#ifndef FVECTOR_H
#define FVECTOR_H

// ---------------------------------------------------------------------------
//
//  File:         fvector.h
//  Author:       Yanjun Zhang
//  Created:      09/01/2000
//  Description:  a fixed-size array (similiar to c_array described in
//                Stroustrup's 3rd, p. 496)
//
//  Copyright Sabre 2001
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#ifndef IOSTREAM_H
#include <iostream>

#define IOSTREAM_H
#endif

#ifndef VECTOR_H
#include <vector>

#define VECTOR_H
#endif

#if defined CHECK_VECTOR_INDEX_RANGE
#define CHECK_VECTOR_INDEX_RANGEX
#elif defined CHECK_VECTOR_INDEX_RANGE2
#define CHECK_VECTOR_INDEX_RANGEX
#endif

#if defined CHECK_VECTOR_INDEX_RANGEX
#ifndef STDIO_H
#include <stdio.h>

#define STDIO_H
#endif
#ifndef DUMPSTACK_H
#endif
#endif

#define STD_OSTREAM std::ostream

typedef unsigned short uint16;

template <class T, uint16 max>
class fvector
{
  friend STD_OSTREAM& operator<<(STD_OSTREAM& os, const fvector<T, max>& x)
  {
    // for (size_type i=0; i<x._size; i++) os << x.v[i] << ",";
    // os << "\n";
    return os;
  }

public:
  typedef T value_type;
  typedef T* iterator;
  typedef const T* const_iterator;
  typedef T& reference;
  typedef const T& const_reference;
  typedef uint16 size_type;

  // constructors
  fvector() { _size = 0; } // default

  fvector(const std::vector<T>& vv) // std vector
  {
    size_type i = 0;
    for (; i < max && i < vv.size(); i++)
      v[i] = vv[i];
    _size = i;
  }

  // Can we chg T to T2 here on the next 2 lines?  What blow up later if this no workee?
  // The construction of 1 fvector from another, perhaps. I try. We see.
  template <class T2, size_type max2>
  fvector(const fvector<T2, max2>& vv) // a fixed-size vector of same or different size
  {
    size_type i = 0;
    for (; i < max && i < vv.size(); i++)
      v[i] = vv[i];
    _size = i;
  }

  fvector(const fvector<T, max>& vv) // copy constructor
  {
    _size = vv._size;
    for (size_type i = 0; i < _size; i++)
      v[i] = vv.v[i];
  }

  // assignment operator using a STL vector
  //
  fvector<T, max>& operator=(const std::vector<T>& vv)
  {
    size_type i;
    for (i = 0; i < vv.size() && i < max; i++)
      v[i] = vv[i];
    _size = i;
    return *this;
  }

  // assignment operator using a fvector
  //
  fvector<T, max>& operator=(fvector<T, max>& vv)
  {
    size_type i;
    for (i = 0; i < vv.size() && i < max; i++)
      v[i] = vv[i];
    _size = i;
    return *this;
  }

  // assignment operator using a fvector
  //
  fvector<T, max>& operator=(const fvector<T, max>& vv)
  {
    size_type i;
    for (i = 0; i < vv.size() && i < max; i++)
      v[i] = vv[i];
    _size = i;
    return *this;
  }

  // size and capacity
  size_type size() const { return _size; }
  size_type max_size() const { return max; }
  bool empty() const { return _size < 1; }
  bool full() const { return !(_size < max); } // !
  void clear() { _size = 0; }
  bool resize(size_type num)
  {
    if (num > max) // overflow
      return false;
    _size = num;
    return true;
  }

  // push/pop
  bool push_back(const T& x)
  {
    if (_size >= max) // overflow
      return false;
    v[_size++] = x;
    return true;
  }

  bool pop_back()
  {
    if (_size == 0) // underflow
      return false;
    _size--;
    return true;
  }

#ifdef CHECK_VECTOR_INDEX_RANGEX
  void dumpStack(size_type index) const
  {
    const int32 stackTraceSize = 100;
    char stackTraceArray[stackTraceSize][1024];

    DumpStack::getTrace(stackTraceArray, stackTraceSize);
    int32 i;

    printf("\n***fvector-overflow***,index=%d,size=%d,max=%d", index, _size, max);

    for (i = 2; i < stackTraceSize; ++i)
    {
      int32 len = (int32)strlen(stackTraceArray[i]);
      if (len == 0)
      {
        break;
      }
      printf(",%s", stackTraceArray[i]);
    }
    printf("\n");
    fflush(stdout);
  }
#endif

  // accessors
  reference operator[](size_type i)
  {
#if defined CHECK_VECTOR_INDEX_RANGE
    if (i >= _size)
    {
      dumpStack(i);
    }
#elif defined CHECK_VECTOR_INDEX_RANGE2
    if (i >= max)
    {
      dumpStack(i);
    }
#endif
    return v[i];
  }
  const_reference operator[](size_type i) const
  {
#if defined CHECK_VECTOR_INDEX_RANGE
    if (i >= _size)
    {
      dumpStack(i);
    }
#elif defined CHECK_VECTOR_INDEX_RANGE2
    if (i >= max)
    {
      dumpStack(i);
    }
#endif
    return v[i];
  }

  // iterators
  iterator begin() { return v; }
  const_iterator begin() const { return v; }
  iterator end() { return v + _size; } // not max, as for c_array
  const_iterator end() const { return v + _size; }
  reference front() { return *v; }
  const_reference front() const { return *v; }
  reference back() { return *(v + _size - 1); }
  const_reference back() const { return *(v + _size - 1); }

  // conversion operator: as an ordinary C-array
  // operator T*() { return v; }     // causing ambiguity for operator [] with MSVC++ 6.0
  const T* c_array() const { return v; }

private:
  T v[max];
  size_type _size;
}; // fvector

#endif //  FVECTOR_H
