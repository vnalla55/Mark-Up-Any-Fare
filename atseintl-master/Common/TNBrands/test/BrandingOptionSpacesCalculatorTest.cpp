//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
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
#include "Common/Logger.h"
#include "Common/TNBrands/ItinGeometryCalculator.h"
#include "Common/TNBrands/BrandingOptionSpacesCalculator.h"
#include "Common/TNBrands/test/TNBrandsMocks.h"

#include <memory>

using namespace ::testing;

namespace tse
{

namespace skipper
{

class SpaceBlockCalculatorTest: public Test
{
public:
  typedef SpaceBlockCalculator<MockTNBrandsFunctions>
      TestedSpaceBlockCalculator;

  void SetUp()
  {
    _functions = new MockTNBrandsFunctions();
    _blockCalculator.reset(new TestedSpaceBlockCalculator(_functions));
  }

  void TearDown()
  {}

  MockTNBrandsFunctions* _functions;
  std::shared_ptr<TestedSpaceBlockCalculator> _blockCalculator;
  static const CarrierCode DUMMY_CARRIER1;
  static const CarrierCode DUMMY_CARRIER2;
  static const BrandCode DUMMY_BRAND1;
  static const BrandCode DUMMY_BRAND2;
  static const BrandCode DUMMY_BRAND3;
  static const size_t DUMMY_TOTAL_NUMBER_OF_SPACES;
  static const size_t DUMMY_SPACE_INDEX;
};

const CarrierCode SpaceBlockCalculatorTest::DUMMY_CARRIER1 = "CX1";
const CarrierCode SpaceBlockCalculatorTest::DUMMY_CARRIER2 = "CX2";
const BrandCode SpaceBlockCalculatorTest::DUMMY_BRAND1 = "TOMATO";
const BrandCode SpaceBlockCalculatorTest::DUMMY_BRAND2 = "POMEGRANATE";
const BrandCode SpaceBlockCalculatorTest::DUMMY_BRAND3 = "POTATO";
const size_t SpaceBlockCalculatorTest::DUMMY_TOTAL_NUMBER_OF_SPACES = 777;
const size_t SpaceBlockCalculatorTest::DUMMY_SPACE_INDEX = 42;


TEST_F(SpaceBlockCalculatorTest, testCalculatesSpaceBlockCorectly)
{
  BrandCodeArraysPerCarrier segmentInfo = {
    {CarrierDirection(DUMMY_CARRIER1, Direction::ORIGINAL), {DUMMY_BRAND1, DUMMY_BRAND3}},
    {CarrierDirection(DUMMY_CARRIER1, Direction::REVERSED), {DUMMY_BRAND1}},
    {CarrierDirection(DUMMY_CARRIER2, Direction::BOTHWAYS), {DUMMY_BRAND3, DUMMY_BRAND2, DUMMY_BRAND1}}
  };

  // Use segmentInfo[].size() to determine for which carrier the call is made.
  // In case of DUMMY_CARRIER1 we want to return DUMMY_BRAND3 which is at index 1.
  EXPECT_CALL(*_functions, calculateProportionalIndex(
      DUMMY_SPACE_INDEX, segmentInfo[CarrierDirection(DUMMY_CARRIER1, Direction::ORIGINAL)].size(),
      DUMMY_TOTAL_NUMBER_OF_SPACES))
          .WillOnce(Return(1));

  // Use segmentInfo[].size() to determine for which carrier the call is made.
  // In case of DUMMY_CARRIER1 we want to return DUMMY_BRAND1 which is at index 0.
  EXPECT_CALL(*_functions, calculateProportionalIndex(
      DUMMY_SPACE_INDEX, segmentInfo[CarrierDirection(DUMMY_CARRIER1, Direction::REVERSED)].size(),
      DUMMY_TOTAL_NUMBER_OF_SPACES))
          .WillOnce(Return(0));

  // Use segmentInfo[].size() to determine for which carrier the call is made.
  // In case of DUMMY_CARRIER2 we want to return DUMMY_BRAND3 which is at index 0.
  EXPECT_CALL(*_functions, calculateProportionalIndex(
      DUMMY_SPACE_INDEX, segmentInfo[CarrierDirection(DUMMY_CARRIER2, Direction::BOTHWAYS)].size(),
      DUMMY_TOTAL_NUMBER_OF_SPACES))
          .WillOnce(Return(0));

  CarrierBrandPairs expected = {
    {CarrierDirection(DUMMY_CARRIER1, Direction::ORIGINAL), DUMMY_BRAND3},
    {CarrierDirection(DUMMY_CARRIER1, Direction::REVERSED), DUMMY_BRAND1},
    {CarrierDirection(DUMMY_CARRIER2, Direction::BOTHWAYS), DUMMY_BRAND3}
  };
  ASSERT_EQ(expected, _blockCalculator->calculateSpaceBlock(
      segmentInfo, DUMMY_TOTAL_NUMBER_OF_SPACES,
      DUMMY_SPACE_INDEX, DUMMY_TOTAL_NUMBER_OF_SPACES));
}

TEST_F(SpaceBlockCalculatorTest, testCalculatesSpaceBlockCorectlyForNoBrandCarrier)
{
  const size_t MAX_BRANDS_ON_THIS_SEGMENT = 666;

  BrandCodeArraysPerCarrier segmentInfo = {
    {CarrierDirection(DUMMY_CARRIER1, Direction::BOTHWAYS), {NO_BRAND}},
    {CarrierDirection(DUMMY_CARRIER2, Direction::BOTHWAYS), {
        DUMMY_BRAND3, DUMMY_BRAND2, DUMMY_BRAND1}}
  };

  // Use MAX_BRANDS_ON_THIS_SEGMENT to determine that this call was made for
  // DUMMY_CARRIER1, and in this case we want to return NO_BRAND which is at index 0.
  EXPECT_CALL(*_functions, calculateProportionalIndex(
      DUMMY_SPACE_INDEX, MAX_BRANDS_ON_THIS_SEGMENT, DUMMY_TOTAL_NUMBER_OF_SPACES))
          .WillOnce(Return(0));

  // Use segmentInfo[].size() to determine for which carrier the call is made.
  // In case of DUMMY_CARRIER2 we want to return DUMMY_BRAND3 which is at index 0.
  EXPECT_CALL(*_functions, calculateProportionalIndex(
      DUMMY_SPACE_INDEX, 3, DUMMY_TOTAL_NUMBER_OF_SPACES))
          .WillOnce(Return(0));

  CarrierBrandPairs segment = {
    {CarrierDirection(DUMMY_CARRIER1, Direction::BOTHWAYS), NO_BRAND}
  };
  EXPECT_CALL(*_functions, setUpdateNoBrand(_, _))
      .WillOnce(SetArgReferee<0>(segment));

  CarrierBrandPairs expected = {
    {CarrierDirection(DUMMY_CARRIER1, Direction::BOTHWAYS), NO_BRAND},
    {CarrierDirection(DUMMY_CARRIER2, Direction::BOTHWAYS), DUMMY_BRAND3}
  };
  ASSERT_EQ(expected, _blockCalculator->calculateSpaceBlock(
      segmentInfo, DUMMY_TOTAL_NUMBER_OF_SPACES,
      DUMMY_SPACE_INDEX, MAX_BRANDS_ON_THIS_SEGMENT));
}

TEST_F(SpaceBlockCalculatorTest, testCalculatesSpaceBlockCorectlyForNoBrandCarrierOnHighCabin)
{
  const bool NOT_FIRST_CABIN = false;
  const size_t MAX_BRANDS_ON_THIS_SEGMENT = 666;

  BrandCodeArraysPerCarrier segmentInfo = {
    {CarrierDirection(DUMMY_CARRIER1, Direction::BOTHWAYS), {NO_BRAND}},
    {CarrierDirection(DUMMY_CARRIER2, Direction::BOTHWAYS),
      {DUMMY_BRAND3, DUMMY_BRAND2, DUMMY_BRAND1}}
  };

  // Use segmentInfo[].size() to determine for which carrier the call is made.
  // In case of DUMMY_CARRIER2 we want to return DUMMY_BRAND3 which is at index 0.
  // calculateProportionalIndex should be called only for DUMMY_CARRIER2 as
  // DUMMY_CARRIER1 contains only NO_BRAND and we request higher cabin (not
  // the first requested).
  EXPECT_CALL(*_functions, calculateProportionalIndex(
      DUMMY_SPACE_INDEX, 3, DUMMY_TOTAL_NUMBER_OF_SPACES))
          .WillOnce(Return(0));

  CarrierBrandPairs expected = {
    {CarrierDirection(DUMMY_CARRIER2, Direction::BOTHWAYS), DUMMY_BRAND3}
  };
  ASSERT_EQ(expected, _blockCalculator->calculateSpaceBlock(
      segmentInfo, DUMMY_TOTAL_NUMBER_OF_SPACES,
      DUMMY_SPACE_INDEX, MAX_BRANDS_ON_THIS_SEGMENT, NOT_FIRST_CABIN));
}

class BrandingOptionSpacesCalculatorTest: public Test
{
public:
  typedef BrandingOptionSpacesCalculator<
      MockTNBrandsFunctions, MockSpaceBlockCalculator>
      TestedBrandingOptionSpacesCalculator;

