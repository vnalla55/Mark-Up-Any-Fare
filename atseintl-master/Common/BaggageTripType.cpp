//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#include "Common/BaggageTripType.h"

#include "Common/LocUtil.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
const char*
BaggageTripType::getJourneyName(const Itin& itin) const
{
  switch (_btt)
  {
  case TO_FROM_CA:
  case WHOLLY_WITHIN_CA:
      return "CTA";
  case TO_FROM_US:
  case WHOLLY_WITHIN_US:
    return "US DOT";
  case BETWEEN_US_CA:
    if (LocUtil::isBaggageUSTerritory(*itin.firstTravelSeg()->origin()))
      return "US DOT";
    if (LocUtil::isCanada(*itin.firstTravelSeg()->origin()))
      return "CTA";
    return LocUtil::isCanada(*itin.lastTravelSeg()->destination()) ? "US DOT" : "CTA";
  default:
    return "NON US DOT";
  }
}
}
