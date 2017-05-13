
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Pricing/Shopping/Utils/SetIntersection.h"

#include <vector>
#include <sstream>
#include <iostream>

using namespace tse::utils;

namespace tse
{

class SetIntersectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SetIntersectionTest);
  CPPUNIT_TEST(checkSetIntersection);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void checkSetIntersection()
  {
    SetIntersection<int> setIntersection;
    int array1[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
    int array2[] = { 2, 4, 6, 8, 10, 12, 14, 16, 18 };
    int array3[] = { 5, 6, 7, 99999 };

    std::set<int> set1, set2, set3, resultSet2, resultSet3, emptySet;
    set1.insert(array1, array1 + 10);
    set2.insert(array2, array2 + 9);
    set3.insert(array3, array3 + 4);

    setIntersection.addSet(set1);
    CPPUNIT_ASSERT(setIntersection.get() == set1);

    setIntersection.addSet(set2);
    int resultArray2[] = { 2, 4, 6, 8 };
    resultSet2.insert(resultArray2, resultArray2 + 4);

    CPPUNIT_ASSERT(setIntersection.get() == resultSet2);

    setIntersection.addSet(set3);
    resultSet3.insert(6);
    CPPUNIT_ASSERT(setIntersection.get() == resultSet3);

    setIntersection.addSet(emptySet);
    CPPUNIT_ASSERT(setIntersection.get().empty());
  }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SetIntersectionTest);
} // namespace tse
