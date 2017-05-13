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

#include "Taxes/LegacyTaxes/TaxSP60.h"
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

TaxSP60::TaxSP60() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxSP60::~TaxSP60() {}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------

bool
TaxSP60::validateTransit(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex)
{
  MirrorImage mirrorImage;

  bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);

  TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

  if ((travelSegTo == taxResponse.farePath()->itin()->travelSeg().front()) || (stopOver))
    return true;

  TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 1];

  if (travelSegFrom->origin()->nation() != travelSegFrom->destination()->nation())
  {
    TransitValidator transitValidator;

    bool transit =
        transitValidator.validateTransitTime(trx, taxResponse, taxCodeReg, travelSegIndex);

    if (transit)
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRANSIT_RESTRICTION, Diagnostic820);

      return false;
    }
  }

  return true;
}
