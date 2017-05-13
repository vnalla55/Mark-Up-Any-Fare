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

#if !defined(UTIL_FLAT_SET_H) && !defined(UTIL_FLAT_MAP_H)
# error "Do NOT include Util/FlatBase.h directly."
#endif

#ifndef UTIL_FLAT_BASE_H
#define UTIL_FLAT_BASE_H

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <type_traits>

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#include "Util/Allocator.h"
#include "Util/FlatFwd.h"
#include "Util/Utility.h"

#ifdef _GLIBCXX_DEBUG
# include <assert.h>
# include <iostream>
# include "Util/StackUtil.h"
#endif

namespace tse
{
namespace detail
{
template <typename Pair, typename Compare>
class PairFirstCompare : public std::binary_function<Pair, Pair, bool>
{
public:
  PairFirstCompare(Compare compare) : _compare(compare) {}

  bool operator()(const Pair& l, const Pair& r) { return _compare(l.first, r.first); }

private:
  Compare _compare;
};

template <typename Pair, typename Compare>
class PairFirstCompareMixed : public std::binary_function<Pair, Pair, bool>
{
  typedef typename Pair::first_type First;

public:
  PairFirstCompareMixed(Compare compare) : _compare(compare) {}

  bool operator()(const Pair& l, const First& r) { return _compare(l.first, r); }

  bool operator()(const First& l, const Pair& r) { return _compare(l, r.first); }

private:
  Compare _compare;
};

template <typename Less>
class EqualFromLess : public std::binary_function<typename Less::first_argument_type,
                                                  typename Less::first_argument_type,
                                                  bool>
{
  typedef typename Less::first_argument_type Type;

public:
  EqualFromLess(Less compare) : _compare(compare) {}

  bool operator()(const Type& l, const Type& r) { return !_compare(l, r) && !_compare(r, l); }

private:
  Less _compare;
};

template <typename InsertResult, typename InsertPair, bool IsUnique>
struct InsertResultNormalizeHelper
{
  static InsertResult normalize(InsertPair p) { return p; }
};

template <typename InsertResult, typename InsertPair>
struct InsertResultNormalizeHelper<InsertResult, InsertPair, false>
{
  static InsertResult normalize(InsertPair p) { return p.first; }
};

template <typename value_type, typename mapped_type, bool IsSet>
struct GetMappedHelper
{
  static       mapped_type& getMapped(      value_type& value) { return value; }
  static const mapped_type& getMapped(const value_type& value) { return value; }
};

template <typename value_type, typename mapped_type>
struct GetMappedHelper<value_type, mapped_type, false>
{
  static       mapped_type& getMapped(      value_type& value) { return value.second; }
  static const mapped_type& getMapped(const value_type& value) { return value.second; }
};

template <typename KeyType,
          typename MappedType,
          typename Compare,
          typename Container,
          bool UniqueKeys>
class FlatBase : private Compare
{
public:
  typedef KeyType key_type;
  typedef MappedType mapped_type;
  typedef Compare key_compare;
  typedef Container container_type;
  typedef typename container_type::value_type value_type;

  static const bool IsSet = boost::is_same<KeyType, value_type>::value;
  static const bool IsUnique = UniqueKeys;

private:
  typedef PairFirstCompare<value_type, Compare> MapValueCompareBase;
  class MapValueCompare : public MapValueCompareBase
  {
    friend class FlatBase;
    MapValueCompare(Compare compare) : MapValueCompareBase(compare) {}
  };

  typedef typename boost::conditional<IsSet,
                                      key_compare,
                                      PairFirstCompareMixed<value_type, Compare> >::type
  CompareWithKey;

  typedef typename boost::conditional<IsSet, key_compare, MapValueCompare>::type ValueCompare;

  typedef EqualFromLess<ValueCompare> ValueEqual;

public:
  typedef ValueCompare value_compare;

  typedef typename container_type::allocator_type allocator_type;
  typedef typename container_type::reference reference;
  typedef typename container_type::const_reference const_reference;
  typedef typename container_type::pointer pointer;
  typedef typename container_type::const_pointer const_pointer;

