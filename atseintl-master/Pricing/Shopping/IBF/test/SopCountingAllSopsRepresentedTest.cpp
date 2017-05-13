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
#include "Pricing/Shopping/IBF/SopCountingAllSopsRepresented.h"
#include "Pricing/Shopping/IBF/test/SwapperTestUtils.h"

#include <memory>
#include <sstream>

using namespace ::testing;
using namespace boost;

namespace tse
{

class SopCountingAllSopsRepresentedTest:
    public IbfAppraiserTest<SopCountingAllSopsRepresented>
{
public:
  void childSetup() override
  {
    _sopUsageTracker.reset(new MockISopUsageTracker());
  }

  SopCountingAllSopsRepresented* newAppraiser()
  {
    return new SopCountingAllSopsRepresented(*_sopUsageTracker);
  }

  void TearDown(){}

  // We know that each time a combination is added, the
  // testing appraiser will add the combination to its tracker.
  void rateCombinationAndExpect(
      const utils::SopCombination& comb,
      const swp::BasicAppraiserScore& expected_score)
  {
    EXPECT_CALL(*_sopUsageTracker, combinationAdded(comb))
          .Times(1);
    rateCombination(comb, expected_score);
  }

  void removeCombinationAndExpect(const utils::SopCombination& comb)
  {
    EXPECT_CALL(*_sopUsageTracker, combinationRemoved(comb))
              .Times(1);
    removeCombination(comb);
  }

  std::shared_ptr<MockISopUsageTracker> _sopUsageTracker;

  static const unsigned int DUMMY_ALL_SOPS_COUNT;
  static const unsigned int DUMMY_UNUSED_SOPS_COUNT;
  static const unsigned int DUMMY_SOP_ID_1;
  static const unsigned int DUMMY_SOP_ID_2;
  static const unsigned int DUMMY_SOP_ID_3;
  static const unsigned int DUMMY_SOP_ID_4;

  // (1 2)
  void t_mustHaveOptionWithTwoNewSops()
  {
    rateCombinationAndExpect(combination(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2),
                score(swp::BasicAppraiserScore::MUST_HAVE, 2));
  }

  // 1 2
  //  (3)
  void t_mustHaveOptionWithExtraOneSop()
  {
    t_mustHaveOptionWithTwoNewSops();
    // Now (1, 2) becomes M(1) since 1 is no longer
    // unique sop.
    EXPECT_CALL(*_blackboard, updateValue(
        combination(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2),
        score(swp::BasicAppraiserScore::MUST_HAVE, 1)))
          .Times(1);
    rateCombinationAndExpect(combination(DUMMY_SOP_ID_1, DUMMY_SOP_ID_3),
          score(swp::BasicAppraiserScore::MUST_HAVE, 1));
  }

  // 1 2
  //(4)3
  void t_mustHaveSecondOptionWithExtraOneSop()
  {
    t_mustHaveOptionWithExtraOneSop();
    // Now (1, 3) becomes ignored since it
    // does not hold any unique SOP.
    EXPECT_CALL(*_blackboard, updateValue(
            combination(DUMMY_SOP_ID_1, DUMMY_SOP_ID_3),
            score(swp::BasicAppraiserScore::IGNORE)))
              .Times(1);
    rateCombinationAndExpect(combination(DUMMY_SOP_ID_4, DUMMY_SOP_ID_3),
          score(swp::BasicAppraiserScore::MUST_HAVE, 1));
  }

  // 1 2
  // 4 3
  void t_ignoresOptionWithNoNewSop()
  {
    // All options are ignored now: none of them
    // contains unique sop.
    EXPECT_CALL(*_blackboard, updateValue(
                combination(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2),
                score(swp::BasicAppraiserScore::IGNORE)))
                  .Times(1);
    EXPECT_CALL(*_blackboard, updateValue(
                    combination(DUMMY_SOP_ID_4, DUMMY_SOP_ID_3),
                    score(swp::BasicAppraiserScore::IGNORE)))
                      .Times(1);
    t_mustHaveSecondOptionWithExtraOneSop();
    rateCombinationAndExpect(combination(DUMMY_SOP_ID_4, DUMMY_SOP_ID_2),
              score(swp::BasicAppraiserScore::IGNORE));
  }

  //(1 2)
  // 4 3
  void t_optionsStartToMatterWhenRedundancyRemoved()
  {
    t_ignoresOptionWithNoNewSop();
    EXPECT_CALL(*_blackboard, updateValue(
                    combination(DUMMY_SOP_ID_1, DUMMY_SOP_ID_3),
                    score(swp::BasicAppraiserScore::MUST_HAVE, 1)))
                      .Times(1);
    EXPECT_CALL(*_blackboard, updateValue(
                    combination(DUMMY_SOP_ID_4, DUMMY_SOP_ID_2),
                    score(swp::BasicAppraiserScore::MUST_HAVE, 1)))
                      .Times(1);
    removeCombinationAndExpect(combination(DUMMY_SOP_ID_1, DUMMY_SOP_ID_2));
  }
};

const unsigned int SopCountingAllSopsRepresentedTest::DUMMY_ALL_SOPS_COUNT = 57;
const unsigned int SopCountingAllSopsRepresentedTest::DUMMY_UNUSED_SOPS_COUNT = 38;
const unsigned int SopCountingAllSopsRepresentedTest::DUMMY_SOP_ID_1 = 91;
const unsigned int SopCountingAllSopsRepresentedTest::DUMMY_SOP_ID_2 = 13;
const unsigned int SopCountingAllSopsRepresentedTest::DUMMY_SOP_ID_3 = 7;
const unsigned int SopCountingAllSopsRepresentedTest::DUMMY_SOP_ID_4 = 0;

TEST_F(SopCountingAllSopsRepresentedTest, isSatisfiedIfNoUnusedSops)
{
  EXPECT_CALL(*_sopUsageTracker, getNbrOfUnusedSops())
      .WillOnce(Return(0));
  ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
}

TEST_F(SopCountingAllSopsRepresentedTest, isUnsatisfiedIfUnusedSopsPresent)
{
  EXPECT_CALL(*_sopUsageTracker, getNbrOfUnusedSops())
      .WillOnce(Return(DUMMY_UNUSED_SOPS_COUNT));
  ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
}

TEST_F(SopCountingAllSopsRepresentedTest, mustHaveOptionWithTwoNewSops)
{
  t_mustHaveOptionWithTwoNewSops();
}

TEST_F(SopCountingAllSopsRepresentedTest, mustHaveOptionWithExtraOneSop)
{
  t_mustHaveOptionWithExtraOneSop();
}

TEST_F(SopCountingAllSopsRepresentedTest, mustHaveSecondOptionWithExtraOneSop)
{
  t_mustHaveSecondOptionWithExtraOneSop();
}

TEST_F(SopCountingAllSopsRepresentedTest, ignoresOptionWithNoNewSop)
{
  t_ignoresOptionWithNoNewSop();
}

TEST_F(SopCountingAllSopsRepresentedTest, optionsStartToMatterWhenRedundancyRemoved)
{
  t_optionsStartToMatterWhenRedundancyRemoved();
}

TEST_F(SopCountingAllSopsRepresentedTest, producesCorrectStringRepresentation)
{
  EXPECT_CALL(*_sopUsageTracker, getNbrOfSops())
    .WillRepeatedly(Return(DUMMY_ALL_SOPS_COUNT));
  EXPECT_CALL(*_sopUsageTracker, getNbrOfUnusedSops())
    .WillRepeatedly(Return(DUMMY_UNUSED_SOPS_COUNT));
  std::ostringstream out;
  out << "Sop counting all SOPs represented ("
      << (DUMMY_ALL_SOPS_COUNT - DUMMY_UNUSED_SOPS_COUNT)
      << "/"
      << DUMMY_ALL_SOPS_COUNT
      << " SOPs represented)";
  ASSERT_EQ(out.str(), _ibfAppraiser->toString());
}

} // namespace tse

