// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
//
//	    The copyright to the computer program(s) herein
//	    is the property of Sabre.
//	    The program(s) may be used and/or copied only with
//	    the written permission of Sabre or in accordance
//	    with the terms and conditions stipulated in the
//	    agreement/contract under which the	program(s)
//	    have been supplied.
//
// ----------------------------------------------------------------------------

#include "ServiceFees/ServiceFeesService.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/Agent.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag878Collector.h"
#include "Diagnostic/Diag879Collector.h"
#include "Server/TseServer.h"
#include "ServiceFees/AncillaryPostTktFeeCollector.h"
#include "ServiceFees/AncillaryPriceModifierProcessor.h"
#include "ServiceFees/AncillaryPricingFeeCollector.h"
#include "ServiceFees/AncillaryWpDisplayFeeCollector.h"
#include "ServiceFees/OptionalFeeCollector.h"

#include <iostream>
#include <string>

using namespace std;

namespace tse
{
static Logger
logger("atseintl.ServiceFees.ServiceFeeService");

static LoadableModuleRegister<Service, ServiceFeesService>
_("libServiceFees.so");

namespace
{
ConfigurableValue<int>
ancillaryTimeout("SERVICE_FEES_SVC", "ANCILLARY_TIMEOUT", 0);
ConfigurableValue<int>
timeToCompleteTransShp("SERVICE_FEES_SVC", "TIME_TO_COMPLETE_TRANS_SHP", 5);
ConfigurableValue<int>
timeToCompleteTransaction("SERVICE_FEES_SVC", "TIME_TO_COMPLETE_TRANS", 1);
}

ServiceFeesService::ServiceFeesService(const std::string& name, TseServer& srv)
  : Service(name, srv), _config(srv.config())
{
}

// ----------------------------------------------------------------------------
// Override for external service
// ----------------------------------------------------------------------------
void
ServiceFeesService::processOCFee(OptionalFeeCollector& ocCollector) const
{
  ocCollector.collect();
}

// ----------------------------------------------------------------------------
// Override for external service
// ----------------------------------------------------------------------------
void
ServiceFeesService::processAncillaryFee(OptionalFeeCollector& ancCollector) const
{
  ancCollector.process();
}

//---------------------------------------------------------------------------
// Function: initialize
//
// description:
//
// params:
//
// returns: true on success, false on error
//
//---------------------------------------------------------------------------

bool
ServiceFeesService::initialize(int argc, char* argv[])
{
  return true;
}

// ----------------------------------------------------------------------------
//
// bool ServiceFeesService::process
//
// Description:	 Process ServiceFees performance..
//
// ----------------------------------------------------------------------------

bool
ServiceFeesService::process(MetricsTrx& trx)
{
  std::ostringstream& oss = trx.response();

  MetricsUtil::header(oss, "Service Fees Metrics");
  MetricsUtil::lineItemHeader(oss);

  MetricsUtil::lineItem(oss, MetricsUtil::SERVICE_FEES_PROCESS);

  return true;
}

// ----------------------------------------------------------------------------
//
// bool ServiceFeesService::process
//
// Description:  Exchange is out of scope
//
// ----------------------------------------------------------------------------

bool
ServiceFeesService::process(ExchangePricingTrx& trx)
{
  LOG4CXX_INFO(logger, "ExchangePricingTrx out of scope");

  return true;
}

// ----------------------------------------------------------------------------
//
// bool ServiceFeesService::process
//
// Description:	 Main controller for all forms of Service Fees services Shopping / Pricing
//
// ----------------------------------------------------------------------------

bool
ServiceFeesService::process(PricingTrx& trx)
{
  LOG4CXX_INFO(logger, "Started Process()");

  TSELatencyData metrics(trx, "SERVICE FEE PROCESS");

  bool collectOcFees = true;
  bool skipFeesInfoInResponse = false;
  std::string skipReason = "";

  if (trx.getRequest()->isCollectOCFees())
  {
    if (trx.isAltDates())
    {
      collectOcFees = false;
      skipFeesInfoInResponse = true;
      skipReason = "ALTERNATE DATES REQUEST";
    }
    else if (!collectOCFee(trx))
    {
      collectOcFees = false;
      skipReason = "OTHER";
    }

    if (collectOcFees)
    {
      if (!checkTimeOut(trx))
        return true;
      OptionalFeeCollector ofc(trx);
      processOCFee(ofc);
    }
    else
    {
      if (skipFeesInfoInResponse)
      {
        trx.getRequest()->collectOCFees() = 'F';
      }
    }
  }
  else
  {
    collectOcFees = false;
    skipReason = "FEES NOT REQUESTED";
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic878)
  {
    DCFactory* factory = DCFactory::instance();
    Diag878Collector* diagPtr = dynamic_cast<Diag878Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic878);
    diagPtr->activate();

    if (collectOcFees)
    {
      (*diagPtr) << trx;
    }
    else
    {
      diagPtr->printSkipOcCollectionReason(skipReason);
    }

    diagPtr->flushMsg();
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic879)
  {
    DCFactory* factory = DCFactory::instance();
    Diag879Collector* diagPtr = dynamic_cast<Diag879Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic879);
    diagPtr->activate();

