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
class DiagManager;
class TaxRestrictionTransit;

class TaxUT_00 : public Tax
{
public:
  TaxUT_00() = default;

  bool validateTransit(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, uint16_t travelSegIndex) override;

  bool validateFinalGenericRestrictions(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, uint16_t& startIndex, uint16_t& endIndex) override;

private:
  TaxUT_00(const TaxUT_00& tax) = delete;
  TaxUT_00& operator=(const TaxUT_00& tax) = delete;

  bool checkStopover(PricingTrx& trx, TaxLocIterator& locIt,
      const TaxRestrictionTransit& transit, uint16_t endIndex, DiagManager& diag) const;

  TaxLocIterator* getLocIterator(FarePath& farePath, uint16_t startIndex,
      const TaxRestrictionTransit& transit);
};

} /* end tse namespace */

