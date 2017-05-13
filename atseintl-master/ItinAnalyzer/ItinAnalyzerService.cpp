//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "ItinAnalyzer/ItinAnalyzerService.h"

#include "ATAE/ContentServices.h"
#include "Common/Assert.h"
#include "Common/CabinType.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/FlownStatusCheck.h"
#include "Common/Global.h"
#include "Common/GlobalDirectionFinder.h"
#include "Common/GoverningCarrier.h"
#include "Common/IAIbfUtils.h"
#include "Common/IntlJourneyUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/MultiTicketUtil.h"
#include "Common/MultiTransportMarkets.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/OBFeesUtils.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "Common/Swapper.h"
#include "Common/TFPUtil.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TNBrands/TrxGeometryCalculator.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "Common/WnSnapUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AltPricingDetailObFeesTrx.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/BrandingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CurrencySelection.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag185Collector.h"
#include "Diagnostic/Diag192Collector.h"
#include "Diagnostic/Diag666Collector.h"
#include "Diagnostic/Diag892Collector.h"
#include "Diagnostic/Diag894Collector.h"
#include "Diagnostic/Diag900Collector.h"
#include "Diagnostic/Diag982Collector.h"
#include "Diagnostic/Diag988Collector.h"
#include "Diagnostic/SimilarItinIADiagCollector.h"
#include "FareDisplay/FDConsts.h"
#include "Fares/BooleanFlagResetter.h"
#include "Fares/FareCurrencySelection.h"
#include "ItinAnalyzer/BrandedFaresDataRetriever.h"
#include "ItinAnalyzer/ExcItinUtil.h"
#include "ItinAnalyzer/FamilyLogicUtils.h"
#include "ItinAnalyzer/FareDisplayAnalysis.h"
#include "ItinAnalyzer/FareMarketBuilder.h"
#include "ItinAnalyzer/FDSuppressFareController.h"
#include "ItinAnalyzer/ItinAnalyzerServiceWrapperSOL.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "ItinAnalyzer/JourneyPrepHelper.h"
#include "ItinAnalyzer/SoldoutCabinValidator.h"
#include "ItinAnalyzer/SoloCarnivalIAUtil.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "MinFares/EPQMinimumFare.h"
#include "Server/TseServer.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <set>

#include <boost/bind.hpp>
#include <boost/next_prior.hpp>

namespace tse
{
FALLBACK_DECL(fallbackGSAMipDifferentialFareFix);
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fallbackForcePricingWayForSFR);
FALLBACK_DECL(fallbackPriceByCabinActivation)
FALLBACK_DECL(spanishResidentFares)
FALLBACK_DECL(fallbackSumUpAvlForMotherItin)
FALLBACK_DECL(fallbackLatamJourneyActivation)
FALLBACK_DECL(setFurthestPointForChildItins)
FALLBACK_DECL(segmentAttributesRefactor)
FALLBACK_DECL(fallbackJumpCabinExistingLogic);
FALLBACK_DECL(fallbackSSDSP2058FMSelection);
FALLBACK_DECL(fallbackRTPricingContextFix);
FALLBACK_DECL(fallbackFixCarrierOriginBasedPricing)
FALLBACK_DECL(fallbackFFGCabinLogicDeffect);
FALLBACK_DECL(fallbackSortItinsByNumFlights);
FALLBACK_DECL(fallbackCheckLegFixFromEnd);

namespace
{
ConfigurableValue<bool>
splitItinsForDomesticOnly("PRICING_SVC", "SPLIT_FAMILY_FOR_DOMESTIC_ONLY", true);
ConfigurableValue<bool>
thruFMOnly("SHOPPING_OPT", "THRU_FM_ONLY", false);
ConfigurableValue<bool>
enableASOLegs("SHOPPING_OPT", "ENABLE_ASO_LEGS", true);
ConfigurableValue<bool>
enableChecksimpleTrip("PRICING_SVC", "CHECK_SIMPLE_TRIP", false);
ConfigurableValue<bool>
showCrossBrandOptionFirst("S8_BRAND_SVC", "SHOW_CROSS_BRAND_OPTION_FIRST", true);
}

Logger
ItinAnalyzerService::_logger("atseintl.ItinAnalyzer.ItinAnalyzerService");

ContentServices ItinAnalyzerService::_contentSvcs;

bool
ItinAnalyzerService::_enableASOLegs(false);

static LoadableModuleRegister<Service, ItinAnalyzerService>
_("libItinAnalyzerService.so");

namespace
{
class IsFMInSet : public std::unary_function<FareMarket*, bool>
{
public:
  IsFMInSet(const std::set<FareMarket*>& fmSet) : _fmSet(fmSet) {}

  bool operator()(FareMarket* fm) const { return _fmSet.find(fm) != _fmSet.end(); }

private:
  const std::set<FareMarket*>& _fmSet;
};

template <typename F>
void handleFailedBrandingService(const PricingTrx& trx, StatusBrandingService status, F onFail)
{
  switch(status)
  {
    case StatusBrandingService::BS_UNAVAILABLE:
      onFail();
      throw ErrorResponseException(ErrorResponseException::BRANDING_SERVICE_UNAVAILABLE);
    case StatusBrandingService::BS_INVALID_RESPONSE:
      onFail();
      throw ErrorResponseException(ErrorResponseException::BRANDING_SERVICE_INVALID_RESPONSE);
    default:
      break;
  }
}

}

bool
ItinAnalyzerService::IsPartButNotWholeDummyFare::
operator()(const FareMarket* fm) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (fm->useDummyFare() || _dummyFCSegs.empty())
    return false;

  std::vector<TravelSeg*>::const_iterator tvlSegI = fm->travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegIEnd = fm->travelSeg().end();

  for (; tvlSegI != tvlSegIEnd; tvlSegI++)
  {
    if (_dummyFCSegs.find(*tvlSegI) != _dummyFCSegs.end())
      return true;
  }
  return false;
}

bool
ItinAnalyzerService::ItinWithoutGoverningCarrier::
operator()(const Itin* itin) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (itin == nullptr || itin->travelSeg().empty())
    return false;

  const std::vector<TravelSeg*>& ts = itin->travelSeg();
  for (const auto tvlSeg : ts)
  {
    if (tvlSeg && tvlSeg->isAir())
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);
      if (airSeg && airSeg->carrier() == _cxrOverride)
        return false;
    }
  }

  return true;
}

ItinAnalyzerService::ItinAnalyzerService(const std::string& name, TseServer& srv)
  : Service(name, srv)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  _splitItinsForDomesticOnly = splitItinsForDomesticOnly.getValue();
  _thruFMOnly = thruFMOnly.getValue();
}

namespace
{
void
diag192(PricingTrx& trx)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic192)
  {
    DCFactory* factory = DCFactory::instance();
    Diag192Collector* diagPtr = dynamic_cast<Diag192Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic192);
    diagPtr->printTrx();
    diagPtr->flushMsg();
  }
}
}

bool
ItinAnalyzerService::initialize(int argc, char* argv[])
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // Set logging services for internal classes
  //_tvlSegAnalysis.logger(_service.logger());
  _enableASOLegs = enableASOLegs.getValue();

  return _contentSvcs.initialize();
}

bool
ItinAnalyzerService::process(MetricsTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::ostringstream& oss = trx.response();

  MetricsUtil::header(oss, "Itin Metrics");
  MetricsUtil::lineItemHeader(oss);

  MetricsUtil::lineItem(oss, MetricsUtil::ITIN_PROCESS);

  return true;
}

bool
ItinAnalyzerService::process(FlightFinderTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN PROCESS");

  LOG4CXX_INFO(_logger, "ItinAnalyzerService::process(FlightFInderTrx)");

  if (trx.avlInS1S3Request())
  {
    if (FlightFinderTrx::STEP_1 == trx.bffStep())
    {
      trx.bffStep() = FlightFinderTrx::STEP_2;
    }

    if (FlightFinderTrx::STEP_3 == trx.bffStep())
    {
      trx.bffStep() = FlightFinderTrx::STEP_4;
    }
  }

  trx.itin().clear();
  trx.itin().push_back(trx.journeyItin());

  setFurthestPoint(trx, trx.journeyItin());

  trx.journeyItin()->simpleTrip() = trx.isSimpleTrip();

  trx.setTravelDate(trx.journeyItin()->travelDate());
  trx.bookingDate() = TseUtil::getBookingDate(trx.journeyItin()->travelSeg());

  if ((trx.isBffReq() &&
       (trx.bffStep() == FlightFinderTrx::STEP_1 || trx.bffStep() == FlightFinderTrx::STEP_3 ||
        trx.bffStep() == FlightFinderTrx::STEP_4 || trx.bffStep() == FlightFinderTrx::STEP_6)) ||
      trx.avlInS1S3Request())
  {
    generateDummySOP(trx);
  }

  // Set itinerary origination and calculations currencies
  FareCalcUtil::getFareCalcConfig(trx); // TODO: check if is it neccessary
  ItinUtil::setItinCurrencies(*(trx.journeyItin()), trx.ticketingDate());

  // Select the ticketing carrier for the transaction
  selectTicketingCarrier(trx);

  if (trx.isFFReq() || trx.bffStep() == FlightFinderTrx::STEP_2 ||
      trx.bffStep() == FlightFinderTrx::STEP_4 || trx.bffStep() == FlightFinderTrx::STEP_5 ||
      trx.bffStep() == FlightFinderTrx::STEP_6)
  {
    // check of Journey
    checkJourneyAndGetCOS(trx);
  }
  // Set retransit points
  setRetransits(trx);

  if (trx.isFFReq() || trx.bffStep() == FlightFinderTrx::STEP_2 ||
      trx.bffStep() == FlightFinderTrx::STEP_4 || trx.bffStep() == FlightFinderTrx::STEP_5 ||
      trx.bffStep() == FlightFinderTrx::STEP_6)
  {
    validateFlightCabin(trx);
  }
  // Check limitations using travelSeg from journey itin
  std::vector<Itin*>::const_iterator itinItem = trx.itin().begin();
  LimitationOnIndirectTravel limits(trx, **itinItem);
  limits.validateJourney();

  // Determine the governing carrier and retrieve the fare
  // market data handles
  getGovCxrAndFareMkt(trx);

  // Group the schedules
  groupSchedules(trx);

  // Set inbound/outbound
  setInboundOutbound(trx);

  // Set the validating carrier
  if (!trx.isValidatingCxrGsaApplicable())
  {
    setValidatingCarrier(trx);
  }
  else
  {
    setFlightFinderValidatingCarrier(trx);
  }
  // Order the segments by their itin containers
  ShoppingUtil::orderSegsByItin(trx);

  setItinRounding(trx);

  setShoppingFareMarketInfo(trx);

  if (false == trx.altDatePairs().empty())
  {
    setUpInfoForAltDateJourneyItin(*trx.journeyItin(), trx.altDatePairs());
  }

  return true;
}

bool
ItinAnalyzerService::process(RepricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN PROCESS");

  LOG4CXX_INFO(_logger, "ItinAnalyzerService::process(RepricingTrx)");

  // Select the ticketing carrier for the transaction
  selectTicketingCarrier(trx);

  // Determine the governing carrier
  Itin* itin = nullptr;
  if (trx.itin().empty())
    return false;

  itin = trx.itin()[0];
  if (itin == nullptr)
    return false;

  getGovCxrAndFareMkt(trx, *itin, true);

  FareMarket* fareMarket = nullptr;
  if (trx.fareMarket().empty())
    return false;

  fareMarket = trx.fareMarket()[0];
  if (fareMarket == nullptr)
    return false;

  if (!trx.carrierOverride().empty())
    fareMarket->governingCarrier() = trx.carrierOverride();

  if (trx.globalDirectionOverride() != GlobalDirection::XX)
    fareMarket->setGlobalDirection(trx.globalDirectionOverride());

  tse::iadetail::JourneyPrepHelper::updateCarrierPreferences(trx);

  prepareForJourney(trx);

  // Assign Fare Maket COS
  setFareMarketCOS(trx);

  // Set currency override
  setCurrencyOverride(trx);

  // Set sortTaxByOrigCity flag
  setSortTaxByOrigCity(trx);

  setValidatingCarrier(trx);

  if (trx.isValidatingCxrGsaApplicable() && !fallback::fallbackGSAMipDifferentialFareFix(&trx))
    processFMsForValidatingCarriers(trx);

  // Set itinerary origination and calculations currencies
  setItinCurrencies(trx);

  return true;
}

void
ItinAnalyzerService::checkFirstDatedSegBeforeDT(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (!trx.isNotExchangeTrx())
    return;

  if (TseUtil::firstDatedSegBeforeDT(trx.travelSeg(), trx.ticketingDate(), trx))
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "TKT DATE MUST BE EQUAL OR EARLIER THAN TVL DATE");
  }
}

void
ItinAnalyzerService::initializeFareMarketsFromItin(
    PricingTrx& trx, ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper)
{
  Itin* itin = trx.itin().front();
  for (FareCompInfo* fc : itin->fareComponent())
  {
    FareMarket* fareMarket = fc->fareMarket();
    if ((fareMarket == nullptr) || fareMarket->travelSeg().empty())
      continue;

    fareMarket->setOrigDestByTvlSegs();
    fareMarket->travelDate() = itin->travelDate();
    fareMarket->fareBasisCode().assign(fc->fareBasisCode().begin(), fc->fareBasisCode().end());

    // Set travel boundary
    baseItinAnalyzerWrapper->setupAndStoreFareMarket(trx, *itin, fareMarket);
  }
}

bool
ItinAnalyzerService::process(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN PROCESS");

  _requestedNumberOfSeats = PaxTypeUtil::totalNumSeats(trx);
  selectProcesing(trx)->initialize(trx);

  LOG4CXX_DEBUG(_logger, "PRICING TRX TICKETING DATE: " << trx.ticketingDate().toSimpleString());
  LOG4CXX_DEBUG(_logger,
                "DATA HANDLE TICKETING DATE: " << trx.dataHandle().ticketDate().toSimpleString());

  if (trx.getTrxType() == PricingTrx::MIP_TRX &&
      trx.getRequest()->isGoverningCarrierOverrideEntry())
  {
    populateGoverningCarrierOverrides(trx);
    removeItinsWithoutGoverningCarrier(trx);
  }
  if (!fallback::fallbackSortItinsByNumFlights(&trx))
    ShoppingUtil::sortItinBySegCount(trx);
  if (PricingTrx::MIP_TRX == trx.getTrxType() && (trx.isFlexFare() || trx.isFlexFarePhase1()))
  {
    Diag982Collector* collector = nullptr;

    if (trx.diagnostic().diagnosticType() == Diagnostic982)
    {
      DCFactory* factory = DCFactory::instance();
      collector = dynamic_cast<Diag982Collector*>(factory->create(trx));
      TSE_ASSERT(collector != nullptr);

      collector->enable(Diagnostic982);
      (*collector) << "FLEX FARE VALIDATION RESULTS:\n";
      (*collector) << "=============================\n";
      collector->flushMsg();
    }

    if (!fallback::fallbackFFGCabinLogicDeffect(&trx) && !trx.isMainFareFFGGroupIDZero())
      validateAllItinsPreferredCabinTypes(trx.itin(), trx, collector);
    else if (fallback::fallbackJumpCabinExistingLogic(&trx) || (!trx.isMainFareFFGGroupIDZero() &&
        trx.getRequest()->getJumpCabinLogic() != JumpCabinLogic::ENABLED))
      validateAllItinsPreferredCabinTypes(trx.itin(), trx, collector);
  }

  if (trx.getRequest()->multiTicketActive())
  {
    MultiTicketUtil::createMultiTicketItins(trx);
  }

  if (trx.getOptions()->isCarnivalSumOfLocal())
  {
    SoloCarnivalIAUtil::generateSubitins(trx);
  }

  if (trx.excTrxType() != PricingTrx::NOT_EXC_TRX)
  {
    BaseExchangeTrx* exchangeTrx = dynamic_cast<BaseExchangeTrx*>(&trx);

    if (exchangeTrx)
      discoverThroughFarePrecedence(trx, *trx.itin().front());
  }
  else if ((trx.getTrxType() == PricingTrx::PRICING_TRX && !trx.noPNRPricing()) ||
           (trx.getTrxType() == PricingTrx::MIP_TRX))
  {
    for (Itin* itin : trx.itin())
    {
      discoverThroughFarePrecedence(trx, *itin);
      failIfThroughFarePrecedenceImpossible(*itin);
    }
  }

  if (trx.getRequest()->originBasedRTPricing())
  {
    if (!addFakeTravelSeg(trx))
      return false;
  }

  tse::iadetail::SoldoutCabinValidator soldoutCabinValidator;
  soldoutCabinValidator.validateSoldouts(trx);

  ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper = selectProcesing(trx);

  ExchangePricingTrx* exchangeTrx =
      (trx.excTrxType() == PricingTrx::PORT_EXC_TRX ? static_cast<ExchangePricingTrx*>(&trx)
                                                    : nullptr);
  if (exchangeTrx)
  {
    Indicator ind = exchangeTrx->newItin().front()->exchangeReissue();
    if (ind == EXCHANGE || ind == REISSUE)
      exchangeTrx->setBsrRoeDate(ind);
    else
      exchangeTrx->dataHandle().setTicketDate(exchangeTrx->purchaseDT());
  }
  else
    setTrxDataHandleTicketDate(trx, trx.ticketingDate());

  tse::iadetail::JourneyPrepHelper::updateCarrierPreferences(trx);

  LOG4CXX_DEBUG(_logger,
                "DATA HANDLE TICKETING DATE: " << trx.dataHandle().ticketDate().toSimpleString());

  // Test restricted currency in currency override transaction
  checkRestrictedCurrencyNation(trx);

  if ((trx.getTrxType() == PricingTrx::MIP_TRX) && !trx.isAltDates())
    FamilyLogicUtils::splitItinFamilies(trx, _splitItinsForDomesticOnly, _requestedNumberOfSeats);

  // set firstUnflownAirSeg in transaction
  trx.firstUnflownAirSeg() = TseUtil::getFirstUnflownAirSeg(trx.travelSeg(), trx.ticketingDate());

  checkFirstDatedSegBeforeDT(trx);

  trx.setTravelDate(TseUtil::getTravelDate(trx.travelSeg()));
  trx.bookingDate() = TseUtil::getBookingDate(trx.travelSeg());

  std::vector<Itin*> allItin = ShoppingUtil::expandEstimatedOptions(trx);
  std::swap(trx.itin(), allItin);

  // Logic to loop through the itin and establish furthest point on the travel segments

  for (Itin* itin : trx.itin())
  {
    setFurthestPoint(trx, itin);
    if (!fallback::setFurthestPointForChildItins(&trx))
    {
      for (const auto& similarItinData : itin->getSimilarItins())
        setFurthestPoint(trx, similarItinData.itin);
    }
  }

  // Set ticketing carrier and geo travel type
  baseItinAnalyzerWrapper->selectTicketingCarrier(trx);

  // Set retransit points
  if (trx.getTrxType() != PricingTrx::MIP_TRX)
  {
    baseItinAnalyzerWrapper->setRetransits(trx);
  }

  // Set openSegAfterDatedSeg
  baseItinAnalyzerWrapper->setOpenSegFlag(trx);

  if (exchangeTrx && exchangeTrx->reqType() == TAG_10_EXCHANGE)
    copyExcFareCompInfoToNewItinFm(*exchangeTrx);

  baseItinAnalyzerWrapper->setATAESchedContent(trx);

  setGeoTravelTypeAndValidatingCarrier(trx);

  if ((trx.getTrxType() == PricingTrx::MIP_TRX) && trx.getRequest()->processVITAData() &&
      !trx.isValidatingCxrGsaApplicable())
  {
    validateTicketingAgreement(trx);
  }

  std::swap(allItin, trx.itin());
  if (trx.getRequest()->multiTicketActive() && trx.isValidatingCxrGsaApplicable() &&
      !trx.multiTicketMap().empty())
  {
    MultiTicketUtil::validateTicketingAgreement(trx);
  }
  else
  {
    removeDeletedItins(trx.itin());
  }

  // Logic to build fare markets

  if (trx.getRequest()->isSFR() && fallback::fallbackForcePricingWayForSFR(&trx))
  {
    if (trx.isMultiPassengerSFRRequestType())
    {
      itinanalyzerutils::removeRedundantFareMarket(*trx.getMultiPassengerFCMapping());
      itinanalyzerutils::initializeFareMarketsFromFareComponents(trx, baseItinAnalyzerWrapper);
    }
    else
    {
      initializeFareMarketsFromItin(trx, baseItinAnalyzerWrapper);
    }
  }
  else
  {
    buildFareMarket(trx);
  }

  if (trx.getRequest()->originBasedRTPricing())
  {
    removeIncorrectFareMarkets(trx);
    updateFakeFM(trx);

    if (!fallback::fallbackFixCarrierOriginBasedPricing(&trx) &&
        !trx.getRequest()->isGoverningCarrierOverrideEntry())
    {
      cloneFakeFareMarkets(trx);
    }
  }

  if (trx.isValidatingCxrGsaApplicable())
    processFMsForValidatingCarriers(trx);

  // Set trip characteristics
  baseItinAnalyzerWrapper->setTripCharacteristics(trx);

  if (trx.snapRequest())
  {
    removeMultiLegFareMarkets(trx);
  }

  // Set international sales indicators
  setIntlSalesIndicator(trx);

  // Check if Journey is active
  baseItinAnalyzerWrapper->checkJourneyActivation(trx);

  // Check if SOLO is active
  baseItinAnalyzerWrapper->checkSoloActivation(trx);

  // Set journey related info :
  // 1. CarrierPref in TravelSegs.
  // 2. Mark Flow FareMarkets (Journey portions).
  // 3. Initialize avail break indicators in FareMarkets.
  prepareForJourney(trx);

  // ATAE content

  baseItinAnalyzerWrapper->setATAEAvailContent(trx);

  // remove higher cabins in MIP
  if (trx.getTrxType() == PricingTrx::MIP_TRX &&
      trx.getRequest()->getJumpCabinLogic() != JumpCabinLogic::ENABLED)
  {
    removeHigherCabins(trx);
  }

  // Set currency override
  setCurrencyOverride(trx);

  // Set sortTaxByOrigCity flag
  setSortTaxByOrigCity(trx);

  // Set itinerary origination and calculations currencies
  setItinCurrencies(trx);

  // Set itinerary origination and calculations currencies for main itinerary (snap request)
  if (trx.snapRequest())
  {
    if (trx.subItinVecFirstCxr().size())
    {
      if (trx.subItinVecFirstCxr().front()->calculationCurrency().empty())
      {
        trx.itin().swap(trx.subItinVecFirstCxr());

        setItinCurrencies(trx);
        trx.itin().swap(trx.subItinVecFirstCxr());
      }
    }
  }
  // Set agent commissions
  setAgentCommissions(trx);

  if (trx.displayOnly())
  {
    // Global direction should always be ZZ
    FareMarket* fm = trx.fareMarket().front();

    if (fm != nullptr)
    {
      fm->setGlobalDirection(GlobalDirection::ZZ);

      //  Governing Carrier comes from the first (and only) AirSeg
      AirSeg* airSeg = dynamic_cast<AirSeg*>(trx.travelSeg().front());
      if (airSeg != nullptr)
      {
        fm->governingCarrier() = airSeg->carrier();
        trx.diagnostic().diagParamMap()[Diagnostic::DIAG_CARRIER] = airSeg->carrier();
      }
    }
  }

  if (exchangeTrx &&
      (exchangeTrx->reqType() == FULL_EXCHANGE || exchangeTrx->reqType() == PARTIAL_EXCHANGE ||
       exchangeTrx->reqType() == TAG_10_EXCHANGE))
  {
    if (!exchangeTrx->newItin().empty() && !exchangeTrx->exchangeItin().empty())
      ExcItinUtil::IsStopOverChange(exchangeTrx->exchangeItin().front(),
                                    exchangeTrx->newItin().front());
  }

  baseItinAnalyzerWrapper->setItinRounding(trx);

  if (trx.getTrxType() == PricingTrx::MIP_TRX && !trx.getRequest()->originBasedRTPricing() &&
      !trx.isAltDates() && !trx.snapRequest())
  {
    baseItinAnalyzerWrapper->setInfoForSimilarItins(trx);
  }

  if (TrxUtil::taxEnhancementActivated(DateTime::fromMilitaryTime(1200)))
  {
    if (trx.getRequest()->originBasedRTPricing())
    {
      WnSnapUtil::buildSubItinVecWithEmptyValues(trx);
    }
  }

  if (trx.getTrxType() == PricingTrx::MIP_TRX || trx.activationFlags().isSearchForBrandsPricing())
  {
    FamilyLogicUtils::setFareFamiliesIds(trx);
    trx.setItinsTimeSpan();

    for (Itin* itin : trx.itin())
    {
      FamilyLogicUtils::fillSimilarItinData(trx, *itin, this);

      iadetail::JourneyPrepHelper::prepareItinForJourney(
          itin->getSimilarItins().begin(), itin->getSimilarItins().end(), trx);

      markSimilarItinMarkets(*itin);
    }

    if(!fallback::fallbackSumUpAvlForMotherItin(&trx) && trx.isBRAll())
      FamilyLogicUtils::sumUpChildAvailabilityForMotherItins(trx, _requestedNumberOfSeats);

    if (UNLIKELY(trx.diagnostic().diagnosticType() == SimilarItinIADiagnostic))
    {
      DiagManager manager(trx, SimilarItinIADiagnostic);
      static_cast<SimilarItinIADiagCollector&>(manager.collector())
          .printSimilarItinData(trx.itin());
    }

    fillSimilarItinFMCOS(trx);
    removeUnwantedFareMarkets(trx);

    if (trx.getRequest()->isBrandedFaresRequest() ||
        trx.isBrandsForTnShopping() ||
        trx.activationFlags().isSearchForBrandsPricing())
    {
      retrieveBrands(trx);
    }
  }

  if (trx.getOptions()->isRtw())
  {
    for (Itin* itin : trx.itin())
    {
      ItinUtil::setRoundTheWorld(trx, *itin);
    }
  }
  diag192(trx);

  // Price By Cabin - try to replace RBD to requested cabin
  if (!fallback::fallbackPriceByCabinActivation(&trx) &&
      !trx.getOptions()->cabin().isUndefinedClass() &&
      (PricingTrx::PRICING_TRX == trx.getTrxType() ||
       (PricingTrx::MIP_TRX == trx.getTrxType() &&
        trx.billing()->actionCode() == "WPNI.C")))
  {
    setAirSegsWithRequestedCabin(trx);
  }

  processSpanishDiscount(trx);

  if (trx.isPbbRequest() && TrxUtil::isRequestFromAS(trx))
  {
    Diag892Collector* diag892 = createDiag<Diag892Collector>(trx);
    if (trx.isAssignLegsRequired())
    {
      for (Itin* itin : trx.itin())
        itinanalyzerutils::assignLegsToSegments(itin->travelSeg(), trx.dataHandle(), diag892);
    }
    else if (UNLIKELY(diag892))
    {
      *diag892 << "LEG IDS AFTER VALIDATION:\n";
      diag892->printSegmentsLegIdInfo(trx.travelSeg());
      diag892->flushMsg();
    }
  }

  processOldBrandedFareSecondaryRBD(trx);

  return true;
}

void
ItinAnalyzerService::processSpanishDiscount(PricingTrx& trx)
{
  if (fallback::spanishResidentFares(&trx))
    return;

  const auto residency = trx.getOptions()->residency();
  bool applicable = SRFEUtil::isPassengerApplicable(residency, trx.residencyState()) &&
                    SRFEUtil::isApplicableForPOS(*trx.getRequest()->ticketingAgent());
  for (auto it : trx.itin())
    it->setSRFEApplicable(SRFEUtil::isItinApplicable(*it) && applicable);
  DiagManager diag666(trx, DiagnosticTypes::Diagnostic666);
  if (diag666.isActive())
  {
    Diag666Collector& dc = static_cast<Diag666Collector&>(diag666.collector());

    dc.printHeader("SPANISH RESIDENT FARES DIAGNOSTIC");
    dc.printPassengerValidation(residency);
    dc << *trx.getRequest()->ticketingAgent();
    dc.printItinValidation(trx);
  }
}

void
ItinAnalyzerService::buildFareMarketsForSimilarItin(PricingTrx& trx, Itin& itin)
{
  selectProcesing(trx)->buildFareMarket(trx, itin);
}

// Fail an itinary if the primary sector does not have B30 booking code in old branded fare
// brandedFareEntry() for old branded fare, isBrandedFaresRequest new branded fare
void
ItinAnalyzerService::processOldBrandedFareSecondaryRBD(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.getRequest()->brandedFareEntry() && !trx.getRequest()->isBrandedFaresRequest())
  {
    std::vector<BookingCode>& sbkc = trx.getRequest()->brandedFareSecondaryBookingCode();
    std::vector<BookingCode>& pbkc = trx.getRequest()->brandedFareBookingCode();
    if (sbkc.empty() || pbkc.empty()) return;

    bool isFareMarketsForBadItin = false;

    for (Itin* itin : trx.itin())
    {
      std::set<TravelSeg*> internationalFlights;
      std::set<TravelSeg*> domesticFlights;

      for (TravelSeg* ts : itin->travelSeg())
      {
        //Filter the segments for international/domestic and ignore if it is a dummy segment
        if (!static_cast<const AirSeg*>(ts)->isFake() )
        {
          if (ts->isInternationalSegment())
            internationalFlights.insert(ts);
          else
            domesticFlights.insert(ts);
        }
      }

      //Ignore flights with dummyFare

      if(!internationalFlights.empty()) //Atleast one flight is international flight
      {
        //Validate if atleast one international flight has primary booking code
        if(!isAnyFlightWithPrimaryBookingCode(itin, internationalFlights, pbkc, false))
        {
          std::string error = "INVALID ITIN FOR THE CLASS USED";
          itin->errResponseCode() = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
          itin->errResponseMsg() = error;
          isFareMarketsForBadItin = true;
        }
        else //Zero out seats for Secondary RBD for International flights with Primary Booking Code
          updateSeatsForFlightsWithSecondaryRBD(itin, internationalFlights, sbkc, true);
      }
      else //All flights are domestic flights
      {
        bool isFlightsWithPBKC = isAnyFlightWithPrimaryBookingCode(itin, domesticFlights, pbkc, true);
        updateSeatsForFlightsWithSecondaryRBD(itin, domesticFlights, sbkc, isFlightsWithPBKC);
      }
    }
    if(isFareMarketsForBadItin)
      processFaremarketsForBadItin(trx);
  }
}

