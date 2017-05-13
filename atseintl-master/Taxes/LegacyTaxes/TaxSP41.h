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
#ifndef TAX_SP41_H
#define TAX_SP41_H

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

class TaxSP41 : public Tax
{

public:
  static constexpr char YES = 'Y';

  //-------------------------------
  //--- Tax Database definition ---
  //-------------------------------

  TaxSP41();
  virtual ~TaxSP41();

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

private:
  bool loc1ExcludeTransit(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t travelSegIndex);

  bool loc2ExcludeTransit(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t travelSegIndex);

  bool validateTransitRestriction(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t startIndex,
                                  bool mirrorImage);

  TaxSP41(const TaxSP41& map);
  TaxSP41& operator=(const TaxSP41& map);
};

} /* end tse namespace */

#endif /* TAX_SP41_H */
