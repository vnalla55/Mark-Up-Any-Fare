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
#include "DataModel/AirSeg.h"
#include "DataModel/FareComponentShoppingContext.h"
#include "Common/TNBrands/ItinGeometryCalculator.h"
#include "Common/TNBrands/test/TNBrandsMocks.h"
#include "DataModel/ArunkSeg.h"

#include <memory>
#include <string>
#include <vector>

using namespace ::testing;
using namespace boost;

namespace tse
{

namespace skipper
{

class ItinGeometryCalculatorTest: public Test
{
public:
  typedef ItinGeometryCalculatorTemplate<MockBrandInItinInclusionPolicy>
    TestedItinGeometryCalculator;

  void SetUp()
  {
    _itin.reset(createItin());
    _inclusionPolicy = new MockBrandInItinInclusionPolicy();
    _trxCalculator.reset(new MockITrxGeometryCalculator());
    _itinCalculator.reset(new TestedItinGeometryCalculator(
        *_itin, *_trxCalculator, _inclusionPolicy));
    _relations.reset(new BrandProgramRelations());
  }

  void TearDown(){}

  Itin* createItin()
  {
    Itin* itin = new Itin();

    _seg1.reset(new AirSeg);
    _seg2.reset(new AirSeg);
    _seg3.reset(new AirSeg);

    _fm1.reset(new FareMarket);
    _fm1->travelSeg().push_back(_seg1.get());
    _fm1->travelSeg().push_back(_seg2.get());

    _fm2.reset(new FareMarket);
    _fm2->travelSeg().push_back(_seg2.get());
    _fm2->travelSeg().push_back(_seg3.get());

    itin->fareMarket().push_back(_fm1.get());
    itin->fareMarket().push_back(_fm2.get());

    itin->travelSeg().push_back(_seg1.get());
    itin->travelSeg().push_back(_seg2.get());
    itin->travelSeg().push_back(_seg3.get());
    itin->travelSeg()[0]->legId() = 0;
    itin->travelSeg()[1]->legId() = 0;
    itin->travelSeg()[2]->legId() = 0;
    itin->travelSeg()[0]->pnrSegment() = 1;
    itin->travelSeg()[1]->pnrSegment() = 2;
    itin->travelSeg()[2]->pnrSegment() = 3;

    return itin;
  }

