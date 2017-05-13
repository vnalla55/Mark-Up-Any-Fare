
// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/Diversity/DsciDiversityItinerarySwapperAdapter.h"

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/PQ/SOPCombination.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/LegsBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"

#include <boost/assign/std/vector.hpp>
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

  void setTODBucketCount(const size_t bucket, size_t size)
  {
    if (_todBuckets.size() < bucket + 1)
      _todBuckets.resize(bucket + 1);

    _todBuckets[bucket] = size;
  }

  void setAvgPrice(MoneyAmount price) { _avgPrice = price; }
  void setAvgDuration(double duration) { _avgDuration = duration; }

  void setTotalOptionsCount(size_t count) { _totalOptionsCount = count; }
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
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 5), DT(obDate, 7) }, // 2h
  { 0, 2, "AA", "JFK", "FRA", "LH", DT(obDate, 4), DT(obDate, 6) }, // 2h
  { 0, 2, "AA", "FRA", "DFW", "LH", DT(obDate, 7), DT(obDate, 8) }, // 1h
  { 0, 3, "AA", "JFK", "DFW", "LH", DT(obDate, 9), DT(obDate, 10) }, // 1h
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }, // 1h
  { 1, 1, "AA", "DFW", "JFK", "LH", DT(ibDate, 11), DT(ibDate, 13) } // 2h
};
#undef DT

} // anonymous namespace

class DsciDiversityItinerarySwapperAdapterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DsciDiversityItinerarySwapperAdapterTest);

  CPPUNIT_TEST(testIsBetterByTODDistWorse);
  CPPUNIT_TEST(testIsBetterByTODDistEquivSameBucket);
  CPPUNIT_TEST(testIsBetterByTODDistEquivSameDist);
  CPPUNIT_TEST(testIsBetterByTODDistBetter);

  CPPUNIT_TEST(testIsBetterBySolutionScoreGoldWorse);
  CPPUNIT_TEST(testIsBetterBySolutionScoreGoldBetter);
  CPPUNIT_TEST(testIsBetterBySolutionScoreJunkEquiv);
  CPPUNIT_TEST(testIsBetterBySolutionScoreJunkBetter);
  CPPUNIT_TEST(testIsBetterBySolutionScoreLuxuryEquiv);
  CPPUNIT_TEST(testIsBetterBySolutionScoreLuxuryBetter);
  CPPUNIT_TEST(testIsBetterBySolutionScoreUglyEquiv);
  CPPUNIT_TEST(testIsBetterBySolutionScoreUglyBetter);

  CPPUNIT_TEST_SUITE_END();

public:
  typedef DsciDiversityItinerarySwapperAdapter::Operand Operand;

  DsciDiversityItinerarySwapperAdapterTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _dc = _memHandle.create<DiagCollector>();
    _dc->activate();

    _trx = TestShoppingTrxFactory::create(
        "/vobs/atseintl/Pricing/Shopping/PQ/test/testdata/ShoppingNGSTrx.xml", true);
    TSE_ASSERT(_trx);
    initTrx();

    _diversityTestUtil = _memHandle.create<shpq::DiversityTestUtil>(_trx->diversity());
    _diversityTestUtil->setNonStopOptionsPerCarrierEnabled(true);
    _diversityTestUtil->setNumberOfOptionsToGenerate(20);

    _stats = _memHandle.create<ItinStatisticMock>(*_trx);
    _stats->setTotalOptionsCount(20);

    _lhsSopIdxVec.clear();
    _rhsSopIdxVec.clear();

    _lhs = _memHandle.create<Operand>();
    _rhs = _memHandle.create<Operand>();
  }

  void tearDown() { _memHandle.clear(); }

  void testIsBetterByTODDistWorse();
  void testIsBetterByTODDistEquivSameBucket();
  void testIsBetterByTODDistEquivSameDist();
  void testIsBetterByTODDistBetter();

  void testIsBetterBySolutionScoreGoldWorse();
  void testIsBetterBySolutionScoreGoldBetter();
  void testIsBetterBySolutionScoreJunkEquiv();
  void testIsBetterBySolutionScoreJunkBetter();
  void testIsBetterBySolutionScoreLuxuryEquiv();
  void testIsBetterBySolutionScoreLuxuryBetter();
  void testIsBetterBySolutionScoreUglyEquiv();
  void testIsBetterBySolutionScoreUglyBetter();

