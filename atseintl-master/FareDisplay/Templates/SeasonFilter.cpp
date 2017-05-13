//-------------------------------------------------------------------
//
//  File:        SeasonFilter
//  Created:     February 17, 2005
//  Authors:     Doug Batchelor/LeAnn Perez
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

// SeasonFilter.cpp: implementation of the SeasonFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "FareDisplay/Templates/SeasonFilter.h"

#include "Common/FareDisplayUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/SeasonsInfo.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/Field.h"

#include <sstream>
#include <string>
#include <vector>

namespace tse
{
namespace SeasonFilter
{
namespace
{
const std::string SEASONS_NO_DATES = "   ---";
const std::string SEASONS_DASH = "-";
const std::string SEASONS_NP = "   NP";
const char SEASONS_BLANK = ' ';
}

void
formatData(FareDisplayInfo& fareDisplayInfo,
           Field& field1,
           std::vector<ElementField>& fields,
           FareDisplayTrx& trx,
           const Indicator dateFormat)

{

  // Check for NP condition (Not auto-Priceable)
  ElementField tempField;
  if (!fareDisplayInfo.isAutoPriceable())
  {
    field1.strValue() = SEASONS_NP;
    tempField.strValue() = SEASONS_NP;
    fields.push_back(tempField);
    return;
  }
  if (fareDisplayInfo.seasons().empty())
  {
    field1.strValue() = SEASONS_NO_DATES;
    tempField.strValue() = SEASONS_NO_DATES;
    fields.push_back(tempField);
    return;
  }

  std::vector<SeasonsInfo*>::const_iterator seasonsInfoI = fareDisplayInfo.seasons().begin();
  std::vector<SeasonsInfo*>::const_iterator seasonsInfoEnd = fareDisplayInfo.seasons().end();

  for (int fieldCnt = 0; seasonsInfoI != seasonsInfoEnd; seasonsInfoI++, fieldCnt++)
  {
    std::ostringstream oss;
    if (!((*seasonsInfoI)->tvlstartDay()) && !((*seasonsInfoI)->tvlstartmonth()) &&
        !((*seasonsInfoI)->tvlstartyear()) && !((*seasonsInfoI)->tvlStopDay()) &&
        !((*seasonsInfoI)->tvlStopmonth()) && !((*seasonsInfoI)->tvlStopyear()))
    {
      oss << SEASONS_NO_DATES;
    }
    else
    {
      if ((*seasonsInfoI)->tvlstartDay())
      {

        oss.setf(std::ios::right, std::ios::adjustfield);
        oss << std::setfill('0') << std::setw(2) << (*seasonsInfoI)->tvlstartDay();
      }
      else
        oss << SEASONS_BLANK;

      FareDisplayUtil::formatMonth((*seasonsInfoI)->tvlstartmonth(), oss, dateFormat);

      if (trx.getRequest()->ticketingAgent()->axessUser())
      {
        int32_t tvlStartYearValue = 0;
        Indicator dateFormatI;
        (dateFormat == '5') ? (dateFormatI = '4') : (dateFormatI = dateFormat);
        if ((*seasonsInfoI)->tvlstartyear() == 0)
        {
          tvlStartYearValue = -1;
          FareDisplayUtil::formatYear(tvlStartYearValue, oss, dateFormatI);
        }
        else
        {
          tvlStartYearValue = (*seasonsInfoI)->tvlstartyear() % 10;
          FareDisplayUtil::formatYear(tvlStartYearValue, oss, dateFormatI);
        }
      }
      else // user is Global (non-Axess)
      {
        int32_t tvlStartYearValue = 0;
        Indicator dateFormatI;
        (dateFormat == '5') ? (dateFormatI = '4') : (dateFormatI = dateFormat);
        if ((*seasonsInfoI)->tvlstartyear() == 0)
        {
          tvlStartYearValue = -1;
          FareDisplayUtil::formatYear(tvlStartYearValue, oss, dateFormatI);
        }
        else if ((*seasonsInfoI)->tvlstartyear() > 2000)
        {
          tvlStartYearValue = (*seasonsInfoI)->tvlstartyear() - 2000;
          FareDisplayUtil::formatYear(tvlStartYearValue, oss, dateFormatI);
        }
        else
          FareDisplayUtil::formatYear((*seasonsInfoI)->tvlstartyear(), oss, dateFormatI);
      }

      oss << SEASONS_DASH;

      if ((*seasonsInfoI)->tvlStopDay())
      {
        oss.setf(std::ios::right, std::ios::adjustfield);
        oss << std::setfill('0') << std::setw(2) << (*seasonsInfoI)->tvlStopDay();
      }
      else
        oss << SEASONS_BLANK;
      FareDisplayUtil::formatMonth((*seasonsInfoI)->tvlStopmonth(), oss, dateFormat);

      if (trx.getRequest()->ticketingAgent()->axessUser())
      {
        int32_t tvlStopYearValue = 0;
        Indicator dateFormatI;
        (dateFormat == '5') ? (dateFormatI = '4') : (dateFormatI = dateFormat);
        if ((*seasonsInfoI)->tvlStopyear() == 0)
        {
          tvlStopYearValue = -1;
          FareDisplayUtil::formatYear(tvlStopYearValue, oss, dateFormatI);
        }
        else
        {
          tvlStopYearValue = ((*seasonsInfoI)->tvlStopyear()) % 10;
          FareDisplayUtil::formatYear(tvlStopYearValue, oss, dateFormatI);
        }
      }
      else
      {
        int32_t tvlStopYearValue = 0;
        Indicator dateFormatI;
        (dateFormat == '5') ? (dateFormatI = '4') : (dateFormatI = dateFormat);
        if ((*seasonsInfoI)->tvlStopyear() == 0)
        {
          tvlStopYearValue = -1;
          FareDisplayUtil::formatYear(tvlStopYearValue, oss, dateFormatI);
        }
        else if ((*seasonsInfoI)->tvlstartyear() < 2010)
        {
          tvlStopYearValue = ((*seasonsInfoI)->tvlStopyear()) % 100;
          FareDisplayUtil::formatYear(tvlStopYearValue, oss, dateFormatI);
        }
        else
        {
          tvlStopYearValue = ((*seasonsInfoI)->tvlStopyear()) % 10;
          FareDisplayUtil::formatYear(tvlStopYearValue, oss, dateFormatI);
        }
      }
    }
    if (!oss.str().empty())
    {
      if (fieldCnt == 0)
      {
        field1.strValue() = oss.str();
      }

      tempField.strValue() = oss.str();
      fields.push_back(tempField);
    }
  }
  if (fields.empty())
  {
    field1.strValue() = SEASONS_NO_DATES;
    tempField.strValue() = SEASONS_NO_DATES;
    fields.push_back(tempField);
    return;
  }
}
}
} // tse namespace
