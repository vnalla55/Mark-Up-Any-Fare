#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class TravelSegTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TravelSegTest);
  CPPUNIT_TEST(testSetGet);
  CPPUNIT_TEST(testBookedCabinCharPremiumFirst);
  CPPUNIT_TEST(testBookedCabinCharFirst);
  CPPUNIT_TEST(testBookedCabinCharPremiumBusiness);
  CPPUNIT_TEST(testBookedCabinCharBusiness);
  CPPUNIT_TEST(testBookedCabinCharPremiumEconomy);
  CPPUNIT_TEST(testBookedCabinCharEconomy);

  CPPUNIT_TEST(testBookedCabinCharAnswerPremiumFirst);
  CPPUNIT_TEST(testBookedCabinCharAnswerFirst);
  CPPUNIT_TEST(testBookedCabinCharAnswerPremiumBusiness);
  CPPUNIT_TEST(testBookedCabinCharAnswerBusiness);
  CPPUNIT_TEST(testBookedCabinCharAnswerPremiumEconomy);
  CPPUNIT_TEST(testBookedCabinCharAnswerEconomy);


  CPPUNIT_TEST(testBookedCabinCharUnknown);
  CPPUNIT_TEST(testBookedCabinCharUndefined);
  CPPUNIT_TEST(testBookedCabinCharInvalid);

  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnTrueWhenBoardAndOffSame);
  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnTrueWhenBoardEWRandOffNYC);
  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnFalseWhenBoardEWRandOffNotNYC);
  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnTrueWhenOffEWRandBoardNYC);
  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnFalseWhenOffEWRandBoardNotNYC);
  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnTrueWhenBoardBWIandOffWAS);
  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnFalseWhenBoardBWIandOffNotWAS);
  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnTrueWhenOffBWIandBoardWAS);
  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnFalseWhenOffBWIandBoardNotWAS);
  CPPUNIT_TEST(testArunkMultiAirportForAvailabilityReturnFalseWhenBoardOffNotSame);

  CPPUNIT_TEST(testIsStopOverReturnFalseWhenTravelSeg0);
  CPPUNIT_TEST(testIsStopOverReturnFalseWhenPrevSegForceCnx);
  CPPUNIT_TEST(testIsStopOverReturnFalseWhenSecondsDiffEqualInputSeconds);
  CPPUNIT_TEST(testIsStopOverReturnFalseWhenSecondsDiffLessThanInputSeconds);
  CPPUNIT_TEST(testIsStopOverReturnTrueWhenSecondsDiffMoreThanInputSeconds);

  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnFalseWhenTravelSeg0);
  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnFalseWhenSecondsDiffEqualInputSeconds);
  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnFalseWhenSecondsDiffLessThanInputSeconds);
  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnTrueWhenSecondsDiffMoreThanInputSeconds);
  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnFalseWhenPrevSegArunkAndDiffEqualInputSeconds);
  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnFalseWhenPrevSegArunkAndDiffLessThanInputSeconds);
  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnTrueWhenPrevSegArunkAndDiffMoreThanInputSeconds);
  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnFalseWhenCurrentSegArunkAndDiffEqualInputSeconds);
  CPPUNIT_TEST(
      testIsStopOverWithOutForceCnxReturnFalseWhenCurrentSegArunkAndDiffLessThanInputSeconds);
  CPPUNIT_TEST(
      testIsStopOverWithOutForceCnxReturnTrueWhenCurrentSegArunkAndDiffMoreThanInputSeconds);
  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnTrueWhenOneSegOpenAndDepDatesNotSameDay);
  CPPUNIT_TEST(testIsStopOverWithOutForceCnxReturnFalseWhenOneSegOpenAndDepDatesSameDay);
  CPPUNIT_TEST(testFbcUsageGetterSetter);
  CPPUNIT_TEST(testCompareSegOrderBasedOnItin);

  CPPUNIT_TEST(testIsNonAirTransportationReturnTrueWhenArunk);
  CPPUNIT_TEST(testIsNonAirTransportationReturnTrueWhenTrain);
  CPPUNIT_TEST(testIsNonAirTransportationReturnTrueWhenTgv);
  CPPUNIT_TEST(testIsNonAirTransportationReturnTrueWhenBus);
  CPPUNIT_TEST(testIsNonAirTransportationReturnTrueWhenBoat);
  CPPUNIT_TEST(testIsNonAirTransportationReturnTrueWhenIce);
  CPPUNIT_TEST(testIsNonAirTransportationReturnTrueWhenLmo);
  CPPUNIT_TEST(testIsNonAirTransportationReturnFalseWhen737);
  CPPUNIT_TEST(testIsNonAirTransportationReturnFalseWhen330);
  CPPUNIT_TEST(testSegmentBrandCodeComparatorForSegmentWithoutBrand);
  CPPUNIT_TEST(testSegmentBrandCodeComparatorForSegmentWithBrand);
  CPPUNIT_TEST_SUITE_END();

