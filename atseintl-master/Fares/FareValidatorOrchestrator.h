//-------------------------------------------------------------------
//
//  File:        FareValidatorOrchestrator.h
//  Created:     Jan 20, 2004
//  Authors:     Abu Islam, Mark Kasprowicz, Bruce Melberg, Vadim Nikushin
//
//  Description: Fare Validator Orchestrator
//
//  Updates:
//          01/20/04 - Abu Islam, Mark Kasprowicz - file created.
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
//-------------------------------------------------------------------

#pragma once

#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseConsts.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/ShpBitValidationCollector.h"
#include "Fares/BitmapOpOrderer.h"
#include "Fares/CarrierFareController.h"
#include "Fares/FareOrchestrator.h"
#include "Fares/FareValidatorOrchestratorShopping.h"
#include "Routing/RoutingInfo.h"
#include "Rules/FareMarketRuleController.h"

#include <functional>

namespace tse
{
class AltPricingTrx;
class ExchangePricingTrx;
class FareMarket;
class FareValidatorOrchestrator;
class FareValidatorOrchestratorShopping;
class MetricsTrx;
class NoPNRPricingTrx;
class RefundPricingTrx;
class RepricingTrx;
class RexExchangeTrx;
class RexPricingTrx;
class StructuredRuleTrx;

class ProcessCarrierTask : public TseCallableTrxTask
{
public:
  ProcessCarrierTask(FareValidatorOrchestrator& fvo,
                     ShoppingTrx& shoppingTrx,
                     const ItinIndex::Key& cxrKey,
                     uint32_t legId,
                     ShoppingTrx::Leg& curLeg,
                     Itin* journeyItin,
                     ShoppingTrx::AltDatePairs& altDatesMap,
                     FareMarketRuleController& ruleController,
                     FareMarketRuleController& cat4ruleController,
                     FareMarket* fM,
                     std::vector<std::vector<BCETuning> >& shoppingBCETuningData, // DEPRECATED
                     ShpBitValidationCollector::FMValidationSharedData* sharedData,
                     const uint32_t numberOfFaresToProcess = DELAY_VALIDATION_OFF);

  void performTask() override;

private:
  FareValidatorOrchestrator* _fvo = nullptr;
  ShoppingTrx* _trx = nullptr;
  const ItinIndex::Key* _cxrKey = nullptr;
  uint32_t _legId = 0;
  ShoppingTrx::Leg* _curLeg = nullptr;
  Itin* _journeyItin = nullptr;
  ShoppingTrx::AltDatePairs _altDatesMap;
  FareMarketRuleController* _ruleController = nullptr;
  FareMarketRuleController* _cat4ruleController = nullptr;
  FareMarket* _fM = nullptr;
  std::vector<std::vector<BCETuning>>* _shoppingBCETuningData = nullptr; // DEPRECATED
  ShpBitValidationCollector::FMValidationSharedData* _sharedData = nullptr;
  const uint32_t _numberOfFaresToProcess = DELAY_VALIDATION_OFF;
};

using CarrierTasksForFareMarket = std::map<FareMarket*, std::list<ProcessCarrierTask>>;

class FareValidatorOrchestrator : public FareOrchestrator
{
public:
  struct FMBackup
  {
    std::vector<TravelSeg*> fMBackupSegs;
    TravelSeg* primarySector = nullptr;
    GlobalDirection globalDirection = GlobalDirection::ZZ;
  };

  FareValidatorOrchestrator(const std::string& name, TseServer& server);

  virtual bool process(MetricsTrx& trx) override;
  virtual bool process(PricingTrx& trx) override;
  virtual bool process(ShoppingTrx& trx) override;
  virtual bool process(FareDisplayTrx& trx) override;
  virtual bool process(AltPricingTrx& trx) override;
  virtual bool process(NoPNRPricingTrx& trx) override;
  virtual bool process(RepricingTrx& trx) override;
  virtual bool process(ExchangePricingTrx& trx) override;
  virtual bool process(RexPricingTrx& trx) override;
  virtual bool process(RefundPricingTrx& trx) override;
  virtual bool process(FlightFinderTrx& trx) override;
  virtual bool process(RexExchangeTrx& trx) override;
  virtual bool process(StructuredRuleTrx& trx) override;

