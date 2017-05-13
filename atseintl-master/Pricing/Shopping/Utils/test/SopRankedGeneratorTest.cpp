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
#include "Pricing/Shopping/Utils/SopRankedGenerator.h"

#include <memory>

using namespace ::testing;
using namespace boost;

namespace tse
{

namespace utils
{


class MockRankedCombinations {
public:
  MOCK_METHOD2(setDimensionFinite,
      void(size_t pos, size_t dimensionLength));
  MOCK_METHOD0(next,
      RankedCombinationsIndices());
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


class MockIRankedCombinationsFactory : public IRankedCombinationsFactory<MockRankedCombinations> {
 public:
  MOCK_METHOD1_T(create,
      MockRankedCombinations*(size_t width));
};


class SopRankedGeneratorTest: public Test
{
public:
  typedef SopRankedGenerator<MockRankedCombinations> TestedSopRankedGenerator;

  void SetUp()
  {
    FIRST_LEG.push_back(5);
    FIRST_LEG.push_back(10);
    SECOND_LEG.push_back(9);
    SECOND_LEG.push_back(79);
    SECOND_LEG.push_back(379);

    _bank = new MockISopBank();
    _factory = new MockIRankedCombinationsFactory();
    _rankedCombinations = new MockRankedCombinations();
    _generator.reset(new TestedSopRankedGenerator(_factory, _bank));
  }

  void TearDown(){}

  RankedCombinationsIndices createIndices(size_t index1, size_t index2)
  {
    RankedCombinationsIndices indices;
    indices.push_back(index1);
    indices.push_back(index2);
    return indices;
  }

  void expectNext(size_t index1, size_t index2)
  {
    SopCombination expected;
    expected.push_back(FIRST_LEG[index1]);
    expected.push_back(SECOND_LEG[index2]);
    ASSERT_EQ(expected, _generator->next());
  }

  void testInit()
  {
    EXPECT_CALL(*_bank, getNumberOfLegs())
      .WillOnce(Return(DUMMY_LEGS_NUMBER));

    EXPECT_CALL(*_factory, create(DUMMY_LEGS_NUMBER))
      .WillOnce(Return(_rankedCombinations));

    EXPECT_CALL(*_bank, getSopsOnLeg(0))
      .WillOnce(ReturnRef(FIRST_LEG));
    EXPECT_CALL(*_bank, getSopsOnLeg(1))
      .WillOnce(ReturnRef(SECOND_LEG));

    EXPECT_CALL(*_rankedCombinations, setDimensionFinite(0, FIRST_LEG.size()));
    EXPECT_CALL(*_rankedCombinations, setDimensionFinite(1, SECOND_LEG.size()));
    _generator->manualInit();
  }

  void testNext(size_t index1, size_t index2)
  {
    EXPECT_CALL(*_rankedCombinations, next())
      .WillOnce(Return(createIndices(index1, index2)));
    EXPECT_CALL(*_bank, getSopsOnLeg(0))
      .WillOnce(ReturnRef(FIRST_LEG));
    EXPECT_CALL(*_bank, getSopsOnLeg(1))
      .WillOnce(ReturnRef(SECOND_LEG));
    expectNext(index1, index2);
  }


  void testNextEmpty()
  {
    EXPECT_CALL(*_rankedCombinations, next())
      .WillRepeatedly(Return(EMPTY_INDICES));
    assureExhausted();
  }

  void assureExhausted()
  {
    for (size_t i = 0; i < ASSURE_EXHAUSTED_COUNT; ++i)
    {
      ASSERT_EQ(EMPTY_COMBINATION, _generator->next());
    }
  }

  MockISopBank* _bank;
  MockIRankedCombinationsFactory* _factory;
  MockRankedCombinations* _rankedCombinations;
  std::shared_ptr<TestedSopRankedGenerator> _generator;

  SopCombination FIRST_LEG;
  SopCombination SECOND_LEG;
  SopCombination EMPTY_COMBINATION;
  RankedCombinationsIndices EMPTY_INDICES;

  static const size_t DUMMY_LEGS_NUMBER;
  static const size_t ASSURE_EXHAUSTED_COUNT;
};

const size_t SopRankedGeneratorTest::DUMMY_LEGS_NUMBER = 2;
const size_t SopRankedGeneratorTest::ASSURE_EXHAUSTED_COUNT = 10;


TEST_F(SopRankedGeneratorTest, testInitialization)
{
  testInit();
}

TEST_F(SopRankedGeneratorTest, testSeveralCallsToNext)
{
  testInit();
  testNext(1, 0);
  testNext(1, 2);
  testNext(0, 0);
  testNext(1, 1);
  testNext(0, 2);
  testNext(1, 0);
}

TEST_F(SopRankedGeneratorTest, testReturnsEmptyCombinationForEmptyIndices)
{
  testInit();
  testNextEmpty();
}

TEST_F(SopRankedGeneratorTest, testReturnsEmptyCombinationForEmptyIndicesAfterSomeNonempty)
{
  testInit();
  testNext(1, 2);
  testNext(0, 1);
  testNextEmpty();
}


} // namespace utils

} // namespace tse

