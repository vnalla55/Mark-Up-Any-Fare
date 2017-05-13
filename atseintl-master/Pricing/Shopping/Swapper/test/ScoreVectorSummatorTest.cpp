
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

#include "Pricing/Shopping/Swapper/ScoreVectorSummator.h"

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

class ScoreVectorSummatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ScoreVectorSummatorTest);
  CPPUNIT_TEST(testEmptySummator);
  CPPUNIT_TEST(emptySummatorErrors);
  CPPUNIT_TEST(simpleUsageTest);
  CPPUNIT_TEST(resizeTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _t = _memHandle.create<ScoreVectorSummator>(WORDS_COUNT); }

  void tearDown() { _memHandle.clear(); }

  void testEmptySummator()
  {
    CPPUNIT_ASSERT_EQUAL(WORDS_COUNT, _t->getWordsCount());
    CPPUNIT_ASSERT_EQUAL(0u, _t->getOffsetsCount());
  }

  void emptySummatorErrors()
  {
    // Zero width
    CPPUNIT_ASSERT_THROW(ScoreVectorSummator(0), ErrorResponseException);

    // No score if newScore never called
    CPPUNIT_ASSERT_THROW(_t->getScore(), ErrorResponseException);

    // Bad offset
    vector<int> sizetwo;
    sizetwo.push_back(123);
    sizetwo.push_back(123);
    CPPUNIT_ASSERT_THROW(_t->addScoreVector(0, sizetwo), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(_t->addScoreVector(1, sizetwo), ErrorResponseException);

    _t->newScore(1);

    // Bad vector size
    vector<int> sizeone;
    sizeone.push_back(123);
    CPPUNIT_ASSERT_THROW(_t->addScoreVector(0, sizeone), ErrorResponseException);

    // Bad offset
    CPPUNIT_ASSERT_THROW(_t->addScoreVector(5, sizetwo), ErrorResponseException);
  }

  void simpleUsageTest()
  {
    CPPUNIT_ASSERT_EQUAL(WORDS_COUNT, _t->getWordsCount());

    _t->newScore(OFFSETS_COUNT);
    CPPUNIT_ASSERT_EQUAL(OFFSETS_COUNT, _t->getOffsetsCount());
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0][0 0][0 0]"), _t->getScore());

    _t->addScoreVector(0, makeVector(0, 2, 6));
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0][2 0][6 0]"), _t->getScore());

    _t->addScoreVector(1, makeVector(2, 67, -4));
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 2][2 67][6 -4]"), _t->getScore());
    CPPUNIT_ASSERT_EQUAL(WORDS_COUNT, _t->getWordsCount());

    _t->addScoreVector(0, makeVector(-1, 0, 10));
    CPPUNIT_ASSERT_EQUAL(makeScore("[-1 2][2 67][16 -4]"), _t->getScore());
    CPPUNIT_ASSERT_EQUAL(makeScore("[-1 2][2 67][16 -4]"), _t->getScore());

    _t->newScore(OFFSETS_COUNT);
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0][0 0][0 0]"), _t->getScore());
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0][0 0][0 0]"), _t->getScore());
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0][0 0][0 0]"), _t->getScore());

    CPPUNIT_ASSERT_EQUAL(OFFSETS_COUNT, _t->getOffsetsCount());
    CPPUNIT_ASSERT_EQUAL(WORDS_COUNT, _t->getWordsCount());
  }

  void resizeTest()
  {
    CPPUNIT_ASSERT_EQUAL(WORDS_COUNT, _t->getWordsCount());
    // Three offsets
    _t->newScore(3);
    CPPUNIT_ASSERT_EQUAL(3u, _t->getOffsetsCount());
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0 0][0 0 0][0 0 0]"), _t->getScore());

    _t->addScoreVector(0, makeVector(1, 2, 4));
    _t->addScoreVector(2, makeVector(1, 2, 4));
    _t->addScoreVector(1, makeVector(1, 2, 4));
    CPPUNIT_ASSERT_EQUAL(3u, _t->getOffsetsCount());
    _t->addScoreVector(2, makeVector(1, 2, 4));
    CPPUNIT_ASSERT_EQUAL(makeScore("[1 1 2][2 2 4][4 4 8]"), _t->getScore());

    _t->newScore(1);
    CPPUNIT_ASSERT_EQUAL(makeScore("[0][0][0]"), _t->getScore());
    _t->addScoreVector(0, makeVector(1, 2, 4));
    CPPUNIT_ASSERT_EQUAL(makeScore("[1][2][4]"), _t->getScore());
    _t->addScoreVector(0, makeVector(3, 5, 1));
    CPPUNIT_ASSERT_EQUAL(1u, _t->getOffsetsCount());
    CPPUNIT_ASSERT_EQUAL(makeScore("[4][7][5]"), _t->getScore());

    _t->newScore(2);
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0][0 0][0 0]"), _t->getScore());
    CPPUNIT_ASSERT_EQUAL(2u, _t->getOffsetsCount());
  }

private:
  PositionalScore makeScore(const std::string& s) const
  {
    return PositionalScoreFormatter::scoreFromString(s);
  }

  static vector<int> makeVector(int a, int b, int c)
  {
    vector<int> tmp(3);
    tmp[0] = a;
    tmp[1] = b;
    tmp[2] = c;
    return tmp;
  }

  TestMemHandle _memHandle;
  ScoreVectorSummator* _t;
  static const unsigned int OFFSETS_COUNT;
  static const unsigned int WORDS_COUNT;
};

const unsigned int ScoreVectorSummatorTest::OFFSETS_COUNT = 2;
const unsigned int ScoreVectorSummatorTest::WORDS_COUNT = 3;

CPPUNIT_TEST_SUITE_REGISTRATION(ScoreVectorSummatorTest);

} // namespace tse
