#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PricingTrx.h"
#include "ItinAnalyzer/test/ItinAnalyzerServiceTestShoppingCommon.h"
#include "ItinAnalyzer/FamilyLogicSplitter.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class ItinAnalyzerService_FindCOSKeysTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ItinAnalyzerService_FindCOSKeysTest);
  CPPUNIT_TEST(testItinAnalyzerService_FindCOSKeys_noKeys);
  CPPUNIT_TEST(testItinAnalyzerService_FindCOSKeys_oneKey);
  CPPUNIT_TEST(testItinAnalyzerService_FindCOSKeys_twoKeys);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _ia = _memHandle.insert(new ItinAnalyzerServiceDerived(*_memHandle.create<MockTseServer>()));
    _trx = ItinAnalyzerServiceTestShoppingCommon::createTrx(
        "/vobs/atseintl/test/testdata/data/PricingTrx_01.xml");
  }

  void tearDown() { _memHandle.clear(); }

  void testItinAnalyzerService_FindCOSKeys_noKeys()
  {
    int expected = 2;
    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");

    itin->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");
    itin->travelSeg().push_back(seg);
    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    itin->travelSeg().push_back(seg);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin->travelSeg().size()));

    _trx->originDest().clear();
    FamilyLogicSplitter splitter(*_trx, 1);
    std::vector<PricingTrx::ClassOfServiceKey> keys = splitter.findAllCOSKeys(itin);

    expected = 0;
    int actual = keys.size();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testItinAnalyzerService_FindCOSKeys_oneKey()
  {
    int expected = 2;
    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");

    itin->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");
    itin->travelSeg().push_back(seg);
    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    itin->travelSeg().push_back(seg);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin->travelSeg().size()));

    while (_trx->originDest().size() > 1)
    {
      _trx->originDest().pop_back();
    }

    FamilyLogicSplitter splitter(*_trx, 1);
    std::vector<PricingTrx::ClassOfServiceKey> keys = splitter.findAllCOSKeys(itin);

    expected = 1;
    int actual = keys.size();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testItinAnalyzerService_FindCOSKeys_twoKeys()
  {
    int expected = 2;
    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");

    itin->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");
    itin->travelSeg().push_back(seg);
    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    itin->travelSeg().push_back(seg);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin->travelSeg().size()));

    FamilyLogicSplitter splitter(*_trx, 1);
    std::vector<PricingTrx::ClassOfServiceKey> keys = splitter.findAllCOSKeys(itin);

    expected = 2;
    int actual = keys.size();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

private:
  ItinAnalyzerServiceDerived* _ia;
  PricingTrx* _trx;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ItinAnalyzerService_FindCOSKeysTest);
}