//Process FMs which are unique to the bad/invalid Itinary across all available Itins
void
ItinAnalyzerService::processFaremarketsForBadItin(PricingTrx& trx)
{
  for (Itin* itin : trx.itin())
  {
    if(itin->errResponseCode() == ErrorResponseException::NO_FARE_FOR_CLASS_USED)
    {
      std::set<FareMarket*> fareMarketsForBadItin;
      for( FareMarket* fm : itin->fareMarket() )
        fareMarketsForBadItin.insert(fm);

      for(Itin* itin1 : trx.itin())
      {
        if(itin1->errResponseCode() != ErrorResponseException::NO_FARE_FOR_CLASS_USED)
        {
          for(FareMarket* fm : itin1->fareMarket())
          {
            if( fareMarketsForBadItin.count(fm) )
              fareMarketsForBadItin.erase(fm);
          }
        }
      }

      for(FareMarket* fm : itin->fareMarket())
      {
        if( fareMarketsForBadItin.count(fm) )
          fm->failCode() = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
      }
    }
  }
}

//Atleast one flight is international and has primary booking code
void
ItinAnalyzerService::updateSeatsForFlightsWithSecondaryRBD(
  Itin* itin, std::set<TravelSeg*> flights, std::vector<BookingCode>& sbkc, bool isFlightsWithPBKC)
{
  for (FareMarket* fm : itin->fareMarket())
  {
    const std::vector<std::vector<ClassOfService*>*>& cosVec = fm->classOfServiceVec();
    const std::vector<TravelSeg*>& travelSegVec = fm->travelSeg();

    if(travelSegVec.size()==cosVec.size())
    {
      for (size_t i = 0; i < travelSegVec.size(); ++i)
      {
        if(flights.count(travelSegVec[i]))
        { //Zero out seats for domestic/international flights with Primary booking code
          //Zero out seats for all domestic flights with No Primary booking code
          zeroOutSeatsForSecondaryRBD(*cosVec[i], sbkc);
          if(isFlightsWithPBKC && travelSegVec[i]->isInternationalSegment())
            travelSegVec[i]->setSecondaryRBDReasonCodeStatus(TravelSeg::INTERNATIONALFLIGHTS_PBKC);
          else if(isFlightsWithPBKC && !travelSegVec[i]->isInternationalSegment())
            travelSegVec[i]->setSecondaryRBDReasonCodeStatus(TravelSeg::DOMESTICFLIGHTS_PBKC);
          else
            travelSegVec[i]->setSecondaryRBDReasonCodeStatus(TravelSeg::DOMESTICFLIGHTS_NO_PBKC);
        }
      }
    }
  }
}

//Find if atleast one international/Domestic flight has primary booking code
bool
ItinAnalyzerService::isAnyFlightWithPrimaryBookingCode(
    Itin* itin, std::set<TravelSeg*>& flights, std::vector<BookingCode>& pbkc, bool isDomesticFlight)
{
  std::set<TravelSeg*> flightsWithPrimaryRBD;
  bool isFlightsWithPrimaryRBD = false;
  for (FareMarket* fm : itin->fareMarket())
  {
    const std::vector<std::vector<ClassOfService*>*>& cosVec = fm->classOfServiceVec();
    const std::vector<TravelSeg*>& travelSegVec = fm->travelSeg();
    if(travelSegVec.size()==cosVec.size())
    {
      for (size_t i = 0; i < travelSegVec.size(); ++i)
      {
        if(flights.count(travelSegVec[i]))
        {
          for (ClassOfService* cos : *cosVec[i])
          {
            if (std::find(pbkc.begin(), pbkc.end(), cos->bookingCode())!=pbkc.end())
            {//Found Primary booking code for Domestic/International Flights
              isFlightsWithPrimaryRBD = true;
              if(!flightsWithPrimaryRBD.count(travelSegVec[i]))
                flightsWithPrimaryRBD.insert(travelSegVec[i]);
              break;
            }
          }
        }
      }
    }
  }

  if (isDomesticFlight)
  {
    if(isFlightsWithPrimaryRBD)
      flights = flightsWithPrimaryRBD;
  }
  else
    flights = flightsWithPrimaryRBD;

  return isFlightsWithPrimaryRBD;
}

//Zero out number of seats for a particular segment with Secondary Bookong code
void
ItinAnalyzerService::zeroOutSeatsForSecondaryRBD(
    std::vector<ClassOfService*>& cosVec, std::vector<BookingCode>& sbkc)
{
  for (ClassOfService* cos : cosVec)
  {
    if (std::find(sbkc.begin(), sbkc.end(), cos->bookingCode())!=sbkc.end())
      cos->numSeats()=0;
  }
}

// Removing solo fare markets if ThruOnly requested
// and fare markets spanning more than one leg if split by leg requested
void
ItinAnalyzerService::removeUnwantedFareMarkets(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);

  struct ScopedSimilarItinMergeBack
  {
    PricingTrx& _trx;
    uint32_t _numMotherItins;

    ScopedSimilarItinMergeBack(PricingTrx& trx) : _trx(trx)
    {
      _numMotherItins = static_cast<uint32_t>(_trx.itin().size());

      for (uint32_t i = 0; i < _numMotherItins; ++i)
      {
        for (const auto& itinData : _trx.itin()[i]->getSimilarItins())
          _trx.itin().push_back(itinData.itin);
      }
    }

    ~ScopedSimilarItinMergeBack()
    {
      _trx.itin().resize(_numMotherItins);
    }
  } scopedSimilarItinMergeBack(trx);

  bool needToUpdateFmsInTrx = false;

  if (_thruFMOnly)
  {
    removeSoloFMs(trx);
    needToUpdateFmsInTrx = true;
  }

  if (trx.getOptions()->isForceFareBreaksAtLegPoints())
  {
    itinanalyzerutils::removeFMsExceedingLegs(trx.itin());
    needToUpdateFmsInTrx = true;
  }

  if (needToUpdateFmsInTrx)
  {
    itinanalyzerutils::removeTrxFMsNotReferencedByAnyItin(trx);
  }
}

void
ItinAnalyzerService::fillSimilarItinFMCOS(PricingTrx& trx)
{
  for (const auto mother : trx.itin())
  {
    for (const auto& itinData : mother->getSimilarItins())
    {
      const auto similar = itinData.itin;
      for (const auto fm : similar->fareMarket())
        ShoppingUtil::getFMCOSBasedOnAvailBreak(trx, similar, fm);
    }
  }
}

void
ItinAnalyzerService::processFMsForValidatingCarriers(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  bool isRepricingFromMIP = false;
  if (!fallback::fallbackGSAMipDifferentialFareFix(&trx) &&
      trx.getTrxType() == PricingTrx::REPRICING_TRX && trx.getParentTrx())
  {
    PricingTrx* parentTrx = dynamic_cast<PricingTrx*>(trx.getParentTrx());
    if (parentTrx && parentTrx->getTrxType() == PricingTrx::MIP_TRX)
      isRepricingFromMIP = true;
  }

  if (!needToPopulateFareMarkets(trx, isRepricingFromMIP))
    return; // For non-MIP, if there is only 1 VC, don't populate FMs.

  std::map<FareMarket*, std::set<CarrierCode>> fmValidatingCxrMap;

  for (Itin* motherItin : trx.itin())
    processItinFMsForValidatingCarriers(trx, *motherItin, fmValidatingCxrMap);

  for (FareMarket* fm : trx.fareMarket())
  {
    fm->validatingCarriers().insert(fm->validatingCarriers().end(),
                                    fmValidatingCxrMap[fm].begin(),
                                    fmValidatingCxrMap[fm].end());
  }
}

void
ItinAnalyzerService::processItinFMsForValidatingCarriers(
    PricingTrx& trx,
    Itin& motherItin,
    std::map<FareMarket*, std::set<CarrierCode>>& fmValidatingCxrMap)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<CarrierCode> validatingCarriers;
  motherItin.getValidatingCarriers(trx, validatingCarriers);

  for (FareMarket* fm : motherItin.fareMarket())
  {
    fmValidatingCxrMap[fm].insert(validatingCarriers.begin(), validatingCarriers.end());
    fm->setThru(itinanalyzerutils::isThruFareMarket(*fm, motherItin));
  }

  for (const SimilarItinData& childItinData : motherItin.getSimilarItins())
  {
    validatingCarriers.clear();
    childItinData.itin->getValidatingCarriers(trx, validatingCarriers);

    for (FareMarket* fm : motherItin.fareMarket())
      if (fm->isThru()) // Only thru FMs are processed for child itin.
        fmValidatingCxrMap[fm].insert(validatingCarriers.begin(), validatingCarriers.end());
  }
}

void
ItinAnalyzerService::initializeTNBrandsData(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // Create calculator objects for trx and all itins

  if (trx.getRequest()->originBasedRTPricing() && !fallback::fallbackRTPricingContextFix(&trx))
  {
    const bool fixed = !trx.getMutableFixedLegs().empty() && trx.getMutableFixedLegs().front();
    if (fixed)
    {
      // we need both legs (real and dummy) fixed in order for it to work
      // dummy leg must have the same brand code fixed as real leg has.
      trx.getMutableFixedLegs().assign(2, fixed);
      skipper::FareComponentShoppingContext* dummyLegContext = nullptr;
      trx.dataHandle().get(dummyLegContext);
      // real segments are numbered from 1. Dummy one has alwats pnrSegment index = 0
      trx.getMutableFareComponentShoppingContexts()[0] = dummyLegContext;

      skipper::FareComponentShoppingContext* realLegContext =
                                       trx.getMutableFareComponentShoppingContexts().at(1);
      TSE_ASSERT(realLegContext != nullptr);
      dummyLegContext->brandCode = realLegContext->brandCode;
    }
  }

  trx.setTrxGeometryCalculator(&trx.dataHandle().safe_create<skipper::TrxGeometryCalculator>(trx));

  const skipper::TrxGeometryCalculator* trxGeometryCalculator = trx.getTrxGeometryCalculator();
  TSE_ASSERT(trxGeometryCalculator != nullptr);

  for (size_t itinIdx = 0; itinIdx < trx.itin().size(); ++itinIdx)
  {
    Itin* itin = trx.itin()[itinIdx];
    TSE_ASSERT(itin != nullptr);
    itin->setItinBranding(
        skipper::ItinBranding::createItinBranding(*itin, *trxGeometryCalculator, trx.dataHandle()));
  }
}

void
ItinAnalyzerService::setThruFareMarketsForBfa(PricingTrx& trx)
{
  for (size_t itinIdx = 0; itinIdx < trx.itin().size(); ++itinIdx)
  {
    Itin* itin = trx.itin()[itinIdx];
    TSE_ASSERT(itin != nullptr);
    for (FareMarket* fm : itin->fareMarket())
    {
      TSE_ASSERT(fm != nullptr);
      fm->setThru(itin->getItinBranding().isThruFareMarket(*fm));
    }
  }
}

bool
ItinAnalyzerService::processBfaBrandRetrieval(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  Diag892Collector* diag892 = createDiag<Diag892Collector>(trx);
  Diag894Collector* diag894 = createDiag<Diag894Collector>(trx);
  auto flushDiagnostics = [this, diag892, diag894]
  {
    if (UNLIKELY(diag892))
      flushDiag(diag892);
    if (UNLIKELY(diag894))
      flushDiag(diag894);
  };

  bool status = false;

  { // Brackets are intentional - diags are printed in the destructor
    const BrandRetrievalMode mode = BrandRetrievalMode::PER_FARE_COMPONENT;
    BrandedFaresDataRetriever bfdr(trx, mode, diag892, diag894);
    status = bfdr.process();
    if (!status && (trx.isBRAll() || trx.activationFlags().isSearchForBrandsPricing()))
      handleFailedBrandingService(trx,
                                  bfdr.getRequestResponseHandler().getStatusBrandingService(),
                                  flushDiagnostics);
  }

  initializeTNBrandsData(trx);
  setThruFareMarketsForBfa(trx);
  flushDiagnostics();

  return status;
}

namespace
{
bool
brandUsedOnAnyFareMarket(const BrandCode& brand,
                         const std::vector<FareMarket*>& fareMarkets,
                         const std::vector<QualifiedBrand>& brandTable)
{
  for (FareMarket* fm : fareMarkets)
    for (auto brandid : fm->brandProgramIndexVec())
    {
      if (brandTable[brandid].second->brandCode() == brand)
        return true;
    }
  return false;
}

const BrandCode
fixedBrandOnLeg(const TravelSegPtrVec& segments)
{
  return segments.front()->getBrandCode();
}

// Fixed legs must form a continuous block.
// Constructions like regular-fixed-regular or fixed-regular-fixed are not permitted
void
checkIfFixedLegsAreContinuous(const std::vector<bool>& fixedLegs,
                              bool fallbackCheckFromEnd)
{
  if (fixedLegs.size() < 3)
    return;

  bool statusHasChanged = false;
  for (unsigned int i = 1; i < fixedLegs.size(); ++i)
  {
    if (fixedLegs.at(i) != fixedLegs.at(i - 1))
    {
      if (statusHasChanged)
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                     "Fixed part of the journey must be continuous");

      statusHasChanged = true;
    }
  }

  if (!fallbackCheckFromEnd)
  {
    // We need to do this check after the first loop for the message to be accurate
    for (unsigned int i = 1; i < fixedLegs.size(); ++i)
    {
      if (fixedLegs.at(i) && !fixedLegs.at(i - 1))
      {
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                     "Fixing legs from end is not permitted in multicity");
      }
    }
  }
}

void
validateContextBrands(const std::vector<bool>& fixedLegs,
                      const std::vector<FareMarket*>& fareMarkets,
                      const Itin* itinerary,
                      const std::vector<QualifiedBrand>& brandTable)
{
  for (unsigned int i = 0; i < fixedLegs.size(); ++i)
  {
    if (!fixedLegs.at(i))
      continue;

    const BrandCode fixedBrand = fixedBrandOnLeg(itinerary->itinLegs().at(i));
    if (fixedBrand.empty())
      continue;

    auto fareMarketUsedOnLeg = [i](const FareMarket* fareMarket)
    {
      for (auto& travelSeg : fareMarket->travelSeg())
        if (travelSeg->legId() == ((int)i))
          return true;
      return false;
    };
    std::vector<FareMarket*> fareMarketsAtLeg;
    std::copy_if(fareMarkets.begin(),
                 fareMarkets.end(),
                 std::back_inserter(fareMarketsAtLeg),
                 fareMarketUsedOnLeg);

    if (!brandUsedOnAnyFareMarket(fixedBrand, fareMarketsAtLeg, brandTable))
    {
      std::string msg("Invalid brand ");
      msg.append(fixedBrand);
      msg.append(" fixed on leg ");
      msg.append(
          std::to_string(i + 1)); // In the final response, in ISELL, legs are numbered from 1
      throw ErrorResponseException(ErrorResponseException::NO_VALID_BRAND_FOUND, msg.c_str());
    }
  }
}

void
validateFixedLegs(PricingTrx& trx)
{
  if (trx.isNotExchangeTrx())
    checkIfFixedLegsAreContinuous(trx.getFixedLegs(), fallback::fallbackCheckLegFixFromEnd(&trx));
  validateContextBrands(
      trx.getFixedLegs(), trx.fareMarket(), trx.itin().front(), trx.brandProgramVec());
}
} // namespace

bool
ItinAnalyzerService::processIbfBrandRetrievalAndParity(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  Diag892Collector* diag892 = createDiag<Diag892Collector>(trx);
  Diag894Collector* diag894 = createDiag<Diag894Collector>(trx);

  auto flushDiagnostics = [this, diag892, diag894]
  {
    if (UNLIKELY(diag892))
    {
      flushDiag(diag892);
      diag892->printFooter();
    }
    if (UNLIKELY(diag894))
      flushDiag(diag894);
  };

  bool status = false;

  { // Brackets are intentional - diags are printed in the destructor
    const BrandRetrievalMode mode = BrandingUtil::getBrandRetrievalMode(trx);
    BrandedFaresDataRetriever bfdr(trx, mode, diag892, diag894);
    status = bfdr.process();
    if (!status)
    {
      handleFailedBrandingService(trx,
                                  bfdr.getRequestResponseHandler().getStatusBrandingService(),
                                  flushDiagnostics);
    }
  }

  initializeTNBrandsData(trx);

  if (status)
  {
    if (trx.getRequest()->isParityBrandsPath()) // IBF, CS or PBO (IBF/CS)
    {
      if (trx.isContextShopping())
        validateFixedLegs(trx);

      if (trx.getRequest()->isProcessParityBrandsOverride()) //PBO for IBF or CS
        status = trx.getTrxGeometryCalculator()->calculateParityBrandsOverrideForAllItins(diag892);
      else if (trx.isContextShopping()) //CS, no PBO
        status = trx.getTrxGeometryCalculator()->calculateContextBrandParityForAllItins(diag892);
      else //IBF, no PBO
        status = trx.getTrxGeometryCalculator()->calculateBrandParityForAllItins(diag892);
    }

    if (!showCrossBrandOptionFirst.getValue())
    {
      if (BrandingUtil::isCrossBrandOptionNeeded(trx))
      {
        for (Itin* itin : trx.itin())
        {
          itin->brandCodes().insert(ANY_BRAND_LEG_PARITY);
        }
      }
    }

    if (status)
    {
      removeFMsWithErrors(trx, diag892);
    }
    else
    {
      // check if all itins have no brand codes
      if (std::all_of(trx.itin().cbegin(),
                      trx.itin().cend(),
                      [](const Itin* itin)
                      { return itin->brandCodes().empty() && !itin->brandFilterMap().empty(); }))
        throw ErrorResponseException(ErrorResponseException::REQUESTED_BRANDS_INVALID_FOR_TRIP);
    }
  }
  flushDiagnostics();

  return status;
}

void
ItinAnalyzerService::removeFMsWithErrors(PricingTrx& trx, Diag892Collector* diag892)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // find fare markets to be removed
  std::set<FareMarket*> fmsToRemove;
  for (FareMarket* fm : trx.fareMarket())
  {
    if (fm->failCode() != 0)
      fmsToRemove.insert(fm);
  }
  // remove them from both trx.fareMarket() and from every itin
  removeFareMarketsFromTrxAndItins(trx, fmsToRemove);
  if (diag892)
  {
    diag892->printRemovedFareMarkets(fmsToRemove);
  }
}

void
ItinAnalyzerService::removeFareMarketsFromTrxAndItins(PricingTrx& trx,
                                                      std::set<FareMarket*>& FmsToBeRemoved)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (FmsToBeRemoved.empty())
    return;

  IsFMInSet isFmInSetPred(FmsToBeRemoved);
  trx.fareMarket().erase(
      std::remove_if(trx.fareMarket().begin(), trx.fareMarket().end(), isFmInSetPred),
      trx.fareMarket().end());

  for (auto itin : trx.itin())
  {
    itin->fareMarket().erase(
        std::remove_if(itin->fareMarket().begin(), itin->fareMarket().end(), isFmInSetPred),
        itin->fareMarket().end());
  }
}

void
ItinAnalyzerService::removeItinsWithoutGoverningCarrier(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  const CarrierCode& cxrOverride = trx.getRequest()->governingCarrierOverrides()[0];
  ItinWithoutGoverningCarrier itinWithoutGoverningCarrier(cxrOverride);
  std::string errMsg = std::string("TRAVEL NOT VIA ") + cxrOverride;
  std::vector<Itin*>::iterator rmBegin;

  rmBegin = std::stable_partition(
      trx.itin().begin(), trx.itin().end(), std::not1(itinWithoutGoverningCarrier));

  diag982(trx, rmBegin, trx.itin().end(), errMsg);
  trx.itin().erase(rmBegin, trx.itin().end());

  if (trx.itin().empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, errMsg.c_str());
}

void
ItinAnalyzerService::diag982(PricingTrx& trx,
                             const std::vector<Itin*>::const_iterator begin,
                             const std::vector<Itin*>::const_iterator end,
                             const std::string& msg)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic982 &&
      trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "REMOVED")
  {
    DCFactory* factory = DCFactory::instance();
    Diag982Collector* diagPtr = dynamic_cast<Diag982Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic982);
    diagPtr->displayRemovedItins(begin, end, msg);
    diagPtr->flushMsg();
  }
}

void
ItinAnalyzerService::populateGoverningCarrierOverrides(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  size_t maxItinSize = 0;
  for (const auto itin : trx.itin()) // find max itin size
  {
    size_t travelSegSize = itin->travelSeg().size();
    if (maxItinSize < travelSegSize)
    {
      maxItinSize = travelSegSize;
    }
  }

  CarrierCode cxrOverride = trx.getRequest()->governingCarrierOverrides()[0];
  for (size_t i = 1; i < maxItinSize; ++i) // first element populated in PRO/B11 section
    trx.getRequest()->governingCarrierOverrides().insert(std::make_pair(i, cxrOverride));
}

void
ItinAnalyzerService::copyExcFareCompInfoToNewItinFm(ExchangePricingTrx& excTrx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  ExcItin* excItin = excTrx.exchangeItin().front();
  Itin* newItin = excTrx.newItin().front();

  std::vector<TravelSeg*>::const_iterator fctrvliter;
  std::vector<TravelSeg*>::const_iterator fctrvliterE;

  std::vector<TravelSeg*>::const_iterator iter = newItin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterE = newItin->travelSeg().end();

  std::vector<FareCompInfo*>::const_iterator fareCompIter = excItin->fareComponent().begin();
  std::vector<FareCompInfo*>::const_iterator fareCompIE = excItin->fareComponent().end();
  for (; fareCompIter != fareCompIE; ++fareCompIter)
  {
    const FareCompInfo& fcInfo = *(*fareCompIter);

    iter = fcInfoMatch(excTrx, fcInfo);
    if (iter == iterE)
    {
      removeOldSurOverrides(excTrx, fcInfo);
      continue;
    }

    fctrvliter = fcInfo.fareMarket()->travelSeg().begin();
    fctrvliterE = fcInfo.fareMarket()->travelSeg().end();

    for (; fctrvliter != fctrvliterE && iter != iterE; ++fctrvliter, ++iter)
    {
      const TravelSeg& tvlSegExc = *(*fctrvliter);
      TravelSeg& tvlSegNew = *(*iter);

      removeNewSurOverrides(excTrx, *iter);
      copyExcStopoverOverride(excTrx, fcInfo, tvlSegExc, &tvlSegNew);

      tvlSegNew.fareCalcFareAmt() = tvlSegExc.fareCalcFareAmt();
      tvlSegNew.fareBasisCode() = tvlSegExc.fareBasisCode();

      tvlSegNew.forcedFareBrk() = tvlSegExc.forcedFareBrk();
      tvlSegNew.forcedNoFareBrk() = tvlSegExc.forcedNoFareBrk();
      excTrx.exchangeOverrides().dummyFCSegs()[(*iter)] = fcInfo.fareCompNumber();
      excTrx.exchangeOverrides().dummyFareMiles()[(*iter)] = fcInfo.mileageSurchargePctg();
      if (!fcInfo.mileageSurchargeCity().empty())
        excTrx.exchangeOverrides().dummyFareMileCity()[(*iter)] = fcInfo.mileageSurchargeCity();

      if (tvlSegExc.forcedSideTrip() == 'T')
      {
        tvlSegNew.forcedSideTrip() = tvlSegExc.forcedSideTrip();
        excTrx.exchangeOverrides().forcedSideTrip()[(*iter)] = tvlSegExc.forcedSideTrip();
      }
    }
  }
  (*newItin).calcCurrencyOverride() = (*excItin).calculationCurrency();
}

std::vector<TravelSeg*>::const_iterator
ItinAnalyzerService::fcInfoMatch(ExchangePricingTrx& excTrx, const FareCompInfo& fcInfo)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  Itin* newItin = excTrx.newItin().front();
  std::vector<TravelSeg*>::const_iterator iter = newItin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterE = newItin->travelSeg().end();
  std::vector<TravelSeg*>::const_iterator firstMatch = iterE;

  // if(fcInfo.fareCalcFareAmt() <= 0.0)
  //   return iterE;

  if (fcInfo.fareBasisCode().empty())
    return iterE;

  const std::map<const TravelSeg*, uint16_t>::const_iterator dummySegE =
      excTrx.exchangeOverrides().dummyFCSegs().end();

  std::vector<TravelSeg*>::const_iterator excTvlI = fcInfo.fareMarket()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator excTvlIE = fcInfo.fareMarket()->travelSeg().end();

  for (; iter != iterE; ++iter)
  {
    if (segMatch(*iter, *excTvlI))
    {
      firstMatch = iter;
      break;
    }
  }

  if (firstMatch == iterE ||
      excTrx.exchangeOverrides().dummyFCSegs().find(*firstMatch) != dummySegE)
    return iterE;

  const uint16_t tvlSegSz = fcInfo.fareMarket()->travelSeg().size();
  if (tvlSegSz == 1)
    return firstMatch;

  // we do not have enough number of new segments for same travel
  if (std::distance(iter, iterE) < tvlSegSz)
    return iterE;

  ++excTvlI;
  ++iter;
  if (iter == iterE)
    return iterE;

  for (; excTvlI != excTvlIE; ++excTvlI, ++iter)
  {
    if (excTrx.exchangeOverrides().dummyFCSegs().find(*iter) != dummySegE)
      return iterE;

    if (!segMatch(*iter, *excTvlI))
      return iterE;
  }
  return firstMatch;
}

bool
ItinAnalyzerService::segMatch(const TravelSeg* tvlSegNew, const TravelSeg* tvlSegExc)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  const AirSeg* airSegNew = dynamic_cast<const AirSeg*>(tvlSegNew);
  const AirSeg* airSegExc = dynamic_cast<const AirSeg*>(tvlSegExc);
  if (airSegNew == nullptr && airSegExc == nullptr)
    return true;

  if (airSegNew == nullptr)
    return false;

  if (airSegExc == nullptr)
    return false;

  if (airSegNew->marketingFlightNumber() != airSegExc->marketingFlightNumber())
    return false;

  if (airSegNew->marketingCarrierCode() != airSegExc->marketingCarrierCode())
    return false;

  if (airSegNew->origAirport() != airSegExc->origAirport())
    return false;

  if (airSegNew->destAirport() != airSegExc->destAirport())
    return false;

  if (airSegNew->departureDT() != airSegExc->departureDT())
    return false;

  if (airSegNew->getBookingCode() != airSegExc->getBookingCode())
    return false;

  if (!airSegNew->fareBasisCode().empty() && !airSegNew->fareCalcFareAmt().empty())
    return false; // safe guard, this seg should have been in dummyFCSegs

  return true;
}

void
ItinAnalyzerService::removeOldSurOverrides(ExchangePricingTrx& excTrx, const FareCompInfo& fcInfo)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (excTrx.exchangeOverrides().surchargeOverride().empty())
    return;

  std::vector<SurchargeOverride*>::iterator surI =
      excTrx.exchangeOverrides().surchargeOverride().begin();
  std::vector<SurchargeOverride*>::iterator surE =
      excTrx.exchangeOverrides().surchargeOverride().end();

  std::vector<TravelSeg*>::const_iterator excTvlI = fcInfo.fareMarket()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator excTvlIE = fcInfo.fareMarket()->travelSeg().end();

  for (; surI != surE; ++surI)
  {
    SurchargeOverride& surchargeOverride = *(*surI);
    if (!surchargeOverride.fromExchange())
      continue;

    excTvlI =
        find(fcInfo.fareMarket()->travelSeg().begin(), excTvlIE, surchargeOverride.travelSeg());
    if (excTvlI != excTvlIE)
      surchargeOverride.removed() = true;
  }
}

void
ItinAnalyzerService::removeNewSurOverrides(ExchangePricingTrx& excTrx, const TravelSeg* tvlSeg)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (excTrx.exchangeOverrides().surchargeOverride().empty())
    return;

  std::vector<SurchargeOverride*>::iterator surI =
      excTrx.exchangeOverrides().surchargeOverride().begin();
  std::vector<SurchargeOverride*>::iterator surE =
      excTrx.exchangeOverrides().surchargeOverride().end();

  for (; surI != surE; ++surI)
  {
    SurchargeOverride& surchargeOverride = *(*surI);
    if (surchargeOverride.fromExchange())
      continue;

    if (surchargeOverride.travelSeg() == tvlSeg)
      surchargeOverride.removed() = true;
  }
}

