//-------------------------------------------------------------------
// File:    BookingCodeUtil.cpp
// Created: Jun 6, 2012
// Authors: Artur de Sousa Rocha
//
//  Copyright Sabre 2012
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Common/BookingCodeUtil.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/MarriedCabin.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"


namespace tse
{
FALLBACK_DECL(fallbackAAExcludedBookingCode);

namespace BookingCodeUtil
{
bool
premiumMarriage(PricingTrx& trx,
                const AirSeg& prevAirSeg,
                const BookingCode& prevBc,
                const BookingCode& currBc)
{
  const std::vector<MarriedCabin*>& marriedCabin =
      trx.dataHandle().getMarriedCabins(prevAirSeg.carrier(), prevBc, prevAirSeg.departureDT());
  if (marriedCabin.size() == 0)
    return false;

  bool passLocation = true;
  std::vector<MarriedCabin*>::const_iterator mBeginI = marriedCabin.begin();
  std::vector<MarriedCabin*>::const_iterator mBeginE = marriedCabin.end();
  for (; mBeginI != mBeginE; mBeginI++)
  {
    const MarriedCabin& marriedCab = *(*mBeginI);
    if (currBc != marriedCab.marriedCabin())
      continue;
    if (LIKELY(marriedCab.loc1().loc().empty() && marriedCab.loc2().loc().empty()))
    {
      return true;
    }

    const tse::DateTime& ticketingDate = trx.getRequest()->ticketingDT();
    passLocation = true;

    switch (marriedCab.directionality())
    {
    case '1': // Travel from location 1 to location 2
    {
      if (!marriedCab.loc1().loc().empty() && // validate loc 1
          !LocUtil::isInLoc(*(prevAirSeg.origin()),
                            marriedCab.loc1().locType(),
                            marriedCab.loc1().loc(),
                            "ATP",
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            ticketingDate))
      {
        passLocation = false;
        break;
      }

      if (!marriedCab.loc2().loc().empty() && // validate loc 2
          !LocUtil::isInLoc(*(prevAirSeg.destination()),
                            marriedCab.loc2().locType(),
                            marriedCab.loc2().loc(),
                            "ATP",
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            ticketingDate))
      {
        passLocation = false;
        break;
      }
      break;
    }
    case '2': // Travel from location 2 to location 1
    {
      if (!marriedCab.loc1().loc().empty() && // validate loc 1
          !LocUtil::isInLoc(*(prevAirSeg.destination()),
                            marriedCab.loc1().locType(),
                            marriedCab.loc1().loc(),
                            "ATP",
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            ticketingDate))
      {
        passLocation = false;
        break;
      }

      if (!marriedCab.loc2().loc().empty() && // validate loc 2
          !LocUtil::isInLoc(*(prevAirSeg.origin()),
                            marriedCab.loc2().locType(),
                            marriedCab.loc2().loc(),
                            "ATP",
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            ticketingDate))
      {
        passLocation = false;
        break;
      }
      break;
    }
    case '3': // Travel originating at location 1
    {
      if (!marriedCab.loc1().loc().empty() && // validate loc 1
          !LocUtil::isInLoc(*(prevAirSeg.origin()),
                            marriedCab.loc1().locType(),
                            marriedCab.loc1().loc(),
                            "ATP",
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            ticketingDate))
      {
        passLocation = false;
      }
      break;
    }
    case '4': // Travel originating at location 2
    {
      if (!marriedCab.loc2().loc().empty() && // validate loc 1
          !LocUtil::isInLoc(*(prevAirSeg.origin()),
                            marriedCab.loc2().locType(),
                            marriedCab.loc2().loc(),
                            "ATP",
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            ticketingDate))
      {
        passLocation = false;
      }
      break;
    }
    case ' ': // Within or between location 1 and location 2
    {
      // first see if it is a within condition
      if (marriedCab.loc1().loc() == marriedCab.loc2().loc() &&
          marriedCab.loc1().locType() == marriedCab.loc2().locType() &&
          !marriedCab.loc1().loc().empty())
      {
        if (!LocUtil::isInLoc(*(prevAirSeg.origin()),
                              marriedCab.loc1().locType(),
                              marriedCab.loc1().loc(),
                              "ATP",
                              RESERVED,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              ticketingDate))
        {
          passLocation = false;
          break;
        }
        if (!LocUtil::isInLoc(*(prevAirSeg.destination()),
                              marriedCab.loc1().locType(),
                              marriedCab.loc1().loc(),
                              "ATP",
                              RESERVED,
                              LocUtil::OTHER,
                              GeoTravelType::International,
                              EMPTY_STRING(),
                              ticketingDate))
        {
          passLocation = false;
          break;
        }
        break;
      }

      // if Not Within then we will come here and validate the between condition
      bool origInLoc1 = false;
      bool origInLoc2 = false;
      bool destInLoc1 = false;
      bool destInLoc2 = false;
      if (marriedCab.loc1().loc().empty() || LocUtil::isInLoc(*(prevAirSeg.origin()),
                                                              marriedCab.loc1().locType(),
                                                              marriedCab.loc1().loc(),
                                                              "ATP",
                                                              RESERVED,
                                                              LocUtil::OTHER,
                                                              GeoTravelType::International,
                                                              EMPTY_STRING(),
                                                              ticketingDate))
      {
        origInLoc1 = true;
      }
      if (marriedCab.loc2().loc().empty() || LocUtil::isInLoc(*(prevAirSeg.origin()),
                                                              marriedCab.loc2().locType(),
                                                              marriedCab.loc2().loc(),
                                                              "ATP",
                                                              RESERVED,
                                                              LocUtil::OTHER,
                                                              GeoTravelType::International,
                                                              EMPTY_STRING(),
                                                              ticketingDate))
      {
        origInLoc2 = true;
      }
      if (marriedCab.loc1().loc().empty() || LocUtil::isInLoc(*(prevAirSeg.destination()),
                                                              marriedCab.loc1().locType(),
                                                              marriedCab.loc1().loc(),
                                                              "ATP",
                                                              RESERVED,
                                                              LocUtil::OTHER,
                                                              GeoTravelType::International,
                                                              EMPTY_STRING(),
                                                              ticketingDate))
      {
        destInLoc1 = true;
      }
      if (marriedCab.loc2().loc().empty() || LocUtil::isInLoc(*(prevAirSeg.destination()),
                                                              marriedCab.loc2().locType(),
                                                              marriedCab.loc2().loc(),
                                                              "ATP",
                                                              RESERVED,
                                                              LocUtil::OTHER,
                                                              GeoTravelType::International,
                                                              EMPTY_STRING(),
                                                              ticketingDate))
      {
        destInLoc2 = true;
      }

      if (!((origInLoc1 && destInLoc2) || (origInLoc2 && destInLoc1)))
        passLocation = false;

      break;
    }
    default:
    {
      passLocation = false;
      break;
    }
    } // switch
    if (passLocation)
      return true;
  } // for(; mBeginI != mBeginE; mBeginI++)
  return false;
}

bool
isRebookedSolution(const FarePath& fPath)
{
  for (const PricingUnit* pu : fPath.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      for (const PaxTypeFare::SegmentStatus& segmentStatus : fu->segmentStatus())
      {
        if (segmentStatus._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
            !segmentStatus._bkgCodeReBook.empty())
          return true;
      }
    }
  }
  return false;
}

bool
validateExcludedBookingCodes(PricingTrx& trx,
                             const std::vector<BookingCode>& bkgCodes,
                             PaxTypeFare::SegmentStatus& segStat,
                             std::vector<BookingCode>& vTempBkgCodes,
                             DiagCollector* diag)
{
  if (fallback::fallbackAAExcludedBookingCode(&trx))
  {
    vTempBkgCodes = bkgCodes;
    return true;
  }

  std::vector<char> vBkgCodes, vExceptionCodes;
  std::string& sExclusionCodes = trx.getOptions()->aaBasicEBC();
  if (sExclusionCodes.length() <= 0) // Handle Empty EBCs
  {
    vTempBkgCodes = bkgCodes;
    return true;
  }
  if (diag)
  {
    *diag << "      BOOKING CODES FROM DB:";
    for (const auto& bc : bkgCodes)
      *diag << bc << " ";
    *diag << "\n";
    *diag << "\n      EXCLUDED BOOKING CODES FROM REQUEST:";
    for (const auto& str : sExclusionCodes)
      *diag << str << " ";
    *diag << "\n";
  }
  if (!sExclusionCodes.empty())
  {
    if (bkgCodes.size() != 0)
    {
      for (const BookingCode& code : bkgCodes)
        vBkgCodes.push_back(code[0]);
      for (const auto& cExCode : sExclusionCodes)
        vExceptionCodes.push_back(cExCode);

       vBkgCodes.erase(std::remove_if(vBkgCodes.begin(), vBkgCodes.end(),
             [=](char code) {
             if(std::find(vExceptionCodes.begin(), vExceptionCodes.end(), code) != vExceptionCodes.end())
             { return true; }else {return false;}}), vBkgCodes.end());
    }
  }
  if (vBkgCodes.size() > 0)
  {
    for(const auto& cCode : vBkgCodes)
    {
      for(const auto& bcFromDb : bkgCodes)
      {
        if (cCode==bcFromDb[0])
        {
          vTempBkgCodes.push_back(bcFromDb);
          if (diag)
            *diag << "      Valid BC: "<<bcFromDb<<"\n";
        }
      }
    }
    return true;
  }
  else
  {
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_FAIL, true);
  }
  if (diag)
     *diag << "      ALL BKCODES EXCLUDED, RETURN FALSE\n";
  return false;
}

void
copyBkgStatusToFu(FarePath& fpath)
{
  for (PricingUnit* pu : fpath.pricingUnit())
    pu->copyBkgStatusForEachFareUsageFromPaxTypeFare();
}

BookingCode
getFuSegmentBookingCode(const FareUsage& fu, const size_t index)
{
  if (!fu.segmentStatus().empty())
  {
    const BookingCode bookingCode = fu.segmentStatus()[index]._bkgCodeReBook;
    if (!bookingCode.empty())
      return bookingCode;
  }

  return fu.travelSeg()[index]->getBookingCode();
}
}
}
