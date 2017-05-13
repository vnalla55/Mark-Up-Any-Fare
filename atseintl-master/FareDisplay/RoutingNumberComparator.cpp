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

#include "FareDisplay/RoutingNumberComparator.h"

#include "Common/TseEnums.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
RoutingNumberComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (_isInternational)
  {
    return compareAllRtgNos(l, r);
  }

  if (l.routingNumber() == r.routingNumber())
    return Comparator::EQUAL;

  else if (_group->sortType() == Group::ASCENDING)
  {
    if (l.routingNumber() < r.routingNumber())
      return Comparator::TRUE;
    if (l.routingNumber() > r.routingNumber())
      return Comparator::FALSE;
  }
  else if (_group->sortType() == Group::DESCENDING)
  {

    if (l.routingNumber() > r.routingNumber())
      return Comparator::TRUE;
    if (l.routingNumber() < r.routingNumber())
      return Comparator::FALSE;
  }

  return Comparator::TRUE;
}

void
RoutingNumberComparator::prepare(const FareDisplayTrx& trx)
{
  if (!trx.itin().empty())
  {
    GeoTravelType geo = trx.itin().front()->geoTravelType();
    _isInternational = (geo == GeoTravelType::International || trx.isSameCityPairRqst());
  }
}

Comparator::Result
RoutingNumberComparator::compareAllRtgNos(const PaxTypeFare& l, const PaxTypeFare& r)
{
  RoutingNumbers lhs(l), rhs(r);

  if (lhs < rhs)
    return _group->sortType() == Group::ASCENDING ? Comparator::TRUE : Comparator::FALSE;
  else if (lhs > rhs)
    return _group->sortType() == Group::ASCENDING ? Comparator::FALSE : Comparator::TRUE;
  return Comparator::EQUAL;
}
}
