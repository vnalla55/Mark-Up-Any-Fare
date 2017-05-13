//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/include/GtestHelperMacros.h"
#include "Pricing/Shopping/IBF/AllDirectOptionsRepresented.h"
#include "Pricing/Shopping/IBF/test/SwapperTestUtils.h"

using namespace ::testing;
using namespace boost;

namespace tse
{

namespace utils {

class MockSopCollection {
 public:
  MockSopCollection(unsigned int legCount) {}

  MOCK_METHOD2(addSop,
      void(unsigned int legId, unsigned int sopId));
  MOCK_CONST_METHOD2(containsSop,
      bool(unsigned int legId, unsigned int sopId));
  MOCK_CONST_METHOD0(getCartesianCardinality,
      unsigned int());
};

}  // namespace utils


typedef AllDirectOptionsRepresented<utils::MockSopCollection> TestedAllDirectOptionsRepresented;

class AllDirectOptionsRepresentedTest:
    public IbfAppraiserTest<TestedAllDirectOptionsRepresented>
{
public:
  void childSetup()
  {
    _sopCollection = new utils::MockSopCollection(2);
  }

  TestedAllDirectOptionsRepresented* newAppraiser()
  {
    // Setting a dummy number of legs. Currently it is only used when
    // constructing a new SopCollection in production use.
    return new TestedAllDirectOptionsRepresented(2, _sopCollection);
  }

  void TearDown(){}


  void expectSopDirectnessCheck(unsigned int leg, unsigned int sop, bool mockAnswer)
  {
    EXPECT_CALL(*_sopCollection, containsSop(leg, sop))
        .WillOnce(Return(mockAnswer));
  }

  void addSopCombinationWithDirectness(unsigned int sop1, unsigned int sop2, bool direct)
  {
    // First is direct: make the appraiser check for both sops
    expectSopDirectnessCheck(0, sop1, true);
    expectSopDirectnessCheck(1, sop2, direct);

    swp::BasicAppraiserScore::CATEGORY expectedCategory;
    if (direct)
    {
      expectedCategory = swp::BasicAppraiserScore::MUST_HAVE;
    }
    else
    {
      expectedCategory = swp::BasicAppraiserScore::IGNORE;
    }
    rateCombination(combination(sop1, sop2), score(expectedCategory));
  }

  void removeSopCombinationWithDirectness(unsigned int sop1, unsigned int sop2, bool direct)
  {
    // First is direct: make the appraiser check for both sops
    expectSopDirectnessCheck(0, sop1, true);
    expectSopDirectnessCheck(1, sop2, direct);
    removeCombination(combination(sop1, sop2));
  }

  static const unsigned int DUMMY_SOP_ID_1;
  static const unsigned int DUMMY_SOP_ID_2;
  static const unsigned int DUMMY_SOP_ID_3;
  static const unsigned int DUMMY_SOP_ID_4;
  static const unsigned int DUMMY_COMBINATIONS;


  utils::MockSopCollection* _sopCollection;
};

const unsigned int AllDirectOptionsRepresentedTest::DUMMY_SOP_ID_1 = 91;
const unsigned int AllDirectOptionsRepresentedTest::DUMMY_SOP_ID_2 = 13;
const unsigned int AllDirectOptionsRepresentedTest::DUMMY_SOP_ID_3 = 0;
const unsigned int AllDirectOptionsRepresentedTest::DUMMY_SOP_ID_4 = 99;
const unsigned int AllDirectOptionsRepresentedTest::DUMMY_COMBINATIONS = 10;


TEST_F(AllDirectOptionsRepresentedTest, addsSopForTracking)
{
  EXPECT_CALL(*_sopCollection, addSop(0, DUMMY_SOP_ID_1))
      .Times(1);
  _ibfAppraiser->addDirectSopForTracking(0, DUMMY_SOP_ID_1);
}

TEST_F(AllDirectOptionsRepresentedTest, takesSopCombinationsCountFromCollection)
{
  EXPECT_CALL(*_sopCollection, getCartesianCardinality())
      .WillOnce(Return(DUMMY_COMBINATIONS));
  ASSERT_EQ(DUMMY_COMBINATIONS, _ibfAppraiser->getAllSopCombinationsCount());
}

TEST_F(AllDirectOptionsRepresentedTest, ignoresPureNondirect)
{
  _ibfAppraiser->setTargetCount(1);
  expectSopDirectnessCheck(0, DUMMY_SOP_ID_1, false);
  rateCombination(combination(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2),
      score(swp::BasicAppraiserScore::IGNORE));
  ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
}

TEST_F(AllDirectOptionsRepresentedTest, ignoresPartlyNondirect)
{
  _ibfAppraiser->setTargetCount(1);
  expectSopDirectnessCheck(0, DUMMY_SOP_ID_1, true);
  expectSopDirectnessCheck(1, DUMMY_SOP_ID_2, false);
  rateCombination(combination(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2),
        score(swp::BasicAppraiserScore::IGNORE));
  ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
}

TEST_F(AllDirectOptionsRepresentedTest, acceptsDirect)
{
  _ibfAppraiser->setTargetCount(1);
  addSopCombinationWithDirectness(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2, true);
  ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
}

TEST_F(AllDirectOptionsRepresentedTest, isSatisfiedAlsoIfTooMuch)
{
  _ibfAppraiser->setTargetCount(1);
  addSopCombinationWithDirectness(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2, true);
  addSopCombinationWithDirectness(DUMMY_SOP_ID_3, DUMMY_SOP_ID_4, true);
  ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
}

TEST_F(AllDirectOptionsRepresentedTest, stillSatisfiedIfNonDirectRemoved)
{
  _ibfAppraiser->setTargetCount(1);
  addSopCombinationWithDirectness(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2, true);
  addSopCombinationWithDirectness(DUMMY_SOP_ID_3, DUMMY_SOP_ID_4, false);
  removeSopCombinationWithDirectness(DUMMY_SOP_ID_3, DUMMY_SOP_ID_4, false);
  ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
}

TEST_F(AllDirectOptionsRepresentedTest, noLongerSatisfiedIfDirectRemoved)
{
  _ibfAppraiser->setTargetCount(1);
  addSopCombinationWithDirectness(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2, true);
  addSopCombinationWithDirectness(DUMMY_SOP_ID_3, DUMMY_SOP_ID_4, false);
  removeSopCombinationWithDirectness(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2, true);
  ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
}

TEST_F(AllDirectOptionsRepresentedTest, producesCorrectStringRepresentation)
{
  _ibfAppraiser->setTargetCount(2);
  addSopCombinationWithDirectness(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2, true);
  ASSERT_EQ("Direct options preferred (1/2)", _ibfAppraiser->toString());
}


} // namespace tse
