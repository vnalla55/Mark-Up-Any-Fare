// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Taxes/LegacyTaxes/TaxSP9800.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Common/TseUtil.h"

namespace tse
{

bool
TaxSP9800::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);
  if (!taxCodeReg.restrictionTransit().empty())
  {
    TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();
    if (restrictTransit.transitHours())
      locIt->setStopHours(restrictTransit.transitHours());
  }
  locIt->setSurfaceAsStop(true);

  locIt->toSegmentNo(startIndex);
  if (locIt->isInLoc1(taxCodeReg, trx) && (locIt->isStop() || locIt->isMirror()))
  {
    while (locIt->hasNext() && locIt->isNextSegAirSeg())
    {
      locIt->next();
      if (locIt->isInLoc2(taxCodeReg, trx) && (locIt->isStop() || locIt->isMirror()))
      {
        endIndex = locIt->prevSegNo();
        return true;
      }
    }
  }
  return false;
}

} /* end tse namespace */
