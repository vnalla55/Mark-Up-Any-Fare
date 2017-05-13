//----------------------------------------------------------------------------
//
//  File:           ItinUtil.cpp
//  Created:        5/5/2004
//  Authors:
//
//  Description:    Common functions required for ATSE shopping/pricing.
//
//  Updates:
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

#include "Common/ItinUtil.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyUtil.h"
#include "Common/DateTime.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/RtwUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/AirlineAllianceContinentInfo.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag192Collector.h"
#include "Util/BranchPrediction.h"
#include "Util/FlatMap.h"

#include <algorithm>
#include <map>
#include <set>

namespace tse
{
FALLBACK_DECL(reduceTemporaryVectors);
namespace
{
Logger
logger("atseintl.Common.ItinUtil");

ConfigurableValue<double>
pointForSopScore("SHOPPING_OPT", "POINT_FOR_SOP_SCORE");
ConfigurableValue<uint32_t>
numberOfClasserForPopScore("SHOPPING_OPT", "NUMBER_OF_CLASSES_FOR_SOP_SCORE");
}

const DateTime ItinUtil::_today = DateTime::localTime();
const std::string ItinUtil::nationListSwapValidCXR =
    ("AF BD BN BT CC CN HK HM ID IO JP KG KH KP KR "
     "KZ LA LK MM MN MO MV MY NP PH PK SG TH TJ TL "
     "TM TW UZ VN XU AU CK CX FJ FM KI MH MP NC NF "
     "NR NU NZ PC PF PG PN PW SB TK TO TV VU WF WS");

template <typename T>
inline static void
removeNonRtwFareMarkets(PricingTrx& trx, std::vector<T*>& itins)
{
  typedef std::vector<FareMarket*> FmVec;
  typedef FmVec::reverse_iterator FmRI;

  for (T* itin : itins)
  {
    FmVec rtwFm;
    FmRI rtwFmI = std::find_if(itin->fareMarket().rbegin(),
                               itin->fareMarket().rend(),
                               [&itin](const FareMarket* fm)
                               { return RtwUtil::isRtwFareMarket(*itin, fm); });

    if (rtwFmI == itin->fareMarket().rend())
      ItinUtil::roundTheWorldDiagOrThrow(trx, dynamic_cast<ExcItin*>(itin));
    else
      rtwFm.push_back(*rtwFmI);

    itin->fareMarket().swap(rtwFm);
  }
}

void
ItinUtil::removeAtaeMarkets(PricingTrx& trx)
{
  if (RtwUtil::isRtw(trx) && trx.diagnostic().diagnosticType() != Diagnostic199)
  {
    if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    {
      BaseExchangeTrx& bet = static_cast<BaseExchangeTrx&>(trx);
      removeNonRtwFareMarkets(trx, bet.newItin());
      removeNonRtwFareMarkets(trx, bet.exchangeItin());
      bet.fareMarket() = bet.newItin().front()->fareMarket();
    }
    else
    {
      removeNonRtwFareMarkets(trx, trx.itin());
      trx.fareMarket() = trx.itin().front()->fareMarket();
    }
  }
}

namespace
{
template <typename Condition>
class SectorCounter
{
public:
  explicit SectorCounter(const std::vector<TravelSeg*>& travelSeg)
  {
    _counter = std::count_if(travelSeg.begin(), travelSeg.end(), _cond);
  }

