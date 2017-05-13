//-------------------------------------------------------------------
//
//  File:        FareValidatorOrchestratorShopping.cpp
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

#include "Fares/FareValidatorOrchestratorShopping.h"

#include "BookingCode/BCETuning.h"
#include "BookingCode/FareBookingCodeValidator.h"
#include "BookingCode/MixedClassController.h"
#include "Common/AltPricingUtil.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/DiagMonitor.h"
#include "Common/FareDisplayUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/FxCnException.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/GlobalManager.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
#include "Common/RoutingUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TSEAlgorithms.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "Common/TseSrvStats.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/QualifyFltAppRuleData.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/ContractPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/MultiTransport.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/AltDatesPairOptionsBounder.h"
#include "Fares/BitmapOpOrderer.h"
#include "Fares/BooleanFlagResetter.h"
#include "Fares/FareCollectorOrchestrator.h"
#include "Fares/FareUtil.h"
#include "Fares/FareValidatorOrchestrator.h"
#include "Fares/FDFareCurrencySelection.h"
#include "Fares/FlightFinderJourneyValidator.h"
#include "Fares/RoutingController.h"
#include "Fares/RuleCategoryProcessor.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Routing/MileageInfo.h"
#include "Rules/DayTimeApplication.h"
#include "Rules/FDFareMarketRuleController.h"
#include "Rules/FDSurchargesRule.h"
#include "Rules/FlightApplication.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"
#include "Server/TseServer.h"
#include "Util/BranchPrediction.h"

#include <iostream>
#include <limits>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackRemoveFailedSopsInvalidation);
FALLBACK_DECL(reworkTrxAborter);
FALLBACK_DECL(unifyMemoryAborter)

namespace
{
static const uint32_t MAX_ASOITIN_CONXN_POINT_DEFAULT = 4;
static const uint32_t MAX_ASOBITMAP_SIZE_THRESHOLD_DEFAULT = 2000;
static const uint32_t MAX_ASO_VALIDATIONS_DEFAULT = 100000;
static const uint32_t MAX_TOTAL_ASO_VALIDATIONS_DEFAULT = 100000;
static const uint32_t NUM_PUBLISHED_ROUTING_FARE_TO_VALIDATE_DEFAULT = 1;

ConfigurableValue<uint32_t>
shoppingSkipThreshold("FARESV_SVC", "SHOPPING_SKIP_THRESHOLD", 10000);
ConfigurableValue<bool>
releaseUnusedFaresAllItins("SHOPPING_OPT", "RELEASE_UNUSED_FARES_ALL_ITINS", false);
ConfigurableValue<uint32_t>
asoConxnPointBitmapSizeThreshold("SHOPPING_OPT",
                                 "ASO_CONXN_POINT_BITSIZE_THRESHOLD",
                                 MAX_ASOBITMAP_SIZE_THRESHOLD_DEFAULT);
ConfigurableValue<uint32_t>
asoConxnPointMax("SHOPPING_OPT", "ASO_CONXN_POINT_MAX", MAX_ASOITIN_CONXN_POINT_DEFAULT);
ConfigurableValue<uint32_t>
asoMaxValidations("SHOPPING_OPT", "ASO_MAX_VALIDATIONS", MAX_ASO_VALIDATIONS_DEFAULT);
ConfigurableValue<uint32_t>
asoTotalMaxValidations("SHOPPING_OPT",
                       "ASO_MAX_TOTAL_VALIDATIONS",
                       MAX_TOTAL_ASO_VALIDATIONS_DEFAULT);
ConfigurableValue<uint32_t>
numPublishedRoutingFareToValidate("SHOPPING_OPT",
                                  "NUM_PUBLISHED_ROUTING_FARE_TO_VALIDATE",
                                  NUM_PUBLISHED_ROUTING_FARE_TO_VALIDATE_DEFAULT);
ConfigurableValue<uint32_t>
numPublishedRoutingFareToValidateDelayed("SHOPPING_OPT",
                                         "NUM_PUBLISHED_ROUTING_FARE_TO_VALIDATE_DELAYED",
                                         NUM_PUBLISHED_ROUTING_FARE_TO_VALIDATE_DEFAULT);
ConfigurableValue<uint32_t>
numberOfFaresToProcessADI("FARESV_SVC",
                          "SHOPPING_ALTDATE_NUMBER_OF_FARES_TO_PROCESS_THRESHOLD",
                          DELAY_VALIDATION_OFF);
ConfigurableValue<uint32_t>
maxApplicableBitsFailCountLimit("FARESV_SVC", "FAILED_BITS_PER_FARE_LIMIT", 0);
ConfigurableValue<uint32_t>
maxApplicableBitsFailLimitCheapestFares("FARESV_SVC",
                                        "FAILED_BITS_PER_FARE_LIMIT_FOR_CHEAPEST_FARES",
                                        0);
ConfigurableValue<uint32_t>
maxApplicableBitsFailLimitCheapestFaresCount("FARESV_SVC",
                                             "FAILED_BITS_PER_FARE_LIMIT_NUM_CHEAPEST_FARES",
                                             0);
ConfigurableValue<bool>
delayValidationMode("FARESV_SVC", "SHOPPING_DELAY_VALIDATION_GLOBAL", true);

Logger
logger("atseintl.Fares.FareValidatorOrchestratorShopping");
}

ConfigurableValue<size_t>
FareValidatorOrchestratorShopping::numberOfFaresToProcess(
    "FARESV_SVC", "SHOPPING_NUMBER_OF_FARES_TO_PROCESS_THRESHOLD");

ProcessCarrierTaskShopping::ProcessCarrierTaskShopping(
    FareValidatorOrchestratorShopping& fvo,
    ShoppingTrx& shoppingTrx,
    DelayValidationController delayController,
    JourneyItinWrapper journeyItin,
    ShpBitValidationCollector::FMValidationSharedData* sharedData)
  : _fvo(fvo),
    _trx(shoppingTrx),
    _delayController(delayController),
    _journeyItin(journeyItin),
    _sharedData(sharedData)
{
  trx(&_trx);
}

void
ProcessCarrierTaskShopping::performTask()
{
  _fvo.processCarrier(_trx, _delayController, _journeyItin, _sharedData);
}

DelayValidationController::DelayValidationController(
    ShoppingTrx& trx,
    DelayValidationController::Mode delayValidationMode,
    uint32_t numberOfFaresToProcess,
    uint32_t numberOfFaresToProcessADI,
    bool callback,
    PaxTypeFare* ptf)
  : _delayValidationMode(DISABLED),
    _callback(callback),
    _initiallSetup(true),
    _fareMarketApplicable(true),
    _numOfFaresToProcess(DELAY_VALIDATION_OFF),
    _ptfToValidate(ptf)
{
  const bool applicable =
      (trx.paxType().size() <= 1) && (trx.isAltDates() || trx.isSumOfLocalsProcessingEnabled());
  if (LIKELY(applicable))
  {
    _numOfFaresToProcess = (trx.isAltDates() ? numberOfFaresToProcessADI : numberOfFaresToProcess);
    if (LIKELY(DELAY_VALIDATION_OFF != _numOfFaresToProcess))
      _delayValidationMode = delayValidationMode;
  }
}

bool
DelayValidationController::isActive() const
{
  return _delayValidationMode != DISABLED && _fareMarketApplicable;
}

bool
DelayValidationController::isFareMarketApplicable(const FareMarket& fareMarket) const
{
  if (LIKELY(_delayValidationMode == DELAY_GLOBAL))
    return true;
  return !fareMarket.specialRtgFound();
}

FareValidatorOrchestratorShopping::FareValidatorOrchestratorShopping(
    FareValidatorOrchestrator& mainFVO, TseServer& server)
  : _bitmapOpOrderer(server.config()),
    _asoConxnPointMax(MAX_ASOITIN_CONXN_POINT_DEFAULT),
    _asoConxnPointBitmapSizeThreshold(MAX_ASOBITMAP_SIZE_THRESHOLD_DEFAULT),
    _asoMaxValidations(MAX_ASO_VALIDATIONS_DEFAULT),
    _asoTotalMaxValidations(MAX_TOTAL_ASO_VALIDATIONS_DEFAULT),
    _numPublishedRoutingFareToValidate(NUM_PUBLISHED_ROUTING_FARE_TO_VALIDATE_DEFAULT),
    _numPublishedRoutingFareToValidateDelayed(NUM_PUBLISHED_ROUTING_FARE_TO_VALIDATE_DEFAULT),
    _numberOfFaresToProcess(DELAY_VALIDATION_OFF),
    _numberOfFaresToProcessADI(DELAY_VALIDATION_OFF),
    _maxApplicableBitsFailCountLimit(0),
    _maxApplicableBitsFailLimitCheapestFares(0),
    _maxApplicableBitsFailLimitCheapestFaresCount(0),
    _delayValidationMode(DelayValidationController::DELAY_GLOBAL),
    _mainFVO(mainFVO)
{
  _asoConxnPointBitmapSizeThreshold = asoConxnPointBitmapSizeThreshold.getValue();
  _asoConxnPointMax = asoConxnPointMax.getValue();
  _asoMaxValidations = asoMaxValidations.getValue();
  _asoTotalMaxValidations = asoTotalMaxValidations.getValue();
  _numPublishedRoutingFareToValidate = numPublishedRoutingFareToValidate.getValue();
  _numPublishedRoutingFareToValidateDelayed = numPublishedRoutingFareToValidateDelayed.getValue();
  _numberOfFaresToProcessADI = numberOfFaresToProcessADI.getValue();
  _numberOfFaresToProcess = numberOfFaresToProcess.getValue();
  _maxApplicableBitsFailCountLimit = maxApplicableBitsFailCountLimit.getValue();
  _maxApplicableBitsFailLimitCheapestFares = maxApplicableBitsFailLimitCheapestFares.getValue();
  _maxApplicableBitsFailLimitCheapestFaresCount =
      maxApplicableBitsFailLimitCheapestFaresCount.getValue();
  if (!delayValidationMode.getValue())
    _delayValidationMode = DelayValidationController::DELAY_NO_SPECIAL_RTG;
  memCheckInterval = 0;
  perMemGrowthCheckInterval = 0;
}

