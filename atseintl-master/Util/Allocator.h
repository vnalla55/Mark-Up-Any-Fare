// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#ifndef UTIL_ALLOCATOR_H
#define UTIL_ALLOCATOR_H

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>


#include "Util/Utility.h"

namespace tse
{
template <typename Allocator>
class AllocatorAdapter : public Allocator
{
public:
  typedef uint32_t size_type;
  typedef int32_t difference_type;

  typedef typename Allocator::pointer pointer;

  // Reallocates memory.
  // Returns null if unimplemented.
  pointer
  reallocate(const pointer /*memory*/, const size_type /*oldSize*/, const size_type /*newSize*/)
  {
    return nullptr;
  }
};

template <typename T>
class MallocAllocator
{
public:
  typedef T value_type;
  typedef       value_type* pointer;
  typedef const value_type* const_pointer;
  typedef       value_type& reference;
  typedef const value_type& const_reference;
  typedef uint32_t size_type;
  typedef int32_t difference_type;

  static const size_type unit_size = sizeof(value_type);

  template <typename U>
  struct rebind { typedef MallocAllocator<U> other; };

  MallocAllocator() noexcept {}
  MallocAllocator(const MallocAllocator& o) noexcept {}
  template <typename U> MallocAllocator(const MallocAllocator<U>& o) noexcept {}
  ~MallocAllocator() noexcept {}

        pointer address(      reference x) const noexcept { return &x; }
  const_pointer address(const_reference x) const noexcept { return &x; }

  size_type max_size() const noexcept
  {
    return size_type(std::min<size_t>(std::numeric_limits<size_type>::max(),
                                      std::numeric_limits<size_t>::max() / unit_size));
  }

  pointer allocate(size_type size, const void* /*hint*/ = nullptr)
  {
    return (pointer)malloc(size * unit_size);
  }

  void deallocate(pointer memory, size_type /*size*/) { free(memory); }

  pointer reallocate(pointer memory, size_type oldSize, size_type newSize)
  {
    assert(oldSize > 0 && newSize > 0);
    return (pointer)realloc(memory, newSize * sizeof(value_type));
  }

  template <typename... Args>
  void construct(pointer memory, Args&&... args) noexcept(noexcept(T()))
  {
    ::new ((void*)memory) value_type(std::forward<Args>(args)...);
  }

  void destroy(pointer memory) noexcept(noexcept(T().~T())) { memory->~T(); }
};

template <typename T, bool IsPod = IsPod<T>::value>
struct GetAllocator
{
  typedef AllocatorAdapter<std::allocator<T> > type;
};

template <typename T>
struct GetAllocator<T, true>
{
  typedef MallocAllocator<T> type;
};
}

#endif
