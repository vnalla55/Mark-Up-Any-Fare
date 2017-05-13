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

#include "test/include/CppUnitHelperMacros.h"
#include <boost/range.hpp>

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloGroupFarePath.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "test/include/LegsBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"

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
const LegsBuilder::Segment segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 6), DT(obDate, 7) }, // 1 h
  { 0, 1, "LH", "JFK", "LAX", "LH", DT(obDate, 5), DT(obDate, 7) }, // 2 h
  { 0, 1, "LH", "LAX", "DFW", "LH", DT(obDate, 7), DT(obDate, 9) }, // 2 h
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }, // 1h
  { 1, 1, "DL", "DFW", "JFK", "DL", DT(ibDate, 12), DT(ibDate, 13) } // 1h
};
#undef DT

const CarrierCode directOptionsCarriers[] = { "LH" };
const std::pair<CarrierCode, float> optionsPerCarrier[] = { std::make_pair("LH", 1.0f),
                                                            std::make_pair("DL", 1.0f) };

// ==================================
// MOCK CLASSES
// ==================================

class MockDiversity : public Diversity
{
  friend class ItinStatisticTest;

protected:
  std::set<CarrierCode>& getDirectOptionsCarriers() { return _directOptionsCarriers; }
  std::map<CarrierCode, float>& getOptionsPerCarrierMap() { return _optionsPerCarrier; }
};

// ==================================
// TEST CLASS
// ==================================

class ItinStatisticTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ItinStatisticTest);
  CPPUNIT_TEST(testStatistics);
  CPPUNIT_TEST(testAdditionalNonStopStats);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  MockDiversity* _diversity;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _diversity = _memHandle.create<MockDiversity>();
    CPPUNIT_ASSERT(_trx);
    CPPUNIT_ASSERT(_diversity);
    initTrx();
  }

  void tearDown() { _memHandle.clear(); }

  void testStatistics()
  {
    const int32_t enabledStats =
        ItinStatistic::STAT_MIN_PRICE | ItinStatistic::STAT_AVG_PRICE |
        ItinStatistic::STAT_MAX_PRICE | ItinStatistic::STAT_MIN_DURATION |
        ItinStatistic::STAT_AVG_DURATION | ItinStatistic::STAT_NON_STOP_COUNT |
        ItinStatistic::STAT_NON_STOP_CARRIERS | ItinStatistic::STAT_CARRIERS |
        ItinStatistic::STAT_TOD | ItinStatistic::STAT_BUCKETS |
        ItinStatistic::STAT_BUCKETS_MIN_PRICE;

    ItinStatistic stats(*_trx, *_diversity);
    stats.setEnabledStatistics(0, this);
    stats.setEnabledStatistics(enabledStats, this);

    CPPUNIT_ASSERT_EQUAL(enabledStats, stats.getEnabledStatistics(this));

    SoloGroupFarePath gfpCheap;
    SoloGroupFarePath gfpExpensive;
    gfpCheap.setTotalNUCAmount(200.0);
    gfpExpensive.setTotalNUCAmount(1000.0);

    const SolutionPatternStorage& storage = SolutionPatternStorage::instance();
    const SolutionPattern& sp = storage.getSPById(SolutionPattern::SP10);
    gfpCheap.setSolutionPattern(&sp);
    gfpExpensive.setSolutionPattern(&sp);

    std::vector<int> onlineNonStop;
    onlineNonStop.push_back(0);
    onlineNonStop.push_back(0);

    std::vector<int> interlineNonStop;
    interlineNonStop.push_back(0);
    interlineNonStop.push_back(1);

    std::vector<int> longInterline;
    longInterline.push_back(1);
    longInterline.push_back(1);

    stats.addSolution(std::make_pair(onlineNonStop, &gfpCheap));
    stats.addSolution(std::make_pair(longInterline, &gfpCheap));
    stats.addSolution(std::make_pair(interlineNonStop, &gfpExpensive));
    stats.addSolution(std::make_pair(longInterline, &gfpExpensive));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, stats.getMinPrice(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.0, stats.getMaxPrice(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(600.0, stats.getAvgPrice(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(2 * 60, stats.getMinDuration());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.5 * 60, stats.getAvgDuration(), EPSILON);

    // We expect 2u because current version of ItinStatistic detects
    // both online and interline non-stop options.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), stats.getNonStopsCount());
    CPPUNIT_ASSERT_EQUAL(1, stats.getNumOfNonStopItinsForCarrier("LH"));
    CPPUNIT_ASSERT_EQUAL(-1, stats.getNumOfNonStopItinsForCarrier("DL"));

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), stats.getTODBucketSize(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), stats.getTODBucketSize(1));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0u), stats.getTODBucketSize(2));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0u), stats.getTODBucketSize(3));

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), stats.getBucketSize(Diversity::GOLD));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), stats.getBucketSize(Diversity::LUXURY));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), stats.getBucketSize(Diversity::UGLY));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), stats.getBucketSize(Diversity::JUNK));

    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, stats.getMinPrice(Diversity::GOLD), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, stats.getMinPrice(Diversity::UGLY), EPSILON);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), stats.getMinPricedItinCount(Diversity::GOLD));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), stats.getMinPricedItinCount(Diversity::UGLY));
  }

  void testAdditionalNonStopStats()
  {
    // Check additional non-stop count [per cxr]
    // Note that additional non-stops can be interline
    ItinStatistic stats(*_trx, *_diversity);
    stats.setEnabledStatistics(0, this);
    stats.setEnabledStatistics(
        ItinStatistic::STAT_NON_STOP_COUNT | ItinStatistic::STAT_NON_STOP_CARRIERS, this);

    GroupFarePath gfp;
    gfp.setTotalNUCAmount(200.0);

    std::vector<int> onlineNonStop;
    onlineNonStop.push_back(0);
    onlineNonStop.push_back(0);

    std::vector<int> interlineNonStop;
    interlineNonStop.push_back(0);
    interlineNonStop.push_back(1);

    stats.addNonStopSolution(std::make_pair(onlineNonStop, &gfp));
    stats.addNonStopSolution(std::make_pair(interlineNonStop, &gfp));

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), stats.getAdditionalNonStopsCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), stats.getAdditionalNonStopCountPerCarrier("LH"));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0u), stats.getAdditionalNonStopCountPerCarrier("DL"));
  }

private:
  void initTrx()
  {
    // init legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();

// init diversity
#define HR(h) (h * 60u)
    std::vector<std::pair<uint16_t, uint16_t> > todRanges;
    todRanges.push_back(std::make_pair(uint16_t(HR(0u)), uint16_t(HR(6u) - 1)));
    todRanges.push_back(std::make_pair(uint16_t(HR(6u)), uint16_t(HR(12u) - 1)));
    todRanges.push_back(std::make_pair(uint16_t(HR(12u)), uint16_t(HR(18u) - 1)));
    todRanges.push_back(std::make_pair(uint16_t(HR(18u)), uint16_t(HR(24u) - 1)));
#undef HR

    _diversity->setTODRanges(todRanges);
    _diversity->getDirectOptionsCarriers().insert(
        directOptionsCarriers, directOptionsCarriers + boost::size(directOptionsCarriers));
    _diversity->getOptionsPerCarrierMap().insert(
        optionsPerCarrier, optionsPerCarrier + boost::size(optionsPerCarrier));
    _diversity->setFareAmountSeparator(200.01);
    _diversity->setTravelTimeSeparator(121);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ItinStatisticTest);

} /* namespace shpq */
} /* namespace tse */
