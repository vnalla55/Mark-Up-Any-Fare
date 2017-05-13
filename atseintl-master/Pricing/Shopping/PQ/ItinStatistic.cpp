// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
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

#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/Billing.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/PQ/AltDatesStatistic.h"
#include "Pricing/Shopping/PQ/SoloGroupFarePath.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include <boost/iterator/counting_iterator.hpp>

#include <algorithm>
#include <cmath>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackSolSwapperUseSopScore);

const MoneyAmount PRICE_COMP_DELTA = 0.01;

namespace
{
ConfigurableValue<bool>
considerAdditionalNsInTodAndOiPairing("SHOPPING_DIVERSITY",
                                      "CONSIDER_ADDITIONAL_NS_IN_TOD_AND_OI_PAIRING",
                                      false);
}

ItinStatistic::ItinStatistic(ShoppingTrx& trx)
  : _diversity(trx.diversity()),
    _legs(trx.legs()),
    _trx(trx),
    _considerAdditionalNsInTodAndOIPairing(considerAdditionalNsInTodAndOiPairing.getValue())
{
  std::fill(_buckets, _buckets + Diversity::BUCKET_COUNT, 0);
}

ItinStatistic::ItinStatistic(ShoppingTrx& trx, Diversity& diversity)
  : _diversity(diversity),
    _legs(trx.legs()),
    _trx(trx),
    _considerAdditionalNsInTodAndOIPairing(considerAdditionalNsInTodAndOiPairing.getValue())
{
  std::fill(_buckets, _buckets + Diversity::BUCKET_COUNT, 0);
}

void
ItinStatistic::resetCarriers()
{
  _carrierDiversity.clear();
  const std::map<CarrierCode, float>& cxrReq = _diversity.getOptionsPerCarrierMap();
  std::map<CarrierCode, float>::const_iterator cxrReqIt(cxrReq.begin());
  for (; cxrReqIt != cxrReq.end(); ++cxrReqIt)
    _carrierDiversity.insert(std::make_pair(cxrReqIt->first, 0));

  if (_diversity.hasDCL())
  {
    for (const auto& element : _diversity.getDCLMap())
    {
      _carrierOptionsOnline.insert(std::make_pair(element.first, 0));
    }
  }
}

void
ItinStatistic::resetNonStopCarriers()
{
  _nonStopCarriers.clear();
  _additionalNonStopCountCxr.clear();
  _additionalNonStopCountCxr.insert(std::make_pair(Diversity::INTERLINE_CARRIER, 0));

  for (CarrierCode cxr : _diversity.getDirectOptionsCarriers())
  {
    std::pair<CarrierCode, size_t> cxrNS(cxr, 0);
    _nonStopCarriers.insert(cxrNS);
    _additionalNonStopCountCxr.insert(cxrNS);
  }
}

int
ItinStatistic::getNumOfNonStopItinsForCarrier(CarrierCode carrier) const
{
  if (!_diversity.shouldConsiderCarrierForNonStop(carrier))
  {
    return -1;
  }
  if (carrier == Diversity::INTERLINE_CARRIER)
  {
    return _interlineNonStopsCount;
  }
  std::map<CarrierCode, size_t>::const_iterator findIt = _nonStopCarriers.find(carrier);
  if (findIt != _nonStopCarriers.end())
    return findIt->second;

  return -1;
}

size_t
ItinStatistic::getAdditionalNonStopCountPerCarrier(CarrierCode carrier) const
{
  std::map<CarrierCode, size_t>::const_iterator findIt = _additionalNonStopCountCxr.find(carrier);
  if (findIt != _additionalNonStopCountCxr.end())
    return findIt->second;
  return 0;
}

size_t
ItinStatistic::getInboundCount(const size_t outboundSOP) const
{
  std::map<size_t, size_t>::const_iterator pairingIt = _sopPairingPerLeg[0].find(outboundSOP);
  if (pairingIt != _sopPairingPerLeg[0].end())
    return pairingIt->second;

  return 0;
}

size_t
ItinStatistic::getInboundOptionsNeeded(size_t outboundSOP) const
{
  size_t ibCount = getInboundCount(outboundSOP);
  if (_diversity.getInboundOutboundPairing() < ibCount)
    return 0;

  return _diversity.getInboundOutboundPairing() - ibCount;
}

size_t
ItinStatistic::getTODBucketSize(const size_t todBucket) const
{
  if (LIKELY(todBucket < _todBuckets.size()))
    return _todBuckets[todBucket];

  return 0;
}

size_t
ItinStatistic::getOptionsNeededForTODBucket(size_t todBucket) const
{
  size_t totalRequested = _diversity.getNumberOfOptionsToGenerate();
  if (UNLIKELY(considerAdditionalNsInTodAndOIPairing()))
    totalRequested += _diversity.getNonStopOptionsCount();

  size_t minTODBucketSize = (size_t)(_diversity.getTODDistribution()[todBucket] * totalRequested);
  size_t todBucketSize = getTODBucketSize(todBucket);
  if (minTODBucketSize < todBucketSize)
    return 0;

  return minTODBucketSize - todBucketSize;
}

