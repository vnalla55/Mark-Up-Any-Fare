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

#ifndef UTIL_VECTOR_H
#define UTIL_VECTOR_H

#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <assert.h>

#include "Util/Iterator.h"
#include "Util/MemoryOperations.h"
#include "Util/Utility.h"
#include "Util/VectorFwd.h"

#include <boost/utility/enable_if.hpp>

#ifdef _GLIBCXX_DEBUG

// In the case of CXX_DEBUG build, just use GNU std::vector.

namespace tse
{
// Wrap std::vector in order to implement the C++11 features that are implementable in C++98.

template <typename Type, typename Allocator>
class Vector : public std::vector<Type, Allocator>
{
  typedef std::vector<Type, Allocator> Base;

public:
  typedef typename Base::value_type value_type;
  typedef typename Base::pointer pointer;
  typedef typename Base::const_pointer const_pointer;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;
  typedef typename Base::iterator iterator;
  typedef typename Base::const_iterator const_iterator;
  typedef typename Base::reverse_iterator reverse_iterator;
  typedef typename Base::const_reverse_iterator const_reverse_iterator;
  typedef typename Base::size_type size_type;
  typedef typename Base::difference_type difference_type;
  typedef typename Base::allocator_type allocator_type;

  using Base::Base;

  const_iterator cbegin() const { return Base::begin(); }
  const_iterator cend() const { return Base::end(); }
  const_reverse_iterator crbegin() const { return Base::rbegin(); }
  const_reverse_iterator crend() const { return Base::rend(); }

  void shrink_to_fit() { /* no-op */ }

        pointer data()       { return &*Base::begin(); }
  const pointer data() const { return &*Base::begin(); }

  iterator insert(const_iterator position, const_reference value)
  {
    return Base::insert(unConst(position), value);
  }

  template <typename A0, typename A1>
  iterator insert(const_iterator position, A0 a0, A1 a1)
  {
    ptrdiff_t offset = position - cbegin();
    Base::insert(Base::begin() + offset, a0, a1);
    return Base::begin() + offset;
  }

  iterator erase(const_iterator position)
  {
    return Base::erase(unConst(position));
  }

  iterator erase(const_iterator first, const_iterator last)
  {
    return Base::erase(unConst(first), unConst(last));
  }

  iterator insert(const_iterator position, value_type&& value)
  {
    return Base::insert(unConst(position), std::move(value));
  }

  iterator insert(const_iterator position, std::initializer_list<value_type> il)
  {
    ptrdiff_t offset = position - cbegin();
    Base::insert(Base::begin() + offset, il);
    return Base::begin() + offset;
  }

  template <typename... Args>
  iterator emplace(const_iterator position, Args&&... args)
  {
    return Base::emplace(unConst(position), std::forward<Args>(args)...);
  }

private:
  iterator unConst(const_iterator it) { return Base::begin() + (it - cbegin()); }
};
}

#else // _GLIBCXX_DEBUG

namespace tse
{
namespace detail
{
uint32_t
nearestPowerOfTwo(uint32_t number);

uint32_t
nextSize(const uint32_t currentSize, const uint32_t neededSize);
}

template <typename Type, typename Allocator>
class Vector : private GetMemoryOperations<Allocator>::type
{
  typedef typename GetMemoryOperations<Allocator>::type Operations;
  typedef Vector<Type, Allocator> Container;

public:
  typedef Allocator allocator_type;

  typedef typename allocator_type::value_type value_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;

  typedef PointerIterator<value_type, Container> iterator;
  typedef PointerIterator<const value_type, Container> const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  typedef std::vector<Type> std_vector;

  Vector() noexcept { init(); }

  explicit Vector(const allocator_type& allocator) : Operations(allocator) { init(); }

  explicit Vector(size_type size, const allocator_type& allocator = allocator_type())
    : Operations(allocator)
  {
    init();
    assignFill(size);
  }

  template <typename Number>
  explicit Vector(Number size,
                  typename boost::enable_if<boost::is_integral<Number>, const allocator_type&>::type
                      allocator = allocator_type())
    : Operations(allocator)
  {
    init();
    assignFill(checkedSize(size));
  }

