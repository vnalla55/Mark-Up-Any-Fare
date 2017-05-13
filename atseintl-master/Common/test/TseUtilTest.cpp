#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "Common/TseUtil.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingRequest.h"
#include "Common/TrxUtil.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/include/TestMemHandle.h"
#include "test/LocGenerator.h"
#include "DataModel/FlightFinderTrx.h"

namespace tse
{
class TseUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TseUtilTest);
  CPPUNIT_TEST(testGreatCircleMilesSameHemisphere);
  CPPUNIT_TEST(testGreatCircleMilesInvalidHemisphere);
  CPPUNIT_TEST(testGreatCircleMilesSameLatHemisphere);
  CPPUNIT_TEST(testGreatCircleMilesSameLngHemisphere);
  CPPUNIT_TEST(testGreatCircleMilesDifferentHemisphere);
  CPPUNIT_TEST(testGreatCircleMilesSameLocations);
  CPPUNIT_TEST(testStringGetGeoType);
  CPPUNIT_TEST(testGTTGetGeoType);
  CPPUNIT_TEST(testPaxTypeFareOperatorShouldReturnFalseWhenFareIsNull);
  CPPUNIT_TEST(testPaxTypeFareOperatorShouldReturnFalseWhenRoundTripFareOnOneWayTrip);
  CPPUNIT_TEST(testPaxTypeFareOperatorShouldReturnFalseWhenOneWayFareOnRoundTrip);
  CPPUNIT_TEST(testPaxTypeFareOperatorShouldReturnTrueWhenOneWayFareOnOneWayTrip);
  CPPUNIT_TEST(testPaxTypeFareOperatorShouldReturnTrueRoundTripFareOnRoundTrip);
  CPPUNIT_TEST_SUITE_END();