void
ItinStatistic::setEnabledStatistics(int32_t value, void* subscriber)
{
  // newStats now has only those bits ON from specified in 'value', that were previously OFF in
  // enabledStats
  int32_t newStats = (_enabledStats ^ value) & value;
  if (newStats != 0)
  {
    if (newStats & STAT_MIN_PRICE)
      _minPrice = std::numeric_limits<MoneyAmount>::max();
    if (newStats & STAT_AVG_PRICE)
      _avgPrice = 0;
    if (newStats & STAT_MAX_PRICE)
      _maxPrice = 0;
    if (newStats & STAT_MIN_DURATION)
      _minDuration = std::numeric_limits<int32_t>::max();
    if (newStats & STAT_AVG_DURATION)
      _avgDuration = 0;
    if (newStats & STAT_NON_STOP_COUNT)
    {
      _onlineNonStopsCount = 0;
      _interlineNonStopsCount = 0;
      _additionalOnlineNonStopsCount = 0;
      _additionalInterlineNonStopsCount = 0;
    }
    if (newStats & STAT_NON_STOP_CARRIERS)
      resetNonStopCarriers();
    if (newStats & STAT_CARRIERS)
      resetCarriers();
    if (newStats & STAT_TOD)
      _todBuckets.assign(_diversity.getTODRanges().size(), 0);
    if (newStats & STAT_BUCKETS)
    {
      for (size_t i = 0; i < Diversity::BUCKET_COUNT; ++i)
        _buckets[i] = 0;
    }
    if (newStats & STAT_SOP_PAIRING)
      _nsObIbPairing.clear();
    if (newStats & STAT_BUCKETS_PAIRING)
    {
      for (size_t i = 0; i < Diversity::BUCKET_COUNT; ++i)
        _bucketPairing[i].clear();
      for (size_t i = 0; i < Diversity::NSBUCKET_COUNT; ++i)
        _nsBucketPairing[i].clear();
    }
    if (newStats & STAT_BUCKETS_MIN_PRICE)
    {
      for (size_t i = 0; i < 2; ++i)
      {
        _minPricePerBucket[i] = std::numeric_limits<MoneyAmount>::max();
        _minPricedItinCount[i] = 0;
      }
    }
    if (newStats & STAT_CUSTOM_SOLUTION)
    {
      _customSolutionCount = 0;
    }
    if (newStats & STAT_LONG_CONNECTION)
    {
      _longConnectionCount = 0;
    }
    if (newStats & STAT_UNUSED_SOPS)
    {
      resetUnusedSops();
    }
    if (newStats & STAT_RC_ONLINES)
    {
      _RCOnlineOptionsCount = 0;
      _missingRCOnlineOptionsCount = 0;

      const tse::Billing* billing = _trx.billing();
      if (billing)
      {
        _requestingCarrier = billing->partitionID();
        resetUnusedRCOnlines();
      }
    }
    if (newStats & STAT_UNUSED_DIRECTS)
    {
      _missingDirectOptionsCount = 0;
      _directOptionsCount = 0;
      resetUnusedDirects();
    }
    if (newStats & STAT_UNUSED_COMB)
    {
      resetUnusedCombinations();
    }
    bool isActivatingTogether = (newStats & STAT_BUCKETS_PAIRING) && (newStats & STAT_SOP_PAIRING);
    // activating both in the middle must be avoided until proper workaround will have been
    // implemented
    TSE_ASSERT(_trx.flightMatrix().empty() || !isActivatingTogether ||
               !"STAT_SOP_PAIRING and STAT_BUCKETS_PAIRING are activated together in the middle");
    for (ShoppingTrx::FlightMatrix::const_iterator solIt = _trx.flightMatrix().begin();
         solIt != _trx.flightMatrix().end();
         ++solIt)
    {
      updateStatistics(*solIt, newStats, nullptr);
    }
  }

  _enabledStats.setEnabledStats(value, subscriber);
}

class IbfBucketSizeComputator
{
public:
  IbfBucketSizeComputator(size_t size, DiagCollector* diagCollector)
    : dc(diagCollector), _availableRoom(size), _bucketsCount(0)
  {
    if (dc)
    {
      *dc << "*****************************************************************************\n";
      *dc << " Setting goals for IBF buckets:\n\n";
    }
  }
  ~IbfBucketSizeComputator()
  {
    if (dc)
      *dc << "*****************************************************************************\n";
  }

  int addBucket(const std::string& name, int existingElementsCount, int maxElementsCount)
  {
    TSE_ASSERT(maxElementsCount >= existingElementsCount);
    int missingElementsCount = maxElementsCount - existingElementsCount;
    int grantedSize = std::min(missingElementsCount, _availableRoom);
    ++_bucketsCount;

    if (dc)
    {
      *dc << " Bucket " << _bucketsCount << " : " << name << "\n";

      if (existingElementsCount > 0)
        *dc << " Elements already generated as Foses: " << existingElementsCount << "\n";

      if (missingElementsCount > 0)
      {
        *dc << " Missing elements : " << missingElementsCount << "\n";
        *dc << " " << name << " bucket max capacity : " << _availableRoom << "\n";
        *dc << " Setting Missing " << name << " bucket size to : " << grantedSize << "\n";
        *dc << " Which in conjunction with options already generated gives a goal of : "
            << grantedSize + existingElementsCount << "\n";
      }
      *dc << "\n";
    }

    _availableRoom -= grantedSize;
    TSE_ASSERT(_availableRoom >= 0);
    return grantedSize;
  }

private:
  DiagCollector* dc;
  int _availableRoom;
  size_t _bucketsCount;
};

