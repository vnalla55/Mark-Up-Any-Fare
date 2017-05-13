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
#ifndef TAX_SL1_H
#define TAX_SL1_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 53 for Australian Goods and Service Tax
//---------------------------------------------------------------------------

class TaxSL1 : public Tax
{

public:
  TaxSL1();
  virtual ~TaxSL1();

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

private:
  TaxSL1(const TaxSL1& tax);
  TaxSL1& operator=(const TaxSL1& tax);
};

} /* end tse namespace */

#endif /* TAX_SL1_H */
