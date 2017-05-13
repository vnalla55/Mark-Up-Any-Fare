#ifndef TAX_SP9000_H
#define TAX_SP9000_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxSP9000 : public Tax
{
  friend class TaxSP9000Test;

private:
  bool validateTransitOnHiddenPoints(const TaxCodeReg& taxCodeReg) const;

public:
  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override
  {
    return true;
  }
};

} /* end tse namespace */

#endif /* TAX_SP9000_H */
