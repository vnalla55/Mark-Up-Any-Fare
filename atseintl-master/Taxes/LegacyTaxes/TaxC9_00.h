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
class Itin;
class Diagnostic;
class PaxTypeFare;
class FareMarket;

class TaxC9_00 : public Tax
{
public:
  TaxC9_00(){};

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

private:

  bool locateAllDomesticSegments(Diagnostic& diag,
      const Itin& itin,
      TaxLocIterator& locIt,
      std::list<TravelSeg*>& lstSeg);

  void calculateTaxFareAmount(PricingTrx& trx,
      TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg,
      std::list<TravelSeg*> lstSeg,
      bool isDiscount);

  void applyPartialAmount(PricingTrx& trx,
      TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg,
      MoneyAmount taxableFare,
      const TravelSeg* travelSeg,
      const PaxTypeFare* sdomFare=nullptr);

  void applyPartialAmountForSegments(PricingTrx& trx,
      TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg,
      const std::vector<TravelSeg*>& tvlSeg,
      const FareUsage* fareUsage,
      const PaxTypeCode& paxTypeCode);

  const PaxTypeFare* getDomesticFare(PricingTrx& trx,
      TravelSeg& seg,
      const FareUsage* fareUsage,
      const PaxTypeCode& paxTypeCode) const;

  PricingTrx* getFareMarkets(PricingTrx& trx,
      const DateTime& dateFare,
      TravelSeg& seg,
      const PaxTypeCode& paxTypeCode,
      std::vector<FareMarket*>& vFareMarkets) const;

  FareUsage* locateFare(const FarePath* farePath, const TravelSeg* travelSegIn) const;

  bool isDomesticSegment(const TravelSeg& travelSeg) const;

  TaxC9_00(const TaxC9_00&) = delete;
  TaxC9_00& operator=(const TaxC9_00&) = delete;
};

}; //namespace tse

