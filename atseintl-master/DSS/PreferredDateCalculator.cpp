//----------------------------------------------------------------------------
//
//  File:           DeterminePreferredDateRange.cpp
//  Created:        06/07/2005
//  Authors:        Adrian Tovar
//
//  Description: Utilize user date inputs to calculate preferred date range.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "DSS/PreferredDateCalculator.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayRequest.h"

namespace tse
{
namespace PreferredDateCalculator
{
static Logger
logger("atseintl.DSS.PreferredDateCalculator");

void
calculate(FareDisplayRequest& request, uint16_t& beginDateAdjValue, uint16_t& endDateAdjValue)
{
  LOG4CXX_DEBUG(logger, " Entered PreferredDateCalculator ");

  DateTime& prefTravelDate = request.preferredTravelDate();
  if (prefTravelDate == DateTime::emptyDate())
    return;

  DateTime& beginRangeDate = request.dateRangeLower();
  DateTime& endRangeDate = request.dateRangeUpper();
  int64_t diffPrefBegTimeSeconds = DateTime::diffTime(prefTravelDate, beginRangeDate);
  int64_t diffPrefEndTimeSeconds = DateTime::diffTime(endRangeDate, prefTravelDate);

  int16_t diffBegPrefDate = (diffPrefBegTimeSeconds / SECONDS_PER_DAY);
  int16_t diffEndPrefDate = (diffPrefEndTimeSeconds / SECONDS_PER_DAY);

  int16_t dateAdjustment = 0;
  int16_t begDateAdjustment = 0;
  int16_t endDateAdjustment = 0;
  int16_t remainderBegin = 0;
  int16_t remainderEnd = 0;

  //**********************************
  // Determine date adjustment
  //**********************************
  // DiffBegPrefDate GTE 3
  //**********************************
  if (diffBegPrefDate >= 3)
  {
    begDateAdjustment = 3;
    dateAdjustment = 0;
    remainderBegin = (diffBegPrefDate - 3);
    //******************************************
    // diffEndPrefDate GTE 3
    //******************************************
    if (diffEndPrefDate >= 3)
    {
      endDateAdjustment = 3;
    }
    //******************************************
    // diffEndPrefDate  LT 3
    //******************************************
    else
    {
      endDateAdjustment = diffEndPrefDate;
      dateAdjustment = (3 - diffEndPrefDate);
      if (dateAdjustment > remainderBegin)
      {
        dateAdjustment = remainderBegin;
      }
      begDateAdjustment += dateAdjustment;
    }
  }

  //***********************************
  // diffBeginPrefDate LT 3
  //***********************************
  else
  {
    begDateAdjustment = diffBegPrefDate;
    dateAdjustment = (3 - diffBegPrefDate);
    //*********************************************
    // diffEndPrefDate LT 3
    //*********************************************
    if (diffEndPrefDate < 3)
    {
      endDateAdjustment = diffEndPrefDate;
    }
    //*********************************************
    // diffEndPrefDate GTE 3
    //*********************************************
    else
    {
      endDateAdjustment = 3;
      remainderEnd = (diffEndPrefDate - 3);
      if (dateAdjustment > remainderEnd)
      {
        dateAdjustment = remainderEnd;
      }

      endDateAdjustment += dateAdjustment;
    }
  }

  //******************************************************************
  // Store preferred date range
  //*******************************************************************
  beginDateAdjValue = begDateAdjustment;
  endDateAdjValue = endDateAdjustment;

  LOG4CXX_DEBUG(logger, " Completed PreferredDateCalculator ");
}
}
}
