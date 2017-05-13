//-----------------------------------------------------------------------------
//
//  File:     Diag185CollectorTest.cpp
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "Common/CabinType.h"
#include "DBAccess/Customer.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "Diagnostic/Diag185Collector.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"


using namespace tse;
using namespace std;

namespace tse
{
namespace
{
class Diag185CollectorTestMock : public Diag185Collector
{
public:
 virtual std::vector<ClassOfService*>*
              getJourneyAvailability(const TravelSeg* travelSeg, Itin* itin) override
  {
    return nullptr;
  }
};
} // anon NS

class Diag185CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag185CollectorTest);

  CPPUNIT_TEST(testdisplayStartDiag185);
  CPPUNIT_TEST(testdisplayChangedHeader);
  CPPUNIT_TEST(testdisplayChangedInfo);
  CPPUNIT_TEST(testaddCOS);
  CPPUNIT_TEST(testaddFareMarket);
  CPPUNIT_TEST(testOperatorTRX);
  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintGcmSelectionHeader);
  CPPUNIT_TEST(testPrintRTWSelectionHeader);
  CPPUNIT_TEST(testPrintSegmentSelectedHeader);
  CPPUNIT_TEST(testPrintProcessSelectedFareMarkets);
  CPPUNIT_TEST(testPrintNoMarketsSelected);
  CPPUNIT_TEST(testPrintInvalidFareMarket);
  CPPUNIT_TEST(testPrintNoOneSegmentInCabinRequested);
  CPPUNIT_TEST(testPrintFurthestPointNotSet);
  CPPUNIT_TEST(testPrintGcmNotCalculated);
  CPPUNIT_TEST(testPrintSegmentGcm);
  CPPUNIT_TEST(testPrintSelectedSegments);
  CPPUNIT_TEST(testPrintInvalidFM_No_Primary);
  CPPUNIT_TEST(testPrintInvalidFM_With_Primary);
  CPPUNIT_TEST(testPrintNoChanges);
  CPPUNIT_TEST(testPrintSkippedSegment);
  CPPUNIT_TEST(testPrintFurthestPoint);
  CPPUNIT_TEST(testPrintNoRBDfoundForSecondarySegment);
  CPPUNIT_TEST(testIsItinOutsideEurope_Domestic);
  CPPUNIT_TEST(testIsItinOutsideEurope_International);
  CPPUNIT_TEST(testisItinOutsideNetherlandAntilles_Domestic);
  CPPUNIT_TEST(testisItinOutsideNetherlandAntilles_International);


  CPPUNIT_TEST(testfinishDiag185);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _collector = new Diag185Collector;
    _collector->_active = true;
    _collectorM = _memHandle.insert(new Diag185CollectorTestMock());
    _collectorM->activate();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    Customer* customer = _memHandle.create<Customer>();
    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = customer;
    _request->ticketingAgent() = agent;
    _trx->setRequest(_request);
    _trx->setOptions(_memHandle.create<PricingOptions>());
    sfo = getLoc("SFO");
    dfw = getLoc("DFW");
    _fm = createFareMarket(sfo, dfw, "VA");
    AirSeg* airSeg = makeAirSeg("SFO", "DFW", "VA");
    airSeg->bookedCabin().setPremiumBusinessClass();
    airSeg->pnrSegment() = 1;
    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back(airSeg);
    _fm->travelSeg() = travelSegs;
    _trx->travelSeg().push_back(airSeg);
    _request->ticketingDT() = DateTime(2015, 11, 25);
    _itin = _memHandle.create<Itin>();
    _itin->travelSeg() = travelSegs;
    _trx->itin().push_back(_itin);
  }

  void tearDown() { delete _collector; _memHandle.clear();}

  FareMarket*
  createFareMarket(const Loc* origin, const Loc* destination, CarrierCode goveringCarrier)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->origin() = origin;
    fm->destination() = destination;
    fm->governingCarrier() = goveringCarrier;
    return fm;
  }

  const Loc* getLoc(const LocCode& locCode)
  {
    return TestLocFactory::create("/vobs/atseintl/test/testdata/data/Loc" + locCode + ".xml");
  }

  AirSeg* makeAirSeg(const LocCode& boardMultiCity,
                     const LocCode& offMultiCity,
                     const CarrierCode& carrier,
                     LocCode origAirport = "",
                     LocCode destAirport = "")
  {
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc(boardMultiCity, DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc(offMultiCity, DateTime::localTime());

    if ("" == origAirport)
      origAirport = boardMultiCity;
    if ("" == destAirport)
      destAirport = offMultiCity;

    AirSeg* as = _memHandle.create<AirSeg>();
    as->origin() = const_cast<Loc*>(loc1);
    as->destination() = const_cast<Loc*>(loc2);
    as->boardMultiCity() = boardMultiCity;
    as->offMultiCity() = offMultiCity;
    as->origAirport() = origAirport;
    as->destAirport() = destAirport;
    as->carrier() = carrier;
    as->flightNumber() = 543;
    as->setBookingCode("A ");
    as->departureDT() = DateTime(2016, 12, 31);
    return as;
  }

  void testdisplayStartDiag185()
  {
    _trx->getOptions()->cabin().setEconomyClass();
    _collector->startDiag185(*_trx);
    CPPUNIT_ASSERT(_collector->str().find("ECONOMY") != std::string::npos);
  }

  void testdisplayChangedHeader()
  {
    _trx->getOptions()->cabin().setEconomyClass();
    _collector->displayChangedHeader();
    CPPUNIT_ASSERT(_collector->str().find("CHANGED") != std::string::npos);
  }

  void testdisplayChangedInfo()
  {
    ClassOfService* cos[1];
    cos[0] = _memHandle.create<ClassOfService>();
    cos[0]->bookingCode() = 'J';
    cos[0]->numSeats() = 3;
    cos[0]->cabin().setPremiumFirstClass();
    _collector->displayChangedInfo(cos[0] , true);
    CPPUNIT_ASSERT(_collector->str().find("FLOW") != std::string::npos); // Availability type
    CPPUNIT_ASSERT(_collector->str().find("J") != std::string::npos);    // BookingCOde
    CPPUNIT_ASSERT(_collector->str().find("3") != std::string::npos);    // Number of seats
    _collector->displayChangedInfo(cos[0] , false);
    CPPUNIT_ASSERT(_collector->str().find("SOLO") != std::string::npos);
  }

  void testaddCOS()
  {
    ClassOfService* cos0 = _memHandle.create<ClassOfService>();
    cos0->bookingCode() = 'J';
    cos0->numSeats() = 3;
    cos0->cabin().setPremiumFirstClass();
    ClassOfService* cos1 = _memHandle.create<ClassOfService>();
    cos1->bookingCode() = 'Y';
    cos1->numSeats() = 6;
    cos1->cabin().setEconomyClass();

    std::vector<ClassOfService*> cosVec;
    cosVec.push_back(cos0);
    cosVec.push_back(cos1);

    _collector->addCOS(&cosVec);
    CPPUNIT_ASSERT(_collector->str().find("J3") != std::string::npos);
    CPPUNIT_ASSERT(_collector->str().find("Y6") != std::string::npos);
  }

  void testaddFareMarket()
  {
    ClassOfService* cos0 = _memHandle.create<ClassOfService>();
    cos0->bookingCode() =  'J';
    cos0->numSeats() = 3;
    cos0->cabin().setPremiumFirstClass();
    ClassOfService* cos1 = _memHandle.create<ClassOfService>();
    cos1->bookingCode() = 'Y';
    cos1->numSeats() = 6;
    cos1->cabin().setEconomyClass();

    std::vector<ClassOfService*> cosVec;
    cosVec.push_back(cos0);
    cosVec.push_back(cos1);
    _fm->classOfServiceVec().push_back(&cosVec);
    _collectorM->addFareMarket(*_trx, *_fm);
    CPPUNIT_ASSERT(_collectorM->str().find("J3") != std::string::npos);
    CPPUNIT_ASSERT(_collectorM->str().find("Y6") != std::string::npos);
  }

  void testOperatorTRX()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();

    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    ClassOfService* cos0 = _memHandle.create<ClassOfService>();
    cos0->bookingCode() =  'J';
    cos0->numSeats() = 3;
    cos0->cabin().setPremiumFirstClass();
    ClassOfService* cos1 = _memHandle.create<ClassOfService>();
    cos1->bookingCode() = 'Y';
    cos1->numSeats() = 6;
    cos1->cabin().setEconomyClass();

    std::vector<ClassOfService*> cosVec;
    cosVec.push_back(cos0);
    cosVec.push_back(cos1);
    _fm->classOfServiceVec().push_back(&cosVec);
    _trx->fareMarket().push_back(_fm);
    diag << *_trx;
    _collector->finishDiag185(*_trx);
    CPPUNIT_ASSERT(_collector->str().find("SFO-DFW CAB:4") != std::string::npos);

  }

  void testPrintHeader()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);
    _collector->printHeader();
    CPPUNIT_ASSERT(_collector->str().find("START DIAG 185") != std::string::npos);
  }

  void testPrintGcmSelectionHeader()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printGcmSelectionHeader();
    CPPUNIT_ASSERT(_collector->str().find("GCM") != std::string::npos);
  }

  void testPrintRTWSelectionHeader()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printRTWSelectionHeader();
    CPPUNIT_ASSERT(_collector->str().find("SELECT RW") != std::string::npos);
  }

  void testPrintSegmentSelectedHeader()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printSegmentSelectedHeader();
    CPPUNIT_ASSERT(_collector->str().find("GC MILES") != std::string::npos);
  }

  void testPrintProcessSelectedFareMarkets()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printProcessSelectedFareMarkets();
    CPPUNIT_ASSERT(_collector->str().find("SELECTED FARE MARKETS") != std::string::npos);
  }

  void testPrintNoMarketsSelected()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printNoMarketsSelected();
    CPPUNIT_ASSERT(_collector->str().find("NO MARKETS") != std::string::npos);
  }

  void testPrintInvalidFareMarket()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printInvalidFareMarket();
    CPPUNIT_ASSERT(_collector->str().find("INVALID FARE MARKET") != std::string::npos);
  }

  void testPrintNoOneSegmentInCabinRequested()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printNoOneSegmentInCabinRequested();
    CPPUNIT_ASSERT(_collector->str().find("NO ONE SEGMENT") != std::string::npos);
  }

  void testPrintFurthestPointNotSet()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printFurthestPointNotSet();
    CPPUNIT_ASSERT(_collector->str().find("POINT NOT SET") != std::string::npos);
  }

  void testPrintGcmNotCalculated()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printGcmNotCalculated();
    CPPUNIT_ASSERT(_collector->str().find("GCM NOT CALCULATED") != std::string::npos);
  }

  void testPrintSelectedFmNotFound()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    _collector->printSelectedFmNotFound();
    CPPUNIT_ASSERT(_collector->str().find("\n* SELECTED FARE MARKET NOT FOUND, BREAK IT TO SMALLER FM *\n") !=
                   std::string::npos);
  }

  void testPrintSegmentGcm()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);
    const uint32_t gcm = 1000;
    std::stringstream expectedDiag;
    expectedDiag
    << "1  SFODFW         1000\n";

    _collector->printSegmentGcm(_fm->travelSeg().front()->origAirport(), *_fm->travelSeg().front(), gcm);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintSelectedSegments()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);
    std::stringstream expectedDiag;
    expectedDiag
    << " \n       MARKET SELECTED :\n"
    << "        1 VA543 A  31DEC SFO-DFW CAB:4 \n"
    << "* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -\n\n";

    _collector->printSelectedSegments(_fm->travelSeg());
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintInvalidFM_No_Primary()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);
    std::stringstream expectedDiag;
    expectedDiag
    << " * INVENTORY REQUIRED FOR REPLACEMENT NOT AVAIABLE *\n"
    << " ---------------------------------------------------------- \n"
    << " * INVALID FARE MARKET SFO-DFW\n"
    << " 1 VA543 A  31DEC SFO-DFW CAB:4 \n";
    bool primary = false;
    _collector->printInvalidFM(*_fm, primary);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintInvalidFM_With_Primary()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);
    std::stringstream expectedDiag;
    expectedDiag
    << " ---------------------------------------------------------- \n"
    << " * INVALID FARE MARKET SFO-DFW\n"
    << " 1 VA543 A  31DEC SFO-DFW CAB:4 \n"
    << " \n* PRIMARY SECTOR COULD NOT BE REPLACED IN REQUESTED CABIN *\n\n"
    <<  "***************** END  DIAG 185 ************************** \n";
    bool primary = true;
    _collector->printInvalidFM(*_fm, primary);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintNoChanges()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);

    std::stringstream expectedDiag;
    expectedDiag
    << " 1 VA543 A  31DEC SFO-DFW CAB:4 \n"
    << "    ALREADY BOOKED IN JUMP DOWN CABIN\n \n\n";

    _collector->printNoChanges(_fm->travelSeg().front());
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintSkippedSegment()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);
    CabinType reqCabin;
    reqCabin.setPremiumBusinessClass();
    std::stringstream expectedDiag;
    expectedDiag
    << " 1 VA543 A  31DEC SFO-DFW CAB:4 \n"
    << "        BOOKED IN REQUESTED CABIN\n \n\n";

    _collector->printSkippedSegment(_fm->travelSeg().front(), reqCabin);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }


  void testPrintFurthestPoint()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);
    const uint32_t gcm = 1000;
    std::stringstream expectedDiag;
    expectedDiag
    << "\nTURNAROUND/FURTHEST POINT:   1 DFW   DISTANCE: 1000\n";

    _collector->printFurthestPoint(*_fm->travelSeg().front(), gcm);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }

  void testPrintNoRBDfoundForSecondarySegment()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);
    std::stringstream expectedDiag;
    expectedDiag
    << " * INVENTORY REQUIRED FOR REPLACEMENT NOT AVAIABLE *\n"
    << " ---------------------------------------------------------- \n"
    << " * INVALID FARE MARKET SFO-DFW\n"
    << " 1 VA543 A  31DEC SFO-DFW CAB:4 \n"
    << " \n"
    << " * CABIN OFFERED BUT UNAVAILABLE ON SECONDARY SECTOR *\n"
    << " 1 VA543 A  31DEC SFO-DFW CAB:4 \n"
    <<  "***************** END  DIAG 185 ************************** \n";

    _collector->printNoRBDfoundForSecondarySegment(_fm->travelSeg().front(), *_fm);
    CPPUNIT_ASSERT_EQUAL( expectedDiag.str(), _collector->str() );
  }


  void testIsItinOutsideEurope_Domestic()
  {
    _itin->geoTravelType() = GeoTravelType::Domestic;

    CPPUNIT_ASSERT(!_collector->isItinOutsideEurope(*_itin));
  }

  void testIsItinOutsideEurope_International()
  {
    _itin->geoTravelType() = GeoTravelType::International;

    CPPUNIT_ASSERT(_collector->isItinOutsideEurope(*_itin));
  }

  void testisItinOutsideNetherlandAntilles_Domestic()
  {
    _itin->geoTravelType() = GeoTravelType::Domestic;

    CPPUNIT_ASSERT(!_collector->isItinOutsideEurope(*_itin));
  }

  void testisItinOutsideNetherlandAntilles_International()
  {
    _itin->geoTravelType() = GeoTravelType::International;

    CPPUNIT_ASSERT(_collector->isItinOutsideEurope(*_itin));
  }


  void testfinishDiag185()
  {
    Diagnostic diagroot(Diagnostic185);
    diagroot.activate();
    Diag185Collector diag(diagroot);
    diag.enable(Diagnostic185);
    _collector->finishDiag185(*_trx);
    CPPUNIT_ASSERT(_collector->str().find("END  DIAG 185") != std::string::npos);
  }


 private:
  Diag185Collector* _collector;
  Diag185CollectorTestMock* _collectorM;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions* _options;
  FareMarket* _fm;
  Itin* _itin;
  const tse::Loc* sfo;
  const tse::Loc* dfw;

};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag185CollectorTest);
}