private:
  TestMemHandle _memHandle;

  ShoppingTrx* _trx;
  ItinStatisticMock* _stats;
  DiagCollector* _dc;
  DsciDiversityItinerarySwapperAdapter* _interpreter;

  shpq::SopIdxVec _lhsSopIdxVec;
  shpq::SopIdxVec _rhsSopIdxVec;

  Operand* _lhs;
  Operand* _rhs;

  shpq::DiversityTestUtil* _diversityTestUtil;

  void initTrx()
  {
    // init legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();
  }

  void initInterpreter(size_t newTODBucket = 0,
                       Diversity::BucketType bucket = Diversity::GOLD,
                       bool isBrandedFaresPath = false)
  {
    _interpreter = _memHandle.create<DsciDiversityItinerarySwapperAdapter>(
        *_trx, *_stats, newTODBucket, bucket, isBrandedFaresPath);
  }

  void initOperand(Operand* operand, shpq::SopIdxVecArg sopVec, MoneyAmount price = 0.0)
  {
    const ShoppingTrx::SchedulingOption* outbound, *inbound;
    SopCombinationUtil::getSops(*_trx, sopVec, &outbound, &inbound);

    operand->_outbound = outbound;
    operand->_inbound = inbound;
    operand->_price = price;
  }
};

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterByTODDistWorse()
{
  using namespace boost::assign; // +=

  typedef std::pair<uint16_t, uint16_t> TODRange;
  std::vector<TODRange> todRanges;
  todRanges += TODRange(0, 330), // 0:00 - 5:30
      TODRange(331, 1439); // 5:31 - 23:59

  std::vector<float> todDistribution;
  todDistribution += 0.5, 0.5;

  _trx->diversity().setTODRanges(todRanges);
  _trx->diversity().setTODDistribution(todDistribution);

  _stats->setTODBucketCount(0, 4);
  _stats->setTODBucketCount(1, 3);

  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0; // tod bucket 1
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 2; // tod bucket 0

  initOperand(_lhs, _lhsSopIdxVec);
  initOperand(_rhs, _rhsSopIdxVec);

  size_t newTODBucket = 0;

  initInterpreter(newTODBucket);

  CPPUNIT_ASSERT_EQUAL(-1, _interpreter->isBetterByTODDist(newTODBucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterByTODDistEquivSameBucket()
{
  using namespace boost::assign; // +=

  typedef std::pair<uint16_t, uint16_t> TODRange;
  std::vector<TODRange> todRanges;
  todRanges += TODRange(0, 330), // 0:00 - 5:30
      TODRange(331, 1439); // 5:31 - 23:59

  std::vector<float> todDistribution;
  todDistribution += 0.5, 0.5;

  _trx->diversity().setTODRanges(todRanges);
  _trx->diversity().setTODDistribution(todDistribution);

  _stats->setTODBucketCount(0, 4);
  _stats->setTODBucketCount(1, 3);

  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 1; // tod bucket 0
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 2; // tod bucket 0

  initOperand(_lhs, _lhsSopIdxVec);
  initOperand(_rhs, _rhsSopIdxVec);

  size_t newTODBucket = 0;

  initInterpreter(newTODBucket);

  CPPUNIT_ASSERT_EQUAL(0, _interpreter->isBetterByTODDist(newTODBucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterByTODDistEquivSameDist()
{
  using namespace boost::assign; // +=

  typedef std::pair<uint16_t, uint16_t> TODRange;
  std::vector<TODRange> todRanges;
  todRanges += TODRange(0, 330), // 0:00 - 5:30
      TODRange(331, 1439); // 5:31 - 23:59

  std::vector<float> todDistribution;
  todDistribution += 0.5, 0.5;

  _trx->diversity().setTODRanges(todRanges);
  _trx->diversity().setTODDistribution(todDistribution);

  _stats->setTODBucketCount(0, 3);
  _stats->setTODBucketCount(1, 4);

  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0; // tod bucket 1
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 2; // tod bucket 0

  initOperand(_lhs, _lhsSopIdxVec);
  initOperand(_rhs, _rhsSopIdxVec);

  size_t newTODBucket = 0;

  initInterpreter(newTODBucket);

  CPPUNIT_ASSERT_EQUAL(0, _interpreter->isBetterByTODDist(newTODBucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterByTODDistBetter()
{
  using namespace boost::assign; // +=

  typedef std::pair<uint16_t, uint16_t> TODRange;
  std::vector<TODRange> todRanges;
  todRanges += TODRange(0, 330), // 0:00 - 5:30
      TODRange(331, 1439); // 5:31 - 23:59

  std::vector<float> todDistribution;
  todDistribution += 0.5, 0.5;

  _trx->diversity().setTODRanges(todRanges);
  _trx->diversity().setTODDistribution(todDistribution);

  _stats->setTODBucketCount(0, 4);
  _stats->setTODBucketCount(1, 3);

  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 2; // tod bucket 0
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 0; // tod bucket 1

  initOperand(_lhs, _lhsSopIdxVec);
  initOperand(_rhs, _rhsSopIdxVec);

  size_t newTODBucket = 0;

  initInterpreter(newTODBucket);

  CPPUNIT_ASSERT_EQUAL(1, _interpreter->isBetterByTODDist(newTODBucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterBySolutionScoreGoldWorse()
{
  _stats->setAvgPrice(1.0);
  _stats->setAvgDuration(1.0);

  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 1;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 2;

  initOperand(_lhs, _lhsSopIdxVec, 150.0);
  initOperand(_rhs, _rhsSopIdxVec, 150.0);

  size_t newTODBucket = 0;
  Diversity::BucketType bucket = Diversity::GOLD;

  initInterpreter(newTODBucket, bucket);

  CPPUNIT_ASSERT_EQUAL(-1, _interpreter->isBetterBySolutionScore(bucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterBySolutionScoreGoldBetter()
{
  _stats->setAvgPrice(1.0);
  _stats->setAvgDuration(1.0);

  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 1;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 3;

  initOperand(_lhs, _lhsSopIdxVec, 150.0);
  initOperand(_rhs, _rhsSopIdxVec, 150.0);

  size_t newTODBucket = 0;
  Diversity::BucketType bucket = Diversity::GOLD;

  initInterpreter(newTODBucket, bucket);

  CPPUNIT_ASSERT_EQUAL(1, _interpreter->isBetterBySolutionScore(bucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterBySolutionScoreJunkEquiv()
{
  _stats->setAvgPrice(1.0);
  _stats->setAvgDuration(1.0);

  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 3;

  initOperand(_lhs, _lhsSopIdxVec, 150.0);
  initOperand(_rhs, _rhsSopIdxVec, 150.0);

  size_t newTODBucket = 0;
  Diversity::BucketType bucket = Diversity::JUNK;

  initInterpreter(newTODBucket, bucket);

  CPPUNIT_ASSERT_EQUAL(0, _interpreter->isBetterBySolutionScore(bucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterBySolutionScoreJunkBetter()
{
  _stats->setAvgPrice(1.0);
  _stats->setAvgDuration(1.0);

  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 3;

  initOperand(_lhs, _lhsSopIdxVec, 150.0);
  initOperand(_rhs, _rhsSopIdxVec, 7.0);

  size_t newTODBucket = 0;
  Diversity::BucketType bucket = Diversity::JUNK;

  initInterpreter(newTODBucket, bucket);

  CPPUNIT_ASSERT_EQUAL(1, _interpreter->isBetterBySolutionScore(bucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterBySolutionScoreLuxuryEquiv()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 3;

  initOperand(_lhs, _lhsSopIdxVec);
  initOperand(_rhs, _rhsSopIdxVec);

  size_t newTODBucket = 0;
  Diversity::BucketType bucket = Diversity::LUXURY;

  initInterpreter(newTODBucket, bucket);

  CPPUNIT_ASSERT_EQUAL(0, _interpreter->isBetterBySolutionScore(bucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterBySolutionScoreLuxuryBetter()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 1;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 0;

  initOperand(_lhs, _lhsSopIdxVec);
  initOperand(_rhs, _rhsSopIdxVec);

  size_t newTODBucket = 0;
  Diversity::BucketType bucket = Diversity::LUXURY;

  initInterpreter(newTODBucket, bucket);

  CPPUNIT_ASSERT_EQUAL(1, _interpreter->isBetterBySolutionScore(bucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterBySolutionScoreUglyEquiv()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 1;

  initOperand(_lhs, _lhsSopIdxVec, 100.0);
  initOperand(_rhs, _rhsSopIdxVec, 100.0);

  size_t newTODBucket = 0;
  Diversity::BucketType bucket = Diversity::UGLY;

  initInterpreter(newTODBucket, bucket);

  CPPUNIT_ASSERT_EQUAL(-1, _interpreter->isBetterBySolutionScore(bucket, *_lhs, *_rhs));
}

void
DsciDiversityItinerarySwapperAdapterTest::testIsBetterBySolutionScoreUglyBetter()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 1;

  initOperand(_lhs, _lhsSopIdxVec, 150.0);
  initOperand(_rhs, _rhsSopIdxVec, 100.0);

  size_t newTODBucket = 0;
  Diversity::BucketType bucket = Diversity::UGLY;

  initInterpreter(newTODBucket, bucket);

  CPPUNIT_ASSERT_EQUAL(1, _interpreter->isBetterBySolutionScore(bucket, *_lhs, *_rhs));
}

CPPUNIT_TEST_SUITE_REGISTRATION(DsciDiversityItinerarySwapperAdapterTest);

} // tse