void
ItinStatistic::setGoalsForIBF(DiagCollector* dc)
{
  bool allFlightsDiversity = _trx.getRequest()->isAllFlightsRepresented();

  _bucketMatching.clear();

  size_t requestedOptions = _trx.getOptions()->getRequestedNumberOfSolutions();
  size_t optionsAlreadyGenerated = _trx.flightMatrix().size();

  // This should not happen (as we stop processing earlier if we have enough options already )
  // but if it did the computations would be incorrect so it's better to leave all the values as
  // default (0)
  if (optionsAlreadyGenerated > requestedOptions)
    return;

  IbfBucketSizeComputator bucketSizeComputator(requestedOptions - optionsAlreadyGenerated, dc);

  if (!allFlightsDiversity)
  {
    bucketSizeComputator.addBucket("Direct Options",
                                   optionsAlreadyGenerated,
                                   static_cast<size_t>(_diversity.getMaxNonStopCount()));
  }

  bucketSizeComputator.addBucket(
      "All Flights Represented",
      0,
      std::max(_unusedSopIdsPerLeg[0].size(), _unusedSopIdsPerLeg[1].size()));

  if (allFlightsDiversity)
  {
    _missingDirectOptionsCount =
        bucketSizeComputator.addBucket("Direct Options", 0, _diversity.getMaxNonStopCount());
  }

  _missingRCOnlineOptionsCount =
      bucketSizeComputator.addBucket("Req.Carrier Online Options",
                                     getRCOnlineOptionsCount(),
                                     static_cast<size_t>(_diversity.getMaxRCOnlineOptionsCount()));

  _ibfNeedsAny =
      (bucketSizeComputator.addBucket("Junk Options up to Q0S", 0, requestedOptions) > 0);

  return;
}

void
ItinStatistic::addSolution(const ShoppingTrx::FlightMatrix::value_type& data,
                           const DatePair* datePair)
{
  _processedOptionsCount++;
  _totalOptionsCount++;
  updateStatistics(data, _enabledStats, datePair);
}

void
ItinStatistic::addNonStopSolution(const ShoppingTrx::FlightMatrix::value_type& data)
{
  const ShoppingTrx::SchedulingOption* outbound = nullptr;
  const ShoppingTrx::SchedulingOption* inbound = nullptr;
  SopCombinationUtil::getSops(_trx, data.first, &outbound, &inbound);

  CarrierCode carrier = SopCombinationUtil::detectCarrier(outbound, inbound);
  size_t pairing = 0;

  _processedOptionsCount++;
  _totalOptionsCount++;

  if (_enabledStats & STAT_NON_STOP_COUNT)
  {
    if (carrier != Diversity::INTERLINE_CARRIER)
      _additionalOnlineNonStopsCount++;
    else
      _additionalInterlineNonStopsCount++;
  }
  if (_enabledStats & STAT_NON_STOP_CARRIERS)
  {
    std::map<CarrierCode, size_t>::iterator cxrIt = _additionalNonStopCountCxr.find(carrier);
    TSE_ASSERT(cxrIt != _additionalNonStopCountCxr.end());
    cxrIt->second++;
  }
  if (_enabledStats & STAT_SOP_PAIRING)
  {
    pairing = ++_nsObIbPairing[data.first[0]];
  }
  if (_enabledStats & STAT_BUCKETS_PAIRING)
  {
    Diversity::NSBucketType bucket = detectNSBucket(outbound, inbound);
    addNSBucketPairing(bucket, pairing, data);
  }

  if (!_considerAdditionalNsInTodAndOIPairing)
    return;

  if (_enabledStats & STAT_TOD)
  {
    _todBuckets[outbound->itin()->getTODBucket(_diversity.getTODRanges())]++;
  }

  if (_enabledStats & STAT_SOP_PAIRING)
  {
    addSopPairing(data.first);
  }
}

