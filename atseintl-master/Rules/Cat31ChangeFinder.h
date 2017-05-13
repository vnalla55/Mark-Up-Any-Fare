// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include "Common/ExchShopCalendarUtils.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"

#include <boost/functional/hash.hpp>

#include <utility>
#include <memory>

namespace tse
{
class AirSeg;

namespace
{
using DateRange = ExchShopCalendar::DateRange;
}

class AbstractChangeFinder
{
public:
  virtual ~AbstractChangeFinder() {}
  virtual bool notChanged(const std::vector<TravelSeg*>& travelSegs) = 0;
};

class Cat31ChangeFinder : public AbstractChangeFinder
{
public:
  using OriginDestination = PricingTrx::OriginDestination;
  using SegOndIndexMapKey = const TravelSeg*;

  Cat31ChangeFinder(const std::vector<TravelSeg*> travelSegVec,
                    const std::vector<PricingTrx::OriginDestination>& ondVec,
                    ExchShopCalendar::R3ValidationResult* r3ValidationResults,
                    const bool isCalendar,
                    const RexPricingTrx& trx);

  bool notChanged(const std::vector<TravelSeg*>& travelSegVec);

private:
  const int32_t findOndIndexForSeg(const TravelSeg& travelSeg) const;
  DateRange createDateRangeFromOnd(const TravelSeg& newTvlSeg, const TravelSeg& travelSeg) const;
  std::pair<const AirSeg*, const AirSeg*>
  castToAirSeg(const TravelSeg* newTvlSeg, const TravelSeg* tvlSeg);
  bool matchBothNotAirSeg(const AirSeg* newAs, const AirSeg* excAs) const;
  bool matchBothAirSeg(const AirSeg* newAs, const AirSeg* excAs) const;
  bool matchCarrierAndFlightNumber(const AirSeg* newAS, const AirSeg* excAs) const;
  void updateCalendarValidationResults(const TravelSeg& tvlSeg) const;
  bool matchedChangedSegOnNewItin(const TravelSeg* tvlSeg);

  std::vector<TravelSeg*> _travelSegVec;
  const std::vector<OriginDestination>& _ondVec;
  ExchShopCalendar::R3ValidationResult* _r3ValidationResults;
  const bool _isCalendar;
  const RexPricingTrx& _trx;

  friend class Cat31ChangeFinderTest;
};

} // tse
