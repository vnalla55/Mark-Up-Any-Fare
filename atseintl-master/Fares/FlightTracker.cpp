//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
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

#include "Fares/FlightTracker.h"

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FltTrkCntryGrp.h"
#include "DBAccess/Loc.h"

#include <algorithm>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.Fares.FlightTracker");

FlightTracker::FlightTracker(const PricingTrx& trx) : _trx(trx), _fareMarket(nullptr) {}

FlightTracker::~FlightTracker() {}

/**
 * Main method to process flight traking validation proecess.
 * @param fareMarket  A reference to the FareMarket object.
 * @return bool true if flight tracking applies.
 *
 */

bool
FlightTracker::process(FareMarket& fareMarket)
{
  LOG4CXX_DEBUG(logger, "Entered FlightTracker::process()");

  _fareMarket = &fareMarket;

  if (fareMarket.geoTravelType() != GeoTravelType::International)
  {
    fareMarket.setFltTrkIndicator(validateFltTracking(fareMarket.origin()->nation(),
                                                      fareMarket.destination()->nation(),
                                                      fareMarket.governingCarrier(),
                                                      fareMarket.travelSeg()));
    LOG4CXX_DEBUG(logger,
                  "Leaving FlightTracker::process :: FlightTracking Carrier Found:"
                      << fareMarket.fltTrkIndicator());
  }

  else
    LOG4CXX_INFO(logger,
                 "Leaving FlightTracker::International Market Doesnt Require Flt Tracking");

  return true;
}

/**
* This will work as a utility function for validation flight tracking.
* Any other object can call this method with an origin, destination
* and carrier to validate flight tracking.
* @param NationCode oigin A reference to the nation of origin.
* @param NationCode destination A reference to the nation of destination.
* @param CarrierCode carrier A reference to the gov carrier of the FareMarket or TravelSegs.
* */

bool
FlightTracker::validateFltTracking(const NationCode& origin,
                                   const NationCode& destination,
                                   const CarrierCode& carrier,
                                   const std::vector<TravelSeg*>& tvlSegs)

{
  LOG4CXX_DEBUG(logger, "Entered FlightTracker::validateFlightTracking()");

  const FltTrkCntryGrp* ftcg = getData(carrier, tvlSegs);

  if (ftcg)
  {
    LOG4CXX_DEBUG(logger, "FlightTracker::Got Data From DataBase()");

    bool isOrigin =
        std::find(ftcg->nations().begin(), ftcg->nations().end(), origin) != ftcg->nations().end();

    bool isDestination = std::find(ftcg->nations().begin(), ftcg->nations().end(), destination) !=
                         ftcg->nations().end();

    if (ftcg->flttrkApplInd() == 'B' && origin != destination)
      return (isOrigin && isDestination);

    else
      return (isOrigin || isDestination);
  }

  LOG4CXX_DEBUG(logger, "FlightTracker::No Data From DataBase!!");
  return false;
}

const FltTrkCntryGrp*
FlightTracker::getData(const CarrierCode& carrier, const std::vector<TravelSeg*>& tvlSegs)

{
  LOG4CXX_DEBUG(logger, "Entered FlightTracker::getData()");
  return _trx.dataHandle().getFltTrkCntryGrp(carrier, getDate(tvlSegs));
}

bool
FlightTracker::getFltTrackingInfo(const std::vector<TravelSeg*>& tvlSegs, const CarrierCode govCxr)
{
  if (LIKELY(!tvlSegs.empty()))
    return validateFltTracking(tvlSegs.front()->origin()->nation(),
                               tvlSegs.back()->destination()->nation(),
                               govCxr,
                               tvlSegs);
  return false;
}

DateTime
FlightTracker::getDate(const std::vector<TravelSeg*>& tvlSegs)
{
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX))
  {
    const RexPricingTrx& rexTrx = static_cast<const RexPricingTrx&>(_trx);
    if (_fareMarket)
      return _fareMarket->travelDate();
    else
      return rexTrx.adjustedTravelDate(TseUtil::getTravelDate(tvlSegs));
  }

  return TseUtil::getTravelDate(tvlSegs);
}
}
