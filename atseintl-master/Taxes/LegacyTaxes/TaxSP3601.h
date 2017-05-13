#ifndef TAX_SP3601_H
#define TAX_SP3601_H

#include "Taxes/Common/LocRestrictionValidator3601.h"
#include "Taxes/LegacyTaxes/TaxSP36.h"

namespace tse
{

class PricingTrx;
class TaxResponse;
class TaxCodeReg;

class TaxSP3601 : public TaxSP36
{
  friend class TaxSP3601Test;

public:
  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;

private:
  LocRestrictionValidator3601 _locRestrictionValidator;
};
}
#endif
