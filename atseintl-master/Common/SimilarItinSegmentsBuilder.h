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
//-------------------------------------------------------------------
#pragma once

#include <vector>

namespace tse
{
class Itin;
class TravelSeg;
class PricingTrx;

namespace similaritin
{
class SegmentsBuilder
{
public:
  typedef std::vector<TravelSeg*> TSVec;
  typedef std::vector<TSVec> ItinTSVec;

  SegmentsBuilder(const PricingTrx& trx, const Itin& mother, const Itin& similar);
  TSVec constructByOriginAndDestination(const TSVec&) const;
  bool similarItinGeoConsistent() const { return _similarItinGeoConsistent; }

private:
  ItinTSVec _itinTSVec;
  const Itin& _motherItin;
  const Itin& _similarItin;
  bool _similarItinGeoConsistent;
};
}
}

