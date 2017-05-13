//----------------------------------------------------------------------------
//
// Copyright Sabre 2007
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "FreeBagService/FreeBagService.h"

#include "Common/BaggageStringFormatter.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/ErrorResponseException.h"
#include "Common/FareCalcUtil.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Thread/ThreadPoolFactory.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Agent.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "Diagnostic/Diag852Collector.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcController.h"
#include "FreeBagService/AllowanceDataStrategy.h"
#include "FreeBagService/AncillaryChargesDataStrategy.h"
#include "FreeBagService/AncillaryResultProcessor.h"
#include "FreeBagService/AncillaryTrxScopeDateSetter.h"
#include "FreeBagService/BaggageDataProcessor.h"
#include "FreeBagService/BaggageItinAnalyzer.h"
#include "FreeBagService/BaggageItinAnalyzerResponseBuilder.h"
#include "FreeBagService/BaggageResultProcessor.h"
#include "FreeBagService/BaggageTicketingDateScope.h"
#include "FreeBagService/CarryOnAllowanceDataStrategy.h"
#include "FreeBagService/CarryOnBaggageItinAnalyzer.h"
#include "FreeBagService/CarryOnBaggageResultProcessor.h"
#include "FreeBagService/CarryOnChargesDataStrategy.h"
#include "FreeBagService/ChargesDataStrategy.h"
#include "FreeBagService/EmbargoesDataStrategy.h"
#include "FreeBagService/EmbargoesResultProcessor.h"
#include "ServiceFees/AncillaryPriceModifierProcessor.h"
#include "ServiceFees/PseudoFarePathBuilder.h"
#include "Util/ScopedSetter.h"

#include <mutex>

namespace tse
{

static Logger
logger("atseintl.FreeBagService.FreeBagService");

static LoadableModuleRegister<Service, FreeBagService>
_("libFreeBagService.so");

//----------------------------------------------------------------------------
// initialize()
//---------------------------------------------------------------------------
bool
FreeBagService::initialize(int argc, char* argv[])
{
  LOG4CXX_INFO(logger, "Entering FreeBagService::initialize");

  LOG4CXX_INFO(logger, "Leaving FreeBagService::initialize");
  return true;
}

bool
FreeBagService::process(PricingTrx& trx)
{
  LOG4CXX_INFO(logger, "Entering FreeBagService::process()");

  const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);

  if (!fcConfig)
    return false;

  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    TSELatencyData metrics(trx, "FREE BAGGAGE FOR MIP PROCESS");
    processIata302BaggageMip(trx);
  }
  else if (fcConfig->itinHeaderTextInd() == FareCalcController::DISPLAY_BAGGAGE_ALLOWANCE ||
            trx.getOptions()->freeBaggage())
  {
    TSELatencyData metrics(trx, "FARE CALC IATA 302 BAGGAGE PROCESS");
    LOG4CXX_DEBUG(logger, "Using IATA 302 baggage service");

    if (trx.getRequest()->multiTicketActive())
    {
      for (Itin* itin : trx.itin())
        processIata302Baggage(trx, itin);
    }
    else
    {
      processIata302Baggage(trx);
    }
  }

  LOG4CXX_INFO(logger, "Leaving FreeBagService::process");
  return true;
}

bool
FreeBagService::process(AltPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::process(AltPricingTrx)");

  PricingTrx& pricingTrx = (PricingTrx&)trx;

  bool res = process(pricingTrx);

  LOG4CXX_INFO(logger, "Leaving FreeBagService::process(AltPricingTrx)");

  return res;
}

bool
FreeBagService::process(ExchangePricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::process(ExchangePricingTrx)");

  PricingTrx& pricingTrx = (PricingTrx&)trx;

  bool res = process(pricingTrx);

  LOG4CXX_INFO(logger, "Leaving FreeBagService::process(ExchangePricingTrx)");

  return res;
}

bool
FreeBagService::process(NoPNRPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::process(NoPNRPricingTrx)");

  PricingTrx& pricingTrx = (PricingTrx&)trx;

  bool res = process(pricingTrx);

  LOG4CXX_INFO(logger, "Leaving FreeBagService::process(NoPNRPricingTrx)");

  return res;
}

bool
FreeBagService::process(RexPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::process(RexPricingTrx)");

  PricingTrx& pricingTrx = (PricingTrx&)trx;

  bool res = process(pricingTrx);

  LOG4CXX_INFO(logger, "Leaving FreeBagService::process(RexPricingTrx)");

  return res;
}