  typedef typename container_type::const_iterator const_iterator;
  typedef typename boost::conditional<IsSet,
                                      const_iterator,
                                      typename container_type::iterator>::type iterator;

  typedef typename container_type::const_reverse_iterator const_reverse_iterator;
  typedef typename boost::conditional<IsSet,
                                      const_reverse_iterator,
                                      typename container_type::reverse_iterator>::type
  reverse_iterator;

  typedef typename container_type::difference_type difference_type;
  typedef typename container_type::size_type size_type;

private:
  typedef std::pair<iterator, bool> InsertPair;
  typedef typename boost::conditional<IsUnique, InsertPair, iterator>::type InsertResult;

  static InsertResult normalizeInsertResult(InsertPair p)
  {
    return InsertResultNormalizeHelper<InsertResult, InsertPair, IsUnique>::normalize(p);
  }

public:
  FlatBase() noexcept { markOrdered(); }

  explicit FlatBase(const key_compare& compare, const allocator_type& allocator = allocator_type())
    : key_compare(compare), _container(allocator)
  {
    markOrdered();
  }

  explicit FlatBase(const allocator_type& allocator) : _container(allocator) { markOrdered(); }

  template <typename Iterator>
  FlatBase(Iterator first,
           Iterator last,
           const key_compare& compare = key_compare(),
           const allocator_type& allocator = allocator_type())
    : key_compare(compare), _container(allocator)
  {
    assignRange(first, last);
  }

  template <typename Iterator>
  FlatBase(Iterator first, Iterator last, const allocator_type& allocator)
    : _container(allocator)
  {
    assignRange(first, last);
  }

  FlatBase(const FlatBase& other, const allocator_type& allocator)
    : key_compare(other), _container(other._container, allocator)
  {
#ifdef _GLIBCXX_DEBUG
    _ordered = other._ordered;
#endif
  }

  FlatBase(const FlatBase& other) = default;
  FlatBase(FlatBase&& other) = default;
  FlatBase(FlatBase&& other, const allocator_type& allocator)
    : key_compare(static_cast<key_compare&&>(other)),
      _container(std::move(other._container), allocator)
  {
#ifdef _GLIBCXX_DEBUG
    _ordered = other._ordered;
#endif
  }

  FlatBase(std::initializer_list<value_type> il,
           const key_compare& compare = key_compare(),
           const allocator_type& allocator = allocator_type())
    : key_compare(compare), _container(allocator)
  {
    assignRange(il.begin(), il.end());
  }

  FlatBase(std::initializer_list<value_type> il, const allocator_type& allocator)
    : _container(allocator)
  {
    assignRange(il.begin(), il.end());
  }

  FlatBase& operator=(const FlatBase& other) = default;
  FlatBase& operator=(FlatBase&& other) = default;
  FlatBase& operator=(std::initializer_list<value_type> il)
  {
    assignRange(il.begin(), il.end());
    return *this;
  }

  container_type steal_container() { return std::move(_container); }
  const container_type& container() const { return _container; }

  key_compare key_comp() const { return *this; }
  value_compare value_comp() const { return value_compare(key_comp()); }

  allocator_type get_allocator() const noexcept { return _container.get_allocator(); }

  size_type size() const noexcept
  {
    if (IsUnique)
    {
      // Only unique containers must be ordered for correct size(), since
      // the .order() method will erase duplicates.
      assertOrdered();
    }
    return _container.size();
  }
  bool empty() const noexcept { return _container.empty(); }
  size_type max_size() const noexcept { return _container.max_size(); }
  size_type capacity() const noexcept { return _container.capacity(); }

        iterator  begin()       noexcept { return _container.begin(); }
  const_iterator  begin() const noexcept { return _container.begin(); }
  const_iterator cbegin() const noexcept { return _container.begin(); }

        iterator  end()       noexcept { return _container.end(); }
  const_iterator  end() const noexcept { return _container.end(); }
  const_iterator cend() const noexcept { return _container.end(); }

