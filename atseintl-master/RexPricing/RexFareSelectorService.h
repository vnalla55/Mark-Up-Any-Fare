//----------------------------------------------------------------------------
//
// Copyright Sabre 2007
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "RexPricing/RexFareSelector.h"
#include "Service/Service.h"


namespace tse
{
class RexFareSelectorService;
class RexFareSelector;
class RexPricingTrx;
class TseServer;
class RexShoppingTrx;
class RefundPricingTrx;
class RexExchangeTrx;
class ConfigMan;

class RexFareSelectorService final : public Service
{
  // friend class RexFareSelectorServiceTest;

public:
  RexFareSelectorService(const std::string& sname, tse::TseServer& tseServer);

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
  virtual bool process(RexPricingTrx& trx) override;
  virtual bool process(RexShoppingTrx& trx) override;
  virtual bool process(RefundPricingTrx& trx) override;
  virtual bool process(RexExchangeTrx& trx) override;
  virtual uint32_t getActiveThreads() override;

  void processPermutations(RexPricingTrx& trx) const;
  void processPermutations(RefundPricingTrx& trx) const;
  void fullRefundDiagnostics(RefundPricingTrx& trx) const;

private:
  ConfigMan& _config;
  void excPlusUpsForNonRefundable(RexPricingTrx& trx) const;
  void matchBrandCodesToNewFareMarkets(RexExchangeTrx& trx);
};

} // tse

