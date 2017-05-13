//-------------------------------------------------------------------
//
//  File:        BrandedFaresUtilTest.cpp
//  Created:     April 2014
//  Authors:
//
//  Description:
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

#include "test/include/CppUnitHelperMacros.h"

#include "test/include/TestMemHandle.h"
#include "BrandedFares/BrandedFaresUtil.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/AirSeg.h"


namespace tse
{

class BrandedFaresUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandedFaresUtilTest);
  CPPUNIT_TEST(testSortFmByBrands_0Fm);
  CPPUNIT_TEST(testSortFmByBrands_1Fm);
  CPPUNIT_TEST(testSortFmByBrands_3GropuFm);
  CPPUNIT_TEST(testSortFmByBrands_2GropuFm);
  CPPUNIT_TEST(testSortFmByBrands_4GropuFm);

  CPPUNIT_TEST(testBrandCode_1FM_noBrands);
  CPPUNIT_TEST(testBrandCode_1FM_noData);
  CPPUNIT_TEST(testBrandCode_1FM_1Brand);
  CPPUNIT_TEST(testBrandCode_1FM_manyBrand);
  CPPUNIT_TEST(testBrandCode_3FM_noBrands);
  CPPUNIT_TEST(testBrandCode_3FM_noData);
  CPPUNIT_TEST(testBrandCode_3FM_1Brand);

  CPPUNIT_TEST(testBrandProgram_1FM_noPrograms);
  CPPUNIT_TEST(testBrandProgram_1FM_manyPrograms);
  CPPUNIT_TEST(testBrandProgram_3FM_noPrograms);
  CPPUNIT_TEST(testBrandProgram_3FM_withPrograms);

  CPPUNIT_TEST(testBrandProgram_validateBrandProgramTrue);
  CPPUNIT_TEST(testBrandProgram_validateBrandProgramFalse);

  CPPUNIT_TEST(testBrandProgram_validateNoBrandProgram);
  CPPUNIT_TEST(testBrandProgram_validateInvalidBrandCode);
  CPPUNIT_TEST(testBrandProgram_validateOK);

  CPPUNIT_TEST(testBrandAndProgram_anyBrandedMarket);
  CPPUNIT_TEST(testBrandAndProgram_marketIDVecIsEmpty);
  CPPUNIT_TEST(testBrandAndProgram_firstMarketWithoutBrandedMarket);
  CPPUNIT_TEST(testBrandAndProgram_marketIDVecWithNonExistentBrandedMarket);


  CPPUNIT_TEST(testBrandProgram_validateProgramFirst);
  CPPUNIT_TEST(testBrandProgram_validateBrandFirst);

  CPPUNIT_TEST_SUITE_END();


