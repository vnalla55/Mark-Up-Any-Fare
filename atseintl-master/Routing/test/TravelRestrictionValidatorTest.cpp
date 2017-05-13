#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/Loc.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/TravelRoute.h"
#include "Routing/TravelRestrictionValidator.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{

namespace
{
class MyDataHandle: public DataHandleMock
{
  const LocCode
  getMultiTransportCityCode(const LocCode& locCode, const CarrierCode& carrierCode,
                            GeoTravelType tvlType, const DateTime& tvlDate)
  {
    return locCode;
  }
};

}

class TravelRestrictionValidatorMock: public TravelRestrictionValidator
{
  friend class TravelRestrictionValidatorTest;
public:
  TravelRestrictionValidatorMock(): TravelRestrictionValidator() {}
};

class TravelRestrictionValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TravelRestrictionValidatorTest);

  CPPUNIT_TEST(testValidateWhenR9UniqueOrigin);
  CPPUNIT_TEST(testValidateWhenR9BackToOrigin);
  CPPUNIT_TEST(testValidateWhenR9BackToOriginForHiddenStop);
  CPPUNIT_TEST(testValidateWhenR9UniqueOriginForHiddenStop);
  CPPUNIT_TEST(testValidateWhenR10UniqueCouponsOnly);
  CPPUNIT_TEST(testValidateWhenR10SameCouponTwice);
  CPPUNIT_TEST(testValidateWhenR10SameCouponTwiceOnHiddenStop);

  CPPUNIT_TEST_SUITE_END();

public:
  void
  setUp()
  {
    _memHandle.create<MyDataHandle>();
    _tvlRestrictionValidatorMock = _memHandle.create<TravelRestrictionValidatorMock>();
    _tvlRoute = _memHandle.create<TravelRoute>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->setRtw(true);
    _memHandle.create<TestConfigInitializer>();
  }

  void
  tearDown()
  {
    _memHandle.clear();
  }

  void prepareValidate()
  {
    AirSeg* seg1 = _memHandle.create<AirSeg>();
    Loc* loc = _memHandle.create<Loc>();

    _tvlRoute->unticketedPointInd() = TKTPTS_ANY;

    seg1->origAirport() = "CHI";
    seg1->destAirport() = "LON";
    seg1->origin() = loc;
    seg1->destination() = loc;
    _tvlRoute->mileageTravelRoute().push_back(seg1);

    AirSeg* seg2 = _memHandle.create<AirSeg>();
    seg2->origAirport() = "LON";
    seg2->destAirport() = "PAR";
    seg2->origin() = loc;
    seg2->destination() = loc;
    _tvlRoute->mileageTravelRoute().push_back(seg2);

    AirSeg* seg3 = _memHandle.create<AirSeg>();
    seg3->origAirport() = "PAR";
    seg3->destAirport() = "KTW";
    seg3->origin() = loc;
    seg3->destination() = loc;
    _tvlRoute->mileageTravelRoute().push_back(seg3);

    AirSeg* seg4 = _memHandle.create<AirSeg>();
    seg4->origAirport() = "KTW";
    seg4->destAirport() = "CHI";
    seg4->origin() = loc;
    seg4->destination() = loc;
    _tvlRoute->mileageTravelRoute().push_back(seg4);
  }

  void testValidateWhenR9UniqueOrigin()
  {
    prepareValidate();
    RoutingRestriction rest;
    rest.restriction() = RTW_ROUTING_RESTRICTION_9;

    CPPUNIT_ASSERT(_tvlRestrictionValidatorMock->validate(*_tvlRoute, rest, *_trx));
  }

  void testValidateWhenR9BackToOrigin()
  {
    prepareValidate();
    _tvlRoute->mileageTravelRoute()[1]->destAirport() = "CHI";
    _tvlRoute->mileageTravelRoute()[2]->origAirport() = "CHI";
    _tvlRoute->mileageTravelRoute()[2]->destAirport() = "DFW";
    _tvlRoute->mileageTravelRoute()[3]->origAirport() = "DFW";
    _tvlRoute->mileageTravelRoute()[3]->destAirport() = "CHI";

    RoutingRestriction rest;
    rest.restriction() = RTW_ROUTING_RESTRICTION_9;

    CPPUNIT_ASSERT(!_tvlRestrictionValidatorMock->validate(*_tvlRoute, rest, *_trx));
  }

  void testValidateWhenR9BackToOriginForHiddenStop()
  {
    prepareValidate();
    _tvlRoute->unticketedPointInd() = TKTPTS_ANY;

    Loc hiddenStop;
    _tvlRoute->mileageTravelRoute()[2]->hiddenStops().push_back(&hiddenStop);
    hiddenStop.loc() = "CHI";

    RoutingRestriction rest;
    rest.restriction() = RTW_ROUTING_RESTRICTION_9;

    CPPUNIT_ASSERT(!_tvlRestrictionValidatorMock->validate(*_tvlRoute, rest, *_trx));
  }

  void testValidateWhenR9UniqueOriginForHiddenStop()
  {
    prepareValidate();
    _tvlRoute->unticketedPointInd() = TKTPTS_TKTONLY;

    Loc hiddenStop;
    _tvlRoute->mileageTravelRoute()[2]->hiddenStops().push_back(&hiddenStop);
    hiddenStop.loc() = "CHI";

    RoutingRestriction rest;
    rest.restriction() = RTW_ROUTING_RESTRICTION_9;

    CPPUNIT_ASSERT(_tvlRestrictionValidatorMock->validate(*_tvlRoute, rest, *_trx));
  }

  void testValidateWhenR10UniqueCouponsOnly()
  {
    prepareValidate();

    RoutingRestriction rest;
    rest.restriction() = RTW_ROUTING_RESTRICTION_10;

    CPPUNIT_ASSERT(_tvlRestrictionValidatorMock->validate(*_tvlRoute, rest, *_trx));
  }

  void testValidateWhenR10SameCouponTwice()
  {
    prepareValidate();
    _tvlRoute->mileageTravelRoute()[2]->origAirport() = "LON";
    _tvlRoute->mileageTravelRoute()[2]->destAirport() = "PAR";

    RoutingRestriction rest;
    rest.restriction() = RTW_ROUTING_RESTRICTION_10;

    CPPUNIT_ASSERT(!_tvlRestrictionValidatorMock->validate(*_tvlRoute, rest, *_trx));
  }

  void testValidateWhenR10SameCouponTwiceOnHiddenStop()
  {
    prepareValidate();
    _tvlRoute->unticketedPointInd() = TKTPTS_ANY;
    _tvlRoute->mileageTravelRoute()[2]->origAirport() = "LON";

    Loc hiddenStop;
    _tvlRoute->mileageTravelRoute()[2]->hiddenStops().push_back(&hiddenStop);
    hiddenStop.loc() = "PAR";

    RoutingRestriction rest;
    rest.restriction() = RTW_ROUTING_RESTRICTION_10;

    CPPUNIT_ASSERT(!_tvlRestrictionValidatorMock->validate(*_tvlRoute, rest, *_trx));
  }

private:
  TestMemHandle _memHandle;
  TravelRestrictionValidatorMock* _tvlRestrictionValidatorMock;
  TravelRoute* _tvlRoute;
  PricingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TravelRestrictionValidatorTest);
}