  void SetUp()
  {
    _functions = new MockTNBrandsFunctions();
    _blockCalculator = new MockSpaceBlockCalculator();
    _calculator.reset(new TestedBrandingOptionSpacesCalculator(
        _functions, _blockCalculator));

    _cxrInfo1[CarrierDirection(DUMMY_CXR1, Direction::BOTHWAYS)];
    _cxrInfo2[CarrierDirection(DUMMY_CXR2, Direction::BOTHWAYS)];
    _brandInfo.push_back(_cxrInfo1);
    _brandInfo.push_back(_cxrInfo2);
  }

  void TearDown(){}

  // returns carrier/brand pair with given carrier and LOVING_BRAND or NO_BRAND
  CarrierBrandPairs dummyBlock(const CarrierDirection& code, bool lovingBrand = true)
  {
    CarrierBrandPairs result;
    if (lovingBrand)
    {
      result[code] = LOVING_BRAND;
    }
    else
    {
      result[code] = NO_BRAND;
    }
    return result;
  }

  std::shared_ptr<TestedBrandingOptionSpacesCalculator> _calculator;
  MockTNBrandsFunctions* _functions;
  MockSpaceBlockCalculator* _blockCalculator;
  SegmentOrientedBrandCodeArraysPerCarrier _brandInfo;
  BrandCodeArraysPerCarrier _cxrInfo1;
  BrandCodeArraysPerCarrier _cxrInfo2;

