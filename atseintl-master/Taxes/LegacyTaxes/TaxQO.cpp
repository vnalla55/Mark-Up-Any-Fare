// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with/
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxQO.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
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

TaxQO::TaxQO() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxQO::~TaxQO() {}

// ----------------------------------------------------------------------------
// Description:  validateTransit
// ----------------------------------------------------------------------------

bool
TaxQO::validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{

  TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

  if (travelSegIndex >= 1)
  {
    TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 1];

    if (travelSegFrom->segmentType() == Arunk)
    {
      TravelSeg* travelSegFromPrev =
          taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 2];

      if ((travelSegFromPrev->offMultiCity() == travelSegTo->boardMultiCity()) &&
          (travelSegFromPrev->destAirport() != travelSegTo->origAirport()))
      {
        return true;
      }
    }
    else if ((travelSegFrom->offMultiCity() == travelSegTo->boardMultiCity()) &&
             (travelSegFrom->destAirport() != travelSegTo->origAirport()))
    {
      return true;
    }
  }

  MirrorImage mirrorImage;

  bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);

  TransitValidator transitValidator;

  return transitValidator.validateTransitRestriction(
      trx, taxResponse, taxCodeReg, travelSegIndex, stopOver, _landToAirStopover);
}
