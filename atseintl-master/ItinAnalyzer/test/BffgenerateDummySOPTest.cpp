#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/ConfigMan.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DiskCache.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "test/include/MockGlobal.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class BffgenerateDummySOPTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(BffgenerateDummySOPTest);
  CPPUNIT_TEST(NoSOPCreated);
  CPPUNIT_TEST(OneLegCreated_STEP1);
  CPPUNIT_TEST(TwoDummyLegsCreated_STEP3);
  CPPUNIT_TEST(OneLegCreated_STEP4);
  CPPUNIT_TEST(OneLegCreated_STEP6);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _server = _memHandle.create<MockTseServer>();
    DiskCache::initialize(Global::config());
    _memHandle.create<MockDataManager>();
    MockGlobal::setStartTime();

    _memHandle.get(_ffTrx);
    _memHandle.get(_ffTrx->journeyItin());
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  AirSeg* buildSegment(std::string origin,
                       std::string destination,
                       std::string carrier,
                       DateTime departure = DateTime::localTime(),
                       DateTime arrival = DateTime::localTime())
  {
    AirSeg* airSeg;
    _memHandle.get(airSeg);

    airSeg->departureDT() = departure;
    airSeg->arrivalDT() = arrival;

    Loc* locorig, *locdest;
    _memHandle.get(locorig);
    _memHandle.get(locdest);
    locorig->loc() = origin;
    locdest->loc() = destination;

    airSeg->origAirport() = origin;
    airSeg->origin() = locorig;
    airSeg->destAirport() = destination;
    airSeg->destination() = locdest;

    airSeg->carrier() = carrier;

    return airSeg;
  }

  void NoSOPCreated()
  {
    ItinAnalyzerService itin("ITIN_SVC", *_server);

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->legs().size());

    itin.generateDummySOP(*_ffTrx);

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->legs().size());
  }

  void OneLegCreated_STEP1()
  {
    ItinAnalyzerService itin("ITIN_SVC", *_server);

    _ffTrx->journeyItin()->travelSeg().push_back(
        buildSegment("AAA", "BBB", "AA", DateTime(2008, Dec, 10)));

    _ffTrx->bffStep() = FlightFinderTrx::STEP_1;

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->legs().size());

    itin.generateDummySOP(*_ffTrx);

    CPPUNIT_ASSERT_EQUAL(1, (int)_ffTrx->legs().size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ffTrx->legs().front().sop().size());
  }

  void OneLegCreated_STEP4()
  {

    ItinAnalyzerService itin("ITIN_SVC", *_server);

    _ffTrx->journeyItin()->travelSeg().push_back(
        buildSegment("AAA", "BBB", "AA", DateTime(2008, Dec, 10)));

    _ffTrx->bffStep() = FlightFinderTrx::STEP_4;

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->legs().size());

    itin.generateDummySOP(*_ffTrx);

    CPPUNIT_ASSERT_EQUAL(1, (int)_ffTrx->legs().size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ffTrx->legs().front().sop().size());
  }

  void OneLegCreated_STEP6()
  {
    ItinAnalyzerService itin("ITIN_SVC", *_server);

    _ffTrx->journeyItin()->travelSeg().push_back(
        buildSegment("AAA", "BBB", "AA", DateTime(2008, Dec, 10)));

    _ffTrx->bffStep() = FlightFinderTrx::STEP_6;

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->legs().size());

    itin.generateDummySOP(*_ffTrx);

    CPPUNIT_ASSERT_EQUAL(1, (int)_ffTrx->legs().size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ffTrx->legs().front().sop().size());
  }

  void TwoDummyLegsCreated_STEP3()
  {
    ItinAnalyzerService itin("ITIN_SVC", *_server);

    _ffTrx->journeyItin()->travelSeg().push_back(
        buildSegment("AAA", "BBB", "AA", DateTime(2008, Dec, 10)));
    _ffTrx->journeyItin()->travelSeg().push_back(
        buildSegment("BBB", "AAA", "AA", DateTime(2008, Dec, 15)));

    _ffTrx->bffStep() = FlightFinderTrx::STEP_3;

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->legs().size());

    itin.generateDummySOP(*_ffTrx);

    CPPUNIT_ASSERT_EQUAL(2, (int)_ffTrx->legs().size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ffTrx->legs().front().sop().size());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ffTrx->legs().back().sop().size());
  }

private:
  FlightFinderTrx* _ffTrx;
  MockTseServer* _server;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BffgenerateDummySOPTest);

} // tse
