//-------------------------------------------------------------------
// File:    PaxFarePathFactory.h
// Created: FEB 2006
// Authors: Mohammad Hossan
//
//  Copyright Sabre 2006
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

#pragma once

#include "Common/Assert.h"
#include "Common/Gauss.h"
#include "Common/TseDateTimeTypes.h"
#include "Pricing/FactoriesConfig.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/PaxFarePathFactoryBase.h"
#include "Pricing/PUPathMatrix.h"
#include "Pricing/PUPQItem.h"
#include "Rules/AccompaniedTravel.h"

#include <boost/heap/priority_queue.hpp>

#include <queue>
#include <vector>

namespace tse
{
class CurrencyConversionFacade;
class DiagCollector;
class FareMarket;
class FarePath;
class PaxFarePathFactory;
class PricingOrchestrator;
class PricingUnit;
class PricingTrx;
class PU;
class PUPath;
class PricingUnitFactory;
class PricingUnitFactoryBucket;

class PaxFarePathFactoryCreator
{
public:
  virtual ~PaxFarePathFactoryCreator() {}
  virtual PaxFarePathFactory*
  create(const FactoriesConfig& factoriesConfig, const PricingTrx& trx) const;
};

class PaxFarePathFactory : public PaxFarePathFactoryBase
{
public:
  typedef boost::heap::priority_queue<FarePathFactory*,
    boost::heap::compare<FarePathFactory::Greater<FPPQItem::GreaterFare> > > FarePathFactoryPQ;
  typedef boost::heap::priority_queue<FarePathFactory*,
    boost::heap::compare<FarePathFactory::Greater<FPPQItem::LowerFare> > > FarePathFactoryASEPQ;

  PaxFarePathFactory(const FactoriesConfig& factoriesConfig)
    : PaxFarePathFactoryBase(factoriesConfig)
  {
  }

  PaxFarePathFactory(const PaxFarePathFactory&) = delete;
  PaxFarePathFactory& operator=(const PaxFarePathFactory&) = delete;

  // Needs to be overridden  by ShoppingPaxFarePathFactory so that it creates (with the Creator)
  // right FarePathFactory (FarePathFactory or ShoppingFarePathFactory) underneath
  virtual bool initPaxFarePathFactory(DiagCollector& diag) override;

  virtual FPPQItem* getFPPQItem(uint32_t idx, DiagCollector& diag) override;

  // Get first 0-amount FP for infant that passes CAT13 validation
  virtual FPPQItem* getFirstValidZeroAmountFPPQItem(uint32_t idx,
                                            const FPPQItem& primaryFPPQItem,
                                            DiagCollector& diag,
                                            uint32_t* newIdx) override;

  virtual FPPQItem* getSameFareBreakFPPQItem(const FPPQItem* primaryFPPQItem, DiagCollector& diag) override;

  virtual FPPQItem* getAlreadyBuiltFPPQItem(uint32_t idx) override
  {
    return (idx < _fpCount) ? _validFPPQItemVect[idx] : nullptr;
  }

  virtual void removeFPF(DiagCollector* diag,
                         const FareMarketPath* fareMarketPathID,
                         const DatePair* datePair = nullptr,
                         bool nonThruPricing = false) override;

  virtual bool checkFailedFPIS(FPPQItem* fppqItem, std::set<DatePair>& datePairSet, bool first) override;

  // TODO move this method to FarePathValidator
  bool isValidForIntegrated(const FarePath* farePath);

  // This clear is needed for 2nd call for NetRemitPricing
  virtual void clear() override
  {
    _fpCount = 0;
    _fpCombTried = 0;
    _validFPPQItemVect.clear();
    while (!pqEmpty())
    {
      pqPop();
    }
  }

  void saveFailedFPIS(FPPQItem* fppqItem);
  void saveFailedFPIS(FPPQItem* fppqItem, const DatePair& datePair);

  const PricingOrchestrator* pricingOrchestrator() const { return _po; }
  PricingOrchestrator*& pricingOrchestrator() { return _po; }

  const int32_t& shortCktTimeOut() const { return _shortCktTimeOut; }
  int32_t& shortCktTimeOut() { return _shortCktTimeOut; }

  const time_t& shortCktShutdownFPFsTime() const { return _shortCktShutdownFPFsTime; }
  time_t& shortCktShutdownFPFsTime() { return _shortCktShutdownFPFsTime; }

  const bool& allowDuplicateTotals() const { return _allowDuplicateTotals; }
  bool& allowDuplicateTotals() { return _allowDuplicateTotals; }

  const uint32_t& reqValidFPCount() const { return _reqValidFPCount; }
  uint32_t& reqValidFPCount() { return _reqValidFPCount; }

  const int32_t& maxSearchNextLevelFarePath() const { return _maxSearchNextLevelFarePath; }
  int32_t& maxSearchNextLevelFarePath() { return _maxSearchNextLevelFarePath; }

