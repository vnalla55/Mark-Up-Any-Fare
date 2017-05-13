// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/ErrorResponseException.h"
#include "Common/FareDisplaySurcharge.h"
#include "Common/FareDisplayTax.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Global.h"
#include "Common/ItinUtil.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/RemoveFakeTravelSeg.h"
#include "Common/TSELatencyData.h"
#include "Common/Thread/ThreadPoolFactory.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TrxUtil.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/Nation.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/Trx.h"
#include "Diagnostic/Diagnostic.h"
#include "Taxes/Dispatcher/TaxOrchestrator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxDisplayDriver.h"
#include "Taxes/LegacyTaxes/TaxDriver.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/Pfc/PfcDisplayDriver.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Server/TseServer.h"

#include <string>
#include <iostream>

namespace tse
{

Logger
TaxOrchestrator::_logger("atseintl.Taxes.TaxOrchestrator");

//---------------------------------------------------------------------------
// your create(...) function is in another class - AtpcoTaxOrchestrator.cpp
//---------------------------------------------------------------------------

TaxOrchestrator::TaxOrchestrator(tse::TseServer& srv, const std::string& name)
  : tse::Service(name, srv), _config(srv.config())
{
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

bool TaxOrchestrator::initialize(int argc, char* argv[])
{
  TaxMap::setupNewUS2Itins();
  return true;
}

// ----------------------------------------------------------------------------
//
// bool TaxOrchestrator::process
//
// Description:	 Process Tax performance..
//
// ----------------------------------------------------------------------------

bool TaxOrchestrator::process(MetricsTrx& trx)
{
  std::ostringstream& oss = trx.response();

  MetricsUtil::header(oss, "Tax Metrics");
  MetricsUtil::lineItemHeader(oss);

  MetricsUtil::lineItem(oss, MetricsUtil::TAX_PROCESS);

  return true;
}

// ----------------------------------------------------------------------------
//
// bool TaxOrchestrator::process
//
// Description:	 Processing Shopping Tax Services..
//
// ----------------------------------------------------------------------------

bool TaxOrchestrator::process(ShoppingTrx& shoppingTrx)
{
  PricingTrx* trx = dynamic_cast<PricingTrx*>(&shoppingTrx);

  if (!trx)
  {
    LOG4CXX_INFO(_logger, "Dynamic Cast Shopping to Base Pricing TRX");

    return false;
  }

  return process(*trx);
}

//----------------------------------------------------------------------------
//
// bool TaxOrchestrator::process
//
// Description:	 Main controller for all forms of Tax services Shopping / Pricing
//
// ----------------------------------------------------------------------------

//---------------------------------------------------------------------------
// process(AltPricingTrx)
//---------------------------------------------------------------------------
bool TaxOrchestrator::process(AltPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(AltPricingTrx)");
  return process((PricingTrx&)trx);
}

//----------------------------------------------------------------------------
// process(NoPNRPricingTrx)
//---------------------------------------------------------------------------
bool TaxOrchestrator::process(NoPNRPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(NoPNRPricingTrx)");
  return process((PricingTrx&)trx);
}

//----------------------------------------------------------------------------
// process(ExchangePricingTrx)
//---------------------------------------------------------------------------
bool TaxOrchestrator::process(ExchangePricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(ExchangePricingTrx)");
  return process(static_cast<PricingTrx&>(trx));
}

//----------------------------------------------------------------------------
// process(RexPricingTrx)
//---------------------------------------------------------------------------
bool TaxOrchestrator::process(RexPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(RexPricingTrx)");
  if (trx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE)
    return true;

  return process((PricingTrx&)trx);
}

//----------------------------------------------------------------------------
// process(RexExchangeTrx)
//---------------------------------------------------------------------------
bool TaxOrchestrator::process(RexExchangeTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(RexExchangeTrx)");
  return process(static_cast<RexPricingTrx&>(trx));
}

//----------------------------------------------------------------------------
// process(RefundPricingTrx)
//---------------------------------------------------------------------------
bool TaxOrchestrator::process(RefundPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(RefundPricingTrx)");
  return (trx.trxPhase() != RefundPricingTrx::PRICE_NEWITIN_PHASE
            ? true
            : process(static_cast<PricingTrx&>(trx)));
}

//----------------------------------------------------------------------------
// getActiveThreads()
//---------------------------------------------------------------------------
uint32_t TaxOrchestrator::getActiveThreads()
{
  if (!ThreadPoolFactory::isMetricsEnabled())
    return 0;

  return ThreadPoolFactory::getNumberActiveThreads(TseThreadingConst::TAX_TASK);
}
} //tse
