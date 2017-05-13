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

#include "Common/ErrorResponseException.h"
#include "Pricing/Shopping/Swapper/IObservableItemSet.h"
#include "Pricing/Shopping/Utils/SopPenaltyGenerator.h"

#include <memory>

using namespace ::testing;
using ::testing::_;

using namespace boost;

namespace tse
{

namespace utils
{

class MockIObservableItemSet:
    public swp::IObservableItemSet<SopCombination>
{
 public:
  MOCK_METHOD1(addItemSetObserver,
      void(swp::IItemSetObserver<SopCombination>* observer));
  MOCK_METHOD1(removeItemSetObserver,
      void(swp::IItemSetObserver<SopCombination>* observer));
};

// usage = sopId + 1000
class FakeISopUsageCounter : public ISopUsageCounter
{
public:
  FakeISopUsageCounter(size_t offset = 1000)
  {
    setOffset(offset);
  }

  unsigned int getUsageCount(unsigned int legId, uint32_t sopId) const
  {
    return sopId + _offset;
  }

  SopUsages getSopUsagesOnLeg(unsigned int legId) const
  {
    // Return empty set: not used here
    return SopUsages();
  }

  void setOffset(size_t offset)
  {
    _offset = offset;
  }

private:
  size_t _offset;
};


class FakePenaltyPolicy
{
public:
  SopPenaltyRecord createPenaltyRecord(
      const utils::SopEntry& entry, size_t usageCount, size_t)
  {
    SopPenaltyRecord record;
    record.legId = entry.legId;
    record.sopId = entry.sopId;
    record.usage_count = usageCount;
    record.penalty = 0;
    // force Sops descendant order according to sopId
    record.rank = 10000 - usageCount;
    return record;
  }
};


class MockISopCombinationsGenerator : public ISopCombinationsGenerator {
 public:
  MOCK_METHOD1(setNumberOfLegs,
      void(unsigned int legs));
  MOCK_CONST_METHOD0(getNumberOfLegs,
      unsigned int());
  MOCK_METHOD2(addSop,
      void(unsigned int legId, uint32_t sopId));
  MOCK_CONST_METHOD1(getSopsOnLeg,
      const SopCombination&(unsigned int legId));
  MOCK_METHOD0(next,
      SopCombination());
  MOCK_METHOD0(manualInit,
      void());
};



class FakeISopCombinationsGeneratorFactory :
    public ISopCombinationsGeneratorFactory
{
public:
  FakeISopCombinationsGeneratorFactory(
      MockISopCombinationsGenerator* mock):
    _mock(mock){}

  // Returns a proxy to mock that may be deleted
  ISopCombinationsGenerator* create()
  {
    return new ISopCombinationsGeneratorDecorator(_mock);
  }

private:
  MockISopCombinationsGenerator* _mock;
};


class MockISopBank : public ISopBank {
 public:
  MOCK_METHOD1(setNumberOfLegs,
      void(unsigned int legs));
  MOCK_CONST_METHOD0(getNumberOfLegs,
      unsigned int());
  MOCK_METHOD2(addSop,
      void(unsigned int legId, uint32_t sopId));
  MOCK_METHOD1(getSopsOnLeg,
      SopCombination&(unsigned int legId));
  MOCK_CONST_METHOD0(toString,
      std::string());
};



class SopPenaltyGeneratorTest: public Test
{
public:
  typedef SopPenaltyGenerator<FakePenaltyPolicy> TestedSopPenaltyGenerator;

  void SetUp()
  {
    _observableItemSet.reset(new MockIObservableItemSet());
    _sopUsageCounter.reset(new FakeISopUsageCounter());
    _childGenerator.reset(new MockISopCombinationsGenerator());
    _childGenFactory = new FakeISopCombinationsGeneratorFactory(_childGenerator.get());
    _bank = new MockISopBank();
    _policy = new FakePenaltyPolicy();
    _generator.reset(new TestedSopPenaltyGenerator(
        *_observableItemSet,
        *_sopUsageCounter, 0, // no logger
        _childGenFactory, _policy, _bank));
    setupCombinations();
    setupCommonExpectations();
  }

  void TearDown(){}

  void setupCombinations()
  {
    FIRST_LEG.push_back(SOPID1);
    FIRST_LEG.push_back(SOPID2);
    SECOND_LEG.push_back(SOPID3);
    SECOND_LEG.push_back(SOPID4);
    SECOND_LEG.push_back(SOPID5);
    SECOND_LEG.push_back(SOPID6);
    THIRD_LEG.push_back(SOPID7);

    DUMMY_COMBINATION.push_back(SOPID2);
    DUMMY_COMBINATION.push_back(SOPID3);
    DUMMY_COMBINATION.push_back(SOPID7);
  }

  void setupCommonExpectations()
  {
    EXPECT_CALL(*_bank, getNumberOfLegs())
      .WillRepeatedly(Return(DUMMY_LEGS_NUMBER));
    EXPECT_CALL(*_bank, getSopsOnLeg(0))
      .WillRepeatedly(ReturnRef(FIRST_LEG));
    EXPECT_CALL(*_bank, getSopsOnLeg(1))
      .WillRepeatedly(ReturnRef(SECOND_LEG));
    EXPECT_CALL(*_bank, getSopsOnLeg(2))
      .WillRepeatedly(ReturnRef(THIRD_LEG));
    EXPECT_CALL(*_observableItemSet, removeItemSetObserver(_generator.get()));
  }