void
ItinAnalyzerService::setATAEContent(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    // Assign Fare Maket COS

    if (trx.getOptions()->callToAvailability() == 'T') // PBM="T"
    {
      _contentSvcs.getAvailShopping(trx);
    }
    else
    {
      bool callDCA = checkCallDCAPerItn(trx.itin());
      if (callDCA)
      {
        _contentSvcs.getAvailShopping(trx);
      }
      setFareMarketCOS(trx);
    }
  }
  else if ((!trx.displayOnly()) && trx.getTrxType() == PricingTrx::PRICING_TRX &&
           dynamic_cast<RepricingTrx*>(&trx) == nullptr)
  {
    TSELatencyData metrics(trx, "ITIN ATAE");
    _contentSvcs.getSchedAndAvail(trx); // DSS v2 and AS2 call
    JourneyUtil::initOAndDCOS(trx.itin().front());
    if (trx.getRequest()->multiTicketActive() && !trx.multiTicketMap().empty())
    {
      setFareMarketCOS(trx);
    }
  }
}

void
ItinAnalyzerService::setATAEAvailContent(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    // Assign Fare Maket COS

    if (trx.getOptions()->callToAvailability() == 'T') // PBM="T"
    {
      _contentSvcs.getAvailShopping(trx);
    }
    else
    {
      bool callDCA = checkCallDCAPerItn(trx.itin());
      if (callDCA)
      {
        _contentSvcs.getAvailShopping(trx);
      }
      setFareMarketCOS(trx);
    }
  }
  else if ((!trx.displayOnly()) && trx.getTrxType() == PricingTrx::PRICING_TRX &&
           dynamic_cast<RepricingTrx*>(&trx) == nullptr)
  {
    TSELatencyData metrics(trx, "ITIN ATAE AVAILABILITY");
    _contentSvcs.getAvailability(trx); //  AS2 call
    JourneyUtil::initOAndDCOS(trx.itin().front());
    if (trx.getRequest()->multiTicketActive() && !trx.multiTicketMap().empty())
    {
      setFareMarketCOS(trx);
    }
  }
}
void
ItinAnalyzerService::setATAESchedContent(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    return;
  }
  else if ((!trx.displayOnly()) && trx.getTrxType() == PricingTrx::PRICING_TRX &&
           dynamic_cast<RepricingTrx*>(&trx) == nullptr)
  {
    TSELatencyData metrics(trx, "ITIN ATAE SCHEDULE");
    _contentSvcs.getSchedule(trx); // DSS v2
  }
}
void
ItinAnalyzerService::setATAEContent(AncillaryPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  markFlownSegments(trx);

  TSELatencyData metrics(trx, "ITIN ATAE ANCILLARY");
  _contentSvcs.getSched(trx);
}

bool
ItinAnalyzerService::process(AltPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, "Entered process(AltPricingTrx)");
  return process((PricingTrx&)trx);
}

bool
ItinAnalyzerService::process(NoPNRPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, "Entered process(NoPNRPricingTrx)");
  return process((AltPricingTrx&)trx);
}

bool
ItinAnalyzerService::buildFareMarket(PricingTrx& trx, bool checkLimitations)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN BUILD FMKT");

  checkSimpleTrip(trx);

  std::string errResponseMsg;
  ErrorResponseException::ErrorResponseCode errResponseCode = ErrorResponseException::NO_ERROR;

  for (std::vector<Itin*>::iterator itinIt = trx.itin().begin(); itinIt != trx.itin().end();
       /*do not increment iterator*/)
  {
    Itin& itin = **itinIt;

    if (checkLimitations)
    {
      // Check limitations
      LimitationOnIndirectTravel limits(trx, itin);

      try
      {
        limits.validateJourney();
      }
      catch (ErrorResponseException& ex)
      {
        itinIt = trx.itin().erase(itinIt);
        errResponseMsg = ex.message();
        errResponseCode = ex.code();
        continue;
      }
    }

    selectProcesing(trx)->buildFareMarket(trx, itin);

    ++itinIt;
  }
  if (trx.itin().empty())
  {
    throw ErrorResponseException(errResponseCode, errResponseMsg.c_str());
  }
  return true;
}

namespace
{
class BreakIt : public std::unary_function<FareMarket*, void>
{
public:
  void operator()(FareMarket* fm) { fm->setBreakIndicator(true); }
};
class EqualFM : public std::unary_function<FareMarket*, bool>
{
  const FareMarket& _fm;

public:
  EqualFM(const FareMarket& fm) : _fm(fm) {}

  bool operator()(FareMarket* fm) { return _fm == *fm; }
};
}

void
ItinAnalyzerService::addRegularFareMarketsForPlusUpCalculation(RexBaseTrx& trx, ExcItin& itin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.isPlusUpCalculationNeeded())
  {
    itin.fareMarket().clear();
    size_t sizeWithoutExcNonPricingFMs = trx.fareMarket().size();

    selectProcesing(trx)->buildFareMarket(trx, static_cast<Itin&>(itin));

    const std::vector<FareMarket*>::iterator beginRegularFMs =
        trx.fareMarket().begin() + sizeWithoutExcNonPricingFMs;

    removeDuplicatedFMs(trx.fareMarket(), sizeWithoutExcNonPricingFMs);

    std::for_each(beginRegularFMs,
                  trx.fareMarket().end(),
                  boost::bind(&Itin::addFareMarketJustForRexPlusUps, &itin, _1));

    std::for_each(beginRegularFMs, trx.fareMarket().end(), BreakIt());
  }
}

void
ItinAnalyzerService::removeDuplicatedFMs(std::vector<FareMarket*>& fareMarkets, size_t regularPart)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  const std::vector<FareMarket*>::iterator beginRegularFMs = fareMarkets.begin() + regularPart;
  std::vector<FareMarket*>::iterator fmi = fareMarkets.begin();
  for (; fmi != beginRegularFMs; ++fmi)
  {
    std::vector<FareMarket*>::iterator toErase =
        std::find_if(beginRegularFMs, fareMarkets.end(), EqualFM(**fmi));

    if (toErase != fareMarkets.end())
      fareMarkets.erase(toErase);
  }
}

void
ItinAnalyzerService::buildFareMarket(RexBaseTrx& trx, ExcItin& itin)
{
  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::buildFareMarket(RexBaseTrx&, ExcItin&)");

  std::vector<FareCompInfo*>& fareComponents = itin.fareComponent();
  std::vector<FareCompInfo*>::iterator i = fareComponents.begin();
  for (; i != fareComponents.end(); ++i)
  {
    FareCompInfo& fc = **i;

    FareMarket* fareMarket = fc.fareMarket();
    if ((fareMarket == nullptr) || fareMarket->travelSeg().empty())
      continue;

    completeExcRtwItin(trx, itin, *fareMarket);

    fareMarket->setOrigDestByTvlSegs();
    fareMarket->travelDate() = itin.travelDate();

    // Set travel boundary
    selectProcesing(trx)->setupAndStoreFareMarket(trx, itin, fareMarket);
  }

  addRegularFareMarketsForPlusUpCalculation(trx, itin);
}

void
ItinAnalyzerService::completeExcRtwItin(RexBaseTrx& trx, ExcItin& itin, FareMarket& fareMarket)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.isExcRtw())
  {
    std::vector<TravelSeg*> travelSegs;
    _itinAnalyzerWrapper.collectSegmentsForRtw(trx, itin, travelSegs);
    if (fareMarket.travelSeg().size() < itin.travelSeg().size())
      fareMarket.travelSeg().push_back(itin.travelSeg().back());
  }
}

std::string
ItinAnalyzerService::getTravelSegOrderStr(const Itin& itin,
                                          const std::vector<TravelSeg*>& travelSegs,
                                          const FareMarket* fm)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::ostringstream tmpStream;

  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  for (; i != travelSegs.end(); ++i)
  {
    tmpStream << itin.segmentOrder(*i);
    if (fm != nullptr)
    {
      if (fm->stopOverTravelSeg().find(*i) != fm->stopOverTravelSeg().end())
        tmpStream << "-O ";
      else
        tmpStream << "-X ";
    }
  }

  return tmpStream.str();
}

std::string
ItinAnalyzerService::getTravelSegOrderStr(const Itin& itin,
                                          const std::vector<std::vector<TravelSeg*>>& sideTrips)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::ostringstream tmpStream;

  std::vector<std::vector<TravelSeg*>>::const_iterator i = sideTrips.begin();
  for (; i != sideTrips.end(); ++i)
    tmpStream << "* " << getTravelSegOrderStr(itin, *i) << " * ";

  return tmpStream.str();
}

bool
ItinAnalyzerService::validateRestrictedCurrencyNation(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  const Loc* saleLoc;
  const PricingRequest* request = trx.getRequest();

  // If the currency is not overridden, no need to check further
  if (trx.getOptions()->currencyOverride().empty())
    return false;

  if (request->salePointOverride().size() != 0)
  {
    saleLoc = trx.dataHandle().getLoc(request->salePointOverride(), time(nullptr));
  }
  else
  {
    saleLoc = request->ticketingAgent()->agentLocation();
  }

  return CurrencyUtil::isRestricted(trx, saleLoc, trx.getOptions()->currencyOverride());
}

void
ItinAnalyzerService::setValidatingCarrier(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (!OBFeesUtils::fallbackObFeesWPA(&trx))
  {
    TktFeesPricingTrx* tktTrx = dynamic_cast<TktFeesPricingTrx*>(&trx);
    if (tktTrx)
      return;
  }

  ValidatingCarrierUpdater validatingCarrier(trx);

  std::vector<Itin*>::const_iterator itinI = trx.itin().begin();

  for (; itinI != trx.itin().end(); itinI++)
  {
    validatingCarrier.update(**itinI);
  }

  typedef PricingTrx::AltDatePairs::value_type ADPair;
  for (ADPair& altDate : trx.altDatePairs())
  {
    validatingCarrier.update(*altDate.second->journeyItin);
  }
}

void
ItinAnalyzerService::setValidatingCarrier(TaxTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  ValidatingCarrierUpdater validatingCarrier(trx);

  std::vector<Itin*>::const_iterator itinI = trx.itin().begin();

  for (; itinI != trx.itin().end(); itinI++)
  {
    if ((*itinI)->validatingCarrier().empty())
      validatingCarrier.update(**itinI);
  }
}

void
ItinAnalyzerService::setFlightFinderValidatingCarrier(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  ValidatingCarrierUpdater validatingCarrier(trx);
  validatingCarrier.updateFlightFinderValidatingCarrier();

  typedef PricingTrx::AltDatePairs::value_type ADPair;
  for (ADPair& altDate : trx.altDatePairs())
  {
    validatingCarrier.update(*altDate.second->journeyItin);
  }
}

void
ItinAnalyzerService::setGeoTravelTypeAndValidatingCarrier(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  ValidatingCarrierUpdater validatingCarrier(trx);

  for (Itin* itin : trx.itin())
  {
    Boundary tvlboundary = _tvlSegAnalysis.selectTravelBoundary(itin->travelSeg());
    ItinUtil::setGeoTravelType(_tvlSegAnalysis, tvlboundary, *itin); // It sets Ticketing Cxr also

    validatingCarrier.update(*itin);

    if (trx.isValidatingCxrGsaApplicable())
    {
      if (!itinHasValidatingCxrData(trx, *itin))
        itin->setItinIndex(DELETED_ITIN_INDEX);

      for (const SimilarItinData& similarItinData : itin->getSimilarItins())
      {
        Itin& childItin = *similarItinData.itin;
        Boundary tvlboundary = _tvlSegAnalysis.selectTravelBoundary(childItin.travelSeg());
        ItinUtil::setGeoTravelType(_tvlSegAnalysis, tvlboundary, childItin);

        validatingCarrier.update(childItin);
        if (!itinHasValidatingCxrData(trx, childItin))
          childItin.setItinIndex(DELETED_ITIN_INDEX);
      }
    }
  }

  // set cat05BookingDateValidationSkip flag only for infini pccs
  if (trx.getRequest()->ticketingAgent()->infiniUser())
    TrxUtil::setInfiniCat05BookingDTSkipFlagInItins(trx);
}

void
ItinAnalyzerService::setItinCurrencies(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::setItinCurrencies(PricingTrx)");
  for (const auto itin : trx.itin())
    ItinUtil::setItinCurrencies(*itin, trx.ticketingDate());
}

void
ItinAnalyzerService::setItinCurrencies(RexBaseTrx& trx)
{
  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::setItinCurrencies(RexBaseTrx)");

  if (trx.excTrxType() == PricingTrx::AF_EXC_TRX &&
      !trx.newItin().empty() && // only for partially refund
      trx.newItin().front()->calcCurrencyOverride().empty()) // when C6Y for new was empty
  {
    // make C6Y on new same as C6Y on exc
    trx.newItin().front()->calcCurrencyOverride() =
        trx.exchangeItin().front()->calculationCurrency();
  }

  const bool oldStat = trx.isAnalyzingExcItin();

  // Set currencies for new itin
  trx.setAnalyzingExcItin(false);
  CurrencyCode newItinCalcCurrency;

  //----------------------------
  std::vector<Itin*>::const_iterator newItinIter = trx.newItin().begin();
  for (; newItinIter != trx.newItin().end(); newItinIter++)
  {
    Itin& itin = **newItinIter;
    ItinUtil::setItinCurrencies(itin, trx.ticketingDate());
  }
  //----------------------------

  if (!trx.newItin().empty())
  {
    newItinCalcCurrency = trx.newItin().front()->calculationCurrency();
  }

  // Set currencies for old itin
  trx.setAnalyzingExcItin(true);
  std::vector<ExcItin*>::const_iterator itinIter = trx.exchangeItin().begin();
  std::vector<ExcItin*>::const_iterator itinIterEnd = trx.exchangeItin().end();

  for (; itinIter != itinIterEnd; itinIter++)
  {
    Itin& itin = **itinIter;
    ItinUtil::setItinCurrencies(itin, trx.originalTktIssueDT());
    if (!newItinCalcCurrency.empty() && (itin.calculationCurrency() != newItinCalcCurrency))
      itin.calculationCurrency() = newItinCalcCurrency;
  }

  trx.setAnalyzingExcItin(oldStat);
}

void
ItinAnalyzerService::setItinCurrencies(RexShoppingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::setItinCurrencies(RexShoppingTrx)");

  Itin& itin = *trx.exchangeItin().front();
  ItinUtil::setItinCurrencies(itin, trx.originalTktIssueDT());
}

void
ItinAnalyzerService::setAgentCommissions(PricingTrx& trx)

{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.getRequest()->ticketingAgent()->agentCommissionType().empty())
    return;

  char theType = trx.getRequest()->ticketingAgent()->agentCommissionType()[0];

  // Percentage or dollar amount
  if (theType == 'P')
  {
    trx.getRequest()->ticketingAgent()->commissionPercent() =
        trx.getRequest()->ticketingAgent()->agentCommissionAmount() / 100.0;
  }
  else if (theType == 'A' || theType == ' ')
  {
    // Use the agents currency
    if (UNLIKELY(trx.getOptions()->currencyOverride().empty()))
    {
      Money money(trx.getRequest()->ticketingAgent()->currencyCodeAgent());
      trx.getRequest()->ticketingAgent()->commissionAmount() =
          trx.getRequest()->ticketingAgent()->agentCommissionAmount() /
          pow(10.0, money.noDec(trx.ticketingDate()));
    }

    else
    {
      Money money(trx.getOptions()->currencyOverride());
      trx.getRequest()->ticketingAgent()->commissionAmount() =
          trx.getRequest()->ticketingAgent()->agentCommissionAmount() /
          pow(10.0, money.noDec(trx.ticketingDate()));
    }
  }
}

//---------------------------------------------------------------------------
// addScheduleGroupMapEntry()
//     In: trx     - Shopping transaction
//         curItin - Itinerary to be added to the map
// Return: bool    - Success of the method
// Adds a schedule (itinerary) to the primary schedule group map
// based on its governing carrier
//---------------------------------------------------------------------------
bool
ItinAnalyzerService::addScheduleGroupMapEntry(ItinIndex& curGroup,
                                              Itin*& curItin,
                                              ItinIndex::ItinCellInfo& itinCellInfo)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (curItin == nullptr)
    return false;

  // Hash the governing carrier
  ItinIndex::Key cxrKey;
  ShoppingUtil::createCxrKey(curItin, cxrKey);

  // Create the schedule connection point key
  ItinIndex::Key scheduleKey;
  ShoppingUtil::createScheduleKey(curItin, scheduleKey);

  // Add entry to the group
  curGroup.addItinCell(curItin, itinCellInfo, cxrKey, scheduleKey);

  // Return success flag
  return true;
}

void
ItinAnalyzerService::getGovCxrAndFareMkt(PricingTrx& trx,
                                         Itin& itin,
                                         bool isPricingOp,
                                         bool isCustomSop,
                                         uint32_t legIndex)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN GOVCXR AND FMKT");
  GoverningCarrier govCxr(&trx);

  // limitation used to check here

  // Get the travel segs
  std::vector<TravelSeg*>& trvSegs = itin.travelSeg();

  // Make sure the travel segment vector is not empty
  if (UNLIKELY(trvSegs.empty()))
  {
    return;
  }

  FareMarket* fareMarket;

  if (fallback::segmentAttributesRefactor(&trx))
  {
    // Set the segment attributes for this schedule
    std::vector<SegmentAttributes> segmentAttributes;
    TravelSegUtil::setSegmentAttributes(trvSegs, segmentAttributes);

    // Create a data handle for the fare market
    trx.dataHandle().get(fareMarket); // lint !e530

    // Set the origin, destination, and boardMultiCity attributes for
    // this fare market
    fareMarket->origin() = segmentAttributes.front().tvlSeg->origin();
    fareMarket->destination() = segmentAttributes.back().tvlSeg->destination();
    fareMarket->boardMultiCity() = segmentAttributes.front().tvlSeg->boardMultiCity();
    fareMarket->offMultiCity() = segmentAttributes.back().tvlSeg->offMultiCity();
    fareMarket->travelDate() = itin.travelDate();

    // Set the travel segments for this fare market
    for (const auto& segAttr : segmentAttributes)
      fareMarket->travelSeg().push_back(segAttr.tvlSeg);
  }
  else
  {
    fareMarket = trx.dataHandle().create<FareMarket>();
    fareMarket->origin() = trvSegs.front()->origin();
    fareMarket->destination() = trvSegs.back()->destination();
    fareMarket->boardMultiCity() = trvSegs.front()->boardMultiCity();
    fareMarket->offMultiCity() = trvSegs.back()->offMultiCity();
    fareMarket->travelDate() = itin.travelDate();
    fareMarket->travelSeg() = trvSegs;
  }

  // Set the travel boundary data
  FareMarket& fareMarketRef = *fareMarket;

  std::vector<TravelSeg*>& tvlSegs = fareMarketRef.travelSeg();
  Boundary tvlBoundary = _tvlSegAnalysis.selectTravelBoundary(tvlSegs);
  ItinUtil::setGeoTravelType(tvlBoundary, fareMarketRef);

  bool fareMarketAdded = selectProcesing(trx)->setupAndStoreFareMarket(trx, itin, fareMarket, legIndex);

  if (fareMarketAdded && trx.getTrxType() == PricingTrx::IS_TRX)
  {
    ShoppingTrx& st = static_cast<ShoppingTrx&>(trx);
    // Add to the ShoppingTrx fare market map for future processing
    if (isCustomSop)
      st.setCustomSolutionFM(fareMarket);

    if (UNLIKELY(ShoppingUtil::isSpanishDiscountApplicableForShopping(trx, &itin)))
    {
      st.setSpanishDiscountFM(fareMarket);
    }
    if (UNLIKELY(trx.getRequest()->cxrOverride() != BLANK_CODE))
    {
      CarrierCode cxrOverride = trx.getRequest()->cxrOverride();
      bool foundSegWithOvrGovCxr = false;
      std::vector<TravelSeg*>::iterator fmTvlSeg = fareMarket->travelSeg().begin();
      for (; fmTvlSeg != fareMarket->travelSeg().end(); ++fmTvlSeg)
      {
        AirSeg* airSeg = dynamic_cast<AirSeg*>(*fmTvlSeg);
        if (airSeg && airSeg->carrier() == cxrOverride)
        {
          foundSegWithOvrGovCxr = true;
          break;
        }
      }
      if (foundSegWithOvrGovCxr)
      {
        fareMarket->primarySector() = *fmTvlSeg;
        fareMarket->governingCarrier() = cxrOverride;
      }
    }
  }

  // Set the inbound/outbound indicators
  if (isPricingOp)
  {
    RepricingTrx* reTrx = dynamic_cast<RepricingTrx*>(&trx);
    if (reTrx && reTrx->getFMDirectionOverride() != FMDirection::UNKNOWN)
      fareMarket->direction() = reTrx->getFMDirectionOverride();
    else
      deprecated_setInboundOutbound(itin, fareMarketRef, trx.dataHandle());
  }

  // Set the fare market break indicator
  FareMarketBuilder::setBreakIndicator(fareMarket, &itin, trx);

  // Select the governing carrier for this itin
  if (fareMarketRef.governingCarrier().empty())
  {
    govCxr.process(fareMarketRef);
  }

  // Set change status
  if (UNLIKELY(!trx.isNotExchangeTrx()))
    fareMarket->setFCChangeStatus(-1 /*itin.pointOfChgSegOrder()*/);

  // Check to see if the governing carrier string is null
  // or empty
  if (UNLIKELY(fareMarketRef.governingCarrier().empty()))
  {
    // If no governing carrier was found, use special processing logic
    govCxr.getGovCxrSpecialCases(fareMarketRef);
  }

  if (UNLIKELY(trx.getTrxType() == PricingTrx::MIP_TRX))
  {
    // Set retransit points
    setRetransits(itin);
  }

  // Push this fare market handle onto the itin vector of fare markets
  FlightFinderTrx* ffTrx = dynamic_cast<FlightFinderTrx*>(&trx);

  if (UNLIKELY((nullptr != ffTrx) && (true == ffTrx->avlInS1S3Request())))
  {
    if (false == fareMarketAdded)
    {
      selectProcesing(trx)->storeFareMarket(trx, fareMarket, itin, false);
    }
  }
  else if (LIKELY(!trx.isIataFareSelectionApplicable()))
  {
    if (false == fareMarketAdded)
      selectProcesing(trx)->storeFareMarket(trx, fareMarket, itin, true);
    else
    {
      bool isISV2 = false;
      if (trx.getTrxType() == PricingTrx::IS_TRX)
      {
        ShoppingTrx& shoppingTrx = static_cast<ShoppingTrx&>(trx);
        isISV2 = !shoppingTrx.isSumOfLocalsProcessingEnabled();
      }

      if (!isISV2)
        selectProcesing(trx)->storeFareMarket(trx, fareMarket, itin, true);
    }
  }
  else if ((trx.getTrxType() == PricingTrx::IS_TRX) && (trx.isIataFareSelectionApplicable()) &&
           (false == fareMarketAdded))
  {
    selectProcesing(trx)->storeFareMarket(trx, fareMarket, itin, true);
  }
}

//---------------------------------------------------------------------------
// selectTicketingCarrier() for Itin
//---------------------------------------------------------------------------
bool
ItinAnalyzerService::selectTicketingCarrier(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<Itin*>::iterator itnItr = trx.itin().begin();

  for (; itnItr != trx.itin().end(); ++itnItr)
  {
    Itin& itin = **itnItr;
    if (!setGeoTravelTypeAndTktCxr(itin))
      return false;
  }

  LOG4CXX_DEBUG(_logger, " Leaving ItinAnalyzerService::selectTicketingCarrier Successfull ");
  return true;
}

bool
ItinAnalyzerService::setGeoTravelTypeAndTktCxr(Itin& itin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  Boundary tvlboundary = _tvlSegAnalysis.selectTravelBoundary(itin.travelSeg());
  ItinUtil::setGeoTravelType(_tvlSegAnalysis, tvlboundary, itin); // It sets Ticketing Cxr also

  if (itin.ticketingCarrier().equalToConst("XX"))
  {
    LOG4CXX_DEBUG(_logger, " Ticketing Carrier Not Found");
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------
// cloneFareMarket()
//---------------------------------------------------------------------------
void
ItinAnalyzerService::cloneFareMarket(FareMarket& fareMarket,
                                     FareMarket& anotherFareMarket,
                                     CarrierCode govCxr)
{
  LOG4CXX_DEBUG(_logger, "Entered ItinAnalyzerService::cloneFareMarket()");
  // initialize iterators.
  std::vector<TravelSeg*>::iterator first = fareMarket.travelSeg().begin();
  std::vector<TravelSeg*>::iterator last = fareMarket.travelSeg().end();
  std::vector<TravelSeg*>::iterator pos = anotherFareMarket.travelSeg().end();

  // populate the clone
  anotherFareMarket.origin() = fareMarket.origin();
  anotherFareMarket.boardMultiCity() = fareMarket.boardMultiCity();
  anotherFareMarket.destination() = fareMarket.destination();
  anotherFareMarket.offMultiCity() = fareMarket.offMultiCity();
  anotherFareMarket.boardMultiCities() = fareMarket.boardMultiCities();
  anotherFareMarket.offMultiCities() = fareMarket.offMultiCities();
  anotherFareMarket.setGlobalDirection(fareMarket.getGlobalDirection());
  anotherFareMarket.geoTravelType() = fareMarket.geoTravelType();
  anotherFareMarket.direction() = fareMarket.direction();
  anotherFareMarket.governingCarrierPref() = fareMarket.governingCarrierPref();
  anotherFareMarket.travelBoundary() = fareMarket.travelBoundary();
  anotherFareMarket.governingCarrier() = govCxr; // assign the GovCxr
  anotherFareMarket.travelSeg().insert(pos, first, last);
  anotherFareMarket.sideTripTravelSeg().insert(anotherFareMarket.sideTripTravelSeg().end(),
                                               fareMarket.sideTripTravelSeg().begin(),
                                               fareMarket.sideTripTravelSeg().end());
  anotherFareMarket.setChildNeeded(fareMarket.isChildNeeded());
  anotherFareMarket.setInfantNeeded(fareMarket.isInfantNeeded());
  anotherFareMarket.setBypassCat19FlagsSet(fareMarket.bypassCat19FlagsSet());
  anotherFareMarket.fareBasisCode() = fareMarket.fareBasisCode();

  if (fareMarket.isMultiPaxUniqueFareBasisCodes())
  {
    anotherFareMarket.createMultiPaxUniqueFareBasisCodes();
    anotherFareMarket.getMultiPaxUniqueFareBasisCodes() =
        fareMarket.getMultiPaxUniqueFareBasisCodes();
  }

  anotherFareMarket.fareCalcFareAmt() = fareMarket.fareCalcFareAmt();
  anotherFareMarket.travelDate() = fareMarket.travelDate();
  anotherFareMarket.fareCompInfo() = fareMarket.fareCompInfo();
  anotherFareMarket.setFmTypeSol(fareMarket.getFmTypeSol());
  anotherFareMarket.setSolComponentDirection(fareMarket.getSolComponentDirection());
  anotherFareMarket.setDualGoverningFlag(fareMarket.isDualGoverning());
  anotherFareMarket.legIndex() = fareMarket.legIndex();
}

//---------------------------------------------------------------------------
// setInboundOutbound()
//---------------------------------------------------------------------------

void
ItinAnalyzerService::setInboundOutbound(PricingTrx& trx, const Itin& itin, FareMarket& fareMarket)
    const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (TrxUtil::isAAAwardPricing(trx) && (fareMarket.geoTravelType() == GeoTravelType::Domestic ||
                                         fareMarket.geoTravelType() == GeoTravelType::Transborder))
    fareMarket.direction() = FMDirection::OUTBOUND;
  else
    fareMarket.direction() = getInboundOutbound(itin, fareMarket, trx.dataHandle());
}

FMDirection
ItinAnalyzerService::getInboundOutbound(const Itin& itin,
                                        const FareMarket& fareMarket,
                                        DataHandle& dataHandle) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // start from origin point, always outbound
  if (itin.segmentOrder(fareMarket.travelSeg().front()) == 1)
  {
    return FMDirection::OUTBOUND;
  }

  if (LIKELY(itin.travelSeg().size() > 1))
  {
    if (itin.geoTravelType() == GeoTravelType::International &&
        (fareMarket.geoTravelType() == GeoTravelType::ForeignDomestic ||
         fareMarket.geoTravelType() == GeoTravelType::Domestic ||
         fareMarket.geoTravelType() == GeoTravelType::Transborder) &&
        LocUtil::isWithinSameCountry(GeoTravelType::International,
                                     false,
                                     *itin.travelSeg().front()->origin(),
                                     *fareMarket.origin()))
    {
      return FMDirection::OUTBOUND;
    }

    if (itin.geoTravelType() == GeoTravelType::Domestic || // Domestic Itin
        itin.geoTravelType() == GeoTravelType::ForeignDomestic)
    {
      if (fareMarket.destination() ==
          itin.travelSeg().front()->origin()) // going back to itin origin
      {
        return FMDirection::INBOUND;
      }
    }
    else // Intl Itin
    {
      bool isGoingBackToSameNation =
          (fareMarket.destination()->nation() == itin.travelSeg().front()->origin()->nation());
      if (isGoingBackToSameNation ||
          (LocUtil::isDomesticUSCA(*fareMarket.destination()) &&
           LocUtil::isDomesticUSCA(*itin.travelSeg().front()->origin())) ||
          (LocUtil::isScandinavia(*fareMarket.destination()) &&
           LocUtil::isScandinavia(*itin.travelSeg().front()->origin())) ||
          (LocUtil::isNetherlandsAntilles(*fareMarket.destination()) &&
           LocUtil::isNetherlandsAntilles(*itin.travelSeg().front()->origin())) ||
          (LocUtil::isRussia(*fareMarket.destination()) &&
           LocUtil::isRussia(*itin.travelSeg().front()->origin())))
      {
        return FMDirection::INBOUND;
      }
    }
  }

  if (LocUtil::oneOfNetherlandAntilles(*fareMarket.destination()) &&
      !LocUtil::oneOfNetherlandAntilles(*fareMarket.origin()))
  {
    int fmOriginSegmentOrder(itin.segmentOrder(fareMarket.travelSeg().front()));
    for (std::vector<TravelSeg*>::const_iterator it(itin.travelSeg().begin()),
         itend(itin.travelSeg().end());
         it != itend;
         ++it)
    {
      const TravelSeg* ts(*it);
      if (itin.segmentOrder(ts) < fmOriginSegmentOrder)
      {
        if (LocUtil::oneOfNetherlandAntilles(*ts->origin()) ||
            LocUtil::oneOfNetherlandAntilles(*ts->destination()))
        {
          return FMDirection::UNKNOWN;
        }
      }
      else
      {
        break;
      }
    }
  }

  if (hasRetransit(fareMarket.travelSeg()) ||
      LocUtil::isSameCity(
          fareMarket.destination()->loc(), itin.travelSeg().front()->origin()->loc(), dataHandle))
  {
    return FMDirection::UNKNOWN;
  }

  return FMDirection::OUTBOUND;
}

