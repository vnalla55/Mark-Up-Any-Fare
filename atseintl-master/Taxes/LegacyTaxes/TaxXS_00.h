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

#include "Diagnostic/DiagManager.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class DiagManager;

class TaxXS_00 : public Tax
{
public:
  TaxXS_00() = default;

  bool validateFinalGenericRestrictions(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, uint16_t& /*startIndex*/, uint16_t& /*endIndex*/) override;

  void taxCreate(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg,
      uint16_t travelSegStartIndex, uint16_t travelSegEndIndex) override;

  void applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  bool validateTransit(PricingTrx& /*trx*/, TaxResponse& /*taxResponse*/,
      TaxCodeReg& /*taxCodeReg*/, uint16_t /*travelSegIndex*/) override;

private:

  TaxXS_00(const TaxXS_00&) = delete;
  TaxXS_00& operator=(const TaxXS_00&) = delete;

  bool isDomesticSegment(const TravelSeg& travelSeg, const TaxCodeReg& taxCodeReg) const;

  TaxLocIterator* getLocIterator(FarePath& farePath, const TaxCodeReg& taxCodeReg);

  void searchDomesticParts(TaxResponse& taxResponse, TaxCodeReg& taxCodeReg, DiagManager& diag);

  DiagManager getDiagManager(PricingTrx& trx, TaxCodeReg& taxCodeReg) const;

  MoneyAmount taxableFareAmount(PricingTrx& trx, TaxResponse& taxResponse, MoneyAmount taxableFare,
      uint32_t unMilesTotal, uint32_t unMilesDomestic, uint16_t startIdx, DiagManager& diag);

  FareUsage* locateFare(const FarePath* farePath, uint16_t segId) const;

  std::vector<uint16_t> _domSegments;
};

} /* end tse namespace */

