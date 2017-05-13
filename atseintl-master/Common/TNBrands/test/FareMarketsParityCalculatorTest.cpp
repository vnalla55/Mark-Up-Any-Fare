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
#include "test/include/TestConfigInitializer.h"

#include "Common/ErrorResponseException.h"
#include "Common/TNBrands/FareMarketsParityCalculator.h"
#include "Common/TNBrands/test/TNBrandsMocks.h"

#include <memory>

using namespace ::testing;
using namespace boost;

namespace tse
{
namespace skipper
{

class FareMarketsParityCalculatorTest: public Test
{
public:
  typedef FareMarketsParityCalculator<
      MockItinGeometryCalculator<FareMarket>, MockBrandedFareMarketFactory>
    TestedFareMarketsParityCalculator;

  void SetUp()
  {
    _itinGeometryCalculator.reset(new MockItinGeometryCalculator<FareMarket>());
    _trxGeoCalculator.reset(new MockITrxGeometryCalculator());

    // mock used to catch recursive calls
    _mockFMParityCalculator.reset(new MockIFareMarketsParityCalculator());
    _mockFactory = new MockBrandedFareMarketFactory();

    // call in constructor
    EXPECT_CALL(*_itinGeometryCalculator, getSegmentCount())
      .WillOnce(Return(SEG_COUNT));

    _testedFMParityCalculator.reset(new TestedFareMarketsParityCalculator(
        *_itinGeometryCalculator, _mockFMParityCalculator.get(), _mockFactory));

    _fm1.reset(new FareMarket());
    _fm2.reset(new FareMarket());
    _fm3.reset(new FareMarket());
  }

  void TearDown(){}

  void addFareMarket(const FareMarket& fm, MockBrandedFareMarket* bfm)
  {
    PricingTrx trx;
    EXPECT_CALL(*_itinGeometryCalculator, getTrxGeometryCalculator())
      .WillOnce(ReturnRef(*_trxGeoCalculator));

    EXPECT_CALL(*_trxGeoCalculator, getTrx())
      .WillOnce(ReturnRef(trx));

    EXPECT_CALL(*_itinGeometryCalculator, isThruFareOnlyItin())
      .WillOnce(Return(false));

    EXPECT_CALL(*_mockFactory, create(_, Eq(ByRef(fm))))
      .WillOnce(Return(bfm));

    EXPECT_CALL(*bfm, getStartSegmentIndex())
      .WillOnce(Return(INDEX_0));
    _testedFMParityCalculator->addFareMarket(fm);
  }

  TestConfigInitializer _cfg;
  std::shared_ptr<MockItinGeometryCalculator<FareMarket>> _itinGeometryCalculator;
  std::shared_ptr<MockITrxGeometryCalculator> _trxGeoCalculator;
  std::shared_ptr<MockIFareMarketsParityCalculator> _mockFMParityCalculator;
  MockBrandedFareMarketFactory* _mockFactory;
  std::shared_ptr<TestedFareMarketsParityCalculator> _testedFMParityCalculator;

  std::shared_ptr<FareMarket> _fm1;
  std::shared_ptr<FareMarket> _fm2;
  std::shared_ptr<FareMarket> _fm3;

  MockBrandedFareMarket* _bfm1;
  MockBrandedFareMarket* _bfm2;
  MockBrandedFareMarket* _bfm3;

  static const BrandCode DUMMY_BRAND_1;
  static const BrandCode DUMMY_BRAND_2;
  static const BrandCode DUMMY_BRAND_3;

