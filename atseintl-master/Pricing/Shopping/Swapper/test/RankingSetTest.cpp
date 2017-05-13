//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/ErrorResponseException.h"
#include "Pricing/Shopping/Swapper/RankingSet.h"

#include <sstream>

using namespace std;

namespace tse
{

using namespace swp;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class RankingSetTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RankingSetTest);
  CPPUNIT_TEST(emptySetTest);
  CPPUNIT_TEST(singleElementTest);
  CPPUNIT_TEST(twoElementsSet1);
  CPPUNIT_TEST(twoElementsSet2);
  CPPUNIT_TEST(multipleElemsTest);
  CPPUNIT_TEST(outputTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void emptySetTest()
  {
    RankingSet<int> t;
    CPPUNIT_ASSERT_EQUAL(0u, t.getSize());
  }

  void singleElementTest()
  {
    RankingSet<int> t;
    t.insert(5);
    CPPUNIT_ASSERT_EQUAL(1u, t.getSize());
    CPPUNIT_ASSERT_EQUAL(0u, t.getRank(5));
    // No such element: 10
    CPPUNIT_ASSERT_THROW(t.getRank(10), ErrorResponseException);
  }

  void twoElementsSet1()
  {
    RankingSet<int> t;
    t.insert(13);
    t.insert(-2);
    CPPUNIT_ASSERT_EQUAL(2u, t.getSize());
    CPPUNIT_ASSERT_EQUAL(1u, t.getRank(13));
    CPPUNIT_ASSERT_EQUAL(0u, t.getRank(-2));
    // No such element: 0
    CPPUNIT_ASSERT_THROW(t.getRank(0), ErrorResponseException);
  }

  // Two elems, one group
  void twoElementsSet2()
  {
    RankingSet<int> t;
    t.insert(13);
    t.insert(13);

    CPPUNIT_ASSERT_EQUAL(1u, t.getSize());
    CPPUNIT_ASSERT_EQUAL(0u, t.getRank(13));

    // No such element: 99
    CPPUNIT_ASSERT_THROW(t.getRank(99), ErrorResponseException);
  }

  void multipleElemsTest()
  {
    RankingSet<int> t;
    t.insert(-5);
    t.insert(0);
    t.insert(-5);
    t.insert(17);
    t.insert(17);
    t.insert(0);
    t.insert(100);

    CPPUNIT_ASSERT_EQUAL(4u, t.getSize());

    CPPUNIT_ASSERT_EQUAL(0u, t.getRank(-5));
    CPPUNIT_ASSERT_EQUAL(1u, t.getRank(0));
    CPPUNIT_ASSERT_EQUAL(2u, t.getRank(17));
    CPPUNIT_ASSERT_EQUAL(3u, t.getRank(100));

    // No such element: 2
    CPPUNIT_ASSERT_THROW(t.getRank(2), ErrorResponseException);
  }

  void outputTest()
  {
    RankingSet<int> t;
    t.insert(17);
    t.insert(-6);
    t.insert(-6);
    t.insert(92);
    t.insert(5);
    t.insert(-6);
    t.insert(5);
    t.insert(-6);

    ostringstream out;
    out << t;

    string expected = "RANKING SET with 4 elements: -6 5 17 92\n";
    CPPUNIT_ASSERT_EQUAL(expected, out.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RankingSetTest);

} // namespace tse
