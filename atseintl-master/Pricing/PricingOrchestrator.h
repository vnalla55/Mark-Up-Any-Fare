/*---------------------------------------------------------------------------
 *  File:    PricingOrchestrator.h
 *  Created: Dec 19, 2003
 *  Authors: Dave Hobt, Steve Suggs, Mark Kasprowicz, Mohammad Hossan
 *
 *  Change History:
 *    2/10/2004 - SMS - adding hooks for various pricing function calls.
 *
 *  Copyright Sabre 2003
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
/*
 @class PricingOrchestrator Documentation
  The main pricing orchestrator for pricing an itinerary.

1. Build all possible FareMarketPath(s) [Each path is a complete journey]
   and put it in FareMarketPathMatrix.

2. for each FareMarketPath, build all possible PUPath and put it in PUPathMatrix.
   Generating PU require following checks
    a. GEO check
    b. Multi Airport
    c. Mileage
    d. CT provision
    e. OJ/RT definition
    f. Chilean Domestic OJ

3. Build FarePath and check Combinability in a loop:
  a. Build FarePath for the PUPath and put it in the PriorityQueue.
     Consider the following while building FarePath:
     i.  PaxType
     ii. FareTag  (Tag:1, 2, 3)
     iii. FareType (NL/SP)
     iv.  FareDirection (Inbound/OutBound)
     v. if RT PU has asymmetric HIP, change to CT PU

  b. Get the FarePath from the PQ and check combinability if it was not processed before.
     if the FarePath was processed before, break the loop.  if combinability has failed,
     continue the loop by expanding the PQ if possible. Otherwise combinability do:
     i. Final rule validation
     ii.  do the Minimum fare check
  c. if there was a plus-up from Final rule validation or minimum fare check,
     put the FarePath back to the PQ indicating it as already been processed and
     continue the loop by expanding the PQ if possible. if there was no plus-up, break the loop.


*--------------------------------------------------------------------------*/
#pragma once

#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/Diag990Collector.h"
#include "Fares/BitmapOpOrderer.h"
#include "Pricing/FactoriesConfig.h"
#include "Pricing/MIPFamilyLogicUtils.h"
#include "Pricing/PQDiversifier.h"
#include "Pricing/PUPathMatrix.h"
#include "Util/FlatFwd.h"

#include <map>
#include <queue>
#include <string>
#include <vector>

namespace tse
{
class FareMarket;

class AltPricingTrx;
class ExchangePricingTrx;
class FPPQItem;
class FareMarket;
class FarePathFactory;
class GroupFarePath;
class GroupFarePathFactory;
class Itin;
class ItinThreadTask;
class Logger;
class MergedFareMarket;
class MetricsTrx;
class NoPNRPricingTrx;
class PaxFarePathFactory;
class PaxFarePathFactoryCreator;
class PricingTrx;
class PricingUnitFactory;
class PricingUnitFactoryBucket;
class RefundPricingTrx;
class RexExchangeTrx;
class RexPricingTrx;
class RexShoppingTrx;
class ShoppingPQ;
class ShoppingTrx;
class StructuredRuleTrx;
class TseServer;
class TseRunnableExecutor;
class ConfigMan;
namespace similaritin
{
struct Context;
}

class PricingOrchestrator
{
  friend class PricingOrchestratorTest;

public:
  using CxrPrecalculatedTaxesArray = std::array<CxrPrecalculatedTaxes, MAX_PAX_COUNT>;
  using FareMarketTaxesCopyMap = FlatMap<FareMarket*, CxrPrecalculatedTaxesArray>;

  /*IS definitions*/
  using SopIdVecI = SopIdVec::iterator;
  using SopIdVecIC = SopIdVec::const_iterator;

  using FamilySopsVec = std::vector<SopIdVec>;
  using FamilySopsVecI = std::vector<SopIdVec>::iterator;
  using FamilySopsVecIC = std::vector<SopIdVec>::const_iterator;

  using FamiliesChildsVec = std::vector<FamilySopsVec>;
  using FamiliesChildsVecI = std::vector<FamilySopsVec>::iterator;
  using FamiliesChildsVecIC = std::vector<FamilySopsVec>::const_iterator;
  /*~IS definitions*/

  using PUPathMatrixMap = std::map<DatePair, PUPathMatrixVec>;
  using PUPathMatrixMapI = std::map<DatePair, PUPathMatrixVec>::iterator;
  using PUPathMatrixMapIC = std::map<DatePair, PUPathMatrixVec>::const_iterator;

