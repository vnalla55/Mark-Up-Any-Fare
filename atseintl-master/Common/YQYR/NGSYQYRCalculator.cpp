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

#include "Common/YQYR/NGSYQYRCalculator.h"

#include "Common/YQYR/NGSYQYRCalculator.t.h"
#include "Common/YQYR/YQYRUtils.h"
#include "DataModel/ShoppingTrx.h"
#include "Util/BranchPrediction.h"

namespace tse
{
namespace YQYR
{
void
NGSYQYRCalculator::init()
{
  FurthestPointToSOPsMap furthestPointsToSops;
  _journeyOrigin = determineFurthestPoints(furthestPointsToSops);
  _globalReturnsToOrigin = YQYRUtils::doesReturnToOrigin(_trx.journeyItin()->travelSeg());

  if (UNLIKELY(_dc.get()))
    printDiagHeader(furthestPointsToSops);

  createBuckets(furthestPointsToSops, _originalBuckets);

  initDefaultFilters();
  setClassifier(&_trx.dataHandle().safe_create<YQYRClassifierFurthestPoint>(_trx, _journeyOrigin));
}

void
NGSYQYRCalculator::initializeFareMarketCarrierStorage(
    FareMarket* fareMarket,
    const StdVectorFlatSet<CarrierCode>& carriersToProcess,
    const std::vector<PaxTypeFare*>& applicableFares)
{
  PaxTypeBucket& cortege = fareMarket->paxTypeCortege().front();

  if (applicableFares.empty() || carriersToProcess.empty())
    return;

  std::vector<CarrierCode> carriersToInitialize;
  for (CarrierCode carrier : carriersToProcess)
    if (!cortege.getYqyrCarrierStorage(carrier).isInitialized())
      carriersToInitialize.push_back(carrier);

  if (carriersToInitialize.empty())
    return;

  if (UNLIKELY(_dc.get()))
  {
    DiagStream stream(_dc->diagStream());
    stream << "PREPARING FEES STORAGE FOR FARE MARKET " << fareMarket->toString();
    stream << " ON CARRIERS: [";
    for (const CarrierCode carrier : carriersToInitialize)
      stream << carrier.c_str() << ", ";
    stream << "]" << std::endl;
  }

  YQYRFilterFareCachingAdapter<YQYRFilterFareBasisCode> fareBasisCodeFilter(_trx, applicableFares);
  YQYRFilterFareCachingAdapter<YQYRFilterPassengerType> passengerTypeFilter(_trx, applicableFares);

  YQYRFilters filters;
  filters.append(&fareBasisCodeFilter);
  filters.append(&passengerTypeFilter);

  for (CarrierCode carrier : carriersToInitialize)
  {
    CarrierStorage& fmCarrierStorage = cortege.getYqyrCarrierStorage(carrier);

    const CarrierStorage* storage = getCarrierStorage(carrier);
    if (!storage)
      continue;

    for (const YQYR::FeeStorage& feeStorage : storage->_feesPerCode)
    {
      fmCarrierStorage._feesPerCode.push_back(FeeStorage(_originalBuckets));
      fmCarrierStorage._feesPerCode.back().copyFees(feeStorage, filters, _dc.get());
    }
  }
}

bool
NGSYQYRCalculator::calculateYQYRs(const CarrierCode validatingCarrier,
                                  const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                                  const PaxTypeFare* fare,
                                  FareMarket* fareMarket,
                                  FarePath* farePath,
                                  const uint32_t sopIndex,
                                  YQYRCalculator::YQYRFeesApplicationVec& results)
{
  if (UNLIKELY(_dc.get()))
  {
    _dc->diagStream() << " - PROCESSING SOP " << sopIndex << " [" << fareMarket->legIndex() << "]"
                      << ", VALIDATING CARRIER: " << validatingCarrier << " AND FARE "
                      << fare->createFareBasis(&_trx) << std::endl;
  }

  const SopAndLegPair key(std::make_pair(sopIndex, fareMarket->legIndex()));
  SopToBucketIndexMap::const_iterator bucketIndexIt(_sopToBucketIndex.find(key));
  if (bucketIndexIt == _sopToBucketIndex.end())
  {
    if (UNLIKELY(_dc.get()))
      _dc->diagStream() << " - COULD NOT FIND BUCKET FOR THE " << sopIndex << " SOP" << std::endl;
    return false;
  }

  const YQYRBucket& originalBucket(_originalBuckets[bucketIndexIt->second]);
  if (!originalBucket.getFurthestPoint())
    return false;

  YQYRCalculator::FareGeography geography;
  geography.furthestLoc = originalBucket.getFurthestPoint();

  if (useGlobalReturnsToOrigin(*fareMarket, *farePath))
  {
    geography.returnsToOrigin = _globalReturnsToOrigin;
    geography.direction = determineDirection(*fareMarket, geography.furthestLoc);
  }
  else
  {
    geography.returnsToOrigin = YQYRUtils::doesReturnToOrigin(farePath->itin()->travelSeg());

    if (geography.returnsToOrigin)
      geography.direction = YQYRCalculator::Directions::BOTH;
    else
      geography.direction = YQYRCalculator::Directions::OUTBOUND;
  }

  geography.init(fareMarket->travelSeg());
  TSE_ASSERT(geography.furthestSeg || geography.direction != YQYRCalculator::Directions::BOTH);

  return calculateYQYRsOnBucket(bucketIndexIt->second,
                                validatingCarrier,
                                carriersToProcess,
                                fareMarket,
                                fareMarket->paxTypeCortege().front(),
                                farePath,
                                geography,
                                results);
}

const Loc*
NGSYQYRCalculator::determineFurthestPoints(FurthestPointToSOPsMap& result)
{
  const AirSeg* journeyOrigin = 0;

  const std::vector<ShoppingTrx::Leg>& legs(_trx.legs());
  std::vector<ShoppingTrx::Leg>::const_iterator legIt(legs.begin());
  for (uint16_t legId = 0; legIt != legs.end(); ++legIt, ++legId)
  {
    if (legIt->stopOverLegFlag())
      continue;

    const std::vector<ShoppingTrx::SchedulingOption>& sops(legIt->sop());
    std::vector<ShoppingTrx::SchedulingOption>::const_iterator sopIt(sops.begin());

    const bool isOutbound(legIt->directionalIndicator() == FMDirection::OUTBOUND);
    for (; sopIt != sops.end(); ++sopIt)
    {
      const Itin* itin(sopIt->itin());
      if (!itin || itin->isDummy())
        continue;

      const std::vector<TravelSeg*>& travelSegs(itin->travelSeg());
      if (travelSegs.empty())
        continue;

      if (!journeyOrigin && legIt == legs.begin()) // journey origin is on the first leg only
        journeyOrigin = determineJourneyOrigin(travelSegs);

      if (!journeyOrigin)
        continue;

      const Loc* furthestPoint = 0;
      if (isOutbound)
        furthestPoint = determineFurthestPoint(
            travelSegs.rbegin(), travelSegs.rend(), journeyOrigin, itin, isOutbound);
      else
        furthestPoint = determineFurthestPoint(
            travelSegs.begin(), travelSegs.end(), journeyOrigin, itin, isOutbound);

      if (furthestPoint)
        result[furthestPoint].insert(std::make_pair(sopIt->originalSopId(), legId));
    }
  }

  if (journeyOrigin)
    return journeyOrigin->origin();

  return 0;
}

const AirSeg*
NGSYQYRCalculator::determineJourneyOrigin(const std::vector<TravelSeg*>& travelSegs)
{
  const AirSeg* result = 0;
  for (const TravelSeg* seg : travelSegs)
  {
    result = seg->toAirSeg();
    if (result)
      break;
  }

  return result;
}

void
NGSYQYRCalculator::createBuckets(const FurthestPointToSOPsMap& map,
                                 std::vector<YQYRBucket>& buckets)
{
  buckets.reserve(map.size());
  for (const FurthestPointToSOPsMap::value_type& vpair : map)
  {
    const uint16_t bucketIndex = buckets.size();
    for (const SopAndLegPair& sopAndLeg : vpair.second)
      _sopToBucketIndex.insert(std::make_pair(sopAndLeg, bucketIndex));

    buckets.push_back(YQYRBucket(vpair.first));
  }
}

void
NGSYQYRCalculator::printDiagHeader(const FurthestPointToSOPsMap& map)
{
  DiagStream stream(_dc->diagStream());

  stream << " SOLO YQYR CALCULATOR" << std::endl;
  stream << " - JOURNEY ORIGIN: " << _journeyOrigin->loc() << std::endl;
  stream << " - FURTHEST POINTS DETERMINED: " << std::endl;
  for (const FurthestPointToSOPsMap::value_type& vpair : map)
  {
    stream << " - " << vpair.first->loc() << ", SOPS APPLICABLE: ";
    for (const SopAndLegPair& sopAndLeg : vpair.second)
      stream << sopAndLeg.first << " [" << sopAndLeg.second << "], ";
    stream << std::endl;
  }

  stream << std::endl;
}

YQYRCalculator::Directions
NGSYQYRCalculator::determineDirection(const FareMarket& fm, const Loc* furthestLoc) const
{
  YQYRCalculator::Directions direction(YQYRCalculator::Directions::OUTBOUND);
  if (fm.legIndex() != 0)
    direction = YQYRCalculator::Directions::INBOUND;

  if (fm.travelSeg().size() <= 1)
    return direction;

  bool found = false;
  if (direction == YQYRCalculator::Directions::OUTBOUND)
    found = std::any_of(
        fm.travelSeg().cbegin(), fm.travelSeg().cend() - 1, [furthestLoc](TravelSeg* seg)
        {
          return seg->destination() == furthestLoc;
        });
  else
    found = std::any_of(
        fm.travelSeg().cbegin() + 1, fm.travelSeg().cend(), [furthestLoc](TravelSeg* seg)
        {
          return seg->origin() == furthestLoc;
        });

  if (found)
  {
    // The furthest point is in the middle of the market, process both directions.
    return YQYRCalculator::Directions::BOTH;
  }

  return direction;
}

bool
NGSYQYRCalculator::useGlobalReturnsToOrigin(const FareMarket& fm, const FarePath& fp) const
{
  // The local fare market will not be a good indicator of return-to-origin flag.
  if (fm.getFmTypeSol() == FareMarket::SOL_FM_LOCAL)
    return true;

  // This fare market is the whole journey.
  if (_trx.legs().size() == 1)
    return false;

  return true;
}
}
}
