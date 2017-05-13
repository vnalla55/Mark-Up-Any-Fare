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

#include "Taxes/LegacyTaxes/TaxSP43.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
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

TaxSP43::TaxSP43() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxSP43::~TaxSP43() {}

// ----------------------------------------------------------------------------
// Description:  validateTransit
// ----------------------------------------------------------------------------

bool
TaxSP43::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  if (taxResponse.farePath()->itin()->travelSeg().size() < 2)
    return true;

  TransitValidator transitValidator;
  Itin* itin = taxResponse.farePath()->itin();
  const AirSeg* airSeg;
  bool transit = false;

  _specialIndex = true;
  _travelSegSpecialTaxStartIndex = travelSegIndex;

  std::vector<TravelSeg*>::const_iterator travelSegIter;
  travelSegIter = itin->travelSeg().begin() + travelSegIndex;

  NationCode originNation = (*travelSegIter)->origin()->nation();
  NationCode destinationNation = (*travelSegIter)->destination()->nation();

  std::vector<TravelSeg*>::const_iterator travelSegToIter = travelSegIter;
  travelSegToIter++;
  travelSegIndex++;

  for (; travelSegToIter != itin->travelSeg().end(); travelSegToIter++, travelSegIndex++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegToIter);

    if (!airSeg)
      continue;

    if ((*travelSegIter) != itin->travelSeg().front() ||
        _travelSegSpecialTaxStartIndex != travelSegIndex - 1)
    {
      if ((*travelSegIter)->origin()->loc() == (*travelSegToIter)->destination()->loc())
        break;
    }

    destinationNation = (*travelSegToIter)->destination()->nation();

    transit = transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, travelSegIndex);

    TravelSeg* travelSeg = itin->travelSeg().begin()[travelSegIndex - 1];

    if (travelSeg->isForcedStopOver())
    {
      _travelSegSpecialTaxEndIndex = travelSegIndex - 1;
      return true;
    }

    if (!transit)
    {
      if (!travelSeg->isForcedConx())
      {
        _travelSegSpecialTaxEndIndex = travelSegIndex - 1;
        return true;
      }
    }

    if (originNation != destinationNation)
      break;
  }

  destinationNation = (*travelSegIter)->destination()->nation();

  if ((*travelSegIter) != itin->travelSeg().front())
  {
    std::vector<TravelSeg*>::const_iterator travelSegFromIter = travelSegIter;
    travelSegIndex = _travelSegSpecialTaxStartIndex;

    for (; travelSegFromIter != itin->travelSeg().begin(); travelSegIndex--)
    {
      travelSegFromIter--;

      airSeg = dynamic_cast<const AirSeg*>(*travelSegFromIter);

      if (!airSeg)
        continue;

      if ((*travelSegIter) != itin->travelSeg().back() ||
          _travelSegSpecialTaxStartIndex != travelSegIndex)
      {
        if ((*travelSegIter)->destination()->loc() == (*travelSegFromIter)->origin()->loc())
          break;
      }

      if ((*travelSegFromIter)->isForcedStopOver())
      {
        _travelSegSpecialTaxEndIndex = travelSegIndex + 1;
        return true;
      }

      if ((*travelSegFromIter)->isForcedConx())
        continue;

      originNation = (*travelSegFromIter)->origin()->nation();

      transit = transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, travelSegIndex);

      if (!transit)
      {
        _travelSegSpecialTaxEndIndex = travelSegIndex + 1;
        return true;
      }

      if (originNation != destinationNation)
        break;
    }
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

  return false;
}
