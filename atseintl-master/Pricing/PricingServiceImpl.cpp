//----------------------------------------------------------------------------
//
//  File:           PricingServiceImpl.C
//  Description:
//  Created:	    Dec 19, 2003
//  Authors:        Dave Hobt, Steve Suggs, Mark Kasprowicz
//
//  Description:
//
//  Return types:
//
//  Copyright Sabre 2003
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Pricing/PricingServiceImpl.h"

#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag982Collector.h"
#include "Pricing/PricingOrchestrator.h"
#include "Server/TseServer.h"

#include <iostream>
#include <string>

namespace tse
{

static LoadableModuleRegister<Service, PricingServiceImpl>
_("libPricing.so");

PricingServiceImpl::PricingServiceImpl(const std::string& serviceName, TseServer& srv)
  : Service(serviceName, srv), _config(srv.config()), _po(srv)
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
bool
PricingServiceImpl::initialize(int argc, char* argv[])
{
  return true;
}

//---------------------------------------------------------------------------
// Function: process
//
// description:
//
// params:
//
// returns: true on success, false on error
//
//---------------------------------------------------------------------------
bool
PricingServiceImpl::process(MetricsTrx& trx)
{
  return _po.process(trx);
}

bool
PricingServiceImpl::process(PricingTrx& trx)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic982 &&
      (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "ITINMIP"))
  {
    DCFactory* factory = DCFactory::instance();
    Diag982Collector* diagPtr = dynamic_cast<Diag982Collector*>(factory->create(trx));

    diagPtr->enable(Diagnostic982);
    (*diagPtr) << trx;
    diagPtr->flushMsg();
  }
  return _po.process(trx);
}

bool
PricingServiceImpl::process(ShoppingTrx& trx)
{
  return _po.process(trx);
}

bool
PricingServiceImpl::process(AltPricingTrx& trx)
{
  return _po.process(trx);
}

bool
PricingServiceImpl::process(NoPNRPricingTrx& trx)
{
  return _po.process(trx);
}

bool
PricingServiceImpl::process(RexPricingTrx& trx)
{
  return _po.process(trx);
}

bool
PricingServiceImpl::process(RexShoppingTrx& trx)
{
  return _po.process(trx);
}

bool
PricingServiceImpl::process(RexExchangeTrx& trx)
{
  return _po.process(trx);
}

bool
PricingServiceImpl::process(ExchangePricingTrx& trx)
{
  return _po.process(trx);
}

bool
PricingServiceImpl::process(RefundPricingTrx& trx)
{
  return _po.process(trx);
}

uint32_t
PricingServiceImpl::getActiveThreads()
{
  return _po.getActiveThreads();
}

bool
PricingServiceImpl::process(TktFeesPricingTrx& trx)
{
  return _po.process(trx);
}

bool
PricingServiceImpl::process(StructuredRuleTrx& trx)
{
  return _po.process(trx);
}

} // tse