bool
ItinAnalyzerService::hasRetransit(const std::vector<TravelSeg*>& segs) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // Retransit logic -
  // If the fare market is transiting back to a country that has been
  // visited previously, this called "retransited" and the directionality
  // cannot be determined.  The fare market cannot include the first
  // or last travel segment of the itinerary
  for (const TravelSeg* tvlSeg : segs)
  {
    if (tvlSeg->retransited())
      return true;
  }
  return false;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// setInboundOutbound() for FlightFInder
//---------------------------------------------------------------------------
void
ItinAnalyzerService::setInboundOutbound(FlightFinderTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<ShoppingTrx::Leg>& leg = trx.legs();
  // Create leg vector iterators
  std::vector<ShoppingTrx::Leg>::iterator legIter = leg.begin();
  std::vector<ShoppingTrx::Leg>::iterator legEndIter = leg.end();
  uint32_t legId = 0;

  // get the journey itin
  Itin*& journeyItin = trx.journeyItin();

  // Loop through the leg vector
  for (; legIter != legEndIter; ++legIter, ++legId)
  {
    // Get leg reference
    ShoppingTrx::Leg& curLeg = (*legIter);
    ItinIndex& cxrIndex = curLeg.carrierIndex();

    // Get the carrier iterators
    ItinIndex::ItinMatrixIterator iMIter = cxrIndex.root().begin();
    ItinIndex::ItinMatrixIterator iMEndIter = cxrIndex.root().end();

    // itin matrix iteration loop
    //
    for (; iMIter != iMEndIter; ++iMIter)
    {
      // Get the leaf
      ItinIndex::ItinCell* curCell =
          ShoppingUtil::retrieveDirectItin(trx, legId, iMIter->first, ItinIndex::CHECK_NOTHING);
      if (!curCell)
      {
        continue;
      }
      // Get the direct itinerary for this carrier
      Itin* thruFareDirectItin = curCell->second;
      if (thruFareDirectItin == nullptr)
      {
        continue;
      }
      // get the thru-fare market
      FareMarket* fM = thruFareDirectItin->fareMarket().front();
      if (fM == nullptr)
      {
        continue;
      }
      std::vector<TravelSeg*> tempTravelSeg;
      tempTravelSeg.push_back(journeyItin->travelSeg()[legId]);
      fM->travelSeg().swap(tempTravelSeg);
      setInboundOutbound(trx, *journeyItin, *fM);
      fM->travelSeg().swap(tempTravelSeg);
    }
  }
}

//---------------------------------------------------------------------------
// setTripCharacteristics()
//---------------------------------------------------------------------------
void
ItinAnalyzerService::setTripCharacteristics(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN SET TRIP-CHARACTERISTICS");

  std::vector<Itin*>::iterator iter = trx.itin().begin();
  for (; iter != trx.itin().end(); iter++)
  {
    setTripCharacteristics(*iter);
  }
}

void
ItinAnalyzerService::setTripCharacteristics(Itin* itin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (!itin)
    return;

  std::vector<TravelSeg*> tvlSegs = itin->travelSeg();
  if (tvlSegs.empty())
    return;

  TravelSeg* firstSeg = nullptr;
  TravelSeg* lastSeg = nullptr;
  unsigned int segsSize = tvlSegs.size();

  bool allFareMktsOutBound = true;

  for (const auto fareMarket : itin->fareMarket())
  {
    if (fareMarket->direction() != FMDirection::OUTBOUND)
    {
      allFareMktsOutBound = false;
      break;
    }
  }

  // One way, or round trip?
  if (segsSize == 1 || allFareMktsOutBound)
    itin->tripCharacteristics().set(Itin::OneWay, true);
  else
  {
    firstSeg = tvlSegs[0];
    lastSeg = tvlSegs[tvlSegs.size() - 1];
    if (firstSeg->boardMultiCity() == lastSeg->offMultiCity())
      itin->tripCharacteristics().set(Itin::RoundTrip, true);
    else
      itin->tripCharacteristics().set(Itin::OneWay, true);
  }

  // Trip originated in US?
  firstSeg = tvlSegs[0];
  if (firstSeg->origin()->nation() == UNITED_STATES)
    itin->tripCharacteristics().set(Itin::OriginatesUS, true);

  // Trip terminates in US?
  lastSeg = tvlSegs[tvlSegs.size() - 1];
  if (lastSeg->destination()->nation() == UNITED_STATES)
    itin->tripCharacteristics().set(Itin::TerminatesUS, true);

  // Trip originates in Canada Maritime?
  firstSeg = tvlSegs[0];
  if (firstSeg->origin()->nation() == CANADA)
    itin->tripCharacteristics().set(Itin::OriginatesCanadaMaritime, true);

  // Trip US only
  TravelSeg* tvlSeg;
  bool USonly = true;
  for (unsigned int i = 0; i < segsSize; i++)
  {
    tvlSeg = tvlSegs[i];
    if (tvlSeg->origin()->nation() == UNITED_STATES &&
        tvlSeg->destination()->nation() == UNITED_STATES)
    {
      continue;
    }
    else
    {
      USonly = false;
      break;
    }
  }
  itin->tripCharacteristics().set(Itin::USOnly, USonly);

  // Trip Canada only
  bool CanadaOnly = true;
  for (unsigned int i = 0; i < segsSize; i++)
  {
    firstSeg = tvlSegs[i];
    if (firstSeg->origin()->nation() == CANADA && firstSeg->destination()->nation() == CANADA)
    {
      continue;
    }
    else
    {
      CanadaOnly = false;
      break;
    }
  }
  itin->tripCharacteristics().set(Itin::CanadaOnly, CanadaOnly);

  // Set geoTravelType and stopOver

  std::vector<bool> stopOver = TravelSegUtil::calculateStopOvers(tvlSegs, itin->geoTravelType());
  for (unsigned int i = 0; i < segsSize; i++)
  {
    _tvlSegAnalysis.setGeoTravelType(tvlSegs[i]);
    tvlSegs[i]->stopOver() = stopOver[i];
  }

  // Travel totally in or between Russian nations RU/XU
  //
  bool RussiaOnly = true;

  if (ItinUtil::isRussian(itin))
    itin->tripCharacteristics().set(Itin::RussiaOnly, RussiaOnly);
  else
  {
    RussiaOnly = false;
    itin->tripCharacteristics().set(Itin::RussiaOnly, RussiaOnly);
  }

  //***************************************
  // Temparily put CollectStopOverTravelSeg here due to it has to be after
  // stopOver is set for all Travel Seg.
  CollectStopOverTravelSeg collStopOvers(*itin);

  std::vector<FareMarket*>& fareMarkets = itin->fareMarket();

  std::for_each(fareMarkets.begin(), fareMarkets.end(), collStopOvers);

  // Dump fare markets
  if (IS_DEBUG_ENABLED(_logger))
  {
    int marketIndex = 1;
    SUPPRESS_UNUSED_WARNING(marketIndex);
    LOG4CXX_DEBUG(_logger, "Number of fare markets: " << fareMarkets.size());

    std::vector<FareMarket*>::iterator iter = fareMarkets.begin();
    for (; iter != fareMarkets.end(); ++iter)
    {
      FareMarket* fm = (*iter);
      LOG4CXX_DEBUG(_logger, "**************** FARE MARKET " << marketIndex++);
      LOG4CXX_DEBUG(_logger, "Board: " << fm->boardMultiCity() << "  Off: " << fm->offMultiCity());
      LOG4CXX_DEBUG(_logger, "  governing carrier: " << fm->governingCarrier());
      LOG4CXX_DEBUG(_logger,
                    "  includes: " << fm->travelSeg().size() << " travel segments: "
                                   << getTravelSegOrderStr(*itin, fm->travelSeg(), fm));
      LOG4CXX_DEBUG(_logger,
                    "  includes: " << fm->sideTripTravelSeg().size() << " side trips: "
                                   << getTravelSegOrderStr(*itin, fm->sideTripTravelSeg()));
      LOG4CXX_DEBUG(_logger, "  breakIndicator: " << fm->breakIndicator());
    }
  }
  //*******************************************
}

//---------------------------------------------------------------------------
// setIntlSalesIndicator()
//---------------------------------------------------------------------------
void
ItinAnalyzerService::setIntlSalesIndicator(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN SET INTL-INDICATOR");

  const Loc* ticketingLoc = getTicketLoc(trx);
  const Loc* saleLoc = getSalesLoc(trx);

  std::vector<Itin*>::iterator iter = trx.itin().begin();
  for (; iter != trx.itin().end(); iter++)
  {
    setIntlSalesIndicator(**iter, *ticketingLoc, *saleLoc);
  }
}

void
ItinAnalyzerService::setIntlSalesIndicator(Itin& itin, const Loc& ticketingLoc, const Loc& saleLoc)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<TravelSeg*>& tvlSegs = itin.travelSeg();
  if (tvlSegs.empty())
  {
    itin.intlSalesIndicator() = Itin::UNKNOWN;
    return;
  }

  TravelSeg* tvlSeg = tvlSegs.front();

  if (tvlSeg->segmentType() == Air)
  {
    if (LocUtil::isSameISINation(ticketingLoc, *tvlSeg->origin()))
    {
      if (LocUtil::isSameISINation(saleLoc, *tvlSeg->origin()))
        itin.intlSalesIndicator() = Itin::SITI;
      else
        itin.intlSalesIndicator() = Itin::SOTI;
    }
    else
    {
      if (LocUtil::isSameISINation(saleLoc, *tvlSeg->origin()))
        itin.intlSalesIndicator() = Itin::SITO;
      else
        itin.intlSalesIndicator() = Itin::SOTO;
    }
  }
}

//---------------------------------------------------------------------------
// setRetransits()
//---------------------------------------------------------------------------
void
ItinAnalyzerService::setRetransits(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN SET RETRANSITS");

  std::vector<Itin*>::iterator iter = trx.itin().begin();
  std::vector<Itin*>::iterator iterEnd = trx.itin().end();
  for (; iter != iterEnd; iter++)
  {
    Itin& itin = **iter;
    setRetransits(itin);
  }
}

void
ItinAnalyzerService::setRetransits(Itin& itin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (itin.geoTravelType() == GeoTravelType::Domestic ||
      itin.geoTravelType() == GeoTravelType::ForeignDomestic) // For Domestic or Foreign Domestic,
                                                              // check retransits on city.
  {
    std::set<std::string> departedCitys;

    // First segment is never retransit.
    int tvlSegSize = itin.travelSeg().size();
    departedCitys.insert(itin.travelSeg()[0]->boardMultiCity());

    for (int i = 1; i < tvlSegSize; i++)
    {
      TravelSeg* currentSeg = itin.travelSeg()[i];
      currentSeg->retransited() = false;

      if (departedCitys.find(currentSeg->offMultiCity()) != departedCitys.end() ||
          departedCitys.find(currentSeg->boardMultiCity()) != departedCitys.end())
      {
        currentSeg->retransited() = true;
        LOG4CXX_INFO(_logger,
                     "Retransit point: [" << currentSeg->boardMultiCity() << "], ["
                                          << currentSeg->offMultiCity() << "], nation ["
                                          << currentSeg->destination()->nation() << "]");
      }

      departedCitys.insert(currentSeg->boardMultiCity());
    }
  }
  else // For International, check retransits on nation and city.
  {
    std::set<std::string> departedNations;
    std::set<std::string> departedCitys;

    // If travel is wholly within Europe then there's no retransit to Europe
    const bool checkTransitEurope = !itin.tripCharacteristics().isSet(Itin::EuropeOnly);

    // First segment is never retransit.
    bool transitEurope =
        checkTransitEurope && ((itin.travelSeg().front()->origin()->subarea() == EUROPE) ||
                               (itin.travelSeg().front()->destination()->subarea() == EUROPE));

    int tvlSegSize = itin.travelSeg().size();
    departedNations.insert(itin.travelSeg().front()->origin()->nation());
    if (LocUtil::isDomesticUSCA(*itin.travelSeg().front()->origin()))
    {
      departedNations.insert(UNITED_STATES); // US and CA considered one country
      departedNations.insert(CANADA);
    }
    else if (LocUtil::isScandinavia(*itin.travelSeg().front()->origin()))
    {
      departedNations.insert(DENMARK); // Denmark, Norway and Sweden considered one country
      departedNations.insert(NORWAY);
      departedNations.insert(SWEDEN);
    }

    departedCitys.insert(itin.travelSeg()[0]->boardMultiCity());

    for (int i = 1; i < tvlSegSize; i++)
    {
      TravelSeg* currentSeg = itin.travelSeg()[i];
      currentSeg->retransited() = false;

      if (currentSeg->origin()->nation() !=
          currentSeg->destination()->nation()) // International travel segment
      {
        if (departedNations.find(currentSeg->destination()->nation()) != departedNations.end())
        {
          currentSeg->retransited() = true;
          LOG4CXX_INFO(_logger,
                       "Retransit point: [" << currentSeg->boardMultiCity() << "],  nation ["
                                            << currentSeg->origin()->nation() << "], ["
                                            << currentSeg->offMultiCity() << "], nation ["
                                            << currentSeg->destination()->nation() << "]");
        }
        else if (transitEurope)
        {
          if (currentSeg->destination()->subarea() == EUROPE)
          {
            currentSeg->retransited() = true;
            LOG4CXX_INFO(_logger,
                         "Retransit point: [" << currentSeg->boardMultiCity() << "],  nation ["
                                              << currentSeg->origin()->nation() << "], ["
                                              << currentSeg->offMultiCity() << "], nation ["
                                              << currentSeg->destination()->nation() << "]");
          }
        }

        if (checkTransitEurope && !transitEurope)
        {
          transitEurope = (currentSeg->destination()->subarea() == EUROPE);
        }
      }
      else // Domestic travel segment
      {
        if (departedCitys.find(currentSeg->offMultiCity()) != departedCitys.end() ||
            departedCitys.find(currentSeg->boardMultiCity()) != departedCitys.end())
        {
          currentSeg->retransited() = true;
          LOG4CXX_INFO(_logger,
                       "Retransit point: [" << currentSeg->boardMultiCity() << "], ["
                                            << currentSeg->offMultiCity() << "], nation ["
                                            << currentSeg->destination()->nation() << "]");
        }
      }

      departedNations.insert(currentSeg->origin()->nation());

      if (LocUtil::isDomesticUSCA(*currentSeg->origin()))
      {
        departedNations.insert(UNITED_STATES); // US and CA considered one country

        departedNations.insert(CANADA);
      }
      else if (LocUtil::isScandinavia(*currentSeg->origin()))
      {
        departedNations.insert(DENMARK); // Denmark, Norway and Sweden considered one country
        departedNations.insert(NORWAY);
        departedNations.insert(SWEDEN);
      }

      departedCitys.insert(currentSeg->boardMultiCity());
    }
  }
}

void
ItinAnalyzerService::setOpenSegFlag(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<Itin*>::iterator iter = trx.itin().begin();
  std::vector<Itin*>::iterator iterEnd = trx.itin().end();
  for (; iter != iterEnd; iter++)
  {
    Itin& itin = **iter;
    setOpenSegFlag(itin);
  }
}

void
ItinAnalyzerService::setOpenSegFlag(Itin& itin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<TravelSeg*>& tvlSegs = itin.travelSeg();
  std::vector<TravelSeg*>::reverse_iterator tvlSegIter = tvlSegs.rbegin();
  std::vector<TravelSeg*>::reverse_iterator tvlSegIterEnd =
      tvlSegs.rend() - 1; // First segment is never set this flag.
  for (; tvlSegIter != tvlSegIterEnd; tvlSegIter++)
  {
    TravelSeg& tvlSeg = **tvlSegIter;
    if (tvlSeg.segmentType() == Open && tvlSeg.pssDepartureDate().empty())
      tvlSeg.openSegAfterDatedSeg() = true;
    else
      break; // Found first segment with date
  }
}

//---------------------------------------------------------------------------
// setFareMarketCOS()
//---------------------------------------------------------------------------
void
ItinAnalyzerService::setFareMarketCOS(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN SET FMKTCOS");

  if (dynamic_cast<BrandingTrx*>(&trx) != nullptr)
    return;

  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    IntlJourneyUtil::constructJourneyInfo(trx);
  }

  for (Itin* itin : trx.itin())
  {
    if (itin->dcaSecondCall())
      continue;

    for (FareMarket* fareMarket : itin->fareMarket())
    {
      if (!fareMarket->classOfServiceVec().empty())
      {
        if (trx.getTrxType() == PricingTrx::MIP_TRX)
          ShoppingUtil::mergeFMCOSBasedOnAvailBreak(trx, itin, fareMarket);
        continue; // next fare market
      }

      if (trx.getTrxType() == PricingTrx::MIP_TRX)
      {
        ShoppingUtil::getFMCOSBasedOnAvailBreak(trx, itin, fareMarket);
        if (LIKELY(!fareMarket->classOfServiceVec().empty()))
          continue; // next fare market
      }

      for (TravelSeg* ts : fareMarket->travelSeg())
      {
        fareMarket->classOfServiceVec().push_back(&ts->classOfService());
      }
    }
  }
}

void
ItinAnalyzerService::checkJourneyAndGetCOS(FlightFinderTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<ShoppingTrx::Leg>::iterator legIter = trx.legs().begin();
  std::vector<ShoppingTrx::Leg>::iterator legIterEnd = trx.legs().end();

  if ((FlightFinderTrx::STEP_4 == trx.bffStep()) || (FlightFinderTrx::STEP_6 == trx.bffStep()))
  {
    ++legIter;
  }

  for (; legIter != legIterEnd; ++legIter)
  {
    std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter = legIter->sop().begin();
    std::vector<ShoppingTrx::SchedulingOption>::iterator sopIterEnd = legIter->sop().end();

    for (; sopIter != sopIterEnd; ++sopIter)
    {
      if (sopIter->getDummy())
      {
        continue;
      }

      sopIter->thrufareClassOfService().clear();

      Itin*& curItin = sopIter->itin();

      // Find carrierPreference for each airSeg
      bool differentCarrier = false;
      bool consecutiveSameCxr = false;

      saveCarrierPref(differentCarrier, consecutiveSameCxr, *curItin, trx);

      std::vector<TravelSeg*>::const_iterator segIter = curItin->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator segIterEnd = curItin->travelSeg().end();

      if (false == differentCarrier)
      {
        // All travel segments have the same carrier

        AirSeg* airSeg = dynamic_cast<AirSeg*>(*segIter);

        if (nullptr == airSeg)
        {
          LOG4CXX_ERROR(_logger,
                        "ItinAnalyzerService::checkJourneyAndGetCOS - Air segment is NULL.");
          continue;
        }

        // If we've got same carrier in all travel segments always use
        // thru availability.
        getThruAvailForFF(trx, curItin->travelSeg(), *sopIter);
      }
      else
      {
        checkFFInterIntraLineAvail(trx, curItin, *sopIter);
      }

      if (sopIter->thrufareClassOfService().size() != sopIter->itin()->travelSeg().size())
      {
        LOG4CXX_ERROR(_logger,
                      "ItinAnalyzerService::checkJourneyAndGetCOS - Travel segment vector "
                      "and thru fare cos vector have different size.");
      }
    }
  }

  return;
}

void
ItinAnalyzerService::checkFFInterIntraLineAvail(FlightFinderTrx& trx,
                                                Itin* itin,
                                                ShoppingTrx::SchedulingOption& sop)
{
  Itin* curItin = itin;
  std::vector<TravelSeg*>::const_iterator segIter = curItin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator segIterEnd = curItin->travelSeg().end();

  for (; segIter != segIterEnd; ++segIter)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*segIter);

    if (!airSeg)
      continue;

    bool findConsecutive = false;

    std::vector<TravelSeg*> tempTravelSeg;
    tempTravelSeg.push_back(*segIter);

    std::vector<TravelSeg*>::const_iterator segIterNext = segIter;
    *segIterNext++;

    for (; segIterNext != segIterEnd; ++segIterNext, ++segIter)
    {
      AirSeg* nextAirSeg = dynamic_cast<AirSeg*>(*segIterNext);

      if (!nextAirSeg)
        continue;

      if ((nextAirSeg->carrier() == airSeg->carrier()) ||
          TrxUtil::interlineAvailabilityApply(trx, airSeg, nextAirSeg) ||
          TrxUtil::intralineAvailabilityApply(
              trx, airSeg->marketingCarrierCode(), nextAirSeg->marketingCarrierCode()))
      {
        tempTravelSeg.push_back(*segIterNext);
        findConsecutive = true;
      }
      else
      {
        break;
      }
    }

    if (findConsecutive == false)
    {
      getLocalAvailForFF(trx, tempTravelSeg, sop);
    }
    else
    {
      getThruAvailForFF(trx, tempTravelSeg, sop);
    }
  }
}

void
ItinAnalyzerService::getThruAvailForFF(FlightFinderTrx& trx,
                                       std::vector<TravelSeg*>& travelSegVec,
                                       ShoppingTrx::SchedulingOption& sop)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<ClassOfServiceList>& cosListVec =
      ShoppingUtil::getThruAvailability(trx, travelSegVec);

  updateSopAvailability(sop, cosListVec);
}

void
ItinAnalyzerService::getLocalAvailForFF(FlightFinderTrx& trx,
                                        std::vector<TravelSeg*>& travelSegVec,
                                        ShoppingTrx::SchedulingOption& sop)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<ClassOfServiceList>& cosListVec =
      ShoppingUtil::getLocalAvailability(trx, travelSegVec);

  updateSopAvailability(sop, cosListVec);
}

void
ItinAnalyzerService::updateSopAvailability(ShoppingTrx::SchedulingOption& sop,
                                           std::vector<ClassOfServiceList>& cosListVec)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  for (auto& elem : cosListVec)
  {
    sop.thrufareClassOfService().push_back(&elem);
  }
}

void
ItinAnalyzerService::saveCarrierPref(bool& differentCarrier,
                                     bool& consecutiveSameCxr,
                                     Itin& curItin,
                                     FlightFinderTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // Find carrier preference for each airSeg
  std::vector<TravelSeg*>::const_iterator segIter = curItin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator segIterEnd = curItin.travelSeg().end();

  const CarrierPreference* cp = nullptr;
  CarrierCode saveCarrier = BLANK_CODE;

  for (; segIter != segIterEnd; ++segIter)
  {
    AirSeg* curAirSeg = dynamic_cast<AirSeg*>(*segIter);

    if (nullptr == curAirSeg)
    {
      LOG4CXX_ERROR(_logger, "ItinAnalyzerService::saveCarrierPref - Air segment is NULL.");
      continue;
    }

    const CarrierCode& carrier = curAirSeg->carrier();

    if (carrier.empty())
    {
      continue;
    }

    const DateTime& travelDate = curAirSeg->departureDT();
    cp = trx.dataHandle().getCarrierPreference(carrier, travelDate);
    curAirSeg->carrierPref() = cp;

    if (saveCarrier == BLANK_CODE)
    {
      saveCarrier = carrier;
    }
    else if (saveCarrier != carrier)
    {
      differentCarrier = true;
      saveCarrier = carrier;
    }
    else if (saveCarrier == carrier)
    {
      consecutiveSameCxr = true;
    }
  }
}

//---------------------------------------------------------------------------
// setCurrencyOverride()
//---------------------------------------------------------------------------
void
ItinAnalyzerService::setCurrencyOverride(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN SET CUR OVERRIDE");
  PricingRequest* request = trx.getRequest();
  PricingOptions* options = trx.getOptions();

  if (!options->currencyOverride().empty())
    return;

  if (!request->salePointOverride().empty())
  {
    if (CurrencyUtil::getSaleLocOverrideCurrency(
            request->salePointOverride(), options->currencyOverride(), request->ticketingDT()))
    {
      return;
    }
  }

  options->currencyOverride() = request->ticketingAgent()->currencyCodeAgent();
}

//---------------------------------------------------------------------------
// setSortTaxByOrigCity()
//---------------------------------------------------------------------------
void
ItinAnalyzerService::setSortTaxByOrigCity(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData(trx, "ITIN SET SORT-TAX");
  std::string userAppl = SABRE_USER; // default
  std::string userPCC = trx.getRequest()->ticketingAgent()->tvlAgencyPCC();
  std::string cxrCode = trx.getRequest()->ticketingAgent()->cxrCode();

  if (cxrCode == "1F")
    userAppl = "INFI";
  else if (cxrCode == "1J")
    userAppl = "AXES";
  else if (cxrCode == "1B")
    userAppl = "ABAC";

  if (userPCC.empty())
    userPCC = trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC();

  const std::vector<FareCalcConfig*>& fareCalcConfig =
      trx.dataHandle().getFareCalcConfig('C', userAppl, userPCC);

  if (fareCalcConfig.empty())
    return;

  std::vector<FareCalcConfig*>::const_iterator iter = fareCalcConfig.begin();
  const Loc* loc =
      trx.dataHandle().getLoc(trx.getRequest()->ticketingAgent()->agentCity(), time(nullptr));
  if (loc == nullptr)
  {
    std::string errMsg = "UNKNOWN TICKET AGENT CITY ";
    errMsg += trx.getRequest()->ticketingAgent()->agentCity();
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, errMsg.c_str());
  }

  for (; iter != fareCalcConfig.end(); iter++)
  {
    if (((*iter)->loc1().loc().empty() && (*iter)->loc1().locType() == ' ') ||
        LocUtil::isInLoc(*loc, (*iter)->loc1().locType(), (*iter)->loc1().loc(), "ATP", RESERVED))
    {
      trx.getOptions()->sortTaxByOrigCity() = (*iter)->taxPlacementInd();
      return;
    }
  }
}

void
ItinAnalyzerService::removeDeletedItins(std::vector<Itin*>& itins)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<Itin*> orphans;

  for (Itin*& itin : itins)
  {
    if (itin->getItinIndex() == DELETED_ITIN_INDEX)
    {
      // if it has child itins, we need to keep them.

      Itin* firstValidChildItin = nullptr;

      for (const SimilarItinData& similarItinData : itin->getSimilarItins())
      {
        Itin* childItin = similarItinData.itin;
        if (childItin->getItinIndex() != DELETED_ITIN_INDEX)
        {
          firstValidChildItin = childItin;
          orphans.push_back(firstValidChildItin);
          continue;
        }

        if (firstValidChildItin && (childItin->getItinIndex() != DELETED_ITIN_INDEX))
          firstValidChildItin->addSimilarItin(childItin);
      }

      itin = nullptr; // to be removed below
    }
    else if (!itin->getSimilarItins().empty())
    {
      std::vector<Itin*> removedItins;

      for (const SimilarItinData& similarItinData : itin->getSimilarItins())
      {
        Itin* childItin = similarItinData.itin;
        if (childItin->getItinIndex() == DELETED_ITIN_INDEX)
          removedItins.push_back(childItin);
      }

      for (Itin* removedItin : removedItins)
        itin->eraseSimilarItin(removedItin);
    }
  }

  Itin* tempItin = nullptr;
  itins.erase(std::remove(itins.begin(), itins.end(), tempItin), itins.end());

  itins.insert(itins.end(), orphans.begin(), orphans.end());

  if (itins.empty())
    throw ErrorResponseException(ErrorResponseException::NO_FLIGHTS_FOUND,
                                 "NO SOLUTION PASSED INTERLINE TICKETING VALIDATION");
}

