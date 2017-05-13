#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/TravelRoute.h"
#include "Routing/AirSurfaceRestrictionValidator.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class AirSurfaceRestrictionValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AirSurfaceRestrictionValidatorTest);

  CPPUNIT_TEST(testRestriction11A);
  CPPUNIT_TEST(testRestriction11B);
  CPPUNIT_TEST(testRestriction11C);
  CPPUNIT_TEST(testRestriction11D);
  CPPUNIT_TEST(testRestriction11E);
  CPPUNIT_TEST(testRestriction11F);
  CPPUNIT_TEST(testRestriction11G);
  CPPUNIT_TEST(testRestriction13A);
  CPPUNIT_TEST(testRestriction13B);
  CPPUNIT_TEST(testRestriction13C);
  CPPUNIT_TEST(testRestriction13D);
  CPPUNIT_TEST_SUITE_END();

  PricingTrx*   _trx;
  TestMemHandle _memH;
  AirSurfaceRestrictionValidator* _validator;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _trx = _memH.create<PricingTrx>();
    _trx->setOptions(_memH.create<PricingOptions>());

    _validator = _memH.create<AirSurfaceRestrictionValidator>();
  }
  void tearDown() { _memH.clear(); }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 11
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case A:       CITY1 and CITY2 exist on travel route.  Surface
  //                     exists between CITY1 and CITY2.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction11A()
  {
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "11";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "CHI"; // City1
    rest.market2() = "ATL"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 11
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case B:       CITY1 and CITY2 exist on travel route.  Surface
  //                     does not exist between CITY1 and CITY2.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   FALSE
  //                     Not Permitted  TRUE
  //-------------------------------------------------------------------------
  void testRestriction11B()
  {
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "11";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "CHI"; // City1
    rest.market2() = "ATL"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);

    rest.negViaAppl() = PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 11
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case B:       CITY1 and CITY2 exist on travel route.  Air
  //                     exists between CITY1 and CITY2.
  //
  //  Expected Returns:  Permitted  FALSE
  //                     Required   FALSE
  //                     Not Permitted  TRUE
  //-------------------------------------------------------------------------
  void testRestriction11C()
  {
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "11";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "ATL"; // City1
    rest.market2() = "MSY"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 11
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case D:       CITY1 and CITY2 exist on travel route.  Air/Surface
  //                     exists between CITY1 and CITY2.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   FALSE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction11D()
  {
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "11";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "ATL"; // City1
    rest.market2() = "CHI"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);

    rest.negViaAppl() = PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 11
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case E:       CITY1 and CITY2 exist on travel route.  Air/Surface
  //                     exists between CITY1 and CITY2.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   FALSE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction11E()
  {
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "11";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "ATL"; // City1
    rest.market2() = "MSY"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);

    rest.negViaAppl() = PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 11
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case F:       CITY1 and CITY2 do not exist on travel route.
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  TRUE
  //-------------------------------------------------------------------------
  void testRestriction11F()
  {
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "11";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "NYC"; // City1
    rest.market2() = "BOS"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 11
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case G:       CITY1 and CITY2 do exist on travel route.
  //                     Surface Required
  //
  //  Expected Returns:  Permitted  FALSE
  //                     Required   TRUE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction11G()
  {
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "11";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "CHI"; // City1
    rest.market2() = "ATL"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = _validator->validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 13
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case A:       Air Sector Required and found
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction13A()
  {
    AirSurfaceRestrictionValidator airSurfaceRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "13";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "ATL"; // City1
    rest.market2() = "MSY"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = PERMITTED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 13
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case B:       Surface Sector Required and found
  //
  //  Expected Returns:  Permitted  TRUE
  //                     Required   TRUE
  //                     Not Permitted  FALSE
  //-------------------------------------------------------------------------
  void testRestriction13B()
  {
    AirSurfaceRestrictionValidator airSurfaceRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "13";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "CHI"; // City1
    rest.market2() = "ATL"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = PERMITTED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 13
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case C:       Surface Sector Required but not found
  //
  //  Expected Returns:  Permitted  FALSE
  //                     Required   TRUE
  //                     Not Permitted  TRUE
  //-------------------------------------------------------------------------
  void testRestriction13C()
  {
    AirSurfaceRestrictionValidator airSurfaceRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "TW";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "13";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'S'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "CHI"; // City1
    rest.market2() = "ATL"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);

    rest.negViaAppl() = PERMITTED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 13
  //
  //  BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT PERM
  //
  //  Test Case D:       Air Sector Required but not found
  //
  //  Expected Returns:  Permitted  FALSE
  //                     Required   TRUE
  //                     Not Permitted  TRUE
  //-------------------------------------------------------------------------
  void testRestriction13D()
  {
    AirSurfaceRestrictionValidator airSurfaceRestrictionValidator;
    bool result = true;

    // Build TravelRoute
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "XX";
    cityCarrier.offCity() = offCty;
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
    rest.restriction() = "13";
    rest.marketAppl() = 'B'; // B=Between City1 and City2, T=To/From City1
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = 'A'; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = "CHI"; // City1
    rest.market2() = "ATL"; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = ""; // Via Carrier

    rest.negViaAppl() = REQUIRED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == false);

    rest.negViaAppl() = PERMITTED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);

    rest.negViaAppl() = NOT_PERMITTED;
    result = airSurfaceRestrictionValidator.validate(tvlRoute, rest, *_trx);
    CPPUNIT_ASSERT(result == true);
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(AirSurfaceRestrictionValidatorTest);
}
