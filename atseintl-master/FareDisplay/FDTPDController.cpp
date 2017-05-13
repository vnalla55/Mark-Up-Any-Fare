//----------------------------------------------------------------------------
//  File: FDTPDController.cpp
//
//  Author: Partha Kumar Chakraborti
//  Created:      08/09/2005
//  Description:  This takes cares of getting Permissible Specified Routing Info
//  Copyright Sabre 2005
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/FDTPDController.h"

#include "Common/FareMarketUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayRequest.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.FDTPDController");

FDTPDController::FDTPDController() {}

//----------------------------------------------------------------------------
//  processThruMktCxrs()
//
//  Process through market carrier requirements or exceptions.
//----------------------------------------------------------------------------
bool
FDTPDController::processThruMktCxrs(const MileageRouteItems& subRoute,
                                    const TpdPsr& tpd,
                                    const CarrierCode& governingCarrier) const
{
  // we need to check this condition only if the carrier of TpdPsr record is empty
  // otherwise the carrier field would be already matched with governing carrier of the route
  if (!tpd.thruMktCxrs().empty() && tpd.carrier().empty())
  {
    bool found = std::find(tpd.thruMktCxrs().begin(), tpd.thruMktCxrs().end(), governingCarrier) !=
                 tpd.thruMktCxrs().end();
    // if result whether governing carrier was found on the vector of carriers or not, does not
    // conform to
    // whether it should be there (thruMktCarrierExcept indicator), the record fails
    if (found != (tpd.thruMktCarrierExcept() == THRUMKTCXREXCEPT_NO))
    {
      return false;
    }
  }
  // if the accordance exists, check if the whole route must be via the same carrier
  if (tpd.thruViaMktSameCxr() == THRUVIAMKTSAMECXR_YES ||
      (!tpd.carrier().empty() && tpd.thisCarrierRestr() == 'Y'))
  {
    MileageRouteItems::const_iterator itr(subRoute.begin());
    MileageRouteItems::const_iterator end(subRoute.end()); // lint !e578
    for (; itr != end; ++itr)
    {
      // if the whole route must be via the same carrier, and another carrier is found in the route,
      // fail this record
      if (itr->segmentCarrier() != governingCarrier)
      {
        return false;
      }
    }
  }
  return true;
}

// ----------------------------------------
// 	buildMileageRoute
// ----------------------------------------
bool
FDTPDController::buildMileageRoute(FareDisplayTrx& trx, MileageRoute& mileageRoute) const
{
  if (trx.itin().empty())
    return false;

  if (trx.itin().front()->fareMarket().empty())
    return false;

  // --------------------------
  // Create Travel Route
  // --------------------------
  TravelRouteBuilder travelRouteBuilder;
  TravelRoute tvlRoute;

  // Take the 1st FareMarket to build TravelRoute
  bool iRet = travelRouteBuilder.buildTravelRoute(trx, *(trx.fareMarket().front()), tvlRoute);
  if (!iRet)
  {
    LOG4CXX_DEBUG(
        logger,
        "Leaving FDPSRController::buildMileageRoute() - FAILED , Not able to build TravelRoute");
    return iRet;
  }

  // --------------------------
  // Create Mileage Route
  // --------------------------
  MileageRouteBuilder mileageRouteBuilder;

  iRet = mileageRouteBuilder.buildMileageRoute(
      trx, tvlRoute, mileageRoute, getDataHandle(trx), trx.getRequest()->ticketingDT());

  if (!iRet)
  {
    LOG4CXX_DEBUG(
        logger,
        "Leaving FDPSRController::buildMileageRoute() - FAILED , Not able to build MileageRoute");
    return iRet;
  }
  return true;
}

// ------------------------------
//  validate
// ------------------------------