  explicit Vector(size_type size,
                  const_reference value,
                  const allocator_type& allocator = allocator_type())
    : Operations(allocator)
  {
    init();
    assignFill(size, value);
  }

  template <typename Number>
  explicit Vector(
      Number size,
      const_reference value,
      typename boost::enable_if_c<boost::is_integral<Number>::value &&
                                      !boost::is_same<Number, value_type>::value,
                                  const allocator_type&>::type allocator = allocator_type())
    : Operations(allocator)
  {
    init();
    assignFill(checkedSize(size), value);
  }

  template <typename Iterator>
  Vector(Iterator first, Iterator last, const allocator_type& allocator = allocator_type())
    : Operations(allocator)
  {
    init();
    assign(first, last);
  }

  Vector(const Vector& other) : Operations(other.get_allocator())
  {
    init();
    assign(other.begin(), other.end());
  }

  Vector(const Vector& other, const allocator_type& allocator) : Operations(allocator)
  {
    init();
    assign(other.begin(), other.end());
  }

  Vector(Vector&& other) noexcept
  {
    init();
    swap(other);
  }

  Vector(Vector&& other, const allocator_type& allocator) : Operations(allocator)
  {
    init();
    assignMove(other.begin(), other.end());
  }

  explicit Vector(const std_vector& other)
  {
    init();
    assign(other.begin(), other.end());
  }

  Vector(const std_vector& other, const allocator_type& allocator) : Operations(allocator)
  {
    init();
    assign(other.begin(), other.end());
  }

  Vector(std::initializer_list<value_type> il, const allocator_type& allocator = allocator_type())
    : Operations(allocator)
  {
    init();
    assign(std::move(il));
  }

  ~Vector() { deallocate(); }

  Vector& operator=(const Vector& other)
  {
    assign(other.begin(), other.end());
    return *this;
  }

  Vector& operator=(Vector&& other) noexcept(noexcept(adlSwap(other, other)))
  {
    swap(other);
    return *this;
  }

  Vector& operator=(std::initializer_list<value_type> il)
  {
    assign(std::move(il));
    return *this;
  }

  void assign(size_type size, const_reference value) { assignFill(size, value); }

  template <typename Number>
  typename boost::enable_if_c<boost::is_integral<Number>::value &&
                                  !boost::is_same<Number, value_type>::value,
                              void>::type
  assign(Number size, const_reference value)
  {
    assignFill(checkedSize(size), value);
  }

  template <typename Iterator>
  void assign(Iterator first, Iterator last)
  {
    // Adopted from GNU libstdc++'s std::vector.
    typedef boost::is_integral<Iterator> Integral;

    assignRange(first, last, Integral());
  }

  void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }

  allocator_type get_allocator() const noexcept { return *this; }

  size_type size() const noexcept { return _size; }
  bool empty() const noexcept { return !_size; }
  size_type max_size() const noexcept { return Operations::max_size(); }
  size_type capacity() const noexcept { return _capacity; }

        iterator  begin()       noexcept { return       iterator(_begin); }
  const_iterator  begin() const noexcept { return const_iterator(_begin); }
  const_iterator cbegin() const noexcept { return const_iterator(_begin); }

        iterator  end()       noexcept { return       iterator(_begin + _size); }
  const_iterator  end() const noexcept { return const_iterator(_begin + _size); }
  const_iterator cend() const noexcept { return const_iterator(_begin + _size); }

        reverse_iterator  rbegin()       noexcept { return       reverse_iterator( end()); }
  const_reverse_iterator  rbegin() const noexcept { return const_reverse_iterator(cend()); }
  const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

        reverse_iterator  rend()       noexcept { return       reverse_iterator( begin()); }
  const_reverse_iterator  rend() const noexcept { return const_reverse_iterator(cbegin()); }
  const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

        reference front()       { return *_begin; }
  const_reference front() const { return *_begin; }

        reference back()       { return *(_begin + _size - 1); }
  const_reference back() const { return *(_begin + _size - 1); }

