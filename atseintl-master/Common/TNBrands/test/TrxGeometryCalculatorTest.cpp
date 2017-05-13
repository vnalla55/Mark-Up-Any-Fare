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

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/ErrorResponseException.h"
#include "Common/TNBrands/TrxGeometryCalculator.h"
#include "Diagnostic/Diag892Collector.h"

#include <memory>
#include <string>
#include <vector>

using namespace ::testing;
using namespace boost;

namespace tse
{

namespace skipper
{

class TrxGeometryCalculatorTest: public Test
{
public:
  void SetUp()
  {
    _trx.reset(new PricingTrx());
    _pricingRequest.reset(new PricingRequest());
    _trx.get()->setRequest(_pricingRequest.get());
    _calculator.reset(new TrxGeometryCalculator(*_trx));
    _fareMarket.reset(new FareMarket());
  }

  void TearDown(){}

  void setUpQualifiedBrands()
  {
    newQualifiedBrand("PROG1", "BRAND1"); //pair 1
    newQualifiedBrand("PROG1", "BRAND2"); //pair 2
    newQualifiedBrand("PROG2", "BRAND3"); //pair 3
    newQualifiedBrand("PROG2", "BRAND2"); //pair 4
    _trx->brandProgramVec() = _qualifiedBrands;
  }

  BrandProgram* newBrandProgram(const std::string& programId)
  {
    BrandProgram* program = new BrandProgram();
    program->programID() = programId;
    _programs.push_back(std::shared_ptr<BrandProgram>(program));
    return program;
  }

  BrandInfo* newBrandInfo(const BrandCode& brandCode)
  {
    BrandInfo* brand = new BrandInfo();
    brand->brandCode() = brandCode;
    _brands.push_back(std::shared_ptr<BrandInfo>(brand));
    return brand;
  }

  void newQualifiedBrand(
      const std::string& programId, const BrandCode& brandCode)
  {
    _qualifiedBrands.push_back(std::make_pair(
        newBrandProgram(programId), newBrandInfo(brandCode)));
  }

  TestConfigInitializer _cfg;

  std::vector<QualifiedBrand> _qualifiedBrands;
  // used to control lifetime of objects
  std::vector<std::shared_ptr<BrandProgram>> _programs;
  std::vector<std::shared_ptr<BrandInfo>> _brands;
  std::shared_ptr<FareComponentShoppingContext> _shoppingContext;