  using PUPathMatrixVecTable = std::vector<PUPathMatrixMap>;
  using PUPathMatrixVecTableI = std::vector<PUPathMatrixMap>::iterator;
  using PUPathMatrixVecTableIC = std::vector<PUPathMatrixMap>::const_iterator;

  using ItinVector = std::vector<Itin*>;
  using TaskVector = std::vector<ItinThreadTask>;

  PricingOrchestrator(TseServer& server);
  virtual ~PricingOrchestrator() = default;

  bool process(MetricsTrx& trx);
  bool process(PricingTrx& trx);
  bool process(ShoppingTrx& trx);
  bool process(AltPricingTrx& trx);
  bool process(NoPNRPricingTrx& trx);
  bool process(RexPricingTrx& trx);
  bool process(ExchangePricingTrx& trx);
  bool process(RexShoppingTrx& trx);
  bool process(RefundPricingTrx& trx);
  bool process(RexExchangeTrx& trx);
  bool process(TktFeesPricingTrx& trx);
  bool process(StructuredRuleTrx& trx);

  void doQueuePostprocessing(ShoppingTrx& trx,
                             ShoppingTrx::ShoppingPQVector& shoppingPQVector,
                             bool& mustHurry,
                             ConfigMan& _config,
                             std::deque<Itin>& journeyItins,
                             bool interlineQueueHasNoSolution);

  virtual uint32_t getActiveThreads();

  // PricingUnitFactory Init-Thread Input
  struct PUFInitThreadInput : public TseCallableTrxTask
  {
    PUFInitThreadInput() { desc("PU FACTORY INIT TASK"); }

    void performTask() override;

    DiagCollector* diag = nullptr;
    PricingUnitFactory* puFactory = nullptr;
    ErrorResponseException::ErrorResponseCode errResponseCode =
        ErrorResponseException::ErrorResponseCode::NO_ERROR;
    std::string errResponseMsg;
  };

  // FarePathFactory Init-Thread Input
  struct PaxFPFInitThreadInput : public TseCallableTrxTask
  {
    PaxFPFInitThreadInput() { desc("PAX FP FACTORY INIT TASK"); }
    void performTask() override;

    DiagCollector* diag = nullptr;
    PaxFarePathFactory* paxFPFactory = nullptr;
    PricingUnitFactoryBucket* puFactoryBucketVect = nullptr;
    ErrorResponseException::ErrorResponseCode errResponseCode =
        ErrorResponseException::ErrorResponseCode::NO_ERROR;
    std::string errResponseMsg;
  };

  struct GetRec2Cat10Task : public TseCallableTrxTask
  {
    GetRec2Cat10Task() { desc("GET REC2-CAT10"); }

    void performTask() override;

    MergedFareMarket* _mfm = nullptr;
  };

  void checkDelayExpansion(PricingTrx& trx);

  bool createPricingUnitFactoryBucket(PricingTrx& trx,
                                      std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);
  bool initPricingUnitFactory(PricingTrx& trx,
                              std::vector<PricingUnitFactoryBucket*>& pufBuckVect,
                              DiagCollector& diag,
                              bool onlyOwFares = false,
                              bool onlyNonStops = false);

  void initPricingUnitFactory(PricingTrx& trx,
                              PricingUnitFactory* puf,
                              DataHandle& dataHandle,
                              TseRunnableExecutor& taskExecutor,
                              std::vector<PUFInitThreadInput*>& thrInputVect,
                              bool onlyOwFares = false,
                              bool onlyNonStops = false);

  bool createPaxFarePathFactory(PricingTrx& trx,
                                PUPathMatrix& puMatrix,
                                std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
                                std::vector<PaxFarePathFactory*>& pfpFactoryBucket,
                                const PaxFarePathFactoryCreator& creator);

  bool initGroupFarePathFactory(PricingTrx& trx,
                                GroupFarePathFactory& groupFarePathFactory,
                                std::vector<PUPathMatrix*>& puMatrixVect,
                                std::vector<PaxFarePathFactory*>& pfpfBucket,
                                std::vector<PricingUnitFactoryBucket*>& pufbv,
                                DiagCollector& diag);

  bool createPaxFarePathFactory(PricingTrx& trx,
                                std::vector<PUPathMatrix*>& puMatrixVect,
                                std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
                                std::vector<PaxFarePathFactory*>& pfpFactoryBucket,
                                const PaxFarePathFactoryCreator& creator);

  bool initPaxFarePathFactory(PricingTrx& trx,
                              std::vector<PaxFarePathFactory*>& pfpFactoryBucket,
                              DiagCollector& diag,
                              bool eoeCombinabilityEnabled = true);

  void static getRec2Cat10(PricingTrx& trx, std::vector<MergedFareMarket*>& mergedFareMarketVect);

