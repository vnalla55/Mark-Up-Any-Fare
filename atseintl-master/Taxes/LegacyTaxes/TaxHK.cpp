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

#include "Taxes/LegacyTaxes/TaxHK.h"
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

TaxHK::TaxHK() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxHK::~TaxHK() {}

// ----------------------------------------------------------------------------
// Description:  validateCarrierExemption
// ----------------------------------------------------------------------------

bool
TaxHK::validateCarrierExemption(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t travelSegIndex)
{
  return true;
}

// ----------------------------------------------------------------------------
// Description:  validateTransit
// ----------------------------------------------------------------------------

bool
TaxHK::validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{
  std::vector<TravelSeg*>::const_iterator travelSegI;
  std::vector<TravelSeg*>::const_iterator travelSegFromI;

  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin() + travelSegIndex;

  if (travelSegI == taxResponse.farePath()->itin()->travelSeg().begin())
    return true;

  const AirSeg* airSeg = nullptr;
  const AirSeg* airSeg2 = nullptr;

  travelSegFromI = travelSegI;

  for (; travelSegFromI != taxResponse.farePath()->itin()->travelSeg().begin();)
  {
    travelSegFromI--;

    airSeg2 = dynamic_cast<const AirSeg*>(*travelSegFromI);

    if (airSeg2)
      break;
  }

  if ((*travelSegFromI)->isForcedStopOver())
    return true;

  if ((*travelSegFromI)->isForcedConx())
    return false;

  if ((*travelSegI)->departureDT().day() == (*travelSegFromI)->arrivalDT().day() &&
      (*travelSegI)->departureDT().month() == (*travelSegFromI)->arrivalDT().month())
  {
    if ((*travelSegI)->origin()->nation() == (*travelSegFromI)->destination()->nation())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

      return false;
    }
  }

  airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

  if (!airSeg || !airSeg2)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

    return false;
  }

  DateTime nextDay = (*travelSegFromI)->arrivalDT().addDays(1);
  bool possibleExemption = false;

  std::vector<TaxExemptionCarrier>::iterator validCxrIter = taxCodeReg.exemptionCxr().begin();
  std::vector<TaxExemptionCarrier>::iterator validCxrEndIter = taxCodeReg.exemptionCxr().end();

  for (; validCxrIter != validCxrEndIter; validCxrIter++)
  {
    if ((*validCxrIter).carrier() != airSeg2->marketingCarrierCode())
      continue;

    possibleExemption = true;
    break;
  }

  if (possibleExemption)
  {
    validCxrIter = taxCodeReg.exemptionCxr().begin();

    for (; validCxrIter != validCxrEndIter; validCxrIter++)
    {
      if ((*validCxrIter).carrier() != airSeg->marketingCarrierCode())
        continue;

      if ((*validCxrIter).flight1() != airSeg->flightNumber() && (*validCxrIter).flight1() != 0)
        continue;

      if ((*travelSegI)->departureDT().day() == nextDay.day() &&
          (*travelSegI)->departureDT().month() == nextDay.month())
      {
        TaxDiagnostic::collectErrors(
            trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

        return false;
      }
    }
  }

  MirrorImage mirrorImage;

  bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);

  TransitValidator transitValidator;

  return transitValidator.validateTransitRestriction(
      trx, taxResponse, taxCodeReg, travelSegIndex, stopOver, _landToAirStopover);
}