bool
FreeBagService::process(RexExchangeTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::process(RexExchangeTrx)");

  PricingTrx& pricingTrx = (PricingTrx&)trx;

  bool res = process(pricingTrx);

  LOG4CXX_INFO(logger, "Leaving FreeBagService::process(RexExchangeTrx)");

  return res;
}

bool
FreeBagService::process(BaggageTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::process(BaggageTrx)");

  PseudoFarePathBuilder builder(trx);
  builder.build();

  {
    TSELatencyData metrics(trx, "FREE BAGGAGE FOR AA PROCESS");
    processIata302Baggage(trx);
  }

  LOG4CXX_INFO(logger, "Leaving FreeBagService::process(BaggageTrx)");
  return true;
}

bool
FreeBagService::process(AncillaryPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::process(AncillaryPricingTrx)");

  if (!checkFarePath(trx))
  {
    PseudoFarePathBuilder builder(trx);
    builder.build();
  }

  {
    TSELatencyData metrics(trx, "FREE BAGGAGE FOR SWS PROCESS");
    adjustTimeOut(trx);
    processIata302BaggageWebService(trx);
  }

  LOG4CXX_INFO(logger, "Leaving FreeBagService::process(AncillaryPricingTrx)");
  return true;
}

static std::mutex
itinFpMutex;

bool
FreeBagService::processBagInPqBruteForce(PricingTrx& trx, FarePath& fp)
{
  Itin* itin = fp.itin();
  TSE_ASSERT(itin);

  // Process bagagge rules - this part should be protected by mutex
  // as itin can be shared between multiple threads.
  {
    std::lock_guard<std::mutex> lock(itinFpMutex);
    ScopedSetter<std::vector<FarePath*>> fpathSetter(itin->farePath(), {&fp});
    BaggageItinAnalyzer itinAnalyzer(trx, *itin);
    itinAnalyzer.analyzeAndSaveIntoFarePaths();

    BaggageDataProcessor dataProcessor(trx,
                                       itinAnalyzer.farePathIndex2baggageTravels(),
                                       itinAnalyzer.furthestCheckedPoint(),
                                       itin->getBaggageTripType());
    dataProcessor.process(AllowanceDataStrategy(trx));
    dataProcessor.process(ChargesDataStrategy(trx));
  }

  // Calc total fee
  MoneyAmount totalCharge = 0.0;
  CurrencyCode chargeCur;

  for (const BaggageTravel* bt : fp.baggageTravels())
  {
    for (const BaggageCharge* bc : bt->_charges)
    {
      if (!bc)
        continue;
      TSE_ASSERT(chargeCur.empty() || chargeCur == bc->feeCurrency());
      totalCharge += bc->feeAmount();
      chargeCur = bc->feeCurrency();
    }
  }

  // Convert to calculation currency
  const CurrencyCode calcCurrency =
      itin->calculationCurrency() == NUC ? USD : itin->calculationCurrency();
  const Money totalNucCharge =
      ServiceFeeUtil(trx).convertBaggageFeeCurrency(totalCharge, chargeCur, calcCurrency);
  fp.setBagChargeNUCAmount(totalNucCharge.value());
  fp.baggageTravels().clear();
  return true;
}

bool
FreeBagService::checkFarePath(const PricingTrx& trx) const
{
  for (Itin* itin : trx.itin())
  {
    if (!itin->farePath().empty())
      return true;
  }
  return false;
}

uint32_t
FreeBagService::getActiveThreads()
{
  if (!ThreadPoolFactory::isMetricsEnabled())
    return 0;

  return ThreadPoolFactory::getNumberActiveThreads(TseThreadingConst::BAGGAGE_TASK);
}

