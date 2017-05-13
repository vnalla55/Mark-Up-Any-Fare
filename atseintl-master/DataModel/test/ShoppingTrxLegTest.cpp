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

#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include <stdint.h>
#include <vector>

#include "DataModel/ShoppingTrx.h"

using namespace boost::assign;

namespace tse
{

class ShoppingTrxLegTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ShoppingTrxLegTest);
  CPPUNIT_TEST(testCreateCxrSopMap);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCreateCxrSopMap()
  {
    ShoppingTrx::Leg leg;

    std::vector<ItinIndex::Key> govCxrs;
    govCxrs += 0xAA, 0xBB, 0xBB, 0xAA, 0xCC, 0xBB, 0xCC, 0xAA;

    std::vector<uint32_t> sopMap;
    sopMap += 0x12345678, 0x12344321, 0x56788765, 0xACDF2345; // random data

    leg.createCxrSopMap(govCxrs, 0xAA, sopMap);

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), sopMap.size());
    CPPUNIT_ASSERT_EQUAL(0u, sopMap[0]);
    CPPUNIT_ASSERT_EQUAL(3u, sopMap[1]);
    CPPUNIT_ASSERT_EQUAL(7u, sopMap[2]);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ShoppingTrxLegTest);
}