public:
  AirSeg* _airSeg;
  TestMemHandle _memHandle;

  void setUp() { _airSeg = _memHandle.create<AirSeg>(); }

  void tearDown() { _memHandle.clear(); }

  void testSetGet()
  {
    // not much of a test...
    std::string orig("DFW");
    std::string dest("NYC");
    Loc loc1 = createCityLoc(orig, "1", "2", "US", "TX");
    Loc loc2 = createCityLoc(dest, "1", "2", "US", "TX");

    _airSeg->pnrSegment() = 1;
    _airSeg->segmentOrder() = 1;
    _airSeg->segmentType() = UnknownTravelSegType;
    DateTime today;
    _airSeg->departureDT() = today;
    _airSeg->bookingDT() = today;
    _airSeg->arrivalDT() = today;
    _airSeg->setBookingCode("YY");
    _airSeg->resStatus() = "ABC";
    _airSeg->origAirport() = orig;
    _airSeg->destAirport() = dest;
    _airSeg->fareBasisCode() = "ABC";
    _airSeg->forcedConx() = 'Y';
    _airSeg->forcedStopOver() = 'Y';
    _airSeg->boardMultiCity() = orig;
    _airSeg->offMultiCity() = dest;

    _airSeg->validatedBookingCode() = "Y";
    _airSeg->geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    _airSeg->stopOver() = true;
    _airSeg->origin() = &loc1;
    _airSeg->destination() = &loc2;
    _airSeg->equipmentType() = "777";
    _airSeg->furthestPoint() = true;

    _airSeg->initialize();

    CPPUNIT_ASSERT(GeoTravelType::Domestic == _airSeg->geoTravelType());
  }

  Loc createCityLoc(const std::string& locName,
                    const std::string& subarea,
                    const std::string& area,
                    const std::string& nation,
                    const std::string& state)
  {
    Loc loc;
    loc.loc() = locName;
    loc.subarea() = subarea;
    loc.area() = area;
    loc.nation() = nation;
    loc.state() = state;
    loc.cityInd() = true;
    return loc;
  }

  void testBookedCabinCharPremiumFirst()
  {
    _airSeg->bookedCabin().setPremiumFirstClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::PREMIUM_FIRST_CLASS, _airSeg->bookedCabinChar());
  }
  void testBookedCabinCharFirst()
  {
    _airSeg->bookedCabin().setFirstClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::FIRST_CLASS, _airSeg->bookedCabinChar());
  }
  void testBookedCabinCharPremiumBusiness()
  {
    _airSeg->bookedCabin().setPremiumBusinessClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::PREMIUM_BUSINESS_CLASS, _airSeg->bookedCabinChar());
  }
  void testBookedCabinCharBusiness()
  {
    _airSeg->bookedCabin().setBusinessClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::BUSINESS_CLASS, _airSeg->bookedCabinChar());
  }
  void testBookedCabinCharPremiumEconomy()
  {
    _airSeg->bookedCabin().setPremiumEconomyClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::PREMIUM_ECONOMY_CLASS, _airSeg->bookedCabinChar());
  }
  void testBookedCabinCharEconomy()
  {
    _airSeg->bookedCabin().setEconomyClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::ECONOMY_CLASS, _airSeg->bookedCabinChar());
  }