void
ItinStatistic::removeSolution(Diversity::BucketType bucket, size_t pairing, const Solution data)
{
  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  SopCombinationUtil::getSops(_trx, data.first, &outbound, &inbound);

  boost::optional<CarrierCode> carrierCode;

  bool isFos = (data.second == nullptr);
  bool isNonStop = false;
  if (LIKELY(_enabledStats & (STAT_NON_STOP_COUNT | STAT_NON_STOP_CARRIERS | STAT_UNUSED_DIRECTS)))
  {
    isNonStop =
        (SopCombinationUtil::detectNonStop(outbound, inbound) & SopCombinationUtil::NON_STOP);
    carrierCode = SopCombinationUtil::detectCarrier(outbound, inbound);
  }
  if (LIKELY(_enabledStats & STAT_NON_STOP_COUNT))
  {
    if (isNonStop)
    {
      if (*carrierCode != Diversity::INTERLINE_CARRIER)
        --_onlineNonStopsCount;
      else
        --_interlineNonStopsCount;
    }
  }
  if (LIKELY(_enabledStats & STAT_NON_STOP_CARRIERS))
  {
    if (isNonStop)
    {
      if (*carrierCode != Diversity::INTERLINE_CARRIER)
      {
        auto noneSCItr = _nonStopCarriers.find(*carrierCode);
        if (noneSCItr != _nonStopCarriers.end())
        noneSCItr->second--;
      }
    }
  }
  if (LIKELY(_enabledStats & STAT_CARRIERS))
  {
    SopCombinationUtil::detectCarrier(outbound, inbound, carrierCode);
	
    if (*carrierCode != Diversity::INTERLINE_CARRIER)
    {
      auto carrierDItr = _carrierDiversity.find(*carrierCode);
      if (carrierDItr != _carrierDiversity.end())
        carrierDItr->second--;
    }

    if (_diversity.isOCO() &&
        ShoppingUtil::isOnlineOptionForCarrier(outbound, inbound, *carrierCode))
    {
      auto carrierItr = _carrierOptionsOnline.find(*carrierCode);
      if (carrierItr != _carrierOptionsOnline.end())
        carrierItr->second--;
    }
  }
  if (_enabledStats & STAT_CUSTOM_SOLUTION)
  {
    if (ShoppingUtil::isCustomSolution(_trx, data.first))
      --_customSolutionCount;
  }
  if (_enabledStats & STAT_LONG_CONNECTION)
  {
    if (ShoppingUtil::isLongConnection(_trx, data.first))
      --_longConnectionCount;
  }
  if (LIKELY(_enabledStats & STAT_SOP_PAIRING))
  {
    removeSopPairing(data.first);
  }

  if (LIKELY(_enabledStats & STAT_TOD))
  {
    _todBuckets[outbound->itin()->getTODBucket(_diversity.getTODRanges())]--;
  }
  if (LIKELY((_enabledStats & STAT_BUCKETS) && !isFos))
  {
    _buckets[bucket]--;
  }
  if (LIKELY((_enabledStats & STAT_BUCKETS_PAIRING) && !isFos))
  {
    TSE_ASSERT((_enabledStats & STAT_SOP_PAIRING) || !"Sop pairing statistic is required");
    removeBucketPairing(bucket, pairing, data);
  }
  if (LIKELY((_enabledStats & STAT_BUCKETS_MIN_PRICE) && !isFos))
  {
    if (bucket == Diversity::GOLD || bucket == Diversity::UGLY)
    {
      MoneyAmount price(data.second->getTotalNUCAmount());
      if (std::abs(_minPricePerBucket[bucket] - price) < PRICE_COMP_DELTA)
        --_minPricedItinCount[bucket];
    }
  }
  if (LIKELY(_enabledStats & STAT_UNUSED_SOPS))
  {
    updateUnusedSops(data.first);
  }
  if (_enabledStats & STAT_RC_ONLINES)
  {
    if (ShoppingUtil::isOnlineOptionForCarrier(_trx, data.first, _requestingCarrier))
    {
      --_RCOnlineOptionsCount;
      ++_missingRCOnlineOptionsCount;
      _unusedRCOnlineCombinations.insert(data.first);
    }
  }
  if (_enabledStats & STAT_UNUSED_DIRECTS)
  {
    if (isNonStop)
    {
      --_directOptionsCount;
      ++_missingDirectOptionsCount;
      _unusedDirectCombinations.insert(data.first);
    }
  }
  if (_enabledStats & STAT_UNUSED_COMB)
  {
    _unusedCombinations.insert(data.first);
  }
  _totalOptionsCount--;
  if (LIKELY(!isFos))
  {
    const shpq::SoloGroupFarePath* gfp = static_cast<const shpq::SoloGroupFarePath*>(data.second);
    if (gfp->getSolutionPattern()->isThruPattern())
      _thruFPCount--;
  }
}

void
ItinStatistic::removeNonStopSolution(Diversity::NSBucketType bucket,
                                     size_t pairing,
                                     const Solution data)
{
  const ShoppingTrx::SchedulingOption* outbound = nullptr;
  const ShoppingTrx::SchedulingOption* inbound = nullptr;
  SopCombinationUtil::getSops(_trx, data.first, &outbound, &inbound);

  CarrierCode carrier = SopCombinationUtil::detectCarrier(outbound, inbound);

  _totalOptionsCount--;

  if (_enabledStats & STAT_NON_STOP_COUNT)
  {
    if (carrier != Diversity::INTERLINE_CARRIER)
      _additionalOnlineNonStopsCount--;
    else
      _additionalInterlineNonStopsCount--;
  }
  if (_enabledStats & STAT_NON_STOP_CARRIERS)
  {
    std::map<CarrierCode, size_t>::iterator cxrIt = _additionalNonStopCountCxr.find(carrier);
    TSE_ASSERT(cxrIt != _additionalNonStopCountCxr.end());
    cxrIt->second--;
  }
  if (_enabledStats & STAT_SOP_PAIRING)
  {
    std::map<size_t, size_t>::iterator obIbIt = _nsObIbPairing.find(data.first[0]);
    if (obIbIt != _nsObIbPairing.end())
    {
      obIbIt->second--;
      if (obIbIt->second == 0)
        _nsObIbPairing.erase(obIbIt);
    }
  }
  if (_enabledStats & STAT_BUCKETS_PAIRING)
  {
    Diversity::NSBucketType bucket = detectNSBucket(outbound, inbound);
    removeNSBucketPairing(bucket, pairing, data);
  }

  if (!_considerAdditionalNsInTodAndOIPairing)
    return;

  if (_enabledStats & STAT_TOD)
  {
    _todBuckets[outbound->itin()->getTODBucket(_diversity.getTODRanges())]--;
  }

  if (_enabledStats & STAT_SOP_PAIRING)
  {
    removeSopPairing(data.first);
  }
}

