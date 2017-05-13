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
#pragma once

#include "Taxes/LegacyTaxes/Tax.h"

#include <boost/noncopyable.hpp>

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;
class TravelSeg;

class TaxSP9500 : public Tax, boost::noncopyable
{

public:
  TaxSP9500() = default;
  virtual ~TaxSP9500() = default;

  bool validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex) override;
  bool isMultiCityMirrorImage(TravelSeg* pFrom, TravelSeg* pTo);
  bool isMultiCityStopOver(TravelSeg* pFrom, TravelSeg* pTo);
};

} /* end tse namespace */

