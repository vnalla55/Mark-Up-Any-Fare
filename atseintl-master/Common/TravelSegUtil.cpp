#include "Common/TravelSegUtil.h"

#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/LocUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"

namespace tse
{
const AirSeg*
TravelSegUtil::firstAirSeg(const std::vector<TravelSeg*>& tvlSeg)
{
  const AirSeg* airSeg = nullptr;
  for (std::vector<TravelSeg*>::const_iterator i = tvlSeg.begin(); i != tvlSeg.end() && !airSeg;
       ++i)
  {
    airSeg = dynamic_cast<AirSeg*>(*i);
  }

  return airSeg;
}

const AirSeg*
TravelSegUtil::lastAirSeg(const std::vector<TravelSeg*>& tvlSeg)
{
  const AirSeg* airSeg = nullptr;
  for (std::vector<TravelSeg*>::const_reverse_iterator i = tvlSeg.rbegin();
       i != tvlSeg.rend() && !airSeg;
       ++i)
  {
    airSeg = dynamic_cast<AirSeg*>(*i);
  }

  return airSeg;
}

const TravelSeg*
TravelSegUtil::firstNoArunkSeg(const std::vector<TravelSeg*>& tvlSegs)
{
  const TravelSeg* tvlSeg = nullptr;
  std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
  for (; tvlSegI != tvlSegs.end(); tvlSegI++)
  {
    tvlSeg = *tvlSegI;
    if (tvlSeg->segmentType() != Arunk)
      break;
  }
  return tvlSeg;
}

GeoTravelType
TravelSegUtil::getGeoTravelType(const std::vector<TravelSeg*>& tvlSeg, DataHandle& dh)
{

  bool withinOneCountry = false;
  bool outsideUSCA = false;

  std::vector<TravelSeg*>::const_iterator i = tvlSeg.begin();
  DateTime& travelDate = (*i)->departureDT();

  const Loc* origin = dh.getLoc((*i)->origin()->loc(), travelDate);

  VALIDATE_OR_THROW(origin != nullptr,
                    INVALID_INPUT,
                    "Could not get location " << (*i)->origin()->loc().c_str() << " for "
                                              << travelDate);
  if (origin->nation() != "US" && origin->nation() != "CA")
    outsideUSCA = true;

  for (; i != tvlSeg.end(); ++i)
  {
    const Loc* destination = dh.getLoc((*i)->destination()->loc(), travelDate);
    VALIDATE_OR_THROW(destination != nullptr,
                      INVALID_INPUT,
                      "Could not get location " << (*i)->destination()->loc().c_str() << " for "
                                                << travelDate);

    if (destination->nation() != "US" && destination->nation() != "CA")
      outsideUSCA = true;
    if (destination->nation() != origin->nation())
      withinOneCountry = false;
  }

  if (UNLIKELY(withinOneCountry))
  {
    if (outsideUSCA)
      return GeoTravelType::ForeignDomestic;
    else
      return GeoTravelType::Domestic;
  }
  else if (outsideUSCA)
    return GeoTravelType::International;
  else
    return GeoTravelType::Transborder;
}

uint32_t
TravelSegUtil::hashCode(const std::vector<TravelSeg*>& tvlSeg)
{
  tse::Hasher h(tse::Global::hasherMethod());
  for (const auto& elem : tvlSeg)
  {
    h << (unsigned long)elem;
  }
  return h.hash();
}

void
TravelSegUtil::setGeoTrvlTypeAndMltCity(AirSeg*& seg)
{
  const Loc* originLoc = seg->origin();
  const Loc* destinationLoc = seg->destination();

  if (originLoc != nullptr && destinationLoc != nullptr)
  {
    if (LocUtil::isInternational(*originLoc, *destinationLoc))
    {
      seg->geoTravelType() = GeoTravelType::International;
    }
    else if (LocUtil::isDomestic(*originLoc, *destinationLoc))
    {
      seg->geoTravelType() = GeoTravelType::Domestic;
    }
    else if (LocUtil::isTransBorder(*originLoc, *destinationLoc))
    {
      seg->geoTravelType() = GeoTravelType::Transborder;
    }
    else
    {
      seg->geoTravelType() = GeoTravelType::ForeignDomestic;
    }

    seg->boardMultiCity() = FareMarketUtil::getMultiCity(
        seg->carrier(), seg->origAirport(), seg->geoTravelType(), seg->departureDT());

    seg->offMultiCity() = FareMarketUtil::getMultiCity(
        seg->carrier(), seg->destAirport(), seg->geoTravelType(), seg->departureDT());
  }
}

void
TravelSegUtil::setupItinerarySegment(DataHandle& dataHandle,
                                     AirSeg*& segment,
                                     const DateTime& departureDT,
                                     const LocCode& boardCity,
                                     const LocCode& offCity,
                                     const CarrierCode& cxrCode,
                                     int16_t pnrSegment)
{

  segment->origAirport() = boardCity;
  segment->destAirport() = offCity;
  segment->departureDT() = departureDT;
  segment->arrivalDT() = departureDT;
  if (!departureDT.isEmptyDate())
  {
    segment->pssDepartureDate() = departureDT.dateToSqlString();
  }

  segment->earliestDepartureDT() = departureDT;
  segment->latestDepartureDT() = departureDT;
  segment->earliestArrivalDT() = departureDT;
  segment->latestArrivalDT() = departureDT;

  segment->bookingDT() = DateTime::localTime();
  segment->origin() = dataHandle.getLoc(boardCity, segment->departureDT());

  if (!segment->origin())
  {
    segment->origin() = dataHandle.getLoc(boardCity, DateTime::localTime());
  }

  segment->destination() = dataHandle.getLoc(offCity, DateTime::localTime());

  if (!segment->origin() || !segment->destination())
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "INVALID_ORIGIN_DESTINATION");
  }

  segment->carrier() = cxrCode;
  segment->setOperatingCarrierCode(cxrCode);

  segment->bookedCabin().setEconomyClass(); // lowest cabin

  segment->setBookingCode(DUMMY_BOOKING); // dummy booking code

  segment->resStatus() = "OK";

  segment->segmentType() = Open;

  segment->pnrSegment() = pnrSegment;
  segment->segmentOrder() = pnrSegment;

  setGeoTrvlTypeAndMltCity(segment);
}

