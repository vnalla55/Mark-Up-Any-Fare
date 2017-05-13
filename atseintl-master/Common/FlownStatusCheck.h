//----------------------------------------------------------------------------
//
//  File:        FlownStatusCheck.h
//  Created:     8/15/2007
//  Authors:
//
//  Description: Common functions required for ATSE shopping/pricing.
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include <vector>

namespace tse
{
class FareCompInfo;
class PaxTypeFare;
class Itin;
class TravelSeg;

class FlownStatusCheck
{

public:
  FlownStatusCheck();
  ~FlownStatusCheck();

  enum FlownStatus
  { Empty,
    FullyFlown,
    PartiallyFlown,
    Unflown };

  FlownStatusCheck(const Itin& itinerary);
  FlownStatusCheck(const FareCompInfo& fareCompInfo);
  FlownStatusCheck(const PaxTypeFare& paxTypeFare);

  inline bool isTotallyFlown() { return FullyFlown == _status; }
  inline bool isTotallyUnflown() { return Unflown == _status; }
  inline bool isPartiallyFlown() { return PartiallyFlown == _status; }
  inline FlownStatus status() { return _status; }

protected:
  void updateStatus(const std::vector<TravelSeg*>& travelSegs);

private:
  FlownStatus _status;
};

} // end tse namespace

