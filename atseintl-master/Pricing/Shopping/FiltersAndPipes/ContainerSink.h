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
#include "Pricing/Shopping/FiltersAndPipes/ICollector.h"

#include <boost/utility.hpp>

namespace tse
{

namespace utils
{

// Stores collected elements in a container,
// using supplied container output iterator
// (DP filters and pipes)
template <typename T, typename OutputIterator_>
class ContainerSink : public ICollector<T>, boost::noncopyable
{
public:
  ContainerSink(const OutputIterator_& it) : _it(it) {}

  // Adds t to container using output iterator
  // given in constructor
  void collect(const T& t) override
  {
    *_it++ = t;
  }

private:
  OutputIterator_ _it;
};

} // namespace utils

} // namespace tse

