//-------------------------------------------------------------------
//
//  File:        FlightCountTemplate.cpp
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize the Flight Counts that
//               appear on the Sabre greenscreen.)
//
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

#include "FareDisplay/Templates/FlightCountSection.h"

#include "Common/FareDisplayUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/Customer.h"
#include "DSS/FlightCount.h"

#include <sstream>
#include <string>

namespace tse
{
namespace
{
// this value defines the Effective Date Indicator
static const char SLASH = '/';
static const std::string SINGLE_BLANK = " ";
static const std::string THREE_BLANKS = "  ";
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  FlightCountSection::buildDisplay()
//
// This is the main processing method of the FlightCountTemplate class.
// It requires a reference to the Fare Display Transaction. When
// called, it first iterates through the collection of fields,
// and displays the column headers.  It then iterates through each
// of the collected fares and for each one, iterates through the
// collection of fields maintained by the FlightCountTemplate object.
//
//
//
//
// @return  void.
//
// </PRE>
// -------------------------------------------------------------------------
void
FlightCountSection::buildDisplay()
{
  if (!_trx.hasScheduleCountInfo())
    return;

  uint16_t count(0);
  for (const auto start : _trx.fdResponse()->scheduleCounts())
  {
    if ((count != 0 && (count % MAX_COUNT) == 0))
    {
      _trx.response() << std::endl;
      displayCount(_trx, *start);
      count = 0; // as this is the last carrier for this line, start from 0 for next line
    }
    else
      displayCount(_trx, *start, (count == MAX_COUNT - 1));

    ++count;
  }

  _trx.response() << std::endl;
}

void
FlightCountSection::displayCount(FareDisplayTrx& trx, FlightCount& ft, bool lastCxrInRow) const
{
  trx.response() << ft._carrier << SINGLE_BLANK << std::setw(2) << ft._nonStop << SLASH
                 << std::setw(2) << ft._direct << SLASH << std::setw(2)
                 << FareDisplayUtil::getConnectingFlightCount(trx, ft);

  if (lastCxrInRow == false)
  {
    trx.response() << THREE_BLANKS;
  }
}
} // tse namespace
