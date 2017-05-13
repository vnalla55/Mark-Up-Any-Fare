//-------------------------------------------------------------------
//
//  File:        CartesianProduct.h
//  Created:     August 9, 2007
//  Authors:     Grzegorz Cholewiak
//
//  Updates:
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

#pragma once

#include <deque>
#include <map>
#include <vector>

namespace tse
{

template <class Set>
class CartesianProduct
{
  typedef typename Set::iterator IteratorType;

public:
  typedef std::deque<typename Set::value_type> ProductType;

  CartesianProduct() : _nOfSets(0), _size(0), _currentElementID(0) {}

  ~CartesianProduct()
  {
    typename std::vector<SetElementSelector*>::const_iterator i = _selectors.begin();

    while (i != _selectors.end())
      delete (*i++);
  }

  size_t size() const { return _size; }

  void addSet(Set& s)
  {
    _size = _nOfSets ? _size * s.size() : s.size();
    _nOfSets++;
    _selectors.push_back(new SetElementSelector(s.begin(), s.end()));
  }

  ProductType getNext()
  {
    ProductType result;
    bool carry = true;
    typedef typename std::vector<SetElementSelector*>::reverse_iterator It;
    if (++_currentElementID <= _size)
      for (It i = _selectors.rbegin(); i != _selectors.rend(); i++)
      {
        result.push_front((*(*i)->current()));
        if (carry)
          carry = (*i)->shift();
      }
    return result;
  }

private:
  size_t _nOfSets;
  size_t _size;
  size_t _currentElementID;

  class SetElementSelector
  {
  public:
    SetElementSelector(const IteratorType& begin, const IteratorType& end)
      : _begin(begin), _end(end), _current(begin)
    {
    }

    ~SetElementSelector() {}

    bool shift()
    {
      bool allVisited = (++_current == _end);
      if (allVisited)
        _current = _begin;
      return allVisited;
    }

    typename Set::pointer current() const { return &(*_current); }

  private:
    const IteratorType _begin;
    const IteratorType _end;
    IteratorType _current;
  };

  std::vector<SetElementSelector*> _selectors;
};
}

