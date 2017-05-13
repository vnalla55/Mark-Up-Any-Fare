// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
#ifndef TAX_APPLY_ON_CHANGE_FEE_H
#define TAX_APPLY_ON_CHANGE_FEE_H

#include <string>
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/TaxApply.h"

namespace tse
{
class CountrySettlementPlanInfo;
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class TaxMap;
class Tax;
class TaxApply;

// ----------------------------------------------------------------------------
// TaxApply will call all the TaxValidator functions for all special taxes
// ----------------------------------------------------------------------------

class TaxApplyOnChangeFee : public TaxApply
{
public:
  static constexpr Indicator TAXINCLIND = 'X';
  static constexpr Indicator TAXEXEMPTIND = 'Y';

  TaxApplyOnChangeFee();
  virtual ~TaxApplyOnChangeFee();

  Tax* findSpecialTax(TaxMap& taxMap, TaxCodeReg& taxCodeReg) override;

  void applyTax(PricingTrx& trx,
                TaxResponse& taxResponse,
                TaxMap& taxMap,
                TaxCodeReg& taxCodeReg,
                const CountrySettlementPlanInfo* cspi) override;

  void initializeTaxItem(PricingTrx& trx,
                         Tax& tax,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg) override;

private:
  TaxApplyOnChangeFee(const TaxApplyOnChangeFee& apply);
  TaxApplyOnChangeFee& operator=(const TaxApplyOnChangeFee& apply);
};
}
#endif
