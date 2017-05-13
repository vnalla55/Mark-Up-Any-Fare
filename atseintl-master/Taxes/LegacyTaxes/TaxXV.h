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
#ifndef TAX_XV_H
#define TAX_XV_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 22 for Mexico
//---------------------------------------------------------------------------

class TaxXV : public Tax
{

public:
  static const std::string TAX_CODE_XD;

  TaxXV();
  virtual ~TaxXV();

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

private:
  TaxXV(const TaxXV& sp);
  TaxXV& operator=(const TaxXV& sp);
};

} /* end tse namespace */

#endif /* TAX_XV_H */
