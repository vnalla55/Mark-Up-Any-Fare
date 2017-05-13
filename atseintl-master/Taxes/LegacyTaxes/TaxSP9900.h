//---------------------------------------------------------------------------
//  Copyright Sabre 2013
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
#ifndef TAX_SP9900_H
#define TAX_SP9900_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxSP9900 : public Tax
{
  bool _sp99Processed;
  bool _sp99Restriction;

public:

  TaxSP9900()
    : _sp99Processed(false),
      _sp99Restriction(false)
  {
  }

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;
};

} /* end tse namespace */

#endif /* TAX_SP9900_H */
