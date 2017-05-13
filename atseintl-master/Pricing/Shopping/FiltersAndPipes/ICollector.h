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

// An interface for a collector object, i.e.
// an element of "pipes and filters" working
// in "push" mode.
template <typename T>
class ICollector
{
public:
  // Collect an element t.
  virtual void collect(const T& t) = 0;
  virtual ~ICollector() {}
};

} // namespace utils

} // namespace tse

