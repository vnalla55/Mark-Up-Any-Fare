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
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/Diversity/DiversityItinerarySwapper.h"
#include "Pricing/GroupFarePath.h"

#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include "test/include/CppUnitHelperMacros.h"
#include <boost/range.hpp>

namespace tse
{
namespace shpq
{

// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 06, 01);
static DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment Segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // 1 h
  { 0, 1, "DL", "JFK", "DFW", "DL", DT(obDate, 10), DT(obDate, 11) }, // 1 h
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // 1h
  { 1, 1, "DL", "DFW", "JFK", "DL", DT(ibDate, 10), DT(ibDate, 11) } // 1h
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class DiversityItinerarySwapperTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiversityItinerarySwapperTest);
  // TODO: testSwapSolution...
  CPPUNIT_TEST(testSwapAdditionalNS_NoneIsWorse);
  CPPUNIT_TEST(testSwapAdditionalNS_MoreExpensiveInterline);
  CPPUNIT_TEST(testSwapAdditionalNS_SamePricedInterline);
  CPPUNIT_TEST_SUITE_END();

private:
  typedef ShoppingTrx::FlightMatrix::value_type FMSolution;

  ShoppingTrx* _trx;
  TestMemHandle _memHandle;

  std::vector<int> _onlineNS0;
  std::vector<int> _onlineNS1;
  std::vector<int> _interlineNS0;
  std::vector<int> _interlineNS1;

public:
  DiversityItinerarySwapperTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    initTrx();
    _onlineNS0.push_back(0);
    _onlineNS0.push_back(0);
    _onlineNS1.push_back(1);
    _onlineNS1.push_back(1);
    _interlineNS0.push_back(0);
    _interlineNS0.push_back(1);
    _interlineNS1.push_back(1);
    _interlineNS1.push_back(0);
  }

  void tearDown() { _memHandle.clear(); }

  void testSwapAdditionalNS_NoneIsWorse()
  {
    ItinStatistic stats(*_trx);
    stats.setEnabledStatistics(ItinStatistic::STAT_BUCKETS_PAIRING, this);
    stats.addNonStopSolution(createFMSolution(_onlineNS0, 300.0));
    stats.addNonStopSolution(createFMSolution(_interlineNS0, 200.0));

    DiversityItinerarySwapper swapper(*_trx, stats, NULL, NULL);

    bool anySolutionRemoved = swapper.swapAdditionalNS(createFMSolution(_interlineNS1, 300.0));
    CPPUNIT_ASSERT(!anySolutionRemoved);
  }

  void testSwapAdditionalNS_MoreExpensiveInterline()
  {
    ItinStatistic stats(*_trx);
    stats.setEnabledStatistics(
        ItinStatistic::STAT_NON_STOP_COUNT | ItinStatistic::STAT_BUCKETS_PAIRING, this);
    stats.addNonStopSolution(createFMSolution(_interlineNS0, 300.0));
    stats.addNonStopSolution(createFMSolution(_onlineNS0, 300.0));

    DiversityItinerarySwapper swapper(*_trx, stats, NULL, NULL);

    bool anySolutionRemoved = swapper.swapAdditionalNS(createFMSolution(_onlineNS1, 200.0));
    CPPUNIT_ASSERT_MESSAGE("No solution was removed", anySolutionRemoved);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interline solution wasn't removed",
                                 static_cast<size_t>(0u),
                                 stats.getAdditionalInterlineNonStopsCount());
  }

  void testSwapAdditionalNS_SamePricedInterline()
  {
    ItinStatistic stats(*_trx);
    stats.setEnabledStatistics(
        ItinStatistic::STAT_NON_STOP_COUNT | ItinStatistic::STAT_BUCKETS_PAIRING, this);
    stats.addNonStopSolution(createFMSolution(_interlineNS0, 200.0));

    DiversityItinerarySwapper swapper(*_trx, stats, NULL, NULL);

    bool anySolutionRemoved = swapper.swapAdditionalNS(createFMSolution(_onlineNS0, 200.0));
    CPPUNIT_ASSERT_MESSAGE("No solution was removed", anySolutionRemoved);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interline solution wasn't removed",
                                 static_cast<size_t>(0u),
                                 stats.getAdditionalInterlineNonStopsCount());
  }

private:
  void initTrx()
  {
    _trx = TestShoppingTrxFactory::create(
        "/vobs/atseintl/Pricing/Shopping/PQ/test/testdata/ShoppingNGSTrx.xml", true);
    TSE_ASSERT(_trx);

    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();
  }

  FMSolution createFMSolution(const std::vector<int>& sopVec, MoneyAmount amt)
  {
    GroupFarePath* gfp = _memHandle.create<GroupFarePath>();
    gfp->setTotalNUCAmount(amt);
    return FMSolution(sopVec, gfp);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DiversityItinerarySwapperTest);

} /* namespace shpq */
} /* namespace tse */
