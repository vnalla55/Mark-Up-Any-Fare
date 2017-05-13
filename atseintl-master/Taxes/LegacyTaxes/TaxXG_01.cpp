// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Taxes/LegacyTaxes/TaxXG_01.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Common/LocUtil.h"

namespace tse
{

bool
TaxXG_01::validateTripTypes(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            uint16_t& startIndex,
                            uint16_t& endIndex)
{
  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));

  if (isInternationalItin(locIt))
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);
    return false;
  }

  locIt->toSegmentNo(startIndex);

  while (locIt->hasNext())
  {
    if (locIt->isInLoc1(taxCodeReg, trx))
      break;
    locIt->next();
  }

  bool loc2Matched = false;

  while (locIt->hasNext())
  {
    locIt->next();
    if (locIt->isInLoc2(taxCodeReg, trx))
    {
      loc2Matched = true;
      break;
    }
  }

  if (loc2Matched)
  {
    startIndex = locIt->prevSegNo();
    if (!locIt->isPrevSegAirSeg())
    {
      while (locIt->hasNext())
      {
        locIt->next();
        if (locIt->isPrevSegAirSeg())
        {
          startIndex = locIt->prevSegNo();
          break;
        }
      }
    }
    endIndex = startIndex;
    return true;
  }
  else
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);
    return false;
  }
}

bool
TaxXG_01::isInternationalItin(TaxLocIterator* locIt)
{
  locIt->toFront();

  while (1)
  {
    const Loc& loc = *(locIt->loc());

    if (LocUtil::isUSTerritoryOnly(loc) || LocUtil::isHawaii(loc))
      return true;

    if (!LocUtil::isUS(loc) && !LocUtil::isCanada(loc) && !LocUtil::isStPierreMiquelon(loc))
      return true;

    if (!locIt->hasNext())
      break;
    locIt->next();
  }

  return false;
}
}
