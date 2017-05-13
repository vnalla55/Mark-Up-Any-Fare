//---------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied..
//
//----------------------------------------------------------------------------
#ifndef TAX_QO_H
#define TAX_QO_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 67 for Argentina
//---------------------------------------------------------------------------

class TaxQO : public Tax
{

public:
  static constexpr char YES = 'Y';

  TaxQO();
  virtual ~TaxQO();

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

private:
  TaxQO(const TaxQO& map);
  TaxQO& operator=(const TaxQO& map);
};

} /* end tse namespace */

#endif /* TAX_QO_H */
