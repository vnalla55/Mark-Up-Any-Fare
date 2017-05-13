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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"

#include "Pricing/Shopping/IBF/test/SwapperTestUtils.h"

#include "Pricing/Shopping/IBF/ScheduleRepeatLimit.h"
#include "Pricing/Shopping/IBF/test/DummyBlackboard.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"
#include "Pricing/Shopping/Utils/StreamLogger.h"

#include <boost/assign/std/vector.hpp>

#include <memory>

using namespace boost::assign;
using namespace ::testing;
using namespace tse::utils;
using namespace tse::swp;

namespace tse
{

const unsigned int DEFAULT_LIMIT = 2;
const unsigned int DEFAULT_Q0S = 400;

class ScheduleRepeatLimitTest: public IbfAppraiserTest<ScheduleRepeatLimit>
{
public:
  typedef DummyBlackboard<SopCombination, swp::BasicAppraiserScore> Blackboard;

  void childSetup()
  {
    _sopUsageCounter.reset(new MockISopUsageCounter());
    _swapperInfo.reset(new MockISwapperInfo());
  }

  ScheduleRepeatLimit* newAppraiser()
  {
    return new ScheduleRepeatLimit(DEFAULT_LIMIT, DEFAULT_Q0S,
        *_sopUsageCounter, *_swapperInfo);
  }

  void TearDown()
  {
    _blackboard.clearLog();
  }

  std::string newTwoLegOption(unsigned int firstSopId, unsigned int secondSopId)
  {
    SopCombination c;
    c += firstSopId, secondSopId;
    const BasicAppraiserScore ret = _ibfAppraiser->beforeItemAdded(c, _blackboard);
    std::ostringstream out;
    out << ret;
    return out.str();
  }

  void removeTwoLegOption(unsigned int firstSopId, unsigned int secondSopId)
  {
    SopCombination c;
    c += firstSopId, secondSopId;
    _ibfAppraiser->beforeItemRemoved(c, _blackboard);
  }


  Blackboard _blackboard;
  std::shared_ptr<MockISopUsageCounter> _sopUsageCounter;
  std::shared_ptr<MockISwapperInfo> _swapperInfo;

  void addSingleOptionNoLimitHit()
  {
    ASSERT_EQ(std::string("."), newTwoLegOption(0, 0));
    ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string(""), _blackboard.getLog());
  }

  void addTwoOptionsNoLimitHit()
  {
    addSingleOptionNoLimitHit();
    ASSERT_EQ(std::string("."), newTwoLegOption(0, 1));
    ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string(""), _blackboard.getLog());
  }

  void addThreeOptionsLimitHit()
  {
    addTwoOptionsNoLimitHit();
    ASSERT_EQ(std::string("R(-1)"), newTwoLegOption(0, 2));
    ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string("{[0, 0]:R(-1)}{[0, 1]:R(-1)}"), _blackboard.getLog());
  }

  void afterHitAddForthOptionNeutral()
  {
    addThreeOptionsLimitHit();
    ASSERT_EQ(std::string("."), newTwoLegOption(7, 7));
    ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string(""), _blackboard.getLog());
  }

  void afterHitRemoveSuperfluousOption()
  {
    addThreeOptionsLimitHit();
    removeTwoLegOption(0, 1);
    ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string("{[0, 0]:.}{[0, 2]:.}"), _blackboard.getLog());
  }

  void hittingOptionAgain()
  {
    afterHitRemoveSuperfluousOption();
    ASSERT_EQ(std::string("R(-1)"), newTwoLegOption(0, 1));
    ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string("{[0, 0]:R(-1)}{[0, 2]:R(-1)}"), _blackboard.getLog());
  }

  void removeSingleOption()
  {
    addSingleOptionNoLimitHit();
    removeTwoLegOption(0, 0);
    ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string(""), _blackboard.getLog());
  }

  void afterHitEvenMoreUsage()
  {
    addThreeOptionsLimitHit();
    ASSERT_EQ(std::string("R(-2)"), newTwoLegOption(0, 3));
    ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string("{[0, 0]:R(-2)}{[0, 1]:R(-2)}{[0, 2]:R(-2)}"),
                         _blackboard.getLog());
  }

  void afterHitEvenMoreUsageEvenMore()
  {
    afterHitEvenMoreUsage();
    ASSERT_EQ(std::string("R(-3)"), newTwoLegOption(0, 6));
    ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string("{[0, 0]:R(-3)}{[0, 1]:R(-3)}{[0, 2]:R(-3)}{[0, 3]:R(-3)}"),
                         _blackboard.getLog());
  }

  void afterHitEvenMoreUsageThenGoBack()
  {
    afterHitEvenMoreUsage();
    removeTwoLegOption(0, 0);
    ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string("{[0, 1]:R(-1)}{[0, 2]:R(-1)}{[0, 3]:R(-1)}"),
                         _blackboard.getLog());
  }

  void fullGoingBack()
  {
    afterHitEvenMoreUsageThenGoBack();
    removeTwoLegOption(0, 1);
    ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string("{[0, 2]:.}{[0, 3]:.}"), _blackboard.getLog());
  }

  void oneOptionBreakingLimitForTwoSops()
  {
    addThreeOptionsLimitHit();
    ASSERT_EQ(std::string("."), newTwoLegOption(1, 2));
    ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string(""), _blackboard.getLog());

    ASSERT_EQ(std::string("R(-1)"), newTwoLegOption(2, 2));
    ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
    ASSERT_EQ(std::string("{[0, 2]:R(-2)}{[1, 2]:R(-1)}"), _blackboard.getLog());
  }
};

