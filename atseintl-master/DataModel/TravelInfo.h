//-------------------------------------------------------------------
//
//  File:        TravelInfo.h
//  Author:      Doug Batchelor
//  Created:     October 18, 2005
//  Description: A class to provide a place for
//               Cat 14 (Travel) info
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

#include "Common/DateTime.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class TravelInfo
{
public:
  TravelInfo(const DateTime& earliestTvlStartDate,
             const DateTime& latestTvlStartDate,
             const DateTime& stopDate,
             Indicator returnTvlInd)
    : _earliestTvlStartDate(earliestTvlStartDate),
      _latestTvlStartDate(latestTvlStartDate),
      _stopDate(stopDate),
      _returnTvlInd(returnTvlInd)
  {
  }

  DateTime& earliestTvlStartDate() { return _earliestTvlStartDate; }
  const DateTime& earliestTvlStartDate() const { return _earliestTvlStartDate; }

  DateTime& latestTvlStartDate() { return _latestTvlStartDate; }
  const DateTime& latestTvlStartDate() const { return _latestTvlStartDate; }

  const DateTime& stopDate() const { return _stopDate; }

  const Indicator returnTvlInd() const { return _returnTvlInd; }

private:
  // Cat 14 (Travel) info
  DateTime _earliestTvlStartDate; // COMM: The earliest date on which outbound travel may commence.
  DateTime _latestTvlStartDate; // EXP: The last date on which outbound travel may commence
  const DateTime _stopDate; // COMP: The last date on which travel must commence or be completed.
  const Indicator _returnTvlInd; // RET: A tag indicating whether return travel must commence or be
  // completed by the following Date/Time.
};
} // namespace tse
