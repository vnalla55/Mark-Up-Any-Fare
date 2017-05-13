//--------------------------------------------------------------------
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DataModel/FareMarket.h"
#include "Common/TseCodeTypes.h"
#include "Pricing/test/MockLoc.h"
#include "Common/SmallBitSet.h"

namespace tse
{

class Loc;

class MockFareMarket : public tse::FareMarket
{
public:
  MockFareMarket() : FareMarket() {}
  virtual ~MockFareMarket() {}

  // virtual bool clear();

  void set_origin(Loc* origin) { _origin = origin; }

  void set_destination(Loc* destination) { _destination = destination; }

  void set_GoverningCarrier(CarrierCode govCxr) { _governingCarrier = govCxr; }

  void set_TravelBoundary(FMTravelBoundary tb)
  {
    // SmallBitSet<uint8_t, FMTravelBoundary>& boundary = fareMkt.travelBoundary();
    _travelBoundary.set(tb, true);
  }
};
} // tse namespace
