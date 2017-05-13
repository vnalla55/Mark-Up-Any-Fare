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

#pragma once

#include "Common/Assert.h"
#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include <boost/logic/tribool.hpp>
#include <boost/noncopyable.hpp>

#include <list>
#include <map>
#include <set>
#include <vector>

#include <tr1/array>

namespace tse
{

class AltDatesStatistic;
class DiagCollector;

class ItinStatistic : private boost::noncopyable
{
public:
  using Solution = std::pair<shpq::SopIdxVec, ShoppingTrx::FlightMatrix::mapped_type>;
  using ScoredCombinations = std::map<size_t, std::list<Solution>, std::greater<size_t>>;
  using BucketPairing = std::map<size_t, ScoredCombinations, std::greater<size_t>>;
  using SopPairing = std::map<size_t, size_t>;
  using SopPairingPerLeg = std::tr1::array<SopPairing, 2>;
  using UnusedSopIds = std::set<size_t>;
  using CombinationWithStatus = std::pair<shpq::SopIdxVecArg, int>;
  using CombinationWithStatusVec = std::vector<CombinationWithStatus>;
  using UnusedSopIdsPerLeg = std::tr1::array<UnusedSopIds, 2>;
  using UnusedRCOnlineCombs = std::set<shpq::SopIdxVec>;
  using UnusedDirectCombinations = std::set<shpq::SopIdxVec>;
  using UnusedCombinations = std::set<shpq::SopIdxVec>;

  enum
  {
    STAT_MIN_PRICE = 0x00001,
    STAT_AVG_PRICE = 0x00002,
    STAT_MAX_PRICE = 0x00004,
    STAT_MIN_DURATION = 0x00008,
    STAT_AVG_DURATION = 0x00010,
    STAT_NON_STOP_COUNT = 0x00020,
    STAT_NON_STOP_CARRIERS = 0x00040,
    STAT_CARRIERS = 0x00080,
    STAT_TOD = 0x00100,
    STAT_SOP_PAIRING = 0x00200,
    STAT_BUCKETS = 0x00400,
    STAT_BUCKETS_PAIRING = 0x00800 | STAT_SOP_PAIRING,
    STAT_BUCKETS_MIN_PRICE = 0x01000,
    STAT_CUSTOM_SOLUTION = 0x02000,
    STAT_LONG_CONNECTION = 0x04000,
    STAT_UNUSED_SOPS = 0x08000 | STAT_SOP_PAIRING,
    STAT_RC_ONLINES = 0x10000,
    STAT_UNUSED_COMB = 0x20000,
    STAT_UNUSED_DIRECTS = 0x40000
  };
  ItinStatistic(ShoppingTrx& trx);
  ItinStatistic(ShoppingTrx& trx, Diversity& diversity);

  // Statistic getters

  /**
   * @return value of std::numeric_limits<MoneyAmount>::max() if has not been initialized yet
   */
  MoneyAmount getMinPrice() const { return _minPrice; }

  MoneyAmount getAvgPrice() const { return _avgPrice; }
  MoneyAmount getMaxPrice() const { return _maxPrice; }
  int32_t getMinDuration() const { return _minDuration; }
  double getAvgDuration() const { return _avgDuration; }
  size_t getNonStopsCount() const { return _onlineNonStopsCount + _interlineNonStopsCount; }
  size_t getOnlineNonStopsCount() const { return _onlineNonStopsCount; }
  size_t getInterlineNonStopsCount() const { return _interlineNonStopsCount; }
  size_t getAdditionalNonStopsCount() const
  {
    return _additionalOnlineNonStopsCount + _additionalInterlineNonStopsCount;
  }
  size_t getAdditionalOnlineNonStopsCount() const { return _additionalOnlineNonStopsCount; }
  size_t getAdditionalInterlineNonStopsCount() const { return _additionalInterlineNonStopsCount; }
  int16_t getCustomSolutionCount() const { return _customSolutionCount; }
  int16_t getLongConnectionCount() const { return _longConnectionCount; }
  /**
   * Interline solutions carriers statistic is not collected into this map
   * and key for interline carrier (CarrierCode("")) is absent
   */
  const std::map<CarrierCode, size_t>& getOptionsPerCarrier() const { return _carrierDiversity; }

  const std::map<CarrierCode, size_t>&
    getCarrierOptionsOnline() const { return _carrierOptionsOnline; }

  size_t getNumOfItinsForCarrier(const CarrierCode& carrier) const
  {
    std::map<CarrierCode, size_t>::const_iterator it = _carrierDiversity.find(carrier);
    return it != _carrierDiversity.end() ? it->second : 0;
  }

  // returns -1 if there is no non-stop flights for this carrier in ItinIndex
  int getNumOfNonStopItinsForCarrier(CarrierCode carrier) const;

  // returns map, which number of elements equals Diversity::getDirectOptionsCarriers().size()
  std::map<CarrierCode, size_t> getNumOfNonStopItinsPerCarrier() const { return _nonStopCarriers; }