        pointer data()       noexcept { return _begin; }
  const_pointer data() const noexcept { return _begin; }

  void resize(size_type size)
  {
    if (size < _size)
      return resizeSmaller(size);
    if (size > _size)
      return resizeBigger(size);
  }

  template <typename Number>
  typename boost::enable_if<boost::is_integral<Number>, void>::type resize(Number size)
  {
    resize(checkedSize(size));
  }

  void resize(size_type size, const_reference value)
  {
    if (size < _size)
      return resizeSmaller(size);
    if (size > _size)
      return resizeBigger(size, value);
  }

  template <typename Number>
  typename boost::enable_if<boost::is_integral<Number>, void>::type
  resize(Number size, const_reference value)
  {
    resize(checkedSize(size), value);
  }

  void reserve(size_type size)
  {
    if (size <= _capacity)
      return;
    reserveBigger(size, true);
  }

  template <typename Number>
  typename boost::enable_if<boost::is_integral<Number>, void>::type reserve(Number size)
  {
    reserve(checkedSize(size));
  }

  void shrink_to_fit()
  {
    if (_size == _capacity)
      return;
    reserveSmaller(_size, true);
  }

  void clear()
  {
    deallocate();
    init();
  }

        reference operator[](size_type i)             { return _begin[i]; }
  const_reference operator[](size_type i)       const { return _begin[i]; }

  template <typename Number>
  typename boost::enable_if<boost::is_integral<Number>, reference>::type
  operator[](Number i)       { return _begin[checkedSize(i)]; }

  template <typename Number>
  typename boost::enable_if<boost::is_integral<Number>, const_reference>::type
  operator[](Number i) const { return _begin[checkedSize(i)]; }

  reference at(size_type i)
  {
    if (i > _size)
      throw std::out_of_range("tse::Vector::at");
    return (*this)[i];
  }

  const_reference at(size_type i) const
  {
    if (i > _size)
      throw std::out_of_range("tse::Vector::at");
    return (*this)[i];
  }

  template <typename Number>
  typename boost::enable_if<boost::is_integral<Number>, reference>::type at(Number i)
  {
    return at(checkedSize(i));
  }

  template <typename Number>
  typename boost::enable_if<boost::is_integral<Number>, const_reference>::type at(Number i) const
  {
    return at(checkedSize(i));
  }

  void push_back(const_reference value)
  {
    if (!remaining())
      reserveBigger(_size + 1);
    Operations::construct(_begin + _size, value);
    ++_size;
  }

  void push_back(value_type&& value)
  {
    if (!remaining())
      reserveBigger(_size + 1);
    Operations::construct(_begin + _size, std::move(value));
    ++_size;
  }

  template <typename... Args>
  void emplace_back(Args&&... args)
  {
    if (!remaining())
      reserveBigger(_size + 1);
    Operations::construct(_begin + _size, std::forward<Args>(args)...);
    ++_size;
  }

  void pop_back() { resizeSmaller(_size - 1); }

  iterator insert(const_iterator position, const_reference value)
  {
    return insert(position, 1, value);
  }

  iterator insert(const_iterator position, size_type count, const_reference value)
  {
    return insertFill(position - cbegin(), count, value);
  }

  template <typename Number>
  typename boost::enable_if_c<boost::is_integral<Number>::value &&
                                  !boost::is_same<Number, value_type>::value,
                              iterator>::type
  insert(const_iterator position, Number count, const_reference value)
  {
    return insert(position, checkedSize(count), value);
  }

  template <typename Iterator>
  iterator insert(const_iterator position, Iterator first, Iterator last)
  {
    // Adopted from GNU libstdc++'s std::vector.
    typedef boost::is_integral<Iterator> Integral;

    return insertRange(position - cbegin(), first, last, Integral());
  }

  iterator insert(const_iterator position, value_type&& value)
  {
    return insertMove(position - cbegin(), std::move(value));
  }

  iterator insert(const_iterator position, std::initializer_list<value_type> il)
  {
    return insert(position, il.begin(), il.end());
  }

