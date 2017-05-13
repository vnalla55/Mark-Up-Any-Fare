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

#include "FareDisplay/ScheduleCountComparator.h"

#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDisplaySort.h"
#include "DSS/FlightCount.h"
#include "FareDisplay/Group.h"

namespace tse
{
Comparator::Result
ScheduleCountComparator::compare(const PaxTypeFare& l, const PaxTypeFare& r)
{
  if (!_scheduleCounts.empty())
  {
    return compareScheduleCounts(l.carrier(), r.carrier());
  }

  return Comparator::EQUAL;
}

void
ScheduleCountComparator::prepare(const FareDisplayTrx& trx)
{
  if (trx.hasScheduleCountInfo())
  {
    _scheduleCounts = trx.fdResponse()->scheduleCounts();
  }
}

Comparator::Result
ScheduleCountComparator::compareScheduleCounts(const CarrierCode& l, const CarrierCode& r)
{
  const FlightCount* _lFltCnt = getFlightCount(l);
  const FlightCount* _rFltCnt = getFlightCount(r);
  if (_lFltCnt == nullptr or _rFltCnt == nullptr)
    return Comparator::EQUAL;
  if (_lFltCnt->_nonStop != _rFltCnt->_nonStop)
  {
    return compareIndividualCounts(_lFltCnt->_nonStop, _rFltCnt->_nonStop);
  }
  else if (_lFltCnt->_direct != _rFltCnt->_direct)
  {

    return compareIndividualCounts(_lFltCnt->_direct, _rFltCnt->_direct);
  }
  else if (_lFltCnt->_onlineConnection != _rFltCnt->_onlineConnection)
  {
    return compareIndividualCounts(_lFltCnt->_onlineConnection, _rFltCnt->_onlineConnection);
  }

  return Comparator::EQUAL;
}

Comparator::Result
ScheduleCountComparator::compareIndividualCounts(uint16_t l, uint16_t r)
{
  // Carrier with higher counts gets lower rank
  return (l > r ? Comparator::TRUE : Comparator::FALSE);
}

const FlightCount*
ScheduleCountComparator::getFlightCount(const CarrierCode& cxr) const
{
  std::vector<FlightCount*>::const_iterator pos =
      std::find_if(_scheduleCounts.cbegin(),
                   _scheduleCounts.cend(),
                   [cxr](const FlightCount* flightCount)
                   { return flightCount && flightCount->equalCxr(cxr); });

  return (pos != _scheduleCounts.cend() ? *pos : nullptr);
}
}