//----------------------------------------------------------------------------
// processIata302Baggage()
//---------------------------------------------------------------------------
void
FreeBagService::processIata302Baggage(PricingTrx& trx, Itin* itn /* = 0 */)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::processIata302Baggage(PricingTrx&, Itin*)\"");
  Itin* itin = (itn)? itn : trx.itin().front();

  if (itin->farePath().empty())
  {
    LOG4CXX_DEBUG(logger, "No FarePath for Itin. FreeBagService cannot continue");
    return;
  }

  BaggageTicketingDateScope dtSetter(trx, itin);
  BaggageItinAnalyzer itinAnalyzer(trx, *itin);

  itinAnalyzer.analyzeAndSaveIntoFarePaths();
  itinAnalyzer.displayDiagnostic();

  const BaggageTripType btt = itin->getBaggageTripType();
  const bool isUsDot = btt.isUsDot();

  BaggageDataProcessor dataProcessor(trx,
                                     itinAnalyzer.farePathIndex2baggageTravels(),
                                     itinAnalyzer.furthestCheckedPoint(),
                                     btt);
  dataProcessor.process(AllowanceDataStrategy(trx));

  BaggageResultProcessor resultProcessor(trx);
  resultProcessor.buildAllowanceText(itinAnalyzer.baggageTravels(), isUsDot);

  const bool disclosureProcessingAllowed =
      trx.isNotExchangeTrx() || TrxUtil::isBaggage302ExchangeActivated(trx);

  if (!trx.noPNRPricing() &&
      ((TrxUtil::isBaggage302GlobalDisclosureActivated(trx) && disclosureProcessingAllowed) ||
       isUsDot) &&
      TrxUtil::isBaggage302DotActivated(trx))
  {
    dataProcessor.process(ChargesDataStrategy(trx));
    resultProcessor.buildBaggageResponse(itin->farePath());

    CarryOnBaggageItinAnalyzer carryOnItinAnalyzer(trx, *itin);
    const bool processCarryOn =
        disclosureProcessingAllowed && TrxUtil::isBaggage302CarryOnActivated(trx);
    const bool processEmbargo =
        disclosureProcessingAllowed && TrxUtil::isBaggage302EmbargoesActivated(trx);

    if (processCarryOn || processEmbargo)
    {
      carryOnItinAnalyzer.analyze();
      carryOnItinAnalyzer.displayDiagnostic(processCarryOn, processEmbargo);
    }

    if (processCarryOn)
    {
      resultProcessor.processFeesAtEachCheck();
      processCarryOnBaggage(trx, carryOnItinAnalyzer, itinAnalyzer.furthestCheckedPoint(), btt, itin);
    }
    resultProcessor.processAdditionalAllowances();

    if (!processCarryOn)
      resultProcessor.processFeesAtEachCheck();

    if (processEmbargo)
      processEmbargoes(trx, carryOnItinAnalyzer, itinAnalyzer.furthestCheckedPoint(), btt, itin);
  }
  LOG4CXX_DEBUG(logger, "Leaving FreeBagService::processIata302Baggage(PricingTrx&, Itin*)\"");
}

void
FreeBagService::processIata302BaggageMip(PricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::processIata302BaggageMip(PricingTrx&)\"");
  bool enoughKnownChargesForAnyItin = false;

  DiagManager diag852Manager(trx, Diagnostic852);
  for (Itin* itin : trx.itin())
  {
    if (itin->farePath().empty())
    {
      LOG4CXX_DEBUG(logger, "No FarePath for Itin. Itin skipped");
      continue;
    }

    BaggageTicketingDateScope dtSetter(trx, itin);
    BaggageItinAnalyzer itinAnalyzer(trx, *itin);

    itinAnalyzer.analyzeAndSaveIntoFarePaths();
    itinAnalyzer.displayDiagnostic();

    BaggageDataProcessor dataProcessor(trx,
                                       itinAnalyzer.farePathIndex2baggageTravels(),
                                       itinAnalyzer.furthestCheckedPoint(),
                                       itin->getBaggageTripType());

    dataProcessor.process(AllowanceDataStrategy(trx));

    if (TrxUtil::isBaggageChargesInMipActivated(trx))
      dataProcessor.process(ChargesDataStrategy(trx));

    if (TrxUtil::isBaggageInPQEnabled(trx))
      enoughKnownChargesForAnyItin = FreeBaggageUtil::isItinHasEnoughKnownCharges(trx, *itin) ||
                                     enoughKnownChargesForAnyItin;
  }

  if (TrxUtil::isBaggageInPQEnabled(trx))
  {
    if (diag852Manager.isActive())
    {
      Diag852Collector* diag852Collector =
          dynamic_cast<Diag852Collector*>(&diag852Manager.collector());
      if (diag852Collector)
        diag852Collector->printInfoAboutUnknownBaggageCharges(trx);
    }
    else
    {
      if (!enoughKnownChargesForAnyItin)
        throw ErrorResponseException(ErrorResponseException::UNKNOWN_BAGGAGE_CHARGES);
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving FreeBagService::processIata302BaggageMip(PricingTrx&)\"");
}

