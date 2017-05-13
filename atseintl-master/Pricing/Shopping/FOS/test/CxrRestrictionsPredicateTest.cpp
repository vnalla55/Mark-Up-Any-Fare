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

#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Pricing/Shopping/FOS/CxrRestrictionsPredicate.h"

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

class CxrRestrictionsPredicateTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CxrRestrictionsPredicateTest);

  CPPUNIT_TEST(testValidOnline);
  CPPUNIT_TEST(testValidNotRestricted);
  CPPUNIT_TEST(testNotValid);

  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
  CxrRestrictionsPredicate* _predicate;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);

    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();

    _predicate = _memHandle.create<CxrRestrictionsPredicate>(*_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void testValidOnline()
  {
    std::vector<int> comb;
    comb += 0, 0;

    CPPUNIT_ASSERT(_predicate->operator()(comb));
  }

  void testValidNotRestricted()
  {
    std::vector<int> comb;
    comb += 0, 1;

    CPPUNIT_ASSERT(_predicate->operator()(comb));
  }

  void testNotValid()
  {
    std::vector<int> comb;
    comb += 0, 1;

    _trx->legs()[1].sop()[1].combineSameCxr() = true;

    CPPUNIT_ASSERT(!_predicate->operator()(comb));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CxrRestrictionsPredicateTest);

} // fos
} // tse
