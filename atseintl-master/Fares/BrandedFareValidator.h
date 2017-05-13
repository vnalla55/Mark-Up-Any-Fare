//----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
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

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"

#include <vector>

namespace tse
{
class DiagManager;
class FareMarket;
class PaxTypeFare;
class PricingTrx;
class PricingRequest;

class BrandedFareValidator
{
  friend class BrandedFareValidatorTest;

public:
  BrandedFareValidator(const PricingTrx& trx, DiagManager& diag);

  void brandedFareValidation(const FareMarket& fareMarket) const;
  void excludeBrandedFareValidation(const FareMarket& fareMarket) const;

private:
  void printFareMarket(const FareMarket& fareMarket) const;
  void regularFaresValidation(const std::vector<PaxTypeFare*>& fares) const;
  void excludeFaresValidation(const std::vector<PaxTypeFare*>& fares) const;

  const PricingTrx& _trx;
  const PricingRequest& _req;
  DiagManager& _diag;
  const bool _allowedPartitionsId;
  static ConfigurableValue<ConfigSet<std::string>> _allowedPartitionsIdsFromConfig;
};

} // tse

