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
#include "Pricing/Shopping/Utils/PrettyPrint.h"
#include "Pricing/Shopping/Utils/UsageProgressGenerator.h"
#include "Pricing/Shopping/Utils/StreamLogger.h"
#include <boost/assign/std/vector.hpp>

#include <vector>
#include <iostream>

namespace tse
{

namespace utils
{

using namespace std;
using boost::assign::operator+=;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class DimensionDataTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DimensionDataTest);
  CPPUNIT_TEST(emptyDimensionDataTest);
  CPPUNIT_TEST(addUnusedTest);
  CPPUNIT_TEST(addUsedTest);
  CPPUNIT_TEST(addMixedTest);
  CPPUNIT_TEST(addDuplicatesTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void emptyDimensionDataTest()
  {
    DD dd;
    CPPUNIT_ASSERT_EQUAL(EMPTY, dd.getType());

    CPPUNIT_ASSERT(EMPTY_SET == dd.getUnusedElements());
    CPPUNIT_ASSERT(EMPTY_SET == dd.getUsedElements());

    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(5));
    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(19));
  }

  void addUnusedTest()
  {
    DD dd;
    dd.addElement(5, false);

    CPPUNIT_ASSERT_EQUAL(UNUSED, dd.getType());

    DD::ElementSetType myUnused;
    myUnused.insert(5);
    CPPUNIT_ASSERT(myUnused == dd.getUnusedElements());
    CPPUNIT_ASSERT(EMPTY_SET == dd.getUsedElements());

    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(19));
    CPPUNIT_ASSERT_EQUAL(1u, dd.removeElement(5));

    CPPUNIT_ASSERT_EQUAL(EMPTY, dd.getType());

    CPPUNIT_ASSERT(EMPTY_SET == dd.getUnusedElements());
    CPPUNIT_ASSERT(EMPTY_SET == dd.getUsedElements());

    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(5));
    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(19));
  }

  void addUsedTest()
  {
    DD dd;
    dd.addElement(20, true);

    CPPUNIT_ASSERT_EQUAL(USED, dd.getType());

    DD::ElementSetType myUsed;
    myUsed.insert(20);
    CPPUNIT_ASSERT(EMPTY_SET == dd.getUnusedElements());
    CPPUNIT_ASSERT(myUsed == dd.getUsedElements());

    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(19));
    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(5));
    CPPUNIT_ASSERT_EQUAL(1u, dd.removeElement(20));

    CPPUNIT_ASSERT_EQUAL(EMPTY, dd.getType());

    CPPUNIT_ASSERT(EMPTY_SET == dd.getUnusedElements());
    CPPUNIT_ASSERT(EMPTY_SET == dd.getUsedElements());

    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(5));
    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(19));
  }

  void addMixedTest()
  {
    DD dd;
    dd.addElement(7, false);
    dd.addElement(20, true);

    CPPUNIT_ASSERT_EQUAL(MIXED, dd.getType());

    DD::ElementSetType myUnused;
    myUnused.insert(7);
    DD::ElementSetType myUsed;
    myUsed.insert(20);

    CPPUNIT_ASSERT(myUnused == dd.getUnusedElements());
    CPPUNIT_ASSERT(myUsed == dd.getUsedElements());

    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(19));
    CPPUNIT_ASSERT_EQUAL(0u, dd.removeElement(5));

    dd.addElement(77, true);
    myUsed.insert(77);

    CPPUNIT_ASSERT_EQUAL(MIXED, dd.getType());

    CPPUNIT_ASSERT(myUnused == dd.getUnusedElements());
    CPPUNIT_ASSERT(myUsed == dd.getUsedElements());

    CPPUNIT_ASSERT_EQUAL(1u, dd.removeElement(20));
    CPPUNIT_ASSERT_EQUAL(1u, dd.removeElement(77));
    CPPUNIT_ASSERT_EQUAL(UNUSED, dd.getType());

    CPPUNIT_ASSERT(myUnused == dd.getUnusedElements());
    CPPUNIT_ASSERT(EMPTY_SET == dd.getUsedElements());
  }

  void addDuplicatesTest()
  {
    DD dd;
    dd.addElement(7, false);
    CPPUNIT_ASSERT_THROW(dd.addElement(7, true), tse::ErrorResponseException);
  }