void
ItinStatistic::updateStatistics(const ShoppingTrx::FlightMatrix::value_type& data,
                                int32_t enabledStatsScoped,
                                const DatePair* datePair)
{
  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  SopCombinationUtil::getSops(_trx, data.first, &outbound, &inbound);

  boost::optional<CarrierCode> carrierCode;

  bool isFos = (data.second == nullptr);
  const MoneyAmount price =
      isFos ? std::numeric_limits<MoneyAmount>::max() : data.second->getTotalNUCAmount();
  if ((enabledStatsScoped & STAT_MIN_PRICE) && !isFos)
  {
    _minPrice = std::min(_minPrice, price);
  }
  if ((enabledStatsScoped & STAT_AVG_PRICE) && !isFos)
  {
    _avgPrice *= (_totalOptionsCount - 1) / (double)_totalOptionsCount;
    _avgPrice += price / _totalOptionsCount;
  }
  if ((enabledStatsScoped & STAT_MAX_PRICE) && !isFos)
  {
    _maxPrice = std::max(_maxPrice, price);
  }

  int32_t duration = 0;
  if (enabledStatsScoped & (STAT_MIN_DURATION | STAT_AVG_DURATION))
  {
    duration = SopCombinationUtil::getDuration(outbound, inbound);
  }
  if (enabledStatsScoped & STAT_MIN_DURATION)
  {
    _minDuration = std::min(_minDuration, duration);
  }
  if (enabledStatsScoped & STAT_AVG_DURATION)
  {
    _avgDuration *= (_totalOptionsCount - 1) / (double)_totalOptionsCount;
    _avgDuration += duration / (double)_totalOptionsCount;
  }
  bool isNonStop = false;
  if (enabledStatsScoped & (STAT_NON_STOP_COUNT | STAT_NON_STOP_CARRIERS | STAT_UNUSED_DIRECTS))
  {
    isNonStop =
        (SopCombinationUtil::detectNonStop(outbound, inbound) & SopCombinationUtil::NON_STOP);
    carrierCode = SopCombinationUtil::detectCarrier(outbound, inbound);
  }
  if (enabledStatsScoped & STAT_NON_STOP_COUNT)
  {
    if (isNonStop)
    {
      if (*carrierCode != Diversity::INTERLINE_CARRIER)
        _onlineNonStopsCount++;
      else
        _interlineNonStopsCount++;
    }
  }
  if (enabledStatsScoped & STAT_NON_STOP_CARRIERS)
  {
    if (isNonStop)
    {
      if (*carrierCode != Diversity::INTERLINE_CARRIER)
      {
        std::map<CarrierCode, size_t>::iterator findIt = _nonStopCarriers.find(*carrierCode);
        TSE_ASSERT(findIt != _nonStopCarriers.end()); // for debug only

        ++findIt->second;
      }
    }
  }
  if (enabledStatsScoped & STAT_CARRIERS)
  {
    SopCombinationUtil::detectCarrier(outbound, inbound, carrierCode);

    if (*carrierCode != Diversity::INTERLINE_CARRIER)
    {
      auto carrierDItr = _carrierDiversity.find(*carrierCode);
      if (carrierDItr != _carrierDiversity.end())
        carrierDItr->second++;
    }

    if (_diversity.isOCO() &&
        ShoppingUtil::isOnlineOptionForCarrier(outbound, inbound, *carrierCode))
    {
      auto carrierItr = _carrierOptionsOnline.find(*carrierCode);
      if (carrierItr != _carrierOptionsOnline.end())
        carrierItr->second++;
    }
  }
  if (enabledStatsScoped & STAT_TOD)
  {
    _todBuckets[outbound->itin()->getTODBucket(_diversity.getTODRanges())]++;
  }
  size_t pairing = 0;
  if (enabledStatsScoped & STAT_SOP_PAIRING)
  {
    pairing = addSopPairing(data.first);
  }
  else if (_enabledStats & STAT_SOP_PAIRING)
  {
    pairing = getSopPairing(0, data.first[0]);
  }
  Diversity::BucketType bucket = Diversity::BUCKET_COUNT;
  if ((enabledStatsScoped & STAT_BUCKETS) && !isFos)
  {
    bucket = detectBucket(outbound, inbound, price);
    _buckets[bucket]++;
  }
  // STAT_PAIRING and STAT_BUCKETS must be enabled!
  if ((enabledStatsScoped & STAT_BUCKETS_PAIRING) && !isFos)
  {
    TSE_ASSERT((_enabledStats & STAT_SOP_PAIRING) || !"Sop pairing statistic is required");
    addBucketPairing(bucket, pairing, outbound, inbound, data);
  }
  if ((enabledStatsScoped & STAT_BUCKETS_MIN_PRICE) && !isFos)
  {
    if (UNLIKELY(bucket == Diversity::BUCKET_COUNT))
      bucket = detectBucket(outbound, inbound, price);
    if (bucket == Diversity::GOLD || bucket == Diversity::UGLY)
    {
      if (fabs(price - _minPricePerBucket[bucket]) < PRICE_COMP_DELTA)
        ++_minPricedItinCount[bucket];
      else if (price < _minPricePerBucket[bucket])
      {
        _minPricePerBucket[bucket] = price;
        _minPricedItinCount[bucket] = 1;
      }
    }
  }
  if (enabledStatsScoped & STAT_CUSTOM_SOLUTION)
  {
    if (ShoppingUtil::isCustomSolution(_trx, data.first))
      ++_customSolutionCount;
  }
  if (enabledStatsScoped & STAT_LONG_CONNECTION)
  {
    if (ShoppingUtil::isLongConnection(_trx, data.first))
      ++_longConnectionCount;
  }
  if (enabledStatsScoped & STAT_UNUSED_SOPS)
  {
    updateUnusedSops(data.first);
  }
  if (UNLIKELY(enabledStatsScoped & STAT_RC_ONLINES))
  {
    if (ShoppingUtil::isOnlineOptionForCarrier(_trx, data.first, _requestingCarrier))
    {
      ++_RCOnlineOptionsCount;
      --_missingRCOnlineOptionsCount;
      _unusedRCOnlineCombinations.erase(data.first);
    }
  }
  if (UNLIKELY(enabledStatsScoped & STAT_UNUSED_DIRECTS))
  {
    if (isNonStop)
    {
      ++_directOptionsCount;
      --_missingDirectOptionsCount;
      _unusedDirectCombinations.erase(data.first);
    }
  }
  if (UNLIKELY(enabledStatsScoped & STAT_UNUSED_COMB))
  {
    _unusedCombinations.erase(data.first);
  }

  if (_altDates != nullptr)
  {
    TSE_ASSERT(datePair != nullptr && "Alt dates statistic cannot be turned on in the middle");
    _altDates->addSolution(data, *datePair);
  }
  if (LIKELY(!isFos))
  {
    const shpq::SoloGroupFarePath* gfp = static_cast<const shpq::SoloGroupFarePath*>(data.second);
    if (gfp->getSolutionPattern()->isThruPattern())
      _thruFPCount++;
  }
}