std::vector<bool>
TravelSegUtil::calculateStopOvers(const std::vector<TravelSeg*>& tvlSeg,
                                  GeoTravelType geoTravelType,
                                  TravelSeg::Application application,
                                  bool shortSO)
{
  std::vector<bool> stopOver;

  const size_t tvlSegSize = tvlSeg.size();
  stopOver.resize(tvlSegSize, false);

  TravelSeg* firstSeg = nullptr;
  TravelSeg* secondSeg = nullptr;
  for (size_t i = 0; i < tvlSegSize - 1; i++) // - 1 last seg is always stopOver
  {
    firstSeg = tvlSeg[i];
    secondSeg = tvlSeg[i + 1];
    if (secondSeg->segmentType() == Surface || secondSeg->segmentType() == Arunk)
    {
      secondSeg->departureDT() = firstSeg->arrivalDT();
      if (tvlSegSize > i + 2)
        secondSeg->arrivalDT() = tvlSeg[i + 2]->departureDT();
    }
    stopOver[i] = secondSeg->isStopOver(firstSeg, geoTravelType, application, shortSO);
  }

  return stopOver;
}

std::vector<bool>
TravelSegUtil::calculateStopOversForMultiTkt(const std::vector<TravelSeg*>& tvlSeg,
                                             TravelSeg::Application application,
                                             bool shortSO)
{
  std::vector<bool> stopOver;

  const size_t tvlSegSize = tvlSeg.size();
  stopOver.resize(tvlSegSize, false);
  GeoTravelType geoTravelType = GeoTravelType::International;
  TravelSeg* firstSeg = nullptr;
  TravelSeg* secondSeg = nullptr;
  for (size_t i = 0; i < tvlSegSize - 1; i++) // - 1 last seg is always stopOver
  {
    firstSeg = tvlSeg[i];
    secondSeg = tvlSeg[i + 1];
    if (secondSeg->segmentType() == Arunk)
    {
      stopOver[i] = true;
      continue;
    }
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(secondSeg);
    if (airSeg)
      geoTravelType = airSeg->geoTravelType();
    stopOver[i] = secondSeg->isStopOver(firstSeg, geoTravelType, application, shortSO);
  }

  return stopOver;
}