  static void prepareJourneyItin(Itin* journeyItin,
                                 ShoppingTrx::AltDatePairs* altDatesMap,
                                 const uint32_t& beginLeg,
                                 uint32_t& endLeg);

  static void prepareShoppingValidation(ShoppingTrx& trx,
                                        Itin* journeyItin,
                                        const ItinIndex::ItinCell& itinCell,
                                        FareMarket* fareMarket,
                                        FMBackup& fareMarketBackup,
                                        const uint32_t& beginLeg,
                                        uint32_t& endLeg);

  static void cleanupAfterShoppingValidation(ShoppingTrx& trx,
                                             Itin* journeyItin,
                                             FareMarket* fareMarket,
                                             FMBackup& fareMarketBackup,
                                             const uint32_t beginLeg,
                                             const uint32_t endLeg);

  // Shopping process() helper methods
  //
  void processCarrier(ShoppingTrx& trx,
                      const ItinIndex::Key& cxrKey,
                      uint32_t legId,
                      ShoppingTrx::Leg& curLeg,
                      Itin* journeyItin,
                      ShoppingTrx::AltDatePairs& altDatesMap,
                      FareMarketRuleController& ruleController,
                      FareMarketRuleController& cat4ruleController,
                      FareMarket* fM,
                      std::vector<std::vector<BCETuning> >& shoppingBCETuningData, // DEPRECATED
                      ShpBitValidationCollector::FMValidationSharedData* sharedData,
                      const uint32_t numberOfFaresToProcess = DELAY_VALIDATION_OFF);

  /** Compute limit of iterations for \ref processCarrier. */
  size_t computeIterationsLimit(const std::vector<PaxTypeFare*>::iterator& faresBegin,
                                const std::vector<PaxTypeFare*>::iterator& faresEnd,
                                bool isStopOverLeg,
                                tse::ItinIndex& sIG,
                                tse::ShoppingTrx& trx,
                                uint32_t legId,
                                int cxrKey);

  static void performBitmapCatValidations(PaxTypeFare* curFare,
                                          FareMarket* fM,
                                          FareMarketRuleController& ruleController,
                                          const uint32_t& bitIndex,
                                          PricingTrx& pricingTrx,
                                          Itin* journeyItin,
                                          ItinIndex::ItinCellInfo& curItinCellInfo);

  static void performQualCat4Validations(PaxTypeFare* curFare,
                                         FareMarketRuleController& cat4RuleController,
                                         const uint32_t& bitIndex,
                                         PricingTrx& pricingTrx,
                                         Itin* journeyItin);

  void
  setFareStatus(ShoppingTrx& trx, std::vector<PaxTypeFare*>& fares, int lastFareProcessed, uint64_t mainDuration = 0);
  static void setFareStatus(ShoppingTrx& trx, std::vector<PaxTypeFare*>& fares);

