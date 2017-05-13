// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Common/ShoppingUtil.h"
#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"
#include "Pricing/Shopping/FOS/FosTypes.h"

namespace tse
{
class ShoppingTrx;

namespace fos
{

class OnlineFlightPredicate : public tse::utils::IPredicate<SopCombination>
{
public:
  explicit OnlineFlightPredicate(const ShoppingTrx& trx) : _trx(trx) {}

  bool operator()(const SopCombination& sopIds) override
  {
    return ShoppingUtil::isOnlineFlightSolution(_trx, sopIds);
  }

private:
  const ShoppingTrx& _trx;
};

} // fos
} // tse