  void processOwPricingRTTaxProcess(PricingTrx& trx);

  static MoneyAmount getTax(ShoppingTrx& trx, Itin& curItin, FarePath* farePath);

  BitmapOpOrderer& getBitmapOpOrderer() { return (_bitmapOpOrderer); }

  const double altDateCutOffAmount() const { return _altDateCutOffAmount; } // IS
  const double altDateCutOffSnowmanAmount() const { return _altDateCutOffSnowmanAmount; } // IS
  const double altDateOptionRemoveFactor() const { return _altDateOptionRemoveFactor; } // IS
  uint16_t getShortCktTimeOut(PricingTrx& trx, uint16_t defaultShortCktTimeOut);
  void getPricingShortCktTimes(PricingTrx& trx,
                               time_t& shortCktOnPushBackTime,
                               time_t& shutDownFPFsTime) const;

  const FactoriesConfig& getFactoriesConfig() const { return _factoriesConfig; }

  const uint16_t getAltDateItinPriceJumpFactor() const { return _altDateItinPriceJumpFactor; }

  const uint16_t getAltDateCutOffNucThreshold() const { return _altDateCutOffNucThreshold; }

protected:
  ConfigMan& _config;
  const FactoriesConfig _factoriesConfig;

  PQDiversifier _pqDiversifier;

  uint16_t _shortCktTimeOut = 60;
  uint16_t _minShortCktTimeOutShopping = 3;
  uint16_t _percentageShortCktTimeOutShopping = 25; // percentage of D70 value
  uint16_t _percentageShortCktTimeOutShoppingMultiPax = 0;

  uint16_t _applyDomShoppingCktTimeoutValues = 0;
  uint16_t _percentageShortCktTimeoutDomShopping = 6;
  uint16_t _minShortCktTimeoutDomShopping = 0;

  bool _delayPUValidationMIP = false;
  uint16_t _puScopeValMaxNum = 500; // num of pu to validate in pu factory
  uint16_t _maxSPCTFareCompCount = 2; // max num of FC allowed for SP-CT
  uint16_t _maxSPCTFareCompCount_1B1F = 2; // max num of FC allowed for SP-CT
  // for transactions from Abacus and Infini
  uint32_t _maxSPCTPUCount = 2000; // max num of SP-CT-PU want to try

  uint32_t _additionalFarePathCount = 0; // for WPA (alt pricing) transactions,
  // create this many extra fare paths when
  // fare calc config does not allow dups.
  int32_t _maxSearchNextLevelFarePath = -1; // max num of fare path build for Shopping IS Process

  int32_t _maxSearchNextLevelFarePathAlt = 100000; // max num of fare path build
  // for Shopping IS-Alternate

  // Date
  int32_t _maxSearchNextLevelFarePathComplex = 3000; // max num of fare path build for
  // Shopping IS-complex

  // trip

  uint16_t _altDateItinPriceJumpFactor = 7; // stop altDate-MIP if price jumps, e.g. is 10 time
  // of the cheapest Itin

  uint16_t _altDateCutOffNucThreshold; // cutoff threshold, e.g. NUC 100. Do not cutoff even if it
                                       // reaches
  // 10 times of the cheapest one when the amount is already very low

  int32_t _hundredsOptionsReqAdjustPercent = 100; // IS
  int32_t _hundredsOptionsResAdjustPercent = 100; // IS

  uint16_t _nextGFPAttemptsNumber = 1;

  double _altDateCutOffAmount; // IS
  double _altDateCutOffSnowmanAmount; // IS
  double _altDateOptionRemoveFactor = 1.5; // by default we return from IS
  // (factor set to  > than 1.0) as
  // many solution as we get from the processing.
  // This factor allow us to limit the number of solutions returned per date to
  // the number specified by Q0S (factor set to 1.0) or less if we set this factor to
  // less than 1.0 value
  //

  int32_t _numTaxCallForMipAltDate = 2; // max num of calling tax for each date pair for Mip
  // Alteanate-date request
  bool _taxPerFareBreak = false; // to call YQ/YR tax and callculate tax amount per fare
  int32_t _maxDirectFlightOnlySolution = 1; // max number of direct flight
  // to create for each carrier
  // for flt only sol. - IS

private:
  static Logger _logger;
  BitmapOpOrderer _bitmapOpOrderer; // IS

  TseServer& _server;
  bool _isForcedCorpFares = false;

  void prepareBaggageInPq(PricingTrx& trx);
  void calculateBaggageLowerBounds(PricingTrx& trx, const Itin& itin);

