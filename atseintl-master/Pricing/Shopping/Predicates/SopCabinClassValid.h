
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
#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"

#include <boost/utility.hpp>

namespace tse
{

namespace utils
{

// Checks the requirement "cabin class validity"
// for a single SOP
class SopCabinClassValid : public IPredicate<SopCandidate>, boost::noncopyable
{
public:
  explicit SopCabinClassValid(const ShoppingTrx& trx) : _trx(trx) {}

  bool operator()(const SopCandidate& candid) override
  {
    TSE_ASSERT(candid.legId < _trx.legs().size());
    const ShoppingTrx::Leg& aLeg = _trx.legs()[candid.legId];
    TSE_ASSERT(candid.sopId < aLeg.sop().size());
    return (aLeg.sop()[candid.sopId]).cabinClassValid();
  }

private:
  const ShoppingTrx& _trx;
};

} // namespace utils

} // namespace tse

