#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "Pricing/ESVPQDiversifier.h"
#include "Common/ShoppingUtil.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/Itin.h"
#include "Pricing/ESVPQItem.h"
#include <iostream>
#include <vector>
#include "test/include/TestMemHandle.h"

using namespace std;
namespace tse
{

class ESVPQDiversifierStub : public ESVPQDiversifier
{
public:
  ESVPQDiversifierStub(ESVOptions* options) : ESVPQDiversifier() { _esvOptions = options; }

  int inboundFlightCDMustMap(ShoppingTrx::SchedulingOption* sop)
  {
    return _inFlightCountdownMPMap[sop];
  }
  int outboundFlightCDMustMap(ShoppingTrx::SchedulingOption* sop)
  {
    return _outFlightCountdownMPMap[sop];
  }

  int inboundOnlineCarrierCDMustMap(CarrierCode carrier)
  {
    return _inOnlineCxrCountdownMPMap[carrier];
  }
  int outboundOnlineCarrierCDMustMap(CarrierCode carrier)
  {
    return _outOnlineCxrCountdownMPMap[carrier];
  }
  int inboundInterlineCarrierCDMustMap(CarrierCode carrier)
  {
    return _inInterlineCxrCountdownMPMap[carrier];
  }
  int outboundInterlineCarrierCDMustMap(CarrierCode carrier)
  {
    return _outInterlineCxrCountdownMPMap[carrier];
  }

  int flightCountdownLFSMap(ShoppingTrx::SchedulingOption* sop)
  {
    return _flightCountdownLFSMap[sop];
  }
  int inCxrCountdownLFSMap(CarrierCode carrier) { return _inCxrCountdownLFSMap[carrier]; }
  int outCxrCountdownLFSMap(CarrierCode carrier) { return _outCxrCountdownLFSMap[carrier]; }
  int onlineCxrLimitLFSMap(CarrierCode carrier) { return _onlineCxrLimitLFSMap[carrier]; }
  int onlineCxrCountMapLFS(CarrierCode carrier) { return _onlineCxrCountMapLFS[carrier]; }

  int* totalOutbound() { return &_totalOutbound; }
  int* totalInbound() { return &_totalInbound; }
  void printCarriersLFS()
  {
    std::map<CarrierCode, int>::iterator it;

    cout << " MAP inCxrCountdown = ";
    for (it = _inCxrCountdownLFSMap.begin(); it != _inCxrCountdownLFSMap.end(); it++)
    {
      cout << " " << it->second << ", ";
    }
    cout << endl;

    cout << " MAP outCxrCountdown = ";
    for (it = _outCxrCountdownLFSMap.begin(); it != _outCxrCountdownLFSMap.end(); it++)
    {
      cout << " " << it->second << ", ";
    }
    cout << endl;

    cout << " MAP onlineCxrLimitLFSMap = ";
    for (it = _onlineCxrLimitLFSMap.begin(); it != _onlineCxrLimitLFSMap.end(); it++)
    {
      cout << " " << it->second << ", ";
    }
    cout << endl;

    /*  cout<<" MAP onlineCxrCountMapLFS = ";
     for(it = _onlineCxrCountMapLFS.begin(); it!= _onlineCxrCountMapLFS.end(); it++)
     {
     cout<<" "<<it->second<<", ";
     }*/
    cout << endl;
  }

  void initFlightMustPriceAndSopsPerCxrMaps(CarrierCode& c1,
                                            CarrierCode& c2,
                                            CarrierCode& c3,
                                            ShoppingTrx::SchedulingOption& sop1,
                                            ShoppingTrx::SchedulingOption& sop2,
                                            ShoppingTrx::SchedulingOption& sop3)
  {
    _esvOptions->avsCarriersString() = "AA";
    initAVSCarriers();
    _outSopsPerCxrMap.insert(std::pair<CarrierCode, int>(c1, 30));
    _outSopsPerCxrMap.insert(std::pair<CarrierCode, int>(c2, 30));
    _outSopsPerCxrMap.insert(std::pair<CarrierCode, int>(c3, 30));

    _inSopsPerCxrMap.insert(std::pair<CarrierCode, int>(c1, 30));
    _inSopsPerCxrMap.insert(std::pair<CarrierCode, int>(c2, 30));
    _inSopsPerCxrMap.insert(std::pair<CarrierCode, int>(c3, 30));

    insertToFlightMap(&sop1, (int)_esvOptions->flightOptionReuseLimit(), _outFlightCountdownMPMap);
    insertToFlightMap(&sop2, (int)_esvOptions->flightOptionReuseLimit(), _outFlightCountdownMPMap);
    insertToFlightMap(&sop3, (int)_esvOptions->flightOptionReuseLimit(), _outFlightCountdownMPMap);

    insertToFlightMap(&sop1, (int)_esvOptions->flightOptionReuseLimit(), _inFlightCountdownMPMap);
    insertToFlightMap(&sop2, (int)_esvOptions->flightOptionReuseLimit(), _inFlightCountdownMPMap);
    insertToFlightMap(&sop3, (int)_esvOptions->flightOptionReuseLimit(), _inFlightCountdownMPMap);
  }
  friend class ESVPQDiversifierTest;
};

class ESVPQDiversifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ESVPQDiversifierTest);
  CPPUNIT_TEST(testInitFlightMustPriceMaps);
  CPPUNIT_TEST(testInitCarrierMustPriceMaps);
  CPPUNIT_TEST(testProcessingFlightAndCarrierOnlineMPSunnyDay);
  CPPUNIT_TEST(testProcessingFlightAndCarrierInterlineMPSunnyDay);
  CPPUNIT_TEST(testProcessingFlightAndCarrierOnlineMPSunnyFlightCloseToBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierInterlineMPSunnyFlightCloseToBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierMPSunnyCarrierCloseToBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierOnlineMPFlightCrossedBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierInterlineMPFlightCrossedBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierMPCarrierCrossedBound);

  CPPUNIT_TEST(testInitFlightLFSMaps);
  CPPUNIT_TEST(testInitCarrierLFSMaps);

  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSSunnyDayPassOneTwo);
  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSSunnyDayPassOneTwoFlightCloseToBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSSunnyDayPassOneTwoCarrierCloseToBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSPassOneTwoFlightCrossedBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSPassOneTwoCarrierCrossedBound);

  CPPUNIT_TEST(testRecalcCarrierLFSMaps);

  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSSunnyDayPassThree);
  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSSunnyDayPassThreeFlightCloseToBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSSunnyDayPassThreeCarrierCloseToBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSPassThreeFlightCrossedBound);
  CPPUNIT_TEST(testProcessingFlightAndCarrierLFSPassThreeCarrierCrossedBound);

  CPPUNIT_TEST(testCalcMustPriceUpperBound);
  CPPUNIT_TEST(testCalcLFSUpperBound);
  CPPUNIT_TEST(testCalcMustPriceLimitForCurrPass);
  CPPUNIT_TEST(initInterlineRestrictedCxr);
  CPPUNIT_TEST(isOnlineOnlyRestricted);
  CPPUNIT_TEST(cxrRestrictionCorrect);
  CPPUNIT_TEST_SUITE_END();

