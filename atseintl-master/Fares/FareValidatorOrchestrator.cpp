//--------------------------------------------------------------------
//
//  File:        FareValidatorOrchestrator.cpp
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
//--------------------------------------------------------------------

#include "Fares/FareValidatorOrchestrator.h"

#include "BookingCode/BCETuning.h"
#include "BookingCode/FareBookingCodeValidator.h"
#include "BookingCode/MixedClassController.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandingRequestResponseHandler.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/AltPricingUtil.h"
#include "Common/BrandingUtil.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/DiagMonitor.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/FxCnException.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
#include "Common/RoutingUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TNBrands/TNBrandsFunctions.h"
#include "Common/TSEAlgorithms.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "Common/TseSrvStats.h"
#include "Common/TseUtil.h"
#include "Common/ValidatingCxrUtil.h"
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
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TNBrandsTypes.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/ContractPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/MultiTransport.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag460Collector.h"
#include "Diagnostic/Diag892Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Fares/AdjustedSellingLevelFareCollector.h"
#include "Fares/AltDatesPairOptionsBounder.h"
#include "Fares/BitmapOpOrderer.h"
#include "Fares/BooleanFlagResetter.h"
#include "Fares/BrandedFareValidator.h"
#include "Fares/FareByRuleController.h"
#include "Fares/FareCollectorOrchestrator.h"
#include "Fares/FareUtil.h"
#include "Fares/FareValidatorOrchestratorESV.h"
#include "Fares/FDFareCurrencySelection.h"
#include "Fares/FlightFinderJourneyValidator.h"
#include "Fares/RoutingController.h"
#include "Fares/RuleCategoryProcessor.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Routing/MileageInfo.h"
#include "Rules/DayTimeApplication.h"
#include "Rules/FareMarketDataAccess.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/FDFareMarketRuleController.h"
#include "Rules/FDSurchargesRule.h"
#include "Rules/FlightApplication.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Server/TseServer.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include <boost/algorithm/string/trim.hpp>
#include <boost/next_prior.hpp>

namespace tse
{
static LoadableModuleRegister<Service, FareValidatorOrchestrator>
_("libFaresValidator.so");

FIXEDFALLBACK_DECL(fallbackRemoveFailedSopsInvalidation)
FALLBACK_DECL(compDiffCmdPricing)
FALLBACK_DECL(fallbackPriceByCabinActivation)
FALLBACK_DECL(skipMixedClassForExcItin)
FALLBACK_DECL(fallbackFRRProcessingRetailerCode)
FALLBACK_DECL(keepCat11ResultInIs)
FALLBACK_DECL(reworkTrxAborter)
FALLBACK_DECL(excPrivPubNoAtpFares)

uint32_t FareValidatorOrchestrator::_minSizeToRemoveDuplicatedFares = 1000;

namespace
{
bool _newRouting = false;

const uint32_t MAX_ASOITIN_CONXN_POINT_DEFAULT = 4;
const uint32_t MAX_ASOBITMAP_SIZE_THRESHOLD_DEFAULT = 2000;
const uint32_t MAX_ASO_VALIDATIONS_DEFAULT = 100000;
const uint32_t MAX_TOTAL_ASO_VALIDATIONS_DEFAULT = 100000;
const uint32_t NUM_PUBLISHED_ROUTING_FARE_TO_VALIDATE_DEFAULT = 1;

ConfigurableValue<uint32_t>
carrierFaresLoopLimit("SHOPPING_SVC", "CARRIER_FARES_LOOP_LIMIT", 0);
ConfigurableValue<uint32_t>
shoppingSkipThreshold("FARESV_SVC", "SHOPPING_SKIP_THRESHOLD", 10000);
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
numberOfFaresToProcessADI("FARESV_SVC",
                          "SHOPPING_ALTDATE_NUMBER_OF_FARES_TO_PROCESS_THRESHOLD",
                          DELAY_VALIDATION_OFF);
ConfigurableValue<uint32_t>
minSizeToRemoveDuplicatedFares("FARESV_SVC", "MIN_SIZE_TO_REMOVE_DUP_FARES", 1000);
ConfigurableValue<bool>
releaseUnusedFaresAllItins("SHOPPING_OPT", "RELEASE_UNUSED_FARES_ALL_ITINS");
ConfigurableValue<bool>
newRtg("FARESV_SVC", "NEW_ROUTING");

inline PaxTypeFare*
getBaseFareFBR(PaxTypeFare* curFare)
{
  auto allRuleData = (*curFare->paxTypeFareRuleDataMap())[25].load(std::memory_order_relaxed);
  if (allRuleData == nullptr)
    return nullptr;
  return allRuleData->fareRuleData->baseFare();
}

inline void
setBaseFareCategoryValid(PaxTypeFare* curFare, const unsigned int category)
{
  PaxTypeFare* baseFare(getBaseFareFBR(curFare));
  if (baseFare)
    baseFare->setCategoryValid(category);
}
Itin*
getCurrCellItinCheckNothing(ShoppingTrx& trx, const uint32_t legId, const ItinIndex::Key& cxrKey)
{
  if (auto itinCell =
          ShoppingUtil::retrieveDirectItin(trx, legId, cxrKey, ItinIndex::CHECK_NOTHING))
    return itinCell->second;
  return nullptr;
}
FareMarket*
getFirstFareMarket(ShoppingTrx& trx, const uint32_t legId, const ItinIndex::Key& cxrKey)
{
  if (auto curItinCellItin = getCurrCellItinCheckNothing(trx, legId, cxrKey))
    return curItinCellItin->fareMarket().front();
  return nullptr;
}

} // namespace

static Logger
logger("atseintl.Fares.FareValidatorOrchestrator");

ProcessCarrierTask::ProcessCarrierTask(
    FareValidatorOrchestrator& fvo,
    ShoppingTrx& shoppingTrx,
    const ItinIndex::Key& cxrKey,
    uint32_t legId,
    ShoppingTrx::Leg& curLeg,
    Itin* journeyItin,
    ShoppingTrx::AltDatePairs& altDatesMap,
    FareMarketRuleController& ruleController,
    FareMarketRuleController& cat4ruleController,
    FareMarket* fM,
    std::vector<std::vector<BCETuning>>& shoppingBCETuningData, // DEPRECATED
    ShpBitValidationCollector::FMValidationSharedData* sharedData,
    const uint32_t numberOfFaresToProcess)
  : _fvo(&fvo),
    _trx(&shoppingTrx),
    _cxrKey(&cxrKey),
    _legId(legId),
    _curLeg(&curLeg),
    _journeyItin(journeyItin),
    _altDatesMap(altDatesMap),
    _ruleController(&ruleController),
    _cat4ruleController(&cat4ruleController),
    _fM(fM),
    _shoppingBCETuningData(&shoppingBCETuningData),
    _sharedData(sharedData),
    _numberOfFaresToProcess(numberOfFaresToProcess)
{
  trx(&shoppingTrx);
}

void
ProcessCarrierTask::performTask()
{
  _fvo->processCarrier(*_trx,
                       *_cxrKey,
                       _legId,
                       *_curLeg,
                       _journeyItin,
                       _altDatesMap,
                       *_ruleController,
                       *_cat4ruleController,
                       _fM,
                       *_shoppingBCETuningData,
                       _sharedData,
                       _numberOfFaresToProcess);
}

//----------------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------------
FareValidatorOrchestrator::FareValidatorOrchestrator(const std::string& name, TseServer& server)
  : FareOrchestrator(name, server, TseThreadingConst::FARESV_TASK),
    _bitmapOpOrderer(server.config()),
    _asoConxnPointMax(MAX_ASOITIN_CONXN_POINT_DEFAULT),
    _asoConxnPointBitmapSizeThreshold(MAX_ASOBITMAP_SIZE_THRESHOLD_DEFAULT),
    _asoMaxValidations(MAX_ASO_VALIDATIONS_DEFAULT),
    _asoTotalMaxValidations(MAX_TOTAL_ASO_VALIDATIONS_DEFAULT),
    _numPublishedRoutingFareToValidate(NUM_PUBLISHED_ROUTING_FARE_TO_VALIDATE_DEFAULT),
    _numberOfFaresToProcessADI(DELAY_VALIDATION_OFF),
    _shoppingFVO(*this, server)
{
  _asoConxnPointBitmapSizeThreshold = asoConxnPointBitmapSizeThreshold.getValue();
  _asoConxnPointMax = asoConxnPointMax.getValue();
  _asoMaxValidations = asoMaxValidations.getValue();
  _asoTotalMaxValidations = asoTotalMaxValidations.getValue();
  _numPublishedRoutingFareToValidate = numPublishedRoutingFareToValidate.getValue();
  _numberOfFaresToProcessADI = numberOfFaresToProcessADI.getValue();
  _minSizeToRemoveDuplicatedFares = minSizeToRemoveDuplicatedFares.getValue();
}

bool
FareValidatorOrchestrator::process(ShoppingTrx& trx)
{
  bool bResult(false);

  if (PricingTrx::ESV_TRX == trx.getTrxType())
  {
    FareValidatorOrchestratorESV fvoESV(trx, _server);
    bResult = fvoESV.process();
  }
  else if (trx.isSumOfLocalsProcessingEnabled())
  {
    bResult = _shoppingFVO.process(trx);
  }
  else
  {
    bResult = processIS(trx);
  }

  countFares(trx);
  return bResult;
}

bool
FareValidatorOrchestrator::processIS(ShoppingTrx& trx)
{
  const TSELatencyData metrics(trx, "FVO PROCESS");
  LOG4CXX_DEBUG(logger, "FareValidatorOrchestrator::process(ShoppingTrx)...");
  bool delayValidationToApply = (trx.paxType().size() <= 1) && trx.isAltDates();
  const uint32_t numberOfFaresToProcess =
      delayValidationToApply ? _numberOfFaresToProcessADI : DELAY_VALIDATION_OFF;

  if (trx.isAltDates())
  {
    if (trx.isSimpleTrip() && trx.isRuleTuningISProcess())
    {
      uint16_t newCat3 = RuleConst::SEASONAL_RULE;
      trx.addCat(newCat3);
    }

    validateNonFltRulesForAltDates(trx, nullptr, nullptr, nullptr, numberOfFaresToProcess);
  }
  else
  {
    validateNonFltRules(trx);
  }

  std::vector<ShoppingTrx::Leg>& legs = trx.legs();

  if (legs.empty())
  {
    return false;
  }

  // get the journey itin
  Itin*& journeyItin = trx.journeyItin();
  journeyItin->validatingCarrier().clear();
  std::vector<TravelSeg*> travelSegVec = journeyItin->travelSeg();
  std::deque<ProcessCarrierTask> tasks;
  std::deque<FareMarketRuleController> ruleControllers;
  std::deque<FareMarketRuleController> cat4ruleControllers;
  std::deque<Itin> journeyItins;
  std::deque<ShoppingTrx::AltDatePairs> altDateJourneyItins;
  ShoppingTrx::AltDatePairs* altDatePairs = &(trx.altDatePairs());
  std::set<FareMarket*> uniqueFareMarkets;
  CarrierTasksForFareMarket tasksForAdditionalSolFareMarkets;
  ShpBitValidationCollector& shareCollector = trx.getBitValidationCollector();
  // Create leg vector iterators
  uint32_t legId = 0;
  for (auto& curLeg : legs)
  {
    bool isStopOverLeg = curLeg.stopOverLegFlag();
    for (auto& itinMatrix : curLeg.carrierIndex().root())
    {
      Itin* thruFareDirectItin = getCurrCellItinCheckNothing(trx, legId, itinMatrix.first);

      if (!thruFareDirectItin)
        continue;

      ShoppingTrx::FareMarketRuleMap& fmrm = trx.fareMarketRuleMap();
      std::vector<FareMarket*>& fareMarkets(thruFareDirectItin->fareMarket());

      for (FareMarket* fM : fareMarkets)
      {
        if (!fM)
          continue;

        // construct the map of journey itins for alternate dates
        altDateJourneyItins.push_back(ShoppingTrx::AltDatePairs());

        for (ShoppingTrx::AltDatePairs::const_iterator i = altDatePairs->begin();
             i != altDatePairs->end();
             ++i)
        {
          journeyItins.push_back(*(i->second)->journeyItin);
          PricingTrx::AltDateInfo*& info = altDateJourneyItins.back()[i->first];
          info = trx.dataHandle().create<PricingTrx::AltDateInfo>();
          info->journeyItin = &journeyItins.back();
        }

        ShpBitValidationCollector::FMValidationSharedData* sharedData(
            shareCollector.getFMSharedData(itinMatrix.first, fM));

        // add the main journey itin
        journeyItins.push_back(*journeyItin);
        ruleControllers.push_back(isStopOverLeg ? trx.ASOfareMarketRuleController()
                                                : trx.fareMarketRuleController());
        cat4ruleControllers.push_back(trx.cat4RuleController());
        ProcessCarrierTask task(*this,
                                trx,
                                itinMatrix.first,
                                legId,
                                curLeg,
                                &journeyItins.back(),
                                altDateJourneyItins.back(),
                                ruleControllers.back(),
                                cat4ruleControllers.back(),
                                fM,
                                fmrm[fM]._shoppingBCETuningData, // insert to the fmrm map here
                                sharedData);

        if (uniqueFareMarkets.insert(fM).second)
          tasks.push_back(task);
        else
          tasksForAdditionalSolFareMarkets[fM].push_back(task);

        if (!trx.isIataFareSelectionApplicable())
          break;
      }
    }

    // Increment leg id
    ++legId;
  }

  const TSELatencyData metricsThreads(trx, "FVO BITMAP VALIDATION");
  processTasks(tasks);

  if (!tasksForAdditionalSolFareMarkets.empty())
  {
    processAdditionalFareMarketsForSOL(tasksForAdditionalSolFareMarkets);
  }

  if (smp::isPenaltyCalculationRequired(trx))
  {
    exec_foreach_valid_fareMarket(TseThreadingConst::SHOPPING_TASK,
                                  trx,
                                  smp::preValidateFareMarket,
                                  FareMarket::FareValidator);
  }

  // restore the journey itin's travel segs to their original value
  journeyItin->travelSeg() = travelSegVec;
  // Temporary turn off for IS
  // removeFaresKeepForRoutingValidation(trx);
  // Release unused fares
  releaseUnusedFares(trx);
  return true;
}

void
FareValidatorOrchestrator::processAdditionalFareMarketsForSOL(
    CarrierTasksForFareMarket& fareMarketsTasks)
{
  int tasksToProcess = 1;

  while (tasksToProcess > 0)
  {
    std::deque<ProcessCarrierTask> tasks;
    for (auto& fmTask : fareMarketsTasks)
    {
      if (!fmTask.second.empty())
      {
        tasks.push_back(std::move(fmTask.second.front()));
        fmTask.second.pop_front();
      }
    }

    processTasks(tasks);
    tasksToProcess = tasks.size();
  }
}

void
FareValidatorOrchestrator::processTasks(std::deque<ProcessCarrierTask>& tasks)
{
  TseRunnableExecutor taskExecutor(TseThreadingConst::SHOPPING_TASK);

  for (ProcessCarrierTask& task : tasks)
    taskExecutor.execute(task);

  taskExecutor.wait();
}

void
FareValidatorOrchestrator::prepareJourneyItin(Itin* journeyItin,
                                              ShoppingTrx::AltDatePairs* altDatesMap,
                                              const uint32_t& beginLeg,
                                              uint32_t& endLeg)
{
  // erase the travel seg in the journey itin for the current leg
  journeyItin->travelSeg().erase(journeyItin->travelSeg().begin() + beginLeg,
                                 journeyItin->travelSeg().begin() + endLeg);

  if (!altDatesMap)
    return;

  for (auto& altDate : *altDatesMap)
  {
    Itin* altDateJourneyItin = altDate.second->journeyItin;
    altDateJourneyItin->travelSeg().erase(altDateJourneyItin->travelSeg().begin() + beginLeg,
                                          altDateJourneyItin->travelSeg().begin() + endLeg);
  }
}

void
FareValidatorOrchestrator::countFares(PricingTrx& trx)
{
  const auto validFares = trx.getValidForPOFaresCount();
  TseSrvStats::recordNumberValidatedFares(trx, validFares);
}

void
FareValidatorOrchestrator::prepareShoppingValidation(ShoppingTrx& trx,
                                                     Itin* journeyItin,
                                                     const ItinIndex::ItinCell& itinCell,
                                                     FareMarket* fareMarket,
                                                     FareValidatorOrchestrator::FMBackup& fmb,
                                                     const uint32_t& beginLeg,
                                                     uint32_t& endLeg)
{
  const Itin* itin = itinCell.second;
  // replace the travel seg in the journey itin with the real flight
  // for the current leg
  /*    journeyItin->travelSeg().erase(
        journeyItin->travelSeg().begin()+ beginLeg,
        journeyItin->travelSeg().begin()+ endLeg);  */
  journeyItin->travelSeg().insert(journeyItin->travelSeg().begin() + beginLeg,
                                  itin->travelSeg().begin(),
                                  itin->travelSeg().end());
  endLeg = itin->travelSeg().size() + beginLeg;
  // set fare market travel seg with the current itin travel seg
  // back up the fare market's travel segs to be restored in cleanup
  fareMarket->travelSeg().swap(fmb.fMBackupSegs);
  fareMarket->travelSeg() = itin->travelSeg();

  // attach class of service to fare market
  std::vector<std::vector<ClassOfService*>*>& classOfServiceVec = fareMarket->classOfServiceVec();
  classOfServiceVec.clear();
  classOfServiceVec.reserve(fareMarket->travelSeg().size());
  for (TravelSeg* travelSeg : fareMarket->travelSeg())
  {
    classOfServiceVec.push_back(&(travelSeg->classOfService()));
  }

  fmb.globalDirection = fareMarket->getGlobalDirection();

  fmb.primarySector = fareMarket->primarySector();
  fareMarket->primarySector() = itinCell.first.getPrimarySector();
}

void
FareValidatorOrchestrator::cleanupAfterShoppingValidation(ShoppingTrx& trx,
                                                          Itin* journeyItin,
                                                          FareMarket* fareMarket,
                                                          FareValidatorOrchestrator::FMBackup& fmb,
                                                          const uint32_t beginLeg,
                                                          const uint32_t endLeg)
{
  journeyItin->travelSeg().erase(journeyItin->travelSeg().begin() + beginLeg,
                                 journeyItin->travelSeg().begin() + endLeg);
  fareMarket->travelSeg().swap(fmb.fMBackupSegs);

  fareMarket->setGlobalDirection(fmb.globalDirection);

  fareMarket->primarySector() = fmb.primarySector;
}

void
FareValidatorOrchestrator::performBitmapCatValidations(PaxTypeFare* curFare,
                                                       FareMarket* fM,
                                                       FareMarketRuleController& ruleController,
                                                       const uint32_t& bitIndex,
                                                       PricingTrx& pricingTrx,
                                                       Itin* journeyItin,
                                                       ItinIndex::ItinCellInfo& curItinCellInfo)
{
  using RuleBitPair = std::pair<uint16_t, uint8_t>;

  if (!fallback::keepCat11ResultInIs(&pricingTrx))
  {
    static const RuleBitPair ruleBitPairs[] = {
        {RuleConst::DAY_TIME_RULE, RuleConst::CAT2_FAIL},
        {RuleConst::SEASONAL_RULE, RuleConst::CAT3_FAIL},
        {RuleConst::FLIGHT_APPLICATION_RULE, RuleConst::CAT4_FAIL},
        {RuleConst::STOPOVER_RULE, RuleConst::CAT8_FAIL},
        {RuleConst::TRANSFER_RULE, RuleConst::CAT9_FAIL},
        {RuleConst::BLACKOUTS_RULE, RuleConst::CAT11_FAIL},
        {RuleConst::TRAVEL_RESTRICTIONS_RULE, RuleConst::CAT14_FAIL}};

    fM->setGlobalDirection(curItinCellInfo.globalDirection());
    for (const RuleBitPair& ruleBitPair : ruleBitPairs)
      curFare->setCategoryProcessed(ruleBitPair.first, false);

    ruleController.validate(pricingTrx, *journeyItin, *curFare);

    if (curFare->areAllCategoryValid())
      return;

    // Move validation results to the flight bitmap
    for (const RuleBitPair& ruleBitPair : ruleBitPairs)
    {
      if (!curFare->isCategoryValid(ruleBitPair.first))
      {
        curFare->setFlightInvalid(bitIndex, ruleBitPair.second);
        curFare->setCategoryValid(ruleBitPair.first);
        setBaseFareCategoryValid(curFare, ruleBitPair.first);
      }
    }

    return;
  }


  curFare->setCategoryProcessed(RuleConst::DAY_TIME_RULE, false);
  curFare->setCategoryProcessed(RuleConst::SEASONAL_RULE, false);
  curFare->setCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE, false);
  curFare->setCategoryProcessed(RuleConst::TRANSFER_RULE, false);
  curFare->setCategoryProcessed(RuleConst::TRAVEL_RESTRICTIONS_RULE, false);
  curFare->setCategoryProcessed(RuleConst::STOPOVER_RULE, false);
  fM->setGlobalDirection(curItinCellInfo.globalDirection());
  // --- validate cat 2/4/9/14 for flight Bitmap
  ruleController.validate(pricingTrx, *journeyItin, *curFare);

  if (!curFare->areAllCategoryValid())
  {
    if (!curFare->isCategoryValid(RuleConst::DAY_TIME_RULE))
    {
      curFare->setFlightInvalid(bitIndex, RuleConst::CAT2_FAIL);
      curFare->setCategoryValid(RuleConst::DAY_TIME_RULE);
      setBaseFareCategoryValid(curFare, RuleConst::DAY_TIME_RULE);
    }

    if (!curFare->isCategoryValid(RuleConst::SEASONAL_RULE))
    {
      curFare->setFlightInvalid(bitIndex, RuleConst::CAT3_FAIL);
      curFare->setCategoryValid(RuleConst::SEASONAL_RULE);
      setBaseFareCategoryValid(curFare, RuleConst::SEASONAL_RULE);
    }

    if (!curFare->isCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE))
    {
      curFare->setFlightInvalid(bitIndex, RuleConst::CAT4_FAIL);
      curFare->setCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE);
      setBaseFareCategoryValid(curFare, RuleConst::FLIGHT_APPLICATION_RULE);
    }

    if (!curFare->isCategoryValid(RuleConst::TRANSFER_RULE))
    {
      curFare->setFlightInvalid(bitIndex, RuleConst::CAT9_FAIL);
      curFare->setCategoryValid(RuleConst::TRANSFER_RULE);
      setBaseFareCategoryValid(curFare, RuleConst::TRANSFER_RULE);
    }

    if (!curFare->isCategoryValid(RuleConst::TRAVEL_RESTRICTIONS_RULE))
    {
      curFare->setFlightInvalid(bitIndex, RuleConst::CAT14_FAIL);
      curFare->setCategoryValid(RuleConst::TRAVEL_RESTRICTIONS_RULE);
      setBaseFareCategoryValid(curFare, RuleConst::TRAVEL_RESTRICTIONS_RULE);
    }

    if (!curFare->isCategoryValid(RuleConst::STOPOVER_RULE))
    {
      curFare->setFlightInvalid(bitIndex, RuleConst::CAT8_FAIL);
      curFare->setCategoryValid(RuleConst::STOPOVER_RULE);
      setBaseFareCategoryValid(curFare, RuleConst::STOPOVER_RULE);
    }
  }
}

void
FareValidatorOrchestrator::performQualCat4Validations(PaxTypeFare* curFare,
                                                      FareMarketRuleController& cat4RuleController,
                                                      const uint32_t& bitIndex,
                                                      PricingTrx& pricingTrx,
                                                      Itin* journeyItin)
{
  if (LIKELY(curFare->isFlightValid(bitIndex)))
  {
    typedef VecMap<uint32_t, QualifyFltAppRuleData*> QMap;
    QMap::const_iterator iter = curFare->qualifyFltAppRuleDataMap().begin();
    QMap::const_iterator iterEnd = curFare->qualifyFltAppRuleDataMap().end();
    LOG4CXX_DEBUG(logger, "-- startRuleController validation Qualified cat 4");

    if (iter != iterEnd)
    {
      while (iter != iterEnd)
      {
        curFare->setCategoryProcessed(iter->first, false);
        LOG4CXX_DEBUG(logger, "category to validate -- " << iter->first);
        ++iter;
      }

      cat4RuleController.validate(pricingTrx, *journeyItin, *curFare);
      iter = curFare->qualifyFltAppRuleDataMap().begin();

      while (iter != iterEnd)
      {
        if (!curFare->isCategoryValid(iter->first))
        {
          LOG4CXX_DEBUG(logger, "--- category" << iter->first << " is not valid");
          curFare->setFlightInvalid(bitIndex, RuleConst::QUALIFYCAT4_FAIL);
          curFare->setCategoryValid(iter->first);
        }

        iter++;
      }
    }
  }
}

void
FareValidatorOrchestrator::performLimitationValidations(PricingTrx& trx,
                                                        Itin*& itin,
                                                        FareMarket*& fareMarket)
{
  LimitationOnIndirectTravel lit(trx, *itin);

  if (UNLIKELY(!lit.validateFareComponent(*fareMarket)))
  {
    if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
    {
      std::string marketString = FareMarketUtil::getDisplayString(*fareMarket);
      LOG4CXX_DEBUG(logger,
                    "FareCollectorOrchestrator - Limitations failed FareMarket [" << marketString
                                                                                  << "]");
    }

    LOG4CXX_INFO(logger, "lit.validateFareComponent returned false");
    fareMarket->failCode() = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
    return;
  }
  if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
  {
    std::string marketString = FareMarketUtil::getDisplayString(*fareMarket);
    LOG4CXX_DEBUG(logger,
                  "FareCollectorOrchestrator - Limitations passed FareMarket [" << marketString
                                                                                << "]");
  }
}

void
FareValidatorOrchestrator::setFareStatus(ShoppingTrx& trx,
                                         std::vector<PaxTypeFare*>& fares,
                                         int lastFareProcessed,
                                         uint64_t mainDuration)
{
  PaxTypeFare* foundValidFare = nullptr;
  int fareIndex = 0;

  for (auto pTFIter = fares.begin(); pTFIter != fares.end(); ++pTFIter, ++fareIndex)
  {
    PaxTypeFare*& curFare = *pTFIter;
    if (UNLIKELY(curFare == nullptr))
    {
      continue;
    }

    if (lastFareProcessed >= 0)
    {
      if (fareIndex > lastFareProcessed)
        break;
    }

    //##############################
    //# - reset isValid disable flag
    //# - isValid will work normally
    //#   now
    //##############################
    curFare->setIsNotShoppingFare();
    curFare->flightBitmapAllInvalid() = false;

    if (curFare->flightBitmap().empty())
    {
      continue;
    }

    if (_shoppingFVO.checkCarriersFlightBitmapsStatus(curFare, mainDuration))
    {
      curFare->setCategoryProcessed(
          RuleConst::FLIGHT_APPLICATION_RULE); // use cat-4 to indicate invalid
      curFare->setCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE,
                                false); // use cat-4 to indicate invalid
      curFare->flightBitmapAllInvalid() = true;
    }
    else
    {
      foundValidFare = curFare;
      // If there are valid flight bitmaps/SOPs in this ptf, we want to keep
      // this ptf valid, ie, isValid() should return true by returning true in
      // areAllCategoryValid(). This is done by re-setting all categories
      // valid in the ptf by calling setAllCategoryValidForShopping() which resets
      // all bits to 1.
      curFare->setAllCategoryValidForShopping();
    }
  }

  if (foundValidFare)
  {
    foundValidFare->fareMarket()->failCode() = ErrorResponseException::NO_ERROR;
  }
}

void
FareValidatorOrchestrator::setFareStatus(ShoppingTrx& trx, std::vector<PaxTypeFare*>& fares)
{
  for (auto curFare : fares)
  {
    if (curFare == nullptr)
    {
      continue;
    }

    curFare->setIsNotShoppingFare();

    if (trx.isAltDates())
    {
      if (curFare->durationFlightBitmap().empty())
      {
        continue;
      }
      // Use cat-4 to indicate invalid
      if (!curFare->hasValidFlight())
      {
        curFare->setCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE, false);
      }
      else
      {
        curFare->setAllCategoryValidForShopping();
      }
    }
    else
    {
      curFare->flightBitmapAllInvalid() = false;

      if (curFare->flightBitmap().empty())
      {
        continue;
      }

      if (curFare->isFlightBitmapInvalid())
      {
        curFare->setCategoryProcessed(RuleConst::FLIGHT_APPLICATION_RULE);
        curFare->setCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE, false);
        curFare->flightBitmapAllInvalid() = true; // used by isOutboundInboundValid
      }
      else
      {
        curFare->setAllCategoryValidForShopping();
      }
    }
  }
}

namespace
{
class TravelSegOpenSetter final
{
  TravelSeg* _seg = nullptr;
  TravelSegType _segmentType = TravelSegType::Open;
  DateTime _departureDT;
  CarrierCode _marketingCarrierCode;
  FlightNumber _operatingFlightNumber = 0;

public:
  ~TravelSegOpenSetter() { swap(); }

  void setSegment(TravelSeg* seg)
  {
    _seg = seg;
    swap();
  }

  void swap()
  {
    if (_seg == nullptr)
    {
      return;
    }

    // std::swap(_departureDT,_seg->departureDT());
    std::swap(_segmentType, _seg->segmentType());

    if (AirSeg* airSeg = _seg->toAirSeg())
    {
      airSeg->swapMarketingCarrierCode(_marketingCarrierCode);
      std::swap(_operatingFlightNumber, airSeg->operatingFlightNumber());
    }
  }
};
}

void
FareValidatorOrchestrator::validateNonFltRules(ShoppingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "FareOrchestrator::validateNonFltRules(ShoppingTrx)...");
  TSELatencyData metrics(trx, "FVO VALIDATE NON FLT RULES");
  // perform basic validation on the fare markets before the expensive
  // operation of creating flight bitmaps
  RuleControllerWithChancelor<FareMarketRuleController> shoppingRuleController(
      ShoppingComponentValidation),
      shoppingASORuleController(ShoppingAcrossStopOverComponentValidation);
  // Process fares and properly size the bitmaps
  // this will build a vector of fares that pass cat 2 validation.
  // Used because at the end we want to reset the processing flag on cat 2,
  // since it should be processed again in ShoppingComponentWithFlightsValidation
  std::vector<uint16_t> ruleControllerCategorySequence = shoppingRuleController.categorySequence();
  std::vector<uint16_t> asoRuleControllerCategorySequence =
      shoppingASORuleController.categorySequence();
  uint32_t legId = 0;

  for (auto& curLeg : trx.legs())
  {
    ItinIndex& curCxrIdx = curLeg.carrierIndex();

    if (curCxrIdx.root().empty())
    {
      ++legId;
      continue;
    }

    // always reset to original set up
    shoppingRuleController.categorySequence() = ruleControllerCategorySequence;
    shoppingASORuleController.categorySequence() = asoRuleControllerCategorySequence;

    if (curLeg.dateChange())
    {
      shoppingRuleController.removeCat(RuleConst::DAY_TIME_RULE);
      shoppingASORuleController.removeCat(RuleConst::DAY_TIME_RULE);
    }

    bool isStopOverLeg = curLeg.stopOverLegFlag();
    FareMarketRuleController& ruleController(isStopOverLeg ? shoppingASORuleController
                                                           : shoppingRuleController);

    for (auto& itinMatrix : curCxrIdx.root())
    {
      Itin* curItin = getCurrCellItinCheckNothing(trx, legId, itinMatrix.first);

      // Make sure the itinerary pointer is valid
      if (curItin == nullptr)
      {
        continue;
      }

      std::vector<FareMarket*>& fareMarkets(curItin->fareMarket());

      for (FareMarket* fm : fareMarkets)
      {
        if (fm == nullptr)
          continue;

        if (trx.isIataFareSelectionApplicable() && !isStopOverLeg)
        {
          fm->setComponentValidationForCarrier(itinMatrix.first);
        }

        JourneyItinWrapper journeyItinWrapper(trx, fm, curLeg, legId, itinMatrix.first);
        validateFMNonFltRules(trx,
                              fm,
                              itinMatrix.first,
                              fm->governingCarrier(),
                              ruleController,
                              curLeg,
                              legId,
                              journeyItinWrapper);

        if (!trx.isIataFareSelectionApplicable())
          break;
      }
    }

    ++legId;
  }
}

