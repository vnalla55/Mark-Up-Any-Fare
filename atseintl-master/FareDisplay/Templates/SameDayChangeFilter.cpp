//-------------------------------------------------------------------
//
//  File:        SameDayChangeFilter.cpp
//  Created:     April 7, 2005
//  Authors:     Doug Batchelor/Tony Lam
//  Description: This class formats teh same day change indicator
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
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/SameDayChangeFilter.h"

#include "Common/DateTime.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayInfo.h"
#include "FareDisplay/Templates/Field.h"

#include <string>

namespace tse
{
namespace
{
// this value defines the fare added Indicator
const std::string ADDED_INDICATOR = "A";
// this value defines the fare deleted Indicator
const std::string DELETED_INDICATOR = "X";
// this value defines the fare increased Indicator
const std::string INCREASED_INDICATOR = "I";
// this value defines the fare reduced Indicator
const std::string REDUCED_INDICATOR = "R";
// this value defines the record changed Indicator
const std::string CHANGED_INDICATOR = "C";
// this value defines the mode date not in last 24 hours Indicator
const std::string BLANK_INDICATOR = " ";
// this value defines the mode date not in last 24 hours Indicator
const char YES_INDICATOR = 'Y';
}

namespace SameDayChangeFilter
{
void
formatData(FareDisplayInfo& fareDisplayInfo, Field& field)
{
  // ================================================
  // Same Day Change Indicator Logic follows
  // ================================================
  bool isWithin24 = false;
  TimeDuration oneDay(24, 0, 0);
  const Fare* f = fareDisplayInfo.paxTypeFare().fare();
  DateTime expDate = f->expirationDate();
  DateTime modDate = f->lastModDate();
  DateTime now = DateTime::localTime();

  // First establish if the modDate falls within the last 24 hours.
  // This window of time determines if anything might be displayed.
  // If the modDate is outside of this window, nothing else
  // matters.
  DateTime twentyFourHoursAgo(now, -oneDay);
  isWithin24 = modDate.isBetween(twentyFourHoursAgo, now);

  if (isWithin24)
  {
    // First check for Fare deleted in last 24 hours
    if (expDate.isBetween(twentyFourHoursAgo, now))
    {
      // It's been discontinued, so show it as Fare Deleted
      field.strValue() = DELETED_INDICATOR;
    }
    else if (f->increasedFareAmtTag() == YES_INDICATOR)
    {
      field.strValue() = INCREASED_INDICATOR;
    }

    else if (f->reducedFareAmtTag() == YES_INDICATOR)
    {
      field.strValue() = REDUCED_INDICATOR;
    }
    else if ((f->footnoteTag() == YES_INDICATOR) || (f->routingTag() == YES_INDICATOR) ||
             (f->effectiveDateTag() == YES_INDICATOR))
    {
      field.strValue() = CHANGED_INDICATOR;
    }
    else
    {
      // No change indicators, but mode date is within 24 hours
      // so just show it as added.
      field.strValue() = ADDED_INDICATOR;
    }
  }
  else
  {
    // ModDate is not within last 24 hours.
    field.strValue() = BLANK_INDICATOR;
  }
}
}
} // tse namespace
