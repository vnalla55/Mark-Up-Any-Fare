#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag942Collector.h"
#include "Pricing/Shopping/Diversity/DmcCarrierRequirement.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/LegsBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"
#include "Pricing/Shopping/PQ/test/TestPQItem.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

// Mocks
class ItinStatisticMock : public ItinStatistic
{
public:
  ItinStatisticMock(ShoppingTrx& trx) : ItinStatistic(trx)
  {
    std::fill(_buckets, _buckets + Diversity::BUCKET_COUNT, 0);
  }

  void setCarrierDiversity(CarrierCode carrier, size_t size) { _carrierDiversity[carrier] = size; }
};

class ContextMock : public DmcRequirementsSharedContext
{
public:
  ContextMock(Diversity& diversity, ItinStatistic& stats, DiagCollector* dc, ShoppingTrx& trx)
    : DmcRequirementsSharedContext(diversity, stats, dc, trx)
  {
  }

  void printRequirements(bool bucketsOnly) {};
  void printCarriersRequirements(bool directOnly = false) {};
};

namespace
{

// =================================
// LEGS DATA
// =================================

DateTime obDate = DateTime(2013, 06, 01);
DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "LH", DT(obDate, 6), DT(obDate, 7) }, // 1h
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 5), DT(obDate, 8) }, // 3h
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }, // 1h
  { 1, 1, "AA", "DFW", "JFK", "LH", DT(ibDate, 11), DT(ibDate, 13) } // 2h
};
#undef DT

} // anonymous namespace

class DmcCarrierRequirementTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DmcCarrierRequirementTest);

  CPPUNIT_TEST(testGetStatusAfterCutoff);
  CPPUNIT_TEST(testGetStatusBeforeCutoffNeedCarriers);
  CPPUNIT_TEST(testGetStatusBeforeCutoffDontNeedCarriers);

  CPPUNIT_TEST(testGetCombinationCouldSatisfyInterline);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyOnlineDontNeed);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyOnlineNeed);

  CPPUNIT_TEST(testGetPQItemCouldSatisfySPCRFMPNeed);
  CPPUNIT_TEST(testGetPQItemCouldSatisfySPCRFMPDontNeed);
  CPPUNIT_TEST(testGetPQItemCouldSatisfyNeed);
  CPPUNIT_TEST(testGetPQItemCouldSatisfyDontNeed);
  CPPUNIT_TEST(testGetPQItemCouldSatisfyApplCxrs);
  CPPUNIT_TEST(testGetPQItemCouldSatisfyApplCxrsDontNeed);
  /*
  CPPUNIT_TEST(testAdjustmentNotCollectedEnaugh);
  CPPUNIT_TEST(testAdjustmentNotNeeded);*/

  CPPUNIT_TEST(testPrint);

  CPPUNIT_TEST_SUITE_END();

