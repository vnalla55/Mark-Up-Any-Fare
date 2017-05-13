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
#include "test/include/TestConfigInitializer.h"

#include "Common/TNBrands/ItinBranding.h"

#include "Common/CabinType.h"
#include "Common/Logger.h"
#include "Common/TNBrands/test/TNBrandsMocks.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "Pricing/FareMarketPathMatrix.h"

#include <memory>
#include <vector>

using namespace ::testing;

namespace tse
{
namespace skipper
{

class ItinBrandingTest: public Test
{
public:
  typedef ItinBrandingTemplate<MockItinGeometryCalculator<FareMarket>,
                               MockTNBrandsFunctions,
                               MockBrandingOptionSpacesCalculator,
                               MockFareMarketsParityCalculatorFactory>
    TestedItinBranding;

  TestConfigInitializer _cfg;

  void SetUp()
  {
    _itinGeoCalculator = new MockItinGeometryCalculator<FareMarket>();
    _functions = new MockTNBrandsFunctions();
    _spaceCalculator = new MockBrandingOptionSpacesCalculator();
    _fareMarketParityCalculatorFactory = new MockFareMarketsParityCalculatorFactory();
    _parityCalculator = new MockFareMarketsParityCalculatorFactory::Type();

    _itinBranding.reset(new TestedItinBranding(
      _itinGeoCalculator, _functions, _spaceCalculator, _fareMarketParityCalculatorFactory));

    _trxGeoCalculator.reset(new MockITrxGeometryCalculator());
    _logger.reset(new Logger("DUMMY TEST LOGGER"));
  }

  void TearDown(){}

  SegmentOrientedBrandCodesPerCarrier createDummyBrandInfo()
  {
    SegmentOrientedBrandCodesPerCarrier dummyBrandInfo;
    BrandCodesPerCarrier tmp;
    tmp[CarrierDirection(DUMMY_CARRIER, Direction::BOTHWAYS)];
    dummyBrandInfo.push_back(tmp);
    return dummyBrandInfo;
  }

  SegmentOrientedBrandCodeArraysPerCarrier createDummyBrandInfoArrays()
  {
    SegmentOrientedBrandCodeArraysPerCarrier dummyBrandInfoArrays;
    BrandCodeArraysPerCarrier tmp;
    tmp[CarrierDirection(DUMMY_CARRIER, Direction::BOTHWAYS)];
    dummyBrandInfoArrays.push_back(tmp);
    return dummyBrandInfoArrays;
  }

  BrandingOptionSpaces createDummySpaces(bool withReserved = false)
  {
    CarrierBrandPairs tmp;
    tmp[CarrierDirection(DUMMY_CARRIER, Direction::BOTHWAYS)] = DUMMY_BRAND_1;
    BrandingOptionSpace tmp2;
    tmp2.push_back(tmp);
    BrandingOptionSpaces result;
    if (withReserved)
    {
      result.push_back(BrandingOptionSpace());
    }
    result.push_back(tmp2);
    return result;
  }

  BrandingOptionSpaces createNonTrivialDummySpaces()
  {
    CarrierBrandPairs cbp;
    BrandingOptionSpace pos;
    BrandingOptionSpaces result;

    // Space 1
    // segment 1
    cbp[CarrierDirection("AA", Direction::BOTHWAYS)] = "LL";
    cbp[CarrierDirection("US", Direction::ORIGINAL)] = "WC";
    pos.push_back(cbp);
    // segment 2
    cbp.clear();
    cbp[CarrierDirection("AA", Direction::BOTHWAYS)] = "LL";
    pos.push_back(cbp);
    result.push_back(pos);

    // Space 2
    pos.clear();
    // segment 1
    cbp[CarrierDirection("AA", Direction::BOTHWAYS)] = "NN";
    cbp[CarrierDirection("US", Direction::ORIGINAL)] = "MO";
    pos.push_back(cbp);
    // segment 2
    cbp.clear();
    cbp[CarrierDirection("AA", Direction::ORIGINAL)] = "NN";
    cbp[CarrierDirection("AA", Direction::REVERSED)] = "SS";
    pos.push_back(cbp);
    result.push_back(pos);

    return result;
  }

