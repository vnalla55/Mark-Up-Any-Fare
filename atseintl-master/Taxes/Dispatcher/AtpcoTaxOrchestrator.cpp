//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/PricingTrxOps.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/Common/Types.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RequestResponse/InputRequest.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Pfc/PfcItem.h"
#include "Server/TseServer.h"
#include "Taxes/AtpcoTaxes/Common/AtpcoTaxesActivationStatus.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Request.h"
#include "Taxes/AtpcoTaxes/Factories/RequestFactory.h"
#include "Taxes/Dispatcher/AtpcoTaxesDriverV2.h"
#include "Taxes/Dispatcher/AtpcoTaxOrchestrator.h"
#include "Taxes/Dispatcher/TaxStringV2Processor.h"
#include "Taxes/LegacyFacades/AtpcoTaxDisplayDriver.h"
#include "Taxes/LegacyFacades/CalculatePfc.h"
#include "Taxes/LegacyFacades/CopyTaxResponse.h"
#include "Taxes/LegacyFacades/DaoDiagnostics.h"
#include "Taxes/LegacyFacades/ForEachTaxResponse.h"
#include "Taxes/LegacyFacades/ServicesFeesCollector.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder.h"
#include "Taxes/LegacyFacades/TaxRequestBuilder2.h"
#include "Taxes/LegacyFacades/V2TrxMappingDetails.h"
#include "Taxes/LegacyTaxes/LegacyTaxProcessor.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/PfcTaxesExemption/AutomaticPfcTaxExemption.h"
#include "Taxes/TaxInfo/TaxInfoDriver.h"

#include "Taxes/LegacyTaxes/TaxDriver.h"

#include <sstream>

