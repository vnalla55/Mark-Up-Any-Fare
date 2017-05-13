//-------------------------------------------------------------------
// File:    BookingCodeUtil.h
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
#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{

class AirSeg;
class FarePath;
class FareUsage;
class PricingTrx;

namespace BookingCodeUtil
{
bool premiumMarriage(PricingTrx& trx,
                     const AirSeg& prevAirSeg,
                     const BookingCode& prevBc,
                     const BookingCode& currBc);
bool isRebookedSolution(const FarePath& fPath);
void copyBkgStatusToFu(FarePath& fpath);
bool validateExcludedBookingCodes(PricingTrx& trx,
                                  const std::vector<BookingCode>& bkgCodes,
                                  PaxTypeFare::SegmentStatus& segStat,
                                  std::vector<BookingCode>& vTempBkgCodes,
                                  DiagCollector* diag = nullptr);
BookingCode getFuSegmentBookingCode(const FareUsage& fu, const size_t index);
}
}