  void setUpBrandingOptionSpace(BrandingOptionSpace& space)
  {
    CarrierBrandPairs brandsOnSegment1, brandsOnSegment2, brandsOnSegment3,
      brandsOnSegment4, brandsOnSegment5, brandsOnSegment6, brandsOnSegment7;

    // same brand multiple times on one segment -> only one should be displayed
    brandsOnSegment1[CarrierDirection("VA", Direction::BOTHWAYS)] = NO_BRAND;
    brandsOnSegment1[CarrierDirection("LH", Direction::BOTHWAYS)] = NO_BRAND;
    brandsOnSegment1[CarrierDirection("LO", Direction::BOTHWAYS)] = NO_BRAND;

    // SV appears twice. It should be deduplicated. FL should stay
    brandsOnSegment2[CarrierDirection("VA", Direction::BOTHWAYS)] = "SV";
    brandsOnSegment2[CarrierDirection("LH", Direction::BOTHWAYS)] = "SV";
    brandsOnSegment2[CarrierDirection("LO", Direction::BOTHWAYS)] = "FL";

    // same brands as on previous segment (FL+SV). Should not be displayed
    brandsOnSegment3[CarrierDirection("LH", Direction::BOTHWAYS)] = "FL";
    brandsOnSegment3[CarrierDirection("VA", Direction::BOTHWAYS)] = "SV";

    // same brand (BZ) appears twice but is separated by another brand. Should be deduplicated.
    brandsOnSegment4[CarrierDirection("VA", Direction::BOTHWAYS)] = "BZ";
    brandsOnSegment4[CarrierDirection("LH", Direction::BOTHWAYS)] = "FL";
    brandsOnSegment4[CarrierDirection("LO", Direction::BOTHWAYS)] = "BZ";

    // only one brand. Should be displayed without brackets
    brandsOnSegment5[CarrierDirection("LO", Direction::BOTHWAYS)] = "PE";

    // same brand with different directions. Both should stay
    brandsOnSegment6[CarrierDirection("LO", Direction::ORIGINAL)] = "SB";
    brandsOnSegment6[CarrierDirection("LO", Direction::REVERSED)] = "SB";

    // same brand with different directions and carriers. Both should stay
    brandsOnSegment7[CarrierDirection("VA", Direction::ORIGINAL)] = "SV";
    brandsOnSegment7[CarrierDirection("LH", Direction::REVERSED)] = "SV";

    space.push_back(brandsOnSegment1);
    space.push_back(brandsOnSegment2);
    space.push_back(brandsOnSegment3);
    space.push_back(brandsOnSegment4);
    space.push_back(brandsOnSegment5);
    space.push_back(brandsOnSegment6);
    space.push_back(brandsOnSegment7);
  }

  void precalculateOptionSpaces(const BrandingOptionSpaces& finalResult)
  {
    const SegmentOrientedBrandCodesPerCarrier dummyBrandInfo =
      createDummyBrandInfo();

    SegmentOrientedBrandCodesPerCarrierInCabin brandsPerCarrierByCabin;

    PricingTrx trx;

    EXPECT_CALL(*_itinGeoCalculator, getTrxGeometryCalculator())
      .WillRepeatedly(ReturnRef(*_trxGeoCalculator));

    EXPECT_CALL(*_functions,
                calculateSegmentOrientedBrandCodesPerCarrier(_, _, _))
      .WillOnce(SetArgReferee<2>(dummyBrandInfo));

    std::vector<QualifiedBrand> qualifiedBrands;
    EXPECT_CALL(*_trxGeoCalculator, getQualifiedBrands())
      .WillOnce(ReturnRef(qualifiedBrands));

    SegmentOrientedBrandCodeArraysPerCarrier dummyBrandInfoArrays =
      createDummyBrandInfoArrays();

    EXPECT_CALL(*_functions, sortSegmentOrientedBrandCodesPerCarrier(dummyBrandInfo, _, _))
      .WillOnce(SetArgReferee<2>(dummyBrandInfoArrays));

    EXPECT_CALL(*_functions, fillEmptyBrandsArraysPerCarrier(dummyBrandInfoArrays))
      .WillOnce(SetArgReferee<0>(dummyBrandInfoArrays));

    EXPECT_CALL(*_trxGeoCalculator, getTrx())
      .WillRepeatedly(ReturnRef(trx));

    EXPECT_CALL(*_spaceCalculator, calculateSpaces(dummyBrandInfoArrays, _, _))
      .WillOnce(SetArgReferee<1>(finalResult));

    EXPECT_CALL(*_functions, updateReservedBrandingOptionSpace(_))
      .WillOnce(SetArgReferee<0>(finalResult));

    _itinBranding->calculateBrandingOptionSpaces(0, brandsPerCarrierByCabin, 0, *_logger, 0);
  }

  void precalculateOptionSpacesByCabin(const BrandingOptionSpacesPerCabin& finalResult,
    bool limit)
  {
    // limit will reduce number of required spaces to 5

    const SegmentOrientedBrandCodesPerCarrier dummyBrandInfo =
      createDummyBrandInfo();

    const size_t economyClass = CabinType::generalIndex(CabinType::ECONOMY_CLASS);
    const size_t businessClass = CabinType::generalIndex(CabinType::BUSINESS_CLASS);
    const size_t firstClass = CabinType::generalIndex(CabinType::FIRST_CLASS);
    const size_t unknownClass = CabinType::generalIndex(CabinType::UNKNOWN_CLASS);

    SegmentOrientedBrandCodesPerCarrierInCabin brandsPerCarrierByCabin;
    brandsPerCarrierByCabin[economyClass] = createDummyBrandInfo();
    brandsPerCarrierByCabin[businessClass] = createDummyBrandInfo();
    brandsPerCarrierByCabin[firstClass] = createDummyBrandInfo();
    brandsPerCarrierByCabin[unknownClass] = createDummyBrandInfo();

    PricingTrx trx;

    EXPECT_CALL(*_itinGeoCalculator, getTrxGeometryCalculator())
      .WillRepeatedly(ReturnRef(*_trxGeoCalculator));

    EXPECT_CALL(*_trxGeoCalculator, getTrx())
      .WillRepeatedly(ReturnRef(trx));

    EXPECT_CALL(*_functions,
                calculateSegmentOrientedBrandCodesPerCarrier(_, _, _))
      .WillOnce(SetArgReferee<2>(dummyBrandInfo));

    std::vector<QualifiedBrand> qualifiedBrands;
    EXPECT_CALL(*_trxGeoCalculator, getQualifiedBrands())
      .WillOnce(ReturnRef(qualifiedBrands));

    SegmentOrientedBrandCodeArraysPerCarrier dummyBrandInfoArrays =
      createDummyBrandInfoArrays();

    EXPECT_CALL(*_functions, sortSegmentOrientedBrandCodesPerCarrier(dummyBrandInfo, _, _))
      .WillOnce(SetArgReferee<2>(dummyBrandInfoArrays));

    EXPECT_CALL(*_functions, filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
        Ref(brandsPerCarrierByCabin[economyClass]), _, _, _, _))
      .WillOnce(DoAll(SetArgReferee<3>(dummyBrandInfoArrays), Return(true)));

