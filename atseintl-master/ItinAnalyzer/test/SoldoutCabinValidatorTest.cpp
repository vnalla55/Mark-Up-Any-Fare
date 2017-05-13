#include "test/include/CppUnitHelperMacros.h"
#include <gmock/gmock.h>

#include "Common/ClassOfService.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/PaxTypeInfo.h"
#include "ItinAnalyzer/SoldoutCabinValidator.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using namespace ::testing;

class SoldoutCabinValidatorMock : public tse::iadetail::SoldoutCabinValidator
{
public:
  MOCK_METHOD1(isReqAwardAltDatesRT, bool(PricingTrx&));
  MOCK_METHOD4(isCabinAvailable, bool(PricingTrx&, CabinType&, Itin&, StatusMap&));
};

class SoldoutCabinValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoldoutCabinValidatorTest);

  CPPUNIT_TEST(testCalendarShopingCabinValidation_OneEconomyAvail_OneEconomyRequested);
  CPPUNIT_TEST(testCalendarShopingCabinValidation_OneEconomyAvail_TwoEconomyRequested);
  CPPUNIT_TEST(testCalendarShopingCabinValidation_TwoBusinessAvail_TwoBusinessRequested);
  CPPUNIT_TEST(testCalendarShopingCabinValidation_NoEconomyAvail_OneEconomyRequested);
  CPPUNIT_TEST(
      testCalendarShopingCabinValidation_2DatePairs_2ItinPerDate_EachItinHave3Seg_TwoEconomyAvailonFirstItin_OneEconomyRequested);
  CPPUNIT_TEST(
      testCalendarShopingCabinValidation_2DatePairs_2ItinPerDate_EachItinHave3Seg_TwoEconomyAvailonLastItin_OneEconomyRequested);
  CPPUNIT_TEST(test_depInDateEq_SCI252);


  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void setupAirSegForCalendar(PricingTrx& trx, AirSeg* airsegs)
  {
    airsegs[0].origAirport() = "KRK";
    airsegs[0].departureDT() = DateTime(2009, 7, 8);
    airsegs[0].origin() =
        trx.dataHandle().getLoc(airsegs[0].origAirport(), airsegs[0].departureDT());
    airsegs[0].destAirport() = "WAW";
    airsegs[0].arrivalDT() = DateTime(2009, 7, 8);
    airsegs[0].destination() =
        trx.dataHandle().getLoc(airsegs[0].destAirport(), airsegs[0].arrivalDT());
    airsegs[0].carrier() = "LO";
    airsegs[0].flightNumber() = 3912;
    airsegs[0].originalId() = 1;

    airsegs[1].origAirport() = "WAW";
    airsegs[1].departureDT() = DateTime(2009, 7, 8);
    airsegs[1].origin() =
        trx.dataHandle().getLoc(airsegs[1].origAirport(), airsegs[1].departureDT());
    airsegs[1].destAirport() = "ORD";
    airsegs[1].arrivalDT() = DateTime(2009, 7, 8);
    airsegs[1].destination() =
        trx.dataHandle().getLoc(airsegs[1].destAirport(), airsegs[1].arrivalDT());
    airsegs[1].carrier() = "LO";
    airsegs[1].flightNumber() = 3;
    airsegs[1].originalId() = 2;

    airsegs[2].origAirport() = "ORD";
    airsegs[2].departureDT() = DateTime(2009, 7, 9);
    airsegs[2].origin() =
        trx.dataHandle().getLoc(airsegs[2].origAirport(), airsegs[2].departureDT());
    airsegs[2].destAirport() = "DFW";
    airsegs[2].arrivalDT() = DateTime(2009, 7, 9);
    airsegs[2].destination() =
        trx.dataHandle().getLoc(airsegs[2].destAirport(), airsegs[2].arrivalDT());
    airsegs[2].carrier() = "UA";
    airsegs[2].flightNumber() = 6441;
    airsegs[2].originalId() = 3;

    airsegs[3].origAirport() = "KRK";
    airsegs[3].departureDT() = DateTime(2009, 7, 8);
    airsegs[3].origin() =
        trx.dataHandle().getLoc(airsegs[3].origAirport(), airsegs[3].departureDT());
    airsegs[3].destAirport() = "WAW";
    airsegs[3].arrivalDT() = DateTime(2009, 7, 8);
    airsegs[3].destination() =
        trx.dataHandle().getLoc(airsegs[3].destAirport(), airsegs[3].arrivalDT());
    airsegs[3].carrier() = "LO";
    airsegs[3].flightNumber() = 3913;
    airsegs[3].originalId() = 4;

    airsegs[4].origAirport() = "WAW";
    airsegs[4].departureDT() = DateTime(2009, 7, 8);
    airsegs[4].origin() =
        trx.dataHandle().getLoc(airsegs[4].origAirport(), airsegs[4].departureDT());
    airsegs[4].destAirport() = "ORD";
    airsegs[4].arrivalDT() = DateTime(2009, 7, 8);
    airsegs[4].destination() =
        trx.dataHandle().getLoc(airsegs[4].destAirport(), airsegs[4].arrivalDT());
    airsegs[4].carrier() = "LO";
    airsegs[4].flightNumber() = 4;
    airsegs[4].originalId() = 5;

    airsegs[5].origAirport() = "ORD";
    airsegs[5].departureDT() = DateTime(2009, 7, 9);
    airsegs[5].origin() =
        trx.dataHandle().getLoc(airsegs[5].origAirport(), airsegs[5].departureDT());
    airsegs[5].destAirport() = "DFW";
    airsegs[5].arrivalDT() = DateTime(2009, 7, 9);
    airsegs[5].destination() =
        trx.dataHandle().getLoc(airsegs[5].destAirport(), airsegs[5].arrivalDT());
    airsegs[5].carrier() = "AA";
    airsegs[5].flightNumber() = 6442;
    airsegs[5].originalId() = 6;

    airsegs[6].origAirport() = "KRK";
    airsegs[6].departureDT() = DateTime(2009, 7, 10);
    airsegs[6].origin() =
        trx.dataHandle().getLoc(airsegs[6].origAirport(), airsegs[6].departureDT());
    airsegs[6].destAirport() = "WAW";
    airsegs[6].arrivalDT() = DateTime(2009, 7, 10);
    airsegs[6].destination() =
        trx.dataHandle().getLoc(airsegs[6].destAirport(), airsegs[6].arrivalDT());
    airsegs[6].carrier() = "LO";
    airsegs[6].flightNumber() = 3906;
    airsegs[6].originalId() = 7;

    airsegs[7].origAirport() = "WAW";
    airsegs[7].departureDT() = DateTime(2009, 7, 10);
    airsegs[7].origin() =
        trx.dataHandle().getLoc(airsegs[7].origAirport(), airsegs[7].departureDT());
    airsegs[7].destAirport() = "ORD";
    airsegs[7].arrivalDT() = DateTime(2009, 7, 10);
    airsegs[7].destination() =
        trx.dataHandle().getLoc(airsegs[7].destAirport(), airsegs[7].arrivalDT());
    airsegs[7].carrier() = "LO";
    airsegs[7].flightNumber() = 3;
    airsegs[7].originalId() = 8;

    airsegs[8].origAirport() = "ORD";
    airsegs[8].departureDT() = DateTime(2009, 7, 11);
    airsegs[8].origin() =
        trx.dataHandle().getLoc(airsegs[8].origAirport(), airsegs[8].departureDT());
    airsegs[8].destAirport() = "DFW";
    airsegs[8].arrivalDT() = DateTime(2009, 7, 11);
    airsegs[8].destination() =
        trx.dataHandle().getLoc(airsegs[8].destAirport(), airsegs[8].arrivalDT());
    airsegs[8].carrier() = "AA";
    airsegs[8].flightNumber() = 2357;
    airsegs[8].originalId() = 9;

    airsegs[9].origAirport() = "KRK";
    airsegs[9].departureDT() = DateTime(2009, 7, 10);
    airsegs[9].origin() =
        trx.dataHandle().getLoc(airsegs[9].origAirport(), airsegs[9].departureDT());
    airsegs[9].destAirport() = "WAW";
    airsegs[9].arrivalDT() = DateTime(2009, 7, 10);
    airsegs[9].destination() =
        trx.dataHandle().getLoc(airsegs[9].destAirport(), airsegs[9].arrivalDT());
    airsegs[9].carrier() = "LO";
    airsegs[9].flightNumber() = 3907;
    airsegs[9].originalId() = 10;

    airsegs[10].origAirport() = "WAW";
    airsegs[10].departureDT() = DateTime(2009, 7, 10);
    airsegs[10].origin() =
        trx.dataHandle().getLoc(airsegs[10].origAirport(), airsegs[10].departureDT());
    airsegs[10].destAirport() = "ORD";
    airsegs[10].arrivalDT() = DateTime(2009, 7, 10);
    airsegs[10].destination() =
        trx.dataHandle().getLoc(airsegs[10].destAirport(), airsegs[10].arrivalDT());
    airsegs[10].carrier() = "LO";
    airsegs[10].flightNumber() = 4;
    airsegs[10].originalId() = 11;

    airsegs[11].origAirport() = "ORD";
    airsegs[11].departureDT() = DateTime(2009, 7, 11);
    airsegs[11].origin() =
        trx.dataHandle().getLoc(airsegs[11].origAirport(), airsegs[11].departureDT());
    airsegs[11].destAirport() = "DFW";
    airsegs[11].arrivalDT() = DateTime(2009, 7, 11);
    airsegs[11].destination() =
        trx.dataHandle().getLoc(airsegs[11].destAirport(), airsegs[11].arrivalDT());
    airsegs[11].carrier() = "AA";
    airsegs[11].flightNumber() = 2358;
    airsegs[11].originalId() = 12;
  }

  void setupClassOfService(PricingTrx& trx, ClassOfService* classOfService)
  {
    // oneEconomyClasses
    classOfService[0].numSeats() = 1;
    classOfService[0].cabin().setEconomyClass();
    classOfService[0].bookingCode() = BookingCode("Y");

    // twoEconomyClasses
    classOfService[1].numSeats() = 2;
    classOfService[1].cabin().setEconomyClass();
    classOfService[1].bookingCode() = BookingCode("Y");

    // oneBusinessClass
    classOfService[2].numSeats() = 1;
    classOfService[2].cabin().setBusinessClass();
    classOfService[2].bookingCode() = BookingCode("C");

    // twoBusinessClasses
    classOfService[3].numSeats() = 2;
    classOfService[3].cabin().setBusinessClass();
    classOfService[3].bookingCode() = BookingCode("C");

    // oneFirstClass
    classOfService[4].numSeats() = 1;
    classOfService[4].cabin().setFirstClass();
    classOfService[4].bookingCode() = BookingCode("R");

    // twoFirstClasses
    classOfService[5].numSeats() = 2;
    classOfService[5].cabin().setFirstClass();
    classOfService[5].bookingCode() = BookingCode("R");

    classOfService[6].numSeats() = 0;
    classOfService[6].cabin().setEconomyClass();
    classOfService[6].bookingCode() = BookingCode("Y");

    classOfService[7].numSeats() = 0;
    classOfService[7].cabin().setEconomyClass();
    classOfService[7].bookingCode() = BookingCode("C");

    classOfService[8].numSeats() = 0;
    classOfService[8].cabin().setEconomyClass();
    classOfService[8].bookingCode() = BookingCode("R");
  }

  void testCalendarShopingCabinValidation_OneEconomyAvail_OneEconomyRequested()
  {
    PricingTrx trx;
    trx.setRequest(trx.dataHandle().create<PricingRequest>());

    ClassOfService classOfService[9];
    setupClassOfService(trx, classOfService);

    AirSeg airsegs[12];
    setupAirSegForCalendar(trx, airsegs);

    // Build itinerary
    Itin sh_itn_1;
    sh_itn_1.travelSeg().push_back(&airsegs[0]);
    trx.itin().push_back(&sh_itn_1);

    // Build AltDates
    PricingTrx::AltDateInfo* altDateInfo = trx.dataHandle().create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 1;
    DatePair datePair(airsegs[0].departureDT(), DateTime::emptyDate());
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    trx.altDatePairs().insert(mapItem);

    sh_itn_1.datePair() = &datePair;

    // Build Pax
    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 1;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    trx.paxType().push_back(&paxType);
    paxType.paxTypeInfo() = &paxTypeInfo;

    // Build Cabin
    CabinType cabinType;
    cabinType.setEconomyClass();
    trx.calendarRequestedCabin() = cabinType;

    // Build ClassOfService
    PricingTrx::ClassOfServiceKey cosKey1;

    cosKey1.push_back(&airsegs[0]);

    ClassOfServiceList cosList1;

    cosList1.push_back(&classOfService[0]);
    cosList1.push_back(&classOfService[6]);
    cosList1.push_back(&classOfService[7]);

    std::vector<ClassOfServiceList>* cosListVec =
        trx.dataHandle().create<std::vector<ClassOfServiceList> >();
    cosListVec->push_back(cosList1);

    // Build Availability map
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey1)] = cosListVec;

    _validator.validateFlightCabin(trx);

    PricingTrx::AltDatePairs::iterator altDateIt = trx.altDatePairs().begin();
    PricingTrx::AltDatePairs::iterator altDateItEnd = trx.altDatePairs().end();

    for (; altDateIt != altDateItEnd; ++altDateIt)
    {
      CPPUNIT_ASSERT(((*altDateIt).second)->goodItinForDatePairFound);
    }
  }

  void testCalendarShopingCabinValidation_OneEconomyAvail_TwoEconomyRequested()
  {
    PricingTrx trx;
    trx.setRequest(trx.dataHandle().create<PricingRequest>());

    ClassOfService classOfService[9];
    setupClassOfService(trx, classOfService);

    AirSeg airsegs[12];
    setupAirSegForCalendar(trx, airsegs);

    // Build itinerary
    Itin sh_itn_1;
    sh_itn_1.travelSeg().push_back(&airsegs[0]);
    trx.itin().push_back(&sh_itn_1);

    // Build AltDates
    PricingTrx::AltDateInfo* altDateInfo = trx.dataHandle().create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 1;
    DatePair datePair(airsegs[0].departureDT(), DateTime::emptyDate());
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    trx.altDatePairs().insert(mapItem);

    sh_itn_1.datePair() = &datePair;

    // Build Pax
    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 2;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    trx.paxType().push_back(&paxType);
    paxType.paxTypeInfo() = &paxTypeInfo;

    // Build Cabin
    CabinType cabinType;
    cabinType.setEconomyClass();
    trx.calendarRequestedCabin() = cabinType;

    // Build ClassOfService
    PricingTrx::ClassOfServiceKey cosKey1;

    cosKey1.push_back(&airsegs[0]);

    ClassOfServiceList cosList1;

    cosList1.push_back(&classOfService[0]);
    cosList1.push_back(&classOfService[6]);
    cosList1.push_back(&classOfService[7]);

    std::vector<ClassOfServiceList>* cosListVec =
        trx.dataHandle().create<std::vector<ClassOfServiceList> >();
    cosListVec->push_back(cosList1);

    // Build Availability map
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey1)] = cosListVec;

    try { _validator.validateFlightCabin(trx); }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.message() == "NO REQUESTED SEAT FOR CALENDAR SHOPPING AVAILABLE");
    }

    PricingTrx::AltDatePairs::iterator altDateIt = trx.altDatePairs().begin();
    PricingTrx::AltDatePairs::iterator altDateItEnd = trx.altDatePairs().end();

    for (; altDateIt != altDateItEnd; ++altDateIt)
    {
      CPPUNIT_ASSERT(!((*altDateIt).second)->goodItinForDatePairFound);
    }
  }

  void testCalendarShopingCabinValidation_TwoBusinessAvail_TwoBusinessRequested()
  {
    PricingTrx trx;
    trx.setRequest(trx.dataHandle().create<PricingRequest>());

    ClassOfService classOfService[9];
    setupClassOfService(trx, classOfService);

    AirSeg airsegs[12];
    setupAirSegForCalendar(trx, airsegs);

    // Build itinerary
    Itin sh_itn_1;
    sh_itn_1.travelSeg().push_back(&airsegs[0]);
    trx.itin().push_back(&sh_itn_1);

    // Build AltDates
    PricingTrx::AltDateInfo* altDateInfo = trx.dataHandle().create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 1;
    DatePair datePair(airsegs[0].departureDT(), DateTime::emptyDate());
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    trx.altDatePairs().insert(mapItem);

    sh_itn_1.datePair() = &datePair;

    // Build Pax
    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 2;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    trx.paxType().push_back(&paxType);
    paxType.paxTypeInfo() = &paxTypeInfo;

    // Build Cabin
    CabinType cabinType;
    cabinType.setBusinessClass();
    trx.calendarRequestedCabin() = cabinType;

    // Build ClassOfService
    PricingTrx::ClassOfServiceKey cosKey1;

    cosKey1.push_back(&airsegs[0]);

    ClassOfServiceList cosList1;

    cosList1.push_back(&classOfService[3]);
    cosList1.push_back(&classOfService[6]);
    cosList1.push_back(&classOfService[7]);

    std::vector<ClassOfServiceList>* cosListVec =
        trx.dataHandle().create<std::vector<ClassOfServiceList> >();
    cosListVec->push_back(cosList1);

    // Build Availability map
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey1)] = cosListVec;

    try { _validator.validateFlightCabin(trx); }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.message() == "NO REQUESTED SEAT FOR CALENDAR SHOPPING AVAILABLE");
    }

    PricingTrx::AltDatePairs::iterator altDateIt = trx.altDatePairs().begin();
    PricingTrx::AltDatePairs::iterator altDateItEnd = trx.altDatePairs().end();

    for (; altDateIt != altDateItEnd; ++altDateIt)
    {
      CPPUNIT_ASSERT(((*altDateIt).second)->goodItinForDatePairFound);
    }
  }

  void testCalendarShopingCabinValidation_NoEconomyAvail_OneEconomyRequested()
  {
    PricingTrx trx;
    trx.setRequest(trx.dataHandle().create<PricingRequest>());

    ClassOfService classOfService[9];
    setupClassOfService(trx, classOfService);

    AirSeg airsegs[12];
    setupAirSegForCalendar(trx, airsegs);

    // Build itinerary
    Itin sh_itn_1;
    sh_itn_1.travelSeg().push_back(&airsegs[0]);
    trx.itin().push_back(&sh_itn_1);

    // Build AltDates
    PricingTrx::AltDateInfo* altDateInfo = trx.dataHandle().create<PricingTrx::AltDateInfo>();
    altDateInfo->numOfSolutionNeeded = 1;
    DatePair datePair(airsegs[0].departureDT(), DateTime::emptyDate());
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem(datePair, altDateInfo);
    trx.altDatePairs().insert(mapItem);

    sh_itn_1.datePair() = &datePair;

    // Build Pax
    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 1;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    trx.paxType().push_back(&paxType);
    paxType.paxTypeInfo() = &paxTypeInfo;

    // Build Cabin
    CabinType cabinType;
    cabinType.setEconomyClass();
    trx.calendarRequestedCabin() = cabinType;

    // Build ClassOfService
    PricingTrx::ClassOfServiceKey cosKey1;

    cosKey1.push_back(&airsegs[0]);
    cosKey1.push_back(&airsegs[1]);

    ClassOfServiceList cosList1;
    ClassOfServiceList cosList2;

    cosList1.push_back(&classOfService[8]);
    cosList1.push_back(&classOfService[6]);
    cosList1.push_back(&classOfService[7]);

    cosList2.push_back(&classOfService[6]);
    cosList2.push_back(&classOfService[7]);
    cosList2.push_back(&classOfService[8]);

    std::vector<ClassOfServiceList>* cosListVec =
        trx.dataHandle().create<std::vector<ClassOfServiceList> >();
    cosListVec->push_back(cosList1);
    cosListVec->push_back(cosList2);

    // Build Availability map
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey1)] = cosListVec;

    try { _validator.validateFlightCabin(trx); }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.message() == "NO REQUESTED SEAT FOR CALENDAR SHOPPING AVAILABLE");
    }

    PricingTrx::AltDatePairs::iterator altDateIt = trx.altDatePairs().begin();
    PricingTrx::AltDatePairs::iterator altDateItEnd = trx.altDatePairs().end();

    for (; altDateIt != altDateItEnd; ++altDateIt)
    {
      CPPUNIT_ASSERT(!((*altDateIt).second)->goodItinForDatePairFound);
    }
  }

  void
  testCalendarShopingCabinValidation_2DatePairs_2ItinPerDate_EachItinHave3Seg_TwoEconomyAvailonFirstItin_OneEconomyRequested()
  {
    PricingTrx trx;
    trx.setRequest(trx.dataHandle().create<PricingRequest>());

    ClassOfService classOfService[9];
    setupClassOfService(trx, classOfService);

    AirSeg airsegs[12];
    setupAirSegForCalendar(trx, airsegs);

    // Build itinerary
    Itin sh_itn_1;
    sh_itn_1.travelSeg().push_back(&airsegs[0]);
    sh_itn_1.travelSeg().push_back(&airsegs[1]);
    sh_itn_1.travelSeg().push_back(&airsegs[2]);

    Itin sh_itn_2;
    sh_itn_2.travelSeg().push_back(&airsegs[3]);
    sh_itn_2.travelSeg().push_back(&airsegs[4]);
    sh_itn_2.travelSeg().push_back(&airsegs[5]);

    Itin sh_itn_3;
    sh_itn_3.travelSeg().push_back(&airsegs[6]);
    sh_itn_3.travelSeg().push_back(&airsegs[7]);
    sh_itn_3.travelSeg().push_back(&airsegs[8]);

    Itin sh_itn_4;
    sh_itn_4.travelSeg().push_back(&airsegs[9]);
    sh_itn_4.travelSeg().push_back(&airsegs[10]);
    sh_itn_4.travelSeg().push_back(&airsegs[11]);

    trx.itin().push_back(&sh_itn_1);
    trx.itin().push_back(&sh_itn_2);
    trx.itin().push_back(&sh_itn_3);
    trx.itin().push_back(&sh_itn_4);

    // Build AltDates
    PricingTrx::AltDateInfo* altDateInfo1 = trx.dataHandle().create<PricingTrx::AltDateInfo>();
    PricingTrx::AltDateInfo* altDateInfo2 = trx.dataHandle().create<PricingTrx::AltDateInfo>();
    altDateInfo1->numOfSolutionNeeded = 1;
    altDateInfo2->numOfSolutionNeeded = 1;
    DatePair datePair1(airsegs[0].departureDT(), DateTime::emptyDate());
    DatePair datePair2(airsegs[6].departureDT(), DateTime::emptyDate());

    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem1(datePair1, altDateInfo1);
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem2(datePair1, altDateInfo2);
    trx.altDatePairs().insert(mapItem1);
    trx.altDatePairs().insert(mapItem2);

    sh_itn_1.datePair() = &datePair1;
    sh_itn_2.datePair() = &datePair1;
    sh_itn_3.datePair() = &datePair2;
    sh_itn_4.datePair() = &datePair2;

    // Build Pax
    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 1;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    trx.paxType().push_back(&paxType);
    paxType.paxTypeInfo() = &paxTypeInfo;

    // Build Cabin
    CabinType cabinType;
    cabinType.setEconomyClass();
    trx.calendarRequestedCabin() = cabinType;

    ClassOfServiceList cosList1;
    ClassOfServiceList cosList2;
    ClassOfServiceList cosList3;
    ClassOfServiceList cosList4;

    cosList1.push_back(&classOfService[0]);
    cosList1.push_back(&classOfService[2]);
    cosList1.push_back(&classOfService[3]);
    cosList2.push_back(&classOfService[0]);
    cosList2.push_back(&classOfService[2]);
    cosList2.push_back(&classOfService[3]);
    cosList3.push_back(&classOfService[7]);
    cosList3.push_back(&classOfService[7]);
    cosList3.push_back(&classOfService[7]);
    cosList4.push_back(&classOfService[7]);
    cosList4.push_back(&classOfService[7]);
    cosList4.push_back(&classOfService[7]);

    // Build ClassOfService
    PricingTrx::ClassOfServiceKey cosKey1;
    PricingTrx::ClassOfServiceKey cosKey2;
    PricingTrx::ClassOfServiceKey cosKey3;
    PricingTrx::ClassOfServiceKey cosKey4;

    cosKey1.push_back(&airsegs[0]);
    cosKey1.push_back(&airsegs[1]);
    cosKey1.push_back(&airsegs[2]);

    cosKey2.push_back(&airsegs[3]);
    cosKey2.push_back(&airsegs[4]);
    cosKey2.push_back(&airsegs[5]);

    cosKey3.push_back(&airsegs[6]);
    cosKey3.push_back(&airsegs[7]);
    cosKey3.push_back(&airsegs[8]);

    cosKey4.push_back(&airsegs[9]);
    cosKey4.push_back(&airsegs[10]);
    cosKey4.push_back(&airsegs[11]);

    std::vector<ClassOfServiceList>* cosListVec =
        trx.dataHandle().create<std::vector<ClassOfServiceList> >();
    std::vector<ClassOfServiceList>* cosListVec1 =
        trx.dataHandle().create<std::vector<ClassOfServiceList> >();

    cosListVec->push_back(cosList1);
    cosListVec->push_back(cosList2);
    cosListVec1->push_back(cosList3);
    cosListVec1->push_back(cosList4);

    // Build Availability map
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey1)] = cosListVec;
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey2)] = cosListVec;
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey3)] = cosListVec1;
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey4)] = cosListVec1;

    try { _validator.validateFlightCabin(trx); }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.message() == "NO REQUESTED SEAT FOR CALENDAR SHOPPING AVAILABLE");
    }

    PricingTrx::AltDatePairs::iterator altDateIt = trx.altDatePairs().begin();
    PricingTrx::AltDatePairs::iterator altDateItEnd = trx.altDatePairs().end();

    for (; altDateIt != altDateItEnd; ++altDateIt)
    {
      DateTime depAltDate((((*altDateIt).first).first).date(),
                          boost::posix_time::time_duration(0, 0, 0));
      if (depAltDate == DateTime(2009, 7, 8))
      {
        // CPPUNIT_ASSERT(((*altDateIt).second)->goodItinForDatePairFound);
      }
      if (depAltDate == DateTime(2009, 7, 10))
      {
        CPPUNIT_ASSERT(!((*altDateIt).second)->goodItinForDatePairFound);
      }
    }
  }

  void
  testCalendarShopingCabinValidation_2DatePairs_2ItinPerDate_EachItinHave3Seg_TwoEconomyAvailonLastItin_OneEconomyRequested()
  {
    PricingTrx trx;
    trx.setRequest(trx.dataHandle().create<PricingRequest>());

    ClassOfService classOfService[9];
    setupClassOfService(trx, classOfService);

    AirSeg airsegs[12];
    setupAirSegForCalendar(trx, airsegs);

    // Build itinerary
    Itin sh_itn_1;
    sh_itn_1.travelSeg().push_back(&airsegs[0]);
    sh_itn_1.travelSeg().push_back(&airsegs[1]);
    sh_itn_1.travelSeg().push_back(&airsegs[2]);

    Itin sh_itn_2;
    sh_itn_2.travelSeg().push_back(&airsegs[3]);
    sh_itn_2.travelSeg().push_back(&airsegs[4]);
    sh_itn_2.travelSeg().push_back(&airsegs[5]);

    Itin sh_itn_3;
    sh_itn_3.travelSeg().push_back(&airsegs[6]);
    sh_itn_3.travelSeg().push_back(&airsegs[7]);
    sh_itn_3.travelSeg().push_back(&airsegs[8]);

    Itin sh_itn_4;
    sh_itn_4.travelSeg().push_back(&airsegs[9]);
    sh_itn_4.travelSeg().push_back(&airsegs[10]);
    sh_itn_4.travelSeg().push_back(&airsegs[11]);

    trx.itin().push_back(&sh_itn_1);
    trx.itin().push_back(&sh_itn_2);
    trx.itin().push_back(&sh_itn_3);
    trx.itin().push_back(&sh_itn_4);

    // Build AltDates
    PricingTrx::AltDateInfo* altDateInfo1 = trx.dataHandle().create<PricingTrx::AltDateInfo>();
    PricingTrx::AltDateInfo* altDateInfo2 = trx.dataHandle().create<PricingTrx::AltDateInfo>();
    altDateInfo1->numOfSolutionNeeded = 1;
    altDateInfo2->numOfSolutionNeeded = 1;
    DatePair datePair1(airsegs[0].departureDT(), DateTime::emptyDate());
    DatePair datePair2(airsegs[6].departureDT(), DateTime::emptyDate());

    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem1(datePair1, altDateInfo1);
    std::pair<DatePair, PricingTrx::AltDateInfo*> mapItem2(datePair1, altDateInfo2);
    trx.altDatePairs().insert(mapItem1);
    trx.altDatePairs().insert(mapItem2);

    sh_itn_1.datePair() = &datePair1;
    sh_itn_2.datePair() = &datePair1;
    sh_itn_3.datePair() = &datePair2;
    sh_itn_4.datePair() = &datePair2;

    // Build Pax
    PaxType paxType;
    paxType.paxType() = "ADT";
    paxType.number() = 1;
    PaxTypeInfo paxTypeInfo;
    paxTypeInfo.numberSeatsReq() = 1;
    trx.paxType().push_back(&paxType);
    paxType.paxTypeInfo() = &paxTypeInfo;

    // Build Cabin
    CabinType cabinType;
    cabinType.setEconomyClass();
    trx.calendarRequestedCabin() = cabinType;

    ClassOfServiceList cosList1;
    ClassOfServiceList cosList2;
    ClassOfServiceList cosList3;
    ClassOfServiceList cosList4;

    cosList1.push_back(&classOfService[1]);
    cosList1.push_back(&classOfService[2]);
    cosList1.push_back(&classOfService[3]);
    cosList2.push_back(&classOfService[7]);
    cosList2.push_back(&classOfService[7]);
    cosList2.push_back(&classOfService[7]);
    cosList3.push_back(&classOfService[7]);
    cosList3.push_back(&classOfService[7]);
    cosList3.push_back(&classOfService[7]);
    cosList4.push_back(&classOfService[7]);
    cosList4.push_back(&classOfService[7]);
    cosList4.push_back(&classOfService[7]);

    // Build ClassOfService
    PricingTrx::ClassOfServiceKey cosKey1;
    PricingTrx::ClassOfServiceKey cosKey2;
    PricingTrx::ClassOfServiceKey cosKey3;
    PricingTrx::ClassOfServiceKey cosKey4;

    cosKey1.push_back(&airsegs[0]);
    cosKey1.push_back(&airsegs[1]);
    cosKey1.push_back(&airsegs[2]);

    cosKey2.push_back(&airsegs[3]);
    cosKey2.push_back(&airsegs[4]);
    cosKey2.push_back(&airsegs[5]);

    cosKey3.push_back(&airsegs[6]);
    cosKey3.push_back(&airsegs[7]);
    cosKey3.push_back(&airsegs[8]);

    cosKey4.push_back(&airsegs[9]);
    cosKey4.push_back(&airsegs[10]);
    cosKey4.push_back(&airsegs[11]);

    std::vector<ClassOfServiceList>* cosListVec =
        trx.dataHandle().create<std::vector<ClassOfServiceList> >();

    cosListVec->push_back(cosList1);
    cosListVec->push_back(cosList2);
    cosListVec->push_back(cosList3);
    cosListVec->push_back(cosList4);

    // Build Availability map
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey1)] = cosListVec;
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey2)] = cosListVec;
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey3)] = cosListVec;
    trx.availabilityMap()[ShoppingUtil::buildAvlKey(cosKey4)] = cosListVec;

    try { _validator.validateFlightCabin(trx); }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.message() == "NO REQUESTED SEAT FOR CALENDAR SHOPPING AVAILABLE");
    }

    PricingTrx::AltDatePairs::iterator altDateIt = trx.altDatePairs().begin();
    PricingTrx::AltDatePairs::iterator altDateItEnd = trx.altDatePairs().end();

    for (; altDateIt != altDateItEnd; ++altDateIt)
    {
      DateTime depAltDate((((*altDateIt).first).first).date(),
                          boost::posix_time::time_duration(0, 0, 0));
      if (depAltDate == DateTime(2009, 7, 8))
      {
        CPPUNIT_ASSERT(!((*altDateIt).second)->goodItinForDatePairFound);
      }
      if (depAltDate == DateTime(2009, 7, 10))
      {
        CPPUNIT_ASSERT(((*altDateIt).second)->goodItinForDatePairFound);
      }
    }
  }

  void test_depInDateEq_SCI252()
  {
    PricingTrx trx;
    PricingTrx::AltDateInfo altDateInfo;
    altDateInfo.goodItinForDatePairFound = true;

    DateTime dateTime1;
    boost::gregorian::date gregorianDate(2014, 12, 12);
    DateTime dateTime2(gregorianDate, boost::posix_time::time_duration(10, 10, 10));
    trx.altDatePairs().insert(std::make_pair(std::make_pair(dateTime1, dateTime2), &altDateInfo));

    AirSeg travelSeg;
    travelSeg.departureDT() = dateTime1;

    std::vector<TravelSeg*> travelSegVec;
    travelSegVec.push_back(&travelSeg);

    Itin itin;
    itin.travelSeg() = travelSegVec;

    DateTime dateTime3(2014, 12, 12);
    DateTime dateTime4, dateTime5(dateTime3, boost::posix_time::time_duration(10, 10, 10));
    std::pair<DateTime, DateTime> datePair = std::make_pair(dateTime4, dateTime5);
    itin.datePair() = &datePair;

    std::vector<Itin*> itinsVec;
    itinsVec.push_back(&itin);

    trx.itin() = itinsVec;

    PricingRequest req;
    req.originBasedRTPricing() = true;
    req.brandedFareEntry() = false;
    trx.setRequest(&req);

    SoldoutCabinValidatorMock validator;
    EXPECT_CALL(validator, isReqAwardAltDatesRT(_)).WillOnce(Return(true));
    EXPECT_CALL(validator, isCabinAvailable(_, _, _, _)).WillOnce(Return(false));
    validator.validateFlightCabin(trx);
  }

private:
  tse::iadetail::SoldoutCabinValidator _validator;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SoldoutCabinValidatorTest);

} // tse
