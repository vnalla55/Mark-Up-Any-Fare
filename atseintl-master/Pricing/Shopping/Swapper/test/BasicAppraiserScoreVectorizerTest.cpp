
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

#include "Pricing/Shopping/Swapper/BasicAppraiserScoreVectorizer.h"

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

namespace tse
{

using namespace swp;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class BasicAppraiserScoreVectorizerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BasicAppraiserScoreVectorizerTest);
  CPPUNIT_TEST(wordNumberTest);
  CPPUNIT_TEST(emptyVectorTest);
  CPPUNIT_TEST(noRankTest);
  CPPUNIT_TEST(rankTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void wordNumberTest()
  {
    BasicAppraiserScoreVectorizer t;
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), t.createEmptyVector().size());
  }

  void emptyVectorTest()
  {
    BasicAppraiserScoreVectorizer t;
    CPPUNIT_ASSERT(equal(t.createEmptyVector(), makeTriple(0, 0, 0)));
  }

  void noRankTest()
  {
    BasicAppraiserScoreVectorizer t;

    CPPUNIT_ASSERT(equal(t.vectorize(BasicAppraiserScore()), makeTriple(0, 0, 0)));

    CPPUNIT_ASSERT(equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::IGNORE)),
                         makeTriple(0, 0, 0)));

    CPPUNIT_ASSERT(
        equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::NICE_TO_HAVE)),
              makeTriple(0, 1, 0)));

    CPPUNIT_ASSERT(equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::MUST_HAVE)),
                         makeTriple(1, 0, 0)));

    CPPUNIT_ASSERT(
        equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::WANT_TO_REMOVE)),
              makeTriple(-1, 0, 0)));
  }

  void rankTest()
  {
    BasicAppraiserScoreVectorizer t;

    CPPUNIT_ASSERT(
        equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::IGNORE, 53)),
              makeTriple(0, 0, 0)));

    CPPUNIT_ASSERT(
        equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::NICE_TO_HAVE, -96)),
              makeTriple(0, 1, -96)));

    CPPUNIT_ASSERT(equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::MUST_HAVE)),
                         makeTriple(1, 0, 0)));

    CPPUNIT_ASSERT(
        equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::MUST_HAVE, 0)),
              makeTriple(1, 0, 0)));

    CPPUNIT_ASSERT(
        equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::MUST_HAVE, 1234)),
              makeTriple(1, 0, 1234)));

    CPPUNIT_ASSERT(
        equal(t.vectorize(BasicAppraiserScore(tse::swp::BasicAppraiserScore::WANT_TO_REMOVE, 8)),
              makeTriple(-1, 0, 8)));
  }

private:
  static BasicAppraiserScoreVectorizer::ScoreVector makeTriple(int a, int b, int c)
  {
    BasicAppraiserScoreVectorizer::ScoreVector tmp(3);
    tmp[0] = a;
    tmp[1] = b;
    tmp[2] = c;
    return tmp;
  }

  static bool equal(const BasicAppraiserScoreVectorizer::ScoreVector& v1,
                    const BasicAppraiserScoreVectorizer::ScoreVector& v2)
  {
    CPPUNIT_ASSERT_EQUAL(v1.size(), v2.size());
    for (unsigned int i = 0; i < v1.size(); ++i)
    {
      if (v1[i] != v2[i])
      {
        return false;
      }
    }
    return true;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BasicAppraiserScoreVectorizerTest);

} // namespace tse
