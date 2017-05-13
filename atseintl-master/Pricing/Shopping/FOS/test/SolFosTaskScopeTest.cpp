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
#include <map>

#include "DataModel/Diversity.h"
#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"
#include "Pricing/Shopping/FOS/SolFosTaskScope.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace fos
{
// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 06, 01);
static DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment Segments2Carriers[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "AA", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 1, "AA", "JFK", "DFW", "AA", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 2, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 3, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 4, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 1, 0, "AA", "DFW", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 1, "AA", "DFW", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 2, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 3, "LH", "DFW", "EWR", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 3, "LH", "EWR", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }
};

const LegsBuilder::Segment Segments3Carriers[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "AA", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 1, "AB", "JFK", "DFW", "AB", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 2, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 3, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 4, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 1, 0, "AA", "DFW", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 1, "AA", "DFW", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 2, "AA", "DFW", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 3, "AB", "DFW", "JFK", "AB", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 4, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 5, "LH", "DFW", "EWR", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 5, "LH", "EWR", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class SolFosTaskScopeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SolFosTaskScopeTest);

  CPPUNIT_TEST(testNothingNeeded);
  CPPUNIT_TEST(testNoAdditionalNeeded);
  CPPUNIT_TEST(testAdditionalNeededWithInterline);
  CPPUNIT_TEST(testAdditionalNeededWithoutInterline);
  CPPUNIT_TEST(testAdditionalNeededLess);

  CPPUNIT_TEST_SUITE_END();

private:
  class ItinStatisticMock : public ItinStatistic
  {
  public:
    ItinStatisticMock(ShoppingTrx& trx) : ItinStatistic(trx)
    {
      std::fill(_buckets, _buckets + Diversity::BUCKET_COUNT, 0);
    }

    void setNumOfNonStopItinsForCarrier(CarrierCode cxr, size_t number)
    {
      _nonStopCarriers[cxr] = number;
    }

    void setAdditionalNonStopCountForCarrier(CarrierCode cxr, size_t count)
    {
      _additionalNonStopCountCxr[cxr] = count;
    }

    void setAdditionalOnlineNonStopsCount(size_t count) { _additionalOnlineNonStopsCount = count; }

    void setAdditionalInterlineNonStopsCount(size_t count)
    {
      _additionalInterlineNonStopsCount = count;
    }
  };

  TestMemHandle _memHandle;

  ShoppingTrx* _trx;
  DiagCollector* _dc;
  ItinStatisticMock* _stats;

  shpq::DiversityTestUtil* _diversityTestUtil;