// Function: Creates a new arunk segment and sets its origin/destination members so it fits between
//           its neighbouring segments.
//           Leg ID is left unset (-1) for backward compatibility reasons.
ArunkSeg*
TravelSegUtil::buildArunk(DataHandle& dataHandle, TravelSeg* leftSeg, TravelSeg* rightSeg)
{
  ArunkSeg* arunk = nullptr;
  arunk = dataHandle.create<ArunkSeg>();

  if (arunk)
  {
    // arunk->originalId() set in ArunkSeg's constructor to ARUNK_SEG_DEFAULT_ID = 65534.
    // arunk->legId() set in TravelSeg's constructor to -1 (unset).
    arunk->segmentType() = Arunk;
    arunk->origin() = leftSeg->destination();
    arunk->destination() = rightSeg->origin();
    arunk->origAirport() = leftSeg->destAirport();
    arunk->destAirport() = rightSeg->origAirport();
    arunk->boardMultiCity() = leftSeg->offMultiCity();
    arunk->offMultiCity() = rightSeg->boardMultiCity();
  }

  return arunk;
}

// Function: Checks whether all travel segments have the same carrier, i.e. are online.
bool
TravelSegUtil::isTravelSegVecOnline(const std::vector<TravelSeg*>& tSegs)
{
  std::vector<TravelSeg*>::const_iterator it = tSegs.begin();
  std::vector<TravelSeg*>::const_iterator endIt = tSegs.end();
  CarrierCode prevCarrier = "";

  for (; it != endIt; ++it)
  {
    if (!(*it)->isAir())
      continue;

    if ("" == prevCarrier)
    {
      prevCarrier = dynamic_cast<const AirSeg*>(*it)->marketingCarrierCode();
      continue;
    }

    if (prevCarrier != dynamic_cast<const AirSeg*>(*it)->marketingCarrierCode())
      return false;
  }

  return true;
}

bool
TravelSegUtil::isNotUSCanadaOrCanadaUS(const TravelSeg* segment)
{
  const NationCode& origin = segment->origin()->nation();
  const NationCode& destination = segment->destination()->nation();

  return (origin != destination && !((origin == UNITED_STATES && destination == CANADA) ||
                                     (origin == CANADA && destination == UNITED_STATES)));
}

std::vector<SegmentAttributes>
TravelSegUtil::calcSegmentAttributes(const std::vector<TravelSeg*>& segs)
{
  std::vector<SegmentAttributes> allAttrs;
  allAttrs.reserve(segs.size());

  for (TravelSeg* const ts : segs)
  {
    uint16_t attr = 0;

    if (ts->segmentType() == Air && ts->isAir() && ts->toAirSeg()->forcedSideTrip() == 'T')
      attr |= FORCED_SIDE_TRIP_ELEMENT;

    allAttrs.push_back({ts, attr});
  }

  // Do side trips
  for (size_t stFirst = 1; stFirst + 1 < segs.size(); ++stFirst)
  {
    for (size_t stLast = stFirst + 1; stLast + 1 < segs.size(); ++stLast)
    {
      if (segs[stFirst]->boardMultiCity() != segs[stLast]->offMultiCity())
        continue;

      if (segs[stFirst - 1]->boardMultiCity() == segs[stLast + 1]->boardMultiCity())
        continue;

      for (size_t stI = stFirst; stI <= stLast; ++stI)
      {
        if (segs[stI]->boardMultiCity() == segs[stFirst]->boardMultiCity())
          allAttrs[stI].attributes |= SIDE_TRIP_BEGIN;
        else if (segs[stI]->offMultiCity() == segs[stLast]->offMultiCity())
          allAttrs[stI].attributes |= SIDE_TRIP_END;
        else
          allAttrs[stI].attributes |= SIDE_TRIP_ELEMENT;
      }
    }
  }

  return allAttrs;
}

