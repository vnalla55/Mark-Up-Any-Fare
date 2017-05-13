//----------------------------------------------------------------
//
//  File:        FareCalcService
//  Authors:    Mike Carroll
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

#pragma once

#include "Service/Service.h"

namespace tse
{
class ConfigMan;
class Trx;
class TseServer;
class FareCalcService;
class AltPricingTrx;
class NoPNRPricingTrx;
class ExchangePricingTrx;
class RexPricingTrx;
class RexExchangeTrx;
class RefundPricingTrx;
class StructuredRuleTrx;

class FareCalcService final : public Service
{
  friend class FareCalcServiceTest;

public:
  FareCalcService(const std::string& name, tse::TseServer& server);

  bool initialize(int argc = 0, char* argv[] = nullptr) override;

  bool process(PricingTrx& trx) override;
  bool process(ShoppingTrx& trx) override;
  bool process(ExchangePricingTrx& trx) override;
  bool process(AltPricingTrx& trx) override;
  bool process(NoPNRPricingTrx& trx) override;
  bool process(RexPricingTrx& trx) override;
  bool process(RexExchangeTrx& trx) override;
  bool process(RefundPricingTrx& trx) override;
  bool process(StructuredRuleTrx& trx) override;

  virtual uint32_t getActiveThreads() override;

private:
  ConfigMan& _config;
};
} /* end namespace tse */