  // 10000 - (usage_offset + sopId)
  size_t expectedFakeRank(size_t sopId, size_t usage_offset = 1000) const
  {
    return 10000 - usage_offset - sopId;
  }

  void testInit()
  {
    // We expect the desdendig SopId order since
    // rank = 10000 - (1000 + sopId) (FakePenaltyPolicy)
    {
      InSequence dummy;
      EXPECT_CALL(*_childGenerator, setNumberOfLegs(DUMMY_LEGS_NUMBER));
      EXPECT_CALL(*_childGenerator, addSop(0, SOPID2));
      EXPECT_CALL(*_childGenerator, addSop(0, SOPID1));
      EXPECT_CALL(*_childGenerator, addSop(1, SOPID6));
      EXPECT_CALL(*_childGenerator, addSop(1, SOPID3));
      EXPECT_CALL(*_childGenerator, addSop(1, SOPID5));
      EXPECT_CALL(*_childGenerator, addSop(1, SOPID4));
      EXPECT_CALL(*_childGenerator, addSop(2, SOPID7));
      EXPECT_CALL(*_observableItemSet, addItemSetObserver(_generator.get()));
    }
    _generator->manualInit();

    // All SOPs in the map should have rank calculated
    // from their sopIds by both fake objects.
    ASSERT_EQ(expectedFakeRank(SOPID1), _generator->getPenaltyRecord(SopEntry(0, SOPID1)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID2), _generator->getPenaltyRecord(SopEntry(0, SOPID2)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID3), _generator->getPenaltyRecord(SopEntry(1, SOPID3)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID4), _generator->getPenaltyRecord(SopEntry(1, SOPID4)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID5), _generator->getPenaltyRecord(SopEntry(1, SOPID5)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID6), _generator->getPenaltyRecord(SopEntry(1, SOPID6)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID7), _generator->getPenaltyRecord(SopEntry(2, SOPID7)).rank);
  }

  void testCombinationAddedOrRemoved(bool added, size_t offset = 2000)
  {
    // Emulate different usages now
    _sopUsageCounter->setOffset(offset);

    {
      // The order is altered by the new usages
      InSequence dummy;
      EXPECT_CALL(*_childGenerator, setNumberOfLegs(DUMMY_LEGS_NUMBER));
      EXPECT_CALL(*_childGenerator, addSop(0, SOPID2));
      EXPECT_CALL(*_childGenerator, addSop(0, SOPID1));
      EXPECT_CALL(*_childGenerator, addSop(1, SOPID3));
      EXPECT_CALL(*_childGenerator, addSop(1, SOPID6));
      EXPECT_CALL(*_childGenerator, addSop(1, SOPID5));
      EXPECT_CALL(*_childGenerator, addSop(1, SOPID4));
      EXPECT_CALL(*_childGenerator, addSop(2, SOPID7));
    }

    if (added)
    {
      _generator->itemAdded(DUMMY_COMBINATION);
    }
    else
    {
      _generator->itemRemoved(DUMMY_COMBINATION);
    }

    // Rank changes only for the newly inserted Sops
    ASSERT_EQ(expectedFakeRank(SOPID1), _generator->getPenaltyRecord(SopEntry(0, SOPID1)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID2, 2000), _generator->getPenaltyRecord(SopEntry(0, SOPID2)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID3, 2000), _generator->getPenaltyRecord(SopEntry(1, SOPID3)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID4), _generator->getPenaltyRecord(SopEntry(1, SOPID4)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID5), _generator->getPenaltyRecord(SopEntry(1, SOPID5)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID6), _generator->getPenaltyRecord(SopEntry(1, SOPID6)).rank);
    ASSERT_EQ(expectedFakeRank(SOPID7, 2000), _generator->getPenaltyRecord(SopEntry(2, SOPID7)).rank);
  }

  SopCombination dummyThreeLegCombination(size_t sopId1, size_t sopId2, size_t sopId3)
  {
    SopCombination dummy;
    dummy.push_back(sopId1);
    dummy.push_back(sopId2);
    dummy.push_back(sopId3);
    return dummy;
  }

  std::shared_ptr<MockIObservableItemSet> _observableItemSet;
  std::shared_ptr<FakeISopUsageCounter> _sopUsageCounter;
  FakeISopCombinationsGeneratorFactory* _childGenFactory;
  std::shared_ptr<MockISopCombinationsGenerator> _childGenerator;
  MockISopBank* _bank;
  FakePenaltyPolicy* _policy;
  std::shared_ptr<TestedSopPenaltyGenerator> _generator;

  SopCombination FIRST_LEG;
  SopCombination SECOND_LEG;
  SopCombination THIRD_LEG;

  SopCombination DUMMY_COMBINATION;