void
ItinStatistic::addBucketPairing(Diversity::BucketType bucket,
                                size_t pairing,
                                const ShoppingTrx::SchedulingOption* outbound,
                                const ShoppingTrx::SchedulingOption* inbound,
                                const ShoppingTrx::FlightMatrix::value_type& data)
{
  if (bucket == Diversity::BUCKET_COUNT)
    bucket = detectBucket(outbound, inbound, data.second->getTotalNUCAmount());

  if (!inbound)
  {
    size_t score = 0;
    if (!fallback::fixed::fallbackSolSwapperUseSopScore())
      score = data.first[0];

    _bucketPairing[bucket][1][score].push_back(data);
    return;
  }

  // Add to one level lower pairing, so it will be upgraded automatically
  if (fallback::fixed::fallbackSolSwapperUseSopScore())
    _bucketPairing[bucket][pairing - 1][0].push_back(data);
  else
  {
    size_t score = (data.first[0] + 1) * (data.first[1] + 1);
    _bucketPairing[bucket][pairing][score].push_back(data);
  }

  if (fallback::fixed::fallbackSolSwapperUseSopScore())
    moveOptionsWithSameOutboundToTheEnd(pairing, data.first[0], false);
}

void
ItinStatistic::addNSBucketPairing(Diversity::NSBucketType bucket,
                                  size_t pairing,
                                  const ShoppingTrx::FlightMatrix::value_type& data)
{
  size_t score = static_cast<size_t>(data.second->getTotalNUCAmount());
  _nsBucketPairing[bucket][pairing][score].push_back(data);
}

void
ItinStatistic::removeBucketPairing(Diversity::BucketType bucket,
                                   size_t pairing,
                                   const Solution data)
{
  if (data.first.size() == 1)
  {
    size_t score = 0;
    if (!fallback::fixed::fallbackSolSwapperUseSopScore())
      score = data.first[0];

    if (_bucketPairing[bucket][1][score].size() == 1)
      _bucketPairing[bucket].erase(1);
    else
      _bucketPairing[bucket][1][score].remove(data);
    return;
  }

  size_t score = 0;
  if (!fallback::fixed::fallbackSolSwapperUseSopScore())
    score = (data.first[0] + 1) * (data.first[1] + 1);

  _bucketPairing[bucket][pairing][score].remove(data);

  if (fallback::fixed::fallbackSolSwapperUseSopScore())
    moveOptionsWithSameOutboundToTheEnd(pairing, data.first[0], true);
}

void
ItinStatistic::removeNSBucketPairing(Diversity::NSBucketType bucket,
                                     size_t pairing,
                                     const Solution data)
{
  size_t score = static_cast<size_t>(data.second->getTotalNUCAmount());
  _nsBucketPairing[bucket][pairing][score].remove(data);
}