void
ItinAnalyzerService::validateTicketingAgreement(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "VALIDATE TICKETING AGREEMENT");

  Diag988Collector* collector = nullptr;

  if (trx.diagnostic().diagnosticType() == Diagnostic988)
  {
    DCFactory* factory = DCFactory::instance();
    collector = dynamic_cast<Diag988Collector*>(factory->create(trx));
    TSE_ASSERT(collector != nullptr);

    collector->enable(Diagnostic988);
    collector->outputHeader();
    collector->flushMsg();
  }

  if ((trx.getOptions()->MIPWithoutPreviousIS() == false) ||
      (trx.getOptions()->validateTicketingAgreement() == false))
  {
    return;
  }

  if (!InterlineTicketCarrier::isPriceInterlineActivated(trx))
  {
    if (collector)
    {
      *collector << "IET PRICING IS NOT ACTIVE\n";
      collector->flushMsg();
    }

    return;
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic988)
  {
    (*collector) << "MIP UP FRONT VALIDATION\n";
  }

  InterlineTicketCarrier interlineTicketCarrierData;

  int itinSize = trx.itin().size();

  for (int itinIndex = 0; itinIndex < itinSize; itinIndex++)
  {
    Itin& itin = *(trx.itin()[itinIndex]);
    std::string validationMessage;
    bool ok = interlineTicketCarrierData.validateInterlineTicketCarrierAgreement(
        trx, itin.validatingCarrier(), itin.travelSeg(), &validationMessage);
    if (trx.diagnostic().diagnosticType() == Diagnostic988)
    {
      (*collector) << interlineTicketCarrierData.getInterlineCarriersForValidatingCarrier(
          trx, itin.validatingCarrier());

      collector->outputVITAData(trx, itin, ok, validationMessage);
      collector->flushMsg();
    }

    if (!ok)
      trx.itin()[itinIndex]->setItinIndex(DELETED_ITIN_INDEX);
  }
}

void
ItinAnalyzerService::removeHigherCabins(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<TravelSeg*>::const_iterator trvSegIter;
  std::vector<TravelSeg*>::const_iterator trvSegEndIter = trx.travelSeg().end();

  // process travel segments
  LOG4CXX_DEBUG(_logger,
                "ItinAnalyzerService::removeHigherCabins - process travelSeg().classOfService()");
  trvSegIter = trx.travelSeg().begin();
  for (; trvSegIter != trvSegEndIter; ++trvSegIter)
  {
    TravelSeg* travelSeg = *trvSegIter;
    removeHigherCabins(trx, travelSeg->classOfService(), travelSeg);
  }

  // process avl
  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::removeHigherCabins - process trx.AvailabilityMap()");
  AvailabilityMap::const_iterator avlIter = trx.availabilityMap().begin();
  AvailabilityMap::const_iterator avlEndIter = trx.availabilityMap().end();
  for (; avlIter != avlEndIter; ++avlIter)
  {
    std::vector<ClassOfServiceList>& cosVec = *avlIter->second;
    std::vector<uint16_t> segIDs;
    ShoppingUtil::getIdsVecForKey(avlIter->first, segIDs);

    for (size_t i = 0; i < segIDs.size(); ++i)
    {
      uint16_t segID = segIDs[i];

      trvSegIter = trx.travelSeg().begin();
      for (; trvSegIter != trvSegEndIter; ++trvSegIter)
      {
        TravelSeg* travelSeg = *trvSegIter;
        if (segID == travelSeg->originalId())
        {
          removeHigherCabins(trx, cosVec[i], travelSeg);
          break;
        }
      }
    }
  }

  // process thru avl
  LOG4CXX_DEBUG(_logger,
                "ItinAnalyzerService::removeHigherCabins - process trx.ThruFareAvailabilityMap()");
  PricingTrx::ThruFareAvailabilityMap::iterator thruAvlIter =
      trx.maxThruFareAvailabilityMap().begin();
  PricingTrx::ThruFareAvailabilityMap::iterator thruAvlEndIter =
      trx.maxThruFareAvailabilityMap().end();
  for (; thruAvlIter != thruAvlEndIter; ++thruAvlIter)
  {
    removeHigherCabins(trx, thruAvlIter->second, thruAvlIter->first);
  }

  // process fare markets avl
  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::removeHigherCabins - process trx.fareMarket()");
  std::vector<FareMarket*>::const_iterator fmIter = trx.fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEndIter = trx.fareMarket().end();
  for (; fmIter != fmEndIter; ++fmIter)
  {
    FareMarket* fareMarket = *fmIter;

    const std::vector<std::vector<ClassOfService*>*>& cosVec = fareMarket->classOfServiceVec();
    const std::vector<TravelSeg*>& travelSegVec = fareMarket->travelSeg();

    for (size_t i = 0; i < cosVec.size(); ++i)
    {
      removeHigherCabins(trx, *cosVec[i], travelSegVec[i]);
    }
  }
}

void
ItinAnalyzerService::removeHigherCabins(PricingTrx& trx,
                                        std::vector<ClassOfService*>& classOfService,
                                        TravelSeg* travelSeg)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  int16_t legIndex = travelSeg->legId();
  if (UNLIKELY(legIndex < 0 || legIndex >= static_cast<int>(trx.legPreferredCabinClass().size())))
  {
    LOG4CXX_ERROR(_logger, "ItinAnalyzerService::removeHigherCabins - leg ID out of range");
    return;
  }
  CabinType preferredCabin = trx.legPreferredCabinClass()[legIndex];

  LOG4CXX_DEBUG(_logger,
                "ItinAnalyzerService::removeHigherCabins - preferred cabin: " << preferredCabin);

  removeHigherCabins(classOfService, preferredCabin);
}

void
ItinAnalyzerService::removeHigherCabins(std::vector<ClassOfService*>& classOfService,
                                        CabinType preferredCabin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<ClassOfService*>::iterator cosIter = classOfService.begin();
  std::vector<ClassOfService*>::iterator cosEndIter = classOfService.end();

  for (; cosIter != cosEndIter; ++cosIter)
  {
    ClassOfService* cos = *cosIter;

    if (UNLIKELY(!cos))
    {
      continue;
    }
    if (UNLIKELY(cos->cabin().isUndefinedClass()))
    {
      LOG4CXX_ERROR(
          _logger,
          "ItinAnalyzerService::removeHigherCabins - cabin undefined for class of service");
    }
    else if (cos->cabin() < preferredCabin)
    {
      LOG4CXX_DEBUG(_logger,
                    "ItinAnalyzerService::removeHigherCabins - remove cabin: "
                        << cos->cabin() << " class: " << cos->bookingCode()
                        << " seats: " << cos->numSeats());
      cos->numSeats() = 0;
    }
  }
}

// In order for FareDisplay commands to reuse Pricing logic,
// need to create a 'dummy' itin and faremarket with one travel segment
bool
ItinAnalyzerService::process(FareDisplayTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN PROCESS");
  FareDisplayAnalysis analysis(trx);
  analysis.process();

  // alloc and put into vectors
  FareMarket* fareMarket;
  trx.dataHandle().get(fareMarket);

  Itin* itin = trx.itin().front();
  TravelSeg* tvlSeg = trx.travelSeg().front();

  itin->fareMarket().push_back(fareMarket);
  trx.fareMarket().push_back(fareMarket);
  fareMarket->travelSeg().push_back(tvlSeg);

  // usually FD only has one seg, so can immediately set fareMarket locs
  fareMarket->boardMultiCity() = tvlSeg->boardMultiCity();
  fareMarket->offMultiCity() = tvlSeg->offMultiCity();
  fareMarket->origin() = tvlSeg->origin();
  fareMarket->destination() = tvlSeg->destination();
  fareMarket->direction() = FMDirection::OUTBOUND;

  // determine dates
  if (trx.getRequest()->ticketingDT() == DateTime::emptyDate())
    trx.getRequest()->ticketingDT() = trx.transactionStartTime();

  trx.dataHandle().setTicketDate(trx.getRequest()->ticketingDT());
  tvlSeg->bookingDT() = trx.getRequest()->ticketingDT();
  trx.bookingDate() = trx.getRequest()->ticketingDT();

  trx.setTravelDate(TseUtil::getTravelDate(trx.travelSeg()));
  LOG4CXX_INFO(_logger,
               "ItinAnalyzerService:: Travel Date in TRX :: "
                   << trx.travelDate().toSimpleString()
                   << " Booking Date in TRX :: " << trx.bookingDate().toSimpleString());

  itin->setTravelDate(trx.travelDate());
  fareMarket->travelDate() = trx.travelDate();

  tvlSeg->departureDT() = (tvlSeg->departureDT() == DateTime::emptyDate())
                              ? trx.transactionStartTime()
                              : tvlSeg->departureDT();

  tvlSeg->arrivalDT() =
      (tvlSeg->arrivalDT() == DateTime::emptyDate()) ? tvlSeg->departureDT() : tvlSeg->arrivalDT();

  trx.getRequest()->requestedDepartureDT() = tvlSeg->departureDT();

  // determine Geo Travel Type
  Boundary tvlBoundary = _tvlSegAnalysis.selectTravelBoundary(trx.travelSeg());
  ItinUtil::setGeoTravelType(_tvlSegAnalysis, tvlBoundary, *itin);

  if (trx.isSameCityPairRqst())
  {
    tvlSeg->isRoundTheWorld() = true;
    itin->geoTravelType() = GeoTravelType::International;
  }
  fareMarket->setGlobalDirection(trx.getRequest()->globalDirection());
  fareMarket->geoTravelType() = itin->geoTravelType();

  // determine carriers
  if (trx.getOptions()->isAllCarriers() == true)
  {
    std::map<MultiTransportMarkets::Market, std::set<CarrierCode>>* mktCxrMap;
    trx.dataHandle().get(mktCxrMap);

    trx.preferredCarriers() = FareDisplayUtil::getUniqueSetOfCarriers(trx, true, mktCxrMap);
    trx.setMarketCarrierMap(mktCxrMap);
  }

  FDSuppressFareController fdSuppress;
  fdSuppress.suppressFare(trx);

  if (trx.preferredCarriers().empty())
  {
    LOG4CXX_ERROR(_logger,
                  "NO CARRIERS FOUND FOR THE MARKET " << fareMarket->boardMultiCity() << " - "
                                                      << fareMarket->offMultiCity());
    trx.errResponse();
    throw NonFatalErrorResponseException(ErrorResponseException::NO_QUOTABLE_FARES,
                                         trx.response().str().c_str());
  }

  fareMarket->governingCarrier() = *trx.preferredCarriers().begin();

  // for multi carrier request, borad multicity might be different
  fareMarket->boardMultiCity() = FareMarketUtil::getMultiCity(fareMarket->governingCarrier(),
                                                              tvlSeg->origAirport(),
                                                              fareMarket->geoTravelType(),
                                                              tvlSeg->departureDT());

  fareMarket->offMultiCity() = FareMarketUtil::getMultiCity(fareMarket->governingCarrier(),
                                                            tvlSeg->destAirport(),
                                                            fareMarket->geoTravelType(),
                                                            tvlSeg->departureDT());

  trx.initializeTemplate();

  FareMarketUtil::setMultiCities(*fareMarket, itin->travelDate());

  analysis.setFareMarketCat19Flags(trx, fareMarket);

  // create inbound faremarket when ReturnDate provided
  trx.inboundFareMarket() = nullptr;
  if (trx.getRequest()->returnDate().isValid())
  {
    FareMarket* inbFareMarket = nullptr;
    trx.dataHandle().get(inbFareMarket);
    buildInboundFareMarket(*fareMarket, *inbFareMarket);
    TravelSeg* travelSeg = itin->travelSeg().back();
    inbFareMarket->travelSeg().push_back(travelSeg);
    trx.inboundFareMarket() = inbFareMarket;
  }

  // set Currency
  CurrencyCode originationCurrency = EMPTY_STRING();

  if (!ItinUtil::getOriginationCurrency(*itin, originationCurrency, trx.ticketingDate(), true))
  {
    LOG4CXX_ERROR(_logger, "getOriginationCurrency failed!");
  }
  itin->originationCurrency() = originationCurrency;
  itin->calculationCurrency() = NUC;
  if (!trx.getOptions()->currencyOverride().empty())
  {
    trx.getOptions()->mOverride() = 'T';
  }

  // load fareCalcConfig into trx for downline FQ use
  FareCalcUtil::getFareCalcConfig(trx);

  setTripCharacteristics(trx);
  if (trx.isShopperRequest())
  {
    processFDShopperRequest(trx, *itin, trx.preferredCarriers(), *fareMarket);
  }

  setItinRounding(trx);

  if (trx.getRequest()->requestType() == FARE_TAX_REQUEST)
    setSortTaxByOrigCity(trx);

  trx.getRequest()->electronicTicket() = 'T';

  // Up front check for invalid RB entry
  if (trx.getRequest()->requestType() == FARE_RB_REQUEST)
  {
    uint16_t secondaryCarrierSize = trx.getRequest()->secondaryCarrier().size();

    //  all RBs must have one specified carrier  (with no YYs in secondary?)
    CarrierCode& requestedCarrier = fareMarket->governingCarrier();

    if (!FDisSpecifiedCarrier(requestedCarrier))
    {
      bool needSpecificCarrier = true;
      if (secondaryCarrierSize > 0)
      {
        needSpecificCarrier = false;
        std::vector<CarrierCode>::const_iterator secondaryCarrierIter =
            trx.getRequest()->secondaryCarrier().begin();
        std::vector<CarrierCode>::const_iterator secondaryCarrierIterEnd =
            trx.getRequest()->secondaryCarrier().end();
        for (; secondaryCarrierIter != secondaryCarrierIterEnd; secondaryCarrierIter++)
        {
          if (!FDisSpecifiedCarrier(*secondaryCarrierIter))
          {
            needSpecificCarrier = true;
            break;
          }
        }
      }
      if (needSpecificCarrier)
      {
        trx.response() << " NEED SPECIFIC CARRIER FOR INTERNATIONAL YY FARE " << std::endl;
        throw NonFatalErrorResponseException(ErrorResponseException::NO_ERROR);
      }
    }

    // all domestic RBs need exactly one specified carrier (but may be secondary)
    if (secondaryCarrierSize > 0 && (fareMarket->geoTravelType() == GeoTravelType::Domestic ||
                                     fareMarket->geoTravelType() == GeoTravelType::Transborder))
    {
      std::vector<CarrierCode>::const_iterator secondaryCarrierIter =
          trx.getRequest()->secondaryCarrier().begin();
      std::vector<CarrierCode>::const_iterator secondaryCarrierIterEnd =
          trx.getRequest()->secondaryCarrier().end();
      if (!FDisSpecifiedCarrier(requestedCarrier))
      {
        requestedCarrier = *secondaryCarrierIter;
      }

      for (; secondaryCarrierIter != secondaryCarrierIterEnd; secondaryCarrierIter++)
      {
        if (requestedCarrier != *secondaryCarrierIter)
        {
          trx.response() << " SECONDARY CARRIER NOT APPLICABLE FOR DOMESTIC " << std::endl;

          throw NonFatalErrorResponseException(ErrorResponseException::NO_ERROR);
        }
      }
    }
  } // RB up front check

  return true;
}

bool
ItinAnalyzerService::FDisSpecifiedCarrier(const CarrierCode& carrier)
{
  return carrier != INDUSTRY_CARRIER && carrier != ANY_CARRIER;
}

//----------------------------------------------------------------------------
// process(TaxTrx)
//---------------------------------------------------------------------------
bool
ItinAnalyzerService::process(TaxTrx& taxTrx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(taxTrx, "ITIN PROCESS");

  if (taxTrx.isShoppingPath())
  {
    return processTaxShoppingTrx(taxTrx);
  }

  std::vector<Itin*>::const_iterator i;

  for (i = taxTrx.itin().begin(); i != taxTrx.itin().end(); i++)
  {
    if (!ItinUtil::setFurthestPoint(taxTrx, (*i)))
    {
      throw ErrorResponseException(ErrorResponseException::EMPTY_TRAVEL_SEG);
      return false;
    }
  }

  for (i = taxTrx.itin().begin(); i != taxTrx.itin().end(); i++)
  {
    (*i)->originationCurrency() = taxTrx.getRequest()->ticketingAgent()->currencyCodeAgent();
    (*i)->calculationCurrency() = taxTrx.getRequest()->ticketingAgent()->currencyCodeAgent();

    std::vector<FarePath*>::iterator j;

    for (j = (*i)->farePath().begin(); j != (*i)->farePath().end(); j++)
    {
      FarePath* farePath = *j;

      if (farePath)
      {
        farePath->calculationCurrency() =
            taxTrx.getRequest()->ticketingAgent()->currencyCodeAgent();
        farePath->baseFareCurrency() = taxTrx.getRequest()->ticketingAgent()->currencyCodeAgent();
        LOG4CXX_DEBUG(_logger, "CALCULATION  CURRENCY: " << farePath->calculationCurrency());
        LOG4CXX_DEBUG(_logger, "BASE FARE  CURRENCY: " << farePath->baseFareCurrency());
      }
    }

    Boundary tvlBoundary = _tvlSegAnalysis.selectTravelBoundary((*i)->travelSeg());
    ItinUtil::setGeoTravelType(_tvlSegAnalysis, tvlBoundary, **i);
  }

  if (!taxTrx.isShoppingPath())
  {
    // Logic to build fare markets
    buildFareMarket(taxTrx);
  }

  // Taxes Needs To Know One Way Or Round Trip etc...
  setTripCharacteristics(taxTrx);

  // Columbia Taxes utilize First Carrier Logic
  setValidatingCarrier(taxTrx);

  return true;
}

bool
ItinAnalyzerService::processTaxShoppingTrx(TaxTrx& taxTrx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<Itin*>::iterator itinIter = taxTrx.itin().begin();
  std::vector<Itin*>::iterator itinIterE = taxTrx.itin().end();

  for (; itinIter != itinIterE; ++itinIter)
  {
    Itin* itin = *itinIter;

    if (!ItinUtil::setFurthestPoint(taxTrx, itin))
    {
      throw ErrorResponseException(ErrorResponseException::EMPTY_TRAVEL_SEG);
      return false;
    }

    Boundary tvlBoundary = _tvlSegAnalysis.selectTravelBoundary(itin->travelSeg());
    ItinUtil::setGeoTravelType(_tvlSegAnalysis, tvlBoundary, *itin);

    setTripCharacteristics(itin);
  }

  return true;
}

//----------------------------------------------------------------------------
// checkSimpleTrip(PricingTrx)
//---------------------------------------------------------------------------

void
ItinAnalyzerService::checkSimpleTrip(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, "Entered ItinAnalyzerService::checkSimpleTrip()");
  if (trx.itin().size() != 1)
  {
    if (!enableChecksimpleTrip.getValue())
      return;
  }

  std::vector<Itin*>::iterator itinI = trx.itin().begin();
  std::vector<Itin*>::iterator itinIE = trx.itin().end();

  for (; itinI != itinIE; ++itinI)
  {
    Itin& itin = *(*itinI);
    uint16_t segCount = itin.travelSeg().size();

    if (segCount == 1) // One travel seg itin
    {
      EPQMinimumFare epq;
      itin.simpleTrip() = epq.process(trx, itin);
    }
    else if (segCount == 2) // Two travel segs as RT
    {
      TravelSeg& firstTravelSeg = *(itin.travelSeg().front());
      TravelSeg& secondTravelSeg = *(itin.travelSeg().back());
      if (firstTravelSeg.boardMultiCity() == secondTravelSeg.offMultiCity() &&
          firstTravelSeg.offMultiCity() == secondTravelSeg.boardMultiCity())
      {
        EPQMinimumFare epq;
        itin.simpleTrip() = epq.process(trx, itin);
        if (itin.simpleTrip())
        {
          // set all FM as simple trip
          for (const auto fareMarket : itin.fareMarket())
          {
            fareMarket->setSimpleTrip(true);
          }
        }
      }
    }
  } // end FOR
}

void
ItinAnalyzerService::processFDShopperRequest(FareDisplayTrx& trx,
                                             Itin& itin,
                                             std::set<CarrierCode>& carrierList,
                                             FareMarket& firstFM)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TravelSeg* tvlSeg = trx.travelSeg().front();

  CarrierCode carrier = firstFM.governingCarrier();

  firstFM.setCarrierInAirSegments(carrier);

  std::set<CarrierCode>::iterator i(carrierList.begin()), end(carrierList.end());

  for (; i != end; ++i)
  {
    if ((*i) == carrier)
    {
      continue;
    }

    FareMarket* fareMarket;
    trx.dataHandle().get(fareMarket); // lint !e530
    cloneFareMarket(firstFM, *fareMarket, *i);

    updateFareMarket(*fareMarket, *i, trx.dataHandle(), trx.travelDate());

    fareMarket->boardMultiCity() = FareMarketUtil::getMultiCity(
        *i, tvlSeg->origAirport(), fareMarket->geoTravelType(), tvlSeg->departureDT());

    fareMarket->offMultiCity() = FareMarketUtil::getMultiCity(
        *i, tvlSeg->destAirport(), fareMarket->geoTravelType(), tvlSeg->departureDT());

    itin.fareMarket().push_back(fareMarket);
    trx.fareMarket().push_back(fareMarket);
  }
}

void
ItinAnalyzerService::updateFareMarket(FareMarket& fm,
                                      CarrierCode carrier,
                                      DataHandle& dataHandle,
                                      const DateTime& tvlDate) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  fm.recreateTravelSegments(dataHandle);
  fm.setCarrierInAirSegments(carrier);
  FareMarketUtil::setMultiCities(fm, tvlDate);
}

void
ItinAnalyzerService::setItinRounding(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<Itin*>::const_iterator i;

  for (i = trx.itin().begin(); i != trx.itin().end(); i++)
  {
    setItinRounding(**i);
  }
}

void
ItinAnalyzerService::setItinRounding(Itin& itin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if ((itin.geoTravelType() == GeoTravelType::International) &&
      (!itin.tripCharacteristics().isSet(Itin::RussiaOnly)))
    itin.useInternationalRounding() = true;
}

//-------------------------------------------------------------------
void
ItinAnalyzerService::checkJourneyActivation(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    trx.getOptions()->journeyActivatedForShopping() = false;
    // see if JPS-ON or JPS-OFF was entered in the entry
    if (trx.getOptions()->jpsEntered() == NO)
    {
      return;
    }
    else if (trx.getOptions()->jpsEntered() == YES)
    {
      trx.getOptions()->journeyActivatedForShopping() = true;
      return;
    }

    if (trx.getRequest()->ticketingAgent() == nullptr ||
        trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
    {
      // check if the entry is coming from an airline partition
      if (!(trx.billing()->partitionID().empty()) && trx.billing()->aaaCity().size() < 4)
      {
        const CarrierPreference* cp =
            getCarrierPref(trx, trx.billing()->partitionID(), trx.getRequest()->ticketingDT());
        if (cp != nullptr)
        {
          if (cp->activateJourneyShopping() == YES)
            trx.getOptions()->journeyActivatedForShopping() = true;
        }
      }
      return;
    }

    // if entry came from an agent check if the agent is activated for Journey thru database
    if (trx.getRequest()->ticketingAgent()->agentTJR()->activateJourneyShopping() != YES)
    {
      return;
    }

    if (trx.getRequest()->ticketingAgent()->agentTJR()->activateJourneyShopping() == YES)
    {
      trx.getOptions()->journeyActivatedForShopping() = true;
      return;
    }
  }
  else if (trx.getTrxType() == PricingTrx::PRICING_TRX)
  {
    trx.getOptions()->journeyActivatedForPricing() = false;

    bool latam = false;
    if(!fallback::fallbackLatamJourneyActivation(&trx))
    {
      if (trx.getRequest()->ticketingAgent() == nullptr ||
          trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
      {
        // check if the entry is coming from an airline partition
        if (!(trx.billing()->partitionID().empty()) && trx.billing()->aaaCity().size() < 4 &&
            trx.billing()->partitionID() == "LA")
        {
          latam = true;
        }
      }

      if (!trx.getRequest()->isLowFareRequested() && !latam)
        return;
    }
    else if (!trx.getRequest()->isLowFareRequested())
      return;
    if (trx.getRequest()->isLowFareNoAvailability())
      return;

    // do not try local with flowAvail if it is shopping path 1 (itin selector)
    if (trx.isShopping())
      return;

    // see if JPS-ON or JPS-OFF was entered in the entry
    if (trx.getOptions()->jpsEntered() == NO)
    {
      return;
    }
    else if (trx.getOptions()->jpsEntered() == YES)
    {
      trx.getOptions()->journeyActivatedForPricing() = true;
      return;
    }

    if (trx.getRequest()->ticketingAgent() == nullptr ||
        trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
    {
      // check if the entry is coming from an airline partition
      if (!(trx.billing()->partitionID().empty()) && trx.billing()->aaaCity().size() < 4)
      {
        const CarrierPreference* cp =
            getCarrierPref(trx, trx.billing()->partitionID(), trx.getRequest()->ticketingDT());
        if (cp != nullptr)
        {
          if (cp->activateJourneyPricing() == YES)
          {
            trx.getOptions()->journeyActivatedForPricing() = true;
            if(latam)
              trx.getOptions()->allowLATAMDualRBD() = true;;
          }
        }
      }
      return;
    }

    // if entry came from an agent check if the agent is activated for Journey thru database

    if (trx.getRequest()->ticketingAgent()->agentTJR()->activateJourneyPricing() != YES)
    {
      return;
    }

    if (trx.getRequest()->ticketingAgent()->agentTJR()->activateJourneyPricing() == YES)
    {
      trx.getOptions()->journeyActivatedForPricing() = true;
      return;
    }
  }
  return;
}

//---------------------------------------------------------------------------
// buildInboundFareMarket()
//---------------------------------------------------------------------------
void
ItinAnalyzerService::buildInboundFareMarket(const FareMarket& fareMarket, FareMarket& inbFareMarket)
{
  LOG4CXX_DEBUG(_logger, "Entered ItinAnalyzerService::buildInboundFareMarket()");
  // populate the clone
  inbFareMarket.origin() = fareMarket.destination();
  inbFareMarket.boardMultiCity() = fareMarket.offMultiCity();
  inbFareMarket.destination() = fareMarket.origin();
  inbFareMarket.offMultiCity() = fareMarket.boardMultiCity();
  inbFareMarket.setGlobalDirection(fareMarket.getGlobalDirection());
  inbFareMarket.geoTravelType() = fareMarket.geoTravelType();
  inbFareMarket.direction() = FMDirection::INBOUND;
  inbFareMarket.governingCarrierPref() = fareMarket.governingCarrierPref();
  inbFareMarket.travelBoundary() = fareMarket.travelBoundary();
  inbFareMarket.governingCarrier() = fareMarket.governingCarrier();
  inbFareMarket.travelDate() = fareMarket.travelDate();
}

//----------------------------------------------------------------------------
// prepareForJourney
//----------------------------------------------------------------------------
void
ItinAnalyzerService::prepareForJourney(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // Delegate - this method overridden in unit tests
  tse::iadetail::JourneyPrepHelper::prepareForJourney(trx);
}

//-------------------------------------------------------------------
void
ItinAnalyzerService::checkSoloActivation(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  trx.getOptions()->soloActiveForPricing() = false;

  if (trx.getTrxType() != PricingTrx::PRICING_TRX)
    return;
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
    return;
  // do not try SOLO if it is shopping path 1 (itin selector)
  if (trx.isShopping())
    return;

  // if its not a WPNC entry - do not try SOLO
  if (!(trx.getRequest()->isLowFareRequested()))
    return;

  if (trx.getRequest()->ticketingAgent() == nullptr ||
      trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
  {
    // check if the entry is coming from an airline partition
    if (!(trx.billing()->partitionID().empty()) && trx.billing()->aaaCity().size() < 4)
    {
      const CarrierPreference* cp =
          getCarrierPref(trx, trx.billing()->partitionID(), trx.getRequest()->ticketingDT());
      if (cp != nullptr)
      {
        if (cp->activateSoloPricing() == YES)
          trx.getOptions()->soloActiveForPricing() = true;
      }
    }
    return;
  }

  // if entry came from an agent check if the agent is activated for SOLO thru database

  if (trx.getRequest()->ticketingAgent()->agentTJR()->availIgRul2StWpnc() != NO)
    return;

  if (trx.getRequest()->ticketingAgent()->agentTJR()->availIgRul2StWpnc() == NO)
  {
    trx.getOptions()->soloActiveForPricing() = true;
    return;
  }
  return;
}

bool
ItinAnalyzerService::checkCallDCAPerItn(std::vector<Itin*>& itins)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<Itin*>::iterator iter = itins.begin();
  std::vector<Itin*>::iterator iterEnd = itins.end();

  for (; iter != iterEnd; ++iter)
  {
    if ((*iter)->dcaSecondCall())
    {
      return true;
    }
  }

  return false;
}

void
ItinAnalyzerService::setTrxDataHandleTicketDate(PricingTrx& trx, const DateTime& date)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  trx.dataHandle().setTicketDate(trx.ticketingDate());
}

void
ItinAnalyzerService::checkRestrictedCurrencyNation(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.getOptions()->isMOverride() && validateRestrictedCurrencyNation(trx))
  {
    throw ErrorResponseException(ErrorResponseException::RESTRICTED_CURRENCY);
  }
}

const Loc*
ItinAnalyzerService::getTicketLoc(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  const Loc* ticketingLoc = nullptr;
  const PricingRequest* request = trx.getRequest();
  if (request->ticketPointOverride().size() != 0)
  {
    ticketingLoc = trx.dataHandle().getLoc(request->ticketPointOverride(), trx.ticketingDate());

    if (ticketingLoc == nullptr)
    {
      std::string errMsg = "UNKNOWN TICKET LOC OVERRIDE ";
      errMsg += request->ticketPointOverride();
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, errMsg.c_str());
    }
  }
  else
  {
    ticketingLoc = request->ticketingAgent()->agentLocation();
  }

  return ticketingLoc;
}

const Loc*
ItinAnalyzerService::getSalesLoc(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  const Loc* saleLoc = nullptr;
  const PricingRequest* request = trx.getRequest();
  if (request->salePointOverride().size() != 0)
  {
    saleLoc = trx.dataHandle().getLoc(request->salePointOverride(), trx.ticketingDate());

    if (saleLoc == nullptr)
    {
      std::string errMsg = "UNKNOWN SALES LOC OVERRIDE ";
      errMsg += request->salePointOverride();
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, errMsg.c_str());
    }
  }
  else
  {
    saleLoc = request->ticketingAgent()->agentLocation();
  }

  return saleLoc;
}

