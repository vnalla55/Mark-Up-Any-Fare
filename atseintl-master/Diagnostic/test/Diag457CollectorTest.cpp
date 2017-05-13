#include "test/include/CppUnitHelperMacros.h"
#include <time.h>
#include "test/include/MockGlobal.h"
#include <iostream>
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/RoutingRestriction.h"
#include "DBAccess/Routing.h"
#include "Routing/TravelRoute.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag457Collector.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Fares/RoutingController.h"
#include "Routing/RtgKey.h"
#include <map>
#include <unistd.h>

using namespace std;
namespace tse
{
class Diag457CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag457CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testEmptyDisplay);
  CPPUNIT_TEST(testDisplay);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(Diagnostic457);
      Diag457Collector diag(diagroot);

      string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  /* public */
  //------------------------------------------------------------------
  // Test Empty Display - Header Only
  //------------------------------------------------------------------
  void testEmptyDisplay()
  {
    Diagnostic diagroot(Diagnostic457);
    diagroot.activate();

    Diag457Collector diag(diagroot);

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic457);

    CPPUNIT_ASSERT(diag.isActive());

    diag.buildHeader();
    string str = diag.str();

    string expected;
    expected += "\n****** ROUTING MAP AND ROUTING LIST DEBUGGING DISPLAY *********\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Routing List
  //------------------------------------------------------------------------------
  void testDisplay()
  {
    Diagnostic diagroot(Diagnostic457);
    diagroot.activate();

    Diag457Collector diag(diagroot);

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic457);

    CPPUNIT_ASSERT(diag.isActive());

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "MIA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "SFO";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");

    int16_t itemNumber = 1;

    // Build Routing
    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 99;
    routing.routing() = "0002";
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 0;
    routing.domRtgvalInd() = ' ';
    Routing* r = &routing;

    // Build Map - Routing Key + Route Status
    typedef std::map<RtgKey, bool> RtgKeyMap;
    RtgKeyMap rtMap;
    RtgKey rKey;
    rKey.vendor() = "ATP";
    rKey.carrier() = "AA";
    rKey.routingTariff() = 99;
    rKey.routingNumber() = "0002";
    bool status = true;
    int16_t mapItemNumber = 1;

    rtMap.insert(std::map<RtgKey, bool>::value_type(rKey, status));

    diag.displayCityCarriers(tvlRoute.govCxr(), tvlRoute.travelRoute());
    diag.displayRoutingListItem(r, itemNumber);

    std::map<RtgKey, bool>::const_iterator rm;
    for (rm = rtMap.begin(); rm != rtMap.end(); ++rm)
    {
      diag.displayRoutingMapItem(rm, mapItemNumber);
      mapItemNumber++;
    }

    string str = diag.str();

    string expected;
    expected += "AA  MIA-AA-SFO\n";
    expected += "\n";
    expected += "      ROUTING LIST ITEM 1  VENDOR          ATP\n";
    expected += "                           ROUTING TARIFF  99\n";
    expected += "                           CARRIER         AA\n";
    expected += "                           ROUTING NUMBER  0002\n";
    // expected += "                         DRV INDICATOR    \n";
    expected += "                           RESTRICTIONS    0\n";
    expected += "                           NO ROUTE MAP\n";
    expected += " \n";
    expected += "      ROUTE MAP ITEM 1     VENDOR           ATP\n";
    expected += "                           ROUTING TARIFF 1 99\n";
    expected += "                           CARRIER          AA\n";
    expected += "                           ROUTING NUMBER   0002\n";
    expected += "                           ROUTE STATUS     TRUE\n";
    expected += " \n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag457CollectorTest);
}
