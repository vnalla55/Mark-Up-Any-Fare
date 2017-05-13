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
#ifndef TAX_SP60_H
#define TAX_SP60_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 55 for Denmark
//---------------------------------------------------------------------------

class TaxSP60 : public Tax
{

public:
  static constexpr char YES = 'Y';

  TaxSP60();
  virtual ~TaxSP60();

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

private:
  TaxSP60(const TaxSP60& map);
  TaxSP60& operator=(const TaxSP60& map);
};

} /* end tse namespace */

#endif /* TAX_SP60_H */
