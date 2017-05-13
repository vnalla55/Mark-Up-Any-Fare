// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipulated in the
//    agreement/contract under which the    program(s)
//    have been supplied.
//
// ----------------------------------------------------------------------------

#include "TicketingFee/TicketingFeesService.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/OBFeesUtils.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/AltPricingDetailObFeesTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag870Collector.h"
#include "Server/TseServer.h"
#include "TicketingFee/PseudoObjectsBuilder.h"
#include "TicketingFee/TicketingFeeCollector.h"

#include <iostream>
#include <string>

using namespace std;

namespace tse
{
FALLBACK_DECL(obFeesWpaOption1);
FALLBACK_DECL(reworkTrxAborter);

namespace
{
ConfigurableValue<int>
obFeesTimeout("TICKETING_FEES_SVC", "OB_FEES_TIMEOUT");
}

static Logger
logger("atseintl.TicketingFee.TicketingFeesService");

static LoadableModuleRegister<Service, TicketingFeesService>
_("libTicketingFee.so");

//---------------------------------------------------------------------------
// class constructor
//---------------------------------------------------------------------------
TicketingFeesService::TicketingFeesService(const std::string& name, TseServer& srv)
  : tse::Service(name, srv), _config(srv.config())
{
}

bool
TicketingFeesService::initialize(int argc, char* argv[])
{
  return true;
}

// ----------------------------------------------------------------------------
//
// bool TicketingFeesService::process
//
// Description:     Process TicketingFees performance..
//
// ----------------------------------------------------------------------------
bool
TicketingFeesService::process(MetricsTrx& trx)
{
  std::ostringstream& oss = trx.response();

  MetricsUtil::header(oss, "Ticketing Fees Metrics");
  MetricsUtil::lineItemHeader(oss);
  MetricsUtil::lineItem(oss, MetricsUtil::TICKETING_FEES_PROCESS);

  return true;
}

// ----------------------------------------------------------------------------
//
// bool TicketingFeesService::process
//
// Description:
//
// ----------------------------------------------------------------------------

bool
TicketingFeesService::process(ShoppingTrx& trx)
{
  PricingTrx& pricingTrx = trx;
  return process(pricingTrx);
}

// ----------------------------------------------------------------------------
//
// bool TicketingFeesService::process
//
// Description:
//
// ----------------------------------------------------------------------------

bool
TicketingFeesService::process(TktFeesPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entered TicketingFeesService::process()");
  setTimeOut(trx);

  createPseudoPricingSolutions(trx);
  try
  {
    if (fallback::reworkTrxAborter(&trx))
      checkTrxAborted(trx);
    else
      trx.checkTrxAborted();
  }
  catch (const ErrorResponseException& e)
  {
    if (e.code() == ErrorResponseException::OB_FEES_TIME_OUT)
    {
      std::string newMsg = "";
      newMsg += "TIMEOUT - USE OB FEES REQUEST WITH SINGLE PRICING SOLUTION";
      throw ErrorResponseException(e.code(), newMsg.c_str());
    }
  }
  //    PricingTrx& pricingTrx = trx;
  //    return process(pricingTrx);
  return processOB(trx);
  LOG4CXX_DEBUG(logger, "Leaving TicketingFeesService::process() - OB Fees done");
}

bool
TicketingFeesService::process(PricingDetailTrx& trx)
{
  if (!trx.getRequest()->isCollectOBFee())
    return true;

  LOG4CXX_DEBUG(logger, "TicketingFeesService() - Entered process(PricingDetailTrx)");
  return process(static_cast<TktFeesPricingTrx&>(trx));
}

bool
TicketingFeesService::process(AltPricingDetailObFeesTrx& trx)
{
  if (!trx.getRequest()->isCollectOBFee())
    return true;

  LOG4CXX_DEBUG(logger, "TicketingFeesService() - Entered process(AltPricingDetailObFeesTrx)");
  return process(static_cast<TktFeesPricingTrx&>(trx));
}

bool
TicketingFeesService::process(AltPricingTrx& trx)
{
  if (!fallback::obFeesWpaOption1(&trx) || !OBFeesUtils::fallbackObFeesWPA(&trx))
  {
    PricingTrx& pricingTrx = trx;
    return process(pricingTrx);
  }
  return true;
}

// ----------------------------------------------------------------------------
//
// bool TicketingFeesService::processOB
//
// Description:
//
// ----------------------------------------------------------------------------

bool
TicketingFeesService::processOB(TktFeesPricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entered TicketingFeesService::processOB()");
  TSELatencyData metrics(trx, "TICKETING FEES PROCESS");

  if (trx.getRequest()->isCollectOBFee() || trx.getRequest()->isCollectTTypeOBFee() ||
      trx.getRequest()->isCollectRTypeOBFee())
  {
    if (collectTicketingFees(trx))
      return processTicketingFeesOB(trx);
  }

  printOBNotRequested(trx);

  LOG4CXX_INFO(logger, "Finished Leaving TicketingFeesService");
  return true;
}

// ----------------------------------------------------------------------------
//
// bool TicketingFeesService::printOBNotRequested
//
// Description:
//
// ----------------------------------------------------------------------------

void
TicketingFeesService::printOBNotRequested(PricingTrx& trx)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic870)
  {
    DCFactory* factory = DCFactory::instance();
    Diag870Collector* diagPtr = dynamic_cast<Diag870Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic870);
    diagPtr->activate();
    diagPtr->printHeader();
    diagPtr->printOBFeesNotRequested();

    diagPtr->flushMsg();
  }
}

