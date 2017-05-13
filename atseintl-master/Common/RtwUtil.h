//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//
//----------------------------------------------------------------------------
#pragma once

#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
class AncRequest;
class TravelSeg;

namespace RtwUtil
{
struct ScopedRtwDisabler
{
  explicit ScopedRtwDisabler(PricingTrx& trx);
  ~ScopedRtwDisabler();
private:
  PricingOptions* const _prOpt;
  const bool _oldValue;
};

inline bool
isRtw(const PricingTrx& trx)
{
  return trx.getOptions() && trx.getOptions()->isRtw();
}

bool
isRtwArunk(const PricingTrx& trx, const TravelSeg* ts);

bool
isRtwFareMarket(const Itin& itin, const FareMarket* fm);

FareMarket*
getFirstRtwFareMarket(const Itin& itin);

bool
isRtwAncillaryRequest(const AncRequest& req);

} // ns RtwUtil
} // ns tse

