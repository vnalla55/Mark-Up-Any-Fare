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

#include "Taxes/LegacyTaxes/TaxCH3901.h"

#define FROM_SPECIAL_FLIGHT 933
#define TO_SPECIAL_FLIGHT 934
#define FROM_TO_SPECIAL_CARRIER "KE"

namespace tse
{
class TravelSeg;

class TaxCH3902 : public TaxCH3901
{
  friend class TaxCH3902Test;

public:
  TaxCH3902() = default;

private:
  bool validateSpecialFlightsTransit(const TravelSeg* pTravelSegFrom,
                                     const TravelSeg* pTravelSegTo) override;

  TaxCH3902(const TaxCH3902&) = delete;
  TaxCH3902& operator=(const TaxCH3902&) = delete;
};
} /* end tse namespace */