  static const CarrierCode DUMMY_CXR1;
  static const CarrierCode DUMMY_CXR2;
  static const size_t DUMMY_TOTAL_SPACES_COUNT;
  static const BrandCode LOVING_BRAND;
};

const CarrierCode BrandingOptionSpacesCalculatorTest::DUMMY_CXR1 = "CX1";
const CarrierCode BrandingOptionSpacesCalculatorTest::DUMMY_CXR2 = "CX2";
const size_t BrandingOptionSpacesCalculatorTest::DUMMY_TOTAL_SPACES_COUNT = 3;
const BrandCode BrandingOptionSpacesCalculatorTest::LOVING_BRAND = "<3";

TEST_F(BrandingOptionSpacesCalculatorTest, testCalculatesForAllSpacesAndSegments)
{
  const size_t NO_LIMIT = 0;
  const bool FIRST_CABIN = true;

  EXPECT_CALL(*_functions, calculateMaxBrandsCountPerCarrier(_brandInfo))
      .WillOnce(Return(DUMMY_TOTAL_SPACES_COUNT));
  EXPECT_CALL(*_functions, calculateMaxBrandsCountPerCarrier(_brandInfo, _))
      .WillRepeatedly(Return(DUMMY_TOTAL_SPACES_COUNT));

  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo1, DUMMY_TOTAL_SPACES_COUNT, 0, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("a", Direction::ORIGINAL))));
  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo2, DUMMY_TOTAL_SPACES_COUNT, 0, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("b", Direction::BOTHWAYS))));

  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo1, DUMMY_TOTAL_SPACES_COUNT, 1, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("c", Direction::ORIGINAL))));
  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo2, DUMMY_TOTAL_SPACES_COUNT, 1, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("d", Direction::ORIGINAL))));

  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo1, DUMMY_TOTAL_SPACES_COUNT, 2, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("e", Direction::ORIGINAL))));
  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo2, DUMMY_TOTAL_SPACES_COUNT, 2, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("d", Direction::REVERSED))));

  BrandingOptionSpaces output;
  _calculator->calculateSpaces(_brandInfo, output, NO_LIMIT, FIRST_CABIN);

  // We expect the following output:
  // segment 1   segment 2
  //     <reserved>
  // (a,o <3)     (b,b <3)
  // (c,o <3)     (d,o <3)
  // (e,o <3)     (d,r <3)
  BrandingOptionSpaces expected;
  BrandingOptionSpace tmp;
  expected.push_back(tmp);
  tmp.push_back(dummyBlock(CarrierDirection("a", Direction::ORIGINAL)));
  tmp.push_back(dummyBlock(CarrierDirection("b", Direction::BOTHWAYS)));
  expected.push_back(tmp);
  tmp.clear();
  tmp.push_back(dummyBlock(CarrierDirection("c", Direction::ORIGINAL)));
  tmp.push_back(dummyBlock(CarrierDirection("d", Direction::ORIGINAL)));
  expected.push_back(tmp);
  tmp.clear();
  tmp.push_back(dummyBlock(CarrierDirection("e", Direction::ORIGINAL)));
  tmp.push_back(dummyBlock(CarrierDirection("d", Direction::REVERSED)));
  expected.push_back(tmp);

  ASSERT_EQ(expected, output);
}

