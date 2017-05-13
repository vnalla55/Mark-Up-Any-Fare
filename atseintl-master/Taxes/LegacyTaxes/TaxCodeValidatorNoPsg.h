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
#pragma once

#include "Taxes/LegacyTaxes/TaxCodeValidator.h"

namespace tse
{

class TaxCodeValidatorNoPsg : public TaxCodeValidator
{
  bool validatePassengerRestrictions(PricingTrx& /*trx*/,
                                     TaxResponse& /*taxResponse*/,
                                     TaxCodeReg& /*taxCodeReg*/,
                                     const PaxType* /*paxType*/) override
  {
    return true;
  }
};

} /* end tse namespace */

