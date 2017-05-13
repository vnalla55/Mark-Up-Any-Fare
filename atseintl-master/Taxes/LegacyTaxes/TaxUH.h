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
#ifndef TAX_UH_H
#define TAX_UH_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//----------------------------------------------------
// Tax special process 52 for Russian Security Charge
//----------------------------------------------------

class TaxUH : public Tax
{

public:
  //-------------------------------
  //--- Tax Database definition ---
  //-------------------------------

  static constexpr char TAX_EXCLUDE = 'Y';
  static constexpr char TAX_CHECKED = 'Y';

  TaxUH();
  virtual ~TaxUH();

private:
  bool validateFromTo(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& startIndex,
                      uint16_t& endIndex);

  TaxUH(const TaxUH& tax);
  TaxUH& operator=(const TaxUH& tax);
};

} /* end tse namespace */

#endif /* TAX_UH_H */