  uint32_t fpCount() const { return _fpCount; }

protected:
  bool processFareMarketPaths();
  void updateGaussForDelayedExpansion();
  void startTimer() { _startTime = time(nullptr); }
  uint32_t fpCountDiagParam();
  void releaseFPPQItem(FarePathFactory& fpf, FPPQItem* fppqItem);
  void removeObsoleteFpfs(DiagCollector& diag);
  virtual bool isFarePathValidAfterPlusUps(FPPQItem& fppqItem, DiagCollector& diag) { return true; }
  bool shouldExpandNextFareMarketPath(FareMarketPath* fmp);
  void expandNextFareMarketPath(DiagCollector* diag);
  bool createFarePathFactoriesForDelayXpn(std::vector<FareMarketPath*>& fmps, DiagCollector* diag);
  void initPricingUnitFactory(std::vector<PUPath*>& puPaths, DiagCollector& diag);
  bool getPuFactoryBucketForFmp(std::vector<PUPath*>& puPaths, PricingUnitFactoryBucket* fmpPufb);
  void setExternalFmpLowerBoundAmount(FareMarketPath* currentFmp);
  bool isNextFmpSameAmount(FareMarketPath* currentFmp);
  bool sameOrLowerAmountFmpExists(const FarePathFactory* fpf);
  bool updateNextLowerBoundFPAmount(MoneyAmount& nextLowerBoundFPAmount);
  bool createFarePathFactories(const FarePathFactoryCreator& creator);
  void
  createFarePathFactories(const FarePathFactoryCreator& creator, std::vector<PUPath*>& puPaths);
  bool addFarePathFactoriesToPQ(DiagCollector& diag);
  bool buildNextLevelFarePath(DiagCollector& diag);
  void checkUniqueFarePathTotals(FarePath* farePath,
                                 CurrencyConversionFacade& ccFacade,
                                 uint32_t& addForDups,
                                 bool markDups = false);
  bool shutdownFPFactory(FarePathFactory& fpFactory);
  bool checkAltDates(FarePathFactory*& fpFactory);
  void expandFareMarketPath(FareMarketPath* fareMarketPath, DiagCollector* diag);
  FarePathFactory* getMatchingFPF(const FPPQItem& primaryFPPQItem);
  void separateDiffFareBreakFPF(const FareMarketPath* fareMarketPath,
                                std::vector<FarePathFactory*>& diffFareBreakFPF);
  void addFPF(std::vector<FarePathFactory*>& diffFareBreakFPF);
  bool passAccompanidRestriction(const FPPQItem& primaryFPPQItem, const FPPQItem& fppqItem);
  bool checkISICode(const FPPQItem& primaryFPPQItem, const FPPQItem& fppqItem);
  bool startMultiPaxShortCkt();
  void buildFarePathsAmounts(CurrencyConversionFacade& ccFacade);
  bool pqEmpty() { return _farePathFactoryPQ.empty(); }
  size_t pqSize() { return _farePathFactoryPQ.size(); }
  void pqPush(FarePathFactory* fpf)
  {
    TSE_ASSERT(fpf->lowerBoundFPAmount() != -1);
    _farePathFactoryPQ.push(fpf);
  }
  void pqPop() { _farePathFactoryPQ.pop(); }
  FarePathFactory* pqTop() { return _farePathFactoryPQ.top(); }

  static const uint16_t SHORT_CKT_COMB_COUNT = 200;
  tse::Gauss<double> _gauss;
  FarePathFactoryPQ _farePathFactoryPQ;
  std::vector<FPPQItem*> _validFPPQItemVect;
  std::set<MoneyAmount> _uniqueFarePathTotals;
  AccompaniedTravel _accompaniedTravel;
  std::map<FPPQItem*, std::set<DatePair> > _failedFarePathIS;
  std::vector<FareMarketPath*> _allFareMarketPaths;
  std::set<DatePair> _doneDatePairs;
  std::set<FareMarketPath*> expandedSameFareBreakFmps;
  MoneyAmount _externalFmpLowerBoundAmount = -1;
  PricingOrchestrator* _po = nullptr;
  time_t _startTime = time(nullptr);
  time_t _shortCktShutdownFPFsTime = 0;
  uint32_t _fpCount = 0;
  uint32_t _reqValidFPCount = 1; // desired maximum number of valid fare paths
  int32_t _shortCktTimeOut = 0; // after 60 sec stop building large CT/OJ
  int32_t _reqDiagFPCount = 0; // for diagnostics display
  int32_t _maxSearchNextLevelFarePath = -1;
  int16_t _fmpXPoint = 0;
  bool _allowDuplicateTotals = true; // create multiple FPs with same total amt?
  bool _maxNbrCombMsgSet = false;
  bool _altDateISCutOffReach = false;
};
} // tse namespace
