// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
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

#include "Taxes/LegacyTaxes/TaxSP4102.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"

namespace tse
{

bool
TaxSP4102::loc2ExcludeTransit(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t travelSegIndex)
{
  _specialIndex = false;
  _travelSegSpecialTaxEndIndex = travelSegIndex;
  _travelSegSpecialTaxStartIndex = travelSegIndex;

  if (travelSegIndex == 0)
  { // First Board Point applies..
    _specialIndex = true;
    return true;
  }

  TransitValidator transitValidator;
  TravelSeg* travelSegFrom;
  TravelSeg* travelSegTo;
  const AirSeg* airSeg;
  bool transit = false;

  MirrorImage mirrorImage;

  if (mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex))
    return true;

  for (; travelSegIndex;)
  {
    TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

    transit = false;

    if (!travelSeg->isOpenWithoutDate())
    {
      transit = transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, travelSegIndex);
    }

    travelSegIndex--;

    travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

    airSeg = dynamic_cast<const AirSeg*>(travelSegFrom);

    if (!airSeg)
    {
      _specialIndex = true;
      _travelSegSpecialTaxStartIndex = travelSegIndex + 1;
      return true;
    }

    if (travelSegFrom->isForcedStopOver())
      transit = false;

    if (travelSegFrom->isForcedConx())
      transit = true;

    if (!transit)
    {
      _specialIndex = true;
      _travelSegSpecialTaxStartIndex = travelSegIndex + 1;
      return true;
    }

    travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex + 1];

    if (travelSegFrom->origin()->nation() != travelSegTo->origin()->nation())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

      return false;
    }

    if (travelSegFrom == taxResponse.farePath()->itin()->travelSeg().front())
    {
      _specialIndex = true;
      _travelSegSpecialTaxStartIndex = travelSegIndex;
      return true;
    }
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);
  return false;
}
}
