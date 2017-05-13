//---------------------------------------------------------------------------
//  Copyright Sabre 2010
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
#ifndef TAX_CA_02_H
#define TAX_CA_02_H

#include "Taxes/LegacyTaxes/TaxCA01.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxCA02 : public TaxCA01
{

public:
  TaxCA02() {}
  virtual ~TaxCA02() {};

  void adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

private:
  TaxCA02(const TaxCA02& tax);
  TaxCA02& operator=(const TaxCA02& tax);
};

} /* end tse namespace */

#endif /* TAX_CA_02_H */