void
TravelSegUtil::setSegmentAttributes(std::vector<TravelSeg*>& tvlSegs,
                                    std::vector<SegmentAttributes>& lclFareMarket)
{
  const size_t numSegs = tvlSegs.size();

  lclFareMarket.reserve(numSegs);
  for (size_t i = 0; i < numSegs; ++i)
  {
    SegmentAttributes segAtts = {tvlSegs[i], 0};

    TravelSegType& tvlSegType = tvlSegs[i]->segmentType();
    if (tvlSegType == Arunk)
    {
      segAtts.attributes |= ARUNK;
    }
    else if (tvlSegType == Open) // Treat same as air seg   /

    {
      segAtts.attributes |= OPEN;
    }
    else if (LIKELY(tvlSegType == Air))
    {
      AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSegs[i]);
      if (LIKELY(airSeg != nullptr))
      {
        // Forced attributes
        if (UNLIKELY(airSeg->forcedSideTrip() == 'T'))
          segAtts.attributes |= FORCED_SIDE_TRIP_ELEMENT;
        if (UNLIKELY(airSeg->forcedFareBrk() == 'T'))
          segAtts.attributes |= FORCED_FARE_BREAK;
        if (UNLIKELY(airSeg->forcedNoFareBrk() == 'T'))
          segAtts.attributes |= FORCED_NO_FARE_BREAK;
      }
    }

    // Start of trip?
    if (i == 0)
      segAtts.attributes |= TRIP_BEGIN;

    // End of trip?
    if (i + 1 >= numSegs)
      segAtts.attributes |= TRIP_END;

    lclFareMarket.push_back(segAtts);
  }

  // Do side trips

  const size_t tvlSegsSize = tvlSegs.size();
  for (size_t i = 0; i < tvlSegsSize; i++)

  {
    if ((lclFareMarket[i].attributes & TRIP_BEGIN) || (lclFareMarket[i].attributes & TRIP_END))
    {
      continue;
    }

    for (size_t j = i + 1; j < tvlSegsSize; j++)
    {
      // End of the trip... then no side trip
      if (lclFareMarket[j].attributes & TRIP_END)
        break;

      if (lclFareMarket[i].tvlSeg->boardMultiCity() == lclFareMarket[j].tvlSeg->offMultiCity())
      {
        if (lclFareMarket[i - 1].tvlSeg->boardMultiCity() ==
            lclFareMarket[j + 1].tvlSeg->boardMultiCity())
        {
          continue;
        }

        for (size_t k = i; k <= j; k++)
        {
          if (k == i ||
              lclFareMarket[i].tvlSeg->boardMultiCity() ==
                  lclFareMarket[k].tvlSeg->boardMultiCity())
          {
            lclFareMarket[k].attributes |= SIDE_TRIP_BEGIN;
          }
          else if (k == j ||
                   lclFareMarket[i].tvlSeg->boardMultiCity() ==
                       lclFareMarket[k].tvlSeg->offMultiCity())
          {
            lclFareMarket[k].attributes |= SIDE_TRIP_END;
          }
          else
          {
            lclFareMarket[k].attributes |= SIDE_TRIP_ELEMENT;
          }
        }
      }
    }
  }
}
}