TEST_F(ScheduleRepeatLimitTest, testInitialization)
{
  ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
  ASSERT_EQ(DEFAULT_LIMIT, _ibfAppraiser->getLimit());
}


TEST_F(ScheduleRepeatLimitTest, testPrintout)
{
  std::ostringstream out;
  out << "Schedule repeat limit = " << DEFAULT_LIMIT;
  ASSERT_EQ(out.str(), _ibfAppraiser->toString());
}

TEST_F(ScheduleRepeatLimitTest, testZeroLimitError)
{
  ASSERT_THROW(ScheduleRepeatLimit(0, DEFAULT_Q0S,
      *_sopUsageCounter, *_swapperInfo), ErrorResponseException);
}


TEST_F(ScheduleRepeatLimitTest, testAddSingleOptionNoLimitHit)
{
  addSingleOptionNoLimitHit();
}

TEST_F(ScheduleRepeatLimitTest, testAddTwoOptionsNoLimitHit)
{
  addTwoOptionsNoLimitHit();
}

TEST_F(ScheduleRepeatLimitTest, testAddThreeOptionsLimitHit)
{
  addThreeOptionsLimitHit();
}

TEST_F(ScheduleRepeatLimitTest, testAfterHitAddForthOptionNeutral)
{
  afterHitAddForthOptionNeutral();
}

TEST_F(ScheduleRepeatLimitTest, testAfterHitRemoveSuperfluousOption)
{
  afterHitRemoveSuperfluousOption();
}


TEST_F(ScheduleRepeatLimitTest, testHittingOptionAgain)
{
  hittingOptionAgain();
}

TEST_F(ScheduleRepeatLimitTest, testRemoveSingleOption)
{
  removeSingleOption();
}

TEST_F(ScheduleRepeatLimitTest, testAfterHitEvenMoreUsage)
{
  afterHitEvenMoreUsage();
}

TEST_F(ScheduleRepeatLimitTest, testAfterHitEvenMoreUsageEvenMore)
{
  afterHitEvenMoreUsageEvenMore();
}

TEST_F(ScheduleRepeatLimitTest, testAfterHitEvenMoreUsageThenGoBack)
{
  afterHitEvenMoreUsageThenGoBack();
}

TEST_F(ScheduleRepeatLimitTest, testFullGoingBack)
{
  fullGoingBack();
}

TEST_F(ScheduleRepeatLimitTest, testRemoveNonexistentOption)
{
  newTwoLegOption(5, 7);
  ASSERT_THROW(removeTwoLegOption(0, 0), ErrorResponseException);
}

TEST_F(ScheduleRepeatLimitTest, testAddDuplicateOptionError)
{
  newTwoLegOption(5, 7);
  newTwoLegOption(5, 8);
  ASSERT_THROW(newTwoLegOption(5, 7), ErrorResponseException);
}

TEST_F(ScheduleRepeatLimitTest, testThreeLegs)
{
  ScheduleRepeatLimit srl(2, DEFAULT_Q0S,
      *_sopUsageCounter, *_swapperInfo);
  SopCombination c;
  c += 2, 1, 2;
  const BasicAppraiserScore ret = srl.beforeItemAdded(c, _blackboard);
  std::ostringstream out;
  out << ret;
  ASSERT_EQ(std::string("."), out.str());
  ASSERT_EQ(true, srl.isSatisfied());
}

