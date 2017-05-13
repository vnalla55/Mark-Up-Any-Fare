//-------------------------------------------------------------------
//
//  File:        AltDatesPairOptionsBounder.h
//  Created:     Mar 05, 2008
//  Authors:     Miroslaw Bartyna
//
//  Description: PQ for sort and reduce alt dates options
//
//
//  Copyright Sabre 2008
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

#include "DataModel/FlightFinderTrx.h"

#include <functional>
#include <queue>

namespace tse
{
class PricingTrx;

class lessPairPQ
{
public:
  bool operator()(std::pair<uint64_t, std::pair<DatePair, FlightFinderTrx::FlightBitInfo> >& lhs,
                  std::pair<uint64_t, std::pair<DatePair, FlightFinderTrx::FlightBitInfo> >& rhs)
  {
    return lhs.first > rhs.first;
  }
};

class AltDatesPairOptionsBounder
{
  typedef std::pair<uint64_t, std::pair<DatePair, FlightFinderTrx::FlightBitInfo> > PQElem;
  typedef std::priority_queue<PQElem, std::vector<PQElem>, lessPairPQ> AltDatePairsPQ;

private:
  static uint64_t countValueWithWeight(const DateTime& originalDate,
                                       const DateTime& nextData,
                                       const uint16_t weight);
  static uint64_t countValue(const std::pair<DateTime, DateTime>& originalPair,
                             const std::pair<DateTime, DateTime>& nextPair);
  static void
  buildPQ(const std::pair<DateTime, DateTime>& originalPair,
          const std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDateStatus,
          AltDatePairsPQ& pairsPQ);
  static void buildReturnMap(const uint8_t desiredNumberOfOutboundDates,
                             const uint8_t desiredNumberOfInboundDates,
                             std::vector<std::pair<DatePair, FlightFinderTrx::FlightBitInfo> >&
                                 boundedCombinedAltDateStatus,
                             AltDatePairsPQ& pairsPQ);
  static void showDiag969AltDatesPQ(const std::pair<DateTime, DateTime>& originalPair,
                                    AltDatePairsPQ& pairsPQ,
                                    PricingTrx& trx);

public:
  static void pickUpDesiredNumberOfPairs(
      const uint8_t desiredNumberOfOutboundDates,
      const uint8_t desiredNumberOfInboundDates,
      const std::pair<DateTime, DateTime>& originalPair,
      const std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDateStatus,
      std::vector<std::pair<DatePair, FlightFinderTrx::FlightBitInfo> >&
          boundedCombinedAltDateStatus,
      PricingTrx& trx);
};

} // tse

