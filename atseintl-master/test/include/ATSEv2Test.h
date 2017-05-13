//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
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

#include "DataModel/ArunkSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

namespace tse
{

class ATSEv2Test : public ::testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _pTrx = _memHandle.create<PricingTrx>();
  }

  void TearDown()
  {
    _memHandle.clear();
  }

  template<typename T>
  T* getDiagCollector()
  {
    T* diag = _memHandle(new T);
    diag->activate();
    return diag;
  }

  void addDiagArg(std::string arg, std::string value = "");

  static AirSeg* createAirSeg(TestMemHandle& memHandle, const Loc* origin, const Loc* destination, CarrierCode carrierCode);
  static TravelSeg* createArunkSeg(TestMemHandle& memHandle, const Loc* origin, const Loc* destination);
  static FareUsage* createFareUsage(TestMemHandle& memHandle, const Loc* origin, const Loc* destination, CarrierCode carrierCode);

  virtual const Loc* createLoc(LocCode locCode);

protected:
  PricingTrx* _pTrx;
  TestMemHandle _memHandle;
};


void
ATSEv2Test::addDiagArg(std::string arg, std::string value)
{
  _pTrx->diagnostic().diagParamMap()[arg] = value;
}

AirSeg*
ATSEv2Test::createAirSeg(TestMemHandle& memHandle, const Loc* origin, const Loc* destination, CarrierCode carrierCode)
{
  AirSeg* seg = memHandle.create<AirSeg>();
  seg->origin() = origin;
  seg->boardMultiCity() = origin->loc();
  seg->destination() = destination;
  seg->offMultiCity() = destination->loc();
  seg->carrier() = carrierCode;
  return seg;
}

TravelSeg*
ATSEv2Test::createArunkSeg(TestMemHandle& memHandle, const Loc* origin, const Loc* destination)
{
  TravelSeg* seg = memHandle.create<ArunkSeg>();
  seg->origin() = origin;
  seg->destination() = destination;
  return seg;
}

const Loc*
ATSEv2Test::createLoc(LocCode locCode)
{
  return _pTrx->dataHandle().getLoc(locCode, DateTime::localTime());
}

FareUsage*
ATSEv2Test::createFareUsage(TestMemHandle& memHandle, const Loc* origin, const Loc* destination, CarrierCode carrierCode)
{
  FareMarket *fm = memHandle.create<FareMarket>();
  fm->origin() = origin;
  fm->destination() = destination;
  fm->boardMultiCity() = origin->loc();
  fm->offMultiCity() = destination->loc();
  fm->governingCarrier() = carrierCode;
  fm->travelSeg().push_back(ATSEv2Test::createAirSeg(memHandle, origin, destination, carrierCode));

  PaxTypeFare* ptf = memHandle.create<PaxTypeFare>();
  ptf->fareMarket() = fm;
  ptf->setFare(memHandle.create<Fare>());

  FareUsage* fareUsage = memHandle.create<FareUsage>();
  fareUsage->paxTypeFare() = ptf;
  fareUsage->travelSeg() = fm->travelSeg();

  return fareUsage;
}

} // tse
