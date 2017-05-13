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

#include "DataModel/MileageTrx.h"

#include <string>

namespace tse
{
class TravelRoute;
class DataHandle;
class MileageTrx;
class MileageRoute;
class MileageRouteItem;
class PricingTrx;
class DiagCollector;
class PricingRequest;

// @class MileageRouteBuilder
// Builds uniform input(WPQ/WN) for the MileageValidation process.
// First step of the mileage validation process is to convert either
// MileageTrx or MileageValidator to a MileageRoute( similar to FareMarket)
// which is a collection of MileageRouteItems(similar to TravelSeg).

class MileageRouteBuilder
{
  friend class MileageRouteBuilderTest;

public:
  MileageRouteBuilder(bool isWNfromItin = false);

  // Creates MileageRoute from a MileageTrx.
  // It has to calculate globalDirection from the first city to
  // the each consecutive city and also has to get the globalDirection
  // for each segments.
  // @param MileageTrx A reference to the MileageTrx passed by  MileageService.
  // @param MileageRoute A reference to the MileageRoute passed to the MileageCollection process.
  // @returns true/false to identify success of failure of the mileage route building process.

  bool buildMileageRoute(MileageTrx&, MileageRoute&, DiagCollector*) const;

  // Creates MileageRoute from a TravelRoute created from FareMarket.
  // It has to calculate globalDirection for each segments and uses the
  // GlobalDirection of the FareMarket as the mileage Route global direction.
  // @param TravelRoute A reference to the TravelRoute passed by  RoutingController.
  // @param MileageRoute A reference to the MileageRoute passed to the MileageValidattion process.
  // @returns true/false to identify success of failure of the mileage route building process.

  bool buildMileageRoute(PricingTrx& trx, const TravelRoute&, MileageRoute&, DataHandle&, DateTime&)
      const;
  void buildWNMileageRoute(MileageRoute&) const;
  void setCRSMultiHost(MileageRoute&, PricingRequest*) const;

private:
  bool getGoverningCarrier(MileageRoute&) const;
  void setOccurrences(MileageRoute&) const;
  void setDirectService(MileageRouteItem*, bool) const;

  bool _isWNfromItin;
};

} // namespace tse