void
FreeBagService::populateEmdInfoMap(AncillaryPricingTrx& trx, EmdInterlineAgreementInfoMap& emdMap)  const
{
  CarrierCode emdValidatingCarrier;
  if (trx.billing() && !trx.billing()->partitionID().empty())
    emdValidatingCarrier = trx.billing()->partitionID();

  NationCode nation;
  const Loc* pointOfSaleLocation = TrxUtil::saleLoc(trx);
  if(pointOfSaleLocation)
    nation = pointOfSaleLocation->nation();

  CrsCode gds;
  if (trx.getRequest() && trx.getRequest()->ticketingAgent())
    gds = trx.getRequest()->ticketingAgent()->cxrCode();

  if (!EmdInterlineAgreementInfoMapBuilder::populateRecords(nation, gds, emdValidatingCarrier, trx.dataHandle(), emdMap))
  {
    LOG4CXX_DEBUG(logger, "No Emd data! Unable to validate Emd Agreement");
  }
}


void
FreeBagService::processIata302BaggageWebService(AncillaryPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entering FreeBagService::processIata302BaggageWebService(AncillaryPricingTrx&)\"");
  AncillaryTrxScopeDateSetter ticketingDateSetter(trx);

  EmdInterlineAgreementInfoMap emdInfoMap;
  if (trx.activationFlags().isEmdForCharges())
    populateEmdInfoMap(trx, emdInfoMap);

  for (std::vector<Itin*>::iterator it = trx.itin().begin(); it != trx.itin().end(); ++it)
  {
    Itin * itin = *it;
    bool dispDiagInItinAnalyzer = (it == trx.itin().begin());
    if (itin->farePath().empty())
    {
      LOG4CXX_DEBUG(logger, "No FarePath for Itin. FreeBagService cannot continue");
      return;
    }

    ticketingDateSetter.updateDate(itin);

    BaggageItinAnalyzer itinAnalyzer(trx, *itin);

    itinAnalyzer.analyzeAndSaveIntoFarePaths();

    if (dispDiagInItinAnalyzer)
      itinAnalyzer.displayDiagnostic();

    trx.baggageTravels()[itin] = itinAnalyzer.baggageTravels();

    BaggageDataProcessor dataProcessor(trx,
                                       itinAnalyzer.farePathIndex2baggageTravels(),
                                       itinAnalyzer.furthestCheckedPoint(),
                                       itin->getBaggageTripType());

    AncRequest* ancReq = static_cast<AncRequest*>(trx.getRequest());
    ancReq->setActiveAgent(AncRequest::TicketingAgent, itin->ticketNumber());
    dataProcessor.process(AllowanceDataStrategy(trx));
    ancReq->setActiveAgent(AncRequest::CheckInAgent);

    dataProcessor.process(AncillaryChargesDataStrategy(trx, emdInfoMap));

    bool processCarryOn = trx.getOptions()->isServiceTypeRequested('B');
    bool processEmbargo  = trx.getOptions()->isServiceTypeRequested('E');

    if(trx.activationFlags().isAB240() && (processCarryOn || processEmbargo))
    {
      CarryOnBaggageItinAnalyzer carryOnItinAnalyzer(trx, *itin);
      carryOnItinAnalyzer.analyze();

      if (dispDiagInItinAnalyzer)
        carryOnItinAnalyzer.displayDiagnostic(processCarryOn, processEmbargo);

      BaggageResultProcessor resultProcessor(trx);
      BaggageTripType btt = itin->getBaggageTripType();

      resultProcessor.buildBaggageResponse(itin->farePath());

      if (processCarryOn)
      {
        resultProcessor.processFeesAtEachCheck();
        processCarryOnBaggage(trx, carryOnItinAnalyzer, itinAnalyzer.furthestCheckedPoint(), btt, itin, emdInfoMap);
      }
      resultProcessor.processAdditionalAllowances();

      if (processEmbargo)
        processEmbargoes(trx, carryOnItinAnalyzer, itinAnalyzer.furthestCheckedPoint(), btt, itin);
    }
    if (ancReq->isWPBGRequest() && ancReq->wpbgDisplayItinAnalysis())
    {
      BaggageItinAnalyzerResponseBuilder responseBuilder(itinAnalyzer);
      responseBuilder.printItinAnalysis(trx.response(), *itin);
    }
    else
    {
      AncillaryResultProcessor ancillaryResultProcessor(trx);
      ancillaryResultProcessor.processAllowanceAndCharges(itinAnalyzer.baggageTravels());
      if(!trx.activationFlags().isAB240())
        continue;
      for (std::vector<FarePath*>::const_iterator it = itin->farePath().begin(); it != itin->farePath().end(); ++it)
      {
        const FarePath& farePath = **it;
        ancillaryResultProcessor.processEmbargoes(farePath.baggageTravelsPerSector());
        ancillaryResultProcessor.processCarryOn(farePath.baggageTravelsPerSector());
      }
    }

    if(trx.activationFlags().isMonetaryDiscount())
    {
      OCFees::BaggageAmountRounder baggageAmountRounder(trx);
      AncillaryPriceModifierProcessor ancPriceModifierProcessor(trx, *itin, baggageAmountRounder);
      ancPriceModifierProcessor.processGroups(itin->ocFeesGroupsFreeBag());
    }
  }
  LOG4CXX_DEBUG(logger, "Leaving FreeBagService::processIata302BaggageWebService(AncillaryPricingTrx&)\"");
}