  std::vector<TravelSeg*>::size_type size() const { return _counter; }

private:
  std::vector<TravelSeg*>::size_type _counter;
  Condition _cond;
};

class EasternHemisphereOrigin : public std::unary_function<const TravelSeg*, bool>
{
public:
  bool operator()(const TravelSeg* ts) const
  {
    return ts->origin()->area() == IATA_AREA2 || ts->origin()->area() == IATA_AREA3;
  }
};

class WesternHemisphereOrigin : public std::unary_function<const TravelSeg*, bool>
{
public:
  bool operator()(const TravelSeg* ts) const { return ts->origin()->area() == IATA_AREA1; }
};

class SurfaceAcrossAreas : public std::unary_function<const TravelSeg*, bool>
{
public:
  bool operator()(const TravelSeg* ts) const
  {
    return (ts->segmentType() == Surface || ts->segmentType() == Arunk) &&
           ts->origin()->area() != ts->destination()->area();
  }
};

struct InternationalSector : public std::unary_function<const TravelSeg*, bool>
{
  bool operator()(const TravelSeg* ts) const
  {
    const Loc& org = *ts->origin();
    const Loc& dst = *ts->destination();
    const bool isDomestic = org.nation() == dst.nation() || LocUtil::isDomesticUSCA(org, dst) ||
                            LocUtil::isDomesticRussia(org, dst);
    return !isDomestic;
  }
};

struct IsArea
{
  explicit IsArea(const IATAAreaCode& area) : _area(area) {}
  bool operator()(const Loc* l) const { return l->area() == _area; }

private:
  const IATAAreaCode& _area;
};

struct IsArea2Or33
{
  bool operator()(const Loc* l) const
  {
    return l->area() == IATA_AREA2 || l->subarea() == IATA_SUB_ARE_33();
  }
};

struct IsArea3Wo33
{
  bool operator()(const Loc* l) const
  {
    return l->area() == IATA_AREA3 && l->subarea() != IATA_SUB_ARE_33();
  }
};

template <class LocPredA, class LocPredB>
inline bool
containsCrossing(const std::vector<const Loc*>& locs, LocPredA predA, LocPredB predB)
{
  const auto marketPred = [&](const Loc* locA, const Loc* locB)
  { return (predA(locA) && predB(locB)) || (predA(locB) && predB(locA)); };

  return std::adjacent_find(locs.begin(), locs.end(), marketPred) != locs.end();
}
} // anonymous ns

void
AreaCrossingDeterminator::determine(const Trx& trx, const std::vector<TravelSeg*>& segs)
{
  for (const TravelSeg* ts : segs)
    determine(trx, *ts);
}

void
AreaCrossingDeterminator::determine(const Trx& trx, const TravelSeg& ts)
{
  std::vector<const Loc*> areas;
  areas.push_back(ts.origin());

  for (const Loc* loc : ts.hiddenStops())
    areas.push_back(loc);

  areas.push_back(ts.destination());

  if (containsCrossing(areas, IsArea(IATA_AREA1), IsArea2Or33()))
    _transatlanticSectors.push_back(&ts);
  if (containsCrossing(areas, IsArea(IATA_AREA1), IsArea3Wo33()))
    _transpacificSectors.push_back(&ts);
  if (containsCrossing(areas, IsArea(IATA_AREA2), IsArea(IATA_AREA3)))
    _area2area3crossing.push_back(&ts);
}

bool
AreaCrossingDeterminator::find(const TravelSeg& ts, const Segments& destination) const
{
  return std::find(destination.begin(), destination.end(), &ts) != destination.end();
}

inline static bool
isRwFareMarket(const std::vector<TravelSeg*>& segs, AreaCrossingDeterminator& acd)
{
  if (acd.transatlanticSectors().size() != 1)
    return false;

  if (acd.transpacificSectors().size() != 1)
    return false;

  if (acd.area2area3crossing().size() > 1)
    return false;

  // transoceanic farebreak surfaces not permitted
  if (acd.isTransoceanicSurface(*segs.front()) || acd.isTransoceanicSurface(*segs.back()))
    return false;

  // #transoceanic embedded surfaces is limited to 1
  if (!acd.transatlanticSectors().front()->isAir() && !acd.transpacificSectors().front()->isAir())
    return false;

  return true;
}

void
ItinUtil::setRoundTheWorld(PricingTrx& trx, Itin& itin)
{
  itin.tripCharacteristics().set(Itin::OneWay, true);
  itin.tripCharacteristics().set(Itin::RoundTrip, false);

  const bool isExcItin = dynamic_cast<const ExcItin*>(&itin) != nullptr;
  const std::vector<TravelSeg*>& segs = itin.travelSeg();

  if (segs.size() <= 1 || (segs.size() == 2 && RtwUtil::isRtwArunk(trx, segs.back())))
  {
    roundTheWorldDiagOrThrow(trx, isExcItin);
    return;
  }

  if (std::find_if(segs.begin(), segs.end(), InternationalSector()) == segs.end())
  {
    roundTheWorldDiagOrThrow(trx, isExcItin);
    return;
  }

  AreaCrossingDeterminator acd;
  acd.determine(trx, segs);

  if (isRwFareMarket(segs, acd))
  {
    itin.tripCharacteristics().set(Itin::RW_SFC, true);
    return;
  }

  SectorCounter<EasternHemisphereOrigin> ehSectors(segs);
  SectorCounter<WesternHemisphereOrigin> whSectors(segs);
  SectorCounter<SurfaceAcrossAreas> asSectors(segs);

  if (asSectors.size() == 0 &&
      (whSectors.size() == segs.size() || ehSectors.size() == segs.size() ||
       (acd.transatlanticSectors().size() == 2 && acd.transpacificSectors().empty()) ||
       (acd.transatlanticSectors().empty() && acd.transpacificSectors().size() == 2)))
  {
    itin.tripCharacteristics().set(Itin::CT_SFC, true);
    return;
  }

  roundTheWorldDiagOrThrow(trx, isExcItin);
}

int16_t
ItinUtil::gcmBasedFurthestPoint(Itin& itin, Diag192Collector* dc)
{
  uint32_t highestGcm = 0;

  std::vector<TravelSeg*>::iterator tsi = itin.travelSeg().begin();
  std::vector<TravelSeg*>::iterator turnaroundSegment = itin.travelSeg().end();
  const std::vector<TravelSeg*>::const_iterator tse = --itin.travelSeg().end();

  for (; tsi != tse; ++tsi)
  {
    const uint32_t gcm =
        TseUtil::greatCircleMiles(*itin.travelSeg().front()->origin(), *(**tsi).destination());

    if (dc)
    {
      *dc << std::right << std::setw(2) << (**tsi).pnrSegment() << " "
          << itin.travelSeg().front()->origAirport() << (**tsi).destAirport()
          << " GREAT CIRCLE MILES: " << gcm;

      *dc << "\n";
    }

    if (highestGcm < gcm)
    {
      highestGcm = gcm;
      turnaroundSegment = tsi;
    }
  }

  if (dc)
  {
    if (turnaroundSegment != itin.travelSeg().end())
      *dc << "\nTURNAROUND/FURTHEST POINT:   " << (**turnaroundSegment).pnrSegment() << " "
          << (**turnaroundSegment).destAirport() << "   DISTANCE: " << highestGcm << "\n";
    else
      *dc << "\nERROR: TURNAROUND/FURTHEST POINT NOT SET!\n";
  }

  if (turnaroundSegment == itin.travelSeg().end())
  {
    LOG4CXX_ERROR(logger, "GCM BASED FURTHEST POINT FAILED !!!");
    return 0;
  }

  (**turnaroundSegment).furthestPoint() = true;
  return static_cast<int16_t>((turnaroundSegment - itin.travelSeg().begin()) + 1);
}

void
ItinUtil::roundTheWorldDiagOrThrow(PricingTrx& trx, bool isExcItin)
{
  const bool excErrorApplicable = isExcItin && trx.excTrxType() == PricingTrx::AR_EXC_TRX;
  const bool newErrorApplicable = !isExcItin && trx.excTrxType() != PricingTrx::AF_EXC_TRX;
  // c33 new = flown portion

  if (excErrorApplicable && !static_cast<BaseExchangeTrx&>(trx).isExcRtw())
    throw ErrorResponseException(
        ErrorResponseException::NON_RTW_FARE_CANNOT_BE_EXCHANGED_FOR_RTW_FARE);

  if (trx.diagnostic().diagnosticType() == Diagnostic192)
  {
    DCFactory* factory = DCFactory::instance();
    Diag192Collector* diagPtr = dynamic_cast<Diag192Collector*>(factory->create(trx));
    diagPtr->enable(Diagnostic192);
    diagPtr->printLine();

    if (excErrorApplicable)
      *diagPtr << "ALLEGED RTW EXCHANGE FARE DO NOT MET TRIP CHARACTERISTIC\n";

    if (newErrorApplicable)
      *diagPtr << "RTW/CT FARE NOT APPLICABLE / USE ALTERNATE PRICING COMMAND\n";

    diagPtr->flushMsg();
  }

  else if (trx.diagnostic().diagnosticType() != DiagnosticNone &&
           trx.diagnostic().diagnosticType() < AllFareDiagnostic)
    return;

  else if (excErrorApplicable)
    throw ErrorResponseException(
        ErrorResponseException::ALLEGED_RTW_EXCHANGE_FARE_DO_NOT_MET_TRIP_CHARACTERISTIC);

  else if (newErrorApplicable)
    throw ErrorResponseException(
        ErrorResponseException::RW_CT_FARE_NOT_APPLICABLE_USE_ALTERNATE_PRICING_COMMAND);
}

//-----------------------------------------------------------------------------
// setFurthestPoint()
//-----------------------------------------------------------------------------
bool
ItinUtil::setFurthestPoint(PricingTrx& trx, Itin* itin)
{
  if (UNLIKELY(trx.getOptions()->isRtw()))
  {
    TSE_ASSERT(itin);
    itin->furthestPointSegmentOrder() = ItinUtil::gcmBasedFurthestPoint(*itin);
    return true;
  }

  const AirSeg* startFlt;
  const AirSeg* currFlt;

  const std::vector<TravelSeg*>& travelSeg = itin->travelSeg();

  std::vector<TravelSeg*>::const_iterator i;
  std::vector<TravelSeg*>::const_iterator furthestI;

  uint32_t highMiles = 0;
  uint32_t currentMiles = 0;

  i = travelSeg.begin();
  startFlt = dynamic_cast<AirSeg*>((*i));

  furthestI = i;

  for (; i != travelSeg.end(); i++)
  {
    if (UNLIKELY(!(*i)->destination()))
      return false;

    currFlt = dynamic_cast<AirSeg*>((*i));

    if (!currFlt)
      continue;

    currentMiles = journeyMileage(startFlt, currFlt, trx, itin->travelDate());

    if (currentMiles > highMiles)
    {
      highMiles = currentMiles;
      furthestI = i;
    }

    (*i)->furthestPoint() = false;
  }

  (*furthestI)->furthestPoint() = true;

  itin->furthestPointSegmentOrder() = itin->segmentOrder(*furthestI);

  return true;
}

//-----------------------------------------------------------------------------
// getFurthestPoint()
//-----------------------------------------------------------------------------
const TravelSeg*
ItinUtil::getFurthestPoint(const Itin* itin)
{
  const std::vector<TravelSeg*>& travelSeg = itin->travelSeg();

  std::vector<TravelSeg*>::const_iterator i;

  for (i = travelSeg.begin(); i != travelSeg.end(); i++)
  {
    if ((*i)->furthestPoint())
    {
      return *i;
    }
  }

  return nullptr;
}

//-----------------------------------------------------------------------------
// originNation()
//-----------------------------------------------------------------------------
NationCode
ItinUtil::originNation(const Itin& itin)
{
  if (LIKELY(itin.travelSeg().size() > 0))
  {
    return itin.travelSeg().front()->origin()->nation();
  }
  else
  {
    return "";
  }
}

//-----------------------------------------------------------------------------
// getFirstValidTravelSeg
//-----------------------------------------------------------------------------
const TravelSeg*
ItinUtil::getFirstValidTravelSeg(const Itin* itin)
{
  std::vector<TravelSeg*>::const_iterator iter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = itin->travelSeg().end();

  for (; iter != iterEnd; iter++)
  {
    TravelSeg* tvlSeg = *iter;
    if (tvlSeg->segmentType() == Open || tvlSeg->segmentType() == Arunk)
      continue;
    return tvlSeg;
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
// getLastValidTravelSeg
//-----------------------------------------------------------------------------
const TravelSeg*
ItinUtil::getLastValidTravelSeg(const Itin* itin)
{
  std::vector<TravelSeg*>::const_reverse_iterator iter = itin->travelSeg().rbegin();
  std::vector<TravelSeg*>::const_reverse_iterator iterEnd = itin->travelSeg().rend();

  for (; iter != iterEnd; iter++)
  {
    TravelSeg* tvlSeg = *iter;
    if (tvlSeg->segmentType() == Open || tvlSeg->segmentType() == Arunk)
      continue;
    return tvlSeg;
  }
  return nullptr;
}

bool
ItinUtil::atleastOneSegmentConfirm(const Itin* itin)
{
  for (const TravelSeg* travelSeg : itin->travelSeg())
  {
    if (travelSeg->resStatus() == CONFIRM_RES_STATUS)
      return true;
  }
  return false;
}

//---------------------------------------------------------------------
//
//   @method getOriginationCurrency
//
//   @Description Determines the itinerary origination currency
//
//   @param Itin
//   @param CurrencyCode
//
//   @return bool - true, retrieved origin nation currency , else false;
//---------------------------------------------------------------------
bool
ItinUtil::getOriginationCurrency(const Itin& itin,
                                 CurrencyCode& originationCurrency,
                                 const DateTime& ticketingDate,
                                 const bool& isFareDisplayTrx)
{
  LOG4CXX_INFO(logger, " Entered ItinUtil::getOriginationCurrency()");

  NationCode nationCode = originNation(itin);

  bool getNationRC = false;

  if (itin.tripCharacteristics().isSet(Itin::RussiaOnly))
    getNationRC = CurrencyUtil::getNationalCurrency(nationCode, originationCurrency, ticketingDate);
  else if (itin.geoTravelType() == GeoTravelType::International)
    getNationRC = CurrencyUtil::getPricingCurrency(nationCode, originationCurrency, ticketingDate);
  else
    getNationRC = CurrencyUtil::getNationCurrency(nationCode, originationCurrency, ticketingDate);

  if (UNLIKELY(!getNationRC))
    return false;

  LOG4CXX_INFO(logger, " Origination currency " << originationCurrency);

  LOG4CXX_INFO(logger, " Leaving ItinUtil::getOriginationCurrency()");

  return true;
}

const DateTime&
ItinUtil::getTravelDate(const std::vector<TravelSeg*>& travelSegs)
{
  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  std::vector<TravelSeg*>::const_iterator end = travelSegs.end();

  for (; i != end; ++i)
  {
    TravelSeg* tvlSeg = *i;

    if (UNLIKELY(tvlSeg == nullptr))
      continue;

    const DateTime& travelDate = tvlSeg->departureDT();

    if (LIKELY(!travelDate.isInfinity()))
      return travelDate;
  }

  return _today;
}

bool
ItinUtil::isOpenSegAfterDatedSeg(const Itin& itin, const TravelSeg* tvlSeg)
{
  if (!tvlSeg || tvlSeg->segmentType() != Open || !tvlSeg->pssDepartureDate().empty())
    return false;

  const std::vector<TravelSeg*>& tvlSegs = itin.travelSeg();
  std::vector<TravelSeg*>::const_reverse_iterator i = tvlSegs.rbegin();
  for (; i != tvlSegs.rend(); i++)
  {
    TravelSeg* curTvlSeg = *i;
    if (curTvlSeg == tvlSeg) // Found the open seg
      return curTvlSeg != tvlSegs.front(); // First seg in itin

    if (!curTvlSeg->pssDepartureDate().empty()) // Found the dated seg
      return false;
  }

  return false;
}

//-----------------------------------------------------------------------------
// isRussia
//-----------------------------------------------------------------------------
bool
ItinUtil::isRussian(const Itin* itin)
{
  std::vector<TravelSeg*>::const_iterator iter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = itin->travelSeg().end();

  for (; iter != iterEnd; iter++)
  {
    TravelSeg* tvlSeg = *iter;

    if (!LocUtil::isRussianGroup(*(tvlSeg->origin())) ||
        !LocUtil::isRussianGroup(*(tvlSeg->destination())))
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
// journeys
//-----------------------------------------------------------------------------
void
ItinUtil::journeys(Itin& itin,
                   PricingTrx& trx,
                   std::vector<FareMarket*>& jnyMarkets,
                   std::vector<TravelSegJourneyType>* journeyOrder)
{
  const AirSeg* airSeg = nullptr;
  size_t flowLen = 0;
  const size_t tvlCount = itin.travelSeg().size();
  for (size_t tvlIndex = 0; tvlIndex < tvlCount; ++tvlIndex)
  {
    airSeg = dynamic_cast<const AirSeg*>(itin.travelSeg()[tvlIndex]);
    if (airSeg == nullptr)
    {
      if (journeyOrder)
        journeyOrder->push_back(TravelSegJourneyType::ArunkSegment);

      continue;
    }

    if (journeyOrder)
      journeyOrder->push_back(TravelSegJourneyType::NonJourneySegment);

    // See if this is start of a "journey by marriage" segment
    const OAndDMarket* od = JourneyUtil::getOAndDMarketFromSegment(airSeg, &itin);
    if (UNLIKELY(od && od->isJourneyByMarriage()))
    {
      FareMarket* fm = const_cast<FareMarket*>(od->fareMarket());
      jnyMarkets.push_back(fm);
      flowLen = fm->travelSeg().size();
    }
    else
    {
      if (tvlIndex + 1 == tvlCount)
      {
        if (!(airSeg->flowJourneyCarrier() || airSeg->localJourneyCarrier()))
          continue;
      }
      else
      {
        const AirSeg* nextSeg = itin.travelSeg()[tvlIndex + 1]->toAirSeg();
        if ((nextSeg == nullptr) ||
            (!TrxUtil::intralineAvailabilityApply(trx, airSeg->carrier(), nextSeg->carrier()) &&
             !TrxUtil::interlineAvailabilityApply(trx, airSeg->carrier(), nextSeg->carrier())))
        {
          if (!(airSeg->flowJourneyCarrier() || airSeg->localJourneyCarrier()))
            continue;
        }
      }

      if (UNLIKELY(airSeg->resStatus() != CONFIRM_RES_STATUS))
        continue;

      flowLen = startFlow(itin, trx, airSeg);
      if (flowLen < 2)
        continue;

      flowLen = foundFlow(itin,
                          airSeg,
                          flowLen,
                          jnyMarkets,
                          airSeg->localJourneyCarrier(),
                          trx,
                          (journeyOrder == nullptr));
      if (flowLen < 2)
        continue;
    }

    if (LIKELY(flowLen > 1))
    {
      tvlIndex += flowLen - 1;
      if (journeyOrder)
      {
        journeyOrder->pop_back();
        for (size_t i = 0; i < flowLen; ++i)
          journeyOrder->push_back(airSeg->localJourneyCarrier()
                                      ? TravelSegJourneyType::LocalJourneySegment
                                      : TravelSegJourneyType::FlowJourneySegment);
      }
    }
  }
}

//-----------------------------------------------------------------------------
// startFlow
//-----------------------------------------------------------------------------
size_t
ItinUtil::startFlow(Itin& itin, PricingTrx& trx, const TravelSeg* startTvlseg)
{
  size_t flowLen = 0;
  size_t nonArunks = 0;
  bool startFound = false;
  AirSeg* startAirSeg = nullptr;
  AirSeg* prevAirSeg = nullptr;
  std::vector<TravelSeg*>::const_iterator tvlI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();
  for (; tvlI != tvlE; tvlI++)
  {
    if (!startFound)
    {
      if ((*tvlI) != startTvlseg)
        continue;
      startFound = true;
      startAirSeg = dynamic_cast<AirSeg*>(*tvlI);
      prevAirSeg = startAirSeg;
      ++nonArunks;
      ++flowLen;
      continue;
    }

    AirSeg* const airSeg = dynamic_cast<AirSeg*>(*tvlI);
    if (airSeg == nullptr)
    {
      if (LIKELY((*tvlI) != nullptr))
      {
        // only multiAirport ARUNKS are considered connections
        if (!(*tvlI)->arunkMultiAirportForAvailability())
          return flowLen;
      }
      ++flowLen;
    }
    else
    {
      if (!TrxUtil::intralineAvailabilityApply(trx, startAirSeg->carrier(), airSeg->carrier()))
      {
        if (airSeg->carrier() != startAirSeg->carrier())
          return flowLen;
        if (UNLIKELY(!(airSeg->flowJourneyCarrier() || airSeg->localJourneyCarrier())))
          return flowLen;
      }

      if (UNLIKELY(airSeg->resStatus() != CONFIRM_RES_STATUS))
        return flowLen;
      if (!journeyConnection(itin, airSeg, prevAirSeg, nonArunks + 1))
        return flowLen;

      // for WP, no journey for local journey carriers if booking code differs
      if (UNLIKELY(!trx.getRequest()->isLowFareRequested() && airSeg->localJourneyCarrier() &&
                   prevAirSeg && prevAirSeg->getBookingCode() != airSeg->getBookingCode()))
      {
        return flowLen;
      }

      ++flowLen;
      ++nonArunks;
    }
    if (airSeg != nullptr)
      prevAirSeg = airSeg;
    // a flow market can have maximum 3 flights
    if (nonArunks == 3)
      break;
  }
  return flowLen;
}

//-----------------------------------------------------------------------------
// journeyMileage
//-----------------------------------------------------------------------------
uint32_t
ItinUtil::journeyMileage(PricingTrx& trx,
                         const AirSeg* startFlt,
                         const AirSeg* currFlt,
                         DateTime travelDate)
{
  if (UNLIKELY(currFlt == nullptr))
    return 0;

  GlobalDirection gd;

  if (startFlt == nullptr)
  {
    const Loc& loc1 = *(currFlt->origin());
    const Loc& loc2 = *(currFlt->destination());

    GlobalDirectionFinderV2Adapter::getGlobalDirection(
        &trx, travelDate, const_cast<AirSeg&>(*currFlt), gd);
    return LocUtil::getTPM(loc1, loc2, gd, currFlt->departureDT(), trx.dataHandle());
  }
  else
  {
    const Loc& loc1 = *(startFlt->origin());
    const Loc& loc2 = *(currFlt->destination());

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(const_cast<AirSeg*>(startFlt));
    tvlSegs.push_back(const_cast<AirSeg*>(currFlt));

    GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, travelDate, tvlSegs, gd);

    return LocUtil::getTPM(loc1, loc2, gd, currFlt->departureDT(), trx.dataHandle());
  }
}

//-----------------------------------------------------------------------------
// journeyMileage
//-----------------------------------------------------------------------------
uint32_t
ItinUtil::journeyMileage(const AirSeg* startFlt,
                         const AirSeg* currFlt,
                         PricingTrx& trx,
                         DateTime travelDate)
{
  if (UNLIKELY(currFlt == nullptr || startFlt == nullptr))
    return 0;

  const Loc* loc1 = startFlt->origin();
  const Loc* loc2 = currFlt->destination();

  return journeyMileage(startFlt, currFlt, loc1, loc2, trx, travelDate);
}

uint32_t
ItinUtil::journeyMileage(const AirSeg* startFlt,
                         const AirSeg* currFlt,
                         const Loc* startLoc,
                         const Loc* endLoc,
                         PricingTrx& trx,
                         DateTime travelDate)
{
  if (UNLIKELY(currFlt == nullptr || startFlt == nullptr || startLoc == nullptr ||
               endLoc == nullptr))
    return 0;

  GlobalDirection gd;

  const Loc& loc1 = *startLoc;
  const Loc& loc2 = *endLoc;

  std::vector<TravelSeg*> tvlSegs;
  tvlSegs.push_back(const_cast<AirSeg*>(startFlt));
  tvlSegs.push_back(const_cast<AirSeg*>(currFlt));

  GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, travelDate, tvlSegs, gd);

  return LocUtil::getTPM(loc1, loc2, gd, currFlt->departureDT(), trx.dataHandle());
}

//-----------------------------------------------------------------------------
// journeyConnection
//-----------------------------------------------------------------------------
bool
ItinUtil::journeyConnection(const Itin& itin,
                            const AirSeg* currFlt,
                            const AirSeg* prevFlt,
                            size_t jnyLength)
{
  if (UNLIKELY(currFlt == nullptr || prevFlt == nullptr))
    return false;

  if ((currFlt->geoTravelType() == GeoTravelType::Domestic ||
       currFlt->geoTravelType() == GeoTravelType::Transborder) &&
      (prevFlt->geoTravelType() == GeoTravelType::Domestic ||
       prevFlt->geoTravelType() == GeoTravelType::Transborder))
  {
    int64_t domCnx = 14400; // domestic connection is 4 hr by default

    // EXCEPTIONS FOR JOURNEY CONNECTION :
    // 1. For NW, journey connection is 24 hours when journey inlude any segment in Hawaii/Alaska.
    // 2. For DL, journey connection is 24 hours when journey include any international segment.

    if ((currFlt->carrier().equalToConst("NW") || currFlt->carrier().equalToConst("DL")) &&
        jnyLength != 0)
    {
      if (connectionException(itin, currFlt, prevFlt, jnyLength))
        domCnx = 86400;
    }
    if (currFlt->isStopOverWithOutForceCnx(prevFlt, domCnx))
      return false;
  }
  else
  {
    // for international 13 hrs for AA and 24 hrs for other
    if (currFlt->carrier() == SPECIAL_CARRIER_AA)
    {
      if (currFlt->isStopOverWithOutForceCnx(prevFlt, 46800))
        return false;
    }
    else
    {
      if (currFlt->isStopOverWithOutForceCnx(prevFlt, 86400))
        return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
// foundFlow
//-----------------------------------------------------------------------------
size_t
ItinUtil::foundFlow(Itin& itin,
                    const TravelSeg* startTvlseg,
                    size_t flowLen,
                    std::vector<FareMarket*>& jnyMarkets,
                    const bool isLocalJourneyCarrier,
                    PricingTrx& trx,
                    bool findFM)
{
  AirSeg* airSeg = nullptr;
  FareMarket* fm = nullptr;
  bool mileageCheckOk = false;
  std::vector<TravelSeg*>::const_iterator tvlI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();
  std::vector<TravelSeg*> jnyTvlSegs;
  std::vector<TravelSeg*> jnyTvlSegs2;

  for (; tvlI != tvlE; tvlI++)
  {
    if ((*tvlI) != startTvlseg)
      continue;

    // point to the last segment of the flow FareMarket
    for (size_t i = 1; (i < flowLen) && (tvlI != tvlE);)
    {
      jnyTvlSegs.push_back(*tvlI);
      ++tvlI;
      ++i;
    }

    if (LIKELY(tvlI != tvlE))
    {
      airSeg = dynamic_cast<AirSeg*>(*tvlI);
      // if last segment of the flow market is not ARUNK
      // then only put it in journey
      if (airSeg != nullptr)
        jnyTvlSegs.push_back(*tvlI);

      mileageCheckOk = checkMileage(trx, jnyTvlSegs, itin.travelDate());
      if (!mileageCheckOk)
      {
        // check if we can make a journey of 1st 2 flights
        if (flowLen == 3 && jnyTvlSegs.size() == 3)
        {
          // make sure the 2nd segment is not ARNK before stepping back
          airSeg = dynamic_cast<AirSeg*>(jnyTvlSegs[1]);

          if (airSeg == nullptr)
            return 0;

          jnyTvlSegs2.resize(0);
          jnyTvlSegs2.push_back(jnyTvlSegs[0]);
          jnyTvlSegs2.push_back(jnyTvlSegs[1]);
          mileageCheckOk = checkMileage(trx, jnyTvlSegs2, itin.travelDate());
          if (mileageCheckOk)
          {
            --flowLen;
            jnyTvlSegs.resize(0);
            jnyTvlSegs.push_back(jnyTvlSegs2[0]);
            jnyTvlSegs.push_back(jnyTvlSegs2[1]);
          }
        }
      }

      if (!mileageCheckOk)
        return 0;

      if (!findFM) // No need to find FM. Just return length of journey.
        return flowLen;

      fm = findMarket(itin, jnyTvlSegs);
      if (fm == nullptr)
      {
        // if we can not find this flow Market then try again with 2 flights
        if (flowLen == 3 && jnyTvlSegs.size() == 3)
        {
          // make sure the 2nd segment is not ARNK before stepping back
          airSeg = dynamic_cast<AirSeg*>(jnyTvlSegs[1]);

          if (airSeg == nullptr)
            return 0;

          jnyTvlSegs2.resize(0);
          --flowLen;
          jnyTvlSegs2.push_back(jnyTvlSegs[0]);
          jnyTvlSegs2.push_back(jnyTvlSegs[1]);
          fm = findMarket(itin, jnyTvlSegs2);
        }
      }
      if (fm == nullptr)
        return 0;

      if (fm->travelSeg().size() > 1)
      {
        jnyMarkets.push_back(fm);
        JourneyUtil::addOAndDMarket(trx, itin, nullptr, fm, isLocalJourneyCarrier);
      }
    }

    break;
  }
  return flowLen;
}
//----------------------------------------------------------------------------
// get arunk segments
//----------------------------------------------------------------------------
std::vector<TravelSeg*>
ItinUtil::getArunks(const Itin* itin)
{
  TSE_ASSERT(itin != nullptr);

  std::vector<TravelSeg*> arunks;
  for (TravelSeg* tSeg : itin->travelSeg())
  {
    if (tSeg && !(tSeg->isAir()))
    {
      arunks.push_back(tSeg);
    }
  }
  return arunks;
}

//----------------------------------------------------------------------------
// findMarket
//----------------------------------------------------------------------------
FareMarket*
ItinUtil::findMarket(Itin& itin, std::vector<TravelSeg*>& tvlSegs)
{
  FareMarket* fmkt = findMarket(tvlSegs, itin.fareMarket());
  if (fmkt == nullptr)
    fmkt = findMarket(tvlSegs, itin.flowFareMarket());

  return fmkt;
}

FareMarket*
ItinUtil::findMarket(const std::vector<TravelSeg*>& tvlSegs, const std::vector<FareMarket*>& fms)
{
  const size_t numTvlSegs = tvlSegs.size();
  if (UNLIKELY(numTvlSegs < 1))
    return nullptr;

  std::vector<FareMarket*>::const_iterator fmIt = fms.begin();
  std::vector<FareMarket*>::const_iterator fmItEnd = fms.end();
  for (; fmIt != fmItEnd; ++fmIt)
  {
    FareMarket* fmkt = (*fmIt);

    if (fmkt->travelSeg().size() != numTvlSegs)
      continue;

    if (fmkt->travelSeg() == tvlSegs)
      return fmkt;
  }

  return nullptr;
}

//----------------------------------------------------------------------------
// checkMileage
//----------------------------------------------------------------------------
bool
ItinUtil::checkMileage(PricingTrx& trx, std::vector<TravelSeg*>& tvlSegs, DateTime travelDate)
{
  if (UNLIKELY(tvlSegs.empty()))
    return false;

  AirSeg* startAirSeg = nullptr;
  AirSeg* airSeg = nullptr;
  AirSeg* lastAirSeg = nullptr;
  uint32_t totalMiles = 0;
  uint32_t origDestMiles = 0;
  std::vector<TravelSeg*>::iterator tvlB = tvlSegs.begin();
  std::vector<TravelSeg*>::iterator tvlI = tvlSegs.begin();
  std::vector<TravelSeg*>::iterator tvlE = tvlSegs.end();
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;

    if (!TrxUtil::journeyMileageApply(airSeg->carrier()))
      return true;

    if (tvlI == tvlB)
      startAirSeg = airSeg;
    totalMiles += journeyMileage(trx, nullptr, airSeg, travelDate);
    lastAirSeg = airSeg;
  }

  if (UNLIKELY(startAirSeg == nullptr || lastAirSeg == nullptr))
    return false;

  origDestMiles = journeyMileage(trx, startAirSeg, lastAirSeg, travelDate);
  if (totalMiles > origDestMiles * 2)
    return false;

  return true;
}

//----------------------------------------------------------------------------
// connectionException
//----------------------------------------------------------------------------
bool
ItinUtil::connectionException(const Itin& itin,
                              const AirSeg* currFlt,
                              const AirSeg* prevFlt,
                              size_t jnyLength)
{
  if (UNLIKELY(currFlt->carrier().equalToConst("NW")))
  {
    if (currFlt->origin()->state() == HAWAII || currFlt->origin()->state() == ALASKA ||
        prevFlt->origin()->state() == HAWAII || prevFlt->origin()->state() == ALASKA ||
        currFlt->destination()->state() == HAWAII || currFlt->destination()->state() == ALASKA)
      return true;
  }

  if (jnyLength == 2)
  {
    const TravelSeg* currTvlSeg = dynamic_cast<const TravelSeg*>(currFlt);
    if (UNLIKELY(currTvlSeg == nullptr))
      return false;

    std::vector<TravelSeg*>::const_iterator tvlB = itin.travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();
    std::vector<TravelSeg*>::const_iterator tvlI = std::find(tvlB, tvlE, currTvlSeg);
    if ((tvlE - tvlI) < 2)
      return false;

    ++tvlI;
    bool foundNextTravelSeg = false;
    for (; tvlI != tvlE; tvlI++)
    {
      if (dynamic_cast<const AirSeg*>(*tvlI) == nullptr)
      {
        if (!(*tvlI)->arunkMultiAirportForAvailability())
          return false;
        continue;
      }
      foundNextTravelSeg = true;
      break;
    }
    if (UNLIKELY(!foundNextTravelSeg))
      return false;

    const TravelSeg* nextTvlSeg = (*tvlI);

    if (UNLIKELY(nextTvlSeg == nullptr))
      return false;

    const AirSeg* nextAirSeg = dynamic_cast<const AirSeg*>(nextTvlSeg);
    if (UNLIKELY(nextAirSeg == nullptr))
      return false;

    if (nextAirSeg->carrier() != currFlt->carrier())
      return false;

    if (UNLIKELY(currFlt->carrier().equalToConst("NW")))
    {
      if (nextTvlSeg->destination()->state() == HAWAII ||
          nextTvlSeg->destination()->state() == ALASKA)
      {
        if (nextTvlSeg->isStopOverWithOutForceCnx(currTvlSeg, 86400))
          return false;
        return true;
      }
    }
    else if (LIKELY(currFlt->carrier().equalToConst("DL")))
    {
      if (!(nextTvlSeg->geoTravelType() == GeoTravelType::Domestic ||
            nextTvlSeg->geoTravelType() == GeoTravelType::Transborder))
      {
        if (nextTvlSeg->isStopOverWithOutForceCnx(currTvlSeg, 86400))
          return false;
        return true;
      }
    }
  }
  else if (LIKELY(jnyLength == 3))
  {
    const TravelSeg* prevTvlSeg = dynamic_cast<const TravelSeg*>(prevFlt);
    if (UNLIKELY(prevTvlSeg == nullptr))
      return false;

    std::vector<TravelSeg*>::const_iterator tvlB = itin.travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();
    std::vector<TravelSeg*>::const_iterator tvlI = std::find(tvlB, tvlE, prevTvlSeg);
    if (UNLIKELY(tvlI == tvlE || tvlI == tvlB))
      return false;

    bool foundPrevPrev = false;
    const size_t toReachBegin = tvlI - tvlB;
    for (size_t i = 0; i < toReachBegin; i++)
    {
      --tvlI;
      if (UNLIKELY(dynamic_cast<const AirSeg*>(*tvlI) == nullptr))
      {
        if (tvlI == tvlB)
          return false;
        if (!(*tvlI)->arunkMultiAirportForAvailability())
          return false;
        continue;
      }
      foundPrevPrev = true;
      break;
    }
    if (UNLIKELY(!foundPrevPrev))
      return false;

    const TravelSeg* prevPrevTvlSeg = (*tvlI);

    if (UNLIKELY(prevPrevTvlSeg == nullptr))
      return false;

    if (UNLIKELY(currFlt->carrier().equalToConst("NW")))
    {
      if (prevPrevTvlSeg->origin()->state() == HAWAII ||
          prevPrevTvlSeg->origin()->state() == ALASKA)
      {
        return true;
      }
    }
    else if (LIKELY(currFlt->carrier().equalToConst("DL")))
    {
      if (!(prevPrevTvlSeg->geoTravelType() == GeoTravelType::Domestic ||
            prevPrevTvlSeg->geoTravelType() == GeoTravelType::Transborder))
        return true;
    }
  }

  return false;
}
//----------------------------------------------------------------------------
// allFlightsBookedInSameCabin
//----------------------------------------------------------------------------
bool
ItinUtil::allFlightsBookedInSameCabin(const Itin& itin)
{
  if (itin.travelSeg().size() == 1)
    return true;

  std::vector<TravelSeg*>::const_iterator tvlB = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();
  std::vector<TravelSeg*>::const_iterator tvlI = tvlB;

  CabinType cabinValue;
  bool cabinValueSet = false;
  for (; tvlI != tvlE; tvlI++)
  {
    if (dynamic_cast<AirSeg*>(*tvlI) == nullptr)
      continue;
    const TravelSeg& tvlSeg = *(*tvlI);
    if (!cabinValueSet)
    {
      cabinValueSet = true;
      cabinValue = tvlSeg.bookedCabin();
      continue;
    }
    else
    {
      if (tvlSeg.bookedCabin() != cabinValue)
        return false;
    }
  }
  return true;
}

bool
ItinUtil::applyMultiCurrencyPricing(PricingTrx* trx, const Itin& itin)
{
  GeoTravelType itinTravelType;
  bool multiCurrencyPricing = false;

  if (UNLIKELY(!trx))
    return false;

  if (trx->isShopping())
  {
    ShoppingTrx* shoppingTrx = static_cast<ShoppingTrx*>(trx);
    itinTravelType = shoppingTrx->journeyItin()->geoTravelType();
  }
  else
    itinTravelType = itin.geoTravelType();

  if (UNLIKELY(!trx->fareCalcConfig()))
    return false;

  Indicator domMCP = trx->fareCalcConfig()->applyDomesticMultiCurrency();
  Indicator intlMCP = trx->fareCalcConfig()->applyIntlMultiCurrency();

  if ((itinTravelType == GeoTravelType::Domestic && (domMCP == YES)) ||
      (itinTravelType == GeoTravelType::International && (intlMCP == YES)) ||
      (itinTravelType == GeoTravelType::ForeignDomestic && (domMCP == YES)))
  {
    multiCurrencyPricing = true;
  }

  return multiCurrencyPricing;
}

bool
ItinUtil::isDomesticOfNation(const Itin* itin, const NationCode nationCode)
{
  if (!itin)
    return false;

  std::vector<TravelSeg*>::const_iterator iter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = itin->travelSeg().end();

  for (; iter != iterEnd; ++iter)
  {
    TravelSeg* travelSeg = *iter;

    if ((travelSeg->origin()->nation() != nationCode) ||
        (travelSeg->destination()->nation() != nationCode))
    {
      return false;
    }
  }

  return true;
}

bool
ItinUtil::isDomesticPeru(PricingTrx* trx, const Itin* itin)
{
  if (UNLIKELY(!trx))
    return false;

  if (UNLIKELY(!itin))
  {
    LOG4CXX_ERROR(logger, "ITINERARY IS NULL");
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID ITINERARY");
  }

  const Loc* salesLoc = TrxUtil::saleLoc(*trx);

  NationCode nationCode = salesLoc->nation();

  if (nationCode != PERU)
    return false;

  return isDomesticOfNation(itin, PERU);
}

double
ItinUtil::calculateEconomyAvailabilityScore(const TravelSeg& tvlSeg,
                                            double point,
                                            int numberOfClassesToAdd,
                                            int requestedNumberOfSeats)
{
  double score = 0.0;
  std::vector<ClassOfService*>::const_reverse_iterator it;

  for (it = tvlSeg.classOfService().rbegin();
       (it != tvlSeg.classOfService().rend()) && (numberOfClassesToAdd > 0);
       it++, --numberOfClassesToAdd)
  {
    if ((*it) == nullptr)
    {
      LOG4CXX_ERROR(logger, "CLASS OF SERVICE  IS NULL");
      continue;
    }
    if (!(*it)->cabin().isEconomyClass())
    {
      continue;
    }
    if ((*it)->numSeats() >= requestedNumberOfSeats)
    {
      score += point * (double)(numberOfClassesToAdd);
    }
  }

  return score;
}

double
ItinUtil::calculateEconomyAvailabilityScore(const Itin* itin,
                                            double point,
                                            int numberOfClassesToAdd,
                                            int requestedNumberOfSeats,
                                            std::map<const Itin*, double>* sopScoreMap)
{
  double score = 0.0;
  std::vector<TravelSeg*>::const_iterator it;

  if (itin == nullptr)
  {
    LOG4CXX_ERROR(logger, "ITINERARY IS NULL");
    return 0;
  }
  if (sopScoreMap)
  {
    std::map<const Itin*, double>::iterator mapIt = sopScoreMap->find(itin);

    if (mapIt != sopScoreMap->end())
    {
      return mapIt->second;
    }
  }

  for (it = itin->travelSeg().begin(); it != itin->travelSeg().end(); ++it)
  {
    if ((*it) == nullptr)
    {
      LOG4CXX_ERROR(logger, "TRAVEL SEGMENT IS NULL");
      continue;
    }
    score += calculateEconomyAvailabilityScore(
        *(*it), point, numberOfClassesToAdd, requestedNumberOfSeats);
  }
  if (sopScoreMap)
  {
    (*sopScoreMap)[itin] = score;
  }
  return score;
}

void
ItinUtil::readConfigDataForSOPScore(double& point, int& numberOfClassesToAdd)
{
  point = pointForSopScore.getValue();
  numberOfClassesToAdd = numberOfClasserForPopScore.getValue();
}

void
ItinUtil::setGeoTravelType(Boundary tvlboundary, FareMarket& fareMarket)
{
  switch (tvlboundary)
  {
  case Boundary::USCA:
  {
    fareMarket.travelBoundary().set(FMTravelBoundary::TravelWithinUSCA, true);
    bool hasUS = false;
    bool hasCA = false;

    std::vector<TravelSeg*>::iterator itr = fareMarket.travelSeg().begin();

    for (; itr != fareMarket.travelSeg().end(); itr++)
    {
      if ((*itr)->origin()->nation() == UNITED_STATES ||
          (*itr)->destination()->nation() == UNITED_STATES)
      {
        hasUS = true;
      }
      if ((*itr)->origin()->nation() == CANADA || (*itr)->destination()->nation() == CANADA)
      {
        hasCA = true;
      }
    } // end for

    if (hasUS && hasCA)
    {
      fareMarket.geoTravelType() = GeoTravelType::Transborder;
      LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to Transborder");
    }
    else
    {
      fareMarket.geoTravelType() = GeoTravelType::Domestic;
      LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to Domestic");
    }
  }
  break;

  case Boundary::EXCEPT_USCA:
    fareMarket.travelBoundary().set(FMTravelBoundary::TravelWithinSameCountryExceptUSCA, true);
    fareMarket.geoTravelType() = GeoTravelType::ForeignDomestic;
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to ForeignDomestic");
    break;

  case Boundary::AREA_21:
    fareMarket.travelBoundary().set(FMTravelBoundary::TravelWithinSubIATA21, true);
    fareMarket.geoTravelType() = GeoTravelType::International;
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to International");
    break;

  case Boundary::AREA_11:
    fareMarket.travelBoundary().set(FMTravelBoundary::TravelWithinSubIATA11, true);
    fareMarket.geoTravelType() = GeoTravelType::International;
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to Transborder");
    break;

  case Boundary::OTHER_SUB_IATA:
    fareMarket.travelBoundary().set(FMTravelBoundary::TravelWithinSameSubIATAExcept21And11, true);
    fareMarket.geoTravelType() = GeoTravelType::International;
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to International");
    break;

  case Boundary::ONE_IATA:
    fareMarket.travelBoundary().set(FMTravelBoundary::TravelWithinOneIATA, true);
    fareMarket.geoTravelType() = GeoTravelType::International;
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to International");
    break;

  case Boundary::TWO_IATA:
    fareMarket.travelBoundary().set(FMTravelBoundary::TravelWithinTwoIATA, true);

    if (LocUtil::isRussia(*(fareMarket.origin())) && LocUtil::isRussia(*(fareMarket.destination())))
      fareMarket.geoTravelType() = GeoTravelType::ForeignDomestic;
    else
      fareMarket.geoTravelType() = GeoTravelType::International;

    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to International");
    break;

  case Boundary::ALL_IATA:
    fareMarket.travelBoundary().set(FMTravelBoundary::TravelWithinAllIATA, true);
    fareMarket.geoTravelType() = GeoTravelType::International;
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to International");
    break;

  default:
    LOG4CXX_ERROR(logger, "ItinUtil::selectTravelBoundary() Failed");
    fareMarket.geoTravelType() = GeoTravelType::UnknownGeoTravelType;

  } // end switch
} // end of setGeoTravelType()

void
ItinUtil::setGeoTravelType(TravelSegAnalysis& tvlSegAnalysis, Boundary tvlboundary, Itin& itn)
{
  LOG4CXX_DEBUG(logger, "Entered ItinUtil::setGeoTravelType for Itin");

  switch (tvlboundary)
  {
  case Boundary::USCA:
  {
    bool hasUS = false;
    bool hasCA = false;
    std::vector<TravelSeg*>::iterator itr = itn.travelSeg().begin();

    for (; itr != itn.travelSeg().end(); itr++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);
      if (airSeg != nullptr)
      {
        if (airSeg->origin()->nation() == UNITED_STATES ||
            airSeg->destination()->nation() == UNITED_STATES)
        {
          hasUS = true;
        }
        if (airSeg->origin()->nation() == CANADA || airSeg->destination()->nation() == CANADA)
        {
          hasCA = true;
        }
      }
    } // end for

    if (hasUS && hasCA)
    {
      itn.geoTravelType() = GeoTravelType::Transborder;
      itn.ticketingCarrier() = tvlSegAnalysis.getFirstIntlFlt(itn.travelSeg());
      LOG4CXX_INFO(logger, "ticketingCarrier= " << itn.ticketingCarrier());
      LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to Transborder");
    }
    else
    {
      itn.geoTravelType() = GeoTravelType::Domestic;
      for (itr = itn.travelSeg().begin(); itr != itn.travelSeg().end(); itr++)
      {
        const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);
        if (LIKELY(airSeg != nullptr))
        {
          itn.ticketingCarrier() = airSeg->carrier();
          break;
        }
      }
      LOG4CXX_INFO(logger, "tickeingCarrier= " << itn.ticketingCarrier());
      LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to Domestic");
    }
  }
  break;

  case Boundary::EXCEPT_USCA:
  {
    itn.geoTravelType() = GeoTravelType::ForeignDomestic;
    std::vector<TravelSeg*>::iterator itr = itn.travelSeg().begin();

    for (itr = itn.travelSeg().begin(); itr != itn.travelSeg().end(); itr++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);
      if (LIKELY(airSeg != nullptr))
      {
        itn.ticketingCarrier() = airSeg->carrier();
        LOG4CXX_INFO(logger, "ticketingCarrier= " << itn.ticketingCarrier());
        break;
      }
    }
  }
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to ForeignDomestic");
    break;

  case Boundary::AREA_21:

    itn.geoTravelType() = GeoTravelType::International;
    itn.ticketingCarrier() = tvlSegAnalysis.getFirstIntlFlt(itn.travelSeg());
    itn.tripCharacteristics().set(Itin::EuropeOnly, true);
    LOG4CXX_INFO(logger, "ticketingCarrier= " << itn.ticketingCarrier());
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to International");
    break;

  case Boundary::AREA_11:

    itn.geoTravelType() = GeoTravelType::International;
    itn.ticketingCarrier() = tvlSegAnalysis.getFirstIntlFlt(itn.travelSeg());
    LOG4CXX_INFO(logger, "ticketingCarrier= " << itn.ticketingCarrier());

    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to International");
    break;

  case Boundary::OTHER_SUB_IATA:

    itn.geoTravelType() = GeoTravelType::International;
    itn.ticketingCarrier() = tvlSegAnalysis.getFirstIntlFlt(itn.travelSeg());
    LOG4CXX_INFO(logger, "ticketingCarrier= " << itn.ticketingCarrier());
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to International");
    break;
  case Boundary::ONE_IATA:
    itn.geoTravelType() = GeoTravelType::International;
    itn.ticketingCarrier() = tvlSegAnalysis.getFirstIntlFlt(itn.travelSeg());
    LOG4CXX_INFO(logger, "ticketingCarrier= " << itn.ticketingCarrier());
    LOG4CXX_DEBUG(logger, "Successfully set TravelBoundary and GeoTravelType to International");
    break;

  case Boundary::TWO_IATA:
  {
    itn.geoTravelType() = GeoTravelType::ForeignDomestic;

    std::vector<TravelSeg*>::iterator itr = itn.travelSeg().begin();

    for (; itr != itn.travelSeg().end(); itr++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);

      if (LIKELY(airSeg != nullptr))
      {
        if ((!LocUtil::isRussia(*(airSeg->origin()))) ||
            (!LocUtil::isRussia(*(airSeg->destination()))))
        {
          itn.geoTravelType() = GeoTravelType::International;
          break;
        }
      }
    }

    itr = itn.travelSeg().begin();

    for (; itr != itn.travelSeg().end(); itr++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);

      if (airSeg != nullptr && (airSeg->origin()->area() != airSeg->destination()->area()))
      {
        itn.ticketingCarrier() = airSeg->carrier();
        LOG4CXX_INFO(logger, "ticketingCarrier= " << itn.ticketingCarrier());
        break;
      }
    }
  }

    LOG4CXX_DEBUG(
        logger, "Setting Itinerary GEO Travel Type to: " << static_cast<int>(itn.geoTravelType()));
    break;

  case Boundary::ALL_IATA:
  {
    LOG4CXX_DEBUG(logger, "Entered ItinUtil::selectGovCxrWithinAllIATA()");
    if (LIKELY(
            tvlSegAnalysis.getCarrierInIATA1(itn, itn.travelSeg().begin(), itn.travelSeg().end())))
    {
      break;
    }
    else
    {
      LOG4CXX_ERROR(logger, "Can Not Select Ticketing Carrier");
      break;
    }
  }

  default:
    LOG4CXX_ERROR(logger, "ItinUtil::selectGeoTravelType() Failed");
    itn.geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    itn.ticketingCarrier() = "XX";
    LOG4CXX_INFO(logger, "ticketingCarrier= " << itn.ticketingCarrier());
  } // end switch
} // end of selectGeoTravelType() for Itin