    EXPECT_CALL(*_functions, filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
        Ref(brandsPerCarrierByCabin[businessClass]), _, _, _, _))
      .WillOnce(DoAll(SetArgReferee<3>(dummyBrandInfoArrays), Return(true)));

    if (!limit)
    {
      // with limit on eco returns 2 spaces, business returns 2 spaces and
      // first returns 2 spaces which exceeds limit of 5. as a result this
      // call won't be made
      EXPECT_CALL(*_functions, filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
          Ref(brandsPerCarrierByCabin[firstClass]), _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(dummyBrandInfoArrays), Return(true)));
    }

    size_t times = limit ? 2 : 3; // three times without limit, two with limit (no first class)
    EXPECT_CALL(*_functions, fillEmptyBrandsArraysPerCarrier(dummyBrandInfoArrays))
      .Times(times).WillRepeatedly(SetArgReferee<0>(dummyBrandInfoArrays));

    testing::Sequence s1;
    EXPECT_CALL(*_functions, calculateMaxBrandsCountPerCarrier(dummyBrandInfoArrays))
      .InSequence(s1).WillOnce(Return(finalResult.at(economyClass).size()));
    EXPECT_CALL(*_spaceCalculator, calculateSpaces(dummyBrandInfoArrays, _, 0, true))
      .InSequence(s1).WillOnce(SetArgReferee<1>(finalResult.at(economyClass)));
    EXPECT_CALL(*_functions, calculateMaxBrandsCountPerCarrier(dummyBrandInfoArrays))
       .InSequence(s1).WillOnce(Return(finalResult.at(businessClass).size()));
    EXPECT_CALL(*_spaceCalculator, calculateSpaces(dummyBrandInfoArrays, _, 0, false))
      .InSequence(s1).WillOnce(SetArgReferee<1>(finalResult.at(businessClass)));
    if (!limit)
    {
      EXPECT_CALL(*_functions, calculateMaxBrandsCountPerCarrier(dummyBrandInfoArrays))
         .InSequence(s1).WillOnce(Return(finalResult.at(firstClass).size()));
      EXPECT_CALL(*_spaceCalculator, calculateSpaces(dummyBrandInfoArrays, _, 0, false))
        .InSequence(s1).WillOnce(SetArgReferee<1>(finalResult.at(firstClass)));
    }

    EXPECT_CALL(*_functions, updateReservedBrandingOptionSpace(_))
      .Times(1).WillRepeatedly(SetArgReferee<0>(finalResult.at(economyClass)));