bool
TicketingFeesService::processTicketingFeesOB(TktFeesPricingTrx& trx)
{
  for (std::vector<Itin*>::iterator itinI = trx.itin().begin(), itinEnd = trx.itin().end();
       itinI != itinEnd;
       ++itinI)
  {
    if ((*itinI) == nullptr || ((*itinI)->errResponseCode() != ErrorResponseException::NO_ERROR) ||
        ((*itinI)->farePath().empty()))
      continue;

    setDataInRequest(trx, *itinI);
    std::vector<FarePath*> fps = (*itinI)->farePath();
    for (const auto fp : fps)
    {
      if (!fp || fp->forbidCreditCardFOP())
        continue;
      if (fallback::reworkTrxAborter(&trx))
        checkTrxAborted(trx);
      else
        trx.checkTrxAborted();
      TicketingFeeCollector tfc(&trx, fp);
      tfc.collect();
    }
  }
  return true;
}

void
TicketingFeesService::setDataInRequest(TktFeesPricingTrx& trx, const Itin* itin)
{
  TktFeesRequest* req = dynamic_cast<TktFeesRequest*>(trx.getRequest());
  if (!req)
    return;
  req->tktDesignator() = req->tktDesignatorPerItin()[itin];
  req->accCodeVec() = req->accountCodeIdPerItin()[itin];
  req->corpIdVec() = req->corpIdPerItin()[itin];
  req->incorrectCorpIdVec() = req->invalidCorpIdPerItin()[itin];
  req->isMultiAccCorpId() =
      !req->accCodeVec().empty() || !req->corpIdVec().empty() || !req->incorrectCorpIdVec().empty();
  TktFeesRequest::PaxTypePayment* ptp = req->paxTypePaymentPerItin()[itin];
  if (ptp != nullptr)
    req->ticketPointOverride() = ptp->tktOverridePoint();

  setFOPBinNumber(trx, itin);
}

void
TicketingFeesService::setFOPBinNumber(TktFeesPricingTrx& trx, const Itin* itin)
{
  TktFeesRequest* req = dynamic_cast<TktFeesRequest*>(trx.getRequest());
  if (!req)
    return;
  TktFeesRequest::PaxTypePayment* ptp = req->paxTypePaymentPerItin()[itin];
  if (ptp == nullptr)
  {
    LOG4CXX_ERROR(logger, "Null pointer to PTP data");
    throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED);
  }
  std::vector<TktFeesRequest::PassengerPaymentInfo*>& ppiV = ptp->ppiV();
  if (ppiV.empty())
  {
    LOG4CXX_ERROR(logger, "Empty ppiVector in PTP data");
    throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED);
  }
  TktFeesRequest::PassengerPaymentInfo* ppi = ppiV[0];
  std::vector<TktFeesRequest::FormOfPayment*>& fopV = ppi->fopVector();
  req->formOfPayment().clear();
  req->secondFormOfPayment().clear();
  if (!fopV.empty())
  {
    req->formOfPayment() = fopV[0]->fopBinNumber();
    req->paymentAmountFop() = fopV[0]->chargeAmount(); // for 870 diagnostic
    trx.getRequest()->ticketingAgent()->currencyCodeAgent() = ptp->currency();
  }
  if (fopV.size() == 2) // secondary BIN#
  {
    req->secondFormOfPayment() = fopV[1]->fopBinNumber();
    req->paymentAmountFop() = fopV[1]->chargeAmount();
  }
}
// ----------------------------------------------------------------------------
//
// void TicketingFeesService::setTimeOut
//
// Description:
//
// ----------------------------------------------------------------------------