public:
  //-------------------------------------------------------------------
  // setUp()
  //-------------------------------------------------------------------
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  //-------------------------------------------------------------------
  // tearDown()
  //-------------------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

  void testGreatCircleMilesSameHemisphere()
  {
    std::pair<Loc*, Loc*> sh118 = _locGenerator.distance118SameHem();
    CPPUNIT_ASSERT_EQUAL(118u, TseUtil::greatCircleMiles(*sh118.first, *sh118.second));
  }

  // This test shows an example of an invalid value being passed
  // in for the latitude hemisphere.  The code puts in a default
  // value but continues to calculates a sometimes meaningless value.
  // I've included this test in order to show no errors are thrown.
  void testGreatCircleMilesInvalidHemisphere()
  {
    Loc* location1 = _locGenerator.createLoc('x', 70, 0, 0, 'E', 70, 0, 0, "ABC");
    Loc* location2 = _locGenerator.createLoc('x', 70, 0, 0, 'E', 75, 0, 0, "DEF");

    CPPUNIT_ASSERT_EQUAL(345u, TseUtil::greatCircleMiles(*location1, *location2));
  }

  void testGreatCircleMilesSameLatHemisphere()
  {
    std::pair<Loc*, Loc*> slh2641 = _locGenerator.distance2641SameLat();

    // Expect around 2629 miles
    CPPUNIT_ASSERT_EQUAL(2641u, TseUtil::greatCircleMiles(*slh2641.first, *slh2641.second));
  }

  void testGreatCircleMilesSameLngHemisphere()
  {
    std::pair<Loc*, Loc*> slh9669 = _locGenerator.distance9657SameLng();

    // Expect around 9669 miles
    CPPUNIT_ASSERT_EQUAL(9657u, TseUtil::greatCircleMiles(*slh9669.first, *slh9669.second));
  }

  void testGreatCircleMilesDifferentHemisphere()
  {
    std::pair<Loc*, Loc*> dh11613 = _locGenerator.distance11613DiffHem();

    // Expect around 11613 miles
    CPPUNIT_ASSERT_EQUAL(11613u, TseUtil::greatCircleMiles(*dh11613.first, *dh11613.second));
  }

  void testGreatCircleMilesSameLocations()
  {
    Loc* location1 = _locGenerator.createLoc('N', 70, 0, 0, 'W', 70, 0, 0, "ABC");
    Loc* location2 = _locGenerator.createLoc('N', 70, 0, 0, 'W', 70, 0, 0, "ABC");

    CPPUNIT_ASSERT_EQUAL(0u, TseUtil::greatCircleMiles(*location1, *location2));
  }

  void testStringGetGeoType()
  {
    CPPUNIT_ASSERT_EQUAL((std::string) "", TseUtil::getGeoType(GeoTravelType::UnknownGeoTravelType));
    CPPUNIT_ASSERT_EQUAL((std::string) "DOM", TseUtil::getGeoType(GeoTravelType::Domestic));
    CPPUNIT_ASSERT_EQUAL((std::string) "INT", TseUtil::getGeoType(GeoTravelType::International));
    CPPUNIT_ASSERT_EQUAL((std::string) "TRB", TseUtil::getGeoType(GeoTravelType::Transborder));
    CPPUNIT_ASSERT_EQUAL((std::string) "FDM", TseUtil::getGeoType(GeoTravelType::ForeignDomestic));
  }

  void testGTTGetGeoType()
  {
    CPPUNIT_ASSERT(GeoTravelType::UnknownGeoTravelType == TseUtil::getGeoType('U'));
    CPPUNIT_ASSERT(GeoTravelType::Domestic == TseUtil::getGeoType('D'));
    CPPUNIT_ASSERT(GeoTravelType::International == TseUtil::getGeoType('I'));
    CPPUNIT_ASSERT(GeoTravelType::Transborder == TseUtil::getGeoType('T'));
    CPPUNIT_ASSERT(GeoTravelType::ForeignDomestic == TseUtil::getGeoType('F'));
  }

  void testPaxTypeFareOperatorShouldReturnFalseWhenFareIsNull()
  {
    FlightFinderTrx trx;
    TseUtil::FFOwrtApplicabilityPred prd(trx);
    PaxTypeFare* fare = 0;
    CPPUNIT_ASSERT_EQUAL(false, prd(fare));
  }

  void testPaxTypeFareOperatorShouldReturnFalseWhenRoundTripFareOnOneWayTrip()
  {
    FlightFinderTrx trx;
    TseUtil::FFOwrtApplicabilityPred prd(trx);
    PaxTypeFare paxTypeFare;
    Fare fare;
    FareInfo fareInfo;
    fare.setFareInfo(&fareInfo);
    paxTypeFare.setFare(&fare);
    fareInfo.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    trx.journeyItin() = _memHandle.create<Itin>();

    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    seg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");
    seg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    seg->stopOver() = false;
    seg->carrier() = "AA";
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();
    CPPUNIT_ASSERT(seg != 0);

    trx.journeyItin()->travelSeg().push_back(seg);

    CPPUNIT_ASSERT_EQUAL(false, prd(&paxTypeFare));
  }

  void testPaxTypeFareOperatorShouldReturnFalseWhenOneWayFareOnRoundTrip()
  {
    FlightFinderTrx trx;
    TseUtil::FFOwrtApplicabilityPred prd(trx);
    PaxTypeFare paxTypeFare;
    Fare fare;
    FareInfo fareInfo;
    fare.setFareInfo(&fareInfo);
    paxTypeFare.setFare(&fare);
    fareInfo.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    trx.journeyItin() = _memHandle.create<Itin>();

    AirSeg* seg = createAirSeg();

    // add segment twice to look like a round trip
    trx.journeyItin()->travelSeg().push_back(seg);
    trx.journeyItin()->travelSeg().push_back(seg);

    CPPUNIT_ASSERT_EQUAL(false, prd(&paxTypeFare));
  }

  void testPaxTypeFareOperatorShouldReturnTrueWhenOneWayFareOnOneWayTrip()
  {
    FlightFinderTrx trx;
    TseUtil::FFOwrtApplicabilityPred prd(trx);
    PaxTypeFare paxTypeFare;
    Fare fare;
    FareInfo fareInfo;
    fare.setFareInfo(&fareInfo);
    paxTypeFare.setFare(&fare);
    fareInfo.owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    trx.journeyItin() = _memHandle.create<Itin>();

    AirSeg* seg = createAirSeg();

    // add segment twice to look like a round trip
    trx.journeyItin()->travelSeg().push_back(seg);

    CPPUNIT_ASSERT_EQUAL(true, prd(&paxTypeFare));
  }

  void testPaxTypeFareOperatorShouldReturnTrueRoundTripFareOnRoundTrip()
  {
    FlightFinderTrx trx;
    TseUtil::FFOwrtApplicabilityPred prd(trx);
    PaxTypeFare paxTypeFare;
    Fare fare;
    FareInfo fareInfo;
    fare.setFareInfo(&fareInfo);
    paxTypeFare.setFare(&fare);
    fareInfo.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    trx.journeyItin() = _memHandle.create<Itin>();

    AirSeg* seg = createAirSeg();

    // add segment twice to look like a round trip
    trx.journeyItin()->travelSeg().push_back(seg);
    trx.journeyItin()->travelSeg().push_back(seg);

    CPPUNIT_ASSERT_EQUAL(true, prd(&paxTypeFare));
  }

  AirSeg* createAirSeg()
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    seg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");
    seg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    seg->stopOver() = false;
    seg->carrier() = "AA";
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();
    return seg;
  }

private:
  TestMemHandle _memHandle;
  LocGenerator _locGenerator;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TseUtilTest);
}