    // if limit, request only 3 spaces (eco cheapest, eco, business)
    // otherwise return 4 (eco cheapest, eco, business, first)
    _itinBranding->calculateBrandingOptionSpaces(0, brandsPerCarrierByCabin,
        limit ? 3 : 0, *_logger, 0);
  }

  void precalculateBrandParity(const std::vector<FareMarket*>& fareMarkets)
  {
    //from my experience Eq(ByRef()) in EXPECT_CALL works only with sequence
    //defined
    InSequence s;

    EXPECT_CALL(*_fareMarketParityCalculatorFactory, create(_))
      .WillOnce(Return(_parityCalculator));

    EXPECT_CALL(*_itinGeoCalculator, getFareMarkets())
      .WillOnce(ReturnRef(fareMarkets));

    EXPECT_CALL(*_parityCalculator, addFareMarket(Eq(ByRef(*fareMarkets[0]))));
    EXPECT_CALL(*_parityCalculator, addFareMarket(Eq(ByRef(*fareMarkets[1]))));

    const size_t START_SEGMENT = 0;
    EXPECT_CALL(*_itinGeoCalculator, getNextTravelSegmentIfCurrentArunk(START_SEGMENT))
      .WillOnce(Return(START_SEGMENT));

    FareMarketsPerBrandCode brandFMRelation;
    brandFMRelation[DUMMY_BRAND_1].insert(fareMarkets[0]);
    brandFMRelation[DUMMY_BRAND_2].insert(fareMarkets[1]);

    EXPECT_CALL(*_parityCalculator, possibleBrands(START_SEGMENT))
      .WillOnce(Return(brandFMRelation));

    _itinBranding->calculateBrandParity();
  }

  void precalculateBrandParityForNonFixedLegs(const std::vector<FareMarket*>& fareMarkets)
  {
    //from my experience Eq(ByRef()) in EXPECT_CALL works only with sequence
    //defined
    InSequence s;

    EXPECT_CALL(*_itinGeoCalculator, reduceFareMarketsBrandsOnFixedLegs());

    EXPECT_CALL(*_fareMarketParityCalculatorFactory, create(_))
      .WillOnce(Return(_parityCalculator));

    EXPECT_CALL(*_itinGeoCalculator, getFareMarkets())
      .WillOnce(ReturnRef(fareMarkets));

    EXPECT_CALL(*_parityCalculator, addFareMarket(Eq(ByRef(*fareMarkets[0]))));
    EXPECT_CALL(*_parityCalculator, addFareMarket(Eq(ByRef(*fareMarkets[1]))));
    EXPECT_CALL(*_parityCalculator, addFareMarket(Eq(ByRef(*fareMarkets[2]))));

    const size_t START_SEGMENT = 1;
    const size_t END_SEGMENT = 2;
    EXPECT_CALL(*_itinGeoCalculator, calculateNonFixedSegmentsForContextShopping())
      .WillOnce(Return(std::make_pair(START_SEGMENT, END_SEGMENT)));

    EXPECT_CALL(*_itinGeoCalculator, getNextTravelSegmentIfCurrentArunk(START_SEGMENT))
      .WillOnce(Return(START_SEGMENT));

    FareMarketsPerBrandCode brandFMRelation;
    brandFMRelation[DUMMY_BRAND_1].insert(fareMarkets[0]);
    brandFMRelation[DUMMY_BRAND_2].insert(fareMarkets[1]);

    EXPECT_CALL(*_parityCalculator, possibleBrands(START_SEGMENT, END_SEGMENT))
      .WillOnce(Return(brandFMRelation));

    BrandFilterMap filter;
    EXPECT_CALL(*_itinGeoCalculator, getBrandFilterMap())
      .WillOnce(ReturnRef(filter));

    EXPECT_CALL(*_itinGeoCalculator, getFareMarkets())
      .WillOnce(ReturnRef(fareMarkets));

    BrandCode returnBrand = DUMMY_BRAND_1;
    std::vector<bool> fixedLegs;
    fixedLegs.push_back(false);

    std::map<uint16_t, UnorderedBrandCodes> segmentsBrands;

    EXPECT_CALL(*_itinGeoCalculator, isFareMarketOnFixedLeg(Eq(ByRef(*fareMarkets[0])), _))
      .WillOnce(Return(false));
    EXPECT_CALL(*_itinGeoCalculator, isFareMarketOnFixedLeg(Eq(ByRef(*fareMarkets[1])), _))
      .WillOnce(Return(false));
    EXPECT_CALL(*_itinGeoCalculator, isFareMarketOnFixedLeg(Eq(ByRef(*fareMarkets[2])), _))
      .WillOnce(DoAll(SetArgReferee<1>(returnBrand), Return(true)));

    EXPECT_CALL(*_itinGeoCalculator, getTrxGeometryCalculator())
      .WillOnce(ReturnRef(*_trxGeoCalculator));
    EXPECT_CALL(*_trxGeoCalculator, getFixedLegs())
      .WillRepeatedly(ReturnRef(fixedLegs));
    EXPECT_CALL(*_functions, isAnySegmentOnNonFixedLeg(_, _))
      .WillOnce(Return(false));

    _itinBranding->calculateBrandParityForNonFixedLegs();
  }

  MockItinGeometryCalculator<FareMarket>* _itinGeoCalculator;
  MockTNBrandsFunctions* _functions;
  MockBrandingOptionSpacesCalculator* _spaceCalculator;
  MockFareMarketsParityCalculatorFactory* _fareMarketParityCalculatorFactory;
  MockFareMarketsParityCalculatorFactory::Type* _parityCalculator;

  std::shared_ptr<TestedItinBranding> _itinBranding;
  std::shared_ptr<MockITrxGeometryCalculator> _trxGeoCalculator;
  std::shared_ptr<Logger> _logger;

  static const CarrierCode DUMMY_CARRIER;
  static const BrandCode DUMMY_BRAND_1;
  static const BrandCode DUMMY_BRAND_2;
  static const BrandCode DUMMY_BRAND_3;
};

const CarrierCode ItinBrandingTest::DUMMY_CARRIER = "ABC";
const BrandCode ItinBrandingTest::DUMMY_BRAND_1 = "WHATEVER";
const BrandCode ItinBrandingTest::DUMMY_BRAND_2 = "OTHERBRAND";
const BrandCode ItinBrandingTest::DUMMY_BRAND_3 = "SOMETHINGELSE";


TEST_F(ItinBrandingTest, testIsCarrierValidForSpaceSegment)
{
  const BrandingOptionSpaces finalResult = createNonTrivialDummySpaces();
  precalculateOptionSpaces(finalResult);
  ASSERT_EQ(true, _itinBranding->isCarrierValidForSpaceSegment(1, 0, "AA"));
  ASSERT_EQ(true, _itinBranding->isCarrierValidForSpaceSegment(0, 0, "US"));
  ASSERT_EQ(true, _itinBranding->isCarrierValidForSpaceSegment(0, 1, "AA"));
  ASSERT_EQ(false, _itinBranding->isCarrierValidForSpaceSegment(100, 99, "LO"));
  ASSERT_EQ(false, _itinBranding->isCarrierValidForSpaceSegment(1, 0, "LO"));
  ASSERT_EQ(false, _itinBranding->isCarrierValidForSpaceSegment(1, 99, "AA"));
}


