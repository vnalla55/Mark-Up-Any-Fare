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
#include "Common/ShoppingUtil.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <boost/utility.hpp>

#include <string>

namespace tse
{

namespace utils
{

// Checks if given carrier is marketing carrier
// for all flights inside given SOP
class SopIsOnlineForCarrier : public IPredicate<SopCandidate>, boost::noncopyable
{
public:
  SopIsOnlineForCarrier(const ShoppingTrx& trx, const CarrierCode& carrierCode)
    : _trx(trx), _carrierCode(carrierCode)
  {
  }

  bool operator()(const SopCandidate& candid) override
  {
    return isSopOnline(candid.legId, candid.sopId);
  }

private:
  bool isSopOnline(unsigned int legId, unsigned int sopId) const
  {
    TSE_ASSERT(legId < _trx.legs().size());
    const std::vector<ShoppingTrx::SchedulingOption>& sops = _trx.legs()[legId].sop();
    TSE_ASSERT(sopId < sops.size());
    const ShoppingTrx::SchedulingOption& sop = sops[sopId];
    const Itin* itin = sop.itin();
    TSE_ASSERT(itin != nullptr);
    return ShoppingUtil::isOnlineFlight(*itin, _carrierCode);
  }

  const ShoppingTrx& _trx;
  CarrierCode _carrierCode;
};

} // namespace utils

} // namespace tse

