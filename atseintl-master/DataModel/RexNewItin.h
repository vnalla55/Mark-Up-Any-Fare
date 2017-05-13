//-------------------------------------------------------------------
//
//  File:        RexNewItin.h
//  Created:     March 16, 2009
//  Authors:     Grzegorz Cholewiak
//
//  Description: The Itinerary class object represents one
//               itinerary from incoming shopping/pricing request.
//
//  Updates:

//
//  Copyright Sabre 2009
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

#include "DataModel/Itin.h"

#include <map>

namespace tse
{
class DateTime;
class RexBaseTrx;
class TravelSeg;

class RexNewItin : public Itin
{

public:
  const RexBaseTrx*& rexTrx() { return _rexTrx; }
  const RexBaseTrx* rexTrx() const { return _rexTrx; }
  const DateTime& travelDate() const override;
  std::map<TravelSeg*, TravelSeg*>& excToNewUnchangedAndInventoryChangedTravelSegmentMap()
  {
    return _excToNewUnchangedAndInventoryChangedTravelSegmentMap;
  }
  const std::map<TravelSeg*, TravelSeg*>&
  excToNewUnchangedAndInventoryChangedTravelSegmentMap() const
  {
    return _excToNewUnchangedAndInventoryChangedTravelSegmentMap;
  }

private:
  const RexBaseTrx* _rexTrx = nullptr;
  std::map<TravelSeg*, TravelSeg*> _excToNewUnchangedAndInventoryChangedTravelSegmentMap;
};

} // tse namespace
