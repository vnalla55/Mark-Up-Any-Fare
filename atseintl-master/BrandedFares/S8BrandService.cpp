//
//  Copyright Sabre 2013
//
//	    The copyright to the computer program(s) herein
//	    is the property of Sabre.
//	    The program(s) may be used and/or copied only with
//	    the written permission of Sabre or in accordance
//	    with the terms and conditions stipulated in the
//	    agreement/contract under which the	program(s)
//	    have been supplied.


#include "BrandedFares/S8BrandService.h"

#include "BrandedFares/BrandedFaresUtil.h"
#include "BrandedFares/BrandingRequestResponseHandler.h"
#include "BrandedFares/BrandingService.h"
#include "BrandedFares/PbbS8BrandedFaresSelector.h"
#include "Common/BrandingUtil.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Billing.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag892Collector.h"
#include "Diagnostic/Diag894Collector.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "Server/TseServer.h"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/cxx11/copy_if.hpp>

#include <iostream>
#include <string>

namespace tse
{
FALLBACK_DECL(fallbackBrandedFaresFareDisplay);
FALLBACK_DECL(fallbackBrandedServiceInterface);
FALLBACK_DECL(fallbackSendDirectionToMM)

static Logger
logger("atseintl.BrandedFares.S8BrandService");

static LoadableModuleRegister<Service, S8BrandService>
_("libBrandedFares.so");

namespace
{
bool
hasToBeProcessed(FareMarket* fareMarket)
{
  TSE_ASSERT(fareMarket != nullptr);

  return (!fareMarket->breakIndicator() && fareMarket->hasBrandCode());
}

class CollectFareMarkets
{

public:
  CollectFareMarkets(std::vector<FareMarket*>& markets) : _markets(markets)
  {}

  //posibble duplicates in MIP!!
  void operator()(const Itin* itin) const
  {
    std::copy(itin->fareMarket().begin(), itin->fareMarket().end(), std::back_inserter(_markets));
  }

private:
  std::vector<FareMarket*>& _markets;
};

}

S8BrandService::S8BrandService(const std::string& name, TseServer& srv)
  : Service(name, srv), _config(srv.config())
{
}

bool
S8BrandService::initialize(int argc, char* argv[])
{
  return true;
}

bool
S8BrandService::process(MetricsTrx& trx)
{
  std::ostringstream& oss = trx.response();

  MetricsUtil::header(oss, "S8 Brand Metrics");
  MetricsUtil::lineItemHeader(oss);
  MetricsUtil::lineItem(oss, MetricsUtil::S8_BRAND_PROCESS);

  return true;
}

bool
S8BrandService::process(RexExchangeTrx& trx)
{
  return true;
}

bool
S8BrandService::process(ExchangePricingTrx& trx)
{
  if (trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS)
  {
    std::vector<FareMarket*> newFareMarkets;
    collectAllFMsFromNewItins(trx, newFareMarkets);

    processImpl(static_cast<PricingTrx&>(trx), newFareMarkets);
  }

  return true;
}

bool
S8BrandService::process(RexPricingTrx& trx)
{
  if (trx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    std::vector<FareMarket*> newFareMarkets;
    collectAllFMsFromNewItins(trx, newFareMarkets);

    processImpl(static_cast<PricingTrx&>(trx), newFareMarkets);
  }

  return true;
}

void
S8BrandService::processImpl(PricingTrx& trx, const std::vector<FareMarket*>& fareMarkets)
{
  std::vector<FareMarket*> markets;

  if (Diagnostic888 == trx.diagnostic().diagnosticType())
  {
    BrandingRequestResponseHandler bRRH(trx);
    if (fallback::fallbackBrandedServiceInterface(&trx))
      bRRH.setClientId(BR_CLIENT_PBB);
    else
    {
      bRRH.setClientId(BR_CLIENT_PBB);
      bRRH.setActionCode(BR_ACTION_SHOPPING);
    }

    bRRH.brandedFaresDiagnostic888();
  }
  else if(!fareMarkets.empty())
  {
    getMarketsToFillWithBrands(fareMarkets, markets);
    getBrandsForFMs(trx, markets);
  }
}

bool
S8BrandService::process(PricingTrx& trx)
{
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
    return true;

  AltPricingTrx* altPricingTrx = dynamic_cast<AltPricingTrx*>(&trx);
  if (altPricingTrx && (altPricingTrx->altTrxType() == AltPricingTrx::WP_NOMATCH ||
      altPricingTrx->altTrxType() == AltPricingTrx::WPA))
    return true;

  LOG4CXX_INFO(logger, "Started Process()");
  TSELatencyData metrics(trx, "S8 BRAND PROCESS");

  if (trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS)
  {
    if (Diagnostic888 == trx.diagnostic().diagnosticType())
    {
      BrandingRequestResponseHandler bRRH(trx);
      if (fallback::fallbackBrandedServiceInterface(&trx))
        bRRH.setClientId(BR_CLIENT_PBB);
      else
      {
        bRRH.setClientId(BR_CLIENT_PBB);
        bRRH.setActionCode(BR_ACTION_SHOPPING);
      }

      bRRH.brandedFaresDiagnostic888();
    }
    else if (!trx.fareMarket().empty())
    {
      std::vector<FareMarket*> markets;
      getMarketsToFillWithBrands(trx.fareMarket(), markets);
      getBrandsForFMs(trx, markets);
    }
  }
  LOG4CXX_INFO(logger, "Leaving S8 BRAND Service");
  return true;
}

namespace {
  template <typename D> D* startActiveDiag(DiagCollector* dc, DiagnosticTypes diagNum)
  {
    D* diag = dynamic_cast<D*>(dc);
    if (diag != nullptr)
    {
      diag->enable(diagNum);
      diag->printHeader();
      return diag;
    }
    return nullptr;
  }