  void buildFareMarketPathMatrix(PricingTrx& trx,
                                 Itin* itin,
                                 std::vector<MergedFareMarket*>& mergedFareMarketVect,
                                 const BrandCode& brandCode = "");

  void startPUFTimers(std::vector<PricingUnitFactoryBucket*>& pufbv);

  bool buildPUPathMatrix(PricingTrx& trx,
                         std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
                         PUPathMatrixMap& puPathMatrixMap,
                         uint16_t brandIndex);

  bool buildPUPathMatrixForItin(PricingTrx& trx,
                                Itin& itin,
                                std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
                                PUPathMatrixMap& puPathMatrixMap,
                                uint16_t pricingOptionSpaceIndex);

  bool priceCheapestItin(PricingTrx& trx,
                         PUPathMatrixVecTable& puPathMatrixVecTable,
                         std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);

  bool priceAllItin(PricingTrx& trx,
                    PUPathMatrixVecTable& puPathMatrixVectTable,
                    std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);

  bool priceAllTNBrandedItin(PricingTrx& trx,
                             std::vector<PUPathMatrixVecTable>& tnShopPuPathMatrixVectTables,
                             std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);

  bool priceItin(PricingTrx& trx,
                 PUPathMatrix& puMatrix,
                 std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);

  bool priceItin(PricingTrx& trx,
                 std::vector<PUPathMatrix*>& puPathMatrixVect,
                 std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);

  void detailedFailureMsg(PricingTrx& trx, Itin& itin);

  void detailedFailureMsgAfterPricing(PricingTrx& trx, Itin& itin);

  bool itinHasAtLeastOneEmptyFMForOvrCxr(PricingTrx& trx, const Itin& itin);

  void getFMCOSBasedOnAvailBreak(PricingTrx& trx, Itin* itin);

  void displayPriceItinDiags(
      PricingTrx& trx, DiagCollector& diag, bool first, PUPathMatrix*, bool ret = true);

  bool returnOrThrow(PricingTrx& trx, TaskVector& tasks);
  bool returnOrThrow(PricingTrx& trx, bool itinPassed);

  bool getGroupFarePath(PricingTrx& trx,
                        GroupFarePathFactory& groupFarePathFactory,
                        DiagCollector& diag);

  bool processFinalGroupFarePath(PricingTrx& trx,
                                 GroupFarePath& groupFPath,
                                 GroupFarePathFactory& groupFarePathFactory,
                                 DiagCollector& diag);

  void preparePricingProcessForAxess(FPPQItem& fppqItem,
                                     GroupFarePathFactory& groupFarePathFactory,
                                     const uint32_t posPax,
                                     DiagCollector& diag);

  void copyFarePathToItin(PricingTrx& trx, GroupFarePath& groupFPath, Itin& itin);

  void copyNetRemitFarePathToNetRemitItin(GroupFarePath& groupFPath, Itin& itin);

  bool priceItin(AltPricingTrx& trx,
                 PUPathMatrix& puMatrix,
                 std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);

  void createItinTaxShopping(ShoppingTrx& trx, std::vector<Itin*>& filteredItin);

  bool canUseMultiThreadPQ(ShoppingTrx& trx);
  void processOnlineShoppingQueue(ShoppingTrx& trx,
                                  std::deque<Itin>& journeyItins,
                                  ShoppingTrx::ShoppingPQVector& tasks);

  void processInterlineShoppingQueue(ShoppingTrx& trx,
                                     std::deque<Itin>& journeyItins,
                                     bool& interlineQueueHasNoSolution,
                                     ShoppingTrx::ShoppingPQVector& tasks);

  void processOwFaresShoppingQueue(ShoppingTrx& trx);

  void processNonStopShoppingQueue(ShoppingTrx& trx,
                                   std::deque<Itin>& journeyItins,
                                   ShoppingTrx::ShoppingPQVector& tasks);

  void addTaxAmount(ShoppingTrx& trx);

  void validateSurchargeForAltDate(PricingTrx& trx);
  void validateSurchargeForFarePath(PricingTrx& trx,
                                    Itin& itin,
                                    FarePath& farePath,
                                    PricingUnitRuleController& ruleController);
  void filterPricingErrors(NoPNRPricingTrx& trx);

  void saveNUCBaseFareAmount(ShoppingTrx& trx);

  void displayBrandCombinationInfo(PricingTrx& trx, DiagCollector& diag, const Itin& itin);

protected:
  typedef std::map<TravelSeg*, unsigned int> TvlSegToItinIdxMap;

