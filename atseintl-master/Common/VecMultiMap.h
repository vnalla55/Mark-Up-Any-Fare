//-------------------------------------------------------------------
//
//  File:        VecMultiMap.h
//  Created:     July 28, 2006
//  Authors:     Kavya Katam
//
//  Description: Interface is designed to be similar to std::multiMap
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

// Documentation: VecMultiMap is an interface which is designed to be similar to std::multimap.
// Implementation is based on
// a sorted vector. VecMultiMap do not allocate any memory at all if they don't grow beyond empty,
// while the std::multimap
// always allocate one node in their constructor. VecMultiMap works well than std::multimap if they
// are of a small size.

// Insert : inserts in a sorted vector. So the time taken to insert is log(n) which is faster
// when compared to std::multimap.

// Find : the time taken to find is log(n) which is faster than std::multimap.

// Count : as the vector is sorted difference between the iterators gives the exact count of the
// key. Time taken is
// logarthmic time which is faster than std::multimap.

// Erase : as the vector is sorted, time taken to erase is logarthmic time which is faster than
// std::multimap.

#pragma once

#include <algorithm>
#include <vector>

#include <sys/types.h>

template <class T, class U> // class T for key, class U for data
class VecMultiMap
{
public:
  VecMultiMap() {}

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

  bool operator==(const VecMultiMap& c2) const { return this->_vecMultiMap == c2._vecMultiMap; }

  bool operator!=(const VecMultiMap& c2) const { return this->_vecMultiMap != c2._vecMultiMap; }

  bool operator<(const VecMultiMap& c2) const { return this->_pairOfVectors < c2._pairOfVectors; }

  bool operator>(const VecMultiMap& c2) const { return this->_vecMultiMap > c2._vecMultiMap; }

  bool operator<=(const VecMultiMap& c2) const { return this->_vecMultiMap <= c2._vecMultiMap; }

  bool operator>=(const VecMultiMap& c2) const { return this->_vecMultiMap >= c2._vecMultiMap; }

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
  Vec _vecMultiMap;

public:
  iterator begin() { return _vecMultiMap.begin(); }

  const_iterator begin() const { return _vecMultiMap.begin(); }

  iterator end() { return _vecMultiMap.end(); }

  const_iterator end() const { return _vecMultiMap.end(); }

  // insert into vector using insert function
  iterator insert(iterator pos, const KeyValuePair& keyValuePair) { return insert(keyValuePair); }

  // Inserts in a sorted vector, this is done by finding the position to insert using
  // std::upper_bound() and inserting in to vector to keep it sorted. upper_bound() uses the functor
  // compare_key_only() provided above to compare two different keys.
  // MultiMap uses a binary tree to insert.
  iterator insert(const KeyValuePair& keyValuePair)
  {
    iterator pos = upper_bound(keyValuePair.first);
    iterator iter = _vecMultiMap.insert(pos, keyValuePair);
    return iter;
  }

  // Function template is used here to handle multiple types of iterators
  // insert in to vector using insert function in vector which takes the iterators start and end.
  // TimeComplexity: O(n) as the vector is sorted.
  template <typename InputIterator>
  void insert(InputIterator first, InputIterator last)
  {
    _vecMultiMap.insert(end(), first, last);
    std::sort(begin(), end());
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
  // then there is no key in the multimap(vector in our case) so return end of the vector
  // or else return the first iterator in the pair which is the key we are looking for.
  // find the item in multimap in logarthmic time
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

  void clear() { _vecMultiMap.clear(); }

  size_type size() const { return _vecMultiMap.size(); }

  bool empty() const { return _vecMultiMap.empty(); }

  void erase(iterator pos) { _vecMultiMap.erase(pos); }

  void erase(iterator start, iterator end) { _vecMultiMap.erase(start, end); }

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
    iterator iter = _vecMultiMap.erase(countPair.first, countPair.second);
    return count;
  }

  size_type max_size() const { return _vecMultiMap.max_size(); }

  reverse_iterator rbegin() { return _vecMultiMap.rbegin(); }

  const_reverse_iterator rbegin() const { return _vecMultiMap.rbegin(); }

  reverse_iterator rend() { return _vecMultiMap.rend(); }

  const_reverse_iterator rend() const { return _vecMultiMap.rend(); }

  void swap(const VecMultiMap& from) { _vecMultiMap.swap(from._vecMultiMap()); }
};

