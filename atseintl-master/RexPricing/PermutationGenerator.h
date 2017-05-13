//-------------------------------------------------------------------
//
//  File:        PermutationGenerator.h
//  Created:     August 9, 2007
//  Authors:     Grzegorz Cholewiak
//
//  Updates:
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

#pragma once

#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/RexPricingTrx.h"
#include "Util/CartesianProduct.h"

#include <deque>
#include <map>
#include <vector>
#include <tuple>

namespace tse
{

class RexPricingTrx;
class ProcessTagPermutation;
class ProcessTagInfo;
class Diag688Collector;
class DiagCollector;
class FareMarket;

class PermutationGenerator
{
  friend class PermutationGeneratorTest;
  friend class TagWarEngineTest;

public:
  explicit PermutationGenerator(RexPricingTrx& trx);
  virtual ~PermutationGenerator();
  void process();

protected:
  typedef std::map<std::pair<ProcessTagInfo*, ProcessTagInfo*>, ProcessTagInfo*> PTICache;

  void markWithFareTypes(ProcessTagPermutation& perm);

  void markKeepForZeroT988(ProcessTagPermutation& perm, const PaxTypeFare& paxTypeFare);

  void checkStopByte(std::vector<ProcessTagInfo*>& FCtags);
  bool
  isProperTagsSequence(CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType& permSelector,
                       std::string& validationOut);

  const std::pair<bool, bool> permCharacteristic(
      const CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType& permSelector,
      std::string& validationOut) const;

  void checkFareRetrieval(const ProcessTagPermutation& perm);
  bool mapKeepForComponent(const ProcessTagPermutation& perm,
                           const PaxTypeFare& excPtf,
                           FareApplication fa) const;

  void mapKeepFares(ProcessTagPermutation& perm);

  void mapExpndKeepFares();

  void matchToNewItinFmForExpndKeepFare(const PaxTypeFare& excPtf);
  FareMarket* matchToNewItinFmForKeepFare(const FareMarket* excItinFm);
  bool overrideProcessTagInfoData(
      CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType& permSelector,
      std::string& postRuleValidationOut);
  bool postRuleValidation(const FareMarket& fm,
                          const ReissueSequence& reissueSeq,
                          FareCompInfo::SkippedValidationsSet& svs,
                          bool runForDomestic = false);
  virtual const Itin* getExcItin();
  virtual bool
  checkElectronicTicket(ProcessTagPermutation& permutation, std::string& validationOut);
  bool checkStopoverConnection(ProcessTagPermutation& permutation, bool connectionChanged,
                               bool stopoverChanged, std::string& validationOut) const;
  ProcessTagInfo* findPtiInCache(ProcessTagInfo* ptiDom, ProcessTagInfo* ptiIntl)
  {
    PTICache::iterator i = _ptiCache.find(std::make_pair(ptiDom, ptiIntl));
    return i == _ptiCache.end() ? nullptr : (*i).second;
  }

  void addPtiToCache(ProcessTagInfo* ptiDom, ProcessTagInfo* ptiIntl, ProcessTagInfo* newDomPti)
  {
    _ptiCache[std::make_pair(ptiDom, ptiIntl)] = newDomPti;
  }

  void saveFareBytesData(ProcessTagPermutation& permutation);
  void matchExcToNewFareMarket(const ExcItin* excItin);

  FareMarket::FareRetrievalFlags getRetrievalFlag(const FareApplication& fareAppl);
  void saveFlownFcFareAppl(const FareMarket* fareMarket, const FareApplication& fareAppl);

  void setReissueExchangeROEConversionDate();

  static void setHealthCheckConfig();
  void
  checkHealth(const uint32_t& permNumber, const size_t& startVM, size_t startRSS, const size_t& numFC);

  const DateTime& getPreviousDate();

  std::tuple<bool, bool>
  stopoversOrConnectionChanged(const FarePath& fp,
                               const std::vector<TravelSeg*>& curTvlSegs) const;

  PTICache _ptiCache;
  std::vector<uint32_t> _matchedExcFCs;
  std::vector<uint32_t> _unmatchedExcFCs;

  static const uint32_t HEALTH_CHECK_INTERVAL = 5000;
  static uint32_t _maxPermNumber;
  static uint32_t _maxMemoryGrow;
  static uint32_t _maxVM;
  static uint32_t _minAvailMem;
  static volatile bool _healthCheckConfigured;
  void updatePermuationInfoForFareMarkets(const FareApplication& fareAppl,
                                          const FCChangeStatus& changeStatus);

private:
  RexPricingTrx& _trx;
  Diag688Collector* _dc;
  DiagCollector* _dc331;
  bool _travelCommenced;

  static boost::mutex _mutex;
  static Logger _logger;
};

} // tse


