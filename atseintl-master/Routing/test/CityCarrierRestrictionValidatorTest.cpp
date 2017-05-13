#include "test/include/CppUnitHelperMacros.h"

#include "Common/Vendor.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/Loc.h"
#include "DBAccess/RoutingRestriction.h"
#include "DBAccess/ZoneInfo.h"
#include "Routing/TravelRoute.h"
#include "Routing/CityCarrierRestrictionValidator.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

#include <memory>

using namespace std;

namespace tse
{

class MyDataHandleMock : public DataHandleMock
{
public:
    const Loc* getLoc(const LocCode& locCode, const DateTime& date)
    {
        Loc* loc = _memHandle.create<Loc>();
        loc->state() = "USWA";
        return loc;
    }

    const ZoneInfo* getZone(const VendorCode& vendor, const Zone& zone,
                            Indicator zoneType, const DateTime& date)
    {
        return DataHandleMock::getZone(Vendor::ATPCO, zone, RESERVED, date);
    }

private:
    ZoneInfo::ZoneSeg prepareZoneSeg(const LocCode& nation)
    {
        ZoneInfo::ZoneSeg zoneSeg;
        zoneSeg.locType() = LOCTYPE_NATION;
        zoneSeg.loc() = nation;
        return zoneSeg;
    }

