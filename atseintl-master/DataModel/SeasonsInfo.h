//-------------------------------------------------------------------
//
//  File:        SeasonsInfo.h
//  Author:      Doug Batchelor
//  Created:     October 18, 2005
//  Description: A class to provide a place for
//               Category 3, Seasons data
//
//  Updates:
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
//
//---------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"

namespace tse
{
// Cat 3 (Season) info follows.  This info is consolidated into a class.
class SeasonsInfo
{
public:
  SeasonsInfo();

  Indicator& direction() { return _direction; }
  const Indicator& direction() const { return _direction; }

  int32_t& tvlstartyear() { return _tvlstartyear; }
  const int32_t& tvlstartyear() const { return _tvlstartyear; }

  int32_t& tvlstartmonth() { return _tvlstartmonth; }
  const int32_t& tvlstartmonth() const { return _tvlstartmonth; }

  int32_t& tvlstartDay() { return _tvlstartDay; }
  const int32_t& tvlstartDay() const { return _tvlstartDay; }

  int32_t& tvlStopyear() { return _tvlStopyear; }
  const int32_t& tvlStopyear() const { return _tvlStopyear; }

  int32_t& tvlStopmonth() { return _tvlStopmonth; }
  const int32_t& tvlStopmonth() const { return _tvlStopmonth; }

  int32_t& tvlStopDay() { return _tvlStopDay; }
  const int32_t& tvlStopDay() const { return _tvlStopDay; }

  void initialize(Indicator dir, // Inbound ('I') or Outbound ( 'O')
                  int32_t startYear, // START: The first date on which travel is permitted.
                  int32_t startMonth, // If year is blank(0), any year is assumed.
                  int32_t startDay, // If day is blank(0), entire month is assumed.
                  int32_t stopYear, // STOP: The last date on which travel is permitted.
                  int32_t stopMonth, // If year is blank (0), any year is assumed.
                  int32_t stopDay) // If day is blank(0), entire month is assumed.
  {
    _direction = dir;
    _tvlstartyear = startYear;
    _tvlstartmonth = startMonth;
    _tvlstartDay = startDay;
    _tvlStopyear = stopYear;
    _tvlStopmonth = stopMonth;
    _tvlStopDay = stopDay;
  }

private:
  Indicator _direction; // Inbound ('I') or Outbound ( 'O')
  int32_t _tvlstartyear; // START: The first date on which travel is permitted.
  int32_t _tvlstartmonth; // If year is blank(0), any year is assumed.
  int32_t _tvlstartDay; // If day is blank(0), entire month is assumed.
  int32_t _tvlStopyear; // STOP: The last date on which travel is permitted.
  int32_t _tvlStopmonth; // If year is blank (0), any year is assumed.
  int32_t _tvlStopDay; // If day is blank(0), entire month is assumed.
};

inline SeasonsInfo::SeasonsInfo()
  : _direction('I'),
    _tvlstartyear(0),
    _tvlstartmonth(0),
    _tvlstartDay(0),
    _tvlStopyear(0),
    _tvlStopmonth(0),
    _tvlStopDay(0)
{
}
} // namespace tse

