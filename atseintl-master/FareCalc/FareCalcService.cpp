//-------------------------------------------------------------------
//
//  File:        FareCalcService.cpp
//  Created:     May 27, 2003
//  Authors:     Mike Carroll
//
//  Description:
//
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

#include "FareCalc/FareCalcService.h"

#include "Common/Logger.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "FareCalc/AltFareCalcController.h"
#include "FareCalc/FareCalcController.h"
#include "FareCalc/NoPNRFareCalcController.h"
#include "FareCalc/PrepareFareCalcForShoppingTrx.h"
#include "Server/TseServer.h"

namespace tse
{
static Logger
logger("atseintl.FareCalc.FareCalcService");

static LoadableModuleRegister<Service, FareCalcService>
_("libFareCalc.so");

FareCalcService::FareCalcService(const std::string& name, tse::TseServer& server)
  : Service(name, server), _config(server.config())
{
}

bool
FareCalcService::initialize(int argc, char* argv[])
{
  return true;
}

bool
FareCalcService::process(PricingTrx& trx)
{
  FareCalcController fcc(trx);
  return fcc.process();
}

bool
FareCalcService::process(StructuredRuleTrx& trx)
{
  FareCalcController fcc(trx);
  return fcc.structuredRuleProcess();
}

bool
FareCalcService::process(ShoppingTrx& trx)
{
  PrepareFareCalcForShoppingTrx prepareFareCalcForShoppingTrx(trx);
  prepareFareCalcForShoppingTrx.process();
  FareCalcController fcc(trx);
  return fcc.process();
}

bool
FareCalcService::process(ExchangePricingTrx& trx)
{
  return process(static_cast<PricingTrx&>(trx));
}

bool
FareCalcService::process(AltPricingTrx& trx)
{
  if (trx.altTrxType() != AltPricingTrx::WP || trx.getRequest()->ticketingAgent()->abacusUser() ||
      trx.getRequest()->ticketingAgent()->infiniUser())
  {
    // Route all non-wp and Abacus entry to Alt processing.
    AltFareCalcController fcc(trx);
    return fcc.process();
  }
  else
  {
    FareCalcController fcc(trx);
    return fcc.process();
  }
}

bool
FareCalcService::process(NoPNRPricingTrx& trx)
{
  LOG4CXX_INFO(logger, "FareCalcService::process(NoPNRPricingTrx& trx)");
  NoPNRFareCalcController fcc(trx);
  return fcc.process();
}

bool
FareCalcService::process(RexPricingTrx& trx)
{
  if (trx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE)
    return true;
  return process(static_cast<PricingTrx&>(trx));
}

bool
FareCalcService::process(RefundPricingTrx& trx)
{
  return (trx.trxPhase() != RefundPricingTrx::PRICE_NEWITIN_PHASE
              ? true
              : process(static_cast<PricingTrx&>(trx)));
}

uint32_t
FareCalcService::getActiveThreads()
{
  return FareCalcController::getActiveThreads();
}

bool
FareCalcService::process(RexExchangeTrx& trx)
{
  return process(static_cast<RexPricingTrx&>(trx));
}
} // tse namespace
