// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/SwapperIBFValidationStrategy.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include "test/include/CppUnitHelperMacros.h"
#include <boost/range.hpp>

namespace tse
{

// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 06, 01);
static DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 6), DT(obDate, 7) },
  { 0, 1, "AA", "JFK", "LAX", "AA", DT(obDate, 5), DT(obDate, 7) },
  { 0, 1, "AA", "LAX", "DFW", "AA", DT(obDate, 7), DT(obDate, 9) },
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) },
  { 1, 1, "AA", "DFW", "JFK", "AA", DT(ibDate, 12), DT(ibDate, 13) }
};
#undef DT

class MockItinStatistic : public ItinStatistic
{
public:
  MockItinStatistic(ShoppingTrx& trx) : ItinStatistic(trx) {}

  void setSopPairing(size_t legIdx, size_t sopIdx, size_t pairing)
  {
    _sopPairingPerLeg[legIdx][sopIdx] = pairing;
  }
  void setRequestingCarrier(const CarrierCode& cxr) { _requestingCarrier = cxr; }
};

// ==================================
// TEST CLASS
// ==================================

class SwapperIBFValidationStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SwapperIBFValidationStrategyTest);
  CPPUNIT_TEST(testCheckNonStopSolution);
  CPPUNIT_TEST(testCheckAllSopsRequirement);
  CPPUNIT_TEST(testCheckRCOnlineRequirement);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  MockItinStatistic* _stats;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _stats = _memHandle.create<MockItinStatistic>(*_trx);
    CPPUNIT_ASSERT(_trx);
    initTrx();
  }

  void tearDown() { _memHandle.clear(); }

  void testCheckNonStopSolution()
  {
    SwapperIBFValidationStrategy strategy(*_trx, *_stats);
    SopIdVec nonStop(2, 0);
    SopIdVec notANonStop(2, 0);
    notANonStop[0] = 1;

    CPPUNIT_ASSERT(SwapperEvaluationResult::NON_STOPS == strategy.checkNonStopSolution(nonStop));
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkNonStopSolution(notANonStop));
  }

  void testCheckAllSopsRequirement()
  {
    SwapperIBFValidationStrategy strategy(*_trx, *_stats);
    SopIdVec comb(2, 0);

    _stats->setSopPairing(0, 0, 10);
    _stats->setSopPairing(1, 0, 10);
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkAllSopsRequirement(comb));

    _stats->setSopPairing(1, 0, 1);
    CPPUNIT_ASSERT(SwapperEvaluationResult::ALL_SOPS == strategy.checkAllSopsRequirement(comb));
  }

  void testCheckRCOnlineRequirement()
  {
    SwapperIBFValidationStrategy strategy(*_trx, *_stats);
    SopIdVec lhComb(2, 0);
    SopIdVec aaComb(2, 1);

    _stats->setRequestingCarrier("LH");
    _stats->setMissingRCOnlineOptionsCount(0);

    CPPUNIT_ASSERT(SwapperEvaluationResult::RC_ONLINES == strategy.checkRCOnlineRequirement(lhComb));
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkRCOnlineRequirement(aaComb));

    _stats->setMissingRCOnlineOptionsCount(-1);
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkRCOnlineRequirement(lhComb));
  }

private:
  void initTrx()
  {
    // init legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwapperIBFValidationStrategyTest);

} /* namespace tse */