        reverse_iterator  rbegin()       noexcept { return _container.rbegin(); }
  const_reverse_iterator  rbegin() const noexcept { return _container.rbegin(); }
  const_reverse_iterator crbegin() const noexcept { return _container.rbegin(); }

        reverse_iterator  rend()       noexcept { return _container.rend(); }
  const_reverse_iterator  rend() const noexcept { return _container.rend(); }
  const_reverse_iterator crend() const noexcept { return _container.rend(); }

  void reserve(size_type size) { _container.reserve(size); }

  template <typename Number>
  typename boost::enable_if<boost::is_integral<Number>, void>::type reserve(Number size)
  {
    _container.reserve(size);
  }

  void shrink_to_fit() { _container.shrink_to_fit(); }
  void clear()
  {
    _container.clear();
    markOrdered();
  }

  iterator find(const key_type& key)
  {
    return unConstIterator(const_cast<const FlatBase&>(*this).find(key));
  }

  const_iterator find(const key_type& key) const
  {
    assertOrdered();

    CompareWithKey compare(key_comp());

    const const_iterator it = lower_bound(key);

    if (it != end() && compare(key, *it))
      return end();

    return it;
  }

  size_type count(const key_type& key) const
  {
    if (IsUnique)
      return (find(key) == end() ? 0 : 1);

    const std::pair<const_iterator, const_iterator> range = equal_range(key);
    return range.second - range.first;
  }

  iterator lower_bound(const key_type& key)
  {
    return unConstIterator(const_cast<const FlatBase&>(*this).lower_bound(key));
  }

  const_iterator lower_bound(const key_type& key) const
  {
    assertOrdered();
    return std::lower_bound(begin(), end(), key, CompareWithKey(key_comp()));
  }

  iterator upper_bound(const key_type& key)
  {
    return unConstIterator(const_cast<const FlatBase&>(*this).upper_bound(key));
  }

  const_iterator upper_bound(const key_type& key) const
  {
    assertOrdered();
    return std::upper_bound(begin(), end(), key, CompareWithKey(key_comp()));
  }

  std::pair<iterator, iterator> equal_range(const key_type& key)
  {
    const std::pair<const_iterator, const_iterator> range =
        const_cast<const FlatBase&>(*this).equal_range(key);
    return std::make_pair(unConstIterator(range.first), unConstIterator(range.second));
  }

  std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
  {
    assertOrdered();
    return std::equal_range(begin(), end(), key, CompareWithKey(key_comp()));
  }

  InsertResult insert(const_reference value)
  {
    assertOrdered();
    return normalizeInsertResult(insertImpl(CopyInserter(value)));
  }

  InsertResult insert(value_type&& value)
  {
    assertOrdered();
    return normalizeInsertResult(insertImpl(MoveInserter(std::move(value))));
  }

  template <typename P>
  typename boost::enable_if_c<!IsSet && std::is_constructible<value_type, P&&>::value,
                              InsertResult>::type
  insert(P&& value)
  {
    assertOrdered();
    return normalizeInsertResult(insertImpl(MoveInserter(std::move(value))));
  }

  template <typename... Args>
  InsertResult emplace(Args&&... args)
  {
    return insert(value_type(std::forward<Args>(args)...));
  }

  iterator insert(const_iterator hint, const_reference value)
  {
    assertOrdered();
    return insertWithHint(unConstIterator(hint), CopyInserter(value));
  }

  iterator insert(const_iterator hint, value_type&& value)
  {
    assertOrdered();
    return insertWithHint(unConstIterator(hint), MoveInserter(std::move(value)));
  }

  template <typename P>
  typename boost::enable_if_c<!IsSet && std::is_constructible<value_type, P&&>::value,
                              iterator>::type
  insert(const_iterator hint, P&& value)
  {
    assertOrdered();
    return insertWithHint(unConstIterator(hint), MoveInserter(std::move(value)));
  }