public:
  void setUp()
  {
   PricingTrx::BrandedMarketMap* brandedMarket = _memHandle.create<PricingTrx::BrandedMarketMap>();
    _brandAndProg = _memHandle.insert(new BrandAndProgramValidator(*brandedMarket));
  }

  void tearDown()
  {

  }

  FareMarket* makeFm(std::string brand)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    AirSeg* ts = _memHandle.create<AirSeg>();
    if (!brand.empty())
      ts->setBrandCode(brand);

    fm->travelSeg().push_back(ts);
    return fm;
  }

  FareMarket* makeFm(std::string brand, int marketIDVec)
  {
    FareMarket* fm = makeFm(brand);
    fm->marketIDVec().push_back(marketIDVec);
    return fm;
  }

  std::vector<FareMarket*> makeFms(std::string brand, int id1, int id2, int id3)
  {
    std::vector<FareMarket*> fms;
    for (int i = 0; i < 3; i++)
    {
      FareMarket* fm = makeFm(brand, id1);
      fm->marketIDVec().push_back(id2);
      fm->marketIDVec().push_back(id3);
      fms.push_back(fm);
    }
    return fms;
  }


  enum BuildDetailEnum
  {
    BuildAll,
    NoProgram,
    NoBrandInfo,
    NoDumyData,
    NoDumyAndBrandInfo
  };

  BrandInfo* buildBrandInfo(std::string brand)
  {
    BrandInfo* bi = _memHandle.create<BrandInfo>();
    bi->brandCode() = brand;
    return bi;
  }

  BrandProgram* buildBrandProgram(std::string brand, BuildDetailEnum bde = BuildAll)
  {
    BrandProgram* bp = _memHandle.create<BrandProgram>();
    if (bde != NoBrandInfo && bde != NoDumyAndBrandInfo)
    {
      bp->brandsData().push_back(buildBrandInfo(brand));

      if (bde != NoDumyData )
        bp->brandsData().push_back(buildBrandInfo("XX"));
    }
    return bp;
  }

  std::vector<MarketResponse*> buildMarketResponse(std::string brand, BuildDetailEnum bde = BuildAll)
  {
    MarketResponse* mr = _memHandle.create<MarketResponse>();
    if (bde != NoProgram)
    {
      mr->brandPrograms().push_back(buildBrandProgram(brand, bde));
      if (bde != NoDumyData && bde != NoDumyAndBrandInfo)
        mr->brandPrograms().push_back(buildBrandProgram("XX", bde));
    }

    std::vector<MarketResponse*> markets;
    markets.push_back(mr);
    return markets;
  }


 void testSortFmByBrands_0Fm()
 {
   std::vector<FareMarket*> fms;
   BrandAndProgramValidator::SortedMarket sfm = _brandAndProg->sortFmByBrands(fms);

   CPPUNIT_ASSERT(sfm.empty());
 }

 void testSortFmByBrands_1Fm()
 {
   std::vector<FareMarket*> fms;
   fms.push_back(makeFm("AB"));
   BrandAndProgramValidator::SortedMarket sfm = _brandAndProg->sortFmByBrands(fms);

   CPPUNIT_ASSERT_EQUAL((int)sfm.size(), 1);
   CPPUNIT_ASSERT_EQUAL((int)sfm[0].size(), 1);
   CPPUNIT_ASSERT_EQUAL(sfm[0].front()->getBrandCode(), std::string("AB"));
 }

 void testSortFmByBrands_3GropuFm()
 {
   std::vector<FareMarket*> fms;
   fms.push_back(makeFm("AC"));
   fms.push_back(makeFm("AB"));
   fms.push_back(makeFm("AD"));
   BrandAndProgramValidator::SortedMarket sfm = _brandAndProg->sortFmByBrands(fms);

   CPPUNIT_ASSERT_EQUAL((int)sfm.size(), 3);
   CPPUNIT_ASSERT_EQUAL((int)sfm[0].size(), 1);
   CPPUNIT_ASSERT_EQUAL((int)sfm[1].size(), 1);
   CPPUNIT_ASSERT_EQUAL((int)sfm[2].size(), 1);

   CPPUNIT_ASSERT_EQUAL(sfm[0].front()->getBrandCode(), std::string("AB"));
   CPPUNIT_ASSERT_EQUAL(sfm[1].front()->getBrandCode(), std::string("AC"));
   CPPUNIT_ASSERT_EQUAL(sfm[2].front()->getBrandCode(), std::string("AD"));
 }

 void testSortFmByBrands_2GropuFm()
 {
   std::vector<FareMarket*> fms;
   fms.push_back(makeFm("DD"));
   fms.push_back(makeFm("AF"));
   fms.push_back(makeFm("AF"));
   fms.push_back(makeFm("AF"));
   fms.push_back(makeFm("DD"));
   fms.push_back(makeFm("AF"));
   BrandAndProgramValidator::SortedMarket sfm = _brandAndProg->sortFmByBrands(fms);

   CPPUNIT_ASSERT_EQUAL((int)sfm.size(), 2);
   CPPUNIT_ASSERT_EQUAL((int)sfm[0].size(), 4);
   CPPUNIT_ASSERT_EQUAL((int)sfm[1].size(), 2);

   CPPUNIT_ASSERT_EQUAL(sfm[0].front()->getBrandCode(), std::string("AF"));
   CPPUNIT_ASSERT_EQUAL(sfm[1].front()->getBrandCode(), std::string("DD"));
 }


 void testSortFmByBrands_4GropuFm()
 {
   std::vector<FareMarket*> fms;
   fms.push_back(makeFm("AF"));
   fms.push_back(makeFm("DD"));
   fms.push_back(makeFm("AF"));
   fms.push_back(makeFm(""));
   fms.push_back(makeFm("DD"));
   fms.push_back(makeFm("AF"));
   fms.push_back(makeFm("ZZ"));
   fms.push_back(makeFm(""));

   BrandAndProgramValidator::SortedMarket sfm = _brandAndProg->sortFmByBrands(fms);

   CPPUNIT_ASSERT_EQUAL((int)sfm.size(), 4);
   CPPUNIT_ASSERT_EQUAL((int)sfm[0].size(), 2);
   CPPUNIT_ASSERT_EQUAL((int)sfm[1].size(), 3);
   CPPUNIT_ASSERT_EQUAL((int)sfm[2].size(), 2);
   CPPUNIT_ASSERT_EQUAL((int)sfm[3].size(), 1);

   CPPUNIT_ASSERT_EQUAL(sfm[0].front()->getBrandCode(), std::string(""));
   CPPUNIT_ASSERT_EQUAL(sfm[1].front()->getBrandCode(), std::string("AF"));
   CPPUNIT_ASSERT_EQUAL(sfm[2].front()->getBrandCode(), std::string("DD"));
   CPPUNIT_ASSERT_EQUAL(sfm[3].front()->getBrandCode(), std::string("ZZ"));
 }


 void testBrandCode_1FM_noBrands()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[0] = buildMarketResponse("YY");

   std::vector<FareMarket*> fms;
   fms.push_back(makeFm("AF", 0));

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_THROW(brandAndProgramValidator.validateBrandCode(fms), ErrorResponseException);
 }

 void testBrandCode_1FM_noData()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[1] = buildMarketResponse("YY", NoBrandInfo);

   std::vector<FareMarket*> fms;
   fms.push_back(makeFm("AF", 1));

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_THROW(brandAndProgramValidator.validateBrandCode(fms), ErrorResponseException);
 }

 void testBrandCode_1FM_1Brand()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[1] = buildMarketResponse("FL", NoDumyData);

   std::vector<FareMarket*> fms;
   fms.push_back(makeFm("FL", 1));

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_NO_THROW(brandAndProgramValidator.validateBrandCode(fms));
 }

 void testBrandCode_1FM_manyBrand()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[1] = buildMarketResponse("AB");
   brandedMarketMap[3] = buildMarketResponse("AA");
   brandedMarketMap[7] = buildMarketResponse("FL");

   std::vector<FareMarket*> fms;
   FareMarket* fm = makeFm("FL", 1);
   fm->marketIDVec().push_back(3);
   fm->marketIDVec().push_back(7);
   fms.push_back(fm);

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_NO_THROW(brandAndProgramValidator.validateBrandCode(fms));
 }

 void testBrandCode_3FM_noBrands()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[1] = buildMarketResponse("AB");
   brandedMarketMap[3] = buildMarketResponse("AA");
   brandedMarketMap[7] = buildMarketResponse("FV");

   std::vector<FareMarket*> fms = makeFms("FV", 1, 3, 1);
   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_THROW(brandAndProgramValidator.validateBrandCode(fms), ErrorResponseException);
 }

 void testBrandCode_3FM_noData()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[1] = buildMarketResponse("XX", NoBrandInfo);
   brandedMarketMap[3] = buildMarketResponse("XX", NoBrandInfo);
   brandedMarketMap[7] = buildMarketResponse("XX", NoBrandInfo);

   std::vector<FareMarket*> fms = makeFms("FV", 1, 3, 7);
   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_THROW(brandAndProgramValidator.validateBrandCode(fms), ErrorResponseException);
 }

 void testBrandCode_3FM_1Brand()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[3] = buildMarketResponse("AA");
   brandedMarketMap[7] = buildMarketResponse("FV");

   std::vector<FareMarket*> fms = makeFms("FV", 3, 3, 7);
   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_NO_THROW(brandAndProgramValidator.validateBrandCode(fms));
 }

 void testBrandProgram_1FM_noPrograms()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[0] = buildMarketResponse("YY", NoProgram);

   std::vector<FareMarket*> fms;
   fms.push_back(makeFm("AF", 0));

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_THROW(brandAndProgramValidator.validateBrandProgram(fms), ErrorResponseException);
 }

 void testBrandProgram_1FM_manyPrograms()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[0] = buildMarketResponse("YY");
   brandedMarketMap[5] = buildMarketResponse("ZZ");
   brandedMarketMap[6] = buildMarketResponse("BB");

   std::vector<FareMarket*> fms;
   FareMarket* fm = makeFm("AF", 0);
   fm->marketIDVec().push_back(5);
   fm->marketIDVec().push_back(6);
   fms.push_back(fm);

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_NO_THROW(brandAndProgramValidator.validateBrandProgram(fms));
 }

 void testBrandProgram_3FM_noPrograms()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[1] = buildMarketResponse("XX", NoProgram);
   brandedMarketMap[3] = buildMarketResponse("XX", NoProgram);
   brandedMarketMap[7] = buildMarketResponse("XX");

   std::vector<FareMarket*> fms = makeFms("FV", 1, 3, 3);
   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_THROW(brandAndProgramValidator.validateBrandProgram(fms), ErrorResponseException);
 }

 void testBrandProgram_3FM_withPrograms()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[1] = buildMarketResponse("XX", NoProgram);
   brandedMarketMap[3] = buildMarketResponse("XX", NoProgram);
   brandedMarketMap[7] = buildMarketResponse("XX");

   std::vector<FareMarket*> fms = makeFms("FV", 1, 3, 7);
   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_NO_THROW(brandAndProgramValidator.validateBrandProgram(fms));
 }

 void testBrandProgram_validateBrandProgramTrue()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[0] = buildMarketResponse("YY", NoProgram);
   brandedMarketMap[5] = buildMarketResponse("ZZ");
   brandedMarketMap[6] = buildMarketResponse("BB", NoProgram);

   FareMarket* fm = makeFm("AF", 0);
   fm->marketIDVec().push_back(5);
   fm->marketIDVec().push_back(6);

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT(brandAndProgramValidator.validateBrandProgram(*fm));
 }

 void testBrandProgram_validateBrandProgramFalse()
 {
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[0] = buildMarketResponse("YY", NoProgram);
   FareMarket* fm = makeFm("AF", 0);

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT(!brandAndProgramValidator.validateBrandProgram(*fm));
 }