void
ItinAnalyzerService::findCommenceDate(ExcItin& itin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  DateTime commenceDate = DateTime::emptyDate();

  TravelSeg* firstSeg = itin.travelSeg().front();
  if (!firstSeg->unflown())
    commenceDate = firstSeg->departureDT();

  itin.travelCommenceDate() = commenceDate;
}

namespace
{
struct FirstUnused : public std::unary_function<const TravelSeg*, bool>
{
  FirstUnused(const DateTime& ticketDate) : _ticketDate(ticketDate) {}
  bool operator()(const TravelSeg* travelSeg) const
  {
    return (travelSeg->departureDT() > _ticketDate && travelSeg->segmentType() != Arunk);
  }

protected:
  const DateTime& _ticketDate;
};
}

namespace
{
class NotHIPFromExchange : public std::unary_function<const PlusUpOverride*, bool>
{
public:
  bool operator()(const PlusUpOverride* plusUp) const
  {
    return plusUp->fromExchange() && plusUp->moduleName() != HIP;
  }
};

class ExcDiffSumarizer
{
public:
  MoneyAmount operator()(MoneyAmount sum, const DifferentialOverride* diff)
  {
    return diff->fromExchange() ? sum + diff->amount() : sum;
  }
};
}

void
ItinAnalyzerService::checkIfPenaltiesAndFCsEqualToSumFromFareCalc(RefundPricingTrx& trx) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (std::find_if(trx.exchangeOverrides().plusUpOverride().begin(),
                   trx.exchangeOverrides().plusUpOverride().end(),
                   NotHIPFromExchange()) != trx.exchangeOverrides().plusUpOverride().end())
    return;

  MoneyAmount sum = std::accumulate(trx.exchangeOverrides().differentialOverride().begin(),
                                    trx.exchangeOverrides().differentialOverride().end(),
                                    0.0,
                                    ExcDiffSumarizer());

  std::vector<SurchargeOverride*>::const_iterator suoIter =
      trx.exchangeOverrides().surchargeOverride().begin();
  for (; suoIter != trx.exchangeOverrides().surchargeOverride().end(); ++suoIter)
  {
    if (!(*suoIter)->fromExchange())
      continue;
    if (!(*suoIter)->travelSeg())
      return;
    sum += (*suoIter)->amount();
  }

  std::vector<StopoverOverride*>::const_iterator soIter =
      trx.exchangeOverrides().stopoverOverride().begin();
  for (; soIter != trx.exchangeOverrides().stopoverOverride().end(); ++soIter)
  {
    if (!(*soIter)->fromExchange())
      continue;
    if (!(*soIter)->travelSeg())
      return;
    sum += (*soIter)->amount();
  }

  std::vector<FareCompInfo*>::iterator fcIter = trx.exchangeItin().front()->fareComponent().begin();
  for (; fcIter != trx.exchangeItin().front()->fareComponent().end(); ++fcIter)
    sum += (*fcIter)->tktFareCalcFareAmt();

  if ((sum + EPSILON) > trx.totalFareCalcAmount() && (sum - EPSILON) < trx.totalFareCalcAmount())
    trx.arePenaltiesAndFCsEqualToSumFromFareCalc() = true;
}

bool
ItinAnalyzerService::process(RefundPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "REFUND ITIN PROCESS");

  checkIfPenaltiesAndFCsEqualToSumFromFareCalc(trx);

  if (trx.travelSeg().empty())
  {
    trx.fullRefund() = true;
    trx.newItin().clear();
  }
  trx.setFareApplicationDT(trx.originalTktIssueDT());

  return processRexCommon(trx);
}

bool
ItinAnalyzerService::process(RexPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "REISSUE ITIN PROCESS");

  bool result = processRexCommon(trx);

  if (!trx.exchangeItin().empty())
  {
    std::vector<Itin*>::iterator itinIt = trx.newItin().begin();
    for (uint16_t index = 0; itinIt != trx.newItin().end(); ++itinIt, index++)
    {
      Itin* newItin = *itinIt;

      trx.setItinIndex(index);
      ExcItinUtil::DetermineChanges(trx.exchangeItin().front(), newItin);
      ExcItinUtil::matchCoupon(trx.exchangeItin().front(), newItin);

      ExcItinUtil::SetFareMarketChangeStatus(trx.exchangeItin().front(), newItin);
      ExcItinUtil::SetFareMarketChangeStatus(newItin, trx.exchangeItin().front());

      ExcItinUtil::CheckSegmentsStatus(trx.exchangeItin().front(), newItin);
      ExcItinUtil::IsStopOverChange(trx.exchangeItin().front(), newItin);

      if (trx.getRequest()->isBrandedFaresRequest() && trx.getTrxType() == PricingTrx::MIP_TRX)
      {
        ExcItinUtil::calculateMaxPriceForFlownOrNotShopped(trx, *newItin);
      }
    }
  }
  applyReissueExchange(trx);

  if (trx.isPbbRequest() && TrxUtil::isRequestFromAS(trx))
  {
    Diag892Collector* diag892 = createDiag<Diag892Collector>(trx);
    for (Itin* itin : trx.itin())
      itinanalyzerutils::assignLegsToSegments(itin->travelSeg(), trx.dataHandle(), diag892);
  }

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.getRequest()->isBrandedFaresRequest())
  {
    if (!processIbfBrandRetrievalAndParity(trx))
    {
      throw ErrorResponseException(ErrorResponseException::NO_VALID_BRAND_FOUND);
    }
  }

  return result;
}

bool
ItinAnalyzerService::process(RexExchangeTrx& trx)
{
  LOG4CXX_DEBUG(_logger, "Process(RexExchangeTrx&)");
  trx.ititializeMultiItinData();

  return process(static_cast<RexPricingTrx&>(trx));
}

bool
ItinAnalyzerService::process(StructuredRuleTrx& trx)
{
  LOG4CXX_DEBUG(_logger, "Process(StructuredRuleTrx&)");

  if (fallback::fallbackForcePricingWayForSFR(&trx))
  {
    return process(static_cast<PricingTrx&>(trx));
  }

  ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper = selectProcesing(trx);

  trx.firstUnflownAirSeg() = TseUtil::getFirstUnflownAirSeg(trx.travelSeg(), trx.ticketingDate());

  trx.setTravelDate(TseUtil::getTravelDate(trx.travelSeg()));
  trx.bookingDate() = TseUtil::getBookingDate(trx.travelSeg());

  for (Itin* itin : trx.itin())
  {
    setFurthestPoint(trx, itin);
  }

  baseItinAnalyzerWrapper->selectTicketingCarrier(trx);
  baseItinAnalyzerWrapper->setRetransits(trx);
  baseItinAnalyzerWrapper->setOpenSegFlag(trx);
  baseItinAnalyzerWrapper->setATAESchedContent(trx);

  setGeoTravelTypeAndValidatingCarrier(trx);

  if (trx.isMultiPassengerSFRRequestType())
  {
    itinanalyzerutils::removeRedundantFareMarket(*trx.getMultiPassengerFCMapping());
    itinanalyzerutils::initializeFareMarketsFromFareComponents(trx, baseItinAnalyzerWrapper);
  }
  else
  {
    initializeFareMarketsFromItin(trx, baseItinAnalyzerWrapper);
  }

  baseItinAnalyzerWrapper->setTripCharacteristics(trx);
  baseItinAnalyzerWrapper->checkJourneyActivation(trx);
  baseItinAnalyzerWrapper->checkSoloActivation(trx);

  setIntlSalesIndicator(trx);

  prepareForJourney(trx);

  baseItinAnalyzerWrapper->setATAEAvailContent(trx);

  setCurrencyOverride(trx);
  setSortTaxByOrigCity(trx);
  setItinCurrencies(trx);
  setAgentCommissions(trx);

  baseItinAnalyzerWrapper->setItinRounding(trx);

  return true;
}

namespace
{
class OffExcPhase
{
public:
  void operator()(RexBaseTrx& trx) { trx.trxPhase() = RexPricingTrx::IDLE_PHASE; }
};

typedef RAIIImpl<RexBaseTrx, OffExcPhase> RAIIOffExcPhase;
}

bool
ItinAnalyzerService::processRexCommon(RexBaseTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.getOptions()->PricingOptions::isRtw() && !trx.isExcRtw())
    ItinUtil::roundTheWorldDiagOrThrow(trx, true);

  setTrxDataHandleTicketDate(trx, trx.ticketingDate());

  checkRestrictedCurrencyNation(trx);

  trx.firstUnflownAirSeg() = TseUtil::getFirstUnflownAirSeg(trx.travelSeg(), trx.ticketingDate());

  tse::iadetail::JourneyPrepHelper::updateCarrierPreferences(trx);

  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    FamilyLogicUtils::splitItinFamilies(trx, _splitItinsForDomesticOnly, _requestedNumberOfSeats);
  }

  trx.setTravelDate(TseUtil::getTravelDate(trx.travelSeg()));
  trx.setAdjustedTravelDate();
  trx.bookingDate() = TseUtil::getBookingDate(trx.travelSeg());

  const bool oldStat = trx.isAnalyzingExcItin();

  // Analyze New itin

  if (!trx.newItin().empty())
  {
    trx.setAnalyzingExcItin(false);

    discoverThroughFarePrecedence(trx, *trx.itin().front());

    ValidatingCarrierUpdater validatingCarrier(trx);

    const Loc* ticketingLoc = getTicketLoc(trx);
    const Loc* saleLoc = getSalesLoc(trx);

    for (Itin* itin : trx.newItin())
    {
      setFurthestPoint(trx, itin);

      // Set GEO Travel Type and Ticketing Carrier
      setGeoTravelTypeAndTktCxr(*itin);

      if (trx.getTrxType() != PricingTrx::MIP_TRX)
      {
        // Set retransit points
        setRetransits(*itin);
      }

      // Set openSegAfterDatedSeg
      setOpenSegFlag(*itin);

      // Build fare markets
      selectProcesing(trx)->buildFareMarket(trx, *itin);

      // Set trip characteristics
      setTripCharacteristics(itin);

      // Set validating carrier
      setATAESchedContent(trx);

      validatingCarrier.update(*itin);

      // Set rounding
      setItinRounding(*itin);

      // Set international sales indicators
      setIntlSalesIndicator(*itin, *ticketingLoc, *saleLoc);
    }

    checkJourneyActivation(trx);

    prepareForJourney(trx);

    // Set currency override
    setCurrencyOverride(trx);
    setATAEAvailContent(trx);
  }
  else
  {
    trx.setAnalyzingExcItin(false);
    // Check only correctness of locations for full refund trx
    getTicketLoc(trx);
    getSalesLoc(trx);
  }

  // Analyzer Exchange Itin
  if (!trx.exchangeItin().empty())
  {
    trx.setAnalyzingExcItin(true);
    trx.trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;

    RAIIOffExcPhase roep(trx);

    ExcItin* itin = trx.exchangeItin().front();

    setFurthestPoint(trx, itin);

    // Set GEO Travel Type and Ticketing Carrier
    setGeoTravelTypeAndTktCxr(*itin);

    // Set retransit points
    setRetransits(*itin);

    // Set openSegAfterDatedSeg
    setOpenSegFlag(*itin);

    // Build fare markets
    buildFareMarket(trx, *itin);

    // Set trip characteristics
    setTripCharacteristics(itin);

    // Set validating carrier
    setATAESchedContent(trx);

    updateValidatingCarrier(trx, *itin);

    // Set rounding
    setItinRounding(*itin);

    // Exc Itin may have different Ticket and/or Sales Locs.
    const Loc* ticketingLoc = getTicketLoc(trx);
    const Loc* saleLoc = getSalesLoc(trx);

    // Set international sales indicators
    setIntlSalesIndicator(*itin, *ticketingLoc, *saleLoc);

    // Set currency override
    setCurrencyOverride(trx);

    // Find travel commence date
    findCommenceDate(*itin);

    if (trx.getTrxType() != PricingTrx::MIP_TRX)
    {
      setATAEAvailContent(trx);
    }

    if (trx.isExcRtw())
      ItinUtil::setRoundTheWorld(trx, *trx.exchangeItin().front());
  }

  if (trx.getOptions()->isRtw())
    ItinUtil::removeAtaeMarkets(trx);

  trx.setAnalyzingExcItin(oldStat);

  trx.setTktValidityDate();

  // Set sortTaxByOrigCity flag
  setSortTaxByOrigCity(trx);

  // Set itinerary origination and calculations currencies
  setItinCurrencies(trx); // for both new and exchange itins

  recalculateFareCalcFareAmt(trx);

  // Set agent commissions
  setAgentCommissions(trx);

  FamilyLogicUtils::setInfoForSimilarItins(trx, this);

  if (trx.getOptions()->isRtw())
  {
    for (Itin* itin : trx.newItin())
    {
      ItinUtil::setRoundTheWorld(trx, *itin);
    }
  }
  diag192(trx);

  return true;
}

void
ItinAnalyzerService::setFurthestPoint(PricingTrx& trx, Itin* itin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  itin->setTravelDate(TseUtil::getTravelDate(itin->travelSeg()));
  itin->bookingDate() = TseUtil::getBookingDate(itin->travelSeg());

  if (!ItinUtil::setFurthestPoint(trx, itin))
    throw ErrorResponseException(ErrorResponseException::EMPTY_TRAVEL_SEG);
}

void
ItinAnalyzerService::determineChanges(ExchangePricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (!trx.newItin().empty() && !trx.exchangeItin().empty())
    ExcItinUtil::DetermineChanges(trx.exchangeItin().front(), trx.newItin().front());
}

ItinAnalyzerServiceWrapper*
ItinAnalyzerService::selectProcesing(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (UNLIKELY(isRexTrxRedirected2ExcTrx(trx)))
    return &_excItinAnalyzerWrapper;

  return &_itinAnalyzerWrapper;
}

bool
ItinAnalyzerService::process(ExchangePricingTrx& trx)
{
  LOG4CXX_INFO(_logger, "ItinAnalyzerService::process(ExchangePricingTrx)");

  ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper = selectProcesing(trx);

  exchangePricingTrxException(trx);

  baseItinAnalyzerWrapper->processExchangeItin(trx);

  baseItinAnalyzerWrapper->determineChanges(trx);

  if (trx.applyReissueExchange())
  {
    BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(&trx);
    determineExchangeReissueStatus(*excTrx);
  }

  return process(static_cast<PricingTrx&>(trx));
}

void
ItinAnalyzerService::exchangePricingTrxException(ExchangePricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.reqType() != AGENT_PRICING_MASK && trx.reqType() != NON_FLIGHT_EXCHANGE)
  {
    if (trx.exchangeItin().empty() || trx.exchangeItin().front() == nullptr ||
        trx.exchangeItin().front()->travelSeg().empty())
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "EXCHANGE ITIN MUST BE PRESENT FOR EXCHANGE TRX");
  }

  if (trx.reqType() == TAG_10_EXCHANGE)
  {
    LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::ExchangePricingTrx Type = CE");

    std::vector<Itin*>::iterator iter = trx.itin().begin();
    std::vector<Itin*>::iterator iterEnd = trx.itin().end();

    for (; iter != iterEnd; ++iter)
    {
      Itin* itin = *iter;
      if (setGeoTravelTypeAndTktCxr(*itin) && itin->geoTravelType() != GeoTravelType::Domestic &&
          itin->geoTravelType() != GeoTravelType::Transborder)
      {
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                     "EXCHANGE TYPE NOT VALID FOR NON US/CA ITINERARIES");
      }
    }
  }
}

void
ItinAnalyzerService::processExchangeItin(ExchangePricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.reqType() == AGENT_PRICING_MASK)
    return;

  if (trx.exchangeItin().empty())
    return;

  ExcItin* itin = trx.exchangeItin().front();

  if (itin == nullptr)
    return;

  setFurthestPoint(trx, itin);

  // Set retransit points
  setRetransits(*itin);

  // Set openSegAfterDatedSeg
  setOpenSegFlag(*itin);

  // Set trip characteristics
  setTripCharacteristics(itin);
}

void
ItinAnalyzerService::setIndicesToSchedulingOption(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN SET INDICES TO SCHEDULING OPTION");
  std::vector<std::map<uint32_t, uint32_t>>::const_iterator it =
      trx.schedulingOptionIndices().begin();
  std::vector<std::map<uint32_t, uint32_t>>::const_iterator itEnd =
      trx.schedulingOptionIndices().end();
  for (size_t leg = 0; it != itEnd; ++it, ++leg)
  {
    const std::map<uint32_t, uint32_t>& sopIdx = *it;
    std::map<uint32_t, uint32_t>::const_iterator itMap = sopIdx.begin();
    std::map<uint32_t, uint32_t>::const_iterator itMapEnd = sopIdx.end();
    for (; itMap != itMapEnd; ++itMap)
    {
      trx.indicesToSchedulingOption()[leg][itMap->second] = itMap->first;
    }
  }
}

void
ItinAnalyzerService::copyExcStopoverOverride(ExchangePricingTrx& excTrx,
                                             const FareCompInfo& fcInfo,
                                             const TravelSeg& tvlSegExc,
                                             TravelSeg* tvlSegNew)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::map<uint16_t, MoneyAmount>::const_iterator soSurchIter =
      fcInfo.stopoverSurcharges().find(tvlSegExc.segmentOrder());
  if (soSurchIter != fcInfo.stopoverSurcharges().end())
  {
    StopoverOverride* soOverride = nullptr;
    excTrx.dataHandle().get(soOverride);
    if (soOverride != nullptr)
    {
      soOverride->count() = 1;
      soOverride->amount() = soSurchIter->second;
      soOverride->travelSeg() = tvlSegNew;
      soOverride->fromExchange() = true;
      excTrx.exchangeOverrides().stopoverOverride().push_back(soOverride);
    }
  }
}

void
ItinAnalyzerService::recalculateFareCalcFareAmt(RexBaseTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  ExcItin& itin = *(trx.exchangeItin().front());
  if (itin.calculationCurrency() != itin.calcCurrencyOverride())
  {
    CurrencyConversionFacade ccFacade;

    std::vector<FareCompInfo*>& fareComponents = itin.fareComponent();
    std::vector<FareCompInfo*>::iterator i = fareComponents.begin();
    for (; i != fareComponents.end(); ++i)
    {
      FareCompInfo& fc = **i;

      const Money sourceAmt(fc.tktFareCalcFareAmt(), itin.calcCurrencyOverride());
      Money targetAmt(itin.calculationCurrency());
      if (ccFacade.convert(targetAmt, sourceAmt, trx, itin.useInternationalRounding()))
      {
        fc.fareCalcFareAmt() = targetAmt.value();
      }
    }
  }
}

void
ItinAnalyzerService::generateDummySOP(FlightFinderTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if ((nullptr == trx.journeyItin()) || (trx.journeyItin()->travelSeg().empty()))
  {
    return;
  }

  size_t legsCount = trx.journeyItin()->travelSeg().size();

  for (size_t legNum = 0; legNum < legsCount; ++legNum)
  {
    AirSeg* journeySeg = dynamic_cast<AirSeg*>(trx.journeyItin()->travelSeg()[legNum]);
    ShoppingTrx::Leg* curLeg = nullptr;

    switch (trx.bffStep())
    {
    case FlightFinderTrx::STEP_1:
      if (0 == legNum)
      {
        curLeg = addDummyLeg(trx, false);
        addDummySop(trx, curLeg, journeySeg);
      }
      break;

    case FlightFinderTrx::STEP_2:
      // Promotional shopping
      if (0 == legNum)
      {
        // No flights in request
        if (trx.legs().empty())
        {
          curLeg = addDummyLeg(trx, false);
          addDummySop(trx, curLeg, journeySeg);
        }
        else
        {
          addDummySop(trx, &(trx.legs()[0]), journeySeg);
        }
      }
      break;

    case FlightFinderTrx::STEP_3:
      curLeg = addDummyLeg(trx, false);
      addDummySop(trx, curLeg, journeySeg);
      break;

    case FlightFinderTrx::STEP_4:
      // Promotional shopping
      if (trx.avlInS1S3Request())
      {
        if (0 == legNum)
        {
          curLeg = addDummyLeg(trx, true);
          addDummySop(trx, curLeg, journeySeg);
        }
        else
        {
          // No flights in request
          if (trx.legs().size() == 1)
          {
            curLeg = addDummyLeg(trx, false);
            addDummySop(trx, curLeg, journeySeg);
          }
          else
          {
            addDummySop(trx, &(trx.legs()[1]), journeySeg);
          }
        }
      }
      else if (0 == legNum)
      {
        curLeg = addDummyLeg(trx, true);
        addDummySop(trx, curLeg, journeySeg);
      }
      break;

    case FlightFinderTrx::STEP_6:
      if (0 == legNum)
      {
        curLeg = addDummyLeg(trx, true);
        addDummySop(trx, curLeg, journeySeg);
      }
      break;

    default:
      LOG4CXX_ERROR(_logger, "ItinAnalyzerService::generateDummySOP - BFF step not specified.");
      return;
    }
  }

  return;
}

ShoppingTrx::Leg*
ItinAnalyzerService::addDummyLeg(FlightFinderTrx& trx, bool addInFront)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  ShoppingTrx::Leg* curLeg = nullptr;

  if (true == addInFront)
  {
    // Add dummy leg in front since request contains only inbound schedules
    trx.legs().insert(trx.legs().begin(), ShoppingTrx::Leg());
    curLeg = &trx.legs().front();

    trx.schedulingOptionIndices().insert(trx.schedulingOptionIndices().begin(),
                                         std::map<uint32_t, uint32_t>());

    trx.indicesToSchedulingOption().insert(trx.indicesToSchedulingOption().begin(),
                                           std::map<uint32_t, uint32_t>());
  }
  else
  {
    // Add dummy leg to back of vector
    trx.legs().push_back(ShoppingTrx::Leg());
    curLeg = &trx.legs().back();

    trx.schedulingOptionIndices().push_back(std::map<uint32_t, uint32_t>());
    trx.indicesToSchedulingOption().push_back(std::map<uint32_t, uint32_t>());
  }

  return curLeg;
}

void
ItinAnalyzerService::addDummySop(FlightFinderTrx& trx, ShoppingTrx::Leg* curLeg, AirSeg* journeySeg)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  uint32_t sopIndex = 1;
  int16_t pnrSegment = 1;

  LocCode boardCity = journeySeg->origAirport();
  LocCode offCity = journeySeg->destAirport();
  CarrierCode cxrCode = journeySeg->carrier();
  DateTime depDate = journeySeg->departureDT();

  Itin* itin;
  trx.dataHandle().get(itin);

  // Create dummy flight
  AirSeg* segment;
  trx.dataHandle().get(segment);

  TravelSegUtil::setupItinerarySegment(
      trx.dataHandle(), segment, depDate, boardCity, offCity, cxrCode, pnrSegment);

  itin->travelSeg().push_back(segment);
  curLeg->sop().push_back(ShoppingTrx::SchedulingOption(itin, sopIndex));
  curLeg->sop().back().setDummy(true);
}

void
ItinAnalyzerService::setUpInfoForAltDateJourneyItin(Itin& journeyItin,
                                                    ShoppingTrx::AltDatePairs& altDatePairs)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  for (ShoppingTrx::AltDatePairs::const_iterator i = altDatePairs.begin(); i != altDatePairs.end();
       ++i)
  {
    Itin& altdateItin = *(i->second)->journeyItin;
    altdateItin.calculationCurrency() = journeyItin.calculationCurrency();
    altdateItin.originationCurrency() = journeyItin.originationCurrency();
  }
}

void
ItinAnalyzerService::calculateFlightTimeMinutes(ShoppingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  Itin* firstItin = trx.legs()[0].sop()[0].itin();
  const TravelSeg* const tsDep = firstItin->firstTravelSeg();
  const TravelSeg* const tsArr = firstItin->lastTravelSeg();
  short utcOffset = 0;

  LocUtil::getUtcOffsetDifference(*tsDep->origin(),
                                  *tsArr->destination(),
                                  utcOffset,
                                  trx.dataHandle(),
                                  tsDep->departureDT(),
                                  tsArr->arrivalDT());
  utcOffset /= 60; // from minutes to hours

  for (size_t legIdx = 0; legIdx < trx.legs().size(); ++legIdx)
  {
    int minFlightDuration = INT_MAX;
    int minFlightDurationSopIdx = -1;
    for (size_t sopIdx = 0; sopIdx < trx.legs()[legIdx].sop().size(); ++sopIdx)
    {
      Itin* itin = trx.legs()[legIdx].sop()[sopIdx].itin();
      itin->calculateFlightTimeMinutes(utcOffset, legIdx);
      if (itin->getFlightTimeMinutes() < minFlightDuration)
      {
        minFlightDuration = itin->getFlightTimeMinutes();
        minFlightDurationSopIdx = sopIdx;
      }
    }
    trx.legs()[legIdx].setMinDurationSopIdx(minFlightDurationSopIdx);
  }
}

void
ItinAnalyzerService::calculateMileage(ShoppingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  for (size_t legIdx = 0; legIdx < trx.legs().size(); ++legIdx)
    for (size_t sopIdx = 0; sopIdx < trx.legs()[legIdx].sop().size(); ++sopIdx)
      trx.legs()[legIdx].sop()[sopIdx].itin()->calculateMileage(&trx);
}

bool
ItinAnalyzerService::process(RexShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "REX ITIN PROCESS");

  LOG4CXX_INFO(_logger, "ItinAnalyzerService::process(RexShoppingTrx)");

  setTrxDataHandleTicketDate(trx, trx.ticketingDate());

  checkRestrictedCurrencyNation(trx);

  trx.firstUnflownAirSeg() = TseUtil::getFirstUnflownAirSeg(trx.travelSeg(), trx.ticketingDate());

  trx.setTravelDate(TseUtil::getTravelDate(trx.travelSeg()));
  trx.setAdjustedTravelDate();

  trx.bookingDate() = TseUtil::getBookingDate(trx.travelSeg());

  //--- ECHANGE ITIN ----------------------------------------------------------

  trx.setAnalyzingExcItin(true);
  ExcItin* exItin = trx.exchangeItin().front();

  setFurthestPoint(trx, exItin);

  // Set GEO Travel Type and Ticketing Carrier
  setGeoTravelTypeAndTktCxr(*exItin);

  // Set retransit points
  setRetransits(*exItin);

  // Set openSegAfterDatedSeg
  setOpenSegFlag(*exItin);

  // Build fare markets
  buildFareMarket(trx, *exItin);

  // Set trip characteristics
  setTripCharacteristics(exItin); // setting stopOvers

  // Set validating carrier
  updateValidatingCarrier(trx, *exItin);

  // Set rounding
  setItinRounding(*exItin);

  // Exc Itin may have different Ticket and/or Sales Locs.
  const Loc* exTicketingLoc = getTicketLoc(trx);
  const Loc* exSaleLoc = getSalesLoc(trx);

  // Set international sales indicators
  setIntlSalesIndicator(*exItin, *exTicketingLoc, *exSaleLoc);

  // Set currency override
  setCurrencyOverride(trx);

  // Find travel commence date
  findCommenceDate(*exItin);

  // Set origin and calculation currency
  setItinCurrencies(trx);

  trx.setAnalyzingExcItin(false);

  trx.setTktValidityDate();

  recalculateFareCalcFareAmt(trx);

  ExcItinUtil::DetermineChanges(trx.exchangeItin().front(), trx.newItin().front());

  applyReissueExchange(trx);

  for (Itin* itin : trx.newItin())
  {
    selectProcesing(trx)->buildFareMarket(trx, *itin);
  }

  return true;
}

bool
ItinAnalyzerService::isRexTrxRedirected2ExcTrx(PricingTrx& trx) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX && trx.getParentTrx()))
  {
    PricingTrx* parentTrx = dynamic_cast<PricingTrx*>(trx.getParentTrx());
    return parentTrx && (parentTrx->excTrxType() == PricingTrx::AR_EXC_TRX ||
                         parentTrx->excTrxType() == PricingTrx::AF_EXC_TRX);
  }
  return false;
}

const CarrierPreference*
ItinAnalyzerService::getCarrierPref(PricingTrx& trx,
                                    const CarrierCode& carrier,
                                    const DateTime& date)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // Delegate - we need to override this in unit tests
  return tse::iadetail::JourneyPrepHelper::getCarrierPref(trx, carrier, date);
}

