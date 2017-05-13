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
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "Common/ErrorResponseException.h"

#include "Pricing/Shopping/Swapper/PriorityScoreBuilder.h"

#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

namespace tse
{

using namespace swp;

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class PriorityScoreBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PriorityScoreBuilderTest);
  CPPUNIT_TEST(testExceptionsForEmptyBuilder);
  CPPUNIT_TEST(testExceptionsForUnknownPriority);
  CPPUNIT_TEST(checkRanks);
  CPPUNIT_TEST(checkDefaultItemScore);
  CPPUNIT_TEST(scoreTest1);
  CPPUNIT_TEST(scoreTest2);
  CPPUNIT_TEST(noNewItemAtStartRound);
  CPPUNIT_TEST(multipleScoresSamePriority);
  CPPUNIT_TEST(scoreTest3);
  CPPUNIT_TEST(newAppraiserSamePriority);
  CPPUNIT_TEST(newAppraiserSamePriorityScoreTest);
  CPPUNIT_TEST_SUITE_END();

public:
  // multiple scores on one priority

  PriorityScoreBuilderTest() : _veryBad(-1), _stillBad(0), _nice(15), _genius(1000) {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _b = _memHandle.create<PriorityScoreBuilder>();
    addAppraisersToBuilder();
  }

  void tearDown() { _memHandle.clear(); }

  void testExceptionsForEmptyBuilder()
  {
    PriorityScoreBuilder b;
    // No appraisers, no item
    CPPUNIT_ASSERT_THROW(b.newItem(), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(b.getItemScore(), ErrorResponseException);

    // Unknown priorities
    CPPUNIT_ASSERT_THROW(b.getDerivedInfo(10), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(b.addScoreFromAppraiser(10, BasicAppraiserScore()),
                         ErrorResponseException);
  }

  void testExceptionsForUnknownPriority()
  {
    // Unknown priorities
    CPPUNIT_ASSERT_THROW(_b->getDerivedInfo(558), ErrorResponseException);
    CPPUNIT_ASSERT_THROW(_b->addScoreFromAppraiser(558, BasicAppraiserScore()),
                         ErrorResponseException);
  }

  void checkRanks()
  {
    CPPUNIT_ASSERT_EQUAL(2u, _b->getDerivedInfo(_stillBad));
    CPPUNIT_ASSERT_EQUAL(3u, _b->getDerivedInfo(_veryBad));
    CPPUNIT_ASSERT_EQUAL(0u, _b->getDerivedInfo(_genius));
    CPPUNIT_ASSERT_EQUAL(1u, _b->getDerivedInfo(_nice));
  }

  void checkDefaultItemScore()
  {
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0 0 0][0 0 0 0][0 0 0 0]"), _b->getItemScore());
    _b->newItem();
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0 0 0][0 0 0 0][0 0 0 0]"), _b->getItemScore());
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0 0 0][0 0 0 0][0 0 0 0]"), _b->getItemScore());
  }

  void scoreTest1() { defaultAppraisingRound(); }

  void scoreTest2() { nontrivialAppraisingRound(); }

  void noNewItemAtStartRound()
  {
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, 75));
    _b->addScoreFromAppraiser(_nice, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE, -10));
    _b->addScoreFromAppraiser(_stillBad,
                              BasicAppraiserScore(BasicAppraiserScore::WANT_TO_REMOVE, 16));
    _b->addScoreFromAppraiser(_veryBad, BasicAppraiserScore(BasicAppraiserScore::IGNORE, 73));
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 1 -1 0][1 0 0 0][75 -10 16 0]"), _b->getItemScore());
  }

  void multipleScoresSamePriority()
  {
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, 75));
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, 75));
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE, 100));
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE, 100));
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE, 100));
    _b->addScoreFromAppraiser(_nice, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE, -10));
    _b->addScoreFromAppraiser(_nice, BasicAppraiserScore(BasicAppraiserScore::WANT_TO_REMOVE, -10));
    _b->addScoreFromAppraiser(_stillBad,
                              BasicAppraiserScore(BasicAppraiserScore::WANT_TO_REMOVE, 16));
    _b->addScoreFromAppraiser(_veryBad, BasicAppraiserScore(BasicAppraiserScore::IGNORE, 73));
    _b->addScoreFromAppraiser(_stillBad,
                              BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, 55));
    CPPUNIT_ASSERT_EQUAL(makeScore("[3 0 -1 0][2 0 1 0][450 -20 71 0]"), _b->getItemScore());
  }

  void scoreTest3()
  {
    nontrivialAppraisingRound();
    defaultAppraisingRound();
    defaultAppraisingRound();
    nontrivialAppraisingRound();
  }

  void newAppraiserSamePriority()
  {
    _b->addAppraiser(_nice);

    CPPUNIT_ASSERT_EQUAL(3u, _b->getDerivedInfo(_veryBad));
    CPPUNIT_ASSERT_EQUAL(2u, _b->getDerivedInfo(_stillBad));
    CPPUNIT_ASSERT_EQUAL(1u, _b->getDerivedInfo(_nice));
    CPPUNIT_ASSERT_EQUAL(0u, _b->getDerivedInfo(_genius));
  }

  void newAppraiserSamePriorityScoreTest()
  {
    _b->addAppraiser(_nice);

    _b->newItem();
    _b->addScoreFromAppraiser(_stillBad, BasicAppraiserScore(BasicAppraiserScore::IGNORE, 123));
    _b->addScoreFromAppraiser(_nice, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE, 100));
    _b->addScoreFromAppraiser(_veryBad, BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, 93));
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE, 12));
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, 13));
    _b->addScoreFromAppraiser(_nice, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE, 100));

    CPPUNIT_ASSERT_EQUAL(makeScore("[1 2 0 0][1 0 0 1][25 200 0 93]"), _b->getItemScore());
  }

private:
  TestMemHandle _memHandle;
  PriorityScoreBuilder* _b;

  int _veryBad;
  int _stillBad;
  int _nice;
  int _genius;

  void addAppraisersToBuilder()
  {
    _b->addAppraiser(_veryBad);
    _b->addAppraiser(_nice);
    _b->addAppraiser(_stillBad);
    _b->addAppraiser(_genius);
  }

  PositionalScore makeScore(const std::string& s) const
  {
    return PositionalScoreFormatter::scoreFromString(s);
  }

  void defaultAppraisingRound()
  {
    _b->newItem();
    _b->addScoreFromAppraiser(_veryBad, BasicAppraiserScore());
    _b->addScoreFromAppraiser(_stillBad, BasicAppraiserScore());
    _b->addScoreFromAppraiser(_nice, BasicAppraiserScore());
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore());
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 0 0 0][0 0 0 0][0 0 0 0]"), _b->getItemScore());
  }

  void nontrivialAppraisingRound()
  {
    _b->newItem();
    _b->addScoreFromAppraiser(_genius, BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, 75));
    _b->addScoreFromAppraiser(_nice, BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE, -10));
    _b->addScoreFromAppraiser(_stillBad,
                              BasicAppraiserScore(BasicAppraiserScore::WANT_TO_REMOVE, 16));
    _b->addScoreFromAppraiser(_veryBad, BasicAppraiserScore(BasicAppraiserScore::IGNORE, 73));
    CPPUNIT_ASSERT_EQUAL(makeScore("[0 1 -1 0][1 0 0 0][75 -10 16 0]"), _b->getItemScore());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PriorityScoreBuilderTest);

} // namespace tse