  std::shared_ptr<TrxGeometryCalculator> _calculator;
  std::shared_ptr<FareMarket> _fareMarket;
  std::shared_ptr<PricingTrx> _trx;
  std::shared_ptr<PricingRequest> _pricingRequest;
};

TEST_F(TrxGeometryCalculatorTest, testReturnsBrandsAndProgramsForFareMarketProperly)
{
  BrandProgramRelations response;
  setUpQualifiedBrands();
  // corresponds to pairs 2 and 3 in setUpQualifiedBrands
  _fareMarket->brandProgramIndexVec().push_back(1);
  _fareMarket->brandProgramIndexVec().push_back(2);

  _calculator->getBrandsAndPrograms(*_fareMarket, response);

  BrandProgramRelations expected;
  expected.addBrandProgramPair("BRAND2", "PROG1", 1);
  expected.addBrandProgramPair("BRAND3", "PROG2", 2);

  ASSERT_EQ(expected, response);
}

TEST_F(TrxGeometryCalculatorTest, testRaisesIfFMcontaisIndexOutOfQualifiedBrandsArray)
{
  BrandProgramRelations response;
  setUpQualifiedBrands();
  // _qualifiedBrands is size 4
  _fareMarket->brandProgramIndexVec().push_back(4);
  ASSERT_THROW(
      _calculator->getBrandsAndPrograms(*_fareMarket, response),
      ErrorResponseException);
}

TEST_F(TrxGeometryCalculatorTest, testRaisesIfFMcontainsNegativeIndex)
{
  BrandProgramRelations response;
  _fareMarket->brandProgramIndexVec().push_back(-1);
  ASSERT_THROW(
      _calculator->getBrandsAndPrograms(*_fareMarket, response),
      ErrorResponseException);
}

TEST_F(TrxGeometryCalculatorTest, testRaisesIfQualifiedBrandsContainsBadBrandProgramPointer)
{
  BrandProgramRelations response;
  // let faremarket point to the first entry in the qualifiedBrands vector
  // which we make invalid
  _fareMarket->brandProgramIndexVec().push_back(0);

  _qualifiedBrands.push_back(std::make_pair(
      static_cast<BrandProgram*>(0), newBrandInfo("BRAND1")));

  ASSERT_THROW(
      _calculator->getBrandsAndPrograms(*_fareMarket, response),
      ErrorResponseException);
}

TEST_F(TrxGeometryCalculatorTest, testRaisesIfQualifiedBrandsContainsBadBrandInfoPointer)
{
  BrandProgramRelations response;
  // let faremarket point to the first entry in the qualifiedBrands vector
  // which we make invalid
  _fareMarket->brandProgramIndexVec().push_back(0);

  _qualifiedBrands.push_back(std::make_pair(
      newBrandProgram("PROG1"), static_cast<BrandInfo*>(0)));

  ASSERT_THROW(
      _calculator->getBrandsAndPrograms(*_fareMarket, response),
      ErrorResponseException);
}

TEST_F(TrxGeometryCalculatorTest, testReturnsQualifiedVectorFromTrxPassedInConstructor)
{
  ASSERT_EQ(_qualifiedBrands, _calculator->getQualifiedBrands());
}

TEST_F(TrxGeometryCalculatorTest, testCorrectlyRemovesItinsWithNoBrands)
{
  BrandCodeSet brands;
  brands.insert("DUMMY_BRAND");
  Itin itin1;
  itin1.brandCodes() = brands;
  Itin itin2;
  itin2.brandCodes() = brands;
  Itin itin3;
  itin3.brandCodes() = brands;
  Itin emptyItin;

  _trx->itin().push_back(&itin1);
  _trx->itin().push_back(&emptyItin);
  _trx->itin().push_back(&itin2);
  _trx->itin().push_back(&itin3);

  std::vector<Itin*> expected;
  expected.push_back(&itin1);
  expected.push_back(&itin2);
  expected.push_back(&itin3);

  _calculator->removeItinsWithNoBrands();
  ASSERT_EQ(expected, _trx->itin());
}

TEST_F(TrxGeometryCalculatorTest, testCorrectlyReturnsFixedLegsInformation)
{
  std::vector<bool> fixInfo;
  fixInfo.push_back(true);
  fixInfo.push_back(false);
  fixInfo.push_back(false);
  fixInfo.push_back(true);
  fixInfo.push_back(true);
  fixInfo.push_back(false);
  _trx->getMutableFixedLegs() = fixInfo;

  ASSERT_EQ(fixInfo, _calculator->getFixedLegs());
}

TEST_F(TrxGeometryCalculatorTest, testCorrectlyReturnsContextShoppingInfoForNotExistingSegment)
{
  ASSERT_EQ(static_cast<FareComponentShoppingContext*>(0),
      _calculator->getFareComponentShoppingContext(0));
}

TEST_F(TrxGeometryCalculatorTest, testCorrectlyReturnsContextShoppingInfoForExistingSegment)
{
  const size_t DUMMY_SEGMENT_INDEX = 42;
  _shoppingContext.reset(new FareComponentShoppingContext());
  FareComponentShoppingContextsForSegments shoppingContextMap;
  shoppingContextMap[DUMMY_SEGMENT_INDEX] = _shoppingContext.get();
  _trx->getMutableFareComponentShoppingContexts() = shoppingContextMap;

  ASSERT_EQ(_shoppingContext.get(),
    _calculator->getFareComponentShoppingContext(DUMMY_SEGMENT_INDEX));
}


} // namespace skipper

} // namespace tse
