//-------------------------------------------------------------------
//  Description: Fare Collector Orchestrator
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

#include "Fares/FareCollectorOrchestrator.h"

#include "AddonConstruction/AddonConstructionOrchestrator.h"
#include "AddonConstruction/ConstructedCacheManager.h"
#include "BrandedFares/BrandedFaresSelector.h"
#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/DiagMonitor.h"
#include "Common/ErrorResponseException.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/RtwUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/SmallBitSet.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TSEAlgorithms.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "Common/TseSrvStats.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FlexFares/GroupsData.h"
#include "DataModel/FlexFares/GroupsDataAnalyzer.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeBucket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/NegFareRest.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/Diag220Collector.h"
#include "Diagnostic/Diag223Collector.h"
#include "Diagnostic/Diag451Collector.h"
#include "Diagnostic/Diag901Collector.h"
#include "Diagnostic/Diag902Collector.h"
#include "Diagnostic/Diag903Collector.h"
#include "Diagnostic/Diag981Collector.h"
#include "Diagnostic/DiagManager.h"
#include "DSS/FlightCountMgr.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Fares/AddonFareController.h"
#include "Fares/AdjustedSellingLevelFareCollector.h"
#include "Fares/BooleanFlagResetter.h"
#include "Fares/BrandedFaresParityValidator.h"
#include "Fares/BrandedFareValidator.h"
#include "Fares/CarrierFareController.h"
#include "Fares/CurrencySelectionValidator.h"
#include "Fares/DiscountedFareController.h"
#include "Fares/DummyFareCreator.h"
#include "Fares/FailedFare.h"
#include "Fares/FareByRuleController.h"
#include "Fares/FareByRuleRevalidator.h"
#include "Fares/FareCollectorOrchestratorESV.h"
#include "Fares/FareController.h"
#include "Fares/FareCurrencySelection.h"
#include "Fares/FareTypeMatcher.h"
#include "Fares/FareUtil.h"
#include "Fares/FDCarrierFareController.h"
#include "Fares/FDFareCurrencySelection.h"
#include "Fares/FDIndustryFareController.h"
#include "Fares/FlightTracker.h"
#include "Fares/IndustryFareController.h"
#include "Fares/NegotiatedFareController.h"
#include "Fares/SalesRestrictionByNation.h"
#include "Fares/ScheduleCountTask.h"
#include "ItinAnalyzer/BrandedFaresDataRetriever.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "Routing/TravelRouteBuilder.h"
#include "Rules/FareFocusRuleValidator.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <vector>

#include <boost/bind.hpp>

namespace tse
{
FALLBACK_DECL(fallbackSpanishDiscountTicketDesignatorRefactoring);
FALLBACK_DECL(reworkTrxAborter);
FALLBACK_DECL(fixSpanishLargeFamilyForSRFE);

static LoadableModuleRegister<Service, FareCollectorOrchestrator>
_("libFaresCollector.so");

bool FareCollectorOrchestrator::_indicator = false;

const FareType FareCollectorOrchestrator::DUMMY_FARE_TYPE = "EIP";
std::map<PaxTypeFare*, PaxTypeFare*> FareCollectorOrchestrator::accDiag981ptFaresMap;

static Logger
logger("atseintl.Fares.FareCollectorOrchestrator");

namespace
{
ConfigurableValue<bool>
rtngFco("FARESC_SVC", "RTNG_FCO");
ConfigurableValue<bool>
preloadFare("FARESC_SVC", "PRELOAD_FARES", false);
ConfigurableValue<bool>
skipFmAddonConstructionWhereAllSopsAreOnlineSol(
    "SHOPPING_DIVERSITY", "SKIP_FM_ADDON_CONSTRUCTION_WHERE_ALL_SOPS_ARE_ONLINE", false);
ConfigurableValue<bool>
skipFareByRuleConstructionSolFM("SHOPPING_DIVERSITY",
                                "SKIP_SOL_FM_FARE_BY_RULE_CONSTRUCTION",
                                false);
ConfigurableValue<uint32_t>
numFaresKeepForRoutingValidation("SHOPPING_OPT", "NUM_FARE_KEEP_FOR_ROUTING_VALIDATION", 0);

bool
validateCarrier(PaxTypeFare* paxTypeFare, const Itin& itin, const PricingTrx& trx)
{
  if (UNLIKELY(!paxTypeFare->isNegotiated()))
    return true;

  if (UNLIKELY(trx.isValidatingCxrGsaApplicable() &&
               !paxTypeFare->fareMarket()->validatingCarriers().empty()))
  {
    return FareUtil::isNegFareCarrierValid(paxTypeFare->negotiatedInfo().carrier(),
                                           paxTypeFare->negotiatedInfo().tktAppl(),
                                           paxTypeFare,
                                           false);
  }
  else
  {
    return FareUtil::isNegFareCarrierValid(paxTypeFare->negotiatedInfo().carrier(),
                                           paxTypeFare->negotiatedInfo().tktAppl(),
                                           itin,
                                           false);
  }
}
}

FareCollectorOrchestrator::FareCollectorOrchestrator(const std::string& name, TseServer& server)
  : FareOrchestrator(name, server, TseThreadingConst::FARESC_TASK)
{
  _indicator = rtngFco.getValue();
  _preloadFares = preloadFare.getValue();
  _skipFmAddonConstructionWhereAllSopsAreOnlineSol =
      skipFmAddonConstructionWhereAllSopsAreOnlineSol.getValue();
  _skipFareByRuleConstructionSolFM = skipFareByRuleConstructionSolFM.getValue();
  AddonConstructionOrchestrator::classInit();
}

FareCollectorOrchestrator::~FareCollectorOrchestrator()
{
  // Flush the constructed fares cache, not doing this
  // causes shutdown problems
  ConstructedCacheManager::instance().flushAll();
}

bool
FareCollectorOrchestrator::process(MetricsTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entered FareCollectorOrchestrator::process(MetricsTrx)");

  std::ostringstream& oss = trx.response();

  MetricsUtil::header(oss, "FCO Metrics");
  MetricsUtil::lineItemHeader(oss);

  MetricsUtil::lineItem(oss, MetricsUtil::FCO_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_GOV_CXR);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FARE_CS);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_GLB_DIR);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FLT_TRACKER);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FIND_FARES);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_DIAG);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_CFC_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_AFC_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FBRC_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_DFC_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_IFC_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FARE_SORT);

  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FC_TVLDATE);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FC_TXREF);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FC_FCA);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FC_FCASEG);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FC_FTMATRIX);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FC_MATCHLOC);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FC_CREATEFARES);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FC_CREATEPTFARES);
  MetricsUtil::lineItem(oss, MetricsUtil::FCO_FC_CURRENCY);

  MetricsUtil::lineItem(oss, MetricsUtil::RC_VALIDATE_PAXFARETYPE);

  LOG4CXX_DEBUG(logger, "Leaving FareCollectorOrchestrator::process(MetricsTrx)");
  return true;
}

bool
FareCollectorOrchestrator::process(RepricingTrx& trx)
{
  try
  {
    LOG4CXX_DEBUG(logger, "Entered FareCollectorOrchestrator::process(RepricingTrx)");

    process((PricingTrx&)trx);
  }
  catch (tse::ErrorResponseException& e)
  {
    // We need to ignore this, if a FareMarket doesnt have fares it will throw this
    // but we dont want it passed all the way back to the client.
  }

  return true;
}

namespace
{
//----------------------------------------------------------------------------
// Binary function for determining the ordering between two Itin objects
// based on travelseg size. The smaller size will be on the top.
//----------------------------------------------------------------------------
class SortByTravelSegSize : public std::binary_function<Itin*, Itin*, bool>
{
public:
  bool operator()(const Itin* x, const Itin* y)
  {
    return (x->travelSeg().size() < y->travelSeg().size());
  }
};
}

// moved to ItinAnalyzerService.cpp
bool
FareCollectorOrchestrator::process(PricingTrx& trx)
{
  TSELatencyData tld(trx, "FCO PROCESS PRICINGTRX");
  LOG4CXX_DEBUG(logger, "Entered FareCollectorOrchestrator::process(PricingTrx)");
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    std::sort(trx.itin().begin(), trx.itin().end(), SortByTravelSegSize());

    removeCat5(trx);
  }

  if (UNLIKELY(trx.isFlexFare()))
  {
    flexFares::GroupsDataAnalyzer::analyzeGroupsAndPutResultsTo(
        trx.getRequest()->getFlexFaresGroupsData(), trx.getMutableFlexFaresTotalAttrs());
  }

  // Find duplicate FareMarkets
  std::map<std::string, std::vector<FareMarket*>> fmMap;
  std::map<FareMarket*, std::vector<int>> fmOrigBrands;
  std::map<FareMarket*, std::vector<CarrierCode>> fmOrigVCs;
  collectFaresStep(trx, fmMap, &fmOrigVCs, &fmOrigBrands);

  setProcessedStepForDummy(trx, true);
  releaseCheckSortMarkStep(trx, fmMap, &fmOrigVCs, &fmOrigBrands);
  setProcessedStepForDummy(trx, false);

  fillDummyFM(trx);
  releaseCheckSortMarkStepForDummy(trx);

  countFares(trx);

  LOG4CXX_DEBUG(logger, "Leaving FareCollectorOrchestrator::process(PricingTrx)");
  return true;
}

void
FareCollectorOrchestrator::fillDummyFM(PricingTrx& trx)
{
  invoke_foreach_valid_fareMarket(trx, fillDummyFareMarket, FareMarket::FareCollector);
}

bool
FareCollectorOrchestrator::removeCat5(PricingTrx& trx)
{
  if (!trx.isAltDates())
  {
    return false;
  }
  if (trx.outboundDepartureDate() != DateTime::emptyDate())
  {
    return trx.getOptions()->newIndicatorToRemoveCat5();
  }

  std::vector<Itin*>::const_iterator itinI = trx.itin().begin();
  std::vector<Itin*>::const_iterator itinIEnd = trx.itin().end();

  std::string departureDT = "";
  std::string currentDepartureDT = "";
  for (; itinI != itinIEnd; ++itinI)
  {
    Itin& itin = **itinI;
    if (itin.travelSeg().empty())
    {
      continue;
    }

    TravelSeg* firstSeg = itin.travelSeg().front();
    AirSeg* airSeg = firstSeg->toAirSeg();
    if (airSeg == nullptr)
    {
      continue;
    }
    if (firstSeg->departureDT() == DateTime::emptyDate())
    {
      continue;
    }
    currentDepartureDT = firstSeg->departureDT().dateToString(DDMMM, "");
    if ((!departureDT.empty()) && (departureDT != currentDepartureDT))
    {
      trx.getOptions()->newIndicatorToRemoveCat5() = true;
      break;
    }
    departureDT = currentDepartureDT;
  }
  return trx.getOptions()->newIndicatorToRemoveCat5();
}

TrxItinFareMarketFunctor
FareCollectorOrchestrator::allFareMarketStepsFunctor()
{
  return boost::bind(&FareCollectorOrchestrator::allFareMarketSteps, this, _1, _2, _3);
}

TrxItinFareMarketFunctor
FareCollectorOrchestrator::checkSpanishDiscountForISFunctor()
{
  return [](PricingTrx& trx, Itin&, FareMarket& fareMarket)
         {
           return SLFUtil::checkSpanishDiscountForIS(trx, fareMarket);
         };
}

void
FareCollectorOrchestrator::collectFaresStep(
    PricingTrx& trx,
    std::map<std::string, std::vector<FareMarket*>>& fmMap,
    std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs,
    std::map<FareMarket*, std::vector<int>>* fmOrigBrands)
{
  TSELatencyData tld(trx, "FCO PROCESS COLLECTSTEP");
  LOG4CXX_DEBUG(logger, "Entered FareCollectorOrchestrator::collectFaresStep(PricingTrx)");

  if (UNLIKELY(trx.getOptions()->isFareFamilyType()))
  {
    if (!validateFareTypePricing(trx))
      throw tse::ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                        "FARE TYPE QUALIFIER NOT FOUND");

    FareTypeMatcher ftMatcher(trx);
    PaxTypeUtil::createFareTypePricingPaxTypes(trx, ftMatcher.psgTypes());
  }

  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  if (UNLIKELY(diagType == AllFareDiagnostic || diagType == Diagnostic208 ||
               diagType == Diagnostic220 || diagType == Diagnostic225 ||
               diagType == Diagnostic325 || diagType == Diagnostic335 ||
               diagType == Diagnostic500 || diagType == Diagnostic550))
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diagPtr = factory->create(trx);
    DiagCollector& diag = *diagPtr;

    diag.enable(diagType);
    diag.printHeader();

    if (diagType == Diagnostic220)
    {
      Diag220Collector& diag220 = dynamic_cast<Diag220Collector&>(*diagPtr);
      diag220.displayPaxTypes(trx);
    }

    diag.flushMsg();
  }

  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  if (UNLIKELY(noPNRTrx != nullptr && !isGIValid(*noPNRTrx)))
  {
    return;
  }

  {
    TSELatencyData tld(trx, "FCO SETUP FARE MARKET");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, setupFareMarket, FareMarket::FareCollector);
  }

  if (UNLIKELY(_preloadFares))
  {
    TSELatencyData tld(trx, "FCO PRELOAD FARES");
    preloadFares(trx);
  }

  ErrorResponseException::ErrorResponseCode newStatus = ErrorResponseException::UNKNOWN_EXCEPTION;

  const bool isMIP = (trx.itin().size() > 1 || !trx.itin().front()->getSimilarItins().empty());

  if (isMIP)
  {
    if (UNLIKELY(trx.getTrxType() == PricingTrx::MIP_TRX &&
                 trx.excTrxType() == PricingTrx::AR_EXC_TRX))
    {
      RexExchangeTrx* rexExcTrx = dynamic_cast<RexExchangeTrx*>(&trx);
      if (rexExcTrx)
      {
        TSELatencyData tld(trx, "FCO SPLIT SHARED FM WITH DIFF ROE");
        if (fallback::reworkTrxAborter(&trx))
          checkTrxAborted(trx);
        else
          trx.checkTrxAborted();
        splitSharedFareMarketsWithDiffROE(*rexExcTrx);
      }
    }

    TSELatencyData tld(trx, "FCO FIND DUP FM");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    findDuplicateFareMarkets(trx, fmMap, fmOrigVCs, fmOrigBrands, &newStatus);
  }

  if (fallback::reworkTrxAborter(&trx))
    checkTrxAborted(trx);
  else
    trx.checkTrxAborted();

  {
    TSELatencyData tld(trx, "FCO ALL FM STEPS");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    exec_foreach_valid_not_dummy_fareMarket(
        _taskId, trx, allFareMarketStepsFunctor(), FareMarket::FareCollector);
  }

  if (fallback::reworkTrxAborter(&trx))
    checkTrxAborted(trx);
  else
    trx.checkTrxAborted();

  LOG4CXX_DEBUG(logger, " - Leaving FareCollectorOrchestrator::collectFaresStep(PricingTrx)");
}

void
FareCollectorOrchestrator::fillDummyFareMarket(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (!fareMarket.useDummyFare())
    return;

  TSELatencyData metrics(trx, "FCO AFM DUMMYFARE");

  if (trx.getRequest()->originBasedRTPricing())
    DummyFareCreator::createFaresForOriginBasedRT(trx, itin, fareMarket);
  else
    DummyFareCreator::createFaresForExchange(trx, itin, fareMarket);
}

void
FareCollectorOrchestrator::releaseCheckSortMarkStepForDummy(PricingTrx& trx)
{
  TSELatencyData tld(trx, "FCO PROCESS RELEASECHECKSORTMARKSTEPFORDUMMY");
  LOG4CXX_DEBUG(logger,
                "Entered FareCollectorOrchestrator::releaseCheckSortMarkStepForDummy(PricingTrx)");

  {
    TSELatencyData tld(trx, "FCO CHECK FARES NO SORT");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, InvokeStep, FareMarket::FareCollector);
  }

  {
    TSELatencyData tld(trx, "FCO SORT FARES");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, sortStep, FareMarket::FareCollector);
  }

  if (fallback::reworkTrxAborter(&trx))
    checkTrxAborted(trx);
  else
    trx.checkTrxAborted();

  {
    TSELatencyData tld(trx, "FCO MARK FM");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, markProcessedStep, FareMarket::FareCollector);
  }

  LOG4CXX_DEBUG(
      logger, " - Leaving FareCollectorOrchestrator::releaseCheckSortMarkStepForDummy(PricingTrx)");
}

void
FareCollectorOrchestrator::releaseCheckSortMarkStep(
    PricingTrx& trx,
    std::map<std::string, std::vector<FareMarket*>>& fmMap,
    const std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs,
    const std::map<FareMarket*, std::vector<int>>* fmOrigBrands)
{
  TSELatencyData tld(trx, "FCO PROCESS RELEASECHECKSORTMARKSTEP");
  LOG4CXX_DEBUG(logger, "Entered FareCollectorOrchestrator::releaseCheckSortMarkStep(PricingTrx)");

  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  {
    TSELatencyData tld(trx, "FCO CHECK FARES NO SORT");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, InvokeStep, FareMarket::FareCollector);
  }

  const bool isMIP = (trx.itin().size() > 1) || !trx.itin().front()->getSimilarItins().empty();
  // Dont do this when we're calling a DDALLFARES, it removes some of the invalid fares
  // and makes it hard to diagnose why

  if (LIKELY((trx.excTrxType() != PricingTrx::AR_EXC_TRX &&
              trx.excTrxType() != PricingTrx::AF_EXC_TRX) ||
             !static_cast<RexBaseTrx*>(&trx)->isAnalyzingExcItin()))
  {
    if (LIKELY((diagType == DiagnosticNone) ||
               ((trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "ALLFARES") &&
                (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "ITININFO"))))
    {
      TSELatencyData tld(trx, "FCO RELEASE FARES");
      if (fallback::reworkTrxAborter(&trx))
        checkTrxAborted(trx);
      else
        trx.checkTrxAborted();
      invoke_foreach_valid_fareMarket(trx, releaseFaresStep, FareMarket::FareCollector);
    }
  }

  if (trx.getRequest()->brandedFareEntry())
  {
    TSELatencyData tld(trx, "FCO CHECK BRANDED DATA");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, checkBrandedFareDataStep, FareMarket::FareCollector);
  }

  processInterlineBrandedFares(trx);

  {
    TSELatencyData tld(trx, "FCO CHECK FARES");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, checkFaresStep, FareMarket::FareCollector);
  }

  if (isMIP)
  {
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    validateCommonFareMarkets(trx, fmMap);
  }

  {
    TSELatencyData tld(trx, "FCO SORT FARES");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, sortStep, FareMarket::FareCollector);
  }

  if (fallback::reworkTrxAborter(&trx))
    checkTrxAborted(trx);
  else
    trx.checkTrxAborted();

  bool isDiagRangeValid =
      ((diagType == DiagnosticNone) || ((diagType >= Diagnostic600) && (diagType < Diagnostic900)));

  if (isDiagRangeValid && !checkIfFailCodeExist(trx, ErrorResponseException::NO_ERROR))
  {
    LOG4CXX_ERROR(
        logger,
        "FareCollectorOrchestrator::releaseCheckSortMarkStep - No valid Fare Markets Found");

    if (checkIfFailCodeExist(trx, ErrorResponseException::PRICING_REST_BY_GOV))
    {
      throw ErrorResponseException(ErrorResponseException::PRICING_REST_BY_GOV,
                                   trx.status().c_str());
    }

    if (trx.getOptions()->isPublishedFares())
    {
      throw ErrorResponseException(ErrorResponseException::NO_PUBLIC_FARES_VALID_FOR_PASSENGER);
    }

    if (trx.getOptions()->isPrivateFares())
    {
      throw ErrorResponseException(ErrorResponseException::NO_PRIVATE_FARES_VALID_FOR_PASSENGER);
    }

    if (checkIfFailCodeExist(trx, ErrorResponseException::MISSING_NUC_RATE))
    {
      const Itin* itin = (trx.getOptions()->isCarnivalSumOfLocal()) ? nullptr : trx.itin().front();

      if (itin)
      {
        throw tse::ErrorResponseException(ErrorResponseException::MISSING_NUC_RATE,
                                          itin->errResponseMsg().c_str());
      }
      else
      {
        throw tse::ErrorResponseException(ErrorResponseException::MISSING_NUC_RATE);
      }
    }

    throw tse::ErrorResponseException(ErrorResponseException::NO_FARE_FOR_CLASS_USED);
  }

  if (trx.getOptions()->isWeb() && trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, removeDuplicateFares, FareMarket::FareCollector);
  }

  if (isMIP)
  {
    TSELatencyData tld(trx, "FCO COPY FM");

    bool& tls = trx.dataHandle().useTLS() = false;
    BooleanFlagResetter bfr(tls);

    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    copyDuplicateFareMarkets(trx, fmMap, fmOrigVCs, fmOrigBrands);
  }

  {
    TSELatencyData tld(trx, "FCO MARK FM");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_fareMarket(trx, markProcessedStep);
  }

  LOG4CXX_DEBUG(logger,
                " - Leaving FareCollectorOrchestrator::releaseCheckSortMarkStep(PricingTrx)");
}

void
FareCollectorOrchestrator::processInterlineBrandedFares(PricingTrx& trx)
{
  if (!trx.getRequest()->isBrandedFaresRequest() && !trx.isBrandsForTnShopping() &&
      !trx.activationFlags().isSearchForBrandsPricing())
    return;

  if (trx.isExchangeTrx())
  {
    if (trx.getTrxType() == PricingTrx::MIP_TRX)
    {
      RexExchangeTrx* rexExchange = static_cast<RexExchangeTrx*>(&trx);
      if (rexExchange->trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE &&
          !rexExchange->getKeepOriginal())
      {
        return;
      }
    }
    else
    {
      RexBaseTrx* rexTrx = static_cast<RexBaseTrx*>(&trx);

      if (rexTrx->trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE)
      {
        return;
      }
    }
  }

  fillFaresWithBrands(trx);

  if (trx.getRequest()->isCatchAllBucketRequest() || trx.isBrandsForTnShopping() ||
      trx.activationFlags().isSearchForBrandsPricing())
    return;

  for (FareMarket* fm : trx.fareMarket())
  {
    for (PaxTypeFare* fare : fm->allPaxTypeFare())
    {
      bool isValid = false;
      for (const PaxTypeFare::BrandStatusWithDirection& brandStatus : fare->getBrandStatusVec())
      {
        if (brandStatus.first != PaxTypeFare::BS_FAIL)
        {
          isValid = true;
          break;
        }
      }

      if (!isValid)
      {
        fare->setIsValidForBranding(false);
      }
    }
  }
}

void
FareCollectorOrchestrator::preloadFares(PricingTrx& trx)
{
  typedef std::pair<LocCode, LocCode> CityPair;
  std::map<CityPair, std::vector<CarrierCode>> markets;
  for (std::vector<FareMarket*>::const_iterator i = trx.fareMarket().begin();
       i != trx.fareMarket().end();
       ++i)
  {
    const FareMarket& fm = **i;
    if (fm.failCode() != ErrorResponseException::NO_ERROR)
    {
      continue;
    }

    const LocCode& origin = fm.origin()->loc();
    const LocCode& destination = fm.destination()->loc();

    const LocCode& boardMultiCity = fm.boardMultiCity();
    const LocCode& offMultiCity = fm.offMultiCity();

    CityPair key(origin, destination);

    if (key.first > key.second)
    {
      std::swap(key.first, key.second);
    }

    markets[key].push_back(fm.governingCarrier());

    if (boardMultiCity != origin)
    {
      key.first = boardMultiCity;
      key.second = destination;
      if (key.first > key.second)
      {
        std::swap(key.first, key.second);
      }

      markets[key].push_back(fm.governingCarrier());
    }

    if (offMultiCity != destination)
    {
      key.first = origin;
      key.second = offMultiCity;
      if (key.first > key.second)
      {
        std::swap(key.first, key.second);
      }

      markets[key].push_back(fm.governingCarrier());
    }

    if (boardMultiCity != origin && offMultiCity != destination)
    {
      key.first = boardMultiCity;
      key.second = offMultiCity;
      if (key.first > key.second)
      {
        std::swap(key.first, key.second);
      }

      markets[key].push_back(fm.governingCarrier());
    }
  }

  for (auto& market : markets)
  {
    std::vector<CarrierCode>& cxr = market.second;

    std::sort(cxr.begin(), cxr.end());
    cxr.erase(std::unique(cxr.begin(), cxr.end()), cxr.end());

    trx.dataHandle().loadFaresForMarket(market.first.first, market.first.second, cxr);
  }
}

