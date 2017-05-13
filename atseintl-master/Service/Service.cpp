//----------------------------------------------------------------------------
//
//  File:               service.cpp
//  Description:        Generic service class to process requests from a client
//  Created:            11/16/2003
//  Authors:
//
//  Description:
//
//  Return types:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Service/Service.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/BrandingTrx.h"
#include "DataModel/CacheTrx.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/DecodeTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/FrequentFlyerTrx.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/SettlementTypesTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/Trx.h"

namespace tse
{
namespace
{
log4cxx::LoggerPtr
_logger(log4cxx::Logger::getLogger("atseintl.Service.Service"));
}

bool
Service::warnMissing(Trx& trx)
{
  LOG4CXX_ERROR(_logger,
                "Call to unimplemented Service.process(" << typeid(trx).name() << ") for Service '"
                                                         << _name << "'");
  return true;
}

bool
Service::process(CurrencyTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(DecodeTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(MetricsTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(MileageTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(FareDisplayTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(PricingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(ShoppingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(RepricingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(CacheTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(PricingDetailTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(TaxTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(AltPricingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(NoPNRPricingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(RexPricingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(ExchangePricingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(FlightFinderTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(RexShoppingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(RefundPricingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(RexExchangeTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(AncillaryPricingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(BaggageTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(TktFeesPricingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(BrandingTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(SettlementTypesTrx& trx)
{
  return warnMissing(trx);
}
bool
Service::process(TicketingCxrTrx& trx)
{
  return warnMissing(trx);
}

bool
Service::process(TicketingCxrDisplayTrx& trx)
{
  return warnMissing(trx);
}

bool
Service::process(StructuredRuleTrx& trx)
{
  return warnMissing(trx);
}

bool
Service::process(FrequentFlyerTrx& trx)
{
  return warnMissing(trx);
}

void
Service::statusLine(StatusTrx& trx, const std::string& name, bool ok)
{
  trx.response() << name;

  if (ok)
    trx.response() << " - YES" << std::endl;
  else
    trx.response() << " - NO" << std::endl;

  return;
}

bool
Service::process(StatusTrx& trx)
{
  statusLine(trx, true);
  return true;
}

uint32_t
Service::getActiveThreads()
{
  return 0;
}
}
