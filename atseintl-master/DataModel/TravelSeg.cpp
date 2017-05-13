//-------------------------------------------------------------------
//
//  File:        TravelSeg.h
//  Created:
//  Authors:
//
//  Description: Common base class for any itinerary segment.
//
//               Class TravelSeg SHOULD NOT be instantiated by itself.
//               So there is no pool for such kind of objects.
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/05/04 - Mike Carroll - Initializer updates
//          08/05/05 - Tony Lam - Date Range
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
//-------------------------------------------------------------------

#include "DataModel/TravelSeg.h"

#include "Common/Assert.h"
#include "Common/ItinUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/Loc.h"
#include "Rules/RuleConst.h"

#include <stdexcept>

namespace tse
{

bool
TravelSeg::initialize()
{
  TSE_ASSERT(_origin != nullptr);
  TSE_ASSERT(_destination != nullptr);

  NationCode oNation = _origin->nation();
  NationCode dNation = _destination->nation();

  if (oNation == dNation)
  {
    if ((oNation == NATION_US) or (oNation == NATION_CA))
    {
      _geoTravelType = GeoTravelType::Domestic;
    }
    else
    {
      _geoTravelType = GeoTravelType::ForeignDomestic;
    }
  }

  else
  {
    if (((oNation == NATION_US) or (oNation == NATION_CA)) and
        ((dNation == NATION_US) or (dNation == NATION_CA)))
    {
      _geoTravelType = GeoTravelType::Transborder;
    }
    else
    {
      _geoTravelType = GeoTravelType::International;
    }
  }

  return true;
}

bool
TravelSeg::isFurthestPoint(const Itin& itin) const
{
  return itin.segmentOrder(this) == itin.furthestPointSegmentOrder();
}

bool
TravelSeg::RequestedFareBasis::operator==(const RequestedFareBasis& rhs) const
{
  if (this->passengerCode.empty() && rhs.passengerCode.empty() &&
      this->fareBasisCode == rhs.fareBasisCode)
    return true;

  return this->fareBasisCode == rhs.fareBasisCode && this->passengerCode == rhs.passengerCode &&
         this->vendor == rhs.vendor && this->carrier == rhs.carrier &&
         this->tariff == rhs.tariff && this->rule == rhs.rule &&
         std::fabs(this->amount - rhs.amount) < EPSILON;
}

bool
TravelSeg::matchFbc(const std::string& fbc,
                    const PaxTypeFare& ptf,
                    PricingTrx& trx) const
{
  if (fbc == ptf.fareClass())
    return true;

  std::string root;
  ptf.createFareBasisRoot(root);

  std::string fb = ptf.createFareBasis(trx);

  if (ptf.fare()->isIndustry())
  {
    const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(ptf.fare());
    if (indFare && indFare->changeFareClass())
    {
      std::string substr = fbc.substr(1);
      return root.substr(1) == substr || fb.substr(1) == substr;
    }
  }

  return root == fbc || fb == fbc;
}

bool
TravelSeg::isRequestedFareBasisValid(const RequestedFareBasis& req,
                                     const PaxTypeFare& ptf,
                                     PricingTrx& trx) const
{
  for (const RequestedFareBasis& rfb : _requestedFareBasis)
  {
    if (rfb.fareBasisCode.empty())
      continue;

    if (matchFbc(rfb.fareBasisCode, ptf, trx) &&
        (rfb.passengerCode.empty() ||
         (rfb.passengerCode == req.passengerCode && rfb.vendor == req.vendor &&
          rfb.carrier == req.carrier && rfb.tariff == req.tariff && rfb.rule == req.rule &&
          std::fabs(rfb.amount - req.amount) < EPSILON)))
    {
      return true;
    }
  }
  return false;
}

bool
TravelSeg::isInternationalSegment() const
{
  if (_origin == nullptr || _destination == nullptr)
    return false;

  if (_origin->nation() != _destination->nation())
    return true;

  return false;
}

bool
TravelSeg::isStopOver(const TravelSeg* travelSeg,
                      const GeoTravelType geoTravelType,
                      Application application,
                      int64_t minSeconds) const
{
  switch (application)
  {
  case OTHER:
    if (geoTravelType == GeoTravelType::International || geoTravelType == GeoTravelType::ForeignDomestic)
      return isStopOver(travelSeg, 86400); // more than or equal to one day
    else
      return isStopOver(travelSeg, 14400); // more than or equal to 4 hours
    break;

  case TAXES:
    return isStopOver(travelSeg, 43200); // more than or equal to 12 hours
    break;

  case BAGGAGE:
    if (UNLIKELY(travelSeg->isForcedStopOver()))
      return true;
    else
      return isStopOver(travelSeg, minSeconds);
    break;

  default:
    break;
  }
  return false;
}

// minSeconds - must stay this long to be considered a stopover
bool
TravelSeg::isStopOver(const TravelSeg* travelSeg, int64_t minSeconds) const
{
  if (UNLIKELY(!travelSeg))
  {
    return false;
  }

  if (UNLIKELY(travelSeg->isForcedConx()))
  {
    return false;
  }

  return isStopOverWithOutForceCnx(travelSeg, minSeconds);
}

bool
TravelSeg::isStopOverWithOutForceCnx(const TravelSeg* travelSeg, int64_t minSeconds) const
{
  if (UNLIKELY(!travelSeg))
    return false;

  if (segmentType() == tse::Open || travelSeg->segmentType() == tse::Open)
  {
    const DateTime& nextDepartDT = travelSeg->departureDT();
    if (_departureDT.isValid() && nextDepartDT.isValid())
    {
      if (_departureDT.year() != nextDepartDT.year() ||
          _departureDT.month() != nextDepartDT.month() || _departureDT.day() != nextDepartDT.day())
        return true;
      return false;
    }
    return false;
  }

  int64_t numSeconds = 0;
  // Special handling to Surface and Arunk travel segment.
  if (travelSeg->segmentType() == Surface || travelSeg->segmentType() == Arunk)
    numSeconds = DateTime::diffTime(travelSeg->arrivalDT(), travelSeg->departureDT());
  else if (segmentType() == Surface || segmentType() == Arunk)
    numSeconds = DateTime::diffTime(_arrivalDT, _departureDT);
  else
    numSeconds = DateTime::diffTime(_departureDT, travelSeg->arrivalDT());

  return (numSeconds > minSeconds);
}

bool
TravelSeg::isStopOver(const TravelSeg* nextTravelSeg,
                      const GeoTravelType geoTravelType,
                      const TimeAndUnit& minTime) const
{
  if (UNLIKELY(!nextTravelSeg))
  {
    return false;
  }

  const TravelSeg& earlierTravelSeg =
      ((this->departureDT() < nextTravelSeg->departureDT()) ? *this : *nextTravelSeg);

  if (UNLIKELY(earlierTravelSeg.isForcedConx()))
  {
    return false;
  }

  return isStopOverWithoutForceCnx(nextTravelSeg, geoTravelType, minTime);
}

bool
TravelSeg::isStopOverWithoutForceCnx(const TravelSeg* nextTravelSeg,
                                     const GeoTravelType geoTravelType,
                                     const TimeAndUnit& minTime) const
{
  if (UNLIKELY(!nextTravelSeg))
  {
    return false;
  }

  TimeAndUnit time = minTime;

  // Use defaults when not set.
  if (time.isEmpty())
  {
    if (geoTravelType == GeoTravelType::International || geoTravelType == GeoTravelType::ForeignDomestic)
    { // 24h
      time.set(static_cast<int16_t>(RuleConst::STOPOVER_SEC_INTL / SECONDS_PER_MINUTE),
               RuleConst::STOPOVER_TIME_UNIT_MINUTES);
    }
    else
    { // 4h
      time.set(static_cast<int16_t>(RuleConst::STOPOVER_SEC_DOMESTIC / SECONDS_PER_MINUTE),
               RuleConst::STOPOVER_TIME_UNIT_MINUTES);
    }
  }

  bool isStopOver = ItinUtil::isStopover(*this, *nextTravelSeg, geoTravelType, time);

  return isStopOver;
}

bool
TravelSeg::isOpenWithoutDate() const
{
  return (_segmentType == Open) && _pssDepartureDate.empty();
}

char
TravelSeg::bookedCabinChar() const
{
  if (_bookedCabin.isPremiumFirstClass())
    return PREMIUM_FIRST_CLASS;
  else if (_bookedCabin.isFirstClass())
    return FIRST_CLASS;
  else if (_bookedCabin.isPremiumBusinessClass())
    return PREMIUM_BUSINESS_CLASS;
  else if (_bookedCabin.isBusinessClass())
    return BUSINESS_CLASS;
  else if (_bookedCabin.isPremiumEconomyClass())
    return PREMIUM_ECONOMY_CLASS;
  return ECONOMY_CLASS;
}

char
TravelSeg::bookedCabinCharAnswer() const
{
  if (_bookedCabin.isPremiumFirstClass())
    return PREMIUM_FIRST_CLASS_ANSWER;
  else if (_bookedCabin.isFirstClass())
    return FIRST_CLASS_ANSWER;
  else if (_bookedCabin.isPremiumBusinessClass())
    return PREMIUM_BUSINESS_CLASS_ANSWER;
  else if (_bookedCabin.isBusinessClass())
    return BUSINESS_CLASS_ANSWER;
  else if (_bookedCabin.isPremiumEconomyClass())
    return PREMIUM_ECONOMY_CLASS_ANSWER;
  return ECONOMY_CLASS_ANSWER;
}

bool
TravelSeg::isAir() const
{
  return false;
}

AirSeg&
TravelSeg::toAirSegRef()
{
  throw std::runtime_error("It is not air segment");
}

const AirSeg&
TravelSeg::toAirSegRef() const
{
  throw std::runtime_error("It is not air segment");
}

bool
TravelSeg::isArunk() const
{
  return false;
}

ArunkSeg&
TravelSeg::toArunkSegRef()
{
  throw std::runtime_error("It is not arunk");
}

const ArunkSeg&
TravelSeg::toArunkSegRef() const
{
  throw std::runtime_error("It is not arunk");
}

bool
TravelSeg::isNonAirTransportation() const
{
  return !isAir() || _equipmentType == TRAIN || _equipmentType == TGV || _equipmentType == BUS ||
         _equipmentType == BOAT || _equipmentType == ICE || _equipmentType == LMO;
}

bool
TravelSeg::arunkMultiAirportForAvailability()
{
  if (UNLIKELY(_boardMultiCity == _offMultiCity))
    return true;

  if ((_boardMultiCity == LOC_EWR && _offMultiCity == LOC_NYC) ||
      (_boardMultiCity == LOC_NYC && _offMultiCity == LOC_EWR) ||
      (_boardMultiCity == LOC_BWI && _offMultiCity == LOC_WAS) ||
      (_boardMultiCity == LOC_WAS && _offMultiCity == LOC_BWI))
    return true;
  return false;
}

TravelSeg::ChangeStatus&
TravelSeg::changeStatus()
{
  TSE_ASSERT(itinIndex() < _changeStatus.size());
  return _changeStatus[itinIndex()];
}
const TravelSeg::ChangeStatus
TravelSeg::changeStatus() const
{
  TSE_ASSERT(itinIndex() < _changeStatus.size());
  return _changeStatus[itinIndex()];
}

std::vector<TravelSeg*>&
TravelSeg::newTravelUsedToSetChangeStatus()
{
  TSE_ASSERT(itinIndex() < _newTravelUsedToSetChangeStatus.size());
  return _newTravelUsedToSetChangeStatus[itinIndex()];
}
const std::vector<TravelSeg*>&
TravelSeg::newTravelUsedToSetChangeStatus() const
{
  TSE_ASSERT(itinIndex() < _newTravelUsedToSetChangeStatus.size());
  return _newTravelUsedToSetChangeStatus[itinIndex()];
}

bool&
TravelSeg::isCabinChanged()
{
  TSE_ASSERT(itinIndex() < _isCabinChanged.size());
  return _isCabinChanged[itinIndex()];
}
bool
TravelSeg::isCabinChanged() const
{
  TSE_ASSERT(itinIndex() < _isCabinChanged.size());
  return _isCabinChanged[itinIndex()];
}

bool
CompareSegOrderBasedOnItin::
operator()(TravelSeg* tvlSeg1, TravelSeg* tvlSeg2)
{
  return (_itin->segmentOrder(tvlSeg1) < _itin->segmentOrder(tvlSeg2));
}

} // namespace tse
