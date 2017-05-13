
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
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FiltersAndPipes/INamedPredicate.h"
#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"

#include <string>

namespace tse
{

namespace utils
{

// Wrapper class adding name to an IPredicate object
template <typename T>
class NamedPredicateWrapper : public INamedPredicate<T>
{
public:
  NamedPredicateWrapper(IPredicate<T>* wrapped, const std::string& name)
    : _wrapped(wrapped), _name(name)
  {
    TSE_ASSERT(wrapped != nullptr);
  }

  bool operator()(const T& t) override
  {
    return (*_wrapped)(t);
  }

  std::string getName() const override
  {
    return _name;
  }

private:
  IPredicate<T>* _wrapped;
  std::string _name;
};

// A convenience function attaching a name to an IPredicate object
// trx is passed to enable creation of a new NamedPredicateWrapper object using DataHandle
template <typename T>
NamedPredicateWrapper<T>*
wrapPredicateWithName(IPredicate<T>* toWrap, const std::string& name, ShoppingTrx& trx)
{
  return &trx.dataHandle().safe_create<NamedPredicateWrapper<T>>(toWrap, name);
}

} // namespace utils

} // namespace tse

