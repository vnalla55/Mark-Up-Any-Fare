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

#include "Taxes/LegacyTaxes/TaxSP9700.h"
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
TaxSP9700::isInternationalCT(TaxLocIterator* locIt)
{
  locIt->toFront();
  const NationCode& startNation = locIt->loc()->nation();
  if (startNation.equalToConst("GR"))
    return false;
  locIt->toBack();
  const NationCode& endNation = locIt->loc()->nation();
  if (startNation != endNation)
    return false;

  uint16_t grCount = 0;

  locIt->toFront();
  while (locIt->hasNext())
  {
    locIt->next();

    if (locIt->loc()->nation().equalToConst("GR"))
      ++grCount;

    if (locIt->loc()->nation() != "GR" && locIt->loc()->nation() != startNation)
    {
      if (locIt->hasNext())
        locIt->next();
      if (locIt->hasNext())
        return false;
    }
    if (locIt->loc()->nation().equalToConst("GR") && (locIt->isStop() || locIt->isMirror()))
      return false;
  }

  if (grCount >= 2)
    return true;
  else
    return false;
}

bool
TaxSP9700::isDomesticCT(TaxLocIterator* locIt)
{
  locIt->toFront();
  if (locIt->loc()->nation() != "GR")
    return false;
  const Loc* startLoc = locIt->loc();

  locIt->toBack();
  if (locIt->loc() != startLoc)
    return false;

  locIt->toFront();
  while (locIt->hasNext())
  {
    locIt->next();
    if (locIt->loc()->nation() != "GR")
      return false;
    if (locIt->hasNext() && (locIt->isStop() || locIt->isMirror()))
      return false;
  }
  return true;
}

bool
TaxSP9700::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  if (Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex))
    return true;

  TravelSeg* ts = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

  if (ts->origin()->nation() != "GR")
    return false;

  if (ts->destination()->nation() != "GR")
  {
    TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
    return isInternationalCT(locIt);
  }
  else
  {
    TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
    if (isDomesticCT(locIt))
    {
      locIt->toSegmentNo(travelSegIndex);
      return locIt->isTurnAround();
    }
  }

  return false;
}

} /* end tse namespace */
