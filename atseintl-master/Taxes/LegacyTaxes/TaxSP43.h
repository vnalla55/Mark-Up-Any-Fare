//---------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#ifndef TAX_SP43_H
#define TAX_SP43_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 38 for Great Britian
//---------------------------------------------------------------------------

class TaxSP43 : public Tax
{

public:
  TaxSP43();
  virtual ~TaxSP43();

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

private:
  TaxSP43(const TaxSP43& map);
  TaxSP43& operator=(const TaxSP43& map);
};

} /* end tse namespace */

#endif /* TAX_SP43_H */