void
FareValidatorOrchestrator::validateFMNonFltRules(ShoppingTrx& trx,
                                                 FareMarket* fM,
                                                 uint32_t carrierIndex,
                                                 const std::string& carrier,
                                                 FareMarketRuleController& ruleController,
                                                 ShoppingTrx::Leg& leg,
                                                 uint32_t legId,
                                                 JourneyItinWrapper& journeyItinWrapper)
{
  std::vector<PaxTypeFare*> faresPassedCat2;
  bool isStopOverLeg = leg.stopOverLegFlag();
  uint32_t beginLeg = (isStopOverLeg) ? leg.jumpedLegIndices().front() : legId;
  uint32_t endLeg = (isStopOverLeg) ? leg.jumpedLegIndices().back() + 1 : legId + 1;
  Itin journeyItin = *trx.journeyItin();

  for (auto tvlSeg : journeyItin.travelSeg())
  {
    if (auto* airSeg = tvlSeg->toAirSeg())
      airSeg->carrier() = fM->governingCarrier();
  }

  std::vector<TravelSeg*> tvlSegTemp;

  fM->travelSeg().swap(tvlSegTemp);
  fM->travelSeg().clear();
  fM->travelSeg().insert(fM->travelSeg().end(),
                         journeyItin.travelSeg().begin() + beginLeg,
                         journeyItin.travelSeg().begin() + endLeg);

  journeyItin.validatingCarrier() = carrier;

  for (auto paxTypeFare : fM->allPaxTypeFare())
  {
    if (UNLIKELY(paxTypeFare == nullptr))
    {
      continue;
    }

    if (UNLIKELY(trx.isIataFareSelectionApplicable() && !isStopOverLeg))
    {
      paxTypeFare->setComponentValidationForCarrier(
          journeyItinWrapper.getCarrierKey(), trx.isAltDates(), journeyItinWrapper.getDuration());
    }

    // Skip validation of fares keep only for routing validation
    if (UNLIKELY(paxTypeFare->isKeepForRoutingValidation()))
      continue;

    paxTypeFare->shoppingComponentValidationPerformed() = true;
    // set the shoppingComponentValidationFailed
    // if the cat 15 validation fail in the fare state or not all category is valid.
    paxTypeFare->shoppingComponentValidationFailed() =
        !(paxTypeFare->isCat15SecurityValid() && paxTypeFare->areAllCategoryValid());

    //################################################################
    //# Checks if fare passed PRE_VALIDATION phase during collection #
    //################################################################
    if (!paxTypeFare->isValid())
      continue;

    if ((fM->direction() == FMDirection::OUTBOUND) && (paxTypeFare->directionality() == TO))
      continue;

    if (UNLIKELY(trx.getTrxType() == PricingTrx::FF_TRX))
    {
      if ((fM->direction() == FMDirection::INBOUND) && (paxTypeFare->directionality() == FROM))
      {
        continue;
      }
    }

    fM->setGlobalDirection(paxTypeFare->globalDirection());
    ruleController.validate(trx, journeyItin, *paxTypeFare);
    paxTypeFare->shoppingComponentValidationFailed() = !(paxTypeFare->areAllCategoryValid());

    // Set valid cat 2 fares to "not processed" for next phase
    // of validation (component with flights rule validation)
    if (paxTypeFare->isCategoryValid(2))
    {
      faresPassedCat2.push_back(paxTypeFare);
    }
  }

  fM->travelSeg().swap(tvlSegTemp);

  // Set the travel segments to all being open
  std::vector<TravelSeg*>& travelSegs = fM->travelSeg();
  std::vector<TravelSegOpenSetter> setters(travelSegs.size());

  for (size_t n = 0; n != setters.size(); ++n)
  {
    setters[n].setSegment(travelSegs[n]);
  }

  // Calculate number of itins applicable for this thru-fare market
  uint32_t flightSize = 0;
  flightSize = leg.getFlightBitmapSize(trx, carrierIndex);
  // Set the flight bitmap sizes to the size of the inner map
  // Iterate through all of the fares
  for (auto paxTypeFare : fM->allPaxTypeFare())
  {
    if (UNLIKELY(nullptr == paxTypeFare))
    {
      continue;
    }

    // Create flight bitmap for fares keep for routing validation
    // only
    if (UNLIKELY(paxTypeFare->isKeepForRoutingValidation()))
    {
      // Add the proper number of "bits" to the flight bitmap
      if (paxTypeFare->flightBitmap().empty())
        createFlightBitmap(trx, paxTypeFare, flightSize, *fM, carrierIndex);

      continue;
    }

    //###########################################################
    //# Checks if the fare passed SHOPPING_COMPONENT_VALIDATION #
    //###########################################################
    if (!paxTypeFare->isValid())
    {
      continue;
    }

    if (fM->direction() == FMDirection::OUTBOUND && paxTypeFare->directionality() == TO)
    {
      continue;
    }

    if (UNLIKELY(trx.getTrxType() == PricingTrx::FF_TRX))
    {
      if (fM->direction() == FMDirection::INBOUND && paxTypeFare->directionality() == FROM)
      {
        continue;
      }
    }

    //#######################################################
    //#Setting this flag effectively disables any call      #
    //#to PaxTypeFare::isValid( ) during the                #
    //# SHOPPING_COMPONENT_WITH_FLIGHTS_VALIDATION phase    #
    //# (i.e. bitmap phase)                                 #
    //# This flag will be reset after bitmap validation     #
    //#######################################################
    paxTypeFare->setIsShoppingFare();

    // Add the proper number of "bits" to the flight bitmap
    if (LIKELY(paxTypeFare->flightBitmap().empty()))
    {
      createFlightBitmap(trx, paxTypeFare, flightSize, *fM, carrierIndex);
    }
  }

  fM->setGlobalDirection(GlobalDirection::ZZ); // return to zz global direction

  // loop over all fares that passed cat 2, and set their processing of cat 2
  // to false, so it will be re-tested in the next phase
  for (auto ptc : faresPassedCat2)
  {
    if (LIKELY(ptc->isCategoryValid(2)))
    {
      ptc->setCategoryProcessed(2, false);
    }
  }
}

void
FareValidatorOrchestrator::createFlightBitmap(ShoppingTrx& trx,
                                              PaxTypeFare* fare,
                                              uint32_t flightSize,
                                              FareMarket& fareMarket,
                                              const ItinIndex::Key& carrierKey)
{
  fare->setFlightBitmapSize(flightSize);

  const unsigned legIndex = fareMarket.legIndex();
  ShoppingTrx::Leg& leg = trx.legs().at(legIndex);

  if (UNLIKELY(trx.isIataFareSelectionApplicable() && !leg.stopOverLegFlag()))
  {
    const SOPUsages& usages = (*fareMarket.getApplicableSOPs())[carrierKey];
    if (usages.size() != fare->flightBitmap().size())
      return;

    for (size_t i = 0; i < fare->flightBitmap().size(); ++i)
      if (!usages[i].applicable_)
        fare->flightBitmap()[i]._flightBit = RuleConst::NOT_APPLICABLE;
  }

  setFlightBitmapForMultiAirport(trx, *fare, fareMarket, carrierKey);
}

void
FareValidatorOrchestrator::setFlightBitmapForMultiAirport(ShoppingTrx& trx,
                                                          PaxTypeFare& fare,
                                                          const FareMarket& fareMarket,
                                                          const ItinIndex::Key& carrierKey)
{
  typedef FareMarket::MultiAirportInfo::Airports Airports;

  if (UNLIKELY(trx.getTrxType() != PricingTrx::IS_TRX))
  {
    return;
  }

  const FareMarket::MultiAirportInfo* multiAirportInfo = fareMarket.getMultiAirportInfo();
  if (!multiAirportInfo)
  {
    return;
  }

  const bool reverseLocations =
      fare.origin() != fareMarket.origin()->loc() && fare.origin() != fareMarket.boardMultiCity();
  const LocCode& fareOrigin = !reverseLocations ? fare.origin() : fare.destination();
  const LocCode& fareDestination = !reverseLocations ? fare.destination() : fare.origin();

  Airports::const_iterator originIt =
      std::find(multiAirportInfo->origin().begin(), multiAirportInfo->origin().end(), fareOrigin);

  Airports::const_iterator destinationIt = std::find(multiAirportInfo->destination().begin(),
                                                     multiAirportInfo->destination().end(),
                                                     fareDestination);

  if (originIt == multiAirportInfo->origin().end() ||
      destinationIt == multiAirportInfo->destination().end())
  {
    return;
  }

  const unsigned legIndex = fareMarket.legIndex();
  ShoppingTrx::Leg& leg = trx.legs().at(legIndex);
  ItinIndex& itinIndex = leg.carrierIndex();

  const SOPUsages* sopUsages = nullptr;
  if (fareMarket.getApplicableSOPs() &&
      (trx.isSumOfLocalsProcessingEnabled() ||
       (trx.isIataFareSelectionApplicable() && !leg.stopOverLegFlag())))
  {
    ApplicableSOP::const_iterator applicableSOPIt =
        fareMarket.getApplicableSOPs()->find(carrierKey);
    if (LIKELY(applicableSOPIt != fareMarket.getApplicableSOPs()->end()))
    {
      sopUsages = &applicableSOPIt->second;
    }
  }

  ItinIndex::ItinIndexIterator itinIndexIt =
      leg.stopOverLegFlag() ? itinIndex.beginAcrossStopOverRow(trx, legIndex, carrierKey)
                            : itinIndex.beginRow(carrierKey);
  ItinIndex::ItinIndexIterator itinIndexEndIt =
      leg.stopOverLegFlag() ? itinIndex.endAcrossStopOverRow() : itinIndex.endRow();

  for (; itinIndexIt != itinIndexEndIt; ++itinIndexIt)
  {
    const Itin& itin = *itinIndexIt->second;
    const uint32_t bitIndex = itinIndexIt.bitIndex();
    if (!fare.isFlightValid(bitIndex))
    {
      continue;
    }

    const SOPUsage* sopUsage = sopUsages ? &sopUsages->at(bitIndex) : nullptr;
    if (UNLIKELY(sopUsage && !sopUsage->applicable_))
    {
      continue;
    }

    const TravelSeg* firstTravelSeg;
    const TravelSeg* lastTravelSeg;
    if (sopUsage && sopUsage->startSegment_ != -1 && sopUsage->endSegment_ != -1)
    {
      firstTravelSeg = itin.travelSeg().at(sopUsage->startSegment_);
      lastTravelSeg = itin.travelSeg().at(sopUsage->endSegment_);
    }
    else
    {
      firstTravelSeg = itin.travelSeg().front();
      lastTravelSeg = itin.travelSeg().back();
    }

    const bool originOk = firstTravelSeg->origin()->loc() == fareOrigin ||
                          firstTravelSeg->boardMultiCity() == fareOrigin;
    const bool destinationOk = lastTravelSeg->destination()->loc() == fareDestination ||
                               lastTravelSeg->offMultiCity() == fareDestination;
    if (!originOk || !destinationOk)
    {
      fare.setFlightInvalid(bitIndex, RuleConst::AIRPORT_NOT_APPLICABLE);
    }
  }
}

//----------------------------------------------------------------------------
// validate non flight related rule and create flight bit map for alt date
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::validateNonFltRulesForAltDates(ShoppingTrx& trx,
                                                          const ItinIndex::Key* cxrKey,
                                                          PaxTypeFare* ptf,
                                                          FareMarket* fMM,
                                                          const uint32_t numberOfFaresToProcess,
                                                          const bool callback)
{
  LOG4CXX_DEBUG(logger, "FareOrchestrator::validateNonFltRulesForAltDates(ShoppingTrx)...");
  TSELatencyData metrics(trx, "FVO VALIDATE NON FLT RULES FOR ALTDATES");
  RuleControllerWithChancelor<FareMarketRuleController> shoppingRuleController(
      ShoppingComponentValidation);
  ShoppingTrx::AltDatePairs altDatePairs = trx.altDatePairs();
  ShoppingTrx::AltDatePairs::reverse_iterator altDatePairIter = altDatePairs.rbegin();
  ShoppingTrx::AltDatePairs::reverse_iterator altDatePairIterEnd = altDatePairs.rend();
  // this will build a vector of fares that pass cat 2 validation.
  // Used because at the end we want to reset the processing flag on cat 2,
  // since it should be processed again in ShoppingComponentWithFlightsValidation
  std::vector<PaxTypeFare*> faresPassedCat2;
  bool firstTime = true;

  for (; altDatePairIter != altDatePairIterEnd; ++altDatePairIter) // iterate through each date pair
  {
    // perform basic validation on the fare markets before the expensive
    // operation of creating flight bitmaps
    // Process fares and properly size the bitmaps
    std::vector<ShoppingTrx::Leg>::iterator sLegIter = trx.legs().begin();
    std::vector<ShoppingTrx::Leg>::iterator sLegEndIter = trx.legs().end();
    uint32_t legId = 0;
    Itin* journeyItin = altDatePairIter->second->journeyItin;
    DatePair datePair = altDatePairIter->first;
    uint64_t duration = ShoppingAltDateUtil::getDuration(datePair);

    // Loop through the legs of the transaction
    for (; sLegIter != sLegEndIter; ++sLegIter, ++legId)
    {
      if (cxrKey == nullptr)
      {
        ItinIndex& curCxrIdx = sLegIter->carrierIndex();

        if (curCxrIdx.root().empty())
          continue;

        for (auto& itinMatrix : curCxrIdx.root())
        {
          Itin* curItin = getCurrCellItinCheckNothing(trx, legId, itinMatrix.first);
          if (curItin == nullptr)
            continue;

          for (FareMarket* fM : curItin->fareMarket())
          {
            if (fM == nullptr)
              continue;

            if (trx.isIataFareSelectionApplicable() && !sLegIter->stopOverLegFlag())
              fM->setComponentValidationForCarrier(itinMatrix.first);

            fM->setFoundPTF(true);

            if (firstTime)
            {
              fM->clearAllPaxTypeFareProcessed();
              fM->setFirstFareProcessed(-1);
              fM->setLastFareProcessed(-1);
            }

            validateAltDateShoppingRules(trx,
                                         fM,
                                         journeyItin,
                                         legId,
                                         datePair,
                                         faresPassedCat2,
                                         ptf,
                                         numberOfFaresToProcess,
                                         callback);

            if (boost::next(altDatePairIter) == altDatePairIterEnd)
            {
              // create flight bit map once with the last DatePair process
              if (!trx.isAltDates() || numberOfFaresToProcess >= DELAY_VALIDATION_OFF ||
                  fM->lastFareProcessed() >= 0)
              {
                // Calculate number of itins applicable for this thru-fare market
                uint32_t flightSize = sLegIter->getFlightBitmapSize(trx, itinMatrix.first);
                createAltDatesFlightBitmaps(trx, fM, legId, flightSize, itinMatrix.first, duration);
              }

              // set the travel segments to all being open
              std::vector<TravelSeg*>& travelSegs = fM->travelSeg();
              std::vector<TravelSegOpenSetter> setters(travelSegs.size());

              for (size_t n = 0; n != setters.size(); ++n)
                setters[n].setSegment(travelSegs[n]);
            }

            fM->setGlobalDirection(GlobalDirection::ZZ); // return to zz global direction

            if (!trx.isIataFareSelectionApplicable())
              break; // for pre-fare selection, only process first FM

          } // For each FareMarket
        } // for (; iMIter != iMEndIter; ++iMIter)
      } // if (cxrKey == 0)
      else
      {
        auto itinCell =
            ShoppingUtil::retrieveDirectItin(trx, legId, *cxrKey, ItinIndex::CHECK_NOTHING);

        // this is not the same leg index, then go to next leg to process;
        // process only fare list of the calling paxTypeFare's leg
        if (fMM != nullptr && legId != fMM->legIndex())
          continue;

        if (!itinCell)
          continue;

        Itin* curItin = itinCell->second;
        if (curItin == nullptr)
          continue;

        for (FareMarket* fM : curItin->fareMarket())
        {
          if (fM == nullptr)
            continue;

          if (trx.isIataFareSelectionApplicable() && !sLegIter->stopOverLegFlag())
            fM->setComponentValidationForCarrier(*cxrKey);

          GlobalDirection direction = fM->getGlobalDirection();

          if (!callback)
            fM->setFoundPTF(true);

          if (firstTime)
          {
            fM->clearAllPaxTypeFareProcessed();
            fM->setFirstFareProcessed(-1);
            fM->setLastFareProcessed(-1);
          }

          validateAltDateShoppingRules(trx,
                                       fM,
                                       journeyItin,
                                       legId,
                                       datePair,
                                       faresPassedCat2,
                                       ptf,
                                       numberOfFaresToProcess,
                                       callback);

          // Bitmap creation
          if (boost::next(altDatePairIter) == altDatePairIterEnd)
          {
            // create flight bit map once with the last DatePair process
            if (!trx.isAltDates() || numberOfFaresToProcess >= DELAY_VALIDATION_OFF ||
                fM->lastFareProcessed() >= 0)
            {
              // Calculate number of itins applicable for this thru-fare market
              uint32_t flightSize = sLegIter->getFlightBitmapSize(trx, *cxrKey);
              createAltDatesFlightBitmaps(trx, fM, legId, flightSize, *cxrKey, duration);
            }

            // set the travel segments to all being open
            std::vector<TravelSeg*>& travelSegs = fM->travelSeg();
            std::vector<TravelSegOpenSetter> setters(travelSegs.size());

            for (size_t n = 0; n != setters.size(); ++n)
              setters[n].setSegment(travelSegs[n]);
          }

          fM->setGlobalDirection(direction);

          if (!trx.isIataFareSelectionApplicable())
            break; // for pre-fare selection, only process first FM

        } // For each fare market
      } // if (cxrKey != 0)
    } // For each leg

    // loop over all fares that passed cat 2, and set their processing of cat 2
    // to false, so it will be re-tested in the next phase
    for (auto ptf : faresPassedCat2)
    {
      if (LIKELY(ptf->isCategoryValid(2) && ptf->isAltDateValid()))
        ptf->setCategoryProcessed(2, false);
    } // leg loop

    firstTime = false;

  } // alt date pairs loop
}

//----------------------------------------------------------------------------
// process(MetricsTrx)
//----------------------------------------------------------------------------
bool
FareValidatorOrchestrator::process(MetricsTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entered FareValidatorOrchestrator::process(MetricsTrx)");
  std::ostringstream& oss = trx.response();
  MetricsUtil::header(oss, "FVO Metrics");
  MetricsUtil::lineItemHeader(oss);
  MetricsUtil::lineItem(oss, MetricsUtil::FVO_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::FVO_ROUTING);
  MetricsUtil::lineItem(oss, MetricsUtil::FVO_RTG_MAP_VALIDATION);
  MetricsUtil::lineItem(oss, MetricsUtil::FVO_RTG_RESTRICTION_VALIDATION);
  MetricsUtil::lineItem(oss, MetricsUtil::FVO_RTG_MILEAGE_VALIDATION);
  MetricsUtil::lineItem(oss, MetricsUtil::FVO_BKGCODE_VALIDATOR);
  MetricsUtil::lineItem(oss, MetricsUtil::FVO_RULE_CONTROLLER);
  MetricsUtil::lineItem(oss, MetricsUtil::FVO_MC_CONTROLLER);
  MetricsUtil::lineItem(oss, MetricsUtil::RC_VALIDATE_PAXFARETYPE);
  LOG4CXX_DEBUG(logger, "Leaving FareValidatorOrchestrator::process(MetricsTrx)");
  return true;
}

//----------------------------------------------------------------------------
// process(PricingTrx)
//----------------------------------------------------------------------------
bool
FareValidatorOrchestrator::process(PricingTrx& trx)
{
  Itin* itin = trx.itin().front();

  if (itin)
    trx.fxCnException() = FxCnException(trx, *itin)();
  else
    trx.fxCnException() = false;

  const TseThreadingConst::TaskId taskId =
      (dynamic_cast<RepricingTrx*>(&trx) == nullptr ? _taskId
                                                    : TseThreadingConst::SYNCHRONOUS_TASK);

  TSELatencyData tld(trx, "FVO PROCESS PRICINGTRX");
  LOG4CXX_DEBUG(logger, "Entered FareValidatorOrchestrator::process(PricingTrx)");

  if (trx.getOptions()->isCarnivalSumOfLocal())
  {
    for (size_t i = 0; i < trx.itin().size(); ++i)
    {
      itin = trx.itin()[i];
      if (itin)
        trx.fxCnExceptionPerItin().insert(std::make_pair(itin, FxCnException(trx, *itin)()));
    }

    itin = nullptr;
  }
  primaryValidation(taskId, trx);

  if (!trx.displayOnly())
  {
    TSELatencyData tld(trx, "FVO MIXED CLASS");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    exec_foreach_valid_fareMarket(taskId, trx, mixedClass, FareMarket::FareValidator);
  }

  releaseCPFares(trx);

  if (UNLIKELY(!trx.displayOnly() && (trx.diagnostic().diagnosticType() == Diagnostic419 ||
                                      trx.diagnostic().diagnosticType() == Diagnostic499) &&
               !(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "BRAND")))
  {
    // We dont thread this, because it would rearrange the diag output
    TSELatencyData tld(trx, "FVO PRINT PTFARES");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_fareMarket(trx,
                              printPTFares); // Left off service bit on purpose.. want to print all
  }

  if (!trx.displayOnly())
  {
    TSELatencyData tld(trx, "FVO FAIL FMS");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    exec_foreach_valid_fareMarket(taskId, trx, failFareMarkets, FareMarket::FareValidator);
  }

  if (trx.getRequest()->brandedFareEntry())
  {
    exec_foreach_valid_fareMarket(taskId, trx, brandedFareMarkets, FareMarket::FareValidator);
  }

  if (trx.isBRAll() || trx.activationFlags().isSearchForBrandsPricing())
  {
    DCFactory* factory = DCFactory::instance();
    Diag892Collector* diag892 = dynamic_cast<Diag892Collector*>(factory->create(trx));
    if (diag892 != nullptr)
    {
      diag892->enable(Diagnostic892);
    }
    skipper::TNBrandsFunctions::buildBrandingOptionSpacesForAllItins(trx, logger, diag892);
    if (diag892)
    {
      diag892->flushMsg();
    }
  }

  if (trx.getRequest()->isBrandedFaresRequest() || trx.isBRAll())
  {
    BrandingUtil::processSoftPassFares(trx);
  }

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic499 &&
               trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "BRAND"))
  {
    TSELatencyData tld(trx, "FVO PRINT BRANDED INFO");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();

    if (trx.getRequest()->brandedFareEntry())
      printBrandLegend(trx);

    invoke_foreach_fareMarket(trx, printPTFares);
  }

  if ((trx.getTrxType() == PricingTrx::MIP_TRX) && (trx.getRequest()->isExpAccCorpId()) &&
      (!TrxUtil::isSimpleTripForMIP(trx)))
  {
    exec_foreach_valid_fareMarket(taskId, trx, duplicatedFareCheck, FareMarket::FareValidator);
  }

  if (trx.isValidatingCxrGsaApplicable())
    exec_foreach_valid_fareMarket(taskId, trx, initPTFvalidatingCxrs, FareMarket::FareValidator);

  if (trx.isRequestedFareBasisInfoUseful())
  {
    trx.checkFareMarketsCoverTravelSegs();
  }

  if (trx.isFlexFare())
    exec_foreach_valid_fareMarket(taskId, trx, initFlexFareFinalCheck, FareMarket::FareValidator);

  processFRRAdjustedSellingLevel(trx, false);

  LOG4CXX_DEBUG(logger, "Leaving FareValidatorOrchestrator::process(PricingTrx)");

  countFares(trx);
  return true;
}

//----------------------------------------------------------------------------
// process(RexPricingTrx)
//----------------------------------------------------------------------------
bool
FareValidatorOrchestrator::process(RexPricingTrx& trx)
{
  if (trx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    LOG4CXX_DEBUG(logger,
                  " - Entered FareValidatorOrchestrator::process(RexPricingTrx) FOR NEW ITIN");
    bool& tls = trx.dataHandle().useTLS();
    tls = true;
    BooleanFlagResetter bfr(tls);
    trx.setAnalyzingExcItin(false);
    TSELatencyData tld(trx, "FVO PROCESS REXPRICINGTRX");
    Itin* itin = trx.itin().front();

    if (itin)
      trx.fxCnException() = FxCnException(trx, *itin)();
    else
      trx.fxCnException() = false;

    auto failFareByTariff = [&](std::vector<PaxTypeFare*> ptfs, TariffCategory allowedTariff)
                            {
                              for (PaxTypeFare* ptf : ptfs)
                              {
                                bool vendorCheck = (!fallback::excPrivPubNoAtpFares(&trx) ||
                                                    ptf->vendor() == ATPCO_VENDOR_CODE);
                                if (vendorCheck &&
                                    ptf->tcrTariffCat() != allowedTariff)
                                {
                                  ptf->setValidForRepricing(false);
                                }
                              }
                            };

    for (Itin* itin : trx.newItin())
    {
      for (FareMarket* fm : itin->fareMarket())
      {
        OptimizationMapper::TariffRestr fareType = trx.optimizationMapper().isFMRestrToPrivOrPub(fm);

        if (fareType != OptimizationMapper::UNRESTRICTED)
        {
          failFareByTariff(fm->allPaxTypeFare(), fareType);
        }
      }
    }

    primaryValidation(_taskId, trx);

    if (!trx.displayOnly())
    {
      TSELatencyData tld(trx, "FVO MIXED CLASS");
      if (fallback::reworkTrxAborter(&trx))
        checkTrxAborted(trx);
      else
        trx.checkTrxAborted();
      exec_foreach_valid_fareMarket(_taskId, trx, mixedClass, FareMarket::FareValidator);
    }

    if (trx.getTrxType() == PricingTrx::MIP_TRX &&
        static_cast<RexExchangeTrx*>(&trx)->getKeepBrandStrategy() == RexExchangeTrx::KEEP_FARE)
    {
      validateKeepOriginalFare(trx);
    }

    releaseCPFares(trx);

    if (UNLIKELY(!trx.displayOnly() && (trx.diagnostic().diagnosticType() == Diagnostic419 ||
                                        trx.diagnostic().diagnosticType() == Diagnostic499)))
    {
      // We dont thread this, because it would rearrange the diag output
      TSELatencyData tld(trx, "FVO PRINT PTFARES");
      if (fallback::reworkTrxAborter(&trx))
        checkTrxAborted(trx);
      else
        trx.checkTrxAborted();
      invoke_foreach_fareMarket(trx, printPTFares);
    }

    if (!trx.displayOnly())
    {
      TSELatencyData tld(trx, "FVO FAIL FMS");
      if (fallback::reworkTrxAborter(&trx))
        checkTrxAborted(trx);
      else
        trx.checkTrxAborted();
      exec_foreach_valid_fareMarket(_taskId, trx, failFareMarkets, FareMarket::FareValidator);
    }

    if (trx.getRequest()->isBrandedFaresRequest())
      BrandingUtil::processSoftPassFares(trx);

    LOG4CXX_DEBUG(logger, "Leaving FareValidatorOrchestrator::process(RexPricingTrx) FOR NEW ITIN");
  }

  return true;
}

bool
FareValidatorOrchestrator::process(RefundPricingTrx& trx)
{
  if (!trx.previousExchangeDT().isEmptyDate())
    trx.setFareApplicationDT(trx.previousExchangeDT());
  else
    trx.setFareApplicationDT(trx.originalTktIssueDT());

  return process(static_cast<PricingTrx&>(trx));
}

namespace
{
bool
checkRoutingValid(const FareMarket& fareMarket, const PaxTypeFare& paxFare)
{
  bool routingValid = (!(paxFare.isRoutingProcessed()) || (paxFare.isRoutingValid()));

  return routingValid;
}

bool
fareFailedCat1_15_35(const PaxTypeFare& paxFare)
{
  if (!paxFare.isCategoryValid(1) || !paxFare.isCategoryValid(15) || !paxFare.isCategoryValid(35))
  {
    return false;
  }

  return true;
}
}

