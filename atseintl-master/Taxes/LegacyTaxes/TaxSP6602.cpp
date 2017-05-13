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

#include "Taxes/LegacyTaxes/TaxSP6602.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Common/TseUtil.h"
#include "Common/FallbackUtil.h"

namespace tse
{

bool
TaxSP6602::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));

  locIt->setSkipHidden(true);

  locIt->toSegmentNo(startIndex);

  if (!(locIt->isInLoc1(taxCodeReg, trx)))
    return false;

  locIt->next();

  if (!(locIt->isInLoc2(taxCodeReg, trx)))
    return false;

  if (locIt->isStop())
    return true;

  locIt->previous();

  bool isStopBeforeMex = false;
  if (locIt->isStop())
    isStopBeforeMex = true;

  NationCode startCountry = locIt->loc()->nation();

  while (locIt->hasNext())
  {
    locIt->next();
    if (locIt->loc()->nation() != "MX")
    {
      if (!(locIt->isPrevSegAirSeg()))
        return true;

      if (locIt->loc()->nation() == startCountry && isStopBeforeMex)
        return true;
      else
        return false;
    }
  }
  return true;
}
}