  static const size_t DUMMY_LEGS_NUMBER;
  static const size_t DUMMY_USAGE_COUNT;
  static const size_t SOPID1;
  static const size_t SOPID2;
  static const size_t SOPID3;
  static const size_t SOPID4;
  static const size_t SOPID5;
  static const size_t SOPID6;
  static const size_t SOPID7;

};

const size_t SopPenaltyGeneratorTest::DUMMY_LEGS_NUMBER = 3;
const size_t SopPenaltyGeneratorTest::DUMMY_USAGE_COUNT = 5;
const size_t SopPenaltyGeneratorTest::SOPID1 = 293;
const size_t SopPenaltyGeneratorTest::SOPID2 = 295;
const size_t SopPenaltyGeneratorTest::SOPID3 = 290;
const size_t SopPenaltyGeneratorTest::SOPID4 = 230;
const size_t SopPenaltyGeneratorTest::SOPID5 = 250;
const size_t SopPenaltyGeneratorTest::SOPID6 = 293;
const size_t SopPenaltyGeneratorTest::SOPID7 = 283;


TEST_F(SopPenaltyGeneratorTest, testInitialization)
{
  testInit();
}

TEST_F(SopPenaltyGeneratorTest, testJustDrawsFromTheChildGeneratorAfterInit)
{
  testInit();
  EXPECT_CALL(*_childGenerator, next())
    .WillOnce(Return(DUMMY_COMBINATION));
  ASSERT_EQ(DUMMY_COMBINATION, _generator->next());
}

TEST_F(SopPenaltyGeneratorTest, reloadsChildGenIfCombinationAdded)
{
  testInit();
  testCombinationAddedOrRemoved(true);
}

TEST_F(SopPenaltyGeneratorTest, testJustDrawsFromTheChildGeneratorAfterCombinationAdded)
{
  testInit();
  testCombinationAddedOrRemoved(true);

  SopCombination DUMMY = dummyThreeLegCombination(
      SOPID6, SOPID2, SOPID3);

  EXPECT_CALL(*_childGenerator, next())
      .WillOnce(Return(DUMMY));
  ASSERT_EQ(DUMMY, _generator->next());
}

TEST_F(SopPenaltyGeneratorTest, reloadsChildGenIfCombinationRemoved)
{
  testInit();
  testCombinationAddedOrRemoved(false);
}

TEST_F(SopPenaltyGeneratorTest, testJustDrawsFromTheChildGeneratorAfterCombinationRemoved)
{
  testInit();
  testCombinationAddedOrRemoved(false);

  SopCombination DUMMY = dummyThreeLegCombination(
        SOPID6, SOPID2, SOPID3);

  EXPECT_CALL(*_childGenerator, next())
      .WillOnce(Return(DUMMY));
  ASSERT_EQ(DUMMY, _generator->next());
}

class BasicPenaltyPolicyTest: public Test
{
public:

  void SetUp()
  {
    _policy.reset(new BasicPenaltyPolicy());
  }

  void TearDown(){}

  std::shared_ptr<BasicPenaltyPolicy> _policy;

  static const size_t EXPECTED_PENALTIES[];
  static const size_t DUMMY_LEG_ID;
  static const size_t DUMMY_SOP_ID;
};


const size_t BasicPenaltyPolicyTest::DUMMY_LEG_ID = 100;
const size_t BasicPenaltyPolicyTest::DUMMY_SOP_ID = 1000;


const size_t BasicPenaltyPolicyTest::EXPECTED_PENALTIES[] = {
    0, // 0
    8, // 1
    27, // 2
    64, // 3
    125, // 4
    216, // 5
    343, // 6
    512, // 7
    729, // 8
    1000, // 9
    1331, // 10
    1728,
    2197,
    2744,
    3375,
    4096,
    4913,
    5832,
    6859,
    8000,
    9261,
    10648,
    12167,
    13824,
    15625,
    17576,
    19683,
    21952,
    24389,
    27000,
    29791,
    32768,
    35937,
    39304,
    42875,
    46656,
    50653,
    54872,
    59319,
    64000,
    68921,
    74088,
    79507,
    85184,
    91125,
    97336,
    103823,
    110592,
    117649,
    125000,
    125000,
    125000,
    125000,
    125000,
};


TEST_F(BasicPenaltyPolicyTest, testCalculatesPenaltyAndRankCorrectly)
{
  for (size_t usage_count = 0; usage_count <
    (sizeof(EXPECTED_PENALTIES)/sizeof(EXPECTED_PENALTIES[0]));
    ++usage_count)
  {
    SopPenaltyRecord record = _policy->createPenaltyRecord(
        SopEntry(DUMMY_LEG_ID, DUMMY_SOP_ID), usage_count, 0);
    ASSERT_EQ(DUMMY_LEG_ID, record.legId);
    ASSERT_EQ(DUMMY_SOP_ID, record.sopId);
    ASSERT_EQ(usage_count, record.usage_count);
    ASSERT_EQ(EXPECTED_PENALTIES[usage_count], record.penalty);
    ASSERT_EQ(DUMMY_SOP_ID + record.penalty, record.rank);
  }
}

} // namespace utils

} // namespace tse

