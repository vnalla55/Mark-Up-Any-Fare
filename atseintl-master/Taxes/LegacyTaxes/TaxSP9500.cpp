// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
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

#include "Taxes/LegacyTaxes/TaxSP9500.h"

#include "Common/FallbackUtil.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;
using namespace std;


bool
TaxSP9500::isMultiCityMirrorImage(TravelSeg* pFrom, TravelSeg* pTo)
{
  return pFrom->origAirport() != pTo->destAirport() &&
         pFrom->boardMultiCity() == pTo->offMultiCity();
}

bool
TaxSP9500::isMultiCityStopOver(TravelSeg* pFrom, TravelSeg* pTo)
{
  return pFrom->destAirport() != pTo->origAirport() &&
         pFrom->offMultiCity() == pTo->boardMultiCity();
}

// ----------------------------------------------------------------------------
// Description:  validateTransit
// ----------------------------------------------------------------------------
bool
TaxSP9500::validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{
  if (handleHiddenPoints() != HIDDEN_POINT_NOT_HANDLED)
  {
    if (taxCodeReg.restrictionTransit().empty())
      return true;

    if (validateTransitOnHiddenPoints(taxCodeReg))
      return taxCodeReg.restrictionTransit().front().transitTaxonly();
  }

  MirrorImage mirrorImage;

  bool stopOver = mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex);

  TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];
  TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex];

  if ((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
      ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
       (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION)))
  {
    if (travelSegFrom != taxResponse.farePath()->itin()->travelSeg().back())
      travelSegTo = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex + 1];
  }
  else
  {
    if (travelSegTo != taxResponse.farePath()->itin()->travelSeg().front())
      travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[travelSegIndex - 1];
  }
  const AirSeg* airSegFrom = dynamic_cast<const AirSeg*>(travelSegFrom);
  const AirSeg* airSegTo = dynamic_cast<const AirSeg*>(travelSegTo);

  if (airSegFrom && airSegTo && airSegFrom != airSegTo)
  {
    const std::string airport = utc::specialTransitAirport(trx, taxCodeReg);
    //only BR tax should check mirror image, UB6 shouldn't
    if (airport.empty() &&
        isMultiCityMirrorImage(travelSegFrom, travelSegTo))
      return true;

    if (airport.empty() ||
        airSegFrom->destAirport() == airport || airSegTo->origAirport() == airport)
    {
      if (!stopOver)
        stopOver = isMultiCityStopOver(travelSegFrom, travelSegTo);
    }
  }

  TransitValidator transitValidator;

  return transitValidator.validateTransitRestriction(
      trx, taxResponse, taxCodeReg, travelSegIndex, stopOver, _landToAirStopover);
}