bool
FareValidatorOrchestratorShopping::process(ShoppingTrx& trx)
{
  const TSELatencyData metrics(trx, "FVO PROCESS");
  LOG4CXX_INFO(logger, "FareValidatorOrchestratorShopping::process(ShoppingTrx)...");

  DelayValidationController delayController(
      trx, _delayValidationMode, _numberOfFaresToProcess, _numberOfFaresToProcessADI);
  if (trx.isAltDates())
  {
    if (trx.isSimpleTrip() && trx.isRuleTuningISProcess())
      trx.addCat(RuleConst::SEASONAL_RULE);
    validateNonFltRulesForAltDates(trx, delayController);
  }
  else
  {
    validateNonFltRulesForFixedDate(trx, delayController);
  }
  delayController.reset();
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  if (legs.empty())
    return false;

  trx.journeyItin()->validatingCarrier().clear();
  std::deque<ProcessCarrierTaskShopping> tasks;
  std::set<FareMarket*> uniqueFareMarkets;
  CarrierTasksShoppingForFareMarket tasksForAdditionalSolFareMarkets;
  ShpBitValidationCollector& shareCollector = trx.getBitValidationCollector();
  std::vector<ShoppingTrx::Leg>::iterator legIt = legs.begin();

  for (uint32_t legId = 0; legIt != legs.end(); ++legIt, ++legId)
  {
    ShoppingTrx::Leg& curLeg = (*legIt);
    // Get the carrier iterators
    ItinIndex::ItinMatrixIterator itinMatrixIt = curLeg.carrierIndex().root().begin();

    for (; itinMatrixIt != curLeg.carrierIndex().root().end(); ++itinMatrixIt)
    {
      ItinIndex::ItinCell* curCell = ShoppingUtil::retrieveDirectItin(
          trx, legId, itinMatrixIt->first, ItinIndex::CHECK_NOTHING);
      if (UNLIKELY(!curCell))
        continue;

      // Get the direct itinerary for this carrier
      Itin* thruFareDirectItin = curCell->second;
      if (UNLIKELY(!thruFareDirectItin))
        continue;

      std::vector<FareMarket*>& fareMarkets(thruFareDirectItin->fareMarket());

      if (LIKELY(trx.isSumOfLocalsProcessingEnabled()))
      {
        std::vector<FareMarket*>::iterator fareMarketIt(fareMarkets.begin());

        for (; fareMarketIt != fareMarkets.end(); ++fareMarketIt)
        {
          FareMarket* fareMarket = *fareMarketIt;
          if (!fareMarket || fareMarket->failCode() != ErrorResponseException::NO_ERROR)
            continue;

          JourneyItinWrapper journeyItin(trx, *fareMarketIt, curLeg, legId, itinMatrixIt->first);
          ShpBitValidationCollector::FMValidationSharedData* sharedData(
              shareCollector.getFMSharedData(journeyItin.getCarrierKey(),
                                             journeyItin.getFareMarket()));

          ProcessCarrierTaskShopping task(*this, trx, delayController, journeyItin, sharedData);

          if (uniqueFareMarkets.insert(*fareMarketIt).second)
            tasks.push_back(task);
          else
            tasksForAdditionalSolFareMarkets[*fareMarketIt].push_back(task);
        }
      }
      else // Non SOL request
      {
        // get the thru-fare market
        FareMarket* fareMarket = fareMarkets.front();

        if (!fareMarket || fareMarket->failCode() != ErrorResponseException::NO_ERROR)
          continue;

        JourneyItinWrapper journeyItin(trx, fareMarket, curLeg, legId, itinMatrixIt->first);
        ShpBitValidationCollector::FMValidationSharedData* sharedData(
            shareCollector.getFMSharedData(journeyItin.getCarrierKey(),
                                           journeyItin.getFareMarket()));

        tasks.push_back(
            ProcessCarrierTaskShopping(*this, trx, delayController, journeyItin, sharedData));
      }
    }
  }

  const TSELatencyData metricsThreads(trx, "FVO BITMAP VALIDATION");
  processTasks(tasks);

  if (!tasksForAdditionalSolFareMarkets.empty())
    processAdditionalFareMarketsForSOL(tasksForAdditionalSolFareMarkets);

  if (smp::isPenaltyCalculationRequired(trx))
  {
    exec_foreach_valid_fareMarket(TseThreadingConst::SHOPPING_TASK,
                                  trx,
                                  smp::preValidateFareMarket,
                                  FareMarket::FareValidator);
  }

  releaseUnusedFares(trx);
  return true;
}

void
FareValidatorOrchestratorShopping::processAdditionalFareMarketsForSOL(
    CarrierTasksShoppingForFareMarket& fareMarketsTasks)
{
  int tasksToProcess = 1;

  while (tasksToProcess > 0)
  {
    std::deque<ProcessCarrierTaskShopping> tasks;
    CarrierTasksShoppingForFareMarket::iterator fmIt(fareMarketsTasks.begin());

    for (; fmIt != fareMarketsTasks.end(); ++fmIt)
    {
      if (!fmIt->second.empty())
      {
        tasks.push_back(fmIt->second.front());
        fmIt->second.pop_front();
      }
    }

    processTasks(tasks);
    tasksToProcess = tasks.size();
  }
}

void
FareValidatorOrchestratorShopping::processTasks(std::deque<ProcessCarrierTaskShopping>& tasks)
{
  TseRunnableExecutor taskExecutor(TseThreadingConst::SHOPPING_TASK);
  std::deque<ProcessCarrierTaskShopping>::iterator taskIter = tasks.begin();
  std::deque<ProcessCarrierTaskShopping>::iterator taskEndIter = tasks.end();

  for (; taskIter != taskEndIter; ++taskIter)
  {
    ProcessCarrierTaskShopping& task = *taskIter;
    taskExecutor.execute(task);
  }

  taskExecutor.wait();
}

namespace
{
class FMSegOpenSetter
{
  DataHandle _dh;
  FareMarket* _fm;
  std::vector<TravelSeg*> _prevSegs;

public:
  FMSegOpenSetter() : _fm(nullptr) {}

  void setSegments(FareMarket* fm)
  {
    if (UNLIKELY(!fm))
    {
      return;
    }

    _fm = fm;
    _prevSegs = _fm->travelSeg();

    for (size_t i = 0; i < _prevSegs.size(); i++)
    {
      TravelSeg* newSeg = _prevSegs[i]->clone(_dh);
      newSeg->segmentType() = Open;
      newSeg->arrivalDT() = DateTime();

      AirSeg* airSeg;
      if (LIKELY(airSeg = dynamic_cast<AirSeg*>(newSeg)))
      {
        airSeg->setMarketingCarrierCode(CarrierCode());
        airSeg->operatingFlightNumber() = FlightNumber();
      }

      newSeg->departureDT() = DateTime(
          newSeg->departureDT().year(), newSeg->departureDT().month(), newSeg->departureDT().day());
      newSeg->pssDepartureDate() = newSeg->departureDT().dateToSqlString();
      _fm->travelSeg()[i] = newSeg;
    }
  }

  ~FMSegOpenSetter()
  {
    if (LIKELY(_fm))
    {
      _fm->travelSeg() = _prevSegs;
    }
  }
};
}

void
FareValidatorOrchestratorShopping::validateNonFltRulesForFixedDate(
    ShoppingTrx& trx,
    DelayValidationController& delayController,
    FareMarket* fareMarketCallback,
    ItinIndex::Key* cxrKey)
{
  TSELatencyData metrics(trx, "FVO VALIDATE NON FLT RULES");
  LOG4CXX_DEBUG(logger, "FareValidatorOrchestratorShopping::validateNonFltRulesForFixedDate...");

  validateNonFltRules(trx, delayController, nullptr, fareMarketCallback, cxrKey);
}

void
FareValidatorOrchestratorShopping::validateNonFltRules(ShoppingTrx& trx,
                                                       DelayValidationController& delayController,
                                                       Itin* journeyItin,
                                                       FareMarket* fareMarketCallback,
                                                       ItinIndex::Key* cxrKey)
{
  RuleControllerWithChancelor<FareMarketRuleController> shoppingRuleController(
      ShoppingComponentValidation),
      shoppingASORuleController(ShoppingAcrossStopOverComponentValidation);

  std::vector<uint16_t> ruleControllerCategorySequence = shoppingRuleController.categorySequence();
  std::vector<uint16_t> asoRuleControllerCategorySequence =
      shoppingASORuleController.categorySequence();

  bool categorySequenceModified = false;

  std::vector<ShoppingTrx::Leg>::iterator legIt = trx.legs().begin();
  for (uint32_t legId = 0; legIt != trx.legs().end(); ++legIt, ++legId)
  {
    ShoppingTrx::Leg& curLeg = (*legIt);
    if (categorySequenceModified)
    {
      // always reset to original set up
      shoppingRuleController.categorySequence() = ruleControllerCategorySequence;
      shoppingASORuleController.categorySequence() = asoRuleControllerCategorySequence;
      categorySequenceModified = false;
    }

    if (legIt->dateChange())
    {
      categorySequenceModified = true;
      shoppingRuleController.removeCat(RuleConst::DAY_TIME_RULE);
      shoppingASORuleController.removeCat(RuleConst::DAY_TIME_RULE);
    }

    FareMarketRuleController& ruleController(curLeg.stopOverLegFlag() ? shoppingASORuleController
                                                                      : shoppingRuleController);

    if (!delayController.isCallback())
    {
      ItinIndex& carrierIndex = legIt->carrierIndex();
      if (carrierIndex.root().empty())
        continue;

      ItinIndex::ItinMatrixIterator itinIt = carrierIndex.root().begin();
      for (; itinIt != carrierIndex.root().end(); ++itinIt)
      {
        validateNonFltRulesForCarrier(
            trx, delayController, ruleController, curLeg, itinIt->first, legId, journeyItin);
      }
    }
    else
    {
      if (UNLIKELY(!trx.isSumOfLocalsProcessingEnabled()))
      {
        LOG4CXX_WARN(logger, "Delay validation should not be called for non SOL path")
        return;
      }

      if (UNLIKELY(!fareMarketCallback))
      {
        LOG4CXX_WARN(logger, "Delay validation should not be called without FM for revalidation")
        return;
      }

      if (legId != fareMarketCallback->legIndex())
        continue;

      validateNonFltRulesForCallback(trx,
                                     delayController,
                                     ruleController,
                                     curLeg,
                                     legId,
                                     fareMarketCallback,
                                     cxrKey,
                                     journeyItin);
    }
  }
}

