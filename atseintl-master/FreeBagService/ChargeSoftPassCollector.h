//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/BaggageS7SubvalidatorInterfaces.h"
#include "Util/Algorithm/Bitset.h"

namespace tse
{
class ChargeSoftPassCollector final : public IChargeCollector
{
public:
  ChargeSoftPassCollector(const PricingTrx& trx,
                          const uint32_t freePieces,
                          std::vector<BaggageCharge*>& charges)
    : _remainingBags(alg::create_bitset<MAX_BAG_PIECES>(
          freePieces, trx.getBaggagePolicy().getRequestedBagPieces())),
      _charges(charges)
  {
  }

  bool needAny() const override { return _remainingBags.any(); }

  bool needThis(const BaggageCharge& bc) const override
  {
    return (bc.matchedBags() & _remainingBags).any();
  }

  bool collect(BaggageCharge& bc) override
  {
    bc.mutableMatchedBags() &= _remainingBags;

    if (bc.bagSoftPass().isNull())
      _remainingBags &= ~bc.mutableMatchedBags();
    _charges.push_back(&bc);

    return !needAny();
  }

private:
  std::bitset<MAX_BAG_PIECES> _remainingBags;
  std::vector<BaggageCharge*>& _charges;
};
}
