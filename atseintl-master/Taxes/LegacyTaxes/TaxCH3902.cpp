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

#include "Taxes/LegacyTaxes/TaxCH3902.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"

using namespace tse;
using namespace std;

bool
TaxCH3902::validateSpecialFlightsTransit(const TravelSeg* pTravelSegFrom,
                                         const TravelSeg* pTravelSegTo)
{
  const AirSeg* airSegFrom = dynamic_cast<const AirSeg*>(pTravelSegFrom);
  const AirSeg* airSegTo = dynamic_cast<const AirSeg*>(pTravelSegTo);

  if (!airSegFrom || !airSegTo)
    return true;

  if (airSegFrom->marketingFlightNumber() == FROM_SPECIAL_FLIGHT &&
      airSegFrom->marketingCarrierCode() == FROM_TO_SPECIAL_CARRIER &&
      airSegTo->marketingFlightNumber() == TO_SPECIAL_FLIGHT &&
      airSegTo->marketingCarrierCode() == FROM_TO_SPECIAL_CARRIER)
    return false;

  return true;
}
