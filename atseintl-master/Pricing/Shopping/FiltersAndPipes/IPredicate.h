
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

namespace tse
{

namespace utils
{

// An interface to a predicate object returning
// a result of a boolean function on element t
template <typename T>
class IPredicate
{
public:
  // Returns a result of a boolean function on element t
  virtual bool operator()(const T& t) = 0;

  virtual ~IPredicate() {}
};

} // namespace utils

} // namespace tse

