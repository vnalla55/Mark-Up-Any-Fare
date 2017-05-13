//----------------------------------------------------------------------------
//  File:        DominatedFlights.h
//  Created:     2009-04-27
//
//  Description: Class used to find and remove dominated flights
//
//  Updates:
//
//  Copyright Sabre 2009
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

#include "Pricing/EstimatedSeatValue.h"

namespace tse
{

struct DominatedFlightsInfo
{
  std::vector<MoneyAmount> esvValues;
  std::string carriersSequence;
  long elapsedTime = 0;
};

class ElapsedTimeSopComparator
{
public:
  bool operator()(ShoppingTrx::SchedulingOption* lhs, ShoppingTrx::SchedulingOption* rhs) const
  {
    return (lhs->itin()->getFlightTimeMinutes() < rhs->itin()->getFlightTimeMinutes());
  }
};

class DominatedFlights
{
public:
  DominatedFlights(const ShoppingTrx& trx);

  void findDominatedFlights(std::vector<ShoppingTrx::SchedulingOption*>& outboundFlightsVec,
                            std::vector<ShoppingTrx::SchedulingOption*>& inboundFlightsVec);

  void removeDominatedFlights(std::vector<ShoppingTrx::SchedulingOption*>& outboundFlightsVec,
                              std::vector<ShoppingTrx::SchedulingOption*>& inboundFlightsVec);

private:
  void removeDominatedFlights(std::vector<ShoppingTrx::SchedulingOption*>& flightsVec);

  void prepareFlightsInfo(const std::vector<ShoppingTrx::SchedulingOption*>& flightsVec,
                          bool isInbound = false);

  void markDominatedFlights(std::vector<ShoppingTrx::SchedulingOption*>& flightsVec,
                            bool isInbound = false);

  bool checkSameDepartureDate(const ShoppingTrx::SchedulingOption* sopFirst,
                              const ShoppingTrx::SchedulingOption* sopSecond);

  bool checkSameArrivalDate(const ShoppingTrx::SchedulingOption* sopFirst,
                            const ShoppingTrx::SchedulingOption* sopSecond);

  bool isNotMoreExpensive(const std::vector<MoneyAmount>& esvValuesFirst,
                          const std::vector<MoneyAmount>& esvValuesSecond);

  const ShoppingTrx& _trx;
  std::vector<DominatedFlightsInfo> _outboundInfo;
  std::vector<DominatedFlightsInfo> _inboundInfo;
  uint32_t _minOutFlightsCount = 0;
  uint32_t _minInFlightsCount = 0;
};
} // End namespace tse
