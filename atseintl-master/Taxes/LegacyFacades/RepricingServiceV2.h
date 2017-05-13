// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Taxes/AtpcoTaxes/ServiceInterfaces/RepricingService.h"
#include "Taxes/LegacyFacades/V2TrxMappingDetails.h"

namespace tse
{
class PricingTrx;

class RepricingServiceV2 : public tax::RepricingService
{
  friend class RepricingServiceV2Test;

public:
  RepricingServiceV2(PricingTrx& trx, const tax::V2TrxMappingDetails* v2Mapping);

  tax::type::MoneyAmount getFareUsingUSDeductMethod(const tax::type::Index& taxPointBegin,
                                                    const tax::type::Index& taxPointEnd,
                                                    const tax::type::Index& itinId) const override;

  tax::type::MoneyAmount getFareFromFareList(const tax::type::Index& taxPointBegin,
                                             const tax::type::Index& taxPointEnd,
                                             const tax::type::Index& itinId) const override;

  tax::type::MoneyAmount getSimilarDomesticFare(const tax::type::Index& taxPointBegin,
                                                const tax::type::Index& taxPointEnd,
                                                const tax::type::Index& itinId,
                                                bool& fareFound) const override;

  tax::type::MoneyAmount getBahamasSDOMFare(const tax::type::Index& taxPointBegin,
                                            const tax::type::Index& taxPointEnd,
                                            const tax::type::Index& itinId) const override;

private:
  FarePath* getFarePathFromItinId(tax::type::Index itinId) const;
  void convertCurrency(MoneyAmount& taxableAmount,
                       FarePath& farePath,
                       CurrencyCode& paymentCurrency) const;

  RepricingTrx*
  getRepricingTrx(std::vector<TravelSeg*>& tvlSeg, FMDirection fmDirectionOverride) const;
  RepricingTrx* getRepricingTrx(std::vector<TravelSeg*>& tvlSeg,
                                Indicator wpncsFlagIndicator,
                                const PaxTypeCode& extraPaxType,
                                bool privateFareCheck) const;

  bool isUsOrBufferZone(const Loc* loc) const;
  bool locateOpenJaw(FarePath& farePath) const;
  bool getNetAmountForLCT(FareUsage* fareUsage, MoneyAmount& netAmount) const;
  bool isGroundTransportation(const Itin& itin,
                              const uint16_t startIndex,
                              const uint16_t endIndex) const;
  virtual uint16_t calculateMiles(FarePath& farePath,
                                  std::vector<TravelSeg*>& travelSegs,
                                  const Loc* origin,
                                  const Loc* destination) const;
  virtual bool subtractPartialFare(FarePath& farePath,
                                   const MoneyAmount& totalFareAmount,
                                   MoneyAmount& taxableAmount,
                                   FareUsage& fareUsage,
                                   std::vector<TravelSeg*>& travelSegsEx,
                                   CurrencyCode& paymentCurrency) const;
  MoneyAmount calculatePartAmount(FarePath& farePath,
                                  uint16_t startIndex,
                                  uint16_t endIndex,
                                  uint16_t fareBreakEnd,
                                  FareUsage& fareUsage) const;


  const PaxTypeFare* getDomesticBahamasFare(TravelSeg& seg,
                                            const FareUsage* fareUsage,
                                            const PaxTypeCode& paxTypeCode) const;
  PricingTrx* getFareMarkets(const DateTime& dateFare,
                             TravelSeg& seg,
                             const PaxTypeCode& paxTypeCode,
                             std::vector<FareMarket*>& vFareMarkets) const;

  void setDepartureAndArrivalDates(AirSeg* ts) const;
  bool isTravelSegWhollyDomestic(const TravelSeg* travelSeg,
                                 const NationCode& nation) const;
  const FareUsage* locateFareUsage(const FarePath& farePath, const TravelSeg* travelSeg) const;
  PaxTypeCode getMappedPaxTypeCode(const PaxTypeCode& paxTypeCode,
                                   const PaxTypeCode& defaultPaxTypeCode = "") const;
  const std::vector<PaxTypeFare*>*
  locatePaxTypeFare(const FareMarket* fareMarketReTrx, const PaxTypeCode& paxTypeCode) const;
  void getBkgCodeReBooked(const FareUsage* fareUsage,
                          TravelSeg* travelSegRef,
                          TravelSeg* travelSegClone) const;
  void applyPartialAmount(FarePath& farePath,
                          MoneyAmount& taxableFare,
                          CurrencyCode& paymentCurrency,
                          TravelSeg* travelSeg) const;
  virtual bool findRepricedFare(const PaxTypeCode& paxTypeCode,
                                const FareUsage& fareUsage,
                                TravelSeg* travelSeg,
                                bool changeDate,
                                Indicator wpncsFlagIndicator,
                                MoneyAmount& taxableFare,
                                const DateTime& travelDate,
                                bool ignoreCabinCheck,
                                bool privateFareCheck = false) const;
  void fillSegmentForRepricing(const FareUsage& fareUsage,
                               TravelSeg* travelSegIter,
                               bool changeDate,
                               std::vector<TravelSeg*>& segs) const;
  bool getAmountFromRepricedFare(const PaxTypeCode& paxTypeCode,
                                 RepricingTrx& retrx,
                                 MoneyAmount& taxableFare,
                                 const CabinType& orgTravelSegCabin,
                                 const DateTime& travelDate,
                                 bool bIgnoreCabinCheck) const;

  PricingTrx& _trx;
  const tax::V2TrxMappingDetails* _v2Mapping;
};
}