public:
  DmcCarrierRequirementTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    TSE_ASSERT(_trx);
    initTrx();

    _dc = new DiagCollector();
    _dc->activate();

    _diversityTestUtil = _memHandle.create<shpq::DiversityTestUtil>(_trx->diversity());
    _diversityTestUtil->setNumberOfOptionsToGenerate(20);

    _stats = _memHandle.create<ItinStatisticMock>(*_trx);
    _context = _memHandle.create<ContextMock>(_trx->diversity(), *_stats, _dc, *_trx);
    _requirement = _memHandle.create<DmcCarrierRequirement>(*_context);
  }

  void tearDown()
  {
    _memHandle.clear();
    delete _dc;
  }

  void testGetStatusBeforeCutoffNeedCarriers()
  {
    _trx->diversity().setOptionsPerCarrier("AA", 3);
    _trx->diversity().setOptionsPerCarrier("LH", 2);

    _stats->setCarrierDiversity("AA", 2);
    _stats->setCarrierDiversity("LH", 0);

    CPPUNIT_ASSERT_EQUAL(0 | DmcRequirement::NEED_CARRIERS, _requirement->getStatus());
  }

  void testGetStatusBeforeCutoffDontNeedCarriers()
  {
    _trx->diversity().setOptionsPerCarrier("AA", 3);
    _trx->diversity().setOptionsPerCarrier("LH", 2);

    _stats->setCarrierDiversity("AA", 3);
    _stats->setCarrierDiversity("LH", 2);

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getStatus());
  }

  void testGetStatusAfterCutoff()
  {
    _trx->diversity().setOptionsPerCarrier("AA", 3);
    _trx->diversity().setOptionsPerCarrier("LH", 2);

    _stats->setCarrierDiversity("AA", 2);
    _stats->setCarrierDiversity("LH", 0);

    _requirement->setFareCutoffReached();

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getStatus());
  }

  void testGetCombinationCouldSatisfyInterline()
  {
    std::vector<int> sopVec(2);
    sopVec[0] = 0;
    sopVec[1] = 0;

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(sopVec, 0.0));

    _requirement->setFareCutoffReached();
    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(sopVec, 0.0));
  }

  void testGetCombinationCouldSatisfyOnlineDontNeed()
  {
    _trx->diversity().setOptionsPerCarrier("AA", 2);
    _stats->setCarrierDiversity("AA", 2);

    std::vector<int> sopVec(1);
    sopVec[0] = 0;

    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(sopVec, 0.0));

    _requirement->setFareCutoffReached();
    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(sopVec, 0.0));
  }

  void testGetCombinationCouldSatisfyOnlineNeed()
  {
    _trx->diversity().setOptionsPerCarrier("AA", 3);
    _stats->setCarrierDiversity("AA", 1);

    std::vector<int> sopVec(2);
    sopVec[0] = 0;
    sopVec[1] = 1;

    CPPUNIT_ASSERT_EQUAL(0 | DmcRequirement::NEED_CARRIERS,
                         _requirement->getCombinationCouldSatisfy(sopVec, 0.0));

    _requirement->setFareCutoffReached();
    CPPUNIT_ASSERT_EQUAL(0, _requirement->getCombinationCouldSatisfy(sopVec, 0.0));
  }

  void testGetPQItemCouldSatisfySPCRFMPNeed()
  {
    using namespace shpq;

    SoloPQItem::SoloPQItemLevel levels[] = { SoloPQItem::SP_LEVEL, SoloPQItem::CR_LEVEL,
                                             SoloPQItem::FMP_LEVEL };

    _trx->diversity().setOptionsPerCarrier("AA", 1);
    _stats->setCarrierDiversity("AA", 0);

    MoneyAmount score = 0.0;
    SolutionPattern* pattern = 0;

    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      CPPUNIT_ASSERT_EQUAL(0 | DmcRequirement::NEED_CARRIERS,
                           _requirement->getPQItemCouldSatisfy(pqItem));
    }

    _requirement->setFareCutoffReached();
    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testGetPQItemCouldSatisfySPCRFMPDontNeed()
  {
    using namespace shpq;

    SoloPQItem::SoloPQItemLevel levels[] = { SoloPQItem::SP_LEVEL, SoloPQItem::CR_LEVEL,
                                             SoloPQItem::FMP_LEVEL };

    MoneyAmount score = 0.0;
    SolutionPattern* pattern = 0;

    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }

    _requirement->setFareCutoffReached();
    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testGetPQItemCouldSatisfyNeed()
  {
    using namespace shpq;

    SoloPQItem::SoloPQItemLevel levels[] = { SoloPQItem::CRC_LEVEL, SoloPQItem::FPF_LEVEL };

    _trx->diversity().setOptionsPerCarrier("AA", 1);
    _stats->setCarrierDiversity("AA", 0);

    MoneyAmount score = 0.0;
    SolutionPattern* pattern = 0;

    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }

    _requirement->setFareCutoffReached();
    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testGetPQItemCouldSatisfyDontNeed()
  {
    using namespace shpq;

    SoloPQItem::SoloPQItemLevel levels[] = { SoloPQItem::CRC_LEVEL, SoloPQItem::FPF_LEVEL };

    _trx->diversity().setOptionsPerCarrier("AA", 1);
    _stats->setCarrierDiversity("AA", 1);

    MoneyAmount score = 0.0;
    SolutionPattern* pattern = 0;

    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }

    _requirement->setFareCutoffReached();
    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testGetPQItemCouldSatisfyApplCxrs()
  {
    using namespace shpq;
    SoloPQItem::SoloPQItemLevel levels[] = { SoloPQItem::CRC_LEVEL, SoloPQItem::FPF_LEVEL };
    MoneyAmount score = 0.0;
    SolutionPattern* pattern = 0;

    _trx->diversity().setOptionsPerCarrier("AA", 1);
    _stats->setCarrierDiversity("AA", 0);

    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
      test::TestPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);
      pqItem->addApplicableCxr("AA");
      CPPUNIT_ASSERT_EQUAL(0 | DmcRequirement::NEED_CARRIERS,
                           _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testGetPQItemCouldSatisfyApplCxrsDontNeed()
  {
    using namespace shpq;
    SoloPQItem::SoloPQItemLevel levels[] = { SoloPQItem::CRC_LEVEL, SoloPQItem::FPF_LEVEL };
    MoneyAmount score = 0.0;
    SolutionPattern* pattern = 0;

    _trx->diversity().setOptionsPerCarrier("AA", 1);
    _stats->setCarrierDiversity("AA", 1);

    for (const SoloPQItem::SoloPQItemLevel& level : levels)
    {
        test::TestPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);
        pqItem->addApplicableCxr("AA");
        CPPUNIT_ASSERT_EQUAL(0, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  /*
    void testAdjustmentNotCollectedEnaugh()
    {
      _trx->diversity().setOptionsPerCarrier("AA", 3);
      _trx->diversity().setOptionsPerCarrier("LH", 2);
      _trx->diversity().setOptionsPerCarrier("LT", 1);

      _stats->setCarrierDiversity("AA", 2);
      _stats->setCarrierDiversity("LH", 0);
      _stats->setCarrierDiversity("LT", 1);

      _requirement->adjustCarrierRequirementsIfNeeded(7, 4);

      CPPUNIT_ASSERT_EQUAL(3, _tr->diversity()->getOptionsPerCarrier("AA"));
      CPPUNIT_ASSERT_EQUAL(2, _tr->diversity()->getOptionsPerCarrier("LH"));
      CPPUNIT_ASSERT_EQUAL(1, _tr->diversity()->getOptionsPerCarrier("LT"));
    }


    void testAdjustmentNotNeeded()
    {
      _trx->diversity().setOptionsPerCarrier("AA", 3);
      _trx->diversity().setOptionsPerCarrier("LH", 2);
      _trx->diversity().setOptionsPerCarrier("LT", 1);

      _stats->setCarrierDiversity("AA", 0);
      _stats->setCarrierDiversity("LH", 0);
      _stats->setCarrierDiversity("LT", 5);

      _requirement->adjustCarrierRequirementsIfNeeded(7, 4);

      CPPUNIT_ASSERT_EQUAL(3, _tr->diversity()->getOptionsPerCarrier("AA"));
      CPPUNIT_ASSERT_EQUAL(2, _tr->diversity()->getOptionsPerCarrier("LH"));
      CPPUNIT_ASSERT_EQUAL(1, _tr->diversity()->getOptionsPerCarrier("LT"));
    }
  */

  void testPrint()
  {
    _trx->diversity().setOptionsPerCarrier("AA", 3);
    _trx->diversity().setOptionsPerCarrier("LH", 2);

    _stats->setCarrierDiversity("AA", 2);
    _stats->setCarrierDiversity("LH", 0);

    _requirement->print();

    std::string expected = " AA[2/3] LH[0/2]\n";

    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
  }

  void testPrintNoOnlineCarriers()
  {
    _requirement->print();

    std::string expected = " no online carriers present\n";

    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
  }

private:
  TestMemHandle _memHandle;

  ShoppingTrx* _trx;
  DiagCollector* _dc;
  ItinStatisticMock* _stats;
  DmcRequirementsSharedContext* _context;
  DmcCarrierRequirement* _requirement;

  shpq::DiversityTestUtil* _diversityTestUtil;

  void initTrx()
  {
    // init legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DmcCarrierRequirementTest);

} // tse
