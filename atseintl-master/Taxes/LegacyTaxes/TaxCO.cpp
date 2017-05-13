// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
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

#include "Taxes/LegacyTaxes/TaxCO.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Common/TseUtil.h"

namespace tse
{

bool
TaxCO::validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)
{
  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);

  locIt->toSegmentNo(startIndex);

  if (locIt->isInLoc1(taxCodeReg, trx) && locIt->loc()->loc() == "CLO" && locIt->isNextSegAirSeg())
  {
    locIt->next();
    if (locIt->loc()->loc() == "BOG" && !(locIt->isStop()))
    {
      if (locIt->isInLoc2(taxCodeReg, trx))
      {
        if (locIt->hasNext() && locIt->isNextSegAirSeg())
        {
          locIt->next();
          if (!locIt->isInLoc2(taxCodeReg, trx))
            return false;
        }
      }
      else if (locIt->hasNext() && locIt->isNextSegAirSeg())
      {
        locIt->next();
        if (locIt->isInLoc2(taxCodeReg, trx))
        {
          endIndex = startIndex + 1;
          _skipTransitValidation = true;
          return true;
        }
      }
    }
  }

  return Tax::validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}

bool
TaxCO::validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{
  if (_skipTransitValidation)
    return true;

  return Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
}
}
