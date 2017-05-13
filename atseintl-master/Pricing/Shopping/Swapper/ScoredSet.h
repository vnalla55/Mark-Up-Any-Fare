//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"
#include "Pricing/Shopping/Utils/StreamFormat.h"

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/unordered_map.hpp>

#include <iostream>
#include <string>

namespace tse
{

namespace swp
{

// Element comparison policy class for ScoredSet
// Use this policy to build a set having the max
// element in 'top'
template <typename S>
class MaxComparePolicy
{
public:
  static bool compare(const S& left, const S& right) { return left < right; }
};

// Element comparison policy class for ScoredSet
// Use this policy to build a set having the min
// element in 'top'
template <typename S>
class MinComparePolicy
{
public:
  static bool compare(const S& left, const S& right) { return right < left; }
};

// A tuple (key, value) which is stored in ScoredSet
// Tuples are compared using a custom policy "ComparePolicyType"
template <typename T, typename S, typename ComparePolicyType>
struct HeapItem
{
  HeapItem(const T& t, const S& s) : key(t), score(s) {}

  bool operator<(const HeapItem& rhs) const { return ComparePolicyType::compare(score, rhs.score); }

  T key;
  S score;
};

// This container maintains a number of pairs (element, score)
// It supports effectively following operations:
// a) adding a pair (element, score)
// b) updating the score for an element
// c) identification/removal of element with top score
//
// It has been built on the basis of fast heap structure and hash map:
// - hash map alone does not support efficient top element search
// - heap alone does not support efficient search for an arbitrary element
// - hash map containing handles for corresponding (element, score)
// tuples in heap efficiently implements operations a), b), c).
//
// For time complexity notes, see method descriptions
template <typename T, typename S, typename ComparePolicyType = MaxComparePolicy<S> >
class ScoredSet
{
public:
  typedef HeapItem<T, S, ComparePolicyType> ImplHeapItem;
  typedef typename boost::heap::fibonacci_heap<ImplHeapItem>::ordered_iterator Iterator;

  // Adds element with given score to this container
  // Throws on element duplicate
  // Constant time
  void add(const T& element, const S& score)
  {
    // ensure we have no duplicates (O(1) on hash map)
    TSE_ASSERT(_heapHandleMap.find(element) == _heapHandleMap.end());

    ImplHeapItem item(element, score);

    // O(1)
    HeapHandle handle = _heap.push(item);

    // Push the handle to the map in order to
    // reach it later for score modification
    // in O(1)
    _heapHandleMap[element] = handle;
  }

  // Sets a new score for the given element
  // Throws if element not found
  // Amortized constant time, worst case logarithmic
  void updateScore(const T& element, const S& score)
  {
    TSE_ASSERT(_heapHandleMap.find(element) != _heapHandleMap.end());

    HeapHandle& handle = _heapHandleMap.at(element);
    // Modify score inside heap item
    // stored in the heap. This requires immediate
    // update of the heap.
    (*handle).score = score;
    _heap.update(handle);
  }

  // Tells whether an element exists in this set
  bool hasElement(const T& element) const
  {
    return _heapHandleMap.find(element) != _heapHandleMap.end();
  }

  // Returns the score for a given element
  // Throws if element not found
  // Constant time
  const S& getScore(const T& element) const
  {
    TSE_ASSERT(_heapHandleMap.find(element) != _heapHandleMap.end());

    // Take heap handle from hash map
    // and the dereference it to get the heap item
    return (*_heapHandleMap.at(element)).score;
  }

  // Returns the element with top score
  // Throws if set empty
  // Constant time
  const T& top() const
  {
    TSE_ASSERT(!_heap.empty());
    return _heap.top().key;
  }

  // Removes the element with top score
  // Throws if set empty
  // Amortized logarithmic time, worst linear
  void pop()
  {
    TSE_ASSERT(!_heap.empty());
    // First, get the top key to find and remove
    // its corresponding handle in the handle map
    unsigned int erased = _heapHandleMap.erase(_heap.top().key);
    TSE_ASSERT(erased == 1);
    _heap.pop();
  }

  // Tells if this set is empty
  // Constant time
  bool isEmpty() const { return _heap.empty(); }

  // Returns the number of elements in this set
  // Constant time
  unsigned int getSize() const { return static_cast<unsigned int>(_heap.size()); }

  // Returns an iterator to the first tuple (key, value)
  // of type HeapItem in this set (iteration is ordered).
  Iterator begin() const { return _heap.ordered_begin(); }

  // Returns an iterator after the last tuple (key, value)
  // of type HeapItem in this set (iteration is ordered)
  Iterator end() const { return _heap.ordered_end(); }

private:
  typedef typename boost::heap::fibonacci_heap<ImplHeapItem> Heap;
  typedef typename Heap::handle_type HeapHandle;
  typedef boost::unordered_map<T, HeapHandle> HeapHandleMap;

  Heap _heap;
  HeapHandleMap _heapHandleMap;
};

template <typename T, typename S, typename ComparePolicyType>
void
formatScoredSet(std::ostream& out, const ScoredSet<T, S, ComparePolicyType>& s)
{
  out << "Scored Set with " << s.getSize() << " elements in order:" << std::endl;
  for (typename ScoredSet<T, S, ComparePolicyType>::Iterator it = s.begin(); it != s.end(); ++it)
  {
    out << it->key << ": " << it->score << std::endl;
  }
}

template <typename T, typename S, typename ComparePolicyType>
std::ostream& operator<<(std::ostream& out, const ScoredSet<T, S, ComparePolicyType>& s)
{
  out << utils::format(s, formatScoredSet<T, S, ComparePolicyType>);
  return out;
}

} // namespace swp

} // namespace tse