  size_t getAdditionalNonStopCountPerCarrier(CarrierCode carrier) const;
  const std::map<CarrierCode, size_t>& getAdditionalNonStopCountPerCarrier() const
  {
    return _additionalNonStopCountCxr;
  }

  size_t getInboundCount(const size_t outboundSOP) const;
  size_t getInboundOptionsNeeded(size_t outboundSOP) const;
  size_t getTODBucketSize(const size_t todBucket) const;
  size_t getOptionsNeededForTODBucket(size_t todBucket) const;
  size_t getBucketSize(const Diversity::BucketType bucket) const { return _buckets[bucket]; }
  const BucketPairing& getBucketPairing(Diversity::BucketType bucket) const
  {
    return _bucketPairing[bucket];
  }
  const BucketPairing& getNSBucketPairing(Diversity::NSBucketType nsbucket) const
  {
    return _nsBucketPairing[nsbucket];
  }
  MoneyAmount getMinPrice(Diversity::BucketType bucket) const { return _minPricePerBucket[bucket]; }
  size_t getMinPricedItinCount(const Diversity::BucketType bucket) const
  {
    return _minPricedItinCount[bucket];
  }
  size_t getTotalOptionsCount() const { return _totalOptionsCount; }
  size_t getProcessedOptionsCount() const { return _processedOptionsCount; }

  size_t getNumOfThruFPOptions() const { return _thruFPCount; }

  size_t getNumOfUniqueSops(size_t legIdx) const { return _sopPairingPerLeg[legIdx].size(); }
  size_t getSopPairing(size_t legIdx, size_t sopIdx) const
  {
    SopPairing::const_iterator it = _sopPairingPerLeg[legIdx].find(sopIdx);
    return (it != _sopPairingPerLeg[legIdx].end()) ? it->second : 0;
  }
  const UnusedSopIds& getUnusedSopIds(size_t legIdx) const { return _unusedSopIdsPerLeg[legIdx]; }
  const UnusedSopIdsPerLeg& getUnusedSopIdsPerLeg() const { return _unusedSopIdsPerLeg; }
  UnusedRCOnlineCombs& getUnusedRCOnlineCombs() { return _unusedRCOnlineCombinations; }
  const UnusedDirectCombinations& getUnusedDirectCombinations() const { return _unusedDirectCombinations; }
  int getMissingDirectOptionsCount() const { return _missingDirectOptionsCount; }
  int getDirectOptionsCount() const { return _directOptionsCount; }
  void createAllDirectCombinations();

  UnusedCombinations& getUnusedCombinations() { return _unusedCombinations; }

  void createAllRCOnlineCombinations();
  int getMissingRCOnlineOptionsCount() const { return _missingRCOnlineOptionsCount; }
  void setMissingRCOnlineOptionsCount(int value) { _missingRCOnlineOptionsCount = value; }
  int getRCOnlineOptionsCount() const { return _RCOnlineOptionsCount; }
  CarrierCode getRequestingCarrier() const { return _requestingCarrier; }

  void addToBucketMatchingVec(CombinationWithStatus comb) { _bucketMatching.push_back(comb); }
  CombinationWithStatusVec& getBucketMatching() { return _bucketMatching; }
  bool ibfNeedsAny() const { return _ibfNeedsAny; }

  // Statistics management
  void setEnabledStatistics(int32_t value, void* subscriber);
  void setGoalsForIBF(DiagCollector* dc);
  int32_t getEnabledStatistics(void* subscriber) const
  {
    return _enabledStats.getEnabledStats(subscriber);
  }

  AltDatesStatistic* getAltDates() { return _altDates; }
  const AltDatesStatistic* getAltDates() const { return _altDates; }
  void setupAltDates(AltDatesStatistic* altDates) { _altDates = altDates; }

  bool considerAdditionalNsInTodAndOIPairing() const
  {
    return _considerAdditionalNsInTodAndOIPairing;
  }

  // Adding/removing solutions
  void addSolution(const ShoppingTrx::FlightMatrix::value_type& data, const DatePair* datePair = nullptr);
  void addNonStopSolution(const ShoppingTrx::FlightMatrix::value_type& data);
  void addFOS(const ShoppingTrx::FlightMatrix::key_type& sops)
  {
    addSolution(ShoppingTrx::FlightMatrix::value_type(sops, nullptr));
  }

  void removeSolution(Diversity::BucketType bucket, size_t pairing, const Solution data);
  void removeNonStopSolution(Diversity::NSBucketType bucket, size_t pairing, const Solution data);

  // Helper functions
  Diversity::BucketType detectBucket(const ShoppingTrx::SchedulingOption* outbound,
                                     const ShoppingTrx::SchedulingOption* inbound,
                                     MoneyAmount price) const;
  Diversity::BucketType detectBucket(int32_t duration, MoneyAmount price) const;
  bool isCarrierInDCLMap(const CarrierCode carrier) const;
  bool adjustBucketDistribution(const CarrierCode carrier);

  Diversity::NSBucketType detectNSBucket(const ShoppingTrx::SchedulingOption* outbound,
                                         const ShoppingTrx::SchedulingOption* inbound) const;

protected:
  class EnabledStats
  {
  public:
    int32_t value() const { return _value; }
    operator int32_t() const { return value(); }

