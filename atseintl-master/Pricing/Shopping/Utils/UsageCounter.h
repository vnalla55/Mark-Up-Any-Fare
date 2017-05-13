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

#include <iostream>
#include <map>
#include <set>

namespace tse
{

namespace utils
{

template <typename T>
class UsageCounter;

// Formats UsageCounter to put it into an output stream
template <typename T>
class UCounterBasicFormatter
{
public:
  void format(const UsageCounter<T>& counter, std::ostream& out)
  {
    out << "{UsageCounter total elements: " << counter.getNbrOfElements()
        << ", unused elements: " << counter.getNbrOfUnusedElements() << "\n";

    out << "Set of unused elements:\n";
    const typename UsageCounter<T>::ElementSet& unused = counter.getUnusedElements();
    for (typename UsageCounter<T>::ElementSet::const_iterator it = unused.begin();
         it != unused.end();
         ++it)
    {
      out << (*it) << std::endl;
    }

    out << "Usage count map:\n";

    const typename UsageCounter<T>::UsageCountMap& usages = counter.getUsageCountMap();
    for (typename UsageCounter<T>::UsageCountMap::const_iterator it = usages.begin();
         it != usages.end();
         ++it)
    {
      out << "element: " << it->first << "\tusages: " << it->second << std::endl;
    }
    out << "}";
  }
};

// Counts the number of usages for
// added elements. Allows to increase,
// decrease and check the usage number for an
// element. Also lets the user quickly examine
// the set of unused elements.
template <typename T>
class UsageCounter
{

public:
  typedef std::set<T> ElementSet;
  typedef std::map<T, unsigned int> UsageCountMap;

  UsageCounter() {}

  void addElement(const T& elem);

  // Returns the number of usages for
  // an element after increase
  unsigned int increaseUsageCount(const T& elem);

  // Returns the number of usages for
  // an element after decrease
  unsigned int decreaseUsageCount(const T& elem);

  // Returns the number of usages for
  // an element
  unsigned int getUsageCount(const T& elem) const;

  // Returns the number of all elements
  unsigned int getNbrOfElements() const;

  // Returns the number of elements with usage zero
  unsigned int getNbrOfUnusedElements() const;

  // Returns all added elements
  ElementSet getAllElements() const;

  // Returns the set of elements with usage zero
  const ElementSet& getUnusedElements() const;

  // Returns the usage map for elements with non-zero usage
  const UsageCountMap& getUsageCountMap() const;

  // Formats the object and dumps its
  // representation to a given stream
  void toStream(std::ostream& out) const;

private:
  UsageCounter(const UsageCounter& right);
  UsageCounter& operator=(const UsageCounter& right);

  // Here are stored elements
  // that have no usages
  ElementSet _unusedElems;

  // If an element has at least one usage,
  // it is transferred to this container
  UsageCountMap _usages;
};

template <typename T>
void
UsageCounter<T>::addElement(const T& elem)
{
  _unusedElems.insert(elem);
}

template <typename T>
unsigned int
UsageCounter<T>::increaseUsageCount(const T& elem)
{
  typename UsageCountMap::iterator it = _usages.find(elem);
  // If element has some usages, simply increase the counter
  if (it != _usages.end())
  {
    unsigned int& u = it->second;
    u += 1;
    return u;
  }

  // If no usages yet exist for an element, transfer it
  // to the usage map.
  typename ElementSet::iterator sit = _unusedElems.find(elem);
  if (sit == _unusedElems.end())
  {
    TSE_ASSERT(!"Trying to increase usage count for non-tracked element");
    return 0;
  }

  _unusedElems.erase(sit);
  _usages.insert(std::make_pair(elem, 1));
  return 1;
}

template <typename T>
unsigned int
UsageCounter<T>::decreaseUsageCount(const T& elem)
{
  typename UsageCountMap::iterator it = _usages.find(elem);
  // You cant decrease usage for element with no usage
  if (it == _usages.end())
  {
    TSE_ASSERT(!"Trying to decrease usage count for element with no usage");
    return 0;
  }

  unsigned int& u = it->second;
  if (u == 1)
  {
    // No usage for element now
    // Transfer it to _unusedElems
    _usages.erase(it);
    _unusedElems.insert(elem);
    return 0;
  }

  u -= 1;
  return u;
}

template <typename T>
unsigned int
UsageCounter<T>::getUsageCount(const T& elem) const
{
  typename UsageCountMap::const_iterator it = _usages.find(elem);
  if (it != _usages.end())
  {
    return it->second;
  }

  // Make sure that the element is tracked
  TSE_ASSERT(_unusedElems.find(elem) != _unusedElems.end());
  return 0;
}

template <typename T>
unsigned int
UsageCounter<T>::getNbrOfElements() const
{
  return static_cast<unsigned int>(_unusedElems.size() + _usages.size());
}

template <typename T>
unsigned int
UsageCounter<T>::getNbrOfUnusedElements() const
{
  return static_cast<unsigned int>(_unusedElems.size());
}

template <typename T>
typename UsageCounter<T>::ElementSet
UsageCounter<T>::getAllElements() const
{
  ElementSet s(_unusedElems);
  for (const auto& elem : _usages)
  {
    s.insert(elem.first);
  }
  return s;
}

template <typename T>
const typename UsageCounter<T>::ElementSet&
UsageCounter<T>::getUnusedElements() const
{
  return _unusedElems;
}

template <typename T>
const typename UsageCounter<T>::UsageCountMap&
UsageCounter<T>::getUsageCountMap() const
{
  return _usages;
}

template <typename T>
void
UsageCounter<T>::toStream(std::ostream& out) const
{
  UCounterBasicFormatter<T> formatter;
  formatter.format(*this, out);
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const UsageCounter<T>& counter)
{
  counter.toStream(out);
  return out;
}

} // namespace utils

} // namespace tse

