#pragma once

#include "Common/MCPCarrierUtil.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "ATAE/PricingDssFlightKey.h"

#include <map>

namespace tse
{
class AirSeg;
class DateTime;

enum class MethodGetFlownSchedule : unsigned int
{
  NORMAL_GET_FLOWN = 0,
  SKIP_FLOWN,
  USE_NEXT_SAME_DOW_FLT_FOR_FLOWN
};

class CarrierSwapUtil
{
  const PricingTrx& _trx;
public:
  CarrierSwapUtil(const PricingTrx& trx): _trx(trx) {}

  CarrierCode getCarrierCode(const CarrierCode& carrier) const
  {
    return MCPCarrierUtil::swapToPseudo(&_trx, carrier);
  }
};

class CurrentTimePolicy
{
public:
  DateTime getCurrentTime() const
  {
    return DateTime::localTime();
  }
};

template <class McpCarrierSwapT, class CurrentTimePolicyT>
class PricingDssFlightMapBuilder : public CurrentTimePolicyT
{
  const McpCarrierSwapT& _carrierSwapUtil;
  const MethodGetFlownSchedule _flownScheduleMethod;
  friend class PricingDssFlightMapBuilderTest;

public:
  PricingDssFlightMapBuilder(const McpCarrierSwapT& carrierSwapUtil, const MethodGetFlownSchedule flownScheduleMethod)
      : _carrierSwapUtil(carrierSwapUtil), _flownScheduleMethod(flownScheduleMethod)
  {}
  template <class TvlSegIter>
  void populateFlightMap(TvlSegIter begin, TvlSegIter end, PricingDssFlightMap& flightMap)
  {
    for (; begin != end; ++begin)
    {
      AirSeg* airSeg = dynamic_cast<AirSeg*>(*begin);
      if (airSeg == nullptr || airSeg->segmentType() == Open || airSeg->segmentType() == Arunk)
      {
        continue;
      }

      if (airSeg->flightNumber() == 0
          ||(!airSeg->unflown() && _flownScheduleMethod == MethodGetFlownSchedule::SKIP_FLOWN))
        continue;

      CarrierCode carrier = _carrierSwapUtil.getCarrierCode(airSeg->carrier());

      flightMap[PricingDssFlightKey(airSeg->departureDT(), calculateDssFlightDate(airSeg),
                                    airSeg->origAirport(), airSeg->destAirport(), carrier,
                                    airSeg->flightNumber(), airSeg->unflown())
              ].push_back(airSeg);
    }
  }

  DateTime calculateDssFlightDate(const AirSeg* airSeg) const
  {
    if (_flownScheduleMethod != MethodGetFlownSchedule::NORMAL_GET_FLOWN && !airSeg->unflown())
    {
      if (_flownScheduleMethod == MethodGetFlownSchedule::USE_NEXT_SAME_DOW_FLT_FOR_FLOWN)
      {
        const DateTime today = this->getCurrentTime();
        const int32_t currentDOW = today.date().day_of_week();
        const int32_t departDOW = airSeg->departureDT().date().day_of_week();
        const int32_t daysForNextSameDOW =
          (departDOW < currentDOW) ? (7 + departDOW - currentDOW) : (departDOW - currentDOW);

        return today.addDays(daysForNextSameDOW);
      }
    }
    return airSeg->departureDT();
  }
};

using FlightMapBuilder = PricingDssFlightMapBuilder<CarrierSwapUtil, CurrentTimePolicy>;
}

