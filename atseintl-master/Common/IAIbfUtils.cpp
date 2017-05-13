//-------------------------------------------------------------------
//
//  Authors:    Michal Mlynek
//
//  Copyright Sabre 2013
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

#include "Common/IAIbfUtils.h"

#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "ItinAnalyzer/BrandedFaresDataRetriever.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"

#include <functional>

namespace tse
{
FALLBACK_DECL(fallbackBrandDirectionality)

std::ostream& operator<<(std::ostream& stream, const IAIbfUtils::TripType tripType)
{
  switch (tripType)
  {
  case IAIbfUtils::ROUNDTRIP:
    stream << "ROUND TRIP";
    break;
  case IAIbfUtils::OPENJAW:
    stream << "OPEN JAW";
    break;
  case IAIbfUtils::ONEWAY:
    stream << "ONE WAY";
    break;
  default:
    stream << "UNDEFINED";
  }

  return stream;
}

OdcTuple::OdcTuple(const CarrierCode& carrier, const LocCode& org,
                   const LocCode& dest)
  : origin(org), destination(dest), governingCarrier(carrier)
{
}

OdcTuple::OdcTuple(const CarrierCode& carrier, const ODPair odPair)
  : origin(odPair.first), destination(odPair.second), governingCarrier(carrier)
{
}

bool
OdcTuple::
operator<(const OdcTuple& b) const
{
  if (origin < b.origin)
    return true;
  if (b.origin < origin)
    return false;
  if (destination < b.destination)
    return true;
  if (b.destination < destination)
    return false;
  if (governingCarrier < b.governingCarrier)
    return true;
  if (b.governingCarrier < governingCarrier)
    return false;

  return false;
}

std::ostream& operator<<(std::ostream& stream, const IAIbfUtils::OdcTuple& odcTuple)
{
 stream << odcTuple.origin << "-" << odcTuple.governingCarrier << "-" << odcTuple.destination;

  return stream;
}

bool
IAIbfUtils::ItinHasBrandsOnAllSegments::operator()(const Itin* itin) const
{
  std::set<TravelSeg*> brandedSegments;

  for (const FareMarket* fm : itin->fareMarket())
  {
    if (!fm->brandProgramIndexVec().empty())
    {
      brandedSegments.insert(fm->travelSeg().begin(), fm->travelSeg().end());
    }
  }

  return brandedSegments.size() == itin->travelSeg().size();
}

bool
IAIbfUtils::isTripOpenJaw(const Itin* itin, DataHandle& dataHandle)
{
  TSE_ASSERT(!itin->travelSeg().empty());

  const Loc* origin = itin->travelSeg().front()->origin();
  const Loc* destination = itin->travelSeg().back()->destination();

  TSE_ASSERT(origin != nullptr && destination != nullptr);

  std::vector<TravelSeg*> arunks = ItinUtil::getArunks(itin);

  if (arunks.size() > 1 || hasArunkThatSpreadsAcrossCountries(arunks))
    return false;

  // Turnaround OJ : one arunk within same country + origin matches destination

  if (LocUtil::isSameCity(origin->loc(), destination->loc(), dataHandle))
  {
    if (arunks.size() == 1) // if arunks were 0 that would mean a round trip
    {
      return true;
    }

    return false;
  }
  // Origin OJ : origin doesn't match destination but is within same country
  // and there's at least one part of the trip abroad ( international )
  // This time we match zero or one arunk. This reflects origin open jaw and a duble open jaw cases
  // respectively

  // This computation of international takes place because a regular international ( itin
  // geoTravelType considers US-CA as domestic
  // and in IBF we need to treat them as international as well. Hence the addition od IsTransborder
  // check
  bool isInternational =
      (itin->geoTravelType() == GeoTravelType::International) || (itin->geoTravelType() == GeoTravelType::Transborder);

  if (isInternational && LocUtil::isInSameCountry(*origin, *destination))
  {
    return true;
  }

  return false;
}

IAIbfUtils::TripType
IAIbfUtils::calculateTripType(const PricingTrx& trx)
{
  const Itin* itin = getRepresentativeItin(trx);
  TSE_ASSERT(itin != nullptr);

  ODPair origDestPair = getODPairFromItin(*itin);

  if (itin->travelSeg().size() == 1)
    return IAIbfUtils::ONEWAY;
  else if (isTripOpenJaw(itin, trx.dataHandle()))
    return IAIbfUtils::OPENJAW;
  else if (LocUtil::isSameCity(origDestPair.first, origDestPair.second, trx.dataHandle()))
    return IAIbfUtils::ROUNDTRIP;
  else
    return IAIbfUtils::ONEWAY;
}

bool
IAIbfUtils::hasArunkThatSpreadsAcrossCountries(const std::vector<TravelSeg*>& arunks)
{
  for (TravelSeg* tSeg : arunks)
  {
    const Loc* origin = tSeg->origin();
    const Loc* destination = tSeg->destination();
    // Arunk that spreads across countries disqualifies the request as an open jaw
    if (!LocUtil::isInSameCountry(*origin, *destination))
      return true;
  }
  return false;
}

const Itin*
IAIbfUtils::getRepresentativeItin(const PricingTrx& trx)
{
  if (trx.getTrxType() == PricingTrx::MIP_TRX ||
      trx.getTrxType() == PricingTrx::PRICING_TRX)
  {
    TSE_ASSERT(!trx.itin().empty());
    return trx.itin().front();
  }
  // It is assumed that this function can be called only from MIP or IS
  const ShoppingTrx* shoppingTrx = dynamic_cast<const ShoppingTrx*>(&trx);
  TSE_ASSERT(shoppingTrx != nullptr);
  return shoppingTrx->journeyItin();
}

// Function that retrieves origin and destination pair from the Itin data structure
ODPair
IAIbfUtils::getODPairFromItin(const Itin& itin)
{
  TSE_ASSERT(!itin.travelSeg().empty());

  const TravelSeg* firstTravelSeg = itin.travelSeg().front();
  const TravelSeg* lastTravelSeg = itin.travelSeg().back();
  const LocCode& origin = firstTravelSeg->origin()->loc();
  const LocCode& destination = lastTravelSeg->destination()->loc();

  return ODPair(origin, destination);
}

/*
FareMarket          OneWay                              RT,OJ,CT
----------          ------                              ---------
Matches Leg         Trip O&D + Leg O&D                  Leg O&D
Matcesh Multi Legs  Trip O&D + O 1st,D Last             O 1st, D Last
Extends Leg         Trip O&D + Significant Leg O&D      Significant O&D
Within Leg          Trip O&D + Leg O&D                  Leg O&D

Refer to IBF SRS for more info
*/
void
IAIbfUtils::fillOdcTupleVec(std::vector<OdcTuple>& odcTupleVec,
                            const OdDataForFareMarket& odData,
                            const CarrierCode& govCarrier,
                            const TripType tripType,
                            const BrandRetrievalMode mode)
{
  odcTupleVec.clear();

  if (mode == BrandRetrievalMode::PER_FARE_COMPONENT)
  {
    odcTupleVec.push_back(OdcTuple(govCarrier, odData.fmOD));
    return;
  }

  if (tripType == IAIbfUtils::ONEWAY)
  {
    odcTupleVec.push_back(OdcTuple(govCarrier, odData.tripOD));
    if (odData.legOD == odData.tripOD)
    {
      return;
    }
  }

  if (odData.isWithinLeg || odData.fmOD == odData.legOD)
  {
    odcTupleVec.push_back(OdcTuple(govCarrier, odData.legOD));
  }
  else
  {
    odcTupleVec.push_back(OdcTuple(govCarrier, odData.significantLegOD));
  }
}

const TravelSeg*
IAIbfUtils::getPrimarySector(const FareMarket* fm)
{
  const TravelSeg* primarySector = fm->primarySector();
  if (primarySector && (primarySector->segmentType() != Arunk))
    return primarySector;

  if (fm->travelSeg().size() == 1)
    return fm->travelSeg().front();

  std::vector<TravelSeg*> segsWithoutArunk;
  for (TravelSeg* tSeg : fm->travelSeg())
    if (tSeg->segmentType() != Arunk)
      segsWithoutArunk.push_back(tSeg);

  if (segsWithoutArunk.size() == 1)
    return segsWithoutArunk.front();

  TravelSegAnalysis tvlSegAnalysis;
  primarySector = tvlSegAnalysis.getHighestTPMSector(segsWithoutArunk);
  if (primarySector)
    return primarySector;

  return fm->travelSeg().front();
}

// This function should be used only in IS
void
IAIbfUtils::fillOdDataForFareMarketShopping(OdDataForFareMarket& odDataForFm,
                                            const Itin& currentItin,
                                            const Itin& journeyItin)
{
  // In IS Fms are always within legs or matching whole legs
  odDataForFm.isWithinLeg = true;
  // In IS, at this stage itin is just one leg ( real itins are not built yet )
  odDataForFm.legOD = getODPairFromItin(currentItin);

  // in IS, at this stage of processing JourneyItin is the only itin that contains tvlSegs covering
  // the whole trip
  odDataForFm.tripOD = getODPairFromItin(journeyItin);

  // Significant sector OD and FmOD don't matter for IS ( They are not processed in IBF logic
  // For the sake of completness they can all be set to LegOD
  odDataForFm.significantLegOD = odDataForFm.legOD;
  odDataForFm.fmOD = odDataForFm.legOD;
}

std::set<ODPair>
IAIbfUtils::calculateODPairsForFareMarket(const FareMarket& fareMarket,
  const Itin& itin, const IAIbfUtils::TripType tripType)
{
  std::set<ODPair> result;

  TSE_ASSERT(!itin.itinLegs().empty());
  std::map<const TravelSeg*, uint16_t> segmentMap;
  getSegmentMap(&itin, segmentMap);

  const std::vector<TravelSeg*>& travelSegVec = fareMarket.travelSeg();
  TSE_ASSERT(!travelSegVec.empty());

  uint16_t startLegId = segmentMap.at(travelSegVec.front());
  uint16_t endLegId = segmentMap.at(travelSegVec.back());

  if (tripType == IAIbfUtils::ONEWAY) // Whole trip is always included for one way itins
    result.insert(std::make_pair(itin.travelSeg().front()->origAirport(),
                                 itin.travelSeg().back()->destAirport()));

  if (startLegId == endLegId)
  {
    // either within leg, or exact match
    result.insert(std::make_pair(itin.itinLegs()[startLegId].front()->origAirport(),
                                 itin.itinLegs()[startLegId].back()->destAirport()));
  }
  else
  {
    if ((fareMarket.origin()->loc() == itin.itinLegs()[startLegId].front()->origAirport()) &&
        (fareMarket.destination()->loc() == itin.itinLegs()[endLegId].back()->destAirport()))
    {
      // exact match to multiple legs
      result.insert(std::make_pair(fareMarket.origin()->loc(), fareMarket.destination()->loc()));
    }
    else
    {
      // FM is extending the leg
      uint16_t primaryLegId = segmentMap.at(getPrimarySector(&fareMarket));
      result.insert(std::make_pair(itin.itinLegs()[primaryLegId].front()->origAirport(),
                                   itin.itinLegs()[primaryLegId].back()->destAirport()));
    }
  }

  return result;
}

// This function should be used only in MIP and Pricing
void
IAIbfUtils::findAllOdDataForMarketPricing(const Itin* itin,
                                          FareMarket* fm,
                                          const IAIbfUtils::TripType tripType,
                                          const BrandRetrievalMode mode,
                                          PricingTrx& trx,
                                          std::function<void (OdcTuple&)> function)
{
  TSE_ASSERT(fm != 0);
  TSE_ASSERT(itin != 0);

  std::set<ODPair> odPairs;

  // In this mode we simply use FM's origin and destination
  if (mode == BrandRetrievalMode::PER_FARE_COMPONENT)
    odPairs.insert(std::make_pair(fm->origin()->loc(), fm->destination()->loc()));
  else // retrieving brands per O&D
    odPairs = calculateODPairsForFareMarket(*fm, *itin, tripType);

  const bool useDirectionality =
    (!fallback::fallbackBrandDirectionality(&trx) &&
    ((trx.getTrxType() == PricingTrx::MIP_TRX) || (trx.getTrxType() == PricingTrx::PRICING_TRX)));

  if (useDirectionality)
  {
    for (const ODPair& odPair : odPairs)
    {
      OdcTuple od(fm->governingCarrier(), odPair);
      function(od);
      // On the old code we stored ORIGIN/DESTINATION requested for each fare market.
      // In new algorithm we store only ORIGIN (in originsRequested) for
      // ORIGINAL direction and DESTINATIONS (in destinationsRequested) for
      // REVERSED direction. In case of UNKNOWN or BOTHWAYS directions both
      // ORIGIN (of the ORIGINAL direction request) and DESTINATION (of the
      // REVERSED direction request) will be stored. This allows to distinguish
      // if program for fare market was requested in one or both ways.
      fm->addOriginRequestedForOriginalDirection(odPair.first);

      const ODPair reversedOD = std::make_pair(odPair.second, odPair.first);
      OdcTuple od2(fm->governingCarrier(), reversedOD);
      function(od2);
      fm->addDestinationRequestedForReversedDirection(reversedOD.second);
    }
  }
  else
  {
    for (const ODPair& odPair : odPairs)
    {
      OdcTuple od(fm->governingCarrier(), odPair);
      function(od);
      fm->addOriginRequestedForOriginalDirection(odPair.first);
    }
  }
}

// This function should be used only in MIP and Pricing
void
IAIbfUtils::fillOdDataForFareMarketPricing(const Itin* itin,
                                           FareMarket* fm,
                                           const IAIbfUtils::TripType tripType,
                                           const BrandRetrievalMode mode,
                                           std::vector<OdcTuple>& odcTupleVec,
                                           PricingTrx& trx)
{

  TSE_ASSERT(fm != 0);
  TSE_ASSERT(itin != 0);

  std::set<ODPair> odPairs;

  if (mode == BrandRetrievalMode::PER_FARE_COMPONENT)
  {
    // In this mode we simply use FM's origin and destination
    odPairs.insert(std::make_pair(fm->origin()->loc(), fm->destination()->loc()));
  }
  else // retrieving brands per O&D
  {
    odPairs = calculateODPairsForFareMarket(*fm, *itin, tripType);
  }

  const bool useDirectionality =
    (!fallback::fallbackBrandDirectionality(&trx) &&
    ((trx.getTrxType() == PricingTrx::MIP_TRX) || (trx.getTrxType() == PricingTrx::PRICING_TRX)));

  if (useDirectionality)
  {
    for (const ODPair& odPair : odPairs)
    {
      // UNKNOWN fare markets are always branded both ways so we want use different directionality
      // each time.
      odcTupleVec.push_back(OdcTuple(fm->governingCarrier(), odPair));

      // On the old code we stored ORIGIN/DESTINATION requested for each fare market.
      // In new algorithm we store only ORIGIN (in originsRequested) for
      // ORIGINAL direction and DESTINATIONS (in destinationsRequested) for
      // REVERSED direction. In case of UNKNOWN or BOTHWAYS directions both
      // ORIGIN (of the ORIGINAL direction request) and DESTINATION (of the
      // REVERSED direction request) will be stored. This allows to distinguish
      // if program for fare market was requested in one or both ways.
      fm->addOriginRequestedForOriginalDirection(odPair.first);

      const ODPair reversedOD = std::make_pair(odPair.second, odPair.first);
      odcTupleVec.push_back(OdcTuple(fm->governingCarrier(), reversedOD));
      fm->addDestinationRequestedForReversedDirection(reversedOD.second);
    }
  }
  else
  {
    for (const ODPair& odPair : odPairs)
    {
      odcTupleVec.push_back(OdcTuple(fm->governingCarrier(), odPair));
      fm->addOriginRequestedForOriginalDirection(odPair.first);
    }
  }
}

void
IAIbfUtils::getSegmentMap(const Itin* itin, std::map<const TravelSeg*, uint16_t>& segmentMap)
{
  for (uint16_t i = 0; i < itin->itinLegs().size(); ++i)
  {
    const TravelSegPtrVec& tSegs = itin->itinLegs()[i];
    for (TravelSeg* tSeg : tSegs)
      segmentMap[tSeg] = i;
  }
}

void
IAIbfUtils::invertOdcToFmMap(const FMsForBranding& inputMap, OdcsForBranding& result)
{
  for (FMsForBranding::const_iterator it = inputMap.begin(); it != inputMap.end(); ++it)
  {
    const OdcTuple& currentTuple = it->first;
    const std::vector<FareMarket*>& currentFareMarkets = it->second;
    for (size_t i = 0; i < currentFareMarkets.size(); ++i)
    {
      result[currentFareMarkets[i]].insert(currentTuple);
    }
  }
}

std::string
IAIbfUtils::legInfoToString(const ItinLegs& itinLegs)
{
  std::ostringstream ss;
  size_t legId = 1;
  for(auto leg : itinLegs)
  {
    if (leg.front()->segmentType() == Arunk)
      continue;

    if (legId > 1)
      ss << CROSS_LORRAINE;

    uint16_t startSegmentOrder = leg.front()->segmentOrder();
    uint16_t endSegmentOrder = leg.back()->segmentOrder();

    ss << "S" << startSegmentOrder;
    if (endSegmentOrder != startSegmentOrder)
      ss << "-" << endSegmentOrder;

    const BrandCode& brandCode = leg.front()->getBrandCode();
    if (!brandCode.empty())
      ss << "*BR" << brandCode;

    ss << "*LG" << legId;
    ++legId;
  }
  return ss.str();
}
}
