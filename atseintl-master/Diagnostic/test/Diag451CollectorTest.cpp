#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag451Collector.h"

namespace tse
{
class Diag451CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag451CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testDisplayRouting);

  CPPUNIT_TEST_SUITE_END();

  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(Diagnostic451);
      Diag451Collector diag(diagroot);

      CPPUNIT_ASSERT_EQUAL(std::string(""), diag.str());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  //------------------------------------------------------------------------------
  // Display Routing
  //------------------------------------------------------------------------------
  void testDisplayRouting()
  {
    Diagnostic diagroot(Diagnostic451);
    diagroot.activate();

    Diag451Collector diag(diagroot);

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic451);

    CPPUNIT_ASSERT(diag.isActive());

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
    tvlRoute.origin() = "MEM";
    tvlRoute.destination() = "MIA";

    std::vector<RoutingKeyInfo*> rtgKeyInfoV1;
    RoutingKeyInfo info;
    info.routing() = "0075";
    info.routingTariff() = 99;
    info.routingCode() = "DRG1";

    rtgKeyInfoV1.push_back(&info);

    diag.displayRoutingValidationResults(tvlRoute, rtgKeyInfoV1);

    std::string expected = "\n******************** ROUTING DATA INFO ************************\n"
                           "AA  MEM-AA-CHI-AA-MIA\n"
                           "    WH  0075 TRF-  99     DRG1\n";

    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag451CollectorTest);
}
