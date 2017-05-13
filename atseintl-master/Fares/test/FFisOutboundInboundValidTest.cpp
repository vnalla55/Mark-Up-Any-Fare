
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include <iostream>
#include "Fares/FareValidatorOrchestrator.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareMarket.h"
#include "Common/DateTime.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/AirSeg.h"
#include "test/include/MockTseServer.h"
#include "Common/VecMap.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using namespace std;

namespace FFisOut
{

class FakedFVO : public FareValidatorOrchestrator
{
public:
  using FareValidatorOrchestrator::isOutboundInboundFltValid;
  using FareValidatorOrchestrator::initFFinderAltDates;

  FakedFVO(const std::string& name, TseServer& server) : FareValidatorOrchestrator(name, server) {}
};
}

class FFisOutboundInboundValid : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(FFisOutboundInboundValid);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_OneLegNoFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_OneLegOneValidFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_OneLegOneInvalidFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_OneLegOneCompFailedFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_OneLegOneCompFailedFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_OneLegOneFltFailedFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_OneLegOneAllFailedFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_TwoLegsNoFares);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_TwoLegsEachLegHasValidFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_TwoLegsSecondLegHasInvalidFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_TwoLegsEachLegHasInvalidFare);
  CPPUNIT_TEST(testIsOutboundInboundFltValid_TwoLegsFirstCompFailSecondFltFail);
  CPPUNIT_TEST(testInitFFinderAltDates_FailedEffDate);
  CPPUNIT_TEST(testInitFFinderAltDates_AllDatesPassed);
  CPPUNIT_TEST_SUITE_END();

private:
  FFisOut::FakedFVO* _fvo;
  FlightFinderTrx* _ffTrx;
  TestMemHandle _memH;

