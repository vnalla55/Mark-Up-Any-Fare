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

#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxSP18.h"

#include <map>

namespace tse
{
class Itin;
class PricingTrx;
class TaxCodeReg;
class TaxResponse;

class TaxSP1800 : public TaxSP18
{
  friend class TaxAYTest;

public:
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

  bool validateRange(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t& startIndex,
                     uint16_t& endIndex) override;

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

  bool validateCarrierExemption(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t travelSegIndex) override;

  bool validateEquipmentExemption(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t travelSegIndex) override;

  bool validateFareClass(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex) override;

  bool validateCabin(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegIndex) override;

  bool validateTicketDesignator(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t travelSegIndex) override;

  bool validateSequence(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& travelSegStartIndex,
                        uint16_t& travelSegEndIndex,
                        bool checkSpn) override;

  bool validateFinalGenericRestrictions(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        TaxCodeReg& taxCodeReg,
                                        uint16_t& startIndex,
                                        uint16_t& endIndex) override;

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

  void adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

private:
  std::vector<TravelSeg*>::const_iterator
  findTravelSegInItin(TravelSeg* travelSeg, const Itin* itin);
  bool wasTravelSegProcessed(uint16_t travelSegIndex);
  void resetTravelSegProcessedInfo();
  bool isBetweenMirrors(TaxLocIterator* locIt);
  bool isExtendedMirror(TaxLocIterator* locIt);

  uint16_t _numberOfFees = 0;
  uint16_t _travelSegProcessedStartIndex = 0xFFFF;
  uint16_t _travelSegProcessedEndIndex = 0xFFFF;
};
} /* end tse namespace */
