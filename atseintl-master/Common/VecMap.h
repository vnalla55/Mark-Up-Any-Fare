//-------------------------------------------------------------------
//
//  File:        VecMap.h
//  Created:     July 28, 2006
//  Authors:     Kavya Katam
//
//  Description: Interface is designed to be similar to std::map
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

// Documentation: VecMap is an interface which is designed to be similar to std::map. Implementation
// is based on
// a sorted vector. VecMap do not allocate any memory at all if they don't grow beyond empty, while
// the std::map
// always allocate one node in their constructor. VecMap works well than std::map if they are of a
// small size.

// Insert : inserts in a sorted vector. So the time taken to insert is O(n) while the time taken to
// insert in
// std::map is O(logn).

// Find : the time taken to find is log(n) in VecMap and std::map.

// Count : as the vector is sorted difference between the iterators gives the exact count of the
// key.

// Erase : as the vector is sorted, time taken to erase is linear while time take to erase in
// std::map is O(logn).

// Warning : Insert and erase in a vector moves elements around. An iterator pointing to an element
// of a vector
// may after insert or erase point to another element or to no element at all. If we access an
// element through an invalid
// iterator, the effect will be disastrous.

// -----------------------------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <vector>

#include <sys/types.h>

template <class T, class U> // class T for key, class U for data
class VecMap
{
public:
  VecMap() {}

  typedef std::pair<T, U> KeyValuePair; // pair to hold key and data
  typedef std::vector<KeyValuePair> Vec;

  typedef typename Vec::iterator iterator;
  typedef typename Vec::value_type value_type;
  typedef std::pair<iterator, iterator> VecPair;
  typedef std::pair<iterator, bool> insertPair;
  typedef typename Vec::const_iterator const_iterator;
  typedef std::pair<const_iterator, const_iterator> const_VecPair;
  typedef typename Vec::size_type size_type;
  typedef typename Vec::reverse_iterator reverse_iterator;
  typedef typename Vec::const_reverse_iterator const_reverse_iterator;

  // overload operators
  bool operator==(const VecMap& c2) const { return this->_vecMap == c2._vecMap; }

  bool operator!=(const VecMap& c2) const { return this->_vecMap != c2._vecMap; }

  bool operator<(const VecMap& c2) const { return this->_vecMap < c2._vecMap; }

  bool operator>(const VecMap& c2) const { return this->_vecMap > c2._vecMap; }

  bool operator<=(const VecMap& c2) const { return this->_vecMap <= c2._vecMap; }

  bool operator>=(const VecMap& c2) const { return this->_vecMap >= c2._vecMap; }

  // operator [] overloaded to insert using the index of position insert in to sorted vector.
  U& operator[](const T& key)
  {
    U second = U();
    KeyValuePair keyValuePair = std::make_pair(key, second);
    insertPair myPair = insert(keyValuePair);
    return (*(myPair.first)).second;
  }

  // Functor that returns boolean by comparing keys.
  struct compare_key_only
  {
    bool operator()(const KeyValuePair& a, const KeyValuePair& b) const
    {
      return a.first < b.first;
    }

    bool operator()(const KeyValuePair& a, const T& b) const { return a.first < b; }

    bool operator()(const T& a, const KeyValuePair& b) const { return a < b.first; }
  };

private:
  Vec _vecMap;

public:
  iterator begin() { return _vecMap.begin(); }

  const_iterator begin() const { return _vecMap.begin(); }

  const_iterator cbegin() const { return _vecMap.cbegin(); }

  iterator end() { return _vecMap.end(); }

  const_iterator end() const { return _vecMap.end(); }

  const_iterator cend() const { return _vecMap.cend(); }

  // insert the pair in to the sorted vector.
  iterator insert(iterator pos, const KeyValuePair& keyValuePair)
  {
    insertPair retPair = insert(keyValuePair);
    return retPair.first;
  }

  // using std::equal_range which returns a pair of iterators the first iterator in this pair
  // is the position to insert in vector, the two conditions to maintain uniqueness in vector are
  // If the map is empty (OR) If the map has atleast one item. the map here is the vector used
  // internally.
  // a pair which states whether the value is inserted(true or false) is returned back.
  // Inserts in a sorted vector, Set uses a binary tree to insert
  // TimeComplexity: log(n)
  insertPair insert(const KeyValuePair& keyValuePair)
  {
    bool bExist = false;
    iterator pos = lower_bound(keyValuePair.first);
    if ((pos == end()) || ((*pos).first != keyValuePair.first))
    {
      iterator iter = _vecMap.insert(pos, keyValuePair);
      pos = iter;
      bExist = true;
    }
    insertPair retPair = std::make_pair(pos, bExist);
    return retPair;
  }