public:
  void setUp()
  {
    MockTseServer* server = _memH.create<MockTseServer>();
    _ffTrx = _memH.insert(new FlightFinderTrx);
    _fvo = new FFisOut::FakedFVO("FVO", *server);
  }

  void tearDown() { _memH.clear(); }

  void testIsOutboundInboundFltValid_OneLegNoFare()
  {
    vector<PaxTypeFare*> fmFaresVec;

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(!foundValidFlight);
  }

  void testIsOutboundInboundFltValid_OneLegOneValidFare()
  {
    vector<PaxTypeFare*> fmFaresVec;

    bool passedPrevalidation = true;
    bool isComponentValid = true;
    bool passedFlightValidation = true;

    fmFaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(foundValidFlight);
  }

  void testIsOutboundInboundFltValid_OneLegOneInvalidFare()
  {
    vector<PaxTypeFare*> fmFaresVec;

    bool passedPrevalidation = false;
    bool isComponentValid = true;
    bool passedFlightValidation = true;

    fmFaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(!foundValidFlight);
  }
  void testIsOutboundInboundFltValid_OneLegOneCompFailedFare()
  {
    vector<PaxTypeFare*> fmFaresVec;

    bool passedPrevalidation = true;
    bool isComponentValid = false;
    bool passedFlightValidation = true;

    fmFaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(!foundValidFlight);
  }
  void testIsOutboundInboundFltValid_OneLegOneFltFailedFare()
  {
    vector<PaxTypeFare*> fmFaresVec;

    bool passedPrevalidation = true;
    bool isComponentValid = true;
    bool passedFlightValidation = false;

    fmFaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(!foundValidFlight);
  }
  void testIsOutboundInboundFltValid_OneLegOneAllFailedFare()
  {
    vector<PaxTypeFare*> fmFaresVec;

    bool passedPrevalidation = false;
    bool isComponentValid = false;
    bool passedFlightValidation = false;

    fmFaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(!foundValidFlight);
  }
  void testIsOutboundInboundFltValid_TwoLegsNoFares()
  {
    vector<PaxTypeFare*> fmFaresVec;

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));
    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(!foundValidFlight);
  }
  void testIsOutboundInboundFltValid_TwoLegsEachLegHasValidFare()
  {
    vector<PaxTypeFare*> fmFaresVec;

    bool passedPrevalidation = true;
    bool isComponentValid = true;
    bool passedFlightValidation = true;

    fmFaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));
    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmFaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(foundValidFlight);
  }
  void testIsOutboundInboundFltValid_TwoLegsSecondLegHasInvalidFare()
  {

    bool passedPrevalidation = true;
    bool isComponentValid = true;
    bool passedFlightValidation = true;

    vector<PaxTypeFare*> fmLeg1FaresVec;
    fmLeg1FaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    passedPrevalidation = false;
    isComponentValid = true;
    passedFlightValidation = true;

    vector<PaxTypeFare*> fmLeg2FaresVec;
    fmLeg2FaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmLeg1FaresVec)));
    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmLeg2FaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(!foundValidFlight);
  }

  void testIsOutboundInboundFltValid_TwoLegsEachLegHasInvalidFare()
  {

    bool passedPrevalidation = false;
    bool isComponentValid = true;
    bool passedFlightValidation = true;

    vector<PaxTypeFare*> fmLeg1FaresVec;
    fmLeg1FaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    passedPrevalidation = false;
    isComponentValid = true;
    passedFlightValidation = true;

    vector<PaxTypeFare*> fmLeg2FaresVec;
    fmLeg2FaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmLeg1FaresVec)));
    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmLeg2FaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(!foundValidFlight);
  }

  void testIsOutboundInboundFltValid_TwoLegsFirstCompFailSecondFltFail()
  {

    bool passedPrevalidation = true;
    bool isComponentValid = false;
    bool passedFlightValidation = true;

    vector<PaxTypeFare*> fmLeg1FaresVec;
    fmLeg1FaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    passedPrevalidation = true;
    isComponentValid = true;
    passedFlightValidation = false;

    vector<PaxTypeFare*> fmLeg2FaresVec;
    fmLeg2FaresVec.push_back(
        buildValidatedPaxTypeFare(passedPrevalidation, isComponentValid, passedFlightValidation));

    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmLeg1FaresVec)));
    _ffTrx->legs().push_back(buildLegWithItinCell("AA", buildFareMarket(fmLeg2FaresVec)));

    bool foundValidFlight = _fvo->isOutboundInboundFltValid(*_ffTrx);

    CPPUNIT_ASSERT(!foundValidFlight);
  }

  void testInitFFinderAltDates_AllDatesPassed()
  {
    DateTime departureDT = DateTime(2008, 2, 20);

    TSEDateInterval fareInterval;
    fareInterval.effDate() = departureDT;
    fareInterval.discDate() = departureDT;

    vector<PaxTypeFare*> fmFaresVec;
    fmFaresVec.push_back(
        buildPaxTypeFare(0, buildFareInfo("ATP", "AA", "KRK", "DFW", fareInterval)));

    _ffTrx->legs().push_back(buildLegWithItinCell(
        "AA", buildFareMarket(fmFaresVec, buildSegment("KRK", "FRA", "AA", departureDT))));

    // create example altdates
    DateTime outDates[] = { DateTime(2008, 2, 20), DateTime(2008, 2, 21), DateTime(2008, 2, 22),
                            DateTime(2008, 2, 23) };
    DateTime inDates[] = { DateTime(2008, 2, 20), DateTime(2008, 2, 22), DateTime(2008, 2, 22),
                           DateTime(2008, 2, 24) };
    size_t countPairs = sizeof(outDates) / sizeof(outDates[0]);

    _ffTrx->altDatePairs() = buildAltDates(outDates, inDates, countPairs);

    // Tested function
    _fvo->initFFinderAltDates(*_ffTrx);

    PaxTypeFare* fare = fmFaresVec.front();
    for (VecMap<DatePair, uint8_t>::const_iterator iter = fare->altDateStatus().begin();
         iter != fare->altDateStatus().end();
         ++iter)
    {
      bool datePairPass = fare->getAltDatePass(iter->first);
      CPPUNIT_ASSERT(datePairPass);
    }
  }

  void testInitFFinderAltDates_FailedEffDate()
  {
    DateTime departureDT = DateTime(2008, 2, 20);
    DateTime effectiveDT = DateTime(2008, 2, 23);
    CPPUNIT_ASSERT(effectiveDT > departureDT);

    TSEDateInterval fareInterval;
    fareInterval.effDate() = effectiveDT;
    fareInterval.discDate() = departureDT;

    vector<PaxTypeFare*> fmFaresVec;
    fmFaresVec.push_back(
        buildPaxTypeFare(0, buildFareInfo("ATP", "AA", "KRK", "DFW", fareInterval)));

    _ffTrx->legs().push_back(buildLegWithItinCell(
        "AA", buildFareMarket(fmFaresVec, buildSegment("KRK", "FRA", "AA", departureDT))));

    // create example altdates
    DateTime outDates[] = { DateTime(2008, 2, 20), DateTime(2008, 2, 21), DateTime(2008, 2, 22),
                            DateTime(2008, 2, 23) };
    DateTime inDates[] = { DateTime(2008, 2, 20), DateTime(2008, 2, 22), DateTime(2008, 2, 22),
                           DateTime(2008, 2, 24) };
    size_t countPairs = sizeof(outDates) / sizeof(outDates[0]);

    _ffTrx->altDatePairs() = buildAltDates(outDates, inDates, countPairs);

    // Tested function
    _fvo->initFFinderAltDates(*_ffTrx);

    PaxTypeFare* fare = fmFaresVec.front();
    for (VecMap<DatePair, uint8_t>::const_iterator iter = fare->altDateStatus().begin();
         iter != fare->altDateStatus().end();
         ++iter)
    {
      bool datePairPass = fare->getAltDatePass(iter->first);
      CPPUNIT_ASSERT(!datePairPass);
    }
  }

