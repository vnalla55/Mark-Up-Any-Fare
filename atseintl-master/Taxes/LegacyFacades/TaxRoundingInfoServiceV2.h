// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Taxes/AtpcoTaxes/ServiceInterfaces/TaxRoundingInfoService.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class PricingTrx;

class TaxRoundingInfoServiceV2 : public tax::TaxRoundingInfoService
{
public:
  TaxRoundingInfoServiceV2(PricingTrx& trx);

  void getFareRoundingInfo(const tax::type::CurrencyCode& currency,
                           tax::type::MoneyAmount& unit,
                           tax::type::TaxRoundingDir& dir) const override;

  void getNationRoundingInfo(const tax::type::Nation& nation,
                             tax::type::MoneyAmount& unit,
                             tax::type::TaxRoundingDir& dir) const override;

  void getTrxRoundingInfo(const tax::type::Nation& /*nation*/,
                          tax::type::MoneyAmount& unit,
                          tax::type::TaxRoundingDir& dir) const override;

  void doStandardRound(tax::type::MoneyAmount& amount,
                       tax::type::MoneyAmount& unit,
                       tax::type::TaxRoundingDir& dir,
                       tax::type::MoneyAmount currencyUnit = -1,
                       bool isOcFee = false) const override;

private:

  CurrencyCode getPaymentCurrency() const;
  RoundingRule convertToRoundingRule(tax::type::TaxRoundingDir dir) const;
  tax::type::TaxRoundingDir convertToTaxRoundingDir(const RoundingRule& rule) const;

  PricingTrx& _trx;
};
}