TEST_F(ItinBrandingTest, testCalculatesBrandingOptionSpacesPerCabinCorrectly)
{
  BrandingOptionSpacesPerCabin finalResult;
  finalResult[CabinType::generalIndex(CabinType::ECONOMY_CLASS)] = createDummySpaces(true);
  finalResult[CabinType::generalIndex(CabinType::BUSINESS_CLASS)] = createDummySpaces();
  finalResult[CabinType::generalIndex(CabinType::FIRST_CLASS)] = createDummySpaces();
  precalculateOptionSpacesByCabin(finalResult, false);

  // no limit, all should be available (3 spaces) + one reserved
  ASSERT_EQ(4, _itinBranding->getBrandingOptionSpacesCount());
  // spaces should be from the cheapest cabin
  ASSERT_EQ(finalResult.at(CabinType::generalIndex(CabinType::ECONOMY_CLASS)).at(0),
    _itinBranding->getBrandingOptionSpace(0));
  ASSERT_EQ(CabinType::generalIndex(CabinType::ECONOMY_CLASS),
    _itinBranding->getBrandingOptionSpaceCabin(0));
  ASSERT_EQ(finalResult.at(CabinType::generalIndex(CabinType::ECONOMY_CLASS)).at(1),
    _itinBranding->getBrandingOptionSpace(1));
  ASSERT_EQ(CabinType::generalIndex(CabinType::ECONOMY_CLASS),
    _itinBranding->getBrandingOptionSpaceCabin(1));
  ASSERT_EQ(finalResult.at(CabinType::generalIndex(CabinType::BUSINESS_CLASS)).at(0),
    _itinBranding->getBrandingOptionSpace(2));
  ASSERT_EQ(CabinType::generalIndex(CabinType::BUSINESS_CLASS),
    _itinBranding->getBrandingOptionSpaceCabin(2));
  ASSERT_EQ(finalResult.at(CabinType::generalIndex(CabinType::FIRST_CLASS)).at(0),
    _itinBranding->getBrandingOptionSpace(3));
  ASSERT_EQ(CabinType::generalIndex(CabinType::FIRST_CLASS),
    _itinBranding->getBrandingOptionSpaceCabin(3));

}

TEST_F(ItinBrandingTest, testCalculatesBrandingOptionSpacesPerCabinWithLimitCorrectly)
{
  BrandingOptionSpacesPerCabin finalResult, expected;
  finalResult[CabinType::generalIndex(CabinType::ECONOMY_CLASS)] = createDummySpaces(true);
  finalResult[CabinType::generalIndex(CabinType::BUSINESS_CLASS)] = createDummySpaces();
  finalResult[CabinType::generalIndex(CabinType::FIRST_CLASS)] = createDummySpaces();
  precalculateOptionSpacesByCabin(finalResult, true);

  // last space from first class should be dropped because of limit (equal to 3)
  ASSERT_EQ(3, _itinBranding->getBrandingOptionSpacesCount());
  // spaces should be from the cheapest cabin
  ASSERT_EQ(finalResult.at(CabinType::generalIndex(CabinType::ECONOMY_CLASS)).at(0),
    _itinBranding->getBrandingOptionSpace(0));
  ASSERT_EQ(CabinType::generalIndex(CabinType::ECONOMY_CLASS),
    _itinBranding->getBrandingOptionSpaceCabin(0));
  ASSERT_EQ(finalResult.at(CabinType::generalIndex(CabinType::ECONOMY_CLASS)).at(1),
    _itinBranding->getBrandingOptionSpace(1));
  ASSERT_EQ(CabinType::generalIndex(CabinType::ECONOMY_CLASS),
    _itinBranding->getBrandingOptionSpaceCabin(1));
  ASSERT_EQ(finalResult.at(CabinType::generalIndex(CabinType::BUSINESS_CLASS)).at(0),
    _itinBranding->getBrandingOptionSpace(2));
}

TEST_F(ItinBrandingTest, testRaisesOnBrandingOptionSpaceIndexOutOfRange)
{
  // no spaces available
  ASSERT_THROW(_itinBranding->getBrandingOptionSpace(0), ErrorResponseException);
  ASSERT_THROW(_itinBranding->getBrandingOptionSpaceCabin(0), ErrorResponseException);
}

TEST_F(ItinBrandingTest, testFindsCarriersBrandsForSegment0)
{
  const BrandingOptionSpaces finalResult = createNonTrivialDummySpaces();
  precalculateOptionSpaces(finalResult);

  EXPECT_CALL(*_itinGeoCalculator, getTravelSegmentIndex(_))
    .Times(2)
    .WillRepeatedly(Return(size_t(0)));

  const AirSeg segment;  // Dummy, we don't get to access it in the calls.
  ASSERT_EQ(finalResult[0][0], _itinBranding->getCarriersBrandsForSegment(0, &segment));
  ASSERT_EQ(finalResult[1][0], _itinBranding->getCarriersBrandsForSegment(1, &segment));
}

