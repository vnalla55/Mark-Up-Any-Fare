//-------------------------------------------------------------------
//
//  File:        InternalService.cpp
//  Created:     July 5, 2004
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

#include "Internal/InternalService.h"

#include "Common/Global.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/BrandingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag853Collector.h"
#include "Diagnostic/Diag854Collector.h"
#include "Server/TseServer.h"

namespace tse
{

static LoadableModuleRegister<Service, InternalService>
_("libInternalService.so");

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
InternalService::InternalService(const std::string& name, tse::TseServer& server)
  : Service(name, server), _config(server.config())
{
}

bool
InternalService::initialize(int argc, char* argv[])
{
  return true;
}

bool
InternalService::process(PricingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(RexPricingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(ExchangePricingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(AltPricingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(NoPNRPricingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(RexShoppingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(RefundPricingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(RexExchangeTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(FareDisplayTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(AncillaryPricingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(TktFeesPricingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(BrandingTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(BaggageTrx& trx)
{
  return _internalServiceController.process(trx);
}

bool
InternalService::process(StructuredRuleTrx& trx)
{
  return _internalServiceController.process(trx);
}
} //tse
