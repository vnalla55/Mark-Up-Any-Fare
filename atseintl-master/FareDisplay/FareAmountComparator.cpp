//-------------------------------------------------------------------
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

#include "FareDisplay/FareAmountComparator.h"

#include "Common/FareDisplayUtil.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDisplaySort.h"
#include "FareDisplay/ComparatorFactory.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/ScheduleCountComparator.h"

namespace tse
{
Comparator::Result
FareAmountComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (l.owrt() == r.owrt())
  {

    if (l.convertedFareAmount() < r.convertedFareAmount())
    {
      return _group->sortType() == Group::ASCENDING ? Comparator::TRUE : Comparator::FALSE;
    }
    if (l.convertedFareAmount() > r.convertedFareAmount())
    {
      return _group->sortType() == Group::ASCENDING ? Comparator::FALSE : Comparator::TRUE;
    }
  }
  else
  {
    if (fareAmount(l) < fareAmount(r))
    {
      return _group->sortType() == Group::ASCENDING ? Comparator::TRUE : Comparator::FALSE;
    }
    if (fareAmount(l) > fareAmount(r))
    {
      return _group->sortType() == Group::ASCENDING ? Comparator::FALSE : Comparator::TRUE;
    }
  }

  return compareSchedCnt(l, r);
}

bool
FareAmountComparator::isDoubleOWFare()
{
  if (_group->sortData() != nullptr)
  {
    return _group->sortData()->doubleOWFares() == YES;
  }
  return true;
}

MoneyAmount
FareAmountComparator::fareAmount(const PaxTypeFare& p)
{
  if (p.isRoundTrip())
    return p.convertedFareAmount();
  else
  {
    if (!isDoubleOWFare())
      return p.convertedFareAmount();
  }
  return (p.convertedFareAmount() * FACTOR);
}

void
FareAmountComparator::prepare(const FareDisplayTrx& trx)
{
  if (trx.isShopperRequest())
  {
    ComparatorFactory rVFactory(trx);
    Group::GroupType grpType = Group::GROUP_BY_SCHEDULE_COUNT;
    _scheduleCountComparator = rVFactory.getComparator(grpType);
    if (_scheduleCountComparator != nullptr)
      _scheduleCountComparator->prepare(trx);
  }

  setSortOption(trx);
}

Comparator::Result
FareAmountComparator::compareSchedCnt(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (l.carrier() == r.carrier() || _scheduleCountComparator == nullptr)
  {
    return Comparator::EQUAL;
  }

  return _scheduleCountComparator->compare(l, r);
}

bool
FareAmountComparator::isSameDirectionality(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (l.owrt() == r.owrt())
  {
    return true;
  }
  else if (!l.isRoundTrip() && !r.isRoundTrip())
  {

    return true;
  }
  return false;
}

void
FareAmountComparator::setSortOption(const FareDisplayTrx& trx)
{
  if (_group != nullptr)
  {
    if (trx.getOptions()->sortAscending() || trx.getRequest()->numberOfFareLevels() > 0)
    {
      _group->sortType() = Group::ASCENDING;
    }
    else if (trx.getOptions()->sortDescending())
    {

      _group->sortType() = Group::DESCENDING;
    }
    else
    {
    }
  }
}
}
