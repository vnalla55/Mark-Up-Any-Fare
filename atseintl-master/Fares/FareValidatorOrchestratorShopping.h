//-------------------------------------------------------------------
//
//  File:        FareValidatorOrchestratorShopping.h
//  Created:     Oct 24, 2011
//
//  Description: Fare Validator Orchestrator for Shopping
//
//  Updates:
//
//  Copyright Sabre 2011
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

#include "BookingCode/BCETuning.h"
#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VecMap.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/ShpBitValidationCollector.h"
#include "Fares/BitmapOpOrderer.h"
#include "Fares/JourneyItinWrapper.h"


#include <functional>

#include <cstdint>

namespace tse
{
class FareMarket;
class FareValidatorOrchestratorShopping;

class DelayValidationController
{
public:
  enum Mode
  {
    DISABLED = 0,
    DELAY_GLOBAL, // Apply delay validation for all fare markets
    DELAY_NO_SPECIAL_RTG // Apply delay validation for fare markets without special routing fares
  };

  DelayValidationController(ShoppingTrx& trx,
                            Mode delayValidationMode,
                            uint32_t numberOfFaresToProcess,
                            uint32_t numberOfFaresToProcessADI,
                            bool callback = false,
                            PaxTypeFare* ptf = nullptr);
  bool isActive() const;
  bool isCallback() const { return _callback; }
  void setCallback(bool callback = false) { _callback = callback; }
  bool isInitiallSetup() const { return _initiallSetup; }
  void setInitiallSetup(bool status = true) { _initiallSetup = status; }

  PaxTypeFare* getPaxTypeFare() { return _ptfToValidate; }

  void trackFareMarket(const FareMarket& fareMarket)
  {
    _fareMarketApplicable = isFareMarketApplicable(fareMarket);
  }

  void untrackFareMarket() { _fareMarketApplicable = true; }

  void reset()
  {
    _callback = false;
    _ptfToValidate = nullptr;
    _initiallSetup = true;
    _fareMarketApplicable = true;
  }

  uint32_t getNumberOfFaresToProcess() const { return _numOfFaresToProcess; }
  void setNumberOfFaresToProcess(uint32_t numberOfFaresToProcess)
  {
    _numOfFaresToProcess = numberOfFaresToProcess;
  }

private:
  bool isFareMarketApplicable(const FareMarket& fareMarket) const;

  Mode _delayValidationMode;

  bool _callback;
  bool _initiallSetup;
  bool _fareMarketApplicable;
  uint32_t _numOfFaresToProcess;
  PaxTypeFare* _ptfToValidate;
};

// TODO: this should be moved to different file and FVO reference should be removed.
// ProcessCarrierFares method should be task method, not FVO method
class ProcessCarrierTaskShopping : public TseCallableTrxTask
{
public:
  ProcessCarrierTaskShopping(FareValidatorOrchestratorShopping& fvo,
                             ShoppingTrx& shoppingTrx,
                             DelayValidationController delayController,
                             JourneyItinWrapper journeyItin,
                             ShpBitValidationCollector::FMValidationSharedData* sharedData);

  void performTask() override;

private:
  FareValidatorOrchestratorShopping& _fvo;
  ShoppingTrx& _trx;
  DelayValidationController _delayController;
  JourneyItinWrapper _journeyItin;
  ShpBitValidationCollector::FMValidationSharedData* _sharedData;
};

typedef std::map<FareMarket*, std::list<ProcessCarrierTaskShopping> >
CarrierTasksShoppingForFareMarket;

class FareValidatorOrchestrator;

// TODO: this class requires further refactoring and cleanup.
class FareValidatorOrchestratorShopping
{
public:
  FareValidatorOrchestratorShopping(FareValidatorOrchestrator& mainFVO, TseServer& server);

  bool process(ShoppingTrx& trx);

  void processCarrier(ShoppingTrx& trx,
                      DelayValidationController& delayController,
                      JourneyItinWrapper& journeyItin,
                      ShpBitValidationCollector::FMValidationSharedData* sharedData);

  void callback(ShoppingTrx& trx, FareMarket* fM, PaxTypeFare* ptf, ItinIndex::Key* carrierKey = nullptr);
  bool checkCarriersFlightBitmapsStatus(const PaxTypeFare* ptf, uint64_t mainDuration = 0) const;

  static ConfigurableValue<size_t> numberOfFaresToProcess;

protected:
  int memCheckInterval;
  int perMemGrowthCheckInterval;

  void processCarrierFares(ShoppingTrx& trx,
                           DelayValidationController& delayController,
                           JourneyItinWrapper& journeyItin,
                           ShpBitValidationCollector::FMValidationSharedData* sharedData);

  void processAdditionalFareMarketsForSOL(CarrierTasksShoppingForFareMarket& fareMarketsTasks);
  void processTasks(std::deque<ProcessCarrierTaskShopping>& tasks);
  void limitationValidation(ShoppingTrx& trx, JourneyItinWrapper& journeyItin);
  void processFareWithPublishedRouting(ShoppingTrx& trx,
                                       const uint32_t bitIndex,
                                       JourneyItinWrapper& journeyItin,
                                       ItinIndex::ItinCellInfo& curItinCellInfo,
                                       ShoppingRtgMap& rtMap,
                                       std::vector<BCETuning>& bceTuningData,
                                       Itin* curItinCellItin,
                                       std::vector<PaxTypeFare*>& tempAllPaxTypeFare);
  void processFareWithPublishedRoutingDelayed(ShoppingTrx& trx,
                                              JourneyItinWrapper& journeyItin,
                                              ShoppingRtgMap& rtMap,
                                              std::vector<PaxTypeFare*>& nonSpecialRtgFares);