  void displayDiag(DiagCollector* diag, PricingTrx& trx,
                   const IAIbfUtils::FMsForBranding& fMsForBranding,
                   BrandRetrievalMode mode,
                   IAIbfUtils::TripType tripType,
                   bool haveYYFmsBeenIgnored)
  {
    if (diag != nullptr)
    {
      BrandedDiagnosticUtil::displayBrandRetrievalMode(*diag, mode);
      BrandedDiagnosticUtil::displayTripType(*diag, tripType);
      if (haveYYFmsBeenIgnored)
      {
        *diag << "\nYY FARE MARKETS NOT SENT TO BRANDING."
          "\nIGNORE_YY_FMS_IN_BRANDING SET TO TRUE IN CONFIG FILE\n\n";
      }
      bool showOnd = (Diagnostic894 == trx.diagnostic().diagnosticType());
      BrandedDiagnosticUtil::displayAllBrandIndices(*diag, trx.brandProgramVec(), showOnd);
      IAIbfUtils::OdcsForBranding invertedMap;
      IAIbfUtils::invertOdcToFmMap(fMsForBranding, invertedMap);
      BrandedDiagnosticUtil::displayFareMarketsWithBrands(*diag, trx.fareMarket(), trx.brandProgramVec(),
                                                          &invertedMap, showOnd);
      diag->flushMsg();
    }
  }
}

void
S8BrandService::getBrandsForFMs(PricingTrx& trx, std::vector<FareMarket*>& fareMarkets)
{
  BrandingRequestResponseHandler bRRH(trx);
  if (fallback::fallbackBrandedServiceInterface(&trx))
    bRRH.setClientId(BR_CLIENT_PBB);
  else
  {
    bRRH.setClientId(BR_CLIENT_PBB);
    bRRH.setActionCode(BR_ACTION_SHOPPING);
  }

  IAIbfUtils::FMsForBranding fMsForBranding;
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagCollector = factory->create(trx);
  Diag892Collector* diag892 = startActiveDiag<Diag892Collector>(diagCollector, Diagnostic892);
  Diag894Collector* diag894 = startActiveDiag<Diag894Collector>(diagCollector, Diagnostic894);

  BrandRetrievalMode mode = BrandRetrievalMode::PER_FARE_COMPONENT;
  IAIbfUtils::TripType tripType = IAIbfUtils::calculateTripType(trx);
  bool haveYYFmsBeenIgnored = false;

  if (TrxUtil::isRequestFromAS(trx))
  {
    mode = BrandingUtil::getBrandRetrievalMode(trx);
    buildASBrandingRequest(
      fareMarkets, trx, bRRH, fMsForBranding, mode, tripType, haveYYFmsBeenIgnored);
  }
  else
    buildBrandingRequest(fareMarkets, trx, bRRH, fMsForBranding);

  if (!bRRH.getBrandedFares())
  {
    switch(bRRH.getStatusBrandingService())
    {
      case StatusBrandingService::BS_UNAVAILABLE:
        throw ErrorResponseException(ErrorResponseException::BRANDING_SERVICE_UNAVAILABLE);
      case StatusBrandingService::BS_INVALID_RESPONSE:
        throw ErrorResponseException(ErrorResponseException::BRANDING_SERVICE_INVALID_RESPONSE);
      default:
        break;
    }
  }
  validateBrandsAndPrograms(trx, fareMarkets);

  BrandedFareValidatorFactory brandedFareValidatorFactory(trx);
  PbbBrandedFaresSelector brandedFaresSelector(trx, brandedFareValidatorFactory);
  for (FareMarket* market : fareMarkets)
    brandedFaresSelector.validate(*market);

  displayDiag(diag892, trx, fMsForBranding, mode, tripType, haveYYFmsBeenIgnored);
  displayDiag(diag894, trx, fMsForBranding, mode, tripType, haveYYFmsBeenIgnored);
}

void
S8BrandService::collectAllFMsFromNewItins(PricingTrx& trx, std::vector<FareMarket*>& fareMarkets)
{
  std::for_each(trx.itin().begin(), trx.itin().end(), CollectFareMarkets(fareMarkets));
}

void
S8BrandService::validateBrandsAndPrograms(PricingTrx& trx, const std::vector<FareMarket*>& markets) const
{
  if (Diagnostic889 == trx.diagnostic().diagnosticType())
    return;

  BrandAndProgramValidator bpValidator(trx.brandedMarketMap());
  bpValidator.validate(markets);
}

bool
S8BrandService::process(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(logger, "Started Process()");
  TSELatencyData metrics(trx, "S8 BRAND PROCESS");
  if (TrxUtil::isFqS8BrandedServiceActivated(trx) &&
      !fallback::fallbackBrandedFaresFareDisplay(&trx) &&
      (trx.getRequest()->requestType() == "FQ" || trx.getRequest()->requestType() == "RD"))
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic888)
    {
      BrandingRequestResponseHandler bRRH(trx);
      if (fallback::fallbackBrandedServiceInterface(&trx))
        bRRH.setClientId(BR_CLIENT_FQ);
      else
      {
        bRRH.setClientId(BR_CLIENT_FQ);
        bRRH.setActionCode(BR_ACTION_FQ);
      }
      bRRH.brandedFaresDiagnostic888();
    }