public:
  ESVPQDiversifierTest()
    : c1("AA"),
      c2("AB"),
      c3("92"),
      sop1(&itin1, 1),
      sop2(&itin2, 2),
      sop3(&itin3, 3),
      div(&esvOptions)
  {

    sop1.governingCarrier() = c1;
    sop2.governingCarrier() = c2;
    sop3.governingCarrier() = c3;
  }

  // MUST PRICE-----------------------------------------------
  void initFlightAndCarrierMapsWithThreeFlights()
  {

    *(div.totalOutbound()) = 300;
    *(div.totalInbound()) = 300;

    div.initFlightMustPriceAndSopsPerCxrMaps(c1, c2, c3, sop1, sop2, sop3); // stub
    div.initCarrierMustPriceMaps();
    div.initFlightLFSMaps();
    div.initCarrierLFSMaps();
  }

  // LFS------------------------------------------------------

  void recalcCarrierOnlineLFSMapsWithThreeCarriers() { div.recalcCarrierOnlineLimitLFSMap(); }

  // Must price
  void checkMPFlight(ShoppingTrx::SchedulingOption* sop, int numberOutbound, int numberInbound)
  {
    // first for outbound
    CPPUNIT_ASSERT_EQUAL(div.outboundFlightCDMustMap(sop), numberOutbound);
    // next for inbound
    CPPUNIT_ASSERT_EQUAL(div.inboundFlightCDMustMap(sop), numberInbound);
  }
  // LFS
  void checkLFSFlight(ShoppingTrx::SchedulingOption* sop, int number)
  {
    // first for outbound
    CPPUNIT_ASSERT_EQUAL(div.flightCountdownLFSMap(sop), number);
  }
  // Must Price
  void checkOnlineMPCarrier(CarrierCode carrier, int numberOutbound, int numberInbound)
  {
    // first for outbound
    CPPUNIT_ASSERT_EQUAL(div.outboundOnlineCarrierCDMustMap(carrier), numberOutbound);
    // next for inbound
    CPPUNIT_ASSERT_EQUAL(div.inboundOnlineCarrierCDMustMap(carrier), numberInbound);
  }

  void checkInterlineMPCarrier(CarrierCode carrier, int numberOutbound, int numberInbound)
  {

    // first for outbound
    CPPUNIT_ASSERT_EQUAL(div.outboundInterlineCarrierCDMustMap(carrier), numberOutbound);
    // next for inbound
    CPPUNIT_ASSERT_EQUAL(div.inboundInterlineCarrierCDMustMap(carrier), numberInbound);
  }
  // LFS
  void checkInOutLFSCarrier(CarrierCode carrier, int numberOutbound, int numberInbound)
  {
    // first for outbound
    CPPUNIT_ASSERT_EQUAL(div.outCxrCountdownLFSMap(carrier), numberOutbound);
    // next for inbound
    CPPUNIT_ASSERT_EQUAL(div.inCxrCountdownLFSMap(carrier), numberInbound);
  }

  void checkOnlineLFSCarrier(CarrierCode carrier, int maxNumber, int currentNumber)
  {
    // first check limit
    CPPUNIT_ASSERT_EQUAL(div.onlineCxrLimitLFSMap(carrier), maxNumber);

    // next check current value
    CPPUNIT_ASSERT_EQUAL(div.onlineCxrCountMapLFS(carrier), currentNumber);
  }

  // Must price
  void testInitFlightMustPriceMaps()
  {
    initFlightAndCarrierMapsWithThreeFlights();

    checkMPFlight(&sop1, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit());
    checkMPFlight(&sop2, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit());
    checkMPFlight(&sop3, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit());
  }

  void testInitCarrierMustPriceMaps()
  {
    initFlightAndCarrierMapsWithThreeFlights();

    int numberOnline = static_cast<int>(((float)esvOptions.percentFactor() + 0.1) *
                                        esvOptions.noOfMustPriceOnlineSolutions());
    int numberInterline = static_cast<int>(((float)esvOptions.percentFactor() + 0.1) *
                                           esvOptions.noOfMustPriceInterlineSolutions());

    checkOnlineMPCarrier(c1, numberOnline, numberOnline);
    checkOnlineMPCarrier(c2, numberOnline, numberOnline);
    checkOnlineMPCarrier(c3, numberOnline, numberOnline);

    checkInterlineMPCarrier(c1, numberInterline, numberInterline);
    checkInterlineMPCarrier(c2, numberInterline, numberInterline);
    checkInterlineMPCarrier(c3, numberInterline, numberInterline);
  }

  void testProcessingFlightAndCarrierOnlineMPSunnyDay()
  {
    initFlightAndCarrierMapsWithThreeFlights();
    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    bool result = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, true);

    int numberOnline = static_cast<int>(((float)esvOptions.percentFactor() + 0.1) *
                                        esvOptions.noOfMustPriceOnlineSolutions());

    CPPUNIT_ASSERT_EQUAL(true, result);

    checkMPFlight(
        &sop1, esvOptions.flightOptionReuseLimit() - 1, esvOptions.flightOptionReuseLimit());
    checkMPFlight(
        &sop2, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit() - 1);

    checkOnlineMPCarrier(c1, numberOnline - 1, numberOnline);
    checkOnlineMPCarrier(c2, numberOnline, numberOnline - 1);
  }
  void testProcessingFlightAndCarrierInterlineMPSunnyDay()
  {
    initFlightAndCarrierMapsWithThreeFlights();
    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop3);

    bool result = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, false);

    int numberInterline = static_cast<int>(((float)esvOptions.percentFactor() + 0.1) *
                                           esvOptions.noOfMustPriceInterlineSolutions());

    CPPUNIT_ASSERT_EQUAL(true, result);

    checkMPFlight(
        &sop1, esvOptions.flightOptionReuseLimit() - 1, esvOptions.flightOptionReuseLimit());
    checkMPFlight(
        &sop3, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit() - 1);

    checkInterlineMPCarrier(c1, numberInterline - 1, numberInterline);
    checkInterlineMPCarrier(c3, numberInterline, numberInterline - 1);
  }

  void testProcessingFlightAndCarrierOnlineMPSunnyFlightCloseToBound()
  {
    esvOptions.flightOptionReuseLimit() = 1;
    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    bool result = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, true);

    int numberOnline = static_cast<int>(((float)esvOptions.percentFactor() + 0.1) *
                                        esvOptions.noOfMustPriceOnlineSolutions());

    CPPUNIT_ASSERT_EQUAL(result, true);
    checkMPFlight(&sop1, 0, 1);
    checkMPFlight(&sop2, 1, 0);
    checkOnlineMPCarrier(c1, numberOnline - 1, numberOnline);
    checkOnlineMPCarrier(c2, numberOnline, numberOnline - 1);
  }

  void testProcessingFlightAndCarrierInterlineMPSunnyFlightCloseToBound()
  {
    esvOptions.flightOptionReuseLimit() = 1;
    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop2);
    sopVec.push_back(&sop3);

    bool result = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, false);

    int numberInterline = static_cast<int>(((float)esvOptions.percentFactor() + 0.1) *
                                           esvOptions.noOfMustPriceInterlineSolutions());

    CPPUNIT_ASSERT_EQUAL(result, true);
    checkMPFlight(&sop2, 0, 1);
    checkMPFlight(&sop3, 1, 0);
    checkInterlineMPCarrier(c2, numberInterline - 1, numberInterline);
    checkInterlineMPCarrier(c3, numberInterline, numberInterline - 1);
  }

  void testProcessingFlightAndCarrierMPSunnyCarrierCloseToBound()
  {
    esvOptions.noOfMustPriceOnlineSolutions() = 5; // will dive 1 after calculation
    esvOptions.noOfMustPriceInterlineSolutions() = 5; // will dive 1 after calculation
    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    bool result1 = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, true);

    CPPUNIT_ASSERT_EQUAL(result1, true);
    checkMPFlight(
        &sop1, esvOptions.flightOptionReuseLimit() - 1, esvOptions.flightOptionReuseLimit());
    checkMPFlight(
        &sop2, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit() - 1);

    checkOnlineMPCarrier(c1, 0, 1);
    checkOnlineMPCarrier(c2, 1, 0);

    bool result2 = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, false);

    CPPUNIT_ASSERT_EQUAL(result2, true);
    checkMPFlight(
        &sop1, esvOptions.flightOptionReuseLimit() - 2, esvOptions.flightOptionReuseLimit());
    checkMPFlight(
        &sop2, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit() - 2);

    checkInterlineMPCarrier(c1, 0, 1);
    checkInterlineMPCarrier(c2, 1, 0);
  }

  void testProcessingFlightAndCarrierOnlineMPFlightCrossedBound()
  {
    esvOptions.flightOptionReuseLimit() = 0;
    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    bool result = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, true);

    int numberOnline = static_cast<int>(((float)esvOptions.percentFactor() + 0.1) *
                                        esvOptions.noOfMustPriceOnlineSolutions());

    CPPUNIT_ASSERT_EQUAL(result, false);
    checkMPFlight(&sop1, 0, 0);
    checkMPFlight(&sop2, 0, 0);
    checkOnlineMPCarrier(c1, numberOnline, numberOnline);
    checkOnlineMPCarrier(c2, numberOnline, numberOnline);
  }
  void testProcessingFlightAndCarrierInterlineMPFlightCrossedBound()
  {
    esvOptions.flightOptionReuseLimit() = 0;
    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop2);
    sopVec.push_back(&sop3);

    bool result = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, false);

    int numberInterline = static_cast<int>(((float)esvOptions.percentFactor() + 0.1) *
                                           esvOptions.noOfMustPriceInterlineSolutions());

    CPPUNIT_ASSERT_EQUAL(result, false);
    checkMPFlight(&sop2, 0, 0);
    checkMPFlight(&sop3, 0, 0);
    checkInterlineMPCarrier(c2, numberInterline, numberInterline);
    checkInterlineMPCarrier(c3, numberInterline, numberInterline);
  }

  void testProcessingFlightAndCarrierMPCarrierCrossedBound()
  {
    esvOptions.noOfMustPriceOnlineSolutions() = 0; // will dive 1 after calculation
    esvOptions.noOfMustPriceInterlineSolutions() = 0; // will dive 1 after calculation
    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    bool result1 = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, true);

    CPPUNIT_ASSERT_EQUAL(result1, false);
    checkMPFlight(&sop1, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit());
    checkMPFlight(&sop2, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit());

    checkOnlineMPCarrier(c1, 0, 0);
    checkOnlineMPCarrier(c2, 0, 0);

    bool result2 = div.processItinMustPriceFlightsAndCarriersLimits(sopVec, false);

    CPPUNIT_ASSERT_EQUAL(result2, false);
    checkMPFlight(&sop1, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit());
    checkMPFlight(&sop2, esvOptions.flightOptionReuseLimit(), esvOptions.flightOptionReuseLimit());

    checkInterlineMPCarrier(c1, 0, 0);
    checkInterlineMPCarrier(c2, 0, 0);
  }
  // LFS
  void testInitFlightLFSMaps()
  {
    initFlightAndCarrierMapsWithThreeFlights();

    checkLFSFlight(&sop1, esvOptions.loMaximumPerOption());
    checkLFSFlight(&sop2, esvOptions.hiMaximumPerOption());
    checkLFSFlight(&sop3, esvOptions.hiMaximumPerOption());
  }

  void testInitCarrierLFSMaps()
  {
    initFlightAndCarrierMapsWithThreeFlights();

    int inoutPercent = static_cast<int>(((float)esvOptions.esvPercent() / 100 + 0.1) *
                                        esvOptions.getRequestedNumberOfSolutions());

    checkInOutLFSCarrier(c1, inoutPercent, inoutPercent);
    checkInOutLFSCarrier(c2, inoutPercent, inoutPercent);
    checkInOutLFSCarrier(c3, inoutPercent, inoutPercent);

    checkOnlineLFSCarrier(c1, esvOptions.noOfMinOnlinePerCarrier(), 0);
    checkOnlineLFSCarrier(c2, esvOptions.noOfMinOnlinePerCarrier(), 0);
    checkOnlineLFSCarrier(c3, esvOptions.noOfMinOnlinePerCarrier(), 0);
  }
  void testRecalcCarrierLFSMaps()
  {
    initFlightAndCarrierMapsWithThreeFlights();

    int onlineMaximum = static_cast<int>((float)esvOptions.getRequestedNumberOfSolutions() *
                                         (float)esvOptions.onlinePercent() / 100);

    int carrierOutItins = 30;
    int recalcLimit =
        static_cast<int>(((double)carrierOutItins) / *(div.totalOutbound()) * onlineMaximum);

    // should be the same for all  carriers
    recalcCarrierOnlineLFSMapsWithThreeCarriers();

    checkOnlineLFSCarrier(c1, recalcLimit, 0);
    checkOnlineLFSCarrier(c2, recalcLimit, 0);
    checkOnlineLFSCarrier(c3, recalcLimit, 0);
  }

  void testProcessingFlightAndCarrierLFSSunnyDayPassOneTwo()
  {
    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop3);

    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersOnlineLimits(sopVec, carrier);

    CPPUNIT_ASSERT_EQUAL(true, result);

    checkLFSFlight(&sop1, esvOptions.loMaximumPerOption() - 1);
    checkLFSFlight(&sop3, esvOptions.hiMaximumPerOption() - 1);

    checkOnlineLFSCarrier(c1, esvOptions.noOfMinOnlinePerCarrier(), 1);
    checkOnlineLFSCarrier(c3, esvOptions.noOfMinOnlinePerCarrier(), 0);
  }

  void testProcessingFlightAndCarrierLFSSunnyDayPassOneTwoFlightCloseToBound()
  {
    esvOptions.hiMaximumPerOption() = 1;

    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersOnlineLimits(sopVec, carrier);

    CPPUNIT_ASSERT_EQUAL(result, true);
    checkLFSFlight(&sop1, esvOptions.loMaximumPerOption() - 1);
    checkLFSFlight(&sop2, 0);

    checkOnlineLFSCarrier(c1, esvOptions.noOfMinOnlinePerCarrier(), 1);
    checkOnlineLFSCarrier(c2, esvOptions.noOfMinOnlinePerCarrier(), 0);
  }

  void testProcessingFlightAndCarrierLFSSunnyDayPassOneTwoCarrierCloseToBound()
  {
    esvOptions.noOfMinOnlinePerCarrier() = 1;

    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersOnlineLimits(sopVec, carrier);

    CPPUNIT_ASSERT_EQUAL(true, result);
    checkLFSFlight(&sop1, esvOptions.loMaximumPerOption() - 1);
    checkLFSFlight(&sop2, esvOptions.hiMaximumPerOption() - 1);

    checkOnlineLFSCarrier(c1, 1, 1);
    checkOnlineLFSCarrier(c2, 1, 0);
  }
  void testProcessingFlightAndCarrierLFSPassOneTwoFlightCrossedBound()
  {
    esvOptions.hiMaximumPerOption() = 0;
    esvOptions.loMaximumPerOption() = 0;

    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersOnlineLimits(sopVec, carrier);

    CPPUNIT_ASSERT_EQUAL(result, false);
    checkLFSFlight(&sop1, 0);
    checkLFSFlight(&sop2, 0);

    checkOnlineLFSCarrier(c1, esvOptions.noOfMinOnlinePerCarrier(), 0);
    checkOnlineLFSCarrier(c2, esvOptions.noOfMinOnlinePerCarrier(), 0);
  }
  void testProcessingFlightAndCarrierLFSPassOneTwoCarrierCrossedBound()
  {
    esvOptions.noOfMinOnlinePerCarrier() = 0;

    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersOnlineLimits(sopVec, carrier);

    CPPUNIT_ASSERT_EQUAL(false, result);
    checkLFSFlight(&sop1, esvOptions.loMaximumPerOption());
    checkLFSFlight(&sop2, esvOptions.hiMaximumPerOption());

    checkOnlineLFSCarrier(c1, 0, 0);
    checkOnlineLFSCarrier(c2, 0, 0);
  }

  // pass 3
  void testProcessingFlightAndCarrierLFSSunnyDayPassThree()
  {
    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop3);
    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersLimits(sopVec, carrier);

    int inoutPercent = static_cast<int>((((float)esvOptions.esvPercent()) / 100 + 0.1) *
                                        esvOptions.getRequestedNumberOfSolutions());

    CPPUNIT_ASSERT_EQUAL(true, result);

    checkLFSFlight(&sop1, esvOptions.loMaximumPerOption() - 1);
    checkLFSFlight(&sop3, esvOptions.hiMaximumPerOption() - 1);

    checkInOutLFSCarrier(c1, inoutPercent - 1, inoutPercent);
    checkInOutLFSCarrier(c3, inoutPercent, inoutPercent - 1);
  }

  void testProcessingFlightAndCarrierLFSSunnyDayPassThreeFlightCloseToBound()
  {
    esvOptions.hiMaximumPerOption() = 1;
    esvOptions.loMaximumPerOption() = 1;

    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersLimits(sopVec, carrier);

    int inoutPercent = static_cast<int>(((float)esvOptions.esvPercent() / 100 + 0.1) *
                                        esvOptions.getRequestedNumberOfSolutions());

    CPPUNIT_ASSERT_EQUAL(result, true);
    checkLFSFlight(&sop1, 0);
    checkLFSFlight(&sop2, 0);

    checkInOutLFSCarrier(c1, inoutPercent - 1, inoutPercent);
    checkInOutLFSCarrier(c2, inoutPercent, inoutPercent - 1);
  }
  void testProcessingFlightAndCarrierLFSSunnyDayPassThreeCarrierCloseToBound()
  {
    esvOptions.esvPercent() = 0;
    esvOptions.setRequestedNumberOfSolutions(10); // both together will give 1 after calculation

    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersLimits(sopVec, carrier);

    CPPUNIT_ASSERT_EQUAL(true, result);
    checkLFSFlight(&sop1, esvOptions.loMaximumPerOption() - 1);
    checkLFSFlight(&sop2, esvOptions.hiMaximumPerOption() - 1);

    checkInOutLFSCarrier(c1, 0, 1);
    checkInOutLFSCarrier(c2, 1, 0);
  }
  void testProcessingFlightAndCarrierLFSPassThreeFlightCrossedBound()
  {
    esvOptions.hiMaximumPerOption() = 0;
    esvOptions.loMaximumPerOption() = 0;

    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersLimits(sopVec, carrier);

    int inoutPercent = static_cast<int>(((float)esvOptions.esvPercent() / 100 + 0.1) *
                                        esvOptions.getRequestedNumberOfSolutions());

    CPPUNIT_ASSERT_EQUAL(result, false);
    checkLFSFlight(&sop1, 0);
    checkLFSFlight(&sop2, 0);

    checkInOutLFSCarrier(c1, inoutPercent, inoutPercent);
    checkInOutLFSCarrier(c2, inoutPercent, inoutPercent);
  }
  void testProcessingFlightAndCarrierLFSPassThreeCarrierCrossedBound()
  {
    esvOptions.esvPercent() = 0;
    esvOptions.setRequestedNumberOfSolutions(0); // both together will give 0 after calculation

    initFlightAndCarrierMapsWithThreeFlights();

    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    sopVec.push_back(&sop1);
    sopVec.push_back(&sop2);

    CarrierCode carrier;
    bool result = div.processItinLFSFlightsAndCarriersLimits(sopVec, carrier);

    CPPUNIT_ASSERT_EQUAL(false, result);
    checkLFSFlight(&sop1, esvOptions.loMaximumPerOption());
    checkLFSFlight(&sop2, esvOptions.hiMaximumPerOption());

    checkInOutLFSCarrier(c1, 0, 0);
    checkInOutLFSCarrier(c2, 0, 0);
  }

  // MUST PRICE
  void testCalcMustPriceUpperBound()
  {
    float minFareValue = 2.5;

    float expectedUpperBound = minFareValue * esvOptions.upperBoundFactorForNonstop();

    float upperBound = div.calcMustPriceUpperBound(minFareValue, true);
    CPPUNIT_ASSERT_EQUAL(expectedUpperBound, upperBound);

    expectedUpperBound = minFareValue * esvOptions.upperBoundFactorForNotNonstop();

    upperBound = div.calcMustPriceUpperBound(minFareValue, false);
    CPPUNIT_ASSERT_EQUAL(expectedUpperBound, upperBound);

    minFareValue = 0;
    upperBound = div.calcMustPriceUpperBound(minFareValue, false);
    CPPUNIT_ASSERT_EQUAL((float)0.0, upperBound);
  }
  // LFS
  void testCalcLFSUpperBound()
  {
    float minFareValue = 2.5;

    float expectedUpperBound = minFareValue * esvOptions.upperBoundFactorForLFS();

    float upperBound = div.calcLFSUpperBound(minFareValue);
    CPPUNIT_ASSERT_EQUAL(expectedUpperBound, upperBound);

    minFareValue = 0;
    upperBound = div.calcLFSUpperBound(minFareValue);
    CPPUNIT_ASSERT_EQUAL((float)0.0, upperBound);
  }

  // MUST PRICE
  void testCalcMustPriceLimitForCurrPass()
  {
    int maxPassSize = 45;
    int numAlreadyPicked = 23;

    int mpLimitForCurrPass = div.calcMustPriceLimitForCurrPass(numAlreadyPicked, maxPassSize);
    CPPUNIT_ASSERT_EQUAL(maxPassSize, mpLimitForCurrPass);

    maxPassSize = 300;

    mpLimitForCurrPass = div.calcMustPriceLimitForCurrPass(numAlreadyPicked, maxPassSize);
    CPPUNIT_ASSERT_EQUAL(esvOptions.getRequestedNumberOfSolutions() -
                             esvOptions.noOfESVLowFareSolutionsReq() - numAlreadyPicked,
                         mpLimitForCurrPass);
  }

  void initInterlineRestrictedCxr()
  {
    ESVPQDiversifier d;
    ShoppingTrx trx;
    PricingOptions options;
    d._shoppingTrx = &trx;
    d._shoppingTrx->setOptions(&options);
    d.initInterlineRestrictedCxr();
    CPPUNIT_ASSERT(true);
  }

  void isOnlineOnlyRestricted()
  {
    ESVPQDiversifier d;
    ShoppingTrx trx;
    PricingOptions options;
    d._shoppingTrx = &trx;
    d._shoppingTrx->setOptions(&options);
    buildOnlineOnlyCxr(d);
    checkOnlineOnlyCxr(d);
    CPPUNIT_ASSERT(d.isOnlineOnlyRestricted("WN"));
    CPPUNIT_ASSERT(d.isOnlineOnlyRestricted("SY"));
    CPPUNIT_ASSERT(d.isOnlineOnlyRestricted("XE"));
    CPPUNIT_ASSERT(d.isOnlineOnlyRestricted("2R"));
    CPPUNIT_ASSERT(d.isOnlineOnlyRestricted("B6"));
    CPPUNIT_ASSERT(d._onlineOnlyCxr.size() == 5);
  }

  void buildOnlineOnlyCxr(ESVPQDiversifier& d)
  {
    d._onlineOnlyCxr.push_back("WN");
    d._onlineOnlyCxr.push_back("SY");
    d._onlineOnlyCxr.push_back("XE");
    d._onlineOnlyCxr.push_back("2R");
    d._onlineOnlyCxr.push_back("B6");
  }

  void checkOnlineOnlyCxr(ESVPQDiversifier& d)
  {
    CPPUNIT_ASSERT(d._onlineOnlyCxr.size() == 5);
    CPPUNIT_ASSERT(findCxr(d._onlineOnlyCxr, "WN"));
    CPPUNIT_ASSERT(findCxr(d._onlineOnlyCxr, "SY"));
    CPPUNIT_ASSERT(findCxr(d._onlineOnlyCxr, "XE"));
    CPPUNIT_ASSERT(findCxr(d._onlineOnlyCxr, "2R"));
    CPPUNIT_ASSERT(findCxr(d._onlineOnlyCxr, "B6"));
    CPPUNIT_ASSERT(d._onlineOnlyCxr.size() == 5);
  }

  void cxrRestrictionCorrect()
  {
    ESVPQDiversifier d;
    ShoppingTrx trx;
    PricingOptions options;
    d._shoppingTrx = &trx;
    d._shoppingTrx->setOptions(&options);
    initRestrictedData(d);
    CPPUNIT_ASSERT(d.cxrRestrictionCorrect("FL", "FI"));
    CPPUNIT_ASSERT(d.cxrRestrictionCorrect("FL", "F9"));
    CPPUNIT_ASSERT(d.cxrRestrictionCorrect("FL", "HA"));
    CPPUNIT_ASSERT(d.cxrRestrictionCorrect("FL", "TZ"));
    CPPUNIT_ASSERT(d.cxrRestrictionCorrect("FL", "UA"));
    CPPUNIT_ASSERT(d.cxrRestrictionCorrect("FL", "US"));
    CPPUNIT_ASSERT(d.cxrRestrictionCorrect("FL", "YX"));
    CPPUNIT_ASSERT(!d.cxrRestrictionCorrect("FL", "FL"));
    CPPUNIT_ASSERT(!d.cxrRestrictionCorrect("FL", "AA"));
    CPPUNIT_ASSERT(!d.cxrRestrictionCorrect("FL", "AB"));
  }

  void initRestrictedData(ESVPQDiversifier& d)
  {
    buildOnlineOnlyCxr(d);
    checkOnlineOnlyCxr(d);
    buildRestrictedCxr(d);
    checkRestrictedCxr(d);
  }

  void checkCxrRestrictions()
  {
    check1cxr();
    check2cxr();
    check3cxr();
    check4cxr();
    check5cxr();
    check6cxr();
  }

  void check1cxr()
  {
    check1leg1cxr("FL", true);
    check1leg1cxr("WN", true);
    check1leg1cxr("HA", true);
    check1leg1cxr("AA", true);
  }

  void check2cxr()
  {
    check2cxr("WN", "WN", true);
    check2cxr("WN", "SY", !true);
    check2cxr("XE", "FL", !true);
    check2cxr("2R", "F9", !true);
    check2cxr("AA", "B6", !true);
    check2cxr("FL", "FL", true);
    check2cxr("FL", "HA", true);
    check2cxr("UA", "FL", true);
    check2cxr("FL", "AA", !true);
    check2cxr("AA", "FL", !true);
    check2cxr("FL", "WN", !true);
    check2cxr("FI", "F9", true);
    check2cxr("AA", "F9", true);
    check2cxr("YX", "AA", true);
  }

  void check3cxr()
  {
    check3cxr("WN", "WN", "WN", true);
    check3cxr("WN", "WN", "SY", !true);
    check3cxr("WN", "WN", "AA", !true);
    check3cxr("WN", "F9", "WN", !true);
    check3cxr("FL", "FL", "FL", true);
    check3cxr("FL", "F9", "HA", true);
    check3cxr("FL", "YX", "AA", !true);
    check3cxr("UA", "US", "AA", true);
    check3cxr("AA", "AB", "AC", true);
  }

  void check4cxr()
  {
    check4cxr("SY", "SY", "SY", "SY", true);
    check4cxr("SY", "SY", "SY", "XE", !true);
    check4cxr("SY", "SY", "FL", "SY", !true);
    check4cxr("SY", "F9", "SY", "SY", !true);
    check4cxr("AA", "SY", "SY", "SY", !true);
    check4cxr("FL", "FL", "HA", "US", true);
    check4cxr("FL", "AA", "FL", "US", !true);
    check4cxr("FL", "SY", "FI", "TZ", !true);
    check4cxr("FL", "AA", "FI", "FI", !true);
    check4cxr("YX", "US", "UA", "TZ", true);
    check4cxr("AA", "FI", "F9", "HA", true);
  }

  void check5cxr()
  {
    check5cxr("XE", "XE", "XE", "XE", "XE", true);
    check5cxr("XE", "XE", "WN", "XE", "XE", !true);
    check5cxr("XE", "XE", "XE", "AA", "XE", !true);
    check5cxr("FL", "FI", "F9", "HA", "TZ", true);
    check5cxr("FL", "WN", "F9", "HA", "TZ", !true);
    check5cxr("FL", "YX", "F9", "HA", "AA", !true);
    check5cxr("FL", "FL", "FL", "FL", "FL", true);
    check5cxr("AA", "YX", "UA", "US", "TZ", true);
  }

  void check6cxr()
  {
    check6cxr("2R", "2R", "2R", "2R", "2R", "2R", true);
    check6cxr("2R", "2R", "2R", "2R", "WN", "2R", !true);
    check6cxr("2R", "2R", "2R", "2R", "2R", "AA", !true);
    check6cxr("2R", "2R", "FL", "2R", "2R", "2R", !true);
    check6cxr("2R", "YX", "2R", "2R", "2R", "2R", !true);
    check6cxr("FL", "FI", "F9", "HA", "TZ", "UA", true);
    check6cxr("FL", "FL", "FL", "FL", "FL", "FL", true);
    check6cxr("FL", "FL", "WN", "FL", "FL", "FL", !true);
    check6cxr("FL", "FL", "FL", "AA", "FL", "FL", !true);
    check6cxr("AA", "AB", "YX", "UA", "US", "TZ", true);
  }

  void check2cxr(CarrierCode cxr1, CarrierCode cxr2, bool exResult)
  {
    std::vector<CarrierCode> cxrVec;
    cxrVec.push_back(cxr1);
    cxrVec.push_back(cxr2);
    checkCxr(cxrVec, exResult);
  }

  void check3cxr(CarrierCode cxr1, CarrierCode cxr2, CarrierCode cxr3, bool exResult)
  {
    std::vector<CarrierCode> cxrVec;
    cxrVec.push_back(cxr1);
    cxrVec.push_back(cxr2);
    cxrVec.push_back(cxr3);
    checkCxr(cxrVec, exResult);
  }

  void
  check4cxr(CarrierCode cxr1, CarrierCode cxr2, CarrierCode cxr3, CarrierCode cxr4, bool exResult)
  {
    std::vector<CarrierCode> cxrVec;
    cxrVec.push_back(cxr1);
    cxrVec.push_back(cxr2);
    cxrVec.push_back(cxr3);
    cxrVec.push_back(cxr4);
    checkCxr(cxrVec, exResult);
  }

  void check5cxr(CarrierCode cxr1,
                 CarrierCode cxr2,
                 CarrierCode cxr3,
                 CarrierCode cxr4,
                 CarrierCode cxr5,
                 bool exResult)
  {
    std::vector<CarrierCode> cxrVec;
    cxrVec.push_back(cxr1);
    cxrVec.push_back(cxr2);
    cxrVec.push_back(cxr3);
    cxrVec.push_back(cxr4);
    cxrVec.push_back(cxr5);
    checkCxr(cxrVec, exResult);
  }

  void check6cxr(CarrierCode cxr1,
                 CarrierCode cxr2,
                 CarrierCode cxr3,
                 CarrierCode cxr4,
                 CarrierCode cxr5,
                 CarrierCode cxr6,
                 bool exResult)
  {
    std::vector<CarrierCode> cxrVec;
    cxrVec.push_back(cxr1);
    cxrVec.push_back(cxr2);
    cxrVec.push_back(cxr3);
    cxrVec.push_back(cxr4);
    cxrVec.push_back(cxr5);
    cxrVec.push_back(cxr6);
    checkCxr(cxrVec, exResult);
  }

  void checkCxr(const std::vector<CarrierCode>& cxrVec, bool exResult)
  {
    // check 1 leg
    check1legCxr(cxrVec, exResult);

    // check all 2 legs combinations
    int cxrNum = cxrVec.size();
    for (int i = 1; i < cxrNum; ++i)
    {
      checkCxr(i, cxrNum - i, cxrVec, exResult);
    }
  }

  void check1legCxr(const std::vector<CarrierCode>& cxrVec, bool exResult)
  {
    ESVPQDiversifier d;
    ShoppingTrx trx;
    PricingOptions options;
    d._shoppingTrx = &trx;
    d._shoppingTrx->setOptions(&options);
    initRestrictedData(d);
    ESVPQItem pqItem;
    ESVSopWrapper sopWrapper1;
    pqItem.outSopWrapper() = &sopWrapper1;
    std::string msg;
    std::vector<PaxTypeFare*> paxTypeFareVec1;
    int cxrVecSize = cxrVec.size();
    for (int i = 0; i < cxrVecSize; ++i)
    {
      FareMarket* fm = _memHandle.create<FareMarket>();
      fm->governingCarrier() = cxrVec[i];

      PaxTypeFare* fare = _memHandle.create<PaxTypeFare>();
      fare->fareMarket() = fm;

      paxTypeFareVec1.push_back(fare);

      msg += cxrVec[i] + " ";
    }
    sopWrapper1._paxTypeFareVec = &paxTypeFareVec1;
    std::string addInfo;
    CPPUNIT_ASSERT_MESSAGE(msg, d.checkRestrictedCarriers(&pqItem, addInfo) == exResult);
  }

  void checkCxr(int out, int in, const std::vector<CarrierCode>& cxrVec, bool exResult)
  {
    CPPUNIT_ASSERT(int(cxrVec.size()) == out + in);
    ESVPQDiversifier d;
    ShoppingTrx trx;
    PricingOptions options;
    d._shoppingTrx = &trx;
    d._shoppingTrx->setOptions(&options);
    initRestrictedData(d);
    ESVPQItem pqItem;
    ESVSopWrapper sopWrapper1;
    ESVSopWrapper sopWrapper2;
    pqItem.outSopWrapper() = &sopWrapper1;
    pqItem.inSopWrapper() = &sopWrapper2;
    std::string msg;
    std::vector<PaxTypeFare*> paxTypeFareVec1;
    std::vector<PaxTypeFare*> paxTypeFareVec2;
    for (int i = 0; i < out + in; ++i)
    {
      FareMarket* fm = _memHandle.create<FareMarket>();
      fm->governingCarrier() = cxrVec[i];

      PaxTypeFare* fare = _memHandle.create<PaxTypeFare>();
      fare->fareMarket() = fm;
      if (i < out)
      {
        paxTypeFareVec1.push_back(fare);
      }
      else
      {
        paxTypeFareVec2.push_back(fare);
      }
      msg += cxrVec[i] + " ";
    }
    sopWrapper1._paxTypeFareVec = &paxTypeFareVec1;
    sopWrapper2._paxTypeFareVec = &paxTypeFareVec2;
    std::string addInfo;
    CPPUNIT_ASSERT_MESSAGE(msg, d.checkRestrictedCarriers(&pqItem, addInfo) == exResult);
  }
  void check1leg1cxr(CarrierCode cxr, bool exResult)
  {
    ESVPQDiversifier d;
    ShoppingTrx trx;
    PricingOptions options;
    d._shoppingTrx = &trx;
    d._shoppingTrx->setOptions(&options);
    initRestrictedData(d);
    ESVPQItem pqItem;
    ESVSopWrapper sopWrapper1;
    pqItem.outSopWrapper() = &sopWrapper1;
    FareMarket fm1;
    fm1.governingCarrier() = cxr;
    PaxTypeFare fare1;
    fare1.fareMarket() = &fm1;
    std::vector<PaxTypeFare*> paxTypeFareVec1;
    paxTypeFareVec1.push_back(&fare1);
    sopWrapper1._paxTypeFareVec = &paxTypeFareVec1;
    std::string msg = cxr;
    std::string addInfo;
    // CPPUNIT_ASSERT_MESSAGE(msg, d.checkRestrictedCarriers(&pqItem, addInfo) == exResult);
  }

  void buildRestrictedCxr(ESVPQDiversifier& d)
  {
    std::vector<CarrierCode> cxrVec;
    cxrVec.push_back("FI");
    cxrVec.push_back("F9");
    cxrVec.push_back("HA");
    cxrVec.push_back("TZ");
    cxrVec.push_back("UA");
    cxrVec.push_back("US");
    cxrVec.push_back("YX");
    std::pair<CarrierCode, std::vector<CarrierCode> > p;
    p.first = "FL";
    p.second = cxrVec;
    d._restrictedCxr.push_back(p);
  }

  void checkRestrictedCxr(ESVPQDiversifier& d)
  {
    CPPUNIT_ASSERT(d._restrictedCxr.size() == 1);
    std::pair<CarrierCode, std::vector<CarrierCode> >& p = d._restrictedCxr[0];
    CPPUNIT_ASSERT(p.first == "FL");
    std::vector<CarrierCode>& cxrVec = p.second;
    CPPUNIT_ASSERT(findCxr(cxrVec, "FI"));
    CPPUNIT_ASSERT(findCxr(cxrVec, "F9"));
    CPPUNIT_ASSERT(findCxr(cxrVec, "HA"));
    CPPUNIT_ASSERT(findCxr(cxrVec, "TZ"));
    CPPUNIT_ASSERT(findCxr(cxrVec, "UA"));
    CPPUNIT_ASSERT(findCxr(cxrVec, "US"));
    CPPUNIT_ASSERT(findCxr(cxrVec, "YX"));
  }

  bool findCxr(std::vector<CarrierCode>& cxrVec, CarrierCode cxr)
  {
    bool found = false;
    std::vector<CarrierCode>::iterator cxrIter = cxrVec.begin();
    std::vector<CarrierCode>::iterator cxrIterEnd = cxrVec.end();
    while (!found && (cxrIter != cxrIterEnd))
    {
      if (cxr == *cxrIter)
      {
        found = true;
      }
      else
      {
        ++cxrIter;
      }
    }

    return found;
  }

  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  CarrierCode c1, c2, c3;
  Itin itin1, itin2, itin3;
  ShoppingTrx::SchedulingOption sop1, sop2, sop3;
  //   log4cxx::LoggerPtr logger;
  ESVOptions esvOptions;
  ESVPQDiversifierStub div;
};
CPPUNIT_TEST_SUITE_REGISTRATION(ESVPQDiversifierTest);

} // namespace tse
