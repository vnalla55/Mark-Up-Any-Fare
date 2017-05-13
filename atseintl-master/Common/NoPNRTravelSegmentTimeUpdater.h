#pragma once
#include <vector>

namespace tse
{
class PricingTrx;
class PricingUnit;
class TravelSeg;
class NoPNRPricingTrx;
class DateTime;

typedef std::pair<TravelSeg*, DateTime> WQUpdatedSegmentDate;

class NoPNRTravelSegmentTimeUpdater
{
public:
  NoPNRTravelSegmentTimeUpdater(const PricingUnit& pu, const PricingTrx& trx_);
  NoPNRTravelSegmentTimeUpdater(const std::vector<TravelSeg*>& segments, const PricingTrx& trx_);

  ~NoPNRTravelSegmentTimeUpdater();
  void updateOpenDateIfNeccesary(const TravelSeg* theSegment, DateTime& toUpdate);

private:
  // to be used only by (friend) NoPNRPricingTrx
  NoPNRTravelSegmentTimeUpdater();
  void initialize(const NoPNRPricingTrx& trx);

  // implementation
  void updateSegmentDates(const std::vector<TravelSeg*>& segments);
  bool updateSegmentDatesIfDifferent(TravelSeg& seg, DateTime& newDate);
  void findAndUpdateSegment(const TravelSeg* seg, DateTime& toUpdate);

  std::vector<WQUpdatedSegmentDate> _updatedDates;

  const std::vector<TravelSeg*>* _travelSegmentsVector;

  const PricingTrx* _trx;
  bool _alreadyProcessed;
  bool _wqTrx;

  friend class NoPNRPricingTrx;
  friend class NoPNRTravelSegmentTimeUpdaterTest;
};
}

