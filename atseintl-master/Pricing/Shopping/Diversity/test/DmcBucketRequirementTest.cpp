#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diag942Collector.h"
#include "Pricing/Shopping/Diversity/DmcBucketRequirement.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"
#include "Pricing/Shopping/PQ/test/TestPQItem.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/LegsBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"

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

  void setBucketSize(Diversity::BucketType bucket, size_t size) { _buckets[bucket] = size; }
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

class DmcBucketRequirementTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DmcBucketRequirementTest);

  CPPUNIT_TEST(testGetStatusAfterCutoff);
  CPPUNIT_TEST(testGetStatusBeforeCutoff);

  CPPUNIT_TEST(testGetCombinationCouldSatisfyGoldOW);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyGoldRT);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyLuxuryOW);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyLuxuryRT);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyUglyOW);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyUglyRT);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyJunkOW);
  CPPUNIT_TEST(testGetCombinationCouldSatisfyJunkRT);

  CPPUNIT_TEST(testBucketAdjustment);

  CPPUNIT_TEST(testGetPQItemCouldSatisfyBeforeCutoff);
  CPPUNIT_TEST(testGetPQItemCouldSatisfyAfterCutoff);

  CPPUNIT_TEST(testSetNumberSolutionsRequiredCollectedPositive);
  CPPUNIT_TEST(testSetNumberSolutionsRequiredCollectedNegative);

  CPPUNIT_TEST(testGetBucketStatusGold);
  CPPUNIT_TEST(testGetBucketStatusLuxury);
  CPPUNIT_TEST(testGetBucketStatusUgly);
  CPPUNIT_TEST(testGetBucketStatusJunk);

  CPPUNIT_TEST(testPrint);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = TestShoppingTrxFactory::create(
        "/vobs/atseintl/Pricing/Shopping/PQ/test/testdata/ShoppingNGSTrx.xml", true);
    TSE_ASSERT(_trx);
    initTrx();

    _dc = _memHandle.create<DiagCollector>();
    _dc->activate();

    _diversityTestUtil = _memHandle.create<shpq::DiversityTestUtil>(_trx->diversity());
    _diversityTestUtil->setNonStopOptionsPerCarrierEnabled(true);
    _diversityTestUtil->setNumberOfOptionsToGenerate(20);

    _stats = _memHandle.create<ItinStatisticMock>(*_trx);
    _context = _memHandle.create<ContextMock>(_trx->diversity(), *_stats, _dc, *_trx);
    _requirement = _memHandle.create<DmcBucketRequirement>(*_context);
  }

  void tearDown() { _memHandle.clear(); }

  void testGetStatusBeforeCutoff()
  {
    for (size_t comb = 0; comb < (1 << Diversity::BUCKET_COUNT); ++comb)
    {
      DmcBucketRequirement::Value expected = 0;
      std::vector<Diversity::BucketType> buckets(0);

      prepareBuckets(comb, buckets, expected);
      initBuckets(buckets);

      CPPUNIT_ASSERT_EQUAL(expected, _requirement->getStatus());
    }
  }

  void testGetStatusAfterCutoff()
  {
    for (size_t comb = 0; comb < (1 << Diversity::BUCKET_COUNT); ++comb)
    {
      DmcBucketRequirement::Value expected = 0;
      std::vector<Diversity::BucketType> buckets(0);

      prepareBuckets(comb, buckets, expected);
      initBuckets(buckets);
      _requirement->setFareCutoffReached();

      CPPUNIT_ASSERT_EQUAL(0, _requirement->getStatus());
    }
  }

  void testGetCombinationCouldSatisfyGoldOW()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min
    initGold();

    std::vector<int> sopVec(1);
    sopVec[0] = 0;

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_GOLD),
                         _requirement->getCombinationCouldSatisfy(sopVec, 5.0));
  }

  void testGetCombinationCouldSatisfyGoldRT()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min
    initGold();

    std::vector<int> sopVec(2);
    sopVec[0] = 0;
    sopVec[1] = 0;

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_GOLD),
                         _requirement->getCombinationCouldSatisfy(sopVec, 5.0));
  }

  void testGetCombinationCouldSatisfyLuxuryOW()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min
    initLuxury();

    std::vector<int> sopVec(1);
    sopVec[0] = 0;

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_LUXURY),
                         _requirement->getCombinationCouldSatisfy(sopVec, 15.0));
  }

  void testGetCombinationCouldSatisfyLuxuryRT()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min
    initLuxury();

    std::vector<int> sopVec(2);
    sopVec[0] = 0;
    sopVec[0] = 0;

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_LUXURY),
                         _requirement->getCombinationCouldSatisfy(sopVec, 15.0));
  }

  void testGetCombinationCouldSatisfyUglyOW()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min
    initUgly();

    std::vector<int> sopVec(1);
    sopVec[0] = 1;

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_UGLY),
                         _requirement->getCombinationCouldSatisfy(sopVec, 5.0));
  }

  void testGetCombinationCouldSatisfyUglyRT()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min
    initUgly();

    std::vector<int> sopVec(2);
    sopVec[0] = 0;
    sopVec[1] = 1;

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_UGLY),
                         _requirement->getCombinationCouldSatisfy(sopVec, 5.0));
  }

  void testGetCombinationCouldSatisfyJunkOW()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min
    initJunk();

    std::vector<int> sopVec(1);
    sopVec[0] = 1;

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_JUNK),
                         _requirement->getCombinationCouldSatisfy(sopVec, 15.0));
  }

  void testGetCombinationCouldSatisfyJunkRT()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min
    initJunk();

    std::vector<int> sopVec(2);
    sopVec[0] = 1;
    sopVec[1] = 1;

    CPPUNIT_ASSERT_EQUAL(int(DmcRequirement::NEED_JUNK),
                         _requirement->getCombinationCouldSatisfy(sopVec, 15.0));
  }

  void testBucketAdjustment()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min

    std::vector<Diversity::BucketType> buckets(2);
    buckets[0] = Diversity::GOLD;
    buckets[1] = Diversity::UGLY;
    initBuckets(buckets);

    std::vector<int> sopVec(2);
    sopVec[0] = 0;
    sopVec[0] = 0;

    _requirement->getCombinationCouldSatisfy(sopVec, 15.0);
    CPPUNIT_ASSERT(!(DmcRequirement::NEED_GOLD & _requirement->getStatus()));
    CPPUNIT_ASSERT(!(DmcRequirement::NEED_UGLY & _requirement->getStatus()));
  }

  void testSetNumberSolutionsRequiredCollectedPositive()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min

    std::vector<Diversity::BucketType> buckets(2);
    buckets[0] = Diversity::GOLD;
    buckets[1] = Diversity::UGLY;
    initBuckets(buckets);

    _requirement->setNumberSolutionsRequiredCollected(10, 10, 15.0);
    CPPUNIT_ASSERT(!(DmcRequirement::NEED_GOLD & _requirement->getStatus()));
    CPPUNIT_ASSERT(!(DmcRequirement::NEED_UGLY & _requirement->getStatus()));
  }

  void testSetNumberSolutionsRequiredCollectedNegative()
  {
    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    _trx->diversity().setTravelTimeSeparator(145); // 145 min

    std::vector<Diversity::BucketType> buckets(2);
    buckets[0] = Diversity::GOLD;
    buckets[1] = Diversity::UGLY;
    initBuckets(buckets);

    _requirement->setNumberSolutionsRequiredCollected(10, 6, 15.0);
    CPPUNIT_ASSERT(DmcRequirement::NEED_GOLD & _requirement->getStatus());
    CPPUNIT_ASSERT(DmcRequirement::NEED_UGLY & _requirement->getStatus());
  }

  void testGetPQItemCouldSatisfyBeforeCutoff()
  {
    using namespace shpq;

    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    DmcBucketRequirement::Value expected = 0;
    std::vector<Diversity::BucketType> buckets(0);

    for (size_t comb = 0; comb < (1 << Diversity::BUCKET_COUNT); ++comb)
    {
      expected = 0;
      std::vector<Diversity::BucketType>().swap(buckets);

      prepareBuckets(comb, buckets, expected);
      initBuckets(buckets);

      MoneyAmount score = 5.0;
      SoloPQItem::SoloPQItemLevel level = SoloPQItem::SP_LEVEL;
      SolutionPattern* pattern = 0;
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      CPPUNIT_ASSERT_EQUAL(expected, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testGetPQItemCouldSatisfyAfterCutoff()
  {
    using namespace shpq;

    _trx->diversity().setFareAmountSeparator(10.0); // 10.0 NUC
    DmcBucketRequirement::Value expected = 0;
    std::vector<Diversity::BucketType> buckets(0);

    for (size_t comb = 0; comb < (1 << Diversity::BUCKET_COUNT); ++comb)
    {
      expected = 0;
      std::vector<Diversity::BucketType>().swap(buckets);

      prepareBuckets(comb, buckets, expected);
      initBuckets(buckets);

      MoneyAmount score = 15.0;
      SoloPQItem::SoloPQItemLevel level = SoloPQItem::SP_LEVEL;
      SolutionPattern* pattern = 0;
      SoloPQItem* pqItem = _memHandle.create<test::TestPQItem>(score, level, pattern);

      expected &= ~(DmcRequirement::NEED_GOLD | DmcRequirement::NEED_UGLY);

      CPPUNIT_ASSERT_EQUAL(expected, _requirement->getPQItemCouldSatisfy(pqItem));
    }
  }

  void testGetBucketStatusGold()
  {
    initGold();

    CPPUNIT_ASSERT_EQUAL(-1, _requirement->getBucketStatus(Diversity::GOLD));
  }

  void testGetBucketStatusLuxury()
  {
    initLuxury();

    CPPUNIT_ASSERT_EQUAL(-1, _requirement->getBucketStatus(Diversity::LUXURY));
  }

  void testGetBucketStatusUgly()
  {
    initUgly();

    CPPUNIT_ASSERT_EQUAL(-1, _requirement->getBucketStatus(Diversity::UGLY));
  }

  void testGetBucketStatusJunk()
  {
    initJunk();

    CPPUNIT_ASSERT_EQUAL(-1, _requirement->getBucketStatus(Diversity::JUNK));
  }

  void testPrint()
  {
    std::vector<Diversity::BucketType> buckets(2);
    buckets[0] = Diversity::GOLD;
    buckets[1] = Diversity::LUXURY;

    initBuckets(buckets);
    _requirement->print();

    std::string expected = "\tG:   4/5     U:   0/0     L:   4/5     J:   0/0";

    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
  }

private:
  TestMemHandle _memHandle;

  ShoppingTrx* _trx;
  DiagCollector* _dc;
  ItinStatisticMock* _stats;
  DmcRequirementsSharedContext* _context;
  DmcBucketRequirement* _requirement;

  shpq::DiversityTestUtil* _diversityTestUtil;

  void initTrx()
  {
    // init legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();
  }

  void
  prepareBuckets(const size_t& comb, std::vector<Diversity::BucketType>& buckets, int& expected)
  {
    if (comb & (1 << Diversity::GOLD))
    {
      expected |= DmcRequirement::NEED_GOLD;
      buckets.push_back(Diversity::GOLD);
    }
    if (comb & (1 << Diversity::LUXURY))
    {
      expected |= DmcRequirement::NEED_LUXURY;
      buckets.push_back(Diversity::LUXURY);
    }
    if (comb & (1 << Diversity::UGLY))
    {
      expected |= DmcRequirement::NEED_UGLY;
      buckets.push_back(Diversity::UGLY);
    }
    if (comb & (1 << Diversity::JUNK))
    {
      expected |= DmcRequirement::NEED_JUNK;
      buckets.push_back(Diversity::JUNK);
    }
  }

  void initGold()
  {
    std::vector<Diversity::BucketType> buckets(1);
    buckets[0] = Diversity::GOLD;
    initBuckets(buckets);
  }

  void initLuxury()
  {
    std::vector<Diversity::BucketType> buckets(1);
    buckets[0] = Diversity::LUXURY;
    initBuckets(buckets);
  }

  void initUgly()
  {
    std::vector<Diversity::BucketType> buckets(1);
    buckets[0] = Diversity::UGLY;
    initBuckets(buckets);
  }

  void initJunk()
  {
    std::vector<Diversity::BucketType> buckets(1);
    buckets[0] = Diversity::JUNK;
    initBuckets(buckets);
  }

  void initBuckets(const std::vector<Diversity::BucketType>& buckets)
  {
    _diversityTestUtil->resetBucketDistribution();
    for (const Diversity::BucketType& bucket : buckets)
    {
      _trx->diversity().setBucketDistribution(bucket, 0.25);
      size_t bucketSize = (size_t)(0.25 * _trx->diversity().getNumberOfOptionsToGenerate() - 1);
      _stats->setBucketSize(bucket, bucketSize);
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DmcBucketRequirementTest);

} // tse
