//---------------------------------------------------------------------------
//  Copyright Sabre 2016
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
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxDU1_00 : public Tax
{
public:
  TaxDU1_00() = default;

private:
  TaxDU1_00(const TaxDU1_00& tax) = delete;
  TaxDU1_00& operator=(const TaxDU1_00& tax) = delete;

  bool validateRange(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg,
      uint16_t& startIndex, uint16_t& endIndex) override;
};

} /* end tse namespace */

