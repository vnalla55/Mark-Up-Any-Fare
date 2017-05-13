//-------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/GlobalDirectionFinder.h"

namespace tse
{
class PricingTrx;
class FareMarket;

namespace GlobalDirectionFinderV2Adapter
{
  using Location = GlobalDirectionFinder::Location;

  bool
  process(PricingTrx& trx, FareMarket& fareMarket);

  bool
  getGlobalDirection(const PricingTrx* trx,
                     DateTime travelDate,
                     TravelSeg& tvlSeg,
                     GlobalDirection& globalDir);

  bool
  getGlobalDirection(const PricingTrx* trx,
                     DateTime travelDate,
                     const std::vector<TravelSeg*>& tvlSegs,
                     GlobalDirection& globalDir);
}
}
