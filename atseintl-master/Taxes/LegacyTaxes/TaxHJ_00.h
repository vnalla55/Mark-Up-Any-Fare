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

class TaxHJ_00 : public Tax
{
public:
  TaxHJ_00(){};

protected:
  bool validateItin(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg);

  bool validateFinalGenericRestrictions(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t& startIndex,
                                        uint16_t& /*endIndex*/);

private:

  TaxHJ_00(const TaxHJ_00&) = delete;
  TaxHJ_00& operator=(const TaxHJ_00&) = delete;
};

}; //namespace tse

