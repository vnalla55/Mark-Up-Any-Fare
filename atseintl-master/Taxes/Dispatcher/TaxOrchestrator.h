// -------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "Common/MetricsMan.h"
#include "Service/Service.h"

namespace tse
{
class FareDisplayTrx;
class MetricsTrx;
class PricingTrx;
class AltPricingTrx;
class NoPNRPricingTrx;
class ExchangePricingTrx;
class ShoppingTrx;
class TaxTrx;
class RexPricingTrx;
class RexExchangeTrx;
class RefundPricingTrx;
class AncillaryPricingTrx;
class ConfigMan;

// ----------------------------------------------------------------------------
//
// class Tax Orchestrator
// Description:
//
// ----------------------------------------------------------------------------

class TaxOrchestrator : public Service
{

public:
  TaxOrchestrator(TseServer& srv, const std::string& name);

  virtual bool initialize(int argc, char* argv[]) override;

  virtual bool process(AncillaryPricingTrx& trx) = 0;
  virtual bool process(FareDisplayTrx& trx) = 0;
  virtual bool process(MetricsTrx& trx) override;
  virtual bool process(PricingTrx& trx) = 0;
  virtual bool process(ShoppingTrx& trx) override;
  virtual bool process(TaxTrx& trx) = 0;
  virtual bool process(AltPricingTrx& trx) override;
  virtual bool process(NoPNRPricingTrx& trx) override;
  virtual bool process(ExchangePricingTrx& trx) override;
  virtual bool process(RexPricingTrx& trx) override;
  virtual bool process(RexExchangeTrx& trx) override;
  virtual bool process(RefundPricingTrx& trx) override;

  virtual uint32_t getActiveThreads() override;

protected:
  ConfigMan& _config;
  static Logger _logger;

private:
  TaxOrchestrator(const TaxOrchestrator& rhs);
  TaxOrchestrator& operator=(const TaxOrchestrator& rhs);
};
}
