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
#ifndef TAX_SP54_H
#define TAX_SP54_H

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

class TaxSP54 : public Tax
{

public:
  static const char* TAX_CODE_ZX;

  TaxSP54();
  virtual ~TaxSP54();

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

private:
  TaxSP54(const TaxSP54& map);
  TaxSP54& operator=(const TaxSP54& map);
};

} /* end tse namespace */

#endif /* TAX_SP54_H */