//
  void testBookedCabinCharAnswerPremiumFirst()
  {
    _airSeg->bookedCabin().setPremiumFirstClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::PREMIUM_FIRST_CLASS_ANSWER, _airSeg->bookedCabinCharAnswer());
  }
  void testBookedCabinCharAnswerFirst()
  {
    _airSeg->bookedCabin().setFirstClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::FIRST_CLASS_ANSWER, _airSeg->bookedCabinCharAnswer());
  }
  void testBookedCabinCharAnswerPremiumBusiness()
  {
    _airSeg->bookedCabin().setPremiumBusinessClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::PREMIUM_BUSINESS_CLASS_ANSWER, _airSeg->bookedCabinCharAnswer());
  }
  void testBookedCabinCharAnswerBusiness()
  {
    _airSeg->bookedCabin().setBusinessClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::BUSINESS_CLASS_ANSWER, _airSeg->bookedCabinCharAnswer());
  }
  void testBookedCabinCharAnswerPremiumEconomy()
  {
    _airSeg->bookedCabin().setPremiumEconomyClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::PREMIUM_ECONOMY_CLASS_ANSWER, _airSeg->bookedCabinCharAnswer());
  }
  void testBookedCabinCharAnswerEconomy()
  {
    _airSeg->bookedCabin().setEconomyClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::ECONOMY_CLASS_ANSWER, _airSeg->bookedCabinCharAnswer());
  }
