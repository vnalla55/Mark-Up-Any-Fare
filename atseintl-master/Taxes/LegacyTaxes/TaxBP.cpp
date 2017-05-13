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

#include "Taxes/LegacyTaxes/TaxBP.h"
#include "DBAccess/Loc.h"
#include "DataModel/TaxResponse.h"
#include "Common/LocUtil.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/FarePath.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"

using namespace tse;
using namespace std;

const std::string TaxBP::ICN = "ICN";
const std::string TaxBP::GMP = "GMP";

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxBP::TaxBP() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxBP::~TaxBP() {}

// ---------------------------------------
// Description:  TaxBP::validateTransit
// ---------------------------------------
bool
TaxBP::validateTransit(PricingTrx& trx,
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

  const bool prevSegInter =
      LocUtil::isInternational(*((*tvlSegPrevI)->origin()), *((*tvlSegPrevI)->destination()));

  const bool currSegInter =
      LocUtil::isInternational(*((*tvlSegI)->origin()), *((*tvlSegI)->destination()));

  if ((endOfPrevSeg->loc() == ICN || endOfPrevSeg->loc() == GMP) && prevSegInter && currSegInter)
  {
    MirrorImage mirrorImage;

    if (!(endOfPrevSeg == startOfCurrentSeg))
    {
      return true;
    }
    else if ((*tvlSegPrevI)->isForcedStopOver() ||
             ((*tvlSegI)->isStopOver((*tvlSegPrevI), SECONDS_PER_DAY)) ||
             mirrorImage.isMirrorImage(trx, taxResponse, taxCodeReg, travelSegIndex))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  return Tax::validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
}
