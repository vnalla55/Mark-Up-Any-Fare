//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <boost/utility.hpp>

namespace tse
{

class ShoppingTrx;

namespace utils
{

// Checks the requirement "positive connect time"
// for flight-only solution
class PositiveConnectTime : public IPredicate<SopCombination>, boost::noncopyable
{
public:
  explicit PositiveConnectTime(const ShoppingTrx& trx) : _trx(trx) {}

  bool operator()(const SopCombination& sopIds) override;

private:
  const ShoppingTrx& _trx;
};

} // namespace utils

} // namespace tse

