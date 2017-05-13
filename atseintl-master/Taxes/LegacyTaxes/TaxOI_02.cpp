#include "Taxes/LegacyTaxes/TaxOI_02.h"

using namespace tse;

bool
TaxOI_02::validateLocRestrictions(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t& startIndex,
                                  uint16_t& endIndex)
{
  return true;
}

bool
TaxOI_02::validateTripTypes(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            uint16_t& startIndex,
                            uint16_t& endIndex)

{
  return Tax::validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}
