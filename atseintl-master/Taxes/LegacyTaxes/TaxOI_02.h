#ifndef TAX_OI_02_H
#define TAX_OI_02_H

#include "Taxes/LegacyTaxes/TaxOI_01.h"

namespace tse
{

class TaxOI_02 : public TaxOI_01
{
public:
  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;
};

} /* end tse namespace */

#endif /* TAX_OI_02_H */