    if (!BrandingUtil::isBrandGrouping(trx))
    {
      if (trx.bfErrorCode() == PricingTrx::CARRIER_NOT_ACTIVE &&
          ("S8BRANDREQ" == trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) ||
           "S8BRANDRES" == trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) ||
           Diagnostic898 == trx.diagnostic().diagnosticType() ||
           Diagnostic889 == trx.diagnostic().diagnosticType()))
        printBFErrorCodeFDDiagnostic(trx);
      else
        trx.bfErrorCode() = PricingTrx::BG_ERROR;
      return true;
    }

    trx.setS8ServiceCalled();

    BrandingService bs(trx);
    if (!bs.getBrandedFares())
    {
      trx.bfErrorCode() = PricingTrx::BS_ERROR;
    }
  }
  LOG4CXX_INFO(logger, "Leaving S8 BRAND Service");

  return true;
}

void
S8BrandService::getPaxTypes(PricingTrx& trx, std::vector<PaxTypeCode>& paxTypes)
{
  //--------------------------------------------------
  // Create a unique set of carriers in the itinerary
  //--------------------------------------------------
  typedef std::set<CarrierCode> CarrierSet;
  CarrierSet cxrSet;
  if (trx.getOptions()->isIataFares())
    cxrSet.insert(INDUSTRY_CARRIER);
  else
    collectCarriers(trx, cxrSet);

  //---------------------------------------------
  // Process each paxType in the Pricing Request
  //---------------------------------------------
  const std::vector<PaxType*>& reqPaxType = trx.paxType();
  std::vector<PaxType*>::const_iterator paxTypeIter = reqPaxType.begin();

  for (; paxTypeIter != reqPaxType.end(); ++paxTypeIter)
  {
    PaxType& curPaxType = **paxTypeIter;

    collectPtcCodes(cxrSet, curPaxType, paxTypes);
  }
}

