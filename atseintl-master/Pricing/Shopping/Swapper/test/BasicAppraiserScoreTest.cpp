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

#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Utils/StreamFormat.h"

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

const int DUMMY_RANK = 123;

class BasicAppraiserScoreTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BasicAppraiserScoreTest);
  CPPUNIT_TEST(emptyScoreTest);
  CPPUNIT_TEST(emptyScoreOutputTest);
  CPPUNIT_TEST(constructorTest);
  CPPUNIT_TEST(setterTest);
  CPPUNIT_TEST(enumSerializationTest);
  CPPUNIT_TEST(outputTest);
  CPPUNIT_TEST(copyConstructorTest);
  CPPUNIT_TEST(assignmentOperatorTest);
  CPPUNIT_TEST(operatorEqualTest);
  CPPUNIT_TEST(operatorNonEqualTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void emptyScoreTest()
  {
    BasicAppraiserScore s;
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::IGNORE, s.getCategory());
    CPPUNIT_ASSERT_EQUAL(0, s.getMinorRank());
  }

  void emptyScoreOutputTest()
  {
    BasicAppraiserScore s;
    ostringstream out;
    out << s;
    string expected = ".";
    CPPUNIT_ASSERT_EQUAL(expected, out.str());
  }

  void constructorTest()
  {
    BasicAppraiserScore s(BasicAppraiserScore::NICE_TO_HAVE);
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::NICE_TO_HAVE, s.getCategory());
    CPPUNIT_ASSERT_EQUAL(0, s.getMinorRank());

    BasicAppraiserScore s1(BasicAppraiserScore::MUST_HAVE);
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::MUST_HAVE, s1.getCategory());
    CPPUNIT_ASSERT_EQUAL(0, s1.getMinorRank());

    BasicAppraiserScore s2(BasicAppraiserScore::MUST_HAVE, 555);
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::MUST_HAVE, s2.getCategory());
    CPPUNIT_ASSERT_EQUAL(555, s2.getMinorRank());
  }

  void setterTest()
  {
    BasicAppraiserScore s;
    s.setCategory(BasicAppraiserScore::WANT_TO_REMOVE);
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::WANT_TO_REMOVE, s.getCategory());
    CPPUNIT_ASSERT_EQUAL(0, s.getMinorRank());

    s.setMinorRank(-317);
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::WANT_TO_REMOVE, s.getCategory());
    CPPUNIT_ASSERT_EQUAL(-317, s.getMinorRank());

    s.setCategory(BasicAppraiserScore::WANT_TO_REMOVE);
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::WANT_TO_REMOVE, s.getCategory());
    CPPUNIT_ASSERT_EQUAL(-317, s.getMinorRank());

    s.setCategory(BasicAppraiserScore::MUST_HAVE);
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::MUST_HAVE, s.getCategory());
    CPPUNIT_ASSERT_EQUAL(-317, s.getMinorRank());

    s.setCategory(BasicAppraiserScore::IGNORE);
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::IGNORE, s.getCategory());
    CPPUNIT_ASSERT_EQUAL(-317, s.getMinorRank());

    s.setMinorRank(1897);
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::IGNORE, s.getCategory());
    CPPUNIT_ASSERT_EQUAL(1897, s.getMinorRank());
  }

  void enumSerializationTest()
  {
    streamCompare(BasicAppraiserScore::IGNORE, "IGNORE");
    streamCompare(BasicAppraiserScore::MUST_HAVE, "MUST_HAVE");
    streamCompare(BasicAppraiserScore::NICE_TO_HAVE, "NICE_TO_HAVE");
    streamCompare(BasicAppraiserScore::WANT_TO_REMOVE, "WANT_TO_REMOVE");
    BasicAppraiserScore::CATEGORY bad = (BasicAppraiserScore::CATEGORY)876;
    streamCompare(bad, "#BAD VALUE#");
  }

  void outputTest()
  {
    ostringstream out;

    BasicAppraiserScore s(BasicAppraiserScore::MUST_HAVE);

    out << s;
    CPPUNIT_ASSERT_EQUAL(string("M"), out.str());
    out.str("");

    s = BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, 12);
    out << s;
    CPPUNIT_ASSERT_EQUAL(string("n(12)"), out.str());
    out.str("");

    s.setCategory(BasicAppraiserScore::IGNORE);
    s.setMinorRank(10000);
    out << s;
    CPPUNIT_ASSERT_EQUAL(string(".(10000)"), out.str());
    out.str("");
  }

  void copyConstructorTest()
  {
    BasicAppraiserScore s1(BasicAppraiserScore::MUST_HAVE, DUMMY_RANK);
    BasicAppraiserScore s2 = s1;
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::MUST_HAVE, s2.getCategory());
    CPPUNIT_ASSERT_EQUAL(DUMMY_RANK, s2.getMinorRank());
  }

  void assignmentOperatorTest()
  {
    BasicAppraiserScore s1(BasicAppraiserScore::MUST_HAVE, DUMMY_RANK);
    BasicAppraiserScore s2;
    s2 = s1;
    CPPUNIT_ASSERT_EQUAL(BasicAppraiserScore::MUST_HAVE, s2.getCategory());
    CPPUNIT_ASSERT_EQUAL(DUMMY_RANK, s2.getMinorRank());
  }

  void operatorEqualTest()
  {
    BasicAppraiserScore s1(BasicAppraiserScore::MUST_HAVE, DUMMY_RANK);
    BasicAppraiserScore s2(BasicAppraiserScore::MUST_HAVE, DUMMY_RANK);
    CPPUNIT_ASSERT(s1 == s2);
    BasicAppraiserScore s3(BasicAppraiserScore::WANT_TO_REMOVE, DUMMY_RANK);
    CPPUNIT_ASSERT(!(s1 == s3));
  }

  void operatorNonEqualTest()
  {
    BasicAppraiserScore s1(BasicAppraiserScore::MUST_HAVE, DUMMY_RANK);
    BasicAppraiserScore s2(BasicAppraiserScore::NICE_TO_HAVE, DUMMY_RANK);
    CPPUNIT_ASSERT(s1 != s2);
    BasicAppraiserScore s3(BasicAppraiserScore::MUST_HAVE, DUMMY_RANK + 1);
    CPPUNIT_ASSERT(s1 != s3);
    CPPUNIT_ASSERT(s2 != s3);
    BasicAppraiserScore s4(BasicAppraiserScore::MUST_HAVE, DUMMY_RANK);
    CPPUNIT_ASSERT(!(s1 != s4));
  }

private:
  template <typename T>
  void streamCompare(const T& x, const std::string& expected)
  {
    std::ostringstream out;
    out << x;
    CPPUNIT_ASSERT_EQUAL(expected, out.str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BasicAppraiserScoreTest);

} // namespace tse