void
FareValidatorOrchestratorShopping::validateNonFltRulesForCarrier(
    ShoppingTrx& trx,
    DelayValidationController& delayController,
    FareMarketRuleController& ruleController,
    ShoppingTrx::Leg& curLeg,
    const ItinIndex::Key& cxrKey,
    const uint32_t legId,
    Itin* journeyItinAltDate)
{
  ItinIndex::ItinCell* itinCell = nullptr;
  itinCell = ShoppingUtil::retrieveDirectItin(trx, legId, cxrKey, ItinIndex::CHECK_NOTHING);
  if (UNLIKELY(!itinCell))
    return;

  Itin* curItin = itinCell->second;
  if (UNLIKELY(!curItin))
    return;

  std::vector<FareMarket*>& fareMarkets(curItin->fareMarket());

  if (LIKELY(trx.isSumOfLocalsProcessingEnabled()))
  {
    std::vector<FareMarket*>::iterator fareMarketIt(fareMarkets.begin());

    for (; fareMarketIt != fareMarkets.end(); ++fareMarketIt)
    {
      FareMarket* fareMarket(*fareMarketIt);
      if (!fareMarket || fareMarket->failCode() != ErrorResponseException::NO_ERROR)
        continue;

      JourneyItinWrapper journeyItin(trx, fareMarket, curLeg, legId, cxrKey, journeyItinAltDate);
      fareMarket->setComponentValidationForCarrier(journeyItin.getCarrierKey());
      delayController.trackFareMarket(*fareMarket);

      if (LIKELY(delayController.isActive()))
      {
        fareMarket->setFoundPTF(true);
        if (delayController.isInitiallSetup())
        {
          fareMarket->setLastFareProcessed(-1);
          fareMarket->setFirstFareProcessed(-1);
        }
      }

      const ShoppingTrx::SchedulingOption& sop(curLeg.sop()[itinCell->first.sopIndex()]);
      validateFMNonFltRules(
          trx, delayController, journeyItin, sop.governingCarrier(), ruleController);
      delayController.untrackFareMarket();
    }
  }
  else
  {
    FareMarket* fareMarket = fareMarkets.front();
    if (!fareMarket || fareMarket->failCode() != ErrorResponseException::NO_ERROR)
      return;

    JourneyItinWrapper journeyItin(trx, fareMarket, curLeg, legId, cxrKey);
    delayController.trackFareMarket(*fareMarket);
    validateFMNonFltRules(
        trx, delayController, journeyItin, fareMarket->governingCarrier(), ruleController);
    delayController.untrackFareMarket();
  }
}

