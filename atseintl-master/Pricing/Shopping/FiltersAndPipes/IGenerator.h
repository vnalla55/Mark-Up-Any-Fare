
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

// An interface for a generator object, i.e.
// an element of "pipes and filters" working
// in "pull" mode.
//
// By convention, if there are no more elements
// to read from a generator, an "empty" element
// is returned - see concrete implementations for
// details.
template <typename T>
class IGenerator
{
public:
  // Returns next element.
  virtual T next() = 0;
  virtual ~IGenerator() {}
};

} // namespace utils

} // namespace tse

