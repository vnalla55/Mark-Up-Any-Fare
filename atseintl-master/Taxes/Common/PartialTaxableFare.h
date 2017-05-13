//---------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class FareMarket;
class FarePath;
class FareUsage;
class Itin;
class PaxTypeFare;
class PricingTrx;
class TaxCodeReg;
class TaxResponse;
class TravelSeg;

class PartialTaxableFare final
{
public:
  PartialTaxableFare() = default;
  PartialTaxableFare(const PartialTaxableFare& fare) = delete;
  PartialTaxableFare& operator=(const PartialTaxableFare& fare) = delete;

  bool locate(PricingTrx& trx,
              TaxResponse& taxResponse,
              TaxCodeReg& taxCodeReg,
              uint16_t travelSegIndex);

  bool locatedThruFare(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSegGateWay);

  const MoneyAmount& thruTotalFare() const { return _thruTotalFare; }
  MoneyAmount& thruTotalFare() { return _thruTotalFare; }
  const FareUsage* fareUsage() const { return _fareUsage; }
  FareUsage*& fareUsage() { return _fareUsage; }
  const MoneyAmount& taxablePartialFare() const { return _taxablePartialFare; }
  MoneyAmount& taxablePartialFare() { return _taxablePartialFare; }
  const MoneyAmount& specialPercentage() const { return _specialPercentage; }
  const uint32_t& partialLocalMiles() const { return _partialLocalMiles; }
  const uint32_t& partialThruMiles() const { return _partialThruMiles; }
  const CurrencyCode& paymentCurrency() const { return _paymentCurrency; }
  CurrencyCode& paymentCurrency() { return _paymentCurrency; }
  const uint16_t& travelSegLocalStartOrder() const { return _travelSegLocalStartOrder; }
  const uint16_t& travelSegLocalEndOrder() const { return _travelSegLocalEndOrder; }
  const uint16_t& travelSegThruStartOrder() const { return _travelSegThruStartOrder; }
  const uint16_t& travelSegThruEndOrder() const { return _travelSegThruEndOrder; }
  const bool& hasSideTrip() const { return _hasSideTrip; }
  bool needsReprice() const { return _needsReprice; }

  MoneyAmount assignSpecialPercentage(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      const uint16_t startOrder,
                                      const uint16_t endOrder);

  bool appliedPartial(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      const CarrierCode& govCarrier,
                      const uint16_t taxExemptStartOrder,
                      const uint16_t taxExemptEndOrder);

  bool appliedFare(PricingTrx& trx,
                   FarePath& farePath,
                   const FareMarket* fareMarket);

private:
  bool appliedMileage(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      const uint16_t startOrder,
                      const uint16_t endOrder);

  bool locateGateways(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TravelSeg& travelSeg,
                      uint16_t& _nonTaxablePrimaryTrvlSegStartOrder,
                      uint16_t& _nonTaxablePrimaryTrvlSegEndOrder,
                      uint16_t& _nonTaxableSecondaryTrvlSegStartOrder,
                      uint16_t& _nonTaxableSecondaryTrvlSegEndOrder);

  MoneyAmount addCAT12Surcharge(const FareMarket* fareMarket);

  MoneyAmount addStopOverSurcharge(const TravelSeg* tvlS);

  MoneyAmount addTransferSurcharge(const TravelSeg* tvlS);

  bool applyMinFarePlusUp(const FareMarket* fareMarket);

  bool isValidForValidatingCarrier(const PricingTrx& trx,
                                   const PaxTypeFare& ptf,
                                   const CarrierCode& vcxr) const;

  MoneyAmount _thruTotalFare = 0;
  MoneyAmount _taxablePartialFare = 0;
  MoneyAmount _specialPercentage = 0;

  uint32_t _partialLocalMiles = 0;
  uint32_t _partialThruMiles = 0;

  uint16_t _travelSegLocalStartOrder = 0;
  uint16_t _travelSegLocalEndOrder = 0;
  uint16_t _travelSegThruStartOrder = 0;
  uint16_t _travelSegThruEndOrder = 0;

  CurrencyCode _paymentCurrency = "";
  FareUsage* _fareUsage = nullptr;

  bool _hasSideTrip = false;
  bool _needsReprice = false;
};
} /* end tse namespace */