//----------------------------------------------------------------------------
// printPTFares()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::printPTFares(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;
  bool isFTDiag = false;
  NoPNRPricingTrx* noPNRTrx = nullptr;

  if (trx.noPNRPricing())
  {
    noPNRTrx = static_cast<NoPNRPricingTrx*>(&trx);
    isFTDiag = trx.diagnostic().diagParamMapItemPresent(Diagnostic::FARE_TYPE);
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic419)
  {
    diag.enable(Diagnostic419);
    diag.finalDiag(trx, fareMarket);
    diag.flushMsg();
    diag.disable(Diagnostic419);
    return;
  }

  diag.enable(Diagnostic499);
  bool displayFareRetrievalDate = false;

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    RexPricingTrx& rexTrx = static_cast<RexPricingTrx&>(trx);
    displayFareRetrievalDate = (rexTrx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE);
  }

  if (!diag.parseFareMarket(trx, fareMarket))
  {
    // Do not display this fare market
    diag.flushMsg();
    return;
  }

  diag << " " << std::endl;
  diag << FareMarketUtil::getBoardMultiCity(fareMarket,
                                            *fareMarket.travelSeg().front()); // origin()->loc();

  bool fxCnException = false;

  if (trx.getOptions()->isCarnivalSumOfLocal())
  {
    fxCnException = trx.fxCnExceptionPerItin().getValForKey(&itin);
  }
  else
  {
    fxCnException = trx.fxCnException();
  }

  for (TravelSeg* tvlSeg : fareMarket.travelSeg())
  {
    ArunkSeg* arunkSeg = tvlSeg->toArunkSeg();

    if (arunkSeg != nullptr)
    {
      diag << "-";
      diag << FareMarketUtil::getOffMultiCity(fareMarket, *arunkSeg); // destination()->loc();
    }
    else
    {
      AirSeg* airSeg = tvlSeg->toAirSeg();

      if (airSeg != nullptr)
      {
        diag << "-";
        diag << airSeg->carrier();
        diag << "-";
        diag << FareMarketUtil::getOffMultiCity(fareMarket, *airSeg); // destination()->loc();
      }
    }
  }

  diag << "    /CXR-" << fareMarket.governingCarrier() << "/";
  std::string globalDirStr;
  globalDirectionToStr(globalDirStr, fareMarket.getGlobalDirection());
  diag << " #GI-" << globalDirStr << "#  " << fareMarket.getDirectionAsString();
  const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

  if (fareMarket.failCode() != ErrorResponseException::NO_ERROR || paxTypeCortegeVec.empty())
  {
    diag << " -FAILED ";
  }

  diag << std::endl;

  if (trx.getOptions()->fareX())
  {
    diag << "FAREX ENTRY\n";
  }

  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    for (TravelSeg* tvlSeg : fareMarket.travelSeg())
    {
      if (tvlSeg->toArunkSeg())
        continue;

      AirSeg* airSeg = tvlSeg->toAirSeg();

      if (airSeg != nullptr)
      {
        diag << " " << airSeg->carrier();
        diag << " " << airSeg->flightNumber();
      }
    }

    diag << std::endl;
  }

  if (SLFUtil::isSpanishFamilyDiscountApplicable(trx))
  {
    if (SLFUtil::DiscountLevel::LEVEL_1 == trx.getOptions()->getSpanishLargeFamilyDiscountLevel())
      diag << "SPANISH LARGE FAMILY DISCOUNT: LEVEL 1 - 5% APPLIED" << std::endl;
    else if (SLFUtil::DiscountLevel::LEVEL_1 ==
                 trx.getOptions()->getSpanishLargeFamilyDiscountLevel())
      diag << "SPANISH LARGE FAMILY DISCOUNT: LEVEL 2 - 10% APPLIED" << std::endl;
  }

  if ((trx.diagnostic().diagnosticType() == Diagnostic499) &&
      (trx.getRequest()->multiTicketActive()) && !trx.multiTicketMap().empty())
  {
    diag.displayFareMarketItinNumsMultiTkt(" APPLIES TO ITINS   :", trx, fareMarket);
  }

  if (!paxTypeCortegeVec.empty())
  {
    uint16_t num = 0;
    std::map<std::string, int> fareCounterMap;

    for (const PaxTypeBucket& cortege : paxTypeCortegeVec)
    {
      const std::vector<PaxTypeFare*>& paxFareVec = cortege.paxTypeFare();
      std::map<std::string, int> ptcFareCounterMap;

      if (!paxFareVec.empty())
      {
        const std::string& diagDD = trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);

        // if( !( (!diagDD.empty()) && (diagDD == "COUNT") ) )
        if (diagDD.empty() || (diagDD != "COUNT" && diagDD != "FARECOUNT"))
        {
          diag << " " << std::endl;
          diag << " REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << std::endl;
          diag << " INBOUND CURRENCY : " << cortege.inboundCurrency() << std::endl;
          diag << "OUTBOUND CURRENCY : " << cortege.outboundCurrency() << std::endl;

          if (fareMarket.retrievalDate() != DateTime::emptyDate())
            diag << "RETRIEVAL DATE: " << fareMarket.retrievalDate().toIsoExtendedString() << "\n";

          if (trx.getTrxType() == PricingTrx::MIP_TRX)
            diag << "VALIDATING CXRS   : "
                 << DiagnosticUtil::containerToString(fareMarket.validatingCarriers()) << "\n";

          diag << " \n";

          displayFareMarketBrands(trx, fareMarket, diag);

          // 499 diagnostic header display
          {
            if (trx.diagnostic().diagParamMapItemPresent(Diagnostic::FVO_BKGCODE_BEFORE_RULES))
            {
              if (isFTDiag)
              {
                diag << "- -- - ---- ----------- --- - - -------- --- --- --- --- --- --\n"
                     << "  GI V RULE FARE BASIS  TRF O O    AMT   CUR FAR RTG FTV RUL  P\n"
                     << "                        NUM R I              TPE              F \n"
                     << "- -- - ---- ----------- --- - - -------- --- --- --- --- --- --\n";
              }
              else if (trx.isFlexFare())
              {
                diag << "- -- - ---- ----------- --- - - -------- --- --- --- --- ---  -- ----\n"
                     << "  GI V RULE FARE BASIS  TRF O O    AMT   CUR PAX RTG BKG RUL  P  FLEX\n"
                     << "                        NUM R I              TPE              F  FARE\n"
                     << "- -- - ---- ----------- --- - - -------- --- --- --- --- ---  -- ----\n";
              }
              else
              {
                diag << "- -- - ---- ----------- --- - - -------- --- --- --- --- --- --\n"
                     << "  GI V RULE FARE BASIS  TRF O O    AMT   CUR PAX RTG BKG RUL  P\n"
                     << "                        NUM R I              TPE              F \n"
                     << "- -- - ---- ----------- --- - - -------- --- --- --- --- --- --\n";
              }
            }
            else
            {
              if (isFTDiag)
              {
                diag << "- -- - ---- ----------- --- - - -------- --- --- --- --- --- --\n"
                     << "  GI V RULE FARE BASIS  TRF O O    AMT   CUR FAR RTG RUL FTV  P\n"
                     << "                        NUM R I              TPE              F \n"
                     << "- -- - ---- ----------- --- - - -------- --- --- --- --- --- --\n";
              }
              else if (trx.awardRequest())
              {
                diag << "- -- - ---- ----------- --- - - -------- --- --------- --- --- --- --- "
                        "--\n"
                     << "  GI V RULE FARE BASIS  TRF O O    AMT   CUR       MIL PAX RTG RUL BKG  "
                        "P\n"
                     << "                        NUM R I                        TPE              "
                        "F \n"
                     << "- -- - ---- ----------- --- - - -------- --- --------- --- --- --- --- "
                        "--\n";
              }
              else if (trx.isFlexFare())
              {
                diag << "- -- - ---- ----------- --- - - -------- --- --- --- --- ---  -- ----\n"
                     << "  GI V RULE FARE BASIS  TRF O O    AMT   CUR PAX RTG RUL BKG  P  FLEX\n"
                     << "                        NUM R I              TPE              F  FARE\n"
                     << "- -- - ---- ----------- --- - - -------- --- --- --- --- ---  -- ----\n";
              }
              else
              {
                diag << "- -- - ---- ----------- --- - - -------- --- --- --- --- --- --\n"
                     << "  GI V RULE FARE BASIS  TRF O O    AMT   CUR PAX RTG RUL BKG  P\n"
                     << "                        NUM R I              TPE              F \n"
                     << "- -- - ---- ----------- --- - - -------- --- --- --- --- --- --\n";
              }
            } // end if ( ... FVO_BKGCODE_BEFORE_RULES)
          } // end block 499 diagnostic header display
        } // end if (!paxFareVec.empty())

        const bool isDDAllPax = (diagDD == "ALLPAX");

        if (isDDAllPax && fareMarket.allPaxTypeFare().empty())
          break;

        auto& ptfVect = isDDAllPax ? fareMarket.allPaxTypeFare() : paxFareVec;

        for (PaxTypeFare* ptf : ptfVect)
        {
          PaxTypeFare& paxFare = *ptf;

          if (!trx.diagnostic().shouldDisplay(paxFare))
            continue;

          //--------------
          if (paxFare.isValidForPricing())
            num++;

          bool displayFare = false;

          if ((!diagDD.empty()) && (diagDD == "ALLFARES"))
          {
            displayFare =
                (paxFare.fareClassAppInfo() != FareClassAppInfo::emptyFareClassApp() &&
                 paxFare.fareClassAppSegInfo() != FareClassAppSegInfo::emptyFareClassAppSeg() &&
                 paxFare.fare()->isValid());
          }
          else
          {
            // Do not display fare fails Cat1, Cat15 or Cat35 when DDALLFARES is not specified

            if (!fareFailedCat1_15_35(paxFare))
            {
              displayFare = false;
            }
            else if ((!diagDD.empty()) && (diagDD == "PASS"))
            {
              displayFare = (paxFare.isValidForPricing() && !(paxFare.puRuleValNeeded()));
            }
            else if ((!diagDD.empty()) && (diagDD == "COUNT"))
            {
              displayFare = false;
            }
            else if (diagDD == "FARECOUNT")
            {
              displayFare = false;
            }
            else if ((diagDD == "BRAND") && trx.getRequest()->brandedFareEntry())
            {
              displayFare =
                  (paxFare.fareClassAppInfo() != FareClassAppInfo::emptyFareClassApp() &&
                   paxFare.fareClassAppSegInfo() != FareClassAppSegInfo::emptyFareClassAppSeg() &&
                   (paxFare.fare()->isValid() ||
                    (!paxFare.fare()->isValid() && !paxFare.fare()->isAnyBrandedFareValid())) &&
                   checkRoutingValid(fareMarket, paxFare));
            }
            else
            {
              displayFare =
                  (paxFare.fareClassAppInfo() != FareClassAppInfo::emptyFareClassApp() &&
                   paxFare.fareClassAppSegInfo() != FareClassAppSegInfo::emptyFareClassAppSeg() &&
                   paxFare.fare()->isValid()) &&
                  checkRoutingValid(fareMarket, paxFare);
            }
          }

          std::string cnvFlags;

          if (diagDD == "FARECOUNT")
          {
            cnvFlags = diag.cnvFlags(paxFare);
            ptcFareCounterMap[cnvFlags] = ++ptcFareCounterMap[cnvFlags];
          }

          if (displayFare)
          {
            std::string fareBasis = paxFare.createFareBasis(trx, false);

            if (fareBasis.size() > 11)
              fareBasis = fareBasis.substr(0, 11) + "*"; // Cross-of-lorraine?

            const std::string& diagFB =
                trx.diagnostic().diagParamMapItem(Diagnostic::FARE_BASIS_CODE);

            if (!diagFB.empty() && (diagFB != fareBasis))
            {
              continue;
            }

            diag.setf(std::ios::left, std::ios::adjustfield);

            if (!cnvFlags.empty())
            {
              diag << std::setw(2) << cnvFlags;
            }
            else
            {
              diag << std::setw(2) << diag.cnvFlags(paxFare);
            }

            std::string gd;
            globalDirectionToStr(gd, paxFare.fare()->globalDirection());
            diag << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(paxFare.vendor())
                 << std::setw(5) << paxFare.ruleNumber();
            diag << std::setw(12) << fareBasis << std::setw(4) << paxFare.fareTariff();

            if (paxFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
              diag << std::setw(2) << "X";
            else if (paxFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
              diag << std::setw(2) << "R";
            else if (paxFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
              diag << std::setw(2) << "O";
            else
              diag << std::setw(2) << " ";

            if (paxFare.directionality() == FROM)
              diag << std::setw(2) << "O";
            else if (paxFare.directionality() == TO)
              diag << std::setw(2) << "I";
            else
              diag << std::setw(2) << " ";

            diag << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";
            PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(diag.trx());

            if (pricingTrx)
              if (pricingTrx->awardRequest())
              {
                diag.setf(std::ios::right);
                diag << std::setw(9) << paxFare.mileage() << " ";
              }

            if (isFTDiag)
            {
              diag << std::setw(3) << paxFare.fcaFareType();
            }
            else
            {
              if (!paxFare.isFareClassAppSegMissing())
              {
                if (paxFare.fcasPaxType().empty())
                  diag << "***";
                else
                  diag << paxFare.fcasPaxType();
              }
              else
              {
                diag << "UNK";
              }
            }

            DiagValidationData* itinData = nullptr;

            itinData = diagGetValidationResults(trx, paxFare, isFTDiag, noPNRTrx);

            diag << " ";
            diag << std::setw(4) << itinData->_rtg;

            // BookingCode before Rules
            if (trx.diagnostic().diagParamMapItemPresent(Diagnostic::FVO_BKGCODE_BEFORE_RULES))
              diag << std::setw(4) << itinData->_bkg;

            diag << std::setw(4) << itinData->_rul;

            // Display BookingCode after Rules
            if (!trx.diagnostic().diagParamMapItemPresent(Diagnostic::FVO_BKGCODE_BEFORE_RULES))
              diag << std::setw(4) << itinData->_bkg;

            if (itinData->_valid)
            {
              diag << " :P";
            }
            else
            {
              diag << " :F";
            }

            displayFlexFaresInfo(trx, paxFare, diag);
            diag << std::endl;

            if (Vendor::displayChar(paxFare.vendor()) == '*')
            {
              diag << "     " << paxFare.vendor() << std::endl;
            }

            displayBrandValidation(trx, paxFare, diag);

            if (itinData->_valid && paxFare.isCmdPricing() &&
                !paxFare.validForCmdPricing(fxCnException))
            {
              if (TrxUtil::cmdPricingTuningEnabled(trx))
                paxFare.setNotValidForCP(true);

              diag << "**** PASSED BUT NOT VALID FOR COMMAND PRICING ****" << std::endl;
            }

            if (paxFare.cmdPricedWFail() &&
                (paxFare.isCategoryValid(15) || paxFare.cat15SoftPass()))
            {
              if (paxFare.isNotValidForCP())
                diag << "**** NOT CHOSEN BY COMMAND PRICING ****" << std::endl;
              else
                diag << "**** PASSED BY COMMAND PRICING ****" << std::endl;

              const bool asCatNum = true;
              RuleUtil::displayCPWarning(diag, paxFare.cpFailedStatus().value(), asCatNum);
              diag << std::endl;
            }

            if (displayFareRetrievalDate)
            {
              diag.displayRetrievalDate(paxFare);
              diag << std::endl;

              if (!paxFare.isValidAccountCode(trx))
              {
                diag << "**** UNMATCHED ACCOUNT CODE/CORP ID " << paxFare.fbrApp().accountCode()
                     << " ****" << std::endl;
              }
            }
            // display branded info
            if (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "BRAND" &&
                trx.getRequest()->brandedFareEntry() &&
                (itinData->_valid ||
                 (!itinData->_valid && !paxFare.fare()->isAnyBrandedFareValid())))
            {
              PricingRequest* request = trx.getRequest();
              BookingCode fareBookingCode = paxFare.bookingCode();

              if (!paxFare.segmentStatus().empty() &&
                  paxFare.segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
              {
                fareBookingCode = paxFare.segmentStatus()[0]._bkgCodeReBook;
              }

              diag << " " << fareBookingCode << " ";

              for (const auto& bfs : paxFare.fare()->brandedFareStatus())
              {
                diag << bfs.first << "-" << request->brandId(bfs.first) << "-"
                     << !bfs.second.isSet(Fare::FS_InvBrand)
                     << !bfs.second.isSet(Fare::FS_InvBrandCorpID);

                diag << " ";
              }

              diag << std::endl;
            }
            if (trx.isValidatingCxrGsaApplicable() &&
                !paxFare.fareMarket()->validatingCarriers().empty())
            {
              if (paxFare.validatingCarriers().empty() && paxFare.isValid() &&
                  !paxFare.fareMarket()->validatingCarriers().empty())
              {
                paxFare.validatingCarriers() = paxFare.fareMarket()->validatingCarriers();
              }
              if (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "VALCXR")
              {
                displayValidatingCarriers(paxFare, diag);
              }
            }
          }
        }
      }
      else
      {
        diag << " " << std::endl;
        diag << "NO FARES FOUND FOR : " << fareMarket.origin()->loc() << '-'
             << fareMarket.destination()->loc()
             << " REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << std::endl;
      }

      diag << "----- " << num << " VALID PAXTYPEFARE FOR " << cortege.requestedPaxType()->paxType()
           << "-----" << std::endl;

      if (ptcFareCounterMap.size() > 0)
      {
        for (auto& elem : ptcFareCounterMap)
        {
          // accummulate to the fm fare counter:
          fareCounterMap[elem.first] = fareCounterMap[elem.first] + elem.second;
        }
      }
      num = 0;
    }

    // Fare count summary for fare market:
    if (fareCounterMap.size() > 0)
    {
      int total = 0;
      diag << "  FARE COUNT - ";

      for (auto& elem : fareCounterMap)
      {
        total += elem.second;
        diag << std::setw(5) << elem.first << ": " << std::setw(8) << elem.second << "\n"
             << "               ";
        // accummulate for all fare market:
        //_fareCounterMap[i->first] = _fareCounterMap[i->first] + i->second;
      }

      diag << "TOTAL: " << std::setw(8) << total << "\n";
    }
  }
  else
  {
    diag << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
         << fareMarket.destination()->description() << std::endl;
  }

  diag << std::endl;

  if (UNLIKELY(!fallback::fallbackPriceByCabinActivation(&trx) &&
               fareMarket.isFMInvalidByRbdByCabin()))
  {
    diag << "-FARE MARKET NOT VALID FOR PRICE BY CABIN - SEE DIAGNOSTIC 185-";
  }

  diag.flushMsg();
}

void
FareValidatorOrchestrator::noMatchPrepare(PricingTrx& trx, Itin&, FareMarket& fareMarket)
{
  const TSELatencyData metrics(trx, "FVO BOOKING CODE PREPARE");
  fareMarket.setBKSNotApplicapleIfBKSNotPASS();
}

//----------------------------------------------------------------------------
// bookingCodesValidation
//----------------------------------------------------------------------------

void
FareValidatorOrchestrator::bookingCodesValidation(const TseThreadingConst::TaskId taskId,
                                                  PricingTrx& trx)
{
  TSELatencyData tld(trx, "FVO BOOKING CODE");
  NoPNRPricingTrx* noPNRTrx = trx.noPNRPricing() ? static_cast<NoPNRPricingTrx*>(&trx) : nullptr;

  if (noPNRTrx && (noPNRTrx->isNoMatch() || (noPNRTrx->isFullFBCItin() && noPNRTrx->noRBDItin())))
  {
    char lowFare = trx.getRequest()->lowFareRequested();
    trx.getRequest()->lowFareRequested() = 'Y';

    if (!noPNRTrx->noRBDItin())
    {
      exec_foreach_valid_fareMarket(taskId, trx, bookingCode, FareMarket::FareValidator);
      exec_foreach_valid_fareMarket(taskId, trx, noMatchPrepare, FareMarket::FareValidator);
    }

    noPNRTrx->prepareNoMatchItin();
    exec_foreach_valid_fareMarket(taskId, trx, bookingCode, FareMarket::FareValidator);
    noPNRTrx->restoreOldItin();
    trx.getRequest()->lowFareRequested() = lowFare;
  }
  else
    exec_foreach_valid_fareMarket(taskId, trx, bookingCode, FareMarket::FareValidator);
}

//----------------------------------------------------------------------------
// primaryValidation()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::primaryValidation(const TseThreadingConst::TaskId taskId,
                                             PricingTrx& trx)
{
  // When the trx is set to displayOnly, we only call RoutingController if its a 450,
  // otherwise we only call the RuleController.  We never call BookingCode for
  // a displayOnly trx
  bool callBookingCode = true;
  bool callRouting = true;
  bool callRules = true;

  if (trx.displayOnly())
  {
    callBookingCode = false;

    if (trx.diagnostic().diagnosticType() == Diagnostic450)
      callRules = false;
    else
      callRouting = false;
  }

  if (callRouting)
  {
    TSELatencyData tld(trx, "FVO ROUTING");

    if (_newRouting)
    {
      newRouting(taskId, trx);
    }
    else
    {
      exec_foreach_valid_fareMarket(taskId, trx, routing, FareMarket::FareValidator);
    }
  }

  releaseUnusedFares(trx);

  if (smp::isPenaltyCalculationRequired(trx))
    exec_foreach_valid_fareMarket(
        taskId, trx, smp::preValidateFareMarket, FareMarket::FareValidator);

  if (trx.diagnostic().diagParamMapItemPresent(Diagnostic::FVO_BKGCODE_BEFORE_RULES))
  {
    if (callBookingCode)
      bookingCodesValidation(taskId, trx);

    if (callRules)
    {
      TSELatencyData tld(trx, "FVO RULES");
      exec_foreach_valid_fareMarket(taskId, trx, rules, FareMarket::FareValidator);
    }
  }
  else
  {
    if (callRules)
    {
      TSELatencyData tld(trx, "FVO RULES");
      exec_foreach_valid_fareMarket(taskId, trx, rules, FareMarket::FareValidator);
    }

    if (callBookingCode)
      bookingCodesValidation(taskId, trx);
  }

  if (TrxUtil::isFVOSurchargesEnabled())
    exec_foreach_valid_fareMarket(taskId, trx, surcharges, FareMarket::FareValidator);

  if (!fallback::compDiffCmdPricing(&trx))
  {
    TSELatencyData tld(trx, "FVO SORT FARES FOR CMD PRICING");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    exec_foreach_valid_fareMarket(taskId, trx, sortStep, FareMarket::FareValidator);
  }

  if (trx.noPNRPricing())
  {
    NoPNRPricingTrx* noPNRTrx = static_cast<NoPNRPricingTrx*>(&trx);

    if (noPNRTrx->isNoMatch())
    {
      TSELatencyData tld(trx, "FVO FARE TYPES");
      exec_foreach_valid_fareMarket(taskId, trx, fareTypes, FareMarket::FareValidator);
    }
  }
}

//----------------------------------------------------------------------------
// mixedClass()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::mixedClass(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)

{
  bool skipCondition = !trx.matchFareRetrievalDate(fareMarket) ||
                       (!fallback::skipMixedClassForExcItin(&trx) &&
                        trx.isRexBaseTrx() &&
                        static_cast<RexBaseTrx&>(trx).isAnalyzingExcItin());
  if (UNLIKELY(skipCondition))
    return;

  MixedClassController mxc(trx);
  mxc.validate(fareMarket, itin);
}

//----------------------------------------------------------------------------
// initFareDisplayInfo()
//----------------------------------------------------------------------------

void
FareValidatorOrchestrator::initFareDisplayInfo(FareDisplayTrx& trx, const FareMarket& fareMarket)
{
  for (auto ptf : fareMarket.allPaxTypeFare())
  {
    FareDisplayInfo* fdInfo = ptf->fareDisplayInfo();

    if (fdInfo)
      fdInfo->fbDisplay().initialize(trx);
  }
}

//----------------------------------------------------------------------------
// brandedFareMarkets()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::brandedFareMarkets(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (refundSkip(trx, fareMarket))
    return;

  if (fareMarket.allPaxTypeFare().empty() ||
      fareMarket.failCode() != ErrorResponseException::NO_ERROR)
    return;

  if (trx.getRequest()->originBasedRTPricing() && fareMarket.useDummyFare())
    return;

  DiagManager diag(trx);
  if (UNLIKELY(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "BRAND"))
    diag.activate(Diagnostic499);

  BrandedFareValidator val(trx, diag);
  val.brandedFareValidation(fareMarket);
}

//----------------------------------------------------------------------------
// failFareMarkets()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::failFareMarkets(PricingTrx& trx, Itin&, FareMarket& fareMarket)
{
  if (UNLIKELY(refundSkip(trx, fareMarket)))
    return;

  if (!fareMarket.hasAnyFareValid())
  {
    DiagManager diag(trx, Diagnostic299);
    fareMarket.failCode() = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
    diag << "FVO FAIL FARE MARKET: " << FareMarketUtil::getDisplayString(fareMarket);
    diag << "\n";
  }
}

//----------------------------------------------------------------------------
// duplicatedFareCheck()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::duplicatedFareCheck(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (fareMarket.failCode() != ErrorResponseException::NO_ERROR)
    return;

  if (fareMarket.allPaxTypeFare().size() < _minSizeToRemoveDuplicatedFares)
    return;

  uint16_t uniqueFareIndex = 0;
  std::vector<PaxTypeFare*> uniquePtfVec = {fareMarket.allPaxTypeFare().front()};
  std::vector<PaxTypeFare*> duplicatedPtfVec;

  for (auto ptcIter = fareMarket.allPaxTypeFare().begin() + 1,
            ptcIterEnd = fareMarket.allPaxTypeFare().end();
       ptcIter != ptcIterEnd;
       ++ptcIter)
  {
    PaxTypeFare* ptFare = *ptcIter;
    bool isDuplicatedFare = false;

    uint16_t currIndex = uniqueFareIndex;

    while ((currIndex < uniquePtfVec.size()) && (!isDuplicatedFare))
    {
      if (ShoppingUtil::isDuplicatedFare(ptFare, uniquePtfVec[currIndex]))
      {
        isDuplicatedFare = true;
        ptFare->setIsMipUniqueFare(false);
        duplicatedPtfVec.push_back(ptFare);
        break;
      }
      currIndex++;
    }

    if (!isDuplicatedFare)
    {
      uniquePtfVec.push_back(ptFare);
      if (ptFare->originalFareAmount() > uniquePtfVec[currIndex - 1]->originalFareAmount())
        uniqueFareIndex++;
    }
  }

  const bool diag325Enabled =
      (trx.diagnostic().diagnosticType() == Diagnostic325) &&
      (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "REC3");

  if (UNLIKELY(diag325Enabled))
  {
    Diagnostic& trxDiag = trx.diagnostic();
    DiagManager diag(trx, trxDiag.diagnosticType());

    ShoppingUtil::displayRemovedFares(fareMarket, uniquePtfVec.size(), duplicatedPtfVec, diag);
  }
}

//----------------------------------------------------------------------------
// Fare Display
//----------------------------------------------------------------------------
bool
FareValidatorOrchestrator::process(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "FareValidatorOrchestrator::process(FareDisplayTrx)...");
  TSELatencyData tld(trx, "FVO PROCESS FAREDISPLAYTRX");

  // only need multithread when multiple fare markets
  if (trx.fareMarket().size() > 1 && (!trx.isRD()))
  {
    exec_foreach_valid_fareMarket(_taskId, trx, rulesFD, FareMarket::FareValidator);
  }
  else
  {
    invoke_foreach_valid_fareMarket(trx, rulesFD, FareMarket::FareValidator);
  }

  processFRRAdjustedSellingLevel(trx, true);
  processExactlyMatchRetailerCode(trx);

  postValidateAllFares(trx);

  return true;
}

void
FareValidatorOrchestrator::rulesFD(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  TSELatencyData tld(trx, "FVO RULES FD/RD");
  FareDisplayTrx* fdTrx = dynamic_cast<FareDisplayTrx*>(&trx);

  if (fdTrx == nullptr)
  {
    LOG4CXX_ERROR(logger, "rulesFD(): Failed to cast transaction to FareDisplayTrx.");
    return;
  }

  if (fdTrx->isRD())
  {
    FDFareMarketRuleController ruleController(RuleDisplayValidation);
    // PreProcess any commandline rule category/alpha codes so that the
    // definitive list is found in _ruleCategories.
    RuleCategoryProcessor rcp;
    rcp.decipherRuleDisplayCodes(*fdTrx, ruleController);
    initFareDisplayInfo(*fdTrx, fareMarket);
    ruleController.validate(*fdTrx, fareMarket, itin);
  }
  else
  {
    FDFareMarketRuleController ruleController(FareDisplayValidation);
    initFareDisplayInfo(*fdTrx, fareMarket);
    ruleController.validate(*fdTrx, fareMarket, itin);
  }
}

void
FareValidatorOrchestrator::prepareForReprocessing(PricingTrx& trx,
                                                  Itin& itin,
                                                  FareMarket& fareMarket)
{
  AltPricingUtil::prepareToReprocess(fareMarket);
}

//----------------------------------------------------------------------------
// fareTypes()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::fareTypes(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  NoPNRPricingTrx& noPNRTrx = static_cast<NoPNRPricingTrx&>(trx);
  NoPNRPricingTrx::FareTypes& types = noPNRTrx.fareTypes();

  for (PaxTypeFare* const ptf : fareMarket.allPaxTypeFare())
  {
    PaxTypeFare::BookingCodeStatus& bkgStatus = ptf->bookingCodeStatus();

    if (bkgStatus.isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED) ||
        !bkgStatus.isSet(PaxTypeFare::BKS_PASS))
      continue;

    const NoPNRPricingTrx::FareTypes::FTGroup group = types.getFareTypeGroup(ptf->fcaFareType());

    if (group == NoPNRPricingTrx::FareTypes::FTG_NONE)
      ptf->bookingCodeStatus().set(
          PaxTypeFare::BookingCodeState(PaxTypeFare::BKS_FAIL | PaxTypeFare::BKS_FAIL_FARE_TYPES));
    else
      fareMarket.setFareTypeGroupValid(group);
  }
}

//----------------------------------------------------------------------------
// routing()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::routing(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (UNLIKELY(!trx.matchFareRetrievalDate(fareMarket) || refundSkip(trx, fareMarket)))
    return;

  RoutingController routingController(trx);
  TravelRoute travelRoute;
  TravelRoute travelRouteTktOnly;
  travelRoute.travelRouteTktOnly() = &travelRouteTktOnly;
  RoutingInfos routingInfos;

  routingController.process(fareMarket, travelRoute, routingInfos);
  routingController.processRoutingDiagnostic(travelRoute, routingInfos, fareMarket);

  if (TrxUtil::isFullMapRoutingActivated(trx) && travelRoute.travelRouteTktOnly())
  {
    routingController.processRoutingDiagnostic(
        *travelRoute.travelRouteTktOnly(), routingInfos, fareMarket);
  }
}

//----------------------------------------------------------------------------
// bookingCode()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::bookingCode(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (UNLIKELY(!trx.matchFareRetrievalDate(fareMarket)))
    return;

  const TSELatencyData metrics(trx, "FVO BOOKING CODE VALIDATION");
  FareBookingCodeValidator fbcv(trx, fareMarket, &itin);
  fbcv.validate();
}

//----------------------------------------------------------------------------
// rules()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::rules(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (UNLIKELY(!trx.matchFareRetrievalDate(fareMarket) || refundSkip(trx, fareMarket)))
    return;

  RuleControllerWithChancelor<FareMarketRuleController> ruleController(NormalValidation, &trx);
  ruleController.validate(trx, fareMarket, itin);

  if (trx.isValidatingCxrGsaApplicable() && (trx.getTrxType() == PricingTrx::MIP_TRX))
  {
    for (PaxTypeFare* ptf : fareMarket.allPaxTypeFare())
    {
      if (ptf && ptf->isFareByRule() && ptf->isValid() && !ptf->validatingCarriers().empty())
        FareByRuleController::analyzeValidatingCarriers(trx, ptf);

      if (ptf && ptf->isDiscounted() && !ptf->isFareByRule())
      {
        PaxTypeFare* baseFare = ptf->baseFare(CHILDREN_DISCOUNT_RULE);
        ptf->validatingCarriers() = baseFare->validatingCarriers();
      }
    }
  }
}

namespace
{
class Cheaper : public std::binary_function<const PaxTypeFare*, const PaxTypeFare*, bool>
{
public:
  bool operator()(const PaxTypeFare* lhs, const PaxTypeFare* rhs) const
  {
    return lhs->nucTotalSurchargeFareAmount() + lhs->nucFareAmount() <
           rhs->nucTotalSurchargeFareAmount() + rhs->nucFareAmount();
  }
};

} // namespace

void
FareValidatorOrchestrator::surcharges(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (!trx.matchFareRetrievalDate(fareMarket) || refundSkip(trx, fareMarket))
    return;

  RuleControllerWithChancelor<FareMarketRuleController> ruleController(
      NormalValidation, std::vector<uint16_t>(1, RuleConst::SURCHARGE_RULE), &trx);
  ruleController.validate(trx, fareMarket, itin);

  std::stable_sort(
      fareMarket.allPaxTypeFare().begin(), fareMarket.allPaxTypeFare().end(), Cheaper());
}

void
FareValidatorOrchestrator::sortStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (!fareMarket.allPaxTypeFare().empty() && fareMarket.allPaxTypeFare().front()->isCmdPricing())
    FareCollectorOrchestrator::sortStep(trx, itin, fareMarket);
}

//----------------------------------------------------------------------------
// releaseCPFares()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::releaseCPFares(PricingTrx& trx)
{
  if (!trx.getOptions() || !trx.getOptions()->fbcSelected()) // not command pricing trx
    return;

  if (!trx.getOptions()->isCarnivalSumOfLocal() &&
      trx.fxCnException()) // keep all the fares upon CnException
    return;

  DiagManager diag499(trx, Diagnostic499);

  bool releaseFares = releaseUnusedFaresAllItins.getValue();

  for (const auto itin : trx.itin())
  {
    if (trx.getOptions()->isCarnivalSumOfLocal() && trx.fxCnExceptionPerItin().getValForKey(itin))
      continue;

    for (const auto fm : itin->fareMarket())
    {
      if (!fm || fm->fareBasisCode().empty())
        continue;

      bool isThereGoodPTF = false;

      for (const auto ptFare : fm->allPaxTypeFare())
      {
        if (ptFare->validForCmdPricing(trx.fxCnException()) && !ptFare->isRoutingProcessed() &&
            !ptFare->bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED) &&
            !ptFare->cmdPricedWFail())
        {
          isThereGoodPTF = true;
          break;
        }
      }

      if (isThereGoodPTF)
      {
        for (const auto ptf : fm->allPaxTypeFare())
        {
          if (ptf->cmdPricedWFail())
            ptf->setNotValidForCP(true);
        }
      }
    }

    if (!releaseFares)
      break;
  }
}

//----------------------------------------------------------------------------
// postValidateAllFares()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::postValidateAllFares(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Entered FareValidatorOrchestrator::postValidateAllFares()...");
  // Set display currency
  CurrencyCode displayCurrency;
  FDFareCurrencySelection::getDisplayCurrency(trx, displayCurrency);
  Itin* itin = trx.itin().front();
  itin->calculationCurrency() = displayCurrency;

  for (const auto fareMarket : itin->fareMarket())
  {
    for (const auto paxTypeFare : fareMarket->allPaxTypeFare())
    {
      if (!paxTypeFare->isValid())
        continue;

      // Sector Surcharges need be processes with all Rule
      // request even if the applySurcharges preference is NO
      // This is to show the rule text with cat 12 if there
      // is any sector surcharge.

      if (trx.getOptions()->applySurcharges() == YES || trx.isRD())
      {
        FDSurchargesRule fdsr;
        fdsr.processSectorSurcharge(trx, *paxTypeFare);
      }

      // Currency Conversion (AAA override currency) step
      convertCurrency(trx, *paxTypeFare);
    }
  }

  LOG4CXX_INFO(logger, "Leaving FareValidatorOrchestrator::postValidateAllFares()...");
}

