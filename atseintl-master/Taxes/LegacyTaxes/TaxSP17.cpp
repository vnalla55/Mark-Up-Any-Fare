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

#include "Taxes/LegacyTaxes/TaxSP17.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
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

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxSP17::TaxSP17() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxSP17::~TaxSP17() {}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
TaxSP17::validateTripTypes(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t& startIndex,
                           uint16_t& endIndex)
{
  if (!hiddenCityUS(trx, taxResponse))
  {
    TripTypesValidator tripTypesValidator;

    return tripTypesValidator.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
  }

  const AirSeg* airSeg;
  bool locMatch;

  std::vector<TravelSeg*>::iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin() + startIndex;

  endIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1;

  for (uint16_t index = startIndex; travelSegI != taxResponse.farePath()->itin()->travelSeg().end();
       index++, travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    locMatch = LocUtil::isInLoc(*airSeg->origin(),
                                taxCodeReg.loc1Type(),
                                taxCodeReg.loc1(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && taxCodeReg.loc1Type() == LOCTYPE_ZONE) ||
        (locMatch && taxCodeReg.loc1ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc1ExclInd() == YES))
    {
      startIndex = index;
      endIndex = index;
      break;
    }
  }

  if (travelSegI != taxResponse.farePath()->itin()->travelSeg().end())
  {
    locMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                taxCodeReg.loc2Type(),
                                taxCodeReg.loc2(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
        (locMatch && taxCodeReg.loc2ExclInd() != YES) ||
        (!locMatch && taxCodeReg.loc2ExclInd() == YES))
      return true;

    std::vector<const Loc*>::iterator locI = (*travelSegI)->hiddenStops().begin();

    for (; locI != (*travelSegI)->hiddenStops().end(); locI++)
    {
      locMatch = LocUtil::isInLoc(*(*locI),
                                  taxCodeReg.loc2Type(),
                                  taxCodeReg.loc2(),
                                  Vendor::SABRE,
                                  MANUAL,
                                  LocUtil::TAXES,
                                  GeoTravelType::International,
                                  EMPTY_STRING(),
                                  trx.getRequest()->ticketingDT());

      if ((locMatch && taxCodeReg.loc2Type() == LOCTYPE_ZONE) ||
          (locMatch && taxCodeReg.loc2ExclInd() != YES) ||
          (!locMatch && taxCodeReg.loc2ExclInd() == YES))
        return true;
    }
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic820);

  return false;
}

// ----------------------------------------------------------------------------
// Description:  hiddenCityUS
// ----------------------------------------------------------------------------

bool
TaxSP17::hiddenCityUS(PricingTrx& trx, TaxResponse& taxResponse)
{
  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();
  std::vector<const Loc*>::iterator locI;
  const AirSeg* airSeg;

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    if (airSeg->marketingCarrierCode() == CARRIER_9B)
      continue;

    if (airSeg->origin()->nation() == airSeg->destination()->nation())
      continue;

    for (locI = (*travelSegI)->hiddenStops().begin(); locI != (*travelSegI)->hiddenStops().end();
         locI++)
    {
      if (LocUtil::isUS((**locI)))
        return true;
    }
  }
  return false;
}