    int32_t getEnabledStats(void* subscriber) const
    {
      std::map<void*, int32_t>::const_iterator findIt = _subscribers.find(subscriber);
      if (findIt == _subscribers.end())
        return 0;
      else
        return findIt->second;
    }
    void setEnabledStats(int32_t value, void* subscriber)
    {
      _subscribers[subscriber] = value;
      update();
    }

  private:
    int32_t _value = 0;
    std::map<void*, int32_t> _subscribers;
    void update()
    {
      _value = 0;
      for (std::map<void*, int32_t>::const_iterator it = _subscribers.begin();
           it != _subscribers.end();
           ++it)
      {
        _value |= it->second;
      }
    }
  };

  void resetCarriers();
  void resetNonStopCarriers();
  void updateStatistics(const ShoppingTrx::FlightMatrix::value_type& data,
                        int32_t enabledStatsScoped,
                        const DatePair* datePair);
  void addBucketPairing(Diversity::BucketType bucket,
                        size_t pairing,
                        const ShoppingTrx::SchedulingOption* outbound,
                        const ShoppingTrx::SchedulingOption* inbound,
                        const ShoppingTrx::FlightMatrix::value_type& data);
  void addNSBucketPairing(Diversity::NSBucketType bucket,
                          size_t pairing,
                          const ShoppingTrx::FlightMatrix::value_type& data);
  void removeBucketPairing(Diversity::BucketType bucket, size_t pairing, const Solution data);
  void removeNSBucketPairing(Diversity::NSBucketType bucket, size_t pairing, const Solution data);
  void moveOptionsWithSameOutboundToTheEnd(size_t pairing, int outboundSop, bool moveDown);

  void resetUnusedSops();
  void updateUnusedSops(shpq::SopIdxVecArg sopVec);
  void updateUnusedSops(size_t legIdx, size_t sopIdx);
  void insertSopIds(size_t legIdx, bool onlyNonDirect);
  size_t addSopPairing(shpq::SopIdxVecArg sopVec);
  size_t addSopPairing(size_t legIdx, size_t sopIdx);
  size_t removeSopPairing(shpq::SopIdxVecArg sopVec);
  size_t removeSopPairing(size_t legIdx, size_t sopIdx);

  void resetUnusedRCOnlines();
  void resetUnusedDirects();
  void resetUnusedCombinations();

  Diversity& _diversity;
  const LegVec& _legs;
  ShoppingTrx& _trx;
  AltDatesStatistic* _altDates = nullptr;

  std::vector<size_t> _todBuckets;

  // Interline carriers statistic is not collected into this map
  // and key for interline carrier (CarrierCode("")) is absent
  std::map<CarrierCode, size_t> _carrierDiversity;

  std::map<CarrierCode, size_t> _carrierOptionsOnline;

  // Online carriers, at least one non-stop option is collected for
  std::map<CarrierCode, size_t> _nonStopCarriers;

  // Number of additional non-stop options per carrier (interline options = "")
  std::map<CarrierCode, size_t> _additionalNonStopCountCxr;

  SopPairingPerLeg _sopPairingPerLeg; // number of solutions for given outbound/ibound SOP
  std::map<size_t, size_t> _nsObIbPairing;
  BucketPairing _bucketPairing[Diversity::BUCKET_COUNT];
  BucketPairing _nsBucketPairing[Diversity::NSBUCKET_COUNT];
  size_t _buckets[Diversity::BUCKET_COUNT];
  size_t _totalOptionsCount = 0;
  size_t _processedOptionsCount = 0;
  size_t _onlineNonStopsCount = 0;
  size_t _interlineNonStopsCount = 0;
  size_t _additionalOnlineNonStopsCount = 0;
  size_t _additionalInterlineNonStopsCount = 0;
  size_t _minPricedItinCount[2];
  size_t _thruFPCount = 0;
  MoneyAmount _minPrice = 0;
  MoneyAmount _avgPrice = 0;
  MoneyAmount _maxPrice = 0;
  MoneyAmount _minPricePerBucket[2];
  EnabledStats _enabledStats;
  int32_t _minDuration = 0;
  double _avgDuration = 0;
  int16_t _customSolutionCount = 0;
  int16_t _longConnectionCount = 0;
  UnusedSopIdsPerLeg _unusedSopIdsPerLeg;
  int _missingRCOnlineOptionsCount = 0;
  int _RCOnlineOptionsCount = 0;
  int _missingDirectOptionsCount = 0;
  int _directOptionsCount = 0;
  CarrierCode _requestingCarrier;
  CombinationWithStatusVec _bucketMatching;
  bool _ibfNeedsAny = false;
  UnusedRCOnlineCombs _unusedRCOnlineCombinations;
  UnusedDirectCombinations _unusedDirectCombinations;
  UnusedCombinations _unusedCombinations;

  bool _considerAdditionalNsInTodAndOIPairing = false;
};
} /* namespace tse */