protected:
  void initSegmentsTwoCarriers()
  {
    TSE_ASSERT(_trx);
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments2Carriers, boost::size(Segments2Carriers));
    builder.endBuilding();
  }

  void initSegmentsThreeCarriers()
  {
    TSE_ASSERT(_trx);
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments3Carriers, boost::size(Segments3Carriers));
    builder.endBuilding();
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    TestConfigInitializer::setValue<float>("ADDITIONAL_DIRECT_FOS_PERC", 1.0, "SHOPPING_DIVERSITY", true);

    _trx = TestShoppingTrxFactory::create(
        "/vobs/atseintl/Pricing/Shopping/PQ/test/testdata/ShoppingNGSTrx.xml", true);
    TSE_ASSERT(_trx);

    _diversityTestUtil = _memHandle.create<shpq::DiversityTestUtil>(_trx->diversity());
    _diversityTestUtil->setNonStopOptionsPerCarrierEnabled(true);

    _stats = _memHandle.create<ItinStatisticMock>(*_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void testNothingNeeded()
  {
    initSegmentsTwoCarriers();

    _diversityTestUtil->setMaxNonStopCountForCarrier("AA", 4);
    _diversityTestUtil->setMaxNonStopCountForCarrier("LH", 3);

    _diversityTestUtil->addDirectCarrier("AA");
    _diversityTestUtil->addDirectCarrier("LH");

    _diversityTestUtil->setNonStopOptionsCount(4);

    _stats->setNumOfNonStopItinsForCarrier("AA", 3);
    _stats->setNumOfNonStopItinsForCarrier("LH", 3);

    _stats->setAdditionalOnlineNonStopsCount(4);
    _stats->setAdditionalInterlineNonStopsCount(0);

    SolFosTaskScope scope(*_trx, *_stats, false);

    CPPUNIT_ASSERT_EQUAL(uint32_t(0), scope.getNumDirectFos());
    CPPUNIT_ASSERT_EQUAL(size_t(0), scope.getNumDirectFosPerCarrier().size());
  }

  void testNoAdditionalNeeded()
  {
    initSegmentsTwoCarriers();

    _diversityTestUtil->setMaxNonStopCountForCarrier("AA", 4);
    _diversityTestUtil->setMaxNonStopCountForCarrier("LH", 3);

    _diversityTestUtil->addDirectCarrier("AA");
    _diversityTestUtil->addDirectCarrier("LH");

    _diversityTestUtil->setNonStopOptionsCount(0);

    _stats->setNumOfNonStopItinsForCarrier("AA", 1);
    _stats->setNumOfNonStopItinsForCarrier("LH", 1);

    _stats->setAdditionalOnlineNonStopsCount(0);
    _stats->setAdditionalInterlineNonStopsCount(0);

    SolFosTaskScope scope(*_trx, *_stats, false);

    CPPUNIT_ASSERT_EQUAL(uint32_t(0), scope.getNumDirectFos());
    CPPUNIT_ASSERT_EQUAL(size_t(0), scope.getNumDirectFosPerCarrier().size());
  }

  void testAdditionalNeededWithInterline()
  {
    initSegmentsThreeCarriers();

    _diversityTestUtil->setNonStopOptionsCount(6);

    _diversityTestUtil->setMaxNonStopCountForCarrier("AA", 3);
    _diversityTestUtil->setMaxNonStopCountForCarrier("AB", 1);
    _diversityTestUtil->setMaxNonStopCountForCarrier("LH", 3);

    _diversityTestUtil->addDirectCarrier("AA");
    _diversityTestUtil->addDirectCarrier("AB");
    _diversityTestUtil->addDirectCarrier("LH");

    _stats->setNumOfNonStopItinsForCarrier("AA", 1);
    _stats->setNumOfNonStopItinsForCarrier("AB", 1);
    _stats->setNumOfNonStopItinsForCarrier("LH", 1);

    _stats->setAdditionalNonStopCountForCarrier("AA", 0);
    _stats->setAdditionalNonStopCountForCarrier("AB", 0);
    _stats->setAdditionalNonStopCountForCarrier("LH", 1);

    _stats->setAdditionalOnlineNonStopsCount(1);
    _stats->setAdditionalInterlineNonStopsCount(0);

    SolFosTaskScope scope(*_trx, *_stats, false);

    CPPUNIT_ASSERT_EQUAL(uint32_t(5), scope.getNumDirectFos());
    CPPUNIT_ASSERT_EQUAL(size_t(3), scope.getNumDirectFosPerCarrier().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), scope.getNumDirectFosPerCarrier().at("AA"));
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), scope.getNumDirectFosPerCarrier().at("LH"));
    CPPUNIT_ASSERT_EQUAL(uint32_t(2),
                         scope.getNumDirectFosPerCarrier().at(Diversity::INTERLINE_CARRIER));
  }

  void testAdditionalNeededWithoutInterline()
  {
    initSegmentsThreeCarriers();

    _diversityTestUtil->setNonStopOptionsCount(3);

    _diversityTestUtil->setMaxNonStopCountForCarrier("AA", 3);
    _diversityTestUtil->setMaxNonStopCountForCarrier("AB", 1);
    _diversityTestUtil->setMaxNonStopCountForCarrier("LH", 3);

    _diversityTestUtil->addDirectCarrier("AA");
    _diversityTestUtil->addDirectCarrier("AB");
    _diversityTestUtil->addDirectCarrier("LH");

    _stats->setNumOfNonStopItinsForCarrier("AA", 2);
    _stats->setNumOfNonStopItinsForCarrier("AB", 1);
    _stats->setNumOfNonStopItinsForCarrier("LH", 1);

    _stats->setAdditionalNonStopCountForCarrier("AA", 0);
    _stats->setAdditionalNonStopCountForCarrier("AB", 0);
    _stats->setAdditionalNonStopCountForCarrier("LH", 1);

    _stats->setAdditionalOnlineNonStopsCount(1);
    _stats->setAdditionalInterlineNonStopsCount(0);

    SolFosTaskScope scope(*_trx, *_stats, false);

    CPPUNIT_ASSERT_EQUAL(uint32_t(2), scope.getNumDirectFos());
    CPPUNIT_ASSERT_EQUAL(size_t(2), scope.getNumDirectFosPerCarrier().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), scope.getNumDirectFosPerCarrier().at("AA"));
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), scope.getNumDirectFosPerCarrier().at("LH"));
  }

  void testAdditionalNeededLess()
  {
    initSegmentsThreeCarriers();

    TestConfigInitializer::setValue("ADDITIONAL_DIRECT_FOS_PERC", 0.6, "SHOPPING_DIVERSITY", true);
    _diversityTestUtil->setNonStopOptionsCount(5);

    _diversityTestUtil->setMaxNonStopCountForCarrier("AA", 3);
    _diversityTestUtil->setMaxNonStopCountForCarrier("AB", 1);
    _diversityTestUtil->setMaxNonStopCountForCarrier("LH", 3);

    _diversityTestUtil->addDirectCarrier("AA");
    _diversityTestUtil->addDirectCarrier("AB");
    _diversityTestUtil->addDirectCarrier("LH");

    _stats->setNumOfNonStopItinsForCarrier("AA", 1);
    _stats->setNumOfNonStopItinsForCarrier("AB", 1);
    _stats->setNumOfNonStopItinsForCarrier("LH", 1);

    _stats->setAdditionalNonStopCountForCarrier("AA", 0);
    _stats->setAdditionalNonStopCountForCarrier("AB", 0);
    _stats->setAdditionalNonStopCountForCarrier("LH", 1);

    _stats->setAdditionalOnlineNonStopsCount(1);
    _stats->setAdditionalInterlineNonStopsCount(0);

    SolFosTaskScope scope(*_trx, *_stats, false);

    CPPUNIT_ASSERT_EQUAL(uint32_t(2), scope.getNumDirectFos());
    CPPUNIT_ASSERT_EQUAL(size_t(2), scope.getNumDirectFosPerCarrier().size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), scope.getNumDirectFosPerCarrier().at("AA"));
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), scope.getNumDirectFosPerCarrier().at("LH"));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SolFosTaskScopeTest);

} // fos
} // tse
