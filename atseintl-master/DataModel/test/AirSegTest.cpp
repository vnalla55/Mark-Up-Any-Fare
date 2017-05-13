#include "test/include/CppUnitHelperMacros.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/CarrierPreference.h"

#include "Common/Config/ConfigMan.h"

using namespace std;

namespace tse
{

class AirSegTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AirSegTest);
  CPPUNIT_TEST(testSetGet);

  CPPUNIT_TEST(testFlowJourneyCarrierReturnFalseWhenSegmentFlown);
  CPPUNIT_TEST(testFlowJourneyCarrierReturnFalseWhenCarrierPrefZero);
  CPPUNIT_TEST(testFlowJourneyCarrierReturnTrueWhenCarrierPrefSaysYes);
  CPPUNIT_TEST(testFlowJourneyCarrierReturnFalseWhenCarrierPrefSaysNo);
  CPPUNIT_TEST(testLocalJourneyCarrierReturnFalseWhenSegmentFlown);
  CPPUNIT_TEST(testLocalJourneyCarrierReturnFalseWhenCarrierPrefZero);
  CPPUNIT_TEST(testLocalJourneyCarrierReturnTrueWhenCarrierPrefSaysYes);
  CPPUNIT_TEST(testLocalJourneyCarrierReturnFalseWhenCarrierPrefSaysNo);
  CPPUNIT_TEST(testToAirSeg);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testSetGet()
  {
    AirSeg airSeg;
    airSeg.considerOnlyCabin() = "1";
    airSeg.forcedFareBrk() = 'T';
    airSeg.forcedNoFareBrk() = 'F';
    airSeg.forcedSideTrip() = 'T';
    airSeg.carrier() = "AA";
    airSeg.flightNumber() = 863;
    airSeg.marketingFlightNumber() = 863;
    airSeg.operatingFlightNumber() = 863;

    CPPUNIT_ASSERT(airSeg.considerOnlyCabin() == "1");
    CPPUNIT_ASSERT(airSeg.forcedFareBrk() == 'T');
    CPPUNIT_ASSERT(airSeg.forcedNoFareBrk() == 'F');
    CPPUNIT_ASSERT(airSeg.forcedSideTrip() == 'T');
    CPPUNIT_ASSERT(airSeg.carrier() == "AA");
    CPPUNIT_ASSERT(airSeg.flightNumber() == 863);
    CPPUNIT_ASSERT(airSeg.marketingFlightNumber() == 863);
    CPPUNIT_ASSERT(airSeg.operatingFlightNumber() == 863);
  }

  void testFlowJourneyCarrierReturnFalseWhenSegmentFlown()
  {
    AirSeg airSeg;
    airSeg.unflown() = false;
    CPPUNIT_ASSERT(!airSeg.flowJourneyCarrier());
  }

  void testFlowJourneyCarrierReturnFalseWhenCarrierPrefZero()
  {
    AirSeg airSeg;
    CPPUNIT_ASSERT(!airSeg.flowJourneyCarrier());
  }

  void testFlowJourneyCarrierReturnTrueWhenCarrierPrefSaysYes()
  {
    AirSeg airSeg;
    _carrierPref.flowMktJourneyType() = YES;
    airSeg.carrierPref() = &_carrierPref;
    CPPUNIT_ASSERT(airSeg.flowJourneyCarrier());
  }

  void testFlowJourneyCarrierReturnFalseWhenCarrierPrefSaysNo()
  {
    AirSeg airSeg;
    _carrierPref.flowMktJourneyType() = NO;
    airSeg.carrierPref() = &_carrierPref;
    CPPUNIT_ASSERT(!airSeg.flowJourneyCarrier());
  }

  void testLocalJourneyCarrierReturnFalseWhenSegmentFlown()
  {
    AirSeg airSeg;
    airSeg.unflown() = false;
    CPPUNIT_ASSERT(!airSeg.localJourneyCarrier());
  }

  void testLocalJourneyCarrierReturnFalseWhenCarrierPrefZero()
  {
    AirSeg airSeg;
    CPPUNIT_ASSERT(!airSeg.localJourneyCarrier());
  }

  void testLocalJourneyCarrierReturnTrueWhenCarrierPrefSaysYes()
  {
    AirSeg airSeg;
    _carrierPref.localMktJourneyType() = YES;
    airSeg.carrierPref() = &_carrierPref;
    CPPUNIT_ASSERT(airSeg.localJourneyCarrier());
  }

  void testLocalJourneyCarrierReturnFalseWhenCarrierPrefSaysNo()
  {
    AirSeg airSeg;
    _carrierPref.localMktJourneyType() = NO;
    airSeg.carrierPref() = &_carrierPref;
    CPPUNIT_ASSERT(!airSeg.localJourneyCarrier());
  }

  void testToAirSeg()
  {
    AirSeg airSeg;
    const AirSeg& airSegConst = airSeg;

    CPPUNIT_ASSERT(airSeg.toAirSeg());
    CPPUNIT_ASSERT(airSegConst.toAirSeg());
    CPPUNIT_ASSERT_NO_THROW(airSeg.toAirSegRef());
    CPPUNIT_ASSERT_NO_THROW(airSegConst.toAirSegRef());
  }

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  CarrierPreference _carrierPref;

}; // class

CPPUNIT_TEST_SUITE_REGISTRATION(AirSegTest);

} // namespace
