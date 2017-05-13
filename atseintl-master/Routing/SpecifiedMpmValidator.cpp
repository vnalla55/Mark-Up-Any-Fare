//----------------------------------------------------------------------------
//  Copyright Sabre 2014
//
//  StopOverRestrictionValidator.cpp
//
//  Description:  Validate Routing Restrictions 8
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Routing/SpecifiedMpmValidator.h"

#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/RoutingRestriction.h"

namespace tse
{

SpecifiedMpmValidator::SpecifiedMpmValidator() {}

SpecifiedMpmValidator::~SpecifiedMpmValidator() {}

bool
SpecifiedMpmValidator::validate(const TravelRoute& tvlRoute, const RoutingRestriction& restriction)
{
  return true;
}

bool
SpecifiedMpmValidator::validate(const TravelRoute& tvlRoute,
                                const RoutingRestriction& rest,
                                const PricingTrx& trx)
{
  if (!trx.getOptions() || !trx.getOptions()->isRtw())
  {
    return validate(tvlRoute, rest);
  }

  uint32_t sum = 0;

  if (rest.mpm())
  {
    for (TravelSeg* tvlSeg : tvlRoute.mileageTravelRoute())
    {
      GlobalDirection gd;
      GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, tvlRoute.travelDate(), *tvlSeg, gd);

      sum += LocUtil::getTPM(
          *tvlSeg->origin(), *tvlSeg->destination(), gd, tvlRoute.travelDate(), trx.dataHandle());
    }
  }

  return static_cast<int64_t>(sum) <= rest.mpm();
}

} // namespace tse
