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

#include "Taxes/LegacyTaxes/TaxXV.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
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

const string
TaxXV::TAX_CODE_XD("XD");

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxXV::TaxXV() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxXV::~TaxXV() {}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
TaxXV::validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{
  Itin* itin = taxResponse.farePath()->itin();
  const AirSeg* airSeg;

  TravelSeg* travelSegFront = itin->travelSeg().front();
  TravelSeg* travelSegBack = itin->travelSeg().back();

  if (travelSegFront == travelSegBack)
    return true;

  std::vector<TravelSeg*>::const_iterator travelSegI = itin->travelSeg().begin() + travelSegIndex;

  if (*travelSegI == travelSegFront || *travelSegI == travelSegBack)
  {
    if (travelSegFront->origin()->loc() == travelSegBack->destination()->loc())
    {
      if (!travelSegBack->isStopOver(travelSegFront, SECONDS_PER_DAY))
        return true;

      std::vector<TravelSeg*>::const_iterator tsIter = itin->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tsToIter = itin->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tsEndIter = itin->travelSeg().end();

      std::string locCode = EMPTY_STRING();
      tsToIter++;
      //
      // Must be 3 segments for circle trip
      //
      if (*tsToIter != travelSegBack)
      {
        for (; tsToIter != tsEndIter; tsIter++, tsToIter++)
        {
          airSeg = dynamic_cast<const AirSeg*>(*tsIter);

          if (!airSeg)
            break;

          airSeg = dynamic_cast<const AirSeg*>(*tsToIter);

          if (!airSeg)
            break;

          if ((*tsToIter)->isStopOver((*tsIter), SECONDS_PER_DAY))
            break;

          if (locCode.empty())
          {
            locCode.append((*tsIter)->destination()->loc());
            continue;
          }

          string::size_type locCodeSize = locCode.find((*tsIter)->destination()->loc());

          if (locCodeSize != string::npos)
            break;

          if (*tsToIter == travelSegBack)
            return true;

          locCode.append((*tsIter)->destination()->loc());
        }
      }
    }
  }

  TransitValidator transitValidator;

  if (taxCodeReg.taxCode() == TAX_CODE_XD)
  {
    MirrorImage mirrorImage;

    bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);

    return transitValidator.validateTransitRestriction(
        trx, taxResponse, taxCodeReg, travelSegIndex, stopOver, _landToAirStopover);
  }

  uint16_t index = travelSegIndex;
  index++;
  travelSegI++;

  for (; travelSegI != itin->travelSeg().end(); travelSegI++, index++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      break;

    if (!transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, index))
    {
      if (travelSegFront == itin->travelSeg()[travelSegIndex])
        return true;

      break;
    }

    if (!LocUtil::isMexico(*airSeg->destination()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

      return false;
    }
  }

  //
  // Requires to be stop over in Mexico to Charge XV
  // MEXMTY/X-MTYLAS LASMTY/x-MTYMEX do not charge XV
  //

  travelSegI = itin->travelSeg().begin() + travelSegIndex;
  index = travelSegIndex;

  for (; (*travelSegI) != travelSegFront; index--)
  {
    travelSegI--;

    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      break;

    if (!transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, index))
      break;

    if (!LocUtil::isMexico(*airSeg->origin()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

      return false;
    }
  }
  //
  // Only charge at stopover segments
  //
  if (index == travelSegIndex)
    return true;

  travelSegI = itin->travelSeg().begin() + travelSegIndex;

  if ((*travelSegI) != travelSegFront)
  {
    travelSegI--;

    if ((*travelSegI)->isForcedStopOver())
      return true;
  }

  MirrorImage mirrorImage;

  if (mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex))
  {
    return true;
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

  return false;
}
