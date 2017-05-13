//----------------------------------------------------------------------------
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

// FIXME: <memory> should be included by Singleton.h
#include "Common/Singleton.h"
#include "DBAccess/TpdPsr.h"
#include "Routing/MileageExclusion.h"

#include <memory>
#include <vector>

namespace tse
{
/**
 *@class PermissibleSpecifiedRouting.
 * Searches for applicable items from the Tpd/Psr database.
 * If an item exists and is validated for the market, mileage validation
 * is complete for Pricing.
 */

class MileageRoute;
class MileageRouteItem;
class Loc;

class PermissibleSpecifiedRouting : public MileageExclusion
{
  friend class PermissibleSpecifiedRoutingTest;

public:
  /**
   * Implements the apply interface for MPMReduction.
   * @param MileageRoute A reference to the MileageRoute.
   * @returns true if MPMReduction is applicable. False otherwise.
   */

  virtual bool apply(MileageRoute& mileageRoute) const override;

private:
  /**
   * Validate the viaGeoLocs specified in the PSR.
   */

  bool validate(MileageRoute& mileageRoute, const std::vector<TpdPsr*>& psrList) const;

  bool processViaGeoLocs(MileageRoute& mileageRoute, TpdPsr& psr) const;

  bool validateGeoLocSet(TpdPsr& psr,
                         MileageRoute& mileageRoute,
                         std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
                         const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd) const;

  bool matchGeoLoc(MileageRoute& mileageRoute,
                   MileageRouteItem& mRouteItem,
                   std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
                   const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd,
                   Indicator& groupRelationalInd,
                   int32_t setNumber) const;

  bool processGeoLocStopoverConditions(MileageRoute& mileageRoute, TpdPsr& psr) const;

  bool checkGeoLocs(MileageRoute& mileageRoute,
                    int32_t setNumber,
                    std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
                    const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd) const;

  bool checkLoc(MileageRoute& mileageRoute,
                MileageRouteItem& mRouteItem,
                int32_t setNumber,
                std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
                const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd,
                Indicator& groupRelationalInd) const;

  bool setGeoLocIterator(MileageRoute& mileageRoute,
                         std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
                         const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd,
                         Indicator& groupRelationalInd,
                         int32_t setNumber) const;

  bool processThruMktCxrs(const MileageRoute& mileageRoute, TpdPsr& psr) const;

  bool processViaMktSameCarrier(const MileageRoute& mileageRoute, TpdPsr& psr) const;

  bool checkAllViaCarriers(const MileageRoute& mileageRoute, const CarrierCode& carrier) const;

  bool checkAnyViaCarrier(const MileageRoute& mileageRoute, const CarrierCode& carrier) const;

  bool processViaCxrLocs(const MileageRoute& mileageRoute, TpdPsr& psr) const;

  bool processViaCxrLocExceptions(const MileageRoute& mileageRoute, TpdPsr& psr) const;

  bool processFareType(const MileageRoute& mileageRoute, TpdPsr& psr) const;

  void reverseRoute(const MileageRoute& mileageRoute, MileageRoute& reverseMileageRoute) const;

  PermissibleSpecifiedRouting() {}
  PermissibleSpecifiedRouting(const PermissibleSpecifiedRouting&);
  PermissibleSpecifiedRouting& operator=(const PermissibleSpecifiedRouting&);
  friend class tse::Singleton<PermissibleSpecifiedRouting>;
};

} // namespace tse