  // Function template is used here to handle multiple types of iterators
  // insert in to vector using insert function in vector which takes the iterators start and end.
  // std::stable_sort preserves the relative ordering of equivalent elements and it differs from
  // sort
  // in terms of time complexity. stable_sort and unique are being used to prevent multiple
  // insertions
  // as Set (internally vector) should hold only one unique key.
  // In a group of consecutive duplicate elements, the algorithm unique removes all but the first
  // element.
  // erase is used on the vector along with unique to erase duplicate keys.
  template <typename InputIterator>
  void insert(InputIterator iterBegin, InputIterator iterEnd)
  {
    _vecMap.insert(end(), iterBegin, iterEnd);
    std::stable_sort(begin(), end());
    _vecMap.erase(unique(begin(), end()), _vecMap.end());
  }

  // lower,upper,equal_range use functor compare_key_onlyh() to compare different keys.
  iterator lower_bound(const T& value)
  {
    iterator pos = std::lower_bound(begin(), end(), value, compare_key_only());
    return pos;
  }

  const_iterator lower_bound(const T& value) const
  {
    const_iterator pos = std::lower_bound(begin(), end(), value, compare_key_only());
    return pos;
  }

  iterator upper_bound(const T& value)
  {
    iterator location = std::upper_bound(begin(), end(), value, compare_key_only());
    return location;
  }

  const_iterator upper_bound(const T& value) const
  {
    const_iterator location = std::upper_bound(begin(), end(), value, compare_key_only());
    return location;
  }

  VecPair equal_range(const T& value)
  {
    VecPair retPair = std::equal_range(begin(), end(), value, compare_key_only());
    return retPair;
  }

  const_VecPair equal_range(const T& value) const
  {
    const_VecPair retPair = std::equal_range(begin(), end(), value, compare_key_only());
    return retPair;
  }

  // using std::equal_range which returns a pair of iterators if both the iterators are equal
  // then there is no key in the map(vector in our case) so return end of the vector
  // or else return the first iterator in the pair which is the key we are looking for.
  // find the item in multiset in logarthmic time
  iterator find(const T& value)
  {
    VecPair findPair = std::equal_range(begin(), end(), value, compare_key_only());
    if (findPair.first == findPair.second)
      return end();
    else
      return findPair.first;
  }

  const_iterator find(const T& value) const
  {
    const_VecPair findPair = std::equal_range(begin(), end(), value, compare_key_only());
    if (findPair.first == findPair.second)
      return end();
    else
      return findPair.first;
  }

  // using std::equal_range which returns a pair of iterators difference between these iterators
  // gives the count of the key, as the vector is sorted difference between the iterators gives the
  // exact
  // count of the key.
  // counts in logarthmic time
  size_type count(const T& value) const
  {
    size_type count = 0;
    const_VecPair countPair = std::equal_range(begin(), end(), value, compare_key_only());
    count = countPair.second - countPair.first;
    return count;
  }

  void clear() { _vecMap.clear(); }

  size_type size() const { return _vecMap.size(); }

  bool empty() const { return _vecMap.empty(); }

  void erase(iterator pos) { _vecMap.erase(pos); }

  void erase(iterator start, iterator end) { _vecMap.erase(start, end); }

  // using std::equal_range which returns a pair of iterators difference between these iterators
  // gives the count of the key, as the vector is sorted difference between the iterators gives the
  // exact count.
  // using the first and second iterators from the pair, use the vector erase function to erase the
  // keys.
  // erases in logarthmic time as the inner vector is already sorted
  size_type erase(const T& key)
  {
    size_type count = 0;
    VecPair countPair = std::equal_range(begin(), end(), key, compare_key_only());
    count = countPair.second - countPair.first;
    iterator iter = _vecMap.erase(countPair.first, countPair.second);
    return count;
  }

  size_type max_size() const { return _vecMap.max_size(); }

  reverse_iterator rbegin() { return _vecMap.rbegin(); }

  const_reverse_iterator rbegin() const { return _vecMap.rbegin(); }

  reverse_iterator rend() { return _vecMap.rend(); }

  const_reverse_iterator rend() const { return _vecMap.rend(); }

  void swap(const VecMap& from) { _vecMap.swap(from._vecMap()); }
};