void
FareCollectorOrchestrator::initAltDates(const ShoppingTrx& trx, FareMarket& fm)
{
  for (std::vector<PaxTypeFare*>::iterator i = fm.allPaxTypeFare().begin();
       i != fm.allPaxTypeFare().end();
       ++i)
  {
    if (UNLIKELY(trx.getTrxType() == PricingTrx::FF_TRX))
    {
      if ((fm.direction() == FMDirection::OUTBOUND && (*i)->directionality() == TO) ||
          (fm.direction() == FMDirection::INBOUND && (*i)->directionality() == FROM))
        continue;
    }

    (*i)->initAltDates(trx);
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void
prepareFareMarketsInJourneyItin(ShoppingTrx& trx, std::vector<ShoppingTrx::Leg>& legs)
{
  bool processOtherMarkets =
      trx.isSumOfLocalsProcessingEnabled() || trx.isIataFareSelectionApplicable();

  uint32_t legIndex = 0;
  for (const ShoppingTrx::Leg& curLeg : legs)
  {
    // For each carrier in carrier index
    for (const ItinIndex::ItinMatrix::value_type& carrierEntry : curLeg.carrierIndex().root())
    {
      if (fallback::reworkTrxAborter(&trx))
        checkTrxAborted(trx);
      else
        trx.checkTrxAborted();

      ItinIndex::ItinCell* curItinCell = ShoppingUtil::retrieveDirectItin(
          trx, legIndex, carrierEntry.first, ItinIndex::CHECK_NOTHING);
      if (UNLIKELY(!curItinCell))
        continue;

      Itin* curItin = curItinCell->second;
      if (UNLIKELY(!curItin))
        continue;

      for (FareMarket* fareMarket : curItin->fareMarket())
      {
        if (LIKELY(fareMarket))
        {
          // Set the global direction to ZZ
          fareMarket->setGlobalDirection(GlobalDirection::ZZ);

          if (UNLIKELY(trx.isIataFareSelectionApplicable()))
          {
            if (std::find(trx.journeyItin()->fareMarket().begin(),
                          trx.journeyItin()->fareMarket().end(),
                          fareMarket) == trx.journeyItin()->fareMarket().end())
              trx.journeyItin()->fareMarket().push_back(fareMarket);
          }
          else
          {
            // old code
            trx.journeyItin()->fareMarket().push_back(fareMarket);
          }
        }

        if (!processOtherMarkets)
          break;
      }
    }

    legIndex++;
  }
}

//----------------------------------------------------------------------------
// process(ShoppingTrx)
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::process(ShoppingTrx& trx)
{
  if (UNLIKELY(PricingTrx::ESV_TRX == trx.getTrxType()))
  {
    FareCollectorOrchestratorESV fcoESV(trx, _server, _taskId);

    bool bResult(fcoESV.process());

    countFares(trx);

    return bResult;
  }

  LOG4CXX_DEBUG(logger, "Entered FareCollectorOrchestrator::process(ShoppingTrx)");

  PricingTrx& pricingTrx = dynamic_cast<PricingTrx&>(trx);

  Diagnostic& trxDiag = pricingTrx.diagnostic();

  bool resetDiag = (!trxDiag.isActive() && (trxDiag.diagnosticType() != DiagnosticNone));

  if (UNLIKELY(resetDiag))
  {
    trxDiag.deActivate();
  }

  LOG4CXX_DEBUG(logger,
                "Pax type fare bitmap bit size = " << sizeof(PaxTypeFare::FlightBit) << " bytes.");

  // Legs vector
  std::vector<ShoppingTrx::Leg> sLV = trx.legs();

  // Check if the legs vector is empty
  if (UNLIKELY(sLV.empty()))
  {
    return (false);
  }

  prepareFareMarketsInJourneyItin(trx, sLV);

  Itin& journeyItin = *(trx.journeyItin());

  {
    TSELatencyData tld(trx, "FCO SETUP FAREMARKET");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, journeyItin, setupFareMarket);
  }

  if (_preloadFares)
  {
    TSELatencyData tld(trx, "FCO PRELOAD FARES");
    preloadFares(trx);
  }

  {
    TSELatencyData tld(trx, "FCO ALL FM STEPS");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    exec_foreach_valid_fareMarket(_taskId, trx, journeyItin, allFareMarketStepsFunctor());
  }

  if (UNLIKELY((trxDiag.diagnosticType() == Diagnostic903) &&
               (trxDiag.diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "VCTRINFO")))
  {
    DCFactory* factory = DCFactory::instance();
    Diag903Collector* diagPtr = dynamic_cast<Diag903Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic903);
    diagPtr->parseQualifiers(trx);

    // Check for no legs
    if (trx.legs().empty())
    {
      (*diagPtr) << "NO SCHEDULES TO GENERATE REPORT" << std::endl;
    }
    else
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }

  {
    TSELatencyData tld(trx, "FCO RELEASE FARES");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, journeyItin, releaseFaresStep);
  }

  if (trx.getRequest()->isBrandedFaresRequest())
  {
    fillFaresWithBrands(trx);
    if (trx.getRequest()->isParityBrandsPath())
    {
      BrandedFaresParityValidator bfParityValidator(trx);
      bfParityValidator.process();
    }
    else if (trx.getRequest()->isCheapestWithLegParityPath())
    {
      invoke_foreach_valid_fareMarket(trx, invalidateFaresWithoutBrands);
    }
  }

  {
    TSELatencyData tld(trx, "FCO SORT FARES");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, journeyItin, sortStep);
  }

  if (trx.getOptions()->isWeb() && trx.getTrxType() == PricingTrx::IS_TRX)
  {
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, removeDuplicateFares);
  }

  if ((trx.getTrxType() == PricingTrx::IS_TRX) && (!trx.paxType().empty()) &&
      (trx.paxType().front() != nullptr) && (trx.paxType().front()->paxType().equalToConst("TV1")))
  {
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    invoke_foreach_valid_fareMarket(trx, checkFareByRuleJCBFares);
  }

  if (trx.getTrxType() == PricingTrx::IS_TRX)
  {
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    exec_foreach_valid_fareMarket(_taskId, trx, journeyItin, checkSpanishDiscountForISFunctor());
  }

  if (trx.getTrxType() == PricingTrx::FF_TRX)
  {
    FlightFinderTrx& ffTrx = dynamic_cast<FlightFinderTrx&>(trx);

    if (ffTrx.reportError() && ffTrx.diagnostic().diagnosticType() == DiagnosticNone)
    {
      throw ErrorResponseException(ffTrx.reportError().errorCode,
                                   ffTrx.reportError().message.c_str());
    }
  }

  if (UNLIKELY(resetDiag))
  {
    trxDiag.activate();
  }

  countFares(trx);

  LOG4CXX_DEBUG(logger, "Leaving FareCollectorOrchestrator::process(ShoppingTrx)");
  return true;
}

void
FareCollectorOrchestrator::fillFaresWithBrands(PricingTrx& trx)
{
  TSELatencyData tld(trx, "FCO INTERLINE BRANDED FARES PROCESS");

  if (trx.getTrxType() == PricingTrx::IS_TRX)
  {
    std::vector<FareMarket*> fareMarkets =
        ShoppingUtil::getUniqueFareMarketsFromItinMatrix(static_cast<ShoppingTrx&>(trx));
    fillFaresWithBrands_impl(trx, fareMarkets);
  }
  else if (trx.isNotExchangeTrx())
  {
    fillFaresWithBrands_impl(trx, trx.fareMarket());
  }
  else
  {
    RexBaseTrx* rexTrx = static_cast<RexBaseTrx*>(&trx);

    if (rexTrx->trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE)
    {
      fillFaresWithBrands_impl(trx, rexTrx->exchangeItin().front()->fareMarket());
    }
    else
    {
      std::vector<FareMarket*> fmsForNewItins;

      for (Itin* itin : trx.itin())
      {
        for (FareMarket* fm : itin->fareMarket())
        {
          fmsForNewItins.push_back(fm);
        }
      }

      std::sort(fmsForNewItins.begin(), fmsForNewItins.end());
      std::vector<FareMarket*>::iterator last =
          std::unique(fmsForNewItins.begin(), fmsForNewItins.end());
      fmsForNewItins.erase(last, fmsForNewItins.end());

      fillFaresWithBrands_impl(trx, fmsForNewItins);
    }
  }
}

void
FareCollectorOrchestrator::fillFaresWithBrands_impl(PricingTrx& trx,
                                                    std::vector<FareMarket*>& fareMarkets)
{
  BrandedFareValidatorFactory brandSelectorFactory(trx);
  BrandedFaresSelector brandedFaresSelector(trx, brandSelectorFactory);
  for (auto fm : fareMarkets)
  {
    if ((fm != nullptr) && !(fm->allPaxTypeFare().empty()))
    {
      brandedFaresSelector.validate(*fm);

      if (isThruFareMarket(trx, *fm))
        failSoftPassedBrands(*fm); // cross cabin
    }
  }
  brandedFaresSelector.clear();
}

bool
FareCollectorOrchestrator::isThruFareMarket(PricingTrx& trx, FareMarket& fm)
{
  if (trx.getTrxType() == PricingTrx::IS_TRX)
  {
    return (fm.getFmTypeSol() == FareMarket::SOL_FM_NOTAPPLICABLE ||
            fm.getFmTypeSol() == FareMarket::SOL_FM_THRU);
  }

  return fm.isThru();
}

void
FareCollectorOrchestrator::failSoftPassedBrands(FareMarket& fareMarket)
{
  for (PaxTypeFare* fare : fareMarket.allPaxTypeFare())
  {
    for (PaxTypeFare::BrandStatusWithDirection& brandStatus : fare->getMutableBrandStatusVec())
    {
      if (brandStatus.first == PaxTypeFare::BS_SOFT_PASS)
        brandStatus.first = PaxTypeFare::BS_FAIL;
    }
  }
}

void
FareCollectorOrchestrator::countFares(PricingTrx& trx)
{
  int totalFareCount(0), fareByRuleCount(0), addonFareCount(0);
  typedef std::vector<FareMarket*> FareMarketVector;
  const FareMarketVector& fareMarkets = trx.fareMarket();
  for (const auto fm : fareMarkets)
  {
    const FareMarket& fareMarket = *fm;
    typedef std::vector<PaxTypeFare*> PaxTypeFareVector;
    const PaxTypeFareVector& allFares = fareMarket.allPaxTypeFare();

    totalFareCount += allFares.size();
    for (const auto ptFare : allFares)
    {
      if (ptFare->isFareByRule())
      {
        ++fareByRuleCount;
      }
      else if (ptFare->isConstructed())
      {
        ++addonFareCount;
      }
    }
  }
  TseSrvStats::recordTotalNumberFares(trx, totalFareCount);
  TseSrvStats::recordNumberCat25Fares(trx, fareByRuleCount);
  TseSrvStats::recordNumberAddOnFares(trx, addonFareCount);
}
//----------------------------------------------------------------------------
// process(flightFinderTrx)
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::process(FlightFinderTrx& trx)
{
  LOG4CXX_DEBUG(logger, " - Entered process(FlightFinderTrx)");

  return process(static_cast<ShoppingTrx&>(trx));
}

void
FareCollectorOrchestrator::invalidateFaresWithoutBrands(PricingTrx& trx, Itin& itin, FareMarket& fm)
{
  const std::vector<PaxTypeFare*>& allFares = fm.allPaxTypeFare();
  for (const auto ptFare : allFares)
  {
    if (!ptFare->hasValidBrands())
      ptFare->setIsValidForBranding(false);
  }
}

//---------------------------------------------------------------------------------
// Search FareByRule JCB fares, if exists then turn on jcb indicator in fare market
//---------------------------------------------------------------------------------
void
FareCollectorOrchestrator::checkFareByRuleJCBFares(PricingTrx& trx, Itin& itin, FareMarket& fm)
{
  if (fm.isJcb())
  {
    return;
  }
  const std::vector<PaxTypeFare*>& allFares = fm.allPaxTypeFare();
  for (const auto ptFare : allFares)
  {
    if (ptFare != nullptr && ptFare->paxType() != nullptr && ptFare->paxType()->paxType() == JCB)
    {
      fm.setJcb(true);
      break;
    }
  }
}


void
FareCollectorOrchestrator::removeDuplicateFares(PricingTrx& trx, Itin& itin, FareMarket& fm)
{
  std::vector<PaxTypeFare*> removedPTF;
  removedPTF.reserve(fm.allPaxTypeFare().size());

  for (auto& ptc : fm.paxTypeCortege())
  {
    std::vector<PaxTypeFare*>& ptFares = ptc.paxTypeFare();

    std::vector<PaxTypeFare*> uniquePTF;
    uniquePTF.reserve(ptFares.size());

    std::vector<PaxTypeFare*>::iterator ptfI = ptFares.begin();
    while (!ptFares.empty() && ptfI != ptFares.end())
    {
      bool nextStartWasSet = false;
      PaxTypeFare* paxTypeFare = *ptfI;
      uniquePTF.push_back(paxTypeFare);
      std::vector<PaxTypeFare*>::iterator ptfINext = ptfI + 1;
      while (ptfINext != ptFares.end())
      {
        PaxTypeFare* paxTypeFareNext = *(ptfINext);
        if (dupFare(*paxTypeFare, *paxTypeFareNext, trx))
        {
          if ((paxTypeFare->footNote1() == paxTypeFareNext->footNote1()) &&
              (paxTypeFare->footNote2() == paxTypeFareNext->footNote2()))
          {
            removedPTF.push_back(paxTypeFareNext);
            if (nextStartWasSet)
            {
              // group same (with same note) together
              PaxTypeFare* nextToSwap = *ptfINext;
              *ptfINext = *ptfI;
              *ptfI = nextToSwap;
              ++ptfI;
            }
          }
          else
          {
            if (!nextStartWasSet)
            {
              ptfI = ptfINext;
              nextStartWasSet = true;
            }
          }
          ++ptfINext;
        }
        else
        {
          if (!nextStartWasSet)
            ptfI = ptfINext;
          break;
        }
      }

      if (!nextStartWasSet && ptfINext == ptFares.end())
        break;
    }

    ptFares.swap(uniquePTF);
  }

  if (trx.paxType().size() > 1)
  {
    // for now, remove from the allPaxTypeFares for Single PaxType only
    return;
  }

  std::sort(removedPTF.begin(), removedPTF.end());
  removedPTF.erase(std::unique(removedPTF.begin(), removedPTF.end()), removedPTF.end());
  RemovedDuplicateFare removedDupFare(removedPTF);
  std::vector<PaxTypeFare*>& allPaxTypeFares = fm.allPaxTypeFare();
  try
  {
    std::vector<PaxTypeFare*>::iterator riter =
        remove_if(allPaxTypeFares.begin(), allPaxTypeFares.end(), removedDupFare);

    if (riter != allPaxTypeFares.end())
    {
      int total = allPaxTypeFares.size();
      allPaxTypeFares.erase(riter, allPaxTypeFares.end());
      int released = total - allPaxTypeFares.size();

      SUPPRESS_UNUSED_WARNING(total);
      SUPPRESS_UNUSED_WARNING(released);

      LOG4CXX_DEBUG(logger,
                    " FARE MARKET: " << fm.boardMultiCity() << "-" << fm.offMultiCity()
                                     << " - RELEASED " << released << " of " << total
                                     << " FARES\n ");
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

bool
FareCollectorOrchestrator::dupFare(const PaxTypeFare& ptf1,
                                   const PaxTypeFare& ptf2,
                                   const PricingTrx& trx)
{
  const MoneyAmount diff = std::abs(ptf2.nucFareAmount() - ptf1.nucFareAmount());
  if (diff < EPSILON && (ptf1.fareClass() == ptf2.fareClass()) &&
      (ptf1.vendor() == ptf2.vendor()) && (ptf1.tcrRuleTariff() == ptf2.tcrRuleTariff()) &&
      (ptf1.carrier() == ptf2.carrier()) && (ptf1.ruleNumber() == ptf2.ruleNumber()) &&
      (ptf1.owrt() == ptf2.owrt()) && (ptf1.currency() == ptf2.currency()) &&
      (ptf1.directionality() == ptf2.directionality()) &&
      (ptf1.routingNumber() == ptf2.routingNumber()))
  {
    if (UNLIKELY(ptf1.isDiscounted() && ptf2.isDiscounted()))
    {
      PaxTypeFareRuleData* ruleData1 = ptf1.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);
      PaxTypeFareRuleData* ruleData2 = ptf2.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);

      if (ruleData1 && ruleData2)
      {
        if (ruleData1->categoryRuleItemInfo() && ruleData2->categoryRuleItemInfo())
        {
          if (!(*(ruleData1->categoryRuleItemInfo()) == *(ruleData2->categoryRuleItemInfo())))
            return false;
        }
        else
          LOG4CXX_ERROR(logger,
                        "FareCollectorOrchestrator::dupFare - missing categoryRuleItemInfo");
      }
      else
        LOG4CXX_ERROR(logger, "FareCollectorOrchestrator::dupFare - missing PaxTypeFareRuleData");
    }
    else if (ptf1.isFareByRule() && ptf2.isFareByRule())
    {
      PaxTypeFareRuleData* ruleData1 = ptf1.paxTypeFareRuleData(RuleConst::FARE_BY_RULE);
      PaxTypeFareRuleData* ruleData2 = ptf2.paxTypeFareRuleData(RuleConst::FARE_BY_RULE);

      if (ruleData1 && ruleData2)
      {
        const FBRPaxTypeFareRuleData* fbrPtfRuleData1 = PTFRuleData::toFBRPaxTypeFare(ruleData1);
        const FBRPaxTypeFareRuleData* fbrPtfRuleData2 = PTFRuleData::toFBRPaxTypeFare(ruleData2);
        if (fbrPtfRuleData1 && fbrPtfRuleData2)
        {
          if (fbrPtfRuleData1->isR8LocationSwapped() != fbrPtfRuleData2->isR8LocationSwapped())
            return false;
        }
        else
          LOG4CXX_ERROR(logger, "FareCollectorOrchestrator::dupFare - missing fbrPtfRuleData");

        if (ruleData1->categoryRuleItemInfo() && ruleData2->categoryRuleItemInfo())
        {
          if (!(*(ruleData1->categoryRuleItemInfo()) == *(ruleData2->categoryRuleItemInfo())))
            return false;
        }
        else
          LOG4CXX_ERROR(logger,
                        "FareCollectorOrchestrator::dupFare - missing categoryRuleItemInfo");
      }
      else
        LOG4CXX_ERROR(logger, "FareCollectorOrchestrator::dupFare - missing PaxTypeFareRuleData");
    }

    if (UNLIKELY(ptf1.isNegotiated() && ptf2.isNegotiated()))
    {
      PaxTypeFareRuleData* ruleData1 = ptf1.paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE);
      PaxTypeFareRuleData* ruleData2 = ptf2.paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE);

      if (ruleData1 && ruleData2)
      {
        const NegFareRest* nData1 = dynamic_cast<const NegFareRest*>(ruleData1->ruleItemInfo());
        const NegFareRest* nData2 = dynamic_cast<const NegFareRest*>(ruleData2->ruleItemInfo());
        if (nData1 && nData2)
        {
          if (nData1->carrier() != nData2->carrier())
            return false;
        }
      }
    }

    // Addon-constructed fares are assumed to have been de-duplicated
    // so it suffices to check if the same ConstructedFareInfo object is referenced.
    if (ptf1.isConstructed() && ptf2.isConstructed())
      return ptf1.fare()->constructedFareInfo() == ptf2.fare()->constructedFareInfo();

    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
// process(FareDisplayTrx)
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::process(FareDisplayTrx& trx)
{
  TSELatencyData tld(trx, "FCO PROCESS FAREDISPLAYTRX");
  LOG4CXX_DEBUG(logger, "Entered FareCollectorOrchestrator::process(FareDisplayTrx)");
  FlightCountMgr mgr;
  ScheduleCountTask task(trx, mgr, trx.fdResponse()->scheduleCounts());
  TseRunnableExecutor dssExecutor(_taskId);
  task.trx(&trx);
  if (isDSSCallNeeded(trx))
  {
    dssExecutor.execute(task);
  }

  {
    TSELatencyData tld(trx, "FCO SETUP FARE MARKET");
    exec_foreach_valid_fareMarket(_taskId, trx, setupFareMarket);
  }

  {
    // Group steps per faremarket
    TSELatencyData tld(trx, "FCO ALL FM STEPS");
    exec_foreach_valid_fareMarket(
        _taskId,
        trx,
        boost::bind(&FareCollectorOrchestrator::allFareMarketStepsFareDisplay, this, _1, _2, _3),
        FareMarket::FareCollector);
  }

  ErrorResponseException::ErrorResponseCode invCurOverride =
      ErrorResponseException::FARE_CURRENCY_OVERRIDE_NOT_VALID;

  ErrorResponseException::ErrorResponseCode missingNUCRate =
      ErrorResponseException::MISSING_NUC_RATE;

  if (checkIfFailCodeExist(trx, invCurOverride))
  {
    LOG4CXX_ERROR(logger, "FareOrchestrator validateFailCodes() returned false");
    throw NonFatalErrorResponseException(ErrorResponseException::FARE_CURRENCY_OVERRIDE_NOT_VALID);
  }

  if (checkIfFailCodeExist(trx, missingNUCRate))
  {
    LOG4CXX_ERROR(logger, "FareOrchestrator validateFailCodes() returned false");
    Itin* itin = trx.itin().front();

    if (itin)
      throw NonFatalErrorResponseException(ErrorResponseException::MISSING_NUC_RATE,
                                           itin->errResponseMsg().c_str());
  }

  {
    TSELatencyData tld(trx, "FCO RELEASE FARES");
    invoke_foreach_valid_fareMarket(trx, releaseFaresStep);
  }

  {
    TSELatencyData tld(trx, "FCO SORT FARES");
    exec_foreach_valid_fareMarket(_taskId, trx, sortStep);
  }

  if (isDSSCallNeeded(trx))
    dssExecutor.wait();

  processFRRAdjustedSellingLevel(trx);

  LOG4CXX_DEBUG(logger, "Leaving FareCollectorOrchestrator::process(FareDisplayTrx)");
  return true;
}

//----------------------------------------------------------------------------
// processFRRAdjustedSellingLevel()
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::processFRRAdjustedSellingLevel(FareDisplayTrx& trx)
{
  if (trx.isShortRD() || trx.isFT())
  {
    if (trx.getOptions()->isXRSForFRRule())
      return false;

    AdjustedSellingLevelFareCollector aslfc(trx);
    aslfc.process();

    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
// selectCurrency
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::selectCurrency(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  LOG4CXX_INFO(logger, "Entering FareCollectorOrchestrator::selectCurrency");
  bool isAlternatePricingRequest = false;
  bool selectRC = true;
  NationCode originNation;

  GeoTravelType itinTravelType;
  if (trx.isShopping())
  {
    ShoppingTrx& shoppingTrx = static_cast<ShoppingTrx&>(trx);
    itinTravelType = shoppingTrx.journeyItin()->geoTravelType();
    originNation = ItinUtil::originNation(*(shoppingTrx.journeyItin()));
  }
  else
  {
    itinTravelType = itin.geoTravelType();
    originNation = ItinUtil::originNation(itin);
  }

  PricingOptions* options = trx.getOptions();

  if (UNLIKELY(!options->alternateCurrency().empty()))
    isAlternatePricingRequest = true;

  FareCurrencySelection fareCurrencySelection;

  if (LIKELY(!isAlternatePricingRequest || itinTravelType == GeoTravelType::Transborder))
  {
    FareDisplayTrx* fdTrx;

    if (UNLIKELY(FareDisplayUtil::getFareDisplayTrx(&trx, fdTrx)))
    {
      FDFareCurrencySelection fdFareCurrSelection;
      selectRC = fdFareCurrSelection.selectPrimeCurrency(*fdTrx, fareMarket, itin);
      return selectRC;
    }

    if ((itinTravelType == GeoTravelType::Domestic) && (originNation == CANADA))
    {
      // Everything defaults to USD so skip Domestic US
      //
      LOG4CXX_INFO(logger, "Processing Canadian Domestic Currency Selection");
      fareCurrencySelection.selectUSCACurrency(trx, originNation, fareMarket);
    }
    else if (itinTravelType == GeoTravelType::Transborder)
    {
      LOG4CXX_INFO(logger, "Processing US/Canadian Transborder Currency Selection");
      fareCurrencySelection.selectUSCACurrency(trx, originNation, fareMarket);
    }
    else if ((itinTravelType == GeoTravelType::International) ||
             (itinTravelType == GeoTravelType::ForeignDomestic))
    {
      LOG4CXX_INFO(logger, "Processing International or Foreign Domestic Currency Selection");

      bool determineOutBoundOnly = false;

      if (itin.tripCharacteristics().isSet(Itin::OneWay) && (itin.travelSeg().size() == 1))
        determineOutBoundOnly = true;

      selectRC =
          fareCurrencySelection.selectPrimeCurrency(trx, fareMarket, itin, determineOutBoundOnly);
    }
  }

  LOG4CXX_INFO(logger, "Leaving FareCollectorOrchestrator::selectCurrency");
  return selectRC;
}

//----------------------------------------------------------------------------
// checkAllApplicableSopsAreInterlineSol()
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::checkAllApplicableSopsAreOnlineInSol(const PricingTrx& trx,
                                                                const FareMarket& fareMarket) const
{
  const ShoppingTrx* shoppingTrx = dynamic_cast<const ShoppingTrx*>(&trx);
  if (LIKELY(!shoppingTrx || !shoppingTrx->isSumOfLocalsProcessingEnabled() ||
             !_skipFmAddonConstructionWhereAllSopsAreOnlineSol))
  {
    return false;
  }

  TSE_ASSERT(fareMarket.getApplicableSOPs());
  bool fareMarketHasApplicableSops = false;

  // iterate over all SopUsage::origSopId_ in FareMarket::_applicableSOPs
  for (const ApplicableSOP::value_type& cxrAndSopUsages : *fareMarket.getApplicableSOPs())
  {
    for (const SOPUsage& sopUsage : cxrAndSopUsages.second)
    {
      if (!sopUsage.applicable_)
        continue;
      else
        fareMarketHasApplicableSops = true;

      size_t sopId =
          ShoppingUtil::findInternalSopId(trx, fareMarket.legIndex(), sopUsage.origSopId_);

      if (shoppingTrx->legs()[fareMarket.legIndex()].sop()[sopId].isInterline())
        return false;
    }
  }

  TSE_ASSERT(fareMarketHasApplicableSops);
  return true;
}

//----------------------------------------------------------------------------
// findPublishedAddOnIndustryFares()
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::findPublishedAddOnIndustryFares(
    PricingTrx& trx,
    Itin& itin,
    FareMarket& fareMarket,
    CarrierFareController& carrierFareController,
    AddonFareController& addonFareController,
    IndustryFareController& indFareController,
    bool sort) const
{
  LOG4CXX_INFO(logger, "Entering FareCollectorOrchestrator::findPublishedAddOnIndustryFares()");

  // retrieve constructed fares
  // ====== =========== ===== ====

  std::vector<Fare*> cxrConstructedFares;
  std::vector<Fare*> indConstructedFares;

  bool addonFareSkipReasonFMSopsAreOnlineSol = false;

  if ((fareMarket.geoTravelType() == GeoTravelType::International) &&
      (fareMarket.origin()->nation() != fareMarket.destination()->nation() ||
       trx.getOptions()->isRtw()) &&
      (!FareDisplayUtil::shortRequestForPublished(trx)) &&
      // if (SOL is not activated or check not applicable)
      !(addonFareSkipReasonFMSopsAreOnlineSol =
            checkAllApplicableSopsAreOnlineInSol(trx, fareMarket)))
  {
    TSELatencyData metrics(trx, "FCO PAI ADDON");

    if (UNLIKELY(!addonFareController.process(cxrConstructedFares, indConstructedFares)))
    {
      LOG4CXX_INFO(logger, "AddonFareController.process() returned false");
      return false;
    }
  }
  else if (UNLIKELY(addonFareSkipReasonFMSopsAreOnlineSol))
  {
    LOG4CXX_DEBUG(logger,
                  "SOL - FM addon construction was skipped because all applicable SOPs are online");
  }

  // retrieve published fares, eliminate duplicates from constructed fares
  // and create PaxTypeFares from constructed and published fares
  // === ====== ============ ==== =========== === ========= ======

  if (fareMarket.governingCarrier() != INDUSTRY_CARRIER)
  {
    TSELatencyData metrics(trx, "FCO PAI PUB");

    if (UNLIKELY(!carrierFareController.process(cxrConstructedFares)))
    {
      LOG4CXX_INFO(logger, "CarrierFareController.process() returned false");
      return false;
    }
  }

  // Industry fares
  if (fareMarket.geoTravelType() == GeoTravelType::International)
  {
    TSELatencyData metrics(trx, "FCO PAI IND");

    if (UNLIKELY(!TrxUtil::isDisableYYForExcItin(trx) &&
                 !indFareController.process(&indConstructedFares)))
    {
      LOG4CXX_INFO(logger, "IndustryFareController.process() returned false");
      return false;
    }
  }

  if (UNLIKELY(!fareMarket.allPaxTypeFare().empty() && sort))
  {
    TSELatencyData metrics(trx, "FCO PAI SORT");
    carrierFareController.sortPaxTypeFares();
  }

  return true;
}

//----------------------------------------------------------------------------
// findDiscountedFares()
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::findDiscountedFares(PricingTrx& trx,
                                               Itin& itin,
                                               FareMarket& fareMarket,
                                               bool sort /* = false */)
{
  LOG4CXX_INFO(logger, "Entering FareCollectorOrchestrator::findDiscountedFares()");

  // create Discounted fares
  // ====== ========== =====

  {
    LOG4CXX_DEBUG(logger, "instantiating DiscountedFareController");
    DiscountedFareController discFareController(trx, itin, fareMarket);
    if (trx.getTrxType() == PricingTrx::MIP_TRX && fareMarket.hasDuplicates())
      discFareController.setPhase(RuleItem::FCOPhase);
    {
      //       TSELatencyData metrics( trx, "FCO DISCFARECNTRLR" );

      if (UNLIKELY(!discFareController.process()))
      {
        LOG4CXX_INFO(logger, "DiscountedFareCountroller.process() returned false");
        return false;
      }
    }

    if (UNLIKELY(!fareMarket.allPaxTypeFare().empty() && sort))
    {
      //       TSELatencyData metrics( trx, "FCO FARE SORT");
      discFareController.sortPaxTypeFares();
    }
  }

  return true;
}

//----------------------------------------------------------------------------
// findFareByRuleFares()
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::findFareByRuleFares(PricingTrx& trx,
                                               Itin& itin,
                                               FareMarket& fareMarket,
                                               bool sort) const
{
  LOG4CXX_INFO(logger, "Entering FareCollectorOrchestrator::findFareByRuleFares()");

  // create Fare By Rule fares
  // ====== ==== == ==== =====

  LOG4CXX_DEBUG(logger, "instantiating FareByRuleController");

  FareByRuleController fareByRuleController(trx, this, itin, fareMarket);
  if (trx.getTrxType() == PricingTrx::MIP_TRX && fareMarket.hasDuplicates())
    fareByRuleController.setPhase(RuleItem::FCOPhase);

  fareByRuleController.initFareMarketCurrency();

  if (UNLIKELY(!fareByRuleController.process()))
  {
    LOG4CXX_INFO(logger, "FareByRuleController.process() returned false");
    return false;
  }

  if (UNLIKELY(!fareMarket.allPaxTypeFare().empty() && sort))
  {
    fareByRuleController.sortPaxTypeFares();
  }

  return true;
}

//----------------------------------------------------------------------------

// findNegotiatedFares()
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::findNegotiatedFares(PricingTrx& trx,
                                               Itin& itin,
                                               FareMarket& fareMarket,
                                               bool sort)
{
  LOG4CXX_INFO(logger, "Entering FareCollectorOrchestrator::findNegotiatedFares()");

  // create Negotiated fares
  // ====== ========== =====

  LOG4CXX_DEBUG(logger, "instantiating NegotiatedFareController");

  //  TSELatencyData metrics( trx, "FCO NEGFARECNTRLR");
  NegotiatedFareController negotiatedFareController(trx, itin, fareMarket);
  if (trx.getTrxType() == PricingTrx::MIP_TRX && fareMarket.hasDuplicates())
    negotiatedFareController.setPhase(RuleItem::FCOPhase);

  if (UNLIKELY(!negotiatedFareController.process()))
  {
    LOG4CXX_INFO(logger, "NegotiatedFareController.process() returned false");
    return false;
  }

  if (UNLIKELY(!fareMarket.allPaxTypeFare().empty() && sort))
  {
    //      TSELatencyData metrics( trx, "FCO FARE SORT");
    negotiatedFareController.sortPaxTypeFares();
  }

  return true;
}

//----------------------------------------------------------------------------
// publishedFaresStep()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::publishedFaresStep(PricingTrx& trx,
                                              Itin& itin,
                                              FareMarket& fareMarket,
                                              bool toSort /*= false*/) const
{
  CarrierFareController cfc(trx, itin, fareMarket);
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  AddonFareController afc(trx, itin, fareMarket, trx.specifiedFareCache());
#else
  AddonFareController afc(trx, itin, fareMarket);
#endif
  IndustryFareController ifc(trx, itin, fareMarket);

  findPublishedAddOnIndustryFares(trx, itin, fareMarket, cfc, afc, ifc, toSort);
}

//----------------------------------------------------------------------------
// publishedFaresStepFareDisplay()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::publishedFaresStepFareDisplay(PricingTrx& trx,
                                                         Itin& itin,
                                                         FareMarket& fareMarket,
                                                         bool toSort /* = false */) const
{
  FDCarrierFareController cfc(trx, itin, fareMarket);
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  AddonFareController afc(trx, itin, fareMarket, trx.specifiedFareCache());
#else
  AddonFareController afc(trx, itin, fareMarket);
#endif
  FDIndustryFareController ifc(dynamic_cast<FareDisplayTrx&>(trx), itin, fareMarket);

  findPublishedAddOnIndustryFares(trx, itin, fareMarket, cfc, afc, ifc, toSort);
}

//----------------------------------------------------------------------------
// diagnostic451Step
//---------------------------------------------------------------------------

void
FareCollectorOrchestrator::diagnostic451Step(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  // const std::string group="FARESC_SVC";
  // const std::string key="RTNG_FCO";
  std::string status;

  Diagnostic& trxDiag = trx.diagnostic();

  if (!_indicator && trxDiag.diagnosticType() == Diagnostic451)
  {
    // throw new ErrorResponseException (ErrorResponseException::SYSTEM_EXCEPTION,
    //"DIAG 451 NOT AVAILABLE - CHECK CONFIG FILE");
    return;
  }

  // Find all the routing keys for the faremarket

  if (trxDiag.diagnosticType() == Diagnostic451)
  {
    LOG4CXX_INFO(logger, "FareCollectorOrchestrator::diagnostic451Step()");

    TravelRoute tvlRoute;

    TravelRouteBuilder builder;

    if (!builder.buildTravelRoute(trx, fareMarket, tvlRoute))
    {
      LOG4CXX_INFO(logger, "TravelRouteBuilder::buildTravelRoute - Failed");
      return;
    }

    const std::vector<RoutingKeyInfo*>& rtgKeyInfoV = trx.dataHandle().getRoutingForMarket(
        fareMarket.boardMultiCity(), fareMarket.offMultiCity(), fareMarket.governingCarrier());
    if (!rtgKeyInfoV.empty())
    {
      DCFactory* factory = DCFactory::instance();
      Diag451Collector* diagPtr = dynamic_cast<Diag451Collector*>(factory->create(trx));

      if (diagPtr != nullptr)
      {
        diagPtr->enable(Diagnostic451);

        diagPtr->displayRoutingValidationResults(tvlRoute, rtgKeyInfoV);

        diagPtr->flushMsg();
      }
    }
  }

  return;
}

void
FareCollectorOrchestrator::selectCurrencyStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  bool selectRC = false;

  try
  {
    selectRC = selectCurrency(trx, itin, fareMarket);
  }
  catch (tse::ErrorResponseException& ex) // lint !e601
  {
    throw;
  }

  SUPPRESS_UNUSED_WARNING(selectRC);
  LOG4CXX_DEBUG(logger, "Currency Selection returned: " << selectRC);

  return;
}
//---------------------------------------------------------------------------
// releaseFaresStep()
//---------------------------------------------------------------------------

