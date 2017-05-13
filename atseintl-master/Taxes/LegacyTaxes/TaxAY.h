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
#ifndef TAX_AY_H
#define TAX_AY_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class TravelSeg;

//---------------------------------------------------------------------------
// Tax special process 16 for US Security Fee
//---------------------------------------------------------------------------

class TaxAY : public Tax
{

public:
  TaxAY();
  virtual ~TaxAY();

  bool validateSequence(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& travelSegStartIndex,
                        uint16_t& travelSegEndIndex,
                        bool checkSpn = false) override;

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

private:
  bool hiddenCityUS(PricingTrx& trx, TravelSeg* travelSeg);

  TaxAY(const TaxAY& tax);
  TaxAY& operator=(const TaxAY& tax);
};

} /* end tse namespace */

#endif /* TAX_AY_H */
