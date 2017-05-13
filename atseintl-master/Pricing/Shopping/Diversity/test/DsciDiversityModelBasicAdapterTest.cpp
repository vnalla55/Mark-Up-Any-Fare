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
#include "Pricing/Shopping/Diversity/DsciDiversityModelBasicAdapter.h"

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/PQ/SOPCombination.h"

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

  void setBucketSize(Diversity::BucketType bucket, size_t size) { _buckets[bucket] = size; }

  void setTODBucketCount(const size_t bucket, size_t size)
  {
    if (_todBuckets.size() < bucket + 1)
      _todBuckets.resize(bucket + 1);

    _todBuckets[bucket] = size;
  }

  void setInboundCount(const size_t outboundSOP, size_t count)
  {
    _sopPairingPerLeg[0][outboundSOP] = count;
  }
};

namespace shpq
{

struct SOPCombinationMock : public SOPCombination
{
public:
  SOPCombinationMock() {};
  SOPCombinationMock(shpq::SopIdxVecArg solution, char status_)
  {
    status = status_;

    oSopVec = solution;
  }
};

} // shpq

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

class DsciDiversityModelBasicAdapterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DsciDiversityModelBasicAdapterTest);

  CPPUNIT_TEST(testIsBetterByRequirementsAndStatusWorseStatus);
  CPPUNIT_TEST(testIsBetterByRequirementsAndStatusBetterStatusCouldNotSatisfy);
  CPPUNIT_TEST(testIsBetterByRequirementsAndStatusBetterStatusCouldSatisfyWorse);
  CPPUNIT_TEST(testIsBetterByRequirementsAndStatusBetterStatusCouldSatisfyEquiv);
  CPPUNIT_TEST(testIsBetterByRequirementsAndStatusBetterStatusCouldSatisfyBetter);

  CPPUNIT_TEST(testIsBetterByPairingWorse);
  CPPUNIT_TEST(testIsBetterByPairingEquiv);
  CPPUNIT_TEST(testIsBetterByPairingBetter);

  CPPUNIT_TEST(testIsBetterByNonStopEquiv);
  CPPUNIT_TEST(testIsBetterByNonStopBetter);

  CPPUNIT_TEST(testIsBetterByTODEquiv);
  CPPUNIT_TEST(testIsBetterByTODBetter);

  CPPUNIT_TEST_SUITE_END();

public:
  typedef shpq::SOPCombinationMock Operand;

  DsciDiversityModelBasicAdapterTest() {}

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
    _requirements = _memHandle.create<DmcRequirementsFacade>(*_stats, _dc, *_trx);

    _lhsSopIdxVec.clear();
    _rhsSopIdxVec.clear();

    _trx->diversity().setTravelTimeSeparator(0);
    _diversityTestUtil->resetBucketDistribution();
  }

  void tearDown() { _memHandle.clear(); }

  void testIsBetterByRequirementsAndStatusWorseStatus();
  void testIsBetterByRequirementsAndStatusBetterStatusCouldNotSatisfy();
  void testIsBetterByRequirementsAndStatusBetterStatusCouldSatisfyWorse();
  void testIsBetterByRequirementsAndStatusBetterStatusCouldSatisfyEquiv();
  void testIsBetterByRequirementsAndStatusBetterStatusCouldSatisfyBetter();

  void testIsBetterByPairingWorse();
  void testIsBetterByPairingEquiv();
  void testIsBetterByPairingBetter();

  void testIsBetterByNonStopEquiv();
  void testIsBetterByNonStopBetter();

  void testIsBetterByTODEquiv();
  void testIsBetterByTODBetter();

