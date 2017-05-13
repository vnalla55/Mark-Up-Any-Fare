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

#include "DataModel/ShoppingTrx.h"
#include "DataModel/Diversity.h"

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

DateTime obDate = DateTime(2013, 06, 01);
DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment nonStopSegments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 12) }, // 2 h
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 12) }, // 2 h
  { 0, 2, "DL", "JFK", "DFW", "DL", DT(obDate, 10), DT(obDate, 12) }, // 2 h
  { 0, 3, "AA", "JFK", "DFW", "AA", DT(obDate, 10), DT(obDate, 12) }, // 2 h
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 12) }, // 2 h
  { 1, 1, "DL", "DFW", "JFK", "DL", DT(ibDate, 10), DT(ibDate, 12) } // 2 h
};
#undef DT

// ==================================
// MOCK CLASSES
// ==================================

class MockDiversity : public Diversity
{
  friend class DiversityTest;
};

// ==================================
// TEST
// ==================================

class DiversityTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiversityTest);
  CPPUNIT_TEST(testNonStopIndex_Initialize);
  CPPUNIT_TEST(testNonStopIndex_UpdateBookingCode);
  CPPUNIT_TEST(testNonStopIndex_AddNSFromTrx);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  TestMemHandle _memHandle;
  Diversity::NonStopIndex* _nonStopIndex;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);

    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(nonStopSegments, boost::size(nonStopSegments));
    builder.endBuilding();

    _nonStopIndex = MockDiversity().createNonStopIndex(*_trx);
    CPPUNIT_ASSERT(_nonStopIndex);
  }

  void tearDown() { _memHandle.clear(); }

  void testNonStopIndex_Initialize()
  {
    size_t maxOnlineNSCount = 1234;
    size_t maxInterlineNSCount = 1234;
    std::map<CarrierCode, size_t> maxNSPerCarrier;
    maxNSPerCarrier["LH"] = 1234;

    _nonStopIndex->calcStatPutResultsTo(maxOnlineNSCount, maxInterlineNSCount, maxNSPerCarrier);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0u), maxOnlineNSCount);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0u), maxInterlineNSCount);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0u), maxNSPerCarrier.size());
  }

  void testNonStopIndex_UpdateBookingCode()
  {
    size_t maxOnlineNSCount;
    size_t maxInterlineNSCount;
    std::map<CarrierCode, size_t> maxNSPerCarrier;

    // add non-stops to index
    for (size_t i = 0; i < boost::size(nonStopSegments); i++)
    {
      const LegsBuilder::Segment& seg = nonStopSegments[i];
      _nonStopIndex->addNS(seg._legId, seg._govCxr, static_cast<int>(seg._sopId), *_trx);
    }

    _nonStopIndex->calcStatPutResultsTo(maxOnlineNSCount, maxInterlineNSCount, maxNSPerCarrier);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3u), maxOnlineNSCount);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5u), maxInterlineNSCount);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), maxNSPerCarrier.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), maxNSPerCarrier["LH"]);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), maxNSPerCarrier["DL"]);

    // change booking code info
    _trx->legs()[0].sop()[0].cabinClassValid() = false;

    bool updateRes = _nonStopIndex->updateBookingCodeInfo(*_trx);
    _nonStopIndex->calcStatPutResultsTo(maxOnlineNSCount, maxInterlineNSCount, maxNSPerCarrier);

    CPPUNIT_ASSERT(updateRes);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), maxOnlineNSCount);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4u), maxInterlineNSCount);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), maxNSPerCarrier.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), maxNSPerCarrier["LH"]);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), maxNSPerCarrier["DL"]);
  }

  void testNonStopIndex_AddNSFromTrx()
  {
    size_t maxOnlineNSCount;
    size_t maxInterlineNSCount;
    std::map<CarrierCode, size_t> maxNSPerCarrier;

    _nonStopIndex->addNS(*_trx);
    _nonStopIndex->calcStatPutResultsTo(maxOnlineNSCount, maxInterlineNSCount, maxNSPerCarrier);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3u), maxOnlineNSCount);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5u), maxInterlineNSCount);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), maxNSPerCarrier.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2u), maxNSPerCarrier["LH"]);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1u), maxNSPerCarrier["DL"]);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DiversityTest);
}