void
ItinStatistic::moveOptionsWithSameOutboundToTheEnd(size_t pairing, int outboundSop, bool moveDown)
{
  const static size_t SCORE = 0;

  size_t pairingFrom = (moveDown ? pairing : pairing - 1);
  size_t pairingTo = (moveDown ? pairing - 1 : pairing);

  for (size_t bktIdx = 0; bktIdx < Diversity::BUCKET_COUNT; bktIdx++)
  {
    BucketPairing::iterator bktIt = _bucketPairing[bktIdx].find(pairingFrom);
    if (bktIt == _bucketPairing[bktIdx].end())
      continue;

    ScoredCombinations::mapped_type& oldList = bktIt->second[SCORE];
    ScoredCombinations::mapped_type newList;
    ScoredCombinations::mapped_type::iterator lstIt = oldList.begin();
    while (lstIt != oldList.end())
    {
      if (lstIt->first[0] == outboundSop)
        newList.splice(newList.end(), oldList, lstIt++);
      else
        ++lstIt;
    }
    if (oldList.empty())
      _bucketPairing[bktIdx].erase(bktIt);
    if (!newList.empty())
    {
      ScoredCombinations::mapped_type& storageList = _bucketPairing[bktIdx][pairingTo][SCORE];
      storageList.splice(storageList.end(), newList);
    }
  }
}

Diversity::BucketType
ItinStatistic::detectBucket(const ShoppingTrx::SchedulingOption* outbound,
                            const ShoppingTrx::SchedulingOption* inbound,
                            MoneyAmount price) const
{
  // if it has diversity carrier codes then check if this is one of the specified carriers
  
    CarrierCode carrier = SopCombinationUtil::detectCarrier(outbound, inbound);
    if (isCarrierInDCLMap(carrier))
      return Diversity::GOLD;
  
  int32_t duration = SopCombinationUtil::getDuration(outbound, inbound);
  return detectBucket(duration, price);
}

Diversity::BucketType
ItinStatistic::detectBucket(int32_t duration, MoneyAmount price) const
{
  const int32_t durationSep = _diversity.getTravelTimeSeparator();
  const MoneyAmount priceSep = _diversity.getFareAmountSeparator();
  const MoneyAmount priceDelta(price - priceSep);
  Diversity::BucketType bucket = Diversity::JUNK;
  if (priceDelta <= PRICE_COMP_DELTA && duration <= durationSep)
    bucket = Diversity::GOLD;
  else if (priceDelta > PRICE_COMP_DELTA && duration <= durationSep)
    bucket = Diversity::LUXURY;
  else if (priceDelta <= PRICE_COMP_DELTA && duration > durationSep)
    bucket = Diversity::UGLY;

  return bucket;
}

bool
ItinStatistic::isCarrierInDCLMap(const CarrierCode carrier) const
{
  if (_diversity.hasDCL() && _diversity.getDCLMap().find(carrier) != _diversity.getDCLMap().end() &&
      getNumOfItinsForCarrier(carrier) < _diversity.getDCLMap().find(carrier)->second)
    return true;
  else
    return false;
}

bool
ItinStatistic::adjustBucketDistribution(const CarrierCode carrier)
{
  if (!isCarrierInDCLMap(carrier))
    return false;

  // Verify the number of solutions required is not reached yet
  // Verify the old bucket still has at least 1 to remove
  if (getBucketSize(Diversity::LUXURY) > 0)
  {
    float newDistribution =
        ((float)(getBucketSize(Diversity::GOLD) + 1)) / _diversity.getNumberOfOptionsToGenerate();
    _diversity.setBucketDistribution(Diversity::GOLD, newDistribution);
    _diversity.setBucketDistribution(Diversity::LUXURY,
                                     1 - _diversity.getBucketDistribution(Diversity::GOLD) -
                                         _diversity.getBucketDistribution(Diversity::UGLY) -
                                         _diversity.getBucketDistribution(Diversity::JUNK));
    return true;
  }
  else
    return false;
}

Diversity::NSBucketType
ItinStatistic::detectNSBucket(const ShoppingTrx::SchedulingOption* outbound,
                              const ShoppingTrx::SchedulingOption* inbound) const
{
  if (!inbound || (outbound->governingCarrier() == inbound->governingCarrier()))
    return Diversity::NSONLINE;
  return Diversity::NSINTERLINE;
}

void
ItinStatistic::resetUnusedCombinations()
{
  const std::vector<ShoppingTrx::Leg>& legs = _trx.legs();
  size_t numberOfLegs = legs.size();
  TSE_ASSERT(numberOfLegs <= 2);

  shpq::SopIdxVec currentCombination;

  for (unsigned int i = 0; i < legs[0].requestSops(); ++i)
  {
    currentCombination[0] = i;
    if (numberOfLegs == 1)
      _unusedCombinations.insert(currentCombination);
    else
    {
      for (unsigned int j = 0; j < legs[1].requestSops(); ++j)
      {
        currentCombination[1] = j;
        _unusedCombinations.insert(currentCombination);
      }
    }
  }
}

void
ItinStatistic::resetUnusedDirects()
{
  _unusedDirectCombinations.clear();
  createAllDirectCombinations();
}

void
ItinStatistic::createAllDirectCombinations()
{
  const std::vector<ShoppingTrx::Leg>& legs = _trx.legs();
  size_t numberOfLegs = legs.size();
  // This function can only be used in NGS where there are no more than 2 legs
  TSE_ASSERT(numberOfLegs <= 2);

  shpq::SopIdxVec currentCombination;

  const ShoppingTrx::Leg& outboundLeg = legs[0];
  for (unsigned int i = 0; i < outboundLeg.requestSops(); ++i)
  {
    const ShoppingTrx::SchedulingOption& aSop = outboundLeg.sop()[i];
    if (aSop.itin()->travelSeg().size() != 1)
    {
      continue;
    }
    else
    {
      currentCombination[0] = i;
      if (numberOfLegs == 1)
      {
        _unusedDirectCombinations.insert(currentCombination);
      }
      else
      {
        const ShoppingTrx::Leg& inboundLeg = legs[1];
        for (unsigned int j = 0; j < inboundLeg.requestSops(); ++j)
        {
          const ShoppingTrx::SchedulingOption& bSop = inboundLeg.sop()[j];
          if (bSop.itin()->travelSeg().size() == 1)
          {
            currentCombination[1] = j;
            _unusedDirectCombinations.insert(currentCombination);
          }
        }
      }
    }
  }
}

