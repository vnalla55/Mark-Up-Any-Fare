//---------------------------------------------------------------------------
//  Copyright Sabre 2012
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

#include <map>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "Taxes/Common/LocRestrictionValidator3601.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class FareMarket;
class FareUsage;
class Itin;
class PricingTrx;
class RepricingTrx;
class TaxResponse;
class TaxCodeReg;
class PartialTaxableFare;

class TaxUS1_01 : public Tax
{
  friend class TaxUS1_01_findApplicableSegs;
  friend class TaxUS1_01_taxCreate;
  friend class TaxUS1_01_calculatePartAmountTest;
  friend class TaxUS1_01_locateOpenJaw;

private:
  int16_t _latestEndIndex;
  bool _fareBreaksFound;
  bool _soldInUS;
  bool _openJaw;
  bool _allUS;
  bool _allUSScanned;
  std::map<uint16_t, FareUsage*> _fareBreaks;
  LocRestrictionValidator3601 _locRestrictionValidator;

  virtual LocRestrictionValidator3601& locRestrictionValidator()
  {
    return _locRestrictionValidator;
  }

  bool doReprice(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 const PaxTypeFare* paxTypeFare,
                 const std::vector<TravelSeg*>& travelSegsEx,
                 const std::set<CarrierCode>& govCxrs,
                 PartialTaxableFare& partialFareLocator);

  RepricingTrx* getRepricingTrx(PricingTrx& trx,
                                const std::vector<TravelSeg*>& tvlSeg,
                                FMDirection fmDirectionOverride);

  bool getNetAmountForLCT(PricingTrx& trx,
                          FareUsage* fareUsage,
                          TaxResponse& taxResponse,
                          MoneyAmount& netAmount);

public:
  TaxUS1_01()
    : _latestEndIndex(-1),
      _fareBreaksFound(false),
      _soldInUS(true),
      _openJaw(false),
      _allUS(false),
      _allUSScanned(false)
  {
  }

  virtual ~TaxUS1_01() {}

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override
  {
    return true;
  }

  bool validateFromTo(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& startIndex,
                      uint16_t& endIndex)
  {
    return true;
  }

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override
  {
    return true;
  }

  virtual bool validateEquipmentExemption(PricingTrx& trx,
                                          TaxResponse& taxResponse,
                                          TaxCodeReg& taxCodeReg,
                                          uint16_t travelSegIndex) override
  {
    return true;
  }

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

  TravelSeg* previousAirSeg(std::vector<TravelSeg*>::const_iterator segI,
                            const std::vector<TravelSeg*>& tsVector);

  TravelSeg*
  nextAirSeg(std::vector<TravelSeg*>::const_iterator segI, const std::vector<TravelSeg*>& tsVector);

  bool isUS(const Loc* loc);

  bool isAllUS(FarePath& farePath);

  virtual bool
  validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

  virtual bool findApplicableSegs(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  int16_t startIndex,
                                  uint16_t& endIndex,
                                  bool fallbackUSSurface);

  virtual MoneyAmount calculatePartAmount(PricingTrx& trx,
                                          TaxResponse& taxResponse,
                                          TaxCodeReg& taxCodeReg,
                                          uint16_t startIndex,
                                          uint16_t endIndex,
                                          uint16_t fareBreakEnd,
                                          FareUsage& fareUsage);

  virtual MoneyAmount calculatePercentage(PricingTrx& trx,
                                          TaxResponse& taxResponse,
                                          TaxCodeReg& taxCodeReg,
                                          uint16_t startIndex,
                                          uint16_t endIndex);

  virtual void convertCurrency(MoneyAmount& taxableAmount,
                               PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg);

  virtual bool substractPartialFare(PricingTrx& trx,
                                    TaxResponse& taxResponse,
                                    TaxCodeReg& taxCodeReg,
                                    const MoneyAmount& totalFareAmount,
                                    MoneyAmount& taxableAmount,
                                    FareUsage& fareUsage,
                                    const std::vector<TravelSeg*>& travelSegsEx);

  virtual uint32_t calculateMiles(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  std::vector<TravelSeg*>& travelSegs,
                                  const Loc& origin,
                                  const Loc& destination);

  bool locateOpenJaw(PricingTrx& trx, TaxResponse& taxResponse);

  void checkEquipment(MoneyAmount& taxableAmount,
                      const uint16_t startIndex,
                      const uint16_t endIndex,
                      PricingTrx& trx,
                      const Itin& itin);

  virtual void
  applyTaxOnTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;
};
}
