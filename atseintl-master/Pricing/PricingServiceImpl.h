//----------------------------------------------------------------------------
//
//  File:  PricingServiceImpl.h
//  Description:	See matching .C file
//  Created:		Dec 17, 2003
//  Authors:  Dave Hobt, Steve Suggs, Mark Kasprowicz
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

#pragma once

#include "Pricing/PricingOrchestrator.h"
#include "Service/Service.h"

namespace tse
{
class MetricsTrx;
class PricingTrx;
class AltPricingTrx;
class NoPNRPricingTrx;
class RexPricingTrx;
class ExchangePricingTrx;
class PricingServiceImpl;
class RexShoppingTrx;
class RefundPricingTrx;
class RexExchangeTrx;
class StructuredRuleTrx;
class ConfigMan;

class PricingServiceImpl final : public Service
{

public:
  PricingServiceImpl(const std::string& name, TseServer& srv);

  virtual bool initialize(int argc, char* argv[]) override;

  virtual bool process(MetricsTrx& trx) override;
  virtual bool process(PricingTrx& trx) override;
  virtual bool process(ShoppingTrx& trx) override;
  virtual bool process(AltPricingTrx& trx) override;
  virtual bool process(NoPNRPricingTrx& trx) override;
  virtual bool process(RexPricingTrx& trx) override;
  virtual bool process(ExchangePricingTrx& trx) override;
  virtual bool process(RexShoppingTrx& trx) override;
  virtual bool process(RefundPricingTrx& trx) override;
  virtual bool process(RexExchangeTrx& trx) override;
  virtual bool process(TktFeesPricingTrx& trx) override;
  virtual bool process(StructuredRuleTrx& trx) override;

  virtual uint32_t getActiveThreads() override;

private:
  ConfigMan& _config;
  PricingOrchestrator _po;
};
}

