//---------------------------------------------------------------------------
//  Copyright Sabre 2009
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
#ifndef TAX_JP1_00_H
#define TAX_JP1_00_H

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include <vector>

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{

class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class RepricingTrx;
class TravelSeg;
class FareUsage;
class FarePath;
class PaxTypeFare;
class CabinType;
class FareMarket;
class PaxTypeFare;
class AirSeg;
class Itin;

class TripTypesValidatorJP1 : public TripTypesValidator
{
  friend class TaxJP1_00Test;

protected:
  bool validateFromTo(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& startIndex,
                      uint16_t& endIndex) override;
  uint16_t findTaxStopOverIndex(const PricingTrx& trx,
                                const TaxResponse& taxResponse,
                                const TaxCodeReg& taxCodeReg,
                                uint16_t startIndex) override;

private:
  bool isTerminationPt(Itin* itin, uint16_t index);
  bool haveOnlyJpSegs(Itin* itin, uint16_t startIndex, uint16_t endIndex);
  bool haveOriginAndTerminationInJp(Itin* itin);
  void applyForcedQualifierOnStopOver(const AirSeg* airSeg, bool& isStopOver);
  bool isReturnToPrecedingPoint(const TaxCodeReg& taxCodeReg, Itin* itin, uint16_t index) const;
};

class TaxJP1_00 : public Tax
{
public:
  friend class TaxJP1_00Test;

  TaxJP1_00();
  virtual ~TaxJP1_00();

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

private:
  static log4cxx::LoggerPtr _logger;
  static const std::string _allowedFareTypeFilter;

  TaxJP1_00(const TaxJP1_00& map);
  TaxJP1_00& operator=(const TaxJP1_00& map);

  void getBkgCodeReBooked(FareUsage* fareUsage, TravelSeg* travelSegRef, TravelSeg* travelSegClone);
  void applyPartialAmount(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          TaxCodeReg& taxCodeReg,
                          MoneyAmount& taxableFare,
                          CurrencyCode& paymentCurrency,
                          TravelSeg* travelSeg);
  bool findRepricedFare(PricingTrx& trx,
                        const PaxTypeCode& paxTypeCode,
                        FareUsage& fareUsage,
                        std::list<TravelSeg*>::iterator& travelSegIter,
                        bool changeDate,
                        Indicator wpncsFlagIndicator,
                        MoneyAmount& taxableFare,
                        const DateTime& travelDate,
                        bool ignoreCabinCheck,
                        std::string repricingTrxReadyMessage,
                        std::string noRepricingTrxMessage,
                        bool privateFareCheck = false);
  void setDepartureAndArrivalDates(AirSeg* ts, PricingTrx& trx);
  void fillSegmentForRepricing(PricingTrx& trx,
                               FareUsage& fareUsage,
                               std::list<TravelSeg*>::iterator& travelSegIter,
                               bool changeDate,
                               std::vector<TravelSeg*>& segs);
  RepricingTrx* getRepricingTrx(PricingTrx& trx,
                                std::vector<TravelSeg*>& tvlSeg,
                                Indicator wpncsFlagIndicator,
                                const PaxTypeCode& extraPaxType,
                                bool privateFareCheck);
  bool getAmountFromRepricedFare(const PaxTypeCode& paxTypeCode,
                                 RepricingTrx* retrx,
                                 TravelSeg* travelSeg,
                                 MoneyAmount& taxableFare,
                                 const CabinType& orgTravelSegCabin,
                                 const DateTime& travelDate,
                                 bool bIgnoreCabinCheck);

  bool isTravelSegWhollyJapan(TravelSeg* travelSeg);
  FareUsage* locateWhollyDomesticFareUsage(FarePath* farePath, TravelSeg* travelSeg);
  MoneyAmount convertCurrency(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              MoneyAmount moneyAmount,
                              CurrencyCode& paymentCurrency);

  const std::vector<PaxTypeFare*>*
  locatePaxTypeFare(const FareMarket* fareMarketReTrx, const PaxTypeCode& paxTypeCode);

  const PaxTypeCode getMappedPaxTypeCode(const PaxTypeCode& paxTypeCode,
                                         const PaxTypeCode& defaultPaxTypeCode = "");
};

} // end tse namespace

#endif // TAX_JP1_00_H
