// ---------------------------------------------------------------------------
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
#ifndef TAX_APPLY_ON_OC_H
#define TAX_APPLY_ON_OC_H

#include <string>
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/TaxApply.h"

namespace tse
{
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class TaxMap;
class Tax;
class TaxApply;
class TaxOnOC;

// ----------------------------------------------------------------------------
// TaxApply will call all the TaxValidator functions for all special taxes
// ----------------------------------------------------------------------------

class TaxApplyOnOC : public TaxApply
{
public:
  static constexpr Indicator TAXINCLIND = 'X';
  static constexpr Indicator TAXEXEMPTIND = 'Y';

  TaxApplyOnOC();
  virtual ~TaxApplyOnOC();

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
  TaxApplyOnOC(const TaxApplyOnOC& apply);
  TaxApplyOnOC& operator=(const TaxApplyOnOC& apply);

  void addTaxResponseToItin(PricingTrx& trx, TaxResponse& taxResponse, TaxOnOC& taxOnOC) const;
};
}
#endif
