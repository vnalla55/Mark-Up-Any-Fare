#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/TravelRoute.h"
#include "Routing/StopOverRestrictionValidator.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{

class StopOverRestrictionValidatorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(StopOverRestrictionValidatorTest);

  CPPUNIT_TEST(testRestriction7A);
  CPPUNIT_TEST(testRestriction7B);
  CPPUNIT_TEST(testRestriction7C);
  CPPUNIT_TEST(testRestriction7D);
  CPPUNIT_TEST(testRestriction7E);
  CPPUNIT_TEST(testRestriction8A);
  CPPUNIT_TEST(testRestriction8B);
  CPPUNIT_TEST(testRestriction8C);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
      _options.setRtw(false);
      _trx.setOptions(&_options);
  }

  void tearDown() {}

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 7
  //
  //  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 REQUIRED/PERMITTED/NOT PERMITTED
  //
  //  Test Case A:       CITY1, CITY2, and CITY3 all exist on travel route.
  //                     A stopover exists in City3.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction7A()
  {
    StopOverRestrictionValidator stopOverRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = true;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "ATL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MSY";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "07";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "NYC"; // City1
    rest.market2() = "MSY"; // City2
    rest.viaMarket() = "ATL"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == false);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 7
  //
  //  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 REQUIRED/PERMITTED/NOT PERMITTED
  //
  //  Test Case B:       CITY1, CITY2, and CITY3 all exist on travel route.
  //                     A stopover exists in City3.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction7B()
  {
    StopOverRestrictionValidator stopOverRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "PHL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "BOS";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = true;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "BOS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "BA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "07";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "PHL"; // City1
    rest.market2() = "LON"; // City2
    rest.viaMarket() = "BOS"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == false);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 7
  //
  //  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 REQUIRED/PERMITTED/NOT PERMITTED
  //
  //  Test Case C:       CITY1, CITY2, and CITY3 all exist on travel route.
  //                     A stopover does not exist in City3.
  //
  //  Expected Returns:  Permitted  FALSE
  //                     Required   FALSE
  //                     Not Permitted  TRUE
  //-------------------------------------------------------------------------
  void testRestriction7C()
  {
    StopOverRestrictionValidator stopOverRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "PHL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "BOS";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "BOS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "BA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "07";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "PHL"; // City1
    rest.market2() = "LON"; // City2
    rest.viaMarket() = "BOS"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == false);

    rest.negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 7
  //
  //  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 REQUIRED/PERMITTED/NOT PERMITTED
  //
  //  Test Case D:       CITY1 exists on travel route, but CITY2 does not.
  //                     A stopover does not exist in City3.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  TRUE
  //-------------------------------------------------------------------------
  void testRestriction7D()
  {
    StopOverRestrictionValidator stopOverRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "PHL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "BOS";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "BOS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "BA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "07";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "PHL"; // City1
    rest.market2() = "MAN"; // City2
    rest.viaMarket() = "BOS"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 7
  //
  //  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 REQUIRED/PERMITTED/NOT PERMITTED
  //
  //  Test Case E:       CITY1 does not exist on travel route, but CITY2 does
  //                     A stopover does not exist in City3.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  TRUE
  //-------------------------------------------------------------------------
  void testRestriction7E()
  {
    StopOverRestrictionValidator stopOverRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "PHL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "BOS";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "BOS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "BA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "07";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "CHI"; // City1
    rest.market2() = "LON"; // City2
    rest.viaMarket() = "BOS"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 8
  //
  //  STOPOVER IN CITY3 REQUIRED/PERMITTED/NOT PERMITTED
  //
  //  Test Case A:       A stopover does exist in City3.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction8A()
  {
    StopOverRestrictionValidator stopOverRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "SIN";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "YVR";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AF";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = true;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "YVR";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "SEA";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "08";
    rest.marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = ""; // City1
    rest.market2() = ""; // City2
    rest.viaMarket() = "YVR"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == false);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 8
  //
  //  STOPOVER IN CITY3 REQUIRED/PERMITTED/NOT PERMITTED
  //
  //  Test Case B:       A stopover does not exist in City3.
  //
  //  Expected Returns:  Permitted  FALSE
  //                     Required   FALSE
  //                     Not Permitted  TRUE
  //-------------------------------------------------------------------------
  void testRestriction8B()
  {
    StopOverRestrictionValidator stopOverRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "SIN";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "YVR";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AF";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "YVR";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "SEA";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "08";
    rest.marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = ""; // City1
    rest.market2() = ""; // City2
    rest.viaMarket() = "YVR"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == false);

    rest.negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 8
  //
  //  STOPOVER IN CITY3 REQUIRED/PERMITTED/NOT PERMITTED
  //
  //  Test Case C:       A stopover does exist in City3.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction8C()
  {
    StopOverRestrictionValidator stopOverRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "DUS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LIS";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AF";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "LIS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MAD";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = true;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "MAD";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DFW";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    cityCarrier.stopover() = false;
    tvlRoute.travelRoute().push_back(cityCarrier);

    // Build RoutingRestriction
    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "08";
    rest.marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = 'C'; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = ""; // City1
    rest.market2() = ""; // City2
    rest.viaMarket() = "MAD"; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = 'R'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'P'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = 'N'; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    result = stopOverRestrictionValidator.validate(tvlRoute, rest, _trx);
    CPPUNIT_ASSERT(result == false);
  }

  PricingOptions _options;
  PricingTrx _trx;
};
CPPUNIT_TEST_SUITE_REGISTRATION(StopOverRestrictionValidatorTest);
}
