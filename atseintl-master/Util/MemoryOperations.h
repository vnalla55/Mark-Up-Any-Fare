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

#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

#include <assert.h>
#include <string.h>

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#include "Util/ContainerTraits.h"
#include "Util/Iterator.h"
#include "Util/Utility.h"

namespace tse
{
template <typename Allocator>
class GenericMemoryOperations : public Allocator
{
public:
  typedef Allocator allocator_type;
  typedef typename allocator_type::value_type value_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;

  // Same as raw_storage_iterator, but bi-directional-like
  class ConstructIterator : public std::iterator<std::bidirectional_iterator_tag,
    value_type, difference_type, pointer, reference>
  {
  public:
    explicit ConstructIterator(pointer ptr) noexcept : _pointer(ptr) {}

    ConstructIterator& operator*() { return *this; }
    ConstructIterator& operator=(value_type value)
    {
      new (static_cast<void*>(&*_pointer)) value_type(value);
      return *this;
    }

    ConstructIterator& operator++() { ++_pointer; return *this; }
    ConstructIterator operator++(int)
    {
      ConstructIterator temp = *this;
      ++_pointer;
      return temp;
    }

    ConstructIterator& operator--() { --_pointer; return *this; }
    ConstructIterator operator--(int)
    {
      ConstructIterator temp = *this;
      --_pointer;
      return temp;
    }

  private:
    pointer _pointer;
  };

  GenericMemoryOperations() {}
  GenericMemoryOperations(const allocator_type& allocator) : allocator_type(allocator) {}

  using allocator_type::destroy;

  // destroy [memory, memory + size).
  void destroy(const pointer memory, const size_type size)
  {
    for (size_type i = 0; i < size; ++i)
      this->destroy(memory + i);
  }

  // fill [memory, memory + size) with value.
  // [memory, memory + existing) is already constructed.
  void fill(const pointer memory, const size_type size, const size_type existing)
  {
    fill(memory, size, existing, value_type());
  }

  void
  fill(const pointer memory, const size_type size, const size_type existing, const_reference value)
  {
    assert(existing <= size);

    std::fill_n(memory, existing, value);
    std::uninitialized_fill_n(memory + existing, size - existing, value);
  }

  // copy [source, source + size) to [memory, memory + size).
  // [memory, memory + existing) is already constructed.
  void copy(const pointer memory,
            const size_type size,
            const size_type existing,
            const const_pointer source)
  {
    assert(existing <= size);

    std::copy_n(source, existing, memory);
    std::uninitialized_copy_n(source + existing, size - existing, memory + existing);
  }

  // Same as above, but source is an iterator.
  template <typename Iterator>
  typename boost::enable_if_c<IteratorTraits<Iterator>::is_pointer, void>::type
  copyRange(const pointer memory,
            const size_type size,
            const size_type existing,
            const Iterator& first)
  {
    copy(memory, size, existing, &*first);
  }

  template <typename Iterator>
  typename boost::enable_if_c<!IteratorTraits<Iterator>::is_pointer, void>::type
  copyRange(const pointer memory,
            const size_type size,
            const size_type existing,
            const Iterator& first)
  {
    assert(existing <= size);

    Iterator it = first;

    for (size_type i = 0; i < existing; ++i)
      memory[i] = *it++;
    for (size_type i = existing; i < size; ++i)
      this->construct(memory + i, *it++);
  }

  // move [source, source + size) to [memory, memory + size).
  // [memory, memory + existing) is already constructed.
  void
  move(const pointer memory, const size_type size, const size_type existing, const pointer source)
  {
    assert(existing <= size);

    std::copy_n(std::make_move_iterator(source), existing, memory);
    std::uninitialized_copy_n(
        std::make_move_iterator(source + existing), size - existing, memory + existing);
  }

  // Same as above, but source is an iterator.
  template <typename Iterator>
  typename boost::enable_if_c<IteratorTraits<Iterator>::is_pointer, void>::type
  moveRange(const pointer memory,
            const size_type size,
            const size_type existing,
            const Iterator& first)
  {
    move(memory, size, existing, &*first);
  }