class FailedFlightFinderFare
{
public:
  FailedFlightFinderFare(FlightFinderTrx& trx,
                         DiagCollector& diag,
                         std::set<const PaxTypeFare*>& baseFares)
    : _trx(trx), _diag(diag), _baseFares(baseFares)
  {
  }

  bool operator()(PaxTypeFare* ptFare)
  {
    if (!ptFare)
    {
      return true;
    }

    bool result = checkFare(ptFare);

    if ((true == result) && (true == ptFare->isKeepForRoutingValidation()))
    {
      return false;
    }

    return result;
  }

  bool checkFare(const PaxTypeFare* ptFare)
  {
    /*If fare is base fare then keep it */
    if (_baseFares.find(ptFare) != _baseFares.end())
    {
      return false;
    }

    std::vector<FlightFinderTrx::FareBasisCodeInfo>& faresInfo = _trx.fareBasisCodesFF();
    std::vector<FlightFinderTrx::FareBasisCodeInfo>::const_iterator faresInfoBeg =
        faresInfo.begin();

    for (; faresInfoBeg != faresInfo.end(); ++faresInfoBeg)
    {
      const FlightFinderTrx::FareBasisCodeInfo& fareBInfo = *faresInfoBeg;

      if ((ptFare->originalFareAmount() == fareBInfo.fareAmount) &&
          (ptFare->fareClass() == fareBInfo.fareBasiscode ||
           ptFare->createFareBasis(_trx) == fareBInfo.fareBasiscode) &&
          (ptFare->currency() == fareBInfo.currencyCode) && ptFare->isValid())
      {
        return false;
      }

      std::string failReason = "";
      failReason = (ptFare->originalFareAmount() != fareBInfo.fareAmount)
                       ? "AMOUNT"
                       : (ptFare->fareClass() != fareBInfo.fareBasiscode)
                             ? "CLASS"
                             : (ptFare->currency() != fareBInfo.currencyCode)
                                   ? "CURRENCY"
                                   : (!ptFare->isValid()) ? "PREVALID" : "UNKNOW";

      _diag << "FLIGHT_FINDER: " << toString(*ptFare, failReason) << "\n";

    } // END_OF_FARES_INFO_BEG

    /* Not matched, remove paxTypeFare */
    return true;
  }

  std::string toString(const PaxTypeFare& paxFare, const std::string& fail)
  {
    std::ostringstream dc;
    std::string gd;
    globalDirectionToStr(gd, paxFare.fare()->globalDirection());

    std::string fareBasis = paxFare.fareClass().c_str();
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
       << std::setw(8) << paxFare.fareAmount() << std::setw(8)
       << "ORIG:" << paxFare.originalFareAmount() << std::setw(4) << paxFare.currency()
       << std::setw(7) << fail << std::endl;
    return dc.str();
  }

private:
  FlightFinderTrx& _trx;
  DiagCollector& _diag;
  std::set<const PaxTypeFare*>& _baseFares;
};

class MatchFlightFinderFare
{
public:
  MatchFlightFinderFare(FlightFinderTrx& trx, DiagCollector& diag, bool checkIsValid = false)
    : _trx(trx), _diag(diag), _checkIsValid(checkIsValid)

  {
  }

  bool operator()(PaxTypeFare* ptFare)
  {
    if (!ptFare)
    {
      return false;
    }
    std::vector<FlightFinderTrx::FareBasisCodeInfo>& faresInfo = _trx.fareBasisCodesFF();
    std::vector<FlightFinderTrx::FareBasisCodeInfo>::const_iterator faresInfoBeg =
        faresInfo.begin();

    for (; faresInfoBeg != faresInfo.end(); ++faresInfoBeg)
    {
      const FlightFinderTrx::FareBasisCodeInfo& fareBInfo = *faresInfoBeg;

      if ((ptFare->originalFareAmount() == fareBInfo.fareAmount) &&
          (ptFare->fareClass() == fareBInfo.fareBasiscode ||
           ptFare->createFareBasis(_trx) == fareBInfo.fareBasiscode) &&
          (ptFare->currency() == fareBInfo.currencyCode))
      {
        bool bRet = true;

        if (_checkIsValid)
        {
          bRet = ptFare->isValid();
        }

        if (bRet)
        {
          ptFare->setKeepForRoutingValidation(false);
          _diag << "FLIGHT_FINDER: " << toString(*ptFare, "MATCH") << "\n";
        }

        return bRet;
      }
    } // END_OF_FARES_INFO_BEG

    /* Not matched fare */
    return false;
  }

  std::string toString(const PaxTypeFare& paxFare, const std::string& fail)
  {
    std::ostringstream dc;
    std::string gd;
    globalDirectionToStr(gd, paxFare.fare()->globalDirection());

    std::string fareBasis = paxFare.fareClass().c_str();
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
       << std::setw(8) << paxFare.fareAmount() << std::setw(8)
       << "ORIG:" << paxFare.originalFareAmount() << std::setw(4) << paxFare.currency()
       << std::setw(7) << fail << std::endl << "   isValid         :" << paxFare.isValid()
       << std::endl << "   isNoDataMissing :" << paxFare.isNoDataMissing() << std::endl
       << "   !isNotValidForCP:" << !paxFare.isNotValidForCP() << std::endl
       << "   fareDisplStatus :" << paxFare.fareDisplayStatus().isNull() << std::endl
       << "   bkCodStatNotproc:"
       << paxFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED) << std::endl
       << "   !bkCodStatFail  :" << !paxFare.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL)
       << std::endl << "   !isRoutProcessed:" << !paxFare.isRoutingProcessed() << std::endl
       << "   isRoutingValid  :" << paxFare.isRoutingValid() << std::endl
       << "   fare.isValid() :" << paxFare.fare()->isValid() << std::endl
       << "   !failedFareGroup:" << !paxFare.failedFareGroup() << std::endl
       << "   areAllCatValid  :" << paxFare.areAllCategoryValid() << std::endl;
    if (!paxFare.areAllCategoryValid())
    {
      const size_t numCategories = 50;
      size_t cat = 1;

      for (cat = 1; cat != numCategories; ++cat)
        if (!paxFare.isCategoryValid(cat))
          break;

      if (cat < numCategories)
        dc << " CAT-" << cat << std::endl;
      else
        dc << " UNK" << std::endl;
    }
    if (!paxFare.isNoDataMissing())
    {
      dc << "   isFareClassAppMissing :" << paxFare.isFareClassAppMissing() << std::endl
         << "   isFareClassAppSegMiss :" << paxFare.isFareClassAppSegMissing() << std::endl
         << "   actualPaxType         :" << (paxFare.actualPaxType() != nullptr) << std::endl;
    }
    if (!paxFare.isFareClassAppSegMissing())
    {
      dc << "PaxType :" << paxFare.fcasPaxType() << std::endl;
    }
    return dc.str();
  }

private:
  FlightFinderTrx& _trx;
  DiagCollector& _diag;
  bool _checkIsValid;
};

class MatchFlightFinderAppSeg
{
public:
  MatchFlightFinderAppSeg(FlightFinderTrx& trx,
                          DiagCollector& diag,
                          bool& flipGeo,
                          FareMarket& fareMarket,
                          const FareClassAppInfo* fcaInfo,
                          const FareClassAppSegInfo* fcasInfo,
                          const Fare* fare,
                          FareClassAppSegInfo* prevFcasWithMatchedTbl990,
                          const FareClassAppSegInfo* lastFcas)
    : _trx(trx),
      _diag(diag),
      _flipGeo(flipGeo),
      _fareMarket(fareMarket),
      _fcaInfo(fcaInfo),
      _fcasInfo(fcasInfo),
      _fare(fare),
      _prevFcasWithMatchedTbl990(prevFcasWithMatchedTbl990),
      _lastFcas(lastFcas)

  {
  }

  bool operator()(const FareClassAppSegInfo* fcas)
  {
    if (!fcas)
    {
      return false;
    }

    // match directional indicator
    if (FareUtil::failFareClassAppSegDirectioanlity(*fcas, _trx, _flipGeo))
    {
      _diag << "FAILED_SEG: DIERECTIONALITY" << std::endl;
      return false;
    }
    // table 994 match
    if (fcas->_overrideDateTblNo != 0)
    {
      DateTime travelDate;
      RuleUtil::getFirstValidTvlDT(travelDate, _fareMarket.travelSeg(), true);

      if (travelDate.isOpenDate())
      {
        travelDate = _fareMarket.travelSeg().front()->departureDT();
      }

      DateTime bookingDate;
      RuleUtil::getLatestBookingDate(bookingDate, _fareMarket.travelSeg());

      if (!RuleUtil::validateDateOverrideRuleItem(_trx,
                                                  fcas->_overrideDateTblNo,
                                                  _fare->vendor(),
                                                  travelDate,
                                                  _trx.getRequest()->ticketingDT(),
                                                  bookingDate))
      {
        _diag << "FAILED_SEG: DATEOVERRIDE" << std::endl;
        return false;
      }
    }

    // table990 match
    bool isYYFareWithTbl990 =
        (fcas->_carrierApplTblItemNo != 0 && _fcaInfo->_carrier == INDUSTRY_CARRIER);
    if (isYYFareWithTbl990)
    {
      // need to check if this is BookingCode overflown FareClassAppSegInfo
      if (_prevFcasWithMatchedTbl990 != nullptr &&
          _prevFcasWithMatchedTbl990->_carrierApplTblItemNo == fcas->_carrierApplTblItemNo)
      {
        if (isForBkgCdsOverFlown(_prevFcasWithMatchedTbl990, fcas))
        {
          _diag << "FAILED_SEG: SAME_SEGMENT" << std::endl;
          return false;
        }
      }
      else
      {
        if (!_fareMarket.findFMCxrInTbl(_trx.getTrxType(),
                                        _trx.dataHandle().getCarrierApplication(
                                            _fare->vendor(), fcas->_carrierApplTblItemNo)))
        {
          // if this is the last AppSeg and we did not have match on 990 before
          // we will need to create this PTFare, otherwise we do not
          if (_prevFcasWithMatchedTbl990 != nullptr || _lastFcas != fcas)
          {
            _diag << "FAILED_SEG: CARRIER_TABLE" << std::endl;
            return false;
          }
        }
      }
    }

    if (isYYFareWithTbl990)
    {
      _prevFcasWithMatchedTbl990 = fcas;
    }
    // Match ticket designators
    if (_fcasInfo->_tktDesignator != fcas->_tktDesignator)
    {
      _diag << "FAILED_SEG: TICKET DESIGNATOR " << toString(*fcas) << std::endl;
      return false;
    }

    _diag << "MATCHED_SEG :" << toString(*fcas) << std::endl;

    // Matched Seg
    return true;
  }
  //------------------------------------------------------------------
  bool
  isForBkgCdsOverFlown(const FareClassAppSegInfo* prevFcas, const FareClassAppSegInfo* thisFcas)
  {
    return (prevFcas->_paxType == thisFcas->_paxType);
  }
  //------------------------------------------------------------------

