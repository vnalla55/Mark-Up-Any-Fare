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

#include <vector>

namespace tse
{

namespace utils
{

// A collector passing each element to all its added sub-collectors
// (DP filter, DP composite)
template <typename T>
class Fork : public ICollector<T>, boost::noncopyable
{
public:
  // Add a subcollector object
  void addCollector(ICollector<T>* collector)
  {
    TSE_ASSERT(collector != nullptr);
    _collectors.push_back(collector);
  }

  // Pass t to all added subcollectors
  void collect(const T& t) override
  {
    for (auto& elem : _collectors)
    {
      elem->collect(t);
    }
  }

private:
  std::vector<ICollector<T>*> _collectors;
};

} // namespace utils

} // namespace tse