  template <typename... Args>
  iterator emplace_hint(const_iterator hint, Args&&... args)
  {
    return insert(hint, value_type(std::forward<Args>(args)...));
  }

  template <typename Iterator>
  void insert(Iterator first, Iterator last)
  {
    // TODO It could be optimized for small std::distance(first, last).

    unsafe_insert(first, last);
    order();
  }

  void insert(std::initializer_list<value_type> il) { insert(il.begin(), il.end()); }

  iterator erase(const_iterator it)
  {
    assertOrdered();
    return _container.erase(it);
  }

  size_type erase(const key_type& key)
  {
    if (IsUnique)
    {
      const iterator it = find(key);
      if (it == end())
        return 0;

      erase(it);
      return 1;
    }

    const std::pair<const_iterator, const_iterator> range = equal_range(key);
    erase(range.first, range.second);
    return range.second - range.first;
  }

  iterator erase(const_iterator first, const_iterator last)
  {
    assertOrdered();
    return _container.erase(first, last);
  }

  void unsafe_insert(const_reference value)
  {
    markUnOrdered();
    _container.push_back(value);
  }

  void unsafe_insert(value_type&& value)
  {
    markUnOrdered();
    _container.push_back(std::move(value));
  }

  template <typename... Args>
  void unsafe_emplace(Args&&... args)
  {
    markUnOrdered();
    _container.emplace_back(std::forward<Args>(args)...);
  }

  template <typename Iterator>
  void unsafe_insert(Iterator first, Iterator last)
  {
    markUnOrdered();
    _container.insert(end(), first, last);
  }

  void order()
  {
    std::stable_sort(_container.begin(), _container.end(), value_comp());

    if (IsUnique)
    {
      const iterator newEnd =
          std::unique(_container.begin(), _container.end(), ValueEqual(key_comp()));
      _container.resize(newEnd - _container.begin());
    }

    markOrdered();
  }

  void swap(FlatBase& other) noexcept(noexcept(adlSwap(static_cast<key_compare&>(other),
                                                       static_cast<key_compare&>(other))) &&
                                      noexcept(adlSwap(other._container, other._container)))
  {
    adlSwap(static_cast<key_compare&>(*this), static_cast<key_compare&>(other));
    adlSwap(_container, other._container);
#ifdef _GLIBCXX_DEBUG
    adlSwap(_ordered, other._ordered);
#endif
  }

  friend void swap(FlatBase& left, FlatBase& right) { left.swap(right); }

  friend bool operator==(const FlatBase& left, const FlatBase& right)
  {
    left.assertOrdered();
    right.assertOrdered();
    return left._container == right._container;
  }

  friend bool operator<(const FlatBase& left, const FlatBase& right)
  {
    left.assertOrdered();
    right.assertOrdered();
    return left._container < right._container;
  }

  friend bool operator!=(const FlatBase& left, const FlatBase& right) { return !(left == right); }

  friend bool operator>(const FlatBase& left, const FlatBase& right) { return right < left; }

  friend bool operator<=(const FlatBase& left, const FlatBase& right) { return !(left > right); }

  friend bool operator>=(const FlatBase& left, const FlatBase& right) { return !(left < right); }

protected:
  // These 4 methods will only be public in FlatMap.
  mapped_type& operator[](const key_type& key)
  {
    assertOrdered();
    const iterator it = insertImpl(EmptyCopyInserter(key)).first;
    return it->second;
  }

  mapped_type& operator[](key_type&& key)
  {
    assertOrdered();
    const iterator it = insertImpl(EmptyMoveInserter(std::move(key))).first;
    return it->second;
  }

  mapped_type& at(const key_type& key)
  {
    const iterator it = this->find(key);
    if (it == this->end())
      throw std::out_of_range("tse::FlatMap::at");
    return it->second;
  }

  const mapped_type& at(const key_type& key) const
  {
    const const_iterator it = this->find(key);
    if (it == this->end())
      throw std::out_of_range("tse::FlatMap::at");
    return it->second;
  }

private:
  container_type _container;

#ifdef _GLIBCXX_DEBUG
  bool _ordered;
#endif

