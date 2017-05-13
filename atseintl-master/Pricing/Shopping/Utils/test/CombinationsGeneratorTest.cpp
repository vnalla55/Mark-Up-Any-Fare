
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
#include "Pricing/Shopping/Utils/CombinationsGenerator.h"
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

class CombinationsGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CombinationsGeneratorTest);
  CPPUNIT_TEST(emptyGeneratorTest);
  CPPUNIT_TEST(oneElemSequence);
  CPPUNIT_TEST(twoElemSequence);
  CPPUNIT_TEST(threeElemSequence);
  CPPUNIT_TEST(resetTest);
  CPPUNIT_TEST(resetAfterExhausted);
  CPPUNIT_TEST(consecutiveLengthCombinations);
  CPPUNIT_TEST_SUITE_END();

public:
  void emptyGeneratorTest()
  {
    CombinationsGenerator<char> g(0);
    CPPUNIT_ASSERT_EQUAL(0u, g.getLength());
    CPPUNIT_ASSERT_EQUAL(0u, g.getElementsCount());
    emptyTest(g);

    CombinationsGenerator<char> h(1);
    CPPUNIT_ASSERT_EQUAL(1u, h.getLength());
    CPPUNIT_ASSERT_EQUAL(0u, h.getElementsCount());
    emptyTest(h);

    CombinationsGenerator<char> i(2);
    CPPUNIT_ASSERT_EQUAL(2u, i.getLength());
    CPPUNIT_ASSERT_EQUAL(0u, i.getElementsCount());
    emptyTest(i);
  }

  void oneElemSequence()
  {
    CombinationsGenerator<char> g0(0);
    g0.addElement('a');
    checkForEmptiness(g0);
    CPPUNIT_ASSERT_EQUAL(0u, g0.getLength());
    CPPUNIT_ASSERT_EQUAL(1u, g0.getElementsCount());

    CombinationsGenerator<char> g1(1);
    g1.addElement('a');
    vector<char> tmp;
    tmp += 'a';
    CPPUNIT_ASSERT(tmp == g1.next());
    checkForEmptiness(g1);
    CPPUNIT_ASSERT_EQUAL(1u, g1.getLength());
    CPPUNIT_ASSERT_EQUAL(1u, g1.getElementsCount());

    CombinationsGenerator<char> g2(2);
    g2.addElement('a');
    checkForEmptiness(g2);
    CPPUNIT_ASSERT_EQUAL(2u, g2.getLength());
    CPPUNIT_ASSERT_EQUAL(1u, g2.getElementsCount());
  }

  void twoElemSequence()
  {
    CombinationsGenerator<char> g0(0);
    g0.addElement('a');
    g0.addElement('b');
    checkForEmptiness(g0);

    CombinationsGenerator<char> g1(1);
    g1.addElement('a');
    g1.addElement('b');
    vector<char> tmp;
    tmp += 'a';
    CPPUNIT_ASSERT(tmp == g1.next());
    tmp[0] = 'b';
    CPPUNIT_ASSERT(tmp == g1.next());
    checkForEmptiness(g1);

    CombinationsGenerator<char> g2(2);
    g2.addElement('a');
    g2.addElement('b');
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g2.next());
    checkForEmptiness(g2);

    CombinationsGenerator<char> g3(3);
    g3.addElement('a');
    g3.addElement('b');
    checkForEmptiness(g3);
  }

  void threeElemSequence()
  {
    CombinationsGenerator<char> g0(0);
    g0.addElement('a');
    g0.addElement('b');
    g0.addElement('c');
    checkForEmptiness(g0);

    CombinationsGenerator<char> g1(1);
    g1.addElement('a');
    g1.addElement('b');
    g1.addElement('c');
    vector<char> tmp;
    tmp += 'a';
    CPPUNIT_ASSERT(tmp == g1.next());
    tmp[0] = 'b';
    CPPUNIT_ASSERT(tmp == g1.next());
    tmp[0] = 'c';
    CPPUNIT_ASSERT(tmp == g1.next());
    checkForEmptiness(g1);
    CPPUNIT_ASSERT_EQUAL(1u, g1.getLength());
    CPPUNIT_ASSERT_EQUAL(3u, g1.getElementsCount());

    CombinationsGenerator<char> g2(2);
    g2.addElement('a');
    g2.addElement('b');
    g2.addElement('c');
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g2.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g2.next());
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(tmp == g2.next());
    checkForEmptiness(g2);
    CPPUNIT_ASSERT_EQUAL(2u, g2.getLength());
    CPPUNIT_ASSERT_EQUAL(3u, g2.getElementsCount());

    CombinationsGenerator<char> g3(3);
    g3.addElement('a');
    g3.addElement('b');
    g3.addElement('c');
    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(tmp == g3.next());
    checkForEmptiness(g3);

    CombinationsGenerator<char> g4(4);
    g4.addElement('a');
    g4.addElement('b');
    g4.addElement('c');
    checkForEmptiness(g4);
  }

  void resetTest()
  {
    CombinationsGenerator<char> g(2);
    g.addElement('a');
    g.addElement('b');
    g.addElement('c');
    g.addElement('d');

    vector<char> tmp;
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'd';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());

    g.reset();

    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());

    g.reset(3);

    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'b', 'd';
    CPPUNIT_ASSERT(tmp == g.next());

    g.reset();

    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'b', 'd';
    CPPUNIT_ASSERT(tmp == g.next());

    g.reset(2);

    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
  }

  void resetAfterExhausted()
  {
    CombinationsGenerator<char> g(2);
    g.addElement('a');
    g.addElement('b');
    g.addElement('c');

    vector<char> tmp;
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());

    checkForEmptiness(g);
    g.reset();

    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());

    checkForEmptiness(g);
    g.reset(1);

    tmp.clear();
    tmp += 'a';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'c';
    CPPUNIT_ASSERT(tmp == g.next());

    checkForEmptiness(g);
  }

  // A real-life scenario
  void consecutiveLengthCombinations()
  {
    CombinationsGenerator<char> g(0);
    g.addElement('a');
    g.addElement('b');
    g.addElement('c');
    g.addElement('d');

    checkForEmptiness(g);
    g.reset(1);

    vector<char> tmp;
    tmp.clear();
    tmp += 'a';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'd';
    CPPUNIT_ASSERT(tmp == g.next());

    checkForEmptiness(g);
    g.reset(2);

    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'd';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b', 'd';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'c', 'd';
    CPPUNIT_ASSERT(tmp == g.next());

    checkForEmptiness(g);
    g.reset(3);

    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'b', 'd';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c', 'd';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b', 'c', 'd';
    CPPUNIT_ASSERT(tmp == g.next());

    checkForEmptiness(g);
    g.reset(4);

    tmp.clear();
    tmp += 'a', 'b', 'c', 'd';
    CPPUNIT_ASSERT(tmp == g.next());

    checkForEmptiness(g);
    g.reset(5);
    checkForEmptiness(g);
  }

private:
  typedef CombinationsGenerator<char> Generator;

  void checkForEmptiness(Generator& gen)
  {
    for (int i = 0; i < 10; ++i)
    {
      CPPUNIT_ASSERT(EMPTY == gen.next());
    }
  }

  void emptyTest(Generator& gen)
  {
    checkForEmptiness(gen);
    gen.reset();
    checkForEmptiness(gen);
    gen.reset(0);
    checkForEmptiness(gen);
    gen.reset(1);
    checkForEmptiness(gen);
    gen.reset(2);
    checkForEmptiness(gen);
  }

  vector<char> EMPTY;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationsGeneratorTest);

} // namespace utils

} // namespace tse
