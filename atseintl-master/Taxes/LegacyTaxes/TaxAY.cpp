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

#include "Taxes/LegacyTaxes/TaxAY.h"
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
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxAY::TaxAY() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxAY::~TaxAY() {}

// ----------------------------------------------------------------------------
// Description:  validateSequence
// ----------------------------------------------------------------------------

bool
TaxAY::validateSequence(PricingTrx& trx,
                        TaxResponse& taxResponse,
                        TaxCodeReg& taxCodeReg,
                        uint16_t& travelSegStartIndex,
                        uint16_t& travelSegEndIndex,
                        bool checkSpn)
{

  return Tax::validateSequence(
      trx, taxResponse, taxCodeReg, travelSegStartIndex, travelSegEndIndex, Tax::CHECK_SPN);
}

bool
TaxAY::validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)
{
  Itin* itin = taxResponse.farePath()->itin();
  TravelSeg* travelSeg = itin->travelSeg()[startIndex];

  if (hiddenCityUS(trx, travelSeg))
  {
    endIndex = startIndex;
    return true;
  }

  TaxDiagnostic::collectErrors(
      trx, taxCodeReg, taxResponse, TaxDiagnostic::TRIP_TYPES_FROM_TO, Diagnostic820);

  return false;
}

// ----------------------------------------------------------------------------
// Description:  hiddenCityUS
// ----------------------------------------------------------------------------

bool
TaxAY::hiddenCityUS(PricingTrx& trx, TravelSeg* travelSeg)
{
  const AirSeg* airSeg;

  airSeg = dynamic_cast<const AirSeg*>(travelSeg);

  if (!airSeg)
    return false;

  if (airSeg->hiddenStops().empty())
    return false;

  if (airSeg->marketingCarrierCode() == CARRIER_9B)
    return false;

  if (airSeg->origin()->nation() == airSeg->destination()->nation())
    return false;

  std::vector<const Loc*>::const_iterator locI;

  for (locI = airSeg->hiddenStops().begin(); locI != airSeg->hiddenStops().end(); locI++)
  {
    if (LocUtil::isUS((**locI)))
      return true;
  }
  return false;
}
