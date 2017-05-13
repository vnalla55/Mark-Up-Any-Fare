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

#include "Pricing/Shopping/FiltersAndPipes/INamedPredicate.h"

namespace tse
{

namespace utils
{

// This interface is implemented by classes that want
// to be notified about elements which were discarded
// by a filter as invalid.
template <typename T>
class IFilterObserver
{
public:
  // Notifies about invalid element t,
  // togehter with the failed predicate for t.
  virtual void elementInvalid(const T& t, const INamedPredicate<T>& failedPredicate) = 0;
  virtual ~IFilterObserver() {}
};

} // namespace utils

} // namespace tse

