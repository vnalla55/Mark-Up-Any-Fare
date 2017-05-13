//---------------------------------------------------------------------------
//  Copyright Sabre 2004-2009
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
#ifndef TAX_SP4102_H
#define TAX_SP4102_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxSP4101.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxSP4102 : public TaxSP4101
{
  friend class TaxSP4102Test;

private:
  bool loc2ExcludeTransit(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t travelSegIndex) override;
};

} /* end tse namespace */

#endif /* TAX_SP4102_H */
