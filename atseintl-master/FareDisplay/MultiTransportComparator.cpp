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

#include "FareDisplay/MultiTransportComparator.h"

#include "Common/MultiTransportMarkets.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Group.h"

#include <map>

namespace tse
{
Comparator::Result
MultiTransportComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{

  if (getPriority(l.origin(), l.destination()) < getPriority(r.origin(), r.destination()))
    return Comparator::TRUE;
  if (getPriority(l.origin(), l.destination()) > getPriority(r.origin(), r.destination()))
    return Comparator::FALSE;

  return Comparator::EQUAL;
}

void
MultiTransportComparator::prepare(const FareDisplayTrx& trx)
{
  if (trx.origin() != nullptr && trx.destination() != nullptr)
  {
    _priorityMap.clear();
    _priority = 1;

    std::pair<LocCode, LocCode> requestedMkt(trx.origin()->loc(), trx.destination()->loc());

    if (trx.fareMarket().empty())
    {
      _priorityMap.insert(make_pair(requestedMkt, _priority));
      ++_priority;
      return;
    }

    // Build priority map with all possible markets
    FareMarket* fareMarket = trx.fareMarket().front();
    std::vector<MultiTransportMarkets::Market> markets;

    MultiTransportMarkets multiTransportMkts(fareMarket->origin()->loc(),
                                             fareMarket->destination()->loc(),
                                             fareMarket->governingCarrier(),
                                             fareMarket->geoTravelType(),
                                             trx.ticketingDate(),
                                             trx.travelDate(),
                                             fareMarket);
    multiTransportMkts.getMarkets(markets);

    std::vector<MultiTransportMarkets::Market>::iterator j(markets.begin());

    for (; j != markets.end(); ++j)
    {
      if (*j == requestedMkt)
        _priorityMap.insert(make_pair(*j, 0));
      else
      {
        _priorityMap.insert(make_pair(*j, _priority));
        ++_priority;
      }
    }
  }
}

uint16_t
MultiTransportComparator::getPriority(const LocCode& origin, const LocCode& destination)
{
  std::map<std::pair<LocCode, LocCode>, uint16_t>::const_iterator i = _priorityMap.end();
  i = _priorityMap.find(std::make_pair(origin, destination));

  if (i == _priorityMap.end())
    i = _priorityMap.find(std::make_pair(destination, origin));

  if (i == _priorityMap.end())
  {
    ++_priority;
    _priorityMap.insert(make_pair(std::make_pair(origin, destination), _priority));
    return _priority;
  }
  return i->second;
}
}