protected:
  PricingTrx::AltDatePairs buildAltDates(DateTime outDates[], DateTime inDates[], size_t numPairs)
  {
    PricingTrx::AltDatePairs datePairsMap;
    for (unsigned int i = 0; i < numPairs; i++)
    {
      datePairsMap[DatePair(outDates[i], inDates[i])] = 0;
    }

    return datePairsMap;
  }

  ShoppingTrx::Leg& buildLegWithItinCell(const std::string& carrier, FareMarket* fM)
  {
    // create itinCellInfo
    ItinIndex::ItinCellInfo* itinCellInfo = _memH.insert(new ItinIndex::ItinCellInfo);

    // create sopItinerary
    Itin* sopItinerary = _memH.insert(new Itin);
    sopItinerary->fareMarket().push_back(fM);
    if (!fM->travelSeg().empty())
    {
      sopItinerary->travelSeg().push_back(fM->travelSeg().front());
    }

    ShoppingTrx::SchedulingOption sop(sopItinerary, 1, true);
    sop.governingCarrier() = carrier;

    // generate crx key
    ItinIndex::Key cxrKey;
    ShoppingUtil::createCxrKey(carrier, cxrKey);

    // generate schedule key
    ItinIndex::Key scheduleKey;
    ShoppingUtil::createScheduleKey(scheduleKey);

    ShoppingTrx::Leg* leg = _memH.insert(new ShoppingTrx::Leg);

    leg->carrierIndex().addItinCell(sopItinerary, *itinCellInfo, cxrKey, scheduleKey);

    leg->sop().push_back(sop);

    return *leg;
  }
  //-----------------------------------------------------

  FareMarket* buildFareMarket(std::vector<PaxTypeFare*>& fmFares, TravelSeg* fmSegment = 0)
  {
    FareMarket* fM = _memH.insert(new FareMarket);
    if (fmSegment)
    {
      fM->travelSeg().push_back(fmSegment);
    }

    for (std::vector<PaxTypeFare*>::iterator fareIter = fmFares.begin(); fareIter != fmFares.end();
         fareIter++)
    {
      if (*fareIter)
      {
        fM->allPaxTypeFare().push_back(*fareIter);
      }
    }

    return fM;
  }

  //-----------------------------------------------------

  PaxTypeFare* buildValidatedPaxTypeFare(bool passedPrevalidation,
                                         bool isComponentValid,
                                         bool passedFlightValidation)
  {
    PaxTypeFare* fare = _memH.insert(new PaxTypeFare);

    if (passedPrevalidation)
      fare->setIsShoppingFare();
    fare->shoppingComponentValidationFailed() = !isComponentValid;
    fare->flightBitmapAllInvalid() = !passedFlightValidation;

    return fare;
  }

  //-----------------------------------------------------
  PaxTypeFare* buildPaxTypeFare(FareMarket* fM, FareInfo* fareInfo = 0)
  {
    PaxTypeFare* paxTypeFare = _memH.insert(new PaxTypeFare);

    // create paxType
    PaxType* paxType = _memH.insert(new PaxType);

    Fare* fare = _memH.insert(new Fare);
    fare->setFareInfo(fareInfo);

    paxTypeFare->initialize(fare, paxType, fM, *_ffTrx);

    return paxTypeFare;
  }
  //-----------------------------------------------------
  FareInfo* buildFareInfo(string vendor,
                          string carrier,
                          string market1,
                          string market2,
                          TSEDateInterval range = TSEDateInterval())
  {
    FareInfo* fareInfo = _memH.insert(new FareInfo);

    fareInfo->_vendor = vendor;
    fareInfo->_carrier = carrier;
    fareInfo->_market1 = market1;
    fareInfo->_market2 = market2;
    fareInfo->_effInterval = range;

    return fareInfo;
  }

  //-----------------------------------------------------
  AirSeg* buildSegment(string origin,
                       string destination,
                       string carrier,
                       DateTime departure = DateTime::localTime(),
                       DateTime arrival = DateTime::localTime())
  {

    AirSeg* airSeg = _memH.insert(new AirSeg);

    airSeg->departureDT() = departure;
    airSeg->arrivalDT() = arrival;

    Loc* locorig = _memH.insert(new Loc);
    Loc* locdest = _memH.insert(new Loc);
    locorig->loc() = origin;
    locdest->loc() = destination;

    airSeg->origAirport() = origin;
    airSeg->origin() = locorig;
    airSeg->destAirport() = destination;
    airSeg->destination() = locdest;

    airSeg->carrier() = carrier;

    return airSeg;
  }

  void outputAltDates(const ShoppingTrx& trx)
  {
    typedef ShoppingTrx::AltDateInfo AltDateInfo;

    if (trx.isAltDates())
    {
      std::cout << "\n";

      for (std::map<DatePair, AltDateInfo*>::const_iterator i = trx.altDatePairs().begin();
           i != trx.altDatePairs().end();
           i++)
      {
        DatePair myPair = i->first;

        std::cout << myPair.first.dateToString(DDMMYYYY, "-") << "   "
                  << myPair.second.dateToString(DDMMYYYY, "-") << endl;
      }
    }
    else
      std::cout << "NO ALTERNATE DATES" << endl;

    std::cout << "------------------------------------------" << std::endl;
  }

protected:
  void test() { CPPUNIT_ASSERT_EQUAL(0, 1); }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FFisOutboundInboundValid);
}
