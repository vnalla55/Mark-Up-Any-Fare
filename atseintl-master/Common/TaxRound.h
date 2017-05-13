// ---------------------------------------------------------------------------
//
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class PricingTrx;

class TaxRound
{

public:
  TaxRound();
  virtual ~TaxRound() = default;

  MoneyAmount doSpecialTaxRound(PricingTrx& trx,
                                const MoneyAmount& fareAmount,
                                const MoneyAmount& taxAmount,
                                uint_least64_t centNumber = 50) const;

  void retrieveNationRoundingSpecifications(PricingTrx& trx,
                                            RoundingFactor& roundingUnit,
                                            CurrencyNoDec& roundingNoDec,
                                            RoundingRule& roundingRule) const;

  void retrieveNationRoundingSpecifications(PricingTrx& trx,
                                            const NationCode& nation,
                                            RoundingFactor& roundingUnit,
                                            CurrencyNoDec& roundingNoDec,
                                            RoundingRule& roundingRule) const;

  MoneyAmount applyTaxRound(const MoneyAmount& taxAmount,
                            const CurrencyCode& paymentCurrency,
                            const RoundingFactor& roundingUnit,
                            RoundingRule roundingRule) const;

private:
  TaxRound(const TaxRound& calc);
  TaxRound& operator=(const TaxRound& calc);
};
}
