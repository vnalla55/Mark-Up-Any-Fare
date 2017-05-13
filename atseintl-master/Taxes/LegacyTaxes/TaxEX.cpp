// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/LegacyTaxes/TaxEX.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/FarePath.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"

#include "Common/FallbackUtil.h"

using namespace tse;
using namespace std;

const string
TaxEX::TAX_CODE_TQ("TQ");

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxEX::TaxEX() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxEX::~TaxEX() {}

// ---------------------------------------
// Description:  TaxEX::validateTransit
// ---------------------------------------
bool
TaxEX::validateTransit(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxCodeReg& taxCodeReg,
                       uint16_t travelSegIndex)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI;
  std::vector<TravelSeg*>::const_iterator tvlSegPrevI;

  tvlSegI = taxResponse.farePath()->itin()->travelSeg().begin() + (travelSegIndex);

  if (travelSegIndex > 0)
  {
    tvlSegPrevI = taxResponse.farePath()->itin()->travelSeg().begin() + (travelSegIndex - 1);

    if ((*tvlSegPrevI) != taxResponse.farePath()->itin()->travelSeg().front())
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*tvlSegPrevI);

      if (!airSeg)
      {
        tvlSegPrevI = taxResponse.farePath()->itin()->travelSeg().begin() + (travelSegIndex - 2);
      }
    }
  }
  else
  {
    tvlSegPrevI = taxResponse.farePath()->itin()->travelSeg().begin() + (travelSegIndex);
  }

  const Loc* endOfPrevSeg = (*tvlSegPrevI)->destination();
  const Loc* startOfCurrentSeg = (*tvlSegI)->origin();

  if ((*tvlSegPrevI)->pnrSegment() != (*tvlSegI)->pnrSegment() && endOfPrevSeg != startOfCurrentSeg)
    return true;

  if (taxCodeReg.taxCode() == TAX_CODE_TQ)
  {
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
      bool stopOver = airSegFrom->origAirport() != airSegTo->destAirport() &&
                      airSegFrom->boardMultiCity() == airSegTo->offMultiCity();
      if (stopOver)
        return true;
    }
  }

  return Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
}