  bool matchLocation(const Fare& fare, bool& flipGeo, const DateTime& travelDate) const
  {
    flipGeo = false;
    GeoTravelType geoTvlType = _fareMarket.geoTravelType();

    // First see if they match the original direction
    const Loc* const locMarket1 = _trx.dataHandle().getLoc(fare.origin(), travelDate);
    const Loc* const locMarket2 = _trx.dataHandle().getLoc(fare.destination(), travelDate);

    if ((locMarket1 == nullptr) || (locMarket2 == nullptr))
    {
      return false;
    }

    if ((_fcaInfo->_location1Type == UNKNOWN_LOC ||
         LocUtil::isInLoc(*locMarket1,
                          _fcaInfo->_location1Type,
                          _fcaInfo->_location1,
                          fare.vendor(),
                          RESERVED,
                          LocUtil::RECORD1_2_6,
                          geoTvlType,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT())) &&
        (_fcaInfo->_location2Type == UNKNOWN_LOC ||
         LocUtil::isInLoc(*locMarket2,
                          _fcaInfo->_location2Type,
                          _fcaInfo->_location2,
                          fare.vendor(),
                          RESERVED,
                          LocUtil::RECORD1_2_6,
                          geoTvlType,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT())))
    {
      return true;
    }

    // Now see if they match by flipping them
    if ((_fcaInfo->_location2Type == UNKNOWN_LOC ||
         LocUtil::isInLoc(*locMarket1,
                          _fcaInfo->_location2Type,
                          _fcaInfo->_location2,
                          fare.vendor(),
                          RESERVED,
                          LocUtil::RECORD1_2_6,
                          geoTvlType,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT())) &&
        (_fcaInfo->_location1Type == UNKNOWN_LOC ||
         LocUtil::isInLoc(*locMarket2,
                          _fcaInfo->_location1Type,
                          _fcaInfo->_location1,
                          fare.vendor(),
                          RESERVED,
                          LocUtil::RECORD1_2_6,
                          geoTvlType,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT())))
    {
      flipGeo = true;
      return true;
    }

    return false;
  }
  //------------------------------------------------------------------
  std::string toString(const FareClassAppSegInfo& fcas)
  {
    std::ostringstream dc;

    dc << std::endl << "OverrideDateTableNo :" << fcas._overrideDateTblNo << std::endl
       << "CarrierApplTableNo :" << fcas._carrierApplTblItemNo << std::endl
       << "BookingCodeTableNo :" << fcas._bookingCodeTblItemNo << std::endl
       << "Carrier :" << _fcaInfo->_carrier << std::endl << "FareType :" << _fcaInfo->_fareType
       << std::endl << "PaxType :" << fcas._paxType << std::endl
       << "TicketDesignator :" << fcas._tktDesignator << std::endl
       << "TicketDesignatorModif :" << fcas._tktDesignatorModifier << std::endl
       << "Directionality :" << fcas._directionality << std::endl << "TicketCode :" << fcas._tktCode
       << std::endl << "TicketCodeModif :" << fcas._tktCodeModifier << std::endl
       << "SequenceNo :" << fcas._seqNo << std::endl << "MinAge : " << fcas._minAge << std::endl
       << "MaxAge : " << fcas._maxAge << std::endl << "BookingCodes :";
    for (int i = 0; i < 8; i++)
      dc << fcas._bookingCode[i] << ",";
    dc << std::endl;

    return dc.str();
  }

private:
  FlightFinderTrx& _trx;
  DiagCollector& _diag;
  bool& _flipGeo;
  FareMarket& _fareMarket;
  const FareClassAppInfo* _fcaInfo;
  const FareClassAppSegInfo* _fcasInfo;
  const Fare* _fare;
  const FareClassAppSegInfo* _prevFcasWithMatchedTbl990;
  const FareClassAppSegInfo* _lastFcas;
};

//----------------------------------------------------------------------------
// validate rules not related to a prticular Itin
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::ruleValidator(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  LOG4CXX_INFO(logger,
               "MIP FCORuleValidation  FARE MARKET: " << fareMarket.boardMultiCity() << "-"
                                                      << fareMarket.offMultiCity());

  if ((trx.getTrxType() == PricingTrx::MIP_TRX) && (trx.isAltDates()))
  {
    RuleControllerWithChancelor<FareMarketRuleController> fmrc(FCORuleValidationMIPALT);
    if (trx.getOptions()->newIndicatorToRemoveCat5())
    {
      fmrc.removeCat(RuleConst::ADVANCE_RESERVATION_RULE);
    }
    fmrc.validate(trx, fareMarket, itin);
  }

  else
  {
    RuleControllerWithChancelor<FareMarketRuleController> fmrc(FCORuleValidation, &trx);
    fmrc.validate(trx, fareMarket, itin);
  }
}

//----------------------------------------------------------------------------
// releaseFaresStep()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::setPrevalidationStatusAndFBRBaseFares(
    FlightFinderTrx& trx,
    std::vector<PaxTypeFare*>& allPaxTypeFares,
    DiagCollector& diag,
    std::map<PaxTypeCode, bool>& psgPrevalidationStatus,
    std::set<const PaxTypeFare*>& baseFares,
    bool& hasIndustryFareFailed)
{
  MatchFlightFinderFare ffMatchFare(trx, diag);

  std::vector<PaxTypeFare*>::iterator foundIt =
      find_if(allPaxTypeFares.begin(), allPaxTypeFares.end(), ffMatchFare);

  while (foundIt != allPaxTypeFares.end())
  {
    PaxTypeFare* ptFare = *foundIt;
    PaxTypeCode psgType = ptFare->fcasPaxType();

    // Keep FareByRule base fare
    if (ptFare->isValid())
    {
      if (ptFare->status().isSet(PaxTypeFare::PTF_FareByRule) && !ptFare->isSpecifiedFare())
      {
        PaxTypeFare* basePtf = ptFare->baseFare(25);
        // Invalidate this fare
        basePtf->setNotValidForCP(true);
        baseFares.insert(basePtf);
      }

      if (ptFare->fare()->isIndustry())
      {
        const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(ptFare->fare());
        if (indFare != nullptr && !indFare->validForPricing())
        {
          // This fare is to remove
          ptFare->setNotValidForCP(true);

          hasIndustryFareFailed = true;
        }
      }
    }

    if (!psgType.empty() && !psgPrevalidationStatus[psgType])
    {
      psgPrevalidationStatus[psgType] = ptFare->areAllCategoryValid();
    }

    foundIt = find_if(++foundIt, allPaxTypeFares.end(), ffMatchFare);

  } // END_WHILE_FOUND_IT
}