// -----------------------------------------------------------------------
// @MethodName  FareValidatorOrchestrator::convertCurrency()
//              Convert Fare from published currency to DisplayCurrency
//              using NUC or BSR currency conversion utilities.
// -----------------------------------------------------------------------
void
FareValidatorOrchestrator::convertCurrency(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare)
{
  LOG4CXX_INFO(logger, "Entered FareValidatorOrchestrator::convertCurrency()...");
  CurrencyConversionFacade ccFacade;
  CurrencyCode displayCurrency = trx.itin().front()->calculationCurrency();
  Money targetMoney(displayCurrency);
  Money sourceMoney(paxTypeFare.isRoundTrip() ? paxTypeFare.originalFareAmount()
                                              : paxTypeFare.fareAmount(),
                    paxTypeFare.currency());

  if (displayCurrency == paxTypeFare.currency())
  {
    paxTypeFare.nucFareAmount() = sourceMoney.value();
  }
  else if (displayCurrency == NUC)
  {
    if (paxTypeFare.isConstructed() && !paxTypeFare.isDiscounted() && !paxTypeFare.isNegotiated())
    {
      paxTypeFare.nucFareAmount() =
          paxTypeFare.fare()->constructedFareInfo()->constructedNucAmount();
    }
    else
    {
      if (paxTypeFare.isRoundTrip())
      {
        if (paxTypeFare.nucOriginalFareAmount() > 0)
        {
          paxTypeFare.nucFareAmount() = paxTypeFare.nucOriginalFareAmount();
        }
        else
        {
          paxTypeFare.nucFareAmount() *= 2.0;
        }
      }

      // else paxTypeFare.nucFareAmount() doesn't need to be modified.
    }
  }
  else
  {
    if (paxTypeFare.isConstructed())
    {
      // convert fare to NUCs, then to local currency
      sourceMoney.value() = convertFare(trx, paxTypeFare);
    }

    if (ccFacade.convert(targetMoney,
                         sourceMoney,
                         trx,
                         paxTypeFare.isInternational(),
                         CurrencyConversionRequest::FAREDISPLAY))
    {
      paxTypeFare.nucFareAmount() = targetMoney.value();
    }
  }

  // Take care of add / deduction of percentage on FT
  if (trx.getRequest()->requestType() == FARE_TAX_REQUEST)
  {
    int16_t percentage = trx.getRequest()->addSubLineNumber();
    paxTypeFare.nucFareAmount() += (paxTypeFare.nucFareAmount() * (percentage * 0.01));
  }

  // ----------------------------
  // Round nucFareAmount
  // ----------------------------
  const std::vector<ContractPreference*>& contractPrefVec = trx.dataHandle().getContractPreferences(
      paxTypeFare.vendor(), paxTypeFare.carrier(), paxTypeFare.ruleNumber(), trx.ticketingDate());
  CurrencyRoundingUtil curRoundingUtil;

  if (!contractPrefVec.empty())
  {
    if (contractPrefVec.front()->applyRoundingException() != 'X' ||
        (contractPrefVec.front()->applyRoundingException() == 'X' &&
         (trx.itin().front()->geoTravelType() == GeoTravelType::Domestic ||
          trx.itin().front()->geoTravelType() == GeoTravelType::ForeignDomestic)))
    {
      LOG4CXX_DEBUG(logger, "ROUNDING EXCEPTION INDICATOR IS OFF");
      paxTypeFare.nucFareAmount() = curRoundingUtil.roundMoneyAmount(
          paxTypeFare.nucFareAmount(), displayCurrency, paxTypeFare.currency(), trx);
    }
    else if (displayCurrency == paxTypeFare.currency() &&
             trx.itin().front()->geoTravelType() != GeoTravelType::Domestic &&
             trx.itin().front()->geoTravelType() != GeoTravelType::ForeignDomestic)
    {
      Money nonIATAsourceMoney(paxTypeFare.nucFareAmount(), paxTypeFare.currency());

      nonIATAsourceMoney.setApplyNonIATARounding();
      double roundingFactor = 0.0;
      RoundingRule roundingRule;
      CurrencyConverter cc;
      cc.round(nonIATAsourceMoney, roundingFactor, roundingRule);

      paxTypeFare.nucFareAmount() = nonIATAsourceMoney.value();
    }
  }
  else
  {
    paxTypeFare.nucFareAmount() = curRoundingUtil.roundMoneyAmount(
        paxTypeFare.nucFareAmount(), displayCurrency, paxTypeFare.currency(), trx);
  }

  LOG4CXX_INFO(logger, "Leaving FareValidatorOrchestrator::convertCurrency()...");
}

//----------------------------------------------------------------------------
// initialize()
//----------------------------------------------------------------------------
bool
FareValidatorOrchestrator::initialize(int argc, char** argv)
{
  if (!FareOrchestrator::initialize(argc, argv))
    return false;

  _newRouting = newRtg.getValue();

  return true;
}

//----------------------------------------------------------------------------
// newRouting()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::newRouting(const TseThreadingConst::TaskId taskId, PricingTrx& trx)
{
  // DataHandle dh;
  TseRunnableExecutor taskExecutor(taskId);
  RoutingController routingController(trx);

  for (int step = RoutingThread::BUILD_MAP; step <= RoutingThread::VALIDATE; ++step)
  {
    std::vector<FareMarket*>::iterator i = trx.fareMarket().begin();
    std::vector<FareMarket*>::iterator j = trx.fareMarket().end();

    for (; i != j; ++i)
    {
      if (!trx.matchFareRetrievalDate(**i) || refundSkip(trx, **i))
        continue;

      RoutingThread* thread = nullptr;
      // dh.get(thread);
      trx.dataHandle().get(thread);
      thread->trx(&trx);
      thread->desc("ROUTING THREAD");
      thread->_routingCntrl = &routingController;
      thread->_fareMarket = *i;
      thread->_step = (RoutingThread::Step)step;
      taskExecutor.execute(*thread);
    }

    taskExecutor.wait();
  }
}

//----------------------------------------------------------------------------
// RoutingThread::performTask()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::RoutingThread::performTask()
{
  if (_step == BUILD_MAP)
    _routingCntrl->buildTvlRouteMapAndProcess(*_fareMarket);
  else if (_step == VALIDATE)
    _routingCntrl->validatePaxTypeFare(*_fareMarket);
}

// -----------------------------------------------------------------------
// @MethodName  FareValidatorOrchestrator::convertFare()
//              Convert Fare from published currency to NUCs
//              Then convert Fare from NUCs to currency of specified Fare.
// -----------------------------------------------------------------------
MoneyAmount
FareValidatorOrchestrator::convertFare(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare)
{
  MoneyAmount convertedFare = 0;
  NUCCurrencyConverter nucConverter;
  Money amountNUC(0, NUC);
  Money amount(paxTypeFare.isRoundTrip() ? paxTypeFare.originalFareAmount()
                                         : paxTypeFare.fareAmount(),
               paxTypeFare.currency());
  NUCCollectionResults nucResults;
  nucResults.collect() = false;
  CurrencyConversionRequest curConvReq(amountNUC,
                                       amount,
                                       trx.getRequest()->ticketingDT(),
                                       *(trx.getRequest()),
                                       trx.dataHandle(),
                                       paxTypeFare.isInternational(),
                                       CurrencyConversionRequest::FAREDISPLAY);
  bool convertRC = nucConverter.convert(curConvReq, &nucResults);
  // Convert the NUC fare to the currency of the constructed fare
  Money amountSpecified(0, paxTypeFare.currency());
  nucResults.collect() = false;
  CurrencyConversionRequest curConvReq2(amountSpecified,
                                        amountNUC,
                                        trx.getRequest()->ticketingDT(),
                                        *(trx.getRequest()),
                                        trx.dataHandle(),
                                        paxTypeFare.isInternational(),
                                        CurrencyConversionRequest::FAREDISPLAY,
                                        false,
                                        trx.getOptions(),
                                        true);
  convertRC = nucConverter.convert(curConvReq2, &nucResults);

  if (convertRC)
  {
    convertedFare = amountSpecified.value();
  }

  return convertedFare;
}

//----------------------------------------------------------------------------
// process(AltPricingTrx)
//---------------------------------------------------------------------------
bool
FareValidatorOrchestrator::process(AltPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, " - Entered process(AltPricingTrx)");
  PricingTrx& pricingTrx = (PricingTrx&)trx;

  // Low Fare Requested can only be true if we are in the No-Match phase
  //  of WPA processing.
  //
  // IMPORTANT NOTE: This indicator will be true for regular WP pricing
  //  transactions if the NC option was used with the WP entry.
  //
  // In order to reuse this algorithm correctly for WP No-Match pricing
  //  transactions, you will need to add an indicator to the PricingTrx
  //  to tell us if we are in the second FVO pass for WP No-Match.
  //
  if (pricingTrx.getRequest()->lowFareRequested() == 'T')
  {
    checkTrxAborted(pricingTrx);
    exec_foreach_fareMarket(_taskId, pricingTrx, prepareForReprocessing);
  }

  return process(pricingTrx);
}

//----------------------------------------------------------------------------
// NoPNRPricingTrx
//---------------------------------------------------------------------------
void
FareValidatorOrchestrator::noMatchReprocess(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  ErrorResponseException::ErrorResponseCode& failCode = fareMarket.failCode();

  if (failCode == ErrorResponseException::NO_ERROR ||
      failCode == ErrorResponseException::NO_FARE_FOR_CLASS ||
      failCode == ErrorResponseException::NO_FARE_FOR_CLASS_USED)
  {
    for (auto paxTypeFare : fareMarket.allPaxTypeFare())
    {
      paxTypeFare->bookingCodeStatus() = PaxTypeFare::BKS_NOT_YET_PROCESSED;
      paxTypeFare->mixClassStatus() = PaxTypeFare::MX_NOT_APPLICABLE;
      paxTypeFare->segmentStatus().clear();
    }

    fareMarket.serviceStatus().set(FareMarket::FareValidator, false);
    failCode = ErrorResponseException::NO_ERROR;
  }
}

bool
FareValidatorOrchestrator::process(NoPNRPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, " - Entered process(NoPNRPricingTrx)");
  PricingTrx& pricingTrx = (PricingTrx&)trx;

  if (trx.reprocess())
  {
    checkTrxAborted(pricingTrx);
    exec_foreach_fareMarket(_taskId, pricingTrx, noMatchReprocess);
  }

  return process(pricingTrx);
}

//----------------------------------------------------------------------------
// process(ExchangePricingTrx)
//---------------------------------------------------------------------------
bool
FareValidatorOrchestrator::process(ExchangePricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, " - Entered process(ExchangePricingTrx)");
  return process(static_cast<PricingTrx&>(trx));
}

//------------------------------------------------------------
// Create flight bitmap for all duration
// main duration will not be in the duration flight bit map
// It will reuse original flight bit map
//------------------------------------------------------------
void
FareValidatorOrchestrator::createAltDateFBMforAllDuration(ShoppingTrx& trx,
                                                          PaxTypeFare* paxTypeFare)
{
  if ((trx.durationAltDatePairs().size() <= 1) && (trx.getTrxType() != PricingTrx::FF_TRX))
  {
    return;
  }

  int FlightBitmapSize = paxTypeFare->getFlightBitmapSize();
  VecMap<uint64_t, PaxTypeFare::FlightBitmap>& durationFlightBitmap =
      paxTypeFare->durationFlightBitmap();
  std::map<uint64_t, PricingTrx::AltDatePairs>::const_iterator iter = trx.durationAltDatePairs()
                                                                          .begin(),
                                                               iterEnd =
                                                                   trx.durationAltDatePairs().end();

  for (; iter != iterEnd; iter++)
  {
    if ((iter->first != trx.mainDuration()) || (trx.getTrxType() == PricingTrx::FF_TRX) ||
        (trx.getTrxType() == PricingTrx::IS_TRX))
    {
      PaxTypeFare::FlightBitmap flightBitmap;
      flightBitmap.resize(FlightBitmapSize);

      // start with the SKIP
      for (int bitIndex = 0; bitIndex < FlightBitmapSize; ++bitIndex)
      {
        paxTypeFare->setFlightInvalid(flightBitmap, bitIndex, RuleConst::SKIP);
      }

      std::pair<uint64_t, PaxTypeFare::FlightBitmap> flightBitmapItem(iter->first, flightBitmap);
      durationFlightBitmap.insert(flightBitmapItem);
    }
  }
}

//------------------------------------------------------------
// Create flight bitmap for all duration
// main duration will not be in the duration flight bit map
// It will reuse original flight bit map
//------------------------------------------------------------
void
FareValidatorOrchestrator::createAltDateFBMforAllDurationNew(ShoppingTrx& trx,
                                                             const ItinIndex::Key& carrierKey,
                                                             const uint32_t& bitmapSize,
                                                             PaxTypeFare* paxTypeFare)
{
  std::map<uint64_t, PricingTrx::AltDatePairs>::const_iterator iter = trx.durationAltDatePairs()
                                                                          .begin(),
                                                               iterEnd =
                                                                   trx.durationAltDatePairs().end();

  for (; iter != iterEnd; iter++)
  {
    paxTypeFare->setComponentValidationForCarrier(carrierKey, trx.isAltDates(), iter->first);

    if (paxTypeFare->getFlightBitmapSize() != bitmapSize)
    {
      paxTypeFare->flightBitmap().resize(bitmapSize);

      for (uint32_t bitIndex = 0; bitIndex < paxTypeFare->getFlightBitmapSize(); ++bitIndex)
        paxTypeFare->setFlightInvalid(bitIndex, RuleConst::SKIP);
    }
  }
}

//-------------------------------------------------------------------
// set flightbitmap status from non flight related rule validation
//-------------------------------------------------------------------
void
FareValidatorOrchestrator::setFBMStatusForAllDuration(const uint32_t& bitIndex,
                                                      const DateTime& dateTime,
                                                      uint8_t legId,
                                                      PaxTypeFare* paxTypeFare,
                                                      ShoppingTrx& trx)
{
  if (paxTypeFare->durationFlightBitmap().empty())
  {
    return;
  }

  VecMap<uint64_t, PaxTypeFare::FlightBitmap>::iterator
  iter = paxTypeFare->durationFlightBitmap().begin(),
  iterEnd = paxTypeFare->durationFlightBitmap().end();

  for (; iter != iterEnd; iter++)
  {
    PricingTrx::DurationAltDatePairs::const_iterator i =
        trx.durationAltDatePairs().find(iter->first);

    if (i == trx.durationAltDatePairs().end())
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
    {
      paxTypeFare->setFlightInvalid(
          iter->second, bitIndex, paxTypeFare->getAltDateStatus(datePair));
    }
    else
    {
      paxTypeFare->setFlightInvalid(iter->second, bitIndex, RuleConst::SKIP);
    }
  }
}

//-------------------------------------------------------------------
// set flightbitmap status from non flight related rule validation
//-------------------------------------------------------------------
void
FareValidatorOrchestrator::setBitmapStatusForDatePair(const uint32_t& bitIndex,
                                                      PaxTypeFare* paxTypeFare,
                                                      const DatePair& datePair)
{
  uint64_t duration =
      (datePair.second.get64BitRepDateOnly() - datePair.first.get64BitRepDateOnly());
  VecMap<uint64_t, PaxTypeFare::FlightBitmap>::iterator iter =
      paxTypeFare->durationFlightBitmap().find(duration);

  if (paxTypeFare->isFlightValid(bitIndex))
  {
    paxTypeFare->setFlightInvalid(iter->second, bitIndex, 0); // flight valid
  }
  else
  {
    paxTypeFare->setFlightInvalid(iter->second, bitIndex, *(paxTypeFare->getFlightBit(bitIndex)));
  }
}

//----------------------------------------------------------------------------
// process(RepricingTrx)
//---------------------------------------------------------------------------
bool
FareValidatorOrchestrator::process(RepricingTrx& trx)
{
  if (trx.skipRuleValidation())
    return true;

  return process((PricingTrx&)trx);
}

////////////////////////////////
// Flight Finder - Step ONE   //
////////////////////////////////
void
FareValidatorOrchestrator::makeFlightList(FlightFinderTrx& fFTrx)
{
  ShoppingTrx& shoppingTrx = static_cast<ShoppingTrx&>(fFTrx);
  uint32_t legId = 0;

  // Loop through the legs of the transaction
  for (auto& leg : fFTrx.legs())
  {
    ItinIndex& curCxrIdx = leg.carrierIndex();

    if (curCxrIdx.root().empty())
    {
      ++legId;
      continue;
    }

    for (auto& itinMatrix : curCxrIdx.root())
    {
      auto fM = getFirstFareMarket(shoppingTrx, legId, itinMatrix.first);
      if (fM == nullptr)
      {
        continue;
      }

      std::vector<FlightFinderTrx::FlightBitInfo> combinedFlightBitmap;

      // create flight bitmap with union of valid bits for all PaxTypeFares
      for (auto curFare : fM->allPaxTypeFare())
      {
        if (curFare && curFare->isValid())
        {
          combineFlightBitmapsForEachPaxTypeFare(
              combinedFlightBitmap, curFare, curFare->flightBitmap(), nullptr);
        }
      }

      makeFlightListGoThroughCombinedBitmap(
          fFTrx, legId, combinedFlightBitmap, curCxrIdx, itinMatrix.first);
    }

    ++legId;
  }
}

void
FareValidatorOrchestrator::makeFlightListGoThroughCombinedBitmap(
    FlightFinderTrx& fFTrx,
    uint16_t legID,
    std::vector<FlightFinderTrx::FlightBitInfo>& combinedFlightBitmap,
    ItinIndex& curCxrIdx,
    const ItinIndex::Key& key)
{
  ItinIndex::ItinIndexIterator iRCIter = curCxrIdx.beginRow(key);
  ItinIndex::ItinIndexIterator iRCEIter = curCxrIdx.endRow();
  uint32_t sopIndex = 0;

  if (!combinedFlightBitmap.empty() && fFTrx.legs().size() > legID)
  {
    std::vector<ShoppingTrx::SchedulingOption>& sop = fFTrx.legs()[legID].sop();
    // retrieve departure date
    DateTime departureDT;

    if (fFTrx.journeyItin() && fFTrx.journeyItin()->travelSeg().size() > legID)
    {
      departureDT = fFTrx.journeyItin()->travelSeg()[legID]->departureDT();
    }

    // Go through combined flight bitmap
    // and SOPs in Matrix pararelaly
    for (uint32_t bitmapIndex = 0; iRCIter != iRCEIter, bitmapIndex < combinedFlightBitmap.size();
         ++iRCIter, ++bitmapIndex)
    {
      if (combinedFlightBitmap[bitmapIndex].flightBitStatus == 0 ||
          (fFTrx.bffStep() == FlightFinderTrx::STEP_6 && legID == 0))
      {
        // std::pair<ItinCellInfo, Itin*> ItinCell
        sopIndex = iRCIter /*ItinCell*/->first /*ItinCellInfo*/ .sopIndex();

        if (sop.size() > sopIndex)
        {
          std::vector<std::vector<FlightFinderTrx::BookingCodeData>> bkgCodeDataVect;

          if (!(fFTrx.bffStep() == FlightFinderTrx::STEP_6 && legID == 0))
          {
            getBookingCodeVectVect(bkgCodeDataVect,
                                   sop[sopIndex].thrufareClassOfService(),
                                   bitmapIndex,
                                   sop[sopIndex].itin()->travelSeg(),
                                   combinedFlightBitmap[bitmapIndex].paxTypeFareVect);
          }

          makeFlightListAddSop(fFTrx,
                               legID,
                               departureDT,
                               sopIndex,
                               combinedFlightBitmap[bitmapIndex].paxTypeFareVect,
                               bkgCodeDataVect);
        }
      }
    }
  }
}

void
FareValidatorOrchestrator::makeFlightListAddSop(
    FlightFinderTrx& fFTrx,
    uint16_t legID,
    DateTime& departureDT,
    uint32_t sopIndex,
    std::vector<PaxTypeFare*>& paxTypeFareVect,
    std::vector<std::vector<FlightFinderTrx::BookingCodeData>>& bkgCodeDataVect)
{
  if (legID == 0) // outbound
  {
    addOutboundToFlightList(fFTrx, departureDT, sopIndex, paxTypeFareVect, bkgCodeDataVect);
  }
  else if (legID == 1) // inbound
  {
    // add to first outbaund date
    addInboundToFlightList(fFTrx,
                           fFTrx.journeyItin()->travelSeg()[0]->departureDT(),
                           departureDT,
                           sopIndex,
                           paxTypeFareVect,
                           bkgCodeDataVect);
  }
}

void
FareValidatorOrchestrator::addOutboundToFlightList(
    FlightFinderTrx& fFTrx,
    const DateTime& outboundDepartureDT,
    const uint32_t sopIndex,
    std::vector<PaxTypeFare*>& paxTypeFareVect,
    std::vector<std::vector<FlightFinderTrx::BookingCodeData>>& bkgCodeDataVect)
{
  if (fFTrx.outboundDateflightMap().count(outboundDepartureDT) == 0)
  {
    FlightFinderTrx::OutBoundDateInfo* outBoundDateInfoPtr = nullptr;
    fFTrx.dataHandle().get(outBoundDateInfoPtr);
    FlightFinderTrx::SopInfo* sopInfo = nullptr;
    fFTrx.dataHandle().get(sopInfo);
    sopInfo->sopIndex = sopIndex;
    sopInfo->bkgCodeDataVect.insert(
        sopInfo->bkgCodeDataVect.end(), bkgCodeDataVect.begin(), bkgCodeDataVect.end());
    sopInfo->paxTypeFareVect.insert(
        sopInfo->paxTypeFareVect.end(), paxTypeFareVect.begin(), paxTypeFareVect.end());
    outBoundDateInfoPtr->flightInfo.flightList.push_back(sopInfo);
    fFTrx.outboundDateflightMap()[outboundDepartureDT] = outBoundDateInfoPtr;
  }
  else
  {
    // if exist add next sopIndex to this date
    if (fFTrx.outboundDateflightMap()[outboundDepartureDT] != nullptr)
    {
      FlightFinderTrx::SopInfo* sopInfo = nullptr;
      fFTrx.dataHandle().get(sopInfo);
      sopInfo->sopIndex = sopIndex;
      sopInfo->bkgCodeDataVect.insert(
          sopInfo->bkgCodeDataVect.end(), bkgCodeDataVect.begin(), bkgCodeDataVect.end());
      sopInfo->paxTypeFareVect.insert(
          sopInfo->paxTypeFareVect.end(), paxTypeFareVect.begin(), paxTypeFareVect.end());
      fFTrx.outboundDateflightMap()[outboundDepartureDT]->flightInfo.flightList.push_back(sopInfo);
    }
  }
}

void
FareValidatorOrchestrator::addInboundToFlightList(
    FlightFinderTrx& fFTrx,
    const DateTime& outboundDepartureDT,
    const DateTime& inboundDepartureDT,
    const uint32_t sopIndex,
    std::vector<PaxTypeFare*>& paxTypeFareVect,
    std::vector<std::vector<FlightFinderTrx::BookingCodeData>>& bkgCodeDataVect)
{
  FlightFinderTrx::OutBoundDateFlightMap::const_iterator it =
      fFTrx.outboundDateflightMap().find(outboundDepartureDT);

  if (it != fFTrx.outboundDateflightMap().end())
  {
    if (it->second->iBDateFlightMap.count(inboundDepartureDT) == 0)
    {
      FlightFinderTrx::FlightDataInfo* flightInfo = nullptr;
      fFTrx.dataHandle().get(flightInfo);
      FlightFinderTrx::SopInfo* sopInfo = nullptr;
      fFTrx.dataHandle().get(sopInfo);
      sopInfo->sopIndex = sopIndex;
      sopInfo->bkgCodeDataVect.insert(
          sopInfo->bkgCodeDataVect.end(), bkgCodeDataVect.begin(), bkgCodeDataVect.end());
      sopInfo->paxTypeFareVect.insert(
          sopInfo->paxTypeFareVect.end(), paxTypeFareVect.begin(), paxTypeFareVect.end());
      flightInfo->flightList.push_back(sopInfo);
      it->second->iBDateFlightMap[inboundDepartureDT] = flightInfo;
    }
    else
    {
      // add to existing one
      FlightFinderTrx::SopInfo* sopInfo = nullptr;
      fFTrx.dataHandle().get(sopInfo);
      sopInfo->sopIndex = sopIndex;
      sopInfo->bkgCodeDataVect.insert(
          sopInfo->bkgCodeDataVect.end(), bkgCodeDataVect.begin(), bkgCodeDataVect.end());
      sopInfo->paxTypeFareVect.insert(
          sopInfo->paxTypeFareVect.end(), paxTypeFareVect.begin(), paxTypeFareVect.end());
      it->second->iBDateFlightMap[inboundDepartureDT]->flightList.push_back(sopInfo);
    }
  }
}

////////////////////////////////
// Flight Finder - Step TWO   //
////////////////////////////////
void
FareValidatorOrchestrator::prepareAltDatePairs(
    FlightFinderTrx& fFTrx, std::map<DatePair, FlightFinderTrx::FlightBitInfo>& dataPairsMap)
{
  if (!fFTrx.ignoreDiagReq() && (fFTrx.diagnostic().diagnosticType() == Diagnostic904 ||
                                 fFTrx.diagnostic().diagnosticType() == Diagnostic911))
  {
    return;
  }

  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(fFTrx);
  uint32_t legId = 0;
  std::vector<std::map<DatePair, FlightFinderTrx::FlightBitInfo>> combinedAltDateStatus;
  std::vector<PaxTypeFare*> paxTypeFarePassedForRequestedDateVect;

  // Loop through the legs of the transaction
  for (auto& leg : fFTrx.legs())

  {
    ItinIndex& curCxrIdx = leg.carrierIndex();

    if (curCxrIdx.root().empty())
    {
      ++legId;
      continue;
    }

    std::map<DatePair, FlightFinderTrx::FlightBitInfo> combinedAltDateStatusPerLeg;

    for (auto& itinMatrix : curCxrIdx.root())
    {
      // take first market because ony this one has fares

      auto fM = getFirstFareMarket(shoppingTrx, legId, itinMatrix.first);

      if (fM == nullptr)
      {
        continue;
      }

      // create flight bitmap with union of valid bits for all PaxTypeFares
      for (auto paxTypeFare : fM->allPaxTypeFare())
      {
        if (paxTypeFare->isKeepForRoutingValidation())
        {
          continue;
        }

        if (paxTypeFare && paxTypeFare->isValid())
        {
          if ((legId == 0 && fFTrx.legsStatus().isSet(FlightFinderTrx::LEG1_HAS_VLD_FARE)) ||
              (legId == 1 && fFTrx.legsStatus().isSet(FlightFinderTrx::LEG2_HAS_VLD_FARE)))
          {
            paxTypeFarePassedForRequestedDateVect.push_back(paxTypeFare);
          }

          combineAltDateStatus(combinedAltDateStatusPerLeg, paxTypeFare, legId);
        }
      }
    }

    combinedAltDateStatus.push_back(combinedAltDateStatusPerLeg);
    ++legId;
  }

  combineAltDateStatusPerLeg(
      dataPairsMap, combinedAltDateStatus, paxTypeFarePassedForRequestedDateVect);
}

void
FareValidatorOrchestrator::makeAltDateFlightList(
    FlightFinderTrx& fFTrx, std::map<DatePair, FlightFinderTrx::FlightBitInfo>& dataPairsMap)
{
  if (!fFTrx.ignoreDiagReq() && (fFTrx.diagnostic().diagnosticType() == Diagnostic965))
  {
    return;
  }

  std::vector<std::pair<DatePair, FlightFinderTrx::FlightBitInfo>> boundedCombinedAltDateStatus;

  if (fFTrx.isBffReq())
  {
    makeAltDateFlightListGoThroughCombinedBitmap(fFTrx, dataPairsMap);
  }
  else
  {
    uint8_t desiredNumberOfOutboundDates = 7;
    uint8_t desiredNumberOfInboundDates = 7;
    std::pair<DateTime, DateTime> originalPair;
    originalPair.first = fFTrx.journeyItin()->travelSeg().front()->departureDT();

    if (fFTrx.legs().size() > 1)
    {
      originalPair.second = fFTrx.journeyItin()->travelSeg().back()->departureDT();
    }
    else
    {
      originalPair.second = DateTime::emptyDate();
    }

    AltDatesPairOptionsBounder::pickUpDesiredNumberOfPairs(desiredNumberOfOutboundDates,
                                                           desiredNumberOfInboundDates,
                                                           originalPair,
                                                           dataPairsMap,
                                                           boundedCombinedAltDateStatus,
                                                           fFTrx);
    makeAltDateFlightListGoThroughCombinedBitmap(fFTrx, boundedCombinedAltDateStatus);
  }
}

void
FareValidatorOrchestrator::makeAltDateFlightListGoThroughCombinedBitmap(
    FlightFinderTrx& fFTrx, std::map<DatePair, FlightFinderTrx::FlightBitInfo>& dataPairsMap)
{
  std::map<DatePair, FlightFinderTrx::FlightBitInfo>::iterator itAltDateStatus =
      dataPairsMap.begin();

  for (; itAltDateStatus != dataPairsMap.end(); ++itAltDateStatus)
  {
    if (itAltDateStatus->second.flightBitStatus == 0) // AltDateStatus is valid
    {
      // add inbound with outbound
      addAltDateToFlightList(fFTrx,
                             itAltDateStatus->first.first,
                             itAltDateStatus->first.second,
                             itAltDateStatus->second.paxTypeFareVect,
                             itAltDateStatus->second.inboundPaxTypeFareVect);
    }
  }
}

void
FareValidatorOrchestrator::makeAltDateFlightListGoThroughCombinedBitmap(
    FlightFinderTrx& fFTrx,
    std::vector<std::pair<DatePair, FlightFinderTrx::FlightBitInfo>>& combinedAltDateStatus)
{
  std::vector<std::pair<DatePair, FlightFinderTrx::FlightBitInfo>>::iterator itAltDateStatus =
      combinedAltDateStatus.begin();

  for (; itAltDateStatus != combinedAltDateStatus.end(); ++itAltDateStatus)
  {
    if (itAltDateStatus->second.flightBitStatus == 0) // AltDateStatus is valid
    {
      // add inbound with outbound
      addAltDateToFlightList(fFTrx,
                             itAltDateStatus->first.first,
                             itAltDateStatus->first.second,
                             itAltDateStatus->second.paxTypeFareVect,
                             itAltDateStatus->second.inboundPaxTypeFareVect);
    }
  }
}

void
FareValidatorOrchestrator::addAltDateToFlightList(FlightFinderTrx& fFTrx,
                                                  const DateTime& outboundDepartureDT,
                                                  const DateTime& inboundDepartureDT,
                                                  std::vector<PaxTypeFare*>& outboundFareVect,
                                                  std::vector<PaxTypeFare*>& inboundFareVect)
{
  FlightFinderTrx::OutBoundDateInfo* outBoundDateInfoPtr = nullptr;

  if (fFTrx.outboundDateflightMap().count(outboundDepartureDT) == 0)
  {
    // if outBoundDateInfo doesnt exist, have to allocate it
    fFTrx.dataHandle().get(outBoundDateInfoPtr);

    // if it's NOT only outbaund
    if (!inboundDepartureDT.isEmptyDate())
    {
      FlightFinderTrx::FlightDataInfo* flightInfo = nullptr;
      fFTrx.dataHandle().get(flightInfo);
      flightInfo->altDatesPaxTypeFareVect.swap(inboundFareVect);
      outBoundDateInfoPtr->iBDateFlightMap[inboundDepartureDT] = flightInfo;
    }

    outBoundDateInfoPtr->flightInfo.altDatesPaxTypeFareVect.swap(outboundFareVect);
    fFTrx.outboundDateflightMap()[outboundDepartureDT] = outBoundDateInfoPtr;
  }
  else
  {
    // add inbound for existing outbound
    if (!inboundDepartureDT.isEmptyDate())
    {
      // if outbound already exist
      outBoundDateInfoPtr = fFTrx.outboundDateflightMap()[outboundDepartureDT];

      if (outBoundDateInfoPtr->iBDateFlightMap.count(inboundDepartureDT) == 0)
      {
        FlightFinderTrx::FlightDataInfo* flightInfo = nullptr;
        fFTrx.dataHandle().get(flightInfo);
        flightInfo->altDatesPaxTypeFareVect.swap(inboundFareVect);
        outBoundDateInfoPtr->iBDateFlightMap[inboundDepartureDT] = flightInfo;
      }
    }
  }
}

