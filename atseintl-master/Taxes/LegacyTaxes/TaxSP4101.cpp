// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/LegacyTaxes/TaxSP4101.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"
#include "Common/Vendor.h"

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxSP4101::TaxSP4101() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxSP4101::~TaxSP4101() {}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
TaxSP4101::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{
  bool retVal;

  if (taxCodeReg.loc1ExclInd() == YES)
  {
    retVal = loc1ExcludeTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
  }
  else
  {
    retVal = loc2ExcludeTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
  }

  if (!taxCodeReg.restrictionTransit().empty())
  {
    TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();

    if (restrictTransit.transitTaxonly() == YES)
      retVal = (!retVal);
  }

  return retVal;
}

// ----------------------------------------------------------------------------
// Description:  loc1ExcludeTransit
// ----------------------------------------------------------------------------

bool
TaxSP4101::loc1ExcludeTransit(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t travelSegIndex)
{
  TransitValidator transitValidator;
  const AirSeg* airSeg;
  bool transit = false;

  _specialIndex = true;
  _travelSegSpecialTaxStartIndex = travelSegIndex;

  uint16_t travelSegSize = taxResponse.farePath()->itin()->travelSeg().size();

  if (travelSegSize < 2)
    return true;

  MirrorImage mirrorImage;

  if (mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex))
    return true;

  TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];
  TravelSeg* travelSegFrom;

  travelSegIndex++;

  for (; travelSegIndex < travelSegSize; travelSegIndex++)
  {
    travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 1];
    travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

    airSeg = dynamic_cast<const AirSeg*>(travelSegTo);

    if (!airSeg)
      return true;

    if (travelSegFrom->isForcedConx())
      continue;

    if (travelSegFrom->origin()->nation() != travelSegTo->origin()->nation())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

      return false;
    }

    transit = transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, travelSegIndex);

    if (!transit)
      break;
  }

  if ((transit) && (travelSegTo->origin()->nation() != travelSegTo->destination()->nation()))
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

    return false;
  }

  _travelSegSpecialTaxEndIndex = travelSegIndex - 1;

  return true;
}

// ----------------------------------------------------------------------------
// Description:  loc2ExcludeTransit
// ----------------------------------------------------------------------------

bool
TaxSP4101::loc2ExcludeTransit(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t travelSegIndex)
{
  if (!travelSegIndex) // First Board Point applies..
    return true;

  TransitValidator transitValidator;
  TravelSeg* travelSegFrom;
  TravelSeg* travelSegTo;
  const AirSeg* airSeg;
  bool transit = false;

  if (!_specialIndex)
    _travelSegSpecialTaxEndIndex = travelSegIndex;

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