TEST_F(ItinBrandingTest, testFindsCarriersBrandsForSegment1)
{
  const BrandingOptionSpaces finalResult = createNonTrivialDummySpaces();
  precalculateOptionSpaces(finalResult);

  EXPECT_CALL(*_itinGeoCalculator, getTravelSegmentIndex(_))
    .Times(2)
    .WillRepeatedly(Return(size_t(1)));

  const AirSeg segment;  // Dummy, we don't get to access it in the calls.
  ASSERT_EQ(finalResult[0][1], _itinBranding->getCarriersBrandsForSegment(0, &segment));
  ASSERT_EQ(finalResult[1][1], _itinBranding->getCarriersBrandsForSegment(1, &segment));
}

TEST_F(ItinBrandingTest, testThrowsWhenNoSpaceForCarriersBrandsForSegment)
{
  const BrandingOptionSpaces finalResult = createDummySpaces();
  precalculateOptionSpaces(finalResult);
  const AirSeg segment;  // Dummy, we don't get to access it in the call.
  ASSERT_THROW(_itinBranding->getCarriersBrandsForSegment(1, &segment), ErrorResponseException);
}

TEST_F(ItinBrandingTest, testFindsFmpMatrixWhenPreviouslySet)
{
  PricingTrx trx;
  Itin itin;
  std::vector<MergedFareMarket*> mfmVec;
  FmpMatrixPtr ptr1(new FareMarketPathMatrix(trx, itin, mfmVec));
  FmpMatrixPtr ptr2(new FareMarketPathMatrix(trx, itin, mfmVec));

  _itinBranding->setFmpMatrix(ptr1, 0);
  _itinBranding->setFmpMatrix(ptr2, 1);

  ASSERT_EQ(ptr1, _itinBranding->getFmpMatrix(0));
  ASSERT_EQ(ptr2, _itinBranding->getFmpMatrix(1));
}

TEST_F(ItinBrandingTest, testThrowsWhenFmpMatrixNotSet)
{
  ASSERT_THROW(_itinBranding->getFmpMatrix(0), ErrorResponseException);
}

TEST_F(ItinBrandingTest, testGetBrandCodeWhenSet)
{
  const BrandingOptionSpaces finalResult = createNonTrivialDummySpaces();
  precalculateOptionSpaces(finalResult);
  ASSERT_EQ("NN", _itinBranding->getBrandCode(1, 0, "AA", Direction::BOTHWAYS));
  ASSERT_EQ("WC", _itinBranding->getBrandCode(0, 0, "US", Direction::ORIGINAL));
  ASSERT_EQ("LL", _itinBranding->getBrandCode(0, 1, "AA", Direction::BOTHWAYS));
}

TEST_F(ItinBrandingTest, testThrowIfIndexOutOfRange)
{
  ASSERT_THROW(_itinBranding->getBrandCode(1,1,"US",Direction::BOTHWAYS), ErrorResponseException);
}

TEST_F(ItinBrandingTest, testThrowIfWrongDirection)
{
  ASSERT_THROW(_itinBranding->getBrandCode(0, 0, "US", Direction::BOTHWAYS), ErrorResponseException);
}

TEST_F(ItinBrandingTest, testThrowIfCarrierNotFound)
{
  const BrandingOptionSpaces finalResult = createNonTrivialDummySpaces();
  precalculateOptionSpaces(finalResult);
  ASSERT_THROW(_itinBranding->getBrandCode(1,0,"LO",Direction::BOTHWAYS), ErrorResponseException);
}

TEST_F(ItinBrandingTest, testCalcultesBrandParityAndBrandCodesPerFareMarketCorrectly)
{
  std::vector<FareMarket*> fareMarkets;
  std::unique_ptr<FareMarket> fm1(new FareMarket);
  std::unique_ptr<FareMarket> fm2(new FareMarket);
  fareMarkets.push_back(fm1.get());
  fareMarkets.push_back(fm2.get());
  precalculateBrandParity(fareMarkets);

  UnorderedBrandCodes expectedBrandCodes;
  expectedBrandCodes.insert(DUMMY_BRAND_1);
  expectedBrandCodes.insert(DUMMY_BRAND_2);
  ASSERT_EQ(expectedBrandCodes, _itinBranding->getParityBrands());

  BrandCodesPerFareMarket expectedBrandCodesPerFareMarket;
  expectedBrandCodesPerFareMarket[fm1.get()].insert(DUMMY_BRAND_1);
  expectedBrandCodesPerFareMarket[fm2.get()].insert(DUMMY_BRAND_2);

  ASSERT_EQ(expectedBrandCodesPerFareMarket,
    _itinBranding->getParityBrandsPerFareMarket());
}

TEST_F(ItinBrandingTest, testRaisesOnImproperFareMarketDuringParityCalculation)
{
  EXPECT_CALL(*_fareMarketParityCalculatorFactory, create(_))
    .WillOnce(Return(_parityCalculator));

  std::vector<FareMarket*> fareMarkets;
  fareMarkets.push_back(static_cast<FareMarket*>(0));
  EXPECT_CALL(*_itinGeoCalculator, getFareMarkets())
    .WillOnce(ReturnRef(fareMarkets));

  ASSERT_THROW(_itinBranding->calculateBrandParity(), ErrorResponseException);
}

