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
#ifndef TAXD8_02_H_
#define TAXD8_02_H_

#include "Taxes/LegacyTaxes/Tax.h"
#include <list>
#include <vector>

namespace tse
{

class Logger;
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class TaxLocIterator;
class RepricingTrx;
class FareMarket;
class FareUsage;
class PaxTypeFare;
class Itin;
class DiagManager;

class TaxD8_02 : public Tax
{
public:
  TaxD8_02() {};

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

private:
  void locateDomesticSegments(TaxResponse& taxResponse,
                              TaxLocIterator& locIt,
                              const PricingTrx& trx);

  void removeConnectionSegmentsR5(DiagManager& diag,
                                  TaxLocIterator& locIt,
                                  const PricingTrx& trx);

  void calculateTaxFareAmount(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg);

  PricingTrx* getFareMarkets(PricingTrx& trx,
                             const FareUsage* fareUsage,
                             std::vector<TravelSeg*>& tvlSeg,
                             const PaxTypeCode& paxTypeCode,
                             std::vector<FareMarket*>& fareMarkets) const;

  void applyPartialAmount(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          MoneyAmount taxableFare,
                          const TravelSeg* travelSeg);

  RepricingTrx* getRepricingTrx(PricingTrx& trx,
                                std::vector<TravelSeg*>& tvlSeg,
                                Indicator wpncsFlagIndicator,
                                const PaxTypeCode& extraPaxType,
                                const bool privateFareCheck) const;

  const PaxTypeFare* getLikeFare(PricingTrx& trx,
                                 const FareUsage* fareUsage,
                                 std::vector<TravelSeg*>& tvlSeg,
                                 const PaxTypeCode& paxTypeCode,
                                 const Itin& itin) const;

  BookingCode getBookingCode(const Itin& itin,
                             const PaxTypeFare& fare,
                             const std::vector<TravelSeg*>& segs,
                             const TravelSeg* travelSegment) const;

  bool validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  void applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  MoneyAmount aLikeMethod(PricingTrx& trx, MoneyAmount taxableFare, const PaxTypeFare* paxFare1,
      const PaxTypeFare* paxFare2, const FareUsage* fareUsage) const;

  MoneyAmount prorateMethod(PricingTrx& trx, MoneyAmount taxableFare, uint32_t milesTotal,
      uint32_t milesDomestic) const;

  MoneyAmount subtractInternationalQSurcharges(PricingTrx& trx,
      MoneyAmount taxableFare, const FareUsage* fareUsage) const;

  bool
  isRuralExempted(TravelSeg* travelSeg,
                  PricingTrx& trx,
                  TaxResponse& taxResponse,
                  TaxCodeReg& taxCodeReg) const;

  void
  setSegsTaxableForYQYR(std::vector<TravelSeg*> segs)
  {
    for (const auto& seg : segs)
      _segsTaxableForYQYR.push_back(seg);
  }

  bool
  isInList(TravelSeg* travelSeg, const TaxResponse& taxResponse) const;

  TaxD8_02(const TaxD8_02&) = delete;
  TaxD8_02& operator=(const TaxD8_02&) = delete;

  static Logger _logger;
  std::list<TravelSeg*> _domesticTravelSegLst;
  std::vector<TravelSeg*> _segsTaxableForYQYR;
};

}; //namespace tse

#endif /* TAXD8_02_H_ */