    if (collectOcFees)
    {
      (*diagPtr) << trx;
    }

    diagPtr->flushMsg();
  }

  LOG4CXX_INFO(logger, "Finished Leaving ServiceFees");

  return true;
}

// ----------------------------------------------------------------------------
//
// bool ServiceFeesService::process
//
// Description: Controller for AncillaryPricing forms of Service Fees services
//
// ----------------------------------------------------------------------------

bool
ServiceFeesService::process(AncillaryPricingTrx& trx)
{
  LOG4CXX_INFO(logger, "Started Process(AncillaryPricingTrx)");

  TSELatencyData metrics(trx, "SERVICE FEE PROCESS");

  adjustTimeOut(trx);

  if (trx.activationFlags().isAB240())
  {
    // AncillaryPricingRequest v3 uses WP*AE path for SERVICE FEE procesing
    AncillaryWpDisplayFeeCollector anfc(trx);
    processAncillaryFee(anfc);

    if (trx.activationFlags().isMonetaryDiscount())
      applyPriceModification(trx);
  }
  else
  {
    switch (static_cast<AncRequest*>(trx.getRequest())->ancRequestType())
    {
    case AncRequest::WPAERequest: // WP*AE request
    {
      AncillaryWpDisplayFeeCollector anfc(trx);
      processAncillaryFee(anfc);
      break;
    }
    case AncRequest::PostTktRequest:
    {
      AncillaryPostTktFeeCollector anfc(trx);
      processAncillaryFee(anfc);
      break;
    }
    default: // M70 request
    {
      AncillaryPricingFeeCollector anfc(trx);
      processAncillaryFee(anfc);
      break;
    }
    }
  }
  LOG4CXX_INFO(logger, "Finished Leaving ServiceFees");

  return true;
}

void
ServiceFeesService::adjustTimeOut(AncillaryPricingTrx& trx) const
{
  TrxAborter* aborter = trx.aborter();
  if (aborter == nullptr)
  {
    return;
  }
  int timeToCompleteAncillary = ancillaryTimeout.getValue();

  aborter->setTimeout(timeToCompleteAncillary);
  aborter->setHurry(timeToCompleteAncillary);
  AncRequest* req = dynamic_cast<AncRequest*>(trx.getRequest());
  if (req->ancRequestType() == AncRequest::M70Request)
  {
    aborter->setErrorCode(ErrorResponseException::ANCILLARY_TIME_OUT_M70);
    aborter->setErrorMsg("ANCILLARY PROCESS TIMEOUT");
  }
  else
    aborter->setErrorCode(ErrorResponseException::ANCILLARY_TIME_OUT_R7);

  return;
}

