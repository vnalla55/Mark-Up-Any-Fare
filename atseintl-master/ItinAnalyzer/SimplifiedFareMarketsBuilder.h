//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"

#include <vector>
#include <utility>
#include <set>

namespace tse {

class PricingTrx;
class Itin;
class TravelSeg;
class SegmentAttributes;

class SimplifiedFareMarketsBuilder
{
public:
  struct FareMarketParams
  {
    std::vector<SegmentAttributes>::iterator begin;
    std::vector<SegmentAttributes>::iterator end;
  };

  SimplifiedFareMarketsBuilder(PricingTrx& trx, Itin& itin);
  std::vector<FareMarketParams> build(std::vector<SegmentAttributes>& segmentAttributes);
  static bool isComplexItin(const Itin& itin);

private:
  PricingTrx& _trx;
  Itin& _itin;
};

} //tse