bool
FDTPDController::validate(FareDisplayTrx& trx,
                          const GlobalDirection& globalDir,
                          const CarrierCode& carrierCode,
                          RoutingInfo& routingInfo,
                          const std::vector<TpdPsr*>& tpdList) const
{
  LOG4CXX_DEBUG(logger,
                "Entering validate. TPD#: " << tpdList.size() << " TPDs in RoutingInfo: "
                                            << routingInfo.fdTPD().size());

  const Loc& origin = *trx.origin();
  const Loc& destination = *trx.destination();

  std::vector<TpdPsr*>::const_iterator iB(tpdList.begin());
  std::vector<TpdPsr*>::const_iterator iE(tpdList.end());

  // -----------------
  // Build Mileage Route
  // -----------------
  MileageRoute mileageRoute;
  if (!buildMileageRoute(trx, mileageRoute))
    return false;

  // ------------------------------
  // Get Multi Transport Origin and Destination
  // ------------------------------
  const Loc* multiTransportOrigin = getDataHandle(trx).getLoc(
      FareMarketUtil::getMultiCity(
          carrierCode, origin.loc(), trx.itin().front()->geoTravelType(), trx.travelDate()),
      trx.travelDate());

  const Loc* multiTransportDestination = getDataHandle(trx).getLoc(
      FareMarketUtil::getMultiCity(
          carrierCode, destination.loc(), trx.itin().front()->geoTravelType(), trx.travelDate()),
      trx.travelDate());

  // ---------------------------
  // Iterate through each tpd and validate
  // ---------------------------
  for (; iB != iE; ++iB)
  {
    TpdPsr& tpd(**iB);

    // -------------------------------
    // Eliminate based on Loc1
    // -------------------------------

    if (!isFrom(origin, destination, tpd) && !isFrom(destination, origin, tpd) &&
        (multiTransportOrigin == nullptr || multiTransportDestination == nullptr ||
         (!isFrom(*multiTransportOrigin, *multiTransportDestination, tpd) &&
          !isFrom(*multiTransportDestination, *multiTransportOrigin, tpd) &&
          !isFrom(origin, *multiTransportDestination, tpd) &&
          !isFrom(*multiTransportDestination, origin, tpd) &&
          !isFrom(*multiTransportOrigin, destination, tpd) &&
          !isFrom(destination, *multiTransportOrigin, tpd))))
      continue;

    // -----------------------------------------
    //	Eliminate based on Through Mkt Carriers
    // -----------------------------------------
    if (!processThruMktCxrs(mileageRoute.mileageRouteItems(), tpd, carrierCode))
      continue;

    // -----------------------------
    //	Save TPD in RoutingInfo
    // -----------------------------
    routingInfo.fdTPD().push_back(&tpd);
  }

  LOG4CXX_DEBUG(logger, "Leaving validate. TPDs in RoutingInfo: " << routingInfo.fdTPD().size());
  return true;
}

// ------------------------------
//  getTPD
// ------------------------------
bool
FDTPDController::getTPD(FareDisplayTrx& trx,
                        const CarrierCode& carrier,
                        GlobalDirection globalDir,
                        RoutingInfo& routingInfo)
{
  bool iRet = false;

  // ------------------------
  //	Get TPD Data
  // ------------------------
  const Loc& origin = *trx.origin(); // lint !e578
  const Loc& destination = *trx.destination(); // lint !e578

  const std::vector<TpdPsr*>& allGDTpdList = getTPDData(
      getDataHandle(trx), carrier, origin.area()[0], destination.area()[0], trx.travelDate());

  std::vector<TpdPsr*> tpdList;
  // if the route global direction is well defined, filter the record vector
  // to have only the records with matching globalDir or blank
  if (globalDir != GlobalDirection::ZZ)
  {
    std::remove_copy_if(allGDTpdList.begin(),
                        allGDTpdList.end(),
                        std::back_inserter(tpdList),
                        GDNotEqualNorBlank(globalDir));
  }
  else
  {
    tpdList = allGDTpdList;
  }

  iRet = validate(trx, globalDir, carrier, routingInfo, tpdList);
  return iRet;
}
}
