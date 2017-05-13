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

#ifndef UTIL_ITERATOR_H
#define UTIL_ITERATOR_H

#include <iterator>
#include <vector>

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace tse
{
template <typename Type, typename Container>
class PointerIterator
{
  typedef typename Container::value_type ConstLessType;

public:
  typedef std::random_access_iterator_tag iterator_category;
  typedef Type value_type;
  typedef value_type* pointer;
  typedef value_type& reference;

  typedef typename Container::difference_type difference_type;

  PointerIterator() : _pointer(nullptr) {}

  explicit PointerIterator(typename Container::pointer ptr) : _pointer(pointer(ptr)) {}

  PointerIterator(const PointerIterator& o) : _pointer(o._pointer) {}

  template <typename Other>
  PointerIterator(const PointerIterator<Other, typename boost::enable_if_c<
                  boost::is_same<ConstLessType, Other>::value, Container>::type>& o)
    : _pointer(pointer(o.get_pointer())) {}

  template <typename Other> explicit
  PointerIterator(const PointerIterator<Other, typename boost::enable_if_c<
                  !boost::is_same<ConstLessType, Other>::value, Container>::type>& o)
    : _pointer(pointer(o.get_pointer())) {}

  pointer get_pointer() const { return _pointer; }

  reference operator*() const { return *_pointer; }
  pointer operator->() const { return _pointer; }

  reference operator[](const difference_type offset) const { return _pointer[offset]; }

  PointerIterator& operator++()
  {
    ++_pointer;
    return *this;
  }
  PointerIterator& operator--()
  {
    --_pointer;
    return *this;
  }

  PointerIterator operator++(int)
  {
    PointerIterator copy(*this);
    ++(*this);
    return copy;
  }
  PointerIterator operator--(int)
  {
    PointerIterator copy(*this);
    --(*this);
    return copy;
  }

  PointerIterator& operator+=(const difference_type offset)
  {
    _pointer += offset;
    return *this;
  }
  PointerIterator& operator-=(const difference_type offset)
  {
    _pointer -= offset;
    return *this;
  }

  friend difference_type operator-(const PointerIterator& l, const PointerIterator& r)
  {
    return l._pointer - r._pointer;
  }

  friend PointerIterator operator+(const PointerIterator& it, const difference_type offset)
  {
    PointerIterator copy(it);
    copy += offset;
    return copy;
  }

  friend PointerIterator operator-(const PointerIterator& it, const difference_type offset)
  {
    PointerIterator copy(it);
    copy -= offset;
    return copy;
  }

  friend PointerIterator operator+(const difference_type offset, const PointerIterator& it)
  {
    return it + offset;
  }

  friend bool operator==(const PointerIterator& l, const PointerIterator& r)
  {
    return l._pointer == r._pointer;
  }
  friend bool operator<(const PointerIterator& l, const PointerIterator& r)
  {
    return l._pointer < r._pointer;
  }

  friend bool operator!=(const PointerIterator& l, const PointerIterator& r) { return !(l == r); }
  friend bool operator>(const PointerIterator& l, const PointerIterator& r) { return r < l; }
  friend bool operator<=(const PointerIterator& l, const PointerIterator& r) { return !(l > r); }
  friend bool operator>=(const PointerIterator& l, const PointerIterator& r) { return !(l < r); }

  friend void swap(PointerIterator& l, PointerIterator& r) { swap(l._pointer, r._pointer); }

private:
  pointer _pointer;
};

template <typename Iterator>
struct IteratorTraitsBase : std::iterator_traits<Iterator>
{
  static const bool is_random_access =
      boost::is_same<typename std::iterator_traits<Iterator>::iterator_category,
                     std::random_access_iterator_tag>::value;
};

template <typename Iterator>
struct IteratorTraits : IteratorTraitsBase<Iterator>
{
  static const bool is_pointer = false;
};

template <typename Type, typename Container>
struct IteratorTraits<PointerIterator<Type, Container> >
    : IteratorTraitsBase<PointerIterator<Type, Container> >
{
  static const bool is_pointer = true;
};

#ifdef _GLIBCXX_BEGIN_NAMESPACE_VERSION
template <typename Pointer, typename Container>
struct IteratorTraits<__gnu_cxx::__normal_iterator<Pointer, Container> >
    : IteratorTraitsBase<__gnu_cxx::__normal_iterator<Pointer, Container> >
{
  static const bool is_pointer =
      boost::is_same<Pointer,
                     typename std::iterator_traits<
                         __gnu_cxx::__normal_iterator<Pointer, Container> >::pointer>::value;
};
#endif
}

#endif
