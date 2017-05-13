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

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class Diagnostic;

class TaxF7_00 : public Tax
{
public:
  TaxF7_00() = default;

  bool validateTripTypes(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, uint16_t& startIndex, uint16_t& endIndex) override;

  bool validateTransit(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, uint16_t travelSegIndex) override;

private:
  TaxF7_00(const TaxF7_00& tax) = delete;
  TaxF7_00& operator=(const TaxF7_00& tax) = delete;

  bool findDeplaneSegment(PricingTrx& trx, TaxCodeReg& taxCodeReg, TaxLocIterator& locIt,
      uint16_t& segStart, Diagnostic* diag) const;
  bool findEnplaneSegment(PricingTrx& trx, TaxCodeReg& taxCodeReg, TaxLocIterator& locIt,
      uint16_t& segEnd, Diagnostic* diag) const;

  TaxLocIterator* getLocIterator(FarePath& farePath, uint16_t startIndex,
      const TaxCodeReg& taxCodeReg);

  int16_t _lastAnalysedIndex = -1;
};

} /* end tse namespace */

