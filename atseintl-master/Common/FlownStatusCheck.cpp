//----------------------------------------------------------------------------
//
//  File:  FlownStatusCheck.cpp
//  Description: See FlownStatusCheck.h file
//  Created:  August 15, 2007
//  Authors:
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

#include "Common/FlownStatusCheck.h"

#include "DataModel/FareCompInfo.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
FlownStatusCheck::FlownStatusCheck() : _status(Empty) {}

FlownStatusCheck::~FlownStatusCheck() {}

void
FlownStatusCheck::updateStatus(const std::vector<TravelSeg*>& travelSegs)
{
  if (!travelSegs.empty())
  {
    std::vector<TravelSeg*>::const_iterator i = std::find_if(
        travelSegs.begin(), travelSegs.end(), std::mem_fun<const bool>(&TravelSeg::unflown));

    if (i == travelSegs.end())
      _status = FullyFlown;
    else if (i == travelSegs.begin())
      _status = Unflown;
    else
      _status = PartiallyFlown;
  }
}

FlownStatusCheck::FlownStatusCheck(const Itin& itinerary) : _status(Empty)
{
  updateStatus(itinerary.travelSeg());
}

FlownStatusCheck::FlownStatusCheck(const FareCompInfo& fareCompInfo) : _status(Empty)
{
  updateStatus(fareCompInfo.fareMarket()->travelSeg());
}

FlownStatusCheck::FlownStatusCheck(const PaxTypeFare& paxTypeFare) : _status(Empty)
{
  updateStatus(paxTypeFare.fareMarket()->travelSeg());
}
}