void
ItinUtil::swapValidatingCarrier(PricingTrx& trx, Itin& itin)
{
  swapValidatingCarrier(trx, itin.validatingCarrier());
}

void
ItinUtil::swapValidatingCarrier(PricingTrx& trx, CarrierCode& validatingCarrier)
{
  // TODO
  // Temporary HARD CODE for missing SWAP table that exists in Ticketing
  // Pricing needs to match ticketing which does SWAPs(KL/NW)(AO/QF)

  if (UNLIKELY(validatingCarrier.equalToConst("AO")))
  {
    validatingCarrier = "QF";
    return;
  }

  if (UNLIKELY(validatingCarrier.equalToConst("AP")))
  {
    const Agent* agent = trx.getRequest()->ticketingAgent();
    if (agent && !agent->tvlAgencyPCC().empty() && agent->sabre1SUser())
    {
      validatingCarrier = "AZ";
    }
    return;
  }

  if ((validatingCarrier.equalToConst("KL")) || (validatingCarrier.equalToConst("NW")))
  {
    const Loc* pointOfSaleLocation = TrxUtil::saleLoc(trx);

    if (!pointOfSaleLocation)
      return;

    if (LocUtil::isUS(*pointOfSaleLocation) || LocUtil::isCanada(*pointOfSaleLocation) ||
        (pointOfSaleLocation->nation().equalToConst("MX")) ||
        LocUtil::isUSTerritoryOnly(*pointOfSaleLocation))
    {
      if (!validatingCarrier.equalToConst("NW"))
        validatingCarrier = CARRIER_DL;
      else
        validatingCarrier = "NW";
      return;
    }

    if (pointOfSaleLocation->nation().equalToConst("BM") ||
        pointOfSaleLocation->nation().equalToConst("JM"))
    {
      if (!validatingCarrier.equalToConst("NW"))
        validatingCarrier = CARRIER_DL;
      else
        validatingCarrier = "NW";
      return;
    }

    if (validatingCarrier.equalToConst("KL"))
      return;

    std::string::size_type nationLocation =
        nationListSwapValidCXR.find(pointOfSaleLocation->nation());

    if (nationLocation == std::string::npos)
      validatingCarrier = "KL";
  }
}

