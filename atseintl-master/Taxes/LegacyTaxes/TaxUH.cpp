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

#include "Taxes/LegacyTaxes/TaxUH.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
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
#include "Common/FallbackUtil.h"

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxUH::TaxUH() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxUH::~TaxUH() {}

bool
TaxUH::validateFromTo(PricingTrx& trx,
                      TaxResponse& taxResponse,
                      TaxCodeReg& taxCodeReg,
                      uint16_t& startIndex,
                      uint16_t& endIndex)

{
  if (taxCodeReg.loc1Type() == LOCTYPE_NONE)
    return true;

  uint16_t stopOverIndex, index;
  bool foundStart = false;
  bool geoMatch;

  stopOverIndex = startIndex;

  std::vector<TravelSeg*>::iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin() + stopOverIndex;

  endIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1;

  for (index = stopOverIndex; index <= endIndex; ++index, ++travelSegI)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    geoMatch = LocUtil::isInLoc(*airSeg->origin(),
                                taxCodeReg.loc1Type(),
                                taxCodeReg.loc1(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((geoMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
        (geoMatch && taxCodeReg.loc1ExclInd() != TAX_EXCLUDE) ||
        (!geoMatch && taxCodeReg.loc1ExclInd() == TAX_EXCLUDE))
    {
      startIndex = index;
      endIndex = index; // From/To and Between must match on the same travelSeg
      foundStart = true;
      break;
    }
  } // end of ItinItem Index loop

  // *** might need to check hiddenstop??? --> not populated/bad address as of - 10/5/04
  if (foundStart) // We have found the Loc1 -- now check loc2
  {
    if (taxCodeReg.loc2Type() == LOCTYPE_NONE)
      return true;

    travelSegI = taxResponse.farePath()->itin()->travelSeg().begin() + startIndex;

    geoMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((geoMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) || // can't check ExclInd for Zone
        (geoMatch && taxCodeReg.loc2ExclInd() != TAX_EXCLUDE) ||
        (!geoMatch && taxCodeReg.loc2ExclInd() == TAX_EXCLUDE))
    {
      return true; // We found a tax -- return it and expect to get called again
      // to check the rest of the itinItems.
    }
  } // foundStart && !foundStop)

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic819);

  return false; // We fail this tax.
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TripTypesValidator::isTaxStopOver
//
// Description:  This function will validate TAX_WITHIN_WHOLLY
//
// @param  PricingTrx - Transaction object
// @param  TaxCodeReg - TaxCodeReg object
// @param  startIndex - first TrvelSeg to serach from
//
// @return uint16_t - stopover point;
//
// </PRE>
// ----------------------------------------------------------------------------
