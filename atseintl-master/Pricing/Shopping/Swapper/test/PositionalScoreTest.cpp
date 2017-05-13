
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

#include "Pricing/Shopping/Swapper/PositionalScore.h"

#include <string>
#include <iostream>
#include <sstream>

using namespace std;

namespace tse
{

using namespace swp;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class PositionalScoreTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PositionalScoreTest);
  CPPUNIT_TEST(testEmptyScore);
  CPPUNIT_TEST(setBadSizeTest);
  CPPUNIT_TEST(initializationTest);
  CPPUNIT_TEST(noResizeTest);
  CPPUNIT_TEST(comparisonTest1);
  CPPUNIT_TEST(comparisonTest2);
  CPPUNIT_TEST(parseTest);
  CPPUNIT_TEST(invalidParseTest);
  CPPUNIT_TEST(outputTest);

  CPPUNIT_TEST_SUITE_END();

public:
  void testEmptyScore()
  {
    PositionalScore ps;
    ostringstream out;
    out << ps;
    CPPUNIT_ASSERT_EQUAL(string("(score empty)"), out.str());
    CPPUNIT_ASSERT_EQUAL((unsigned int)0, ps.getWordsNbr());
    CPPUNIT_ASSERT_EQUAL((unsigned int)0, ps.getWordWidth());

    CPPUNIT_ASSERT_THROW(ps.getWord(0), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.getWord(17), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.getValue(0, 0), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.getValue(3, 938), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.setValue(0, 0, 5), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.setValue(1, 2, 3), ErrorResponseException);
  }

  void setBadSizeTest()
  {
    PositionalScore ps;
    CPPUNIT_ASSERT_THROW(ps.setSize(0, 0), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.setSize(0, 10), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.setSize(10, 0), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.setSize(0, 0, 5), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.setSize(0, 10, -7), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(ps.setSize(10, 0, 358), ErrorResponseException);
  }

  void initializationTest()
  {
    for (unsigned int i = 1; i < 5; ++i)
    {
      for (unsigned int j = 1; j < 5; ++j)
      {
        PositionalScore s(i, j);
        checkPropertiesOfInitializedScore(s, i, j, 0);
        checkIfCopyingIsCorrect(s);

        for (int v = -3; v < 4; ++v)
        {
          PositionalScore ps(i, j, v);
          checkPropertiesOfInitializedScore(ps, i, j, v);
          checkIfCopyingIsCorrect(ps);

          PositionalScore ps2;
          ps2.setSize(i, j, v);
          checkIfScoresAreIdentical(ps, ps2);
          checkPropertiesOfInitializedScore(ps2, i, j, v);
          checkIfCopyingIsCorrect(ps2);
        }
      }
    }
  }

  void noResizeTest()
  {
    PositionalScore s(5, 5, 10);
    CPPUNIT_ASSERT_THROW(s.setSize(5, 6, 13), ErrorResponseException);
  }

  void comparisonTest1()
  {
    CPPUNIT_ASSERT(make("") == make("(empty score)"));
    CPPUNIT_ASSERT(make("[5]") != make("[6]"));

    CPPUNIT_ASSERT(make("[5 -700]") == make("[5 -700]"));
    CPPUNIT_ASSERT(make("[5 -700]") != make("[4 -700]"));
    CPPUNIT_ASSERT(make("[5][6]") == make("[5][6]"));
    CPPUNIT_ASSERT(make("[5][6][-2]") == make("[5][6][-2]"));

    CPPUNIT_ASSERT(make("[-28][-3]") == make("[-28][-3]"));
    CPPUNIT_ASSERT(make("[-2][-3]") != make("[-3][-2]"));
  }

  void comparisonTest2()
  {
    CPPUNIT_ASSERT(make("[5]") < make("[8]"));
    CPPUNIT_ASSERT(make("[5][5600]") < make("[6][0]"));
    CPPUNIT_ASSERT(make("[0 0][150 1000]") < make("[0 1][-1000 -300]"));
    CPPUNIT_ASSERT(make("[0 0][150 1000]") < make("[1 0][-1000 -300]"));
    CPPUNIT_ASSERT(make("[0 178][150 1000]") < make("[1 0][-1000 -300]"));
    CPPUNIT_ASSERT(make("[0 178][150 1000]") < make("[0 178][150 1001]"));
  }

  void parseTest()
  {
    PositionalScore empty;
    PositionalScore seven(1, 1, 7);

    PositionalScore s;
    s = PositionalScoreFormatter::scoreFromString("(empty score)");
    checkIfScoresAreIdentical(s, empty);
    s = PositionalScoreFormatter::scoreFromString("");
    checkIfScoresAreIdentical(s, empty);
    s = PositionalScoreFormatter::scoreFromString("[7]");
    checkIfScoresAreIdentical(s, seven);
    s = PositionalScoreFormatter::scoreFromString("[ 7 ]");
    checkIfScoresAreIdentical(s, seven);
    s = PositionalScoreFormatter::scoreFromString("[7 ]");
    checkIfScoresAreIdentical(s, seven);
    s = PositionalScoreFormatter::scoreFromString("[ 7]");
    checkIfScoresAreIdentical(s, seven);
    s = PositionalScoreFormatter::scoreFromString(" [   7 ]  ");
    checkIfScoresAreIdentical(s, seven);

    s = PositionalScoreFormatter::scoreFromString("[4] [ -19]");
    PositionalScore s2(2, 1);
    s2.setValue(0, 0, 4);
    s2.setValue(1, 0, -19);
    checkIfScoresAreIdentical(s, s2);

    s = PositionalScoreFormatter::scoreFromString("[   -4 5 ] [    708 -19]");
    PositionalScore s3(2, 2);
    s3.setValue(0, 0, -4);
    s3.setValue(0, 1, 5);
    s3.setValue(1, 0, 708);
    s3.setValue(1, 1, -19);
    checkIfScoresAreIdentical(s, s3);

    s = PositionalScoreFormatter::scoreFromString("[1557 56 -3][478 -35 0]");
    PositionalScore s4(2, 3);
    s4.setValue(0, 0, 1557);
    s4.setValue(0, 1, 56);
    s4.setValue(0, 2, -3);
    s4.setValue(1, 0, 478);
    s4.setValue(1, 1, -35);
    s4.setValue(1, 2, 0);
    checkIfScoresAreIdentical(s, s4);
  }

  void invalidParseTest()
  {
    CPPUNIT_ASSERT_THROW(PositionalScoreFormatter::scoreFromString("[5] [8 9]"),
                         ErrorResponseException);
  }

  void outputTest()
  {
    ostringstream out;

    PositionalScore s0;
    out << s0;
    CPPUNIT_ASSERT_EQUAL(string("(score empty)"), out.str());
    out.str("");

    PositionalScore s1(2, 3);
    out << s1;
    CPPUNIT_ASSERT_EQUAL(string("[0 0 0][0 0 0]"), out.str());
    out.str("");

    PositionalScore s2(1, 1, 13);
    out << s2;
    CPPUNIT_ASSERT_EQUAL(string("[13]"), out.str());
    out.str("");

    PositionalScore s3;
    s3.setSize(3, 2, -7);
    out << s3;
    CPPUNIT_ASSERT_EQUAL(string("[-7 -7][-7 -7][-7 -7]"), out.str());
    out.str("");
  }

private:
  PositionalScore make(const std::string& s) const
  {
    return PositionalScoreFormatter::scoreFromString(s);
  }

  void checkPropertiesOfInitializedScore(const PositionalScore& s,
                                         unsigned int words,
                                         unsigned int wWidth,
                                         int value)
  {
    CPPUNIT_ASSERT_EQUAL(words, s.getWordsNbr());
    CPPUNIT_ASSERT_EQUAL(wWidth, s.getWordWidth());

    // Check if all words filled correctly
    vector<int> tmp(wWidth, value);
    for (unsigned int i = 0; i < words; ++i)
    {
      CPPUNIT_ASSERT(tmp == s.getWord(i));
    }

    // Check if all values OK
    for (unsigned int i = 0; i < words; ++i)
    {
      for (unsigned int j = 0; j < wWidth; ++j)
      {
        CPPUNIT_ASSERT_EQUAL(value, s.getValue(i, j));
      }
    }
  }

  void checkIfScoresAreIdentical(const PositionalScore& s1, const PositionalScore& s2)
  {
    CPPUNIT_ASSERT_EQUAL(s1, s2);
    CPPUNIT_ASSERT(s1 == s2);
    CPPUNIT_ASSERT(!(s1 != s2));
    CPPUNIT_ASSERT(s1.equalSizes(s2));
    CPPUNIT_ASSERT(s2.equalSizes(s1));
    CPPUNIT_ASSERT(!(s1 < s2));
    CPPUNIT_ASSERT(!(s2 < s1));
    CPPUNIT_ASSERT_EQUAL(s1.getWordsNbr(), s2.getWordsNbr());
    CPPUNIT_ASSERT_EQUAL(s1.getWordWidth(), s2.getWordWidth());

    for (unsigned int i = 0; i < s1.getWordsNbr(); ++i)
    {
      CPPUNIT_ASSERT(s1.getWord(i) == s2.getWord(i));
    }

    // Check if all values OK
    for (unsigned int i = 0; i < s1.getWordsNbr(); ++i)
    {
      for (unsigned int j = 0; j < s1.getWordWidth(); ++j)
      {
        CPPUNIT_ASSERT_EQUAL(s1.getValue(i, j), s2.getValue(i, j));
      }
    }

    // Compare stream output
    ostringstream o1;
    ostringstream o2;

    o1 << s1;
    o2 << s2;
    CPPUNIT_ASSERT_EQUAL(o1.str(), o2.str());
  }

  void checkIfCopyingIsCorrect(const PositionalScore& s)
  {
    // Copy constructor
    PositionalScore s1(s);
    checkIfScoresAreIdentical(s, s1);
    checkIfScoresAreIdentical(s1, s);

    // Assignment operator
    PositionalScore s2(5, 17, 91);
    s2 = s1;
    checkIfScoresAreIdentical(s, s1);
    checkIfScoresAreIdentical(s, s2);
    checkIfScoresAreIdentical(s1, s2);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PositionalScoreTest);

} // namespace tse