  template <typename Iterator>
  typename boost::enable_if_c<!IteratorTraits<Iterator>::is_pointer, void>::type
  moveRange(const pointer memory,
            const size_type size,
            const size_type existing,
            const Iterator& first)
  {
    assert(existing <= size);

    copyRange(memory, size, existing, std::make_move_iterator(first));
  }

  // move [memory, memory + size) to [memory + difference, memory + difference + size).
  // [memory, memory + size) is already constructed.
  void moveRight(const pointer memory, const size_type size, const size_type difference)
  {
    // Make sure there will be no unconstructed hole left.
    //TODO It may be faster to handle this hole differently.
    size_type constructed = size;
    if (difference > constructed)
    {
      fill(memory + size, difference - constructed, 0);
      constructed = difference;
    }

    const size_type constructLimit = constructed - difference;

    std::move_backward(memory + constructLimit,
                       memory + size,
                       ConstructIterator(memory + size + difference));
    std::move_backward(memory,
                       memory + constructLimit,
                       memory + constructLimit + difference);
  }

  // move [memory, memory + size) to [memory - difference, memory - difference + size).
  // [memory - difference, memory + size) is already constructed.
  void moveLeft(const pointer memory, const size_type size, const size_type difference)
  {
    std::copy_n(std::make_move_iterator(memory), size, memory - difference);
  }

  static bool compareEqual(const pointer memory1, const pointer memory2, const size_type size)
  {
    return std::equal(memory1, memory1 + size, memory2);
  }

  static bool compareLess(const pointer memory1,
                          const size_type size1,
                          const pointer memory2,
                          const size_type size2)
  {
    return std::lexicographical_compare(memory1, memory1 + size1, memory2, memory2 + size2);
  }
};

template <typename Allocator>
class PodMemoryOperations : public GenericMemoryOperations<Allocator>
{
  typedef GenericMemoryOperations<Allocator> Base;

public:
  typedef typename Base::value_type value_type;
  typedef typename Base::pointer pointer;
  typedef typename Base::const_pointer const_pointer;
  typedef typename Base::const_reference const_reference;
  typedef typename Base::size_type size_type;
  typedef typename Base::difference_type difference_type;

  static const size_type unit_size = sizeof(value_type);

  using Base::Base;

  void destroy(const pointer, const size_type) {}

  void fill(const pointer memory, const size_type size, const size_type)
  {
    memset(memory, 0, size * unit_size);
  }

  void fill(const pointer memory, const size_type size, const size_type, const_reference value)
  {
    Base::fill(memory, size, size, value);
  }

  void copy(const pointer memory, const size_type size, const size_type, const const_pointer source)
  {
    memcpy(memory, source, size * unit_size);
  }

  template <typename Iterator>
  typename boost::enable_if_c<IteratorTraits<Iterator>::is_pointer, void>::type
  copyRange(const pointer memory,
            const size_type size,
            const size_type existing,
            const Iterator& first)
  {
    copy(memory, size, existing, &*first);
  }

  template <typename Iterator>
  typename boost::enable_if_c<!IteratorTraits<Iterator>::is_pointer, void>::type
  copyRange(const pointer memory, const size_type size, const size_type, const Iterator& first)
  {
    Base::copyRange(memory, size, size, first);
  }

  void
  move(const pointer memory, const size_type size, const size_type existing, const pointer source)
  {
    copy(memory, size, existing, source);
  }

  template <typename Iterator>
  void moveRange(const pointer memory,
                 const size_type size,
                 const size_type existing,
                 const Iterator& first)
  {
    copyRange(memory, size, existing, first);
  }

  void moveRight(const pointer memory, const size_type size, const size_type difference)
  {
    memmove(memory + difference, memory, size * unit_size);
  }

  void moveLeft(const pointer memory, const size_type size, const size_type difference)
  {
    memmove(memory - difference, memory, size * unit_size);
  }
};

template <typename Allocator,
          bool isContainer = ContainerTraits<typename Allocator::value_type>::is_container,
          bool isPod = IsPod<typename Allocator::value_type>::value>
struct GetMemoryOperations
{
  typedef GenericMemoryOperations<Allocator> type;
};

template <typename Allocator>
struct GetMemoryOperations<Allocator, false, true>
{
  typedef PodMemoryOperations<Allocator> type;
};
}