TEST_F(BrandingOptionSpacesCalculatorTest, testCalculatesForAllSpacesAndSegmentsWithLimit)
{
  const size_t LIMIT = 2;
  const bool FIRST_CABIN = true;

  EXPECT_CALL(*_functions, calculateMaxBrandsCountPerCarrier(_brandInfo))
      .WillOnce(Return(DUMMY_TOTAL_SPACES_COUNT));
  EXPECT_CALL(*_functions, calculateMaxBrandsCountPerCarrier(_brandInfo, _))
      .WillRepeatedly(Return(DUMMY_TOTAL_SPACES_COUNT));


  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo1, DUMMY_TOTAL_SPACES_COUNT, 0, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("a", Direction::BOTHWAYS))));
  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo2, DUMMY_TOTAL_SPACES_COUNT, 0, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("b", Direction::BOTHWAYS))));

  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo1, DUMMY_TOTAL_SPACES_COUNT, 1, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("c", Direction::BOTHWAYS))));
  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo2, DUMMY_TOTAL_SPACES_COUNT, 1, DUMMY_TOTAL_SPACES_COUNT, FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("d", Direction::BOTHWAYS))));

  BrandingOptionSpaces output;
  _calculator->calculateSpaces(_brandInfo, output, LIMIT, FIRST_CABIN);

  // We expect the following output:
  // segment 1   segment 2
  //     <reserved>
  // (a, <3)     (b, <3)
  // (c, <3)     (d, <3)
  BrandingOptionSpaces expected;
  BrandingOptionSpace tmp;
  expected.push_back(tmp);
  tmp.push_back(dummyBlock(CarrierDirection("a", Direction::BOTHWAYS)));
  tmp.push_back(dummyBlock(CarrierDirection("b", Direction::BOTHWAYS)));
  expected.push_back(tmp);
  tmp.clear();
  tmp.push_back(dummyBlock(CarrierDirection("c", Direction::BOTHWAYS)));
  tmp.push_back(dummyBlock(CarrierDirection("d", Direction::BOTHWAYS)));
  expected.push_back(tmp);

  ASSERT_EQ(expected, output);
}

TEST_F(BrandingOptionSpacesCalculatorTest, testCalculatesForAllSpacesAndSegmentsOnHighCabin)
{
  const size_t NO_LIMIT = 0;
  const bool NOT_FIRST_CABIN = false;

  const size_t ONLY_ONE_SPACE = 1;

  EXPECT_CALL(*_functions, calculateMaxBrandsCountPerCarrier(_brandInfo))
      .WillOnce(Return(ONLY_ONE_SPACE));
  EXPECT_CALL(*_functions, calculateMaxBrandsCountPerCarrier(_brandInfo, _))
      .WillRepeatedly(Return(ONLY_ONE_SPACE));

  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo1, ONLY_ONE_SPACE, 0, ONLY_ONE_SPACE, NOT_FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("a", Direction::BOTHWAYS), NOT_FIRST_CABIN)));
  EXPECT_CALL(*_blockCalculator,
      calculateSpaceBlock(_cxrInfo2, ONLY_ONE_SPACE, 0, ONLY_ONE_SPACE, NOT_FIRST_CABIN))
        .WillOnce(Return(dummyBlock(CarrierDirection("b", Direction::BOTHWAYS), NOT_FIRST_CABIN)));

  BrandingOptionSpaces output;
  _calculator->calculateSpaces(_brandInfo, output, NO_LIMIT, NOT_FIRST_CABIN);

  // We expect the following output:
  // segment 1   segment 2
  // (a, NO_BRAND)     (b, NO_BRAND)
  // but because this is higher cabin (not the first) the result should be empty.
  BrandingOptionSpaces expected;

  ASSERT_EQ(expected, output);
}

TEST_F(BrandingOptionSpacesCalculatorTest, testRaisesOnLimitSetToOne)
{
  BrandingOptionSpaces output;
  const size_t IMPROPER_LIMIT = 1;

  ASSERT_THROW(
      _calculator->calculateSpaces(_brandInfo, output, IMPROPER_LIMIT),
      ErrorResponseException);
}

} // namespace skipper

} // namespace tse