TEST_F(ItinBrandingTest, testUpdatesProgramsForCalculatesBrandsCorrectly)
{
  std::vector<FareMarket*> fareMarkets;
  std::shared_ptr<FareMarket> fm1;
  fm1.reset(new FareMarket());
  std::shared_ptr<FareMarket> fm2;
  fm2.reset(new FareMarket());
  fareMarkets.push_back(fm1.get());
  fareMarkets.push_back(fm2.get());
  precalculateBrandParity(fareMarkets);

  EXPECT_CALL(*_itinGeoCalculator, getFareMarkets()).WillOnce(ReturnRef(fareMarkets));

  const ProgramID DUMMY_PROGRAM_1 = "1";
  const ProgramID DUMMY_PROGRAM_2 = "2";
  const ProgramID DUMMY_PROGRAM_3 = "3";
  BrandProgramRelations fm1Relations;
  fm1Relations.addBrandProgramPair(DUMMY_BRAND_1, DUMMY_PROGRAM_1, 1);
  fm1Relations.addBrandProgramPair(DUMMY_BRAND_1, DUMMY_PROGRAM_2, 2);
  fm1Relations.addBrandProgramPair(DUMMY_BRAND_3, DUMMY_PROGRAM_3, 3);
  BrandProgramRelations fm2Relations;
  fm2Relations.addBrandProgramPair(DUMMY_BRAND_2, DUMMY_PROGRAM_2, 4);
  fm2Relations.addBrandProgramPair(DUMMY_BRAND_2, DUMMY_PROGRAM_3, 5);
  fm2Relations.addBrandProgramPair(DUMMY_BRAND_3, DUMMY_PROGRAM_3, 3);

  PricingRequest request;
  PricingTrx trx;
  trx.setRequest(&request);
  MockITrxGeometryCalculator trxCalculator;

  EXPECT_CALL(*_itinGeoCalculator, getTrxGeometryCalculator())
    .WillRepeatedly(ReturnRef(trxCalculator));
  EXPECT_CALL(trxCalculator, getTrx()).WillOnce(ReturnRef(trx));

  EXPECT_CALL(trxCalculator, getBrandsAndPrograms(_, _))
    .WillOnce(SetArgReferee<1>(fm1Relations)).WillOnce(SetArgReferee<1>(fm2Relations));

  EXPECT_CALL(*_itinGeoCalculator, addBrandProgramPair(DUMMY_BRAND_1, DUMMY_PROGRAM_1));
  EXPECT_CALL(*_itinGeoCalculator, addBrandProgramPair(DUMMY_BRAND_1, DUMMY_PROGRAM_2));
  EXPECT_CALL(*_itinGeoCalculator, addBrandProgramPair(DUMMY_BRAND_2, DUMMY_PROGRAM_2));
  EXPECT_CALL(*_itinGeoCalculator, addBrandProgramPair(DUMMY_BRAND_2, DUMMY_PROGRAM_3));

  _itinBranding->updateProgramsForCalculatedBrands();
}

TEST_F(ItinBrandingTest, testRaisesOnImproperFareMarketDuringProagramsUpdate)
{
  std::vector<FareMarket*> fareMarkets;
  fareMarkets.push_back(static_cast<FareMarket*>(0));

  PricingRequest request;
  PricingTrx trx;
  trx.setRequest(&request);
  MockITrxGeometryCalculator trxCalculator;

  EXPECT_CALL(*_itinGeoCalculator, getTrxGeometryCalculator())
    .WillRepeatedly(ReturnRef(trxCalculator));
  EXPECT_CALL(trxCalculator, getTrx()).WillOnce(ReturnRef(trx));
  EXPECT_CALL(*_itinGeoCalculator, getFareMarkets())
    .WillOnce(ReturnRef(fareMarkets));

  ASSERT_THROW(_itinBranding->updateProgramsForCalculatedBrands(), ErrorResponseException);
}

TEST_F(ItinBrandingTest, testReturnsIsThruFareMarketCorrectly)
{
  FareMarket fm1;
  FareMarket fm2;
  EXPECT_CALL(*_itinGeoCalculator, isThruFareMarket(Eq(ByRef(fm1))))
    .WillOnce(Return(true));

  ASSERT_TRUE(_itinBranding->isThruFareMarket(fm1));
}

TEST_F(ItinBrandingTest, testReturnsQualifiedBrandIndicesForCarriersBrandCorrectly)
{
  QualifiedBrandIndices expected;
  expected.insert(0);

  EXPECT_CALL(*_itinGeoCalculator, getQualifiedBrandIndicesForCarriersBrand(
    Ref(DUMMY_CARRIER), Ref(DUMMY_BRAND_1))).WillRepeatedly(ReturnRef(expected));

  ASSERT_EQ(expected,
      _itinBranding->getQualifiedBrandIndicesForCarriersBrand(DUMMY_CARRIER, DUMMY_BRAND_1));
}

