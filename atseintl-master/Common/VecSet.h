//-------------------------------------------------------------------
//
//  File:        VecSet.h
//  Created:     July 26, 2006
//  Authors:     Kavya Katam
//
//  Description: Interface is designed to be similar to std::set
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

// Documentation: VecSet is an interface which is designed to be similar to std::set. Implementation
// is based on a sorted
// vector. VecSet do not allocate any memory at all if they don't grow beyond empty, while the
// std::set
// always allocate one node in their constructor. VecSet works well than std::set if they are of a
// small size.

// Insert : inserts in a sorted vector. So the time taken to insert is log(n) which is faster
// when compared to std::set.

// Find : the time taken to find is log(n) which is faster than std::set.

// Count : as the vector is sorted difference between the iterators gives the exact count of the
// key. Time taken is
// logarthmic time which is faster than std::set.

// Erase : as the vector is sorted, time taken to erase is logarthmic time which is faster than
// std::set.

#pragma once

#include <algorithm>
#include <vector>

#include <sys/types.h>

template <class T>
class VecSet
{
public:
  typedef std::vector<T> Vec;
  typedef typename Vec::iterator iterator;
  typedef std::pair<iterator, iterator> vecPair;
  typedef std::pair<iterator, bool> insertPair;
  typedef typename Vec::const_iterator const_iterator;
  typedef std::pair<const_iterator, const_iterator> const_vecPair;
  typedef typename Vec::size_type size_type;
  typedef typename Vec::reverse_iterator reverse_iterator;
  typedef typename Vec::const_reverse_iterator const_reverse_iterator;

  VecSet() {}

  // Operators overloaded
  bool operator==(const VecSet& c2) const { return this->_vecSet == c2._vecSet; }

  bool operator!=(const VecSet& c2) const { return this->_vecSet != c2._vecSet; }

  bool operator<(const VecSet& c2) const { return this->_vecSet < c2._vecSet; }

  bool operator>(const VecSet& c2) const { return this->_vecSet > c2._vecSet; }

  bool operator<=(const VecSet& c2) const { return this->_vecSet <= c2._vecSet; }

  bool operator>=(const VecSet& c2) const { return this->_vecSet >= c2._vecSet; }

private:
  // Vector member variable
  Vec _vecSet;

public:
  // insert into vector using insert function
  iterator insert(iterator pos, const T& t) { return insert(t); }

  // using std::equal_range which returns a pair of iterators the first iterator in this pair
  // is the position to insert in vector, the two conditions maintain uniqueness in vector are
  // If the set is empty (OR) If the set has atleast one item. the set here is the vector used
  // internally.
  // a pair which states whether the value is inserted(true or false) is returned back.
  // Inserts in a sorted vector, Set uses a binary tree to insert
  // TimeComplexity: log(n)
  insertPair insert(const T& key)
  {
    bool bExist = false;
    vecPair myPair = std::equal_range(begin(), end(), key);
    iterator pos = myPair.first;
    if (pos == end() || *pos != key)
    {
      iterator iter = _vecSet.insert(pos, key);
      pos = iter;
      bExist = true;
    }
    insertPair retPair = make_pair(pos, bExist);
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
  void insert(InputIterator first, InputIterator last)
  {
    _vecSet.insert(end(), first, last);
    std::stable_sort(begin(), end());
    _vecSet.erase(unique(begin(), end()), _vecSet.end());
  }

  iterator lower_bound(const T& key)
  {
    iterator location = std::lower_bound(begin(), end(), key);
    return location;
  }

  iterator lower_bound(const T& key) const
  {
    iterator location = std::lower_bound(begin(), end(), key);
    return location;
  }

  iterator upper_bound(const T& key)
  {
    iterator location = std::upper_bound(begin(), end(), key);
    return location;
  }

  iterator upper_bound(const T& key) const
  {
    iterator location = std::upper_bound(begin(), end(), key);
    return location;
  }

  // using std::equal_range which returns a pair of iterators if both the iterators are equal
  // then there is no key in the multiset(vector in our case) so return end of the vector
  // or else return the first iterator in the pair which is the key we are looking for.
  // find the item in multiset in logarthmic time
  iterator find(const T& key)
  {
    vecPair findPair = std::equal_range(begin(), end(), key);
    if (findPair.first == findPair.second)
      return end();
    else
      return findPair.first;
  }

  const_iterator find(const T& key) const
  {
    const_vecPair findPair = std::equal_range(begin(), end(), key);
    if (findPair.first == findPair.second)
      return end();
    else
      return findPair.first;
  }

  iterator begin() { return _vecSet.begin(); }

  const_iterator begin() const { return _vecSet.begin(); }

  iterator end() { return _vecSet.end(); }

  const_iterator end() const { return _vecSet.end(); }

  void clear() { _vecSet.clear(); }

  // using std::equal_range which returns a pair of iterators difference between these iterators
  // gives the count of the key, as the vector is sorted difference between the iterators gives the
  // exact
  // count of the key.
  // counts in logarthmic time
  size_type count(const T& key) const
  {
    size_type count = 0;
    vecPair countPair = std::equal_range(begin(), end(), key);
    count = countPair.second - countPair.first;
    return count;
  }

  // return the size of the vector
  size_type size() const { return _vecSet.size(); }

  // empty the vector.
  bool empty() const { return _vecSet.empty(); }

  // erase the vector at the give pos/iterator.
  void erase(iterator pos) { _vecSet.erase(pos); }

  void erase(iterator start, iterator end) { _vecSet.erase(start, end); }

  // using std::equal_range which returns a pair of iterators difference between these iterators
  // gives the count of the key, as the vector is sorted difference between the iterators gives the
  // exact count.
  // using the first and second iterators from the pair, use the vector erase function to erase the
  // keys.
  // erases in logarthmic time as the inner vector is already sorted
  size_type erase(const T& key)
  {
    size_type count = 0;
    vecPair countPair = std::equal_range(begin(), end(), key);
    count = countPair.second - countPair.first;
    iterator iter = _vecSet.erase(countPair.first, countPair.second);
    return count;
  }

  size_type max_size() const { return _vecSet.max_size(); }

  vecPair equal_range(const T& key)
  {
    vecPair pair = std::equal_range(begin(), end(), key);
    return pair;
  }

  const_vecPair equal_range(const T& key) const
  {
    const_vecPair pair = std::equal_range(begin(), end(), key);
    return pair;
  }

  reverse_iterator rbegin() { return _vecSet.rbegin(); }

  const_reverse_iterator rbegin() const { return _vecSet.rbegin(); }

  reverse_iterator rend() { return _vecSet.rend(); }

  const_reverse_iterator rend() const { return _vecSet.rend(); }

  void swap(const VecSet& from) { _vecSet.swap(from._vecSet()); }
};