  void markOrdered()
  {
#ifdef _GLIBCXX_DEBUG
    _ordered = true;
#endif
  }

  void markUnOrdered()
  {
#ifdef _GLIBCXX_DEBUG
    _ordered = false;
#endif
  }

  void assertOrdered() const
  {
#ifdef _GLIBCXX_DEBUG
    if (_ordered)
      return;

    std::cerr << "Incorrect access to unordered flat-like container." << std::endl;
    std::cerr << "Did you forget the order() call?" << std::endl;
    std::cerr.flush();
    StackUtil::outputStackTrace();
    abort();
#endif
  }

  static       mapped_type& getMapped(      reference value)
  {
    return GetMappedHelper<value_type, mapped_type, IsSet>::getMapped(value);
  }
  static const mapped_type& getMapped(const_reference value)
  {
    return GetMappedHelper<value_type, mapped_type, IsSet>::getMapped(value);
  }

  iterator unConstIterator(const_iterator it) { return unConstIterator(_container, it); }

  static typename container_type::iterator
  unConstIterator(container_type& container, const_iterator it)
  {
    return container.begin() + (it - container.cbegin());
  }

  template <typename Iterator>
  void assignRange(Iterator first, Iterator last)
  {
    clear();
    insert(first, last);
  }

  struct CopyInserter
  {
    CopyInserter(const_reference value) : value(value) {}
    const_reference value;

    iterator insert(container_type& container, const_iterator it) const
    {
      return container.insert(unConstIterator(container, it), value);
    }

    typedef value_compare compare;

    const_reference compareArgument() const { return value; }
  };

  struct EmptyCopyInserter
  {
    EmptyCopyInserter(const key_type& key) : key(key) {}
    const key_type& key;

    iterator insert(container_type& container, const_iterator it) const
    {
      return container.insert(unConstIterator(container, it), value_type(key, mapped_type()));
    }

    typedef CompareWithKey compare;

    const key_type& compareArgument() const { return key; }
  };

  struct MoveInserter
  {
    template <typename P>
    MoveInserter(P&& value) : value(std::move(value)) {}
    value_type value;

    iterator insert(container_type& container, const_iterator it) const
    {
      return container.emplace(unConstIterator(container, it), std::move(value));
    }

    typedef value_compare compare;

    const value_type& compareArgument() const { return value; }
  };

  struct EmptyMoveInserter
  {
    EmptyMoveInserter(key_type&& key) : key(std::move(key)) {}
    key_type&& key;

    iterator insert(container_type& container, const_iterator it) const
    {
      return container.insert(unConstIterator(container, it),
                              value_type(std::move(key), mapped_type()));
    }

    typedef CompareWithKey compare;

    const key_type& compareArgument() const { return key; }
  };

  template <typename Inserter>
  InsertPair insertImpl(const Inserter& inserter)
  {
    typename Inserter::compare compare(key_comp());

    iterator it;

    if (IsUnique)
    {
      it = std::lower_bound(begin(), end(), inserter.compareArgument(), compare);

      if (it != end() && !compare(inserter.compareArgument(), *it))
        return InsertPair(it, false);
    }
    else
    {
      it = std::upper_bound(begin(), end(), inserter.compareArgument(), compare);
    }

    return InsertPair(inserter.insert(_container, it), true);
  }

  template <typename Inserter>
  iterator insertWithHint(iterator hint, const Inserter& inserter)
  {
    typename Inserter::compare compare(key_comp());

    if (hint != end())
    {
      if (!compare(inserter.compareArgument(), *hint))
        return insertImpl(inserter).first;
    }

    if (hint != begin())
    {
      if (compare(inserter.compareArgument(), *(hint - 1)))
        return insertImpl(inserter).first;

      if (IsUnique)
      {
        if (!compare(*(hint - 1), inserter.compareArgument()))
        {
          // (hint - 1) is equal to value.
          return hint - 1;
        }
      }
    }

    return inserter.insert(_container, hint);
  }
};
}
}

#endif
