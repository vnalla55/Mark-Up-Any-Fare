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

#include "FareDisplay/TravelDiscontinueDateComparator.h"

#include "Common/DateTime.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelInfo.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
TravelDiscontinueDateComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{

  bool l_hasCat14 = hasCat14(l);
  bool r_hasCat14 = hasCat14(r);

  if (l_hasCat14 && r_hasCat14)
  {
    return compareDate(l, r);
  }
  else
    return compareTravelTicketInfo(l_hasCat14, r_hasCat14);
}

void
TravelDiscontinueDateComparator::prepare(const FareDisplayTrx& trx)
{
  if (trx.ticketingDate().isValid())
  {
    _ticketingDate = trx.ticketingDate();
  }
  else
  {
    _ticketingDate = DateTime::localTime();
  }
  _emptyDate = DateTime::emptyDate();
}

const DateTime&
TravelDiscontinueDateComparator::getDate(const PaxTypeFare& p) const
{
  const FareDisplayInfo* info = p.fareDisplayInfo();

  if (info)
  {
    if (!info->travelInfo().empty())
    {
      if (info->travelInfo().front()->latestTvlStartDate().isValid())
      {
        return info->travelInfo().front()->latestTvlStartDate();
      }
      else if (info->travelInfo().front()->stopDate().isValid())
      {
        return info->travelInfo().front()->stopDate();
      }
    }
  }
  return _emptyDate;
}

bool
TravelDiscontinueDateComparator::hasCat14(const PaxTypeFare& p) const
{
  const FareDisplayInfo* info = p.fareDisplayInfo();

  if (info)
  {
    if (info->travelInfo().empty())
    {
      return false;
    }

    if (!info->travelInfo().front()->latestTvlStartDate().isValid() &&
        !info->travelInfo().front()->stopDate().isValid())
    {
      return false;
    }
  }
  return true;
}

Comparator::Result
TravelDiscontinueDateComparator::compareDate(const PaxTypeFare& l, const PaxTypeFare& r) const

{
  const DateTime& lDate = getDate(l);
  const DateTime& rDate = getDate(r);

  if (lDate < rDate)
  {
    return _group->sortType() == Group::ASCENDING ? Comparator::Result::TRUE
                                                  : Comparator::Result::FALSE;
  }
  else if (lDate > rDate)
  {
    return _group->sortType() == Group::DESCENDING ? Comparator::Result::TRUE
                                                   : Comparator::Result::FALSE;
  }

  return Comparator::Result::EQUAL;
}

Comparator::Result
TravelDiscontinueDateComparator::compareTravelTicketInfo(const bool& l, const bool& r) const
{

  if (l)
  {
    return _group->sortType() == Group::ASCENDING ? Comparator::Result::TRUE
                                                  : Comparator::Result::FALSE;
  }
  else if (r)
  {
    return _group->sortType() == Group::DESCENDING ? Comparator::Result::TRUE
                                                   : Comparator::Result::FALSE;
  }
  else

    return Comparator::Result::EQUAL;
}

uint16_t
TravelDiscontinueDateComparator::diffTime(const DateTime& tvlDiscDate) const
{
  return DateTime::diffTime(tvlDiscDate, _ticketingDate);
}
}