void
FareValidatorOrchestrator::combineAltDateStatus(
    std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDateStatus,
    PaxTypeFare* curFare,
    const uint16_t legID)
{
  VecMap<DatePair, uint8_t>::const_iterator itAltDateStatus = curFare->altDateStatus().begin();

  if (combinedAltDateStatus.empty()) // create combinedAltDateStatus
  {
    for (; itAltDateStatus != curFare->altDateStatus().end(); ++itAltDateStatus)
    {
      FlightFinderTrx::FlightBitInfo flightBInfo;
      flightBInfo.flightBitStatus = itAltDateStatus->second;

      if (legID == 0)
      {
        flightBInfo.paxTypeFareVect.push_back(curFare);
      }
      else
      {
        flightBInfo.inboundPaxTypeFareVect.push_back(curFare);
      }

      combinedAltDateStatus[itAltDateStatus->first] = flightBInfo;
    }
  }
  else // combine combinedAltDateStatus (0 + 0 = 0, 0 + F = 0, F + 0 = 0, F + F = F)
  {
    for (; itAltDateStatus != curFare->altDateStatus().end(); ++itAltDateStatus)
    {
      if (itAltDateStatus->second == 0)
      {
        if (combinedAltDateStatus[itAltDateStatus->first].flightBitStatus != 0)
        {
          combinedAltDateStatus[itAltDateStatus->first].flightBitStatus = itAltDateStatus->second;
        }

        if (legID == 0)
        {
          combinedAltDateStatus[itAltDateStatus->first].paxTypeFareVect.push_back(curFare);
        }
        else
        {
          combinedAltDateStatus[itAltDateStatus->first].inboundPaxTypeFareVect.push_back(curFare);
        }
      }
    }
  }
}

void
FareValidatorOrchestrator::combineAltDateStatusPerLeg(
    std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDatePairsWithStatuses,
    std::vector<std::map<DatePair, FlightFinderTrx::FlightBitInfo>>& combinedAltDateStatusPerLeg,
    std::vector<PaxTypeFare*>& paxTypeFarePassedForRequestedDateVect)
{
  if (combinedAltDateStatusPerLeg.size() > 0)
  {
    if (combinedAltDateStatusPerLeg.size() == 1) // OW
    {
      combinedAltDatePairsWithStatuses.swap(combinedAltDateStatusPerLeg.front());
    }
    else // RT
    {
      if (combinedAltDateStatusPerLeg.front().size() == 0) // orig out valid
      {
        combinedAltDatePairsWithStatuses.swap(combinedAltDateStatusPerLeg.back());
        std::map<DatePair, FlightFinderTrx::FlightBitInfo>::iterator altDateObjIter =
            combinedAltDatePairsWithStatuses.begin();

        for (; altDateObjIter != combinedAltDatePairsWithStatuses.end(); ++altDateObjIter)
        {
          altDateObjIter->second.paxTypeFareVect.insert(
              altDateObjIter->second.paxTypeFareVect.end(),
              paxTypeFarePassedForRequestedDateVect.begin(),
              paxTypeFarePassedForRequestedDateVect.end());
        }
      }
      else if (combinedAltDateStatusPerLeg.back().size() == 0) // orig in valid
      {
        combinedAltDatePairsWithStatuses.swap(combinedAltDateStatusPerLeg.front());
        std::map<DatePair, FlightFinderTrx::FlightBitInfo>::iterator altDateObjIter =
            combinedAltDatePairsWithStatuses.begin();

        for (; altDateObjIter != combinedAltDatePairsWithStatuses.end(); ++altDateObjIter)
        {
          altDateObjIter->second.inboundPaxTypeFareVect.insert(
              altDateObjIter->second.inboundPaxTypeFareVect.end(),
              paxTypeFarePassedForRequestedDateVect.begin(),
              paxTypeFarePassedForRequestedDateVect.end());
        }
      }
      else // combine status in pairs in two legs (0 + 0 = 0, 0 + F = F, F + 0 = F, F + F = F)
      {
        combinedAltDatePairsWithStatuses.swap(combinedAltDateStatusPerLeg.front());
        std::map<DatePair, FlightFinderTrx::FlightBitInfo>::iterator altDateObjOutboundIter =
            combinedAltDateStatusPerLeg.back().begin();

        for (; altDateObjOutboundIter != combinedAltDateStatusPerLeg.back().end();
             ++altDateObjOutboundIter)
        {
          if (altDateObjOutboundIter->second.flightBitStatus != 0)
          {
            combinedAltDatePairsWithStatuses[altDateObjOutboundIter->first].flightBitStatus =
                altDateObjOutboundIter->second.flightBitStatus;
          }
          else
          {
            combinedAltDatePairsWithStatuses[altDateObjOutboundIter->first]
                .inboundPaxTypeFareVect.insert(
                    combinedAltDatePairsWithStatuses[altDateObjOutboundIter->first]
                        .inboundPaxTypeFareVect.end(),
                    altDateObjOutboundIter->second.inboundPaxTypeFareVect.begin(),
                    altDateObjOutboundIter->second.inboundPaxTypeFareVect.end());
          }
        }
      }
    }
  }
}

////////////////////////////////
// Flight Finder - Step THREE //
////////////////////////////////
void
FareValidatorOrchestrator::makeAltDateFlightListWithSOP(FlightFinderTrx& fFTrx)
{
  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(fFTrx);
  uint32_t legId = 0;
  std::map<uint64_t, std::map<DateTime, FlightFinderTrx::FlightDataInfo>>* tempDurationOutboundMap =
      nullptr;

  if (fFTrx.legs().size() > 1)
  {
    fFTrx.dataHandle().get(tempDurationOutboundMap);
  }

  // Loop through the legs of the transaction
  for (auto& leg : fFTrx.legs())
  {
    ItinIndex& curCxrIdx = leg.carrierIndex();

    if (curCxrIdx.root().empty())
    {
      ++legId;
      continue;
    }

    std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo>> combinedDurationFlightBitmap;

    for (auto& itinMatrix : curCxrIdx.root())
    {
      // take first market because ony this one has fares
      ItinIndex::ItinCell* itinCell = ShoppingUtil::retrieveDirectItin(
          shoppingTrx, legId, itinMatrix.first, ItinIndex::CHECK_NOTHING);

      if (!itinCell)
        continue;

      Itin* curItinCellItin = itinCell->second;

      if (curItinCellItin == nullptr)
        continue;
      // take first market because ony this one has fares
      auto fM = curItinCellItin->fareMarket().front();
      if (fM == nullptr)
        continue;

      fM->setGlobalDirection(itinCell->first.globalDirection());

      // create flight bitmap with union of valid bits for all PaxTypeFares
      for (auto curFare : fM->allPaxTypeFare())
      {
        if (curFare && curFare->isValid())
        {
          combineDurationsFlightBitmapsForEachPaxTypeFare(
              combinedDurationFlightBitmap, curFare, fFTrx);
        }
      }

      makeAltDateFlightListWithSOPGoThroughDurations(fFTrx,
                                                     legId,
                                                     combinedDurationFlightBitmap,
                                                     curCxrIdx,
                                                     itinMatrix.first,
                                                     tempDurationOutboundMap);
    }

    ++legId;
  }
}

void
FareValidatorOrchestrator::makeAltDateFlightListWithSOPGoThroughDurations(
    FlightFinderTrx& fFTrx,
    const uint16_t legId,
    std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo>>& combinedDurationFlightBitmap,
    ItinIndex& curCxrIdx,
    const ItinIndex::Key& key,
    std::map<uint64_t, std::map<DateTime, FlightFinderTrx::FlightDataInfo>>*
        tempDurationOutboundMap)
{
  uint64_t duration = 0;

  if (combinedDurationFlightBitmap.size() > 0)
  {
    std::vector<bool> thisSOPWasAlreadyAdded(combinedDurationFlightBitmap.begin()->second.size(),
                                             false);
    std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo>>::iterator
    itCombinedDurationFlightBitmap = combinedDurationFlightBitmap.begin();

    for (; itCombinedDurationFlightBitmap != combinedDurationFlightBitmap.end();
         ++itCombinedDurationFlightBitmap)
    {
      duration = itCombinedDurationFlightBitmap->first;
      makeAltDateFlightListWithSOPGoThroughCombinedBitmap(fFTrx,
                                                          legId,
                                                          duration,
                                                          itCombinedDurationFlightBitmap->second,
                                                          curCxrIdx,
                                                          key,
                                                          thisSOPWasAlreadyAdded,
                                                          tempDurationOutboundMap);
    }
  }
}

void
FareValidatorOrchestrator::makeAltDateFlightListWithSOPGoThroughCombinedBitmap(
    FlightFinderTrx& fFTrx,
    const uint16_t legId,
    uint64_t& duration,
    std::vector<FlightFinderTrx::FlightBitInfo>& combinedFlightBitmap,
    ItinIndex& curCxrIdx,
    const ItinIndex::Key& key,
    std::vector<bool>& thisSOPWasAlreadyAdded,
    std::map<uint64_t, std::map<DateTime, FlightFinderTrx::FlightDataInfo>>*
        tempDurationOutboundMap)
{
  ItinIndex::ItinIndexIterator iRCIter = curCxrIdx.beginRow(key);
  ItinIndex::ItinIndexIterator iRCEIter = curCxrIdx.endRow();
  uint32_t sopIndex = 0;

  if (!combinedFlightBitmap.empty() && fFTrx.legs().size() > legId)
  {
    std::vector<ShoppingTrx::SchedulingOption>& sop = fFTrx.legs()[legId].sop();

    // Go through combined flight bitmap
    // and SOPs in Matrix pararelaly
    for (uint32_t bitmapIndex = 0; iRCIter != iRCEIter, bitmapIndex < combinedFlightBitmap.size();
         ++iRCIter, ++bitmapIndex)
    {
      if (!thisSOPWasAlreadyAdded[bitmapIndex] &&
          (combinedFlightBitmap[bitmapIndex].flightBitStatus == 0 ||
           (fFTrx.bffStep() == FlightFinderTrx::STEP_4 && legId == 0)))
      {
        DateTime& departureDT = iRCIter->second->travelSeg().front()->departureDT();
        sopIndex = iRCIter->first.sopIndex(); // ItinCell->ItinCellInfo.sopIndex

        if (sop.size() > sopIndex)
        {
          std::vector<std::vector<FlightFinderTrx::BookingCodeData>> bkgCodeDataVect;

          if (fFTrx.bffStep() != FlightFinderTrx::STEP_4)
          {
            getBookingCodeVectVect(bkgCodeDataVect,
                                   sop[sopIndex].thrufareClassOfService(),
                                   bitmapIndex,
                                   duration,
                                   sop[sopIndex].itin()->travelSeg(),
                                   combinedFlightBitmap[bitmapIndex].paxTypeFareVect);
          }

          // check durationAltDatePairs and add to flight list
          thisSOPWasAlreadyAdded[bitmapIndex] =
              makeAltDateFlightListWithSOPAddSop(fFTrx,
                                                 duration,
                                                 departureDT,
                                                 legId,
                                                 sopIndex,
                                                 tempDurationOutboundMap,
                                                 combinedFlightBitmap[bitmapIndex].paxTypeFareVect,
                                                 bkgCodeDataVect);
        }
      }
    }
  }
}

bool
FareValidatorOrchestrator::makeAltDateFlightListWithSOPAddSop(
    FlightFinderTrx& fFTrx,
    uint64_t& duration,
    DateTime& departureDT,
    const uint16_t legID,
    uint32_t sopIndex,
    std::map<uint64_t, std::map<DateTime, FlightFinderTrx::FlightDataInfo>>*
        tempDurationOutboundMap,
    std::vector<PaxTypeFare*> paxTypeFareVect,
    std::vector<std::vector<FlightFinderTrx::BookingCodeData>>& bkgCodeDataVect)
{
  DatePair datePair;
  ShoppingTrx::AltDatePairs& altDatePairs = (fFTrx.durationAltDatePairs().find(duration))->second;

  if (ShoppingAltDateUtil::getDatePair(altDatePairs, departureDT, legID, datePair))
  {
    if (legID == 0) // outbound
    {
      // add outbound
      if (fFTrx.legs().size() > 1 && nullptr != tempDurationOutboundMap)
      {
        FlightFinderTrx::SopInfo* sopInfo = nullptr;
        fFTrx.dataHandle().get(sopInfo);
        sopInfo->sopIndex = sopIndex;
        sopInfo->bkgCodeDataVect.insert(
            sopInfo->bkgCodeDataVect.end(), bkgCodeDataVect.begin(), bkgCodeDataVect.end());
        sopInfo->paxTypeFareVect.insert(
            sopInfo->paxTypeFareVect.end(), paxTypeFareVect.begin(), paxTypeFareVect.end());
        (*tempDurationOutboundMap)[duration][datePair.first].flightList.push_back(sopInfo);
      }
      else
      {
        addOutboundToFlightList(fFTrx, datePair.first, sopIndex, paxTypeFareVect, bkgCodeDataVect);
        return true;
      }
    }
    else if (legID == 1) // inbound
    {
      // add inbound
      if (fFTrx.legs().size() > 1 && nullptr != tempDurationOutboundMap)
      {
        addInboundToFlightListForRT(fFTrx,
                                    datePair.first, // outbound
                                    datePair.second, // inbound
                                    sopIndex,
                                    tempDurationOutboundMap,
                                    duration,
                                    paxTypeFareVect,
                                    bkgCodeDataVect);
      }
      else
      {
        addInboundToFlightList(
            fFTrx, datePair.first, datePair.second, sopIndex, paxTypeFareVect, bkgCodeDataVect);
      }
    }
  }

  return false;
}

void
FareValidatorOrchestrator::addInboundToFlightListForRT(
    FlightFinderTrx& fFTrx,
    const DateTime& outboundDepartureDT,
    const DateTime& inboundDepartureDT,
    const uint32_t sopIndex,
    std::map<uint64_t, std::map<DateTime, FlightFinderTrx::FlightDataInfo>>*
        tempDurationOutboundMap,
    uint64_t& duration,
    std::vector<PaxTypeFare*>& paxTypeFareVect,
    std::vector<std::vector<FlightFinderTrx::BookingCodeData>>& bkgCodeDataVect)
{
  if (fFTrx.outboundDateflightMap().count(outboundDepartureDT) == 0)
  {
    if ((*tempDurationOutboundMap)[duration].count(outboundDepartureDT) != 0)
    {
      // add inbound with outbound
      FlightFinderTrx::OutBoundDateInfo* outBoundDateInfoPtr = nullptr;
      fFTrx.dataHandle().get(outBoundDateInfoPtr);
      outBoundDateInfoPtr->flightInfo.flightList.swap(
          (*tempDurationOutboundMap)[duration][outboundDepartureDT].flightList);
      FlightFinderTrx::FlightDataInfo* flightInfo = nullptr;
      fFTrx.dataHandle().get(flightInfo);
      FlightFinderTrx::SopInfo* sopInfo = nullptr;
      fFTrx.dataHandle().get(sopInfo);
      sopInfo->sopIndex = sopIndex;
      sopInfo->bkgCodeDataVect.insert(
          sopInfo->bkgCodeDataVect.end(), bkgCodeDataVect.begin(), bkgCodeDataVect.end());
      sopInfo->paxTypeFareVect.insert(
          sopInfo->paxTypeFareVect.end(), paxTypeFareVect.begin(), paxTypeFareVect.end());
      flightInfo->flightList.push_back(sopInfo);
      outBoundDateInfoPtr->iBDateFlightMap[inboundDepartureDT] = flightInfo;
      fFTrx.outboundDateflightMap()[outboundDepartureDT] = outBoundDateInfoPtr;
    }
  }
  else
  {
    if (fFTrx.outboundDateflightMap()[outboundDepartureDT]->iBDateFlightMap.count(
            inboundDepartureDT) == 0)
    {
      FlightFinderTrx::FlightDataInfo* flightInfo = nullptr;
      fFTrx.dataHandle().get(flightInfo);
      FlightFinderTrx::SopInfo* sopInfo = nullptr;
      fFTrx.dataHandle().get(sopInfo);
      sopInfo->sopIndex = sopIndex;
      sopInfo->bkgCodeDataVect.insert(
          sopInfo->bkgCodeDataVect.end(), bkgCodeDataVect.begin(), bkgCodeDataVect.end());
      sopInfo->paxTypeFareVect.insert(
          sopInfo->paxTypeFareVect.end(), paxTypeFareVect.begin(), paxTypeFareVect.end());
      flightInfo->flightList.push_back(sopInfo);
      fFTrx.outboundDateflightMap()[outboundDepartureDT]->iBDateFlightMap[inboundDepartureDT] =
          flightInfo;
    }
    else
    {
      FlightFinderTrx::SopInfo* sopInfo = nullptr;
      fFTrx.dataHandle().get(sopInfo);
      sopInfo->sopIndex = sopIndex;
      sopInfo->bkgCodeDataVect.insert(
          sopInfo->bkgCodeDataVect.end(), bkgCodeDataVect.begin(), bkgCodeDataVect.end());
      sopInfo->paxTypeFareVect.insert(
          sopInfo->paxTypeFareVect.end(), paxTypeFareVect.begin(), paxTypeFareVect.end());
      fFTrx.outboundDateflightMap()[outboundDepartureDT]
          ->iBDateFlightMap[inboundDepartureDT]
          ->flightList.push_back(sopInfo);
    }
  }
}

void
FareValidatorOrchestrator::combineDurationsFlightBitmapsForEachPaxTypeFare(
    std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo>>& combinedDurationFlightBitmap,
    PaxTypeFare* curFare,
    const FlightFinderTrx& fFTrx)
{
  if (combinedDurationFlightBitmap.empty())
  {
    VecMap<uint64_t, PaxTypeFare::FlightBitmap>::const_iterator itCombDurFlBitmap =
        curFare->durationFlightBitmap().begin();

    for (; itCombDurFlBitmap != curFare->durationFlightBitmap().end(); ++itCombDurFlBitmap)
    {
      std::vector<PaxTypeFare::FlightBit>::const_iterator itFlightBit =
          itCombDurFlBitmap->second.begin();

      for (; itFlightBit != itCombDurFlBitmap->second.end(); ++itFlightBit)
      {
        FlightFinderTrx::FlightBitInfo flightBInfo;
        flightBInfo.flightBitStatus = itFlightBit->_flightBit;

        if (itFlightBit->_flightBit == 0 || fFTrx.bffStep() == FlightFinderTrx::STEP_4)
        {
          flightBInfo.paxTypeFareVect.push_back(curFare);
        }

        combinedDurationFlightBitmap[itCombDurFlBitmap->first].push_back(flightBInfo);
      }
    }
  }
  else
  {
    std::map<uint64_t, std::vector<FlightFinderTrx::FlightBitInfo>>::iterator itCombDurFlBitmap =
        combinedDurationFlightBitmap.begin();

    for (; itCombDurFlBitmap != combinedDurationFlightBitmap.end(); ++itCombDurFlBitmap)
    {
      VecMap<uint64_t, PaxTypeFare::FlightBitmap>::const_iterator itCurDurFlBitmap =
          curFare->durationFlightBitmap().find(itCombDurFlBitmap->first);

      if (itCurDurFlBitmap != curFare->durationFlightBitmap().end() &&
          !(itCurDurFlBitmap->second.empty()))
      {
        combineFlightBitmapsForEachPaxTypeFare(itCombDurFlBitmap->second,
                                               curFare,
                                               itCurDurFlBitmap->second,
                                               &(itCurDurFlBitmap->first));
      }
    }
  }
}

void
FareValidatorOrchestrator::combineFlightBitmapsForEachPaxTypeFare(
    std::vector<FlightFinderTrx::FlightBitInfo>& combinedFlightBitmap,
    PaxTypeFare* curFare,
    const std::vector<PaxTypeFare::FlightBit>& flightBitmap,
    const uint64_t* const duration)
{
  std::vector<PaxTypeFare::FlightBit>::const_iterator itFlightBit = flightBitmap.begin();
  uint32_t flightBitNumber = 0;

  if (combinedFlightBitmap.empty()) // for first time add whole bitMap at once
  {
    for (; itFlightBit != flightBitmap.end(); ++itFlightBit, ++flightBitNumber)
    {
      FlightFinderTrx::FlightBitInfo flightBInfo;
      flightBInfo.flightBitStatus = itFlightBit->_flightBit;

      if (itFlightBit->_flightBit == 0 || itFlightBit->_flightBit == RuleConst::SKIP)
      {
        flightBInfo.paxTypeFareVect.push_back(curFare);
      }

      combinedFlightBitmap.push_back(flightBInfo);
    }
  }
  else // for next fare
  {
    for (; itFlightBit != flightBitmap.end(); ++itFlightBit, ++flightBitNumber)
    {
      if (duration == nullptr) // for flightBitmap inside curFare
      {
        if (curFare->isFlightValid(flightBitNumber) ||
            *(curFare->getFlightBit(flightBitNumber)) == RuleConst::SKIP) // change bitmap
        {
          combinedFlightBitmap[flightBitNumber].paxTypeFareVect.push_back(curFare);
          combinedFlightBitmap[flightBitNumber].flightBitStatus = 0;
        }
      }
      else // for flightBitmap inside durationFlightBitmap for proper duration
      {
        if (curFare->isFlightValid(*duration, flightBitNumber) ||
            *(curFare->getFlightBit(*duration, flightBitNumber)) ==
                RuleConst::SKIP) // change bitmap
        {
          combinedFlightBitmap[flightBitNumber].paxTypeFareVect.push_back(curFare);
          combinedFlightBitmap[flightBitNumber].flightBitStatus = 0;
        }
      }
    }
  }
}

void
FareValidatorOrchestrator::createFFinderAltDates(ShoppingTrx& trx)
{
  FlightFinderTrx& ffTrx = dynamic_cast<FlightFinderTrx&>(trx);
  std::vector<DateTime> altDepartureDates;
  std::vector<DateTime> altArrivalDates;
  const uint32_t shiftDays = 7;
  const uint32_t altDatesPeriod = 15;
  ffTrx.setAltDates(true);
  TravelSeg* firstSeg = ffTrx.journeyItin()->travelSeg().front();
  const DateTime& depDate = firstSeg->departureDT();

  if (ffTrx.legsStatus().isSet(FlightFinderTrx::LEG1_HAS_VLD_FARE))
  {
    altDepartureDates.push_back(depDate);
  }
  else
  {
    DateTime altDepDate(depDate.subtractDays(shiftDays));

    for (uint32_t dayIter = 0; dayIter < altDatesPeriod; ++dayIter)
    {
      altDepartureDates.push_back(altDepDate);
      altDepDate = altDepDate.nextDay();
    }
  }

  DateTime arvlDate(DateTime::emptyDate());
  TravelSeg* lastSeg = nullptr;

  if (ffTrx.journeyItin()->travelSeg().size() < 2)
  {
    altArrivalDates.push_back(arvlDate);
  }
  else
  {
    // In this case there are two legs
    lastSeg = ffTrx.journeyItin()->travelSeg().back();
    arvlDate = lastSeg->departureDT();

    if (ffTrx.legsStatus().isSet(FlightFinderTrx::LEG2_HAS_VLD_FARE))
    {
      altArrivalDates.push_back(arvlDate);
    }
    else
    {
      DateTime altArvlDate(arvlDate.subtractDays(shiftDays));

      for (uint32_t dayIter = 0; dayIter < altDatesPeriod; ++dayIter)
      {
        altArrivalDates.push_back(altArvlDate);
        altArvlDate = altArvlDate.nextDay();
      }
    }
  }

  std::vector<DateTime>::const_iterator depIter = altDepartureDates.begin();

  for (; depIter != altDepartureDates.end(); ++depIter)
  {
    std::vector<DateTime>::const_iterator arvlIter = altArrivalDates.begin();

    for (; arvlIter != altArrivalDates.end(); ++arvlIter)
    {
      if (((*depIter).get64BitRepDateOnly() > (*arvlIter).get64BitRepDateOnly() &&
           (*arvlIter) != DateTime::emptyDate()) ||
          (*depIter == depDate && *arvlIter == arvlDate))
      {
        continue;
      }

      DatePair datePair(*depIter, *arvlIter);
      PricingTrx::AltDateInfo* altDateInfo;
      ffTrx.dataHandle().get(altDateInfo);
      setFFinderAltDateJourneyItin(datePair, *altDateInfo, ffTrx);
      ffTrx.altDatePairs()[datePair] = altDateInfo;
    }
  }
}

void
FareValidatorOrchestrator::setFFinderAltDateJourneyItin(const DatePair& datePair,
                                                        PricingTrx::AltDateInfo& altInfo,
                                                        ShoppingTrx& trx)
{
  AirSeg* firstJrnTrvSeg = (trx.journeyItin()->travelSeg().front())->toAirSeg();
  LocCode boardCity = firstJrnTrvSeg->origin()->loc();
  LocCode offCity = firstJrnTrvSeg->destination()->loc();
  CarrierCode cxrCode = firstJrnTrvSeg->carrier();
  int16_t pnrSegment = 1;
  trx.dataHandle().get(altInfo.journeyItin);
  DatePair*& myPair = altInfo.journeyItin->datePair();
  trx.dataHandle().get(myPair);
  *myPair = datePair;
  /* Setup first segment */
  ShoppingAltDateUtil::generateJourneySegAndFM(trx.dataHandle(),
                                               *altInfo.journeyItin,
                                               datePair.first,
                                               boardCity,
                                               offCity,
                                               cxrCode,
                                               pnrSegment);

  if (trx.journeyItin()->travelSeg().size() > 1)
  {
    pnrSegment++;
    /* Setup second segment */
    ShoppingAltDateUtil::generateJourneySegAndFM(trx.dataHandle(),
                                                 *altInfo.journeyItin,
                                                 datePair.second,
                                                 offCity,
                                                 boardCity,
                                                 cxrCode,
                                                 pnrSegment);
  }

  altInfo.journeyItin->setTravelDate(TseUtil::getTravelDate(altInfo.journeyItin->travelSeg()));
  altInfo.journeyItin->bookingDate() = TseUtil::getBookingDate(altInfo.journeyItin->travelSeg());
  // Setup currency
  const Itin* itin = trx.journeyItin();
  altInfo.journeyItin->calculationCurrency() = itin->calculationCurrency();
  altInfo.journeyItin->originationCurrency() = itin->originationCurrency();
}

void
FareValidatorOrchestrator::prevalidateFares(FlightFinderTrx& ffTrx)
{
  if (ffTrx.diagnostic().isActive())
  {
    return;
  }

  uint32_t legId = 0;
  TseUtil::FFOwrtApplicabilityPred owrtPred(ffTrx);

  for (auto& leg : ffTrx.legs())
  {
    // reset flag for next leg
    ItinIndex& curCxrIdx = leg.carrierIndex();

    if (curCxrIdx.root().empty())
    {
      ++legId;
      continue;
    }

    for (auto& itinMatrix : curCxrIdx.root())
    {
      auto fM = getFirstFareMarket(ffTrx, legId, itinMatrix.first);

      if (fM == nullptr)
      {
        continue;
      }

      std::vector<PaxTypeFare*>& allPaxTypeFares = fM->allPaxTypeFare();
      std::vector<PaxTypeFare*>::iterator validFareIt =
          find_if(allPaxTypeFares.begin(), allPaxTypeFares.end(), owrtPred);

      /* Report error only when there are any fares retrieved */
      if (validFareIt == allPaxTypeFares.end() && allPaxTypeFares.size() > 0)
      {
        throw ErrorResponseException(ffTrx.reportError().errorCode,
                                     ffTrx.reportError().message.c_str());
      }
      else if (allPaxTypeFares.empty())
      {
        throw ErrorResponseException(ErrorResponseException::NO_VALID_FARE_FOUND,
                                     "NO VALID FARE FOUND");
      }
    }

    ++legId;
  }
}

//-----------------------------------------------------------------
bool
FareValidatorOrchestrator::isOutboundInboundFltValid(FlightFinderTrx& ffTrx)
{
  uint32_t legId = 0;

  for (auto& leg : ffTrx.legs())
  {
    // reset flag for next leg
    ItinIndex& curCxrIdx = leg.carrierIndex();

    if (curCxrIdx.root().empty())
    {
      ++legId;
      continue;
    }

    for (auto& curPair : curCxrIdx.root())
    {
      // Retrieve first flight ,not FAKEDDIRECTFLIGHT
      // If carrier index contains only faked flight invalidate this leg
      const ItinIndex::ItinCell* topCell =
          curCxrIdx.retrieveTopItinCell(curPair.first, ItinIndex::CHECK_FAKEDIRECTFLIGHT);

      if ((topCell != nullptr) &&
          (topCell->first.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDFLIGHT))
      {
        // Invalidate this leg since there is no valid SOP
        continue;
      }

      auto fM = getFirstFareMarket(ffTrx, legId, curPair.first);

      if (fM == nullptr)
      {
        continue;
      }

      for (auto curFare : fM->allPaxTypeFare())
      {
        if (!curFare->isValid())
        {
          continue;
        }

        if ((fM->direction() == FMDirection::OUTBOUND && curFare->directionality() == TO) ||
            (fM->direction() == FMDirection::INBOUND && curFare->directionality() == FROM))
        {
          continue;
        }

        if (!curFare->shoppingComponentValidationFailed() && !curFare->flightBitmapAllInvalid())
        {
          if (legId == 0)
            ffTrx.legsStatus().set(FlightFinderTrx::LEG1_HAS_VLD_FARE);
          else
            ffTrx.legsStatus().set(FlightFinderTrx::LEG2_HAS_VLD_FARE);

          break;
        }
      }
    }

    ++legId;
  }

  bool isOutboundInboundVld = false;

  if (legId <= 1)
    isOutboundInboundVld = ffTrx.legsStatus().isSet(FlightFinderTrx::LEG1_HAS_VLD_FARE);
  else
    isOutboundInboundVld = ffTrx.legsStatus().isSet(FlightFinderTrx::LEG1_HAS_VLD_FARE) &&
                           ffTrx.legsStatus().isSet(FlightFinderTrx::LEG2_HAS_VLD_FARE);

  return isOutboundInboundVld;
}
//---------------------------------------------------------------

bool
FareValidatorOrchestrator::process(FlightFinderTrx& trx)
{
  const TSELatencyData metrics(trx, "FVO PROCESS");
  LOG4CXX_INFO(logger, "FareValidatorOrchestrator::process(FlightFinder...");
  /* Check if there is any applicable fare to validate,if not report error */
  prevalidateFares(trx);

  if (trx.isBffReq())
  {
    processBFFTrx(trx);
  }
  else
  {
    if (!trx.isAltDates())
    {
      // STEP_01
      validateNonFltRules(trx);

      /* Flight bitmap validation for original date pair */
      if (validateFlightBitmap(trx) == false)
      {
        // Diagnostic 904 or 911 requested
        return true;
      }

      removeFaresKeepForRoutingValidation(trx);
      bool generateAltDates = true;

      if (isOutboundInboundFltValid(trx))
      {
        generateAltDates = false;
        makeFlightList(trx);
        FlightFinderJourneyValidator jv(&trx);
        jv.validate();

        if (trx.outboundDateflightMap().empty())
        {
          generateAltDates = true;
          // clear flags for correct alt dates generation
          trx.legsStatus().setNull();
        }
      }

      if (generateAltDates)
      {
        // STEP_2
        createFFinderAltDates(trx);
        // Init alt dates for in fares for each valid fare market
        initFFinderAltDates(trx);
        // Component rule validation for alt dates
        validateNonFltRulesForAltDates(trx);
        std::map<DatePair, FlightFinderTrx::FlightBitInfo> dataPairsMap;
        prepareAltDatePairs(trx, dataPairsMap);
        FlightFinderJourneyValidator jv(&trx, &dataPairsMap);
        jv.validate();
        makeAltDateFlightList(trx, dataPairsMap);

        // check if no valid data ware found
        // if(trx.outboundDateflightMap().empty()&& !trx.diagnostic().isActive())
        if (trx.outboundDateflightMap().empty() &&
            trx.diagnostic().diagnosticType() == DiagnosticNone)
        {
          throw ErrorResponseException(ErrorResponseException::NO_VALID_DATAPAIR_FOUND,
                                       "No valid datapair found");
        }
      }
    }
    else
    {
      // STEP_3
      // Component rule validation for alt dates
      validateNonFltRulesForAltDates(trx);
      // Flight bitmap validation
      bool bRet = validateFlightBitmap(trx);
      removeFaresKeepForRoutingValidation(trx);

      if (false == bRet)
      {
        // Diagnostic 904 or 911 requested
        return true;
      }

      makeAltDateFlightListWithSOP(trx);
      FlightFinderJourneyValidator jv(&trx);
      jv.validate();

      // Check if any valid data was found
      // if(trx.outboundDateflightMap().empty()&& !trx.diagnostic().isActive())
      if (trx.outboundDateflightMap().empty() &&
          trx.diagnostic().diagnosticType() == DiagnosticNone)
      {
        throw ErrorResponseException(ErrorResponseException::NO_VALID_FLIGHT_DATE_FOUND,
                                     "No valid flight/date found");
      }
    }
  }

  return true;
}

