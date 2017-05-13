#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "DBAccess/SurfaceSectorExempt.h"
#include "DBAccess/TPMExclusion.h"
#include "Diagnostic/Diag450Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RoutingInfo.h"
#include "Routing/TravelRoute.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class MockRoutingInfo : public RoutingInfo
{
public:
  MockRoutingInfo() {}

  void setDataTPMExclusion_OldLogic()
  {
    routingStatus() = true;
    _mileageInfo.processed() = true;
    _mileageInfo.valid() = true;
    _mileageInfo.southAtlanticTPMExclusion() = true;

    Market market, market2;
    market.first = "LON";
    market.second = "BUE";
    _mileageInfo.southAtlanticTPDCities().push_back(market);
    market2.first = "BUE";
    market2.second = "NYC";
    _mileageInfo.southAtlanticTPDCities().push_back(market2);
    mileageInfo() = &_mileageInfo;
  }

  void setDataTPMExclusion()
  {
    setDataTPMExclusion_OldLogic();
    _tpmExclusion.carrier() = "LH";
    _tpmExclusion.seqNo() = 1;
    _mileageInfo.tpmExclusion() = &_tpmExclusion;
  }

  // data
  MileageInfo _mileageInfo;
  TPMExclusion _tpmExclusion;
};

class Diag450CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag450CollectorTest);
  CPPUNIT_TEST(testIsActive);
  CPPUNIT_TEST(testBuildRoutingValidationEmptyDisplay);
  CPPUNIT_TEST(testDisplayRouting);

  CPPUNIT_TEST(testUnticketedPointMessage_AnyPoint);
  CPPUNIT_TEST(testUnticketedPointMessage_TktOnly);

  CPPUNIT_TEST(testEntryExitPointInfo_AnyPoint);
  CPPUNIT_TEST(testEntryExitPointInfo_EntryExitOnly);

  CPPUNIT_TEST(testTerminalPointMessage_WithTerminalPoints);
  CPPUNIT_TEST(testTerminalPointMessage_WithoutTerminalPoints);

  CPPUNIT_TEST(testDisplayMissingCityMessage);
  CPPUNIT_TEST(testSurfaceMessage);
  CPPUNIT_TEST(testSouthAtlanticMessage_OldLogic);
  CPPUNIT_TEST(testSouthAtlanticMessage);
  CPPUNIT_TEST(testDisplayConstructedRouting);
  CPPUNIT_TEST(testDisplayFare);
  CPPUNIT_TEST(testDisplayCityCarrierVec);
  CPPUNIT_TEST(testDisplayMileageSurcharge);
  CPPUNIT_TEST(testDisplayMileageSurchargeB);
  CPPUNIT_TEST(testDisplayMileageEqualization);
  CPPUNIT_TEST(testDisplayMileageException);
  CPPUNIT_TEST(testDisplayTPDCities);
  CPPUNIT_TEST(testDisplayPSRs);
  CPPUNIT_TEST(testDisplayPSRZone);
  CPPUNIT_TEST(testDisplayPSRWrapDisplay);
  CPPUNIT_TEST(testDisplayRestriction1);
  CPPUNIT_TEST(testDisplayRestriction2);
  CPPUNIT_TEST(testDisplayRestriction3);
  CPPUNIT_TEST(testDisplayRestriction4);
  CPPUNIT_TEST(testDisplayRestriction5);
  CPPUNIT_TEST(testDisplayRestriction6);
  CPPUNIT_TEST(testDisplayRestriction7);
  CPPUNIT_TEST(testDisplayRestriction8_NotRtw);
  CPPUNIT_TEST(testDisplayRestriction8);
  CPPUNIT_TEST(testDisplayRestriction9_NotRtw);
  CPPUNIT_TEST(testDisplayRestriction9);
  CPPUNIT_TEST(testDisplayRestriction10);
  CPPUNIT_TEST(testDisplayRestriction10_NotRtw);
  CPPUNIT_TEST(testDisplayRestriction11);
  CPPUNIT_TEST(testDisplayRestriction12);
  CPPUNIT_TEST(testDisplayRestriction12_Split);
  CPPUNIT_TEST(testDisplayRestriction13);
  CPPUNIT_TEST(testDisplayRestriction14);
  CPPUNIT_TEST(testDisplayRestriction15);
  CPPUNIT_TEST(testDisplayRestriction16);
  CPPUNIT_TEST(testDisplayRestriction17);
  CPPUNIT_TEST(testDisplayRestriction18);
  CPPUNIT_TEST(testDisplayRestriction19);
  CPPUNIT_TEST(testDisplayRestriction21);
  CPPUNIT_TEST_SUITE_END();

  Diagnostic* _diagroot;
  Diag450Collector* _diag;
  RoutingRestriction* _rest;
  RestrictionInfo* _restInfo;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _diagroot = _memH.insert(new Diagnostic(Diagnostic450));
    _diagroot->activate();

    _diag = _memH.insert(new Diag450Collector(*_diagroot));
    _diag->enable(Diagnostic450);

    _rest = _memH.create<RoutingRestriction>();
    _restInfo = _memH.create<RestrictionInfo>();

    _rest->mpm() = 1000;
    _restInfo->processed() = true;
    _restInfo->valid() = true;
  }

  void tearDown() { _memH.clear(); }

  void testIsActive() { CPPUNIT_ASSERT(_diag->isActive()); }

  void testBuildRoutingValidationEmptyDisplay()
  {
    _diag->buildHeader();

    std::string expected = "\n**************** ROUTING VALIDATION RESULTS *******************\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRouting()
  {
    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "MEM";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MIA";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 99;
    routing.routing() = "0519";
    routing.effDateStr() = " ";
    routing.linkNo() = 0;
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 1;
    routing.nooftexts() = 0;
    routing.validityInd() = ' ';
    routing.inhibit() = ' ';
    routing.directionalInd() = ' ';
    routing.domRtgvalInd() = ' ';
    routing.commonPointInd() = ' ';
    routing.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;

    // Build a Restriction
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "1";
    _rest->marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "MIA"; // City2
    _rest->viaMarket() = "CHI"; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    // Build the RestrictionInfos Map and set pointer to it in RoutingInfo
    RestrictionInfos resInfos;
    resInfos.insert(std::map<RoutingRestriction*, RestrictionInfo>::value_type(_rest, *_restInfo));
    RestrictionInfos* resInfoPtr = &resInfos;
    routingInfo.restrictions() = resInfoPtr;
    routingInfo.tcrRoutingTariffCode() = "SABRE";

    // Build a map of RoutingInfos
    typedef std::map<Routing*, RoutingInfo> RoutingInfos;
    RoutingInfos routingInfos;
    routingInfos.insert(std::map<Routing*, RoutingInfo>::value_type(&routing, routingInfo));

    _diag->displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    _diag->displayRoutingStatus(tvlRoute, routingInfo);
    _diag->displayMileageMessages(routingInfo);
    _diag->displayMapMessages(routingInfo);

    std::string str = _diag->str();

    std::string expected;
    expected += "AA  MEM-AA-CHI-AA-MIA\n";
    expected += "    ATP   WH  0519 TRF-  99    SABRE  ROUTING VALID\n";
    expected += "    NO ROUTE MAP EXISTS\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    //---------------------------------------
    // Test Surface Sector Display
    //---------------------------------------
    TravelRoute tvlRoute2;
    boardCty.loc() = "DFW";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MEM";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute2.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "MEM";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
    tvlRoute2.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "ATL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "BOS";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute2.travelRoute().push_back(cityCarrier);
    tvlRoute2.govCxr() = "AA";
    strToGlobalDirection(tvlRoute2.globalDir(), "WH");

    routing.routingTariff() = 1;
    routing.routing() = "0075";

    // Create MapInfo Validation Info
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;
    routingInfo.routingStatus() = true;

    _diag->flushMsg();
    _diag->displayCityCarriers(tvlRoute2.govCxr(), tvlRoute2.travelRoute());
    _diag->displayRoutingStatus(tvlRoute2, routingInfo);
    _diag->displayMileageMessages(routingInfo);
    _diag->displayMapMessages(routingInfo);

    str = _diag->str();
    expected = "AA  DFW-AA-MEM // ATL-AA-BOS\n";
    expected += "    ATP   WH  0075 TRF-   1    SABRE  ROUTING VALID\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    //---------------------------------------
    // Test Hidden City Display
    //---------------------------------------
    TravelRoute tvlRoute3;
    boardCty.loc() = "MIA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CLT";
    offCty.isHiddenCity() = true;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "US";
    cityCarrier.offCity() = offCty;
    tvlRoute3.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CLT";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DCA";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "US";
    cityCarrier.offCity() = offCty;
    tvlRoute3.travelRoute().push_back(cityCarrier);
    tvlRoute3.govCxr() = "AA";
    strToGlobalDirection(tvlRoute3.globalDir(), "WH");

    routing.routingTariff() = 1;
    routing.routing() = "0005";
    mapInfo.processed() = false;

    _diag->flushMsg();
    _diag->displayCityCarriers(tvlRoute3.govCxr(), tvlRoute3.travelRoute());
    _diag->displayRoutingStatus(tvlRoute3, routingInfo);
    _diag->displayMileageMessages(routingInfo);
    _diag->displayMapMessages(routingInfo);

    str = _diag->str();
    expected = "AA  MIA-US-*CLT-US-DCA\n";
    expected += "      * INTERMEDIATE NON-TICKETED POINTS TRACKED ON ROUTING\n";
    expected += "    ATP   WH  0005 TRF-   1    SABRE  ROUTING VALID\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    //----------------------------------------------------------------------
    // Test Travel Route that Exceeds 63 characters and breaks after carrier
    //----------------------------------------------------------------------
    TravelRoute tvlRoute4;
    boardCty.loc() = "SAT";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "TUL";
    offCty.isHiddenCity() = true;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "TUL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DFW";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "DFW";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MSP";
    offCty.isHiddenCity() = true;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "MSP";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DTT";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "DTT";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "JFK";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "JFK";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "BOS";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "BOS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "FLL";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "FLL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MIA";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "MIA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "SJU";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute4.travelRoute().push_back(cityCarrier);
    tvlRoute4.govCxr() = "AA";
    strToGlobalDirection(tvlRoute4.globalDir(), "WH");

    routing.routingTariff() = 787;
    routing.routing() = "5555";

    // Create MapInfo Validation Info
    mapInfo.processed() = true;
    mapInfo.valid() = false;

    _diag->flushMsg();
    _diag->displayCityCarriers(tvlRoute4.govCxr(), tvlRoute4.travelRoute());
    _diag->displayRoutingStatus(tvlRoute4, routingInfo);
    _diag->displayMileageMessages(routingInfo);
    _diag->displayMapMessages(routingInfo);

    str = _diag->str();
    expected = "AA  SAT-AA-*TUL-AA-DFW-AA-CHI-AA-*MSP-AA-DTT-AA-JFK-AA-\n";
    expected += "    BOS-AA-FLL-AA-MIA-AA-SJU\n";
    expected += "      * INTERMEDIATE NON-TICKETED POINTS TRACKED ON ROUTING\n";
    expected += "    ATP   WH  5555 TRF- 787    SABRE  ROUTING VALID\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    // Test Mileage Routing Display
    routing.routingTariff() = 4;
    routing.routing() = "0000";

    // Create MileageInfo Validation Info with a vector of Surface Sector Markets
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    routingInfo.mileageInfo() = mileageInfoPtr;
    routingInfo.mileageInfo()->totalApplicableMPM() = 8011;
    routingInfo.routingTariff() = 4;

    routingInfo.mapInfo() = 0;
    _diag->flushMsg();
    _diag->displayCityCarriers(tvlRoute4.govCxr(), tvlRoute4.travelRoute());
    _diag->displayRoutingStatus(tvlRoute4, routingInfo);
    _diag->displayMileageMessages(routingInfo);
    _diag->displayMapMessages(routingInfo);

    str = _diag->str();
    expected = "AA  SAT-AA-*TUL-AA-DFW-AA-CHI-AA-*MSP-AA-DTT-AA-JFK-AA-\n";
    expected += "    BOS-AA-FLL-AA-MIA-AA-SJU\n";
    expected += "      * INTERMEDIATE NON-TICKETED POINTS TRACKED ON ROUTING\n";
    expected += "    MPM   WH  8011 TRF-   4    SABRE  ROUTING VALID\n";
    expected += "    NO ROUTE MAP EXISTS\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    tvlRoute4.maxPermittedMileage() = 811;

    _diag->flushMsg();
    _diag->displayCityCarriers(tvlRoute4.govCxr(), tvlRoute4.travelRoute());
    _diag->displayRoutingStatus(tvlRoute4, routingInfo);
    _diag->displayMileageMessages(routingInfo);
    _diag->displayMapMessages(routingInfo);

    str = _diag->str();
    expected = "AA  SAT-AA-*TUL-AA-DFW-AA-CHI-AA-*MSP-AA-DTT-AA-JFK-AA-\n";
    expected += "    BOS-AA-FLL-AA-MIA-AA-SJU\n";
    expected += "      * INTERMEDIATE NON-TICKETED POINTS TRACKED ON ROUTING\n";
    expected += "    MPM   WH  8011 TRF-   4    SABRE  ROUTING VALID\n";
    expected += "    NO ROUTE MAP EXISTS\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testUnticketedPointMessage_AnyPoint()
  {
    RoutingMap* rtgMap = new RoutingMap;

    Routing routing;
    routing.routing() = "0001";
    routing.unticketedPointInd() = TKTPTS_ANY;
    routing.rmaps().push_back(rtgMap);

    RoutingInfo routingInfo;
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    routingInfo.mapInfo() = mapInfoPtr;
    routingInfo.routing() = &routing;

    _diag->flushMsg();
    _diag->displayUnticketedPointInfo(routingInfo);

    std::string str = _diag->str();
    std::string expected = "    VALIDATION APPLIED TO TICKETED/UNTICKETED POINTS\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testUnticketedPointMessage_TktOnly()
  {
    RoutingMap* rtgMap = new RoutingMap;

    Routing routing;
    routing.routing() = "0001";
    routing.unticketedPointInd() = TKTPTS_TKTONLY;
    routing.rmaps().push_back(rtgMap);

    RoutingInfo routingInfo;
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    routingInfo.mapInfo() = mapInfoPtr;
    routingInfo.routing() = &routing;

    _diag->flushMsg();
    _diag->displayUnticketedPointInfo(routingInfo);

    std::string str = _diag->str();
    std::string expected = "    VALIDATION APPLIED TO TICKETED POINTS\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testEntryExitPointInfo_AnyPoint()
  {
    RoutingMap* rtgMap = new RoutingMap;

    Routing routing;
    routing.routing() = "0001";
    routing.entryExitPointInd() = ANYPOINT;
    routing.rmaps().push_back(rtgMap);

    RoutingInfo routingInfo;
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    routingInfo.mapInfo() = mapInfoPtr;
    routingInfo.routing() = &routing;

    _diag->flushMsg();
    _diag->displayEntryExitPointInfo(routingInfo);

    std::string str = _diag->str();
    std::string expected = "    VALIDATION APPLIED TO ENTRY/EXIT/INTERMEDIATE POINTS\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testEntryExitPointInfo_EntryExitOnly()
  {
    RoutingMap* rtgMap = new RoutingMap;

    Routing routing;
    routing.routing() = "0001";
    routing.entryExitPointInd() = ENTRYEXITONLY;
    routing.rmaps().push_back(rtgMap);

    RoutingInfo routingInfo;
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    routingInfo.mapInfo() = mapInfoPtr;
    routingInfo.routing() = &routing;

    _diag->flushMsg();
    _diag->displayEntryExitPointInfo(routingInfo);

    std::string str = _diag->str();
    std::string expected = "    VALIDATION APPLIED TO ENTRY/EXIT POINTS\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testTerminalPointMessage_WithoutTerminalPoints()
  {
    // Build Travel Route
    TravelRoute tvlRoute;
    tvlRoute.terminalPoints() = false;

    RoutingInfo routingInfo;
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    _diag->flushMsg();
    _diag->displayTerminalPointMessage(tvlRoute, routingInfo);

    std::string str = _diag->str();
    std::string expected = "";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testTerminalPointMessage_WithTerminalPoints()
  {
    TravelRoute tvlRoute;
    tvlRoute.terminalPoints() = true;

    RoutingInfo routingInfo;
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    _diag->flushMsg();
    _diag->displayTerminalPointMessage(tvlRoute, routingInfo);

    std::string str = _diag->str();
    std::string expected = "    ROUTING VALIDATION APPLIED TO TERMINAL ON/OFF POINTS ONLY\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Missing City Message
  // * CHI NOT FOUND ON ROUTE MAP
  //------------------------------------------------------------------------------
  void testDisplayMissingCityMessage()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "FSD";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DFW";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "DFW";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MEX";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "MX";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = false;

    // Create MapInfo Validation Info
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = false;
    mapInfo.missingCityIndex() = 1; // DFW
    routingInfo.mapInfo() = mapInfoPtr;

    // Create Route Strings
    RoutingMapStrings rMapStringVec;
    std::string rString = "FSD-CHI/STL-MEX";
    rMapStringVec.push_back(rString);
    RoutingMapStrings* rStringPtr = &rMapStringVec;
    mapInfo.routeStrings() = rStringPtr;

    std::string str = _diag->str();

    _diag->flushMsg();
    _diag->displayMissingCity(tvlRoute.travelRoute(), routingInfo, false);
    str = _diag->str();
    std::string expected;
    expected = "    * CHI-AA-DFW NOT FOUND ON ROUTE MAP\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    mapInfo.missingCarrier() = true;

    _diag->flushMsg();
    _diag->displayMissingCity(tvlRoute.travelRoute(), routingInfo, false);
    str = _diag->str();
    expected = "    * CHI-AA-DFW NOT FOUND ON ROUTE MAP\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Surface Message
  //     SURFACE SECTOR MILEAGE EXEMPT BETWEEN CTY AND CTY
  //------------------------------------------------------------------------------
  void testSurfaceMessage()
  {
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    routingInfo.mileageInfo() = mileageInfoPtr;

    Market market;
    market.first = "BRE";
    market.second = "HAM";
    mileageInfo.surfaceSectorExemptCities().push_back(market);

    std::string str = _diag->str();

    _diag->flushMsg();
    _diag->displayMileageMessages(routingInfo);
    _diag->displayMapMessages(routingInfo);
    str = _diag->str();
    std::string expected;
    expected = "    SURFACE SECTOR MILEAGE EXEMPT BETWEEN BRE AND HAM\n";
    expected += "    MILEAGE VALIDATION PASSED\n";
    expected += "    NO ROUTE MAP EXISTS\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display South Atlantic Message
  //------------------------------------------------------------------------------
  void testSouthAtlanticMessage_OldLogic()
  {
    MockRoutingInfo routingInfo;
    routingInfo.setDataTPMExclusion_OldLogic();

    _diag->displayMileageMessages(routingInfo);

    std::string expected = "    SOUTH ATLANTIC TPM EXCLUSION APPLIED LON-BUE-NYC\n"
                           "    MILEAGE VALIDATION PASSED\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testSouthAtlanticMessage()
  {
    MockRoutingInfo routingInfo;
    routingInfo.setDataTPMExclusion();

    _diag->displayMileageMessages(routingInfo);

    std::string expected = "    TPM EXCLUSION APPLIED LON-BUE-NYC\n"
                           "    RECORD LH / SEQ NO 1      MATCHED - CHECK DIAGNOSTIC 452\n"
                           "    MILEAGE VALIDATION PASSED\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  //------------------------------------------------------------------------------
  // Display Constructed Routing
  //------------------------------------------------------------------------------
  void testDisplayConstructedRouting()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "BOD";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "PAR";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "PAR";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "WAS";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "WAS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "COS";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "UA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.terminalPoints() = false;

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "UA";
    routing.routingTariff() = 15;
    routing.routing() = "0085";
    routing.effDateStr() = " ";
    routing.linkNo() = 0;
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 0;
    routing.nooftexts() = 0;
    routing.validityInd() = ' ';
    routing.inhibit() = ' ';
    routing.directionalInd() = ' ';
    routing.domRtgvalInd() = ' ';
    routing.commonPointInd() = ' ';
    routing.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    Routing addOn1;
    addOn1.vendor() = "ATP";
    addOn1.carrier() = "UA";
    addOn1.routingTariff() = 1234;
    addOn1.routing() = "0017";
    addOn1.effDateStr() = " ";
    addOn1.linkNo() = 0;
    addOn1.noofheaders() = 1;
    addOn1.noofRestrictions() = 0;
    addOn1.nooftexts() = 0;
    addOn1.validityInd() = ' ';
    addOn1.inhibit() = ' ';
    addOn1.directionalInd() = ' ';
    addOn1.domRtgvalInd() = ' ';
    addOn1.commonPointInd() = ' ';
    addOn1.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    Routing addOn2;
    addOn2.vendor() = "ATP";
    addOn2.carrier() = "UA";
    addOn2.routingTariff() = 7890;
    addOn2.routing() = "0031";
    addOn2.effDateStr() = " ";
    addOn2.linkNo() = 0;
    addOn2.noofheaders() = 1;
    addOn2.noofRestrictions() = 0;
    addOn2.nooftexts() = 0;
    addOn2.validityInd() = ' ';
    addOn2.inhibit() = ' ';
    addOn2.directionalInd() = ' ';
    addOn2.domRtgvalInd() = ' ';
    addOn2.commonPointInd() = ' ';
    addOn2.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;
    Routing* rtgPtr1 = &addOn1;
    routingInfo.origAddOnRouting() = rtgPtr1;
    Routing* rtgPtr2 = &addOn2;
    routingInfo.destAddOnRouting() = rtgPtr2;
    routingInfo.tcrRoutingTariffCode() = "SABRE";
    routingInfo.tcrAddonTariff1Code() = "SABRE";
    routingInfo.tcrAddonTariff2Code() = "LEANN";
    routingInfo.origAddOnGateway() = "BOD";
    routingInfo.origAddOnInterior() = "PAR";
    routingInfo.market1() = "PAR";
    routingInfo.market2() = "WAS";
    routingInfo.destAddOnGateway() = "WAS";
    routingInfo.destAddOnInterior() = "COS";

    // Build a map of RoutingInfos
    typedef std::map<Routing*, RoutingInfo> RoutingInfos;
    RoutingInfos routingInfos;
    routingInfos.insert(std::map<Routing*, RoutingInfo>::value_type(&routing, routingInfo));

    // Create MapInfo
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    _diag->displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    _diag->displayRoutingStatus(tvlRoute, routingInfo);

    std::string str = _diag->str();

    std::string expected;
    expected += "UA  BOD-UA-PAR-UA-WAS-UA-COS\n";
    expected += "    CONSTRUCTED ROUTING               ROUTING VALID\n";
    expected += "      PAR-BOD ORIGIN ROUTING ADDON\n";
    expected += "        ATP     0017 TRF-1234 SABRE  \n";
    expected += "      PAR-WAS SPECIFIED BASE ROUTING\n";
    expected += "        ATP AT  0085 TRF-15   SABRE  \n";
    expected += "        ROUTE MAP DOES NOT EXIST\n";
    expected += "      WAS-COS DESTINATION ROUTING ADDON\n";
    expected += "        ATP     0031 TRF-7890 LEANN  \n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Highest Oneway Fare for DRV
  //------------------------------------------------------------------------------
  void testDisplayFare()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "BOD";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "PAR";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "PAR";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "WAS";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "WAS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "COS";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "UA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.doNotApplyDRV() = false;

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "UA";
    routing.routingTariff() = 15;
    routing.routing() = "0085";
    routing.effDateStr() = " ";
    routing.linkNo() = 0;
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 0;
    routing.nooftexts() = 0;
    routing.validityInd() = ' ';
    routing.inhibit() = ' ';
    routing.directionalInd() = ' ';
    routing.domRtgvalInd() = ' ';
    routing.commonPointInd() = 'Y';
    routing.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;
    routingInfo.tcrRoutingTariffCode() = "SABRE";

    // Create MapInfo Validation Info
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;
    routingInfo.mapInfo()->drvStatus() = true;

    // Build a map of RoutingInfos
    typedef std::map<Routing*, RoutingInfo> RoutingInfos;
    RoutingInfos routingInfos;
    routingInfos.insert(std::map<Routing*, RoutingInfo>::value_type(&routing, routingInfo));

    // Build a DRVInfo
    DRVInfo drvInfo;
    drvInfo.drvInfoStatus() = true;
    drvInfo.fareClass() = "YH30";
    const_cast<CurrencyCode&>(drvInfo.currency()) = std::string("USD");
    const_cast<MoneyAmount&>(drvInfo.fareAmount()) = 763;
    drvInfo.vendor() = "ATP";
    strToGlobalDirection(drvInfo.global(), "WH");
    drvInfo.routingNumber() = "0095";
    drvInfo.routingTariff1() = 15;
    drvInfo.tariffCode1() = "AURG";
    drvInfo.routingStatus() = true;
    DRVInfo* drvInfoPtr = &drvInfo;

    // Create MapInfo within DRVInfo
    MapInfo drvMapInfo;
    MapInfo* drvMapInfoPtr = &drvMapInfo;
    drvMapInfo.processed() = true;
    drvMapInfo.valid() = true;
    drvInfo.mapInfo() = drvMapInfoPtr;

    // Build DRVInfos
    DRVInfos drvInfos;
    drvInfos.push_back(drvInfoPtr);
    routingInfo.mapInfo()->drvInfos() = drvInfos;

    _diag->displayDRVInfos(tvlRoute, routingInfo);

    std::string str = _diag->str();

    std::string expected;
    expected = "    DRV INDICATOR - PERFORM DOMESTIC ROUTE VALIDATION\n";
    expected += "    HIGHEST FARE SELECTED FOR DRV\n";
    expected += "    YH30               763.00 USD\n";
    expected += "    ATP  WH   0095  TRF-15    AURG    LOCAL ROUTING VALID\n";
    expected += "    ROUTE MAP EXISTS - MAP VALIDATION PASSED\n";

    // Handy Bit of Code to find space discrepancy
    /*std::cout << "Length of expected = " << expected.size() << std::endl;
     std::cout << "Length of actual  = " << str.size() << std::endl;

     for( int i = 0; i < str.size(); i++ )
     {
     std::cout << str.substr(i,1);
     if( str.substr(i,1) != expected.substr(i,1) )
     {
     std::cout << "XX Difference detected" << std::endl;
     break;
     }
     }*/

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display vector of CityCarriers
  //------------------------------------------------------------------------------
  void testDisplayCityCarrierVec()
  {
    // Create a cityCarrierVec
    std::vector<TravelRoute::CityCarrier> cityCarrierVec;

    TravelRoute::CityCarrier cityCarrier1;
    cityCarrier1.carrier() = "AA";
    cityCarrier1.boardCity().loc() = "MSP";
    cityCarrier1.boardNation() = "US";
    cityCarrier1.boardCity().isHiddenCity() = false;
    cityCarrier1.offCity().loc() = "CHI";
    cityCarrier1.offNation() = "US";
    cityCarrier1.offCity().isHiddenCity() = false;
    cityCarrierVec.push_back(cityCarrier1);

    TravelRoute::CityCarrier cityCarrier2;
    cityCarrier2.carrier() = "AA";
    cityCarrier2.boardCity().loc() = "CHI";
    cityCarrier2.boardNation() = "US";
    cityCarrier2.boardCity().isHiddenCity() = false;
    cityCarrier2.offCity().loc() = "NYC";
    cityCarrier2.offNation() = "US";
    cityCarrier2.offCity().isHiddenCity() = false;
    cityCarrierVec.push_back(cityCarrier2);

    TravelRoute::CityCarrier cityCarrier3;
    cityCarrier3.carrier() = "AA";
    cityCarrier3.boardCity().loc() = "NYC";
    cityCarrier3.boardNation() = "US";
    cityCarrier3.boardCity().isHiddenCity() = false;
    cityCarrier3.offCity().loc() = "ALB";
    cityCarrier3.offNation() = "US";
    cityCarrier3.offCity().isHiddenCity() = false;
    cityCarrierVec.push_back(cityCarrier3);

    // Populate the Governing Carrier
    CarrierCode carrier = "AA";

    _diag->displayCityCarriers(carrier, cityCarrierVec);

    std::string str = _diag->str();
    std::string expected;
    expected = "AA  MSP-AA-CHI-AA-NYC-AA-ALB\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Mileage Surcharge Amount
  //------------------------------------------------------------------------------
  void testDisplayMileageSurcharge()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "MUC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "PAR";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "LH";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "LH";
    strToGlobalDirection(tvlRoute.globalDir(), "EH");
    tvlRoute.origin() = "MUC";
    tvlRoute.originNation() = "DE";
    tvlRoute.destination() = "PAR";
    tvlRoute.destinationNation() = "FR";
    tvlRoute.travelDate() = DateTime::localTime();

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "LH";
    routing.routingTariff() = 4;
    routing.routing() = "0000";
    routing.effDateStr() = " ";
    routing.linkNo() = 0;
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 0;
    routing.nooftexts() = 0;
    routing.validityInd() = ' ';
    routing.inhibit() = ' ';
    routing.directionalInd() = ' ';
    routing.domRtgvalInd() = ' ';
    routing.commonPointInd() = ' ';
    routing.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    // Build routingInfo
    RoutingInfo routingInfo;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;
    routingInfo.mapInfo() = 0;

    // Create MileageInfo Validation Info with a vector of Surface Sector Markets
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    routingInfo.mileageInfo() = mileageInfoPtr;
    routingInfo.mileageInfo()->totalApplicableMPM() = 8555;
    routingInfo.mileageInfo()->totalApplicableTPM() = 5600;
    routingInfo.mileageInfo()->surchargeAmt() = 5;
    routingInfo.routingTariff() = 4;

    _diag->displayRoutingStatus(tvlRoute, routingInfo);

    std::string str = _diag->str();
    std::string expected;
    expected += "    MPM   EH  8555 TRF-   4           ROUTING VALID\n";
    expected += "    MILEAGE SURCHARGE APPLIES - 5M TOTAL TPM 5600\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Mileage Surcharge Amount for Constructed Routing
  //------------------------------------------------------------------------------
  void testDisplayMileageSurchargeB()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "MUC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "PAR";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "LH";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "LH";
    strToGlobalDirection(tvlRoute.globalDir(), "EH");
    tvlRoute.origin() = "MUC";
    tvlRoute.originNation() = "DE";
    tvlRoute.destination() = "PAR";
    tvlRoute.destinationNation() = "FR";
    tvlRoute.travelDate() = DateTime::localTime();
    tvlRoute.terminalPoints() = false;

    // Build Addon 1 Travel Route
    TravelRoute addonTvlRoute;
    boardCty.loc() = "MUC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "PAR";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "LH";
    cityCarrier.offCity() = offCty;
    addonTvlRoute.travelRoute().push_back(cityCarrier);

    addonTvlRoute.origin() = "MUC";
    addonTvlRoute.originNation() = "DE";
    addonTvlRoute.destination() = "PAR";
    addonTvlRoute.destinationNation() = "FR";
    addonTvlRoute.travelDate() = DateTime::localTime();

    // Build Addon 2 Travel Route
    TravelRoute addon2TvlRoute;
    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "STL";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    addon2TvlRoute.travelRoute().push_back(cityCarrier);

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "LH";
    routing.routingTariff() = 4;
    routing.routing() = "0000";
    routing.effDateStr() = " ";
    routing.linkNo() = 0;
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 0;
    routing.nooftexts() = 0;
    routing.validityInd() = ' ';
    routing.inhibit() = ' ';
    routing.directionalInd() = ' ';
    routing.domRtgvalInd() = ' ';
    routing.commonPointInd() = ' ';
    routing.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    Routing addOn1;
    addOn1.vendor() = "ATP";
    addOn1.carrier() = "LH";
    addOn1.routingTariff() = 1234;
    addOn1.routing() = "0017";
    addOn1.effDateStr() = " ";
    addOn1.linkNo() = 0;
    addOn1.noofheaders() = 1;
    addOn1.noofRestrictions() = 0;
    addOn1.nooftexts() = 0;
    addOn1.validityInd() = ' ';
    addOn1.inhibit() = ' ';
    addOn1.directionalInd() = ' ';
    addOn1.domRtgvalInd() = ' ';
    addOn1.commonPointInd() = ' ';
    addOn1.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    Routing addOn2;
    addOn2.vendor() = "ATP";
    addOn2.carrier() = "LH";
    addOn2.routingTariff() = 7890;
    addOn2.routing() = "0031";
    addOn2.effDateStr() = " ";
    addOn2.linkNo() = 0;
    addOn2.noofheaders() = 1;
    addOn2.noofRestrictions() = 0;
    addOn2.nooftexts() = 0;
    addOn2.validityInd() = ' ';
    addOn2.inhibit() = ' ';
    addOn2.directionalInd() = ' ';
    addOn2.domRtgvalInd() = ' ';
    addOn2.commonPointInd() = ' ';
    addOn2.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;
    Routing* rtgPtr1 = &addOn1;
    routingInfo.origAddOnRouting() = rtgPtr1;
    Routing* rtgPtr2 = &addOn2;
    routingInfo.destAddOnRouting() = rtgPtr2;
    routingInfo.tcrRoutingTariffCode() = "SABRE";
    routingInfo.tcrAddonTariff1Code() = "SABRE";
    routingInfo.tcrAddonTariff2Code() = "SABRE";
    routingInfo.origAddOnGateway() = "BOD";
    routingInfo.origAddOnInterior() = "PAR";
    routingInfo.market1() = "PAR";
    routingInfo.market2() = "WAS";
    routingInfo.destAddOnGateway() = "WAS";
    routingInfo.destAddOnInterior() = "COS";
    routingInfo.rtgAddon1CityCarrier() = addonTvlRoute.travelRoute();
    routingInfo.rtgAddon2CityCarrier() = addon2TvlRoute.travelRoute();

    // Create MapInfo
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    MapInfo rtgAddonMapInfo;
    MapInfo* rtgAddonMapInfoPtr = &rtgAddonMapInfo;
    rtgAddonMapInfo.processed() = true;
    rtgAddonMapInfo.valid() = true;
    routingInfo.rtgAddonMapInfo() = rtgAddonMapInfoPtr;

    // Create MileageInfo Validation Info with a vector of Surface Sector Markets
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    routingInfo.mileageInfo() = mileageInfoPtr;
    routingInfo.mileageInfo()->totalApplicableMPM() = 8555;
    routingInfo.mileageInfo()->totalApplicableTPM() = 5600;
    routingInfo.mileageInfo()->surchargeAmt() = 5;

    _diag->displayRoutingStatus(tvlRoute, routingInfo);

    std::string str = _diag->str();
    std::string expected;
    expected = "    CONSTRUCTED ROUTING               ROUTING VALID\n";
    expected += "    MPM EH  8555 APPLIES TO ENTIRE ROUTE OF TRAVEL\n";
    expected += "    MILEAGE SURCHARGE APPLIES - 5M TOTAL TPM 5600\n";
    expected += "      PAR-BOD ORIGIN ROUTING ADDON\n";
    expected += "        ATP     0017 TRF-1234 SABRE  \n";
    expected += "      PAR-WAS SPECIFIED BASE MILEAGE\n";
    expected += "        ATP EH  0000 TRF-4    SABRE  \n";
    expected += "      CHI-AA-STL\n";
    expected += "      WAS-COS DESTINATION ROUTING ADDON\n";
    expected += "        ATP     0031 TRF-7890 SABRE  \n";

    /*std::cout << "Length of expected = " << expected.size() << std::endl;
     std::cout << "Length of actual  = " << str.size() << std::endl;

     for( int i = 0; i < str.size(); i++ )
     {
     std::cout << str.substr(i,1);
     if( str.substr(i,1) != expected.substr(i,1) )
     {
     std::cout << "XX Difference detected" << std::endl;
     break;
     }
     }*/

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Mileage Surcharge Amount
  //------------------------------------------------------------------------------
  void testDisplayMileageEqualization()
  {
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "LH";
    routing.routingTariff() = 4;
    routing.routing() = "0000";
    routing.effDateStr() = " ";
    routing.linkNo() = 0;
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 0;
    routing.nooftexts() = 0;
    routing.validityInd() = ' ';
    routing.inhibit() = ' ';
    routing.directionalInd() = ' ';
    routing.domRtgvalInd() = ' ';
    routing.commonPointInd() = ' ';
    routing.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    // Build routingInfo
    RoutingInfo routingInfo;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;
    routingInfo.mapInfo() = 0;

    // Create MileageInfo Validation Info with a vector of Surface Sector Markets
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    mileageInfo.mileageEqualizationApplies() = true;
    mileageInfo.equalizationSurcharges().first = 15;
    mileageInfo.equalizationSurcharges().second = 10;
    routingInfo.mileageInfo() = mileageInfoPtr;

    _diag->displayMileageMessages(routingInfo);
    std::string str = _diag->str();
    std::string expected;
    expected = "    MILEAGE EQUALIZATION APPLIED. SURCHG REDUCED FROM 15 TO 10\n";
    expected += "    MILEAGE VALIDATION PASSED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Mileage Surcharge Amount
  //------------------------------------------------------------------------------
  void testDisplayMileageException()
  {
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "LH";
    routing.routingTariff() = 4;
    routing.routing() = "0000";
    routing.effDateStr() = " ";
    routing.linkNo() = 0;
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 0;
    routing.nooftexts() = 0;
    routing.validityInd() = ' ';
    routing.inhibit() = ' ';
    routing.directionalInd() = ' ';
    routing.domRtgvalInd() = ' ';
    routing.commonPointInd() = ' ';
    routing.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    // Build routingInfo
    RoutingInfo routingInfo;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;
    routingInfo.mapInfo() = 0;

    // Create MileageInfo Validation Info with a vector of Surface Sector Markets
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    mileageInfo.surchargeAmtSAException() = 5;
    routingInfo.mileageInfo() = mileageInfoPtr;

    _diag->displayMileageMessages(routingInfo);

    std::string str = _diag->str();
    std::string expected;
    expected += "    MILEAGE VALIDATION PASSED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Ticketed Point Deductions
  //------------------------------------------------------------------------------
  void testDisplayTPDCities()
  {
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "LH";
    routing.routingTariff() = 4;
    routing.routing() = "0000";
    routing.effDateStr() = " ";
    routing.linkNo() = 0;
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 0;
    routing.nooftexts() = 0;
    routing.validityInd() = ' ';
    routing.inhibit() = ' ';
    routing.directionalInd() = ' ';
    routing.domRtgvalInd() = ' ';
    routing.commonPointInd() = ' ';
    routing.jointRoutingOpt() = ' ';
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    // Build routingInfo
    RoutingInfo routingInfo;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;
    routingInfo.mapInfo() = 0;

    // Create MileageInfo Validation Info with a vector of Surface Sector Markets
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    routingInfo.mileageInfo() = mileageInfoPtr;
    routingInfo.mileageInfo()->totalApplicableMPM() = 8555;
    routingInfo.mileageInfo()->surchargeAmt() = 5;
    routingInfo.mileageInfo()->tpd() = 1000;

    _diag->displayMileageMessages(routingInfo);
    _diag->displayMapMessages(routingInfo);

    std::string str = _diag->str();
    std::string expected;
    expected = "    TICKETED POINT DEDUCTION 1000 APPLIED\n";
    expected += "    MILEAGE VALIDATION PASSED\n";
    expected += "    NO ROUTE MAP EXISTS\n";

    /*std::cout << "Length of expected = " << expected.size() << std::endl;
     std::cout << "Length of actual  = " << str.size() << std::endl;

     for( int i = 0; i < str.size(); i++ )
     {
     std::cout << str.substr(i,1);
     if( str.substr(i,1) != expected.substr(i,1) )
     {
     std::cout << "XX Difference detected" << std::endl;
     break;
     }
     }*/

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display PermissibleSpecifiedRoutings
  //------------------------------------------------------------------------------
  void testDisplayPSRs()
  {
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    mileageInfo.psrApplies() = true;
    routingInfo.mileageInfo() = mileageInfoPtr;

    // Build a vector of viaGeoLocItems in MileageInfo
    TpdPsrViaGeoLoc psrGeoLoc1;
    psrGeoLoc1.setNo() = 1;
    psrGeoLoc1.orderNo() = 1;
    psrGeoLoc1.loc().locType() = 'C';
    psrGeoLoc1.loc().loc() = "CHI";
    psrGeoLoc1.relationalInd() = ' ';
    psrGeoLoc1.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc1);

    TpdPsrViaGeoLoc psrGeoLoc2;
    psrGeoLoc2.setNo() = 1;
    psrGeoLoc2.orderNo() = 2;
    psrGeoLoc2.loc().locType() = 'C';
    psrGeoLoc2.loc().loc() = "BNA";
    psrGeoLoc2.relationalInd() = VIAGEOLOCREL_OR;
    psrGeoLoc2.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc2);

    TpdPsrViaGeoLoc psrGeoLoc3;
    psrGeoLoc3.setNo() = 1;
    psrGeoLoc3.orderNo() = 3;
    psrGeoLoc3.loc().locType() = 'C';
    psrGeoLoc3.loc().loc() = "DFW";
    psrGeoLoc3.relationalInd() = VIAGEOLOCREL_OR;
    psrGeoLoc3.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc3);

    TpdPsrViaGeoLoc psrGeoLoc4;
    psrGeoLoc4.setNo() = 1;
    psrGeoLoc4.orderNo() = 4;
    psrGeoLoc4.loc().locType() = 'C';
    psrGeoLoc4.loc().loc() = "MIA";
    psrGeoLoc4.relationalInd() = VIAGEOLOCREL_AND;
    psrGeoLoc4.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc4);

    TpdPsrViaGeoLoc psrGeoLoc5;
    psrGeoLoc5.setNo() = 1;
    psrGeoLoc5.orderNo() = 4;
    psrGeoLoc5.loc().locType() = 'C';
    psrGeoLoc5.loc().loc() = "MSP";
    psrGeoLoc5.relationalInd() = VIAGEOLOCREL_AND;
    psrGeoLoc5.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc5);

    mileageInfo.psrGovCxr() = "LH";

    /*int _setNo;
     int _orderNo;
     LocKey _loc;
     Indicator _relationalInd;
     Indicator _stopoverNotAllowed;

     const Indicator VIAGEOLOCREL_AND = '&';
     const Indicator VIAGEOLOCREL_OR = '/';
     const Indicator VIAGEOLOCREL_ANDOR = '-';*/

    _diag->displayPSRs(routingInfo);

    std::string str = _diag->str();
    std::string expected;
    expected = "    LH PERMISSIBLE SPECIFIED ROUTING APPLIES\n";
    expected += "    CHI OR BNA OR DFW AND MIA AND MSP\n";
    expected += "    NO MPM CHECK DUE TO PERMISSIBLE SPECIFIED ROUTING\n";

    /*std::cout << "Length of expected = " << expected.size() << std::endl;
     std::cout << "Length of actual  = " << str.size() << std::endl;

     for( int i = 0; i < str.size(); i++ )
     {
     std::cout << str.substr(i,1);
     if( str.substr(i,1) != expected.substr(i,1) )
     {
     std::cout << "XX Difference detected" << std::endl;
     break;
     }
     }*/

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display PermissibleSpecifiedRoutings
  //------------------------------------------------------------------------------
  void testDisplayPSRZone()
  {
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    mileageInfo.psrApplies() = true;
    routingInfo.mileageInfo() = mileageInfoPtr;

    // Build a vector of viaGeoLocItems in MileageInfo
    TpdPsrViaGeoLoc psrGeoLoc1;
    psrGeoLoc1.setNo() = 1;
    psrGeoLoc1.orderNo() = 1;
    psrGeoLoc1.loc().locType() = 'Z';
    psrGeoLoc1.loc().loc() = "1734";
    psrGeoLoc1.relationalInd() = ' ';
    psrGeoLoc1.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc1);

    TpdPsrViaGeoLoc psrGeoLoc2;
    psrGeoLoc2.setNo() = 1;
    psrGeoLoc2.orderNo() = 2;
    psrGeoLoc2.loc().locType() = 'Z';
    psrGeoLoc2.loc().loc() = "2345";
    psrGeoLoc2.relationalInd() = VIAGEOLOCREL_OR;
    psrGeoLoc2.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc2);

    mileageInfo.psrGovCxr() = "LH";

    /*int _setNo;
     int _orderNo;
     LocKey _loc;
     Indicator _relationalInd;
     Indicator _stopoverNotAllowed;

     const Indicator VIAGEOLOCREL_AND = '&';
     const Indicator VIAGEOLOCREL_OR = '/';
     const Indicator VIAGEOLOCREL_ANDOR = '-';*/

    _diag->displayPSRs(routingInfo);

    std::string str = _diag->str();
    std::string expected;
    expected = "    LH PERMISSIBLE SPECIFIED ROUTING APPLIES\n";
    expected += "    ZONE 1734 OR ZONE 2345\n";
    expected += "    NO MPM CHECK DUE TO PERMISSIBLE SPECIFIED ROUTING\n";

    /*std::cout << "Length of expected = " << expected.size() << std::endl;
     std::cout << "Length of actual  = " << str.size() << std::endl;

     for( int i = 0; i < str.size(); i++ )
     {
     std::cout << str.substr(i,1);
     if( str.substr(i,1) != expected.substr(i,1) )
     {
     std::cout << "XX Difference detected" << std::endl;
     break;
     }
     }*/

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display PermissibleSpecifiedRoutings
  //------------------------------------------------------------------------------
  void testDisplayPSRWrapDisplay()
  {
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    mileageInfo.psrApplies() = true;
    routingInfo.mileageInfo() = mileageInfoPtr;

    // Build a vector of viaGeoLocItems in MileageInfo
    TpdPsrViaGeoLoc psrGeoLoc1;
    psrGeoLoc1.setNo() = 1;
    psrGeoLoc1.orderNo() = 1;
    psrGeoLoc1.loc().locType() = 'S';
    psrGeoLoc1.loc().loc() = "MN";
    psrGeoLoc1.relationalInd() = ' ';
    psrGeoLoc1.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc1);

    TpdPsrViaGeoLoc psrGeoLoc2;
    psrGeoLoc2.setNo() = 1;
    psrGeoLoc2.orderNo() = 2;
    psrGeoLoc2.loc().locType() = 'S';
    psrGeoLoc2.loc().loc() = "TX";
    psrGeoLoc2.relationalInd() = VIAGEOLOCREL_ANDOR;
    psrGeoLoc2.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc2);

    TpdPsrViaGeoLoc psrGeoLoc3;
    psrGeoLoc3.setNo() = 1;
    psrGeoLoc3.orderNo() = 2;
    psrGeoLoc3.loc().locType() = 'S';
    psrGeoLoc3.loc().loc() = "OK";
    psrGeoLoc3.relationalInd() = VIAGEOLOCREL_ANDOR;
    psrGeoLoc3.stopoverNotAllowed() = NO;
    mileageInfo.psrGeoLocs().push_back(psrGeoLoc3);

    mileageInfo.psrGovCxr() = "LH";

    /*int _setNo;
     int _orderNo;
     LocKey _loc;
     Indicator _relationalInd;
     Indicator _stopoverNotAllowed;

     const Indicator VIAGEOLOCREL_AND = '&';
     const Indicator VIAGEOLOCREL_OR = '/';
     const Indicator VIAGEOLOCREL_ANDOR = '-';*/

    _diag->displayPSRs(routingInfo);

    std::string str = _diag->str();
    std::string expected;
    expected = "    LH PERMISSIBLE SPECIFIED ROUTING APPLIES\n";
    expected += "    STATE/PROVINCE MN AND/OR STATE/PROVINCE TX AND/OR \n";
    expected += "    STATE/PROVINCE OK\n";
    expected += "    NO MPM CHECK DUE TO PERMISSIBLE SPECIFIED ROUTING\n";

    /*std::cout << "Length of expected = " << expected.size() << std::endl;
     std::cout << "Length of actual  = " << str.size() << std::endl;

     for( int i = 0; i < str.size(); i++ )
     {
     std::cout << str.substr(i,1);
     if( str.substr(i,1) != expected.substr(i,1) )
     {
     std::cout << "XX Difference detected" << std::endl;
     break;
     }
     }*/

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Restriction 1
  //------------------------------------------------------------------------------
  void testDisplayRestriction1()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "1";
    _rest->marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "MIA"; // City2
    _rest->viaMarket() = "CHI"; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 1                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST BE VIA CHI\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 1                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 1                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST NOT BE VIA CHI\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 1                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction2()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "2";
    _rest->marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->market1() = "   "; // City1
    _rest->market2() = "   "; // City2
    _rest->viaMarket() = "CHI"; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    _restInfo->valid() = false;

    std::string expected;

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 2                    RESTRICTION FAILED\n";
    expected += "    TRAVEL MUST BE VIA CHI\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 2                    RESTRICTION FAILED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _restInfo->valid() = true;

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 2                    RESTRICTION PASSED\n";
    expected += "    TRAVEL MUST NOT BE VIA CHI\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 2                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction3()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "3";
    _rest->marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->market1() = "   "; // City1
    _rest->market2() = "   "; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    TRAVEL MUST BE NONSTOP\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    TRAVEL MUST NOT BE NONSTOP\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    TRAVEL MUST BE DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    TRAVEL MUST NOT BE DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'E'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'E'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    TRAVEL MUST BE NONSTOP OR DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'E'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    TRAVEL MUST NOT BE NONSTOP OR DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 3                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction4()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "4";
    _rest->marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "MIA"; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST BE NONSTOP\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST NOT BE NONSTOP\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST BE DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST NOT BE DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'E'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'E'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST BE NONSTOP OR DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'E'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST NOT BE NONSTOP OR DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 4                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction5()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "5";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->marketAppl() = 'T'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "   "; // City2
    _rest->viaMarket() = "CHI"; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 5                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 5                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST BE VIA CHI\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 5                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST NOT BE VIA CHI\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 5                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction6()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "6";
    _rest->marketAppl() = 'T'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "   "; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST BE NONSTOP\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST NOT BE NONSTOP\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST BE DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'D'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST NOT BE DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'E'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'E'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST BE NONSTOP OR DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->nonStopDirectInd() = 'E'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST NOT BE NONSTOP OR DIRECT\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 6                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction7()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "7";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "MIA"; // City2
    _rest->viaMarket() = "CHI"; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 7                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA STOPOVER IN CHI PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 7                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA STOPOVER IN CHI REQUIRED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 7                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA STOPOVER IN CHI NOT PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 7                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  PricingTrx* createRtwTrx()
  {
    PricingTrx* trx = _memH.create<PricingTrx>();
    PricingOptions* opt = _memH.create<PricingOptions>();

    opt->setRtw(true);
    trx->setOptions(opt);

    return trx;
  }

  void testDisplayRestriction8_NotRtw()
  {
    _rest->restriction() = "8";
    _diag->displayRestriction(*_rest, *_restInfo);

    std::string expected;
    expected += "    RESTRICTION 8                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - NON-RTW/CT\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRestriction8()
  {
    _rest->restriction() = "8";
    _diag->trx() = createRtwTrx();
    _diag->displayRestriction(*_rest, *_restInfo);

    std::string expected;
    expected += "    RESTRICTION 8                    RESTRICTION PASSED\n";
    expected += "    MPM: 1000\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRestriction9_NotRtw()
  {
    _rest->restriction() = "9";
    _diag->displayRestriction(*_rest, *_restInfo);

    std::string expected;
    expected += "    RESTRICTION 9                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - NON-RTW/CT\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRestriction9()
  {
    _rest->restriction() = "9";
    _diag->trx() = createRtwTrx();
    _diag->displayRestriction(*_rest, *_restInfo);

    std::string expected;
    expected += "    RESTRICTION 9                    RESTRICTION PASSED\n";
    expected += "    TRAVEL IS NOT PERMITTED VIA THE FARE ORIGIN\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRestriction10()
  {
    _rest->restriction() = "10";
    _diag->trx() = createRtwTrx();
    _diag->displayRestriction(*_rest, *_restInfo);

    std::string expected;
    expected += "    RESTRICTION 10                    RESTRICTION PASSED\n";
    expected += "    TRAVEL CANNOT CONTAIN MORE THAN ONE COUPON BETWEEN\n";
    expected += "    THE SAME POINTS IN THE SAME DIRECTION\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRestriction10_NotRtw()
  {
    _rest->restriction() = "10";
    _diag->displayRestriction(*_rest, *_restInfo);

    std::string expected;
    expected += "    RESTRICTION 10                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - NON-RTW/CT\n";

    CPPUNIT_ASSERT_EQUAL(expected, _diag->str());
  }

  void testDisplayRestriction11()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "11";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "MIA"; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;
    _rest->airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA AIR SECTOR PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA AIR SECTOR REQUIRED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA AIR SECTOR NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA SURFACE SECTOR PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA SURFACE SECTOR REQUIRED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA SURFACE SECTOR NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'E'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA AIR OR SURFACE SECTOR PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'E'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA AIR OR SURFACE SECTOR REQUIRED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'E'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA AIR OR SURFACE SECTOR NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction12()
  {
    FlatSet<std::pair<Indicator, LocCode>> locs;
    locs.unsafe_insert(std::make_pair(' ', "AAA"));
    locs.unsafe_insert(std::make_pair(' ', "BBB"));
    locs.unsafe_insert(std::make_pair(' ', "CCC"));

    std::string expected;
    _diag->trx() = createRtwTrx();
    _diag->displayRestriction12(locs, locs);
    std::string str = _diag->str();
    expected += "    ONLY ONE INSTANCE OF NONSTOP SINGLE COUPON TRAVEL \n";
    expected += "    IS PERMITTED BETWEEN AAA/BBB/CCC AND AAA/BBB/CCC\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction12_Split()
  {
    FlatSet<std::pair<Indicator, LocCode>> locs;
    locs.unsafe_insert(std::make_pair(' ', "AAA"));
    locs.unsafe_insert(std::make_pair(' ', "BBB"));
    locs.unsafe_insert(std::make_pair(' ', "CCC"));
    locs.unsafe_insert(std::make_pair(' ', "DDD"));
    locs.unsafe_insert(std::make_pair(' ', "EEE"));
    locs.unsafe_insert(std::make_pair(' ', "FFF"));

    std::string expected;
    _diag->trx() = createRtwTrx();
    _diag->displayRestriction12(locs, locs);
    std::string str = _diag->str();
    expected += "    ONLY ONE INSTANCE OF NONSTOP SINGLE COUPON TRAVEL \n";
    expected += "    IS PERMITTED BETWEEN AAA/BBB/CCC/DDD/EEE/FFF AND AAA/BBB/\n";
    expected += "    CCC/DDD/EEE/FFF\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction13()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "13";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "MIA"; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _rest->airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 13                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA AIR SECTOR PERMITTED AT ADDL EXP\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 13                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS REQUIRED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 13                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR - NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 13                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA SURFACE SECTOR PERMITTED AT ADDL EXP\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 13                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS REQUIRED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 13                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR - NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'E'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 13                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA AIR OR SURFACE PERMITTED AT ADDL EXP\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'E'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 13                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS REQUIRED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->airSurfaceInd() = 'E'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 13                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR - NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction14()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "14";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "MIA"; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 14                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA NO LOCAL TRAFFIC PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction15()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "15";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->market1() = "   "; // City1
    _rest->market2() = "   "; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 15                    RESTRICTION PASSED\n";
    expected += "    BAGGAGE MUST BE CHECKED THROUGH TO DESTINATION ONLY\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction16()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "16";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->market1() = "   "; // City1
    _rest->market2() = "   "; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "   "; // Via Carrier

    std::string expected;

    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 16                    RESTRICTION PASSED\n";
    expected += "    MAXIMUM PERMITTED MILEAGE APPLIES\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction17()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "17";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->market1() = "   "; // City1
    _rest->market2() = "   "; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "TG"; // Via Carrier

    std::string expected;

    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 17                    RESTRICTION PASSED\n";
    expected += "    CARRIER LISTING ONLY - TG\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction18()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "18";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "MIA"; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "AA"; // Via Carrier

    std::string expected;
    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 18                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 18                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST BE VIA AA\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 18                    RESTRICTION PASSED\n";
    expected += "    TRAVEL BETWEEN MEM AND MIA MUST NOT BE VIA AA\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 18                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction19()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "19";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->marketAppl() = 'T'; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "   "; // City2
    _rest->viaMarket() = "   "; // Via City
    _rest->viaCarrier() = "AA"; // Via Carrier

    std::string expected;
    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 19                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 19                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST BE VIA AA\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 19                    RESTRICTION PASSED\n";
    expected += "    TRAVEL TO/FROM MEM MUST NOT BE VIA AA\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 19                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  void testDisplayRestriction21()
  {
    _rest->restrSeqNo() = 1;
    _rest->restriction() = "21";
    _rest->nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    _rest->airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    _rest->marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    _rest->viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    _rest->market1() = "MEM"; // City1
    _rest->market2() = "MIA"; // City2
    _rest->viaMarket() = "CHI"; // Via City
    _rest->viaCarrier() = "AA"; // Via Carrier

    std::string expected;
    _rest->negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    std::string str = _diag->str();
    expected += "    RESTRICTION 21                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 21                    RESTRICTION PASSED\n";
    expected += "    WHEN ORIGIN IS MEM AND DEST IS MIA TRAVEL MUST BE VIA CHI\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 21                    RESTRICTION PASSED\n";
    expected += "    WHEN ORIGIN IS MEM AND DEST IS MIA TRAVEL MUST NOT BE VIA CHI\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);

    _rest->negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    _diag->flushMsg();
    _diag->displayRestriction(*_rest, *_restInfo);
    str = _diag->str();
    expected = "    RESTRICTION 21                    RESTRICTION PASSED\n";
    expected += "    RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

}; // class

CPPUNIT_TEST_SUITE_REGISTRATION(Diag450CollectorTest);
} // namespace tse