namespace tse
{
FALLBACK_DECL(fallbackP98609IDRCurrency);
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(AF_CAT33_ATPCO)
FALLBACK_DECL(AF_CAT33_TaxRequestBuilder)

namespace
{
bool
gsaDiagnosticRequested(PricingTrx& trx)
{
  return trx.diagnostic().diagnosticType() == Diagnostic820 &&
      trx.diagnostic().diagParamMapItem("GS") == "A";
}

template <typename TrxType>
bool
isAncillaryTrx(TrxType& trx)
{
  return dynamic_cast<AncillaryPricingTrx*>(&trx) != nullptr;
}

const tax::AtpcoTaxesActivationStatus
readAtpcoTaxesActivationStatus(AncillaryPricingTrx& trx)
{
  tax::AtpcoTaxesActivationStatus activationStatus;
  activationStatus.setTaxOnBaggage(TrxUtil::isTOBAllowed(trx));
  activationStatus.setTaxOnOC(TrxUtil::isTOCAllowed(trx));
  activationStatus.setTaxOnChangeFee(TrxUtil::isTOEAllowed(trx));
  activationStatus.setTaxOnItinYqYrTaxOnTax(TrxUtil::isTOIAllowed(trx));

  return activationStatus;
}

const tax::AtpcoTaxesActivationStatus
readAtpcoTaxesActivationStatus(BaseExchangeTrx& trx)
{
  const DateTime& originalTicketDt = trx.originalTktIssueDT();
  tax::AtpcoTaxesActivationStatus activationStatus;
  activationStatus.setTaxOnOC(TrxUtil::isTOCAllowed(trx));
  activationStatus.setTaxOnChangeFee(TrxUtil::isTOEAllowed(trx) &&
                                     TrxUtil::isTOEAllowed(trx, originalTicketDt));
  activationStatus.setTaxOnItinYqYrTaxOnTax(TrxUtil::isTOIAllowed(trx) &&
                                            TrxUtil::isTOIAllowed(trx, originalTicketDt));
  activationStatus.setTaxOnBaggage(false);

  return activationStatus;
}

template <typename TrxType>
const tax::AtpcoTaxesActivationStatus
readAtpcoTaxesActivationStatus(TrxType& trx)
{
  tax::AtpcoTaxesActivationStatus activationStatus;
  activationStatus.setTaxOnBaggage(false);
  activationStatus.setTaxOnOC(TrxUtil::isTOCAllowed(trx));
  activationStatus.setTaxOnChangeFee(TrxUtil::isTOEAllowed(trx));
  activationStatus.setTaxOnItinYqYrTaxOnTax(TrxUtil::isTOIAllowed(trx));

  return activationStatus;
}

bool
isLegacyDisplayRequest(TaxTrx& trx)
{
  if (trx.requestType() == DISPLAY_REQUEST)
    return true;

  if (trx.requestType() == PFC_DISPLAY_REQUEST)
    return true;

  if (dynamic_cast<AncillaryPricingTrx*>(&trx)) // Ancillary Pricing
    return true;

  return false;
}

Logger
_logger("atseintl.Taxes.AtpcoTaxOrchestrator");
} // anonymous namespace

static LoadableModuleRegister<Service, AtpcoTaxOrchestrator>
_("libtaxes.so");

AtpcoTaxOrchestrator::AtpcoTaxOrchestrator(const std::string& name, TseServer& srv)
  : TaxOrchestrator(srv, name),
    _legacyTaxProcessor(new LegacyTaxProcessor(TseThreadingConst::TAX_TASK, _logger))
{
}

AtpcoTaxOrchestrator::~AtpcoTaxOrchestrator()
{
  delete _legacyTaxProcessor;
}

template <typename TrxType>
bool
AtpcoTaxOrchestrator::processTrx(TrxType& trxType)
{
  if (!Global::hasConfig())
  {
    LOG4CXX_ERROR(_logger, "Cannot access global config");
    return false;
  }

  trxType.atpcoTaxesActivationStatus() = readAtpcoTaxesActivationStatus(trxType);

  bool result = false;

  if (trxType.atpcoTaxesActivationStatus().isAllEnabled())
  {
    result = atpcoProcess(trxType);
  }
  else if (!trxType.atpcoTaxesActivationStatus().isAnyEnabled())
  {
    result = _legacyTaxProcessor->process(trxType);
  }
  else
  {
    _legacyTaxProcessor->process(trxType);
    trxType.atpcoTaxesActivationStatus().setOldTaxesCalculated(true);

    result = atpcoProcess(trxType);
  }

  copyTaxResponse(trxType);

  return result;
}

void
AtpcoTaxOrchestrator::copyTaxResponse(PricingTrx& trx)
{
  if (!fallback::AF_CAT33_ATPCO(&trx))
  {
    if (TrxUtil::isAutomatedRefundCat33Enabled(trx))
    {
      ItinSelector itinSelector(trx);
      CopyTaxResponse copyTaxResponse(trx, itinSelector);
      if (copyTaxResponse.isCopyToExcItin())
        copyTaxResponse.copyToExcItin();
    }
  }
}

class ServiceFeesYqYrAndTaxesTask : public tse::TseCallableTrxTask
{
  AtpcoTaxesDriverV2& _driver;
  tax::ServicesFeesMap& _servicesFees;
  boost::optional<std::string> _failureMessage;

public:
  ServiceFeesYqYrAndTaxesTask(PricingTrx& trx,
                              AtpcoTaxesDriverV2& driver,
                              tax::ServicesFeesMap& servicesFees)
    : _driver(driver), _servicesFees(servicesFees)
  {
    this->trx(&trx);
  }

  virtual void performTask() override try
  {
    if (!_driver.buildRequest())
    {
      _failureMessage = "atpcoProcess buildRequest";
      return;
    }

    _driver.setServices();

    if (!_driver.processTaxes())
    {
      _failureMessage = "atpcoProcess processTaxes";
      return;
    }
  }
  catch (std::logic_error& err)
  {
    _failureMessage = "atpcoProcess buildRequest - ";
    *_failureMessage += err.what();
  }