//---------------------------------------------------------------

void
FareValidatorOrchestrator::initFFinderAltDates(ShoppingTrx& trx)
{
  FlightFinderTrx& ffTrx = dynamic_cast<FlightFinderTrx&>(trx);
  uint32_t legId = 0;

  for (auto& leg : trx.legs())
  {
    ItinIndex& curCxrIdx = leg.carrierIndex();

    if (curCxrIdx.root().empty() ||
        ((legId == 0) ? (ffTrx.legsStatus().isSet(FlightFinderTrx::LEG1_HAS_VLD_FARE))
                      : (ffTrx.legsStatus().isSet(FlightFinderTrx::LEG2_HAS_VLD_FARE))))
    {
      ++legId;
      continue;
    }

    for (auto& itinMatrix : curCxrIdx.root())
    {
      // take first market because ony this one has fares
      auto fareMarket = getFirstFareMarket(trx, legId, itinMatrix.first);

      if (fareMarket == nullptr)
      {
        continue;
      }

      //   initialize alt dates for all fares
      for (auto curFare : fareMarket->allPaxTypeFare())
      {
        if (curFare == nullptr)
        {
          continue;
        }

        if ((fareMarket->direction() == FMDirection::OUTBOUND && curFare->directionality() == TO) ||
            (fareMarket->direction() == FMDirection::INBOUND && curFare->directionality() == FROM))
        {
          continue;
        }

        // Skip valid and passed fares
        if (curFare->isValid() && !curFare->shoppingComponentValidationFailed() &&
            !curFare->flightBitmapAllInvalid() && !ffTrx.legsStatus().isNull())
        {
          continue;
        }

        curFare->initAltDates(trx);
        // Clearing category status after STEP1 validation
        clearCategoryStatus(curFare);

        for (VecMap<DatePair, uint8_t>::const_iterator j = curFare->altDateStatus().begin();
             j != curFare->altDateStatus().end();
             j++)
        {
          DatePair myPair = j->first;
          uint8_t ret = 'P';
          ShoppingAltDateUtil::checkEffExpDate(trx, *curFare, ret);

          if (ret == 'P')
          {
            continue;
            // break;
          }
          else
          {
            curFare->setAltDateStatus(myPair, ret);
          }
        }
      }
    } // END_OF IMIter

    ++legId;
  }
}

bool
FareValidatorOrchestrator::validateFlightBitmap(FlightFinderTrx& trx)
{
  if (UNLIKELY(!trx.ignoreDiagReq() && trx.diagnostic().diagnosticType() == Diagnostic904 &&
               trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "ALTDATES"))
  {
    return false;
  }

  std::vector<ShoppingTrx::Leg>& sLV = trx.legs();

  if (sLV.empty())
  {
    return (false);
  }

  Itin*& journeyItin = trx.journeyItin();
  std::vector<TravelSeg*> jrnTravelSegs = journeyItin->travelSeg();
  const TSELatencyData metricsThreads(trx, "FVO BITMAP VALIDATION");
  ShpBitValidationCollector& shareCollector = trx.getBitValidationCollector();
  // Loop through the leg vector
  uint32_t legId = 0;
  std::vector<ShoppingTrx::Leg>::iterator sLVIter = sLV.begin();

  for (; sLVIter != sLV.end(); ++sLVIter)
  {
    if ((trx.bffStep() == FlightFinderTrx::STEP_4 || trx.bffStep() == FlightFinderTrx::STEP_6) &&
        legId == 0)
    {
      ++legId;
      continue;
    }

    // Get leg reference
    ShoppingTrx::Leg& curLeg = (*sLVIter);
    ItinIndex& sIG = curLeg.carrierIndex();
    // reset travel segments
    journeyItin->travelSeg() = jrnTravelSegs;
    // Need to check if could create outside this loop
    FareMarketRuleController& ruleController = trx.fareMarketRuleController();
    FareMarketRuleController& cat4ruleController = trx.cat4RuleController();
    // Function prepareJourneyItin need to work on copy of altDates and journeyItin
    ShoppingTrx::AltDatePairs altDatePairsCopy;
    std::deque<Itin> journeyItins;

    for (auto& itinMatrix : sIG.root())
    {
      // crete copy of alt dates
      ShoppingAltDateUtil::cloneAltDates(trx, altDatePairsCopy, journeyItins);
      // add the main journey itin
      journeyItins.push_back(*journeyItin);

      auto fM = getFirstFareMarket(trx, legId, itinMatrix.first);
      if (fM == nullptr)
      {
        continue;
      }
      ShpBitValidationCollector::FMValidationSharedData* sharedData(
          shareCollector.getFMSharedData(itinMatrix.first, fM));
      ShoppingTrx::FareMarketRuleMap& fmrm = trx.fareMarketRuleMap();
      processFFCarrier(trx,
                       itinMatrix.first,
                       legId,
                       curLeg,
                       &journeyItins.back(),
                       altDatePairsCopy,
                       ruleController,
                       cat4ruleController,
                       fM,
                       fmrm[fM]._shoppingBCETuningData,
                       sharedData);
    } // END_OF_iMIter

    // Increment leg id
    ++legId;
  }

  // restore the journey itin's travel segs to their original value
  journeyItin->travelSeg() = jrnTravelSegs;

  // Check if diagnostic 911 requested
  return !(UNLIKELY(!trx.ignoreDiagReq() && trx.diagnostic().diagnosticType() == Diagnostic911));
}
//----------------------------------------------------

bool
FareValidatorOrchestrator::invalidForValidatingCxr(FlightFinderTrx& trx,
                                                   PaxTypeFare* ptf,
                                                   Itin* itin)
{
  std::vector<CarrierCode> participatingCarriers;
  ValidatingCxrUtil::getParticipatingCxrs(trx, *itin, participatingCarriers);

  for (CarrierCode cxrCode1 : participatingCarriers)
  {
    for (CarrierCode cxrCode2 : ptf->validatingCarriers())
    {
      if (cxrCode1 == cxrCode2)
        return false;
    }
  }

  return true;
}

void
FareValidatorOrchestrator::processFFCarrier(
    FlightFinderTrx& trx,
    const ItinIndex::Key& cxrKey,
    uint32_t legId,
    ShoppingTrx::Leg& curLeg,
    Itin* journeyItin,
    ShoppingTrx::AltDatePairs& altDatesMap,
    FareMarketRuleController& ruleController,
    FareMarketRuleController& cat4ruleController,
    FareMarket* fM,
    std::vector<std::vector<BCETuning>>& shoppingBCETuningData, // DEPRECATED
    ShpBitValidationCollector::FMValidationSharedData* sharedData)
{
  const TSELatencyData Metrics(trx, "FVO PROCESS CARRIER");
  ItinIndex& sIG = curLeg.carrierIndex();
  uint32_t beginLeg = legId;
  uint32_t endLeg = legId + 1;
  prepareJourneyItin(journeyItin, &altDatesMap, beginLeg, endLeg);
  // Limitations check
  checkLimitationsFF(trx, cxrKey, legId, curLeg, journeyItin, altDatesMap, fM);

  //-----------------------------------------------------------------------------
  // Real bitmap validation
  //------------------------------------------------------------------------------

  // Diag 911 DD=BEFOREFLTVALIDATION
  if (UNLIKELY(!trx.ignoreDiagReq() && !trx.isAltDates() &&
               trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "BEFFLTVLD"))
  {
    return;
  }

  const TSELatencyData metrics(trx, "FVO FARE BITMAP VALIDATE");

  // Create map of duration date pairs for which we need to find valid flight
  bool skipFoundDates = (trx.avlInS1S3Request() && trx.isAltDates());
  PricingTrx::DurationAltDatePairs datesToFind;

  if (skipFoundDates)
  {
    datesToFind.insert(trx.durationAltDatePairs().begin(), trx.durationAltDatePairs().end());
  }

  ItinIndex::ItinIndexIterator iRCIter = sIG.beginRow(cxrKey);
  ItinIndex::ItinIndexIterator iRCEIter = sIG.endRow();

  for (; iRCIter != iRCEIter; ++iRCIter)
  {
    ItinIndex::ItinCell& cell = *iRCIter;
    ItinIndex::ItinCellInfo& curItinCellInfo = cell.first;
    Itin* curItinCellItin = cell.second;
    const int bitIndex = iRCIter.bitIndex();
    // Routing map for current flight bit
    ShoppingRtgMap rtMap;
    //----setup PTF iterators
    std::vector<PaxTypeFare*> tempAllPaxTypeFare(fM->allPaxTypeFare().size());
    // Iterate through all of the fares
    std::vector<PaxTypeFare*>::iterator pTFIter = fM->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator pTFEndIter = fM->allPaxTypeFare().end();

    // Setup iterators
    if (find_if(pTFIter, pTFEndIter, isSpecialRout<PaxTypeFare*>()) == fM->allPaxTypeFare().end())
    {
      pTFIter = fM->allPaxTypeFare().begin();
      pTFEndIter = fM->allPaxTypeFare().end();
    }
    else
    {
      std::copy(
          fM->allPaxTypeFare().begin(), fM->allPaxTypeFare().end(), tempAllPaxTypeFare.begin());
      std::stable_partition(
          tempAllPaxTypeFare.begin(), tempAllPaxTypeFare.end(), isNotSpecialRout<PaxTypeFare*>());
      pTFIter = tempAllPaxTypeFare.begin();
      pTFEndIter = tempAllPaxTypeFare.end();
    }

    bool isFlightBitPassed = false;
    std::map<uint64_t, uint8_t> prevFareDurationBitStatusMap;
    std::map<uint64_t, uint8_t>::iterator durBitIter;
    ShoppingTrx::SchedulingOption& sop = curLeg.sop()[curItinCellInfo.sopIndex()];
    bool dummySop = sop.getDummy();

    if (!dummySop)
    {
      ShoppingUtil::prepareFFClassOfService(sop);
    }

    for (; pTFIter != pTFEndIter; ++pTFIter)
    {
      if (fallback::reworkTrxAborter(&trx))
        checkTrxAborted(trx);
      else
        trx.checkTrxAborted();
      PaxTypeFare* curFare = *pTFIter;

      if (dummySop)
      {
        curFare->setFlightInvalid(bitIndex, RuleConst::DUMMY_SOP);
        continue;
      }

      if (trx.isValidatingCxrGsaApplicable())
      {
        if (invalidForValidatingCxr(trx, curFare, curItinCellItin))
        {
          curFare->setFlightInvalid(bitIndex, RuleConst::GSA_FAIL);
          continue;
        }
      }

      // bitmap was not changed since creation, should be set to "0"
      if (curFare->isFlightValid(bitIndex) == false)
      {
        continue;
      }

      DateTime departureDT(curItinCellItin->travelSeg().front()->departureDT());
      DatePair datePair;

      if (trx.isAltDates())
      {
        // Update duration bitmap for current flight (bitIndex)after component validation(date pair
        // status)
        // If date pair valid will set bit to SKIP
        setFBMStatusForAllDuration(bitIndex, departureDT, legId, curFare, trx);

        if (curFare->durationFlightBitmap().empty() ||
            (!trx.ignoreDiagReq() &&
             trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "BEFFLTVLD"))
        {
          continue;
        }

        VecMap<uint64_t, PaxTypeFare::FlightBitmap>::iterator
        durIter = curFare->durationFlightBitmap().begin(),
        durEndIter = curFare->durationFlightBitmap().end();

        // ptf->flightBitmap()will be used by all durations validation
        // need to clean up this bitmap after validation

        for (; durIter != durEndIter; durIter++)
        {
          // if element does not exsist, create new one
          durBitIter = prevFareDurationBitStatusMap.find(durIter->first);

          if (durBitIter == prevFareDurationBitStatusMap.end())
          {
            prevFareDurationBitStatusMap[durIter->first] = RuleConst::SKIP;
          }

          // move to next duration when bit valid for current dur in previous ptf
          if (prevFareDurationBitStatusMap[durIter->first] == 0)
          {
            continue;
          }

          // Check if duration exists
          ShoppingTrx::AltDatePairs& durAltDatePairs =
              (trx.durationAltDatePairs().find(durIter->first))->second;
          PaxTypeFare::FlightBitmap& flightBitmap = durIter->second;

          // Try to match flight to date pair from current duration
          if (!ShoppingAltDateUtil::getDatePair(durAltDatePairs, departureDT, legId, datePair))
          {
            continue; // move to next duration
          }

          // Check if for processing duration date pair we've
          // already found valid flight, if so skip processing.
          PricingTrx::DurationAltDatePairs::iterator foundDatesIter;

          if (skipFoundDates)
          {
            foundDatesIter = datesToFind.find(durIter->first);

            if (foundDatesIter == datesToFind.end())
            {
              continue;
            }
            else
            {
              if ((foundDatesIter->second).end() == (foundDatesIter->second).find(datePair))
              {
                continue;
              }
            }
          }

          // if datePair is not valid after component validation
          // don't validate flight for this datePair
          if (!curFare->getAltDatePass(datePair))
          {
            continue; // move to next duration
          }

          // Retrieve journey Itin for date pair with matched flight
          journeyItin = ShoppingAltDateUtil::getJourneyItin(altDatesMap, datePair);

          if (!journeyItin)
          {
            curFare->setFlightInvalid(flightBitmap, bitIndex, RuleConst::NO_DATEPAIR_FOUND);
            continue; // move to next duration
          }

          const TSELatencyData metrics(trx, "FVO FARE BIT VALIDATE");

          ShoppingRtgMap* rtgMap(nullptr);
          std::vector<BCETuning>* bceTunningData(nullptr);

          TSE_ASSERT(sharedData);
          sharedData->updateFareMarketData(fM, bitIndex);
          rtgMap = sharedData->getRoutingMapForBit(bitIndex);
          bceTunningData = sharedData->getBCEData(bitIndex);

          FareValidatorOrchestrator::FMBackup fmb;
          prepareShoppingValidation(trx, journeyItin, cell, fM, fmb, beginLeg, endLeg);
          // will validate current flight and set bit in main bitmap
          CabinType cab;
          cab.setEconomyClass();
          _bitmapOpOrderer.performBitmapOperations(trx,
                                                   *curFare,
                                                   *fM,
                                                   bitIndex,
                                                   *journeyItin,
                                                   curItinCellInfo,
                                                   (curLeg.preferredCabinClass() < cab),
                                                   ruleController,
                                                   cat4ruleController,
                                                   *rtgMap,
                                                   *bceTunningData);
          cleanupAfterShoppingValidation(trx, journeyItin, fM, fmb, beginLeg, endLeg);
          updateDurationFlightBitmapAfterValidation(curFare, flightBitmap, bitIndex);
          // set current flight bit status for next fare processing
          prevFareDurationBitStatusMap[durIter->first] = *(curFare->getFlightBit(bitIndex));

          if ((true == skipFoundDates) && (0 == *(curFare->getFlightBit(bitIndex))))
          {
            // If flight bit is valid keep original out date flight
            // map and create new for journey validation
            FlightFinderTrx::OutBoundDateFlightMap tempMap;
            tempMap.swap(trx.outboundDateflightMap());
            prepareMapToJourneyValidation(
                trx, datePair, legId, cell.first.sopIndex(), curFare, bitIndex, durIter->first);
            // Perform journey validation
            FlightFinderJourneyValidator ffjv(&trx);
            ffjv.validate();

            // If result map is not empty after journey validation
            // it means that we found valid flight
            if (!trx.outboundDateflightMap().empty())
            {
              (foundDatesIter->second).erase(datePair);

              if ((foundDatesIter->second).empty())
              {
                datesToFind.erase(foundDatesIter);
              }

              // Add found date to out date flight
              mergeDateFlightMaps(tempMap, trx.outboundDateflightMap(), false);
            }

            // Restore original out date flight map
            trx.outboundDateflightMap().swap(tempMap);
          }

          // reset bit in main bitmap for next validation (next duration)
          // main duration bitmap is working bitmap for all alt dates durations
          curFare->setFlightInvalid(bitIndex, 0);

          //__TODO__ optimalization
          if (!curFare->altDateFltBitStatus().empty())
          {
            // need investigate
            ShoppingAltDateUtil::setAltDateFltBit(curFare, bitIndex);
          }

          sharedData->collectFareMarketData(fM, bitIndex);

          // Break the loop if there are no duration date pairs
          // to find
          if (skipFoundDates && datesToFind.empty())
          {
            break;
          }
        }

        // Break the loop if there are no duration date pairs
        // to find
        if (skipFoundDates && datesToFind.empty())
        {
          break;
        }
      }
      else
      {
        // for single pair
        if (isFlightBitPassed)
        {
          curFare->setFlightInvalid(bitIndex, RuleConst::SKIP);
          continue;
        }

        ShoppingRtgMap* rtgMap(nullptr);
        std::vector<BCETuning>* bceTunningData(nullptr);

        TSE_ASSERT(sharedData);
        sharedData->updateFareMarketData(fM, bitIndex);
        rtgMap = sharedData->getRoutingMapForBit(bitIndex);
        bceTunningData = sharedData->getBCEData(bitIndex);

        FareValidatorOrchestrator::FMBackup fmb;
        prepareShoppingValidation(trx, journeyItin, cell, fM, fmb, beginLeg, endLeg);
        CabinType cab;
        cab.setEconomyClass();
        _bitmapOpOrderer.performBitmapOperations(trx,
                                                 *curFare,
                                                 *fM,
                                                 bitIndex,
                                                 *journeyItin,
                                                 curItinCellInfo,
                                                 (curLeg.preferredCabinClass() < cab),
                                                 ruleController,
                                                 cat4ruleController,
                                                 *rtgMap,
                                                 *bceTunningData);
        cleanupAfterShoppingValidation(trx, journeyItin, fM, fmb, beginLeg, endLeg);

        sharedData->collectFareMarketData(fM, bitIndex);

        if (curFare->isFlightValid(bitIndex))
        {
          isFlightBitPassed = true;
        }
      }

      // Break the loop if there are no duration date pairs
      // to find
      if (skipFoundDates && datesToFind.empty())
      {
        break;
      }
    }

    // Break the loop if there are no duration date pairs
    // to find
    if (skipFoundDates && datesToFind.empty())
    {
      break;
    }
  }

  LOG4CXX_DEBUG(logger, "- setting fare status");
  //__TODO__ for alt dates would be different
  setFareStatus(trx, fM->allPaxTypeFare());
}

void
FareValidatorOrchestrator::checkLimitationsFF(FlightFinderTrx& trx,
                                              const ItinIndex::Key& cxrKey,
                                              uint32_t legId,
                                              ShoppingTrx::Leg& curLeg,
                                              Itin* journeyItin,
                                              ShoppingTrx::AltDatePairs& altDatesMap,
                                              FareMarket* fareMarket)
{
  TSELatencyData Metrics(trx, "FVO CHECK LIMITATIONS FF");
  uint32_t beginLeg = legId;
  uint32_t endLeg = legId + 1;
  ItinIndex::ItinIndexIterator iRCIter = curLeg.carrierIndex().beginRow(cxrKey);
  ItinIndex::ItinIndexIterator iRCEIter = curLeg.carrierIndex().endRow();

  for (; iRCIter != iRCEIter; ++iRCIter)
  {
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    ItinIndex::ItinCell& curItinCell = *iRCIter;
    // Get cell itin and itin information
    Itin*& curItinCellItin = curItinCell.second;

    if (!curItinCellItin)
    {
      continue;
    }

    DateTime departureDT(curItinCellItin->travelSeg().front()->departureDT());
    DatePair datePair;

    if (trx.isAltDates())
    {
      if (!ShoppingAltDateUtil::getDatePair(altDatesMap, departureDT, legId, datePair))
      {
        continue;
      }

      journeyItin = ShoppingAltDateUtil::getJourneyItin(altDatesMap, datePair);

      if (!journeyItin)
      {
        continue;
      }
    }

    FareValidatorOrchestrator::FMBackup fmb;
    prepareShoppingValidation(trx, journeyItin, curItinCell, fareMarket, fmb, beginLeg, endLeg);
    performLimitationValidations(trx, journeyItin, fareMarket);
    cleanupAfterShoppingValidation(trx, journeyItin, fareMarket, fmb, beginLeg, endLeg);
  }
}

void
FareValidatorOrchestrator::prepareMapToJourneyValidation(FlightFinderTrx& trx,
                                                         DatePair& datePair,
                                                         uint32_t legId,
                                                         uint32_t sopId,
                                                         PaxTypeFare* paxTypeFare,
                                                         uint32_t bitmapIndex,
                                                         uint64_t duration)
{
  TSELatencyData Metrics(trx, "FVO PREPARE MAP TO JOURNEY VALIDATION");
  trx.outboundDateflightMap().clear();
  FlightFinderTrx::OutBoundDateInfo* outBoundDateInfo = nullptr;

  if (legId != 0)
  {
    // Process round trip (step 3/4)
    outBoundDateInfo = prepareOutMapToJourneyValidation(trx);
    FlightFinderTrx::FlightDataInfo* flightDataInfo = nullptr;
    trx.dataHandle().get(flightDataInfo);
    FlightFinderTrx::SopInfo* sopInfo = nullptr;
    trx.dataHandle().get(sopInfo);
    sopInfo->sopIndex = sopId;
    sopInfo->paxTypeFareVect.push_back(paxTypeFare);
    getBookingCodeVectVect(sopInfo->bkgCodeDataVect,
                           trx.legs()[legId].sop()[sopId].thrufareClassOfService(),
                           bitmapIndex,
                           duration,
                           trx.legs()[legId].sop()[sopId].itin()->travelSeg(),
                           sopInfo->paxTypeFareVect);
    flightDataInfo->flightList.push_back(sopInfo);
    outBoundDateInfo->iBDateFlightMap[datePair.second] = flightDataInfo;
  }
  else
  {
    // Process one way step (1/2)
    trx.dataHandle().get(outBoundDateInfo);
    FlightFinderTrx::SopInfo* sopInfo = nullptr;
    trx.dataHandle().get(sopInfo);
    sopInfo->sopIndex = sopId;
    sopInfo->paxTypeFareVect.push_back(paxTypeFare);
    getBookingCodeVectVect(sopInfo->bkgCodeDataVect,
                           trx.legs()[legId].sop()[sopId].thrufareClassOfService(),
                           bitmapIndex,
                           duration,
                           trx.legs()[legId].sop()[sopId].itin()->travelSeg(),
                           sopInfo->paxTypeFareVect);
    outBoundDateInfo->flightInfo.flightList.push_back(sopInfo);
  }

  trx.outboundDateflightMap()[datePair.first] = outBoundDateInfo;
}

FlightFinderTrx::OutBoundDateInfo*
FareValidatorOrchestrator::prepareOutMapToJourneyValidation(FlightFinderTrx& trx)
{
  TSELatencyData Metrics(trx, "FVO PREPARE OUT MAP TO JOURNEY VALIDATION");
  FlightFinderTrx::OutBoundDateInfo* outBoundDateInfoPtr = nullptr;
  trx.dataHandle().get(outBoundDateInfoPtr);
  uint32_t legId = 0;
  ItinIndex& curCxrIdx = trx.legs()[0].carrierIndex();

  for (auto& itinMatrix : curCxrIdx.root())
  {
    FlightFinderTrx::SopInfo* sopInfo = nullptr;
    trx.dataHandle().get(sopInfo);
    ItinIndex::ItinCell* itinCell =
        ShoppingUtil::retrieveDirectItin(trx, legId, itinMatrix.first, ItinIndex::CHECK_NOTHING);

    if (!itinCell)
    {
      continue;
    }

    Itin* curItinCellItin = itinCell->second;

    if (curItinCellItin == nullptr)
    {
      continue;
    }

    sopInfo->sopIndex = itinCell->first.sopIndex();
    FareMarket* fM = curItinCellItin->fareMarket().front();

    if (fM == nullptr)
    {
      continue;
    }

    std::copy_if(fM->allPaxTypeFare().begin(),
                 fM->allPaxTypeFare().end(),
                 std::back_inserter(sopInfo->paxTypeFareVect),
                 [](const auto paxTypeFare)
                 { return paxTypeFare && paxTypeFare->isValid(); });

    outBoundDateInfoPtr->flightInfo.flightList.push_back(sopInfo);
  }

  return outBoundDateInfoPtr;
}

void
FareValidatorOrchestrator::clearCategoryStatus(PaxTypeFare* paxTypeFare)
{
  paxTypeFare->setCategoryValid(RuleConst::DAY_TIME_RULE);
  paxTypeFare->setCategoryValid(RuleConst::SEASONAL_RULE);
  paxTypeFare->setCategoryValid(RuleConst::ADVANCE_RESERVATION_RULE);
  paxTypeFare->setCategoryValid(RuleConst::MINIMUM_STAY_RULE);
  paxTypeFare->setCategoryValid(RuleConst::MAXIMUM_STAY_RULE);
  paxTypeFare->setCategoryValid(RuleConst::BLACKOUTS_RULE);
  paxTypeFare->setCategoryValid(RuleConst::TRAVEL_RESTRICTIONS_RULE);
  paxTypeFare->setCategoryValid(RuleConst::SALE_RESTRICTIONS_RULE);
  paxTypeFare->setCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE);
}

void
FareValidatorOrchestrator::updateDurationFlightBitmapAfterValidation(
    PaxTypeFare* curFare, PaxTypeFare::FlightBitmap& flightBitmap, const int bitIndex)
{
  curFare->setFlightInvalid(flightBitmap, bitIndex, *(curFare->getFlightBit(bitIndex)));
  flightBitmap[bitIndex]._segmentStatus.swap(curFare->flightBitmap()[bitIndex]._segmentStatus);
}
namespace
{
uint16_t
getNumSeats(const std::vector<ClassOfService*>& thrufareClassOfService, const BookingCode& bkgCode)
{
  for (auto classOfService : thrufareClassOfService)
  {
    if (classOfService->bookingCode() == bkgCode)
      return classOfService->numSeats();
  }

  return 0;
}

void
getBookingCodeVect(std::vector<std::vector<FlightFinderTrx::BookingCodeData>>& bkgCodeDataVect,
                   const std::vector<std::vector<ClassOfService*>*>& thrufareClassOfService,
                   const std::vector<PaxTypeFare::SegmentStatus>& segStatus,
                   PaxTypeFare* paxTypeFare,
                   const std::vector<TravelSeg*>& travelSegs)
{
  std::vector<FlightFinderTrx::BookingCodeData> bkgCodeVect;

  if (segStatus.size() == travelSegs.size())
  {
    int16_t tvlItem = 0;
    std::vector<PaxTypeFare::SegmentStatus>::const_iterator segItem = segStatus.begin();

    for (; segItem != segStatus.end(); ++segItem, ++tvlItem)
    {
      FlightFinderTrx::BookingCodeData bkgDodeData;

      if (segItem->_bkgCodeReBook.empty())
      {
        if (travelSegs[tvlItem]->getBookingCode().empty())
        {
          bkgDodeData.bkgCode = BookingCode(paxTypeFare->fareClass()[0]);
        }
        else
        {
          bkgDodeData.bkgCode = travelSegs[tvlItem]->getBookingCode();
        }
      }
      else
      {
        bkgDodeData.bkgCode = segItem->_bkgCodeReBook;
      }

      bkgDodeData.numSeats = getNumSeats(*(thrufareClassOfService[tvlItem]), bkgDodeData.bkgCode);
      bkgCodeVect.push_back(bkgDodeData);
    }
  }

  bkgCodeDataVect.push_back(bkgCodeVect);
}
}

// if bookingcode rebook for segmentStatus is blank, get it from the travelSeg
void
FareValidatorOrchestrator::getBookingCodeVectVect(
    std::vector<std::vector<FlightFinderTrx::BookingCodeData>>& bkgCodeDataVect,
    const std::vector<std::vector<ClassOfService*>*>& thrufareClassOfService,
    const uint32_t bitmapIndex,
    uint64_t& duration,
    const std::vector<TravelSeg*>& travelSegs,
    const std::vector<PaxTypeFare*> paxTypeFareVect)
{
  for (auto paxTypeFare : paxTypeFareVect)
  {
    const std::vector<PaxTypeFare::SegmentStatus>& segStatus =
        paxTypeFare->durationFlightBitmap()[duration][bitmapIndex]._segmentStatus;
    getBookingCodeVect(bkgCodeDataVect, thrufareClassOfService, segStatus, paxTypeFare, travelSegs);
  }
}

// if bookingcode rebook for segmentStatus is blank, get it from the travelSeg
void
FareValidatorOrchestrator::getBookingCodeVectVect(
    std::vector<std::vector<FlightFinderTrx::BookingCodeData>>& bkgCodeDataVect,
    const std::vector<std::vector<ClassOfService*>*>& thrufareClassOfService,
    const uint32_t bitmapIndex,
    const std::vector<TravelSeg*>& travelSegs,
    const std::vector<PaxTypeFare*> paxTypeFareVect)
{
  for (auto paxTypeFare : paxTypeFareVect)
  {
    const std::vector<PaxTypeFare::SegmentStatus>& segStatus =
        paxTypeFare->flightBitmap()[bitmapIndex]._segmentStatus;
    getBookingCodeVect(bkgCodeDataVect, thrufareClassOfService, segStatus, paxTypeFare, travelSegs);
  }
}


void
FareValidatorOrchestrator::processBFFTrx(FlightFinderTrx& trx)
{
  // Process Promotional shopping path
  if (trx.avlInS1S3Request())
  {
    processPromotionalShopping(trx);
    return;
  }

  if (trx.bffStep() == FlightFinderTrx::STEP_1 || trx.bffStep() == FlightFinderTrx::STEP_3)
  {
    validateNonFltRulesForAltDates(trx);
    std::map<DatePair, FlightFinderTrx::FlightBitInfo> dataPairsMap;
    prepareAltDatePairs(trx, dataPairsMap);
    FlightFinderJourneyValidator jv(&trx, &dataPairsMap);
    jv.validate();
    makeAltDateFlightList(trx, dataPairsMap);

    // check if no valid data ware found
    if (trx.outboundDateflightMap().empty() && trx.diagnostic().diagnosticType() == DiagnosticNone)
    {
      throw ErrorResponseException(ErrorResponseException::NO_VALID_DATAPAIR_FOUND,
                                   "No valid datapair found");
    }
  }
  else if (trx.bffStep() == FlightFinderTrx::STEP_2 || trx.bffStep() == FlightFinderTrx::STEP_4)
  {
    // Component rule validation
    validateNonFltRulesForAltDates(trx);

    // Flight bitmap validation
    if (validateFlightBitmap(trx) == false)
    {
      // Diagnostic 904 or 911 requested
      return;
    }

    removeFaresKeepForRoutingValidation(trx);
    makeAltDateFlightListWithSOP(trx);
    FlightFinderJourneyValidator jv(&trx);
    jv.validate();

    // Check if any valid data was found
    if (trx.outboundDateflightMap().empty() && trx.diagnostic().diagnosticType() == DiagnosticNone)
    {
      throw ErrorResponseException(ErrorResponseException::NO_VALID_FLIGHT_DATE_FOUND,
                                   "No valid flight/date found");
    }
  }
  else if (trx.bffStep() == FlightFinderTrx::STEP_5 || trx.bffStep() == FlightFinderTrx::STEP_6)
  {
    // Component rule validation
    validateNonFltRules(trx);

    // Flight bitmap validation
    if (validateFlightBitmap(trx) == false)
    {
      // Diagnostic 904 or 911 requested
      return;
    }

    removeFaresKeepForRoutingValidation(trx);
    makeFlightList(trx);
    FlightFinderJourneyValidator jv(&trx);
    jv.validate();

    // Check if any valid data was found
    if (trx.outboundDateflightMap().empty() && trx.diagnostic().diagnosticType() == DiagnosticNone)
    {
      throw ErrorResponseException(ErrorResponseException::NO_VALID_FLIGHT_DATE_FOUND,
                                   "No valid flight/date found");
    }
  }
}