void testBrandProgram_validateNoBrandProgram()
{
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[0] = buildMarketResponse("AC", NoProgram);
   brandedMarketMap[1] = buildMarketResponse("AB");

   FareMarket* fm1 = makeFm("AC", 0);
   fm1->marketIDVec().push_back(0);
   FareMarket* fm2 = makeFm("AB", 0);
   fm2->marketIDVec().push_back(1);

   std::vector<FareMarket*> fms;
   fms.push_back(fm1);
   fms.push_back(fm2);

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   try
   {
      brandAndProgramValidator.validate(fms);
   }
   catch (ErrorResponseException e)
   {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::REQUESTED_BRAND_NOT_FOUND);
       return ;
   }
   catch (...)
   {
      CPPUNIT_FAIL("Bad exception caught");
   }
   CPPUNIT_FAIL("Any exception caught");
}

void testBrandProgram_validateInvalidBrandCode()
{
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[0] = buildMarketResponse("AC");
   brandedMarketMap[1] = buildMarketResponse("AB", NoBrandInfo);

   FareMarket* fm1 = makeFm("AC", 0);
   fm1->marketIDVec().push_back(0);
   FareMarket* fm2 = makeFm("AB", 0);
   fm2->marketIDVec().push_back(1);

   std::vector<FareMarket*> fms;
   fms.push_back(fm1);
   fms.push_back(fm2);

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   try
   {
      brandAndProgramValidator.validate(fms);
   }
   catch (ErrorResponseException e)
   {
      CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::NO_VALID_BRAND_FOUND);
      return ;
   }
   catch (...)
   {
      CPPUNIT_FAIL("Bad exception caught");
   }
   CPPUNIT_FAIL("Any exception caught");
}

