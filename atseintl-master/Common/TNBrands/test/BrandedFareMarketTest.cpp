//-------------------------------------------------------------------
//
//  Authors:     Andrzej Fediuk
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
#include "Common/TNBrands/BrandedFareMarket.h"
#include "Common/TNBrands/test/TNBrandsMocks.h"

#include <memory>

using namespace ::testing;

namespace tse
{

namespace skipper
{

class BrandedFareMarketTest: public Test
{
public:
  typedef BrandedFareMarket<MockItinGeometryCalculator<FareMarket> >
    TestedBrandedFareMarket;

  void SetUp()
  {
    _itinCalculator.reset(new MockItinGeometryCalculator<FareMarket>());
    _fareMarket.reset(new FareMarket());
    _brandedFareMarket.reset(
        new TestedBrandedFareMarket(*_itinCalculator, *_fareMarket));
  }

  void TearDown(){}

  std::shared_ptr<TestedBrandedFareMarket> _brandedFareMarket;
  std::shared_ptr<MockItinGeometryCalculator<FareMarket>> _itinCalculator;
  std::shared_ptr<FareMarket> _fareMarket;

  static const BrandCode DUMMY_BRAND_1;
  static const BrandCode DUMMY_BRAND_2;

  static const size_t DUMMY_SEGMENT_INDEX;
};

const BrandCode BrandedFareMarketTest::DUMMY_BRAND_1 = "APPLE";
const BrandCode BrandedFareMarketTest::DUMMY_BRAND_2 = "ORANGE";
const size_t BrandedFareMarketTest::DUMMY_SEGMENT_INDEX = 10;

TEST_F(BrandedFareMarketTest, testReturnsBrandCodesProperly)
{
  UnorderedBrandCodes expected;
  expected.insert(DUMMY_BRAND_1);
  expected.insert(DUMMY_BRAND_2);
  EXPECT_CALL(*_itinCalculator, getItinSpecificBrandCodes(Eq(ByRef(*_fareMarket))))
    .WillOnce(Return(expected));
  ASSERT_EQ(expected, _brandedFareMarket->getBrands());
}

TEST_F(BrandedFareMarketTest, testReturnsStartSegmentIndexProperly)
{
  EXPECT_CALL(*_itinCalculator,
    getFareMarketStartSegmentIndex(Eq(ByRef(*_fareMarket))))
    .WillOnce(Return(DUMMY_SEGMENT_INDEX));
  ASSERT_EQ(DUMMY_SEGMENT_INDEX, _brandedFareMarket->getStartSegmentIndex());
}

TEST_F(BrandedFareMarketTest, testReturnsEndSegmentIndexProperly)
{
  EXPECT_CALL(*_itinCalculator,
    getFareMarketEndSegmentIndex(Eq(ByRef(*_fareMarket))))
    .WillOnce(Return(DUMMY_SEGMENT_INDEX));
  ASSERT_EQ(DUMMY_SEGMENT_INDEX, _brandedFareMarket->getEndSegmentIndex());
}

TEST_F(BrandedFareMarketTest, testReturnsFareMarketProperly)
{
  ASSERT_EQ(_fareMarket.get(), _brandedFareMarket->getFareMarket());
}

} // namespace skipper

} // namespace tse