//
  void testBookedCabinCharUnknown()
  {
    _airSeg->bookedCabin().setUnknownClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::ECONOMY_CLASS, _airSeg->bookedCabinChar());
  }
  void testBookedCabinCharUndefined()
  {
    _airSeg->bookedCabin().setUndefinedClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::ECONOMY_CLASS, _airSeg->bookedCabinChar());
  }
  void testBookedCabinCharInvalid()
  {
    _airSeg->bookedCabin().setInvalidClass();
    CPPUNIT_ASSERT_EQUAL((char)TravelSeg::ECONOMY_CLASS, _airSeg->bookedCabinChar());
  }
  void testArunkMultiAirportForAvailabilityReturnTrueWhenBoardAndOffSame()
  {
    _airSeg->boardMultiCity() = _airSeg->offMultiCity() = LOC_NYC;
    CPPUNIT_ASSERT(_airSeg->arunkMultiAirportForAvailability());
  }
  void testArunkMultiAirportForAvailabilityReturnTrueWhenBoardEWRandOffNYC()
  {
    _airSeg->boardMultiCity() = LOC_EWR;
    _airSeg->offMultiCity() = LOC_NYC;
    CPPUNIT_ASSERT(_airSeg->arunkMultiAirportForAvailability());
  }
  void testArunkMultiAirportForAvailabilityReturnFalseWhenBoardEWRandOffNotNYC()
  {
    _airSeg->boardMultiCity() = LOC_EWR;
    _airSeg->offMultiCity() = LOC_KUL;
    CPPUNIT_ASSERT(!_airSeg->arunkMultiAirportForAvailability());
  }
  void testArunkMultiAirportForAvailabilityReturnTrueWhenOffEWRandBoardNYC()
  {
    _airSeg->boardMultiCity() = LOC_NYC;
    _airSeg->offMultiCity() = LOC_EWR;
    CPPUNIT_ASSERT(_airSeg->arunkMultiAirportForAvailability());
  }
  void testArunkMultiAirportForAvailabilityReturnFalseWhenOffEWRandBoardNotNYC()
  {
    _airSeg->boardMultiCity() = LOC_KUL;
    _airSeg->offMultiCity() = LOC_EWR;
    CPPUNIT_ASSERT(!_airSeg->arunkMultiAirportForAvailability());
  }
  void testArunkMultiAirportForAvailabilityReturnTrueWhenBoardBWIandOffWAS()
  {
    _airSeg->boardMultiCity() = LOC_BWI;
    _airSeg->offMultiCity() = LOC_WAS;
    CPPUNIT_ASSERT(_airSeg->arunkMultiAirportForAvailability());
  }
  void testArunkMultiAirportForAvailabilityReturnFalseWhenBoardBWIandOffNotWAS()
  {
    _airSeg->boardMultiCity() = LOC_BWI;
    _airSeg->offMultiCity() = LOC_KUL;
    CPPUNIT_ASSERT(!_airSeg->arunkMultiAirportForAvailability());
  }
  void testArunkMultiAirportForAvailabilityReturnTrueWhenOffBWIandBoardWAS()
  {
    _airSeg->boardMultiCity() = LOC_WAS;
    _airSeg->offMultiCity() = LOC_BWI;
    CPPUNIT_ASSERT(_airSeg->arunkMultiAirportForAvailability());
  }
  void testArunkMultiAirportForAvailabilityReturnFalseWhenOffBWIandBoardNotWAS()
  {
    _airSeg->boardMultiCity() = LOC_KUL;
    _airSeg->offMultiCity() = LOC_BWI;
    CPPUNIT_ASSERT(!_airSeg->arunkMultiAirportForAvailability());
  }
  void testArunkMultiAirportForAvailabilityReturnFalseWhenBoardOffNotSame()
  {
    _airSeg->boardMultiCity() = LOC_KUL;
    _airSeg->offMultiCity() = LOC_NYC;
    CPPUNIT_ASSERT(!_airSeg->arunkMultiAirportForAvailability());
  }
  void testIsStopOverReturnFalseWhenTravelSeg0()
  {
    TravelSeg* prevSeg = 0;
    CPPUNIT_ASSERT(!_airSeg->isStopOver(prevSeg, 14400));
  }
  void testIsStopOverReturnFalseWhenPrevSegForceCnx()
  {
    AirSeg prevSeg;
    prevSeg.forcedConx() = 'T';
    CPPUNIT_ASSERT(!_airSeg->isStopOver(&prevSeg, 14400));
  }
  void testIsStopOverReturnFalseWhenSecondsDiffEqualInputSeconds()
  {
    AirSeg prevSeg;
    DateTime prevArrival = DateTime(2009, 7, 10, 8, 15, 0);
    prevSeg.arrivalDT() = prevArrival;
    _airSeg->departureDT() = prevArrival.addSeconds(14400);
    CPPUNIT_ASSERT(!_airSeg->isStopOver(&prevSeg, 14400));
  }
  void testIsStopOverReturnFalseWhenSecondsDiffLessThanInputSeconds()
  {
    AirSeg prevSeg;
    DateTime prevArrival = DateTime(2009, 7, 10, 8, 15, 0);
    prevSeg.arrivalDT() = prevArrival;
    _airSeg->departureDT() = prevArrival.addSeconds(14399);
    CPPUNIT_ASSERT(!_airSeg->isStopOver(&prevSeg, 14400));
  }
  void testIsStopOverReturnTrueWhenSecondsDiffMoreThanInputSeconds()
  {
    AirSeg prevSeg;
    DateTime prevArrival = DateTime(2009, 7, 10, 8, 15, 0);
    prevSeg.arrivalDT() = prevArrival;
    _airSeg->departureDT() = prevArrival.addSeconds(14401);
    CPPUNIT_ASSERT(_airSeg->isStopOver(&prevSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnFalseWhenTravelSeg0()
  {
    TravelSeg* prevSeg = 0;
    CPPUNIT_ASSERT(!_airSeg->isStopOverWithOutForceCnx(prevSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnFalseWhenSecondsDiffEqualInputSeconds()
  {
    AirSeg prevSeg;
    DateTime prevArrival = DateTime(2009, 7, 10, 8, 15, 0);
    prevSeg.arrivalDT() = prevArrival;
    _airSeg->departureDT() = prevArrival.addSeconds(14400);
    CPPUNIT_ASSERT(!_airSeg->isStopOverWithOutForceCnx(&prevSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnFalseWhenSecondsDiffLessThanInputSeconds()
  {
    AirSeg prevSeg;
    DateTime prevArrival = DateTime(2009, 7, 10, 8, 15, 0);
    prevSeg.arrivalDT() = prevArrival;
    _airSeg->departureDT() = prevArrival.addSeconds(14399);
    CPPUNIT_ASSERT(!_airSeg->isStopOverWithOutForceCnx(&prevSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnTrueWhenSecondsDiffMoreThanInputSeconds()
  {
    AirSeg prevSeg;
    DateTime prevArrival = DateTime(2009, 7, 10, 8, 15, 0);
    prevSeg.arrivalDT() = prevArrival;
    _airSeg->departureDT() = prevArrival.addSeconds(14401);
    CPPUNIT_ASSERT(_airSeg->isStopOverWithOutForceCnx(&prevSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnFalseWhenPrevSegArunkAndDiffEqualInputSeconds()
  {
    ArunkSeg prevSeg;
    prevSeg.segmentType() = Arunk;
    DateTime prevDep = DateTime(2009, 7, 10, 8, 15, 0);
    prevSeg.departureDT() = prevDep;
    prevSeg.arrivalDT() = prevDep.addSeconds(14400);
    CPPUNIT_ASSERT(!_airSeg->isStopOverWithOutForceCnx(&prevSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnFalseWhenPrevSegArunkAndDiffLessThanInputSeconds()
  {
    ArunkSeg prevSeg;
    prevSeg.segmentType() = Arunk;
    DateTime prevDep = DateTime(2009, 7, 10, 8, 15, 0);
    prevSeg.departureDT() = prevDep;
    prevSeg.arrivalDT() = prevDep.addSeconds(14399);
    CPPUNIT_ASSERT(!_airSeg->isStopOverWithOutForceCnx(&prevSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnTrueWhenPrevSegArunkAndDiffMoreThanInputSeconds()
  {
    ArunkSeg prevSeg;
    prevSeg.segmentType() = Arunk;
    DateTime prevDep = DateTime(2009, 7, 10, 8, 15, 0);
    prevSeg.departureDT() = prevDep;
    prevSeg.arrivalDT() = prevDep.addSeconds(14401);
    CPPUNIT_ASSERT(_airSeg->isStopOverWithOutForceCnx(&prevSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnFalseWhenCurrentSegArunkAndDiffEqualInputSeconds()
  {
    ArunkSeg currSeg;
    currSeg.segmentType() = Arunk;
    DateTime currDep = DateTime(2009, 7, 10, 8, 15, 0);
    currSeg.departureDT() = currDep;
    currSeg.arrivalDT() = currDep.addSeconds(14400);
    CPPUNIT_ASSERT(!currSeg.isStopOverWithOutForceCnx(_airSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnFalseWhenCurrentSegArunkAndDiffLessThanInputSeconds()
  {
    ArunkSeg currSeg;
    currSeg.segmentType() = Arunk;
    DateTime currDep = DateTime(2009, 7, 10, 8, 15, 0);
    currSeg.departureDT() = currDep;
    currSeg.arrivalDT() = currDep.addSeconds(14399);
    CPPUNIT_ASSERT(!currSeg.isStopOverWithOutForceCnx(_airSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnTrueWhenCurrentSegArunkAndDiffMoreThanInputSeconds()
  {
    ArunkSeg currSeg;
    currSeg.segmentType() = Arunk;
    DateTime currDep = DateTime(2009, 7, 10, 8, 15, 0);
    currSeg.departureDT() = currDep;
    currSeg.arrivalDT() = currDep.addSeconds(14401);
    CPPUNIT_ASSERT(currSeg.isStopOverWithOutForceCnx(_airSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnTrueWhenOneSegOpenAndDepDatesNotSameDay()
  {
    AirSeg prevSeg;
    prevSeg.segmentType() = Open;
    prevSeg.departureDT() = DateTime(2009, 7, 10, 8, 15, 0);
    ;
    _airSeg->departureDT() = DateTime(2009, 7, 11, 8, 15, 0);
    CPPUNIT_ASSERT(_airSeg->isStopOverWithOutForceCnx(&prevSeg, 14400));
  }
  void testIsStopOverWithOutForceCnxReturnFalseWhenOneSegOpenAndDepDatesSameDay()
  {
    AirSeg prevSeg;
    prevSeg.segmentType() = Open;
    prevSeg.departureDT() = DateTime(2009, 7, 10, 8, 15, 0);
    ;
    _airSeg->departureDT() = DateTime(2009, 7, 10, 8, 15, 0);
    CPPUNIT_ASSERT(!_airSeg->isStopOverWithOutForceCnx(&prevSeg, 14400));
  }

  void testFbcUsageGetterSetter()
  {
    _airSeg->fbcUsage() = FILTER_FBC;
    CPPUNIT_ASSERT_EQUAL(FILTER_FBC, _airSeg->fbcUsage());
  }

  void testCompareSegOrderBasedOnItin()
  {
    Itin itin;
    AirSeg tvlSeg1, tvlSeg2;
    CompareSegOrderBasedOnItin comparor(&itin);

    itin.travelSeg().push_back(&tvlSeg1);
    itin.travelSeg().push_back(&tvlSeg2);
    CPPUNIT_ASSERT(comparor(&tvlSeg1, &tvlSeg2));

    itin.travelSeg().clear();
    itin.travelSeg().push_back(&tvlSeg2);
    itin.travelSeg().push_back(&tvlSeg1);
    CPPUNIT_ASSERT(!comparor(&tvlSeg1, &tvlSeg2));
  }
  void testIsNonAirTransportationReturnTrueWhenArunk()
  {
    ArunkSeg arunk;
    CPPUNIT_ASSERT(arunk.isNonAirTransportation());
  }

  void testIsNonAirTransportationReturnTrueWhenTrain()
  {
    _airSeg->equipmentType() = TRAIN;
    CPPUNIT_ASSERT(_airSeg->isNonAirTransportation());
  }

  void testIsNonAirTransportationReturnTrueWhenTgv()
  {
    _airSeg->equipmentType() = TRAIN;
    CPPUNIT_ASSERT(_airSeg->isNonAirTransportation());
  }

  void testIsNonAirTransportationReturnTrueWhenBus()
  {
    _airSeg->equipmentType() = TRAIN;
    CPPUNIT_ASSERT(_airSeg->isNonAirTransportation());
  }

  void testIsNonAirTransportationReturnTrueWhenBoat()
  {
    _airSeg->equipmentType() = TRAIN;
    CPPUNIT_ASSERT(_airSeg->isNonAirTransportation());
  }

  void testIsNonAirTransportationReturnTrueWhenIce()
  {
    _airSeg->equipmentType() = TRAIN;
    CPPUNIT_ASSERT(_airSeg->isNonAirTransportation());
  }

  void testIsNonAirTransportationReturnTrueWhenLmo()
  {
    _airSeg->equipmentType() = TRAIN;
    CPPUNIT_ASSERT(_airSeg->isNonAirTransportation());
  }

  void testIsNonAirTransportationReturnFalseWhen737()
  {
    _airSeg->equipmentType() = "737";
    CPPUNIT_ASSERT(!_airSeg->isNonAirTransportation());
  }

  void testIsNonAirTransportationReturnFalseWhen330()
  {
    _airSeg->equipmentType() = "330";
    CPPUNIT_ASSERT(!_airSeg->isNonAirTransportation());
  }

  void testSegmentBrandCodeComparatorForSegmentWithBrand()
  {
    AirSeg tvlSeg;
    tvlSeg.setBrandCode("AA");
    SegmentBrandCodeComparator comparator(&tvlSeg);

    AirSeg comparedTvlSeg;
    comparedTvlSeg.setBrandCode("AA");
    CPPUNIT_ASSERT(comparator(&comparedTvlSeg));

    comparedTvlSeg.setBrandCode("BB");
    CPPUNIT_ASSERT(!comparator(&comparedTvlSeg));

    comparedTvlSeg.setBrandCode("");
    CPPUNIT_ASSERT(!comparator(&comparedTvlSeg));

    AirSeg arunk;
    arunk.segmentType() = Arunk;
    CPPUNIT_ASSERT(comparator(&arunk));

    arunk.setBrandCode("AA");
    CPPUNIT_ASSERT(comparator(&arunk));

    arunk.setBrandCode("BB");
    CPPUNIT_ASSERT(comparator(&arunk));
  }

  void testSegmentBrandCodeComparatorForSegmentWithoutBrand()
  {
    AirSeg tvlSeg;
    SegmentBrandCodeComparator comparator(&tvlSeg);

    AirSeg comparedTvlSeg;
    comparedTvlSeg.setBrandCode("AA");
    CPPUNIT_ASSERT(!comparator(&comparedTvlSeg));

    comparedTvlSeg.setBrandCode("BB");
    CPPUNIT_ASSERT(!comparator(&comparedTvlSeg));

    comparedTvlSeg.setBrandCode("");
    CPPUNIT_ASSERT(comparator(&comparedTvlSeg));

    AirSeg arunk;
    arunk.segmentType() = Arunk;
    CPPUNIT_ASSERT(comparator(&arunk));

    arunk.setBrandCode("AA");
    CPPUNIT_ASSERT(comparator(&arunk));

    arunk.setBrandCode("BB");
    CPPUNIT_ASSERT(comparator(&arunk));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TravelSegTest);
} // end namespace