private:
  typedef DimensionData<unsigned int> DD;
  const DD::ElementSetType EMPTY_SET;
};

class DimSelectionGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DimSelectionGeneratorTest);
  CPPUNIT_TEST(emptyGeneratorTest);
  CPPUNIT_TEST(errorAddingEmpty);
  CPPUNIT_TEST(oneUnused);
  CPPUNIT_TEST(oneUsed);
  CPPUNIT_TEST(unusedUsed);
  CPPUNIT_TEST(usedUnusedUsedUsedUnusedUnused);
  CPPUNIT_TEST(mixed);
  CPPUNIT_TEST(twoMixed);
  CPPUNIT_TEST(usage1);
  CPPUNIT_TEST(usage2);
  CPPUNIT_TEST(onlyUsed);
  CPPUNIT_TEST_SUITE_END();

public:
  void emptyGeneratorTest()
  {
    DimSelectionGenerator g;
    assertEmpty(g);
  }

  void errorAddingEmpty()
  {
    DimSelectionGenerator g;
    CPPUNIT_ASSERT_THROW(g.addDimension(EMPTY), tse::ErrorResponseException);
  }

  void oneUnused()
  {
    DimSelectionGenerator g;
    g.addDimension(UNUSED);
    Vec v;

    v.clear();
    v += false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    assertEmpty(g);
  }

  void oneUsed()
  {
    DimSelectionGenerator g;
    g.addDimension(USED);
    assertEmpty(g);
  }

  void unusedUsed()
  {
    DimSelectionGenerator g;
    g.addDimension(UNUSED);
    g.addDimension(USED);

    Vec v;
    v.clear();
    v += false, true;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    assertEmpty(g);
  }

  void usedUnusedUsedUsedUnusedUnused()
  {
    DimSelectionGenerator g;
    g.addDimension(USED);
    g.addDimension(UNUSED);
    g.addDimension(USED);
    g.addDimension(USED);
    g.addDimension(UNUSED);
    g.addDimension(UNUSED);

    Vec v;
    v.clear();
    v += true, false, true, true, false, false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    assertEmpty(g);
  }

  void mixed()
  {
    DimSelectionGenerator g;
    g.addDimension(MIXED);
    Vec v;
    v.clear();
    v += false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    assertEmpty(g);
  }

  void twoMixed()
  {
    DimSelectionGenerator g;
    g.addDimension(MIXED);
    g.addDimension(MIXED);
    Vec v;
    v.clear();
    v += false, false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    v.clear();
    v += false, true;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    v.clear();
    v += true, false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    assertEmpty(g);
  }

  void usage1()
  {
    DimSelectionGenerator g;
    g.addDimension(USED);
    g.addDimension(MIXED);
    g.addDimension(UNUSED);
    g.addDimension(MIXED);

    Vec v;
    v.clear();
    v += true, false, false, false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    v.clear();
    v += true, false, false, true;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    v.clear();
    v += true, true, false, false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    v.clear();
    v += true, true, false, true;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    assertEmpty(g);
  }

  void usage2()
  {
    DimSelectionGenerator g;
    g.addDimension(MIXED);
    g.addDimension(USED);
    g.addDimension(MIXED);
    g.addDimension(MIXED);

    Vec v;
    // C(3, 3)
    v.clear();
    v += false, true, false, false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));

    // C(2, 3)
    v.clear();
    v += false, true, false, true;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    v.clear();
    v += false, true, true, false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    v.clear();
    v += true, true, false, false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));

    // C (1, 3)
    v.clear();
    v += false, true, true, true;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    v.clear();
    v += true, true, false, true;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));
    v.clear();
    v += true, true, true, false;
    CPPUNIT_ASSERT(equalCombinations(v, g.next()));

    assertEmpty(g);
  }

  void onlyUsed()
  {
    DimSelectionGenerator g;
    g.addDimension(USED);
    g.addDimension(USED);
    g.addDimension(USED);
    g.addDimension(USED);
    g.addDimension(USED);
    g.addDimension(USED);
    g.addDimension(USED);
    assertEmpty(g);
  }

