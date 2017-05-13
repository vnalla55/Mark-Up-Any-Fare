#ifndef TAX_XG_10_H
#define TAX_XG_10_H

#include "Taxes/LegacyTaxes/TaxXG.h"
#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "DBAccess/Loc.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxXG_10 : public TaxXG
{

private:
  bool validateXG(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  bool validateXG1(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

public:
  virtual ~TaxXG_10() {}

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;
};

} /* end tse namespace */

#endif /* TAX_XG_10_H */
