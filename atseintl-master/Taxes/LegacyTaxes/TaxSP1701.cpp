// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Taxes/LegacyTaxes/TaxSP1701.h"
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
TaxSP1701::isUsPrVi(const Loc& loc)
{
  static StateCode pr_vi[] = { PUERTORICO, VIRGIN_ISLAND };

  return (LocUtil::isUS(loc) &&
          (!LocUtil::isUSTerritoryOnly(loc) || TseUtil::isMember(loc.state(), pr_vi)));
}

bool
TaxSP1701::validateTripTypes(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             TaxCodeReg& taxCodeReg,
                             uint16_t& startIndex,
                             uint16_t& endIndex)
{
  bool isNotUsPrViLocation = false;

  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));

  locIt->toSegmentNo(startIndex);

  if (!(locIt->isInLoc1(taxCodeReg, trx) && locIt->isNextSegAirSeg()))
    return false;

  while (locIt->hasPrevious())
  {
    locIt->previous();
    if (locIt->isInLoc2(taxCodeReg, trx))
    {
      if (UNLIKELY(!isUsPrVi(*(locIt->loc()))))
        isNotUsPrViLocation = true;
      break;
    }
  }

  locIt->toSegmentNo(startIndex);

  while (locIt->hasNext())
  {
    locIt->next();
    if (!locIt->isHidden() || locIt->isInLoc2(taxCodeReg, trx))
      break;
  }

  if (!(locIt->isInLoc2(taxCodeReg, trx)))
    return false;

  if (!isUsPrVi(*(locIt->loc())))
    isNotUsPrViLocation = true;

  if (locIt->isHidden())
    _hiddenOffAirport = locIt->loc()->loc();

  while (locIt->hasPrevious())
  {
    locIt->previous();
    if (LIKELY(locIt->isInLoc1(taxCodeReg, trx)))
    {
      if (!isNotUsPrViLocation || locIt->isStop() || locIt->isTurnAround() ||
          (!locIt->hasPrevious()))
      {
        endIndex = startIndex;
        return true;
      }
    }
    else
    {
      return false;
    }
  }
  return false;
}
}
