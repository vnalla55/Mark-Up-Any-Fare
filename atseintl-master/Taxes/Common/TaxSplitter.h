// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/ConfigList.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/WnSnapUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "Taxes/Common/AbstractTaxSplitter.h"

namespace log4cxx
{

class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

class FareUsage;
class FarePath;
class Itin;
class TaxItem;

class TaxSplitter : public AbstractTaxSplitter
{

public:
  typedef std::map<FarePath::FarePathKey, MoneyAmount> FarePathAmountMap;
  typedef std::map<Itin*, FarePathAmountMap> FarePathAmountMapForItin;
  typedef std::map<TaxCode, FarePathAmountMap> FarePathAmountMapPerTax;
  typedef std::map<TaxCode, FarePathAmountMapForItin> FarePathAmountMapForItinPerTax;

  TaxSplitter(ShoppingTrx& trx);

  void setupTaxes(Itin* const primaryItin, Itin* subItin, const int legId) override;

  void setupTaxesForFarePath(Itin* const primaryItin,
                             Itin* subItin,
                             const int legId,
                             const FarePath* farePath,
                             FarePath::FarePathKey& farePathKey) override;

  void clearTaxMaps() override;

  Itin* getSubItin(Itin* const primaryItin, const int legId) const override;

private:
  void setupFareAmountTax(const TaxCode& taxCode,
                          Itin* const primaryItin,
                          Itin* subItin,
                          const int legId,
                          const FarePath* const farePath,
                          FarePath::FarePathKey& farePathKey,
                          bool checkTax);

  MoneyAmount getTotalTax(const Itin* const itin,
                          const FarePath::FarePathKey& farePathKey,
                          const TaxCode& taxCode,
                          const CarrierCode& carrierCode);

  MoneyAmount getFareAndSurchargeAmount(const Itin* const itin,
                                        const FarePath* const farePath,
                                        const GeoTravelType geoTravelType);

  MoneyAmount getFareAndSurchargeAmount(Itin* itin,
                                        const FarePath::FarePathKey& farePathKey,
                                        const GeoTravelType geoTravelType);

  MoneyAmount
  getSurcharge(const Itin* const itin, const FarePath* const farePath, const FareUsage* fareUsage);

  TaxItem* findFirstDeleteOtherTax(const Itin* const itin,
                                   const FarePath::FarePathKey& farePathKey,
                                   const TaxCode& taxCode,
                                   const bool checkTax);

  TaxItem*
  findFirstDeleteOtherTax(const TaxCode& taxCode, const bool checkTax, TaxResponse* taxResponse);

  bool copyTaxFromOppositeDirection(TaxItem*& us1tax,
                                    const Itin* const subItin,
                                    const FarePath* const farePath,
                                    const Itin* const oppositeDirSubItin,
                                    FarePath::FarePathKey& farePathKey,
                                    const TaxCode& taxCode);

  void setSegmentAndCxrInTax(TaxItem*& taxItem, const FarePath* const farePath);

  void setupSegmentBasedTax(const TaxCode& taxCode,
                            Itin* const primaryItin,
                            const int legId,
                            FarePath::FarePathKey& farePathKey);

  void setupSegmentBasedTax(const TaxCode& taxCode,
                            Itin* const primaryItin,
                            const CarrierCode& carrierCode,
                            FarePath::FarePathKey& farePathKey);

  void setupTaxOnTax(const TaxCode& taxCode,
                     const FarePath* const farePath,
                     const int legId,
                     Itin* const itin,
                     TaxResponse* const taxResponse);

  void updateCalcTotals(Itin* const itin,
                        const FarePath* const farePath,
                        const TaxResponse* const taxResponse);

  void rebuildTaxRecordVec(TaxResponse* const taxResponse);

  CarrierCode getFirstCxr(Itin* const primaryItin);

  CarrierCode getSecondCxr(Itin* const primaryItin);

  CarrierCode getCarrier(const TravelSeg* const ts);

  int getCxrSegNumInUSA(Itin* const itin, const CarrierCode& carrierCode);

  TaxItem* findFirstTax(const Itin* const itin,
                        const FarePath::FarePathKey& farePathKey,
                        const TaxCode& taxCode,
                        const CarrierCode& carrierCode);

  bool createTax(TaxItem*& taxItem,
                 Itin* const itin,
                 const FarePath::FarePathKey& farePathKey,
                 const TaxItem* const taxItemToBeCopied,
                 const int legId);

  LocCode getFirstLocCode(Itin* const itin, const CarrierCode& carrierCode);

  void setupFlatRateTaxes(const TaxCode& taxCode,
                          Itin* const primaryItin,
                          FarePath::FarePathKey& farePathKey);

  bool appliesToEntireTrip(const TaxItem* const taxItem) const;

  bool convertCurrency(const MoneyAmount fromAmount,
                       CurrencyCode fromCurrencyCode,
                       CurrencyCode toCurrencyCode,
                       MoneyAmount& toAmount);
  static log4cxx::LoggerPtr _logger;
  ShoppingTrx& _trx;
  FarePathAmountMapPerTax _taxMap;
  FarePathAmountMapForItinPerTax _taxMapForItin;
  const ConfigSet<TaxCode>* _fareAmountTaxes;
  const ConfigSet<TaxCode>* _flatRateTaxes;
  const ConfigSet<TaxCode>* _segmentBasedTaxes;
  const ConfigSet<TaxCode>* _taxOnTaxSpecialTaxes;
  struct ExtractedTaxItems
  {
    std::set<std::string> fareAmountTaxItems;
    std::set<std::string> taxOnTaxItems;
  };
};

} // namespace tse