void testBrandProgram_validateOK()
{
   PricingTrx::BrandedMarketMap brandedMarketMap;
   brandedMarketMap[0] = buildMarketResponse("AC");
   brandedMarketMap[1] = buildMarketResponse("AB");

   FareMarket* fm1 = makeFm("AC", 0);
   fm1->marketIDVec().push_back(0);
   FareMarket* fm2 = makeFm("AB", 0);
   fm2->marketIDVec().push_back(1);

   std::vector<FareMarket*> fms;
   fms.push_back(fm1);
   fms.push_back(fm2);

   BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
   CPPUNIT_ASSERT_NO_THROW(brandAndProgramValidator.validate(fms));
}

void testBrandAndProgram_anyBrandedMarket()
{
  PricingTrx::BrandedMarketMap brandedMarketMap;

  FareMarket* fm1 = makeFm("AB");
  FareMarket* fm2 = makeFm("AC");

  std::vector<FareMarket*> fms;
  fms.push_back(fm1);
  fms.push_back(fm2);

  BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
  try
  {
    brandAndProgramValidator.validateBrandProgram(fms);
  }
  catch (ErrorResponseException e)
  {
    CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::REQUESTED_BRAND_NOT_FOUND);
    return ;
  }
  catch (...)
  {
    CPPUNIT_FAIL("Bad exception caught");
  }
  CPPUNIT_FAIL("Any exception caught");
}