  const boost::optional<std::string>& failureMessage() const { return _failureMessage; }
};

class BuildTicketLine
{
  PricingTrx& _trx;

public:
  explicit BuildTicketLine(tse::PricingTrx& trx) : _trx(trx) {}
  void operator()(TaxResponse& taxResponse, Itin&, FarePath&) const
  {
    bool isAtpcoProcessing = true;
    TaxRecord().buildTicketLine(_trx, taxResponse, false, false, isAtpcoProcessing);
  }
};

class PrintDiagnostic877
{
  PricingTrx& _trx;

public:
  explicit PrintDiagnostic877(tse::PricingTrx& trx) : _trx(trx) {}
  void operator()(TaxResponse& taxResponse, Itin&, FarePath&) const
  {
    if (_trx.diagnostic().diagnosticType() == Diagnostic877 &&
        _trx.diagnostic().diagParamMapItem("TA") == "X")
    {
      taxResponse.diagCollector()->enable(Diagnostic877);
      *(taxResponse.diagCollector()) << taxResponse;
      taxResponse.diagCollector()->flushMsg();
    }
  }
};

class FillAutomaticPfcExemptData
{
  PricingTrx& _trx;

public:
  explicit FillAutomaticPfcExemptData(tse::PricingTrx& trx) : _trx(trx) {}
  void operator()(TaxResponse& taxResponse, Itin&, FarePath&) const
  {
    AutomaticPfcTaxExemption::analyzePfcExemtpion(_trx, taxResponse);
  }
};

bool
AtpcoTaxOrchestrator::atpcoProcess(PricingTrx& trx)
{
  bool isTaxOverride = !trx.getRequest()->taxOverride().empty();

  if (initialDiagnostic(trx))
  {
    setDefaultValidatingCarrier(trx);
    return true;
  }

  bool isATPCOTaxesActivated =
      trx.atpcoTaxesActivationStatus().isTaxOnChangeFee() ||
      trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax() ||
      (trx.atpcoTaxesActivationStatus().isTaxOnOCBaggage() && isAncillaryTrx(trx));

  if (isATPCOTaxesActivated)
  {
    initTseTaxResponses(trx);
  }

  if (trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
  {
    tax::computeServiceFees(trx);
  }

  tax::ServicesFeesMap servicesFees;
  tax::mapServiceFees(trx, servicesFees);

  AtpcoTaxesDriverV2 driver(trx, servicesFees);

  std::ostringstream gsaDiag;
  const bool logGca = gsaDiagnosticRequested(trx);
  if (logGca)
    driver.setGsaDiagnostic(gsaDiag);

  ServiceFeesYqYrAndTaxesTask othersTask(trx, driver, servicesFees);

  TaxMap::TaxFactoryMap taxFactoryMap;
  TaxMap::buildTaxFactoryMap(trx.dataHandle(), taxFactoryMap);

  if (TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx))
    tax::forEachTaxResponse(trx, FillAutomaticPfcExemptData(trx));

  tax::PfcTask pfcTask(trx);
  TseRunnableExecutor taskExecutor(TseThreadingConst::TAX_TASK);

  if (!trx.atpcoTaxesActivationStatus().isOldTaxesCalculated())
  {
    taskExecutor.execute(pfcTask);
  }

  if (!isTaxOverride)
    taskExecutor.execute(othersTask);

  taskExecutor.wait();

  setDefaultValidatingCarrier(trx);

  if (isATPCOTaxesActivated)
  {
    if (pfcTask.failureMessage())
    {
      LOG4CXX_ERROR(_logger, *pfcTask.failureMessage());
      return false;
    }
  }

  if (isTaxOverride)
  {
    if(trx.getOptions()->getCalcPfc() &&
      trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
    {
      tax::forEachTaxResponse(trx, BuildTicketLine(trx));
    }

    return true;
  }

  if (othersTask.failureMessage())
  {
    LOG4CXX_ERROR(_logger, *othersTask.failureMessage());
    return false;
  }

  if (!driver.convertResponse())
  {
    LOG4CXX_ERROR(_logger, "atpcoProcess convertResponse");
    return false;
  }

  if (logGca)
  {
    trx.diagnostic().activate();
    trx.diagnostic().insertDiagMsg(gsaDiag.str());
  }

  tax::forEachTaxResponse(trx, PrintDiagnostic877(trx));

  return true;
}

void
AtpcoTaxOrchestrator::createTaxResponse(PricingTrx& trx,
                                        tse::Itin& tseItin,
                                        tse::FarePath& tseFarePath,
                                        DCFactory& factory)
{
  if (tax::detail::findTaxResponse(&tseItin, &tseFarePath, tseItin.validatingCarrier()) != nullptr)
  {
    return;
  }

  tse::TaxResponse* taxResponse;
  trx.dataHandle().get(taxResponse);

  addTaxResponseInAdvance(*taxResponse, trx, tseItin);

  tseFarePath.itin() = &tseItin;
  taxResponse->farePath() = &tseFarePath;
  taxResponse->paxTypeCode() = tseFarePath.paxType()->paxType();
  taxResponse->diagCollector() = factory.create(trx);
  taxResponse->validatingCarrier() = tseItin.validatingCarrier();
}

void
AtpcoTaxOrchestrator::createTaxResponses(PricingTrx& trx,
                                         tse::Itin* tseItin,
                                         tse::FarePath* tseFarePath,
                                         DCFactory* factory)
{
  const bool isCat35TFSFEnabled = TrxUtil::isCat35TFSFEnabled(trx);
  createTaxResponse(trx, *tseItin, *tseFarePath, *factory);
  if (isCat35TFSFEnabled && tseFarePath->netFarePath())
    createTaxResponse(trx, *tseItin, *tseFarePath->netFarePath(), *factory);

  if (tseFarePath->adjustedSellingFarePath())
    createTaxResponse(trx, *tseItin, *tseFarePath->adjustedSellingFarePath(), *factory);
}

void
AtpcoTaxOrchestrator::initTseTaxResponses(PricingTrx& trx)
{
  DCFactory* factory = DCFactory::instance();
  const bool isValCxrApplicable = trx.isValidatingCxrGsaApplicable();
  for (tse::Itin* tseItin: trx.itin())
  {
    for (tse::FarePath* tseFarePath: tseItin->farePath())
    {
      if (!isValCxrApplicable || tseFarePath->validatingCarriers().empty())
      {
        createTaxResponses(trx, tseItin, tseFarePath, factory);
      }
      else
      {
        for (const tse::CarrierCode cxr : tseFarePath->validatingCarriers())
        {
          if (!tseFarePath->gsaClonedFarePaths().empty())
          {
            FarePath* fp = tseFarePath->findTaggedFarePath(cxr);
            if (!fp)
              fp = tseFarePath;

            fp->itin()->validatingCarrier() = cxr;
            createTaxResponses(trx, fp->itin(), fp, factory);
          }
          else
          {
            tseFarePath->itin()->validatingCarrier() = cxr;
            createTaxResponses(trx, tseFarePath->itin(), tseFarePath, factory);
          }
        }
      }
    }
  }
}

bool
AtpcoTaxOrchestrator::process(PricingTrx& trx)
{
  return processTrx(trx);
}

bool
AtpcoTaxOrchestrator::process(TaxTrx& trx)
{
  if (isLegacyDisplayRequest(trx))
    return _legacyTaxProcessor->process(trx);

  if (trx.requestType() == ATPCO_DISPLAY_REQUEST)
  {
    AtpcoTaxDisplayDriver taxDisplayDriver(trx);
    return taxDisplayDriver.buildResponse();
  }

  if (trx.requestType() == NEW_OTA_REQUEST)
  {
    const std::string& xmlRequest =
        (trx.rawRequest()[0] == '<') ? trx.rawRequest() : trx.rawRequest().substr(8);

    tax::TaxStringV2Processor processor;
    processor.setUseRepricing(false);
    processor.processString(xmlRequest, trx.response());
    return true;
  }

  if (trx.requestType() == TAX_INFO_REQUEST)
  {
    trx.atpcoTaxesActivationStatus() = readAtpcoTaxesActivationStatus(trx);

    TaxInfoDriver driver(&trx);
    driver.buildTaxInfoResponse();
    return true;
  }

  return processTrx(trx);
}

bool
AtpcoTaxOrchestrator::initialDiagnostic(PricingTrx& trx)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic831)
  {
    if (trx.diagnostic().diagParamMapItemPresent(DaoDiagnostics::PARAM_TABLE_NAME)) // TB
    {
      // another DBDiagnostic version
      DaoDiagnostics::printDBDiagnostic(trx);
      return true;
    }
  }

