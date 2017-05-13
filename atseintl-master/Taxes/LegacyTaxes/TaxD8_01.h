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

#ifndef TAXD8_01_H_
#define TAXD8_01_H_

#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{

class Logger;
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class RepricingTrx;
class AirSeg;
class CabinType;
class FareMarket;
class PaxTypeFare;
class Itin;
class Diagnostic;

class TaxD8_01 : public Tax
{
public:
  TaxD8_01() {};

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

private:
  bool locateDomesticSegments(TaxResponse& taxResponse,
                              TaxLocIterator& locIt,
                              const PricingTrx& trx);

  void removeConnectionSegmentsR5(Diagnostic& diag,
                                  std::list<TravelSeg*>& lstSeg,
                                  TaxLocIterator& locIt,
                                  const PricingTrx& trx) const;

  void calculateTaxFareAmount(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              std::list<TravelSeg*> lstSeg);

  PricingTrx* getFareMarkets(PricingTrx& trx,
                             const FareUsage* fareUsage,
                             std::vector<TravelSeg*>& tvlSeg,
                             const PaxTypeCode& paxTypeCode,
                             std::vector<FareMarket*>& vFareMarkets) const;

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

  MoneyAmount prorateMethod(PricingTrx& trx, MoneyAmount taxableFare, uint32_t unMilesTotal,
      uint32_t unMilesDomestic) const;

  MoneyAmount subtractInternationalQSurcharges(PricingTrx& trx,
      MoneyAmount taxableFare, const FareUsage* fareUsage) const;

  static Logger _logger;
  std::list<TravelSeg*> _domesticTravelSegLst;

  TaxD8_01(const TaxD8_01&);
  TaxD8_01& operator=(const TaxD8_01&);

};

}; //namespace tse

#endif /* TAXD8_01_H_ */
