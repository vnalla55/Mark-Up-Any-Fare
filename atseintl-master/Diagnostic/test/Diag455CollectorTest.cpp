#include "DataModel/PricingTrx.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "Diagnostic/Diag455Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/RoutingInfo.h"
#include "Routing/TravelRoute.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class Diag455CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag455CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testDisplayHeader);
  CPPUNIT_TEST(testDisplayTravelRoute);
  CPPUNIT_TEST(testNoFaresMessage);
  CPPUNIT_TEST(testNoFaresDisplay);
  CPPUNIT_TEST(testBuildRouteMapDisplay1);
  CPPUNIT_TEST(testBuildRouteMapDisplay2);
  CPPUNIT_TEST(testBuildRouteMapDisplay3);
  CPPUNIT_TEST(testBuildRouteMapDisplay4);
  CPPUNIT_TEST(testBuildRouteMapDisplay5);
  CPPUNIT_TEST(testBuildRouteMapDisplay6);
  CPPUNIT_TEST(testBuildRoutingAddonDisplay);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(Diagnostic455);
      Diag455Collector diag(diagroot);

      std::string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(std::string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  //------------------------------------------------------------------
  // Test Empty Display - Header Only
  //------------------------------------------------------------------
  void testDisplayHeader()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    diag.buildHeader();

    std::string str = diag.str();

    std::string expected;
    expected += "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------
  // Test Display Travel Route
  //------------------------------------------------------------------
  void testDisplayTravelRoute()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "BOM";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.travelRoute().push_back(cityCarrier);

    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());

    std::string str = diag.str();

    std::string expected;
    expected = "AA  CHI-AA-BOM\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------
  // Test No Fares Message
  //------------------------------------------------------------------
  void testNoFaresMessage()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    diag.displayNoFaresMessage();

    std::string str = diag.str();

    std::string expected;
    expected = "    NO FARES EXIST IN THIS MARKET\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------
  // Test No Fares Display
  //------------------------------------------------------------------
  void testNoFaresDisplay()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "BOM";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.travelRoute().push_back(cityCarrier);

    RoutingInfos routingInfos;
    RoutingInfos* routingInfosPtr = &routingInfos;

    diag.displayRouteMapAndRestrictions(trx, tvlRoute, routingInfosPtr);

    std::string str = diag.str();
    std::string expected;
    expected = "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";
    expected += "AA  CHI-AA-BOM\n";
    expected += "    NO FARES EXIST IN THIS MARKET\n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //--------------------------------------------------------------------------
  // Test a simple origin/destination route extraction from a small set of
  // route strings. We pick a direct destination with no intermediate points.
  //--------------------------------------------------------------------------
  void testBuildRouteMapDisplay1()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "GRB";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "STL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 15;
    routing.routing() = "0747";
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

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "16";
    rest.marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    rest.negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = ""; // City1
    rest.market2() = ""; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;

    // Build a restrictionInfo
    RestrictionInfo resInfo;
    resInfo.processed() = true;
    resInfo.valid() = true;

    // Build the RestrictionInfos Map and set pointer to it in RoutingInfo
    RestrictionInfos resInfos;
    resInfos.insert(std::map<RoutingRestriction*, RestrictionInfo>::value_type(&rest, resInfo));
    RestrictionInfos* resInfoPtr = &resInfos;
    routingInfo.restrictions() = resInfoPtr;

    // Create MapInfo
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;

    // Create Route Strings
    RoutingMapStrings rMapStringVec;
    std::string rString = "GRB-STL";
    rMapStringVec.push_back(rString);
    RoutingMapStrings* rStringPtr = &rMapStringVec;
    mapInfo.routeStrings() = rStringPtr;

    // Call Validation
    diag.buildHeader();
    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    diag.displayRoutingStatus(tvlRoute, routingInfo);
    diag.displayRestrictions(routingInfo);
    diag.displayMileageMessages(routingInfo);
    diag.displayMapMessages(routingInfo);
    diag.displayHeader2(routingInfo);
    diag.displayRouteStrings(rStringPtr);

    std::string str = diag.str();

    std::string expected;
    expected = "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";
    expected += "AA  GRB-AA-STL\n";
    expected += "    ATP   WH  0747 TRF-  15           ROUTING VALID\n";
    expected += "    RESTRICTION 16                    RESTRICTION PASSED\n";
    expected += "    MAXIMUM PERMITTED MILEAGE APPLIES\n";
    expected += " \n";
    expected += " \n";
    expected += "    ROUTE MAP DISPLAY\n";
    expected += "    ***********************************************************\n";
    expected += "    GRB-STL\n";
    expected += " \n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //--------------------------------------------------------------------------
  // Test a simple origin/destination route extraction from a small set of
  // route strings. We pick a destination with intermediate points.ts.
  //--------------------------------------------------------------------------
  void testBuildRouteMapDisplay2()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "GRB";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DFW";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 7;
    routing.routing() = "1630";
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

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "03";
    rest.marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = 'N'; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = ""; // City1
    rest.market2() = ""; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;

    // Build a restrictionInfo
    RestrictionInfo resInfo;
    resInfo.processed() = true;
    resInfo.valid() = true;

    // Build the RestrictionInfos Map and set pointer to it in RoutingInfo
    RestrictionInfos resInfos;
    resInfos.insert(std::map<RoutingRestriction*, RestrictionInfo>::value_type(&rest, resInfo));
    RestrictionInfos* resInfoPtr = &resInfos;
    routingInfo.restrictions() = resInfoPtr;

    // Create MapInfo
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    // Create Route Strings
    RoutingMapStrings rMapStringVec;
    std::string rString = "GRB-CHI/STL-DFW";
    rMapStringVec.push_back(rString);
    RoutingMapStrings* rStringPtr = &rMapStringVec;
    mapInfo.routeStrings() = rStringPtr;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;

    // Call Validation
    diag.buildHeader();
    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    diag.displayRoutingStatus(tvlRoute, routingInfo);
    diag.displayRestrictions(routingInfo);
    diag.displayMileageMessages(routingInfo);
    diag.displayMapMessages(routingInfo);
    diag.displayHeader2(routingInfo);
    diag.displayRouteStrings(rStringPtr);

    std::string str = diag.str();

    std::string expected;
    expected = "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";
    expected += "AA  GRB-AA-DFW\n";
    expected += "    ATP   WH  1630 TRF-   7           ROUTING VALID\n";
    expected += "    RESTRICTION 03                    RESTRICTION PASSED\n";
    expected += "    TRAVEL MUST BE NONSTOP\n";
    expected += " \n";
    expected += " \n";
    expected += "    ROUTE MAP DISPLAY\n";
    expected += "    ***********************************************************\n";
    expected += "    GRB-CHI/STL-DFW\n";
    expected += " \n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //--------------------------------------------------------------------------
  // Test a origin/intermediate/destination route extraction from a small set of
  // route strings.
  //--------------------------------------------------------------------------
  void testBuildRouteMapDisplay3()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "MEM";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MIA";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 17;
    routing.routing() = "0027";
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

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "11";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'E'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "MEM"; // City1
    rest.market2() = "MIA"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;

    // Build a restrictionInfo
    RestrictionInfo resInfo;
    resInfo.processed() = true;
    resInfo.valid() = true;

    // Build the RestrictionInfos Map and set pointer to it in RoutingInfo
    RestrictionInfos resInfos;
    resInfos.insert(std::map<RoutingRestriction*, RestrictionInfo>::value_type(&rest, resInfo));
    RestrictionInfos* resInfoPtr = &resInfos;
    routingInfo.restrictions() = resInfoPtr;

    // Create MapInfo
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    // Create Route Strings
    RoutingMapStrings rMapStringVec;
    std::string rString = "MEM-CHI/STL-DFW-ORL-MIA";
    rMapStringVec.push_back(rString);
    RoutingMapStrings* rStringPtr = &rMapStringVec;
    mapInfo.routeStrings() = rStringPtr;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;

    // Call Validation
    diag.buildHeader();
    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    diag.displayRoutingStatus(tvlRoute, routingInfo);
    diag.displayRestrictions(routingInfo);
    diag.displayMileageMessages(routingInfo);
    diag.displayMapMessages(routingInfo);
    diag.displayHeader2(routingInfo);
    diag.displayRouteStrings(rStringPtr);

    std::string str = diag.str();

    std::string expected;
    expected = "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";
    expected += "AA  MEM-AA-MIA\n";
    expected += "    ATP   WH  0027 TRF-  17           ROUTING VALID\n";
    expected += "    RESTRICTION 11                    RESTRICTION PASSED\n";
    expected += "    BETWEEN MEM AND MIA AIR OR SURFACE SECTOR NOT PERMITTED\n";
    expected += " \n";
    expected += " \n";
    expected += "    ROUTE MAP DISPLAY\n";
    expected += "    ***********************************************************\n";
    expected += "    MEM-CHI/STL-DFW-ORL-MIA\n";
    expected += " \n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //--------------------------------------------------------------------------
  // Test an international market with different carriers.
  // simple origin/destination route extraction from a small set of
  // route strings. We pick a destination with intermediate points.
  //--------------------------------------------------------------------------
  void testBuildRouteMapDisplay4()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "MAN";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LAX";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 1;
    routing.routing() = "0003";
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

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "16";
    rest.marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    rest.negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = ""; // City1
    rest.market2() = ""; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;

    // Build a restrictionInfo
    RestrictionInfo resInfo;
    resInfo.processed() = true;
    resInfo.valid() = true;

    // Build the RestrictionInfos Map and set pointer to it in RoutingInfo
    RestrictionInfos resInfos;
    resInfos.insert(std::map<RoutingRestriction*, RestrictionInfo>::value_type(&rest, resInfo));
    RestrictionInfos* resInfoPtr = &resInfos;
    routingInfo.restrictions() = resInfoPtr;

    // Create MapInfo
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    // Create Route Strings
    RoutingMapStrings rMapStringVec;
    std::string rString = "MAN-BA/LH-LAX";
    rMapStringVec.push_back(rString);
    RoutingMapStrings* rStringPtr = &rMapStringVec;
    mapInfo.routeStrings() = rStringPtr;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;

    // Call Validation
    diag.buildHeader();
    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    diag.displayRoutingStatus(tvlRoute, routingInfo);
    diag.displayRestrictions(routingInfo);
    diag.displayMileageMessages(routingInfo);
    diag.displayMapMessages(routingInfo);
    diag.displayHeader2(routingInfo);
    diag.displayRouteStrings(rStringPtr);

    std::string str = diag.str();

    std::string expected;
    expected = "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";
    expected += "AA  MAN-AA-LAX\n";
    expected += "    ATP   AT  0003 TRF-   1           ROUTING VALID\n";
    expected += "    RESTRICTION 16                    RESTRICTION PASSED\n";
    expected += "    MAXIMUM PERMITTED MILEAGE APPLIES\n";
    expected += " \n";
    expected += " \n";
    expected += "    ROUTE MAP DISPLAY\n";
    expected += "    ***********************************************************\n";
    expected += "    MAN-BA/LH-LAX\n";
    expected += " \n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  /**
   * Test an international market with length greater than 63.
   **/

  //--------------------------------------------------------------------------
  // Test an international market with different carriers.
  // simple origin/destination route extraction from a small set of
  // route strings. We pick a destination with intermediate points.
  //--------------------------------------------------------------------------
  void testBuildRouteMapDisplay5()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "BOS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MAN";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 6;
    routing.routing() = "1300";
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

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "07";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "BOS"; // City1
    rest.market2() = "LON"; // City2
    rest.viaMarket() = "CHI"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;

    // Build a restrictionInfo
    RestrictionInfo resInfo;
    resInfo.processed() = true;
    resInfo.valid() = true;

    // Build the RestrictionInfos Map and set pointer to it in RoutingInfo
    RestrictionInfos resInfos;
    resInfos.insert(std::map<RoutingRestriction*, RestrictionInfo>::value_type(&rest, resInfo));
    RestrictionInfos* resInfoPtr = &resInfos;
    routingInfo.restrictions() = resInfoPtr;

    // Create MapInfo
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    // Create Route Strings
    RoutingMapStrings rMapStringVec;
    std::string rString = "BOS-CHI/CLT/DFW/DTT/EWR/HOU/LAX/MIA/NYC/ORL/PHL/PHX/PIT/SEA/SFO/TPA/"
                          "WAS-BWI-US-PHL/WAS-GLA/LON-MAN";
    rMapStringVec.push_back(rString);
    RoutingMapStrings* rStringPtr = &rMapStringVec;
    mapInfo.routeStrings() = rStringPtr;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;

    // Call Validation
    diag.buildHeader();
    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    diag.displayRoutingStatus(tvlRoute, routingInfo);
    diag.displayRestrictions(routingInfo);
    diag.displayMileageMessages(routingInfo);
    diag.displayMapMessages(routingInfo);
    diag.displayHeader2(routingInfo);
    diag.displayRouteStrings(rStringPtr);

    std::string str = diag.str();

    std::string expected;
    expected += "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";
    expected += "AA  BOS-AA-MAN\n";
    expected += "    ATP   AT  1300 TRF-   6           ROUTING VALID\n";
    expected += "    RESTRICTION 07                    RESTRICTION PASSED\n";
    expected += "    BETWEEN BOS AND LON STOPOVER IN CHI REQUIRED\n";
    expected += " \n";
    expected += " \n";
    expected += "    ROUTE MAP DISPLAY\n";
    expected += "    ***********************************************************\n";
    expected += "    BOS-CHI/CLT/DFW/DTT/EWR/HOU/LAX/MIA/NYC/ORL/PHL/PHX/PIT/SEA\n";
    expected += "    /SFO/TPA/WAS-BWI-US-PHL/WAS-GLA/LON-MAN\n";
    expected += " \n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //--------------------------------------------------------------------------
  // Test a map with long strings and no slashes
  //--------------------------------------------------------------------------
  void testBuildRouteMapDisplay6()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();

    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "YVR";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.govCxr() = "UA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 6;
    routing.routing() = "1300";
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

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "07";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "BOS"; // City1
    rest.market2() = "LON"; // City2
    rest.viaMarket() = "CHI"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;

    // Build a restrictionInfo
    RestrictionInfo resInfo;
    resInfo.processed() = true;
    resInfo.valid() = true;

    // Build the RestrictionInfos Map and set pointer to it in RoutingInfo
    RestrictionInfos resInfos;
    resInfos.insert(std::map<RoutingRestriction*, RestrictionInfo>::value_type(&rest, resInfo));
    RestrictionInfos* resInfoPtr = &resInfos;
    routingInfo.restrictions() = resInfoPtr;

    // Create MapInfo
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    // Create Route Strings
    RoutingMapStrings rMapStringVec;
    std::string rString = "YVR-SEA-GEG-PDX-SFO-OAK-SJC-SFO-PHX-DEN-LNK-OMA-DSM-CID-MLI-CHI";
    rMapStringVec.push_back(rString);
    RoutingMapStrings* rStringPtr = &rMapStringVec;
    mapInfo.routeStrings() = rStringPtr;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;

    // Call Validation
    diag.buildHeader();
    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    diag.displayRoutingStatus(tvlRoute, routingInfo);
    diag.displayRestrictions(routingInfo);
    diag.displayMileageMessages(routingInfo);
    diag.displayMapMessages(routingInfo);
    diag.displayHeader2(routingInfo);
    diag.displayRouteStrings(rStringPtr);

    std::string str = diag.str();

    std::string expected;
    expected += "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";
    expected += "UA  YVR-UA-CHI\n";
    expected += "    ATP   WH  1300 TRF-   6           ROUTING VALID\n";
    expected += "    RESTRICTION 07                    RESTRICTION PASSED\n";
    expected += "    BETWEEN BOS AND LON STOPOVER IN CHI REQUIRED\n";
    expected += " \n";
    expected += " \n";
    expected += "    ROUTE MAP DISPLAY\n";
    expected += "    ***********************************************************\n";
    expected += "    YVR-SEA-GEG-PDX-SFO-OAK-SJC-SFO-PHX-DEN-LNK-OMA-DSM-CID-MLI\n";
    expected += "    -CHI\n";
    expected += " \n";

    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //--------------------------------------------------------------------------
  // Test a Routing Addon Display
  //--------------------------------------------------------------------------
  void testBuildRoutingAddonDisplay()
  {
    Diagnostic diagroot(Diagnostic455);
    diagroot.activate();
    Diag455Collector diag(diagroot);
    PricingTrx trx;
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic455);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NGO";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "OSA";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "NH";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "OSA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "TYO";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "NH";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "TYO";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "NH";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "STL";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "STL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DFW";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "NH";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "NGO";
    tvlRoute.originNation() = "JP";
    tvlRoute.destination() = "DFW";
    tvlRoute.destinationNation() = "US";
    tvlRoute.travelDate() = DateTime::localTime();
    tvlRoute.terminalPoints() = false;

    // Build Addon 1 Travel Route
    TravelRoute addonTvlRoute;
    boardCty.loc() = "NGO";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "OSA";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "NH";
    cityCarrier.offCity() = offCty;
    addonTvlRoute.travelRoute().push_back(cityCarrier);

    addonTvlRoute.origin() = "NGO";
    addonTvlRoute.originNation() = "JP";
    addonTvlRoute.destination() = "OSA";
    addonTvlRoute.destinationNation() = "JP";
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

    boardCty.loc() = "STL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DFW";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    addon2TvlRoute.travelRoute().push_back(cityCarrier);

    // Build Routing
    Routing routing;
    //  RoutingMapStrings* rStringPtr2 = &rMapStringVec2;
    routing.vendor() = "ATP";
    routing.carrier() = "NH";
    routing.routingTariff() = 3;
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

    // Rtg Addon 1  NGO-OSA
    Routing addOn1;
    addOn1.vendor() = "ATP";
    addOn1.carrier() = "NH";
    addOn1.routingTariff() = 3;
    addOn1.routing() = "2001";
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
    RoutingMap rm; // dummy to trigger header2
    addOn1.rmaps().push_back(&rm);
    // routing.rests() is an empty vector at this point.
    // routing.rmaps() is an empty vector at this point.

    // Rtg Addon 2 CHI-DFW
    Routing addOn2;
    addOn2.vendor() = "ATP";
    addOn2.carrier() = "NH";
    addOn2.routingTariff() = 3;
    addOn2.routing() = "2002";
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

    // Build routingInfo
    RoutingInfo routingInfo;
    routingInfo.routingStatus() = true;
    Routing* rtgPtr = &routing;
    routingInfo.routing() = rtgPtr;
    Routing* rtgPtr1 = &addOn1;
    routingInfo.origAddOnRouting() = rtgPtr1;
    Routing* rtgPtr2 = &addOn2;
    routingInfo.destAddOnRouting() = rtgPtr2;
    routingInfo.tcrRoutingTariffCode() = "PUSA";
    routingInfo.tcrAddonTariff1Code() = "TPRG";
    routingInfo.tcrAddonTariff2Code() = "PUSA";
    routingInfo.origAddOnGateway() = "NGO";
    routingInfo.origAddOnInterior() = "OSA";
    routingInfo.market1() = "OSA";
    routingInfo.market2() = "TYO";
    routingInfo.destAddOnGateway() = "CHI";
    routingInfo.destAddOnInterior() = "DFW";
    routingInfo.rtgAddon1CityCarrier() = addonTvlRoute.travelRoute();
    routingInfo.rtgAddon2CityCarrier() = addon2TvlRoute.travelRoute();

    // Create MapInfo
    MapInfo mapInfo;
    MapInfo* mapInfoPtr = &mapInfo;
    mapInfo.processed() = true;
    mapInfo.valid() = true;
    routingInfo.mapInfo() = mapInfoPtr;

    // Create rtgAddonMapInfo
    MapInfo rtgAddonMapInfo;
    MapInfo* addonMapInfoPtr = &rtgAddonMapInfo;
    rtgAddonMapInfo.processed() = true;
    rtgAddonMapInfo.valid() = true;
    routingInfo.rtgAddonMapInfo() = addonMapInfoPtr;

    // Create MileageInfo Validation Info with a vector of Surface Sector Markets
    MileageInfo mileageInfo;
    MileageInfo* mileageInfoPtr = &mileageInfo;
    mileageInfo.processed() = true;
    mileageInfo.valid() = true;
    routingInfo.mileageInfo() = mileageInfoPtr;
    routingInfo.mileageInfo()->totalApplicableMPM() = 8500;
    routingInfo.mileageInfo()->surchargeAmt() = 0;

    // Create Route Strings for Map Info (Rtg Addon 1)
    RoutingMapStrings rMapStringVec;
    std::string rString = "NGO-OSA";
    rMapStringVec.push_back(rString);

    // Create Route Strings for rtgAddonMapInfo (Rtg Addon 2)
    RoutingMapStrings rMapStringVec2;
    std::string rString2 = "CHI-STL-DFW";
    rMapStringVec2.push_back(rString2);

    mapInfo.routeStrings() = &rMapStringVec;

    // Call Validation
    diag.buildHeader();
    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    diag.displayRoutingStatus(tvlRoute, routingInfo);
    diag.displayMileageMessages(routingInfo);
    diag.displayMapMessages(routingInfo);
    diag.displayHeader2(routingInfo);
    diag.displayRouteStrings(&rMapStringVec);
    diag.displayRtgAddonHeader(routingInfo);
    diag.displayRouteStrings(&rMapStringVec2);

    std::string str = diag.str();

    std::string expected;
    expected = "\n************ ROUTE MAP AND RESTRICTIONS DISPLAY ***************\n";
    expected += "NH  NGO-NH-OSA-NH-TYO-NH-CHI-AA-STL-AA-DFW\n";
    expected += "    CONSTRUCTED ROUTING               ROUTING VALID\n";
    expected += "    MPM AT  8500 APPLIES TO ENTIRE ROUTE OF TRAVEL\n";
    expected += "      NGO-NH-OSA\n";
    expected += "      OSA-NGO ORIGIN ROUTING ADDON\n";
    expected += "        ATP     2001 TRF-3    TPRG   \n";
    expected += "        ROUTE MAP EXISTS - MAP VALIDATION PASSED\n";
    expected += "      OSA-TYO SPECIFIED BASE MILEAGE\n";
    expected += "        ATP AT  0000 TRF-3    PUSA   \n";
    expected += "      CHI-AA-STL-AA-DFW\n";
    expected += "      CHI-DFW DESTINATION ROUTING ADDON\n";
    expected += "        ATP     2002 TRF-3    PUSA   \n";
    expected += "    MILEAGE VALIDATION PASSED\n";
    expected += " \n";
    expected += "    ROUTE MAP DISPLAY       ORIGIN ROUTING ADDON 2001\n";
    expected += "    ***********************************************************\n";
    expected += "    NGO-OSA\n";
    expected += " \n";
    expected += " \n";
    expected += "    ROUTE MAP DISPLAY       DESTINATION ROUTING ADDON 2002\n";
    expected += "    ***********************************************************\n";
    expected += "    CHI-STL-DFW\n";
    expected += " \n";

    CPPUNIT_ASSERT_EQUAL(expected, str);

    // help out default deconstructor
    addOn1.rmaps().clear();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag455CollectorTest);
}
