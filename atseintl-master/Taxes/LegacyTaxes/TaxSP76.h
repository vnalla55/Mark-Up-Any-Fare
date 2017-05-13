//---------------------------------------------------------------------------
//  Copyright Sabre 2009
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
#ifndef TAX_SP76_H
#define TAX_SP76_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 76 for Brazil SP76 tax
//---------------------------------------------------------------------------

class TaxSP76 : public Tax
{

public:
  TaxSP76();
  virtual ~TaxSP76();

  bool validateCarrierExemption(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t travelSegIndex) override;

private:
  TaxSP76(const TaxSP76& map);
  TaxSP76& operator=(const TaxSP76& map);
};

} /* end tse namespace */

#endif /* TAX_SP76_H */
