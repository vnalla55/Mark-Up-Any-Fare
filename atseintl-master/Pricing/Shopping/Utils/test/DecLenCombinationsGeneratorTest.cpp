
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
#include "Pricing/Shopping/Utils/DecLenCombinationsGenerator.h"
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

class DecLenCombinationsGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DecLenCombinationsGeneratorTest);
  CPPUNIT_TEST(emptyGeneratorTest);
  CPPUNIT_TEST(oneElemSequence);
  CPPUNIT_TEST(twoElemSequence);
  CPPUNIT_TEST(threeElemSequence);
  CPPUNIT_TEST(fourElemSequence);
  CPPUNIT_TEST(resetTest);
  CPPUNIT_TEST(resetAfterExhausted);
  CPPUNIT_TEST_SUITE_END();

public:
  void emptyGeneratorTest()
  {
    DecLenCombinationsGenerator<char> g;
    CPPUNIT_ASSERT_EQUAL(0u, g.getElementsCount());
    g.reset();
    CPPUNIT_ASSERT_EQUAL(0u, g.getElementsCount());
    checkForEmptiness(g);
  }

  void oneElemSequence()
  {
    DecLenCombinationsGenerator<char> g;
    g.addElement('a');
    CPPUNIT_ASSERT_EQUAL(1u, g.getElementsCount());
    vector<char> tmp;

    tmp.clear();
    tmp += 'a';
    CPPUNIT_ASSERT(tmp == g.next());
    checkForEmptiness(g);
  }

  void twoElemSequence()
  {
    DecLenCombinationsGenerator<char> g;
    g.addElement('a');
    g.addElement('b');
    CPPUNIT_ASSERT_EQUAL(2u, g.getElementsCount());
    vector<char> tmp;

    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    CPPUNIT_ASSERT_EQUAL(2u, g.getElementsCount());
    checkForEmptiness(g);
  }

  void threeElemSequence()
  {
    DecLenCombinationsGenerator<char> g;
    CPPUNIT_ASSERT_EQUAL(0u, g.getElementsCount());
    g.addElement('a');
    g.addElement('b');
    CPPUNIT_ASSERT_EQUAL(2u, g.getElementsCount());
    g.addElement('c');
    CPPUNIT_ASSERT_EQUAL(3u, g.getElementsCount());
    vector<char> tmp;

    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    CPPUNIT_ASSERT_EQUAL(3u, g.getElementsCount());
    tmp.clear();
    tmp += 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    checkForEmptiness(g);
  }

  void fourElemSequence()
  {
    DecLenCombinationsGenerator<char> g;
    g.addElement('a');
    g.addElement('b');
    g.addElement('c');
    g.addElement('d');
    vector<char> tmp;

    tmp.clear();
    tmp += 'a', 'b', 'c', 'd';
    CPPUNIT_ASSERT(tmp == g.next());
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
  }

  void resetTest()
  {
    DecLenCombinationsGenerator<char> g;
    g.addElement('a');
    g.addElement('b');
    g.addElement('c');

    vector<char> tmp;

    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());

    CPPUNIT_ASSERT_EQUAL(3u, g.getElementsCount());
    g.reset();
    CPPUNIT_ASSERT_EQUAL(3u, g.getElementsCount());
    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    CPPUNIT_ASSERT_EQUAL(3u, g.getElementsCount());
  }

  void resetAfterExhausted()
  {
    DecLenCombinationsGenerator<char> g;
    g.addElement('a');
    g.addElement('b');
    g.addElement('c');
    vector<char> tmp;

    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
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

    g.reset();
    tmp.clear();
    tmp += 'a', 'b', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'b';
    CPPUNIT_ASSERT(tmp == g.next());
    tmp.clear();
    tmp += 'a', 'c';
    CPPUNIT_ASSERT(tmp == g.next());
  }

private:
  typedef DecLenCombinationsGenerator<char> Generator;

  void checkForEmptiness(Generator& gen)
  {
    for (int i = 0; i < 10; ++i)
    {
      CPPUNIT_ASSERT(EMPTY == gen.next());
    }
  }

  vector<char> EMPTY;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DecLenCombinationsGeneratorTest);

} // namespace utils

} // namespace tse
