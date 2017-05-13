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

#ifndef TAXD8_00_H_
#define TAXD8_00_H_

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

class TaxD8_00 : public Tax
{
public:
  TaxD8_00() {};

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

private:
  bool isDomesticSegment(TravelSeg* travelSeg) const;

  const FareUsage* locateFare(const FarePath* farePath, const TravelSeg* travelSegIn) const;

  void applyPartialAmount(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          MoneyAmount taxableFare,
                          const CurrencyCode& paymentCurrency,
                          const TravelSeg* travelSeg);

  MoneyAmount convertCurrency(PricingTrx& trx,
                              const TaxResponse& taxResponse,
                              MoneyAmount moneyAmount,
                              const CurrencyCode& paymentCurrency) const;

  bool findRepricedFare(PricingTrx& trx,
                        const PaxTypeCode& paxTypeCode,
                        const FareUsage& fareUsage,
                        std::list<TravelSeg*>::iterator& travelSegIter,
                        bool changeDate,
                        Indicator wpncsFlagIndicator,
                        MoneyAmount& taxableFare,
                        const DateTime& travelDate,
                        bool ignoreCabinCheck,
                        const char* repricingTrxReadyMessage,
                        const char* noRepricingTrxMessage,
                        bool privateFareCheck = false) const;

  void fillSegmentForRepricing(PricingTrx& trx,
                               const FareUsage& fareUsage,
                               std::list<TravelSeg*>::iterator& travelSegIter,
                               bool changeDate,
                               std::vector<TravelSeg*>& segs) const;

  RepricingTrx* getRepricingTrx(PricingTrx& trx,
                                std::vector<TravelSeg*>& tvlSeg,
                                Indicator wpncsFlagIndicator,
                                const PaxTypeCode& extraPaxType,
                                const bool privateFareCheck) const;

  bool getAmountFromRepricedFare(const PaxTypeCode& paxTypeCode,
                                 RepricingTrx* retrx,
                                 const TravelSeg* travelSeg,
                                 MoneyAmount& taxableFare,
                                 const CabinType& orgTravelSegCabin,
                                 const DateTime& travelDate,
                                 bool bIgnoreCabinCheck) const;

  void getBkgCodeReBooked(const FareUsage* fareUsage,
                          TravelSeg* travelSegRef,
                          TravelSeg* travelSegClone) const;

  void setDepartureAndArrivalDates(AirSeg* ts, const PricingTrx& trx) const;

  const std::vector<PaxTypeFare*>* locatePaxTypeFare(const FareMarket* fareMarketReTrx,
                                                     const PaxTypeCode& paxTypeCode) const;

  static Logger _logger;

  TaxD8_00(const TaxD8_00&);
  TaxD8_00& operator=(const TaxD8_00&);
};

}; //namespace tse

#endif /* TAXD8_00_H_ */
