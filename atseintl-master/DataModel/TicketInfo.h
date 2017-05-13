//-------------------------------------------------------------------
//
//  File:        TicketInfo.h
//  Author:      Doug Batchelor
//  Created:     October 18, 2005
//  Description: A class to provide a place for
//               Category 15, Sales data
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
class TicketInfo
{
public:
  TicketInfo(const DateTime& earliestTktDate,
             const DateTime& latestTktDate,
             const Indicator ruleType)
    : _earliestTktDate(earliestTktDate), // TKTG:  Earliest date for ticketing on any one sector.
      _latestTktDate(latestTktDate), // TKTG:  Last date for ticketing on all sectors.
      _ruleType(ruleType)
  {
  }

  // Cat 15 (Sales) info
  DateTime& earliestTktDate() { return _earliestTktDate; }
  const DateTime& earliestTktDate() const { return _earliestTktDate; }

  DateTime& latestTktDate() { return _latestTktDate; }
  const DateTime& latestTktDate() const { return _latestTktDate; }

  const Indicator ruleType() const { return _ruleType; }

private:
  // Cat 15 (Sales) info
  DateTime _earliestTktDate; // TKTG:  Earliest date for ticketing on any one sector.
  DateTime _latestTktDate; // TKTG:  Last date for ticketing on all sectors.
  const Indicator _ruleType; // FootNote, FareRule or GeneralRule
};
} // namespace tse