TEST_F(ItinBrandingTest, testGetsFareMarketStartEndSegmentsCorrectly)
{
  FareMarket fm;
  const size_t START = 0;
  const size_t END = 1;

  EXPECT_CALL(*_itinGeoCalculator, getFareMarketStartSegmentIndex(Eq(ByRef(fm))))
    .WillOnce(Return(START));
  EXPECT_CALL(*_itinGeoCalculator, getFareMarketEndSegmentIndex(Eq(ByRef(fm))))
    .WillOnce(Return(END));

  std::pair<size_t, size_t> expected;
  expected.first = START;
  expected.second = END;
  ASSERT_EQ(expected, _itinBranding->getFareMarketStartEndSegments(fm));
}

TEST_F(ItinBrandingTest, testGetsTravelSegmentLegIdCorrectly)
{
  const size_t SEGMENT = 5;
  const size_t LEG = 7;

  EXPECT_CALL(*_itinGeoCalculator, getTravelSegmentLegId(SEGMENT))
    .WillOnce(Return(LEG));

  ASSERT_EQ(LEG, _itinBranding->getTravelSegmentLegId(SEGMENT));
}

TEST_F(ItinBrandingTest, testSetsFmpMatrixPtrToSpaceRelationCorrectly)
{
  PricingTrx trx;
  Itin itin;
  std::vector<MergedFareMarket*> mfmVec;
  FmpMatrixPtr ptr1(new FareMarketPathMatrix(trx, itin, mfmVec));
  FmpMatrixPtr ptr2(new FareMarketPathMatrix(trx, itin, mfmVec));

  _itinBranding->setFmpMatrix(ptr1, 0);
  _itinBranding->setFmpMatrix(ptr2, 1);

  ASSERT_EQ(0, _itinBranding->getBrandingSpaceIndex(ptr1));
  ASSERT_EQ(1, _itinBranding->getBrandingSpaceIndex(ptr2));
}

TEST_F(ItinBrandingTest, testThrowsOnUnknownFmpMatrixPtr)
{
  PricingTrx trx;
  Itin itin;
  std::vector<MergedFareMarket*> mfmVec;
  FmpMatrixPtr ptr1(new FareMarketPathMatrix(trx, itin, mfmVec));

  ASSERT_THROW(_itinBranding->getBrandingSpaceIndex(ptr1), ErrorResponseException);
}

TEST_F(ItinBrandingTest, testCalcultesBrandParityAndBrandCodesPerFareMarketForNonFixedLegsCorrectly)
{
  std::vector<FareMarket*> fareMarkets;
  std::unique_ptr<FareMarket> fm1(new FareMarket);
  std::unique_ptr<FareMarket> fm2(new FareMarket);
  std::unique_ptr<FareMarket> fm3(new FareMarket);

  std::vector<int> brands;
  brands.push_back(0);
  fm1->brandProgramIndexVec() = brands;
  fm2->brandProgramIndexVec() = brands;

  fareMarkets.push_back(fm1.get());
  fareMarkets.push_back(fm2.get());
  fareMarkets.push_back(fm3.get());

  precalculateBrandParityForNonFixedLegs(fareMarkets);

  UnorderedBrandCodes expectedBrandCodes;
  expectedBrandCodes.insert(DUMMY_BRAND_1);
  expectedBrandCodes.insert(DUMMY_BRAND_2);
  ASSERT_EQ(expectedBrandCodes, _itinBranding->getParityBrands());

  BrandCodesPerFareMarket expectedBrandCodesPerFareMarket;
  expectedBrandCodesPerFareMarket[fm1.get()].insert(DUMMY_BRAND_1);
  expectedBrandCodesPerFareMarket[fm2.get()].insert(DUMMY_BRAND_2);
  expectedBrandCodesPerFareMarket[fm3.get()].insert(DUMMY_BRAND_1);

  ASSERT_EQ(expectedBrandCodesPerFareMarket,
    _itinBranding->getParityBrandsPerFareMarket());
}

TEST_F(ItinBrandingTest, testBrandsFromSpaceToString)
{
  BrandingOptionSpace space;
  std::string expected = "_NO_BRAND_(B)-{FL(B),SV(B)}-{BZ(B),FL(B)}-PE(B)-{SB(O),SB(R)}-{SV(O),SV(R)}";

  setUpBrandingOptionSpace(space);

  ASSERT_EQ(expected, _itinBranding->brandsFromSpaceToString(space));
}

TEST_F(ItinBrandingTest, testGetBrandCodeForMarket)
{
  const size_t SEGMENTS_NUM = 5;
  AirSeg segs[SEGMENTS_NUM];
  FareMarket fareMarket;

  fareMarket.governingCarrier() = "LH";

  for (size_t cnt = 0; cnt < SEGMENTS_NUM-1; cnt++)
  {
    fareMarket.travelSeg().push_back(&(segs[cnt]));
  }

  const size_t START = 1;

  EXPECT_CALL(*_itinGeoCalculator, getFareMarketStartSegmentIndex(Eq(ByRef(fareMarket))))
    .WillRepeatedly(Return(START));

  std::map<Direction, BrandCode> expected = { {Direction::BOTHWAYS, "SV"} };
  BrandingOptionSpace space;

  setUpBrandingOptionSpace(space);

  ASSERT_EQ(expected, _itinBranding->getBrandCodeForMarket(fareMarket, space));
}

} // namespace skipper

} // namespace tse

