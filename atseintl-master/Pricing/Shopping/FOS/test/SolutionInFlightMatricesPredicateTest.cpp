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


#include "Pricing/Shopping/FOS/SolutionInFlightMatricesPredicate.h"

#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/LegsBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TseServerStub.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{
namespace fos
{
// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 6, 1);
static DateTime ibDate = DateTime(2013, 6, 8);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment Segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "AA", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 1, 0, "AA", "DFW", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 1, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
};
#undef DT

// ==================================
// TEST CLASS
// ==================================
using namespace boost::assign;

class SolutionInFlightMatricesPredicateTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SolutionInFlightMatricesPredicateTest);

  CPPUNIT_TEST(testNoQueuesValid);
  CPPUNIT_TEST(testOneQueueValid);
  CPPUNIT_TEST(testOneQueueInvalidInEstimateMatrix);

  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
  SolutionInFlightMatricesPredicate* _predicate;

  CarrierCode _cxr;
  PricingOrchestratorDerived* _po;
  BitmapOpOrderer* _orderer;
  ShoppingPQDerived* _pq;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);

    _cxr = "DL";
    _po = new PricingOrchestratorDerived(*_memHandle.create<TseServerStub>());
    _orderer = new BitmapOpOrderer(Global::config());
    _pq = new ShoppingPQDerived(*_po, *_trx, 4, 0, &_cxr, *_orderer);

    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();
  }

  void tearDown()
  {
    delete _po;
    _po = NULL;
    delete _pq;
    _pq = NULL;
    delete _orderer;
    _orderer = NULL;
    _memHandle.clear();
  }

  void testNoQueuesValid()
  {
    SolutionInFlightMatricesPredicate predicate(*_trx);

    std::vector<int> comb;
    comb += 0, 0;

    CPPUNIT_ASSERT(predicate(comb));
  }

  void testOneQueueValid()
  {
    std::vector<int> comb00, comb01;
    comb00 += 0, 0;
    comb01 += 0, 1;

    _trx->shoppingPQVector() += (tse::ShoppingTrx::PQPtr)_pq;
    _trx->shoppingPQVector().at(0)->flightMatrix().insert(
        ShoppingTrx::FlightMatrix::value_type(comb01, 0));
    SolutionInFlightMatricesPredicate predicate(*_trx);

    CPPUNIT_ASSERT(predicate(comb00));
  }

  void testOneQueueInvalidInFlightMatrix()
  {
    std::vector<int> comb01;
    comb01 += 0, 1;

    _trx->shoppingPQVector() += (tse::ShoppingTrx::PQPtr)_pq;
    _trx->shoppingPQVector().at(0)->flightMatrix().insert(
        ShoppingTrx::FlightMatrix::value_type(comb01, 0));
    SolutionInFlightMatricesPredicate predicate(*_trx);

    CPPUNIT_ASSERT(predicate(comb01));
  }

  void testOneQueueInvalidInEstimateMatrix()
  {
    std::vector<int> comb00, comb01;
    comb00 += 0, 0;
    comb01 += 0, 1;

    _trx->shoppingPQVector() += (tse::ShoppingTrx::PQPtr)_pq;
    _trx->shoppingPQVector().at(0)->estimateMatrix().insert(ShoppingTrx::EstimateMatrix::value_type(
        comb01, ShoppingTrx::FlightMatrix::value_type(comb00, 0)));
    SolutionInFlightMatricesPredicate predicate(*_trx);

    CPPUNIT_ASSERT(!predicate(comb01));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SolutionInFlightMatricesPredicateTest);

} // fos
} // tse
