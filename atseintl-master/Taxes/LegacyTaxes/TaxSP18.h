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
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

//---------------------------------------------------------------------------
// Tax special process 16 for US Security Fee
//---------------------------------------------------------------------------

class TaxSP18 : public Tax
{
  friend class TaxAYTest;

public:
  bool validateSequence(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& travelSegStartIndex,
                        uint16_t& travelSegEndIndex,
                        bool checkSpn = false) override;

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

  void taxCreate(PricingTrx& trx,
                 TaxResponse& taxResponse,
                 TaxCodeReg& taxCodeReg,
                 uint16_t travelSegStartIndex,
                 uint16_t travelSegEndIndex) override;

  void adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

protected:
  bool hasTransfer(const std::vector<TravelSeg*>::const_iterator& travelSegCurrent,
                   const std::vector<TravelSeg*>::const_iterator& travelSegNext,
                   const std::vector<TravelSeg*>::const_iterator& travelSegEnd,
                   const GeoTravelType& geoTravelType) const;

  static constexpr char ONEWAY_TRIP = 'O';
  static constexpr char ROUND_TRIP = 'R';
  uint16_t _segmentOrderRoundTrip = 0;
  uint16_t _segmentOrderStopOver = 0;
};
} /* end tse namespace */