void
S8BrandService::collectCarriers(PricingTrx& trx, std::set<CarrierCode>& cxrSet)
{
  const AirSeg* airSeg;
  std::vector<Itin*>::const_iterator itinI = trx.itin().begin();
  for (; itinI != trx.itin().end(); itinI++)
  {
    std::vector<TravelSeg*>::const_iterator travelSegI = (*itinI)->travelSeg().begin();
    for (; travelSegI != (*itinI)->travelSeg().end(); travelSegI++)
    {
      airSeg = dynamic_cast<AirSeg*>(*travelSegI);
      if (!airSeg)
        continue;

      cxrSet.insert(airSeg->carrier());
    }
  }
}

void
S8BrandService::collectPtcCodes(std::set<CarrierCode>& cxrSet,
                                PaxType& curPaxType,
                                std::vector<PaxTypeCode>& paxTypes)
{
  //-----------------------------------
  // For each carrier in the itinerary
  //-----------------------------------
  std::set<CarrierCode>::const_iterator cxrItr = cxrSet.begin();
  std::set<CarrierCode>::const_iterator cxrEnd = cxrSet.end();

  for (; cxrItr != cxrEnd; ++cxrItr)
  {
    collectPaxTypes(*cxrItr, curPaxType, paxTypes);
  }
  collectPaxTypes(ANY_CARRIER, curPaxType, paxTypes);
}

void
S8BrandService::collectPaxTypes(const CarrierCode& carrier,
                                const PaxType& paxType,
                                std::vector<PaxTypeCode>& paxTypes)
{
  const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypes = paxType.actualPaxType();
  std::map<CarrierCode, std::vector<PaxType*>*>::const_iterator grpI = actualPaxTypes.find(carrier);

  if (grpI == actualPaxTypes.end())
    return;

  const std::vector<PaxType*>* grpPaxTypes = (*grpI).second;
  std::vector<PaxType*>::const_iterator paxTypesI, paxTypesEnd;
  paxTypesI = grpPaxTypes->begin();
  paxTypesEnd = grpPaxTypes->end();

  for (; paxTypesI != paxTypesEnd; ++paxTypesI)
  {
    PaxType& curPaxType = **paxTypesI;
    if (find(paxTypes.begin(), paxTypes.end(), curPaxType.paxType()) == paxTypes.end())
      paxTypes.push_back(curPaxType.paxType());
  }
}

void
S8BrandService::printBFErrorCodeFDDiagnostic(FareDisplayTrx& trx)
{
  if (trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX)
    return;
  std::string data = "REQUESTED CARRIER " + trx.requestedCarrier() + " INACTIVE FOR BRAND SERVICE";
  trx.response() << data.c_str() << std::endl << " " << std::endl;
}