  template <typename... Args>
  iterator emplace(const_iterator position, Args&&... args)
  {
    return insertEmplace(position - cbegin(), std::forward<Args>(args)...);
  }

  iterator erase(const_iterator position) { return eraseImpl(position - cbegin(), 1); }

  iterator erase(const_iterator first, const_iterator last)
  {
    assert(last >= first);
    return eraseImpl(first - cbegin(), last - first);
  }

  void swap(Vector& other) noexcept(noexcept(adlSwap(static_cast<Operations&>(other),
                                                     static_cast<Operations&>(other))))
  {
    adlSwap(static_cast<Operations&>(*this), static_cast<Operations&>(other));
    adlSwap(_begin, other._begin);
    adlSwap(_size, other._size);
    adlSwap(_capacity, other._capacity);
  }

  friend void swap(Vector& left, Vector& right) { left.swap(right); }

  friend bool operator==(const Vector& left, const Vector& right)
  {
    if (left._size != right._size)
      return false;
    return Operations::compareEqual(left._begin, right._begin, left._size);
  }

  friend bool operator<(const Vector& left, const Vector& right)
  {
    return Operations::compareLess(left._begin, left._size, right._begin, right._size);
  }

  friend bool operator!=(const Vector& left, const Vector& right) { return !(left == right); }

  friend bool operator>(const Vector& left, const Vector& right) { return right < left; }

  friend bool operator<=(const Vector& left, const Vector& right) { return !(left > right); }

  friend bool operator>=(const Vector& left, const Vector& right) { return !(left < right); }

private:
  pointer _begin;
  size_type _size, _capacity;

  size_type remaining() const { return _capacity - _size; }

  size_type checkedSize(const size_type size) const { return size; }

  template <typename Number>
  typename boost::enable_if_c<boost::is_integral<Number>::value, size_type>::type
  checkedSize(const Number size) const
  {
    assert(size >= 0 && size_type(size) <= max_size());
    return size;
  }

  void init()
  {
    _begin = pointer();
    _size = _capacity = size_type();
  }

  size_type alignSize(const size_type size, const bool exactly = false)
  {
    if (exactly)
      return size;
    return detail::nextSize(_capacity, size);
  }

  void deallocate() { deallocate(_begin, _size, _capacity); }

  void deallocate(const pointer allocation, const size_type size, const size_type capacity)
  {
    if (!allocation)
      return;

    Operations::destroy(allocation, size);
    Operations::deallocate(allocation, capacity);
  }

  // Makes the size() equal to its argument.
  // The current objects are NOT preserved.
  // Returns the number of existing, constructed objects. It will be equal or smaller than size.
  size_type allocate(const size_type size, const bool exactly = false)
  {
    const pointer oldBegin = _begin;
    const size_type oldSize = _size;
    const size_type oldCapacity = _capacity;

    _capacity = alignSize(size, exactly);

    if (_capacity == 0)
    {
      _begin = pointer();
      deallocate(oldBegin, oldSize, oldCapacity);
      return 0;
    }

    if (_capacity == oldCapacity)
    {
      if (size >= oldSize)
        return oldSize;

      Operations::destroy(_begin + size, oldSize - size);
      return size;
    }

    _begin = Operations::allocate(_capacity);
    deallocate(oldBegin, oldSize, oldCapacity);
    return 0;
  }

  // Similar to the above one, but preserves already allocated objects.
  // Returns the number of existing, constructed objects in the new allocation.
  // The return value will never be lower than min(_size, size).
  size_type reallocate(const size_type size, const bool exactly = false)
  {
    const size_type elementsToCopy = std::min(_size, size);
    if (elementsToCopy == 0)
      return allocate(size, exactly);

    // This way we now know that size > 0

    const size_type oldSize = _size;
    const size_type oldCapacity = _capacity;

    _capacity = alignSize(size, exactly);

    if (_capacity == oldCapacity)
    {
      if (size >= oldSize)
        return oldSize;

      Operations::destroy(_begin + size, oldSize - size);
      return size;
    }

    if (const pointer newAllocation = Operations::reallocate(_begin, oldCapacity, _capacity))
    {
      _begin = newAllocation;
      return elementsToCopy;
    }

    const pointer newAllocation = Operations::allocate(_capacity);
    Operations::move(newAllocation, elementsToCopy, 0, _begin);

    Operations::destroy(_begin, oldSize);
    Operations::deallocate(_begin, oldCapacity);

    _begin = newAllocation;
    return elementsToCopy;
  }