void
FareValidatorOrchestratorShopping::validateNonFltRulesForCallback(
    ShoppingTrx& trx,
    DelayValidationController& delayController,
    FareMarketRuleController& ruleController,
    ShoppingTrx::Leg& curLeg,
    const uint32_t legId,
    FareMarket* fareMarketCallback,
    ItinIndex::Key* carrierKey,
    Itin* journeyItinAltDate)
{
  fareMarketCallback->setComponentValidationForCarrier(*carrierKey);
  JourneyItinWrapper journeyItin(
      trx, fareMarketCallback, curLeg, legId, *carrierKey, journeyItinAltDate);
  validateFMNonFltRules(
      trx, delayController, journeyItin, fareMarketCallback->governingCarrier(), ruleController);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void
FareValidatorOrchestratorShopping::validateFMNonFltRules(ShoppingTrx& trx,
                                                         DelayValidationController& delayController,
                                                         JourneyItinWrapper& journeyItin,
                                                         const std::string& carrier,
                                                         FareMarketRuleController& ruleController)
{
  FareMarket* fareMarket(journeyItin.getFareMarket());
  FMSegOpenSetter setter;
  setter.setSegments(fareMarket);

  journeyItin.applySegments();
  journeyItin.getJourneyItin().validatingCarrier() = carrier;

  std::vector<PaxTypeFare*> faresPassedCat2;
  validateFaresNonFltRules(trx, delayController, journeyItin, ruleController, faresPassedCat2);

  journeyItin.eraseSegments();

  std::vector<PaxTypeFare*>::iterator fareIt = fareMarket->allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator fareItEnd = fareMarket->allPaxTypeFare().end();

  if (LIKELY(delayController.isActive()))
  {
    fareIt = fareMarket->allPaxTypeFareProcessed().begin();
    fareItEnd = fareMarket->allPaxTypeFareProcessed().end();
  }

  createFaresFlightBitmaps(trx, fareIt, fareItEnd, journeyItin);

  fareMarket->setGlobalDirection(GlobalDirection::ZZ); // return to zz global direction

  // loop over all fares that passed cat 2, and set their processing of cat 2
  // to false, so it will be re-tested in the next phase
  for (auto ptf : faresPassedCat2)
  {
    if (LIKELY(ptf->isCategoryValid(2)))
      ptf->setCategoryProcessed(2, false);
  }
}

void
FareValidatorOrchestratorShopping::validateFaresNonFltRules(
    ShoppingTrx& trx,
    DelayValidationController& delayController,
    JourneyItinWrapper& journeyItin,
    FareMarketRuleController& ruleController,
    std::vector<PaxTypeFare*>& faresPassedCat2)
{
  FareMarket* fareMarket(journeyItin.getFareMarket());
  std::vector<PaxTypeFare*>::iterator fareIt = fareMarket->allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator fareItEnd = fareMarket->allPaxTypeFare().end();

  int fareIndex = 0;
  if (LIKELY(delayController.isActive()))
  {
    if (delayController.isInitiallSetup())
    {
      // If callback from PO we need to start from the scratch because fares in FM were sorted again
      if (!delayController.getPaxTypeFare())
      {
        if (fareMarket->lastFareProcessed() + 1 >=
            static_cast<int>(fareMarket->allPaxTypeFare().size()))
          return;
        if (delayController.isCallback())
          std::advance(fareIt, fareMarket->lastFareProcessed() + 1);
        fareIndex = std::distance(fareMarket->allPaxTypeFare().begin(), fareIt);
      }
    }
    else
    {
      TSE_ASSERT(fareMarket->firstFareProcessed() >= 0 && fareMarket->lastFareProcessed() >= 0);
      fareIt = fareMarket->allPaxTypeFareProcessed().begin();
      fareItEnd = fareMarket->allPaxTypeFareProcessed().end();
      if (delayController.isCallback())
        fareIndex = fareMarket->lastFareProcessed();
      else
        fareIndex = fareMarket->firstFareProcessed();
    }
  }

  DatePair* datePair = (trx.isAltDates() ? journeyItin.getJourneyItin().datePair() : nullptr);
  for (int noOfFaresPassed = 0; fareIt != fareItEnd; ++fareIt, ++fareIndex)
  {
    PaxTypeFare* curFare(*fareIt);
    if (UNLIKELY(!curFare))
      continue;

    if (LIKELY(trx.isSumOfLocalsProcessingEnabled()))
      curFare->setComponentValidationForCarrier(
          journeyItin.getCarrierKey(), trx.isAltDates(), journeyItin.getDuration());
    if (LIKELY(delayController.isActive()))
    {
      if (delayController.isInitiallSetup() && delayController.getPaxTypeFare())
      {
        if (curFare == delayController.getPaxTypeFare())
          fareMarket->setFoundPTF(true);
        if (curFare->isFltIndependentValidationFVO() || !curFare->flightBitmap().empty())
          continue;
      }

      if (delayController.isCallback())
        curFare->setFareCallbackFVO();

      if (fareMarket->firstFareProcessed() < 0)
        fareMarket->setFirstFareProcessed(fareIndex);
      if (delayController.isInitiallSetup())
        fareMarket->setLastFareProcessed(fareIndex);
      fareMarket->addProcessedPTF(curFare);
    }

    if (UNLIKELY((fareMarket->direction() == FMDirection::OUTBOUND) &&
                 (curFare->directionality() == TO)))
    {
      curFare->fare()->setDirectionalityFail();
      curFare->shoppingComponentValidationFailed() = true;
      curFare->setFltIndependentValidationFVO();
      continue;
    }

    curFare->setFltIndependentValidationFVO();

    // Skip validation of fares keep only for routing validation
    if (UNLIKELY(curFare->isKeepForRoutingValidation()))
      continue;

    curFare->shoppingComponentValidationPerformed() = true;
    // set the shoppingComponentValidationFailed
    // if the cat 15 validation fail in the fare state or not all category is valid.
    curFare->shoppingComponentValidationFailed() =
        !(curFare->isCat15SecurityValid() && curFare->areAllCategoryValid());

    if (!isFareValid(trx, curFare, fareMarket))
      continue;

    if (trx.isAltDates())
    {
      if (!curFare->getAltDatePass(*datePair))
        continue;
      ShoppingAltDateUtil::cleanUpCategoryProcess(curFare);
    }

    fareMarket->setGlobalDirection(curFare->globalDirection());
    ruleController.validate(trx, journeyItin.getJourneyItin(), *curFare);

    if (trx.isAltDates())
    {
      if (!curFare->areAllCategoryValid())
        ShoppingAltDateUtil::setAltDateStatus(curFare, *datePair, journeyItin.getLegId());

      if (!(delayController.isActive() && curFare->getAltDatePass(*datePair)))
        continue;
    }

    curFare->shoppingComponentValidationFailed() = !(curFare->areAllCategoryValid());

    // Set valid cat 2 fares to "not processed" for next phase
    // of validation (component with flights rule validation)
    if (curFare->isCategoryValid(2))
      faresPassedCat2.push_back(curFare);

    if (delayController.isActive() && delayController.isInitiallSetup())
    {
      if (isFareValid(trx, curFare, fareMarket))
        ++noOfFaresPassed;

      if (fareMarket->foundPTF() &&
          noOfFaresPassed >= static_cast<int>(delayController.getNumberOfFaresToProcess()))
        break;
    }
  }
}

void
FareValidatorOrchestratorShopping::createFaresFlightBitmaps(
    ShoppingTrx& trx,
    std::vector<PaxTypeFare*>::iterator it,
    std::vector<PaxTypeFare*>::iterator itEnd,
    JourneyItinWrapper& journeyItin)
{
  uint32_t flightSize = journeyItin.getLeg().getFlightBitmapSize(trx, journeyItin.getCarrierKey());
  for (; it != itEnd; ++it)
  {
    PaxTypeFare* curFare(*it);
    if (UNLIKELY(!curFare))
      continue;

    if (UNLIKELY(curFare->isKeepForRoutingValidation()))
    {
      if (curFare->flightBitmap().empty())
        createFlightBitmap(trx, curFare, flightSize, journeyItin.getFareMarket(), journeyItin);
      continue;
    }

    if (!isFareValid(trx, curFare, journeyItin.getFareMarket()))
      continue;

    //#######################################################
    //#Setting this flag effectively disables any call      #
    //#to PaxTypeFare::isValid( ) during the                #
    //# SHOPPING_COMPONENT_WITH_FLIGHTS_VALIDATION phase    #
    //# (i.e. bitmap phase)                                 #
    //# This flag will be reset after bitmap validation     #
    //#######################################################
    curFare->setIsShoppingFare();

    if (LIKELY(curFare->flightBitmap().empty()))
      createFlightBitmap(trx, curFare, flightSize, journeyItin.getFareMarket(), journeyItin);
  }
}

void
FareValidatorOrchestratorShopping::createFlightBitmap(ShoppingTrx& trx,
                                                      PaxTypeFare* fare,
                                                      const uint32_t flightSize,
                                                      FareMarket* fareMarket,
                                                      JourneyItinWrapper& journeyItin)

{
  fare->setFlightBitmapSize(flightSize);
  if (LIKELY(trx.isSumOfLocalsProcessingEnabled()))
  {
    setNotUsedBits(fare->flightBitmap(),
                   (*fareMarket->getApplicableSOPs())[journeyItin.getCarrierKey()]);
  }

  _mainFVO.setFlightBitmapForMultiAirport(trx, *fare, *fareMarket, journeyItin.getCarrierKey());
}

void
FareValidatorOrchestratorShopping::setNotUsedBits(PaxTypeFare::FlightBitmap& bitmap,
                                                  const SOPUsages& usages)
{
  if (UNLIKELY(usages.size() != bitmap.size()))
    return;

  for (size_t i = 0; i < bitmap.size(); ++i)
  {
    if (!usages[i].applicable_)
      bitmap[i]._flightBit = RuleConst::NOT_APPLICABLE;
  }
}

bool
FareValidatorOrchestratorShopping::isFareValid(ShoppingTrx& trx,
                                               const PaxTypeFare* fare,
                                               const FareMarket* fareMarket) const
{
  if (!fare->isValid())
    return false;

  if (UNLIKELY((fareMarket->direction() == FMDirection::OUTBOUND) && (fare->directionality() == TO)))
    return false;

  if (UNLIKELY(trx.getTrxType() == PricingTrx::FF_TRX))
  {
    if ((fareMarket->direction() == FMDirection::INBOUND) && (fare->directionality() == FROM))
      return false;
  }
  return true;
}

//----------------------------------------------------------------------------
// Limitations check
//----------------------------------------------------------------------------
void
FareValidatorOrchestratorShopping::limitationValidation(ShoppingTrx& trx,
                                                        JourneyItinWrapper& journeyItin)
{
  ShoppingTrx::Leg& leg(journeyItin.getLeg());
  ItinIndex& carrierIndex(leg.carrierIndex());
  const uint32_t carrierKey(journeyItin.getCarrierKey());
  ItinIndex::ItinIndexIterator itinIt =
      (leg.stopOverLegFlag())
          ? carrierIndex.beginAcrossStopOverRow(trx, journeyItin.getLegId(), carrierKey)
          : carrierIndex.beginRow(carrierKey);
  ItinIndex::ItinIndexIterator itinItEnd =
      (leg.stopOverLegFlag()) ? carrierIndex.endAcrossStopOverRow() : carrierIndex.endRow();

  for (; itinIt != itinItEnd; ++itinIt)
  {
    TSELatencyData iterMetrics(trx, "FVO CHECK LIMITATIONS");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    const Itin* curItinCellItin = itinIt->second;

    if (UNLIKELY(!curItinCellItin))
      continue;

    if (!journeyItin.applySegments(curItinCellItin, itinIt.bitIndex()))
      continue;

    Itin* journeyItinRaw(&journeyItin.getJourneyItin());
    if (trx.isAltDates())
    {
      DatePair datePair;
      ShoppingTrx::AltDatePairs& altDatePairs =
          (trx.durationAltDatePairs().find(trx.mainDuration()))->second;

      if (!ShoppingAltDateUtil::getDatePair(altDatePairs,
                                            curItinCellItin->travelSeg().front()->departureDT(),
                                            journeyItin.getLegId(),
                                            datePair))
      {
        continue;
      }

      journeyItinRaw = ShoppingAltDateUtil::getJourneyItin(trx.altDatePairs(), datePair);
      if (UNLIKELY(!journeyItinRaw))
        continue;
    } // if (trx.altDates())

    // perform Limitation check
    FareMarket* fareMarket(journeyItin.getFareMarket());
    _mainFVO.performLimitationValidations(trx, journeyItinRaw, fareMarket);
    journeyItin.eraseSegments();
  }
}

void
FareValidatorOrchestratorShopping::processCarrier(
    ShoppingTrx& trx,
    DelayValidationController& delayController,
    JourneyItinWrapper& journeyItin,
    ShpBitValidationCollector::FMValidationSharedData* sharedData)
{
  const TSELatencyData Metrics(trx, "FVO PROCESS CARRIER");

  if (LIKELY(trx.isSumOfLocalsProcessingEnabled()))
    journeyItin.getFareMarket()->setComponentValidationForCarrier(journeyItin.getCarrierKey());

  limitationValidation(trx, journeyItin);
  processCarrierFares(trx, delayController, journeyItin, sharedData);

  _mainFVO.setFareStatus(trx,
                         journeyItin.getFareMarket()->allPaxTypeFare(),
                         journeyItin.getFareMarket()->lastFareProcessed(),
                         trx.mainDuration());
}

bool
FareValidatorOrchestratorShopping::copyAlreadyValidatedBit(PaxTypeFare::FlightBitmap& bitmap,
                                                           ApplicableSOP* applicableSops,
                                                           const uint32_t carrierKey,
                                                           const uint32_t bitIndex)
{
  if (UNLIKELY(!applicableSops))
    return false;

  const SOPUsage& usage((*applicableSops)[carrierKey][bitIndex]);

  if ((uint32_t)usage.flightsFirstProcessedIn_ < bitIndex && usage.flightsFirstProcessedIn_ >= 0)
  {
    bitmap[bitIndex] = bitmap[usage.flightsFirstProcessedIn_];
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
//### ASO Optimization #2/#3 ###//
// Check to see if we have an ASO leg and that the bitmap size warrants the
// invalidation of bits with flights that exceed the configured number
// of connection point
// Check to see if the formulated flight has more than the configured
// connection points limit. If it does, we do not want to use the
// filter logic at all, we will fail it for now
//----------------------------------------------------------------------------
bool
FareValidatorOrchestratorShopping::validateAsoConectionPointLimits(
    const ShoppingTrx& trx,
    PaxTypeFare* fare,
    const uint32_t bitIndex,
    const uint32_t totalTravelSegmentSize)
{
  if (fare->getFlightBitmapSize() >= ((trx.asoBitmapThreshold() > 0)
                                          ? trx.asoBitmapThreshold()
                                          : _asoConxnPointBitmapSizeThreshold))
  {
    if (totalTravelSegmentSize >
        ((trx.asoConxnPointLimit() > 0) ? (trx.asoConxnPointLimit() + 1) : _asoConxnPointMax + 1))
    {
      // Use invalid shopping flight bitmap value to invalidate this bit
      fare->setFlightInvalid(bitIndex, RuleConst::FLIGHT_EXCEEDS_CONXN_POINT_LIMIT);
      return false;
    }
  }
  return true;
}

namespace
{
struct ResetCallbackFlag
{
  void operator()(PaxTypeFare* ptf) { ptf->setFareCallbackFVO(false); }
};
}

void
FareValidatorOrchestratorShopping::processCarrierFares(
    ShoppingTrx& trx,
    DelayValidationController& delayController,
    JourneyItinWrapper& journeyItin,
    ShpBitValidationCollector::FMValidationSharedData* sharedData)
{
  // Set intervals for memory checks
  memCheckInterval = TrxUtil::getShoppingMemCheckInterval(trx);
  perMemGrowthCheckInterval = TrxUtil::getShoppingPercentageMemGrowthCheckInterval(trx);

  if (LIKELY(trx.isSumOfLocalsProcessingEnabled()))
    journeyItin.getFareMarket()->setComponentValidationForCarrier(journeyItin.getCarrierKey());

  delayController.trackFareMarket(*journeyItin.getFareMarket());

  if (!trx.isAltDates())
  {
    processCarrierFaresImpl(trx, delayController, journeyItin, sharedData);
  }
  else
  {
    ShoppingTrx::AltDatePairs& altDatePairs = getAltDatePairs(trx, journeyItin.getLegId());
    for (ShoppingTrx::AltDatePairs::const_iterator altDatePairsIter = altDatePairs.begin();
         altDatePairsIter != altDatePairs.end();
         ++altDatePairsIter)
    {
      JourneyItinWrapper journeyItinNew(trx,
                                        journeyItin.getFareMarket(),
                                        journeyItin.getLeg(),
                                        journeyItin.getLegId(),
                                        journeyItin.getCarrierKey(),
                                        altDatePairsIter->second->journeyItin);
      processCarrierFaresImpl(trx, delayController, journeyItinNew, sharedData);
      delayController.setInitiallSetup(false);
      if (delayController.isActive() && altDatePairs.size() > 1)
        findFaresRequiresMoreValidation(journeyItinNew.getFareMarket(),
                                        journeyItinNew.getDuration());
    }
  }
}

void
FareValidatorOrchestratorShopping::processCarrierFaresImpl(
    ShoppingTrx& trx,
    DelayValidationController& delayController,
    JourneyItinWrapper& journeyItinWrapper,
    ShpBitValidationCollector::FMValidationSharedData* sharedData)
{
  FareMarket* fareMarket(journeyItinWrapper.getFareMarket());
  std::vector<PaxTypeFare*> tempAllPaxTypeFare(fareMarket->allPaxTypeFare().size());
  std::vector<PaxTypeFare*> allPaxTypeFareProcessed;
  std::vector<PaxTypeFare*>::iterator fareIt = fareMarket->allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator fareItEnd = fareMarket->allPaxTypeFare().end();
  bool faresSrcExhausted = false;
  uint32_t carrierKey(journeyItinWrapper.getCarrierKey());

  std::vector<PaxTypeFare*> nonSpecialRtgFares;

  if (LIKELY(delayController.isActive()))
  {
    if (delayController.isInitiallSetup() && fareMarket->specialRtgFound())
    {
      collectNonSpecialRtgFares(*fareMarket, nonSpecialRtgFares);
    }
    if (delayController.isCallback() && delayController.isInitiallSetup())
    {
      if (trx.isAltDates())
      {
        validateNonFltRulesForAltDates(trx, delayController, fareMarket, &carrierKey);
        delayController.setInitiallSetup();
      }
      else
      {
        validateNonFltRulesForFixedDate(trx, delayController, fareMarket, &carrierKey);
      }
    }
    fareIt = fareMarket->allPaxTypeFareProcessed().begin();
    fareItEnd = fareMarket->allPaxTypeFareProcessed().end();

    if (fareIt == fareItEnd)
      return;

    if (delayController.getNumberOfFaresToProcess() > fareMarket->allPaxTypeFareProcessed().size())
      faresSrcExhausted = true;
  }
  else
  {
    if (std::find_if(fareIt, fareItEnd, isSpecialRout<PaxTypeFare*>()) != fareItEnd)
    {
      std::copy(fareIt, fareItEnd, tempAllPaxTypeFare.begin());
      std::stable_partition(
          tempAllPaxTypeFare.begin(), tempAllPaxTypeFare.end(), isNotSpecialRout<PaxTypeFare*>());
      fareIt = tempAllPaxTypeFare.begin();
      fareItEnd = tempAllPaxTypeFare.end();
    }
  }

  ItinIndex& carrierIndex = journeyItinWrapper.getLeg().carrierIndex();

  const bool isStopOverLeg = journeyItinWrapper.getLeg().stopOverLegFlag();
  const size_t iterationsLimit = _mainFVO.computeIterationsLimit(fareIt,
                                                                 fareItEnd,
                                                                 isStopOverLeg,
                                                                 carrierIndex,
                                                                 trx,
                                                                 journeyItinWrapper.getLegId(),
                                                                 carrierKey);

  const int skipThreshold = shoppingSkipThreshold.getValue();
  const bool fareMarketForSpanishDiscount = trx.isSpanishDiscountFM(fareMarket);
  const bool isSkipingBitsAllowed = (trx.diagnostic().diagnosticType() != Diagnostic911) ||
                                    (trx.diagnostic().diagParamMapItem("DD") == "SKIPS");
  std::map<PaxTypeCode, int> faresPassedPerPax;
  int faresPassed = 0, bitsValidated = 0;
  bool isSkippingBits = false;
  uint32_t noOfValidFares = 0;
  uint16_t totalFaresExamined = 0;

  std::vector<PaxTypeFare*> uniqueFares;
  std::vector<PaxTypeFare*> duplicatedFares;

  const bool memoryChangesFallback = Memory::changesFallback;

  while (true)
  {
    for (; fareIt != fareItEnd; ++fareIt)
    {
      bool isFirstValidBit = true;
      if (fallback::reworkTrxAborter(&trx))
        checkTrxAborted(trx);
      else
        trx.checkTrxAborted();
      if (fallback::unifyMemoryAborter(&trx))
      {
        if (!memoryChangesFallback)
          TrxUtil::checkTrxMemoryFlag(trx);
      }

      if (--memCheckInterval == 0)
      {
        // Checks available memory only
        if (memoryChangesFallback)
        {
          TrxUtil::checkTrxMemoryLimits(trx);
        }
        else
        {
          Memory::GlobalManager::instance()->checkTrxMemoryLimits(trx);
        }
        memCheckInterval = TrxUtil::getShoppingMemCheckInterval(trx);
      }

      if (memoryChangesFallback)
      {
        if (--perMemGrowthCheckInterval == 0)
        {
          // Checks percentage memory growth only
          TrxUtil::checkTrxMemoryGrowth(trx);
          perMemGrowthCheckInterval = TrxUtil::getShoppingPercentageMemGrowthCheckInterval(trx);
        }
      }

      PaxTypeFare* curFare = *fareIt;
      bool curFareEligibleForSpanishDiscount = curFare->isSpanishDiscountEnabled();

      if (UNLIKELY(!curFare))
        continue;

      if (LIKELY(trx.isSumOfLocalsProcessingEnabled()))
        curFare->setComponentValidationForCarrier(
            carrierKey, trx.isAltDates(), journeyItinWrapper.getDuration());

      if (curFare->isFlightBitmapInvalid())
        continue;

      bool isBitPassedOrSkipping = isSkippingBits;
      bool isAllSopsInvalid = true;
      // a set of travel segments that will always fail booking code
      // validation for this fare. Check if any segs are in this set
      // before proceeding with other validation.
      std::set<const TravelSeg*> failedTravelSegments;
      const TSELatencyData metrics(trx, "FVO FARE BITMAP VALIDATE");
      ItinIndex::ItinIndexIterator itinIt =
          (isStopOverLeg)
              ? carrierIndex.beginAcrossStopOverRow(trx, journeyItinWrapper.getLegId(), carrierKey)
              : carrierIndex.beginRow(carrierKey);
      ItinIndex::ItinIndexIterator itinItEnd =
          (isStopOverLeg) ? carrierIndex.endAcrossStopOverRow() : carrierIndex.endRow();

      bool isThisFareDuplicated = false;
      if ((trx.getTrxType() == PricingTrx::IS_TRX) && (trx.getRequest()->isExpAccCorpId()))
      {
        for (PaxTypeFare* currUniqueFare : uniqueFares)
        {
          if (UNLIKELY(ShoppingUtil::isDuplicatedFare(currUniqueFare, curFare)))
          {
            isThisFareDuplicated = true;
            curFare->setCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE, false);
            curFare->flightBitmapAllInvalid() = true;
            duplicatedFares.push_back(curFare);
            break;
          }
        }

        if (!isThisFareDuplicated)
          uniqueFares.push_back(curFare);
      }

      curFare->setDurationUsedInFVO(trx.mainDuration());

      if (trx.isSumOfLocalsProcessingEnabled() && trx.isAltDates())
        curFare->setDurationUsedInFVO(journeyItinWrapper.getDuration());

      bool isBitsSkippedDueToFailLimit(false);

      size_t iterationsCount = 0, noOfFailedBits = 0;
      for (; itinIt != itinItEnd; ++itinIt, ++iterationsCount)
      {
        if (UNLIKELY(iterationsLimit != 0 && iterationsCount >= iterationsLimit))
          break;

        // For the first nth fares, determined by _maxApplicableBitsFailLimitCheapestFaresCount,
        // examine _maxApplicableBitsFailLimitCheapestFares instead of
        // _maxApplicableBitsFailCountLimit.
        // Starting at the nth+1 fares, examine _maxApplicableBitsFailCountLimit instead.
        size_t failed_bit_check_count = _maxApplicableBitsFailCountLimit;

        if (totalFaresExamined < _maxApplicableBitsFailLimitCheapestFaresCount)
          failed_bit_check_count = _maxApplicableBitsFailLimitCheapestFares;

        if (UNLIKELY(trx.isSumOfLocalsProcessingEnabled() && failed_bit_check_count &&
                     noOfFailedBits >= failed_bit_check_count))
        {
          isBitsSkippedDueToFailLimit = true;
          break;
        }

        if (UNLIKELY(curFare->flightBitmap().empty()))
          break;

        ItinIndex::ItinCellInfo& cellInfo = itinIt->first;
        Itin* itin = itinIt->second;
        const uint32_t bitIndex = itinIt.bitIndex();

        if (copyAlreadyValidatedBit(
                curFare->flightBitmap(), fareMarket->getApplicableSOPs(), carrierKey, bitIndex))
        {
          if (curFare->isFlightValid(bitIndex))
            noOfFailedBits = 0;
          continue;
        }

        if (!curFare->isFlightValid(bitIndex))
          continue;

        // If this is a duplicated fare, set all sops invalid
        if (UNLIKELY((trx.getTrxType() == PricingTrx::IS_TRX) &&
                     (trx.getRequest()->isExpAccCorpId()) && (isThisFareDuplicated)))
        {
          curFare->setFlightInvalid(bitIndex, RuleConst::DUPLICATED_MULTI_CORPID_FARE);
          continue;
        }

        // Check if this particular fare market and the current pax type fare
        // are eligible for the Spanish large family discount
        if (UNLIKELY((fareMarketForSpanishDiscount) && (curFareEligibleForSpanishDiscount)))
        {
          FareMarket::FMItinMapCI consItr = fareMarket->fMItinMap().find(itin);
          if (consItr == fareMarket->fMItinMap().end())
          {
            if (!LocUtil::isWholeTravelInSpain(itin->travelSeg()))
            {
              fareMarket->addFMItinMap(itin, false);
              curFare->setFlightInvalid(bitIndex, RuleConst::SPANISH_DISCOUNT_NOT_APPLICABLE);
              continue;
            }
            else
              fareMarket->addFMItinMap(itin, true);
          }
          else
          {
            if (!(consItr->second))
            {
              curFare->setFlightInvalid(bitIndex, RuleConst::SPANISH_DISCOUNT_NOT_APPLICABLE);
              continue;
            }
          }
        }
        if (trx.isAltDates())
        {
          DateTime& itinDeparture = itin->travelSeg().front()->departureDT();
          setFBMStatusForAllDuration(
              bitIndex, itinDeparture, journeyItinWrapper.getLegId(), curFare, trx, carrierKey);

          DatePair datePairUsed;
          if (UNLIKELY(!ShoppingAltDateUtil::getDatePair(trx.altDatePairs(),
                                                         itinDeparture,
                                                         journeyItinWrapper.getLegId(),
                                                         datePairUsed)))
          {
            curFare->setFlightInvalid(bitIndex, RuleConst::NO_DATEPAIR_FOUND);
            continue;
          }

          DatePair datePair(*journeyItinWrapper.getJourneyItin().datePair());
          if (!curFare->getAltDatePass(datePair))
          {
            curFare->setFlightInvalid(bitIndex, curFare->getAltDateStatus(datePair));
            continue;
          }
        }

        ++noOfFailedBits;

        if ((isStopOverLeg && bitIndex >= _asoMaxValidations) ||
            (isSkipingBitsAllowed && isBitPassedOrSkipping))
        {
          break;
        }

        if (UNLIKELY(isStopOverLeg))
        {
          if (!validateAsoConectionPointLimits(trx, curFare, bitIndex, itinIt.totalTravelSegSize()))
            continue;
        }

        isAllSopsInvalid = false;
        journeyItinWrapper.eraseSegments();

        if (UNLIKELY(!journeyItinWrapper.applySegments(itin, bitIndex)))
          continue;

        const std::vector<TravelSeg*>& travelSegs =
            (trx.isSumOfLocalsProcessingEnabled() ? journeyItinWrapper.getJourneyItin().travelSeg()
                                                  : itin->travelSeg());
        if (UNLIKELY(!trx.isSumOfLocalsProcessingEnabled()))
        {
          const TSELatencyData metrics(trx, "FVO FARE CHECK BAD TRAVEL SEG");
          std::vector<TravelSeg*>::const_iterator seg = travelSegs.begin();
          std::vector<TravelSeg*>::const_iterator segEnd = travelSegs.end();

          for (; seg != segEnd; ++seg)
          {
            if (failedTravelSegments.count(*seg) != 0)
              break;
          }

          if (seg != segEnd)
          {
            curFare->setFlightInvalid(bitIndex, RuleConst::BOOKINGCODE_FAIL);
            continue;
          }
        }

        const TSELatencyData metrics(trx, "FVO FARE BIT VALIDATE");

        TSE_ASSERT(sharedData);
        sharedData->updateFareMarketData(fareMarket, bitIndex);
        ShoppingRtgMap* rtgMap = sharedData->getRoutingMapForBit(bitIndex);
        std::vector<BCETuning>* bceTunningData = sharedData->getBCEData(bitIndex);

        if (LIKELY(delayController.isActive()))
        {
          if (RoutingUtil::isSpecialRouting(*curFare, true))
            processFareWithPublishedRoutingDelayed(
                trx, journeyItinWrapper, *rtgMap, nonSpecialRtgFares);
        }
        else
        {
          if ((*rtgMap).empty() && RoutingUtil::isSpecialRouting(*curFare, false))
            processFareWithPublishedRouting(trx,
                                            bitIndex,
                                            journeyItinWrapper,
                                            cellInfo,
                                            *rtgMap,
                                            *bceTunningData,
                                            itin,
                                            tempAllPaxTypeFare);
        }

        processFlightBitMap(
            trx, curFare, bitIndex, journeyItinWrapper, cellInfo, *rtgMap, *bceTunningData, itin);

        sharedData->collectFareMarketData(fareMarket, bitIndex);

        if (UNLIKELY(curFare->flightBitmap().empty()))
          continue;

        if (UNLIKELY(!trx.isSumOfLocalsProcessingEnabled()))
        {
          const PaxTypeFare::FlightBit& bitInfo = curFare->flightBitmap()[bitIndex];

          uint32_t startN = 0;
          uint32_t segmentCount = travelSegs.size();

          TSE_ASSERT(bitInfo._segmentStatus.size() <= segmentCount);
          for (uint32_t bitInfoSegIndex = 0; bitInfoSegIndex < bitInfo._segmentStatus.size();
               ++bitInfoSegIndex)
          {
            const uint32_t failFlags =
                PaxTypeFare::BKSS_FAIL | PaxTypeFare::BKSS_FAIL_T999 |
                PaxTypeFare::BKSS_FAIL_REC1_T999 | PaxTypeFare::BKSS_FAIL_CONV1_T999 |
                PaxTypeFare::BKSS_FAIL_CONV2_T999 | PaxTypeFare::BKSS_FAIL_MIXEDCLASS |
                PaxTypeFare::BKSS_FAIL_LOCALMARKET | PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC |
                PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL | PaxTypeFare::BKSS_DIFFERENTIAL |
                PaxTypeFare::BKSS_FAIL_PRIME_RBD_CAT25;
            const uint32_t failedFlag =
                bitInfo._segmentStatus[bitInfoSegIndex]._bkgCodeSegStatus.value() & failFlags;

            if (failedFlag != 0)
            {
              const TSELatencyData metrics(trx, "FVO FARE ADD BAD SEG");
              const TravelSeg* failedSeg = travelSegs[startN + bitInfoSegIndex];
              failedTravelSegments.insert(failedSeg);
            }
          }
        }

        ++bitsValidated;

        if (curFare->isFlightValid(bitIndex))
        {
          isBitPassedOrSkipping = true;
          noOfFailedBits = 0;

          if (LIKELY(isFirstValidBit))
          {
            // For the next iteration(of bitmap), it is no longer first valid bit
            isFirstValidBit = false;

            if (!curFare->isLongConnectFare() &&
                !(trx.legs()[journeyItinWrapper.getLegId()].isSopEmpty()) &&
                trx.legs()[journeyItinWrapper.getLegId()].sop()[cellInfo.sopIndex()].isLngCnxSop())
            {
              // Set PTF flag for long connect if the first valid bit is long connect
              curFare->setLongConnectFare(true);
            }
          }
        }
      }

      journeyItinWrapper.eraseSegments();

      if (itinIt != itinItEnd)
      {
        const uint32_t numBits = curFare->flightBitmap().size();
        for (uint32_t flightBit = itinIt.bitIndex(); flightBit < numBits; ++flightBit)
        {
          if (*curFare->getFlightBit(flightBit) == RuleConst::NOT_APPLICABLE ||
              *curFare->getFlightBit(flightBit) == RuleConst::AIRPORT_NOT_APPLICABLE)
            continue;

          if (isBitsSkippedDueToFailLimit)
          {
            curFare->setFlightInvalid(flightBit, RuleConst::EXCEED_BIT_LIMIT_FAIL);
          }
          else
          {
            if (LIKELY(isSkipingBitsAllowed && isBitPassedOrSkipping &&
                       (!isStopOverLeg || flightBit < _asoTotalMaxValidations)))
              curFare->setFlightInvalid(flightBit, RuleConst::SKIP);
            else
              curFare->setFlightInvalid(flightBit, RuleConst::FLIGHT_EXCEEDS_CONXN_POINT_LIMIT);
          }
        }
      }

      if (UNLIKELY(trx.isSumOfLocalsProcessingEnabled() && isBitPassedOrSkipping &&
                   isSkippingBits && !isAllSopsInvalid))
        ++noOfValidFares;

      if (isSkippingBits == false && isAllSopsInvalid == false)
      {
        if (isBitPassedOrSkipping)
        {
          ++noOfValidFares;
        }

        if (LIKELY(isSkipingBitsAllowed))
        {
          if (isBitPassedOrSkipping)
          {
            ++faresPassed;
            ++faresPassedPerPax[curFare->actualPaxType()->paxType()];
          }

          if (faresPassed > 0 && bitsValidated > 0)
          {
            const int bitsValidatedPerPassedFare = bitsValidated / faresPassed;
            const int solutionsWanted = 1 + skipThreshold / bitsValidatedPerPassedFare;

            if (UNLIKELY(faresPassed >= solutionsWanted))
            {
              isSkippingBits = true;

              // if there are multiple pax types, check that we
              // have enough solutions for each pax type to start skipping
              if (trx.paxType().size() > 1)
              {
                for (std::vector<PaxType*>::const_iterator p = trx.paxType().begin();
                     p != trx.paxType().end();
                     ++p)
                {
                  const int passed = faresPassedPerPax[(*p)->paxType()];
                  if (passed < solutionsWanted)
                  {
                    isSkippingBits = false;
                    break;
                  }
                }
              }
            }
          }
        }
      }
      totalFaresExamined++;
    }

    if (!delayController.isActive() || !delayController.isInitiallSetup())
      break;

    allPaxTypeFareProcessed.insert(allPaxTypeFareProcessed.end(),
                                   fareMarket->allPaxTypeFareProcessed().begin(),
                                   fareMarket->allPaxTypeFareProcessed().end());

    if ((noOfValidFares < delayController.getNumberOfFaresToProcess()) && !faresSrcExhausted)
    {
      const uint32_t faresToProcess =
          (delayController.getNumberOfFaresToProcess() - noOfValidFares);
      DelayValidationController newDelayController(delayController);
      newDelayController.setNumberOfFaresToProcess(faresToProcess);
      newDelayController.setCallback(true);
      fareMarket->clearAllPaxTypeFareProcessed();

      if (trx.isAltDates())
        validateNonFltRulesForAltDates(trx, newDelayController, fareMarket, &carrierKey);
      else
        validateNonFltRulesForFixedDate(trx, newDelayController, fareMarket, &carrierKey);

      if (!delayController.isCallback())
      {
        std::for_each(fareMarket->allPaxTypeFareProcessed().begin(),
                      fareMarket->allPaxTypeFareProcessed().end(),
                      ResetCallbackFlag());
      }

      if (faresToProcess > fareMarket->allPaxTypeFareProcessed().size())
        faresSrcExhausted = true;
    }
    else
    {
      break;
    }

    if (fareMarket->allPaxTypeFareProcessed().size() == 0)
      break;

    fareIt = fareMarket->allPaxTypeFareProcessed().begin();
    fareItEnd = fareMarket->allPaxTypeFareProcessed().end();
  } // end while

  if ((trx.getTrxType() == PricingTrx::IS_TRX) && (trx.getRequest()->isExpAccCorpId()))
  {
    Diagnostic& trxDiag = trx.diagnostic();
    DiagManager diag(trx, trxDiag.diagnosticType());
    bool diag325Enabled = (trx.diagnostic().diagnosticType() == Diagnostic325) &&
                          (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "REC3");

    if (diag325Enabled)
      ShoppingUtil::displayRemovedFares(*fareMarket, uniqueFares.size(), duplicatedFares, diag);
  }

  if (delayController.isActive() && allPaxTypeFareProcessed.size())
  {
    fareMarket->allPaxTypeFareProcessed().clear();
    fareMarket->allPaxTypeFareProcessed().insert(fareMarket->allPaxTypeFareProcessed().end(),
                                                 allPaxTypeFareProcessed.begin(),
                                                 allPaxTypeFareProcessed.end());
  }

  if (fallback::fixed::fallbackRemoveFailedSopsInvalidation())
  {
    // The following code invalidates SOP cabin class
    // using some obscure criteria which is not related
    // to actual cabin class validity of the SOP. We want
    // to disable this logic for IBF requests.
    TSE_ASSERT(trx.getRequest() != nullptr);
    // We cannot invalidate sops if delay controller is active because in this case
    // we don't have all the flight bitmaps available for all the fares
    // and this function relies on this data structure
    if (LIKELY(delayController.isActive() || trx.getRequest()->isBrandedFaresRequest()))
    {
      return;
    }

    if (fareMarket->getFmTypeSol() == FareMarket::SOL_FM_NOTAPPLICABLE ||
        fareMarket->getFmTypeSol() == FareMarket::SOL_FM_THRU)
    {
      FareUtil::invalidateFailedSops(trx, fareMarket, journeyItinWrapper);
    }
  }
}

void
FareValidatorOrchestratorShopping::callback(ShoppingTrx& trx,
                                            FareMarket* fareMarket,
                                            PaxTypeFare* fare,
                                            ItinIndex::Key* carrierKey)
{
  TSELatencyData metrics(trx, "FVO CALLBACK");
  uint32_t legId = fareMarket->legIndex();
  ShoppingTrx::Leg curLeg = trx.legs()[legId];

  JourneyItinWrapper journeyItin(trx, fareMarket, curLeg, legId, *carrierKey);
  DelayValidationController delayController(
      trx, _delayValidationMode, _numberOfFaresToProcess, _numberOfFaresToProcessADI, true, fare);
  delayController.trackFareMarket(*fareMarket);

  if (trx.isSumOfLocalsProcessingEnabled())
    fareMarket->setComponentValidationForCarrier(journeyItin.getCarrierKey());

  ShpBitValidationCollector::FMValidationSharedData* sharedData(
      trx.getBitValidationCollector().getFMSharedData(journeyItin.getCarrierKey(),
                                                      journeyItin.getFareMarket()));
  if (delayController.isActive())
  {
    fareMarket->clearAllPaxTypeFareProcessed();
    fareMarket->setFirstFareProcessed(-1);
    fareMarket->setFoundPTF(false);
  }

  processCarrierFares(trx, delayController, journeyItin, sharedData);
  _mainFVO.setFareStatus(
      trx, fareMarket->allPaxTypeFare(), fareMarket->lastFareProcessed(), trx.mainDuration());
  LOG4CXX_DEBUG(logger, "- setting fare status from callback");
}

void
FareValidatorOrchestratorShopping::processFareWithPublishedRouting(
    ShoppingTrx& trx,
    const uint32_t bitIndex,
    JourneyItinWrapper& journeyItin,
    ItinIndex::ItinCellInfo& curItinCellInfo,
    ShoppingRtgMap& rtMap,
    std::vector<BCETuning>& bceTuningData,
    Itin* curItinCellItin,
    std::vector<PaxTypeFare*>& tempAllPaxTypeFare)
{
  if (_numPublishedRoutingFareToValidate == 0)
    return;

  uint32_t numOfFaresProcessed = 0;
  std::vector<PaxTypeFare*>::iterator fareIt = tempAllPaxTypeFare.begin();
  std::vector<PaxTypeFare*>::iterator fareItEnd = tempAllPaxTypeFare.end();

  for (; fareIt != fareItEnd; ++fareIt)
  {
    PaxTypeFare* curFare = *fareIt;

    if (!curFare || curFare->isFlightBitmapInvalid())
      continue;

    if (RoutingUtil::isSpecialRouting(*curFare, false))
      return;

    // process routing only
    curFare->setKeepForRoutingValidation();
    processFlightBitMap(trx,
                        curFare,
                        bitIndex,
                        journeyItin,
                        curItinCellInfo,
                        rtMap,
                        bceTuningData,
                        curItinCellItin);
    curFare->setKeepForRoutingValidation(false);
    ++numOfFaresProcessed;

    if (numOfFaresProcessed >= _numPublishedRoutingFareToValidate)
      return;
  }
}

void
FareValidatorOrchestratorShopping::processFareWithPublishedRoutingDelayed(
    ShoppingTrx& trx,
    JourneyItinWrapper& journeyItin,
    ShoppingRtgMap& rtMap,
    std::vector<PaxTypeFare*>& nonSpecialRtgFares)
{
  if (nonSpecialRtgFares.empty())
    return;

  RoutingController shoppingRoutingController(trx);

  std::vector<PaxTypeFare*>::iterator fareIt = nonSpecialRtgFares.begin();
  std::vector<PaxTypeFare*>::iterator fareItEnd = nonSpecialRtgFares.end();

  for (; fareIt != fareItEnd; ++fareIt)
  {
    PaxTypeFare& curFare = **fareIt;

    if (curFare.isRoutingProcessed() || !curFare.isValid())
      continue;

    curFare.setKeepForRoutingValidation();
    _bitmapOpOrderer.doPublishedRoutingDelayed(
        trx, shoppingRoutingController, *journeyItin.getFareMarket(), curFare, rtMap);
    curFare.setKeepForRoutingValidation(false);
  }
}

void
FareValidatorOrchestratorShopping::processFlightBitMap(ShoppingTrx& trx,
                                                       PaxTypeFare*& curFare,
                                                       const uint32_t bitIndex,
                                                       JourneyItinWrapper& journeyItin,
                                                       ItinIndex::ItinCellInfo& curItinCellInfo,
                                                       ShoppingRtgMap& rtMap,
                                                       std::vector<BCETuning>& bceTuningData,
                                                       Itin* curItinCellItin)
{
  FareMarket* fareMarket = journeyItin.getFareMarket();

  FareMarketRuleController ruleController =
      (journeyItin.getLeg().stopOverLegFlag() ? trx.ASOfareMarketRuleController()
                                              : trx.fareMarketRuleController());
  FareMarketRuleController cat4ruleController = trx.cat4RuleController();
  CabinType cab;
  cab.setEconomyClass();
  Itin& journeyItinRaw = journeyItin.getJourneyItin();
  _bitmapOpOrderer.performBitmapOperations(trx,
                                           *curFare,
                                           *fareMarket,
                                           bitIndex,
                                           journeyItinRaw,
                                           curItinCellInfo,
                                           (journeyItin.getLeg().preferredCabinClass() < cab),
                                           ruleController,
                                           cat4ruleController,
                                           rtMap,
                                           bceTuningData);
}

class FailedUnusedFareShopping final
{
public:
  FailedUnusedFareShopping(ShoppingTrx& trx, DiagCollector& diag) : _trx(trx), _diag(diag) {}

  bool operator()(const PaxTypeFare* ptFare)
  {
    if (ptFare->cat25BasePaxFare())
      return false;

    if (!ptFare->isNoDataMissing())
    {
      if (_diag.isActive())
        _diag << toString(*ptFare) << "\n";
      return true;
    }

    // Release invalid fares for SOL ShoppingTrx.
    if (!ptFare->isValid())
    {
      if (_diag.isActive())
      {
        const size_t numCategories = 50;
        size_t category;

        for (category = 1;
             (category < numCategories) && (true == ptFare->isCategoryValid(category));
             category++)
          ;

        if (category < numCategories)
          _diag << toString(*ptFare) << " CAT" << category << "\n\n";
        else
          _diag << toString(*ptFare) << " CAT-UNT"
                << "\n\n";
      }
      return true;
    }
    return false;
  }

  std::string toString(const PaxTypeFare& paxFare)
  {
    std::ostringstream dc;
    std::string gd;
    globalDirectionToStr(gd, paxFare.fare()->globalDirection());
    std::string fareBasis = paxFare.createFareBasis(_trx, false);

    if (fareBasis.size() > 12)
      fareBasis = fareBasis.substr(0, 12) + "*";

    dc << std::setw(2) << _diag.cnvFlags(paxFare) << std::setw(3) << gd << std::setw(2)
       << (paxFare.vendor() == Vendor::ATPCO ? "A" : (paxFare.vendor() == Vendor::SITA ? "S" : "?"))
       << std::setw(5) << paxFare.ruleNumber() << std::setw(13) << fareBasis << std::setw(4)
       << paxFare.fareTariff() << std::setw(2)
       << (paxFare.owrt() == ONE_WAY_MAY_BE_DOUBLED
               ? "X"
               : (paxFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED
                      ? "R"
                      : (paxFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED ? "O" : " "))) << std::setw(2)
       << (paxFare.directionality() == FROM ? "O" : (paxFare.directionality() == TO ? "I" : " "))
       << std::setw(12) << Money(paxFare.fareAmount(), paxFare.currency()) << std::setw(4)
       << (paxFare.fcasPaxType().empty() ? "***" : paxFare.fcasPaxType());
    return dc.str();
  }

private:
  ShoppingTrx& _trx;
  DiagCollector& _diag;
};

void
FareValidatorOrchestratorShopping::releaseUnusedFares(ShoppingTrx& trx)
{
  // Check for WPNETT
  if (trx.getRequest()->isWpNettRequested())
    return;

  if (UNLIKELY((trx.diagnostic().diagnosticType() == Diagnostic911) &&
               (trx.diagnostic().diagParamMapItem("DD") == "SKIPS")))
  {
    return;
  }

  DiagMonitor diag(trx, Diagnostic469);
  bool diagEnabled = diag.diag().isActive();
  FailedUnusedFareShopping gFailedFare(trx, diag.diag());
  const bool releaseFares = releaseUnusedFaresAllItins.getValue();

  for (Itin* itin : trx.itin())
  {
    for (FareMarket* fmPtr : itin->fareMarket())
    {
      if (UNLIKELY(!fmPtr))
        continue;

      FareMarket& fareMarket = *fmPtr;

      if (!fareMarket.specialRtgFound())
        continue;

      if (diagEnabled)
      {
        diag << "\nRELEASE FARES UNUSED BY SOL SHOPPING TRX AT FVO:\n";
        diag << "FARE MARKET: " << fareMarket.origin()->loc() << "-"
             << fareMarket.governingCarrier() << "-" << fareMarket.destination()->loc() << "\n";
        const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

        if (!paxTypeCortegeVec.empty())
        {
          if (paxTypeCortegeVec.begin() != paxTypeCortegeVec.end())
          {
            const PaxTypeBucket& cortege = *(paxTypeCortegeVec.begin());
            diag << "REQUESTED PAXTYPE: " << cortege.requestedPaxType()->paxType() << "\n";
          }
        }

        diag << "\n";
        diag << "   GI V RULE   FARE BASIS TRF O O         AMT CUR PAX RULE \n";
        diag << "                          NUM R I                 TYP FAILED\n";
        diag << "-- -- - ---- ------------ --- - - ----------- --- --- ------\n";
      }

      std::vector<PaxTypeFare*>& allPaxTypeFares = fareMarket.allPaxTypeFare();

      try
      {
        std::vector<PaxTypeFare*>::iterator riter =
            remove_if(allPaxTypeFares.begin(), allPaxTypeFares.end(), gFailedFare);

        if (riter != allPaxTypeFares.end())
        {
          int total = allPaxTypeFares.size();
          int released = std::distance(riter, allPaxTypeFares.end());
          allPaxTypeFares.erase(riter, allPaxTypeFares.end());

          if (diagEnabled)
          {
            diag << "RELEASED " << released << " OF " << total << " FARES FROM ALLPAXTYPEFARE\n \n";
          }

          // remove fares from corteges
          // enabled diagnostic to not duplicate fares while removing from corteges
          diag.diag().disable(Diagnostic469);
          std::vector<PaxTypeBucket>::iterator cortIt = fareMarket.paxTypeCortege().begin();
          std::vector<PaxTypeBucket>::iterator cortEnd = fareMarket.paxTypeCortege().end();

          for (; cortIt != cortEnd; ++cortIt)
          {
            std::vector<PaxTypeFare*>& cortegeFares = cortIt->paxTypeFare();
            cortegeFares.erase(remove_if(cortegeFares.begin(), cortegeFares.end(), gFailedFare),
                               cortegeFares.end());
          }

          diag.diag().enable(Diagnostic469);
        }
      }
      catch (std::exception& e)
      {
        LOG4CXX_ERROR(logger, "Exception: " << e.what());
      }
      catch (...)
      {
        LOG4CXX_ERROR(logger, "Unknown Exception");
      }
    }

    if (!releaseFares)
      break;
  }
}

//----------------------------------------------------------------------------
// validate non flight related rule and create flight bit map for alt date
//----------------------------------------------------------------------------
void
FareValidatorOrchestratorShopping::validateNonFltRulesForAltDates(
    ShoppingTrx& trx,
    DelayValidationController& delayController,
    FareMarket* fareMarketCallback,
    ItinIndex::Key* cxrKey)
{
  LOG4CXX_DEBUG(logger, "FareValidatorOrchestratorShopping::validateNonFltRulesForAltDates...");
  TSELatencyData metrics(trx, "FVO VALIDATE NON FLT RULES FOR ALTDATES");

  ShoppingTrx::AltDatePairs::reverse_iterator altDatePairIter = trx.altDatePairs().rbegin();
  ShoppingTrx::AltDatePairs::reverse_iterator altDatePairIterEnd = trx.altDatePairs().rend();

  for (; altDatePairIter != altDatePairIterEnd; ++altDatePairIter)
  {
    Itin* journeyItin = altDatePairIter->second->journeyItin;
    validateNonFltRules(trx, delayController, journeyItin, fareMarketCallback, cxrKey);
    delayController.setInitiallSetup(false);
  }
}

//-------------------------------------------------------------------
// set flightbitmap status from non flight related rule validation
//-------------------------------------------------------------------
void
FareValidatorOrchestratorShopping::setFBMStatusForAllDuration(const uint32_t& bitIndex,
                                                              const DateTime& dateTime,
                                                              uint8_t legId,
                                                              PaxTypeFare* paxTypeFare,
                                                              ShoppingTrx& trx,
                                                              uint32_t carrierKey)
{
  if (UNLIKELY(paxTypeFare->durationFlightBitmapPerCarrier().empty()))
    return;

  VecMap<uint64_t, PaxTypeFare::FlightBitmap>* pFlightBitmapPerDuration =
      &(paxTypeFare->durationFlightBitmapPerCarrier()[carrierKey]);

  if (UNLIKELY(pFlightBitmapPerDuration->empty()))
    return;

  VecMap<uint64_t, PaxTypeFare::FlightBitmap>::iterator iter = pFlightBitmapPerDuration->begin();
  VecMap<uint64_t, PaxTypeFare::FlightBitmap>::iterator iterEnd = pFlightBitmapPerDuration->end();

  for (; iter != iterEnd; iter++)
  {
    PricingTrx::DurationAltDatePairs::const_iterator i =
        trx.durationAltDatePairs().find(iter->first);

    if (UNLIKELY(i == trx.durationAltDatePairs().end()))
    {
      paxTypeFare->setFlightInvalid(iter->second, bitIndex, RuleConst::NO_DATEPAIR_FOUND);
      continue;
    }

    DatePair datePair;

    if (!ShoppingAltDateUtil::getDatePair(i->second, dateTime, legId, datePair))
    {
      paxTypeFare->setFlightInvalid(iter->second, bitIndex, RuleConst::NO_DATEPAIR_FOUND);
      continue;
    }

    if (!paxTypeFare->getAltDatePass(datePair))
      paxTypeFare->setFlightInvalid(
          iter->second, bitIndex, paxTypeFare->getAltDateStatus(datePair));
  }
}

bool
FareValidatorOrchestratorShopping::checkCarriersFlightBitmapsStatus(const PaxTypeFare* ptf,
                                                                    uint64_t mainDuration) const
{
  if (ptf->flightBitmapPerCarrier().size() == 0 &&
      ptf->durationFlightBitmapPerCarrier().size() == 0) // Not SOL
    return ptf->isFlightBitmapInvalid();

  // SOL processing

  // Check if more carriers on their way before we could finally say a PTF is valid or not so we
  // cannot invalid this one now
  if (ptf->fareMarket()->getApplicableSOPs()->size() > ptf->flightBitmapPerCarrier().size() &&
      ptf->fareMarket()->getApplicableSOPs()->size() > ptf->durationFlightBitmapPerCarrier().size())
    return false;

  // Check if at least one flight bitmap valid so PTF stays valid

  if (!ptf->durationFlightBitmapPerCarrier().empty())
  {
    VecMap<uint32_t, VecMap<uint64_t, PaxTypeFare::FlightBitmap>>::const_iterator carrierIter =
        ptf->durationFlightBitmapPerCarrier().begin();
    for (; carrierIter != ptf->durationFlightBitmapPerCarrier().end(); ++carrierIter)
    {
      if (checkFlightBitmapsValidity(carrierIter->second, ptf))
        return false;
    }
  }
  else
    return !checkFlightBitmapsValidity(ptf->flightBitmapPerCarrier(), ptf);

  return true;
}

template <typename T>
bool
FareValidatorOrchestratorShopping::checkFlightBitmapsValidity(
    const VecMap<T, PaxTypeFare::FlightBitmap>& collection, const PaxTypeFare* ptf) const
{
  typename VecMap<T, PaxTypeFare::FlightBitmap>::const_iterator iter = collection.begin();

  for (; iter != collection.end(); ++iter)
    if (!ptf->isFlightBitmapInvalid(iter->second))
      return true;
  return false;
}

ShoppingTrx::AltDatePairs&
FareValidatorOrchestratorShopping::getAltDatePairs(ShoppingTrx& trx, const uint8_t legId) const
{
  return trx.altDatePairs();
}

void
FareValidatorOrchestratorShopping::findFaresRequiresMoreValidation(FareMarket* fareMarket,
                                                                   uint64_t duration) const
{
  const uint8_t BitValid(0);
  const uint8_t BitSkipped('S');

  std::vector<PaxTypeFare*> temporaryFareContainer;
  for (PaxTypeFare* paxTypeFare : fareMarket->allPaxTypeFareProcessed())
    if (paxTypeFare->isFlightBitmapInvalid())
      temporaryFareContainer.push_back(paxTypeFare);

  if (temporaryFareContainer.size() == fareMarket->allPaxTypeFareProcessed().size())
    return;

  for (PaxTypeFare* paxTypeFare : temporaryFareContainer)
  {
    fareMarket->allPaxTypeFareProcessed().erase(
        std::find(fareMarket->allPaxTypeFareProcessed().begin(),
                  fareMarket->allPaxTypeFareProcessed().end(),
                  paxTypeFare));
  }
  fareMarket->allPaxTypeFareProcessed().swap(temporaryFareContainer);

  for (PaxTypeFare* paxTypeFare : temporaryFareContainer)
  {
    VecMap<uint64_t, PaxTypeFare::FlightBitmap>::iterator durIter =
        paxTypeFare->durationFlightBitmap().begin();
    for (; durIter != paxTypeFare->durationFlightBitmap().end(); ++durIter)
    {
      if (duration == durIter->first)
        continue;
      PaxTypeFare::FlightBitmap& flightBitmap = durIter->second;
      if (paxTypeFare->isFlightBitmapInvalid(flightBitmap))
        continue;

      PaxTypeFare::FlightBitmap::iterator fbIter = flightBitmap.begin();
      for (; fbIter != flightBitmap.end(); ++fbIter)
        if (fbIter->_flightBit == BitValid)
          fbIter->_flightBit = BitSkipped;
    }
  }
}

void
FareValidatorOrchestratorShopping::collectNonSpecialRtgFares(
    FareMarket& fareMarket, std::vector<PaxTypeFare*>& nonSpecialRtgFares)
{
  nonSpecialRtgFares.clear();

  if (_numPublishedRoutingFareToValidateDelayed == 0 || fareMarket.allPaxTypeFare().size() == 0)
    return;

  std::vector<PaxTypeFare*>::iterator fareIt = fareMarket.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator fareItEnd = fareMarket.allPaxTypeFare().end();

  uint32_t noOfNotSpecialFares = std::count_if(fareIt, fareItEnd, isNotSpecialRout<PaxTypeFare*>());

  uint32_t maxNoOfFares = std::min(noOfNotSpecialFares, _numPublishedRoutingFareToValidateDelayed);

  nonSpecialRtgFares.resize(maxNoOfFares);

  fareIt = fareMarket.allPaxTypeFare().begin();

  for (size_t idx = 0; fareIt != fareItEnd; ++fareIt)
  {
    PaxTypeFare* curFare = *fareIt;
    if (curFare && !RoutingUtil::isSpecialRouting(*curFare, true))
    {
      nonSpecialRtgFares[idx++] = curFare;
      if (idx >= maxNoOfFares)
        return;
    }
  }
}
} // namespace tse