void
ItinUtil::setItinCurrencies(Itin& itin, const DateTime& ticketingDate)
{
  CurrencyCode currencyCode = NUC;

  if (UNLIKELY(!ItinUtil::getOriginationCurrency(itin, currencyCode, ticketingDate)))
  {
    LOG4CXX_FATAL(logger, "getOriginationCurrency failed!");
    throw ErrorResponseException(ErrorResponseException::CANNOT_CALCULATE_CURRENCY,
                                 "FAILED TO GET ORIGINATION CURRENCY");
  }

  itin.originationCurrency() = currencyCode;

  if (LIKELY(itin.calculationCurrency().empty()))
  {
    if (UNLIKELY(!itin.calcCurrencyOverride().empty()))
      itin.calculationCurrency() = itin.calcCurrencyOverride();
    else
      itin.calculationCurrency() = NUC;
  }

  LOG4CXX_DEBUG(logger, "originationCurrency: " << itin.originationCurrency());
  LOG4CXX_DEBUG(logger, "calculationCurrency: " << itin.calculationCurrency());
}

bool
ItinUtil::isStopover(const TravelSeg& earlier,
                     const TravelSeg& later,
                     const GeoTravelType geoTravelType,
                     const TimeAndUnit& minTime,
                     const FareMarket* fareMarket)
{
  bool isEarlier = (earlier.departureDT() < later.departureDT());
  const TravelSeg& from = (isEarlier ? earlier : later);
  const TravelSeg& to = (isEarlier ? later : earlier);

  bool isStopOver = false;

  // Special handling to OPEN travel segment.
  if (UNLIKELY((from.segmentType() == tse::Open) || (to.segmentType() == tse::Open)))
  {
    if (from.isOpenWithoutDate() && to.isOpenWithoutDate())
    {
      isStopOver = true;
    }
    else if (from.isOpenWithoutDate() && !to.isOpenWithoutDate())
    {
      isStopOver = true;
    }
    else if (!from.isOpenWithoutDate() && to.isOpenWithoutDate())
    {
      isStopOver = true;
    }
    else if (from.arrivalDT().isValid() && to.departureDT().isValid())
    {
      if ((from.arrivalDT().year() != to.departureDT().year()) ||
          (from.arrivalDT().month() != to.departureDT().month()) ||
          (from.arrivalDT().day() != to.departureDT().day()))
      {
        isStopOver = true;
      }
    }
  }
  else
  {
    int64_t stopTimeSeconds = DateTime::diffTime(to.departureDT(), from.arrivalDT());

    // Special handling to SURFACE and ARUNK travel segment.
    if (to.segmentType() == Surface || to.segmentType() == Arunk)
    {
      stopTimeSeconds = DateTime::diffTime(to.arrivalDT(), to.departureDT());
    }
    else if (from.segmentType() == Surface || from.segmentType() == Arunk)
    {
      stopTimeSeconds = DateTime::diffTime(from.arrivalDT(), from.departureDT());
    }

    if ((minTime.unit() == RuleConst::STOPOVER_TIME_UNIT_BLANK) ||
        ((minTime.value() == 0) && (minTime.unit() != RuleConst::STOPOVER_TIME_UNIT_DAYS)))
    {
      if ((geoTravelType == GeoTravelType::International) ||
          (geoTravelType == GeoTravelType::ForeignDomestic))
      {
        if (stopTimeSeconds > RuleConst::STOPOVER_SEC_INTL)
        {
          isStopOver = true;
        }
      }
      else
      {
        if (stopTimeSeconds > RuleConst::STOPOVER_SEC_DOMESTIC)
        {
          isStopOver = true;
        }
      }
    }
    else if (minTime.unit() == RuleConst::STOPOVER_TIME_UNIT_MINUTES)
    {
      if (stopTimeSeconds > (minTime.value() * SECONDS_PER_MINUTE))
      {
        isStopOver = true;
      }
    }
    else if (minTime.unit() == RuleConst::STOPOVER_TIME_UNIT_HOURS)
    {
      if (stopTimeSeconds > (minTime.value() * SECONDS_PER_HOUR))
      {
        isStopOver = true;
      }
    }
    else if (minTime.unit() == RuleConst::STOPOVER_TIME_UNIT_DAYS)
    {
      if (!ItinUtil::isStayTimeWithinDays(from.arrivalDT(), to.departureDT(), minTime.value()))
      {
        isStopOver = true;
      }
    }
    else if (minTime.unit() == RuleConst::STOPOVER_TIME_UNIT_MONTHS)
    {
      DateTime arrival = from.arrivalDT();
      if (to.departureDT() >= arrival.addMonths(minTime.value()))
      {
        isStopOver = true;
      }
    }
    else
    {
      if (fareMarket &&
          std::find(fareMarket->stopOverTravelSeg().begin(),
                    fareMarket->stopOverTravelSeg().end(),
                    &from) != fareMarket->stopOverTravelSeg().end())
      {
        isStopOver = true;
      }
    }
  }

  return isStopOver;
}