  void getValidBrands(PricingTrx& trx);
  uint16_t getBrandSize(PricingTrx& trx);
  void moveEstimatedSolutionsToFlightMatrix(ShoppingTrx& trx);
  void fillMissingAltDatePairsWithFOS(ShoppingTrx& trx);
  void removeExcessiveOptions(ShoppingTrx& trx, ShoppingTrx::FlightMatrix& fltMatrix);
  bool checkSopsCnxTimeMoreThan4hExist(const ShoppingTrx& trx, const SopIdVec& sops) const;
  bool checkDomesticCnxTimeIntegrity(const ShoppingTrx& trx,
                                     const std::vector<int>& key,
                                     FamilySopsVec& childs);
  void createFamily(ShoppingTrx& trx,
                    FamilySopsVec& itinElements,
                    ShoppingTrx::FlightMatrix& family,
                    ShoppingTrx::EstimateMatrix& estimates);
  void splitFamilies(ShoppingTrx& trx);
  void promoteChildItins(PricingTrx& trx);

  virtual bool initFpFactory(GroupFarePathFactory& factory) const;
  bool displayDiagnostics(PricingTrx& trx,
                          DiagnosticTypes& diagType,
                          std::vector<PricingUnitFactoryBucket*>& pufbv);

  void displayDiagnostics606(DiagnosticTypes& diagType, DCFactory* factory, DiagCollector& diag);

public:
  bool priceItinTask(PricingTrx& trx,
                     Itin& itin,
                     PUPathMatrix& puMatrix,
                     std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);

  TseServer& server() { return _server; }

  bool priceADItinTask(PricingTrx& trx,
                       PUPathMatrixVec& puMatrix,
                       std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);

  void completePUFactoryInit(std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect);
private:
  void
  cloneFmForEstimatedTaxes(PricingTrx& trx, Itin* itin, FareMarketTaxesCopyMap& previousAmounts);
  FareMarket* cloneFareMarket(PricingTrx& trx, const FareMarket* fm);

  void checkWPCYYentry(PricingTrx& trx, std::string& markets, const Itin& itin);
  std::string getWPCxxErrMsg(PricingTrx& trx, const Itin& itin);

  bool yyFareValid(FareMarket& fmkt);

  PricingOrchestrator(const PricingOrchestrator& rhs);
  PricingOrchestrator& operator=(const PricingOrchestrator& rhs);

  /**
   * print option to diagnostics 910 output
   */
  void printNumOption(ShoppingTrx& trx,
                      const char* const stage,
                      const ShoppingTrx::ShoppingPQVector& shoppingPQVector);
  void printNumOption(ShoppingTrx& trx, const std::string& str, const int& opt);
  void printAsDia910(ShoppingTrx& trx, const ShoppingTrx::PQDiversifierResult& pqResult);
  /**
   * print something to diagnostics 910 output
   */
  void printAsDia910(ShoppingTrx& trx, const std::string& msg);

  void generateAltDatePairSolutionWithNoFares(ShoppingTrx&,
                                              const DatePair& dp,
                                              GroupFarePath& gfp,
                                              uint32_t count);

  void cloneItinObjects(PricingTrx& trx);
  void getDatePair(PricingTrx& trx,
                   Itin* itin,
                   DatePair& datePair,
                   DatePair& outbDatePair,
                   DatePair& inbDatePair,
                   std::map<DatePair, int>& datePairASE,
                   bool& outbAdded,
                   bool& inbAdded);

  void setupCabinForDummyFareUsage(PricingTrx& trx) const;
  void resetSurchargeForDummyFareUsage(std::vector<Itin*>& itins) const;
  void removeDuplicatedSolutions(ShoppingTrx::FlightMatrix& flightMatrix,
                                 ShoppingTrx::EstimateMatrix& estimateMatrix);
  void precalculateTaxes(PricingTrx& trx, Itin& itin);

  void checkFlexFaresForceCorpFares(PricingTrx& trx, const uint32_t& brandIndex);
  void checkFlexFareGroupXOFareStatus(PricingTrx& trx, const flexFares::GroupId groupId);

  void throughAvailabilitySetup(PricingTrx& trx, std::vector<PUPathMatrix*>& puMatrixVect);

  void lazyBrandAllPaxFares(PricingTrx& trx, DiagCollector& diag);
  void lazyBrandAllPaxFaresInItin(Itin& itin, std::set<PaxTypeFare*>& faresToBrand);

  bool hasHardPassFareOnEachLeg(const PricingTrx& trx, const FPPQItem& fppQItem) const;

  void
  priceSimilarItins(similaritin::Context&, GroupFarePathFactory&, const std::vector<FPPQItem*>&);
  void priceSimilarItinsWithSavedFPPQItems(similaritin::Context&, GroupFarePathFactory& gfpf);
};
} // tse

