//----------------------------------------------------------------------------
//
//  File:           BookingCodeQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare for booking code criterion.
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

#include "FareDisplay/BookingCodeQualifier.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleConst.h"

namespace tse
{
BookingCodeQualifier::BookingCodeQualifier() {}

BookingCodeQualifier::~BookingCodeQualifier() {}

const tse::PaxTypeFare::FareDisplayState
BookingCodeQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics1(trx, "QUAL BKG CODE");
  LOG4CXX_DEBUG(_logger, "Entering BookingCodeQualifier::qualify()...");

  const FareDisplayRequest* request = trx.getRequest();

  if (!request->bookingCode().empty() ||
      (!request->fareBasisCode().empty() && ptFare.fare()->isIndustry()))
  {
    getBookingCode(trx, ptFare); // ptFare now has the Booking Code
  }

  if (!request->bookingCode().empty())
  {
    if (!qualifyBookingCode(request->bookingCode(), ptFare))
    {
      LOG4CXX_DEBUG(_logger, "BookingCodeQualifier::qualify()..!qualifyBookingCode.");
      return PaxTypeFare::FD_BookingCode;
    }

    else if (request->bookingCode()[0] != ptFare.bookingCode()[0])
    {
      LOG4CXX_DEBUG(
          _logger,
          "BookingCodeQualifier::qualify()..request->bookingCode()[0] != ptFare.bookingCode()[0].");
      return PaxTypeFare::FD_BookingCode;
    }
  }
  return retProc(trx, ptFare);
}

bool
BookingCodeQualifier::qualifyBookingCode(const BookingCode& requestedBookingCode,
                                         const PaxTypeFare& ptFare) const
{
  if (requestedBookingCode[0] == ptFare.bookingCode()[0])
  {
    return true;
  }

  std::vector<BookingCode> bkgCodes;
  if (!ptFare.getPrimeBookingCode(bkgCodes))
  {
    if (ptFare.isFareByRule())
    {
      FBRPaxTypeFareRuleData* fbrData = ptFare.getFbrRuleData();
      if (fbrData != nullptr)
      {
        // Get booking codes from base fare
        fbrData->getBaseFarePrimeBookingCode(bkgCodes);
      }
    }
  }

  std::vector<BookingCode>::iterator bkgCodeIter = bkgCodes.begin();
  std::vector<BookingCode>::iterator bkgCodeEnd = bkgCodes.end();

  for (; bkgCodeIter != bkgCodeEnd; ++bkgCodeIter)
  {
    if ((*bkgCodeIter) == requestedBookingCode)
    {
      return true;
    }
  }

  return false;
}

void
BookingCodeQualifier::getBookingCode(FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare) const
{
  TSELatencyData metrics(trx, "GET BKG CODE");
  FareDisplayBookingCode fdbc;
  fdbc.getBookingCode(trx,
                      const_cast<PaxTypeFare&>(paxTypeFare),
                      (const_cast<PaxTypeFare&>(paxTypeFare)).bookingCode());
}

bool
BookingCodeQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
