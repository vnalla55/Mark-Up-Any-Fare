/***************************************************************
 * @file     BillingService.h
 * @date     04/25/2005
 * @author   Konstantin Sidorin, Valentin Perov
 *
 * @brief    Header file Billing Service.
 *
 *  Updates:
 *
 *  Copyright Sabre 2005
 *
 *          The copyright to the computer program(s) herein
 *          is the property of Sabre.
 *          The program(s) may be used and/or copied only with
 *          the written permission of Sabre or in accordance
 *          with the terms and conditions stipulated in the
 *          agreement/contract under which the program(s)
 *          have been supplied.
 *
 ****************************************************************/

#pragma once

#include "DataModel/AltPricingTrx.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/DecodeTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/FrequentFlyerTrx.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "Server/TseServer.h"
#include "Service/Service.h"

#include <string>

#include <cstdint>
#include <unistd.h>


namespace tse
{
class ConfigMan;
class PricingTrx;

class BillingService final : public Service
{
public:
  BillingService(const std::string& serviceName, TseServer& tseServer)
    : Service(serviceName, tseServer), _config(tseServer.config())
  {
  }

  bool initialize(int argc = 0, char* argv[] = nullptr) override;

  // Add overrides of the various process() methods here
  virtual bool process(PricingTrx& trx) override { return processTrx(trx); }
  virtual bool process(AltPricingTrx& trx) override { return processTrx(trx); }
  virtual bool process(NoPNRPricingTrx& trx) override { return processTrx(trx); }
  virtual bool process(TaxTrx& trx) override { return processTrx(trx); }
  virtual bool process(FareDisplayTrx& trx) override { return processTrx(trx); }
  virtual bool process(ShoppingTrx& trx) override { return processTrx(trx); }
  virtual bool process(CurrencyTrx& trx) override { return processTrx(trx); }
  virtual bool process(MileageTrx& trx) override { return processTrx(trx); }
  virtual bool process(TicketingCxrTrx& trx) override { return processTrx(trx); }
  virtual bool process(TicketingCxrDisplayTrx& trx) override { return processTrx(trx); }
  virtual bool process(RexPricingTrx& trx) override { return processTrx(trx); }
  virtual bool process(ExchangePricingTrx& trx) override { return processTrx(trx); }
  virtual bool process(FlightFinderTrx& trx) override { return processTrx(trx); }
  virtual bool process(MultiExchangeTrx& trx) override { return processTrx(trx); }
  virtual bool process(PricingDetailTrx& trx) override { return processTrx(trx); }
  virtual bool process(RexShoppingTrx& trx) override { return processTrx(trx); }
  virtual bool process(RexExchangeTrx& trx) override { return processTrx(trx); }
  virtual bool process(RefundPricingTrx& trx) override { return processTrx(trx); }
  virtual bool process(AncillaryPricingTrx& trx) override { return processTrx(trx); }
  virtual bool process(BaggageTrx& trx) override { return processTrx(trx); }
  virtual bool process(TktFeesPricingTrx& trx) override { return processTrx(trx); }
  virtual bool process(StructuredRuleTrx& trx) override { return processTrx(trx); }

  // Those should not be processed in Billing
  virtual bool process(DecodeTrx& trx) override { return true; }
  virtual bool process(FrequentFlyerTrx& trx) override { return true; }
  virtual bool process(BrandingTrx& trx) override { return true; }
  virtual bool process(SettlementTypesTrx& trx) override { return true; }

private:
  bool send(const std::string& msg, Trx& trx);
  bool processTrx(Trx& trx);

  tse::ConfigMan& _config;
  std::string _xformName;
};
} // tse namespace
