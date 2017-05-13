#include "Common/NoPNRTravelSegmentTimeUpdater.h"

#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
NoPNRTravelSegmentTimeUpdater::NoPNRTravelSegmentTimeUpdater(const PricingUnit& pu,
                                                             const PricingTrx& trx)
  : _travelSegmentsVector(&pu.travelSeg()),
    _trx(&trx),
    _alreadyProcessed(false),
    _wqTrx((dynamic_cast<const NoPNRPricingTrx*>(&trx) != nullptr))

{
}

NoPNRTravelSegmentTimeUpdater::NoPNRTravelSegmentTimeUpdater(
    const std::vector<TravelSeg*>& segments, const PricingTrx& trx)
  : _travelSegmentsVector(&segments),
    _trx(&trx),
    _alreadyProcessed(false),
    _wqTrx((dynamic_cast<const NoPNRPricingTrx*>(&trx) != nullptr))
{
}

NoPNRTravelSegmentTimeUpdater::NoPNRTravelSegmentTimeUpdater()
  : _travelSegmentsVector(nullptr), _trx(nullptr), _alreadyProcessed(false), _wqTrx(true)
{
}

void
NoPNRTravelSegmentTimeUpdater::initialize(const NoPNRPricingTrx& trx)
{
  _travelSegmentsVector = &trx.travelSeg();
  _trx = &trx;
  updateSegmentDates(*_travelSegmentsVector);
  _alreadyProcessed = true;
}

NoPNRTravelSegmentTimeUpdater::~NoPNRTravelSegmentTimeUpdater() {}

void
NoPNRTravelSegmentTimeUpdater::updateOpenDateIfNeccesary(const TravelSeg* theSegment,
                                                         DateTime& toUpdate)
{
  // if this is WQ trx, and we found some segment without date,
  // the segments dates may have to be recalculated
  if (!_wqTrx || theSegment == nullptr || !theSegment->hasEmptyDate())
    return; // never do anything for non-WQ trx or segment with date

  // lazy initialization
  if (!_alreadyProcessed)
  {
    if (_trx == nullptr || _travelSegmentsVector == nullptr)
    {
      return; // here: some problem - NoPNR transaction should've initialized() this object earlier!
    }
    _alreadyProcessed = true;
    // may need to change the dates for NoPNR (WQ) validation purposes
    updateSegmentDates(*_travelSegmentsVector);
  }

  // look for the segment - and update the date if it needs to be updated
  findAndUpdateSegment(theSegment, toUpdate);
}

void
NoPNRTravelSegmentTimeUpdater::findAndUpdateSegment(const TravelSeg* seg, DateTime& toUpdate)
{
  // look for the segment - and update the date if it needs to be updated
  for (std::vector<WQUpdatedSegmentDate>::const_iterator it = _updatedDates.begin();
       it != _updatedDates.end();
       ++it)
  {
    if ((*it).first == seg)
    {
      toUpdate = (*it).second;
      break;
    }
  }
}

void
NoPNRTravelSegmentTimeUpdater::updateSegmentDates(const std::vector<TravelSeg*>& segments)
{
  if (segments.empty())
    return;

  std::vector<TravelSeg*>::const_iterator lastSeg = segments.begin();
  std::vector<TravelSeg*>::const_iterator i = segments.begin();

  DateTime currentDate = (*i)->bookingDT(); // DateTime::localTime();
  DateTime lastDatedSegDepartureDT;
  // if first segment in PricingUnit is open - try to apply today date (if not already)
  if ((*i)->hasEmptyDate())
  {
    updateSegmentDatesIfDifferent(*(*i), currentDate);
    lastDatedSegDepartureDT = currentDate;
  }
  else
  {
    lastDatedSegDepartureDT = (*i)->departureDT();
  }

  ++i; // continue with other segments
  while (i != segments.end())
  {
    if ((*i) == nullptr)
      continue; // surface sector (?) - don't update

    TravelSeg& firstInSequence = *(*i);
    if (firstInSequence.hasEmptyDate())
    {
      // 'plus one' logic
      std::vector<TravelSeg*>::const_iterator j = i;

      // find the end of open segments sequence
      while (j != segments.end() && (*j)->hasEmptyDate())
        ++j;

      DateTime upperDateLimit;
      if (j != segments.end())
      {
        const DateTime& tmp = (*j)->departureDT();
        // set the hour to 0:00:00 - to make the limit stricter
        upperDateLimit = DateTime(tmp.year(), tmp.month(), tmp.day(), 0, 0, 0);
      }
      else
      {
        // no upper date limit
        upperDateLimit = DateTime(9999, 12, 31);
      }

      DateTime plusOneDate = lastDatedSegDepartureDT;
      for (std::vector<TravelSeg*>::const_iterator k = i; k != j; ++k)
      {
        // advance date if the limit is not exceeded
        if (plusOneDate < upperDateLimit)
          plusOneDate = plusOneDate.addDays(1);
        updateSegmentDatesIfDifferent(*(*k), plusOneDate);
      }
      // continue with segments that follow
      i = j;
    }
    lastSeg = i;
    if (i != segments.end())
    {
      lastDatedSegDepartureDT = (*i)->departureDT();
      ++i;
    }
  }
}

bool
NoPNRTravelSegmentTimeUpdater::updateSegmentDatesIfDifferent(TravelSeg& seg, DateTime& newDate)
{
  // update only if year/month/day differs (hours/minutes - not important here)
  if (newDate.year() != seg.arrivalDT().year() || newDate.month() != seg.arrivalDT().month() ||
      newDate.day() != seg.arrivalDT().day() || newDate.year() != seg.departureDT().year() ||
      newDate.month() != seg.departureDT().month() || newDate.day() != seg.departureDT().day())
  {
    _updatedDates.push_back(std::make_pair(&seg, newDate));
    return true;
  }
  else
  {
    return false;
  }
}
}
