//-------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/GlobalDirectionFinderV2Adapter.h"

#include "Common/TseEnums.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

namespace tse
{
namespace GlobalDirectionFinderV2Adapter
{
bool
process(PricingTrx& trx, FareMarket& fareMarket)
{
  GlobalDirection dir = fareMarket.getGlobalDirection();
  bool ret = getGlobalDirection(&trx, fareMarket.travelDate(), fareMarket.travelSeg(), dir);
  fareMarket.setGlobalDirection(dir);
  return ret;
}

bool
getGlobalDirection(const PricingTrx* trx,
                   DateTime travelDate,
                   const std::vector<TravelSeg*>& tvlSegs,
                   GlobalDirection& globalDir)
{
  if (UNLIKELY(tvlSegs.empty()))
    return false;

  std::vector<Location> locations;
  std::set<CarrierCode> carriers;

  locations.reserve(tvlSegs.size());

  locations.emplace_back(tvlSegs.front()->origin());
  for (const TravelSeg* travelSeg : tvlSegs)
  {
    if (travelSeg->origin()->loc() != locations.back().locCode())
      locations.emplace_back(travelSeg->origin());

    for (const Loc* hiddenStop : travelSeg->hiddenStops())
      locations.emplace_back(hiddenStop, true);

    locations.emplace_back(travelSeg->destination());

    if (travelSeg->isAir())
      carriers.insert(static_cast<const AirSeg&>(*travelSeg).carrier());
  }

  GlobalDirectionFinder gdf(locations);
  return gdf.getGlobalDirection(trx, travelDate, carriers, globalDir);
}

bool
getGlobalDirection(const PricingTrx* trx,
                   DateTime travelDate,
                   TravelSeg& travelSeg,
                   GlobalDirection& globalDir)
{
  const Loc* origin = travelSeg.origin();
  const Loc* dest = travelSeg.destination();

  const bool fsActiveForShoping = trx->isIataFareSelectionApplicable();

  if (UNLIKELY((trx->getTrxType() == PricingTrx::IS_TRX) && fsActiveForShoping &&
                (trx->getGlobalDirectionFromMap(origin, dest, globalDir))))
    return true;

  const std::vector<TravelSeg*> tvlSegs = {&travelSeg};

  const bool retVal = getGlobalDirection(trx, travelDate, tvlSegs, globalDir);

  if (UNLIKELY((trx->getTrxType() == PricingTrx::IS_TRX) && retVal && fsActiveForShoping))
    trx->setGlobalDirectionToMap(origin, dest, globalDir);

  return retVal;
}
}
}
