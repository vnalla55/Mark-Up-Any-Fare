//-------------------------------------------------------------------
//  Created:Jul 1, 2005
//  Author:Abu
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/OneWayRoundTripComparator.h"

#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
OneWayRoundTripComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (l.owrt() == r.owrt())
    return Comparator::EQUAL;

  if (_group->sortType() == Group::ONE_WAY_OVER_ROUND_TRIP)
  {
    if (!l.isRoundTrip())
      return TRUE;
    if (!r.isRoundTrip())
      return FALSE;
  }
  else if (_group->sortType() == Group::ROUND_TRIP_OVER_ONE_WAY)
  {
    if (l.isRoundTrip())
      return TRUE;
    if (r.isRoundTrip())
      return FALSE;
  }
  return Comparator::EQUAL;
}

void
OneWayRoundTripComparator::prepare(const FareDisplayTrx& trx)
{
}
}