  return false;
}

bool
AtpcoTaxOrchestrator::process(tse::FareDisplayTrx& fareDisplayTrx)
{
  bool isAtpcoTaxesOnItinEnabled =
    fareDisplayTrx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax();

  // TODO check
  if (isAtpcoTaxesOnItinEnabled &&
      (fareDisplayTrx.diagnostic().diagnosticType() == Diagnostic830))
  {
    return requestDiagnostic(fareDisplayTrx);
  }
  else
  {
    return _legacyTaxProcessor->process(fareDisplayTrx);
  }
}

bool
AtpcoTaxOrchestrator::process(tse::AncillaryPricingTrx& trx)
{
  return processTrx(trx);
}

bool
AtpcoTaxOrchestrator::process(ExchangePricingTrx& trx)
{
  return processTrx(static_cast<BaseExchangeTrx&>(trx));
}

bool
AtpcoTaxOrchestrator::process(RexPricingTrx& trx)
{
  return processTrx(static_cast<BaseExchangeTrx&>(trx));
}

bool
AtpcoTaxOrchestrator::process(RexExchangeTrx& trx)
{
  return processTrx(static_cast<BaseExchangeTrx&>(trx));
}

bool
AtpcoTaxOrchestrator::process(RefundPricingTrx& trx)
{
  return processTrx(static_cast<BaseExchangeTrx&>(trx));
}