void
ItinStatistic::resetUnusedRCOnlines()
{
  _unusedRCOnlineCombinations.clear();
  createAllRCOnlineCombinations();
}

void
ItinStatistic::createAllRCOnlineCombinations()
{
  const std::vector<ShoppingTrx::Leg>& legs = _trx.legs();
  size_t numberOfLegs = legs.size();
  TSE_ASSERT(numberOfLegs <= 2);

  shpq::SopIdxVec currentCombination;

  const ShoppingTrx::Leg& outboundLeg = legs[0];
  for (unsigned int i = 0; i < outboundLeg.requestSops(); ++i)
  {
    const ShoppingTrx::SchedulingOption& aSop = outboundLeg.sop()[i];
    if (ShoppingUtil::isOnlineFlight(*aSop.itin(), _requestingCarrier))
    {
      currentCombination[0] = i;
      if (numberOfLegs == 1)
      {
        _unusedRCOnlineCombinations.insert(currentCombination);
      }
      else
      {
        const ShoppingTrx::Leg& inboundLeg = legs[1];
        for (unsigned int j = 0; j < inboundLeg.requestSops(); ++j)
        {
          const ShoppingTrx::SchedulingOption& bSop = inboundLeg.sop()[j];
          if (ShoppingUtil::isOnlineFlight(*bSop.itin(), _requestingCarrier))
          {
            currentCombination[1] = j;
            _unusedRCOnlineCombinations.insert(currentCombination);
          }
        }
      }
    }
  }
}

void
ItinStatistic::resetUnusedSops()
{
  _unusedSopIdsPerLeg[0].clear();
  _unusedSopIdsPerLeg[1].clear();
  bool onlyNonDirect = _diversity.shouldGenerateAllNonStopFlightOnlySolutions();
  insertSopIds(0, onlyNonDirect);
  if (_trx.legs().size() > 1)
    insertSopIds(1, onlyNonDirect);
}

void
ItinStatistic::updateUnusedSops(shpq::SopIdxVecArg sopVec)
{
  updateUnusedSops(0, sopVec[0]);
  if (sopVec.size() > 1)
    updateUnusedSops(1, sopVec[1]);
}

void
ItinStatistic::updateUnusedSops(size_t legIdx, size_t sopIdx)
{
  size_t pairing = getSopPairing(legIdx, sopIdx);
  if (pairing == 1)
  {
    UnusedSopIds::iterator it = _unusedSopIdsPerLeg[legIdx].find(sopIdx);
    if (it != _unusedSopIdsPerLeg[legIdx].end())
      _unusedSopIdsPerLeg[legIdx].erase(it);
  }
  else if (pairing == 0)
  {
    _unusedSopIdsPerLeg[legIdx].insert(sopIdx);
  }
}

void
ItinStatistic::insertSopIds(size_t legIdx, bool onlyNonDirect)
{
  typedef std::vector<ShoppingTrx::SchedulingOption> SopVec;
  const SopVec& sops = _trx.legs()[legIdx].sop();
  UnusedSopIds& sopIds = _unusedSopIdsPerLeg[legIdx];
  size_t first = 0;
  if (onlyNonDirect)
  {
    SopVec::const_iterator it = sops.begin();
    SopVec::const_iterator itEnd = sops.end();
    // shift begin iterator to the first non-direct
    for (; it != itEnd && (it->itin()->travelSeg().size() == 1); ++it)
      first++;
  }
  sopIds.insert(boost::counting_iterator<size_t>(first),
                boost::counting_iterator<size_t>(_trx.legs()[legIdx].requestSops()));
}

size_t
ItinStatistic::addSopPairing(shpq::SopIdxVecArg sopVec)
{
  size_t pairing = addSopPairing(0, sopVec[0]);
  if (sopVec.size() > 1)
    addSopPairing(1, sopVec[1]);
  return pairing;
}

size_t
ItinStatistic::addSopPairing(size_t legIdx, size_t sopIdx)
{
  return ++_sopPairingPerLeg[legIdx][sopIdx];
}

size_t
ItinStatistic::removeSopPairing(shpq::SopIdxVecArg sopVec)
{
  size_t pairing = removeSopPairing(0, sopVec[0]);
  if (sopVec.size() > 1)
    removeSopPairing(1, sopVec[1]);
  return pairing;
}

size_t
ItinStatistic::removeSopPairing(size_t legIdx, size_t sopIdx)
{
  size_t pairing = 0;
  SopPairing::iterator it = _sopPairingPerLeg[legIdx].find(sopIdx);
  if (LIKELY(it != _sopPairingPerLeg[legIdx].end()))
  {
    pairing = it->second--;
    if (it->second == 0)
    {
      _sopPairingPerLeg[legIdx].erase(it);
    }
  }
  return pairing;
}

} /* namespace tse */
