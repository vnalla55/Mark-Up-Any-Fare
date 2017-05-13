//-------------------------------------------------------------------
//
//  File:        ShoppingAltDateUtil.h
//  Created:     July 12, 2006
//  Authors:     Kavya Katam
//
//  Description: Validates Duration and Saturday Nights
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

#pragma once

#include "Common/DateTime.h"
#include "DataModel/ShoppingTrx.h"

#include <vector>

namespace tse
{
class AirSeg;
class FareMarket;
class FarePath;
class Itin;
class PaxTypeFare;

class ShoppingAltDateUtil
{
public:
  static bool validateAltDatePair(const ShoppingTrx& trx, DatePair datePair);
  static bool getDatePair(const PricingTrx::AltDatePairs& altDatePairs,
                          const DateTime& date,
                          uint8_t leg,
                          DatePair& datePair);
  static DatePair getTravelDateRange(const ShoppingTrx& trx);

  static DateTime getDateSop(const ShoppingTrx::SchedulingOption& sop);
  static DateTime getDateSop(const ShoppingTrx& trx, int leg, int sop);
  static DatePair getDatePairSops(const ShoppingTrx& trx, const SopIdVec& sops);
  static DatePair getDatePairSops(const ShoppingTrx::SchedulingOption* sop1,
                                  const ShoppingTrx::SchedulingOption* sop2 /*= 0 for OW*/);

  static bool checkEffExpDate(const ShoppingTrx& trx, const PaxTypeFare& ptFare, uint8_t& ret);
  static bool checkEffDisc(const DateTime& x, const TSEDateInterval& range, uint8_t& ret);
  static bool
  checkDateRange(const DateTime& x, const DateTime& lo, const DateTime& hi, uint8_t& ret);

  // convenience functions which return only the date component
  // of a DateTime.
  static DateTime dateOnly(const DateTime& d) { return d.date(); }
  static DatePair dateOnly(const DatePair& d)
  {
    return DatePair(dateOnly(d.first), dateOnly(d.second));
  }

  static Itin*
  getJourneyItin(const PricingTrx::AltDatePairs& altDatePairs, const DatePair& datePair);
  static void setAltDateFltBit(PaxTypeFare* ptFare, const int bitIndex);

  static void cloneAltDates(const ShoppingTrx& trx,
                            PricingTrx::AltDatePairs& altDatePairsCopy,
                            std::deque<Itin>& journeyItins);
  static void setJrnItinFM(FareMarket& fm, AirSeg* seg);

  static void generateJourneySegAndFM(DataHandle& dateHandle,
                                      Itin& jourenyItin,
                                      DateTime outboundDate,
                                      LocCode boardCity,
                                      LocCode offCity,
                                      CarrierCode cxrCode,
                                      int16_t pnrSegment);

  static DatePair getMIPTravelDateRange(const PricingTrx& trx);
  static uint64_t getDuration(const ShoppingTrx& trx, const SopIdVec& sops);
  static uint64_t getNoOfDays(const uint64_t duration);
  static uint64_t getDuration(const DatePair& datePair);
  static bool cutOffAltDate(PricingTrx& trx,
                            const FarePath* farePath,
                            const uint16_t altDateItinPriceJumpFactor,
                            const uint16_t altDateCutOffNucThreshold);
  static void setAltDateCutOffNucThreshold(PricingTrx& trx,
                                           const FarePath* farePath,
                                           const uint16_t altDateItinPriceJumpFactor,
                                           const uint16_t altDateCutOffNucThreshold);

  static void setAltDateStatus(PaxTypeFare* paxTypeFare,
                               const DatePair datePair,
                               const uint32_t legId);

  static void applyStatustoSimilarDatePair(PaxTypeFare* paxTypeFare,
                                           const DatePair datePair,
                                           const unsigned int category,
                                           const uint32_t legId);

  static void cleanUpCategoryProcess(PaxTypeFare* paxTypeFare);
  static void setCategoriesUnprocessed(PaxTypeFare* paxTypeFare);

private:
  ShoppingAltDateUtil() {}
  ~ShoppingAltDateUtil() {}

  static bool validateDuration(const ShoppingTrx& trx, DatePair datePair);
};
} // End namespace tse

