// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
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

#include "Taxes/LegacyTaxes/TaxWN.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/Common/TaxUtility.h"

using namespace tse;

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxWN::TaxWN() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxWN::~TaxWN() {}

// ----------------------------------------------------------------------------
// Description:  validateTransit
// ----------------------------------------------------------------------------
bool
TaxWN::validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{

  bool isValidTransit = Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

  if (travelSegIndex >= 1)
  {
    TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 1];

    if (taxUtil::isTransitSeq(taxCodeReg))
    {
      if (travelSegFrom->isForcedConx())
        isValidTransit = true;
      else if (travelSegFrom->isForcedStopOver())
        isValidTransit = false;
      else if (MirrorImage().isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex))
        isValidTransit = false;
    }
  }

  return isValidTransit;
}