bool
ServiceFeesService::collectOCFee(PricingTrx& pricingTrx) const
{
  if (!(pricingTrx.getTrxType() == PricingTrx::PRICING_TRX ||
        pricingTrx.getTrxType() == PricingTrx::MIP_TRX))
    return false;

  if (pricingTrx.excTrxType() != PricingTrx::NOT_EXC_TRX ||
      pricingTrx.altTrxType() != PricingTrx::WP || pricingTrx.noPNRPricing())
    return false;

  // Ticketing entry
  if (pricingTrx.getRequest()->isTicketEntry())
    return false;

  // entry from carrier partition
  if (pricingTrx.getRequest()->ticketingAgent()->agentTJR() == nullptr && pricingTrx.billing() &&
      !pricingTrx.billing()->partitionID().empty() && pricingTrx.billing()->aaaCity().size() < 4)
    return false;

  // FareX entry
  if (pricingTrx.getOptions()->fareX())
    return false;

  return true;
}

bool
ServiceFeesService::checkTimeOut(PricingTrx& trx) const
{
  if ((trx.getTrxType() != PricingTrx::PRICING_TRX) && (trx.getTrxType() != PricingTrx::MIP_TRX))
    return true;

  TrxAborter* aborter = trx.aborter();
  if (aborter == nullptr)
  {
    return true;
  }
  bool diagRequested = diagActive(trx);
  bool wpae = trx.getOptions() && (trx.getOptions()->isProcessAllGroups() || // WPAE, SET is set
                                   !trx.getOptions()->serviceGroupsVec().empty()); // WPAE-XX

  int timeToCompleteTrans = 1;
  if (!diagRequested)
  {
    if (trx.getTrxType() == PricingTrx::MIP_TRX) // shopping
    {
      // For shopping treat this as a percentage of transaction timeout value (D70)
      timeToCompleteTrans =
          static_cast<int>(ceil((aborter->timeout() * timeToCompleteTransShp.getValue()) / 100));
    }
    else
    {
      timeToCompleteTrans = timeToCompleteTransaction.getValue();
    }
  }

  int timeLeft = aborter->getTimeOutAt() - time(nullptr); // time left for OC+Tax+FC

  timeLeft = timeLeft - timeToCompleteTrans; // time left for OC only
  if (timeLeft <= 0)
  {
    if (wpae)
    {
      std::vector<Itin*>::const_iterator itinI = trx.itin().begin();
      for (; itinI != trx.itin().end(); ++itinI)
      {
        Itin& itin = **itinI;
        itin.timeOutForExceeded() = true;
      }
    }
    return false;
  }

  if (wpae || diagRequested) // WPAE or WPAE-XX or diag=875,877,880
  {
    aborter->setHurry(timeLeft);
    return true;
  }

  // WP entry
  int percentTime = ((aborter->timeout() - timeLeft) * 10) / 100;
  if (percentTime == 0)
    percentTime = 1;
  int hurryAt = percentTime < timeLeft ? percentTime : timeLeft;
  aborter->setHurry(hurryAt);
  return true;
}

bool
ServiceFeesService::diagActive(PricingTrx& trx) const
{
  if (!trx.diagnostic().isActive())
    return false;

  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  if (diagType == Diagnostic875 || diagType == Diagnostic876 || diagType == Diagnostic877 ||
      diagType == Diagnostic880)
  {
    DiagCollector* diag = DCFactory::instance()->create(trx);

    if (diag != nullptr)
      return true;
  }
  return false;
}

void
ServiceFeesService::applyPriceModification(AncillaryPricingTrx& trx)
{
  OCFees::OcAmountRounder ocAmountRounder(trx);

  for (auto itin : trx.itin())
  {
    AncillaryPriceModifierProcessor ancPriceModifierProcessor(trx, *itin, ocAmountRounder);
    ancPriceModifierProcessor.processGroups(itin->ocFeesGroup());
  }
}
}