  virtual bool initialize(int argc, char** argv) override;
  void callback(ShoppingTrx& trx, FareMarket* fM, PaxTypeFare* ptf, ItinIndex::Key* carrierKey = nullptr);
  static void displayValidatingCarriers(PaxTypeFare& paxFare,
                                        DiagCollector& diag);

protected:
  static void primaryValidation(const TseThreadingConst::TaskId taskId, PricingTrx& trx);
  static void bookingCodesValidation(const TseThreadingConst::TaskId taskId, PricingTrx& trx);
  static void mixedClass(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void duplicatedFareCheck(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void printPTFares(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void brandedFareMarkets(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void initPTFvalidatingCxrs(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void initFlexFareFinalCheck(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void checkFlexFareAccountCodes(PricingTrx&, Itin*, PaxTypeFare*, RuleValidationChancelor&);
  static void failFareMarkets(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void fareTypes(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void routing(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void bookingCode(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void rules(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void surcharges(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void sortStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void releaseCPFares(PricingTrx& trx);

  static void noMatchReprocess(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);
  static void noMatchPrepare(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void initFareDisplayInfo(FareDisplayTrx& trx, const FareMarket& fareMarket);

  static void newRouting(const TseThreadingConst::TaskId taskId, PricingTrx& trx);
  static void rulesFD(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void prepareForReprocessing(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  void validateKeepOriginalFare(RexPricingTrx& trx);
  std::string findValidFareBasisForNotShoppedFareMarket(const RexPricingTrx& trx,
                                                        const FareMarket& fm) const;

  void makeFlightList(FlightFinderTrx& fFTrx);
  void makeFlightListGoThroughCombinedBitmap(
      FlightFinderTrx& fFTrx,
      uint16_t legID,
      std::vector<FlightFinderTrx::FlightBitInfo>& combinedFlightBitmap,
      ItinIndex& curCxrIdx,
      const ItinIndex::Key& key);

  void makeFlightListAddSop(
      FlightFinderTrx& fFTrx,
      uint16_t legID,
      DateTime& departureDT,
      uint32_t sopIndex,
      std::vector<PaxTypeFare*>& paxTypeFareVect,
      std::vector<std::vector<FlightFinderTrx::BookingCodeData> >& bkgCodeDataVect);

  void prepareAltDatePairs(FlightFinderTrx& fFTrx,
                           std::map<DatePair, FlightFinderTrx::FlightBitInfo>& dataPairsMap);
  void makeAltDateFlightList(FlightFinderTrx& fFTrx,
                             std::map<DatePair, FlightFinderTrx::FlightBitInfo>& dataPairsMap);
  void makeAltDateFlightListGoThroughCombinedBitmap(
      FlightFinderTrx& fFTrx,
      std::vector<std::pair<DatePair, FlightFinderTrx::FlightBitInfo> >& combinedAltDateStatus);

  void makeAltDateFlightListGoThroughCombinedBitmap(
      FlightFinderTrx& fFTrx, std::map<DatePair, FlightFinderTrx::FlightBitInfo>& dataPairsMap);

  void addAltDateToFlightList(FlightFinderTrx& fFTrx,
                              const DateTime& outboundDepartureDT,
                              const DateTime& inboundDepartureDT,
                              std::vector<PaxTypeFare*>& outboundFareVect,
                              std::vector<PaxTypeFare*>& inboundFareVect);

  void makeAltDateFlightListWithSOP(FlightFinderTrx& fFTrx);

  void makeAltDateFlightListWithSOPGoThroughDurations(
      FlightFinderTrx& fFTrx,
      const uint16_t legId,
      std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo> >&
          combinedDurationFlightBitmap,
      ItinIndex& curCxrIdx,
      const ItinIndex::Key& key,
      std::map<uint64_t, std::map<DateTime, FlightFinderTrx::FlightDataInfo> >*
          tempDurationOutboundMap);

  void makeAltDateFlightListWithSOPGoThroughCombinedBitmap(
      FlightFinderTrx& fFTrx,
      const uint16_t legId,
      uint64_t& duration,
      std::vector<FlightFinderTrx::FlightBitInfo>& combinedFlightBitmap,
      ItinIndex& curCxrIdx,
      const ItinIndex::Key& key,
      std::vector<bool>& thisSOPWasAlreadyAdded,
      std::map<uint64_t, std::map<DateTime, FlightFinderTrx::FlightDataInfo> >*
          tempDurationOutboundMap);

  bool makeAltDateFlightListWithSOPAddSop(
      FlightFinderTrx& fFTrx,
      uint64_t& duration,
      DateTime& departureDT,
      const uint16_t legID,
      uint32_t sopIndex,
      std::map<uint64_t, std::map<DateTime, FlightFinderTrx::FlightDataInfo> >*
          tempDurationOutboundMap,
      std::vector<PaxTypeFare*> paxTypeFareVect,
      std::vector<std::vector<FlightFinderTrx::BookingCodeData> >& bkgCodeDataVect);

  void addOutboundToFlightList(
      FlightFinderTrx& fFTrx,
      const DateTime& outboundDepartureDT,
      const uint32_t sopIndex,
      std::vector<PaxTypeFare*>& paxTypeFareVect,
      std::vector<std::vector<FlightFinderTrx::BookingCodeData> >& bkgCodeDataVect);

  void addInboundToFlightList(
      FlightFinderTrx& fFTrx,
      const DateTime& outboundDepartureDT,
      const DateTime& inboundDepartureDT,
      const uint32_t sopIndex,
      std::vector<PaxTypeFare*>& paxTypeFareVect,
      std::vector<std::vector<FlightFinderTrx::BookingCodeData> >& bkgCodeDataVect);

  void addInboundToFlightListForRT(
      FlightFinderTrx& fFTrx,
      const DateTime& outboundDepartureDT,
      const DateTime& inboundDepartureDT,
      const uint32_t sopIndex,
      std::map<uint64_t, std::map<DateTime, FlightFinderTrx::FlightDataInfo> >*
          tempDurationOutboundMap,
      uint64_t& duration,
      std::vector<PaxTypeFare*>& paxTypeFareVect,
      std::vector<std::vector<FlightFinderTrx::BookingCodeData> >& bkgCodeDataVect);

  void
  combineAltDateStatus(std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDateStatus,
                       PaxTypeFare* curFare,
                       const uint16_t legID);

  void combineAltDateStatusPerLeg(
      std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDatePairsWithStatuses,
      std::vector<std::map<DatePair, FlightFinderTrx::FlightBitInfo> >& combinedAltDateStatusPerLeg,
      std::vector<PaxTypeFare*>& paxTypeFarePassedForRequestedDateVect);

  void combineDurationsFlightBitmapsForEachPaxTypeFare(
      std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo> >&
          combinedDurationFlightBitmap,
      PaxTypeFare* curFare,
      const FlightFinderTrx& fFTrx);

  void combineFlightBitmapsForEachPaxTypeFare(
      std::vector<FlightFinderTrx::FlightBitInfo>& combinedFlightBitmap,
      PaxTypeFare* curFare,
      const std::vector<PaxTypeFare::FlightBit>& flightBitmap,
      const uint64_t* const duration);

  void getBookingCodeVectVect(
      std::vector<std::vector<FlightFinderTrx::BookingCodeData> >& bkgCodeDataVect,
      const std::vector<std::vector<ClassOfService*>*>& thrufareClassOfService,
      const uint32_t bitmapIndex,
      const std::vector<TravelSeg*>& travelSegs,
      const std::vector<PaxTypeFare*> paxTypeFareVect);

  void getBookingCodeVectVect(
      std::vector<std::vector<FlightFinderTrx::BookingCodeData> >& bkgCodeDataVect,
      const std::vector<std::vector<ClassOfService*>*>& thrufareClassOfService,
      const uint32_t bitmapIndex,
      uint64_t& duration,
      const std::vector<TravelSeg*>& travelSegs,
      const std::vector<PaxTypeFare*> paxTypeFareVect);

  void updateDurationFlightBitmapAfterValidation(PaxTypeFare* curFare,
                                                 PaxTypeFare::FlightBitmap& flightBitmap,
                                                 const int bitIndex);

  ShoppingTrx::AltDatePairs*
  getAltDatePairsForDurationWithAtLeastOneValidPair(ShoppingTrx& trx,
                                                    PaxTypeFare* curFare,
                                                    const uint32_t legId,
                                                    const ItinIndex::Key& cxrKey,
                                                    const bool isStopOverLeg,
                                                    ItinIndex& sIG);
  bool durationWithAtLeastOneValidPair(ShoppingTrx& trx,
                                       const uint32_t legId,
                                       const ItinIndex::Key& cxrKey,
                                       const bool isStopOverLeg,
                                       ItinIndex& sIG,
                                       const uint64_t currDuration,
                                       const PaxTypeFare* curFare);
  uint64_t getNextDuration(const ShoppingTrx& trx, const std::set<uint64_t>& checkedDurations);

  void createFFinderAltDates(ShoppingTrx& trx);
  void initFFinderAltDates(ShoppingTrx& trx);
  void setFFinderAltDateJourneyItin(const DatePair& datePair,
                                    PricingTrx::AltDateInfo& altInfo,
                                    ShoppingTrx& trx);

  void prevalidateFares(FlightFinderTrx& ffTrx);
  bool isOutboundInboundFltValid(FlightFinderTrx& trx);
  bool validateFlightBitmap(FlightFinderTrx& trx);
  void processBFFTrx(FlightFinderTrx& trx);

  void processPromotionalShopping(FlightFinderTrx& trx);

  void mergeDateFlightMaps(FlightFinderTrx::OutBoundDateFlightMap& mapOut,
                           FlightFinderTrx::OutBoundDateFlightMap& map,
                           bool changeFlag = true);

  void processFFCarrier(FlightFinderTrx& trx,
                        const ItinIndex::Key& cxrKey,
                        uint32_t legId,
                        ShoppingTrx::Leg& curLeg,
                        Itin* journeyItin,
                        ShoppingTrx::AltDatePairs& altDatesMap,
                        FareMarketRuleController& ruleController,
                        FareMarketRuleController& cat4ruleController,
                        FareMarket* fM,
                        std::vector<std::vector<BCETuning> >& shoppingBCETuningData, // DEPRECATED
                        ShpBitValidationCollector::FMValidationSharedData* sharedData);

  void checkLimitationsFF(FlightFinderTrx& trx,
                          const ItinIndex::Key& cxrKey,
                          uint32_t legId,
                          ShoppingTrx::Leg& curLeg,
                          Itin* journeyItin,
                          ShoppingTrx::AltDatePairs& altDatesMap,
                          FareMarket* fareMarket);

  bool invalidForValidatingCxr(FlightFinderTrx& trx,
                               PaxTypeFare* ptf,
                               Itin* itin);

  void prepareMapToJourneyValidation(FlightFinderTrx& trx,
                                     DatePair& datePair,
                                     uint32_t legId,
                                     uint32_t sopId,
                                     PaxTypeFare* paxTypeFare,
                                     uint32_t bitmapIndex,
                                     uint64_t duration);

  FlightFinderTrx::OutBoundDateInfo* prepareOutMapToJourneyValidation(FlightFinderTrx& trx);

  void clearCategoryStatus(PaxTypeFare* paxTypeFare);

  void validateNonFltRulesForAltDates(ShoppingTrx& trx,
                                      const ItinIndex::Key* cxrKey = nullptr,
                                      PaxTypeFare* ptf = nullptr,
                                      FareMarket* fM = nullptr,
                                      const uint32_t numberOfFaresToProcess = DELAY_VALIDATION_OFF,
                                      const bool callback = false);

  void createAltDatesFlightBitmaps(ShoppingTrx& trx,
                                   FareMarket* fM,
                                   const uint32_t legId,
                                   const uint32_t flightSize,
                                   const ItinIndex::Key& carrierKey,
                                   const uint64_t& duration);

  void validateAltDateShoppingRules(ShoppingTrx& trx,
                                    FareMarket* fM,
                                    Itin* journeyItin,
                                    uint32_t legId,
                                    DatePair& datePair,
                                    std::vector<PaxTypeFare*>& faresPassedCat2,
                                    PaxTypeFare* ptf,
                                    const uint32_t numberOfFaresToProcess,
                                    const bool callback);

  void
  processCarrierFares(ShoppingTrx& trx,
                      const ItinIndex::Key& cxrKey,
                      uint32_t legId,
                      ShoppingTrx::Leg& curLeg,
                      Itin* journeyItin,
                      ShoppingTrx::AltDatePairs& altDatesMap,
                      FareMarketRuleController& ruleController,
                      FareMarketRuleController& cat4ruleController,
                      FareMarket* fM,
                      ShpBitValidationCollector::FMValidationSharedData* sharedData,
                      PaxTypeFare* ptf = nullptr,
                      const bool callback = false,
                      const uint32_t numberOfFaresToProcess = DELAY_VALIDATION_OFF);

  void processAdditionalFareMarketsForSOL(CarrierTasksForFareMarket& fareMarketsTasks);
  void processTasks(std::deque<ProcessCarrierTask>& tasks);
  void limitationValidation(ShoppingTrx& trx,
                            const ItinIndex::Key& cxrKey,
                            uint32_t legId,
                            ItinIndex& sIG,
                            Itin* journeyItin,
                            FareMarket* fM,
                            ShoppingTrx::AltDatePairs& altDatesMap,
                            uint32_t beginLeg,
                            uint32_t endLeg,
                            bool isStopOverLeg);
  void processFareWithPublishedRouting(ShoppingTrx& trx,
                                       FareMarket*& fM,
                                       const uint32_t& bitIndex,
                                       Itin*& journeyItin,
                                       ItinIndex::ItinCell& curItinCell,
                                       FareMarketRuleController& ruleController,
                                       FareMarketRuleController& cat4RuleController,
                                       ShoppingRtgMap& rtMap,
                                       std::vector<BCETuning>& bceTuningData,
                                       ShoppingTrx::Leg& curLeg,
                                       uint32_t beginLeg,
                                       uint32_t endLeg,
                                       std::vector<PaxTypeFare*>& tempAllPaxTypeFare);
  void processFlightBitMap(ShoppingTrx& trx,
                           PaxTypeFare*& curFare,
                           FareMarket*& fM,
                           const uint32_t& bitIndex,
                           Itin*& journeyItin,
                           ItinIndex::ItinCell& curItinCell,
                           FareMarketRuleController& ruleController,
                           FareMarketRuleController& cat4RuleController,
                           ShoppingRtgMap& rtMap,
                           std::vector<BCETuning>& bceTuningData,
                           ShoppingTrx::Leg& curLeg,
                           uint32_t beginLeg,
                           uint32_t endLeg);
  static void releaseUnusedFares(PricingTrx& trx);

  static bool processFRRAdjustedSellingLevel(PricingTrx& trx, const bool& isFareDisplay);

  void processExactlyMatchRetailerCode(FareDisplayTrx& trx);
private:
  FareValidatorOrchestrator(const FareValidatorOrchestrator& rhs);
  FareValidatorOrchestrator& operator=(const FareValidatorOrchestrator& rhs);

  // shopping specific functionality to create flight bitmaps
  void validateNonFltRules(ShoppingTrx& trx);

  void validateFMNonFltRules(ShoppingTrx& trx,
                             FareMarket* fM,
                             uint32_t carrierIndex,
                             const std::string& carrier,
                             FareMarketRuleController& ruleController,
                             ShoppingTrx::Leg& leg,
                             uint32_t legId,
                             JourneyItinWrapper& journeyItinWrapper);

  void createFlightBitmap(ShoppingTrx& trx,
                          PaxTypeFare* fare,
                          uint32_t flightSize,
                          FareMarket& fareMarket,
                          const ItinIndex::Key& carrierKey);

  void setFlightBitmapForMultiAirport(ShoppingTrx& trx,
                                      PaxTypeFare& fare,
                                      const FareMarket& fareMarket,
                                      const ItinIndex::Key& carrierKey);

  void createAltDateFBMforAllDuration(ShoppingTrx& trx, PaxTypeFare* paxTypeFare);
  void createAltDateFBMforAllDurationNew(ShoppingTrx& trx,
                                         const ItinIndex::Key& carrierKey,
                                         const uint32_t& bitmapSize,
                                         PaxTypeFare* paxTypeFare);

  void performLimitationValidations(PricingTrx& trx, Itin*& itin, FareMarket*& fareMarket);
  void setFBMStatusForAllDuration(const uint32_t& bitIndex,
                                  const DateTime& dateTime,
                                  uint8_t legId,
                                  PaxTypeFare* paxTypeFare,
                                  ShoppingTrx& trx);
  void setBitmapStatusForDatePair(const uint32_t& bitIndex,
                                  PaxTypeFare* paxTypeFare,
                                  const DatePair& datePair);

  void postValidateAllFares(FareDisplayTrx& trx);
  void convertCurrency(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare);
  MoneyAmount convertFare(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare);

  static void countFares(PricingTrx& trx);

  //  void qualifyCorpIDAccCode(FareDisplayTrx &trx);

  bool processIS(ShoppingTrx& trx);

  void removeFaresKeepForRoutingValidation(ShoppingTrx& trx);
  static bool refundSkip(const PricingTrx& trx, const FareMarket& fareMarket);
  void printBrandLegend(PricingTrx& trx);

  BitmapOpOrderer _bitmapOpOrderer;
  uint32_t _asoConxnPointMax;
  uint32_t _asoConxnPointBitmapSizeThreshold;
  uint32_t _asoMaxValidations;
  uint32_t _asoTotalMaxValidations;
  uint32_t _numPublishedRoutingFareToValidate;
  uint32_t _numberOfFaresToProcessADI;
  static uint32_t _minSizeToRemoveDuplicatedFares;
  FareValidatorOrchestratorShopping _shoppingFVO;

  friend class FareValidatorOrchestratorShopping;
  friend class FareValidatorOrchestratorShoppingOld;
  //
  // Create a new predicate from unary_function.
  //
  template <class Arg>
  class isNotSpecialRout : public std::unary_function<Arg, bool>
  {
  public:
    bool operator()(const Arg& arg1) const
    {
      return (!(arg1->routingNumber() == CAT25_DOMESTIC ||
                arg1->routingNumber() == CAT25_INTERNATIONAL ||
                arg1->routingNumber() == CAT25_EMPTY_ROUTING));
    }
  };
  template <class Arg>
  class isSpecialRout : public std::unary_function<Arg, bool>
  {
  public:
    bool operator()(const Arg& arg1) const
    {
      return (arg1->routingNumber() == CAT25_DOMESTIC ||
              arg1->routingNumber() == CAT25_INTERNATIONAL ||
              arg1->routingNumber() == CAT25_EMPTY_ROUTING);
    }
  };

  class RoutingThread : public tse::TseCallableTrxTask
  {
  public:
    enum Step
    {
      UNKNOWN = 0,
      BUILD_MAP = 1,
      VALIDATE = 2
    };

    RoutingThread() : _routingCntrl(nullptr), _fareMarket(nullptr), _step(UNKNOWN) {}

    RoutingController* _routingCntrl;
    FareMarket* _fareMarket;
    Step _step;

    void performTask() override;
  };

  class FaresKeepForRoutingValidationRemover
  {
  public:
    bool operator()(PaxTypeFare* ptFare)
    {
      if (nullptr == ptFare)
      {
        return true;
      }

      if (ptFare->isKeepForRoutingValidation())
      {
        return true;
      }

      return false;
    }
  };

  struct DiagValidationData
  {
    std::string _rtg;
    std::string _bkg;
    std::string _rul;
    bool _valid;
  };

  static void displayBrandValidation(PricingTrx& trx, PaxTypeFare& paxFare, DiagCollector& diag);
  static void displayFareMarketBrands(PricingTrx& trx, FareMarket& fm, DiagCollector& diag);

  static DiagValidationData* diagGetValidationResults(PricingTrx& trx,
                                                      PaxTypeFare& paxFare,
                                                      bool isFTDiag,
                                                      NoPNRPricingTrx* noPNRTrx);

  static void displayFlexFaresInfo(PricingTrx& trx,
                                   PaxTypeFare& paxFare,
                                   DiagCollector& diag);

 };

} // tse