TEST_F(ScheduleRepeatLimitTest, testOneOptionBreakingLimitForTwoSops)
{
  oneOptionBreakingLimitForTwoSops();
}

TEST_F(ScheduleRepeatLimitTest, testOneOptionBreakingLimitForTwoSopsGoBack)
{
  oneOptionBreakingLimitForTwoSops();
  removeTwoLegOption(0, 2);
  ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
  ASSERT_EQ(std::string("{[0, 0]:.}{[0, 1]:.}{[1, 2]:.}{[2, 2]:.}"), _blackboard.getLog());
}

class BasicCovThresholdCalculationPolicyTest: public Test
{
public:

  void SetUp()
  {
    _policy.reset(new BasicCovThresholdCalculationPolicy());
  }

  void TearDown(){}

  std::shared_ptr<BasicCovThresholdCalculationPolicy> _policy;
};

TEST_F(BasicCovThresholdCalculationPolicyTest, testReturnsCorrectThreshold)
{
  ASSERT_TRUE(utils::equal(0.4, (*_policy)(1000)));
  ASSERT_TRUE(utils::equal(0.08, (*_policy)(200)));
}




typedef ScheduleRepeatLimitT<MockCoefficientOfVariation<std::vector<unsigned int> >,
    MockCovThresholdCalculationPolicy> InjectableScheduleRepeatLimit;


class ScheduleRepeatLimitNonFeasibleLegsTest:
    public IbfAppraiserTest<InjectableScheduleRepeatLimit>
{
public:
  void childSetup()
  {
    _sopUsageCounter.reset(new MockISopUsageCounter());
    _swapperInfo.reset(new MockISwapperInfo());
    _covF = new MockCoefficientOfVariation<std::vector<unsigned int> >();
    _thresholdCalc = new MockCovThresholdCalculationPolicy();
    fillDummyUsages();
  }

  InjectableScheduleRepeatLimit* newAppraiser()
  {
    return new InjectableScheduleRepeatLimit(DEFAULT_LIMIT, DEFAULT_Q0S,
        *_sopUsageCounter, *_swapperInfo, 0, _covF, _thresholdCalc);
  }

  void TearDown(){}

  void fillDummyUsages()
  {
    DUMMY_USAGES[DUMMY_SOP_0] = DUMMY_USAGE1;
    DUMMY_USAGES[DUMMY_SOP_1] = DUMMY_USAGE2;
    DUMMY_USAGES[DUMMY_SOP_2] = DUMMY_USAGE3;
    for (ISopUsageCounter::SopUsages::const_iterator it = DUMMY_USAGES.begin();
        it != DUMMY_USAGES.end(); ++it)
    {
      DUMMY_USAGES_VECTOR.push_back(it->second);
    }
  }

  static const unsigned int DUMMY_LEG_0;
  static const unsigned int DUMMY_LEG_1;
  static const unsigned int DUMMY_SOP_0;
  static const unsigned int DUMMY_SOP_1;
  static const unsigned int DUMMY_SOP_2;
  static const unsigned int DUMMY_USAGE1;
  static const unsigned int DUMMY_USAGE2;
  static const unsigned int DUMMY_USAGE3;
  static const unsigned int DUMMY_ITERATIONS_COUNT;
  static const double DUMMY_COV_THRESHOLD;
  static const double DUMMY_COV1;
  static const double DUMMY_COV2;

  std::shared_ptr<MockISopUsageCounter> _sopUsageCounter;
  std::shared_ptr<MockISwapperInfo> _swapperInfo;
  MockCoefficientOfVariation<std::vector<unsigned int> >* _covF;
  MockCovThresholdCalculationPolicy* _thresholdCalc;
  ISopUsageCounter::SopUsages DUMMY_USAGES;
  ISopUsageCounter::SopUsages EMPTY_USAGES;
  std::vector<unsigned int> DUMMY_USAGES_VECTOR;
};

