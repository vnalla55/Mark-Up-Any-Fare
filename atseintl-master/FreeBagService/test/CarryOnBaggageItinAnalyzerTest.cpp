// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "Common/TseEnums.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/SurfaceSeg.h"
#include "FreeBagService/CarryOnBaggageItinAnalyzer.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"

namespace tse
{
class CarryOnBaggageItinAnalyzerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CarryOnBaggageItinAnalyzerTest);
  CPPUNIT_TEST(testAnalyze);
  CPPUNIT_TEST(testCreateBaggageTravels);
  CPPUNIT_TEST(testCreateBaggageTravel);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _itin;
  FarePath* _farePath;
  CarryOnBaggageItinAnalyzer* _carryOnBaggageItinAnalyzer;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _itin = _memHandle.create<Itin>();
    _farePath = _memHandle.create<FarePath>();
    _itin->farePath().push_back(_farePath);
    _carryOnBaggageItinAnalyzer = _memHandle.insert(new CarryOnBaggageItinAnalyzer(*_trx, *_itin));
  }

  void tearDown() { _memHandle.clear(); }

  TravelSeg* getAirSeg(const TravelSegType& travelSegType)
  {
    return AirSegBuilder(&_memHandle).setType(travelSegType).build();
  }

  TravelSeg* getArunkSeg(const TravelSegType& travelSegType)
  {
    ArunkSeg* travelSeg = _memHandle.create<ArunkSeg>();
    travelSeg->segmentType() = travelSegType;
    return travelSeg;
  }

  TravelSeg* getSurfaceSeg(const TravelSegType& travelSegType)
  {
    SurfaceSeg* travelSeg = _memHandle.create<SurfaceSeg>();
    travelSeg->segmentType() = travelSegType;
    return travelSeg;
  }

  void testAnalyze()
  {
    _itin->travelSeg().push_back(getAirSeg(Air));
    _itin->travelSeg().push_back(getArunkSeg(Air));
    _itin->travelSeg().push_back(getSurfaceSeg(Air));

    _itin->farePath().push_back(_memHandle.create<FarePath>());
    _itin->farePath().push_back(_memHandle.create<FarePath>());
    _itin->farePath().push_back(_memHandle.create<FarePath>());

    _carryOnBaggageItinAnalyzer->analyze();

    CPPUNIT_ASSERT(_carryOnBaggageItinAnalyzer->baggageTravels().size() == 4);
  }

  void testCreateBaggageTravels()
  {
    _itin->travelSeg().push_back(getAirSeg(Air));
    _itin->travelSeg().push_back(getArunkSeg(Air));
    _itin->travelSeg().push_back(getSurfaceSeg(Air));

    _carryOnBaggageItinAnalyzer->createBaggageTravels();

    CPPUNIT_ASSERT(_carryOnBaggageItinAnalyzer->baggageTravels().size() == 1);
    CPPUNIT_ASSERT(_carryOnBaggageItinAnalyzer->baggageTravels()[0]->_stopOverLength == 86400);
  }

  void testCreateBaggageTravel()
  {
    _itin->travelSeg().push_back(getAirSeg(Air));
    _itin->travelSeg().push_back(getAirSeg(Air));

    TravelSegPtrVecCI travelSegIter  = _itin->travelSeg().begin();
    TravelSegPtrVecCI mssJourneyIter = _itin->travelSeg().end();

    const BaggageTravel* bt = _carryOnBaggageItinAnalyzer->createBaggageTravel(
        travelSegIter, mssJourneyIter);

    CPPUNIT_ASSERT(bt);
    CPPUNIT_ASSERT_EQUAL(bt->_trx, _trx);
    CPPUNIT_ASSERT_EQUAL(bt->farePath(), _farePath);
    CPPUNIT_ASSERT(bt->_MSS == travelSegIter);
    CPPUNIT_ASSERT(travelSegIter != mssJourneyIter);
    CPPUNIT_ASSERT(bt->_MSSJourney == mssJourneyIter);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CarryOnBaggageItinAnalyzerTest);
}