bool
ItinUtil::isStayTimeWithinDays(const DateTime& arriveDT,
                               const DateTime& departDT,
                               const int16_t& stayDays)
{
  if (stayDays == 0) // Special: Same day
  {
    return (departDT.year() == arriveDT.year() && departDT.month() == arriveDT.month() &&
            departDT.day() == arriveDT.day());
  }
  else
  {
    DateTime arrive(arriveDT.year(), arriveDT.month(), arriveDT.day(), 23, 59, 59);

    DateTime depart(departDT.year(), departDT.month(), departDT.day());

    return (depart < arrive.addDays(stayDays));
  }
}

int
ItinUtil::countContinents(const PricingTrx& trx,
                          const FareMarket& fm,
                          const AirlineAllianceCarrierInfo& carrierInfo)
{
  std::set<Continent> continents;
  std::map<NationCode, Continent> nationMap;

  auto insertNationFromContinentInfo = [&](AirlineAllianceContinentInfo* continentInfo)
  {
    Zone zoneId = continentInfo->locCode();
    LocUtil::padZoneNo(zoneId);

    const ZoneInfo* zone =
        trx.dataHandle().getZone(Vendor::SABRE, zoneId, MANUAL, trx.getRequest()->ticketingDT());
    if (zone)
    {
      for (const std::vector<ZoneInfo::ZoneSeg>& zoneSet : zone->sets())
      {
        for (const ZoneInfo::ZoneSeg& seg : zoneSet)
        {
          if (seg.locType() == LOCTYPE_NATION)
            nationMap.insert(std::make_pair(seg.loc(), continentInfo->continent()));
        }
      }
    }
  };

  if (!fallback::reduceTemporaryVectors(&trx))
  {
    trx.dataHandle().forEachAirlineAllianceContinent(carrierInfo.genericAllianceCode(),
                                                     insertNationFromContinentInfo);
  }
  else
  {
    const std::vector<AirlineAllianceContinentInfo*>& airlineAllianceContinents =
        trx.dataHandle().getAirlineAllianceContinent(carrierInfo.genericAllianceCode(), true);

    for (AirlineAllianceContinentInfo* continentInfo : airlineAllianceContinents)
    {
      Zone zoneId = continentInfo->locCode();
      LocUtil::padZoneNo(zoneId);

      const ZoneInfo* zone =
          trx.dataHandle().getZone(Vendor::SABRE, zoneId, MANUAL, trx.getRequest()->ticketingDT());
      if (zone)
      {
        for (const std::vector<ZoneInfo::ZoneSeg>& zoneSet : zone->sets())
        {
          for (const ZoneInfo::ZoneSeg& seg : zoneSet)
          {
            if (seg.locType() == LOCTYPE_NATION)
              nationMap.insert(std::make_pair(seg.loc(), continentInfo->continent()));
          }
        }
      }
    }
  }

  std::vector<NationCode> nations;
  nations.reserve(fm.travelSeg().size() + 1);
  for (TravelSeg* travelSeg : fm.travelSeg())
  {
    nations.push_back(travelSeg->origin()->nation());
  }
  nations.push_back(fm.travelSeg().back()->destination()->nation());

  Continent prev = UNKNOWN_CONTINENT;

  for (NationCode& nation : nations)
  {
    std::map<NationCode, Continent>::const_iterator found = nationMap.find(nation);
    if (found != nationMap.end())
    {
      Continent current = found->second;
      continents.insert(current);

      if ((prev == CONTINENT_SOUTH_WEST_PACIFIC && current == CONTINENT_EUROPE_MIDDLE_EAST) ||
          (prev == CONTINENT_EUROPE_MIDDLE_EAST && current == CONTINENT_SOUTH_WEST_PACIFIC))
      {
        continents.insert(CONTINENT_ASIA);
      }
      prev = current;
    }
    else
      prev = UNKNOWN_CONTINENT;
  }

  return int(continents.size());
}

