
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

#include "Pricing/Shopping/FiltersAndPipes/IFilterObserver.h"
#include "Pricing/Shopping/FiltersAndPipes/INamedPredicate.h"

namespace tse
{

namespace utils
{

// An interface for a filter object, passing
// through some elements and discarding some others
// The basic behavior of collecting elements
// is usually realized by implementing ICollector
// by a filter
// This interface exposes the functionality
// of adding predicates and registering observers
// for discarded elements
template <typename T>
class IFilter
{
public:
  // Adds a new predicate to this filter object
  virtual void addPredicate(INamedPredicate<T>* predicate) = 0;

  // Adds an observer notified about failed elements
  virtual void addObserver(IFilterObserver<T>* observer) = 0;

  virtual ~IFilter() {}
};

} // namespace utils

} // namespace tse

