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
#include "Pricing/Shopping/FiltersAndPipes/FilterObserverList.h"
#include "Pricing/Shopping/FiltersAndPipes/ICollector.h"
#include "Pricing/Shopping/FiltersAndPipes/IFilter.h"
#include "Pricing/Shopping/FiltersAndPipes/IFilterObserver.h"
#include "Pricing/Shopping/FiltersAndPipes/INamedPredicate.h"

#include <boost/utility.hpp>

#include <vector>

namespace tse
{

namespace utils
{

// A collector filtering elements using registered predicates
// (DP filter)
template <typename T>
class CollectingFilter : public ICollector<T>, public IFilter<T>, boost::noncopyable
{
public:
  // Constructor accepting a destination collector
  // where valid elements are passed
  CollectingFilter(ICollector<T>& destination) : _destination(destination) {}

  // Adds a new predicate to this object
  void addPredicate(INamedPredicate<T>* predicate) override
  {
    TSE_ASSERT(nullptr != predicate);
    _predicates.push_back(predicate);
  }

  // Adds an observer notified about failed elements
  void addObserver(IFilterObserver<T>* observer) override
  {
    _observers.addFilterObserver(observer);
  }

  // Passes t to the destination collector
  // only if it is valid, i.e. all predicates
  // return true for it
  void collect(const T& t) override
  {
    if (isElementValid(t))
    {
      _destination.collect(t);
    }
  }

private:
  // An element is valid only if all registered
  // predicates return true for it
  bool isElementValid(const T& t)
  {
    for (auto& elem : _predicates)
    {
      if (!(*elem)(t))
      {
        _observers.elementInvalid(t, *elem);
        return false;
      }
    }
    return true;
  }

  ICollector<T>& _destination;
  std::vector<INamedPredicate<T>*> _predicates;
  FilterObserverList<T> _observers;
};

} // namespace utils

} // namespace tse

