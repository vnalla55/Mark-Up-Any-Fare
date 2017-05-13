
#include "DataModel/PricingTrx.h"
#include "ItinAnalyzer/FamilyLogicSplitter.h"
#include "ItinAnalyzer/test/ItinAnalyzerServiceTestShoppingCommon.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class ItinAnalyzerService_CheckItinFamilyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ItinAnalyzerService_CheckItinFamilyTest);
  CPPUNIT_TEST(testItinAnalyzerService_CheckItinFamily_ValidFamily);
  CPPUNIT_SKIP_TEST(testItinAnalyzerService_CheckItinFamily_InValidFamily);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _ia = _memHandle.insert(new ItinAnalyzerServiceDerived(*_memHandle.create<MockTseServer>()));
    _trx = ItinAnalyzerServiceTestShoppingCommon::createTrx(
        "/vobs/atseintl/test/testdata/data/PricingTrx_01.xml");
  }

  void tearDown() { _memHandle.clear(); }

  void testItinAnalyzerService_CheckItinFamily_ValidFamily()
  {
    ClassOfService cos[4];
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[0], "Y", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[1], "J", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[2], "G", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[3], "T", 9);

    ClassOfServiceList cosList;
    cosList.push_back(&cos[0]);
    cosList.push_back(&cos[1]);

    Itin* itin1 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin2 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin3 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin4 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");

    itin1->travelSeg().clear();
    itin2->travelSeg().clear();
    itin3->travelSeg().clear();
    itin4->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");

    itin1->travelSeg().push_back(seg);
    itin2->travelSeg().push_back(seg);
    itin3->travelSeg().push_back(seg);
    itin4->travelSeg().push_back(seg);

    itin1->origBooking().push_back(&cos[0]);
    itin2->origBooking().push_back(&cos[0]);
    itin3->origBooking().push_back(&cos[0]);
    itin4->origBooking().push_back(&cos[0]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");

    itin1->travelSeg().push_back(seg);
    itin2->travelSeg().push_back(seg);
    itin3->travelSeg().push_back(seg);
    itin4->travelSeg().push_back(seg);

    itin1->origBooking().push_back(&cos[0]);
    itin2->origBooking().push_back(&cos[0]);
    itin3->origBooking().push_back(&cos[0]);
    itin4->origBooking().push_back(&cos[0]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    int expected = 2;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin1->travelSeg().size()));
    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin2->travelSeg().size()));
    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin3->travelSeg().size()));
    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin4->travelSeg().size()));

    CPPUNIT_ASSERT(itin1->getSimilarItins().empty());
    CPPUNIT_ASSERT(itin2->getSimilarItins().empty());
    CPPUNIT_ASSERT(itin3->getSimilarItins().empty());
    CPPUNIT_ASSERT(itin4->getSimilarItins().empty());

    itin1->addSimilarItin(itin2);
    itin1->addSimilarItin(itin3);
    itin1->addSimilarItin(itin4);

    std::vector<Itin*> splittedItins;

    expected = 0;

    FamilyLogicSplitter splitter(*_trx, 1);
    splitter.checkItinFamily(itin1, splittedItins, false, false);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(splittedItins.size()));
  }

  void testItinAnalyzerService_CheckItinFamily_InValidFamily()
  {
    ClassOfService cos[4];
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[0], "Y", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[1], "J", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[2], "G", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[3], "T", 9);

    ClassOfServiceList cosList;
    cosList.push_back(&cos[0]);
    cosList.push_back(&cos[1]);

    Itin* itin1 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin2 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin3 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin4 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");

    itin1->travelSeg().clear();
    itin2->travelSeg().clear();
    itin3->travelSeg().clear();
    itin4->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");

    itin1->travelSeg().push_back(seg);
    itin2->travelSeg().push_back(seg);
    itin3->travelSeg().push_back(seg);
    itin4->travelSeg().push_back(seg);

    itin1->origBooking().push_back(&cos[3]);
    itin2->origBooking().push_back(&cos[0]);
    itin3->origBooking().push_back(&cos[0]);
    itin4->origBooking().push_back(&cos[0]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");

    itin1->travelSeg().push_back(seg);
    itin2->travelSeg().push_back(seg);
    itin3->travelSeg().push_back(seg);
    itin4->travelSeg().push_back(seg);

    itin1->origBooking().push_back(&cos[0]);
    itin2->origBooking().push_back(&cos[0]);
    itin3->origBooking().push_back(&cos[0]);
    itin4->origBooking().push_back(&cos[0]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    int expected = 2;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin1->travelSeg().size()));
    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin2->travelSeg().size()));
    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin3->travelSeg().size()));
    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin4->travelSeg().size()));

    CPPUNIT_ASSERT(itin1->getSimilarItins().empty());
    CPPUNIT_ASSERT(itin2->getSimilarItins().empty());
    CPPUNIT_ASSERT(itin3->getSimilarItins().empty());
    CPPUNIT_ASSERT(itin4->getSimilarItins().empty());

    itin1->addSimilarItin(itin2);
    itin1->addSimilarItin(itin3);
    itin1->addSimilarItin(itin4);

    std::vector<Itin*> splittedItins;

    expected = 1;
    FamilyLogicSplitter splitter(*_trx, 1);

    splitter.checkItinFamily(itin1, splittedItins, false, false);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(splittedItins.size()));
  }

private:
  ItinAnalyzerServiceDerived* _ia;
  PricingTrx* _trx;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ItinAnalyzerService_CheckItinFamilyTest);
}