void
TicketingFeesService::setTimeOut(TktFeesPricingTrx& trx) const
{
  TrxAborter* aborter = trx.aborter();
  if (aborter == nullptr)
    return;

  int timeToCompleteOBFees = obFeesTimeout.getValue();
  aborter->setTimeout(timeToCompleteOBFees);
  aborter->setHurry(timeToCompleteOBFees);
  aborter->setErrorCode(ErrorResponseException::OB_FEES_TIME_OUT);
  aborter->setErrorMsg("OB FEES PROCESS TIMEOUT");

  return;
}

// ----------------------------------------------------------------------------
//
// bool TicketingFeesService::createPseudoPricingSolutions
//
// Description: create pseudo FarePath for each itineraty in request
//               for Ticketing Fees service.
//
// ----------------------------------------------------------------------------
void
TicketingFeesService::createPseudoPricingSolutions(TktFeesPricingTrx& trx)
{
  PseudoObjectsBuilder builder(trx);
  builder.build();
}

// ----------------------------------------------------------------------------
//
// bool TicketingFeesService::process
//
// Description:     Main controller for all forms of Ticketing Fees services Shopping / Pricing
//
// ----------------------------------------------------------------------------

bool
TicketingFeesService::process(PricingTrx& pricingTrx)
{
  LOG4CXX_INFO(logger, "TicketingFeesService::process()");
  TSELatencyData metrics(pricingTrx, "TICKETING FEES PROCESS");

  if (pricingTrx.getRequest()->isCollectOBFee() || pricingTrx.getRequest()->isCollectTTypeOBFee() ||
      pricingTrx.getRequest()->isCollectRTypeOBFee())
  {
    if (collectTicketingFees(pricingTrx))
      return processTicketingFees(pricingTrx);
  }
  else
  {
    printOBNotRequested(pricingTrx);
  }

  LOG4CXX_INFO(logger, "Finished Leaving TicketingFeesService");

  return true;
}

// ----------------------------------------------------------------------------
//
// bool TicketingFeesService::processTicketingFee
//
// Description:     Main controller for all forms of Ticketing Fees services Shopping / Pricing
//
// ----------------------------------------------------------------------------

bool
TicketingFeesService::processTicketingFees(PricingTrx& pricingTrx)
{
  for (std::vector<Itin*>::iterator itinI = pricingTrx.itin().begin(),
                                    itinEnd = pricingTrx.itin().end();
       itinI != itinEnd;
       ++itinI)
  {
    if ((*itinI) == nullptr || ((*itinI)->errResponseCode() != ErrorResponseException::NO_ERROR) ||
        ((*itinI)->farePath().empty()))
      continue;

    std::vector<FarePath*> fps = (*itinI)->farePath();

    for (const auto fp : fps)
    {
      if (fp == nullptr)
        continue;

      if ((pricingTrx.getRequest()->owPricingRTTaxProcess()) && (fp->duplicate()))
      {
        continue;
      }

      if (fallback::reworkTrxAborter(&pricingTrx))
        checkTrxAborted(pricingTrx);
      else
        pricingTrx.checkTrxAborted();

      // Validating carrier project
      if (pricingTrx.isValidatingCxrGsaApplicable() && fp->defaultValidatingCarrier().empty())
        continue;

      if (fp->forbidCreditCardFOP())
        continue;

      TicketingFeeCollector tfc(&pricingTrx, fp);
      tfc.collect();
    }
  }
  return true;
}

bool
TicketingFeesService::collectTicketingFees(PricingTrx& pricingTrx) const
{
  if (!(pricingTrx.getTrxType() == PricingTrx::PRICING_TRX ||
        pricingTrx.getTrxType() == PricingTrx::MIP_TRX ||
        (pricingTrx.getTrxType() == PricingTrx::IS_TRX &&
         pricingTrx.startShortCutPricingItin() > 0)))
    return false;

  if (pricingTrx.excTrxType() != PricingTrx::NOT_EXC_TRX ||
      (pricingTrx.altTrxType() != PricingTrx::WP &&
       ((OBFeesUtils::fallbackObFeesWPA(&pricingTrx) || !pricingTrx.isSingleMatch())) &&
       ((fallback::obFeesWpaOption1(&pricingTrx) || pricingTrx.altTrxType() != PricingTrx::WPA))) ||
      pricingTrx.noPNRPricing())
    return false;

  if (!pricingTrx.getRequest()->ticketingAgent() ||
      pricingTrx.getRequest()->ticketingAgent()->axessUser())
    return false;

  if (!pricingTrx.getRequest()->isCollectTTypeOBFee() &&
      !pricingTrx.getRequest()->isCollectRTypeOBFee())
  {
    // entry from carrier partition
    if (pricingTrx.getRequest()->ticketingAgent()->agentTJR() == nullptr && pricingTrx.billing() &&
        !pricingTrx.billing()->partitionID().empty() && pricingTrx.billing()->aaaCity().size() < 4)
      return false;
  }

  return true;
}
}