void testBrandAndProgram_marketIDVecIsEmpty()
{
  PricingTrx::BrandedMarketMap brandedMarketMap;
  brandedMarketMap[0] = buildMarketResponse("AC");

  FareMarket* fm1 = makeFm("AB");
  FareMarket* fm2 = makeFm("AC");

  std::vector<FareMarket*> fms;
  fms.push_back(fm1);
  fms.push_back(fm2);

  BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
  try
  {
    brandAndProgramValidator.validateBrandProgram(fms);
  }
  catch (ErrorResponseException e)
  {
    CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::REQUESTED_BRAND_NOT_FOUND);
    return ;
  }
  catch (...)
  {
    CPPUNIT_FAIL("Bad exception caught");
  }
  CPPUNIT_FAIL("Any exception caught");
}

void testBrandAndProgram_firstMarketWithoutBrandedMarket()
{
  PricingTrx::BrandedMarketMap brandedMarketMap;
  brandedMarketMap[0] = buildMarketResponse("AC");

  FareMarket* fm1 = makeFm("AB");
  FareMarket* fm2 = makeFm("AC");
  fm2->marketIDVec().push_back(0);

  std::vector<FareMarket*> fms;
  fms.push_back(fm1);
  fms.push_back(fm2);

  BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
  CPPUNIT_ASSERT_NO_THROW(brandAndProgramValidator.validateBrandProgram(fms));
}

void testBrandAndProgram_marketIDVecWithNonExistentBrandedMarket()
{
  PricingTrx::BrandedMarketMap brandedMarketMap;

  FareMarket* fm1 = makeFm("AB");
  FareMarket* fm2 = makeFm("AC");
  fm2->marketIDVec().push_back(0);

  std::vector<FareMarket*> fms;
  fms.push_back(fm1);
  fms.push_back(fm2);

  BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
  try
  {
    brandAndProgramValidator.validateBrandProgram(fms);
  }
  catch (ErrorResponseException e)
  {
    CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::REQUESTED_BRAND_NOT_FOUND);
    return ;
  }
  catch (...)
  {
    CPPUNIT_FAIL("Bad exception caught");
  }
  CPPUNIT_FAIL("Any exception caught");
}

void testBrandProgram_validateProgramFirst()
{
  PricingTrx::BrandedMarketMap brandedMarketMap;
  brandedMarketMap[0] = buildMarketResponse("AC", NoProgram);
  brandedMarketMap[1] = buildMarketResponse("AD", NoBrandInfo);

  std::vector<FareMarket*> fms;
  fms.push_back(makeFm("AC", 0));
  fms.push_back(makeFm("AD", 1));

  try
  {
     BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
     brandAndProgramValidator.validate(fms);
  }
  catch (ErrorResponseException e)
  {
     CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::REQUESTED_BRAND_NOT_FOUND);
  }
  catch (...)
  {
     CPPUNIT_FAIL("Bad exception caught");
  }
}

void testBrandProgram_validateBrandFirst()
{
  PricingTrx::BrandedMarketMap brandedMarketMap;
  brandedMarketMap[0] = buildMarketResponse("AC", NoBrandInfo);
  brandedMarketMap[1] = buildMarketResponse("AD", NoProgram);

  std::vector<FareMarket*> fms;
  fms.push_back(makeFm("AC", 0));
  fms.push_back(makeFm("AD", 1));

  try
  {
     BrandAndProgramValidator brandAndProgramValidator(brandedMarketMap);
     brandAndProgramValidator.validate(fms);
  }
  catch (ErrorResponseException e)
  {
     CPPUNIT_ASSERT_EQUAL(e.code(), ErrorResponseException::REQUESTED_BRAND_NOT_FOUND);
  }
  catch (...)
  {
     CPPUNIT_FAIL("Bad exception caught");
  }
}


private:
 TestMemHandle _memHandle;
 BrandAndProgramValidator* _brandAndProg;

};
CPPUNIT_TEST_SUITE_REGISTRATION(BrandedFaresUtilTest);
}