std::vector<int32_t>
ItinUtil::generateNextFareMarketList(Itin& itin)
{
  // Generate a list of next fare markets.
  // For each fare market itin.fareMarket()[i], next[i] will be the next fare market
  // index in the itinerary if it exists, -1 if it does not.
  // The end of side trip does NOT have the next fare market.
  const uint32_t fareMarkets = itin.fareMarket().size();

  std::vector<int32_t> next(fareMarkets, -1);
  FlatMap<TravelSeg*, int32_t> beginSegToFareMarket;
  beginSegToFareMarket.reserve(fareMarkets);

  for (size_t i = 0; i < fareMarkets; ++i)
  {
    FareMarket* fareMarket = itin.fareMarket()[i];
    if (UNLIKELY(fareMarket->travelSeg().empty()))
      continue;

    beginSegToFareMarket.unsafe_emplace(fareMarket->travelSeg().front(), i);
  }

  beginSegToFareMarket.order();

  for (size_t i = 0; i < fareMarkets; ++i)
  {
    FareMarket* fareMarket = itin.fareMarket()[i];
    if (UNLIKELY(fareMarket->travelSeg().empty()))
      continue;

    const int16_t order = itin.segmentOrder(fareMarket->travelSeg().back());
    if (order <= 0 || order >= int(itin.travelSeg().size()))
      continue;

    // Since order is the segment index + 1, the next instruction gets the segment after that.
    TravelSeg* const nextSegment = itin.travelSeg()[order];
    const auto it = beginSegToFareMarket.find(nextSegment);

    if (it != beginSegToFareMarket.end())
      next[i] = it->second;
  }

  return next;
}

void
ItinUtil::collectMarkupsForFarePaths(Itin& itin)
{
  for (FarePath* farePath : itin.farePath())
  {
    MoneyAmount markup = 0;
    for (PricingUnit* pricingUnit : farePath->pricingUnit())
    {
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        markup += fareUsage->paxTypeFare()->fare()->nucMarkupAmount();
      }
    }

    if (farePath->getTotalNUCMarkupAmount() == 0)
    {
      farePath->setTotalNUCMarkupAmount(markup);
    }
  }
}
} // tse
