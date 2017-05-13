//----------------------------------------------------------------------------
//  File: FDPSRController.cpp
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

#include "FareDisplay/FDPSRController.h"

#include "Common/FareMarketUtil.h"
#include "Common/Logger.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.FDPSRController");

FDPSRController::FDPSRController() {}

//----------------------------------------------------------------------------
//  processThruMktCxrs()
//
//  Process through market carrier requirements or exceptions.
//----------------------------------------------------------------------------
bool
FDPSRController::processThruMktCxrs(const CarrierCode& carrierCode, TpdPsr& psr) const
{

  if (psr.carrier().empty() && !psr.thruMktCxrs().empty())
  {
    const bool found = std::find(psr.thruMktCxrs().begin(), psr.thruMktCxrs().end(), carrierCode) !=
                       psr.thruMktCxrs().end();

    if (found == (psr.thruMktCarrierExcept() == YES))
    {
      return false;
    }
  }

  else if (!psr.carrier().empty())
  {
    if (psr.carrier() != carrierCode)
    {
      return false;
    }
  }

  return true;
}

// ------------------------------
//  validate
// ------------------------------

bool
FDPSRController::validate(FareDisplayTrx& trx,
                          const GlobalDirection& globalDir,
                          const CarrierCode& carrierCode,
                          RoutingInfo& routingInfo,
                          const std::vector<TpdPsr*>& psrList) const
{
  LOG4CXX_DEBUG(logger,
                "Entering validate. PSR#: " << psrList.size() << " PSRs in RoutingInfo: "
                                            << routingInfo.fdPSR().size());

  const Loc& origin = *trx.origin();
  const Loc& destination = *trx.destination();

  LocCode tempMultiCode = FareMarketUtil::getMultiCity(
      carrierCode, origin.loc(), trx.itin().front()->geoTravelType(), trx.travelDate());
  const Loc* multiOrigin = getDataHandle(trx).getLoc(tempMultiCode, trx.travelDate());

  tempMultiCode = FareMarketUtil::getMultiCity(
      carrierCode, destination.loc(), trx.itin().front()->geoTravelType(), trx.travelDate());
  const Loc* multiDestination = getDataHandle(trx).getLoc(tempMultiCode, trx.travelDate());

  std::vector<TpdPsr*>::const_iterator iB(psrList.begin());
  std::vector<TpdPsr*>::const_iterator iE(psrList.end());

  for (; iB != iE; ++iB)
  {
    TpdPsr& psr(**iB);

    // if either loc fails to match in one direction...
    if (!validateLoc(origin, multiOrigin, psr.loc1()) ||
        !validateLoc(destination, multiDestination, psr.loc2()))
    {
      // ... try to match the other direction
      if (!validateLoc(origin, multiOrigin, psr.loc2()) ||
          !validateLoc(destination, multiDestination, psr.loc1()))
      {
        // both directions don't match, so skip
        continue;
      }
    }

    // Eliminate based on Globals
    if (psr.globalDir() != GlobalDirection::ZZ && psr.globalDir() != globalDir)
      continue;

    // Eliminate based on Through Mkt Carriers
    if (!processThruMktCxrs(carrierCode, psr))
      continue;

    // Save PSR in RoutingInfo
    routingInfo.fdPSR().push_back(&psr);

  } // endfor - each PSR

  LOG4CXX_DEBUG(logger, "Leaving validate. PSRs in RoutingInfo: " << routingInfo.fdPSR().size());
  return true;
}

// ------------------------------
//  validateLoc - does a loc in the PRS fit the request?
// ------------------------------
bool
FDPSRController::validateLoc(const Loc& reqLoc, const Loc* reqMultiLoc, LocKey& psrLoc) const
{
  if (LocUtil::isInLoc(reqLoc, psrLoc.locType(), psrLoc.loc(), Vendor::SABRE, MANUAL))
    return true;

  if (reqMultiLoc == nullptr)
    return false;

  return LocUtil::isInLoc(*reqMultiLoc, psrLoc.locType(), psrLoc.loc(), Vendor::SABRE, MANUAL);
}

// ------------------------------
//  getPSR
// ------------------------------

bool
FDPSRController::getPSR(FareDisplayTrx& trx,
                        const GlobalDirection& globalDir,
                        RoutingInfo& routingInfo)
{ // lint --e{578}
  bool iRet = false;

  // ------------------------
  //	Get PSR Data
  // ------------------------
  const Loc& origin = *trx.origin();
  const Loc& destination = *trx.destination();

  std::set<CarrierCode>::const_iterator iB = trx.preferredCarriers().begin();
  std::set<CarrierCode>::const_iterator iE = trx.preferredCarriers().end();

  // -------------------------------------------------
  // Iterate through each carrer and get PSR for that
  // -------------------------------------------------

  for (; iB != iE; ++iB)
  {
    const CarrierCode& carrier(*iB);

    const std::vector<TpdPsr*>& psrList = getPSRData(
        getDataHandle(trx), carrier, origin.area()[0], destination.area()[0], trx.travelDate());

    iRet = validate(trx, globalDir, carrier, routingInfo, psrList);
  }

  return iRet;
}
}