void
S8BrandService::buildBrandingRequest(const std::vector<FareMarket*>& markets,
                                     PricingTrx& trx,
                                     BrandingRequestResponseHandler& bRRH,
                                     IAIbfUtils::FMsForBranding& fMsForBranding)
{
  std::vector<PaxTypeCode> paxTypes;
  getPaxTypes(trx, paxTypes);

  BrandRetrievalMode mode = BrandRetrievalMode::PER_FARE_COMPONENT;
  IAIbfUtils::TripType tripType = IAIbfUtils::calculateTripType(trx);
  Itin* itin = trx.itin().front();
  for (FareMarket* fm : markets)
  {
    IAIbfUtils::findAllOdDataForMarketPricing(itin, fm, tripType, mode, trx,
        [&](IAIbfUtils::OdcTuple& odcTuple){ fMsForBranding[odcTuple].push_back(fm); });
  }

  AlphaCode direction;
  if (!fallback::fallbackSendDirectionToMM(&trx))
    direction = "OT";

  for (const auto& item : fMsForBranding)
  {
    const DateTime& travelDate(item.second.front()->travelSeg().front()->departureDT());
    bRRH.buildMarketRequest(item.first.origin,
                            item.first.destination,
                            travelDate,
                            paxTypes,
                            std::vector<CarrierCode>(1, item.first.governingCarrier),
                            item.second,
                            GlobalDirection::NO_DIR,
                            direction);
  }
}

void
S8BrandService::buildASBrandingRequest(const std::vector<FareMarket*>& markets,
                                       PricingTrx& trx,
                                       BrandingRequestResponseHandler& bRRH,
                                       IAIbfUtils::FMsForBranding& fMsForBranding,
                                       const BrandRetrievalMode mode,
                                       const IAIbfUtils::TripType tripType,
                                       bool& haveYYFmsBeenIgnored)
{
  std::string value;
  Global::config().getValue("IGNORE_YY_FMS_IN_BRANDING", value, "S8_BRAND_SVC");
  // At the time of writing this project there were no YY fares filed in S8
  // so it was useless to use FMs with YY as governing carrier in brand retrieval
  const bool shouldYYFMsBeIgnored = (value == "Y" || value == "True");
  haveYYFmsBeenIgnored = false;

  for (const TravelSeg* tSeg: trx.travelSeg())
    TSE_ASSERT(tSeg->legId() != -1);

  TSE_ASSERT(trx.itin().size() == 1);

  Itin* itin = trx.itin().front();
  //split arunks into separate sections, update itin->itinLegs() vector
  itinanalyzerutils::setItinLegs(itin);

  for (FareMarket* fm : markets)
  {
    if (fm->travelSeg().empty())
      continue;

    if (shouldYYFMsBeIgnored && fm->governingCarrier().equalToConst("YY"))
    {
      haveYYFmsBeenIgnored = true;
      continue;
    }

    IAIbfUtils::findAllOdDataForMarketPricing(itin, fm, tripType, mode, trx,
        [&](IAIbfUtils::OdcTuple& odcTuple){ fMsForBranding[odcTuple].push_back(fm); });
  }

  AlphaCode direction = "";
  if (!fallback::fallbackSendDirectionToMM(&trx))
    direction = "OT";

  std::vector<PaxTypeCode> paxTypes = PaxTypeUtil::retrievePaxTypes(trx);
  for (const auto& item : fMsForBranding)
  {
    bRRH.buildMarketRequest(item.first.origin,
                            item.first.destination,
                            trx.travelDate(),
                            paxTypes,
                            std::vector<CarrierCode>(1, item.first.governingCarrier),
                            item.second,
                            GlobalDirection::NO_DIR,
                            direction);
  }
}

void
S8BrandService::getMarketsToFillWithBrands(const std::vector<FareMarket*>& allMarkets,
                                           std::vector<FareMarket*>& marketsToFill)
{
  marketsToFill.clear();

  boost::algorithm::copy_if(allMarkets.begin(),
                            allMarkets.end(),
                            std::back_inserter(marketsToFill),
                            hasToBeProcessed);
}

} // tse
