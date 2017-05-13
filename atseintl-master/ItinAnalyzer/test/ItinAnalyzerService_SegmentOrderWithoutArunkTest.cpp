#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include "DataModel/PricingTrx.h"
#include "test/include/TestMemHandle.h"
#include "ItinAnalyzer/test/ItinAnalyzerServiceTestShoppingCommon.h"
#include "ItinAnalyzer/FamilyLogicUtils.h"

namespace tse
{
class ItinAnalyzerService_SegmentOrderWithoutArunkTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ItinAnalyzerService_SegmentOrderWithoutArunkTest);
  CPPUNIT_TEST(testItinAnalyzerService_SegmentOrderWithoutArunk_noArunk);
  CPPUNIT_TEST(testItinAnalyzerService_SegmentOrderWithoutArunk_oneArunk);
  CPPUNIT_TEST(testItinAnalyzerService_SegmentOrderWithoutArunk_twoArunks);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _ia = _memHandle.insert(new ItinAnalyzerServiceDerived(*_memHandle.create<MockTseServer>()));
    _trx = ItinAnalyzerServiceTestShoppingCommon::createTrx(
        "/vobs/atseintl/test/testdata/data/pricingTrx.xml");
  }

  void tearDown() { _memHandle.clear(); }

  void testItinAnalyzerService_SegmentOrderWithoutArunk_noArunk()
  {
    int expected = 2;
    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin->travelSeg().size()));
    TravelSeg* seg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    int actual = FamilyLogicUtils::segmentOrderWithoutArunk(itin, seg);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testItinAnalyzerService_SegmentOrderWithoutArunk_oneArunk()
  {
    int expected_init = 3;
    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_01.xml");
    itin->travelSeg()[1]->segmentType() = Arunk;
    CPPUNIT_ASSERT_EQUAL(expected_init, static_cast<int>(itin->travelSeg().size()));
    TravelSeg* seg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    int actual = FamilyLogicUtils::segmentOrderWithoutArunk(itin, seg);
    int expected = 2;
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testItinAnalyzerService_SegmentOrderWithoutArunk_twoArunks()
  {
    int expected_init = 4;
    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_02.xml");
    itin->travelSeg()[1]->segmentType() = Arunk;
    itin->travelSeg()[3]->segmentType() = Arunk;
    CPPUNIT_ASSERT_EQUAL(expected_init, static_cast<int>(itin->travelSeg().size()));
    TravelSeg* seg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    int actual = FamilyLogicUtils::segmentOrderWithoutArunk(itin, seg);
    int expected = 2;
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

private:
  ItinAnalyzerServiceDerived* _ia;
  PricingTrx* _trx;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(ItinAnalyzerService_SegmentOrderWithoutArunkTest);
}