bool
ItinAnalyzerService::addFakeTravelSeg(PricingTrx& trx) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // Validate inboundDepartureDate and outboundDepartureDate
  if (!addFakeTravelSegValidateDates(trx))
  {
    return false;
  }

  // Nothing to do
  if (trx.itin().empty())
  {
    return true;
  }

  std::vector<Itin*>::iterator itinIter = trx.itin().begin();
  const std::vector<Itin*>::iterator itinIterEnd = trx.itin().end();

  // Create fake travel segment
  bool addOutbound = true;
  AirSeg* fakeTS = nullptr;

  addFakeTravelSegCreateFakeTS(trx, *itinIter, fakeTS, addOutbound);

  // Add fake travel segment to all Itins
  for (; itinIter != itinIterEnd; ++itinIter)
  {
    Itin* itin = (*itinIter);

    /*
     * Modyfied logic to get always stopover on fake segment.
     * If arrival/departure time of fake segment (first/last) is after/before departure/arrival time
     * of next/previous segment
     * or time break is lower than 4h 01min then set fake segment to 4h 0min 1sec before
     * departure/after arrival
     * to get stopover.
     * We want to get always stopover becouse we want to avoid situation when shopping
     * gives us lower tax AY than pricing.
     */
    if (addOutbound)
    {
      const AirSeg* const firstTS = itin->travelSeg().front()->toAirSeg();
      if (firstTS &&
          DateTime::diffTime(firstTS->departureDT(), fakeTS->arrivalDT()) <=
              RuleConst::STOPOVER_SEC_DOMESTIC)
      {
        AirSeg* fakeTSWithStopover = nullptr;
        addFakeTravelSegCreateFakeTS(trx, *itinIter, fakeTSWithStopover, addOutbound);
        fakeTSWithStopover->departureDT() =
            firstTS->departureDT().subtractSeconds(RuleConst::STOPOVER_SEC_DOMESTIC + 1);
        fakeTSWithStopover->arrivalDT() = fakeTSWithStopover->departureDT();
        itin->travelSeg().insert(itin->travelSeg().begin(), fakeTSWithStopover);
        addFakeTravelSegUpdateAvlMap(trx, fakeTSWithStopover);
      }
      else
      {
        itin->travelSeg().insert(itin->travelSeg().begin(), fakeTS);
      }
    }
    else // add inbound
    {
      const AirSeg* const lastTS = itin->travelSeg().back()->toAirSeg();
      if (lastTS &&
          DateTime::diffTime(fakeTS->departureDT(), lastTS->arrivalDT()) <=
              RuleConst::STOPOVER_SEC_DOMESTIC)
      {
        AirSeg* fakeTSWithStopover = nullptr;
        addFakeTravelSegCreateFakeTS(trx, *itinIter, fakeTSWithStopover, addOutbound);
        fakeTSWithStopover->departureDT() =
            lastTS->arrivalDT().addSeconds(RuleConst::STOPOVER_SEC_DOMESTIC + 1);
        fakeTSWithStopover->arrivalDT() = fakeTSWithStopover->departureDT();
        itin->travelSeg().push_back(fakeTSWithStopover);
        addFakeTravelSegUpdateAvlMap(trx, fakeTSWithStopover);
      }
      else
      {
        itin->travelSeg().push_back(fakeTS);
      }
    }

    if (TrxUtil::taxEnhancementActivated(DateTime::fromMilitaryTime(1200)))
    {
      ShoppingUtil::orderSegsInItin(itin);
    }
  }

  addFakeTravelSegUpdateAvlMap(trx, fakeTS);

  return true;
}

bool
ItinAnalyzerService::addFakeTravelSegValidateDates(const PricingTrx& trx) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.outboundDepartureDate().isEmptyDate() && trx.inboundDepartureDate().isEmptyDate())
  {
    LOG4CXX_ERROR(_logger,
                  "ItinAnalyzerService::addFakeTravelSegValidateDates - "
                  "outboundDepartureDate and inboundDepartureDate are empty");
    return false;
  }

  if (!trx.outboundDepartureDate().isEmptyDate() && !trx.inboundDepartureDate().isEmptyDate())
  {
    LOG4CXX_ERROR(_logger,
                  "ItinAnalyzerService::addFakeTravelSegValidateDates - "
                  "outboundDepartureDate and inboundDepartureDate are provided");
    return false;
  }

  return true;
}

void
ItinAnalyzerService::addFakeTravelSegCreateFakeTS(const PricingTrx& trx,
                                                  const Itin* const itin,
                                                  AirSeg*& fakeTS,
                                                  bool& addOutbound) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  const AirSeg* const firstTS = dynamic_cast<const AirSeg*>(itin->travelSeg().front());
  const AirSeg* const lastTS = dynamic_cast<const AirSeg*>(itin->travelSeg().back());

  fakeTS = trx.dataHandle().create<AirSeg>();

  fakeTS->segmentType() = tse::Air;
  fakeTS->stopOver() = true;
  fakeTS->origin() = lastTS->destination();
  fakeTS->destination() = firstTS->origin();
  fakeTS->origAirport() = lastTS->destAirport();
  fakeTS->destAirport() = firstTS->origAirport();
  fakeTS->boardMultiCity() = lastTS->offMultiCity();
  fakeTS->offMultiCity() = firstTS->boardMultiCity();

  if (trx.getTrxType() == PricingTrx::MIP_TRX &&
      trx.getRequest()->isGoverningCarrierOverrideEntry())
  {
    fakeTS->carrier() = trx.getRequest()->governingCarrierOverrides().begin()->second;
  }
  else
  {
    fakeTS->carrier() = firstTS->carrier();
  }

  fakeTS->setOperatingCarrierCode(firstTS->operatingCarrierCode());
  fakeTS->makeFake();

  fakeTS->bookedCabin() = trx.calendarRequestedCabin();
  fakeTS->carrierPref() = trx.dataHandle().create<CarrierPreference>();
  const_cast<CarrierPreference*>(fakeTS->carrierPref())->localMktJourneyType() = YES;
  fakeTS->setBookingCode(BookingCode('Y'));

  if (trx.outboundDepartureDate().isEmptyDate())
  {
    addOutbound = false;
    fakeTS->departureDT() = trx.inboundDepartureDate();
    fakeTS->arrivalDT() = trx.inboundDepartureDate();
    fakeTS->legId() = 1;
  }
  else
  {
    addOutbound = true;
    fakeTS->departureDT() = trx.outboundDepartureDate();
    fakeTS->arrivalDT() = trx.outboundDepartureDate();
    fakeTS->legId() = 0;
  }
}

void
ItinAnalyzerService::getCabins(PricingTrx& trx,
                               AirSeg* const fakeTS,
                               std::vector<ClassOfService*>& cosList) const
{
  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
  {
    for (char bkd = 'A'; bkd <= 'Z'; ++bkd)
    {
      ClassOfService* cos;
      trx.dataHandle().get(cos);
      cos->numSeats() = 9;
      cos->bookingCode() = bkd;
      cosList.push_back(cos);
    }
    RBDByCabinUtil rbdCabin(trx, ITIN_SHP_PYA);
    rbdCabin.getCabinsByRbd(*fakeTS, cosList);
  }
  else
  {
    for (char bkd = 'A'; bkd <= 'Z'; ++bkd)
    {
      const Cabin* const cabin =
          trx.dataHandle().getCabin(fakeTS->carrier(), BookingCode(bkd), fakeTS->departureDT());
      ClassOfService* cos;
      trx.dataHandle().get(cos);
      cos->cabin() = cabin->cabin();
      cos->numSeats() = 9;
      cos->bookingCode() = bkd;
      cosList.push_back(cos);
    }
  }
}

void
ItinAnalyzerService::addFakeTravelSegUpdateAvlMap(PricingTrx& trx, AirSeg* const fakeTS) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  trx.travelSeg().push_back(fakeTS);
  if (trx.availabilityMap().empty())
    return; // nothing to do
  std::vector<TravelSeg*> newKey(1, fakeTS);

  ClassOfServiceList cosList;

  getCabins(trx, fakeTS, cosList);

  fakeTS->classOfService() = cosList;

  std::vector<ClassOfServiceList>* mapElem;
  trx.dataHandle().get(mapElem);
  mapElem->push_back(cosList);
  (trx.availabilityMap())[ShoppingUtil::buildAvlKey(newKey)] = mapElem;
}

void
ItinAnalyzerService::cloneFakeFareMarkets(PricingTrx& trx)
{
  TSE_ASSERT(trx.getRequest()->originBasedRTPricing());
  TSE_ASSERT(!trx.getRequest()->isGoverningCarrierOverrideEntry());

  // find fake leg
  int fakeLegId = 0;
  if (trx.outboundDepartureDate().isEmptyDate())
    fakeLegId = 1;

  const std::vector<CarrierCode> emptyGoverningCarrierOverrides;
  const std::set<CarrierCode> emptyParticipatingCarriers;
  for (auto itin: trx.itin())
  {
    // find all carriers on real leg
    std::set<CarrierCode> carriersOnRealLeg;
    for (const auto& segment: itin->travelSeg())
    {
      if (!segment->isAir())
        continue;
      if (segment->legId() == fakeLegId)
        continue;
      carriersOnRealLeg.insert(segment->toAirSeg()->carrier());
    }

    if (carriersOnRealLeg.size() < 2) // nothing to do
      continue;

    const bool yyOverride = FareMarketUtil::isYYOverride(trx, itin->travelSeg());

    // find "fake" faremarket (one per itin)
    auto fmIter = std::find_if(itin->fareMarket().begin(), itin->fareMarket().end(),
                               [fakeLegId](const FareMarket* fm) {
                                   return fm->travelSeg().front()->isAir() &&
                                          fm->travelSeg().front()->legId() == fakeLegId;
                                });

    if (fmIter != itin->fareMarket().end())
    {
      FareMarket* fm = *fmIter;
      // clone it for rest of the carriers
      for (CarrierCode carrier: carriersOnRealLeg)
      {
        if (carrier == fm->governingCarrier())
          continue;

        selectProcesing(trx)->cloneAndStoreFareMarket(trx,
                                fm,
                                *itin,
                                fm->primarySector(),
                                carrier,
                                emptyGoverningCarrierOverrides,
                                emptyParticipatingCarriers,
                                yyOverride,
                                fm->breakIndicator(),
                                fm->removeOutboundFares(),
                                fm->isHighTPMGoverningCxr());
      }
    }
  }
}

void
ItinAnalyzerService::removeIncorrectFareMarkets(PricingTrx& trx) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::vector<Itin*>::iterator it = trx.itin().begin();
  const std::vector<Itin*>::iterator itE = trx.itin().end();
  for (; it != itE; ++it)
  {
    bool deleteFM = false;
    std::vector<FareMarket*>::iterator fm = (*it)->fareMarket().begin();
    while (fm != (*it)->fareMarket().end())
    {
      std::vector<TravelSeg*>::const_iterator tvlIt = (*fm)->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tvlIte = (*fm)->travelSeg().end();
      for (; tvlIt != tvlIte; ++tvlIt)
      {
        if ((*tvlIt)->isAir())
        {
          const AirSeg* airSeg = static_cast<const AirSeg*>((*tvlIt));
          if (airSeg->isFake())
          {
            if ((*fm)->travelSeg().size() > 1)
            {
              deleteFM = true;
            }
            break;
          }
        }
      }

      if (deleteFM)
      {
        // delete from trx
        std::vector<FareMarket*>::iterator fmTrx =
            std::find(trx.fareMarket().begin(), trx.fareMarket().end(), *fm);
        if (fmTrx != trx.fareMarket().end())
        {
          fmTrx = trx.fareMarket().erase(fmTrx);
        }

        // delete from itin
        fm = (*it)->fareMarket().erase(fm);
        deleteFM = false;
      }
      else
        ++fm;
    }
  }
}

void
ItinAnalyzerService::updateFakeFM(PricingTrx& trx) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);

  for (Itin* itin : trx.itin())
  {
    FareMarket* fakeFM = findFakeFM(trx, *itin);

    if (!fakeFM)
    {
      LOG4CXX_ERROR(_logger, "ItinAnalyzerService::updateFakeFM - fakeFM not found");
      return;
    }

    fakeFM->fareBasisCode() = 'Y';
    fakeFM->fareCalcFareAmt() = "0";
  }

  if (!setCalendarOutboundCurrencyBFRT(trx))
  {
    LOG4CXX_ERROR(_logger, "ItinAnalyzerService::updateFakeFM - currency for fakeFM not set");
    return;
  }
}

bool
ItinAnalyzerService::setCalendarOutboundCurrencyBFRT(PricingTrx& trx) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  DateTime& ticketingDate = trx.getRequest()->ticketingDT();
  NationCode nationCode;
  CurrencyCode nationCurrency;

  const Itin* const itin = trx.itin()[0];
  if (!itin)
    return false;

  const TravelSeg* const firstTS = itin->travelSeg()[0];
  if (!firstTS)
    return false;

  const NationCode& nation = firstTS->origin()->loc();
  if (FareCurrencySelection::getNationCode(trx, nation, ticketingDate, nationCode))
  {
    if (CurrencyUtil::getNationCurrency(nationCode, nationCurrency, ticketingDate))
    {
      trx.calendarCurrencyCode() = nationCurrency;
      return true;
    }
  }
  return false;
}

FareMarket*
ItinAnalyzerService::findFakeFM(PricingTrx& trx, Itin& itin) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  bool found = false;
  std::vector<FareMarket*>::const_iterator fm = itin.fareMarket().begin();
  const std::vector<FareMarket*>::const_iterator fmE = itin.fareMarket().end();
  while (!found && (fm != fmE))
  {
    if ((*fm)->travelSeg().size() == 1)
    {
      if (trx.getRequest()->originBasedRTPricing() && ((*fm)->travelSeg().front()->isAir()))
      {
        const AirSeg* airSeg = static_cast<const AirSeg*>((*fm)->travelSeg().front());
        if (airSeg->isFake())
        {
          found = true;
          break;
        }
      }
    }

    if (!found)
      ++fm;
  }

  return found ? *fm : nullptr;
}

void
ItinAnalyzerService::applyReissueExchange(RexPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (trx.applyReissueExchange())
  {
    trx.setHistoricalBsrRoeDate();
    if (!trx.previousExchangeDT().isEmptyDate())
      trx.previousExchangeDateFare() = true;

    BaseExchangeTrx* excTrx = static_cast<BaseExchangeTrx*>(&trx);
    determineExchangeReissueStatus(*excTrx);
  }
}

bool
ItinAnalyzerService::isSegmentChanged(const TravelSeg* seg) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  return seg->changeStatus() == TravelSeg::CHANGED ||
         seg->changeStatus() == TravelSeg::INVENTORYCHANGED;
}

void
ItinAnalyzerService::determineExchangeReissueStatus(BaseExchangeTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (!trx.newItin().empty())
  {
    TravelSeg* firstNewSeg = nullptr;
    TravelSeg* firstExcSeg = trx.exchangeItin().front()->travelSeg().front();
    bool firstExcSegChanged = isSegmentChanged(firstExcSeg);
    for (Itin* newItin : trx.itin())
    {
      firstNewSeg = newItin->travelSeg().front();
      (firstNewSeg->unflown() && (isSegmentChanged(firstNewSeg) || firstExcSegChanged))
          ? newItin->exchangeReissue() =
                EXCHANGE : newItin->exchangeReissue() = REISSUE;
    }
  }
}

void
ItinAnalyzerService::setPnrSegmentCollocation(PricingTrx* pricingTrx, std::vector<Itin*>& itins)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if (!itins.empty())
  {
    std::vector<Itin*>::iterator itn = itins.begin();
    for (; itn != itins.end(); ++itn)
    {
      const Itin* itin = *itn;
      std::vector<TravelSeg*>::const_iterator tvlSeg = itin->travelSeg().begin();
      for (int16_t pnrSegIndex = 0; tvlSeg != itin->travelSeg().end(); ++tvlSeg, ++pnrSegIndex)
      {
        int16_t itinPnrSegment = (*tvlSeg)->pnrSegment();
        for (const SimilarItinData& similarItinData : itin->getSimilarItins())
        {
          const Itin* simItin = similarItinData.itin;
          if (LIKELY(pnrSegIndex < static_cast<int>(simItin->travelSeg().size())))
          {
            int16_t similarItinPnrSegment = simItin->travelSeg()[pnrSegIndex]->pnrSegment();
            pricingTrx->setPnrSegmentCollocation(itinPnrSegment, similarItinPnrSegment);
          }
        }
      }
    }
  }
}

ItinAnalyzerService::SOPScoreComparator::SOPScoreComparator(
    double point,
    int noToAdd,
    int reqNoOfSeats,
    std::map<const Itin*, double>* sopScoreMap,
    const ShoppingTrx::Leg& curLeg,
    double qualityCoeff)
  : _point(point),
    _numberOfClassesToAdd(noToAdd),
    _requestedNumberOfSeats(reqNoOfSeats),
    _sopScoreMap(sopScoreMap),
    _curLeg(curLeg),
    _halfSops(curLeg.sop().size() / 2.),
    _qualityCoeff(qualityCoeff)
{
}

bool
ItinAnalyzerService::SOPScoreComparator::
operator()(const ItinIndex::ItinCell& firstCell, const ItinIndex::ItinCell& secondCell) const
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  if ((firstCell.second == nullptr) || (secondCell.second == nullptr))
  {
    return false;
  }
  int firstCellConnectionsNumber = firstCell.second->travelSeg().size();
  int secondCellConnectionsNumber = secondCell.second->travelSeg().size();
  if (firstCellConnectionsNumber != secondCellConnectionsNumber)
  {
    return firstCellConnectionsNumber < secondCellConnectionsNumber;
  }
  const ShoppingTrx::SchedulingOption& firstSched(_curLeg.sop()[firstCell.first.sopIndex()]),
      &secondSched(_curLeg.sop()[secondCell.first.sopIndex()]);
  bool firstInterline(firstSched.isInterline()), secondInterline(secondSched.isInterline());
  if (firstInterline != secondInterline)
  {
    return !firstInterline;
  }
  double score1 = ItinUtil::calculateEconomyAvailabilityScore(
      firstCell.second, _point, _numberOfClassesToAdd, _requestedNumberOfSeats, _sopScoreMap);
  double score2 = ItinUtil::calculateEconomyAvailabilityScore(
      secondCell.second, _point, _numberOfClassesToAdd, _requestedNumberOfSeats, _sopScoreMap);
  int firstSopIndex(firstCell.first.sopIndex()), secondSopIndex(secondCell.first.sopIndex());
  double firstWeight(1), secondWeight(1);
  firstWeight = 1 + (_halfSops - firstSopIndex) * _qualityCoeff / _halfSops;
  secondWeight = 1 + (_halfSops - secondSopIndex) * _qualityCoeff / _halfSops;
  double effScore1(score1 * firstWeight), effScore2(score2 * secondWeight);
  if (effScore1 != effScore2)
  {
    return effScore1 > effScore2;
  }
  return firstCell.first.sopIndex() < secondCell.first.sopIndex();
}

void
ItinAnalyzerService::discoverThroughFarePrecedence(const Trx& trx, Itin& itin)
{
  itin.setThroughFarePrecedence(TFPUtil::isThroughFarePrecedenceNeeded(trx, itin.travelSeg()));
}

void
ItinAnalyzerService::failIfThroughFarePrecedenceImpossible(Itin& itin)
{
  if (!itin.isThroughFarePrecedence())
    return;

  for (const auto ts : itin.travelSeg())
  {
    if (ts->isForcedNoFareBrk() || ts->isForcedStopOver() || ts->isForcedFareBrk() ||
        ts->isForcedConx())
    {
      throw NonFatalErrorResponseException(ErrorResponseException::NO_ERROR,
                                           "UNABLE TO PRICE AS REQUESTED");
    }
  }
}

// ******************************** WN SNAP  ******************************** //
namespace
{
class MultiLegFareMarkets
{
public:
  MultiLegFareMarkets(PricingTrx& trx, const Itin& itin) : _trx(trx), _itin(itin) {}

  bool operator()(FareMarket* fareMarket) const
  {
    const std::vector<TravelSeg*>& travelSeg = fareMarket->travelSeg();

    if (fareMarket->travelSeg().size() <= 1)
    {
      return false;
    }

    int16_t legId = -1;

    std::vector<TravelSeg*>::const_iterator travelSegIter = travelSeg.begin();
    std::vector<TravelSeg*>::const_iterator travelSegIterEnd = travelSeg.end();

    for (; travelSegIter != travelSegIterEnd; ++travelSegIter)
    {
      const TravelSeg* travelSeg = (*travelSegIter);

      if ((-1 == legId) && (travelSeg->segmentType() != Arunk))
      {
        legId = travelSeg->legId();
      }
      else if ((travelSeg->legId() != legId) && (travelSeg->segmentType() != Arunk))
      {
        // now remove from trx
        _trx.fareMarket().erase(
            std::remove(_trx.fareMarket().begin(), _trx.fareMarket().end(), fareMarket),
            _trx.fareMarket().end());
        return true;
      }
    }

    return false;
  }

private:
  PricingTrx& _trx;
  const Itin& _itin;
};
}

void
ItinAnalyzerService::removeMultiLegFareMarkets(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  // Remove stopover markets from both trx and itins
  std::vector<Itin*>::iterator itinIter = trx.itin().begin();
  std::vector<Itin*>::iterator itinIterEnd = trx.itin().end();

  for (; itinIter != itinIterEnd; ++itinIter)
  {
    Itin* itin = (*itinIter);
    itin->fareMarket().erase(std::remove_if(itin->fareMarket().begin(),
                                            itin->fareMarket().end(),
                                            MultiLegFareMarkets(trx, *itin)),
                             itin->fareMarket().end());
  }
}
// ******************************** WN SNAP  ******************************** //

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
bool
ItinAnalyzerService::process(AncillaryPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN PROCESS");

  LOG4CXX_DEBUG(_logger, "ANCILLARY TRX TICKETING DATE: " << trx.ticketingDate().toSimpleString());
  LOG4CXX_DEBUG(_logger,
                "DATA HANDLE TICKETING DATE: " << trx.dataHandle().ticketDate().toSimpleString());

  ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper = selectProcesing(trx);

  setTrxDataHandleTicketDate(trx, trx.ticketingDate());

  LOG4CXX_DEBUG(_logger,
                "DATA HANDLE TICKETING DATE: " << trx.dataHandle().ticketDate().toSimpleString());

  trx.setTravelDate(TseUtil::getTravelDate(trx.travelSeg()));
  trx.bookingDate() = TseUtil::getBookingDate(trx.travelSeg());

  // Logic to loop through the itin and establish furthest point on the travel segments

  std::vector<Itin*>::const_iterator i;

  for (i = trx.itin().begin(); i != trx.itin().end(); i++)
  {
    setFurthestPoint(trx, *i);
  }

  // Set ticketing carrier and geo travel type
  baseItinAnalyzerWrapper->selectTicketingCarrier(trx);

  // Set retransit points
  baseItinAnalyzerWrapper->setRetransits(trx);

  // Logic to build fare markets
  const AncRequest* ancRq = static_cast<AncRequest*>(trx.getRequest());
  const bool checkLimitations =
      !(ancRq->majorSchemaVersion() >= 2 &&
        (ancRq->isWPBGRequest() || trx.billing()->actionCode().substr(0, 5) == "MISC6"));

  buildFareMarket(trx, checkLimitations);

  // Set trip characteristics
  baseItinAnalyzerWrapper->setTripCharacteristics(trx);

  // Set international sales indicators
  setIntlSalesIndicator(trx);

  // Set currency override
  setCurrencyOverride(trx);

  setValidatingCarrier(trx);

  baseItinAnalyzerWrapper->setItinRounding(trx);

  bool isdssCallNeeded = trx.activationFlags().isAB240() || ancRq->isWPAERequest();
  bool isSchemaVersionCorrect = (ancRq->majorSchemaVersion() >= 2);
  bool isRequestFromACS = (trx.billing() && trx.billing()->requestPath() == ACS_PO_ATSE_PATH);
  bool isWPBGRequest = ancRq->isWPBGRequest();
  bool isActionCodeMISC6 = (trx.billing() && trx.billing()->actionCode().substr(0, 5) == "MISC6");

  if (isdssCallNeeded ||
      (isSchemaVersionCorrect && (isRequestFromACS || isWPBGRequest || isActionCodeMISC6)))
    setATAEContent(trx);

  return true;
}

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
bool
ItinAnalyzerService::process(TktFeesPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN PROCESS");

  LOG4CXX_DEBUG(_logger,
                "TICKETING FEES TRX TICKETING DATE: " << trx.ticketingDate().toSimpleString());
  LOG4CXX_DEBUG(_logger,
                "DATA HANDLE TICKETING DATE: " << trx.dataHandle().ticketDate().toSimpleString());
  ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper = selectProcesing(trx);
  setTrxDataHandleTicketDate(trx, trx.ticketingDate());
  LOG4CXX_DEBUG(_logger,
                "DATA HANDLE TICKETING DATE: " << trx.dataHandle().ticketDate().toSimpleString());

  trx.setTravelDate(TseUtil::getTravelDate(trx.travelSeg()));
  // Set ticketing carrier and geo travel type
  baseItinAnalyzerWrapper->selectTicketingCarrier(trx);
  // Set retransit points
  baseItinAnalyzerWrapper->setRetransits(trx);
  // Logic to build fare markets
  const bool checkLimitations = true;
  buildFareMarket(trx, checkLimitations);
  // Set trip characteristics
  baseItinAnalyzerWrapper->setTripCharacteristics(trx);
  // Set international sales indicators
  setIntlSalesIndicator(trx);
  // Set currency override
  setCurrencyOverride(trx);
  setValidatingCarrier(trx);
  baseItinAnalyzerWrapper->setItinRounding(trx);
  return true;
}

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
bool
ItinAnalyzerService::process(BrandingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);

  TSELatencyData metrics(trx, "ITIN PROCESS");

  _requestedNumberOfSeats = 1; // number of passengers doesn't influence brand retrieval process
  selectProcesing(trx)->initialize(trx);

  if (trx.getRequest()->isGoverningCarrierOverrideEntry())
  {
    populateGoverningCarrierOverrides(trx);
    removeItinsWithoutGoverningCarrier(trx);
  }

  ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper = selectProcesing(trx);

  setTrxDataHandleTicketDate(trx, trx.ticketingDate());

  tse::iadetail::JourneyPrepHelper::updateCarrierPreferences(trx);

  trx.setTravelDate(TseUtil::getTravelDate(trx.travelSeg()));

  setGeoTravelTypeAndValidatingCarrier(trx);

  buildFareMarket(trx);

  baseItinAnalyzerWrapper->setTripCharacteristics(trx);

  removeUnwantedFareMarkets(trx);

  retrieveBrands(trx);

  return true;
}

bool
ItinAnalyzerService::process(PricingDetailTrx& trx)
{
  if (!trx.getRequest()->isCollectOBFee())
    return true;

  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService() - Entered process(PricingDetailTrx)");
  return process(static_cast<TktFeesPricingTrx&>(trx));
}

bool
ItinAnalyzerService::process(AltPricingDetailObFeesTrx& trx)
{
  if (!trx.getRequest()->isCollectOBFee())
    return true;

  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService() - Entered process(AltPricingDetailObFeesTrx)");
  return process(static_cast<TktFeesPricingTrx&>(trx));
}

bool
ItinAnalyzerService::process(BaggageTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(trx, "ITIN PROCESS");

  if (trx.itin().empty())
    throw std::runtime_error("ITN WITH PRICING SOLUTION MISSING");

  ItinAnalyzerServiceWrapper* baseItinAnalyzerWrapper = selectProcesing(trx);

  setTrxDataHandleTicketDate(trx, trx.ticketingDate());

  trx.setTravelDate(TseUtil::getTravelDate(trx.travelSeg()));
  trx.bookingDate() = TseUtil::getBookingDate(trx.travelSeg());
  trx.itin().front()->setTravelDate(TseUtil::getTravelDate(trx.itin().front()->travelSeg()));

  // Set ticketing carrier and geo travel type
  baseItinAnalyzerWrapper->selectTicketingCarrier(trx);

  // Logic to build fare markets
  buildFareMarket(trx, false);

  // Set trip characteristics
  baseItinAnalyzerWrapper->setTripCharacteristics(trx);

  // Set international sales indicators
  setIntlSalesIndicator(trx);

  // Set currency override
  setCurrencyOverride(trx);

  setValidatingCarrier(trx);

  baseItinAnalyzerWrapper->setItinRounding(trx);

  _contentSvcs.getSched(trx);
  return true;
}

void
ItinAnalyzerService::retrieveBrands(PricingTrx& trx)
{
  if (trx.getRequest()->isBrandedFaresRequest())
  {
    if (!processIbfBrandRetrievalAndParity(trx))
    {
      throw ErrorResponseException(ErrorResponseException::NO_VALID_BRAND_FOUND);
    }
  }
  else if (trx.isBrandsForTnShopping() || trx.activationFlags().isSearchForBrandsPricing())
  {
    if (!processBfaBrandRetrieval(trx) &&
        (trx.isBRAll() || trx.activationFlags().isSearchForBrandsPricing()))
    {
      throw ErrorResponseException(ErrorResponseException::NO_VALID_BRAND_FOUND);
    }
  }
}

void
ItinAnalyzerService::removeSoloFMs(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  std::set<FareMarket*> fmsToRemove;
  for (const auto itin : trx.itin())
  {
    checkThruFMOnlyforNonAirSegs(*itin);

    if (!itin->isThruFareOnly())
      continue;

    // Go ahead for thru fare only itin
    // For this itin, we want to remove all
    // non-thru fare markets (i.e. not covering
    // exactly one leg)

    for (std::vector<FareMarket*>::iterator fmIter = itin->fareMarket().begin();
         fmIter != itin->fareMarket().end();
         ++fmIter)
    {
      FareMarket* fm = *fmIter;
      if (fm->isKeepSoloFM())
        continue;

      std::vector<TravelSeg*>& travelSeg = fm->travelSeg();
      TravelSeg* firstTvlSeg = travelSeg.front();
      TravelSeg* lastTvlSeg = travelSeg.back();
      // If leg ids for single fare market different,
      // mark it for removal
      if (firstTvlSeg->legId() == lastTvlSeg->legId())
      {
        std::vector<TravelSeg*>::iterator foundTvlSeg =
            std::find(itin->travelSeg().begin(), itin->travelSeg().end(), firstTvlSeg);
        TSE_ASSERT(foundTvlSeg != itin->travelSeg().end());

        bool isThruFMStart = (*foundTvlSeg == itin->travelSeg().front() ||
                              ((*(foundTvlSeg - 1))->legId() != firstTvlSeg->legId()));

        if (isThruFMStart)
        {
          foundTvlSeg = std::find(itin->travelSeg().begin(), itin->travelSeg().end(), lastTvlSeg);
          TSE_ASSERT(foundTvlSeg != itin->travelSeg().end());

          bool isThruFMEnd = (*foundTvlSeg == itin->travelSeg().back() ||
                              ((*(foundTvlSeg + 1))->legId() != lastTvlSeg->legId()));

          if (isThruFMEnd)
            // Leave a fare market only if it
            // exactly covers a leg
            continue;
        }
      }
      fmsToRemove.insert(fm);
    }
  }

  // actually remove them
  removeFareMarketsFromTrxAndItins(trx, fmsToRemove);
}