private:
  typedef vector<bool> Vec;

  void assertEmpty(DimSelectionGenerator& g)
  {
    for (unsigned int i = 0; i < 20; ++i)
    {
      CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(g.next().size()));
    }
  }

  bool equalCombinations(const Vec& a, const Vec& b)
  {
    if (a == b)
    {
      return true;
    }
    cout << endl;
    cout << "Left : " << a << endl;
    cout << "Right: " << b << endl;
    return false;
  }
};

class UsageProgressGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(UsageProgressGeneratorTest);
  CPPUNIT_TEST(buildWithZeroDimension);
  CPPUNIT_TEST(emptyGeneratorTest);
  CPPUNIT_TEST(fetchTest);
  CPPUNIT_TEST(removeTest);
  CPPUNIT_TEST(generateEmptyDimension);
  CPPUNIT_TEST(generateNoUnusedElems);
  CPPUNIT_TEST(simpleScenario1);
  CPPUNIT_TEST(simpleScenario2);
  CPPUNIT_TEST(simpleScenario3);
  CPPUNIT_TEST(simpleScenario4);
  CPPUNIT_TEST(bigScenario);
  CPPUNIT_TEST(removingScenario);
  CPPUNIT_TEST_SUITE_END();

public:
  void buildWithZeroDimension() { CPPUNIT_ASSERT_THROW(UPG(0), tse::ErrorResponseException); }

  void emptyGeneratorTest()
  {
    UPG upg(3);
    CPPUNIT_ASSERT_EQUAL(3u, upg.getDimensionsCount());
    CPPUNIT_ASSERT_EQUAL(0u, upg.getSize());
    CPPUNIT_ASSERT_EQUAL(0u, upg.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(0u, upg.getUsedCount());
    upg.reset();
    CPPUNIT_ASSERT_EQUAL(3u, upg.getDimensionsCount());
    CPPUNIT_ASSERT_EQUAL(0u, upg.getSize());
    CPPUNIT_ASSERT_EQUAL(0u, upg.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(0u, upg.getUsedCount());
  }

  void fetchTest()
  {
    UPG upg(2);
    upg.addElement('a', 0, false);
    upg.addElement('B', 1, true);
    CPPUNIT_ASSERT_EQUAL(2u, upg.getDimensionsCount());
    CPPUNIT_ASSERT_EQUAL(2u, upg.getSize());
    CPPUNIT_ASSERT_EQUAL(1u, upg.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(1u, upg.getUsedCount());

    upg.reset();
    CPPUNIT_ASSERT_EQUAL(2u, upg.getDimensionsCount());
    CPPUNIT_ASSERT_EQUAL(2u, upg.getSize());
    CPPUNIT_ASSERT_EQUAL(1u, upg.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(1u, upg.getUsedCount());

    upg.next();
    upg.next();
    CPPUNIT_ASSERT_EQUAL(2u, upg.getDimensionsCount());
    CPPUNIT_ASSERT_EQUAL(2u, upg.getSize());
    CPPUNIT_ASSERT_EQUAL(1u, upg.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(1u, upg.getUsedCount());
  }

  void removeTest()
  {
    UPG upg(2);
    upg.addElement('a', 0, false);
    upg.addElement('B', 1, true);

    CPPUNIT_ASSERT_THROW(upg.addElement('x', 2, false), tse::ErrorResponseException);

    CPPUNIT_ASSERT_EQUAL(0u, upg.removeElement('c', 0));
    CPPUNIT_ASSERT_EQUAL(0u, upg.removeElement('c', 1));
    CPPUNIT_ASSERT_THROW(upg.removeElement('c', 2), tse::ErrorResponseException);

    CPPUNIT_ASSERT_EQUAL(1u, upg.removeElement('B', 1));

    CPPUNIT_ASSERT_EQUAL(2u, upg.getDimensionsCount());
    CPPUNIT_ASSERT_EQUAL(1u, upg.getSize());
    CPPUNIT_ASSERT_EQUAL(1u, upg.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(0u, upg.getUsedCount());
  }

  void generateEmptyDimension()
  {
    UPG upg(2);
    upg.addElement('a', 0, false);
    CPPUNIT_ASSERT_THROW(upg.next(), tse::ErrorResponseException);
  }

  void generateNoUnusedElems()
  {
    UPG upg(2);
    upg.addElement('x', 0, true);
    upg.addElement('y', 1, true);
    assertEmpty(upg);
  }

  void simpleScenario1()
  {
    UPG g(2);
    g.addElement('A', 0, true);
    g.addElement('a', 0, false);
    g.addElement('b', 1, false);

    // a  b
    // A

    Comb tmp;
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));

    tmp.clear();
    tmp += 'A', 'b';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));

    assertEmpty(g);
  }

  void simpleScenario2()
  {
    UPG g(2);
    // A B
    g.addElement('A', 0, true);
    g.addElement('B', 1, true);
    assertEmpty(g);
  }

  void simpleScenario3()
  {
    UPG g(2);
    // a   c
    // b
    //     C
    // A   D
    g.addElement('a', 0, false);
    g.addElement('b', 0, false);
    g.addElement('c', 1, false);

    g.addElement('A', 0, true);
    g.addElement('C', 1, true);
    g.addElement('D', 1, true);

    Comb tmp;

    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));

    tmp.clear();
    tmp += 'a', 'C';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
    tmp.clear();
    tmp += 'a', 'D';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
    tmp.clear();
    tmp += 'b', 'C';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));

    g.reset();

    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));

    tmp.clear();
    tmp += 'a', 'C';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
    tmp.clear();
    tmp += 'a', 'D';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
    tmp.clear();
    tmp += 'b', 'C';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
    tmp.clear();
    tmp += 'b', 'D';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));

    tmp.clear();
    tmp += 'A', 'c';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));

    assertEmpty(g);

    g.reset();

    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
  }

  void simpleScenario4()
  {
    UPG g(3);
    // a   b   c
    g.addElement('a', 0, false);
    g.addElement('b', 1, false);
    g.addElement('c', 2, false);

    Comb tmp;
    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(equalCombinations(tmp, g.next()));
    assertEmpty(g);
  }

  // Dimensions:      d1      d2      d3
  // -----------------------------------
  // Unused elements:  5       2       5
  //                   7              12
  //                  31
  //
  // Used elements:    2       6      11
  //                           8
  void bigScenario()
  {
    UsageProgressGenerator<unsigned int> g(3);
    g.addElement(5, 0, false);
    g.addElement(7, 0, false);
    g.addElement(31, 0, false);
    g.addElement(2, 1, false);
    g.addElement(12, 2, false);
    g.addElement(5, 2, false);

    g.addElement(2, 0, true);
    g.addElement(8, 1, true);
    g.addElement(6, 1, true);
    g.addElement(11, 2, true);

    CPPUNIT_ASSERT(checkNext(g, 5, 2, 5));
    CPPUNIT_ASSERT(checkNext(g, 5, 2, 12));
    CPPUNIT_ASSERT(checkNext(g, 7, 2, 5));
    CPPUNIT_ASSERT(checkNext(g, 7, 2, 12));
    CPPUNIT_ASSERT(checkNext(g, 31, 2, 5));
    CPPUNIT_ASSERT(checkNext(g, 31, 2, 12));

    CPPUNIT_ASSERT(checkNext(g, 5, 2, 11));
    CPPUNIT_ASSERT(checkNext(g, 7, 2, 11));
    CPPUNIT_ASSERT(checkNext(g, 31, 2, 11));

    CPPUNIT_ASSERT(checkNext(g, 5, 6, 5));
    CPPUNIT_ASSERT(checkNext(g, 5, 6, 12));
    CPPUNIT_ASSERT(checkNext(g, 5, 8, 5));
    CPPUNIT_ASSERT(checkNext(g, 5, 8, 12));
    CPPUNIT_ASSERT(checkNext(g, 7, 6, 5));
    CPPUNIT_ASSERT(checkNext(g, 7, 6, 12));
    CPPUNIT_ASSERT(checkNext(g, 7, 8, 5));
    CPPUNIT_ASSERT(checkNext(g, 7, 8, 12));
    CPPUNIT_ASSERT(checkNext(g, 31, 6, 5));
    CPPUNIT_ASSERT(checkNext(g, 31, 6, 12));
    CPPUNIT_ASSERT(checkNext(g, 31, 8, 5));
    CPPUNIT_ASSERT(checkNext(g, 31, 8, 12));

    CPPUNIT_ASSERT(checkNext(g, 2, 2, 5));
    CPPUNIT_ASSERT(checkNext(g, 2, 2, 12));

    CPPUNIT_ASSERT(checkNext(g, 5, 6, 11));
    CPPUNIT_ASSERT(checkNext(g, 5, 8, 11));
    CPPUNIT_ASSERT(checkNext(g, 7, 6, 11));
    CPPUNIT_ASSERT(checkNext(g, 7, 8, 11));
    CPPUNIT_ASSERT(checkNext(g, 31, 6, 11));
    CPPUNIT_ASSERT(checkNext(g, 31, 8, 11));

    CPPUNIT_ASSERT(checkNext(g, 2, 2, 11));

    CPPUNIT_ASSERT(checkNext(g, 2, 6, 5));
    CPPUNIT_ASSERT(checkNext(g, 2, 6, 12));
    CPPUNIT_ASSERT(checkNext(g, 2, 8, 5));
    CPPUNIT_ASSERT(checkNext(g, 2, 8, 12));

    for (unsigned int i = 0; i < 20; ++i)
    {
      CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(g.next().size()));
    }
  }

  void removingScenario()
  {
    // Dimensions:      d1      d2      d3
    // -----------------------------------
    // Unused elements:  5       2       5
    //                   7              12
    //                  31
    //
    // Used elements:    2       6      11
    //                           8

    UsageProgressGenerator<unsigned int> g(3);
    g.addElement(5, 0, false);
    g.addElement(7, 0, false);
    g.addElement(31, 0, false);
    g.addElement(2, 1, false);
    g.addElement(12, 2, false);
    g.addElement(5, 2, false);

    g.addElement(2, 0, true);
    g.addElement(8, 1, true);
    g.addElement(6, 1, true);
    g.addElement(11, 2, true);

    CPPUNIT_ASSERT(checkNext(g, 5, 2, 5));
    CPPUNIT_ASSERT(checkNext(g, 5, 2, 12));
    CPPUNIT_ASSERT(checkNext(g, 7, 2, 5));
    // This combination is OK
    // we mark these elements as used now (2, 5)
    // or remove (7) e.g. no more 7 usages possible
    g.removeElement(7, 0);
    g.removeElement(2, 1);
    g.removeElement(5, 2);
    g.addElement(2, 1, true);
    g.addElement(5, 2, true);

    // Dimensions:      d1      d2      d3
    // -----------------------------------
    // Unused elements:  5              12
    //                  31
    //
    // Used elements:    2       2       5
    //                           6      11
    //                           8
    CPPUNIT_ASSERT_EQUAL(3u, g.getDimensionsCount());
    CPPUNIT_ASSERT_EQUAL(3u, g.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(6u, g.getUsedCount());
    CPPUNIT_ASSERT_EQUAL(9u, g.getSize());

    CPPUNIT_ASSERT(checkNext(g, 5, 2, 12));
    CPPUNIT_ASSERT(checkNext(g, 5, 6, 12));
    CPPUNIT_ASSERT(checkNext(g, 5, 8, 12));
    CPPUNIT_ASSERT(checkNext(g, 31, 2, 12));
    CPPUNIT_ASSERT(checkNext(g, 31, 6, 12));
    CPPUNIT_ASSERT(checkNext(g, 31, 8, 12));
    CPPUNIT_ASSERT(checkNext(g, 5, 2, 5));
    g.removeElement(5, 0);
    g.removeElement(5, 2);
    g.addElement(5, 0, true);

    g.addElement(99, 1, false);
    g.removeElement(6, 1);

    // Dimensions:      d1      d2      d3
    // -----------------------------------
    // Unused elements: 31      99      12
    //
    // Used elements:    2       2      11
    //                   5       8

    CPPUNIT_ASSERT_EQUAL(3u, g.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(5u, g.getUsedCount());
    CPPUNIT_ASSERT_EQUAL(8u, g.getSize());

    CPPUNIT_ASSERT(checkNext(g, 31, 99, 12));
    CPPUNIT_ASSERT(checkNext(g, 31, 99, 11));
    CPPUNIT_ASSERT(checkNext(g, 31, 2, 12));
    CPPUNIT_ASSERT(checkNext(g, 31, 8, 12));
    CPPUNIT_ASSERT(checkNext(g, 2, 99, 12));

    CPPUNIT_ASSERT_EQUAL(3u, g.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(5u, g.getUsedCount());
    CPPUNIT_ASSERT_EQUAL(8u, g.getSize());

    g.removeElement(2, 0);
    g.removeElement(99, 1);
    g.removeElement(12, 2);

    // Dimensions:      d1      d2      d3
    // -----------------------------------
    // Unused elements: 31
    //
    // Used elements:            2      11
    //                   5       8

    CPPUNIT_ASSERT_EQUAL(1u, g.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(4u, g.getUsedCount());
    CPPUNIT_ASSERT_EQUAL(5u, g.getSize());

    CPPUNIT_ASSERT(checkNext(g, 31, 2, 11));
    CPPUNIT_ASSERT(checkNext(g, 31, 8, 11));

    for (unsigned int i = 0; i < 20; ++i)
    {
      CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(g.next().size()));
    }

    g.removeElement(2, 1);
    g.removeElement(5, 0);

    // Dimensions:      d1      d2      d3
    // -----------------------------------
    // Unused elements: 31
    //
    // Used elements:            8      11

    CPPUNIT_ASSERT(checkNext(g, 31, 8, 11));
    for (unsigned int i = 0; i < 20; ++i)
    {
      CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(g.next().size()));
    }

    g.removeElement(11, 2);
    CPPUNIT_ASSERT_EQUAL(1u, g.getUnusedCount());
    CPPUNIT_ASSERT_EQUAL(1u, g.getUsedCount());
    CPPUNIT_ASSERT_EQUAL(2u, g.getSize());
    // dim 2 empty
    CPPUNIT_ASSERT_THROW(g.next(), tse::ErrorResponseException);

    // recovery - add sth to dim 2
    g.addElement(55, 2, false);
    CPPUNIT_ASSERT(checkNext(g, 31, 8, 55));
    for (unsigned int i = 0; i < 20; ++i)
    {
      CPPUNIT_ASSERT_EQUAL(0u, static_cast<unsigned int>(g.next().size()));
    }
  }

private:
  typedef UsageProgressGenerator<char> UPG;
  typedef vector<char> Comb;

  bool
  checkNext(UsageProgressGenerator<unsigned int>& g, unsigned int a, unsigned int b, unsigned int c)
  {
    std::vector<unsigned int> tmp;
    tmp += a, b, c;
    return equalCombinations(tmp, g.next());
  }

  void assertEmpty(UPG& upg)
  {
    for (unsigned int i = 0; i < 20; ++i)
    {
      CPPUNIT_ASSERT(EMPTY_COMB == upg.next());
    }
  }

  bool equalCombinations(const Comb& a, const Comb& b)
  {
    if (a == b)
    {
      return true;
    }
    cout << endl;
    cout << "Left : " << a << endl;
    cout << "Right: " << b << endl;
    return false;
  }

  bool equalCombinations(const std::vector<unsigned int>& a, const std::vector<unsigned int>& b)
  {
    if (a == b)
    {
      return true;
    }
    cout << endl;
    cout << "Left : " << a << endl;
    cout << "Right: " << b << endl;
    return false;
  }

  const Comb EMPTY_COMB;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DimensionDataTest);
CPPUNIT_TEST_SUITE_REGISTRATION(DimSelectionGeneratorTest);
CPPUNIT_TEST_SUITE_REGISTRATION(UsageProgressGeneratorTest);

} // namespace utils

} // namespace tse