  void processFlightBitMap(ShoppingTrx& trx,
                           PaxTypeFare*& curFare,
                           const uint32_t bitIndex,
                           JourneyItinWrapper& journeyItin,
                           ItinIndex::ItinCellInfo& curItinCellInfo,
                           ShoppingRtgMap& rtMap,
                           std::vector<BCETuning>& bceTuningData,
                           Itin* curItinCellItin);

private:
  FareValidatorOrchestratorShopping(const FareValidatorOrchestratorShopping& rhs);
  FareValidatorOrchestratorShopping& operator=(const FareValidatorOrchestratorShopping& rhs);

  void processCarrierFaresImpl(ShoppingTrx& trx,
                               DelayValidationController& delayController,
                               JourneyItinWrapper& journeyItin,
                               ShpBitValidationCollector::FMValidationSharedData* sharedData);

  void validateNonFltRules(ShoppingTrx& trx,
                           DelayValidationController& delayController,
                           Itin* journeyItin = nullptr,
                           FareMarket* fareMarketCallback = nullptr,
                           ItinIndex::Key* carrierKey = nullptr);

  void validateNonFltRulesForFixedDate(ShoppingTrx& trx,
                                       DelayValidationController& delayController,
                                       FareMarket* fareMarketCallback = nullptr,
                                       ItinIndex::Key* carrierKey = nullptr);

  void validateNonFltRulesForAltDates(ShoppingTrx& trx,
                                      DelayValidationController& delayController,
                                      FareMarket* fareMarketCallback = nullptr,
                                      ItinIndex::Key* carrierKey = nullptr);

  void validateNonFltRulesForCarrier(ShoppingTrx& trx,
                                     DelayValidationController& delayController,
                                     FareMarketRuleController& ruleController,
                                     ShoppingTrx::Leg& curLeg,
                                     const ItinIndex::Key& carrierKey,
                                     const uint32_t legId,
                                     Itin* journeyItinAltDate = nullptr);

  void validateNonFltRulesForCallback(ShoppingTrx& trx,
                                      DelayValidationController& delayController,
                                      FareMarketRuleController& ruleController,
                                      ShoppingTrx::Leg& curLeg,
                                      const uint32_t legId,
                                      FareMarket* fareMarketCallback = nullptr,
                                      ItinIndex::Key* carrierKey = nullptr,
                                      Itin* journeyItinAltDate = nullptr);

  void validateFMNonFltRules(ShoppingTrx& trx,
                             DelayValidationController& delayController,
                             JourneyItinWrapper& journeyItin,
                             const std::string& carrier,
                             FareMarketRuleController& ruleController);

  void setFBMStatusForAllDuration(const uint32_t& bitIndex,
                                  const DateTime& dateTime,
                                  uint8_t legId,
                                  PaxTypeFare* paxTypeFare,
                                  ShoppingTrx& trx,
                                  uint32_t carrierKey);

  bool isFareValid(ShoppingTrx& trx, const PaxTypeFare* fare, const FareMarket* fareMarket) const;

  void setNotUsedBits(PaxTypeFare::FlightBitmap& bitmap, const SOPUsages& usages);

  bool validateAsoConectionPointLimits(const ShoppingTrx& trx,
                                       PaxTypeFare* fare,
                                       const uint32_t bitIndex,
                                       const uint32_t totalTravelSegmentSize);

  bool copyAlreadyValidatedBit(PaxTypeFare::FlightBitmap& bitmap,
                               ApplicableSOP* applicableSops,
                               const uint32_t carrierKey,
                               const uint32_t bitIndex);
  void releaseUnusedFares(ShoppingTrx& trx);
  void createFlightBitmap(ShoppingTrx& trx,
                          PaxTypeFare* fare,
                          const uint32_t flightSize,
                          FareMarket* fareMarket,
                          JourneyItinWrapper& journeyItin);
  template <typename T>
  bool checkFlightBitmapsValidity(const VecMap<T, PaxTypeFare::FlightBitmap>& collection,
                                  const PaxTypeFare* ptf) const;
  ShoppingTrx::AltDatePairs& getAltDatePairs(ShoppingTrx& trx, const uint8_t legId) const;
  void createFaresFlightBitmaps(ShoppingTrx& trx,
                                std::vector<PaxTypeFare*>::iterator it,
                                std::vector<PaxTypeFare*>::iterator itEnd,
                                JourneyItinWrapper& journeyItin);
  void validateFaresNonFltRules(ShoppingTrx& trx,
                                DelayValidationController& delayController,
                                JourneyItinWrapper& journeyItin,
                                FareMarketRuleController& ruleController,
                                std::vector<PaxTypeFare*>& faresPassedCat2);
  void findFaresRequiresMoreValidation(FareMarket* fareMarket, uint64_t duration) const;

  void
  collectNonSpecialRtgFares(FareMarket& fareMarket, std::vector<PaxTypeFare*>& nonSpecialRtgFares);

  BitmapOpOrderer _bitmapOpOrderer;
  uint32_t _asoConxnPointMax;
  uint32_t _asoConxnPointBitmapSizeThreshold;
  uint32_t _asoMaxValidations;
  uint32_t _asoTotalMaxValidations;
  uint32_t _numPublishedRoutingFareToValidate;
  uint32_t _numPublishedRoutingFareToValidateDelayed;
  uint32_t _numberOfFaresToProcess;
  uint32_t _numberOfFaresToProcessADI;
  uint32_t _maxApplicableBitsFailCountLimit;
  uint32_t _maxApplicableBitsFailLimitCheapestFares;
  uint32_t _maxApplicableBitsFailLimitCheapestFaresCount;
  DelayValidationController::Mode _delayValidationMode;

  FareValidatorOrchestrator& _mainFVO;

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
};

} // End namespace tse

