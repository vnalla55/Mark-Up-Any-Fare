/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Util/FlatMap.h"

#include <utility>

namespace tse
{
struct MaxStayData
{
  LocCode _location = LocCode();
  DateTime _mustCommence = DateTime::openDate();
  DateTime _mustComplete = DateTime::openDate();

  MaxStayData(LocCode location, DateTime mustCommence, DateTime mustComplete)
    : _location(location), _mustCommence(mustCommence), _mustComplete(mustComplete)
  {
  }
  MaxStayData() = default;
  bool operator==(const MaxStayData& ref) const
  {
    return (_location == ref._location && _mustCommence == ref._mustCommence &&
            _mustComplete == ref._mustComplete);
  }
  void setCommenceWithEarlier(DateTime mustCommence) { _mustCommence.setWithEarlier(mustCommence); }
  void setCompleteWithEarlier(DateTime mustComplete) { _mustComplete.setWithEarlier(mustComplete); }
};

using SegmentOrder = uint16_t;
using MinStayMap = FlatMap<SegmentOrder, std::pair<DateTime, LocCode>>;
using MinStayMapElem = std::pair<SegmentOrder, std::pair<DateTime, LocCode>>;

using MaxStayMap = FlatMap<SegmentOrder, MaxStayData>;
using MaxStayMapElem = std::pair<SegmentOrder, MaxStayData>;

struct MostRestrictiveJourneySFRData
{
  // Journey Level Most Restrictive Data
  DateTime _advanceReservation = DateTime::openDate();
  DateTime _advanceTicketing = DateTime::openDate();
  void setAdvanceReservationWithEarlier(DateTime reservationDate)
  {
    _advanceReservation.setWithEarlier(reservationDate);
  }
  void setAdvanceTicketingWithEarlier(DateTime reservationDate)
  {
    _advanceTicketing.setWithEarlier(reservationDate);
  }
  bool isAnyMRJData() const { return _advanceReservation.isValid() || _advanceTicketing.isValid(); }
};

struct MostRestrictivePricingUnitSFRData
{
  // Pricing Unit Level Most Restrictive Data
  MinStayMap _minStayMap;
  MaxStayMap _maxStayMap;
};

struct PaxTypeFareStructuredRuleData
{
  MaxStayMap _maxStayMap;
  DateTime _advanceReservation = DateTime::openDate();
  DateTime _advanceTicketing = DateTime::openDate();
  void setAdvanceReservationWithEarlier(DateTime reservationDate)
  {
    _advanceReservation.setWithEarlier(reservationDate);
  }
  void setAdvanceTicketingWithEarlier(DateTime reservationDate)
  {
    _advanceTicketing.setWithEarlier(reservationDate);
  }
};

struct StructuredRuleData
{
  // cat 6
  LocCode _minStayLocation = LocCode();
  DateTime _minStayDate = DateTime();
  SegmentOrder _minStaySegmentOrder = -1;

  // Most restrictive data form Cat 7 and Cat 14 on fare component level
  MaxStayMap _maxStayMostRestrictiveFCData;

  //Advance reservation/ticketing
  DateTime _advanceReservation = DateTime::openDate();
  DateTime _advanceTicketing = DateTime::openDate();
  void setAdvanceReservationWithEarlier(DateTime reservationDate)
  {
    _advanceReservation.setWithEarlier(reservationDate);
  }
  void setAdvanceTicketingWithEarlier(DateTime reservationDate)
  {
    _advanceTicketing.setWithEarlier(reservationDate);
  }
};
}