  // Allocate another buffer.
  pointer allocateAnother(const size_type size, size_type* capacity, const bool exactly = false)
  {
    if (size == 0)
    {
      *capacity = 0;
      return nullptr;
    }

    *capacity = alignSize(size, exactly);
    return Operations::allocate(*capacity);
  }

  void reserveSmaller(const size_type size, const bool exactly = false)
  {
    assert(size < _capacity);
    assert(size >= _size);

    const size_type existing = reallocate(size, exactly);
    assert(existing == _size);
  }

  void reserveBigger(const size_type size, const bool exactly = false)
  {
    assert(size > _capacity);

    const size_type existing = reallocate(size, exactly);
    assert(existing == _size);
  }

  void resizeSmaller(const size_type size, const bool exactly = false)
  {
    assert(size < _size);

    const size_type existing = reallocate(size, exactly);
    _size = size;
    assert(existing == _size);
  }

  void resizeBigger(const size_type size, const bool exactly = false)
  {
    assert(size > _size);

    const size_type existing = reallocate(size, exactly);
    Operations::fill(_begin + existing, size - existing, 0);
    _size = size;
  }

  void resizeBigger(const size_type size, const_reference value, const bool exactly = false)
  {
    assert(size > _size);

    const size_type existing = reallocate(size, exactly);
    Operations::fill(_begin + existing, size - existing, 0, value);
    _size = size;
  }

  void assignFill(const size_type size)
  {
    if (size == 0)
      return clear();

    const size_type existing = allocate(size, true);
    _size = size;
    Operations::fill(_begin, size, existing);
  }

  void assignFill(const size_type size, const_reference value)
  {
    if (size == 0)
      return clear();

    const size_type existing = allocate(size, true);
    _size = size;
    Operations::fill(_begin, size, existing, value);
  }

  template <typename Number>
  void assignRange(const Number& size, const Number& value, boost::true_type)
  {
    assignFill(size, value);
  }

  template <typename Iterator>
  void assignRange(const Iterator& first, const Iterator& last, boost::false_type)
  {
    const size_type size = checkedSize(std::distance(first, last));
    if (!size)
      return clear();

    const size_type existing = allocate(size, true);
    _size = size;
    Operations::copyRange(_begin, size, existing, first);
  }

  template <typename Iterator>
  void assignMove(const Iterator& first, const Iterator& last)
  {
    const size_type size = checkedSize(std::distance(first, last));
    if (!size)
      return clear();

    const size_type existing = allocate(size, true);
    _size = size;
    Operations::moveRange(_begin, size, existing, first);
  }

  struct UniformFiller
  {
    UniformFiller(const_reference value, Operations& operations)
      : value(value), operations(operations)
    {
    }
    const_reference value;
    Operations& operations;

    void operator()(const pointer memory, const size_type size, const size_type existing) const
    {
      operations.fill(memory, size, existing, value);
    }
  };

  struct MoveFiller
  {
    MoveFiller(value_type&& value, Operations& operations)
      : value(std::move(value)), operations(operations)
    {
    }
    value_type&& value;
    Operations& operations;

    void operator()(const pointer memory, const size_type size, const size_type existing) const
    {
      assert(size == 1);
      operations.move(memory, size, existing, &value);
    }
  };

  template <typename... Args>
  struct EmplaceFiller
  {
    EmplaceFiller(std::tuple<Args&&...> args, Operations& operations)
      : args(std::move(args)), operations(operations)
    {
    }
    std::tuple<Args&&...> args;
    Operations& operations;

    template <int... S>
    void construct(const pointer memory, Sequence<S...>) const
    {
      operations.construct(memory, std::move(std::get<S>(args))...);
    }

