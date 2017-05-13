//----------------------------------------------------------------------------
//
//  File:           TravelDateQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare.
//
//  Updates:
//
//  Copyright Sabre 2006
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/TravelDateQualifier.h"

#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TravelDateQualifier::TravelDateQualifier() {}

TravelDateQualifier::~TravelDateQualifier() {}

const tse::PaxTypeFare::FareDisplayState
TravelDateQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  //
  // validate Effective and Discontinue dates here
  //
  const Indicator bothDates = trx.getOptions()->returnDateValidation();
  if (bothDates == 'Y')
  {
    // this is a new option to allow combinability of fares
    if (!qualifyAnyTravelDate(trx, ptFare))
      return PaxTypeFare::FD_Eff_Disc_Dates;
  }
  else
  {
    // this option happens in Legacy, could be N or empty
    if (!qualifyBothTravelDates(trx, ptFare))
      return PaxTypeFare::FD_Eff_Disc_Dates;
  }
  return retProc(trx, ptFare);
}

bool
TravelDateQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}

/**
* Validate any travel date for Effective/Discontinue dates
**/
bool
TravelDateQualifier::qualifyAnyTravelDate(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics(trx, "QUAL ANY TRVL DATE");
  LOG4CXX_DEBUG(_logger, "Entered qualifyAnyTravelDate()");

  const FareDisplayRequest* request = trx.getRequest();

  if (request == nullptr)
  {
    LOG4CXX_ERROR(_logger, "Logic error - FareDisplayTrx doesn't have a FareDisplayRequest")
    return false;
  }
  if (!checkEffDisc(request->dateRangeLower(), ptFare.fare()->fareInfo()->effInterval()))
  {
    LOG4CXX_DEBUG(_logger, "OUT OF RANGE lower request date");
    return false;
  }
  if (!checkEffDisc(request->dateRangeUpper(), ptFare.fare()->fareInfo()->effInterval()))
  {
    LOG4CXX_DEBUG(_logger, "OUT OF RANGE upper request date");
    return false;
  }
  // TODO if higher / lower actually checked, ignore other dates ???
  if (!checkEffDisc(trx.travelDate(), ptFare.fare()->fareInfo()->effInterval()))
  {
    LOG4CXX_DEBUG(_logger, "OUT OF RANGE travel date");
    return false;
  }
  return true;
}

/**
* Validate both travel dates for Effective/Discontinue dates
**/
bool
TravelDateQualifier::qualifyBothTravelDates(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics(trx, "QUAL BOTH TRVL DATES");
  LOG4CXX_DEBUG(_logger, "Entered qualifyBothTravelDates()");

  if (!qualifyAnyTravelDate(trx, ptFare))
    return false;

  // qualifyAnyTravelDate() already validates cast
  const FareDisplayRequest* request = trx.getRequest();

  if (!checkEffDisc(request->returnDate(), ptFare.fare()->fareInfo()->effInterval()))
  {
    LOG4CXX_DEBUG(_logger, "OUT OF RANGE return date");
    return false;
  }
  return true;
}

bool
TravelDateQualifier::checkDateRange(const DateTime& x, const DateTime& lo, const DateTime& hi) const
{
  if (x.isValid())
  {
    if (lo.isValid() && lo > x)
      return false;
    if (hi.isValid() && hi < x)
      return false;
  }
  return true;
}

bool
TravelDateQualifier::checkEffDisc(const DateTime& x, const TSEDateInterval& range) const
{
  return checkDateRange(x, range.effDate(), range.discDate());
}
}