void
FareCollectorOrchestrator::releaseFFinderInvalidFares(
    FlightFinderTrx& trx,
    Itin& itin,
    FareMarket& fareMarket,
    DiagCollector& diag,
    std::map<PaxTypeCode, bool>& psgPrevalidationStatus)
{
  if (!trx.ignoreDiagReq() &&
      trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALLFARES")
  {
    return;
  }

  std::vector<PaxTypeFare*>& allPaxTypeFares = fareMarket.allPaxTypeFare();
  std::set<const PaxTypeFare*> baseFares;
  bool hasIndustryFareFailed = false;

  setPrevalidationStatusAndFBRBaseFares(
      trx, allPaxTypeFares, diag, psgPrevalidationStatus, baseFares, hasIndustryFareFailed);

  // Now all base fares are known
  // Which paxTypeFare is to remove
  FailedFlightFinderFare ffFailedFare(trx, diag, baseFares);

  diag << "\n RELEASE FLIGHT FINDER INVALID FARES \n";
  diag << "FARE MARKET: " << FareMarketUtil::getDisplayString(fareMarket) << "\n";

  std::vector<PaxTypeBucket>& ptCortege = fareMarket.paxTypeCortege();
  std::vector<PaxTypeBucket>::iterator ptCortegeBeg = ptCortege.begin();

  for (; ptCortegeBeg != ptCortege.end(); ++ptCortegeBeg)
  {
    try
    {
      std::vector<PaxTypeFare*>& ptFares = ptCortegeBeg->paxTypeFare();
      std::vector<PaxTypeFare*>::iterator newEndIt =
          remove_if(ptFares.begin(), ptFares.end(), ffFailedFare);

      if (newEndIt != ptFares.end())
      {
        int total = ptFares.size();
        int released = std::distance(newEndIt, ptFares.end());

        ptFares.erase(newEndIt, ptFares.end());

        diag << "FARE MARKET: " << FareMarketUtil::getDisplayString(fareMarket) << " - RELEASED "
             << released << " of " << total << " FARES\n \n";
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

  } // END_PT_CORTEGE_BEG

  std::vector<PaxTypeFare*>::iterator newEndIt =
      remove_if(allPaxTypeFares.begin(), allPaxTypeFares.end(), ffFailedFare);

  if (newEndIt != allPaxTypeFares.end())
  {
    int total = allPaxTypeFares.size();
    int released = std::distance(newEndIt, allPaxTypeFares.end());

    allPaxTypeFares.erase(newEndIt, allPaxTypeFares.end());

    diag << "FARE MARKET: " << FareMarketUtil::getDisplayString(fareMarket) << " - RELEASED "
         << released << " of " << total << " FARES\n \n";
  }

  // See if there is still any valid fare, if not, report error,otherwise clear error
  std::vector<PaxTypeFare*>::iterator foundFareIt = find_if(
      allPaxTypeFares.begin(), allPaxTypeFares.end(), MatchFlightFinderFare(trx, diag, true));

  if (foundFareIt == allPaxTypeFares.end() && hasIndustryFareFailed)
  {
    std::string fareBasisCode = (*trx.fareBasisCodesFF().begin()).fareBasiscode;

    trx.reportError().errorCode = ErrorResponseException::FARE_NOT_PERMITTED_FOR_PRICING;
    trx.reportError().message =
        fareBasisCode + " NOT PERMITTED ON " + fareMarket.governingCarrier();
  }
}

void
FareCollectorOrchestrator::validateFFinderPsgType(
    FlightFinderTrx& trx,
    Itin& itin,
    FareMarket& fareMarket,
    DiagCollector& diag,
    std::map<PaxTypeCode, bool>& psgPrevalidationStatus)
{
  LOG4CXX_DEBUG(logger, "Entered FareCollectorOrchestrator::validateFFinderPsgType()");

  bool swappedAppSeg = false;

  std::vector<PaxTypeFare*>& allPaxTypeFares = fareMarket.allPaxTypeFare();

  // This collection contains matched and valid paxTypeFare
  std::vector<PaxTypeFare*>::iterator ptfIter = allPaxTypeFares.begin();

  for (; ptfIter != allPaxTypeFares.end() && !swappedAppSeg; ++ptfIter)
  {
    PaxTypeFare* ptFare = *ptfIter;

    if (ptFare->isKeepForRoutingValidation())
    {
      continue;
    }

    FareClassAppSegInfo* prevFcasWithMatchedTbl990 = nullptr;

    bool flipGeo = false;
    std::map<PaxTypeCode, const FareClassAppSegInfo*> matchedAppSegMap;

    if (ptFare->fareClassAppInfo()->_segs.size() == 1)
    {
      PaxTypeCode actualPsgType = ptFare->fareClassAppSegInfo()->_paxType;

      if (!actualPsgType.empty())
      {
        matchedAppSegMap[actualPsgType] = ptFare->fareClassAppSegInfo();
      }
    }
    else
    {
      MatchFlightFinderAppSeg appSegMatcher(trx,
                                            diag,
                                            flipGeo,
                                            fareMarket,
                                            ptFare->fareClassAppInfo(),
                                            ptFare->fareClassAppSegInfo(),
                                            ptFare->fare(),
                                            prevFcasWithMatchedTbl990,
                                            ptFare->fareClassAppInfo()->_segs.back());

      // To set fligGeo flag
      appSegMatcher.matchLocation(
          *ptFare->fare(), flipGeo, ItinUtil::getTravelDate(itin.travelSeg()));

      FareClassAppSegInfoList::const_iterator fcasI = ptFare->fareClassAppInfo()->_segs.begin(),
                                              fcasEnd = ptFare->fareClassAppInfo()->_segs.end();

      // Match all segments
      FareClassAppSegInfoList::const_iterator matchedSegI = find_if(fcasI, fcasEnd, appSegMatcher);

      while (matchedSegI != fcasEnd)
      {
        const PaxTypeCode& actualPaxType = (*matchedSegI)->_paxType;

        if (!actualPaxType.empty() &&
            (!trx.reqPaxType().empty() ||
             PaxTypeUtil::isAdult(actualPaxType, ptFare->fareClassAppInfo()->_vendor)))
        {
          matchedAppSegMap[actualPaxType] = *matchedSegI;
        }
        matchedSegI = find_if(++matchedSegI, fcasEnd, appSegMatcher);
      }
    }

    std::map<PaxTypeCode, const FareClassAppSegInfo*>::const_iterator appSegMapI =
        matchedAppSegMap.begin();

    if (trx.reqPaxType().empty())
    {
      // Pax Type no requested
      if (matchedAppSegMap.size() > 1)
      {
        // Create error message
        std::string error = "$MULTIPLE PASSENGER TYPES APPLY - SPECIFY ";
        appSegMapI = matchedAppSegMap.begin();

        while (appSegMapI != matchedAppSegMap.end())
        {
          error += (*appSegMapI).first;
          ++appSegMapI;
          error += (appSegMapI == matchedAppSegMap.end()) ? "" : std::string("/");
        }

        trx.reportError().message = error;
        trx.reportError().errorCode = ErrorResponseException::PSG_TYPE_ERROR;
        break; // Stop processing other fares
      }
    }
    else
    {
      // Pax Type requested

      const std::vector<PaxTypeCode>& reqPaxTypes = trx.reqPaxType();

      // if there is no requested pax type among fare pax type
      if (!PaxTypeUtil::isPaxInTrx(trx, reqPaxTypes))
      {
        LOG4CXX_DEBUG(logger,
                      "FareCollectorOrchestrator::validateFFinderPsgType"
                          << " - PAX NOT IN TRX - "
                          << "PSG: " << ptFare->fcasPaxType()
                          << " ACTUAL_PSG:" << ptFare->actualPaxType()->paxType());

        trx.reportError().message = "$FORMAT - PSGR TYPES";
        trx.reportError().errorCode = ErrorResponseException::PSG_TYPE_ERROR;

        // Try to match requested pax type with fareclassappseg
        appSegMapI = matchedAppSegMap.begin();

        for (; appSegMapI != matchedAppSegMap.end(); ++appSegMapI)
        {
          PaxTypeCode segPsgType = (*appSegMapI).first;

          if (trx.isPaxTypeReq(segPsgType))
          { // segment matches

            if ((psgPrevalidationStatus.count(segPsgType) != 0) &&
                !psgPrevalidationStatus[segPsgType])
            {
              LOG4CXX_DEBUG(logger,
                            "FareCollectorOrchestrator::validateFFinderPsgType"
                                << " - SKIPPING SEGMENT " << segPsgType
                                << " - PREVALIDATION FAILED");
              continue;
            }

            // swapping segment
            ptFare->fareClassAppSegInfo() = (*appSegMapI).second;
            swappedAppSeg = true;

            trx.reportError().message = "";
            trx.reportError().errorCode = ErrorResponseException::NO_ERROR;

            LOG4CXX_DEBUG(logger,
                          "FareCollectorOrchestrator::validateFFinderPsgType"
                              << " - FARE MATCHED FOR PAX " << segPsgType << " - SEGMENT SWAPPED");

            break; // stop processing segments
          }

        } // END_FOR

      } // END_isPaxInTrx
      else
      {
        LOG4CXX_DEBUG(logger,
                      "FareCollectorOrchestrator::validateFFinderPsgType"
                          << " - REQUEST IS VALID");
      }
    } // END_paxTypeRequested

  } // END_FOR_ptfIter
}

void
FareCollectorOrchestrator::releaseFaresStep(PricingTrx& trx, Itin& itin, FareMarket& curFareMarket)
{
  if (UNLIKELY(curFareMarket.useDummyFare()))
    return;

  FareDisplayUtil fdUtil;
  FareDisplayTrx* fdTrx;
  if (UNLIKELY(fdUtil.getFareDisplayTrx(&trx, fdTrx)))
  {
    if (fdTrx->getRequest()->diagnosticNumber() == DIAG_200_ID)
    {
      return;
    }
  }

  DiagMonitor diag(trx, Diagnostic299);

  if (UNLIKELY(trx.getTrxType() == PricingTrx::FF_TRX))
  {
    FlightFinderTrx& ffTrx = dynamic_cast<FlightFinderTrx&>(trx);

    if (ffTrx.isFFReq() || ((ffTrx.bffStep() != FlightFinderTrx::STEP_1) &&
                            (ffTrx.bffStep() != FlightFinderTrx::STEP_3)))
    {
      if (specialRoutingFoundInRequestedFares(trx, diag.diag(), curFareMarket))
      {
        findFaresWithUniqueRoutings(trx, curFareMarket);
      }
    }
  }

  bool diagEnabled = diag.diag().isActive();
  if (diagEnabled)
  {
    diag << "\nRELEASE FARES INVALIDATED BY FCO RULES VALIDATION:\n";
    diag << "FARE MARKET: " << curFareMarket.origin()->loc() << "-"
         << curFareMarket.governingCarrier() << "-" << curFareMarket.destination()->loc() << "\n";
    diag << "\n";
    diag << "   GI V RULE   FARE BASIS TRF O O         AMT CUR RULE \n";
    diag << "                          NUM R I                 FAILED\n";
    diag << "-- -- - ---- ------------ --- - - ----------- --- ------\n";
  }

  std::vector<PaxTypeBucket>& ptCortege = curFareMarket.paxTypeCortege();
  for (auto& elem : ptCortege)
  {
    try
    {
      std::vector<PaxTypeFare*>& ptFares = elem.paxTypeFare();
      std::vector<PaxTypeFare*>::iterator riter;
      riter = std::remove_if(ptFares.begin(),
                             ptFares.end(),
                             FailedFare(trx, diag.diag(), curFareMarket.specialRtgFound()));
      if (riter != ptFares.end())
      {
        int total = ptFares.size();
        int released = std::distance(riter, ptFares.end());

        ptFares.erase(riter, ptFares.end());

        if (diagEnabled)
        {
          diag << "RELEASED " << released << " OF " << total << " FARES FROM "
               << elem.requestedPaxType()->paxType() << " CORTEGE\n \n";
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

  std::vector<PaxTypeFare*>& allPaxTypeFares = curFareMarket.allPaxTypeFare();
  try
  {
    std::vector<PaxTypeFare*>::iterator riter;

    riter = std::remove_if(allPaxTypeFares.begin(),
                           allPaxTypeFares.end(),
                           FailedFare(trx, diag.diag(), curFareMarket.specialRtgFound()));

    if (riter != allPaxTypeFares.end())
    {
      int total = allPaxTypeFares.size();
      int released = std::distance(riter, allPaxTypeFares.end());

      allPaxTypeFares.erase(riter, allPaxTypeFares.end());

      if (diagEnabled)
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

  /* Flight finder removal step */
  if (UNLIKELY(trx.getTrxType() == PricingTrx::FF_TRX))
  {
    FlightFinderTrx& ffTrx = dynamic_cast<FlightFinderTrx&>(trx);
    std::map<PaxTypeCode, bool> psgPrevalidationStatus;
    releaseFFinderInvalidFares(ffTrx, itin, curFareMarket, diag.diag(), psgPrevalidationStatus);

    validateFFinderPsgType(ffTrx, itin, curFareMarket, diag.diag(), psgPrevalidationStatus);

    FareFocusRuleValidator ffRuleValidator(trx, curFareMarket);
    ffRuleValidator.process();
  }

  return;
}

void
FareCollectorOrchestrator::findFaresWithUniqueRoutings(PricingTrx& trx, FareMarket& fareMarket)
{
  uint32_t numFaresKeepForRtgValidation = numFaresKeepForRoutingValidation.getValue();

  if (!numFaresKeepForRtgValidation)
  {
    return;
  }

  std::map<RtgKey, PaxTypeFare*> uniqueRoutingsMap;
  for (const auto paxTypeFare : fareMarket.allPaxTypeFare())
  {
    if (uniqueRoutingsMap.size() >= numFaresKeepForRtgValidation)
    {
      break;
    }

    if (nullptr == paxTypeFare)
    {
      continue;
    }

    RtgKey rtgKey;

    rtgKey.vendor() = paxTypeFare->vendor();
    rtgKey.carrier() = fareMarket.governingCarrier();
    rtgKey.routingNumber() = paxTypeFare->routingNumber();
    rtgKey.routingTariff() = paxTypeFare->tcrRoutingTariff1();

    if (uniqueRoutingsMap.end() == uniqueRoutingsMap.find(rtgKey))
    {
      uniqueRoutingsMap[rtgKey] = paxTypeFare;
      paxTypeFare->setKeepForRoutingValidation();
    }
  }
}

bool
FareCollectorOrchestrator::specialRoutingFoundInRequestedFares(PricingTrx& trx,
                                                               DiagCollector& diag,
                                                               FareMarket& fareMarket)
{
  FlightFinderTrx& ffTrx = dynamic_cast<FlightFinderTrx&>(trx);

  MatchFlightFinderFare ffMatchFare(ffTrx, diag);

  std::vector<PaxTypeFare*>::iterator foundIt =
      find_if(fareMarket.allPaxTypeFare().begin(), fareMarket.allPaxTypeFare().end(), ffMatchFare);

  while (foundIt != fareMarket.allPaxTypeFare().end())
  {
    PaxTypeFare* ptFare = *foundIt;

    if (RoutingUtil::isSpecialRouting(*ptFare, false))
    {
      return true;
    }

    foundIt = find_if(++foundIt, fareMarket.allPaxTypeFare().end(), ffMatchFare);
  }

  return false;
}

//----------------------------------------------------------------------------
// invokeStep()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::InvokeStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (fareMarket.allPaxTypeFare().empty() &&
      fareMarket.failCode() == ErrorResponseException::NO_ERROR)
  {
    fareMarket.failCode() = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
  }
  else
  {
    if (fareMarket.geoTravelType() == GeoTravelType::Transborder)
      fareMarket.setMarketCurrencyPresent(trx);
  }
}

//----------------------------------------------------------------------------
// sortStep()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::sortStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (fareMarket.allPaxTypeFare().empty() &&
      fareMarket.failCode() == ErrorResponseException::NO_ERROR)
  {
    fareMarket.failCode() = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
  }
  else
  {
    CarrierFareController carrierFareController(trx, itin, fareMarket);
    carrierFareController.sortPaxTypeFares();
    // fareMarket.setMarketCurrencyPresent(trx);
  }

  Diagnostic& trxDiag = trx.diagnostic();

  if (UNLIKELY(trxDiag.diagnosticType() == AllFareDiagnostic))
  {
    //    TSELatencyData metrics( trx, "FCO DIAGNOSTICS");

    DCFactory* factory = DCFactory::instance();
    Diag200Collector* diagPtr = dynamic_cast<Diag200Collector*>(factory->create(trx));

    if (diagPtr != nullptr)
    {
      diagPtr->enable(AllFareDiagnostic);

      if (diagPtr->parseQualifiers(trx, fareMarket, &itin))
      {
        diagPtr->print(itin, fareMarket);

        diagPtr->flushMsg();
      }
    }
  }
  else if (UNLIKELY(trxDiag.diagnosticType() == Diagnostic223))
  {
    DCFactory* const factory = DCFactory::instance();
    Diag223Collector* const diagPtr = dynamic_cast<Diag223Collector*>(factory->create(trx));

    if (diagPtr != nullptr)
    {
      diagPtr->enable(Diagnostic223);
      diagPtr->parseQualifiers(trx);

      (*diagPtr) << fareMarket;

      diagPtr->flushMsg();
    }
  }

  return;
}

void
FareCollectorOrchestrator::setProcessedStepForDummy(PricingTrx& trx, bool mark)
{
  if (mark)
    invoke_foreach_fareMarket(trx, markProcessedStepDummy);
  else
    invoke_foreach_fareMarket(trx, unmarkProcessedStepDummy);
}

void
FareCollectorOrchestrator::setProcessedStep(PricingTrx& trx,
                                            Itin& itin,
                                            FareMarket& fareMarket,
                                            bool set = true)
{
  fareMarket.serviceStatus().set(FareMarket::FareCollector, set);
}

void
FareCollectorOrchestrator::markProcessedStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  setProcessedStep(trx, itin, fareMarket);
}

//----------------------------------------------------------------------------
// markProcessedStep()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::markProcessedStepDummy(PricingTrx& trx,
                                                  Itin& itin,
                                                  FareMarket& fareMarket)
{
  if (LIKELY(!fareMarket.useDummyFare()))
    return;
  setProcessedStep(trx, itin, fareMarket);
}

void
FareCollectorOrchestrator::unmarkProcessedStepDummy(PricingTrx& trx,
                                                    Itin& itin,
                                                    FareMarket& fareMarket)
{
  if (LIKELY(!fareMarket.useDummyFare()))
    return;
  setProcessedStep(trx, itin, fareMarket, false);
}

std::string
FareCollectorOrchestrator::getConnectingCity(std::vector<TravelSeg*>& travelSegVec)
{
  std::string connectingCity = "";
  if (travelSegVec.size() < 2)
  {
    return connectingCity;
  }
  std::vector<TravelSeg*>::iterator tvlSegB = travelSegVec.begin();
  std::vector<TravelSeg*>::iterator tvlSegE = travelSegVec.end();
  for (; tvlSegB != tvlSegE; ++tvlSegB)
  {
    // first travel segment process
    if (connectingCity.empty())
    {
      connectingCity += (*tvlSegB)->destAirport();
      continue;
    }
    connectingCity += (*tvlSegB)->origAirport() + (*tvlSegB)->destAirport();
  }
  return connectingCity;
}

//----------------------------------------------------------------------------
// findDuplicateFareMarkets()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::findDuplicateFareMarkets(
    PricingTrx& trx,
    std::map<std::string, std::vector<FareMarket*>>& fmMap,
    std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs,
    std::map<FareMarket*, std::vector<int>>* fmOrigBrands,
    ErrorResponseException::ErrorResponseCode* duplicateStatus)
{
  // Loop through all the fareMarkets in the Transaction and find one that
  // have the same computed hash, lumping them
  // all into the same vector

  uint32_t skipped = 0;
  uint32_t market = 0;

  std::vector<FareMarket*>& fareMarkets = trx.fareMarket();

  std::vector<FareMarket*>::iterator k = fareMarkets.begin();
  std::vector<FareMarket*>::iterator l = fareMarkets.end();

  for (; k != l; ++k)
  {
    market++;

    FareMarket* fm = *k;
    if (fm == nullptr || fm->failCode() != ErrorResponseException::NO_ERROR ||
        fm->serviceStatus().isSet(tse::FareMarket::FareCollector) ||
        fm->allPaxTypeFare().empty() == false) // paxtype fare is not empty
    {
      continue;
    }

    if (UNLIKELY(fm->useDummyFare()))
    {
      continue;
    }

    std::ostringstream ossHash;

    TravelSeg* orig = fm->travelSeg().front();
    TravelSeg* dest = fm->travelSeg().back();

    std::ostringstream date;
    date << fm->travelDate().get64BitRepDateOnly();

    std::string connectingCity;

    if (UNLIKELY(trx.getRequest()->originBasedRTPricing() &&
                 trx.outboundDepartureDate() != DateTime::emptyDate()))
    {
      connectingCity = "";
    }
    else
    {
      connectingCity = getConnectingCity(fm->travelSeg());
    }

    if ((trx.getTrxType() == PricingTrx::MIP_TRX) && (trx.isAltDates()))
    {
      ossHash << orig->origAirport() << "."
              //<<fm->destination()->loc()<<"."
              << dest->destAirport() << "." << fm->getGlobalDirection() << "."
              << fm->governingCarrier() << "." << static_cast<int>(fm->geoTravelType());

      ossHash << (fm->isSimpleTrip() ? "T" : "F");
    }
    else if (UNLIKELY(trx.getTrxType() == PricingTrx::MIP_TRX &&
                      trx.excTrxType() == PricingTrx::AR_EXC_TRX))
    {
      ossHash << orig->origAirport() << "." << dest->destAirport() << "."
              << fm->getGlobalDirection() << "." << fm->governingCarrier() << "."
              << static_cast<int>(fm->geoTravelType()) << ".";

      ossHash << (fm->isSimpleTrip() ? "T" : "F");

      ossHash << date.str() << "." << fm->retrievalDate().get64BitRepDateOnly() << "."
              << orig->departureDT().get64BitRepDateOnly();
    }
    else
    {
      ossHash << orig->origAirport() << "." << dest->destAirport() << "."
              << fm->getGlobalDirection() << "." << fm->governingCarrier() << "."
              << static_cast<int>(fm->geoTravelType()) << ".";

      ossHash << (fm->isSimpleTrip() ? "T" : "F");

      ossHash << date.str() << "." << orig->departureDT().get64BitRepDateOnly();
    }
    std::string fmHash = ossHash.str();

    // Note this makes the vector on the first one, this is what we want
    std::vector<FareMarket*>& items = fmMap[fmHash];

    bool dupe = false;
    if (!items.empty())
    {
      dupe = true;
      skipped++;

      if (LIKELY(duplicateStatus != nullptr))
        fm->failCode() = *duplicateStatus;
      items.front()->setHasDuplicates(true);

      if (LIKELY(fmOrigBrands))
        mergeBrands(items.front()->brandProgramIndexVec(), fm->brandProgramIndexVec());

      if (LIKELY(fmOrigVCs))
        mergeValidatingCarriers(items.front()->validatingCarriers(), fm->validatingCarriers());
    }
    else
    {
      if (LIKELY(fmOrigBrands))
        (*fmOrigBrands)[fm] = fm->brandProgramIndexVec();

      if (LIKELY(fmOrigVCs))
        (*fmOrigVCs)[fm] = fm->validatingCarriers();
    }

    items.push_back(fm);
    SUPPRESS_UNUSED_WARNING(dupe);

    if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
    {
      LOG4CXX_DEBUG(logger,
                    "FM" << market << " (" << FareMarketUtil::getDisplayString(*fm) << ") hash ("
                         << fmHash << ") duplicate (" << dupe << ")"
                         << ",fm->travelDate():" << fm->travelDate()
                         << ",fm->retrievalDate():" << fm->retrievalDate());
    }
  }

  LOG4CXX_DEBUG(logger,
                "Identified " << skipped << " of " << market << " total FareMarkets as duplicates");
}

//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::mergeBrands(std::vector<int>& brand1, std::vector<int>& brand2)
{
  for (int index : brand2)
    if (UNLIKELY(std::find(brand1.begin(), brand1.end(), index) == brand1.end()))
      brand1.push_back(index);
}

//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::mergeValidatingCarriers(std::vector<CarrierCode>& vcs1,
                                                   const std::vector<CarrierCode>& vcs2)
{
  for (const CarrierCode& carrier : vcs2)
    if (std::find(vcs1.begin(), vcs1.end(), carrier) == vcs1.end())
      vcs1.push_back(carrier);
}

//----------------------------------------------------------------------------
// validateCommonFareMarkets()
//--------------------------------------------------------------------------
void
FareCollectorOrchestrator::validateCommonFareMarkets(
    PricingTrx& trx, std::map<std::string, std::vector<FareMarket*>>& fmMap)
{
  LOG4CXX_INFO(logger, "Entered FareCollectorOrchestrator::validateCommonFareMarkets ");

  std::vector<FareMarket*> fmv;

  std::map<std::string, std::vector<FareMarket*>>::iterator i = fmMap.begin();

  std::map<std::string, std::vector<FareMarket*>>::iterator j = fmMap.end();

  for (; i != j; ++i)
  {
    std::vector<FareMarket*>& fareMarkets = (*i).second;

    // See if multiple markets are in this vector, if so
    // then we have common fare market
    if (fareMarkets.size() > 1)
    {
      fmv.push_back(fareMarkets.front());
    }
  }

  if (!fmv.empty())
  {
    LOG4CXX_INFO(logger, "MIP CALL:  NUM OF COMMON FARE MARKET " << fmv.size());
    exec_foreach_valid_common_fareMarket(_taskId, trx, fmv, ruleValidator); // validate rule
  }
  else
  {
    LOG4CXX_INFO(logger, "MIP CALL:  NO COMMON FARE MARKET");
  }

  return;
}

void
FareCollectorOrchestrator::restoreOrigBrands(
    std::map<std::string, std::vector<FareMarket*>>& fmMap,
    const std::map<FareMarket*, std::vector<int>>* fmOrigBrands) const
{
  if (!fmOrigBrands)
    return;

  std::map<std::string, std::vector<FareMarket*>>::iterator i = fmMap.begin();
  std::map<std::string, std::vector<FareMarket*>>::iterator iEnd = fmMap.end();
  for (; i != iEnd; ++i)
  {
    std::vector<FareMarket*>& fareMarkets = (*i).second;
    if (fareMarkets.size() > 1)
    {
      std::vector<FareMarket*>::iterator fmI = fareMarkets.begin();
      std::vector<FareMarket*>::iterator fmIEnd = fareMarkets.end();

      FareMarket* srcFm = *fmI;
      const std::vector<int> mergedBrands = srcFm->brandProgramIndexVec();

      std::map<FareMarket*, std::vector<int>>::const_iterator i = fmOrigBrands->find(srcFm);
      TSE_ASSERT(i != fmOrigBrands->end());

      srcFm->brandProgramIndexVec() = i->second;

      restorePtfBrands(srcFm, mergedBrands, i->second);

      for (++fmI; fmI != fmIEnd; ++fmI)
        restorePtfBrands(*fmI, mergedBrands, (*fmI)->brandProgramIndexVec());
    }
  }
}

void
FareCollectorOrchestrator::restoreOrigValidatingCarriers(
    std::map<std::string, std::vector<FareMarket*>>& fmMap,
    const std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs) const
{
  if (!fmOrigVCs)
    return;

  std::map<std::string, std::vector<FareMarket*>>::iterator i = fmMap.begin();
  std::map<std::string, std::vector<FareMarket*>>::iterator iEnd = fmMap.end();
  for (; i != iEnd; ++i)
  {
    std::vector<FareMarket*>& fareMarkets = (*i).second;
    if (fareMarkets.size() > 1)
    {
      std::vector<FareMarket*>::iterator fmI = fareMarkets.begin();
      std::vector<FareMarket*>::iterator fmIEnd = fareMarkets.end();

      FareMarket* srcFm = *fmI;
      const std::vector<CarrierCode> mergedVCs = srcFm->validatingCarriers();

      std::map<FareMarket*, std::vector<CarrierCode>>::const_iterator i = fmOrigVCs->find(srcFm);
      TSE_ASSERT(i != fmOrigVCs->end());

      srcFm->validatingCarriers() = i->second;

      restorePtfVCs(srcFm, mergedVCs, i->second);

      for (++fmI; fmI != fmIEnd; ++fmI)
        restorePtfVCs(*fmI, mergedVCs, (*fmI)->validatingCarriers());
    }
  }
}

void
FareCollectorOrchestrator::restorePtfVCs(FareMarket* fm,
                                         const std::vector<CarrierCode>& mergedVCs,
                                         const std::vector<CarrierCode>& origVCs) const
{
  if (mergedVCs == origVCs)
    return;

  std::set<CarrierCode> origValidatingCarriers(origVCs.begin(), origVCs.end());

  for (PaxTypeFare* ptf : fm->allPaxTypeFare())
  {
    if (ptf->validatingCarriers().empty())
      continue;

    std::set<CarrierCode> ptfVCs(ptf->validatingCarriers().begin(),
                                 ptf->validatingCarriers().end());

    std::set<CarrierCode> actualVCs;
    std::set_intersection(origValidatingCarriers.begin(),
                          origValidatingCarriers.end(),
                          ptfVCs.begin(),
                          ptfVCs.end(),
                          std::inserter(actualVCs, actualVCs.begin()));

    ptf->validatingCarriers().clear();

    if (actualVCs.empty())
      ptf->setCategoryValid(15, false);
    else
      ptf->validatingCarriers().insert(
          ptf->validatingCarriers().end(), actualVCs.begin(), actualVCs.end());
  }
}

void
FareCollectorOrchestrator::restorePtfBrands(FareMarket* fm,
                                            const std::vector<int>& mergedBrands,
                                            const std::vector<int>& origBrands) const
{
  if (LIKELY(mergedBrands == origBrands))
    return;

  const size_t NOT_MATCHED_INDEX = 65535;

  // build a map brandIndex->location in the original brandIndex vector
  std::map<int, size_t> origBrandsMap;
  for (size_t i = 0; i < origBrands.size(); ++i)
    origBrandsMap[origBrands[i]] = i;

  // build a mapping from location in the merged vector to the location in the original vecctor
  // so that once we reorder brand indexes in the fare market
  // we can rearrange brand status in fares in exactly the same way
  std::map<size_t, size_t> mergedToOriginalMapping;
  for (size_t i = 0; i < mergedBrands.size(); ++i)
  {
    const int brandIdFromMerged = mergedBrands[i];
    if (origBrandsMap.find(brandIdFromMerged) != origBrandsMap.end())
      mergedToOriginalMapping[i] = origBrandsMap[brandIdFromMerged];
    else
      mergedToOriginalMapping[i] = NOT_MATCHED_INDEX;
  }

  for (PaxTypeFare* ptf : fm->allPaxTypeFare())
  {
    // rearranging brand statuses in fares so they correspond correctly to brand indexes in fms
    std::vector<PaxTypeFare::BrandStatusWithDirection> newStatusVec(
        origBrands.size(), std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    for (size_t i = 0; i < ptf->getBrandStatusVec().size(); ++i)
    {
      const size_t indexInOrigBrands = mergedToOriginalMapping[i];
      if (indexInOrigBrands == NOT_MATCHED_INDEX)
        continue;
      newStatusVec[indexInOrigBrands] = ptf->getBrandStatusVec()[i];
    }
    ptf->getMutableBrandStatusVec() = newStatusVec;
  }
}

//----------------------------------------------------------------------------
// copyDuplicateFareMarkets()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::copyDuplicateFareMarkets(
    PricingTrx& trx,
    std::map<std::string, std::vector<FareMarket*>>& fmMap,
    const std::map<FareMarket*, std::vector<CarrierCode>>* fmOrigVCs,
    const std::map<FareMarket*, std::vector<int>>* fmOrigBrands) const
{
  const uint16_t numSeatsRequired = PaxTypeUtil::totalNumSeats(trx);
  FareByRuleRevalidator fbrRevalidator;

  std::map<std::string, std::vector<FareMarket*>>::iterator i = fmMap.begin();
  std::map<std::string, std::vector<FareMarket*>>::iterator j = fmMap.end();

  for (; i != j; ++i)
  {
    std::vector<FareMarket*>& fareMarkets = (*i).second;

    // See if multiple markets are in this vector, if so we
    // need to copy from the first one
    if (fareMarkets.size() > 1)
    {
      std::vector<FareMarket*>::iterator y = fareMarkets.begin();
      std::vector<FareMarket*>::iterator z = fareMarkets.end();

      FareMarket* src = *y;
      ++y;

      if (UNLIKELY((trx.diagnostic().diagnosticType() == Diagnostic981) &&
                   (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "ADRELFARES")))
      {
        performDiagnostic981(trx, src);
      }
      LOG4CXX_DEBUG(logger,
                    "Copying FareMarket (" << tse::FareMarketUtil::getDisplayString(*src) << ") "
                                           << (fareMarkets.size() - 1) << " times");

      for (; y != z; ++y)
      {
        FareMarket* dest = *y;

        copyFareMarket(trx, *src, *dest);

        if (dest->geoTravelType() == GeoTravelType::Transborder)
          dest->setMarketCurrencyPresent(trx);
      }
      if (trx.isAltDates() && trx.getTrxType() == PricingTrx::MIP_TRX)
      {
        std::vector<PaxTypeBucket>& ptCortageVec = src->paxTypeCortege();

        std::vector<PaxTypeBucket>::iterator ptcIt = ptCortageVec.begin();
        std::vector<PaxTypeBucket>::iterator ptcItE = ptCortageVec.end();

        for (; ptcIt != ptcItE; ++ptcIt)
        {
          PaxTypeBucket& ptCortage = *ptcIt;
          std::vector<PaxTypeFare*>& ptfFromCortageVec = ptCortage.paxTypeFare();

          removeNotEffPTFares(trx, ptfFromCortageVec);
        }

        std::vector<PaxTypeFare*>& ptFareVec = src->allPaxTypeFare();
        removeNotEffPTFares(trx, ptFareVec);
      }
    }

    if (UNLIKELY((trx.diagnostic().diagnosticType() == Diagnostic981) &&
                 (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ADRELFARES")))
    {
      performDiagnostic981(trx, getAccaccumulateDataDiag981());
      clearAccaccumulateDataDiag981();
    }

    std::vector<FareMarket*>::iterator y = fareMarkets.begin();
    FareMarket* src = *y;
    if (src->allPaxTypeFare().empty())
      continue; // No fares to validate

    std::set<PaxTypeFare*> validNegFares;
    bool doNegFaresPostValidation = false;
    std::vector<Itin*> owningItins = FareMarketUtil::collectOwningItins(*src, trx.itin());
    setUpNegFaresPostValidation(*src,
                                src->getAllNegFaresBuckets(),
                                trx,
                                owningItins,
                                validNegFares,
                                doNegFaresPostValidation);

    std::vector<PaxTypeFare*>::iterator i = src->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator j = src->allPaxTypeFare().end();
    const bool removeOutboundFares = src->removeOutboundFares();

    for (; i != j; ++i)
    {
      PaxTypeFare* ptFare = *i;

      if (src->hasDuplicates() &&
          FareUtil::postCheckOutboundFare(removeOutboundFares,
                                          ptFare->isReversed(),
                                          ptFare->fare()->fareInfo()->directionality()))
      {
        ptFare->fare()->setDirectionalityFail(true);
      }

      if (ptFare->isFareByRule())
      {
        if (!fbrRevalidator.checkFBR(*ptFare, numSeatsRequired, *src, owningItins))
          ptFare->setCategoryValid(RuleConst::FARE_BY_RULE, false);
      }

      if (UNLIKELY(ptFare->isNegotiated() && doNegFaresPostValidation &&
                   validNegFares.find(ptFare) == validNegFares.end()))
        ptFare->setCategoryValid(RuleConst::NEGOTIATED_RULE, false);
    }

    for (PaxTypeBucket cortage : src->paxTypeCortege())
    {
      for (PaxTypeFare* cptf : cortage.paxTypeFare())
      {
        if (src->hasDuplicates() &&
            FareUtil::postCheckOutboundFare(removeOutboundFares,
                                            cptf->isReversed(),
                                            cptf->fare()->fareInfo()->directionality()))
        {
          cptf->fare()->setDirectionalityFail(true);
        }

        if (cptf->isFareByRule())
        {
          if (!fbrRevalidator.checkFBR(*cptf, numSeatsRequired, *src, owningItins))
            cptf->setCategoryValid(RuleConst::FARE_BY_RULE, false);
        }

        if (UNLIKELY(cptf->isNegotiated() && doNegFaresPostValidation &&
                     validNegFares.find(cptf) == validNegFares.end()))
          cptf->setCategoryValid(RuleConst::NEGOTIATED_RULE, false);
      }
    }
  }

  restoreOrigBrands(fmMap, fmOrigBrands);
  restoreOrigValidatingCarriers(fmMap, fmOrigVCs);
}

//----------------------------------------------------------------------------
// copyFareMarket()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::copyFareMarket(PricingTrx& trx, FareMarket& src, FareMarket& dest) const
{
  dest.failCode() = src.failCode();

  if (src.allPaxTypeFare().empty())
    return; // No fares to copy skip it

  const uint16_t numSeatsRequired = PaxTypeUtil::totalNumSeats(trx);
  FareByRuleRevalidator fbrRevalidator;
  dest.setSpecialRtgFound(src.specialRtgFound());

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    const tse::DateTime& retrievalDate = src.retrievalDate();
    if (retrievalDate != tse::DateTime::emptyDate())
    {
      static_cast<RexPricingTrx&>(trx).setFareApplicationDT(retrievalDate);
    }
  }

  std::set<PaxTypeFare*> validNegFares;
  bool doNegFaresPostValidation = false;

  std::vector<Itin*> owningItins = FareMarketUtil::collectOwningItins(dest, trx.itin());
  setUpNegFaresPostValidation(
      dest, src.getAllNegFaresBuckets(), trx, owningItins, validNegFares, doNegFaresPostValidation);

  // Get fare markets mileage for Cat 25 fares
  uint16_t mileageSrc = FareByRuleController::getMileage(src, trx);
  uint16_t mileageDest = FareByRuleController::getMileage(dest, trx);

  // First clone all the fares and keep track of the old/new mapping
  std::map<Fare*, Fare*> fareMap;
  std::map<PaxTypeFare*, PaxTypeFare*> ptFareMap;

  std::vector<PaxTypeFare*>::iterator i = src.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator j = src.allPaxTypeFare().end();

  std::vector<PaxTypeFare*>& fareVect = dest.allPaxTypeFare();
  const bool removeOutboundFares = dest.removeOutboundFares();

  for (; i != j; ++i)
  {
    PaxTypeFare* ptFare = *i;

    // check if fare qualify for outb removal
    if (FareUtil::postCheckOutboundFare(removeOutboundFares,
                                        ptFare->isReversed(),
                                        ptFare->fare()->fareInfo()->directionality()))
    {
      continue;
    }

    // check if this Cat 25 fare is valid for this FareMarket
    if (ptFare->isFareByRule())
    {
      if (!fbrRevalidator.checkFBR(*ptFare, numSeatsRequired, dest, owningItins))
        continue;
    }

    if (doNegFaresPostValidation && ptFare->isNegotiated())
      if (UNLIKELY(validNegFares.find(ptFare) == validNegFares.end()))
        continue;

    PaxTypeFare* newPTFare = copyPaxTypeFare(
        trx.dataHandle(), *ptFare, dest, fareMap, ptFareMap, trx, mileageSrc, mileageDest);

    if (UNLIKELY((trx.isAltDates() && trx.getTrxType() == PricingTrx::MIP_TRX) &&
                 newPTFare != nullptr))
    {
      if (UNLIKELY(!altDateCAT15Validation(trx, dest, newPTFare)))
      {
        accumulateDataDiag981(newPTFare);
        continue;
      }
    }
    if (LIKELY(newPTFare != nullptr))
      fareVect.push_back(newPTFare);
  }

  // Now copy the Corteges
  std::vector<PaxTypeBucket>& corteges = src.paxTypeCortege();

  std::vector<PaxTypeBucket>::iterator k = corteges.begin();
  std::vector<PaxTypeBucket>::iterator l = corteges.end();

  for (; k != l; ++k)
  {
    PaxTypeBucket& cortege = *k;
    PaxTypeBucket* destCortege = dest.paxTypeCortege(cortege.requestedPaxType());

    if (destCortege == nullptr)
      continue;

    destCortege->inboundCurrency() = cortege.inboundCurrency();
    destCortege->outboundCurrency() = cortege.outboundCurrency();
    destCortege->paxIndex() = cortege.paxIndex();

    std::vector<PaxTypeFare*>& fares = cortege.paxTypeFare();

    std::vector<PaxTypeFare*>::iterator m = fares.begin();
    std::vector<PaxTypeFare*>::iterator n = fares.end();

    std::vector<PaxTypeFare*>& destVect = destCortege->paxTypeFare();

    for (; m != n; ++m)
    {
      PaxTypeFare* ptFare = *m;

      // check if fare qualify for outb removal
      if (FareUtil::postCheckOutboundFare(removeOutboundFares,
                                          ptFare->isReversed(),
                                          ptFare->fare()->fareInfo()->directionality()))
      {
        continue;
      }

      // check if this Cat 25 fare is valid for this FareMarket
      if (ptFare->isFareByRule())
      {
        if (!fbrRevalidator.checkFBR(*ptFare, numSeatsRequired, dest, owningItins))
          continue;
      }

      if (UNLIKELY(doNegFaresPostValidation && ptFare->isNegotiated() &&
                   validNegFares.find(ptFare) == validNegFares.end()))
        continue;

      PaxTypeFare* newPTFare = copyPaxTypeFare(
          trx.dataHandle(), *ptFare, dest, fareMap, ptFareMap, trx, mileageSrc, mileageDest);
      if (LIKELY(newPTFare != nullptr))
        destVect.push_back(newPTFare);
    }
  }
}

PaxTypeFare*
FareCollectorOrchestrator::copyPaxTypeFare(DataHandle& dataHandle,
                                           PaxTypeFare& paxTypeFare,
                                           FareMarket& fareMarket,
                                           std::map<Fare*, Fare*>& fareMap,
                                           std::map<PaxTypeFare*, PaxTypeFare*>& ptFareMap,
                                           PricingTrx& trx,
                                           uint16_t mileageSrc,
                                           uint16_t mileageDest) const
{
  // First see if we've already done this one
  const auto ptFareIter = ptFareMap.find(&paxTypeFare);

  if (ptFareIter != ptFareMap.end())
    return ptFareIter->second;

  Fare* newFare = nullptr;
  Fare* oldFare = paxTypeFare.fare();

  // Clone the fare
  std::map<Fare*, Fare*>::iterator fareIter = fareMap.find(oldFare);

  if (LIKELY(fareIter == fareMap.end()))
  {
    if (UNLIKELY(trx.isAltDates() && trx.getTrxType() == PricingTrx::MIP_TRX))
    {
      if (!checkAltDateIsNOTEffectiveFare(&paxTypeFare))
      {
        newFare = oldFare->clone(dataHandle, false);
        fareMap[oldFare] = newFare;
      }
      else
      {
        if (UNLIKELY(
                (trx.diagnostic().diagnosticType() == Diagnostic981) &&
                (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ADRELFARES")))
        {
          accumulateDataDiag981(&paxTypeFare);
        }
        return nullptr;
      }
    }
    else
    {
      // This fare hasnt been cloned yet, clone it and add it
      newFare = oldFare->clone(dataHandle, false);
      fareMap[oldFare] = newFare;
      // Global direction???
    }
  }
  else
  {
    // We already cloned this fare, use the pointer in the map
    newFare = fareIter->second;
  }

  // Clone the PaxTypeFare
  PaxTypeFare* newPTFare = paxTypeFare.clone(dataHandle, false, &fareMarket, newFare);

  newPTFare->setFare(newFare);

  ptFareMap[&paxTypeFare] = newPTFare;

  // Copy the PaxTypeFareRuleData elements
  auto& ptfRuleData = *newPTFare->paxTypeFareRuleDataMap();
  auto copyPtfRD = [&](uint16_t cat)
  {
    PaxTypeFare::PaxTypeFareAllRuleData* oldAllRuleData =
        ptfRuleData[cat].load(std::memory_order_relaxed);
    // Only copy if there's rule data
    if (oldAllRuleData != nullptr)
    {
      PaxTypeFare::PaxTypeFareAllRuleData* allRuleData = nullptr;
      dataHandle.get(allRuleData);
      // Make a memberwise copy of the rule data and save it
      *allRuleData = *oldAllRuleData;
      ptfRuleData[cat].store(allRuleData, std::memory_order_relaxed);

      // Copy fareRuleData
      allRuleData->fareRuleData = copyPaxTypeFareRuleData(dataHandle,
                                                          allRuleData->fareRuleData,
                                                          fareMarket,
                                                          fareMap,
                                                          ptFareMap,
                                                          trx,
                                                          mileageSrc,
                                                          mileageDest);

      // Copy gfrRuleData
      allRuleData->gfrRuleData = copyPaxTypeFareRuleData(dataHandle,
                                                         allRuleData->gfrRuleData,
                                                         fareMarket,
                                                         fareMap,
                                                         ptFareMap,
                                                         trx,
                                                         mileageSrc,
                                                         mileageDest);
    }
  };

  for (uint16_t catNo = 0; catNo < ptfRuleData.size(); ++catNo)
    copyPtfRD(catNo);

  if (UNLIKELY(newPTFare->isFareByRule() && (mileageDest != mileageSrc) &&
               (newPTFare->fareByRuleInfo().fareInd() == FareByRuleItemInfo::SPECIFIED_E)))
  {
    // It is OK to use the first itin.
    // CalcMoney only needs to know itin type (Inter/Dom) for rounding purpose
    Itin* itin = nullptr;

    if (trx.isAltDates() && trx.getTrxType() == PricingTrx::MIP_TRX)
    {
      for (Itin* it : trx.itin())
      {
        if (!it)
          continue;

        itin = it;

        if (it->travelSeg().front()->departureDT() == fareMarket.travelDate())
          break;
      }
    }
    else if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.getOptions()->isCarnivalSumOfLocal())
      itin = ShoppingUtil::getItinForFareMarketMIP(trx, &fareMarket);
    else
      itin = trx.itin().front();

    if (!itin)
      return nullptr;

    const CurrencyCode& currency = paxTypeFare.currency();
    const FareByRuleItemInfo& fbrItemInfo = paxTypeFare.fareByRuleInfo();

    FareCollectorOrchestrator::recalculateCat25FareAmount(
        trx, itin, fareMarket, currency, fbrItemInfo, mileageDest, *newPTFare);
  }

  return newPTFare;
}

void
FareCollectorOrchestrator::recalculateCat25FareAmount(PricingTrx& trx,
                                                      Itin* itin,
                                                      FareMarket& fareMarket,
                                                      const CurrencyCode& currencyCode,
                                                      const FareByRuleItemInfo& fbrItemInfo,
                                                      uint16_t mileage,
                                                      PaxTypeFare& paxTypeFare) const
{
  FareByRuleController fbrController(trx, this, *itin, fareMarket);
  fbrController.initFareMarketCurrency();

  MoneyAmount fareAmount = 0;

  if (currencyCode == fbrItemInfo.specifiedCur1())
  {
    fareAmount = fbrItemInfo.specifiedFareAmt1();
  }
  else if (currencyCode == fbrItemInfo.specifiedCur2())
  {
    fareAmount = fbrItemInfo.specifiedFareAmt2();
  }

  if (fareAmount != 0)
  {
    fareAmount = FareByRuleController::calculateFareAmtPerMileage(fareAmount, mileage);
    FareInfo* fareInfoClone = paxTypeFare.fare()->fareInfo()->clone(trx.dataHandle());

    if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED && !trx.getOptions()->isRtw())
    {
      fareInfoClone->fareAmount() = fareAmount / 2;
    }
    else
    {
      fareInfoClone->fareAmount() = fareAmount;
    }

    paxTypeFare.fare()->setFareInfo(fareInfoClone);
    fbrController.calcMoney().setRT(paxTypeFare.isRoundTrip());
    fbrController.calcMoney().setCurrency(fareInfoClone->currency());
    fbrController.calcMoney().setFareAmount(fareAmount);
    fbrController.calcMoney().putIntoPTF(paxTypeFare, *fareInfoClone);

    if (paxTypeFare.isDiscounted())
    {
      DiscountedFareController discController(trx, *itin, fareMarket);
      discController.calcAmount(paxTypeFare, paxTypeFare.discountInfo());
      discController.calcMoney().putIntoPTF(paxTypeFare, *fareInfoClone);
    }
  }
}

//----------------------------------------------------------------------------
// copyPaxTypeFareRuleData()
//---------------------------------------------------------------------------
PaxTypeFareRuleData*
FareCollectorOrchestrator::copyPaxTypeFareRuleData(DataHandle& dataHandle,
                                                   PaxTypeFareRuleData* from,
                                                   FareMarket& dest,
                                                   std::map<Fare*, Fare*>& fareMap,
                                                   std::map<PaxTypeFare*, PaxTypeFare*>& ptFareMap,
                                                   PricingTrx& trx,
                                                   uint16_t mileageSrc,
                                                   uint16_t mileageDest) const
{
  if (from != nullptr)
  {
    PaxTypeFareRuleData* const to = from->clone(dataHandle);

    // If the rule data has a base fare, copy it
    if (LIKELY(from->baseFare() != nullptr))
    {
      PaxTypeFare* newPTFare = copyPaxTypeFare(
          dataHandle, *(from->baseFare()), dest, fareMap, ptFareMap, trx, mileageSrc, mileageDest);
      if (LIKELY(newPTFare != nullptr))
        to->baseFare() = newPTFare;
    }
    return to;
  }
  else
    return nullptr;
}

void
FareCollectorOrchestrator::setupFareMarket(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (UNLIKELY(!fareMarket.paxTypeCortege().empty()))
  {
    // This fareMarket was setup already
    return;
  }

  if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
  {
    fareMarket.travelDate() = trx.adjustedTravelDate(fareMarket.travelDate());
  }

  if (fareMarket.getGlobalDirection() == GlobalDirection::XX)
  {
    //    TSELatencyData metrics(trx, "FCO GLOBAL DIR");
    GlobalDirectionFinderV2Adapter::process(trx, fareMarket);
  }

  // If the global direction is still not set fail the faremarket
  if (fareMarket.getGlobalDirection() == GlobalDirection::XX)
  {
    fareMarket.failCode() = ErrorResponseException::INVALID_ROUTING_OR_SEG_BETWEEN_CO_TERM;
    return;
  }

  GoverningCarrier governingCarrier(&trx);

  if (UNLIKELY(fareMarket.governingCarrier().empty()))
  {
    //      TSELatencyData metrics( trx, "FCO GOV CXR");

    LOG4CXX_ERROR(logger, "Governing carrier should be already set in FCO.");
    governingCarrier.process(fareMarket);
  }

  if (UNLIKELY(!governingCarrier.processCarrierPreference(fareMarket)))
  {
    LOG4CXX_DEBUG(logger, "Failed governing carrier preferences check");
    LOG4CXX_ERROR(logger,
                  "Preferences for carrier '" << fareMarket.governingCarrier()
                                              << "' have not been found. Transaction aborted.");

    fareMarket.failCode() = ErrorResponseException::NEED_PREFERRED_CARRIER;
    return;
  }
  else
  {
    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic270))
    {
      DCFactory* factory = DCFactory::instance();
      DiagCollector* diagPtr = factory->create(trx);
      DiagCollector& diag = *diagPtr;

      diag.enable(Diagnostic270);

      if (diag.isActive())
      {
        diag << fareMarket;
        diag.flushMsg();
      }
    }

    LOG4CXX_DEBUG(logger, "Passed governing carrier preferences check");
  }

  const bool isDummyFM = fareMarket.useDummyFare();
  ShoppingTrx* shoppingTrx = dynamic_cast<ShoppingTrx*>(&trx);

  if (shoppingTrx == nullptr && !isDummyFM)
  {
    LimitationOnIndirectTravel lit(trx, itin);

    if (!lit.validateFareComponent(fareMarket))
    {
      if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
      {
        std::string marketString = FareMarketUtil::getDisplayString(fareMarket);
        LOG4CXX_DEBUG(logger,
                      "FareCollectorOrchestrator - Limitations failed FareMarket [" << marketString
                                                                                    << "]");
      }

      LOG4CXX_INFO(logger, "lit.validateFareComponent returned false");

      fareMarket.failCode() = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
      return;
    }
    else
    {
      if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
      {
        std::string marketString = FareMarketUtil::getDisplayString(fareMarket);
        LOG4CXX_DEBUG(logger,
                      "FareCollectorOrchestrator - Limitations passed FareMarket [" << marketString
                                                                                    << "]");
      }

      setCxrPref(trx, fareMarket);
    }
  }

  if (LIKELY(!isDummyFM))
  {
    SalesRestrictionByNation saleRestr;
    if (UNLIKELY(saleRestr.isRestricted(itin, fareMarket, trx)))
    {
      LOG4CXX_INFO(logger, " Pricing and Ticketing Restricted By Government");
      fareMarket.failCode() = ErrorResponseException::PRICING_REST_BY_GOV;
      return;
    }
  }

  if (UNLIKELY(!fareMarket.initialize(trx)))
  {
    fareMarket.failCode() = ErrorResponseException::UNKNOWN_EXCEPTION;
    return;
  }

  {
    //      TSELatencyData metrics( trx, "FCO FLT TRACKER");

    FlightTracker flightTracker(trx);
    flightTracker.process(fareMarket); // will move to FVO when routing will be ready.
  }
}

void
FareCollectorOrchestrator::allFareMarketSteps(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
    const
{
  TSELatencyData metrics(trx, "FCO AFM");
  if (UNLIKELY(fareMarket.serviceStatus().isSet(FareMarket::FareCollector)))
    return;

  {
    TSELatencyData metrics(trx, "FCO AFM DIAG451");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    diagnostic451Step(trx, itin, fareMarket);
  }

  if (UNLIKELY(fareMarket.failCode() != ErrorResponseException::NO_ERROR))
    return;

  {
    TSELatencyData metrics(trx, "FCO AFM PUBFARES");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    publishedFaresStep(trx, itin, fareMarket);
  }

  if (UNLIKELY(fareMarket.failCode() != ErrorResponseException::NO_ERROR))
    return;

  ShoppingTrx* const shoppingTrx = dynamic_cast<ShoppingTrx*>(&trx);
  const bool isSolTrx = shoppingTrx && shoppingTrx->isSumOfLocalsProcessingEnabled();

  if (retrieveFbrFares(trx, isSolTrx, fareMarket.getFmTypeSol()))
  {
    TSELatencyData metrics(trx, "FCO AFM FBR");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();

    findFareByRuleFares(trx, itin, fareMarket);
  }

  if (UNLIKELY(fareMarket.failCode() != ErrorResponseException::NO_ERROR))
    return;

  {
    TSELatencyData metrics(trx, "FCO AFM DISC");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    findDiscountedFares(trx, itin, fareMarket);
  }

  if (UNLIKELY(fareMarket.failCode() != ErrorResponseException::NO_ERROR))
    return;

  bool retrNegFares = retrieveNegFares(trx);
  if (retrNegFares)
  {
    TSELatencyData metrics(trx, "FCO AFM NEG");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    findNegotiatedFares(trx, itin, fareMarket);
  }

  if (UNLIKELY(fareMarket.failCode() != ErrorResponseException::NO_ERROR))
    return;

  if (LIKELY(trx.getTrxType() != PricingTrx::FF_TRX))
  {
    TSELatencyData metrics(trx, "FARE FOCUS");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    FareFocusRuleValidator ffRuleValidator(trx, fareMarket);
    ffRuleValidator.process();

    if (UNLIKELY(fareMarket.failCode() != ErrorResponseException::NO_ERROR))
      return;
  }

  fareMarket.setSpecialRtgFound(findSpecialRouting(fareMarket));

  invalidateContinentBasedFares(trx, fareMarket);

  verifySpecificFareBasis(trx, fareMarket);

  NationCode originNation;

  if (shoppingTrx != nullptr)
  {
    originNation = ItinUtil::originNation(*(shoppingTrx->journeyItin()));
  }
  else
  {
    originNation = ItinUtil::originNation(itin);
  }

  if (!ItinUtil::applyMultiCurrencyPricing(&trx, itin) ||
      fareMarket.geoTravelType() == GeoTravelType::Transborder)
  {
    LOG4CXX_DEBUG(logger, "FCO PERFORMING SINGLE CURRENCY SELECTION ");
    TSELatencyData metrics(trx, "FCO AFM CURR");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    selectCurrencyStep(trx, itin, fareMarket);
  }

  if (fallback::fixSpanishLargeFamilyForSRFE(&trx))
  {
    if (UNLIKELY(SLFUtil::isSpanishFamilyDiscountApplicable(trx)))
    {
      const Percent percentDiscount = SLFUtil::getDiscountPercent(*trx.getOptions());
      SLFUtil::applySpanishFamilyDisountToFares(trx, fareMarket, percentDiscount);
    }
  }

  if (shoppingTrx != nullptr && shoppingTrx->isAltDates())
  {
    initAltDates(*shoppingTrx, fareMarket);
    for (std::vector<PaxTypeFare*>::const_iterator i = fareMarket.allPaxTypeFare().begin();
         i != fareMarket.allPaxTypeFare().end();
         ++i)
    {
      for (VecMap<DatePair, uint8_t>::const_iterator j = (*i)->altDateStatus().begin();
           j != (*i)->altDateStatus().end();
           j++)
      {
        DatePair myPair = j->first;
        uint8_t ret = 'P';
        ShoppingAltDateUtil::checkEffExpDate(*shoppingTrx, **i, ret);
        if (ret == 'P')
        {
          return;
        }
        else
        {
          (*i)->setAltDateStatus(myPair, ret);
        }
      }
    }
  }
}

void
FareCollectorOrchestrator::allFareMarketStepsFareDisplay(PricingTrx& trx,
                                                         Itin& itin,
                                                         FareMarket& fareMarket) const
{
  if (fareMarket.serviceStatus().isSet(FareMarket::FareCollector))
  {
    LOG4CXX_ERROR(logger, "FareMarket already processed.");
    return;
  }

  TSELatencyData tld(trx, "FCO PUB FARES");
  publishedFaresStepFareDisplay(trx, itin, fareMarket);
  if (fareMarket.failCode() != ErrorResponseException::NO_ERROR)
    return;

  FareDisplayTrx* fdTrx;
  if (FareDisplayUtil::getFareDisplayTrx(&trx, fdTrx))
  {
    if (fdTrx->needFbrFareCtrl())
    {
      LOG4CXX_DEBUG(
          logger,
          "begin findFareByRuleFares()   allPTF size = " << fareMarket.allPaxTypeFare().size());
      TSELatencyData tld(trx, "FCO FBR FARES");
      findFareByRuleFares(trx, itin, fareMarket);
    }
    if (fareMarket.failCode() != ErrorResponseException::NO_ERROR)
      return;

    if (fdTrx->needDiscFareCtrl())
    {
      LOG4CXX_DEBUG(
          logger,
          "begin findDiscountedFares()   allPTF size = " << fareMarket.allPaxTypeFare().size());
      TSELatencyData tld(trx, "FCO DISC FARES");
      findDiscountedFares(trx, itin, fareMarket);
    }
    if (fareMarket.failCode() != ErrorResponseException::NO_ERROR)
      return;

    if (fdTrx->needNegFareCtrl())
    {
      LOG4CXX_DEBUG(
          logger,
          "begin findNegotiatedFares()   allPTF size = " << fareMarket.allPaxTypeFare().size());
      TSELatencyData tld(trx, "FCO NEG FARES");
      findNegotiatedFares(trx, itin, fareMarket);
    }
    if (fareMarket.failCode() != ErrorResponseException::NO_ERROR)
      return;

    TSELatencyData metrics(trx, "FARE FOCUS");
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
    FareFocusRuleValidator ffRuleValidator(trx, fareMarket);
    ffRuleValidator.process();

    if (fareMarket.failCode() != ErrorResponseException::NO_ERROR)
      return;

  } // endif - optional FareControllers

  { // scope for latenct data
    TSELatencyData tld(trx, "FCO SELECT CURRENCY");
    selectCurrencyStep(trx, itin, fareMarket);
  }
}

void
FareCollectorOrchestrator::setCxrPref(PricingTrx& trx, FareMarket& fm)
{
  std::vector<TravelSeg*>& tvlSegs = fm.travelSeg();

  if (tvlSegs.size() <= 1)
    return;

  if (tvlSegs.front()->segmentType() == Arunk || tvlSegs.front()->segmentType() == Surface ||
      tvlSegs.back()->segmentType() == Arunk || tvlSegs.back()->segmentType() == Surface)
  {
    // Check Carrier Preference table to see if surface is allowed
    // at fare break point
    const CarrierPreference* cxrPref =
        trx.dataHandle().getCarrierPreference(fm.governingCarrier(), fm.travelDate());

    if (cxrPref != nullptr && cxrPref->noSurfaceAtFareBreak() == 'Y')
    {
      fm.setBreakIndicator(true);
    }
  }
}

//----------------------------------------------------------------------------
// checkBrandedFareDataStep()
//----------------------------------------------------------------------------
void
FareCollectorOrchestrator::checkBrandedFareDataStep(PricingTrx& trx,
                                                    Itin& itin,
                                                    FareMarket& fareMarket)
{
  PricingOptions* secondAction = trx.getOptions();

  if (fareMarket.allPaxTypeFare().empty() ||
      fareMarket.failCode() != ErrorResponseException::NO_ERROR)
    return;

  if (fareMarket.useDummyFare() || (secondAction == nullptr))
    return;

  DiagManager diag(trx);
  if (UNLIKELY(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "BRAND"))
    diag.activate(Diagnostic499);

  BrandedFareValidator val(trx, diag);
  val.excludeBrandedFareValidation(fareMarket);
}

//----------------------------------------------------------------------------
// checkFaresStep()
//---------------------------------------------------------------------------
void
FareCollectorOrchestrator::checkFaresStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  if (UNLIKELY(fareMarket.useDummyFare()))
    return;

  PricingOptions* secondAction = trx.getOptions();
  if (UNLIKELY(secondAction == nullptr))
    return;

  bool fareValid = false;

  if (!fareMarket.allPaxTypeFare().empty() &&
      fareMarket.failCode() == ErrorResponseException::NO_ERROR)
  {
    std::vector<PaxTypeFare*>::iterator ptIter = fareMarket.allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator ptIterEnd = fareMarket.allPaxTypeFare().end();

    bool isFareTypePricing = secondAction->isFareFamilyType();
    FareTypeMatcher fareTypeMatch(trx);

    for (; ptIter != ptIterEnd; ++ptIter)
    {
      PaxTypeFare* paxTypeFare = *ptIter;

      if (UNLIKELY(secondAction->isNormalFare() && !paxTypeFare->isNormal()))
      {
        // Need NL fare but paxtypefare is not Normal
        paxTypeFare->fare()->setFailBySecondaryActionCode();
        continue;
      }

      // WPT/{NL,EX,IT} entry
      if (UNLIKELY(isFareTypePricing && !fareTypeMatch(paxTypeFare)))
      {
        paxTypeFare->fare()->setFailBySecondaryActionCode();
        continue;
      }

      if (UNLIKELY(trx.isFlexFare()))
      {
        if (paxTypeFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
        {
          paxTypeFare->getMutableFlexFaresValidationStatus()->setStatus<flexFares::PRIVATE_FARES>(
              true);
          if (trx.getFlexFaresTotalAttrs().getTariffType() == TariffType::Published)
          {
            paxTypeFare->fare()->setFailBySecondaryActionCode();
            continue;
          }
        }
        else
        {
          paxTypeFare->getMutableFlexFaresValidationStatus()->setStatus<flexFares::PUBLIC_FARES>(
              true);
          if (trx.getFlexFaresTotalAttrs().getTariffType() == TariffType::Private)
          {
            paxTypeFare->fare()->setFailBySecondaryActionCode();
            continue;
          }
        }
      }
      else
      {
        if (secondAction->isPublishedFares() &&
            paxTypeFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
        {
          // Need PL fare but fare Private
          paxTypeFare->fare()->setFailBySecondaryActionCode();
          continue;
        }

        if (secondAction->isPrivateFares() &&
            paxTypeFare->tcrTariffCat() == RuleConst::PUBLIC_TARIFF)
        {
          // PV needs but fare is Public
          paxTypeFare->fare()->setFailBySecondaryActionCode();
          continue;
        }
      }

      fareValid = true;
    }
    if (!fareValid)
    {
      fareMarket.failCode() = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
    }
  }
  return;
}

//----------------------------------------------------------------------------
// process(AltPricingTrx)
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::process(AltPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, " - Entered process(AltPricingTrx)");
  return process((PricingTrx&)trx);
}

//----------------------------------------------------------------------------
// process(StructuredRuleTrx)
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::process(StructuredRuleTrx& trx)
{
  LOG4CXX_DEBUG(logger, " - Entered process(StructuredRuleTrx)");
  return process(static_cast<PricingTrx&>(trx));
}

//----------------------------------------------------------------------------
// process(NoPNRPricingTrx)
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::process(NoPNRPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, " - Entered process(NoPNRPricingTrx)");
  return process((PricingTrx&)trx);
}

//----------------------------------------------------------------------------
// process(RexPricingTrx)
//---------------------------------------------------------------------------

FareCollectorOrchestrator::IsKeepFare::IsKeepFare(const PaxTypeFare& excItinFare,
                                                  const DateTime& retrievalDate)
  : _excItinFare(excItinFare), _retrievalDate(retrievalDate)
{
  if (_excItinFare.actualPaxType())
    _excItinFarePaxType = _excItinFare.actualPaxType()->paxType();
}

FareCollectorOrchestrator::IsExpndKeepFare::IsExpndKeepFare(const PaxTypeFare& excItinFare,
                                                            const DateTime& retrievalDate)
  : _excItinFare(excItinFare), _retrievalDate(retrievalDate)
{
  if (_excItinFare.actualPaxType())
    _excItinFarePaxType = _excItinFare.actualPaxType()->paxType();
}

bool
FareCollectorOrchestrator::IsKeepFare::
operator()(PaxTypeFare* fare)
{
  if ((fare->retrievalInfo() != nullptr) && (fare->retrievalInfo()->_date != _retrievalDate))
    return false;

  if ((fabs(fare->fareAmount() - _excItinFare.fareAmount()) < EPSILON) &&
      (fare->fareClass() == _excItinFare.fareClass()) &&
      (fare->vendor() == _excItinFare.vendor()) && (fare->carrier() == _excItinFare.carrier()) &&
      (fare->fareTariff() == _excItinFare.fareTariff()) &&
      (fare->ruleNumber() == _excItinFare.ruleNumber()) &&
      (fare->currency() == _excItinFare.currency()) &&
      (fare->directionality() == _excItinFare.directionality()) &&
      (fare->owrt() == _excItinFare.owrt()) &&
      (fare->actualPaxType() && fare->actualPaxType()->paxType() == _excItinFarePaxType))
  {
    if (_excItinFare.isDiscounted())
    {
      if (_excItinFare.isNegotiated())
        return fare->isNegotiated();
      else if (_excItinFare.isFareByRule())
        return fare->isFareByRule();
      else
        return fare->isDiscounted();
    }
    else if (_excItinFare.isNegotiated())
      return fare->isNegotiated();
    else if (_excItinFare.isFareByRule())
      return fare->isFareByRule();
    else if (_excItinFare.fare()->isIndustry())
      return fare->fare()->isIndustry();
    else if (_excItinFare.isConstructed())
      return fare->isConstructed();
    else
      return true;
  }

  return false;
}

bool
FareCollectorOrchestrator::IsExpndKeepFare::
operator()(PaxTypeFare* fare)
{
  if ((fare->retrievalInfo() != nullptr) && (fare->retrievalInfo()->_date != _retrievalDate))
    return false;

  return fare->directionality() == _excItinFare.directionality() &&
         fare->vendor() == _excItinFare.vendor() && fare->carrier() == _excItinFare.carrier() &&
         fare->fareTariff() == _excItinFare.fareTariff() &&
         fare->ruleNumber() == _excItinFare.ruleNumber();
}

bool
FareCollectorOrchestrator::process(RexShoppingTrx& trx)
{
  const TSELatencyData metrics(trx, "FCO PROCESS EXC ITIN");
  LOG4CXX_DEBUG(logger, " - Entered process(RexShoppingTrx)");

  if ((trx.trxPhase() == RexPricingTrx::REPRICE_EXCITIN_PHASE) ||
      (trx.trxPhase() == RexPricingTrx::MATCH_EXC_RULE_PHASE))
  {
    return process(static_cast<RexBaseTrx&>(trx));
  }
  return true;
}

namespace
{
typedef FareMarket::RetrievalInfo RetrievalInfo;

struct UpdateTravelDate : std::unary_function<FareMarket*, void>
{
  UpdateTravelDate(RexPricingTrx& rexTrx) : _rexTrx(rexTrx) {}
  void operator()(FareMarket* fareMarket)
  {
    if (TrxUtil::isAdjustRexTravelDateEnabled() &&
        fareMarket->travelDate() < fareMarket->retrievalDate())
      fareMarket->travelDate() = _rexTrx.adjustedTravelDate();
    if (_rexTrx.applyReissueExchange())
      fareMarket->rexBaseTrx() = &_rexTrx;
  }

protected:
  RexPricingTrx& _rexTrx;
};
}

bool
FareCollectorOrchestrator::process(RexPricingTrx& trx)
{
  if ((trx.trxPhase() == RexPricingTrx::REPRICE_EXCITIN_PHASE) ||
      (trx.trxPhase() == RexPricingTrx::MATCH_EXC_RULE_PHASE))
  {
    return process(static_cast<RexBaseTrx&>(trx));
  }

  if (trx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    processNewItins(trx);
  }

  return true;
}

void
FareCollectorOrchestrator::cloneNewFMsWithFareApplication(RexPricingTrx& trx, Itin& newItin)
{
  std::vector<std::vector<FareMarket*>> dupFareMarketsVec;
  dupFareMarketsVec.resize(newItin.fareMarket().size());

  std::vector<FareMarket*>::iterator fmIter = newItin.fareMarket().begin();
  const std::vector<FareMarket*>::const_iterator fmIterEnd = newItin.fareMarket().end();

  std::vector<std::vector<FareMarket*>>::iterator dupFMVecIter = dupFareMarketsVec.begin();
  const std::vector<std::vector<FareMarket*>>::const_iterator dupFMVecIterEnd =
      dupFareMarketsVec.end();

  bool isEmptyCommenceDate =
      (trx.exchangeItin().front()->travelCommenceDate() == DateTime::emptyDate());

  for (; fmIter != fmIterEnd; ++fmIter, ++dupFMVecIter)
  {
    if (trx.exchangeItin().front()->doNotUseForPricing(**fmIter, newItin))
      (**fmIter).setBreakIndicator(true);

    bool duplicatedFareMarket =
        trx.needRetrieveKeepFare() && keepFareAloneInFM(trx, *dupFMVecIter, **fmIter);

    if (trx.needRetrieveHistoricalFare())
    {
      retrieveOtherThenKeepFareFacade(trx,
                                      *dupFMVecIter,
                                      **fmIter,
                                      trx.originalTktIssueDT(),
                                      FareMarket::RetrievHistorical,
                                      duplicatedFareMarket);
    }

    if (trx.needRetrieveTvlCommenceFare() && !isEmptyCommenceDate)
    {
      retrieveOtherThenKeepFareFacade(trx,
                                      *dupFMVecIter,
                                      **fmIter,
                                      trx.exchangeItin().front()->travelCommenceDate(),
                                      FareMarket::RetrievTvlCommence,
                                      duplicatedFareMarket);
    }

    if (trx.needRetrieveLastReissueFare())
    {
      retrieveOtherThenKeepFareFacade(trx,
                                      *dupFMVecIter,
                                      **fmIter,
                                      trx.lastTktReIssueDT(),
                                      FareMarket::RetrievLastReissue,
                                      duplicatedFareMarket);
    }

    if (trx.needRetrieveCurrentFare())
    {
      FareMarket::FareRetrievalFlags flags = FareMarket::RetrievCurrent;
      if (trx.needRetrieveTvlCommenceFare() && isEmptyCommenceDate)
        flags = (FareMarket::FareRetrievalFlags)(flags | FareMarket::RetrievTvlCommence);

      retrieveOtherThenKeepFareFacade(
          trx, *dupFMVecIter, **fmIter, trx.currentTicketingDT(), flags, duplicatedFareMarket);
    }

    if (!trx.needRetrieveHistoricalFare())
    {
      if (trx.isFareMarketNeededForMinFares(**fmIter))
      {
        retrieveOtherThenKeepFareFacade(trx,
                                        *dupFMVecIter,
                                        **fmIter,
                                        trx.originalTktIssueDT(),
                                        FareMarket::RetrievHistorical,
                                        duplicatedFareMarket);

        if (trx.excTrxType() == PricingTrx::AR_EXC_TRX && trx.isPlusUpCalculationNeeded())
          ExchangeUtil::avoidValidationOfCategoriesInMinFares(trx, (**fmIter).allPaxTypeFare());

        dupFMVecIter->back()->setBreakIndicator(true);
        newItin.addFareMarketJustForRexPlusUps(*fmIter);
      }
    }
  }

  newItin.fareMarket().clear();
  dupFMVecIter = dupFareMarketsVec.begin();
  for (; dupFMVecIter != dupFMVecIterEnd; ++dupFMVecIter)
  {
    std::copy(dupFMVecIter->begin(), dupFMVecIter->end(), back_inserter(newItin.fareMarket()));
  }
}

namespace
{
template <typename T>
inline void
purgeFares_if(std::vector<PaxTypeFare*>& container, const T& condition)
{
  container.erase(std::remove_if(container.begin(), container.end(), condition), container.end());
}

template <typename T>
inline void
purgeFares_if(FareMarket& fm, const T& condition)
{
  purgeFares_if(fm.allPaxTypeFare(), condition);

  for (PaxTypeBucket& cortege : fm.paxTypeCortege())
    purgeFares_if(cortege.paxTypeFare(), condition);
}

struct IsNotValidForHip
{
  bool operator()(const PaxTypeFare* ptf) { return ptf->isFareByRule() || ptf->isNegotiated(); }
};
}

void
FareCollectorOrchestrator::removeFaresInvalidForHip(RexBaseTrx& trx)
{
  for (size_t itinIndex = 0; itinIndex < trx.itin().size(); itinIndex++)
  {
    trx.setItinIndex(itinIndex);
    Itin& currItin = *trx.itin()[itinIndex];

    std::vector<FareMarket*>::iterator fmIt = currItin.fareMarket().begin();
    for (; fmIt != currItin.fareMarket().end(); ++fmIt)
    {
      FareMarket& fm = **fmIt;
      if (!currItin.isFareMarketJustForRexPlusUps(&fm))
        continue;

      purgeFares_if(fm, IsNotValidForHip());

      if (trx.excTrxType() == PricingTrx::AR_EXC_TRX && trx.isPlusUpCalculationNeeded())
        ExchangeUtil::avoidValidationOfCategoriesInMinFares(trx, fm.allPaxTypeFare());
    }
  }
}

bool
FareCollectorOrchestrator::process(RefundPricingTrx& trx)
{
  if ((trx.trxPhase() == RexPricingTrx::REPRICE_EXCITIN_PHASE) ||
      (trx.trxPhase() == RexPricingTrx::MATCH_EXC_RULE_PHASE))
    return process(static_cast<RexBaseTrx&>(trx));

  if (trx.trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE)
  {
    trx.prepareNewFareMarkets();

    for_each(trx.exchangeItin().front()->fareComponent().begin(),
             trx.exchangeItin().front()->fareComponent().end(),
             bind2nd(std::mem_fun(&FareCompInfo::updateFMMapping), trx.newItin().front()));

    bool& tls = trx.dataHandle().useTLS() = true;
    BooleanFlagResetter resetter(tls);

    // Find duplicate FareMarkets
    std::map<std::string, std::vector<FareMarket*>> fmMap;

    collectFaresStep(trx, fmMap);

    updateFaresRetrievalInfo(trx);

    releaseCheckSortMarkStep(trx, fmMap);

    return true;
  }

  return false;
}

bool
FareCollectorOrchestrator::process(RexBaseTrx& trx)
{
  const TSELatencyData metrics(trx,
                               trx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE
                                   ? "FCO PROCESS EXC ITIN"
                                   : "FCO PROCESS NEW ITIN");
  LOG4CXX_DEBUG(logger, " - Entered process(RexPricingTrx)");

  const bool oldStat = trx.isAnalyzingExcItin();
  // exchange itin and dataHandle TicketDate and fareApplicationDT set to original Ticket issue date
  trx.setAnalyzingExcItin(true);

  const DateTime savedTktDate = trx.ticketingDate();
  if (trx.previousExchangeDateFare())
  {
    trx.dataHandle().setTicketDate(trx.previousExchangeDT());
    trx.ticketingDate() = trx.previousExchangeDT();
  }

  try
  {
    process(static_cast<PricingTrx&>(trx));
  }
  catch (ErrorResponseException& ex)
  {
  }

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX && trx.isPlusUpCalculationNeeded())
  {
    removeFaresInvalidForHip(trx);
  }

  RetrievalInfo* info =
      RetrievalInfo::construct(trx, trx.fareApplicationDT(), FareMarket::RetrievNone);
  ExcItin& excItin = *trx.exchangeItin().front();
  std::for_each(excItin.fareMarket().begin(),
                excItin.fareMarket().end(),
                ExchangeUtil::SetFaresRetrievalInfo(info));

  if (trx.previousExchangeDateFare())
  {
    trx.ticketingDate() = savedTktDate;
    trx.dataHandle().setTicketDate(savedTktDate);
    trx.previousExchangeDateFare() = false;
  }

  trx.setAnalyzingExcItin(oldStat);
  return true;
}

//----------------------------------------------------------------------------
// process(ExchangePricingTrx)
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::process(ExchangePricingTrx& trx)

{
  LOG4CXX_DEBUG(logger, " - Entered process(ExchangePricingTrx)");
  return process(static_cast<PricingTrx&>(trx));
}

/**
 * Validate fare type pricing request: for fare type pricing request we should
 * have a matching fare type qualifier on file.  If none is coded, then return
 * as data error
 **/
bool
FareCollectorOrchestrator::validateFareTypePricing(PricingTrx& trx)
{
  if (!trx.getOptions()->isFareFamilyType())
    return true;

  // TODO: after we are done implementing the matching logic, there might
  // be more checks

  FareTypeMatcher ftMatch(trx);
  if (ftMatch.fareTypeQualifier().empty())
  {
    return false;
  }

  return true;
}

bool
FareCollectorOrchestrator::keepFareAloneInFM(RexPricingTrx& trx,
                                             std::vector<FareMarket*>& dupFareMarkets,
                                             FareMarket& fareMarket)
{
  const PaxTypeFare* excItinFare = trx.getKeepPtf(fareMarket);
  if (!excItinFare)
    return false;

  const DateTime& retrievalDate = excItinFare->retrievalDate();

  if ((retrievalDate == trx.currentTicketingDT() && trx.needRetrieveCurrentFare()) ||
      (retrievalDate == trx.exchangeItin().front()->travelCommenceDate() &&
       trx.needRetrieveTvlCommenceFare()) ||
      (retrievalDate == trx.originalTktIssueDT() && trx.needRetrieveHistoricalFare()))
    return false;

  dupFareMarkets.push_back(&fareMarket);

  // if already processed for other itin duplicate since it cannot be shared
  bool needDuplicateSharedFm = (fareMarket.retrievalInfo() != nullptr &&
                                (fareMarket.retrievalDate() != retrievalDate ||
                                 fareMarket.retrievalFlag() ^ FareMarket::RetrievNone));

  if (trx.getTrxType() == PricingTrx::MIP_TRX && needDuplicateSharedFm)
  {
    duplicateAndOverrideFareMarket(
        trx, dupFareMarkets, fareMarket, retrievalDate, FareMarket::RetrievNone);
  }
  else
  {
    fareMarket.retrievalInfo() =
        RetrievalInfo::construct(trx, retrievalDate, FareMarket::RetrievNone);
  }

  return true;
}

void
FareCollectorOrchestrator::retrieveOtherThenKeepFare(RexPricingTrx& trx,
                                                     std::vector<FareMarket*>& dupFareMarkets,
                                                     FareMarket& fareMarket,
                                                     const DateTime& date,
                                                     FareMarket::FareRetrievalFlags flags,
                                                     bool& currFareMarketUpdated)
{
  if (!currFareMarketUpdated)
  {
    currFareMarketUpdated = true;
    fareMarket.retrievalInfo() = RetrievalInfo::construct(trx, date, flags);

    dupFareMarkets.push_back(&fareMarket);
  }
  else
  {
    duplicateFareMarket(trx, dupFareMarkets, fareMarket, date, flags);
  }
}

void
FareCollectorOrchestrator::retrieveOtherThenKeepFareFacade(RexPricingTrx& trx,
                                                           std::vector<FareMarket*>& dupFareMarkets,
                                                           FareMarket& fareMarket,
                                                           const DateTime& date,
                                                           FareMarket::FareRetrievalFlags flags,
                                                           bool& currFareMarketUpdated)
{
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
    retrieveOtherThenKeepFareForMip(
        trx, dupFareMarkets, fareMarket, date, flags, currFareMarketUpdated);
  else
    retrieveOtherThenKeepFare(trx, dupFareMarkets, fareMarket, date, flags, currFareMarketUpdated);
  return;
}

FareMarket&
FareCollectorOrchestrator::duplicateFareMarket(RexPricingTrx& trx,
                                               std::vector<FareMarket*>& dupFareMarkets,
                                               const FareMarket& origFM,
                                               const DateTime& date,
                                               FareMarket::FareRetrievalFlags flags)
{
  FareMarket* fm = nullptr;
  trx.dataHandle().get(fm);

  origFM.clone(*fm);

  RetrievalInfo* info = RetrievalInfo::construct(trx, date, flags);
  fm->retrievalInfo() = info;

  dupFareMarkets.push_back(fm);
  return *fm;
}

void
FareCollectorOrchestrator::updateFaresRetrievalInfo(RexPricingTrx& trx)
{
  for (size_t itinIndex = 0; itinIndex < trx.newItin().size(); ++itinIndex)
  {
    trx.setItinIndex(itinIndex);
    Itin& newItin = *trx.curNewItin();

    std::vector<FareMarket*>::const_iterator fmIter = newItin.fareMarket().begin();
    const std::vector<FareMarket*>::const_iterator fmIterEnd = newItin.fareMarket().end();

    for (; fmIter != fmIterEnd; ++fmIter)
    {
      FareMarket& fareMarket = **fmIter;

      if (fareMarket.retrievalFlag() == FareMarket::RetrievNone)
      {
        setKeepFareRetrievalInfo(trx, fareMarket);
      }
      else
      {
        std::for_each(fareMarket.allPaxTypeFare().begin(),
                      fareMarket.allPaxTypeFare().end(),
                      ExchangeUtil::SetPaxTypeFareRetrievalInfo(fareMarket.retrievalInfo()));
      }
    }
  }
}

void
FareCollectorOrchestrator::updateFaresRetrievalInfo(RexBaseTrx& trx)
{
  for (size_t itinIndex = 0; itinIndex < trx.newItin().size(); ++itinIndex)
  {
    trx.setItinIndex(itinIndex);
    Itin& newItin = *trx.curNewItin();

    typedef std::vector<FareMarket*>::const_iterator It;
    for (It fmIter = newItin.fareMarket().begin(); fmIter != newItin.fareMarket().end(); ++fmIter)
    {
      FareMarket& fareMarket = **fmIter;
      std::for_each(fareMarket.allPaxTypeFare().begin(),
                    fareMarket.allPaxTypeFare().end(),
                    ExchangeUtil::SetPaxTypeFareRetrievalInfo(fareMarket.retrievalInfo()));
    }
  }
}

const PaxTypeFare*
FareCollectorOrchestrator::getKeepFareForNewFareMarket(const RexPricingTrx& trx,
                                                       const FareMarket* fareMarket) const
{
  const RexPricingTrx::NewItinKeepFareMap& keepFareMap = trx.newItinKeepFares();
  RexPricingTrx::NewItinKeepFareMap::const_iterator curr = keepFareMap.begin();
  for (; curr != keepFareMap.end(); ++curr)
    if (curr->second == fareMarket)
      return curr->first;
  return nullptr;
}

void
FareCollectorOrchestrator::setKeepFareRetrievalInfo(RexPricingTrx& trx, FareMarket& fareMarket)
    const
{
  const PaxTypeFare* excItinFare = getKeepFareForNewFareMarket(trx, &fareMarket);
  if (excItinFare)
  {
    const DateTime& retrievalDate = excItinFare->retrievalDate();

    forEachKeepFare(fareMarket.allPaxTypeFare().begin(),
                    fareMarket.allPaxTypeFare().end(),
                    *excItinFare,
                    ExchangeUtil::SetPaxTypeFareRetrievalInfo(
                        RetrievalInfo::construct(trx, retrievalDate, FareMarket::RetrievKeep)));
  }

  std::pair<RexPricingTrx::ExpndKeepMapI, RexPricingTrx::ExpndKeepMapI> range =
      trx.expndKeepMap().equal_range(&fareMarket);

  for (; range.first != range.second; ++range.first)
  {
    const PaxTypeFare& excPtf = *range.first->second;
    forEachExpndKeepFare(fareMarket.allPaxTypeFare().begin(),
                         fareMarket.allPaxTypeFare().end(),
                         excPtf,
                         ExchangeUtil::SetPaxTypeFareRetrievalInfo(RetrievalInfo::construct(
                             trx, excPtf.retrievalDate(), FareMarket::RetrievExpndKeep)));
  }
}

void
FareCollectorOrchestrator::updateKeepFareRetrievalInfo(RexPricingTrx& trx, FareMarket& fareMarket)
    const
{
  if (fareMarket.retrievalFlag() == FareMarket::RetrievNone)
    return;

  const PaxTypeFare* excItinFare = getKeepFareForNewFareMarket(trx, &fareMarket);
  if (excItinFare)
  {
    const DateTime& retrievalDate = excItinFare->retrievalDate();

    if (historicalCommence(fareMarket.retrievalFlag(), retrievalDate, trx))
      forEachKeepFare(
          fareMarket.allPaxTypeFare().begin(),
          fareMarket.allPaxTypeFare().end(),
          *excItinFare,
          ExchangeUtil::UpdateRetrievalInfo(trx, retrievalDate, FareMarket::RetrievKeep));
  }

  std::pair<RexPricingTrx::ExpndKeepMapI, RexPricingTrx::ExpndKeepMapI> range =
      trx.expndKeepMap().equal_range(&fareMarket);

  for (; range.first != range.second; ++range.first)
  {
    const PaxTypeFare& excPtf = *range.first->second;

    if (historicalCommence(fareMarket.retrievalFlag(), excPtf.retrievalDate(), trx))
      forEachExpndKeepFare(fareMarket.allPaxTypeFare().begin(),
                           fareMarket.allPaxTypeFare().end(),
                           excPtf,
                           ExchangeUtil::UpdateRetrievalInfo(
                               trx, excPtf.retrievalDate(), FareMarket::RetrievExpndKeep));
  }
}

bool
FareCollectorOrchestrator::historicalCommence(const FareMarket::FareRetrievalFlags& flags,
                                              const DateTime& retrievDt,
                                              const RexPricingTrx& trx) const
{
  return ((flags & FareMarket::RetrievHistorical) && retrievDt == trx.originalTktIssueDT()) ||
         ((flags & FareMarket::RetrievTvlCommence) &&
          retrievDt == trx.exchangeItin().front()->travelCommenceDate());
}

bool
FareCollectorOrchestrator::checkAltDateIsNOTEffectiveFare(PaxTypeFare* ptFare)
{
  if (LIKELY(ptFare != nullptr))
  {
    Fare* oldFare = ptFare->fare();
    if (LIKELY(oldFare))
    {
      bool fareValid = false;
      const DateTime& travelDT = ptFare->fareMarket()->travelDate();

      fareValid = oldFare->fareInfo()->effInterval().isEffective(travelDT);

      if (ptFare->isFareByRule() && fareValid)
      {
        if (LIKELY(!ptFare->isSpecifiedFare()))
          return !ptFare->baseFare()->fare()->fareInfo()->effInterval().isEffective(travelDT);
      }
      return !fareValid;
    }
  }
  return true;
}

bool
FareCollectorOrchestrator::altDateCAT15Validation(PricingTrx& trx,
                                                  FareMarket& dest,
                                                  PaxTypeFare* newPTFare)
{
  if (LIKELY(newPTFare != nullptr))
  {
    if (PaxTypeFareRuleData* ptfRuleData =
            newPTFare->paxTypeFareRuleData(RuleConst::SALE_RESTRICTIONS_RULE))
    {
      const GeneralFareRuleInfo* generalFareRuleInfo =
          (dynamic_cast<const GeneralFareRuleInfo*>(ptfRuleData->categoryRuleInfo()));

      if (LIKELY(generalFareRuleInfo))
      {
        const DateTime& effecDate = generalFareRuleInfo->effDate();
        const DateTime& expDate = generalFareRuleInfo->expireDate();

        bool validDate = false;
        const DateTime& travelDT = dest.travelDate();
        validDate = effecDate > travelDT || travelDT > expDate;

        if (UNLIKELY(validDate))
        {
          TSELatencyData tld(trx, "FCO ALT DATE CAT15 PREVALIDATE");
          Itin* itin = nullptr;

          bool flightMatch = false;
          std::vector<Itin*>::iterator iI = trx.itin().begin();
          std::vector<Itin*>::iterator iE = trx.itin().end();
          for (; iI != iE; ++iI)
          {
            if (*iI)
            {
              itin = *iI;

              if (itin->travelSeg().front()->departureDT() == dest.travelDate())
              {
                flightMatch = true;
                break;
              }
            }
          }

          if (!flightMatch)
            return false;

          // Reset the PaxTypeFareRuleData elements
          auto& ptfRData = *newPTFare->paxTypeFareRuleDataMap();
          auto resetCat = [&ptfRData](unsigned int cat)
          {
            auto allRData = ptfRData[cat].load(std::memory_order_relaxed);
            if (allRData == nullptr)
              return;
            allRData->chkedRuleData = false;
            allRData->chkedGfrData = false;
            allRData->fareRuleData = nullptr;
            allRData->gfrRuleData = nullptr;
          };
          resetCat(RuleConst::ELIGIBILITY_RULE);
          resetCat(RuleConst::SALE_RESTRICTIONS_RULE);

          newPTFare->setCategoryProcessed(RuleConst::ELIGIBILITY_RULE, false);
          newPTFare->setCategoryProcessed(RuleConst::SALE_RESTRICTIONS_RULE, false);

          RuleControllerWithChancelor<FareMarketRuleController> fmrc(PreValidation, &trx);
          fmrc.validate(trx, *itin, *newPTFare);
        }
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return true;
    }
  }
  return false;
}

int
FareCollectorOrchestrator::removeNotEffPTFares(PricingTrx& trx,
                                               std::vector<PaxTypeFare*>& ptFareVec)
{
  try
  {
    std::vector<PaxTypeFare*>::iterator riter;

    riter = std::stable_partition(ptFareVec.begin(),
                                  ptFareVec.end(),
                                  std::not1(std::ptr_fun(checkAltDateIsNOTEffectiveFare)));

    if (riter != ptFareVec.end())
    {
      int total = ptFareVec.size();
      int released = std::distance(riter, ptFareVec.end());

      if (UNLIKELY((trx.diagnostic().diagnosticType() == Diagnostic981) &&
                   (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ADRELFARES")))
      {
        std::vector<PaxTypeFare*>::iterator accIter = riter;
        for (; accIter != ptFareVec.end(); ++accIter)
          accumulateDataDiag981(*accIter);
      }
      ptFareVec.erase(riter, ptFareVec.end());

      LOG4CXX_DEBUG(logger,
                    " ALTDATE CAT15 PREVALIDATION: "
                        << " RELEASED " << released << " of " << total << " PAXTYPEFARES\n ");
      return total - released;
    }
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(logger, "Exception: " << e.what());
    return 0;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "Unknown Exception");
    return 0;
  }
  return 0;
}

namespace
{
struct LessByGD : public std::binary_function<Mileage, Mileage, bool>
{
  bool operator()(const Mileage* l, const Mileage* r) const
  {
    return l->globaldir() < r->globaldir();
  }

  // lint --e{1509}
} lessByGD;

struct EqualByGD : public std::binary_function<Mileage, Mileage, bool>
{
  bool operator()(const Mileage* l, const Mileage* r) const
  {
    return l->globaldir() == r->globaldir();
  }

  // lint --e{1509}
} equalByGD;
}

bool
FareCollectorOrchestrator::isGIValid(NoPNRPricingTrx& noPNRTrx)
{
  std::map<const TravelSeg*, const std::string>& directionWarningMap = noPNRTrx.GIWarningMap();
  bool isError = false;

  tse::Itin* itin = noPNRTrx.itin().front();
  std::vector<FareMarket*>& fareMarkets = itin->fareMarket();
  std::vector<FareMarket*>::iterator iter = fareMarkets.begin();
  std::vector<FareMarket*>::iterator iterEnd = fareMarkets.end();

  for (; iter != iterEnd; ++iter)
  {
    std::vector<TravelSeg*>& travelSegs = (*iter)->travelSeg();
    std::vector<TravelSeg*>::iterator segIt = travelSegs.begin();
    std::vector<TravelSeg*>::iterator segItEnd = travelSegs.end();
    if (travelSegs.size() > 1)
      continue;

    for (; segIt != segItEnd; ++segIt)
    {
      TravelSeg* tvlSeg = *segIt;
      std::set<GlobalDirection> validGlobalDirs;
      GlobalDirection globalDir, requestedGD;

      GlobalDirectionFinderV2Adapter::getGlobalDirection(
          &noPNRTrx, (*iter)->travelDate(), *tvlSeg, globalDir);

      bool isRequestedGDValid =
          strToGlobalDirection(requestedGD, noPNRTrx.globalDirectionOverride()[tvlSeg]);

      std::vector<Mileage*> tpms;
      const Loc* origLoc = tvlSeg->origin();
      const Loc* destLoc = tvlSeg->destination();

      tpms = noPNRTrx.dataHandle().getMileage(
          (!origLoc->city().empty() ? origLoc->city() : origLoc->loc()),
          (!destLoc->city().empty() ? destLoc->city() : destLoc->loc()),
          noPNRTrx.travelDate());

      // Get Unique data
      std::vector<Mileage*> alternateGDs(tpms);
      std::sort(alternateGDs.begin(), alternateGDs.end(), lessByGD);
      alternateGDs.erase(std::unique(alternateGDs.begin(), alternateGDs.end(), equalByGD),
                         alternateGDs.end());

      std::vector<Mileage*>::const_iterator i = alternateGDs.begin();
      for (; i != alternateGDs.end(); ++i)
      {
        if ((*i)->globaldir() != GlobalDirection::ZZ)
          validGlobalDirs.insert((*i)->globaldir());
      }

      if (!validGlobalDirs.empty())
      {
        std::set<GlobalDirection>::iterator i(validGlobalDirs.begin()), j(validGlobalDirs.end()),
            end(validGlobalDirs.end());
        j = find(i, end, requestedGD);

        // we have multiple (more than 1) valid GD for travel segment
        // and the requested GD is not among them
        // we stop the processing and display error message
        if (validGlobalDirs.size() > 1 && ((requestedGD != GlobalDirection::ZZ && j == end) ||
                                           (requestedGD == GlobalDirection::ZZ)))
        {
          std::string error =
              "REQUIRE GI " + (tvlSeg)->origin()->loc() + (tvlSeg)->destination()->loc() + " - TRY";
          for (; i != end; ++i)
          {
            error = error + " " + *(globalDirectionToStr(*i));
          }

          throw ErrorResponseException(ErrorResponseException::INVALID_GLOBAL_DIRECTION_REQUESTED,
                                       error.c_str());
        }

        // we have only one valid GD for travel segment and the requested GD is not the one
        // we use the valid GD and display warning message
        if ((validGlobalDirs.size() == 1 && j == end && requestedGD != GlobalDirection::ZZ) ||
            !isRequestedGDValid)
        {
          std::string warning = "INVALID GLOBAL INDICATOR REQUESTED " + (tvlSeg)->origin()->loc() +
                                (tvlSeg)->destination()->loc() + " - GLOBAL " +
                                *(globalDirectionToStr(globalDir)) + " USED";

          directionWarningMap.insert(std::pair<TravelSeg*, std::string>(tvlSeg, warning));
          (*iter)->setGlobalDirection(globalDir);
        }
        else if (requestedGD != GlobalDirection::ZZ)
        {
          (*iter)->setGlobalDirection(requestedGD);
        }
      }
      else
      {
        if (requestedGD != GlobalDirection::ZZ && requestedGD != globalDir)
        {
          std::string warning = "INVALID GLOBAL INDICATOR REQUESTED " + (tvlSeg)->origin()->loc() +
                                (tvlSeg)->destination()->loc() + " - GLOBAL " +
                                *(globalDirectionToStr(globalDir)) + " USED";

          directionWarningMap.insert(std::pair<TravelSeg*, std::string>(tvlSeg, warning));
          (*iter)->setGlobalDirection(globalDir);
        }
      }
    }
  }

  return !isError;
}

bool
FareCollectorOrchestrator::isDSSCallNeeded(FareDisplayTrx& trx)
{
  if ((trx.isShortRequest()) || (trx.getRequest()->requestType() == "MP"))
  {
    return false;
  }
  return true;
}

bool
FareCollectorOrchestrator::isSpanishFamilyDiscountAlreadyApplied(const PaxTypeFare* ptf)
{
  if (ptf && ptf->isFareByRule())
  {
    const FBRPaxTypeFareRuleData* fbrPaxTypeFare = ptf->getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (fbrPaxTypeFare)
    {
      const FareByRuleItemInfo* fbrItemInfo =
          dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFare->ruleItemInfo());

      if (fbrItemInfo && (fbrItemInfo->fareInd() == FareByRuleController::CALCULATED ||
                          fbrItemInfo->fareInd() ==
                              FareByRuleController::SUBTRACT_SPECIFIED_FROM_BASE_CALC_PERCENTAGE) &&
          fbrPaxTypeFare->isSpanishResidence() && PaxTypeUtil::isSpanishPaxType(ptf->fcasPaxType()))
        return true;
    }
  }
  return false;
}

void
FareCollectorOrchestrator::applyDiscountPercentToFare(MoneyAmount discountPercent,
                                                      MoneyAmount& fareAmount)
{
  const MoneyAmount discount = fareAmount * discountPercent;
  fareAmount -= discount;
  // For the unlikely case that the percent discount is >= 1.00
  if (fareAmount < 0.0)
    fareAmount = 0.0;
}

bool
FareCollectorOrchestrator::findSpecialRouting(const FareMarket& fareMarket,
                                              const bool checkEmptyRouting)
{
  std::vector<PaxTypeBucket>::const_iterator ptcI = fareMarket.paxTypeCortege().begin();
  std::vector<PaxTypeBucket>::const_iterator ptcIEnd = fareMarket.paxTypeCortege().end();

  for (; ptcI != ptcIEnd; ++ptcI)
  {
    if (true == findSpecialRouting(ptcI->paxTypeFare(), checkEmptyRouting))
    {
      return true;
    }
  }

  return false;
}

bool
FareCollectorOrchestrator::findSpecialRouting(const std::vector<PaxTypeFare*>& paxTypeFares,
                                              const bool checkEmptyRouting)
{
  std::vector<PaxTypeFare*>::const_iterator paxTypeFareI = paxTypeFares.begin();
  std::vector<PaxTypeFare*>::const_iterator paxTypeFareIEnd = paxTypeFares.end();

  for (; paxTypeFareI != paxTypeFareIEnd; ++paxTypeFareI)
  {
    const PaxTypeFare& paxTypeFare = **paxTypeFareI;

    if (RoutingUtil::isSpecialRouting(paxTypeFare, checkEmptyRouting))
    {
      return true;
    }
  }

  return false;
}

void
FareCollectorOrchestrator::performDiagnostic981(PricingTrx& trx,
                                                std::map<PaxTypeFare*, PaxTypeFare*>& ptFareMap)
{
  if ((trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ADRELFARES") &&
      (trx.diagnostic().diagnosticType() == Diagnostic981))
  {
    DCFactory* factory = DCFactory::instance();
    Diag981Collector* diag981 = dynamic_cast<Diag981Collector*>(factory->create(trx));
    diag981->enable(Diagnostic981);
    diag981->setAdrelfares(true);
    diag981->showRemFaresForDatePair(ptFareMap);
    diag981->flushMsg();
  }
}

void
FareCollectorOrchestrator::performDiagnostic981(PricingTrx& trx, FareMarket* fm)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic981 && fm != nullptr &&
      (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "ADRELFARES"))
  {
    DCFactory* factory = DCFactory::instance();
    Diag981Collector* diag981 = dynamic_cast<Diag981Collector*>(factory->create(trx));
    diag981->enable(Diagnostic981);
    (*diag981) << *fm;
    diag981->flushMsg();
  }
}

void
FareCollectorOrchestrator::accumulateDataDiag981(PaxTypeFare* ptFare)
{
  accDiag981ptFaresMap[ptFare] = ptFare;
}

std::map<PaxTypeFare*, PaxTypeFare*>&
FareCollectorOrchestrator::getAccaccumulateDataDiag981()
{
  return accDiag981ptFaresMap;
}

void
FareCollectorOrchestrator::clearAccaccumulateDataDiag981()
{
  accDiag981ptFaresMap.clear();
}

//----------------------------------------------------------------------------
// process(RexExchangeTrx)
//---------------------------------------------------------------------------
bool
FareCollectorOrchestrator::process(RexExchangeTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entered FareCollectorOrchestrator::process(RexExchangeTrx)");

  return process(static_cast<RexPricingTrx&>(trx));
}

// TODO:
void
FareCollectorOrchestrator::retrieveOtherThenKeepFareForMip(RexPricingTrx& trx,
                                                           std::vector<FareMarket*>& dupFareMarkets,
                                                           FareMarket& fareMarket,
                                                           const DateTime& date,
                                                           FareMarket::FareRetrievalFlags flags,
                                                           bool& currFareMarketUpdated)
{
  // was already processed for other itin?
  bool needDuplicateSharedFm =
      fareMarket.retrievalInfo() != nullptr &&
      (fareMarket.retrievalDate() != date || fareMarket.retrievalFlag() ^ flags);

  if (!currFareMarketUpdated && needDuplicateSharedFm)
  {
    currFareMarketUpdated = true;
    duplicateAndOverrideFareMarket(trx, dupFareMarkets, fareMarket, date, flags);
  }
  else
  {
    retrieveOtherThenKeepFare(trx, dupFareMarkets, fareMarket, date, flags, currFareMarketUpdated);
  }
}

void
FareCollectorOrchestrator::duplicateAndOverrideFareMarket(RexPricingTrx& trx,
                                                          std::vector<FareMarket*>& dupFareMarkets,
                                                          const FareMarket& origFM,
                                                          const DateTime& date,
                                                          FareMarket::FareRetrievalFlags flags)
{
  LOG4CXX_DEBUG(logger,
                "duplicateAndOverrideFareMarket-Duplicating FM ("
                    << tse::FareMarketUtil::getDisplayString(origFM) << ")"
                    << origFM.travelDate().dateToString(DDMMYYYY, "-"));
  FareMarket* fm = nullptr;
  trx.dataHandle().get(fm);

  origFM.clone(*fm);

  RetrievalInfo* info = RetrievalInfo::construct(trx, date, flags);
  fm->retrievalInfo() = info;

  // override original fm (at position 0) since it cannot be shared across itins
  if (dupFareMarkets.empty())
    dupFareMarkets.push_back(fm);
  else
    dupFareMarkets[0] = fm;

  trx.fareMarket().push_back(fm);
}

bool
FareCollectorOrchestrator::retrieveNegFares(const PricingTrx& trx)
{
  if (trx.getTrxType() != PricingTrx::REPRICING_TRX)
    return true;

  const RepricingTrx* rpTrx = dynamic_cast<const RepricingTrx*>(&trx);
  return (rpTrx != nullptr) ? rpTrx->retrieveNegFares() : false;
}

bool
FareCollectorOrchestrator::retrieveFbrFares(const PricingTrx& trx,
                                            bool isSolTrx,
                                            FareMarket::SOL_FM_TYPE fmTypeSol) const
{
  if (trx.getTrxType() != PricingTrx::REPRICING_TRX)
  {
    const bool skipFMFbrConstructionSol = isSolTrx && _skipFareByRuleConstructionSolFM;
    const bool skipFMFbrConstruction =
        skipFMFbrConstructionSol && fmTypeSol == FareMarket::SOL_FM_LOCAL;
    return !skipFMFbrConstruction;
  }

  const RepricingTrx* rpTrx = dynamic_cast<const RepricingTrx*>(&trx);
  return (rpTrx != nullptr) ? rpTrx->retrieveFbrFares() : false;
}

void
FareCollectorOrchestrator::splitSharedFareMarketsWithDiffROE(RexExchangeTrx& rexExcTrx)
{
  uint32_t splitted = 0;
  uint32_t markets = 0;
  typedef std::map<int64_t, FareMarket*> SharedMap;
  std::map<FareMarket*, SharedMap> fmMap; // map<shared *fm, map<ROE, splitted *fm>>

  for (size_t itinIndex = 0; itinIndex < rexExcTrx.itin().size(); ++itinIndex)
  {
    DateTime& fmROE = rexExcTrx.newItinROEConversionDate(itinIndex);
    int64_t fmROEBitRep = fmROE.get64BitRep();
    std::vector<FareMarket*>& fareMarkets = rexExcTrx.itin()[itinIndex]->fareMarket();

    std::vector<FareMarket*>::iterator fareMarket = fareMarkets.begin();
    for (; fareMarket != fareMarkets.end(); ++fareMarket)
    {
      if (fmMap.count(*fareMarket) != 0)
      {
        SharedMap& shrMap = fmMap[*fareMarket];

        if (shrMap.count(fmROEBitRep) != 0)
        {
          if (*fareMarket != shrMap[fmROEBitRep])
          {
            *fareMarket = shrMap[fmROEBitRep];
          }
        }
        else
        {
          FareMarket* newFM = nullptr;
          cloneFareMarket(rexExcTrx, *fareMarket, newFM);
          fmMap[*fareMarket][fmROEBitRep] = newFM;
          ++splitted;
        }
        ++markets;
      }
      else
      {
        fmMap[*fareMarket][fmROEBitRep] = *fareMarket;
      }
    }
  }

  LOG4CXX_DEBUG(logger,
                "Identified " << splitted << " splitted of " << markets
                              << " total shared FareMarkets");
}

void
FareCollectorOrchestrator::cloneFareMarket(RexExchangeTrx& rexExcTrx,
                                           FareMarket*& origFM,
                                           FareMarket*& newFM)
{
  newFM = nullptr;
  rexExcTrx.dataHandle().get(newFM);

  origFM->clone(*newFM);

  RetrievalInfo* info = RetrievalInfo::construct(
      rexExcTrx, origFM->retrievalInfo()->_date, origFM->retrievalInfo()->_flag);
  newFM->retrievalInfo() = info;

  origFM = newFM;
  rexExcTrx.fareMarket().push_back(newFM);
}

void
FareCollectorOrchestrator::invalidateSomeFareMarkets(RexPricingTrx& trx)
{
  int16_t maxFlownSegOrder = findMaxSegmentOrderForFlownInExc(trx.exchangeItin().front());
  for (size_t itinIndex = 0; itinIndex < trx.itin().size(); itinIndex++) // TODO: index by newItin()
  {
    trx.setItinIndex(itinIndex);
    Itin& newItin = *trx.curNewItin();

    std::vector<FareMarket*>::const_iterator fmIter = newItin.fareMarket().begin();
    const std::vector<FareMarket*>::const_iterator fmIterEnd = newItin.fareMarket().end();
    bool allPermutationsRequireCurrent;
    bool allPermutationsRequireNotCurrent;

    for (; fmIter != fmIterEnd; ++fmIter)
    {
      FareMarket& fareMarket = **fmIter;

      if (fareMarket.changeStatus() == FL)
      {
        int16_t lastFlownSeg = fareMarket.travelSeg().back()->segmentOrder();
        if (lastFlownSeg > maxFlownSegOrder)
        {
          continue;
        }
        allPermutationsRequireCurrent = trx.allPermutationsRequireCurrentForFlown();
        allPermutationsRequireNotCurrent = trx.allPermutationsRequireNotCurrentForFlown();
      }
      else
      {
        allPermutationsRequireCurrent = trx.allPermutationsRequireCurrentForUnflown();
        allPermutationsRequireNotCurrent = trx.allPermutationsRequireNotCurrentForUnflown();
      }
      if ((allPermutationsRequireCurrent &&
           !(fareMarket.retrievalFlag() & FareMarket::RetrievCurrent)) ||
          (allPermutationsRequireNotCurrent &&
           (fareMarket.retrievalFlag() == FareMarket::RetrievCurrent)))
        fareMarket.setBreakIndicator(true);
    }
  }
}

int16_t
FareCollectorOrchestrator::findMaxSegmentOrderForFlownInExc(const ExcItin* excItin) const
{
  int16_t maxFlownSegOrder(0);
  std::vector<FareMarket*>::const_iterator fmIter = excItin->fareMarket().begin();
  const std::vector<FareMarket*>::const_iterator fmIterEnd = excItin->fareMarket().end();
  for (; fmIter != fmIterEnd; ++fmIter)
  {
    FareMarket* fareMarket = *fmIter;
    if (fareMarket->changeStatus() == FL)
      if (fareMarket->travelSeg().back()->segmentOrder() > maxFlownSegOrder)
        maxFlownSegOrder = fareMarket->travelSeg().back()->segmentOrder();
  }
  return maxFlownSegOrder;
}

void
FareCollectorOrchestrator::processNewItins(RexPricingTrx& trx)
{
  const bool oldStat = trx.isAnalyzingExcItin();
  trx.setAnalyzingExcItin(false);

  // to keep duplicated fare markets for all new itins
  // allItinsDupFareMarketsVec[a][b][c] means :
  // a-new itin index , b-new itin fareMarket, c - fare Market dupl num
  std::vector<std::vector<std::vector<FareMarket*>>> allItinsDupFareMarketsVec;
  allItinsDupFareMarketsVec.resize(trx.newItin().size());

  (static_cast<RexPricingRequest*>(trx.getRequest()))->setNewAndExcAccCodeCorpId();

  // Setup all FM with retrievalInfo(retrievalDate and retrieval Flag basing on war of tags info
  for (size_t itinIndex = 0; itinIndex < trx.newItin().size(); itinIndex++)
  {
    trx.setItinIndex(itinIndex);
    Itin& newItin = *trx.curNewItin();

    std::vector<std::vector<FareMarket*>> dupFareMarketsVec;
    processNewItin(trx, newItin, dupFareMarketsVec);

    // save duplicate FM  for this itin, all FM have retrievalInfo set to proper date and status
    // flag
    allItinsDupFareMarketsVec.push_back(dupFareMarketsVec);
  } // end for itinIndex

  bool& tls = trx.dataHandle().useTLS();
  tls = true;
  BooleanFlagResetter bfr(tls);

  // Find duplicate FareMarkets
  std::map<std::string, std::vector<FareMarket*>> fmMap;
  collectFaresStep(trx, fmMap);

  updateFaresRetrievalInfo(trx);
  invalidateSomeFareMarkets(trx);

  for (size_t itinIndex = 0; itinIndex < trx.newItin().size(); itinIndex++)
  {
    trx.setItinIndex(itinIndex);
    std::vector<std::vector<FareMarket*>>& dupFareMarketsVec = allItinsDupFareMarketsVec[itinIndex];
    Itin& newItin = *trx.curNewItin();
    updateNewItinRelatedFareMarkets(trx, newItin, dupFareMarketsVec);
  }

  if (trx.diagnostic().diagnosticType() != AllFareDiagnostic ||
      (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "ALLFARES" &&
       trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "INFO"))
  {
    removeFaresInvalidForHip(trx);
    // due to routing bug, 'M' indicator in fare calc
    // depends on fare set inside fare market rather than single fare
    // second bug: occasioanl dissappearing keep fare between 200/499
    // we have to turn it off :-(
    //            removeFaresWithNoApplication(trx);
  }

  trx.setItinIndex(0);
  releaseCheckSortMarkStep(trx, fmMap);

  if (trx.exchangeItin().front()->fareMarket().front()->changeStatus() == FL)
    trx.filterFareMarketByFlownFareAppl();

  trx.setAnalyzingExcItin(oldStat);
  trx.logFaresInfo();
}

void
FareCollectorOrchestrator::processNewItin(RexPricingTrx& trx,
                                          Itin& newItin,
                                          std::vector<std::vector<FareMarket*>>& dupFareMarketsVec)
{
  for (FareMarket* fm : newItin.fareMarket())
  {
    if (!trx.getOptimizationMapper().isFareMarketAllowed(fm))
    {
      fm->failCode() = ErrorResponseException::FARE_MARKET_NOT_AVAILABLE_FOR_REPRICE;
    }
  }

  dupFareMarketsVec.resize(newItin.fareMarket().size());
  std::vector<std::vector<FareMarket*>>::iterator dupFMVecIter = dupFareMarketsVec.begin();

  cloneNewFMsWithFareApplication(trx, newItin);

  std::for_each(newItin.fareMarket().begin(), newItin.fareMarket().end(), UpdateTravelDate(trx));
}

void
FareCollectorOrchestrator::updateNewItinRelatedFareMarkets(
    RexPricingTrx& trx, Itin& newItin, std::vector<std::vector<FareMarket*>>& dupFareMarketsVec)
{
  std::vector<std::vector<FareMarket*>>::iterator dupFMVecIter = dupFareMarketsVec.begin();

  if (trx.needRetrieveKeepFare() &&
      (trx.needRetrieveHistoricalFare() || trx.needRetrieveTvlCommenceFare()))
  {
    for (FareMarket* fm : newItin.fareMarket())
      updateKeepFareRetrievalInfo(trx, *fm);
  }
}

void
FareCollectorOrchestrator::invalidateContinentBasedFares(PricingTrx& trx, FareMarket& fareMarket)
    const
{
  if (LIKELY(!RtwUtil::isRtw(trx)))
    return;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == AllFareDiagnostic &&
               (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALLFARES" ||
                trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO")))
    return;

  const std::vector<AirlineAllianceCarrierInfo*>& aaci =
      trx.dataHandle().getAirlineAllianceCarrier(fareMarket.governingCarrier());

  if (!aaci.empty() && aaci.front()->genericAllianceCode() == ONE_WORLD_ALLIANCE)
    purgeFares_if(
        fareMarket,
        ValidateContinentBased(trx, ItinUtil::countContinents(trx, fareMarket, *aaci.front())));
}

void
FareCollectorOrchestrator::verifySpecificFareBasis(PricingTrx& trx, FareMarket& fareMarket) const
{
  if (!fareMarket.isRequestedFareBasisInfoUseful())
    return;

  for (PaxTypeFare* ptf : fareMarket.allPaxTypeFare())
  {
    TravelSeg::RequestedFareBasis rfb;

    if (ptf->actualPaxType())
      rfb.passengerCode = ptf->actualPaxType()->paxType();

    rfb.vendor = ptf->vendor();
    rfb.carrier = ptf->carrier();
    if (ptf->fareTariff() >= 0)
      rfb.tariff = TypeConvert::valueToString((uint32_t)ptf->fareTariff());

    rfb.rule = ptf->ruleNumber();
    rfb.amount = ptf->fareAmount();

    if (!fareMarket.isRequestedFareBasisValid(rfb, *ptf, trx))
      ptf->setRequestedFareBasisInvalid();
    else if (ptf->fare()->isIndustry())
    {
      bool found = false;

      for (TravelSeg* ts : fareMarket.travelSeg())
      {
        if (ts->requestedFareBasis().size())
        {
          if (!found)
          {
            found = true;
            ptf->setAllowedChangeFareBasisBkgCode(
                ts->requestedFareBasis().front().fareBasisCode[0]);
          }
          else if (ts->requestedFareBasis().front().fareBasisCode[0] !=
                   ptf->getAllowedChangeFareBasisBkgCode())
          {
            ptf->setRequestedFareBasisInvalid();
            break;
          }
        }
      }
    }
  }
}

template <typename Buckets>
void
FareCollectorOrchestrator::setUpNegFaresPostValidation(const FareMarket& fareMarket,
                                                       const Buckets& buckets,
                                                       const PricingTrx& trx,
                                                       const std::vector<Itin*>& owningItins,
                                                       std::set<PaxTypeFare*>& validNegFares,
                                                       bool& doNegFaresPostValidation) const
{
  TSE_ASSERT(!owningItins.empty()); // FareMarket has to be in some Itin
  doNegFaresPostValidation = !buckets.empty() && owningItins.size() == 1;
  if (doNegFaresPostValidation)
    collectValidNegFares(
        buckets,
        boost::bind(validateCarrier, _1, boost::cref(*owningItins.front()), boost::cref(trx)),
        validNegFares);
}

} // tse namespace