void
FreeBagService::adjustTimeOut(AncillaryPricingTrx& trx) const
{
  TrxAborter* aborter = trx.aborter();
  if (!aborter)
    return;

  int baggageTimeout(0);
  if (!Global::config().getValue("BAGGAGE_SWS_TIMEOUT", baggageTimeout, "FREE_BAGGAGE"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(logger, "BAGGAGE_SWS_TIMEOUT", "FREE_BAGGAGE");
  }

  aborter->setTimeout(baggageTimeout);
  aborter->setHurry(baggageTimeout);
  aborter->setErrorCode(ErrorResponseException::BAGGAGE_SWS_TIME_OUT);
  aborter->setErrorMsg("PROCESS TIMEOUT");
}

void
FreeBagService::processCarryOnBaggage(PricingTrx& trx,
                                      const BaggageItinAnalyzer& analyzer,
                                      const CheckedPoint& furthestCheckedPoint,
                                      BaggageTripType btt,
                                      const Itin* itin)
{
  BaggageDataProcessor dataProcessor(trx, analyzer.farePathIndex2baggageTravels(), furthestCheckedPoint, btt);

  dataProcessor.process(CarryOnAllowanceDataStrategy(trx));
  dataProcessor.process(CarryOnChargesDataStrategy(trx));
  processCarryOnDisclosure(trx, btt, itin);
}


void
FreeBagService::processCarryOnBaggage(AncillaryPricingTrx& trx,
                                      const BaggageItinAnalyzer& analyzer,
                                      const CheckedPoint& furthestCheckedPoint,
                                      BaggageTripType btt,
                                      const Itin* itin,
                                      const EmdInterlineAgreementInfoMap& emdInfoMap)
{
  BaggageDataProcessor dataProcessor(trx, analyzer.farePathIndex2baggageTravels(), furthestCheckedPoint, btt);

  AncRequest* ancReq = static_cast<AncRequest*>(trx.getRequest());
  ancReq->setActiveAgent(AncRequest::TicketingAgent, itin->ticketNumber());
  dataProcessor.process(CarryOnAllowanceDataStrategy(trx));
  ancReq->setActiveAgent(AncRequest::CheckInAgent);

  dataProcessor.process(CarryOnChargesDataStrategy(trx, emdInfoMap));
  processCarryOnDisclosure(trx, btt, itin);
}


void
FreeBagService::processCarryOnDisclosure(PricingTrx& trx,
                           BaggageTripType btt,
                           const Itin* itin)
{
  if (TrxUtil::isBaggage302GlobalDisclosureActivated(trx) || btt.isUsDot())
  {
    CarryOnBaggageResultProcessor resultProcessor(trx);
    resultProcessor.processAllowance(itin->farePath());
    resultProcessor.processCharges(itin->farePath());
  }
}

void
FreeBagService::processEmbargoes(PricingTrx& trx,
                                 const BaggageItinAnalyzer& analyzer,
                                 const CheckedPoint& furthestCheckedPoint,
                                 BaggageTripType btt,
                                 const Itin* itin)
{
  BaggageDataProcessor dataProcessor(
      trx, analyzer.farePathIndex2baggageTravels(), furthestCheckedPoint, btt);

  dataProcessor.process(EmbargoesDataStrategy(trx));
  if (TrxUtil::isBaggage302GlobalDisclosureActivated(trx) || btt.isUsDot())
  {
    EmbargoesResultProcessor embargoesResultProcessor(trx);
    embargoesResultProcessor.process(itin->farePath());
  }
}

} // tse