  static const size_t INDEX_0;
  static const size_t INDEX_1;
  static const size_t INDEX_2;
  static const size_t INDEX_3;
  static const size_t SEG_COUNT;
};

const BrandCode FareMarketsParityCalculatorTest::DUMMY_BRAND_1 = "APPLE";
const BrandCode FareMarketsParityCalculatorTest::DUMMY_BRAND_2 = "ORANGE";
const BrandCode FareMarketsParityCalculatorTest::DUMMY_BRAND_3 = "BANANA";
const size_t FareMarketsParityCalculatorTest::INDEX_0 = 0;
const size_t FareMarketsParityCalculatorTest::INDEX_1 = 1;
const size_t FareMarketsParityCalculatorTest::INDEX_2 = 2;
const size_t FareMarketsParityCalculatorTest::INDEX_3 = 3;
const size_t FareMarketsParityCalculatorTest::SEG_COUNT = 3;

TEST_F(FareMarketsParityCalculatorTest, testAddsFareMarketCorrectly)
{
  _bfm1 = new MockBrandedFareMarket();
  addFareMarket(*_fm1, _bfm1);
}

TEST_F(FareMarketsParityCalculatorTest, testReturnsEmptyOnSegmentIndexWithNoFaremarkets)
{
  FareMarketsPerBrandCode expected;
  ASSERT_EQ(expected, _testedFMParityCalculator->possibleBrands(INDEX_0, SEG_COUNT));
}

TEST_F(FareMarketsParityCalculatorTest, testRaisesCorrectlyOnEndSegmentIndexOutOfRange)
{
  ASSERT_THROW(_testedFMParityCalculator->possibleBrands(INDEX_0, SEG_COUNT + 1),
      ErrorResponseException);
}

TEST_F(FareMarketsParityCalculatorTest, testCorrectlyCallsPossibleBrandsWithEndSegmentSet)
{
  FareMarketsPerBrandCode singleBrand;
  EXPECT_CALL(*_mockFMParityCalculator, possibleBrands(INDEX_0, SEG_COUNT))
    .WillOnce(Return(singleBrand));

  _testedFMParityCalculator->possibleBrands(INDEX_0);
}

TEST_F(FareMarketsParityCalculatorTest, testCorrectlyReturnsPossibleBrandsWithAndWithoutCache)
{
  _bfm1 = new MockBrandedFareMarket();
  _bfm2 = new MockBrandedFareMarket();
  _bfm3 = new MockBrandedFareMarket();

  addFareMarket(*_fm1, _bfm1);
  addFareMarket(*_fm2, _bfm2);
  addFareMarket(*_fm3, _bfm3);

  EXPECT_CALL(*_bfm1, getFareMarket()).WillRepeatedly(Return(_fm1.get()));
  EXPECT_CALL(*_bfm2, getFareMarket()).WillRepeatedly(Return(_fm2.get()));
  EXPECT_CALL(*_bfm3, getFareMarket()).WillRepeatedly(Return(_fm3.get()));

  // return INDEX_1 for the bfm1, and so on.
  EXPECT_CALL(*_bfm1, getEndSegmentIndex())
    .WillOnce(Return(INDEX_1));
  EXPECT_CALL(*_bfm2, getEndSegmentIndex())
    .WillOnce(Return(INDEX_2));
  EXPECT_CALL(*_bfm3, getEndSegmentIndex())
    .WillOnce(Return(INDEX_3));

  // return INDEX_2 as the next segment for INDEX_1 + 1 (bfm1)
  EXPECT_CALL(*_itinGeometryCalculator,
    getNextTravelSegmentIfCurrentArunk(INDEX_1 + 1)).WillOnce(Return(INDEX_2));
  // return SEG_COUNT (end of travel) as the next segment for INDEX_2 + 1 and
  // INDEX_3 + 1 (bfm2 and bfm3)
  EXPECT_CALL(*_itinGeometryCalculator,
    getNextTravelSegmentIfCurrentArunk(INDEX_2 + 1)).WillOnce(Return(SEG_COUNT));
  EXPECT_CALL(*_itinGeometryCalculator,
    getNextTravelSegmentIfCurrentArunk(INDEX_3 + 1)).WillOnce(Return(SEG_COUNT));

  UnorderedBrandCodes fm1Brands;
  fm1Brands.insert(DUMMY_BRAND_1);
  fm1Brands.insert(DUMMY_BRAND_3);
  UnorderedBrandCodes fm2Brands;
  fm2Brands.insert(DUMMY_BRAND_2);
  fm2Brands.insert(DUMMY_BRAND_3);
  UnorderedBrandCodes fm3Brands;
  fm3Brands.insert(DUMMY_BRAND_1);
  fm3Brands.insert(DUMMY_BRAND_2);
  // return fm1Brands for the first fare market, fm2Brands for the second
  // and fm3Brands for the third
  EXPECT_CALL(*_bfm1, getBrands()).WillOnce(Return(fm1Brands));
  EXPECT_CALL(*_bfm2, getBrands()).WillOnce(Return(fm2Brands));
  EXPECT_CALL(*_bfm3, getBrands()).WillOnce(Return(fm3Brands));

  // bfm2 and bfm3 "saturate" the whole travel - all brands are considered
  // possible
  FareMarketsPerBrandCode expected;
  expected[DUMMY_BRAND_2].insert(_fm2.get());
  expected[DUMMY_BRAND_3].insert(_fm2.get());
  expected[DUMMY_BRAND_1].insert(_fm3.get());
  expected[DUMMY_BRAND_2].insert(_fm3.get());

  // fm1 recursively calls possibleBands() with only DUMMY_BRAND_2 as a
  // possible solution which generates empty intersection
  FareMarketsPerBrandCode singleBrand;
  singleBrand[DUMMY_BRAND_3].insert(_fm3.get());
  // add this to the response with both fm1 (as a caller) and fm3 (returned
  // from possibleBrands call)
  expected[DUMMY_BRAND_3].insert(_fm3.get());
  expected[DUMMY_BRAND_3].insert(_fm1.get());

  EXPECT_CALL(*_mockFMParityCalculator, possibleBrands(INDEX_2, SEG_COUNT))
    .WillOnce(Return(singleBrand));

  // call feeds the cache
  ASSERT_EQ(expected, _testedFMParityCalculator->possibleBrands(INDEX_0, SEG_COUNT));

  // call using the cache - no additional functions called
  ASSERT_EQ(expected, _testedFMParityCalculator->possibleBrands(INDEX_0, SEG_COUNT));
}

TEST_F(FareMarketsParityCalculatorTest, testCorrectlyReturnsPossibleBrandsWithAndWithoutCacheWithSetEndSegment)
{
  _bfm1 = new MockBrandedFareMarket();
  _bfm2 = new MockBrandedFareMarket();
  _bfm3 = new MockBrandedFareMarket();

  addFareMarket(*_fm1, _bfm1);
  addFareMarket(*_fm2, _bfm2);
  addFareMarket(*_fm3, _bfm3);

  EXPECT_CALL(*_bfm1, getFareMarket()).WillRepeatedly(Return(_fm1.get()));
  EXPECT_CALL(*_bfm2, getFareMarket()).WillRepeatedly(Return(_fm2.get()));
  EXPECT_CALL(*_bfm3, getFareMarket()).WillRepeatedly(Return(_fm3.get()));

  // return INDEX_1 for the bfm1, and so on.
  EXPECT_CALL(*_bfm1, getEndSegmentIndex())
    .WillOnce(Return(INDEX_1));
  EXPECT_CALL(*_bfm2, getEndSegmentIndex())
    .WillOnce(Return(INDEX_2));
  EXPECT_CALL(*_bfm3, getEndSegmentIndex())
    .WillOnce(Return(INDEX_3));

  // return INDEX_2 as the next segment for INDEX_1 + 1 (bfm1)
  EXPECT_CALL(*_itinGeometryCalculator,
    getNextTravelSegmentIfCurrentArunk(INDEX_1 + 1)).WillOnce(Return(INDEX_2));
  // return SEG_COUNT (end of travel) as the next segment for INDEX_2 + 1 and
  // INDEX_3 + 1 (bfm2 and bfm3)
  EXPECT_CALL(*_itinGeometryCalculator,
    getNextTravelSegmentIfCurrentArunk(INDEX_2 + 1)).WillOnce(Return(SEG_COUNT));
  EXPECT_CALL(*_itinGeometryCalculator,
    getNextTravelSegmentIfCurrentArunk(INDEX_3 + 1)).WillOnce(Return(SEG_COUNT));

  UnorderedBrandCodes fm1Brands;
  fm1Brands.insert(DUMMY_BRAND_1);
  fm1Brands.insert(DUMMY_BRAND_3);
  UnorderedBrandCodes fm2Brands;
  fm2Brands.insert(DUMMY_BRAND_2);
  fm2Brands.insert(DUMMY_BRAND_3);
  UnorderedBrandCodes fm3Brands;
  fm3Brands.insert(DUMMY_BRAND_1);
  fm3Brands.insert(DUMMY_BRAND_2);
  // return fm1Brands for the first fare market, fm2Brands for the second
  // and fm3Brands for the third
  EXPECT_CALL(*_bfm1, getBrands()).WillOnce(Return(fm1Brands));
  EXPECT_CALL(*_bfm2, getBrands()).WillOnce(Return(fm2Brands));
  EXPECT_CALL(*_bfm3, getBrands()).WillOnce(Return(fm3Brands));

  // bfm2 and bfm3 will "saturate" the whole travel - all brands are considered
  // possible
  FareMarketsPerBrandCode expected;
  expected[DUMMY_BRAND_2].insert(_fm2.get());
  expected[DUMMY_BRAND_3].insert(_fm2.get());
  expected[DUMMY_BRAND_1].insert(_fm3.get());
  expected[DUMMY_BRAND_2].insert(_fm3.get());

  // also bfm1 will "saturate" the whole travel as we request INDEX_2 as the end
  // of the travel
  expected[DUMMY_BRAND_1].insert(_fm1.get());
  expected[DUMMY_BRAND_3].insert(_fm1.get());

  // call feeds the cache
  ASSERT_EQ(expected, _testedFMParityCalculator->possibleBrands(INDEX_0, INDEX_2));

  // call using the cache - no additional functions called
  ASSERT_EQ(expected, _testedFMParityCalculator->possibleBrands(INDEX_0, INDEX_2));
}

} // namespace skipper

} // namespace tse
