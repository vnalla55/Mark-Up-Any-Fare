//---------------------------------------------------------------------------
//  Copyright Sabre 2004-2008
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
#ifndef TAX_SP4101_H
#define TAX_SP4101_H

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

class TaxSP4101 : public Tax
{
  friend class TaxSP4102Test;

public:
  static constexpr char YES = 'Y';

  //-------------------------------
  //--- Tax Database definition ---
  //-------------------------------

  TaxSP4101();
  virtual ~TaxSP4101();

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

private:
  bool loc1ExcludeTransit(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          uint16_t travelSegIndex);

  virtual bool loc2ExcludeTransit(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t travelSegIndex);

  TaxSP4101(const TaxSP4101& map);
  TaxSP4101& operator=(const TaxSP4101& map);
};

} /* end tse namespace */

#endif /* TAX_SP4101_H */
