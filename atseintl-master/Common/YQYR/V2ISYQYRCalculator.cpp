// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Common/YQYR/V2ISYQYRCalculator.h"

#include "Common/Assert.h"
#include "Common/FareMarketUtil.h"
#include "Common/YQYR/YQYRUtils.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/ShoppingSurcharges.h"
#include "Util/BranchPrediction.h"

#include <boost/iterator/filter_iterator.hpp>

namespace tse
{
namespace YQYR
{
void
V2ISYQYRCalculator::init()
{
  _international = YQYRUtils::isInternational(_trx.journeyItin()->travelSeg());
  _returnsToOrigin = YQYRUtils::doesReturnToOrigin(_trx.journeyItin()->travelSeg(), _international);

  determineOriginAndFurthestPoint(_journeyOrigin, _furthestPoint, _firstInboundLeg);

  if (UNLIKELY(_dc.get()))
    printDiagHeader(_furthestPoint);

  createBuckets(_furthestPoint, _originalBuckets);

  initDefaultFilters();
  appendFilter(&_trx.dataHandle().safe_create<YQYRFilterJourneyLocation>(
      _trx, _journeyOrigin, _furthestPoint));
  prependFilter(&_trx.dataHandle().safe_create<YQYRFilterReturnsToOrigin>(_trx, _returnsToOrigin));
  setClassifier(&_trx.dataHandle().safe_create<YQYRClassifierPassengerType>(_trx));
}

void
V2ISYQYRCalculator::determineOriginAndFurthestPoint(const Loc*& origin,
                                                    const Loc*& furthestPoint,
                                                    uint32_t& firstInboundLeg)
{
  const Itin* const journeyItin = _trx.journeyItin();
  const DateTime travelDate = journeyItin->travelDate();
  origin = journeyItin->travelSeg().front()->origin();

  const auto begin = journeyItin->travelSeg().begin();
  const auto end = journeyItin->travelSeg().end();

  if (!_returnsToOrigin)
  {
    furthestPoint = (*(end - 1))->destination();
    firstInboundLeg = _trx.legs().size();
    return;
  }

  std::vector<TravelSeg*> gdSegs;
  std::vector<GlobalDirection> gds;
  gdSegs.reserve(end - begin);
  gds.reserve(end - begin);

  const NationCode& origNation = origin->nation();

  struct Segment
  {
    const Loc* loc;
    int leg;
  };
  std::map<uint32_t, Segment> segsByDistance;

  for (auto curSegIt = begin; curSegIt < end; ++curSegIt)
  {
    TravelSeg* curSeg = *curSegIt;
    auto nextSegIt = curSegIt + 1;

    const bool hasArunk =
        (nextSegIt != end) && ((*nextSegIt)->boardMultiCity() != (*curSegIt)->offMultiCity());

    Segment segment;
    segment.leg = curSegIt - begin;

    {
      gdSegs.push_back(curSeg);

      GlobalDirection gd;
      GlobalDirectionFinderV2Adapter::getGlobalDirection(&_trx, travelDate, gdSegs, gd);

      gds.push_back(gd);

      segment.loc = (*curSegIt)->destination();

      segsByDistance.insert(std::make_pair(
          YQYRUtils::journeyMileage(origin, segment.loc, gds, travelDate, _trx), segment));
    }

    if (hasArunk)
    {
      ArunkSeg* arunk = _trx.dataHandle().create<ArunkSeg>();
      arunk->origin() = (*curSegIt)->destination();
      arunk->destination() = (*nextSegIt)->origin();

      gdSegs.push_back(arunk);

      GlobalDirection gd;
      GlobalDirectionFinderV2Adapter::getGlobalDirection(&_trx, travelDate, gdSegs, gd);

      gds.push_back(gd);

      segment.loc = (*nextSegIt)->origin();

      segsByDistance.insert(std::make_pair(
          YQYRUtils::journeyMileage(origin, segment.loc, gds, travelDate, _trx), segment));
    }
  }

  auto furthestIt = segsByDistance.rbegin();

  if (_international)
    while (furthestIt != segsByDistance.rend() && furthestIt->second.loc->nation() == origNation)
      ++furthestIt;

  if (furthestIt == segsByDistance.rend())
  {
    // In order not to fail now, use journey destination as a furthest loc.
    // It will be the ROI, but with only the outbound part.

    // Doesn't make much sense but is what YQYRCalculator does.
    // Since it's not documented, we can do anything we want.

    furthestPoint = (*(end - 1))->destination();
    firstInboundLeg = _trx.legs().size();
  }
  else
  {
    furthestPoint = furthestIt->second.loc;
    firstInboundLeg = furthestIt->second.leg + 1;
  }
}

void
V2ISYQYRCalculator::createBuckets(const Loc* furthestPoint, std::vector<YQYRBucket>& buckets)
{
  buckets.reserve(_trx.paxType().size());

  for (PaxType* paxType : _trx.paxType())
    buckets.push_back(YQYRBucket(furthestPoint, paxType));
}

void
V2ISYQYRCalculator::printDiagHeader(const Loc* furthestPoint)
{
  DiagStream stream(_dc->diagStream());

  stream << " V2IS YQYR CALCULATOR" << std::endl;
  stream << " - JOURNEY ORIGIN: " << _journeyOrigin->loc() << std::endl;
  stream << " - FURTHEST POINT: " << furthestPoint->loc() << std::endl;
  stream << std::endl;
}

V2ISCortegeCalculator::V2ISCortegeCalculator(V2ISYQYRCalculator& calculator,
                                             ShoppingSurchargesCortegeContext& ctx)
  : _calculator(calculator),
    _fareMarket(ctx.fareMarket),
    _cortege(ctx.cortege),
    _legsBegin(ctx.legsBegin),
    _legsEnd(ctx.legsEnd),
    _aso(ctx.aso)
{
  _geography.furthestLoc = _calculator._furthestPoint;
  _geography.returnsToOrigin = _calculator._returnsToOrigin;
  _geography.direction = determineDirection();
  _geography.init(_fareMarket.travelSeg(), false);

  _bucket = determineBucket();

  determineFurthestSegSop(_furthestSegSop, _furthestSegSopDestination);
}

void
V2ISCortegeCalculator::initializeCortegeCarrierStorage(
    const StdVectorFlatSet<CarrierCode>& carriersToProcess,
    const std::vector<PaxTypeFare*>& applicableFares)
{
  if (applicableFares.empty() || carriersToProcess.empty())
    return;

  std::vector<CarrierCode> carriersToInitialize;
  for (CarrierCode carrier : carriersToProcess)
    if (!_cortege.getYqyrCarrierStorage(carrier).isInitialized())
      carriersToInitialize.push_back(carrier);

  if (carriersToInitialize.empty())
    return;

  if (UNLIKELY(dc()))
  {
    DiagStream stream(dc()->diagStream());
    stream << "PREPARING FEES STORAGE FOR FARE MARKET " << _fareMarket.toString();
    stream << " ON PAX " << _cortege.requestedPaxType()->paxType();
    stream << " ON CARRIERS: [";
    for (const CarrierCode carrier : carriersToInitialize)
      stream << carrier.c_str() << ", ";
    stream << "]" << std::endl;
  }

  YQYRFilterFareBasisCode fareBasisCodeFilter(trx(), applicableFares);

  YQYRFilters filters;
  filters.append(&fareBasisCodeFilter);

  for (CarrierCode carrier : carriersToInitialize)
  {
    CarrierStorage& cortegeCarrierStorage = _cortege.getYqyrCarrierStorage(carrier);

    const CarrierStorage* storage = _calculator.getCarrierStorage(carrier);
    if (!storage)
      continue;

    for (const YQYR::FeeStorage& feeStorage : storage->_feesPerCode)
    {
      cortegeCarrierStorage._feesPerCode.push_back(FeeStorage(_calculator._originalBuckets));
      cortegeCarrierStorage._feesPerCode.back().copyFees(feeStorage, filters, _bucket, dc());
    }
  }
}

void
V2ISCortegeCalculator::determineFurthestSeg(const ItinIndex::ItinIndexIterator& cellIterator)
{
  if (_furthestSegSop < 0)
    return;

  const std::vector<TravelSeg*>* segments = nullptr;

  if (!_aso)
  {
    TSE_ASSERT(_furthestSegSop == 0);
    segments = &cellIterator->second->travelSeg();
  }
  else
  {
    TSE_ASSERT(_legsBegin[_furthestSegSop] != ASOLEG_SURFACE_SECTOR_ID &&
               cellIterator.currentSopSet()[_furthestSegSop] != int(ASOLEG_SURFACE_SECTOR_ID));
    ShoppingTrx::Leg& leg = trx().legs()[_legsBegin[_furthestSegSop]];
    ShoppingTrx::SchedulingOption& sop = leg.sop()[cellIterator.currentSopSet()[_furthestSegSop]];
    segments = &sop.itin()->travelSeg();
  }

  if (_furthestSegSopDestination)
    _geography.furthestSeg = segments->back();
  else
    _geography.furthestSeg = segments->front();
}

bool
V2ISCortegeCalculator::calculateYQYRs(const CarrierCode validatingCarrier,
                                      const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                                      const PaxTypeFare& fare,
                                      FarePath& farePath,
                                      YQYRCalculator::YQYRFeesApplicationVec& results)
{
  if (UNLIKELY(dc()))
  {
    dc()->diagStream() << " - PROCESSING FM " << FareMarketUtil::getDisplayString(_fareMarket)
                       << " ON PAX " << _cortege.requestedPaxType()->paxType()
                       << ", VALIDATING CARRIER: " << validatingCarrier << " AND FARE "
                       << fare.createFareBasis(&trx()) << std::endl;
  }

  TSE_ASSERT(_geography.furthestSeg || _geography.direction != YQYRCalculator::Directions::BOTH);

  return _calculator.calculateYQYRsOnBucket(_bucket,
                                            validatingCarrier,
                                            carriersToProcess,
                                            &_fareMarket,
                                            _cortege,
                                            &farePath,
                                            _geography,
                                            results);
}

YQYRCalculator::Directions
V2ISCortegeCalculator::determineDirection() const
{
  // The begin and end should NOT be arunk.
  TSE_ASSERT(*_legsBegin != ASOLEG_SURFACE_SECTOR_ID &&
             *(_legsEnd - 1) != ASOLEG_SURFACE_SECTOR_ID);

  uint8_t direction = uint8_t(YQYRCalculator::Directions::NONE);

  if (*_legsBegin < _calculator._firstInboundLeg)
    direction |= uint8_t(YQYRCalculator::Directions::OUTBOUND);

  if (*(_legsEnd - 1) >= _calculator._firstInboundLeg)
    direction |= uint8_t(YQYRCalculator::Directions::INBOUND);

  return YQYRCalculator::Directions(direction);
}

uint32_t
V2ISCortegeCalculator::determineBucket() const
{
  const auto it =
      std::find(trx().paxType().begin(), trx().paxType().end(), _cortege.requestedPaxType());
  TSE_ASSERT(it != trx().paxType().end());

  return it - trx().paxType().begin();
}

namespace
{
struct IsNotSurface
{
  bool operator()(uint32_t n) const { return n != ASOLEG_SURFACE_SECTOR_ID; }
};

using LegIterator = boost::filter_iterator<IsNotSurface, const uint32_t*>;
}

void
V2ISCortegeCalculator::determineFurthestSegSop(int& sop, bool& destination) const
{
  if (*(_legsEnd - 1) < _calculator._firstInboundLeg - 1)
    return;

  if (*_legsBegin > _calculator._firstInboundLeg)
    return;

  if (*_legsBegin == _calculator._firstInboundLeg)
  {
    sop = 0;
    destination = false;
    return;
  }

  const LegIterator begin(_legsBegin, _legsEnd), end(_legsEnd, _legsEnd);
  const auto lastOutbound = std::find(begin, end, _calculator._firstInboundLeg - 1);

  TSE_ASSERT(lastOutbound != end);

  sop = lastOutbound.base() - _legsBegin;
  destination = true;
}
}
}