    template <int... S>
    void assign(const pointer memory, Sequence<S...>) const
    {
      *memory = value_type(std::move(std::get<S>(args))...);
    }

    void operator()(const pointer memory, const size_type size, const size_type existing) const
    {
      assert(size == 1);

      if (existing == 0)
      {
        construct(memory, makeSequence<sizeof...(Args)>());
      }
      else
      {
        assign(memory, makeSequence<sizeof...(Args)>());
      }
    }
  };

  template <typename Iterator>
  struct RangeFiller
  {
    RangeFiller(const Iterator& first, Operations& operations)
      : first(first), operations(operations)
    {
    }
    const Iterator& first;
    Operations& operations;

    void operator()(const pointer memory, const size_type size, const size_type existing) const
    {
      operations.copyRange(memory, size, existing, first);
    }
  };

  iterator insertFill(const size_type offset, size_type count, const_reference value)
  {
    return insertImpl(offset, count, UniformFiller(value, *this));
  }

  iterator insertMove(const size_type offset, value_type&& value)
  {
    return insertImpl(offset, 1, MoveFiller(std::move(value), *this));
  }

  template <typename... Args>
  iterator insertEmplace(const size_type offset, Args&&... args)
  {
    return insertImpl(
        offset,
        1,
        EmplaceFiller<Args...>(std::forward_as_tuple(std::forward<Args>(args)...), *this));
  }

  template <typename Number>
  iterator
  insertRange(const size_type offset, const Number& count, const Number& value, boost::true_type)
  {
    return insertFill(offset, count, value);
  }

  template <typename Iterator>
  iterator insertRange(const size_type offset, Iterator first, Iterator last, boost::false_type)
  {
    const size_type count = checkedSize(std::distance(first, last));
    return insertImpl(offset, count, RangeFiller<Iterator>(first, *this));
  }

  template <typename Filler>
  iterator insertImpl(const size_type offset, size_type count, const Filler& filler)
  {
    const size_type size = _size + count;

    const bool reallocate = (remaining() < count);
    if (reallocate)
    {
      if (!boost::is_pod<value_type>::value || offset != _size)
        return insertAllocateAnother(offset, count, filler);

      reserveBigger(size);
    }

    if (offset == _size)
    {
      // We don't have to move anything in this case.
      filler(_begin + offset, count, 0);
    }
    else
    {
      Operations::moveRight(_begin + offset, _size - offset, count);
      filler(_begin + offset, count, count);
    }

    _size = size;
    return begin() + offset;
  }

  template <typename Filler>
  iterator insertAllocateAnother(const size_type offset, size_type count, const Filler& filler)
  {
    const size_type size = _size + count;

    size_type capacity;
    const pointer another = allocateAnother(size, &capacity);

    Operations::move(another, offset, 0, _begin);
    filler(another + offset, count, 0);
    Operations::move(another + offset + count, _size - offset, 0, _begin + offset);

    deallocate();
    _begin = another;
    _size = size;
    _capacity = capacity;

    return begin() + offset;
  }

  iterator eraseImpl(const size_type offset, size_type count)
  {
    assert(offset + count <= _size);
    const size_type size = _size - count;

    const bool reallocate = (alignSize(size) != _capacity);
    if (reallocate)
    {
      if (!boost::is_pod<value_type>::value)
        return eraseAllocateAnother(offset, count);
    }

    Operations::moveLeft(_begin + offset + count, _size - offset - count, count);
    Operations::destroy(_begin + size, count);

    if (reallocate)
      resizeSmaller(size);

    _size = size;
    return begin() + offset;
  }

  iterator eraseAllocateAnother(const size_type offset, size_type count)
  {
    const size_type size = _size - count;

    size_type capacity;
    const pointer another = allocateAnother(size, &capacity);

    Operations::move(another, offset, 0, _begin);
    Operations::move(another + offset, _size - offset - count, 0, _begin + offset + count);

    deallocate();
    _begin = another;
    _size = size;
    _capacity = capacity;

    return begin() + offset;
  }
};
}

#endif // _GLIBCXX_DEBUG

#endif // UTIL_VECTOR_H
