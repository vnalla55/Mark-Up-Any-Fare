//----------------------------------------------------------------
//
//  File:        InternalService
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

#include "Internal/InternalServiceController.h"
#include "Service/Service.h"

namespace tse
{
class ConfigMan;
class Trx;
class TseServer;
class InternalService;
class AltPricingTrx;
class NoPNRPricingTrx;
class RexPricingTrx;
class ExchangePricingTrx;
class RefundPricingTrx;
class RexExchangeTrx;
class FareDisplayTrx;
class AncillaryPricingTrx;
class BrandingTrx;
class BaggageTrx;
class TktFeesPricingTrx;
class StructuredRuleTrx;

class InternalService final : public Service
{
  friend class InternalServiceTest;

public:
  InternalService(const std::string& name, tse::TseServer& server);

  /**
   * Ititialize the object
   *
   * @param argc number of parameters
   * @param argv parameters
   *
   * @return true if successful, false otherwise
   */
  bool initialize(int argc = 0, char* argv[] = nullptr) override;

  /**
   * Process a transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */
  bool process(PricingTrx& trx) override;
  bool process(AltPricingTrx& trx) override;
  bool process(NoPNRPricingTrx& trx) override;
  bool process(RexPricingTrx& trx) override;
  bool process(ExchangePricingTrx& trx) override;
  bool process(RexShoppingTrx& trx) override;
  bool process(RefundPricingTrx& trx) override;
  bool process(RexExchangeTrx& trx) override;
  bool process(FareDisplayTrx& trx) override;
  bool process(AncillaryPricingTrx& trx) override;
  bool process(BrandingTrx& trx) override;
  bool process(BaggageTrx& trx) override;
  bool process(TktFeesPricingTrx& trx) override;
  bool process(StructuredRuleTrx& trx) override;

private:
  ConfigMan& _config;
  InternalServiceController _internalServiceController;
};
} /* end namespace tse */