bool
AtpcoTaxOrchestrator::requestDiagnostic(tse::PricingTrx& trx) const
{
  tax::V2TrxMappingDetails ignoreDetails;

  tax::InputRequest* inputRequest = nullptr;

  if (!fallback::AF_CAT33_TaxRequestBuilder(&trx))
  {
    inputRequest =
        tax::TaxRequestBuilder(trx, trx.atpcoTaxesActivationStatus()).buildInputRequest(
            tax::ServicesFeesMap(), ignoreDetails, nullptr);
  }
  else
  {
    inputRequest = tax::TaxRequestBuilder_DEPRECATED().buildInputRequest(trx, tax::ServicesFeesMap(), ignoreDetails, nullptr);
  }

  if (inputRequest != nullptr)
  {
    tax::Request request;
    tax::RequestFactory().createFromInput(*inputRequest, request);

    printDiagnosticMessage(
        trx, Diagnostic830, "ATPCO TAX ACTIVATED - REQ RECEIVED", &request);

    return true;
  }

  return false;
}

void
AtpcoTaxOrchestrator::printDiagnosticMessage(Trx& trx,
                                             DiagnosticTypes diagnostic,
                                             const std::string msg,
                                             tax::Request* request) const
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(trx));

  diag.trx() = &trx;
  diag.enable(diagnostic);

  diag << "\n************************************************************\n";
  diag << boost::format("%|=60|") % str(boost::format("----%|=32|----") % msg);
  diag << "\n************************************************************\n";

  if (request)
  {
    request->print(diag);
  }

  diag.flushMsg();
}

void
AtpcoTaxOrchestrator::setDefaultValidatingCarrier(PricingTrx& trx) const
{
  if (!trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax())
    return; // only makes sense wheen taxes on itin are enabled

  for (Itin* itin : trx.itin())
  {
    for (FarePath* farePath : itin->farePath())
    {
      if (!farePath)
        continue;

      farePath->validatingCarriers().push_back(farePath->itin()->validatingCarrier());
      if (trx.isValidatingCxrGsaApplicable())
      {
        CarrierCode defVCxr, marketVcxr;
        bool retVal = false;
        if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
        {
          DefaultValidatingCarrierFinder defValCxrFinder(trx, *(farePath->itin()));
          retVal = defValCxrFinder.determineDefaultValidatingCarrier(
              farePath->validatingCarriers(), defVCxr, marketVcxr);
        }
        else
        {
          ValidatingCarrierUpdater validatingCarrier(trx);
          retVal = validatingCarrier.determineDefaultValidatingCarrier(
                *(farePath->itin()), farePath->validatingCarriers(), defVCxr, marketVcxr);
        }

        if (retVal)
        {
          farePath->defaultValidatingCarrier() = defVCxr;
          farePath->itin()->validatingCarrier() = defVCxr;

          if ((defVCxr != marketVcxr) && !marketVcxr.empty())
            farePath->marketingCxrForDefaultValCxr() = marketVcxr;
        }
        else
        {
          farePath->itin()->validatingCarrier() = CarrierCode();
        }
      }
      farePath->validatingCarriers().clear();
    }
  }
}

} // namespace tse