void
FareValidatorOrchestrator::processPromotionalShopping(FlightFinderTrx& trx)
{
  // First execute step 1/3 in order to build applicability map.
  // Save results in temporary outbound date flight map.
  if (FlightFinderTrx::STEP_2 == trx.bffStep())
  {
    trx.bffStep() = FlightFinderTrx::STEP_1;
  }

  if (FlightFinderTrx::STEP_4 == trx.bffStep())
  {
    trx.bffStep() = FlightFinderTrx::STEP_3;
  }

  FlightFinderTrx::OutBoundDateFlightMap tempOutBoundDateFlightMap;
  validateNonFltRulesForAltDates(trx);
  std::map<DatePair, FlightFinderTrx::FlightBitInfo> dataPairsMap;
  prepareAltDatePairs(trx, dataPairsMap);
  FlightFinderJourneyValidator jv(&trx, &dataPairsMap);
  jv.validate();
  makeAltDateFlightList(trx, dataPairsMap);

  // Check if no valid data ware found
  if (trx.outboundDateflightMap().empty() && trx.diagnostic().diagnosticType() == DiagnosticNone)
  {
    throw ErrorResponseException(ErrorResponseException::NO_VALID_DATAPAIR_FOUND,
                                 "No valid datapair found");
  }

  tempOutBoundDateFlightMap.swap(trx.outboundDateflightMap());
  // After processing step 1/3 (applicability) reset results keep in
  // outbound date flight map and try to find applicability and availability
  // by processing step 2/4
  trx.outboundDateflightMap().clear();

  if (FlightFinderTrx::STEP_1 == trx.bffStep())
  {
    trx.bffStep() = FlightFinderTrx::STEP_2;
  }

  if (FlightFinderTrx::STEP_3 == trx.bffStep())
  {
    trx.bffStep() = FlightFinderTrx::STEP_4;
  }

  // Flight bitmap validation
  if (validateFlightBitmap(trx) == false)
  {
    // Diagnostic 904 or 911 requested
    return;
  }

  removeFaresKeepForRoutingValidation(trx);

  // We need to keep original process for single date
  if (!trx.isAltDates())
  {
    makeAltDateFlightListWithSOP(trx);
    FlightFinderJourneyValidator ffjv(&trx);
    ffjv.validate();
  }

  // Finally merge results from step 1/3 with results from step 2/4
  mergeDateFlightMaps(trx.outboundDateflightMap(), tempOutBoundDateFlightMap);
}

void
FareValidatorOrchestrator::mergeDateFlightMaps(FlightFinderTrx::OutBoundDateFlightMap& mapOut,
                                               FlightFinderTrx::OutBoundDateFlightMap& map,
                                               bool changeFlag)
{
  FlightFinderTrx::OutBoundDateFlightMap::iterator outboundIterOut;

  for (auto& outboundInfo : map)
  {
    outboundIterOut = mapOut.find(outboundInfo.first);

    if (mapOut.end() != outboundIterOut)
    {
      FlightFinderTrx::OutBoundDateInfo* outDateInfo = outboundInfo.second;
      FlightFinderTrx::OutBoundDateInfo* outDateInfoOut = outboundIterOut->second;

      for (auto& inboundInfo : outDateInfo->iBDateFlightMap)
      {
        if (outDateInfoOut->iBDateFlightMap.end() ==
            outDateInfoOut->iBDateFlightMap.find(inboundInfo.first))
        {
          if (changeFlag)
            (inboundInfo.second)->onlyApplicabilityFound = true;

          outDateInfoOut->iBDateFlightMap[inboundInfo.first] = inboundInfo.second;
        }
      }
    }
    else
    {
      if (changeFlag)
        (outboundInfo.second)->flightInfo.onlyApplicabilityFound = true;

      FlightFinderTrx::OutBoundDateInfo* outDateInfo = outboundInfo.second;

      for (auto& inbound : outDateInfo->iBDateFlightMap)
      {
        if (changeFlag)
          (inbound.second)->onlyApplicabilityFound = true;
      }

      mapOut[outboundInfo.first] = outboundInfo.second;
    }
  }
}

//----------------------------------------------------------------------------
// Limitations check
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::limitationValidation(ShoppingTrx& trx,
                                                const ItinIndex::Key& cxrKey,
                                                uint32_t legId,
                                                ItinIndex& sIG,
                                                Itin* journeyItin,
                                                FareMarket* fM,
                                                ShoppingTrx::AltDatePairs& altDatesMap,
                                                uint32_t beginLeg,
                                                uint32_t endLeg,
                                                bool isStopOverLeg)
{
  ItinIndex::ItinIndexIterator iRCIter =
      (isStopOverLeg) ? sIG.beginAcrossStopOverRow(trx, legId, cxrKey) : sIG.beginRow(cxrKey);
  ItinIndex::ItinIndexIterator iRCEIter =
      (isStopOverLeg) ? sIG.endAcrossStopOverRow() : sIG.endRow();

  for (; iRCIter != iRCEIter; ++iRCIter)
  {
    TSELatencyData iterMetrics(trx, "FVO CHECK LIMITATIONS");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    ItinIndex::ItinCell& curItinCell = *iRCIter;
    // Get cell itin and itin information
    Itin*& curItinCellItin = curItinCell.second;

    if (UNLIKELY(!curItinCellItin))
    {
      continue;
    }

    DatePair datePair;

    if (UNLIKELY(trx.isAltDates()))
    {
      ShoppingTrx::DurationAltDatePairs::iterator findIt(
          trx.durationAltDatePairs().find(trx.mainDuration()));
      TSE_ASSERT(findIt != trx.durationAltDatePairs().end());
      ShoppingTrx::AltDatePairs& altDatePairs = findIt->second;

      if (!ShoppingAltDateUtil::getDatePair(
              altDatePairs, curItinCellItin->travelSeg().front()->departureDT(), legId, datePair))
      {
        continue;
      }

      journeyItin = ShoppingAltDateUtil::getJourneyItin(altDatesMap, datePair);

      if (!journeyItin)
      {
        continue;
      }
    } // if (trx.altDates())

    FareValidatorOrchestrator::FMBackup fmb;
    prepareShoppingValidation(trx, journeyItin, curItinCell, fM, fmb, beginLeg, endLeg);
    // perform Limitation check
    performLimitationValidations(trx, journeyItin, fM);
    cleanupAfterShoppingValidation(trx, journeyItin, fM, fmb, beginLeg, endLeg);
  }
}

void
FareValidatorOrchestrator::processCarrier(
    ShoppingTrx& trx,
    const ItinIndex::Key& cxrKey,
    uint32_t legId,
    ShoppingTrx::Leg& curLeg,
    Itin* journeyItin,
    ShoppingTrx::AltDatePairs& altDatesMap,
    FareMarketRuleController& ruleController,
    FareMarketRuleController& cat4ruleController,
    FareMarket* fM,
    std::vector<std::vector<BCETuning>>& shoppingBCETuningData, // DEPRECATED
    ShpBitValidationCollector::FMValidationSharedData* sharedData,
    const uint32_t numberOfFaresToProcess)
{
  const TSELatencyData Metrics(trx, "FVO PROCESS CARRIER");
  TSELatencyData iterOutMetrx(trx, "FVO ITERATE CREATE");
  ItinIndex& sIG = curLeg.carrierIndex();

  bool isStopOverLeg = curLeg.stopOverLegFlag();

  if (trx.isIataFareSelectionApplicable() && !isStopOverLeg)
  {
    fM->setComponentValidationForCarrier(cxrKey);
  }

  uint32_t beginLeg = (isStopOverLeg) ? curLeg.jumpedLegIndices().front() : legId;
  uint32_t endLeg = (isStopOverLeg) ? curLeg.jumpedLegIndices().back() + 1 : legId + 1;
  prepareJourneyItin(journeyItin, &altDatesMap, beginLeg, endLeg);
  limitationValidation(
      trx, cxrKey, legId, sIG, journeyItin, fM, altDatesMap, beginLeg, endLeg, isStopOverLeg);
  fM->setFoundPTF(true);
  processCarrierFares(trx,
                      cxrKey,
                      legId,
                      curLeg,
                      journeyItin,
                      altDatesMap,
                      ruleController,
                      cat4ruleController,
                      fM,
                      sharedData,
                      nullptr,
                      false,
                      numberOfFaresToProcess);
  LOG4CXX_DEBUG(logger, "- setting fare status");
  int lastFareProcessed = -1;

  if (trx.isAltDates() && numberOfFaresToProcess < DELAY_VALIDATION_OFF)
  {
    lastFareProcessed = fM->lastFareProcessed();
  }

  setFareStatus(trx, fM->allPaxTypeFare(), lastFareProcessed);
}
namespace
{
size_t
distanceItinIterators(ItinIndex::ItinIndexIterator begin, const ItinIndex::ItinIndexIterator& end)
{
  size_t itinDistance = 0;

  for (; begin != end; ++begin)
  {
    ++itinDistance;
  }

  return itinDistance;
}
}

size_t
FareValidatorOrchestrator::computeIterationsLimit(
    const std::vector<PaxTypeFare*>::iterator& faresBegin,
    const std::vector<PaxTypeFare*>::iterator& faresEnd,
    bool isStopOverLeg,
    tse::ItinIndex& sIG,
    tse::ShoppingTrx& trx,
    uint32_t legId,
    int cxrKey)
{
  if (!isStopOverLeg)
    return 0;

  ItinIndex::ItinIndexIterator iRCIter = sIG.beginAcrossStopOverRow(trx, legId, cxrKey);
  ItinIndex::ItinIndexIterator iRCEIter = sIG.endAcrossStopOverRow();
  size_t itinDistance = distanceItinIterators(iRCIter, iRCEIter);
  size_t totalIterations = std::distance(faresBegin, faresEnd) * itinDistance;

  if (totalIterations == 0)
  {
    LOG4CXX_DEBUG(logger,
                  "compute iterations limit disabled - invalid input, fares times sops yields 0");
    return 0;
  }

  const uint32_t limit = carrierFaresLoopLimit.getValue();

  if (limit == 0)
  {
    LOG4CXX_DEBUG(logger, "compute iterations limit disabled");
    return 0;
  }

  double scale = static_cast<double>(limit) / static_cast<double>(totalIterations);

  if (scale > 1.0)
    return 0;

  size_t result = std::max(static_cast<size_t>(itinDistance * scale), static_cast<size_t>(1));
  LOG4CXX_INFO(logger,
               ", compute iterations limit total=" << totalIterations << ", limit=" << limit
                                                   << ", itins=" << itinDistance << ", scale="
                                                   << scale << ", result itins=" << result);
  return result;
}

void
FareValidatorOrchestrator::processCarrierFares(
    ShoppingTrx& trx,
    const ItinIndex::Key& cxrKey,
    uint32_t legId,
    ShoppingTrx::Leg& curLeg,
    Itin* journeyItin,
    ShoppingTrx::AltDatePairs& altDatesMap,
    FareMarketRuleController& ruleController,
    FareMarketRuleController& cat4ruleController,
    FareMarket* fM,
    ShpBitValidationCollector::FMValidationSharedData* sharedData,
    PaxTypeFare* ptf,
    const bool callback,
    const uint32_t numberOfFaresToProcess)
{
  const int skipThreshold = shoppingSkipThreshold.getValue();
  const bool skipFares = trx.diagnostic().diagnosticType() != Diagnostic911 ||
                         trx.diagnostic().diagParamMapItem("DD") == "SKIPS";
  ItinIndex& sIG = curLeg.carrierIndex();

  bool isStopOverLeg = curLeg.stopOverLegFlag();
  bool usePerCarrierBitmap = trx.isIataFareSelectionApplicable() && !isStopOverLeg;

  if (usePerCarrierBitmap)
    fM->setComponentValidationForCarrier(cxrKey);

  uint32_t beginLeg = (isStopOverLeg) ? curLeg.jumpedLegIndices().front() : legId;
  uint32_t endLeg = (isStopOverLeg) ? curLeg.jumpedLegIndices().back() + 1 : legId + 1;
  std::map<PaxTypeCode, int> faresPassedPerPax;
  std::vector<PaxTypeFare*> tempAllPaxTypeFare(fM->allPaxTypeFare().size());
  // Iterate through all of the fares
  std::vector<PaxTypeFare*>::iterator pTFIter = fM->allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator pTFEndIter = fM->allPaxTypeFare().end();
  bool fareMarketForSpanishDiscount = trx.isSpanishDiscountFM(fM);

  if (trx.isAltDates() && numberOfFaresToProcess < DELAY_VALIDATION_OFF)
  {
    if (callback)
    {
      validateNonFltRulesForAltDates(trx, &cxrKey, ptf, fM, numberOfFaresToProcess, callback);
    }

    pTFIter = fM->allPaxTypeFareProcessed().begin();
    pTFEndIter = fM->allPaxTypeFareProcessed().end();

    if (pTFIter == pTFEndIter) // no valid fares found
      return;
  }
  else
  {
    if (find_if(pTFIter, pTFEndIter, isSpecialRout<PaxTypeFare*>()) == fM->allPaxTypeFare().end())
    {
      pTFIter = fM->allPaxTypeFare().begin();
      pTFEndIter = fM->allPaxTypeFare().end();
    }
    else
    {
      std::copy(
          fM->allPaxTypeFare().begin(), fM->allPaxTypeFare().end(), tempAllPaxTypeFare.begin());
      std::stable_partition(
          tempAllPaxTypeFare.begin(), tempAllPaxTypeFare.end(), isNotSpecialRout<PaxTypeFare*>());
      pTFIter = tempAllPaxTypeFare.begin();
      pTFEndIter = tempAllPaxTypeFare.end();
    }
  }

  int faresPassed = 0, bitsValidated = 0;
  bool skipping = false;
  uint32_t noOfValidFares = 0;
  const size_t iterationsLimit =
      computeIterationsLimit(pTFIter, pTFEndIter, isStopOverLeg, sIG, trx, legId, cxrKey);
  std::vector<PaxTypeFare*> uniqueFares;
  std::vector<PaxTypeFare*> duplicatedFares;

  while (true)
  {
    for (; pTFIter != pTFEndIter; ++pTFIter)
    {
      bool isFirstValidBit = true;
      if (fallback::reworkTrxAborter(&trx))
        checkTrxAborted(trx);
      else
        trx.checkTrxAborted();
      PaxTypeFare* curFare = *pTFIter;

      if (UNLIKELY(curFare == nullptr))
        continue;

      bool curFareEligibleForSpanishDiscount = curFare->isSpanishDiscountEnabled();

      if (UNLIKELY(usePerCarrierBitmap))
        curFare->setComponentValidationForCarrier(cxrKey, trx.isAltDates(), trx.mainDuration());

      if (curFare->isFlightBitmapInvalid())
        continue;

      bool passed = skipping;
      bool isAllInvalid = true;
      // a set of travel segments that will always fail booking code
      // validation for this fare. Check if any segs are in this set
      // before proceeding with other validation.
      std::set<const TravelSeg*> failedSegs;
      const TSELatencyData metrics(trx, "FVO FARE BITMAP VALIDATE");
      ItinIndex::ItinIndexIterator iRCIter =
          (isStopOverLeg) ? sIG.beginAcrossStopOverRow(trx, legId, cxrKey) : sIG.beginRow(cxrKey);
      ItinIndex::ItinIndexIterator iRCEIter =
          (isStopOverLeg) ? sIG.endAcrossStopOverRow() : sIG.endRow();
      ShoppingTrx::AltDatePairs* altDatePairs = getAltDatePairsForDurationWithAtLeastOneValidPair(
          trx, curFare, legId, cxrKey, isStopOverLeg, sIG);

      bool isThisFareDuplicated = false;

      if (UNLIKELY((trx.getTrxType() == PricingTrx::IS_TRX) &&
                   (trx.getRequest()->isExpAccCorpId())))
      {
        for (PaxTypeFare* currUniqueFare : uniqueFares)
        {
          if (ShoppingUtil::isDuplicatedFare(currUniqueFare, curFare))
          {
            isThisFareDuplicated = true;
            // curFare->setCategoryValid(RuleConst::FLIGHT_APPLICATION_RULE, false );
            // curFare->flightBitmapAllInvalid() = true;
            duplicatedFares.push_back(curFare);
            break;
          }
        }

        if (!isThisFareDuplicated)
          uniqueFares.push_back(curFare);
      }

      for (size_t iterationsCount = 0; iRCIter != iRCEIter; ++iRCIter, ++iterationsCount)
      {
        if (iterationsLimit != 0)
        {
          if (iterationsCount >= iterationsLimit)
            break;
        }

        ItinIndex::ItinCell& cell = *iRCIter;
        ItinIndex::ItinCellInfo& curItinCellInfo = cell.first;
        Itin* curItinCellItin = cell.second;
        const int bitIndex = iRCIter.bitIndex();

        if (curFare->isFlightValid(bitIndex) == false)
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
          FareMarket::FMItinMapCI consItr = fM->fMItinMap().find(curItinCellItin);
          if (consItr == fM->fMItinMap().end())
          {
            if (!LocUtil::isWholeTravelInSpain(curItinCellItin->travelSeg()))
            {
              fM->addFMItinMap(curItinCellItin, false);
              curFare->setFlightInvalid(bitIndex, RuleConst::SPANISH_DISCOUNT_NOT_APPLICABLE);
              continue;
            }
            else
              fM->addFMItinMap(curItinCellItin, true);
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

        bool datePairOutsideMainDuration = false;
        DatePair datePair;

        if (UNLIKELY(trx.isAltDates()))
        {
          setFBMStatusForAllDuration(
              bitIndex, curItinCellItin->travelSeg().front()->departureDT(), legId, curFare, trx);

          if (!ShoppingAltDateUtil::getDatePair(*altDatePairs,
                                                curItinCellItin->travelSeg().front()->departureDT(),
                                                legId,
                                                datePair))
          {
            datePairOutsideMainDuration = true;

            if (usePerCarrierBitmap)
            {
              uint64_t duration = ShoppingAltDateUtil::getDuration(datePair);
              curFare->setComponentValidationForCarrier(cxrKey, true, duration);
            }

            if (!ShoppingAltDateUtil::getDatePair(
                    trx.altDatePairs(),
                    curItinCellItin->travelSeg().front()->departureDT(),
                    legId,
                    datePair))
            {
              curFare->setFlightInvalid(bitIndex, RuleConst::NO_DATEPAIR_FOUND);
              continue;
            }
          }

          journeyItin = ShoppingAltDateUtil::getJourneyItin(altDatesMap, datePair);

          if (!journeyItin)
          {
            curFare->setFlightInvalid(bitIndex, RuleConst::NO_DATEPAIR_FOUND);
            continue;
          }

          if (!curFare->getAltDatePass(datePair))
          {
            curFare->setFlightInvalid(bitIndex, curFare->getAltDateStatus(datePair));
            continue;
          }
        }

        if ((isStopOverLeg && bitIndex >= int(_asoMaxValidations)) || (skipFares && passed))
          break;

        //### ASO Optimiziation #2/#3 ###//
        // Check to see if we have an ASO leg and that the
        // bitmap size warrants the invalidation of bits with
        // flights that exceed the configured number of connection
        // point
        // Check to see if the formulated flight has more than
        // the configured connection points limit.  If it does,
        // we do not want to use the filter logic at all, we will fail it for now
        if (UNLIKELY((isStopOverLeg) &&
                     (curFare->getFlightBitmapSize() >= ((trx.asoBitmapThreshold() > 0)
                                                             ? trx.asoBitmapThreshold()
                                                             : _asoConxnPointBitmapSizeThreshold))))
        {
          if (iRCIter.totalTravelSegSize() > ((trx.asoConxnPointLimit() > 0)
                                                  ? (trx.asoConxnPointLimit() + 1)
                                                  : _asoConxnPointMax + 1))
          {
            // Use invalid shopping flight bitmap value to invalidate this bit
            curFare->setFlightInvalid(bitIndex, RuleConst::FLIGHT_EXCEEDS_CONXN_POINT_LIMIT);
            continue;
          }
        }

        isAllInvalid = false;
        const std::vector<TravelSeg*>& travelSegs = curItinCellItin->travelSeg();
        {
          std::vector<TravelSeg*>::const_iterator seg = travelSegs.begin();
          std::vector<TravelSeg*>::const_iterator segEnd = travelSegs.end();

          const TSELatencyData metrics(trx, "FVO FARE CHECK BAD TRAVEL SEG");

          for (; seg != segEnd; ++seg)
          {
            if (failedSegs.count(*seg) != 0)
              break;
          }

          if (seg != segEnd)
          {
            curFare->setFlightInvalid(bitIndex, RuleConst::BOOKINGCODE_FAIL);
            continue;
          }
        }
        const TSELatencyData metrics(trx, "FVO FARE BIT VALIDATE");

        ShoppingRtgMap* rtgMap(nullptr);
        std::vector<BCETuning>* bceTunningData(nullptr);

        TSE_ASSERT(sharedData);
        sharedData->updateFareMarketData(fM, bitIndex);
        rtgMap = sharedData->getRoutingMapForBit(bitIndex);
        bceTunningData = sharedData->getBCEData(bitIndex);

        if (UNLIKELY((*rtgMap).empty() && RoutingUtil::isSpecialRouting(*curFare, false)))
        {
          processFareWithPublishedRouting(trx,
                                          fM,
                                          bitIndex,
                                          journeyItin,
                                          cell,
                                          ruleController,
                                          cat4ruleController,
                                          (*rtgMap),
                                          *bceTunningData,
                                          curLeg,
                                          beginLeg,
                                          endLeg,
                                          tempAllPaxTypeFare);
        }

        FareValidatorOrchestrator::FMBackup fmb;
        prepareShoppingValidation(trx, journeyItin, cell, fM, fmb, beginLeg, endLeg);

        CabinType cab;
        cab.setEconomyClass();
        _bitmapOpOrderer.performBitmapOperations(trx,
                                                 *curFare,
                                                 *fM,
                                                 bitIndex,
                                                 *journeyItin,
                                                 curItinCellInfo,
                                                 (curLeg.preferredCabinClass() < cab),
                                                 ruleController,
                                                 cat4ruleController,
                                                 (*rtgMap),
                                                 *bceTunningData);

        sharedData->collectFareMarketData(fM, bitIndex);

        if (UNLIKELY(trx.isAltDates()))
        {
          if (datePairOutsideMainDuration && !usePerCarrierBitmap)
          {
            setBitmapStatusForDatePair(bitIndex, curFare, datePair);
          }

          if (!curFare->altDateFltBitStatus().empty())
          {
            ShoppingAltDateUtil::setAltDateFltBit(curFare, bitIndex);
          }
        }

        cleanupAfterShoppingValidation(trx, journeyItin, fM, fmb, beginLeg, endLeg);
        const PaxTypeFare::FlightBit& bitInfo = curFare->flightBitmap()[bitIndex];
        uint32_t startN = 0;
        uint32_t segmentCount = travelSegs.size();

        for (uint32_t n = startN; n < segmentCount && n < bitInfo._segmentStatus.size(); ++n)
        {
          const uint32_t failFlags =
              (PaxTypeFare::BKSS_FAIL | PaxTypeFare::BKSS_FAIL_T999 |
               PaxTypeFare::BKSS_FAIL_REC1_T999 | PaxTypeFare::BKSS_FAIL_CONV1_T999 |
               PaxTypeFare::BKSS_FAIL_CONV2_T999 | PaxTypeFare::BKSS_FAIL_MIXEDCLASS |
               PaxTypeFare::BKSS_FAIL_LOCALMARKET | PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC |
               PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL | PaxTypeFare::BKSS_DIFFERENTIAL |
               PaxTypeFare::BKSS_FAIL_PRIME_RBD_CAT25);

          if (bitInfo._segmentStatus[n]._bkgCodeSegStatus.value() & failFlags)
          {
            const TSELatencyData metrics(trx, "FVO FARE ADD BAD SEG");
            failedSegs.insert(travelSegs[n]);
          }
        }

        ++bitsValidated;

        if (curFare->isFlightValid(bitIndex))
        {
          passed = true;

          if (LIKELY(isFirstValidBit))
          {
            // For the next iteration(of bitmap), it is no longer first valid bit
            isFirstValidBit = false;

            if (UNLIKELY(!curFare->isLongConnectFare() && !curLeg.isSopEmpty() &&
                         curLeg.sop()[curItinCellInfo.sopIndex()].isLngCnxSop()))
            {
              // Set PTF flag for long connect if the first valid bit is long connect
              curFare->setLongConnectFare(true);
            }
          }
        }
      }

      if (iRCIter != iRCEIter)
      {
        const int numBits = int(curFare->flightBitmap().size());

        for (int bit = iRCIter.bitIndex(); bit < numBits; ++bit)
        {
          if (skipFares && passed && (!isStopOverLeg || bit < int(_asoTotalMaxValidations)))
          {
            curFare->setFlightInvalid(bit, RuleConst::SKIP);
          }
          else
          {
            curFare->setFlightInvalid(bit, RuleConst::FLIGHT_EXCEEDS_CONXN_POINT_LIMIT);
          }
        }
      }

      if (LIKELY(skipping == false && isAllInvalid == false))
      {
        if (passed)
          ++noOfValidFares;

        if (LIKELY(skipFares))
        {
          if (passed)
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
              skipping = true;

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
                    skipping = false;
                    break;
                  }
                }
              }
            }
          }
        }
      }
    }

    if (!trx.isAltDates() || numberOfFaresToProcess >= DELAY_VALIDATION_OFF)
    {
      break;
    }

    if (noOfValidFares < numberOfFaresToProcess)
    {
      int faresToProcess = (numberOfFaresToProcess - noOfValidFares);
      validateNonFltRulesForAltDates(trx, &cxrKey, ptf, fM, faresToProcess, callback);
    }
    else
      break;

    if (fM->allPaxTypeFareProcessed().size() == 0)
      break;

    pTFIter = fM->allPaxTypeFareProcessed().begin();
    pTFEndIter = fM->allPaxTypeFareProcessed().end();
  } // end while

  if ((trx.getTrxType() == PricingTrx::IS_TRX) && (trx.getRequest()->isExpAccCorpId()))
  {
    Diagnostic& trxDiag = trx.diagnostic();
    DiagManager diag(trx, trxDiag.diagnosticType());
    bool diag325Enabled = (trx.diagnostic().diagnosticType() == Diagnostic325) &&
                          (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "REC3");

    if (diag325Enabled)
      ShoppingUtil::displayRemovedFares(*fM, uniqueFares.size(), duplicatedFares, diag);
  }

  if (fallback::fixed::fallbackRemoveFailedSopsInvalidation())
  {
    // The following code invalidates SOP cabin class
    // using some obscure criteria which is not related
    // to actual cabin class validity of the SOP. We want
    // to disable this logic for IBF requests.
    TSE_ASSERT(trx.getRequest() != nullptr);
    if (trx.getRequest()->isBrandedFaresRequest())
    {
      return;
    }

    if (trx.excTrxType() == PricingTrx::EXC_IS_TRX)
    {
      return;
    }

    FareUtil::invalidateFailedSops(trx, fM, cxrKey, curLeg, legId);

  } // fallback end: fallback::fixed::fallbackRemoveFailedSopsInvalidation
}

//----------------------------------------------------------------------------
// validate non flight related rule
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::validateAltDateShoppingRules(ShoppingTrx& trx,
                                                        FareMarket* fM,
                                                        Itin* journeyItin,
                                                        uint32_t legId,
                                                        DatePair& datePair,
                                                        std::vector<PaxTypeFare*>& faresPassedCat2,
                                                        PaxTypeFare* ptf,
                                                        const uint32_t numberOfFaresToProcess,
                                                        const bool callback)
{
  TSELatencyData metrics(trx, "FVO VALIDATE ALTDATE SHOPPING RULES");
  RuleControllerWithChancelor<FareMarketRuleController> shoppingRuleController(
      ShoppingComponentValidation);
  // get leg reference
  uint32_t beginLeg = legId;
  uint32_t endLeg = legId + 1;
  // set up carrier
  // set up travelSeg for FareMarket
  std::vector<TravelSeg*> tvlSegTemp;
  fM->travelSeg().swap(tvlSegTemp);
  fM->travelSeg().clear();
  fM->travelSeg().insert(fM->travelSeg().end(),
                         journeyItin->travelSeg().begin() + beginLeg,
                         journeyItin->travelSeg().begin() + endLeg);

  for (auto* tvlSeg : fM->travelSeg())
  {
    if (AirSeg* airSeg = tvlSeg->toAirSeg())
      airSeg->carrier() = fM->governingCarrier();
  }

  uint32_t noOfFaresPassed = 0;
  int fareIndex = 0;
  int lastFareProcessed = fM->lastFareProcessed();
  int firstFareProcessed = fM->firstFareProcessed();
  // Process rules once for each carrier/leg
  std::vector<PaxTypeFare*>::iterator pTFIter = fM->allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator pTFEndIter = fM->allPaxTypeFare().end();

  for (; pTFIter != pTFEndIter; ++pTFIter, ++fareIndex)
  {
    PaxTypeFare*& curFare = static_cast<PaxTypeFare*&>(*pTFIter);
    PaxTypeFare* curFarePtr = &(*(*pTFIter));

    if (curFare == nullptr)
    {
      continue;
    }

    if (curFare->isKeepForRoutingValidation())
    {
      continue;
    }

    if (trx.isIataFareSelectionApplicable() && !trx.legs()[legId].stopOverLegFlag())
    {
      ItinIndex::Key cxrKey;
      ShoppingUtil::createCxrKey(fM->governingCarrier(), cxrKey);
      curFare->setComponentValidationForCarrier(
          cxrKey, trx.isAltDates(), ShoppingAltDateUtil::getDuration(datePair));
    }

    if (trx.isAltDates() && numberOfFaresToProcess < 10000)
    {
      if (callback && curFarePtr == ptf) // found fare?
        fM->setFoundPTF(true);

      if (lastFareProcessed < 0) // haven't found valid fares yet
      {
        if (curFare->isFltIndependentValidationFVO())
          continue;
      }
      else // succeeding dates (have found valid fares from preceeding datepair)
      {
        if (fareIndex <
            firstFareProcessed) // start process on the first valid fare by preceeding date
          continue;

        if (fareIndex >
            lastFareProcessed) // process only up to last valid fare processed by preceeding date
          break;
      }
    }

    curFare->setFltIndependentValidationFVO();

    // if the cat 15 validation fail in the fare state or not all category is valid.
    curFare->shoppingComponentValidationFailed() =
        !(curFare->isCat15SecurityValid() && curFare->areAllCategoryValid());

    //################################################################
    //# Checks if fare passed PRE_VALIDATION phase during collection
    //#  or it was it was failed in previous call
    //################################################################
    if (!curFare->flightBitmap().empty() || !curFare->isValid())
      continue;

    if (fM->firstFareProcessed() < 0)
      fM->setFirstFareProcessed(fareIndex);

    // set the shoppingComponentValidationFailed
    if (fM->direction() == FMDirection::OUTBOUND && curFare->directionality() == TO)
      continue;

    if (trx.getTrxType() == PricingTrx::FF_TRX)
    {
      if (fM->direction() == FMDirection::INBOUND && curFare->directionality() == FROM)
        continue;
    }

    if (!curFare->getAltDatePass(datePair))
    {
      continue;
    }
    else
    {
      ShoppingAltDateUtil::cleanUpCategoryProcess(curFare);
    }

    fM->setGlobalDirection(curFare->globalDirection());
    TSELatencyData metrics(trx, "FVO VALIDATE ALTDATE SHOPPING RULES-VALIDATE");
    shoppingRuleController.validate(trx, *journeyItin, *curFare);

    if (!curFare->areAllCategoryValid())
    {
      ShoppingAltDateUtil::setAltDateStatus(curFare, datePair, legId);
    }
    else if (curFare->isCategoryValid(2))
    {
      // set valid cat 2 fares to "not processed" for next phase
      // of validation (component with flights rule validation)
      faresPassedCat2.push_back(curFare);
    }

    if (trx.isAltDates() && numberOfFaresToProcess < 10000 &&
        lastFareProcessed < 0) // no set of valid fares found yet
      fM->setLastFareProcessed(fareIndex);

    if (trx.isAltDates() && numberOfFaresToProcess < 10000 && curFare->getAltDatePass(datePair))
    {
      if (callback)
        curFare->setFareCallbackFVO();

      if (lastFareProcessed < 0) // no set of valid fares found yet
      {
        ++noOfFaresPassed;

        fM->addProcessedPTF(curFarePtr);

        if (fM->foundPTF() && noOfFaresPassed >= numberOfFaresToProcess)
          break;
      }
      else // valid fares has been found by preceeding date
      {
        fM->addProcessedPTF(curFarePtr);
      }
    }
  }

  fM->travelSeg().swap(tvlSegTemp);
}