    TestMemHandle _memHandle;
};


class CityCarrierRestrictionValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CityCarrierRestrictionValidatorTest);

  CPPUNIT_TEST(testRestriction1A);
  CPPUNIT_TEST(testRestriction1B);
  CPPUNIT_TEST(testRestriction1C);
  CPPUNIT_TEST(testRestriction1D);
  CPPUNIT_TEST(testRestriction1E);
  CPPUNIT_TEST(testRestriction1F);
  CPPUNIT_TEST(testRestriction1G);
  CPPUNIT_TEST(testRestriction2A);
  CPPUNIT_TEST(testRestriction2B);
  CPPUNIT_TEST(testRestriction2C);
  CPPUNIT_TEST(testRestriction2D);
  CPPUNIT_TEST(testRestriction2_nation);
  CPPUNIT_TEST(testRestriction2_zone);
  CPPUNIT_TEST(testRestriction2_genericCityCode);
  CPPUNIT_TEST(testRestriction5A);
  CPPUNIT_TEST(testRestriction5B);
  CPPUNIT_TEST(testRestriction5C);
  CPPUNIT_TEST(testRestriction5D);
  CPPUNIT_TEST(testRestriction5FromOriginViaNotExist);
  CPPUNIT_TEST(testRestriction18A);
  CPPUNIT_TEST(testRestriction18B);
  CPPUNIT_TEST(testRestriction18C);
  CPPUNIT_TEST(testRestriction18D);
  CPPUNIT_TEST(testRestriction18E);
  CPPUNIT_TEST(testRestriction19A);
  CPPUNIT_TEST(testRestriction19B);
  CPPUNIT_TEST(testRestriction19C);
  CPPUNIT_TEST(testRestriction19D);
  CPPUNIT_TEST(testRestriction21A);
  CPPUNIT_TEST(testRestriction21B);
  CPPUNIT_TEST(testRestriction21D);
  CPPUNIT_TEST(testRestriction21E);
  CPPUNIT_TEST(testRestriction21F);
  CPPUNIT_TEST(testRestriction21G);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  MyDataHandleMock* _mdh;
  PricingOptions* _options;
  PricingTrx* _trx;
  std::shared_ptr<CityCarrierRestrictionValidator> _cityCarrierRestVal;

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _options = _memHandle.create<PricingOptions>();
    _options->setRtw(false);
    _trx->setOptions(_options);
    _mdh = _memHandle.create<MyDataHandleMock>();
    _cityCarrierRestVal.reset(new CityCarrierRestrictionValidator());
  }

  void tearDown() {}

  RoutingRestriction prepareRestriction(int seqNo, RestrictionNumber res, Indicator marketAppl,
                                        Indicator nonStopDirectInd, Indicator airSurfaceInd,
                                        LocCode market1, LocCode market2, LocCode viaMarket,
                                        LocCode viaCarrier, Indicator viaType = 'C',
                                        Indicator market1type = 'C', Indicator market2type = 'C')
  {
      RoutingRestriction rest;
      rest.restrSeqNo() = seqNo;
      rest.restriction() = res;
      rest.marketAppl() = marketAppl; // B=Between City1 and City2, T=To/From City1
      rest.viaType() = viaType; // A=Airline, C=City, Blank=Not Applicable
      rest.nonStopDirectInd() = nonStopDirectInd; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
      rest.airSurfaceInd() = airSurfaceInd; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
      rest.market1() = market1; // City1
      rest.market2() = market2; // City2
      rest.viaMarket() = viaMarket; // Via City
      rest.viaCarrier() = viaCarrier; // Via Carrier
      rest.market1type() = market1type;  // A=Airline, C=City, Blank=Not Applicable
      rest.market2type() = market2type; // A=Airline, C=City, Blank=Not Applicable

      return rest;
  }

  TravelRoute::CityCarrier prepareCityCarrier(LocCode bordCity, LocCode offCity,
                                              CarrierCode carrierCode,
                                              NationCode offNation = "")
  {
      TravelRoute::City boardCty, offCty;
      boardCty.loc() = bordCity;
      boardCty.isHiddenCity() = false;
      offCty.loc() = offCity;
      offCty.isHiddenCity() = false;

      TravelRoute::CityCarrier cityCarrier;
      cityCarrier.boardCity() = boardCty;
      cityCarrier.carrier() = carrierCode;
      cityCarrier.offCity() = offCty;

      cityCarrier.offNation() = offNation;

      return cityCarrier;
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 1
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CITY3
  //
  //  Test Case A:       CITY1, CITY2, and CITY3 all exist on travel route.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //-------------------------------------------------------------------------
  void testRestriction1A()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "01", 'B', ' ', ' ', "NYC", "MSY", "ATL", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 1
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CITY3
  //
  //  Test Case B:       CITY1 and CITY2 do not exist on travel route.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //-------------------------------------------------------------------------
  void testRestriction1B()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SAT", "DFW", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "CHI", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("CHI", "MIA", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "01", 'B', ' ', ' ', "DEN", "MIA", "CHI", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 1
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CITY3
  //
  //  Test Case C:       CITY1 and CITY2 do exist on the travel route, but
  //                     CITY3 does not exist on the travel route.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //-------------------------------------------------------------------------
  void testRestriction1C()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "MSY", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "02", ' ', ' ', ' ', "", "", "CHI", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 1
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CITY3
  //
  //  Test Case D:       CITY1 and CITY2 do exist on the travel route, but
  //                     CITY3 does not exist on the travel route.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //-------------------------------------------------------------------------
  void testRestriction1D()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SAT", "DFW", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "CHI", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("CHI", "MIA", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "01", 'B', ' ', ' ', "SAT", "MIA", "DFW", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 1
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CITY3
  //
  //  Test Case E:       CITY1 and CITY2 do exist on the travel route, but
  //                     CITY3 does not exist on the travel route.
  //                     CITY1 and CITY2 are in reverse order.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //-------------------------------------------------------------------------
  void testRestriction1E()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SAT", "DFW", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "CHI", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("CHI", "MIA", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "01", 'B', ' ', ' ', "SAT", "MIA", "DFW", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 1
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CITY3
  //
  //  Test Case F:       CITY1 and CITY2 do exist on the travel route, but
  //                     CITY3 does not exist on the travel route.
  //                     CITY1 and CITY2 are in reverse order.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //-------------------------------------------------------------------------
  void testRestriction1F()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ALB", "DFW", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "SAT", "TW"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SAT", "MIA", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "01", 'B', ' ', ' ', "DFW", "SAT", "ALB", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 1
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CITY3
  //
  //  Test Case G:       CITY1 and CITY2 do exist on the travel route, but
  //                     CITY3 does not exist on the travel route.
  //                     CITY1 and CITY2 are in reverse order.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //-------------------------------------------------------------------------
  void testRestriction1G()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ALB", "DFW", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "SAT", "TW"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SAT", "MIA", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "01", 'B', ' ', ' ', "SAT", "DFW", "ALB", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //--------------------------------------------------------------------
  //  Test Routing Restriction 2
  //
  //  TRAVEL MUST BE VIA CITY3
  //
  //  Test Case A:       City3 exists on TravelRoute as ticketed point.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //--------------------------------------------------------------------
  void testRestriction2A()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "02", ' ', ' ', ' ', "", "", "ATL", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //--------------------------------------------------------------------
  //  Test Routing Restriction 2
  //
  //  TRAVEL MUST/MUST NOT BE VIA CITY3
  //
  //  Test Case B:       City3 exists on TravelRoute as a flight stop.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //--------------------------------------------------------------------
  void testRestriction2B()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "02", ' ', ' ', ' ', "", "", "ATL", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //--------------------------------------------------------------------
  //  Test Restriction 2
  //
  //  TRAVEL MUST BE VIA CITY3
  //
  //  Test Case C:       City3 does not exist on TravelRoute
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //--------------------------------------------------------------------
  void testRestriction2C()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SAT", "DFW", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "CHI", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("CHI", "MIA", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "02", ' ', ' ', ' ', "", "", "STL", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 2
  //
  //  TRAVEL MUST BE VIA CITY3
  //
  //  Test Case D:       City3 does not exist on TravelRoute, simple O&D
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction2D()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "MSY", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "02", ' ', ' ', ' ', "", "", "CHI", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //--------------------------------------------------------------------
  //  Test Routing Restriction 2
  //
  //  TRAVEL MUST/MUST NOT BE VIA CITY3 nation
  //
  //  Test Case B:       City3 exists on TravelRoute as a flight stop.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //--------------------------------------------------------------------
  void testRestriction2_nation()
  {
    _trx->getOptions()->setRtw(true);
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL", "AA", "US"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA", "PL"));

    RoutingRestriction rest = prepareRestriction(1, "02", ' ', ' ', ' ', "", "", "US", "", 'N');

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //--------------------------------------------------------------------
  //  Test Routing Restriction 2
  //
  //  TRAVEL MUST/MUST NOT BE VIA CITY3 zona
  //
  //  Test Case B:       City3 exists on TravelRoute as a flight stop.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //--------------------------------------------------------------------
  void testRestriction2_zone()
  {
    _trx->getOptions()->setRtw(true);
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "ATL", "AA", "PL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA", "US"));

    RoutingRestriction rest = prepareRestriction(1, "02", ' ', ' ', ' ', "", "", "210", "", 'Z');

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //--------------------------------------------------------------------
  //  Test Routing Restriction 2
  //
  //  TRAVEL MUST/MUST NOT BE VIA CITY3 generic city code
  //
  //  Test Case B:       City3 exists on TravelRoute as a flight stop.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //--------------------------------------------------------------------
  void testRestriction2_genericCityCode()
  {
    _trx->getOptions()->setRtw(true);
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "USWA", "AA", "US"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "AA", "PL"));

    RoutingRestriction rest = prepareRestriction(1, "02", ' ', ' ', ' ', "", "", "WCC", "", 'C');

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }
  //----------------------------------------------------------------------
  //  Test Restriction 5
  //
  //  TRAVEL TO/FROM CITY1 MUST/MUST NOT BE VIA CITY3
  //
  //  Test Case A:       City1 and City3 exist on TravelRoute,
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //----------------------------------------------------------------------
  void testRestriction5A()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "PAR", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("PAR", "ROM", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ROM", "ZRH", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "05", 'T', ' ', ' ', "PAR", "", "ROM", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 5
  //
  //  TRAVEL TO/FROM CITY1 MUST/MUST NOT BE VIA CITY3
  //
  //  Test Case B:       City1 and City3 exist on TravelRoute, but City3 is
  //                     the origin which is not allowed,
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction5B()
  {
    TravelRoute tvlRoute;
    tvlRoute.origin() = "ROM";
    tvlRoute.destination() = "LON";
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ROM", "PAR", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("PAR", "LON", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "05", 'T', ' ', ' ', "PAR", "", "ROM", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 5
  //
  //  TRAVEL TO/FROM CITY1 MUST/MUST NOT BE VIA CITY3
  //
  //  Test Case C:       City1 and City3 exist on TravelRoute, but City3 is
  //                     the destination which is not allowed,
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction5C()
  {
    TravelRoute tvlRoute;
    tvlRoute.origin() = "NYC";
    tvlRoute.destination() = "ROM";
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "PAR", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("PAR", "ROM", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "05", 'T', ' ', ' ', "PAR", "", "ROM", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 5
  //
  //  TRAVEL TO/FROM CITY1 MUST/MUST NOT BE VIA CITY3
  //
  //  Test Case D:       City1 does not exist on the TravelRoute.
  //                     The restriction is not applicable.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction5D()
  {
    TravelRoute tvlRoute;
    tvlRoute.origin() = "MEM";
    tvlRoute.destination() = "MSY";
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MEM", "ATL", "DL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "DL"));

    RoutingRestriction rest = prepareRestriction(1, "05", 'T', ' ', ' ', "PIB", "", "MEM", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 5
  //
  //  TRAVEL TO/FROM CITY1 MUST/MUST NOT BE VIA CITY3
  //
  //  Test Case E:       City1 is Origin, City3 doesn't exist on the TravelRoute.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Required)  FALSE
  //                     TRAVEL MUST BE VIA (Permitted) TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction5FromOriginViaNotExist()
  {
    TravelRoute tvlRoute;
    tvlRoute.origin() = "PAR";
    tvlRoute.destination() = "LON";
    tvlRoute.travelRoute().push_back(prepareCityCarrier("PAR", "LON", "BA"));

    RoutingRestriction rest = prepareRestriction(1, "05", 'T', ' ', ' ', "PAR", "", "FRA", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 18
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CARRIER
  //
  //  Test Case A:       CITY1, CITY2, and CARRIER all exist on travel route.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //-------------------------------------------------------------------------
  void testRestriction18A()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MEM", "ATL", "DL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "DL"));

    RoutingRestriction rest = prepareRestriction(1, "18", 'B', ' ', ' ', "MEM", "MSY", "", "DL", 'A');

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 18
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CARRIER
  //
  //  Test Case B:       CITY1 and CITY2 exist on travel route, but CARRIER
  //                     does not.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //-------------------------------------------------------------------------
  void testRestriction18B()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MEM", "ATL", "DL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ATL", "MSY", "DL"));

    RoutingRestriction rest = prepareRestriction(1, "18", 'B', ' ', ' ', "MEM", "ATL", "", "UA", 'A');


    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 18
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CARRIER
  //
  //  Test Case C:       CITY1, CITY2, and CARRIER all exist on travel route.
  //                     Surface sector included.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //-------------------------------------------------------------------------
  void testRestriction18C()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("CUN", "SAT", "MX"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SAT", "DFW", "XX"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "CHI", "MX"));

    RoutingRestriction rest = prepareRestriction(1, "18", 'B', ' ', ' ', "CUN", "CHI", "", "MX", 'A');

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 18
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CARRIER
  //
  //  Test Case D:       CITY1, CITY2, and CARRIER all exist on travel route.
  //                     Surface sector included, but not ALL carriers match.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //-------------------------------------------------------------------------
  void testRestriction18D()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("CUN", "SAT", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SAT", "DFW", "XX"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "CHI", "MX"));

    RoutingRestriction rest = prepareRestriction(1, "18", 'B', ' ', ' ', "CUN", "CHI", "", "MX", 'A');

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //-------------------------------------------------------------------------
  //  Test Routing Restriction 18
  //
  //  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CARRIER
  //
  //  Test Case E:       CITY1, CITY2, exist on travel route but carrier .
  //                     does not.  City1 and City2 are in reverse order.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //-------------------------------------------------------------------------
  void testRestriction18E()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MIA", "SJU", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SJU", "AUA", "DL"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("AUA", "CUR", "CX"));

    RoutingRestriction rest = prepareRestriction(1, "18", 'B', ' ', ' ', "MIA", "CUR", "", "LH", 'A');


    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 19
  //
  //  TRAVEL MUST BE VIA CARRIER
  //
  //  Test Case A:       Carrier does not exist on TravelRoute, simple O&D
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction19A()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "LON", "AA"));

    RoutingRestriction rest = prepareRestriction(1, "19", 'T', ' ', ' ', "LON", "", "", "UA", 'A');

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 19
  //
  //  TRAVEL MUST BE VIA CARRIER
  //
  //  Test Case B:       Carrier does exist on TravelRoute.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //----------------------------------------------------------------------
  void testRestriction19B()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SAT", "DFW", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "NYC", "UA"));


    RoutingRestriction rest = prepareRestriction(1, "19", 'T', ' ', ' ', "DFW", "", "", "UA", 'A');


    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 19
  //
  //  TRAVEL MUST BE VIA CARRIER
  //
  //  Test Case C:       Carrier does not exist on TravelRoute.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction19C()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ELP", "DFW", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DFW", "CHI", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("CHI", "VIE", "BA"));

    RoutingRestriction rest = prepareRestriction(1, "19", 'T', ' ', ' ', "CHI", "", "", "UA", 'A');

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 19
  //
  //  TRAVEL MUST BE VIA CARRIER
  //
  //  Test Case D:       Carrier does not exist on TravelRoute.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction19D()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("MIA", "SJU", "AA"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SJU", "AUA", "AN"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("AUA", "CUR", "XX"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("CUR", "BON", "AN"));

    RoutingRestriction rest = prepareRestriction(1, "19", 'T', ' ', ' ', "AUA", "", "", "AN", 'A');

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 21
  //
  //  WHEN ORIGIN IS CITY1 AND DEST IS CITY2 TRAVEL MUST BE VIA CITY3
  //
  //  Test Case A:       City1 is Origin, City2 is Destination, and
  //                     City3 is on the travel route.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //----------------------------------------------------------------------
  void testRestriction21A()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("BUR", "LAX", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "LAS", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAS", "PHX", "HP"));

    RoutingRestriction rest = prepareRestriction(1, "21", ' ', ' ', ' ', "BUR", "PHX", "LAS", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 21
  //
  //  WHEN ORIGIN IS CITY1 AND DEST IS CITY2 TRAVEL MUST BE VIA CITY3
  //
  //  Test Case B:       City1 is the Origin but City2 is not the Dest.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction21B()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("BUR", "LAX", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "LAS", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAS", "PHX", "HP"));


    RoutingRestriction rest = prepareRestriction(1, "21", ' ', ' ', ' ', "BUR", "ELP", "LAS", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 21
  //
  //  WHEN ORIGIN IS CITY1 AND DEST IS CITY2 TRAVEL MUST BE VIA CITY3
  //
  //  Test Case C:       City1 is not the Origin but City2 is the Dest.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction21C()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("BUR", "LAX", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "LAS", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAS", "PHX", "HP"));

    RoutingRestriction rest = prepareRestriction(1, "21", ' ', ' ', ' ', "MEX", "PHX", "LAS", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 21
  //
  //  WHEN ORIGIN IS CITY1 AND DEST IS CITY2 TRAVEL MUST BE VIA CITY3
  //
  //  Test Case D:       City1 is not the Origin and City2 is not the Dest.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction21D()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("BUR", "LAX", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "LAS", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAS", "PHX", "HP"));


    RoutingRestriction rest = prepareRestriction(1, "21", ' ', ' ', ' ', "VIE", "ABQ", "LAS", "");

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 21
  //
  //  WHEN ORIGIN IS CITY1 AND DEST IS CITY2 TRAVEL MUST BE VIA CITY3
  //
  //  Test Case E:       City1 is the Origin and City2 is the Dest,
  //                     but City3 does not exist on the travelRoute.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  FALSE
  //                     TRAVEL MUST BE VIA (Required)   FALSE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction21E()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("BUR", "LAX", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "LAS", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAS", "PHX", "HP"));


    RoutingRestriction rest = prepareRestriction(1, "21", ' ', ' ', ' ', "BUR", "PHX", "SVO", "");
    //rest.restrSeqNo() = 1;

    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 21
  //
  //  WHEN ORIGIN IS CITY1 AND DEST IS CITY2 TRAVEL MUST BE VIA CITY3
  //
  //  Test Case F:       City1 is the Origin and City2 is the Dest,
  //                     City3 does exist on the travelRoute.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  FALSE
  //----------------------------------------------------------------------
  void testRestriction21F()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("BUR", "LAX", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "LAS", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAS", "PHX", "HP"));

    RoutingRestriction rest = prepareRestriction(1, "21", ' ', ' ', ' ', "BUR", "PHX", "LAS", "");


    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(!_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }

  //----------------------------------------------------------------------
  //  Test Restriction 21
  //
  //  WHEN ORIGIN IS CITY1 AND DEST IS CITY2 TRAVEL MUST BE VIA CITY3
  //
  //  Test Case G:       City1 is the Dest and City2 is the Origin.
  //                     City3 does exist on the travelRoute.
  //
  //  Expected Returns:  TRAVEL MUST BE VIA (Permitted)  TRUE
  //                     TRAVEL MUST BE VIA (Required)   TRUE
  //                     TRAVEL MUST NOT BE VIA (Not Permitted)  TRUE
  //----------------------------------------------------------------------
  void testRestriction21G()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("BUR", "LAX", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "LAS", "HP"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAS", "PHX", "HP"));

    RoutingRestriction rest = prepareRestriction(1, "21", ' ', ' ', ' ', "PHX", "BUR", "LAX", "");


    rest.negViaAppl() = REQUIRED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));

    rest.negViaAppl() = NOT_PERMITTED;
    CPPUNIT_ASSERT(_cityCarrierRestVal->validate(tvlRoute, rest, *_trx));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CityCarrierRestrictionValidatorTest);
}