const unsigned int ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_LEG_0 = 0;
const unsigned int ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_LEG_1 = 1;
const unsigned int ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_SOP_0 = 5;
const unsigned int ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_SOP_1 = 10;
const unsigned int ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_SOP_2 = 15;
const unsigned int ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_USAGE1 = 50;
const unsigned int ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_USAGE2 = 60;
const unsigned int ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_USAGE3 = 70;
const unsigned int ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_ITERATIONS_COUNT = 1000;
const double ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_COV_THRESHOLD = 0.5;
const double ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_COV1 = 0.3;
const double ScheduleRepeatLimitNonFeasibleLegsTest::DUMMY_COV2 = 0.6;


TEST_F(ScheduleRepeatLimitNonFeasibleLegsTest, testIsSatisfiedIfSopUsageBalanced)
{
  EXPECT_CALL(*_sopUsageCounter,
      getSopUsagesOnLeg(_))
        .WillRepeatedly(Return(DUMMY_USAGES));
  EXPECT_CALL(*_swapperInfo,
      getTotalIterationsCount())
        .WillRepeatedly(Return(DUMMY_ITERATIONS_COUNT));
  EXPECT_CALL(*_thresholdCalc,
      RoundBracketOperator(DUMMY_ITERATIONS_COUNT))
        .WillRepeatedly(Return(DUMMY_COV_THRESHOLD));
  EXPECT_CALL(*_covF,
        RoundBracketOperator(DUMMY_USAGES_VECTOR))
          .WillRepeatedly(Return(DUMMY_COV1));

  const unsigned int INSUFFICIENT_SOPS = 20;
  _ibfAppraiser->reportSopCount(DUMMY_LEG_0, INSUFFICIENT_SOPS);
  _ibfAppraiser->reportSopCount(DUMMY_LEG_1, INSUFFICIENT_SOPS);
  ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
}

TEST_F(ScheduleRepeatLimitNonFeasibleLegsTest, testIsNotSatisfiedIfSopUsageUnbalanced)
{
  EXPECT_CALL(*_sopUsageCounter,
      getSopUsagesOnLeg(_))
        .WillRepeatedly(Return(DUMMY_USAGES));
  EXPECT_CALL(*_swapperInfo,
      getTotalIterationsCount())
        .WillRepeatedly(Return(DUMMY_ITERATIONS_COUNT));
  EXPECT_CALL(*_thresholdCalc,
      RoundBracketOperator(DUMMY_ITERATIONS_COUNT))
        .WillRepeatedly(Return(DUMMY_COV_THRESHOLD));
  EXPECT_CALL(*_covF,
        RoundBracketOperator(DUMMY_USAGES_VECTOR))
          .WillRepeatedly(Return(DUMMY_COV2));

  const unsigned int INSUFFICIENT_SOPS = 20;
  _ibfAppraiser->reportSopCount(DUMMY_LEG_0, INSUFFICIENT_SOPS);
  _ibfAppraiser->reportSopCount(DUMMY_LEG_1, INSUFFICIENT_SOPS);
  ASSERT_EQ(false, _ibfAppraiser->isSatisfied());
}

TEST_F(ScheduleRepeatLimitNonFeasibleLegsTest, testIsSatisfiedIfUsagesEmpty)
{
  EXPECT_CALL(*_sopUsageCounter,
      getSopUsagesOnLeg(_))
        .WillRepeatedly(Return(EMPTY_USAGES));

  const unsigned int INSUFFICIENT_SOPS = 20;
  _ibfAppraiser->reportSopCount(DUMMY_LEG_0, INSUFFICIENT_SOPS);
  _ibfAppraiser->reportSopCount(DUMMY_LEG_1, INSUFFICIENT_SOPS);
  ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
}

TEST_F(ScheduleRepeatLimitNonFeasibleLegsTest, testIsSatisfiedIfOnlyOneSopUsed)
{
  DUMMY_USAGES.clear();
  DUMMY_USAGES[DUMMY_SOP_0] = DUMMY_USAGE1;
  EXPECT_CALL(*_sopUsageCounter,
      getSopUsagesOnLeg(_))
        .WillRepeatedly(Return(DUMMY_USAGES));

  const unsigned int INSUFFICIENT_SOPS = 20;
  _ibfAppraiser->reportSopCount(DUMMY_LEG_0, INSUFFICIENT_SOPS);
  _ibfAppraiser->reportSopCount(DUMMY_LEG_1, INSUFFICIENT_SOPS);
  ASSERT_EQ(true, _ibfAppraiser->isSatisfied());
}

} // namespace tse