void
ItinAnalyzerService::checkThruFMOnlyforNonAirSegs(Itin& itin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  const TravelSeg* legFirstSeg = itin.travelSeg().front();
  const TravelSeg* legLastSeg = itin.travelSeg().front();

  for (std::vector<TravelSeg*>::const_iterator it = itin.travelSeg().begin();
       it != itin.travelSeg().end();
       ++it)
  {
    if ((*it)->legId() == legFirstSeg->legId())
      legLastSeg = *it;
    else
    {
      if (legFirstSeg->isNonAirTransportation() || legLastSeg->isNonAirTransportation())
      {
        itin.setThruFareOnly(false);
        break;
      }
      legFirstSeg = *it;
      legLastSeg = *it;
    }
  }

  if (legFirstSeg->isNonAirTransportation() || legLastSeg->isNonAirTransportation())
    itin.setThruFareOnly(false);
}

void
ItinAnalyzerService::markFlownSegments(PricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  DateTime localTime = DateTime::localTime();
  const Loc* hdqLoc = trx.dataHandle().getLoc("HDQ", localTime);
  for (Itin* itin : trx.itin())
  {
    for (TravelSeg* travelSeg : itin->travelSeg())
    {
      short utcOffset(0);
      LocUtil::getUtcOffsetDifference(
          *travelSeg->origin(), *hdqLoc, utcOffset, trx.dataHandle(), localTime, localTime);
      if (localTime.addSeconds(utcOffset * 60) > travelSeg->departureDT())
        travelSeg->unflown() = false;
      else
        break; // no need to check subsequent segments after finding the first unflown
    }
  }
}

void
ItinAnalyzerService::validateAllItinsPreferredCabinTypes(std::vector<Itin*>& itinVec,
                                                         PricingTrx& trx,
                                                         DiagCollector* collector)
// This method goes thru each Itin in a vector to see if it
// offers the same or the jumpdowned cabin type of that leg
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  size_t itinId = 0;
  while (itinId != itinVec.size())
  {
    Itin* itin = itinVec[itinId];
    if (validateSingleItinPreferredCabinTypes(itin, trx, collector))
    {
      validateSimilarItinsPreferredCabinTypes(*itin, trx, collector);
      ++itinId;
    }
    else
    {
      // If it's a mother itin, promote the first child as mother,
      // and keep the others as children of the new mother
      if (!itin->getSimilarItins().empty())
      {
        Itin* firstChildItin = itin->getSimilarItins().front().itin;
        itinVec.push_back(firstChildItin);
        firstChildItin->addSimilarItins(itin->getSimilarItins().begin() + 1,
                                        itin->getSimilarItins().end());
      }

      // remove this itn
      itinVec.erase(itinVec.begin() + itinId);
    }
  }
}

void
ItinAnalyzerService::validateSimilarItinsPreferredCabinTypes(Itin& itin,
                                                             PricingTrx& trx,
                                                             DiagCollector* collector)
{
  if (itin.getSimilarItins().empty())
    return;

  std::vector<Itin*> failedItins;
  for (const SimilarItinData& similarItinData : itin.getSimilarItins())
  {
    if (!validateSingleItinPreferredCabinTypes(similarItinData.itin, trx, collector))
      failedItins.push_back(similarItinData.itin);
  }

  for (Itin* failedItin : failedItins)
    itin.eraseSimilarItin(failedItin);
}

// The following method returns true if every travel segment of the current itin offers
// the same or the jumped down cabin type of its corresponding legs. In addition, for each
// leg in the itin, at least one preferred cabin must be offerred, i.e., if a leg of an
// itin only offers jumped down cabin, then this itin will be considered invalid.
bool
ItinAnalyzerService::validateSingleItinPreferredCabinTypes(Itin* itin,
                                                           PricingTrx& trx,
                                                           DiagCollector* collector)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  bool validItin = true;

  std::vector<TravelSeg*>::iterator tsItr = itin->travelSeg().begin();
  int16_t totalLegs = static_cast<int16_t>(trx.legPreferredCabinClass().size());
  std::set<int16_t> itinCabinData;

  if (collector)
  {
    (*collector) << "ITIN: " << itin->itinNum()
                 << ", TOTAL TRAVEL SEGMENTS: " << itin->travelSeg().size() << "\n";
    collector->flushMsg();
  }

  while ((tsItr != itin->travelSeg().end()) && (validItin))
  {
    TravelSeg* ts = *tsItr;

    if (ts->segmentType() == Arunk) // skip area unknown
    {
      ++tsItr;
      continue;
    }

    int16_t travelSegmentLegId = ts->legId();
    std::vector<ClassOfService*>::const_iterator cosItr = ts->classOfService().begin();

    if (travelSegmentLegId >= totalLegs)
    {
      validItin = false;
      if (collector)
      {
        (*collector) << "INVALID ITIN BECAUSE TRAVEL SEG LEG ID IS INVALID\n";
        collector->flushMsg();
      }
      break;
    }

    CabinType legPreferredCabin = trx.legPreferredCabinClass()[travelSegmentLegId];
    bool preferredCabinOffered = false;
    bool validSegment = false;
    int jumpedDownDistance = INT_MAX;
    BookingCode newBookingCode = "Z";
    CabinType newCabin;

    if (collector)
    {
      (*collector) << "TRVLSEG " << ts->origin()->loc() << "-" << ts->destination()->loc()
                   << " LEGID:" << travelSegmentLegId << " PREF CABIN:" << legPreferredCabin
                   << " ORIG BKC/CAB: " << ts->getBookingCode() << ts->bookedCabin() << "\n"
                   << " BKC|CAB: ";
      while (cosItr != ts->classOfService().end())
      {
        (*collector) << (*cosItr)->bookingCode() << (*cosItr)->cabin() << " ";
        ++cosItr;
      }
      (*collector) << "\n";
      collector->flushMsg();
    }

    cosItr = ts->classOfService().begin();
    while ((cosItr != ts->classOfService().end()) && (!preferredCabinOffered))
    {
      ClassOfService* cos = *cosItr;

      if (cos->cabin() == legPreferredCabin)
      {
        preferredCabinOffered = true;
        validSegment = true;
        itinCabinData.insert(travelSegmentLegId);
        newBookingCode = cos->bookingCode();
        newCabin = cos->cabin();
      }
      else if (isJumpedDownCabinOffered(legPreferredCabin, cos->cabin()))
      {
        validSegment = true;
        int newJumpedDownDistance = getJumpedDownDistance(legPreferredCabin, cos->cabin());

        if (newJumpedDownDistance < jumpedDownDistance)
        {
          jumpedDownDistance = newJumpedDownDistance;
          newBookingCode = cos->bookingCode();
          newCabin = cos->cabin();
        }
      }

      ++cosItr;
    }

    // no preferred or jumped down cabin offered for this particular travel segment
    if (!validSegment)
    {
      validItin = false;
      if (collector)
      {
        (*collector) << "RESULT:\n INVALID ITIN. SEG " << ts->origin()->loc() << "-"
                     << ts->destination()->loc() << " OFFERS NO PREFERRED OR JUMPED DOWN CABIN\n";
        collector->flushMsg();
      }
    }
    else // This is a valid segment
    {
      ts->setBookingCode(newBookingCode);
      ts->bookedCabin() = newCabin;
      if (collector)
      {
        if (preferredCabinOffered)
        {
          (*collector) << " PREFERRED CABIN OFFERED:";
        }
        else
        {
          (*collector) << " JUMPED DOWN CABIN OFFERED.";
        }
        (*collector) << "NEW BKC/CABIN:" << newBookingCode << newCabin << "\n";
      }
    }
    ++tsItr;
  }

  // Each travel segment in this itin at least offers all jumped down cabins.
  // Now check if every leg of the itin offers at least one preferred cabin.
  if (true == validItin)
  {
    if (static_cast<int>(itinCabinData.size()) < totalLegs)
    {
      validItin = false;
      if (collector)
      {
        (*collector) << "RESULT:\n INVALID ITIN. AT LEAST ONE LEG OFFERS NO PREFERRED CABIN\n";
        collector->flushMsg();
      }
    }
    else if (collector)
    {
      (*collector) << "RESULT:\n VALID ITIN\n";
      collector->flushMsg();
    }
  }

  if (collector)
  {
    (*collector) << "\n";
    collector->flushMsg();
  }

  return validItin;
}

bool
ItinAnalyzerService::isJumpedDownCabinOffered(const CabinType& preferredCabinForLeg,
                                              const CabinType& travelSegCabin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  bool isJumpedDownCabinOffered = false;
  CabinType currCabin = preferredCabinForLeg;
  CabinType nextJumpedDownCabin = preferredCabinForLeg;

  while ((!isJumpedDownCabinOffered) && (!nextJumpedDownCabin.isInvalidClass()))
  {
    nextJumpedDownCabin = CabinType::addOneLevelToCabinType(currCabin);

    if (travelSegCabin == nextJumpedDownCabin)
      isJumpedDownCabinOffered = true;

    currCabin = nextJumpedDownCabin;
  }

  return isJumpedDownCabinOffered;
}

int
ItinAnalyzerService::getJumpedDownDistance(const CabinType& preferredCabin,
                                           const CabinType& jumpedDownCabin)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__);
  return (jumpedDownCabin.getCabinIndicator() - preferredCabin.getCabinIndicator());
}

void
ItinAnalyzerService::markSimilarItinMarkets(const Itin& itin) const
{
  if (itin.getSimilarItins().empty())
    return;

  for (FareMarket* fm : itin.fareMarket())
    fm->setSimilarItinMarket();
}

bool
ItinAnalyzerService::needToPopulateFareMarkets(PricingTrx& trx, bool isRepricingFromMIP) const
{
  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    //@todo Is it good place for assert then null check?
    if (!trx.itin()[0]->spValidatingCxrGsaDataMap()) // should never happen here
      return false; // nothing to populate

    size_t valCxrCount = 0;
    for (const auto& it : *trx.itin()[0]->spValidatingCxrGsaDataMap())
      if (it.second)
        valCxrCount += it.second->size();

    if (trx.getTrxType() != PricingTrx::MIP_TRX && !isRepricingFromMIP && trx.itin().size() == 1 &&
        valCxrCount == 1)
      return false;
  }
  else if ((trx.getTrxType() != PricingTrx::MIP_TRX) && !isRepricingFromMIP &&
           (trx.itin().size() == 1) && (trx.itin()[0]->validatingCxrGsaData()->size() == 1))
    return false;

  return true;
}

bool
ItinAnalyzerService::itinHasValidatingCxrData(PricingTrx& trx, Itin& itin) const
{
  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    return itin.isValidatingCxrGsaDataForMultiSp();
  else
    return itin.validatingCxrGsaData() != nullptr;
}

void
ItinAnalyzerService::setAirSegsWithRequestedCabin(PricingTrx& trx)
{
  bool allCabin = false;

  CabinType reqCabin = trx.getOptions()->cabin();

  Diag185Collector* diag185 = createDiag185(trx);

  if (trx.getOptions()->cabin().isAllCabin())
  {
    reqCabin.setEconomyClass();
    allCabin = true;
  }

  std::vector<FareMarket*> gcmFareMarkets;
  if(!selectGcmFareMarkets(trx, gcmFareMarkets, diag185))
  {
    if(diag185)
    {
      diag185->printNoMarketsSelected();
      diag185->finishDiag185(trx);
    }
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "CABIN REQUESTED IS NOT OFFERED/AVAILABLE");
  }

  if(diag185)
    diag185->printProcessSelectedFareMarkets();

  int16_t skippedSegments = 0;

  bool internationalItin = isInternationalItin(trx);
  bool domesticItin = isItinWhollyWithInSameNation(*trx.itin().front());

  for (auto fm : gcmFareMarkets)
  {
    skippedSegments = 0;
    bool primarySector = false;
    bool allSecondary = false;
    // On wholly domestic within same nation or
    // international itinerary, do not consider primary sector on Domestic FMs
    if( domesticItin || (internationalItin && isDomesticFM(trx, *fm)))
      allSecondary = true;

    if(diag185)
      diag185->printFareMarket(trx, *fm);

    for (auto tvlSeg : fm->travelSeg())
    {
      if(!allSecondary&& !allCabin && fm->primarySector() && fm->primarySector() == tvlSeg &&
         tvlSeg->rbdReplaced() && tvlSeg->bookedCabin() > reqCabin)
      {
        fm->setInvalidByRbdByCabin();
        if(diag185)
          diag185->printInvalidFareMarket();
      }

      if (!tvlSeg->isAir() || tvlSeg->bookedCabin() == reqCabin || tvlSeg->rbdReplaced())
      {
        ++skippedSegments;
        if(diag185)
          diag185->printSkippedSegment(tvlSeg, reqCabin);
        continue;
      }
      if(!allSecondary)
        primarySector = fm->primarySector() && fm->primarySector() == tvlSeg;

      ClassOfService * cos = nullptr;
      Itin& itin = *trx.itin().front();
      std::vector<ClassOfService*>* cosVec = JourneyUtil::availability(tvlSeg, &itin);
      if (cosVec)
      {
        cos = findOfferedAndAvailable(trx, *cosVec, reqCabin,
                                      tvlSeg->bookedCabin(), allCabin);
        if (cos != nullptr && tvlSeg->bookedCabin() != cos->cabin())
        {
          tvlSeg->setBookingCode(cos->bookingCode());
          tvlSeg->bookedCabin() = cos->cabin();
          tvlSeg->rbdReplaced() = true;
          if (UNLIKELY(diag185))
            diag185->displayChangesDiag185(cos, tvlSeg, true);
        }
        else if(cos != nullptr && tvlSeg->bookedCabin() == cos->cabin())
        {
          if (UNLIKELY(diag185))
            diag185->printNoChanges(tvlSeg);
        }
      }
      if (cos == nullptr)
      {
        cos = findOfferedAndAvailable(trx, tvlSeg->classOfService(),
                                      reqCabin, tvlSeg->bookedCabin(),
                                      allCabin);
        if (cos != nullptr && tvlSeg->bookedCabin() != cos->cabin())
        {
          tvlSeg->setBookingCode(cos->bookingCode());
          tvlSeg->bookedCabin() = cos->cabin();
          tvlSeg->rbdReplaced() = true;
          if (UNLIKELY(diag185))
            diag185->displayChangesDiag185(cos, tvlSeg, false);
        }
        else if(cos != nullptr && tvlSeg->bookedCabin() == cos->cabin())
        {
          if (UNLIKELY(diag185))
            diag185->printNoChanges(tvlSeg);
        }
      }
      if (cos == nullptr && !allCabin)
      {
        if(primarySector) // invalidate FM for the primary sector
        {
          fm->setInvalidByRbdByCabin();
          if(diag185)
            diag185->printInvalidFareMarket();
        }
        else
        {
          if (_UNLIKELY(diag185))
          {
            diag185->printNoRBDfoundForSecondarySegment(tvlSeg, *fm);
          }
          throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                       "CABIN REQUESTED IS NOT OFFERED/AVAILABLE");
        }
      }
    }
  }

  if(gcmFareMarkets.empty())
  {
    if(diag185)
    {
      diag185->printNoMarketsSelected();
      diag185->finishDiag185(trx);
    }
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "CABIN REQUESTED IS NOT OFFERED/AVAILABLE");
  }
  // itinerary wholly within the same nation,
  // at least one segment in the itinerary must be booked or re-rebooked in the cabin requested
  if(!allCabin && domesticItin &&
     !atLeastOneSegmentInRequestedCabin(*trx.itin().front(), reqCabin))
  {
    if(diag185)
    {
      diag185->printNoOneSegmentInCabinRequested();
      diag185->finishDiag185(trx);
    }
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                "CABIN REQUESTED IS NOT OFFERED/AVAILABLE");
  }

  if(!allCabin && !domesticItin &&
     !isAllFareMarketsValid(trx, gcmFareMarkets, reqCabin, diag185))

    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                "CABIN REQUESTED IS NOT OFFERED/AVAILABLE");

  if (UNLIKELY(diag185))
    diag185->finishDiag185(trx);
}

ClassOfService *
ItinAnalyzerService::findOfferedAndAvailable(PricingTrx& trx,
                                             std::vector<ClassOfService*>& classOfService,
                                             CabinType reqCabin,
                                             CabinType bookedCabin,
                                             bool allCabin) const
{
  bool cabinOffered = false;
  CabinType nextJumpedDownCabin = reqCabin;
  while (!nextJumpedDownCabin.isInvalidClass())
  {
    for (std::vector<ClassOfService*>::reverse_iterator cE = classOfService.rbegin();
                 cE != classOfService.rend(); ++cE)
    {
      const ClassOfService& cos = **cE;

      if ((allCabin && bookedCabin <= cos.cabin()) ||
          (!allCabin && cos.cabin() == nextJumpedDownCabin))
      {
        cabinOffered = (cos.cabin() == reqCabin);
        if (trx.getRequest()->isLowFareNoAvailability() ||
            trx.getRequest()->isWpas() ||
            cos.numSeats() >= _requestedNumberOfSeats)
        {
          return *cE;
        }
      }
    }
    if (cabinOffered)  //Do not jump down if requested cabin is offered.
      break;
    nextJumpedDownCabin = CabinType::addOneLevelToCabinType(nextJumpedDownCabin);
  }
  return nullptr;
}

Diag185Collector*
ItinAnalyzerService::createDiag185(PricingTrx& trx)
{
  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic185))
  {
    Diag185Collector* diag185 = createDiag<Diag185Collector>(trx);
    diag185->startDiag185(trx);
    return diag185;
  }
  else
    return nullptr;
}

bool
ItinAnalyzerService::selectGcmFareMarkets(PricingTrx& trx,
                                          std::vector<FareMarket*>& gcmFareMarkets,
                                          Diag185Collector* diag185)
{
  if(trx.getOptions()->isRtw())
  {
    if(diag185)
    {
      diag185->printRTWSelectionHeader();
      diag185->printSelectedSegments(trx.itin().front()->travelSeg());
    }
    TrxUtil::getFareMarket(trx, trx.itin().front()->travelSeg(),
                           trx.itin().front()->travelSeg().front()->departureDT(),
                           gcmFareMarkets, trx.itin().front());
    return true;
  }

  std::vector<TravelSeg*> itinTvlSegs;
  itinTvlSegs.reserve(trx.itin().front()->travelSeg().size());

  for (auto tvl : trx.itin().front()->travelSeg())
  {
    itinTvlSegs.push_back(tvl);
  }

  if(diag185)
    diag185->printGcmSelectionHeader();

  int16_t tsAfterFurthest = -1;
  std::size_t foundMarketsSizeBefore = 0;
  std::vector<TravelSeg*> tvlSegs;

  // get fare markets till 1st turnaround point
  while(!itinTvlSegs.empty())
  {
    tsAfterFurthest = getGcm(itinTvlSegs, diag185);
    if( tsAfterFurthest == -1)
      break;

    tvlSegs.clear();
    for(int16_t i = 0; i <= tsAfterFurthest; ++i)
      tvlSegs.push_back(itinTvlSegs[i]);

    if(diag185)
      diag185->printSelectedSegments(tvlSegs);

    foundMarketsSizeBefore = gcmFareMarkets.size();

    TrxUtil::getFareMarket(trx, tvlSegs,
                           itinTvlSegs[0]->departureDT(),
                           gcmFareMarkets, trx.itin().front());

    if(foundMarketsSizeBefore == gcmFareMarkets.size())
    {
      if(diag185)
        diag185->printSelectedFmNotFound();
      selectGcmFareMarkets(trx, gcmFareMarkets, tvlSegs, diag185);
    }

    if(itinTvlSegs.size() == 1)
      itinTvlSegs.clear();
    else if(tsAfterFurthest == 0)
      itinTvlSegs.erase(itinTvlSegs.begin());
    else
      itinTvlSegs.erase(itinTvlSegs.begin(), (itinTvlSegs.begin() + tsAfterFurthest + 1));
  } // while

  if(gcmFareMarkets.empty())
    return false;

  return true;
}

void
ItinAnalyzerService::selectGcmFareMarkets(PricingTrx& trx, std::vector<FareMarket*>& gcmFareMarkets,
                                          std::vector<TravelSeg*>& tempTvlSegs, Diag185Collector* diag185)
{
  std::vector<TravelSeg*> itinTvlSegs;
  std::vector<TravelSeg*>::const_iterator it = tempTvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator itLast = tempTvlSegs.end() - 1;

  for (; it != itLast; ++it)
  {
    itinTvlSegs.push_back(*it);
  }

  bool first = true;
  int16_t tsAfterFurthest = -1;
  std::size_t foundMarketsSizeBefore = 0;
  std::vector<TravelSeg*> marketTvlSegs;

  // get fare markets till 1st turnaround point
  while(!itinTvlSegs.empty())
  {
    tsAfterFurthest = getGcm(itinTvlSegs, diag185);
    if( tsAfterFurthest == -1)
      break;

    if(first)
    {
      itinTvlSegs.push_back(tempTvlSegs.back());
      first = false;
    }
    marketTvlSegs.clear();
    for(int16_t i = 0; i <= tsAfterFurthest; ++i)
      marketTvlSegs.push_back(itinTvlSegs[i]);

    if(diag185)
      diag185->printSelectedSegments(marketTvlSegs);

    if(!fallback::fallbackSSDSP2058FMSelection(&trx))
      foundMarketsSizeBefore = gcmFareMarkets.size();

    TrxUtil::getFareMarket(trx, marketTvlSegs,
                           itinTvlSegs[0]->departureDT(),
                           gcmFareMarkets, trx.itin().front());

    if(foundMarketsSizeBefore == gcmFareMarkets.size())
    {
      if(!fallback::fallbackSSDSP2058FMSelection(&trx))
      {
        if(diag185)
          diag185->printSelectedFmNotFound();
      }
      selectGcmFareMarkets(trx, gcmFareMarkets, marketTvlSegs, diag185);
    }

    if(itinTvlSegs.size() == 1)
      itinTvlSegs.clear();
    else if(tsAfterFurthest == 0)
      itinTvlSegs.erase(itinTvlSegs.begin());
    else
      itinTvlSegs.erase(itinTvlSegs.begin(), (itinTvlSegs.begin() + tsAfterFurthest + 1));
  } // while

}



int16_t
ItinAnalyzerService::getGcm(std::vector<TravelSeg*>& itinTvlSegs, Diag185Collector* diag185)
{
  if(diag185)
    diag185->printSegmentSelectedHeader();

  uint32_t highestGcm = 0;

  std::vector<TravelSeg*>::iterator tsi = itinTvlSegs.begin();
  std::vector<TravelSeg*>::iterator turnaroundSegment = itinTvlSegs.end();

  int16_t segmentNumber = 0;
  for (int16_t i = 0; tsi != itinTvlSegs.end(); ++tsi, ++i)
  {
    const uint32_t gcm =
        TseUtil::greatCircleMiles(*itinTvlSegs.front()->origin(), *(**tsi).destination());

    if (diag185)
      diag185->printSegmentGcm(itinTvlSegs.front()->origAirport(), **tsi, gcm);

    if (highestGcm < gcm)
    {
      highestGcm = gcm;
      turnaroundSegment = tsi;
      segmentNumber = i;
    }
  }

  if (diag185)
  {
    if (turnaroundSegment != itinTvlSegs.end())
      diag185->printFurthestPoint(**turnaroundSegment, highestGcm);
    else
      diag185->printFurthestPointNotSet();
  }

  if (turnaroundSegment == itinTvlSegs.end())
  {
    if (diag185)
      diag185->printGcmNotCalculated();

    return -1;
  }
  return segmentNumber;
}

bool
ItinAnalyzerService::isAllFareMarketsValid(const PricingTrx& trx,
                                           std::vector<FareMarket*>&gcmFareMarkets,
                                           CabinType reqCabin,
                                           Diag185Collector* diag185)
{
  FareMarket* prevFM = 0;
  FareMarket* invalidFM = 0;
  bool prevFMinvalid = false;
  bool internationalItin = isInternationalItin(trx);

  for(auto currentFm : gcmFareMarkets)
  {
    if(prevFM && isCurrentFmSameAsPrevFM(currentFm, prevFM)) // current FM same as previous FM
    {
      if(( currentFm->isFMInvalidByRbdByCabin() ||
          (currentFm->primarySector() && currentFm->primarySector()->bookedCabin() != reqCabin &&
           internationalItin && !isDomesticFM(trx, *currentFm))) && prevFMinvalid )
      {
        currentFm->setInvalidByRbdByCabin();
        prevFMinvalid = true;
        invalidFM = currentFm;
      }
      else
        prevFMinvalid = false;
    }
    else // current FM is not the same as previous
    {
      if(prevFMinvalid)
      {
        if(diag185)
          diag185->printInvalidFM(*invalidFM);
        return false;
      }
      else if(currentFm->isFMInvalidByRbdByCabin())
      {
        prevFMinvalid = true;
        invalidFM = currentFm;
      }
      else if(currentFm->primarySector() && currentFm->primarySector()->bookedCabin() != reqCabin &&
              internationalItin && !isDomesticFM(trx, *currentFm))
      {
        currentFm->setInvalidByRbdByCabin();
        prevFMinvalid = true;
        invalidFM = currentFm;
      }
    }
    prevFM = currentFm;
  }

  if(prevFMinvalid)
  {
    if(diag185)
      diag185->printInvalidFM(*invalidFM);
    return false;
  }

  return true;
}

bool
ItinAnalyzerService::isCurrentFmSameAsPrevFM(const FareMarket* currentFm, const FareMarket* prevFM)
{
  if(!currentFm || !prevFM)
    return false;

  if(currentFm->travelSeg().size()  != prevFM->travelSeg().size() ||
     currentFm->travelSeg().front() != prevFM->travelSeg().front() ||
     currentFm->travelSeg().back()  != prevFM->travelSeg().back())
    return false;

  return true;
}

bool
ItinAnalyzerService::isDomesticFM(const PricingTrx& trx, const FareMarket& fm)
{
  return (!trx.getOptions()->isRtw() &&
          (fm.isDomestic() || fm.isForeignDomestic() ||
           fm.isTransBorder() || fm.isWithinRussianGroup() ||
          (LocUtil::oneOfNetherlandAntilles(*fm.origin()) &&
           LocUtil::oneOfNetherlandAntilles(*fm.destination())&&
           isItinOutsideNetherlandAntilles(*trx.itin().front())) ||
          (LocUtil::isEurope(*fm.origin()) && LocUtil::isEurope(*fm.destination()) &&
           isItinOutsideEurope(*trx.itin().front()))));
}

bool
ItinAnalyzerService::isItinOutsideEurope(const Itin& itin)
{
  if ( itin.geoTravelType() != GeoTravelType::International)
    return false;

  for(auto tvl : itin.travelSeg())
  {
    if(!LocUtil::isEurope(*tvl->origin()) ||
       !LocUtil::isEurope(*tvl->destination()) )
      return true;
  }
  return false;
}

bool
ItinAnalyzerService::isItinOutsideNetherlandAntilles(const Itin& itin)
{
  if ( itin.geoTravelType() != GeoTravelType::International)
    return false;

  for(auto tvl : itin.travelSeg())
  {
    if(!LocUtil::oneOfNetherlandAntilles(*tvl->origin()) ||
       !LocUtil::oneOfNetherlandAntilles(*tvl->destination()) )
      return true;
  }
  return false;
}

bool
ItinAnalyzerService::isInternationalItin(const PricingTrx& trx)
{
  return (trx.itin().front()->geoTravelType() == GeoTravelType::International);
}

bool
ItinAnalyzerService::isItinWhollyWithInSameNation(const Itin& itin)
{
  if ( itin.geoTravelType() == GeoTravelType::International)
    return false;

  for(auto tvl : itin.travelSeg())
  {
    if(itin.travelSeg().front()->origin()->nation() == tvl->destination()->nation())
      continue;
    else if((LocUtil::isDomesticUSCA(*tvl->destination()) &&
             LocUtil::isDomesticUSCA(*itin.travelSeg().front()->origin())) ||
            (LocUtil::isNetherlandsAntilles(*tvl->destination()) &&
             LocUtil::isNetherlandsAntilles(*itin.travelSeg().front()->origin())) ||
            (LocUtil::isRussia(*tvl->destination()) &&
             LocUtil::isRussia(*itin.travelSeg().front()->origin())))
      continue;
    else
      return false;
  }
  return true;
}

bool
ItinAnalyzerService::atLeastOneSegmentInRequestedCabin(const Itin& itin, CabinType reqCabin)
{
  for(auto tvl : itin.travelSeg())
  {
    if(!tvl->isAir())
      continue;
    if(tvl->bookedCabin() == reqCabin)
      return true;
  }
  return false;
}

} // tse
