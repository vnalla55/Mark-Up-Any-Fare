#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PricingTrx.h"
#include "ItinAnalyzer/test/ItinAnalyzerServiceTestShoppingCommon.h"
#include "ItinAnalyzer/FamilyLogicSplitter.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class ItinAnalyzerService_CheckItinTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ItinAnalyzerService_CheckItinTest);
  CPPUNIT_TEST(testItinAnalyzerService_ValidItin);
  CPPUNIT_TEST(testItinAnalyzerService_BkcNotAvailable);
  CPPUNIT_TEST(testItinAnalyzerService_NotEnoughSeats);
  CPPUNIT_TEST(testItinAnalyzerService_ValidItinWithArunk);
  CPPUNIT_TEST(testItinAnalyzerService_ItinWithArunk_noBkcAvailable);
  CPPUNIT_TEST(testItinAnalyzerService_CreateFamily);
  CPPUNIT_TEST(testItinAnalyzerService_CreateFamily_OnlyMother);
  CPPUNIT_TEST(testItinAnalyzerService_CreateFamily_NoFamily);
  CPPUNIT_TEST(testItinAnalyzerService_CleanInvalidatedItins_OneItin);
  CPPUNIT_TEST(testItinAnalyzerService_CleanInvalidatedItins_TwoItins);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _ia = _memHandle.insert(new ItinAnalyzerServiceDerived(*_memHandle.create<MockTseServer>()));
    _trx = ItinAnalyzerServiceTestShoppingCommon::createTrx(
        "/vobs/atseintl/test/testdata/data/PricingTrx_01.xml");
  }

  void tearDown() { _memHandle.clear(); }

  void testAddNewMothers()
  {
    PricingTrx trx;
    FamilyLogicSplitter splitter(trx, 1);
    Itin itin1, itin2, itin3, itin4, itin5;
    std::vector<Itin*> newMothers, itinsToSplit;

    itinsToSplit.push_back(&itin1);
    itinsToSplit.push_back(&itin2);
    itinsToSplit.push_back(&itin3);
    itinsToSplit.push_back(&itin4);
    itinsToSplit.push_back(&itin5);

    std::stringstream diag;

    splitter.addNewMothers(itinsToSplit, newMothers, 1, 0, diag);
    CPPUNIT_ASSERT_EQUAL(size_t(1), newMothers.size());
    CPPUNIT_ASSERT_EQUAL(size_t(4), newMothers[0]->getSimilarItins().size());

    newMothers[0]->clearSimilarItins();
    newMothers.clear();

    splitter.addNewMothers(itinsToSplit, newMothers, 2, 0, diag);
    CPPUNIT_ASSERT_EQUAL(size_t(2), newMothers.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), newMothers[0]->getSimilarItins().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), newMothers[1]->getSimilarItins().size());
  }
  void testItinAnalyzerService_ValidItin()
  {
    int expected = 2;
    ClassOfService cos[2];
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[0], "Y", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[1], "J", 9);

    ClassOfServiceList cosList;
    cosList.push_back(&cos[0]);
    cosList.push_back(&cos[1]);

    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");

    itin->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[0]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[0]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin->travelSeg().size()));

    bool expected_Result = true;
    FamilyLogicSplitter splitter(*_trx, 1);
    bool actual_Result = splitter.checkItin(itin, itin->origBooking());

    CPPUNIT_ASSERT_EQUAL(expected_Result, actual_Result);
  }

  void testItinAnalyzerService_BkcNotAvailable()
  {
    int expected = 2;
    ClassOfService cos[4];
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[0], "Y", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[1], "J", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[2], "G", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[3], "T", 9);

    ClassOfServiceList cosList;
    cosList.push_back(&cos[0]);
    cosList.push_back(&cos[1]);

    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");

    itin->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[0]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[3]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin->travelSeg().size()));

    bool expected_Result = false;
    FamilyLogicSplitter splitter(*_trx, 1);
    bool actual_Result = splitter.checkItin(itin, itin->origBooking());

    CPPUNIT_ASSERT_EQUAL(expected_Result, actual_Result);
  }

  void testItinAnalyzerService_NotEnoughSeats()
  {
    int expected = 2;
    ClassOfService cos[4];
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[0], "Y", 0);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[1], "J", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[2], "Y", 1);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[3], "T", 9);

    ClassOfServiceList cosList;
    cosList.push_back(&cos[0]);
    cosList.push_back(&cos[1]);

    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");

    itin->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[0]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[3]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin->travelSeg().size()));

    bool expected_Result = false;
    FamilyLogicSplitter splitter(*_trx, 1);
    bool actual_Result = splitter.checkItin(itin, itin->origBooking());

    CPPUNIT_ASSERT_EQUAL(expected_Result, actual_Result);
  }

  void testItinAnalyzerService_ValidItinWithArunk()
  {
    int expected = 3;
    ClassOfService cos[4];
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[0], "Y", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[1], "J", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[2], "Y", 1);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[3], "T", 1);

    ClassOfServiceList cosList;
    cosList.push_back(&cos[0]);
    cosList.push_back(&cos[1]);

    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_01.xml");

    itin->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");
    seg->originalId() = 1;
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[2]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    ArunkSeg* seg1 = ItinAnalyzerServiceTestShoppingCommon::buildArunkSegment(
        "/vobs/atseintl/test/testdata/data/ArunkSegLAX_LAX.xml");
    seg1->originalId() = 2;
    seg1->segmentType() = Arunk;
    itin->travelSeg().push_back(seg1);
    // itin->origBooking().push_back(&cos[2]);

    // avail for arunk to solve throwing an exception
    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg1, cosList);

    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    seg->originalId() = 3;
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[2]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin->travelSeg().size()));

    bool expected_Result = true;
    FamilyLogicSplitter splitter(*_trx, 1);
    bool actual_Result = splitter.checkItin(itin, itin->origBooking());

    CPPUNIT_ASSERT_EQUAL(expected_Result, actual_Result);
  }

  void testItinAnalyzerService_ItinWithArunk_noBkcAvailable()
  {
    int expected = 3;
    ClassOfService cos[4];
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[0], "Y", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[1], "J", 9);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[2], "Y", 1);
    ItinAnalyzerServiceTestShoppingCommon::setupClassOfService(cos[3], "T", 1);

    ClassOfServiceList cosList;
    cosList.push_back(&cos[0]);
    cosList.push_back(&cos[1]);

    // build itin
    Itin* itin = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_01.xml");

    itin->travelSeg().clear();

    AirSeg* seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");
    seg->originalId() = 1;
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[2]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    ArunkSeg* seg1 = ItinAnalyzerServiceTestShoppingCommon::buildArunkSegment(
        "/vobs/atseintl/test/testdata/data/ArunkSegLAX_LAX.xml");
    seg1->originalId() = 2;
    seg1->segmentType() = Arunk;
    itin->travelSeg().push_back(seg1);
    // itin->origBooking().push_back(&cos[2]);

    // avail for arunk to solve throwing an exception
    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg1, cosList);

    seg = ItinAnalyzerServiceTestShoppingCommon::buildSegment(
        "/vobs/atseintl/test/testdata/data/AirSegLAX_DFW.xml");
    seg->originalId() = 3;
    itin->travelSeg().push_back(seg);
    itin->origBooking().push_back(&cos[3]);

    ItinAnalyzerServiceTestShoppingCommon::addAvailabilityMapItem(*_trx, seg, cosList);

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itin->travelSeg().size()));

    bool expected_Result = false;
    FamilyLogicSplitter splitter(*_trx, 1);
    bool actual_Result = splitter.checkItin(itin, itin->origBooking());

    CPPUNIT_ASSERT_EQUAL(expected_Result, actual_Result);
  }

  void testItinAnalyzerService_CreateFamily()
  {
    // build itin
    Itin* itin1 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin2 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_01.xml");
    Itin* itin3 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin4 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_01.xml");

    std::vector<SimilarItinData> itinVec;

    itinVec.emplace_back(itin1);
    itinVec.emplace_back(itin2);
    itinVec.emplace_back(itin3);
    itinVec.emplace_back(itin4);

    int expected = 4;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itinVec.size()));
    FamilyLogicSplitter splitter(*_trx, 1);
    Itin* mother = splitter.createFamily(itinVec);

    CPPUNIT_ASSERT(mother != NULL);

    expected = 3;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(mother->getSimilarItins().size()));
  }

  void testItinAnalyzerService_CreateFamily_OnlyMother()
  {
    // build itin
    Itin* itin1 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");

    std::vector<SimilarItinData> itinVec;

    itinVec.emplace_back(itin1);

    int expected = 1;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itinVec.size()));
    FamilyLogicSplitter splitter(*_trx, 1);
    Itin* mother = splitter.createFamily(itinVec);

    CPPUNIT_ASSERT(mother != NULL);

    expected = 0;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(mother->getSimilarItins().size()));
  }

  void testItinAnalyzerService_CreateFamily_NoFamily()
  {
    std::vector<SimilarItinData> itinVec;

    int expected = 0;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itinVec.size()));
    FamilyLogicSplitter splitter(*_trx, 1);
    Itin* mother = splitter.createFamily(itinVec);

    CPPUNIT_ASSERT(mother == NULL);
  }

  void testItinAnalyzerService_CleanInvalidatedItins_OneItin()
  {
    // build itin
    Itin* itin1 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin2 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_01.xml");
    Itin* itin3 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin4 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_01.xml");

    std::vector<SimilarItinData> itinVec;
    std::vector<Itin*> invalidItinVec;

    itinVec.emplace_back(itin1);
    itinVec.emplace_back(itin2);
    itinVec.emplace_back(itin3);
    itinVec.emplace_back(itin4);

    invalidItinVec.push_back(itin3);

    int expected = 4;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itinVec.size()));
    FamilyLogicSplitter splitter(*_trx, 1);
    Itin* mother = splitter.createFamily(itinVec);

    CPPUNIT_ASSERT(mother != NULL);

    expected = 3;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(mother->getSimilarItins().size()));

    splitter.cleanInvalidatedItins(mother, invalidItinVec);

    expected = 2;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(mother->getSimilarItins().size()));
  }

  void testItinAnalyzerService_CleanInvalidatedItins_TwoItins()
  {
    // build itin
    Itin* itin1 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin2 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_01.xml");
    Itin* itin3 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX.xml");
    Itin* itin4 = ItinAnalyzerServiceTestShoppingCommon::buildItin(
        "/vobs/atseintl/test/testdata/data/ItinDFW_LAX_Arunk_01.xml");

    std::vector<SimilarItinData> itinVec;
    std::vector<Itin*> invalidItinVec;

    itinVec.emplace_back(itin1);
    itinVec.emplace_back(itin2);
    itinVec.emplace_back(itin3);
    itinVec.emplace_back(itin4);

    invalidItinVec.push_back(itin3);
    invalidItinVec.push_back(itin2);

    int expected = 4;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(itinVec.size()));
    FamilyLogicSplitter splitter(*_trx, 1);
    Itin* mother = splitter.createFamily(itinVec);

    CPPUNIT_ASSERT(mother != NULL);

    expected = 3;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(mother->getSimilarItins().size()));

    splitter.cleanInvalidatedItins(mother, invalidItinVec);

    expected = 1;

    CPPUNIT_ASSERT_EQUAL(expected, static_cast<int>(mother->getSimilarItins().size()));
  }

private:
  ItinAnalyzerServiceDerived* _ia;
  PricingTrx* _trx;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ItinAnalyzerService_CheckItinTest);
}
