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

#ifndef SWAPPER_TEST_UTILS_H
#define SWAPPER_TEST_UTILS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"

#include "Pricing/Shopping/IBF/ISopUsageTracker.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Swapper/IMapUpdater.h"
#include "Pricing/Shopping/Swapper/IObservableItemSet.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <memory>

using namespace ::testing;
using namespace boost;

namespace tse
{

class MockIbfBlackboard : public swp::IMapUpdater<utils::SopCombination, swp::BasicAppraiserScore> {
 public:
  MOCK_METHOD2(updateValue,
      void(const utils::SopCombination& key, const swp::BasicAppraiserScore& value));
};

class MockISopUsageTracker : public ISopUsageTracker {
 public:
  MOCK_METHOD1(combinationAdded, bool(const utils::SopCombination& combination));
  MOCK_METHOD1(combinationRemoved, void(const utils::SopCombination& combination));
  MOCK_CONST_METHOD0(getNbrOfSops, unsigned int());
  MOCK_CONST_METHOD0(getNbrOfUnusedSops, unsigned int());
  MOCK_CONST_METHOD1(getNbrOfUnusedSopsOnLeg, unsigned int(unsigned int));
  MOCK_CONST_METHOD0(toString,std::string());
};

class MockISopUsageCounter: public ISopUsageCounter {
public:
  MOCK_CONST_METHOD2(getUsageCount, unsigned int(unsigned int legId, uint32_t sopId));
  MOCK_CONST_METHOD1(getSopUsagesOnLeg, SopUsages(unsigned int legId));
};

class MockISwapperInfo: public swp::ISwapperInfo
{
public:
  MOCK_CONST_METHOD0(getTotalIterationsCount, unsigned int());
};


template<typename C>
class MockCoefficientOfVariation
{
public:
  double operator()(const C& c) const {
      return RoundBracketOperator(c); }

  MOCK_CONST_METHOD1_T(RoundBracketOperator,
        double(const C& c));
};


class MockCovThresholdCalculationPolicy
{
public:
  double operator()(size_t swapperIterationsSoFar) const {
      return RoundBracketOperator(swapperIterationsSoFar); }

  MOCK_CONST_METHOD1(RoundBracketOperator,
      double(size_t swapperIterationsSoFar));
};


template<typename TestedAppraiserType>
class IbfAppraiserTest: public Test
{
public:
  void SetUp()
  {
    _blackboard.reset(new MockIbfBlackboard());
    childSetup();
    _ibfAppraiser.reset(newAppraiser());
  }

  void TearDown(){}

  utils::SopCombination combination(unsigned int sop1, unsigned int sop2)
  {
    utils::SopCombination comb;
    comb.push_back(sop1);
    comb.push_back(sop2);
    return comb;
  }

  swp::BasicAppraiserScore addCombination(const utils::SopCombination& comb)
  {
    return _ibfAppraiser->beforeItemAdded(comb, *_blackboard);
  }

  void rateCombination(const utils::SopCombination& comb,
      const swp::BasicAppraiserScore& expected_score)
  {
    ASSERT_EQ(expected_score, addCombination(comb));
  }

  void removeCombination(const utils::SopCombination& comb)
  {
    _ibfAppraiser->beforeItemRemoved(comb, *_blackboard);
  }

  swp::BasicAppraiserScore score(swp::BasicAppraiserScore::CATEGORY category,
      int minorRank = swp::BasicAppraiserScore::DEFAULT_MINOR_RANK)
  {
    return swp::BasicAppraiserScore(category, minorRank);
  }

  std::shared_ptr<MockIbfBlackboard> _blackboard;
  std::shared_ptr<TestedAppraiserType> _ibfAppraiser;

protected:
  virtual void childSetup() {}
  virtual TestedAppraiserType* newAppraiser() = 0;
};


} // namespace tse

#endif // SWAPPER_TEST_UTILS_H
