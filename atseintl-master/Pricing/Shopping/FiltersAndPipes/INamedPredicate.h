
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

#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"

#include <string>

namespace tse
{

namespace utils
{

// An interface to a predicate object returning
// a result of a boolean function on element t
template <typename T>
class INamedPredicate : public IPredicate<T>
{
public:
  // Returns a result of a boolean function on element t
  bool operator()(const T& t)override = 0;

  // Returns the name of this predicate as a string
  virtual std::string getName() const = 0;

  virtual ~INamedPredicate() {}
};

} // namespace utils

} // namespace tse