  MockBrandInItinInclusionPolicy* _inclusionPolicy;
  std::shared_ptr<MockITrxGeometryCalculator> _trxCalculator;
  std::shared_ptr<TestedItinGeometryCalculator> _itinCalculator;
  std::shared_ptr<Itin> _itin;
  std::shared_ptr<FareMarket> _fm1;
  std::shared_ptr<FareMarket> _fm2;
  std::shared_ptr<FareMarket> _fm3;
  std::shared_ptr<AirSeg> _seg1;
  std::shared_ptr<AirSeg> _seg2;
  std::shared_ptr<AirSeg> _seg3;
  std::shared_ptr<BrandProgramRelations> _relations;
  static const BrandCode DUMMY_BRAND;
  static const BrandCode ANOTHER_DUMMY_BRAND;
};

const BrandCode ItinGeometryCalculatorTest::DUMMY_BRAND = "ORANGE";
const BrandCode ItinGeometryCalculatorTest::ANOTHER_DUMMY_BRAND = "APPLE";

TEST_F(ItinGeometryCalculatorTest, testReturnsCorrectNumberOfSegments)
{
  // We added three segments in the test class.
  ASSERT_EQ(3, _itinCalculator->getSegmentCount());
}

TEST_F(ItinGeometryCalculatorTest, testReturnsFareMarketsProperly)
{
  std::vector<FareMarket*> expected;
  expected.push_back(_fm1.get());
  expected.push_back(_fm2.get());

  ASSERT_EQ(expected, _itinCalculator->getFareMarkets());
}

TEST_F(ItinGeometryCalculatorTest, testReturnsSegmentsIndexInItin)
{
  // segments 1, 2, 3 have indices 0, 1, 2 accordingly
  ASSERT_EQ(0, _itinCalculator->getTravelSegmentIndex(_seg1.get()));
  ASSERT_EQ(1, _itinCalculator->getTravelSegmentIndex(_seg2.get()));
  ASSERT_EQ(2, _itinCalculator->getTravelSegmentIndex(_seg3.get()));
}

TEST_F(ItinGeometryCalculatorTest, testRaisesOnBadSegment)
{
  ASSERT_THROW(_itinCalculator->getTravelSegmentIndex(0), ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testRaisesOnSegmentNotInItin)
{
  std::shared_ptr<AirSeg> seg(new AirSeg);
  ASSERT_THROW(_itinCalculator->getTravelSegmentIndex(seg.get()),
               ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testReturnsFirstFareMarketSegmentsIndexInItin)
{
  // faremarkets:
  // fm1  seg1  seg2
  // fm2  seg2  seg3
  // so starting indices are 0, 1 accordingly
  ASSERT_EQ(0, _itinCalculator->getFareMarketStartSegmentIndex(*_fm1));
  ASSERT_EQ(1, _itinCalculator->getFareMarketStartSegmentIndex(*_fm2));
}

TEST_F(ItinGeometryCalculatorTest, testReturnsLastFareMarketSegmentsIndexInItin)
{
  // faremarkets:
  // fm1  seg1  seg2
  // fm2  seg2  seg3
  // so ending indices are 1, 2 accordingly
  ASSERT_EQ(1, _itinCalculator->getFareMarketEndSegmentIndex(*_fm1));
  ASSERT_EQ(2, _itinCalculator->getFareMarketEndSegmentIndex(*_fm2));
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyFiltersItinBrandsUsingPolicy)
{
  _relations->addBrandProgramPair("A", "PROG", 0);
  _relations->addBrandProgramPair("B", "PROG", 1);
  _relations->addBrandProgramPair("C", "PROG", 2);

  EXPECT_CALL(*_trxCalculator, getBrandsAndPrograms(_, _))
      .WillOnce(SetArgReferee<1>(*_relations));
  EXPECT_CALL(*_inclusionPolicy, isBrandCodeInItin("A", _, _))
      .WillOnce(Return(false));
  EXPECT_CALL(*_inclusionPolicy, isBrandCodeInItin("B", _, _))
      .WillOnce(Return(true));
  EXPECT_CALL(*_inclusionPolicy, isBrandCodeInItin("C", _, _))
      .WillOnce(Return(false));

  UnorderedBrandCodes expected;
  expected.insert("B");

  ASSERT_EQ(expected, _itinCalculator->getItinSpecificBrandCodes(*_fm1));
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyFiltersItinBrandsIndicesUsingPolicy)
{
  const int INDEX_A = 0;
  const int INDEX_B = 1;
  const int INDEX_C = 2;

  _relations->addBrandProgramPair("A", "PROG", INDEX_A);
  _relations->addBrandProgramPair("B", "PROG", INDEX_B);
  _relations->addBrandProgramPair("C", "PROG", INDEX_C);

  _fm1->brandProgramIndexVec().push_back(INDEX_A);
  _fm1->brandProgramIndexVec().push_back(INDEX_B);
  _fm1->brandProgramIndexVec().push_back(INDEX_C);

  BrandInfo brandA, brandB, brandC;
  brandA.brandCode() = "A";
  brandB.brandCode() = "B";
  brandC.brandCode() = "C";
  BrandProgram program;
  std::vector<QualifiedBrand> allBrands;
  allBrands.push_back(std::make_pair(&program, &brandA)); //INDEX_A
  allBrands.push_back(std::make_pair(&program, &brandB)); //INDEX_B
  allBrands.push_back(std::make_pair(&program, &brandC)); //INDEX_C

  EXPECT_CALL(*_trxCalculator, getBrandsAndPrograms(_, _))
      .WillOnce(SetArgReferee<1>(*_relations));
  EXPECT_CALL(*_trxCalculator, getQualifiedBrands())
      .WillOnce(ReturnRef(allBrands));
  EXPECT_CALL(*_inclusionPolicy, isBrandCodeInItin("A", _, _))
      .WillOnce(Return(false));
  EXPECT_CALL(*_inclusionPolicy, isBrandCodeInItin("B", _, _))
      .WillOnce(Return(true));
  EXPECT_CALL(*_inclusionPolicy, isBrandCodeInItin("C", _, _))
      .WillOnce(Return(false));

  QualifiedBrandIndices expected;
  expected.insert(INDEX_B);

  ASSERT_EQ(expected, _itinCalculator->getItinSpecificBrandCodesIndices(*_fm1));
}

TEST_F(ItinGeometryCalculatorTest, testReturnsTrxCalculatorCorrectly)
{
  ASSERT_EQ(_trxCalculator.get(), &_itinCalculator->getTrxGeometryCalculator());
}

TEST_F(ItinGeometryCalculatorTest, testReportsSegmentAsArunkCorrectly)
{
  std::shared_ptr<ArunkSeg> seg(new ArunkSeg);
  _itin->travelSeg().push_back(seg.get());
  //segments 0, 1, 2 are AirSeg (set in createItin()), segment 3 is ArunkSeg
  ASSERT_FALSE(_itinCalculator->isTravelSegmentArunk(0));
  ASSERT_FALSE(_itinCalculator->isTravelSegmentArunk(1));
  ASSERT_FALSE(_itinCalculator->isTravelSegmentArunk(2));
  ASSERT_TRUE(_itinCalculator->isTravelSegmentArunk(3));
}

TEST_F(ItinGeometryCalculatorTest, testRaisesIfSegmentIndexOutOfRange)
{
  //itin travels segment size is 3
  ASSERT_THROW(_itinCalculator->isTravelSegmentArunk(3), ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testReturnsCorrectlyTravelSegmentIfNotArunk)
{
  //segments 0, 1, 2 are AirSeg
  ASSERT_EQ(0, _itinCalculator->getNextTravelSegmentIfCurrentArunk(0));
  ASSERT_EQ(1, _itinCalculator->getNextTravelSegmentIfCurrentArunk(1));
  ASSERT_EQ(2, _itinCalculator->getNextTravelSegmentIfCurrentArunk(2));
}

TEST_F(ItinGeometryCalculatorTest, testReturnsCorrectlyNextTravelSegmentIfArunk)
{
  std::shared_ptr<ArunkSeg> seg(new ArunkSeg);
  std::vector<TravelSeg*>::iterator it = _itin->travelSeg().begin();
  _itin->travelSeg().insert(it + 1, seg.get());
  //segments 0, 2 are AirSeg (set in createItin()), segment 1 is ArunkSeg
  ASSERT_EQ(0, _itinCalculator->getNextTravelSegmentIfCurrentArunk(0));
  // return next
  ASSERT_EQ(2, _itinCalculator->getNextTravelSegmentIfCurrentArunk(1));
  ASSERT_EQ(2, _itinCalculator->getNextTravelSegmentIfCurrentArunk(2));

}

TEST_F(ItinGeometryCalculatorTest, testReturnsCorrectlySegmentCountIfSegmentOutOfRange)
{
  //segment count is 3, see createItin()
  ASSERT_EQ(3, _itinCalculator->getNextTravelSegmentIfCurrentArunk(10));
}

TEST_F(ItinGeometryCalculatorTest, testReturnsCorrectlySegmentCountIfLastSegmentIsArunk)
{
  std::shared_ptr<ArunkSeg> seg(new ArunkSeg);
  std::vector<TravelSeg*>::iterator it = _itin->travelSeg().begin();
  _itin->travelSeg().insert(it + 2, seg.get());
  //segments 0, 1 are AirSeg (set in createItin()), segment 2 is ArunkSeg
  ASSERT_EQ(0, _itinCalculator->getNextTravelSegmentIfCurrentArunk(0));
  ASSERT_EQ(1, _itinCalculator->getNextTravelSegmentIfCurrentArunk(1));
  //segment count is 3, see createItin()
  ASSERT_EQ(3, _itinCalculator->getNextTravelSegmentIfCurrentArunk(2));
}

TEST_F(ItinGeometryCalculatorTest, testReturnsTravelSegmentsLegIdCorrectly)
{
  _itin->travelSeg()[0]->legId() = 123;
  ASSERT_EQ(123, _itinCalculator->getTravelSegmentLegId(0));
}

TEST_F(ItinGeometryCalculatorTest, testRaisesOnSegmentIndexOutOfRange)
{
  ASSERT_THROW(_itinCalculator->getTravelSegmentLegId(_itin->travelSeg().size()),
      ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testRaisesOnNotExistingTravelSegment)
{
  _itin->travelSeg().push_back(static_cast<TravelSeg*>(0));
  ASSERT_THROW(_itinCalculator->getTravelSegmentLegId(_itin->travelSeg().size() - 1),
      ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testAddsBrandProgramPairCorrectly)
{
  const std::string DUMMY_BRAND = "DUMMY_BRAND";
  const std::string DUMMY_PROGRAM_ID = "DUMMY_PROGRAM";
  Itin::ProgramsForBrandMap expected = _itin->getProgramsForBrandMap();
  expected[DUMMY_BRAND].insert(DUMMY_PROGRAM_ID);

  _itinCalculator->addBrandProgramPair(DUMMY_BRAND, DUMMY_PROGRAM_ID);
  ASSERT_EQ(expected, _itin->getProgramsForBrandMap());
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyReturnsFMThruForSingleLeg)
{
  //initial state:
  //fm1 = seg1, seg2
  //fm2 = seg2, seg3
  //legs: seg1 = 1, seg2 = 1, seg3 = 1
  //travel: 0 = seg1, 1 = seg2, 2 = seg3
  _itin->travelSeg()[2]->legId() = 2;
  ASSERT_TRUE(_itinCalculator->isThruFareMarket(*_fm1));
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyReturnsFMThruForMultipleLegs)
{
  //initial state:
  //fm1 = seg1, seg2
  //fm2 = seg2, seg3
  //legs: seg1 = 1, seg2 = 1, seg3 = 1
  //travel: 0 = seg1, 1 = seg2, 2 = seg3
  _fm1->travelSeg().push_back(_seg3.get());
  _itin->travelSeg()[2]->legId() = 2;
  ASSERT_TRUE(_itinCalculator->isThruFareMarket(*_fm1));
}

TEST_F(ItinGeometryCalculatorTest, testReturnsQualifiedBrandsForCarriersBrandProperly)
{
  const BrandCode DUMMY_BRAND_1 = "DUMMY_BRAND_1";
  const BrandCode DUMMY_BRAND_2 = "DUMMY_BRAND_2";
  const BrandCode DUMMY_BRAND_3 = "DUMMY_BRAND_3";
  const ProgramCode DUMMY_PROGRAM_1 = "DUMMY_PROG_1";
  const ProgramCode DUMMY_PROGRAM_2 = "DUMMY_PROG_2";
  const CarrierCode DUMMY_CARRIER_1 = "CX1";
  const CarrierCode DUMMY_CARRIER_2 = "CX2";

  _fm3.reset(new FareMarket);
  _itin->fareMarket().push_back(_fm3.get());

  _fm1->governingCarrier() = DUMMY_CARRIER_1;
  _fm2->governingCarrier() = DUMMY_CARRIER_2;
  _fm3->governingCarrier() = DUMMY_CARRIER_1;


  const int DUMMY_INDEX_1 = 1;
  const int DUMMY_INDEX_2 = 2;
  const int DUMMY_INDEX_3 = 3;
  const int DUMMY_INDEX_4 = 4;
  const int DUMMY_INDEX_5 = 5;
  const int DUMMY_INDEX_6 = 6;

  _relations->addBrandProgramPair(DUMMY_BRAND_1, DUMMY_PROGRAM_1, DUMMY_INDEX_1);
  _relations->addBrandProgramPair(DUMMY_BRAND_2, DUMMY_PROGRAM_1, DUMMY_INDEX_2);
  _relations->addBrandProgramPair(DUMMY_BRAND_1, DUMMY_PROGRAM_2, DUMMY_INDEX_3);
  _relations->addBrandProgramPair(DUMMY_BRAND_2, DUMMY_PROGRAM_2, DUMMY_INDEX_4);
  _relations->addBrandProgramPair(DUMMY_BRAND_3, DUMMY_PROGRAM_2, DUMMY_INDEX_5);

  std::shared_ptr<BrandProgramRelations> _relations2;
  _relations2.reset(new BrandProgramRelations());
  //the same relations for fm2 and fm3
  _relations2->addBrandProgramPair(DUMMY_BRAND_3, DUMMY_PROGRAM_1, DUMMY_INDEX_6);

  EXPECT_CALL(*_trxCalculator, getBrandsAndPrograms(Eq(ByRef(*_fm1)), _))
    .WillOnce(SetArgReferee<1>(*_relations));
  EXPECT_CALL(*_trxCalculator, getBrandsAndPrograms(Eq(ByRef(*_fm2)), _))
    .WillOnce(SetArgReferee<1>(*_relations2));
  EXPECT_CALL(*_trxCalculator, getBrandsAndPrograms(Eq(ByRef(*_fm3)), _))
    .WillOnce(SetArgReferee<1>(*_relations2));

  QualifiedBrandIndices qbiForFm1B1;
  qbiForFm1B1.insert(DUMMY_INDEX_1);
  qbiForFm1B1.insert(DUMMY_INDEX_3);
  EXPECT_CALL(*_inclusionPolicy, getIndicesForBrandCodeWithFiltering(
      DUMMY_BRAND_1, *_relations, _))
    .WillOnce(Return(qbiForFm1B1));
  QualifiedBrandIndices qbiForFm1B2;
  qbiForFm1B2.insert(DUMMY_INDEX_2);
  qbiForFm1B2.insert(DUMMY_INDEX_4);
  EXPECT_CALL(*_inclusionPolicy, getIndicesForBrandCodeWithFiltering(
      DUMMY_BRAND_2, *_relations, _))
    .WillOnce(Return(qbiForFm1B2));
  QualifiedBrandIndices qbiForFm1B3;
  qbiForFm1B3.insert(DUMMY_INDEX_5);
  EXPECT_CALL(*_inclusionPolicy, getIndicesForBrandCodeWithFiltering(
      DUMMY_BRAND_3, *_relations, _))
    .WillOnce(Return(qbiForFm1B3));
  QualifiedBrandIndices qbiForFm2Fm3B3;
  qbiForFm2Fm3B3.insert(DUMMY_INDEX_6);
  EXPECT_CALL(*_inclusionPolicy, getIndicesForBrandCodeWithFiltering(
      DUMMY_BRAND_3, *_relations2, _))
    .WillRepeatedly(Return(qbiForFm2Fm3B3));

  QualifiedBrandIndices expected;
  expected.insert(DUMMY_INDEX_1);
  expected.insert(DUMMY_INDEX_3);
  ASSERT_EQ(expected, _itinCalculator->getQualifiedBrandIndicesForCarriersBrand(
      DUMMY_CARRIER_1, DUMMY_BRAND_1));

  expected.clear();
  expected.insert(DUMMY_INDEX_2);
  expected.insert(DUMMY_INDEX_4);
  ASSERT_EQ(expected, _itinCalculator->getQualifiedBrandIndicesForCarriersBrand(
      DUMMY_CARRIER_1, DUMMY_BRAND_2));

  expected.clear();
  expected.insert(DUMMY_INDEX_5);
  expected.insert(DUMMY_INDEX_6);
  ASSERT_EQ(expected, _itinCalculator->getQualifiedBrandIndicesForCarriersBrand(
      DUMMY_CARRIER_1, DUMMY_BRAND_3));

  expected.clear();
  expected.insert(DUMMY_INDEX_6);
  ASSERT_EQ(expected, _itinCalculator->getQualifiedBrandIndicesForCarriersBrand(
      DUMMY_CARRIER_2, DUMMY_BRAND_3));
}

TEST_F(ItinGeometryCalculatorTest, testRaisesOnCarrierNotInMap)
{
  const CarrierCode DUMMY_CARRIER = "ABC";
  const CarrierCode OTHER_DUMMY_CARRIER = "XYZ";
  const BrandCode DUMMY_BRAND = "DUMMY_BRAND";
  const ProgramCode DUMMY_PROGRAM = "DUMMY_PROGRAM";

  _fm1->governingCarrier() = DUMMY_CARRIER;
  _fm2->governingCarrier() = DUMMY_CARRIER;

  const int DUMMY_INDEX_1 = 1;

  _relations->addBrandProgramPair(DUMMY_BRAND, DUMMY_PROGRAM, DUMMY_INDEX_1);

  EXPECT_CALL(*_trxCalculator, getBrandsAndPrograms(Eq(ByRef(*_fm1)), _))
    .WillOnce(SetArgReferee<1>(*_relations));
  EXPECT_CALL(*_trxCalculator, getBrandsAndPrograms(Eq(ByRef(*_fm2)), _))
    .WillOnce(SetArgReferee<1>(*_relations));

  QualifiedBrandIndices qbiForDummyBrand;
  qbiForDummyBrand.insert(DUMMY_INDEX_1);
  EXPECT_CALL(*_inclusionPolicy, getIndicesForBrandCodeWithFiltering(
      DUMMY_BRAND, *_relations, _))
    .WillRepeatedly(Return(qbiForDummyBrand));

  ASSERT_THROW(_itinCalculator->getQualifiedBrandIndicesForCarriersBrand(
      OTHER_DUMMY_CARRIER, DUMMY_BRAND), ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testRaisesOnBrandNotInMap)
{
  const CarrierCode DUMMY_CARRIER = "ABC";
  const BrandCode DUMMY_BRAND = "DUMMY_BRAND";
  const BrandCode DUMMY_OTHER_BRAND = "DUMMY_OTHER_BRAND";
  const ProgramCode DUMMY_PROGRAM = "DUMMY_PROGRAM";

  _fm1->governingCarrier() = DUMMY_CARRIER;
  _fm2->governingCarrier() = DUMMY_CARRIER;

  const int DUMMY_INDEX_1 = 1;

  _relations->addBrandProgramPair(DUMMY_BRAND, DUMMY_PROGRAM, DUMMY_INDEX_1);

  EXPECT_CALL(*_trxCalculator, getBrandsAndPrograms(Eq(ByRef(*_fm1)), _))
    .WillOnce(SetArgReferee<1>(*_relations));
  EXPECT_CALL(*_trxCalculator, getBrandsAndPrograms(Eq(ByRef(*_fm2)), _))
    .WillOnce(SetArgReferee<1>(*_relations));

  QualifiedBrandIndices qbiForDummyBrand;
  qbiForDummyBrand.insert(DUMMY_INDEX_1);
  EXPECT_CALL(*_inclusionPolicy, getIndicesForBrandCodeWithFiltering(
      DUMMY_BRAND, *_relations, _))
    .WillRepeatedly(Return(qbiForDummyBrand));

  ASSERT_THROW(_itinCalculator->getQualifiedBrandIndicesForCarriersBrand(
      DUMMY_CARRIER, DUMMY_OTHER_BRAND), ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyCalculatesLegsStartEndSegmentIndices)
{
  std::shared_ptr<AirSeg> _seg4, _seg5, _seg6;
  _seg4.reset(new AirSeg);
  _seg5.reset(new AirSeg);
  _seg6.reset(new AirSeg);
  _itin->travelSeg().push_back(_seg4.get());
  _itin->travelSeg().push_back(_seg5.get());
  _itin->travelSeg().push_back(_seg6.get());
  _itin->travelSeg()[0]->legId() = 0;
  _itin->travelSeg()[1]->legId() = 0;
  _itin->travelSeg()[2]->legId() = 1;
  _itin->travelSeg()[3]->legId() = 2;
  _itin->travelSeg()[4]->legId() = 2;
  _itin->travelSeg()[5]->legId() = 2;

  std::vector<std::pair<size_t, size_t> > expected;
  expected.push_back(std::make_pair(0, 1));
  expected.push_back(std::make_pair(2, 2));
  expected.push_back(std::make_pair(3, 5));

  ASSERT_EQ(expected,_itinCalculator->calculateLegsStartEndSegmentIndices());
}

TEST_F(ItinGeometryCalculatorTest, testRaisesOnNotExistingTravelSegmentDuringLegsStartEndSegmentIndicesCalculation)
{
  _itin->travelSeg().push_back(static_cast<TravelSeg*>(0));
  ASSERT_THROW(_itinCalculator->calculateLegsStartEndSegmentIndices(),
      ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testRaisesOnMissingLegDuringLegsStartEndSegmentIndicesCalculation)
{
  _itin->travelSeg()[0]->legId() = 0;
  _itin->travelSeg()[1]->legId() = 2;
  _itin->travelSeg()[2]->legId() = 2;
  ASSERT_THROW(_itinCalculator->calculateLegsStartEndSegmentIndices(),
      ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyCalcualtesNonFixedSegmentsForContextShopping)
{
  std::shared_ptr<AirSeg> _seg4, _seg5, _seg6;
  _seg4.reset(new AirSeg);
  _seg5.reset(new AirSeg);
  _seg6.reset(new AirSeg);
  _itin->travelSeg().push_back(_seg4.get());
  _itin->travelSeg().push_back(_seg5.get());
  _itin->travelSeg().push_back(_seg6.get());
  _itin->travelSeg()[0]->legId() = 0;
  _itin->travelSeg()[1]->legId() = 0;
  _itin->travelSeg()[2]->legId() = 1;
  _itin->travelSeg()[3]->legId() = 2;
  _itin->travelSeg()[4]->legId() = 2;
  _itin->travelSeg()[5]->legId() = 2;

  std::vector<bool> noFixedLegs;
  noFixedLegs.push_back(false);
  noFixedLegs.push_back(false);
  noFixedLegs.push_back(false);

  std::vector<bool> fixedFirstLeg;
  fixedFirstLeg.push_back(true);
  fixedFirstLeg.push_back(false);
  fixedFirstLeg.push_back(false);

  std::vector<bool> fixedLastLeg;
  fixedLastLeg.push_back(false);
  fixedLastLeg.push_back(false);
  fixedLastLeg.push_back(true);

  std::vector<bool> fixedFirstAndLastLeg;
  fixedFirstAndLastLeg.push_back(true);
  fixedFirstAndLastLeg.push_back(false);
  fixedFirstAndLastLeg.push_back(true);

  std::pair<size_t, size_t> expectedForNoFixedLegs = std::make_pair(0, 6);
  std::pair<size_t, size_t> expectedForFixedFirstLeg = std::make_pair(2, 6);
  std::pair<size_t, size_t> expectedForFixedLastLeg  = std::make_pair(0, 3);
  std::pair<size_t, size_t> expectedForFixedFirstAndLastLeg  = std::make_pair(2, 3);

  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillOnce(ReturnRef(noFixedLegs))
    .WillOnce(ReturnRef(fixedFirstLeg))
    .WillOnce(ReturnRef(fixedLastLeg))
    .WillOnce(ReturnRef(fixedFirstAndLastLeg));

  ASSERT_EQ(expectedForNoFixedLegs, _itinCalculator->calculateNonFixedSegmentsForContextShopping());
  ASSERT_EQ(expectedForFixedFirstLeg, _itinCalculator->calculateNonFixedSegmentsForContextShopping());
  ASSERT_EQ(expectedForFixedLastLeg, _itinCalculator->calculateNonFixedSegmentsForContextShopping());
  ASSERT_EQ(expectedForFixedFirstAndLastLeg, _itinCalculator->calculateNonFixedSegmentsForContextShopping());
}

TEST_F(ItinGeometryCalculatorTest, testRaisesOnEmptyFixedLegsVector)
{
  std::vector<bool> fixedLegs;
  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillOnce(ReturnRef(fixedLegs));

  ASSERT_THROW(_itinCalculator->calculateNonFixedSegmentsForContextShopping(),
      ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testReturnsProperResponseForAllLegsFixed)
{
  _itin->travelSeg()[0]->legId() = 0;
  _itin->travelSeg()[1]->legId() = 1;
  _itin->travelSeg()[2]->legId() = 2;

  std::vector<bool> fixedLegs;
  fixedLegs.push_back(true);
  fixedLegs.push_back(true);
  fixedLegs.push_back(true);

  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillOnce(ReturnRef(fixedLegs));

  std::pair<size_t, size_t> expected =
      std::make_pair(TRAVEL_SEG_DEFAULT_ID, TRAVEL_SEG_DEFAULT_ID);
  ASSERT_EQ(expected, _itinCalculator->calculateNonFixedSegmentsForContextShopping());
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyVerifiesIfFareMarketIsOnFixedLegAndNoBrandSet)
{
  std::vector<bool> fixedLegs;
  fixedLegs.push_back(true);
  fixedLegs.push_back(false);

  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillOnce(ReturnRef(fixedLegs));

  std::shared_ptr<FareComponentShoppingContext> _shoppingContext;
  _shoppingContext.reset(new FareComponentShoppingContext());

  EXPECT_CALL(*_trxCalculator, getFareComponentShoppingContext(_))
    .WillRepeatedly(Return(_shoppingContext.get()));

  BrandCode restulBrandCode;
  BrandCode expectedBrandCode = NO_BRAND;

  ASSERT_TRUE(_itinCalculator->isFareMarketOnFixedLeg(*_fm1, restulBrandCode));
  ASSERT_EQ(expectedBrandCode, restulBrandCode);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyVerifiesIfFareMarketIsOnFixedLegAndDifferrentBrandsOnFixedLegs)
{
  std::vector<bool> fixedLegs;
  fixedLegs.push_back(true);
  fixedLegs.push_back(true);

  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillOnce(ReturnRef(fixedLegs));

  std::shared_ptr<FareComponentShoppingContext> _shoppingContext0;
  std::shared_ptr<FareComponentShoppingContext> _shoppingContext1;
  _shoppingContext0.reset(new FareComponentShoppingContext());
  _shoppingContext1.reset(new FareComponentShoppingContext());
  _shoppingContext0->brandCode = DUMMY_BRAND;
  _shoppingContext1->brandCode = ANOTHER_DUMMY_BRAND;

  EXPECT_CALL(*_trxCalculator, getFareComponentShoppingContext(1))
    .WillRepeatedly(Return(_shoppingContext0.get()));
  EXPECT_CALL(*_trxCalculator, getFareComponentShoppingContext(2))
    .WillRepeatedly(Return(_shoppingContext1.get()));

  BrandCode restulBrandCode;
  BrandCode expectedBrandCode = NO_BRAND;

  ASSERT_TRUE(_itinCalculator->isFareMarketOnFixedLeg(*_fm1, restulBrandCode));
  ASSERT_EQ(expectedBrandCode, restulBrandCode);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyVerifiesIfFareMarketIsOnFixedLegAndBrandSet)
{
  std::vector<bool> fixedLegs;
  fixedLegs.push_back(true);
  fixedLegs.push_back(false);

  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillOnce(ReturnRef(fixedLegs));

  std::shared_ptr<FareComponentShoppingContext> _shoppingContext;
  _shoppingContext.reset(new FareComponentShoppingContext());
  _shoppingContext->brandCode = DUMMY_BRAND;

  EXPECT_CALL(*_trxCalculator, getFareComponentShoppingContext(_))
    .WillRepeatedly(Return(_shoppingContext.get()));

  std::shared_ptr<BrandProgram> brandProgram1;
  std::shared_ptr<BrandInfo> brandInfo1;
  std::shared_ptr<BrandInfo> brandInfo2;
  std::shared_ptr<BrandInfo> brandInfo3;
  brandProgram1.reset(new BrandProgram());
  brandInfo1.reset(new BrandInfo());
  brandInfo1->brandCode() = DUMMY_BRAND;

  std::vector<QualifiedBrand> qualifiedBrands;
  qualifiedBrands.push_back(std::make_pair(brandProgram1.get(), brandInfo1.get()));
  _fm1->brandProgramIndexVec().push_back(0);

  EXPECT_CALL(*_trxCalculator, getQualifiedBrands())
    .WillOnce(ReturnRef(qualifiedBrands));

  BrandCode restulBrandCode;
  BrandCode expectedBrandCode = DUMMY_BRAND;

  ASSERT_TRUE(_itinCalculator->isFareMarketOnFixedLeg(*_fm1, restulBrandCode));
  ASSERT_EQ(expectedBrandCode, restulBrandCode);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyVerifiesIfFareMarketIsOnFixedLegAndItIsNot)
{
  std::vector<bool> fixedLegs;
  fixedLegs.push_back(false);
  fixedLegs.push_back(true);

  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillOnce(ReturnRef(fixedLegs));

  std::shared_ptr<FareComponentShoppingContext> _shoppingContext;
  _shoppingContext.reset(new FareComponentShoppingContext());
  _shoppingContext->brandCode = DUMMY_BRAND;

  EXPECT_CALL(*_trxCalculator, getFareComponentShoppingContext(_))
    .WillRepeatedly(Return(_shoppingContext.get()));

  BrandCode restulBrandCode;
  BrandCode expectedBrandCode = NO_BRAND;

  ASSERT_FALSE(_itinCalculator->isFareMarketOnFixedLeg(*_fm1, restulBrandCode));
  ASSERT_EQ(expectedBrandCode, restulBrandCode);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyRaisesOnFareMarketOutsideOfTravel)
{
  std::vector<bool> fixedLegs;
  std::vector<bool> fixedLegsSingle;
  fixedLegsSingle.push_back(false);
  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillOnce(ReturnRef(fixedLegs))
    .WillOnce(ReturnRef(fixedLegsSingle));

  BrandCode restulBrandCode;

  ASSERT_THROW(_itinCalculator->isFareMarketOnFixedLeg(*_fm1, restulBrandCode),
    ErrorResponseException);

  _itin->travelSeg()[1]->legId() = 1;

  ASSERT_THROW(_itinCalculator->isFareMarketOnFixedLeg(*_fm1, restulBrandCode),
      ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyRaisesOnFareMarketStartingAfterEnd)
{
  std::vector<bool> fixedLegs;
  fixedLegs.push_back(false);
  fixedLegs.push_back(false);
  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillOnce(ReturnRef(fixedLegs));

  _itin->travelSeg()[0]->legId() = 1;

  BrandCode restulBrandCode;

  ASSERT_THROW(_itinCalculator->isFareMarketOnFixedLeg(*_fm1, restulBrandCode),
    ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyReducesFareMarketsBrandsOnFixedLegs)
{
  std::shared_ptr<BrandProgram> brandProgram1;
  std::shared_ptr<BrandProgram> brandProgram2;
  std::shared_ptr<BrandProgram> brandProgram3;
  std::shared_ptr<BrandInfo> brandInfo1;
  std::shared_ptr<BrandInfo> brandInfo2;
  std::shared_ptr<BrandInfo> brandInfo3;
  brandProgram1.reset(new BrandProgram());
  brandProgram2.reset(new BrandProgram());
  brandProgram3.reset(new BrandProgram());
  brandInfo1.reset(new BrandInfo());
  brandInfo2.reset(new BrandInfo());
  brandInfo3.reset(new BrandInfo());
  brandInfo1->brandCode() = ANOTHER_DUMMY_BRAND;
  brandInfo2->brandCode() = DUMMY_BRAND;
  brandInfo3->brandCode() = ANOTHER_DUMMY_BRAND;

  std::vector<QualifiedBrand> qualifiedBrands;
  qualifiedBrands.push_back(std::make_pair(brandProgram1.get(), brandInfo1.get()));
  qualifiedBrands.push_back(std::make_pair(brandProgram2.get(), brandInfo2.get()));
  qualifiedBrands.push_back(std::make_pair(brandProgram3.get(), brandInfo3.get()));

  EXPECT_CALL(*_trxCalculator, getQualifiedBrands())
    .WillOnce(ReturnRef(qualifiedBrands));

  std::vector<int> inputBrandProgramIndexVec;
  inputBrandProgramIndexVec.push_back(0);
  inputBrandProgramIndexVec.push_back(1);
  inputBrandProgramIndexVec.push_back(2);
  _fm1->brandProgramIndexVec() = inputBrandProgramIndexVec;

  std::vector<int> expectedBrandProgramIndexVec;
  expectedBrandProgramIndexVec.push_back(1);

  BrandCode fixedBrand = DUMMY_BRAND;
  _itinCalculator->reduceFareMarketBrands(*_fm1, fixedBrand);

  ASSERT_EQ(expectedBrandProgramIndexVec, _fm1->brandProgramIndexVec());
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyRaisesOnBrandNotInQualifiedBrands)
{
  std::vector<QualifiedBrand> qualifiedBrands;
  qualifiedBrands.push_back(std::make_pair(static_cast<BrandProgram*>(0),
    static_cast<BrandInfo*>(0)));
  EXPECT_CALL(*_trxCalculator, getQualifiedBrands())
    .WillOnce(ReturnRef(qualifiedBrands));

  std::vector<int> inputBrandProgramIndexVec;
  inputBrandProgramIndexVec.push_back(0);
  _fm1->brandProgramIndexVec() = inputBrandProgramIndexVec;

  BrandCode fixedBrand = DUMMY_BRAND;
  ASSERT_THROW(_itinCalculator->reduceFareMarketBrands(*_fm1, fixedBrand);,
    ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyReducesBrandsForFareMarketsOnFixedLegs)
{
  std::shared_ptr<BrandProgram> brandProgram1;
  std::shared_ptr<BrandProgram> brandProgram2;
  std::shared_ptr<BrandProgram> brandProgram3;
  std::shared_ptr<BrandInfo> brandInfo1;
  std::shared_ptr<BrandInfo> brandInfo2;
  std::shared_ptr<BrandInfo> brandInfo3;
  brandProgram1.reset(new BrandProgram());
  brandProgram2.reset(new BrandProgram());
  brandProgram3.reset(new BrandProgram());
  brandInfo1.reset(new BrandInfo());
  brandInfo2.reset(new BrandInfo());
  brandInfo3.reset(new BrandInfo());
  brandInfo1->brandCode() = ANOTHER_DUMMY_BRAND;
  brandInfo2->brandCode() = DUMMY_BRAND;
  brandInfo3->brandCode() = ANOTHER_DUMMY_BRAND;

  std::vector<QualifiedBrand> qualifiedBrands;
  qualifiedBrands.push_back(std::make_pair(brandProgram1.get(), brandInfo1.get()));
  qualifiedBrands.push_back(std::make_pair(brandProgram2.get(), brandInfo2.get()));
  qualifiedBrands.push_back(std::make_pair(brandProgram3.get(), brandInfo3.get()));

  EXPECT_CALL(*_trxCalculator, getQualifiedBrands())
    .WillRepeatedly(ReturnRef(qualifiedBrands));

  std::vector<bool> fixedLegs;
  fixedLegs.push_back(false);
  fixedLegs.push_back(true);
  EXPECT_CALL(*_trxCalculator, getFixedLegs())
    .WillRepeatedly(ReturnRef(fixedLegs));

  _fm3.reset(new FareMarket);
  _itin->fareMarket().push_back(_fm3.get());
  _fm3->travelSeg().push_back(_seg1.get());
  _fm3->travelSeg().push_back(_seg2.get());
  _fm3->travelSeg().push_back(_seg3.get());

  _itin->travelSeg()[0]->legId() = 0;
  _itin->travelSeg()[1]->legId() = 0;
  _itin->travelSeg()[2]->legId() = 1;

  std::vector<int> inputBrandProgramIndexVec;
  inputBrandProgramIndexVec.push_back(0);
  inputBrandProgramIndexVec.push_back(1);
  inputBrandProgramIndexVec.push_back(2);
  _fm1->brandProgramIndexVec() = inputBrandProgramIndexVec;
  _fm2->brandProgramIndexVec() = inputBrandProgramIndexVec;
  _fm3->brandProgramIndexVec() = inputBrandProgramIndexVec;

  std::shared_ptr<FareComponentShoppingContext> _shoppingContext;
  _shoppingContext.reset(new FareComponentShoppingContext());
  _shoppingContext->brandCode = DUMMY_BRAND;

  EXPECT_CALL(*_trxCalculator, getFareComponentShoppingContext(_))
    .WillRepeatedly(Return(_shoppingContext.get()));

  _itinCalculator->reduceFareMarketsBrandsOnFixedLegs();

  std::vector<int> expectedBrandProgramIndexVecBrand;
  expectedBrandProgramIndexVecBrand.push_back(1);
  std::vector<int> expectedBrandProgramIndexVecNoChange;
  expectedBrandProgramIndexVecNoChange.push_back(0);
  expectedBrandProgramIndexVecNoChange.push_back(1);
  expectedBrandProgramIndexVecNoChange.push_back(2);

  ASSERT_EQ(expectedBrandProgramIndexVecNoChange, _fm1->brandProgramIndexVec());
  ASSERT_EQ(expectedBrandProgramIndexVecBrand, _fm2->brandProgramIndexVec());
  ASSERT_EQ(expectedBrandProgramIndexVecBrand, _fm3->brandProgramIndexVec());
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyReducesRaisesOnBadFareMarket)
{
  _itin->fareMarket().clear();
  _itin->fareMarket().push_back(static_cast<FareMarket*>(0));
  ASSERT_THROW(_itinCalculator->reduceFareMarketsBrandsOnFixedLegs(),
    ErrorResponseException);
}

TEST_F(ItinGeometryCalculatorTest, testCorrectlyCalculatesPnrForSegments)
{
  _itin->travelSeg()[0]->pnrSegment() = 1;
  _itin->travelSeg()[1]->pnrSegment() = 3;
  _itin->travelSeg()[2]->pnrSegment() = 9;

  std::vector<size_t> expected;
  expected.push_back(1);
  expected.push_back(3);
  expected.push_back(9);
  ASSERT_EQ(expected, _itinCalculator->calculatePnrForSegments());
}

} // namespace skipper

} // namespace tse
