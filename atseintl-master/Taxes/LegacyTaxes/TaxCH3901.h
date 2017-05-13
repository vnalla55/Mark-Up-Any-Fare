//---------------------------------------------------------------------------
//  Copyright Sabre 2008
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
// Tax special process 38 for Great Britian
//---------------------------------------------------------------------------

class TaxCH3901 : public Tax
{

public:
  static constexpr char YES = 'Y';
  static const DateTime specialFlightTransitDateLimit;

  TaxCH3901() = default;

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;

private:
  virtual bool
  validateSpecialFlightsTransit(const TravelSeg* pTravelSegFrom, const TravelSeg* pTravelSegTo);

  TaxCH3901(const TaxCH3901& map) = delete;
  TaxCH3901& operator=(const TaxCH3901& map) = delete;
};
} /* end tse namespace */
