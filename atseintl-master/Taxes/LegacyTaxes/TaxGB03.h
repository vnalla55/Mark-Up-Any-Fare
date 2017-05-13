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
#include "Taxes/LegacyTaxes/Tax.h"

#include <vector>

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class Itin;
class Loc;

class TaxGB03 : public Tax
{
  friend class TaxGB03Test;

public:
  TaxGB03();
  void preparePortionOfTravelIndexes(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     TaxCodeReg& taxCodeReg) override;
  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;
  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;
  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;
  bool validateCabin(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegIndex) override;

protected:
  uint16_t findEndOfConnectedJourney(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     TaxCodeReg& taxCodeReg,
                                     uint16_t startIndex,
                                     bool skipIntSurface = false) const;

  bool validateCabins(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& startIndex,
                      uint16_t& endIndex) const;

  bool onValidLoc1(PricingTrx& trx,
                   TaxResponse& taxResponse,
                   TaxCodeReg& taxCodeReg,
                   uint16_t& startIndex,
                   uint16_t& endIndex) const;
  bool onInvalidLoc1(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t& startIndex,
                     uint16_t& endIndex) const;

  uint16_t findMirrorImage(PricingTrx& trx,
                           const Itin* itin,
                           const uint16_t& startIndex,
                           const uint16_t& endIndex) const;
  uint16_t findIntSurface(const Itin* itin, uint16_t& startIndex, uint16_t& endIndex) const;
  bool isRoundTrip(const Itin* itin, uint16_t& startIndex, uint16_t& endIndex) const;
  uint16_t findTurnaroundPt(const Itin* itin, uint16_t& startIndex, uint16_t& endIndex) const;
  bool isUKJourney(const Itin& itin, uint16_t startIndex, uint16_t endIndex) const;

  bool isGeoMatch(const Loc& aLoc, LocTypeCode& locType, LocCode& loc, Indicator& locExclInd) const;
  bool validateLoc1(const Loc*& loc, TaxCodeReg& taxCodeReg) const;
  bool validateLoc2(const Loc*& loc, TaxCodeReg& taxCodeReg) const;

  bool isIntStopOver(TravelSeg* travelSegFrom, TravelSeg* travelSegTo) const;
  bool isDomStopOver(TravelSeg* travelSegFrom, TravelSeg* travelSegTo) const;
  bool isStopOver(TravelSeg* travelSegFrom, TravelSeg* travelSegTo, bool isDomestic = false) const;
  bool isUKSeg(TravelSeg* travelSeg) const;

  bool isDomSeg(TravelSeg* travelSeg) const;
  bool isDomOrUKArival(TravelSeg* travelSeg) const;
  bool isUKDeparture(TravelSeg* travelSeg) const;

  uint16_t findMirror(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t startIndex) const;
  bool isConnNotDomItin(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& startIndex,
                        uint16_t& endIndex) const;

private:
  bool _connectedJourney = false;
  uint16_t _nLowCabinTaxCodeSPN;
  uint16_t _nHighCabinTaxCodeSPN;
};
} /* end tse namespace */
