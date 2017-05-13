// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/LegacyTaxes/TaxCH3901.h"
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

const tse::DateTime
TaxCH3901::specialFlightTransitDateLimit(2010, 3, 27);

bool
TaxCH3901::validateSpecialFlightsTransit(const TravelSeg* pTravelSegFrom,
                                         const TravelSeg* pTravelSegTo)
{
  return true;
}

bool
TaxCH3901::validateTransit(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           TaxCodeReg& taxCodeReg,
                           uint16_t travelSegIndex)
{

  TransitValidator transitValidator;

  bool transit =
      transitValidator.validateTransitRestriction(trx, taxResponse, taxCodeReg, travelSegIndex);

  TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

  if (travelSegTo == taxResponse.farePath()->itin()->travelSeg().front()) // Start of Itinerary has
                                                                          // no Train Transit....
    return transit;

  if (taxCodeReg.restrictionTransit().empty())
    return transit;

  TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 1];

  if (transit &&
      trx.travelDate() <= specialFlightTransitDateLimit && // this is exception... it should be
                                                           // expired after March 2010, but since we
                                                           // can't do it via GUI it's hardcoded
      !validateSpecialFlightsTransit(travelSegFrom, travelSegTo))
    return false;

  TaxRestrictionTransit& restrictionTransit = taxCodeReg.restrictionTransit().front();

  MirrorImage mirrorImage;
  bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);

  if ((travelSegTo->segmentType() == Open || travelSegFrom->segmentType() == Open) &&
      !travelSegFrom->isForcedConx())
    stopOver = true;

  if (strncmp(taxCodeReg.taxCode().c_str(), "WN", 2) == 0)
  {
    if (stopOver)
    {
      if (!travelSegTo->isStopOver(travelSegFrom, 172800))
      {
        return true;
      }
    }
  }

  if (YES == restrictionTransit.transitTaxonly())
  {
    if (stopOver)
    {
      return false;
    }
  }
  else
  {
    if (stopOver)
      return true;
  }

  const AirSeg* airSeg;

  airSeg = dynamic_cast<const AirSeg*>(travelSegFrom);

  if (restrictionTransit.transitTaxonly() != YES)
  {
    if (!airSeg || travelSegFrom->equipmentType() == TRAIN || travelSegFrom->isForcedStopOver())
      return true;
    else if (travelSegFrom->isForcedConx())
      return false;
  }
  else
  {
    if (!airSeg || travelSegFrom->equipmentType() == TRAIN || travelSegFrom->isForcedStopOver())
      return false;
    else if (travelSegFrom->isForcedConx())
      return true;
  }
  return transit;
}