private:
  TestMemHandle _memHandle;

  ShoppingTrx* _trx;
  ItinStatisticMock* _stats;
  DiagCollector* _dc;
  DmcRequirementsFacade* _requirements;
  DsciDiversityModelBasicAdapter* _interpreter;

  shpq::SopIdxVec _lhsSopIdxVec;
  shpq::SopIdxVec _rhsSopIdxVec;

  shpq::DiversityTestUtil* _diversityTestUtil;

  void initTrx()
  {
    // init legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();
  }

  void initInterpreter(int firstRequirements, MoneyAmount score = 0.0)
  {
    _interpreter = _memHandle.create<DsciDiversityModelBasicAdapter>(
        *_trx, *_stats, _requirements, score, firstRequirements);
  }

  void initGold()
  {
    std::vector<Diversity::BucketType> buckets(1);
    buckets[0] = Diversity::GOLD;
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

void
DsciDiversityModelBasicAdapterTest::testIsBetterByRequirementsAndStatusWorseStatus()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 1;
  char lhsStatus = 'S', rhsStatus = 0;
  Operand lhs(_lhsSopIdxVec, lhsStatus);
  Operand rhs(_rhsSopIdxVec, rhsStatus);

  initInterpreter(0);

  CPPUNIT_ASSERT_EQUAL(
      -1, _interpreter->isBetterByRequirementsAndStatus(_requirements, 100.0, lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::testIsBetterByRequirementsAndStatusBetterStatusCouldNotSatisfy()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 1;
  char lhsStatus = 0, rhsStatus = 0;
  Operand lhs(_lhsSopIdxVec, lhsStatus);
  Operand rhs(_rhsSopIdxVec, rhsStatus);

  initInterpreter(0);

  CPPUNIT_ASSERT_EQUAL(
      -1, _interpreter->isBetterByRequirementsAndStatus(_requirements, 100.0, lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::
    testIsBetterByRequirementsAndStatusBetterStatusCouldSatisfyWorse()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 1; // 2h
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 0; // 1h
  char lhsStatus = 0, rhsStatus = 0;
  Operand lhs(_lhsSopIdxVec, lhsStatus);
  Operand rhs(_rhsSopIdxVec, rhsStatus);

  initJunk();

  _trx->diversity().setTravelTimeSeparator(90); // 1.5h
  _trx->diversity().setFareAmountSeparator(50.0); // lhs is too long, too expensive = JUNK

  initInterpreter(DmcRequirement::NEED_GOLD);

  CPPUNIT_ASSERT_EQUAL(
      -1, _interpreter->isBetterByRequirementsAndStatus(_requirements, 100.0, lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::
    testIsBetterByRequirementsAndStatusBetterStatusCouldSatisfyEquiv()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0; // 1h
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 1; // 2h
  char lhsStatus = 0, rhsStatus = 0;
  Operand lhs(_lhsSopIdxVec, lhsStatus);
  Operand rhs(_rhsSopIdxVec, rhsStatus);

  initGold();

  _trx->diversity().setTravelTimeSeparator(90); // 1.5h
  _trx->diversity().setFareAmountSeparator(150.0); // lhs is short and cheap = GOLD

  initInterpreter(DmcRequirement::NEED_GOLD);

  CPPUNIT_ASSERT_EQUAL(
      0, _interpreter->isBetterByRequirementsAndStatus(_requirements, 100.0, lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::
    testIsBetterByRequirementsAndStatusBetterStatusCouldSatisfyBetter()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0; // 1h
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 1; // 2h
  char lhsStatus = 0, rhsStatus = 0;
  Operand lhs(_lhsSopIdxVec, lhsStatus);
  Operand rhs(_rhsSopIdxVec, rhsStatus);

  initGold();

  _trx->diversity().setTravelTimeSeparator(90); // 1.5h
  _trx->diversity().setFareAmountSeparator(150.0); // lhs is short and cheap = GOLD

  initInterpreter(DmcRequirement::NEED_LUXURY);

  CPPUNIT_ASSERT_EQUAL(
      1, _interpreter->isBetterByRequirementsAndStatus(_requirements, 100.0, lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::testIsBetterByPairingWorse()
{
  _trx->diversity().setInboundOutboundPairing(10);
  _stats->setInboundCount(0, 5);
  _stats->setInboundCount(1, 2);

  _lhsSopIdxVec.resize(2);
  _lhsSopIdxVec[0] = 0;
  _lhsSopIdxVec[1] = 0;
  _rhsSopIdxVec.resize(2);
  _rhsSopIdxVec[0] = 1;
  _rhsSopIdxVec[1] = 0;

  Operand lhs(_lhsSopIdxVec, 0);
  Operand rhs(_rhsSopIdxVec, 0);

  initInterpreter(0);

  CPPUNIT_ASSERT_EQUAL(-1, _interpreter->isBetterByPairing(lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::testIsBetterByPairingEquiv()
{
  _trx->diversity().setInboundOutboundPairing(10);
  _stats->setInboundCount(0, 5);
  _stats->setInboundCount(1, 5);

  _lhsSopIdxVec.resize(2);
  _lhsSopIdxVec[0] = 0;
  _lhsSopIdxVec[1] = 0;
  _rhsSopIdxVec.resize(2);
  _rhsSopIdxVec[0] = 1;
  _rhsSopIdxVec[1] = 0;

  Operand lhs(_lhsSopIdxVec, 0);
  Operand rhs(_rhsSopIdxVec, 0);

  initInterpreter(0);

  CPPUNIT_ASSERT_EQUAL(0, _interpreter->isBetterByPairing(lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::testIsBetterByPairingBetter()
{
  _trx->diversity().setInboundOutboundPairing(10);
  _stats->setInboundCount(0, 5);
  _stats->setInboundCount(1, 7);

  _lhsSopIdxVec.resize(2);
  _lhsSopIdxVec[0] = 0;
  _lhsSopIdxVec[1] = 0;
  _rhsSopIdxVec.resize(2);
  _rhsSopIdxVec[0] = 1;
  _rhsSopIdxVec[1] = 0;

  Operand lhs(_lhsSopIdxVec, 0);
  Operand rhs(_rhsSopIdxVec, 0);

  initInterpreter(0);

  CPPUNIT_ASSERT_EQUAL(1, _interpreter->isBetterByPairing(lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::testIsBetterByNonStopEquiv()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 1;

  Operand lhs(_lhsSopIdxVec, 0);
  Operand rhs(_rhsSopIdxVec, 0);

  initInterpreter(0);

  CPPUNIT_ASSERT_EQUAL(0, _interpreter->isBetterByNonStop(lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::testIsBetterByNonStopBetter()
{
  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 2;

  Operand lhs(_lhsSopIdxVec, 0);
  Operand rhs(_rhsSopIdxVec, 0);

  initInterpreter(0);

  CPPUNIT_ASSERT_EQUAL(1, _interpreter->isBetterByNonStop(lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::testIsBetterByTODEquiv()
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

  _lhsSopIdxVec.resize(1);
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 3;

  Operand lhs(_lhsSopIdxVec, 0);
  Operand rhs(_rhsSopIdxVec, 0);

  initInterpreter(0);

  CPPUNIT_ASSERT_EQUAL(0, _interpreter->isBetterByTOD(lhs, rhs));
}

void
DsciDiversityModelBasicAdapterTest::testIsBetterByTODBetter()
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
  _lhsSopIdxVec[0] = 0;
  _rhsSopIdxVec.resize(1);
  _rhsSopIdxVec[0] = 1;

  Operand lhs(_lhsSopIdxVec, 0);
  Operand rhs(_rhsSopIdxVec, 0);

  initInterpreter(0);

  CPPUNIT_ASSERT_EQUAL(1, _interpreter->isBetterByTOD(lhs, rhs));
}

CPPUNIT_TEST_SUITE_REGISTRATION(DsciDiversityModelBasicAdapterTest);

} // tse
