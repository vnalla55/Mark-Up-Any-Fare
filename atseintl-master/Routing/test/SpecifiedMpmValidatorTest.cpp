#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/SpecifiedMpmValidator.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"

namespace tse
{
class SpecifiedMpmValidatorTest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
    const Mileage* getMileage(const LocCode& origin,
                              const LocCode& destination,
                              Indicator mileageType,
                              const GlobalDirection globalDirection,
                              const DateTime& dateTime)
    {

      return &_mil;
    }
    public:
    Mileage _mil;
  };

  CPPUNIT_TEST_SUITE(SpecifiedMpmValidatorTest);
  CPPUNIT_TEST(testNotRtw);
  CPPUNIT_TEST(testNoMpm);
  CPPUNIT_TEST(testPass);
  CPPUNIT_TEST(testFail);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memH;
  SpecifiedMpmValidator* _validator;
  TravelRoute* _tvlRoute;
  RoutingRestriction* _rest;
  PricingTrx* _trx;
  MyDataHandle* _dh;

  public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();

    _dh = _memH.create<MyDataHandle>();
    _dh->_mil.mileage() = 50;
    _validator = _memH.create<SpecifiedMpmValidator>();
    _tvlRoute = _memH.create<TravelRoute>();
    _rest = _memH.create<RoutingRestriction>();
    _trx = _memH.create<PricingTrx>();

    PricingOptions* opt = _memH.create<PricingOptions>();
    opt->setRtw(true);
    _trx->setOptions(opt);
    _trx->setRequest(_memH.create<PricingRequest>());

    _rest->mpm() = 100;

    _tvlRoute->mileageTravelRoute().push_back(TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LON.xml"));
  }

  void tearDown()
  {
    _memH.clear();
  }

  void testNotRtw()
  {
    _trx->setOptions(_memH.create<PricingOptions>());
    CPPUNIT_ASSERT(_validator->validate(*_tvlRoute, *_rest, *_trx));
  }

  void testNoMpm()
  {
    _rest->mpm() = 0;
    CPPUNIT_ASSERT(_validator->validate(*_tvlRoute, *_rest, *_trx));
  }


  void testPass()
  {
    CPPUNIT_ASSERT(_validator->validate(*_tvlRoute, *_rest, *_trx));
  }

  void testFail()
  {
    _dh->_mil.mileage() = 9999;
    CPPUNIT_ASSERT(!_validator->validate(*_tvlRoute, *_rest, *_trx));
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(SpecifiedMpmValidatorTest);
}