//----------------------------------------------------------------------------
// create flight bit map for alt date
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::createAltDatesFlightBitmaps(ShoppingTrx& trx,
                                                       FareMarket* fM,
                                                       const uint32_t legId,
                                                       const uint32_t flightSize,
                                                       const ItinIndex::Key& carrierKey,
                                                       const uint64_t& duration)
{
  TSELatencyData metrics(trx, "FVO CREATE FLIGHT BITMAPS");
  // createFlightBitmapsForPaxTypeFare(fM,flightSize);
  // Set the flight bitmap sizes to the size of the inner map
  // Iterate through all of the fares
  int lastFareProcessed = fM->lastFareProcessed();
  int firstFareProcessed = fM->firstFareProcessed();
  int fareIndex = -1;
  const bool usePerCarrierBitmap = (trx.getTrxType() == PricingTrx::IS_TRX) &&
                                   trx.isIataFareSelectionApplicable() &&
                                   !trx.legs()[legId].stopOverLegFlag();

  for (auto* paxTypeFare : fM->allPaxTypeFare())
  {
    ++fareIndex;
    if (!paxTypeFare)
      continue;

    if (trx.isAltDates() && lastFareProcessed >= 0)
    {
      if (fareIndex < firstFareProcessed)
        continue;

      if (fareIndex > lastFareProcessed)
        break;
    }

    if (usePerCarrierBitmap)
      paxTypeFare->setComponentValidationForCarrier(carrierKey, trx.isAltDates(), duration);

    // if flight bitmap is already created during the previous call we do not want to spend a time
    // to process it twice
    if (!paxTypeFare->flightBitmap().empty() || !paxTypeFare->isValid())
    {
      continue;
    }

    //###########################################################
    //# Checks if the fare passed SHOPPING_COMPONENT_VALIDATION #
    //###########################################################
    paxTypeFare->shoppingComponentValidationFailed() =
        !paxTypeFare->isAltDateValid() || !paxTypeFare->isValid();

    if (paxTypeFare->shoppingComponentValidationFailed())
      continue;

    if (fM->direction() == FMDirection::OUTBOUND && paxTypeFare->directionality() == TO)
      continue;

    if (trx.getTrxType() == PricingTrx::FF_TRX)
    {
      if (fM->direction() == FMDirection::INBOUND && paxTypeFare->directionality() == FROM)
        continue;
    }

    // Add the proper number of "bits" to the flight bitmap
    if (paxTypeFare->flightBitmap().empty())
    {
      createFlightBitmap(trx, paxTypeFare, flightSize, *fM, carrierKey);

      if ((legId == 0 && trx.altDateReuseOutBoundProcessed()) ||
          (legId == 1 && trx.altDateReuseInBoundProcessed()))
      {
        paxTypeFare->setAltDateFltBitStatusSize();
      }
    }

    if (usePerCarrierBitmap)
      createAltDateFBMforAllDurationNew(trx, carrierKey, flightSize, paxTypeFare);
    else
      createAltDateFBMforAllDuration(trx, paxTypeFare);
  } // for (; pTFIter != pTFEndIter; ++pTFIter)
}

void
FareValidatorOrchestrator::callback(ShoppingTrx& trx,
                                    FareMarket* fM,
                                    PaxTypeFare* ptf,
                                    ItinIndex::Key* carrierKey)
{
  if (trx.isSumOfLocalsProcessingEnabled())
  {
    _shoppingFVO.callback(trx, fM, ptf, carrierKey);
    return;
  }

  TSELatencyData metrics(trx, "FVO CALLBACK");
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();

  uint32_t legIndex = fM->legIndex();
  ShoppingTrx::Leg& curLeg = legs[legIndex];

  FareMarketRuleController* ruleController = &trx.fareMarketRuleController();
  FareMarketRuleController* cat4ruleController = &trx.cat4RuleController();
  ShoppingTrx::AltDatePairs* altDatePairs = &trx.altDatePairs();
  Itin* journeyItin = trx.journeyItin();

  std::deque<Itin> journeyItins;
  std::deque<FareMarketRuleController> ruleControllers;
  std::deque<FareMarketRuleController> cat4ruleControllers;
  ShoppingTrx::AltDatePairs altDateJourneyItins;

  ItinIndex::Key cxrKey;
  ShoppingUtil::createCxrKey(fM->governingCarrier(), cxrKey);

  if (trx.isIataFareSelectionApplicable() && !curLeg.stopOverLegFlag())
    fM->setComponentValidationForCarrier(cxrKey);

  ShpBitValidationCollector::FMValidationSharedData* sharedData(
      trx.getBitValidationCollector().getFMSharedData(cxrKey, fM));

  fM->setFoundPTF(false);
  bool delayValidationToApply = (trx.paxType().size() <= 1) && trx.isAltDates();
  const uint32_t numberOfFaresToProcess =
      (delayValidationToApply ? _numberOfFaresToProcessADI : DELAY_VALIDATION_OFF);

  processCarrierFares(trx,
                      cxrKey,
                      legIndex,
                      curLeg,
                      journeyItin,
                      *altDatePairs,
                      *ruleController,
                      *cat4ruleController,
                      &(*fM),
                      sharedData,
                      ptf,
                      true,
                      numberOfFaresToProcess);

  setFareStatus(trx, fM->allPaxTypeFare(), fM->lastFareProcessed());
  LOG4CXX_DEBUG(logger, "- setting fare status from callback");
}

class FailedUnusedFare
{
public:
  FailedUnusedFare(PricingTrx& trx, DiagCollector& diag) : _trx(trx), _diag(diag) {}

  inline bool operator()(const PaxTypeFare* ptFare)
  {
    if (ptFare->cat25BasePaxFare())
      return false;

    if (!ptFare->isNoDataMissing())
    {
      if (_diag.isActive())
      {
        _diag << toString(*ptFare) << "\n";
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
       << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << std::endl;
    return dc.str();
  }

private:
  PricingTrx& _trx;
  DiagCollector& _diag;
};

//----------------------------------------------------------------------------
// releaseUnusedFares()
//----------------------------------------------------------------------------
void
FareValidatorOrchestrator::releaseUnusedFares(PricingTrx& trx)
{
  // Check for WPNETT
  if (trx.getRequest()->isWpNettRequested())
    return;

  DiagMonitor diag(trx, Diagnostic469);
  bool diagEnabled = diag.diag().isActive();
  FailedUnusedFare gFailedFare(trx, diag.diag());

  bool releaseFares = releaseUnusedFaresAllItins.getValue();

  for (auto const itin : trx.itin())
  {
    for (const auto fm : itin->fareMarket())
    {
      if (UNLIKELY(!fm))
        continue;

      FareMarket& fareMarket = *fm;

      if (!fareMarket.specialRtgFound())
        continue;

      if (UNLIKELY(diagEnabled))
      {
        diag << "\nRELEASE FARES UNUSED BY CAT25/ROUTING:\n";
        diag << "FARE MARKET: " << FareMarketUtil::getDisplayString(fareMarket) << "\n";
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

          if (UNLIKELY(diagEnabled))
          {
            diag << "RELEASED " << released << " OF " << total << " FARES FROM ALLPAXTYPEFARE\n \n";
          }
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

    if (UNLIKELY(!releaseFares))
      break;
  }
}

ShoppingTrx::AltDatePairs*
FareValidatorOrchestrator::getAltDatePairsForDurationWithAtLeastOneValidPair(
    ShoppingTrx& trx,
    PaxTypeFare* curFare,
    const uint32_t legId,
    const ItinIndex::Key& cxrKey,
    const bool isStopOverLeg,
    ItinIndex& sIG)
{
  if (UNLIKELY(trx.isAltDates()))
  {
    std::set<uint64_t> checkedDurations;
    uint64_t currDuration = trx.mainDuration();
    bool isDurationValid = durationWithAtLeastOneValidPair(
        trx, legId, cxrKey, isStopOverLeg, sIG, currDuration, curFare);
    checkedDurations.insert(currDuration);

    while (!isDurationValid && trx.durationAltDatePairs().size() > checkedDurations.size())
    {
      currDuration = getNextDuration(trx, checkedDurations);
      isDurationValid = durationWithAtLeastOneValidPair(
          trx, legId, cxrKey, isStopOverLeg, sIG, currDuration, curFare);
      checkedDurations.insert(currDuration);
    }

    // nothing valid found, return main
    if (isDurationValid)
    {
      curFare->setDurationUsedInFVO(currDuration);
      return &(trx.durationAltDatePairs().find(currDuration))->second;
    }
    else
    {
      curFare->setDurationUsedInFVO(trx.mainDuration());
      return &(trx.durationAltDatePairs().find(trx.mainDuration()))->second;
    }
  }
  else
  {
    curFare->setDurationUsedInFVO(trx.mainDuration());
    return nullptr;
  }
}

bool
FareValidatorOrchestrator::durationWithAtLeastOneValidPair(ShoppingTrx& trx,
                                                           const uint32_t legId,
                                                           const ItinIndex::Key& cxrKey,
                                                           const bool isStopOverLeg,
                                                           ItinIndex& sIG,
                                                           const uint64_t currDuration,
                                                           const PaxTypeFare* curFare)
{
  ShoppingTrx::AltDatePairs& altDatePairs = (trx.durationAltDatePairs().find(currDuration))->second;
  ItinIndex::ItinIndexIterator iRCIter =
      (isStopOverLeg) ? sIG.beginAcrossStopOverRow(trx, legId, cxrKey) : sIG.beginRow(cxrKey);
  ItinIndex::ItinIndexIterator iRCEIter =
      (isStopOverLeg) ? sIG.endAcrossStopOverRow() : sIG.endRow();

  for (; iRCIter != iRCEIter; ++iRCIter)
  {
    ItinIndex::ItinCell& cell = *iRCIter;
    Itin* curItinCellItin = cell.second;
    DatePair datePair;

    if (ShoppingAltDateUtil::getDatePair(
            altDatePairs, curItinCellItin->travelSeg().front()->departureDT(), legId, datePair))
    {
      if (curFare->getAltDatePass(datePair))
      {
        return true;
      }
    }
  }

  return false;
}

uint64_t
FareValidatorOrchestrator::getNextDuration(const ShoppingTrx& trx,
                                           const std::set<uint64_t>& checkedDurations)
{
  uint8_t altDatePairSize = 0;
  uint64_t nextDuration = 0;
  PricingTrx::DurationAltDatePairs::const_iterator durAltDatePairsIter =
      trx.durationAltDatePairs().begin();

  for (; durAltDatePairsIter != trx.durationAltDatePairs().end(); ++durAltDatePairsIter)
  {
    if (checkedDurations.count(durAltDatePairsIter->first) == 0)
    {
      if (altDatePairSize < durAltDatePairsIter->second.size())
      {
        altDatePairSize = durAltDatePairsIter->second.size();
        nextDuration = durAltDatePairsIter->first;
      }
    }
  }

  return nextDuration;
}

void
FareValidatorOrchestrator::removeFaresKeepForRoutingValidation(ShoppingTrx& trx)
{
  std::vector<ShoppingTrx::Leg>& sLV = trx.legs();

  if (sLV.empty())
    return;

  uint32_t legId = 0;

  for (auto& curLeg : sLV)
  {
    // Get leg reference
    ItinIndex& sIG = curLeg.carrierIndex();
    for (auto& itinMatrix : sIG.root())
    {
      auto curFareMarket = getFirstFareMarket(trx, legId, itinMatrix.first);
      if (nullptr == curFareMarket)
        continue;

      FaresKeepForRoutingValidationRemover failFares;

      for (auto& ptb : curFareMarket->paxTypeCortege())
      {
        std::vector<PaxTypeFare*>& ptFares = ptb.paxTypeFare();
        ptFares.erase(std::remove_if(ptFares.begin(), ptFares.end(), failFares), ptFares.end());
      }

      std::vector<PaxTypeFare*>& allPaxTypeFares = curFareMarket->allPaxTypeFare();
      allPaxTypeFares.erase(
          std::remove_if(allPaxTypeFares.begin(), allPaxTypeFares.end(), failFares),
          allPaxTypeFares.end());
    }

    ++legId;
  }
}

void
FareValidatorOrchestrator::processFareWithPublishedRouting(
    ShoppingTrx& trx,
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
    std::vector<PaxTypeFare*>& tempAllPaxTypeFare)
{
  if (_numPublishedRoutingFareToValidate == 0)
  {
    return;
  }

  uint32_t numPaxProcessed = 0;

  for (PaxTypeFare* curFare : tempAllPaxTypeFare)
  {

    if (curFare == nullptr || curFare->isFlightBitmapInvalid())
    {
      continue;
    }

    if (RoutingUtil::isSpecialRouting(*curFare, false))
    {
      return;
    }

    // process routing only
    curFare->setKeepForRoutingValidation();
    processFlightBitMap(trx,
                        curFare,
                        fM,
                        bitIndex,
                        journeyItin,
                        curItinCell,
                        ruleController,
                        cat4RuleController,
                        rtMap,
                        bceTuningData,
                        curLeg,
                        beginLeg,
                        endLeg);
    curFare->setKeepForRoutingValidation(false);
    ++numPaxProcessed;

    if (numPaxProcessed >= _numPublishedRoutingFareToValidate)
    {
      return;
    }
  }

  return;
}

void
FareValidatorOrchestrator::processFlightBitMap(ShoppingTrx& trx,
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
                                               uint32_t endLeg)
{
  FareValidatorOrchestrator::FMBackup fmb;

  prepareShoppingValidation(trx, journeyItin, curItinCell, fM, fmb, beginLeg, endLeg);

  CabinType cab;
  cab.setEconomyClass();
  _bitmapOpOrderer.performBitmapOperations(trx,
                                           *curFare,
                                           *fM,
                                           bitIndex,
                                           *journeyItin,
                                           curItinCell.first,
                                           (curLeg.preferredCabinClass() < cab),
                                           ruleController,
                                           cat4RuleController,
                                           rtMap,
                                           bceTuningData);

  cleanupAfterShoppingValidation(trx, journeyItin, fM, fmb, beginLeg, endLeg);
  return;
}

//----------------------------------------------------------------------------
// process(RexExchangeTrx)
//---------------------------------------------------------------------------
bool
FareValidatorOrchestrator::process(RexExchangeTrx& trx)
{
  LOG4CXX_DEBUG(logger, " - Entered process(RexExchangeTrx)");
  return process(static_cast<RexPricingTrx&>(trx));
}
namespace
{
void
prevalidateSecuritySFR(Itin itin, StructuredRuleTrx& trx)
{
  RuleControllerWithChancelor<FareMarketRuleController> prevalidationRuleController(PreValidation,
                                                                                    &trx);
  RuleItem& prevalRI =
      prevalidationRuleController.categoryRuleItemSet().categoryRuleItem().ruleItem();
  prevalRI.clearHandlers();
  prevalRI.setHandler(RuleConst::SALE_RESTRICTIONS_RULE, &RuleItem::handleSalesSecurity);
  prevalRI.setHandler(RuleConst::ELIGIBILITY_RULE, &RuleItem::handleEligibility);

  for (FareMarket* fm : trx.fareMarket())
  {
    for (PaxTypeFare* paxTypeFare : fm->allPaxTypeFare())
    {
      if (paxTypeFare->paxType() == nullptr || paxTypeFare->fareClassAppInfo() == nullptr ||
          paxTypeFare->isFareClassAppMissing() || paxTypeFare->fareClassAppSegInfo() == nullptr ||
          paxTypeFare->isFareClassAppSegMissing())
        continue;
      prevalidationRuleController.validate(trx, itin, *paxTypeFare);
    }
  }
}
}

bool
FareValidatorOrchestrator::process(StructuredRuleTrx& trx)
{
  LOG4CXX_DEBUG(logger, " - Entered process(StructuredRuleTrx)");

  Itin itin = *trx.itin().front();

  prevalidateSecuritySFR(itin, trx);

  bookingCodesValidation(_taskId, trx);

  std::vector<uint16_t> categories{
      TRAVEL_RESTRICTIONS_RULE, ADVANCE_RESERVATION_RULE, SALE_RESTRICTIONS_RULE};
  FareMarketRuleController fmRuleController(NormalValidation, categories);
  for (auto* fm : itin.fareMarket())
    fmRuleController.validate(trx, *fm, itin);

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic499) &&
      (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "BRAND"))
  {
    TSELatencyData tld(trx, "FVO PRINT PTFARES");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_fareMarket(trx, printPTFares);
  }

  return true;
}

bool
FareValidatorOrchestrator::refundSkip(const PricingTrx& trx, const FareMarket& fareMarket)
{
  return trx.excTrxType() == PricingTrx::AF_EXC_TRX &&
         static_cast<const RefundPricingTrx&>(trx).trxPhase() !=
             RexPricingTrx::PRICE_NEWITIN_PHASE &&
         !fareMarket.breakIndicator();
}

void
FareValidatorOrchestrator::printBrandLegend(PricingTrx& trx)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;
  diag.enable(Diagnostic499);
  diag << "\nBRAND INFO:";
  diag << "\n EXAMPLE: H 4-QT-10";

  diag << "\n  H  - BKG CODE";
  diag << "\n  4  - BRAND SEQ NO";
  diag << "\n  QT - BRAND ID";
  diag << "\n  1  - BOOKING CODE/BASIS CODE/FARE FAMILY STATUS";
  diag << "\n  0  - BRAND CORP ID STATUS";

  diag << "\n       /1-VALID/0-INVALID/";
  diag << "\n\n";
  diag.flushMsg();
}

void
FareValidatorOrchestrator::displayBrandValidation(PricingTrx& trx,
                                                  PaxTypeFare& paxFare,
                                                  DiagCollector& diag)
{
  if (!(trx.getRequest()->isBrandedFaresRequest() || trx.isBrandsForTnShopping()) ||
      (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "BRAND") ||
      paxFare.getBrandStatusVec().empty())
    return;

  diag << "BRAND VALIDATION:";

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);

  int index = 0;
  for (int i : paxFare.fareMarket()->brandProgramIndexVec())
  {
    diag << " " << i << char(paxFare.getBrandStatusVec()[index].first);
    if (useDirectionality)
    {
      if ((paxFare.getBrandStatusVec()[index].first == PaxTypeFare::BS_HARD_PASS) ||
          (paxFare.getBrandStatusVec()[index].first == PaxTypeFare::BS_SOFT_PASS))
        diag << directionToIndicator(paxFare.getBrandStatusVec()[index].second);
      else
        diag << '-';
    }
    ++index;
  }

  std::ostringstream ibfErrorMessage;
  ibfErrorMessage << " MSG : " << paxFare.getIbfErrorMessage();
  diag << ibfErrorMessage.str();

  diag << "\n";
}

void
FareValidatorOrchestrator::displayFareMarketBrands(PricingTrx& trx,
                                                   FareMarket& fm,
                                                   DiagCollector& diag)
{
  if (!(trx.getRequest()->isBrandedFaresRequest() || trx.isBrandsForTnShopping()) ||
      (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "BRAND"))
    return;

  diag << "FARE MARKET BRANDS:\n";

  BrandedDiagnosticUtil::displayBrandIndices(diag, trx.brandProgramVec(), fm.brandProgramIndexVec(),
                                             trx.diagnostic().diagnosticType() == Diagnostic894);
  BrandedDiagnosticUtil::displayFareMarketErrorMessagePerBrand(diag, trx, fm);
}

FareValidatorOrchestrator::DiagValidationData*
FareValidatorOrchestrator::diagGetValidationResults(PricingTrx& trx,
                                                    PaxTypeFare& paxFare,
                                                    bool isFTDiag,
                                                    NoPNRPricingTrx* noPNRTrx)
{
  DiagValidationData* itinData;
  trx.dataHandle().get(itinData);
  std::ostringstream rtgOss;

  if (!paxFare.isRoutingProcessed())
    rtgOss << "NP";
  else if (!paxFare.isRoutingValid())
    rtgOss << "F";
  else if (paxFare.isRoutingValid())
    rtgOss << "P";
  else
    rtgOss << "";

  itinData->_rtg = rtgOss.str();

  PaxTypeFare::BookingCodeStatus& bkgStatus = paxFare.bookingCodeStatus();
  if (isFTDiag)
  {
    if (!noPNRTrx->isNoMatch())
      itinData->_bkg = "NP";
    else
    {
      if (bkgStatus.isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
        itinData->_bkg = "NP";
      else if (bkgStatus.isSet(PaxTypeFare::BKS_FAIL))
      {
        if (bkgStatus.isSet(PaxTypeFare::BKS_FAIL_FARE_TYPES))
          itinData->_bkg = "F";
        else
          itinData->_bkg = "NP";
      }
      else if (bkgStatus.isSet(PaxTypeFare::BKS_PASS))
        itinData->_bkg = "P";
      else
        itinData->_bkg = "*";
    }
  }
  else
  {
    if (noPNRTrx)
    {
      if (bkgStatus.isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
        itinData->_bkg = "NP";
      else if (bkgStatus.isSet(PaxTypeFare::BKS_FAIL))
      {
        if (bkgStatus.isSet((PaxTypeFare::BookingCodeState)(PaxTypeFare::BKS_FAIL_FARE_TYPES)))
          itinData->_bkg = "P";
        else
          itinData->_bkg = "F";
      }
      else if (bkgStatus.isSet(PaxTypeFare::BKS_PASS))
        itinData->_bkg = "P";
      else
        itinData->_bkg = "*";
    }
    else
    {
      if (bkgStatus.isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
        itinData->_bkg = "NP";
      else if (bkgStatus.isSet(PaxTypeFare::BKS_FAIL_TAG_N))
        itinData->_bkg = "FN";
      else if (bkgStatus.isSet(PaxTypeFare::BKS_FAIL_NOT_KEEP))
        itinData->_bkg = "NK";
      else if (bkgStatus.isSet(PaxTypeFare::BKS_FAIL))
        itinData->_bkg = "F";
      else if (bkgStatus.isSet(PaxTypeFare::BKS_PASS))
        itinData->_bkg = "P";
      else
        itinData->_bkg = "*";

      if (paxFare.getAsBookedClone())
        itinData->_bkg += "R";
    }
  }

  if (!paxFare.isValidForBranding())
    itinData->_rul = "BRN";
  else if (!(paxFare.isCategoryProcessed(2)) && !(paxFare.isCategoryProcessed(3)) &&
           !(paxFare.isCategoryProcessed(4)) && !(paxFare.isCategoryProcessed(5)) &&
           !(paxFare.isCategoryProcessed(6)) && !(paxFare.isCategoryProcessed(7)) &&
           !(paxFare.isCategoryProcessed(8)) && !(paxFare.isCategoryProcessed(9)) &&
           !(paxFare.isCategoryProcessed(11)) && !(paxFare.isCategoryProcessed(15)) &&
           !(paxFare.isCategoryProcessed(16)))
  {
    if (paxFare.puRuleValNeeded())
      itinData->_rul = "NPD";
    else
      itinData->_rul = "NP";
  }
  else if (paxFare.areAllCategoryValid())
  {
    if (paxFare.fare() && paxFare.fare()->isMissingFootnote())
    {
      itinData->_rul = "FFN";
    }
    else
    {
      if (paxFare.isSoftPassed())
        itinData->_rul = "P*";
      else
        itinData->_rul = "P";
    }
  }
  else if (!(paxFare.isCategoryValid(1)))
    itinData->_rul = "F01";
  else if (!(paxFare.isCategoryValid(2)))
    itinData->_rul = "F02";
  else if (!(paxFare.isCategoryValid(3)))
    itinData->_rul = "F03";
  else if (!(paxFare.isCategoryValid(4)))
    itinData->_rul = "F04";
  else if (!(paxFare.isCategoryValid(5)))
  {
    const std::string& failedCat = trx.diagnostic().diagParamMapItem("RL");

    if (failedCat == "F05")
    {
      if (paxFare.reProcessCat05NoMatch())
        itinData->_rul = "F*5";
      else
        itinData->_rul = "F05";
    }
    else
    {
      itinData->_rul = "F05";
    }
  }
  else if (!(paxFare.isCategoryValid(6)))
    itinData->_rul = "F06";
  else if (!(paxFare.isCategoryValid(7)))
    itinData->_rul = "F07";
  else if (!(paxFare.isCategoryValid(8)))
    itinData->_rul = "F08";
  else if (!(paxFare.isCategoryValid(9)))
    itinData->_rul = "F09";
  else if (!(paxFare.isCategoryValid(11)))
    itinData->_rul = "F11";
  else if (!(paxFare.isCategoryValid(13)))
    itinData->_rul = "F13";
  else if (!(paxFare.isCategoryValid(14)))
    itinData->_rul = "F14";
  else if (!(paxFare.isCategoryValid(15)))
    itinData->_rul = "F15";
  else if (!(paxFare.isCategoryValid(16)))
    itinData->_rul = "F16";
  else if (!(paxFare.isCategoryValid(19)))
    itinData->_rul = "F19";
  else if (!(paxFare.isCategoryValid(20)))
    itinData->_rul = "F20";
  else if (!(paxFare.isCategoryValid(21)))
    itinData->_rul = "F21";
  else if (!(paxFare.isCategoryValid(22)))
    itinData->_rul = "F22";
  else if (!(paxFare.isCategoryValid(23)))
    itinData->_rul = "F23";
  else if (!(paxFare.isCategoryValid(25)))
    itinData->_rul = "F25";
  else if (!(paxFare.isCategoryValid(35)))
  {
    if (paxFare.failedByJalAxessRequest())
    {
      if (paxFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
          paxFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
          paxFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD)
        itinData->_rul = "FJN";
      else
        itinData->_rul = "FJP";
    }
    else
      itinData->_rul = "F35";
  }
  else
    itinData->_rul = "FXX";

  if (paxFare.isMatchFareFocusRule())
  {
    itinData->_rul = "FF ";
    itinData->_valid = false;
  }
  else
    itinData->_valid = paxFare.isValid();

  return itinData;
}

void
FareValidatorOrchestrator::displayFlexFaresInfo(PricingTrx& trx,
                                                PaxTypeFare& paxFare,
                                                DiagCollector& diag)
{
  if (!trx.isFlexFare())
    return;

  bool status =
      paxFare.getFlexFaresValidationStatus()->getStatusForAttribute<flexFares::PRIVATE_FARES>();
  diag << (status ? " PRIV" : "");

  if (!status)
  {
    status =
        paxFare.getFlexFaresValidationStatus()->getStatusForAttribute<flexFares::PUBLIC_FARES>();
    diag << (status ? " PUBL" : " N/A ");
  }
}
void
FareValidatorOrchestrator::displayValidatingCarriers(PaxTypeFare& paxFare, DiagCollector& diag)
{
  diag << "VAL-CXR:  ";
  for (const CarrierCode& vcx : paxFare.fareMarket()->validatingCarriers())
  {
    diag << vcx << ":";
    std::vector<CarrierCode>::iterator vcxIt =
        std::find(paxFare.validatingCarriers().begin(), paxFare.validatingCarriers().end(), vcx);
    if (vcxIt != paxFare.validatingCarriers().end())
      diag << "P ";
    else
      diag << "F ";
  }
  diag << "\n";
}

void
FareValidatorOrchestrator::initPTFvalidatingCxrs(PricingTrx& trx,
                                                 Itin& itin,
                                                 FareMarket& fareMarket)
{
  if (UNLIKELY(fareMarket.validatingCarriers().empty()))
    return;

  for (PaxTypeFare* ptf : fareMarket.allPaxTypeFare())
  {
    if (ptf->isValidNoBookingCode() && ptf->validatingCarriers().empty())
      ptf->validatingCarriers() = fareMarket.validatingCarriers();
  }
}

void
FareValidatorOrchestrator::validateKeepOriginalFare(RexPricingTrx& trx)
{
  for (Itin* itin : trx.newItin())
  {
    for (FareMarket* fareMarket : itin->fareMarket())
    {
      if (fareMarket->isNotShopped())
      {
        std::string fare = findValidFareBasisForNotShoppedFareMarket(trx, *fareMarket);

        for (PaxTypeFare* ptf : fareMarket->allPaxTypeFare())
        {
          if (ptf->createFareBasis(0) != fare)
          {
            ptf->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL_NOT_KEEP);
            ptf->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
          }
        }
      }
    }
  }
}

std::string
FareValidatorOrchestrator::findValidFareBasisForNotShoppedFareMarket(const RexPricingTrx& trx,
                                                                     const FareMarket& fm) const
{
  auto isSameFM = [](const FareMarket& fm1, const FareMarket& fm2) -> bool
  {
    return fm1.origin() == fm2.origin() && fm1.destination() == fm2.destination() &&
           fm1.travelSeg()[0]->departureDT() == fm2.travelSeg()[0]->departureDT();
  };

  for (PricingUnit* pu : trx.exchangeItin().front()->farePath().front()->pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      if (isSameFM(*fu->paxTypeFare()->fareMarket(), fm))
      {
        return fu->paxTypeFare()->createFareBasis(0);
      }
    }
  }

  return "";
}

void
FareValidatorOrchestrator::initFlexFareFinalCheck(PricingTrx& trx,
                                                  Itin& itin,
                                                  FareMarket& fareMarket)
{
  RuleValidationChancelor chancelor;

  chancelor.updateContextType(RuleValidationContext::FARE_MARKET);
  chancelor.setPolicy(ELIGIBILITY_RULE, new FlexFaresValidationPolicyNoEligibility());
  chancelor.setPolicy(ADVANCE_RESERVATION_RULE, new FlexFaresValidationPolicyNoAdvancePurchase());
  chancelor.setPolicy(MINIMUM_STAY_RULE, new FlexFaresValidationPolicyNoMinMax());
  chancelor.setPolicy(MAXIMUM_STAY_RULE, new FlexFaresValidationPolicyNoMinMax());
  chancelor.setPolicy(PENALTIES_RULE, new FlexFaresValidationPolicyNoPenalties());
  CategoryValidationObserver resultObserver(chancelor.getMutableMonitor());

  for (PaxTypeFare* ptf : fareMarket.allPaxTypeFare())
  {
    checkFlexFareAccountCodes(trx, &itin, ptf, chancelor);
  }
}

void
FareValidatorOrchestrator::checkFlexFareAccountCodes(PricingTrx& trx,
                                                     Itin* itin,
                                                     PaxTypeFare* ptFare,
                                                     RuleValidationChancelor& chancelor)
{
  if (!ptFare->isFareByRule()) // cat 25
    return;

  const FBRPaxTypeFareRuleData* fbrData = ptFare->getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (fbrData && !fbrData->fbrApp()->accountCode().empty())
  {
    const std::string ptfAccountCode = fbrData->fbrApp()->accountCode().c_str();
    const flexFares::GroupsData ffgData = trx.getRequest()->getFlexFaresGroupsData();
    if (0 == ffgData.getSize())
      return;

    FareMarketDataAccess fareMarketDataAccess(trx, itin, *ptFare);
    chancelor.updateContext(fareMarketDataAccess);
    auto notifyIfHasAccountCode = [&, ptFare](const auto& idsSet, const bool isAccountCode)
    {
      if (idsSet.find(ptfAccountCode) != idsSet.end())
      {
        chancelor.getMutableMonitor().notify(RuleValidationMonitor::VALIDATION_RESULT,
                                             chancelor.getContext(),
                                             ptfAccountCode,
                                             ptFare->getMutableFlexFaresValidationStatus(),
                                             isAccountCode);
      }
    };
    for (auto& ffg : ffgData)
    {
      const uint16_t groupId = ffg.first;
      notifyIfHasAccountCode(ffgData.getAccCodes(groupId), true);
      notifyIfHasAccountCode(ffgData.getCorpIds(groupId), false);
    }
  }
}

void
FareValidatorOrchestrator::processExactlyMatchRetailerCode(FareDisplayTrx& trx)
{
  if (fallback::fallbackFRRProcessingRetailerCode(&trx))
    return;

  if (!trx.getRequest()->prmValue())
    return;

  for (auto const itin : trx.itin())
    itin->invalidateAllPaxTypeFaresForRetailer(trx);
}


bool
FareValidatorOrchestrator::processFRRAdjustedSellingLevel(PricingTrx& trx,
                                                          const bool& isFareDisplay)
{
  if (UNLIKELY(trx.excTrxType() != PricingTrx::NOT_EXC_TRX))
    return false;

  if (trx.getOptions()->isXRSForFRRule())
    return false;

  if (isFareDisplay)
  {
    FareDisplayTrx* fdTrx = static_cast<FareDisplayTrx*>(&trx);

    if (fdTrx->isShortRD() || fdTrx->isFT())
      return false;
  }

  AdjustedSellingLevelFareCollector aslfc(trx);
  aslfc.process();

  return true;
}
} // tse
