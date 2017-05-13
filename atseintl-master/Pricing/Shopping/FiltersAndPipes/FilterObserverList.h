
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
#include "Pricing/Shopping/FiltersAndPipes/IFilterObserver.h"

#include <boost/utility.hpp>

#include <vector>

namespace tse
{

namespace utils
{

// A composite filter observer, notifying all
// added observers about an invalid element
// (DP filter, DP composite)
template <typename T>
class FilterObserverList : public IFilterObserver<T>, boost::noncopyable
{
public:
  void elementInvalid(const T& t, const INamedPredicate<T>& failedPredicate) override
  {
    for (auto& elem : _observers)
    {
      elem->elementInvalid(t, failedPredicate);
    }
  }

  // Add a subobserver object
  void addFilterObserver(IFilterObserver<T>* observer)
  {
    TSE_ASSERT(observer != nullptr);
    _observers.push_back(observer);
  }

private:
  std::vector<IFilterObserver<T>*> _observers;
};

} // namespace utils

} // namespace tse

