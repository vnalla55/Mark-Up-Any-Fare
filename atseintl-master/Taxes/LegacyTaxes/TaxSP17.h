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
#ifndef TAX_SP17_H
#define TAX_SP17_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 15 for US Customs / Aphis Fee / Inspection
//---------------------------------------------------------------------------

class TaxSP17 : public Tax
{

public:
  TaxSP17();
  virtual ~TaxSP17();

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

private:
  bool hiddenCityUS(PricingTrx& trx, TaxResponse& taxResponse);

  TaxSP17(const TaxSP17& spc);
  TaxSP17& operator=(const TaxSP17& spc);
};

} /* end tse namespace */

#endif /* TAX_SP17_H */
