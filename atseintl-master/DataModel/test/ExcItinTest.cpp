#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "Server/TseServer.h"
#include "DataModel/RexPricingTrx.h"
#include "Common/TrxUtil.h"

namespace tse
{
class FakeTseServer : public TseServer
{
public:
  void initializeGlobal()
  {
    TseServer::initializeGlobalConfigMan();
    TseServer::initializeGlobal();
  }
};

class ExcItinTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ExcItinTest);
  CPPUNIT_TEST(testDefaultValue);
  CPPUNIT_TEST(testSetFareComp);
  CPPUNIT_TEST(testSetValidityDate);
  CPPUNIT_TEST(testTravelDateWhenRexTrxEmpty);
  CPPUNIT_TEST(testTravelDateWhenTravelDateIsBeforeTicketDate);
  CPPUNIT_TEST(testTravelDateWhenTicketDateIsBeforeTravelDate);
  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  FakeTseServer* _myServer;
  tse::ExcItin _itin;
  tse::RexPricingTrx _rexTrx;

public:
  void setUp()
  {
    _myServer = _memHandle.create<FakeTseServer>();
    _myServer->initializeGlobal();
    _memHandle.create<TestConfigInitializer>();

    TestConfigInitializer::setValue("ADJUST_REX_TRAVEL_DATE", "Y", "REX_FARE_SELECTOR_SVC");
  }

  void tearDown() { _memHandle.clear(); }

  void testDefaultValue() { CPPUNIT_ASSERT(_itin.fareComponent().size() == 0); }

  void testSetFareComp()
  {
    FareCompInfo fc;
    _itin.fareComponent().push_back(&fc);
    CPPUNIT_ASSERT(_itin.fareComponent().size() == 1);
  }

  void testSetValidityDate()
  {
    CPPUNIT_ASSERT(_itin.tktValidityDate() == DateTime(1980, 1, 1));
    _itin.setTktValidityDate(2008, 5, 1);
    CPPUNIT_ASSERT(_itin.tktValidityDate() == DateTime(2008, 5, 1));
  }

  void testTravelDateWhenRexTrxEmpty()
  {
    DateTime travelDate(2009, 01, 01);

    _itin.setTravelDate(travelDate);

    CPPUNIT_ASSERT_EQUAL(travelDate, _itin.travelDate());
  }

  void testTravelDateWhenTravelDateIsBeforeTicketDate()
  {
    DateTime travelDate(2009, 01, 01);
    DateTime ticketDate(2009, 02, 01);

    _itin.setTravelDate(travelDate);
    _itin.rexTrx() = &_rexTrx;
    _rexTrx.dataHandle().setTicketDate(ticketDate);

    CPPUNIT_ASSERT_EQUAL(ticketDate, _itin.travelDate());
  }

  void testTravelDateWhenTicketDateIsBeforeTravelDate()
  {
    DateTime travelDate(2009, 02, 01);
    DateTime ticketDate(2009, 01, 01);

    _itin.setTravelDate(travelDate);
    _itin.rexTrx() = &_rexTrx;
    _rexTrx.dataHandle().setTicketDate(ticketDate);

    CPPUNIT_ASSERT_EQUAL(travelDate, _itin.travelDate());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExcItinTest);
}
