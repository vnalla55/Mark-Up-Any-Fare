//---------------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{

class PricingTrx;
class TaxResponse;
class TaxCodeReg;

class TaxJN_00 : public Tax
{
public:
  virtual bool
  validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  virtual bool validateCabin(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t travelSegIndex) override;

private:
  bool _cabinWasChecked = false;
  bool _cabinIsValid = false;
};
} // namespace tse
