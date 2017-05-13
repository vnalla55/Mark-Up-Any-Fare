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
#ifndef TAX_SP31_H
#define TAX_SP31_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 31 for Japan Partial Tax Processing
//---------------------------------------------------------------------------

class TaxSP31 : public Tax
{

public:
  TaxSP31();
  virtual ~TaxSP31();

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

  void applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

private:
  static constexpr char DOMESTIC = 'D';
  static const std::string TAX_CODE_XS;

  TaxSP31(const TaxSP31& tax);
  TaxSP31& operator=(const TaxSP31& tax);

  void applyTaxOnTax_new(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);
  void applyTaxOnTax_old(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg);
};

} /* end tse namespace */

#endif /* TAX_SP31_H */
