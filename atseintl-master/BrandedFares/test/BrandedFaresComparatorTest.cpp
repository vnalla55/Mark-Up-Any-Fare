//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Pricing/PricingOrchestrator.h"
#include "Common/Logger.h"
#include "BrandedFares/BrandedFaresComparator.h"

namespace tse
{
  class Logger;

  class BrandedFaresComparatorTest : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(BrandedFaresComparatorTest);
    CPPUNIT_TEST(testAcyclicGraphOneProgram);
    CPPUNIT_TEST(testAcyclicGraphTwoPrograms);
    CPPUNIT_TEST(testCyclicGraph);
    CPPUNIT_TEST(singleBrandInProgram);
    CPPUNIT_TEST(brandsNotInTrx);
    CPPUNIT_TEST_SUITE_END();
  public:

      void setUp()
      {
        brandInfo1.brandCode() = "SV";
        brandInfo2.brandCode() = "SL";
        brandInfo3.brandCode() = "FL";
        brandInfo4.brandCode() = "RB";
        brandInfo5.brandCode() = "RZ";
        brandInfo6.brandCode() = "DT";

      }
      void teatDown() {}

    void testAcyclicGraphOneProgram()
    {
      Logger _logger("atseintl.Pricing.Shopping.IBF.BrandedFaresComparator");

      std::vector<QualifiedBrand> brandProgramVec;
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo1));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo2));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo3));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo4));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo5));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo6));


      BrandCode sortedBrands[] = { "SV" , "SL" , "FL" , "RB" , "RZ" , "DT" };
      std::vector<BrandCode> sortedBrandsVec(sortedBrands, sortedBrands +6 );
      BrandCode brands[] = { "RZ" , "SV" , "SL" , "DT" , "RB" , "FL" };
      std::vector<BrandCode> brandsVec(brands, brands + 6);

      BrandedFaresComparator brandComparator(brandProgramVec, _logger);
      CPPUNIT_ASSERT(brandsVec != sortedBrandsVec);
      std::sort(brandsVec.begin(), brandsVec.end(), brandComparator);
      CPPUNIT_ASSERT(brandsVec == sortedBrandsVec);

    }
    void testAcyclicGraphTwoPrograms()
    {
      Logger _logger("atseintl.Pricing.Shopping.IBF.BrandedFaresComparator");
      std::vector<QualifiedBrand> brandProgramVec;
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo2));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo3));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo4));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo6));

      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo1));
      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo3));
      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo4));
      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo5));
      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo6));

      BrandCode sortedBrands[] = { "SV" , "SL" , "FL" , "RB" , "RZ" , "DT" };
      std::vector<BrandCode> sortedBrandsVec(sortedBrands, sortedBrands +6 );
      BrandCode brands[] = { "SV" , "RZ" , "FL" , "SL" , "DT" , "RB" };
      std::vector<BrandCode> brandsVec(brands, brands + 6);

      BrandedFaresComparator brandComparator(brandProgramVec, _logger);
      CPPUNIT_ASSERT(brandsVec != sortedBrandsVec);
      std::sort(brandsVec.begin(), brandsVec.end(), brandComparator);
      CPPUNIT_ASSERT(brandsVec == sortedBrandsVec);
    }
    void testCyclicGraph()
    {
      // With cyclic graph it is impossible to create a topological order of brands
      // in this case we should return brands in the order they were added to the trx
      // ( brands from program 1 first and then those from program 2 that didn't
      // appear in program 1
      Logger _logger("atseintl.Pricing.Shopping.IBF.BrandedFaresComparator");
      std::vector<QualifiedBrand> brandProgramVec;
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo6));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo5));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo4));
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo3));

      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo1));
      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo2));
      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo5));
      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo6));

      BrandCode sortedBrands[] = { "DT" , "RZ" , "RB" , "FL" , "SV" , "SL" };
      std::vector<BrandCode> sortedBrandsVec(sortedBrands, sortedBrands +6 );
      BrandCode brands[] = { "SV" , "RZ" , "FL" , "SL" , "DT" , "RB" };
      std::vector<BrandCode> brandsVec(brands, brands + 6);

      BrandedFaresComparator brandComparator(brandProgramVec, _logger);
      CPPUNIT_ASSERT(brandsVec != sortedBrandsVec);
      std::sort(brandsVec.begin(), brandsVec.end(), brandComparator);
      CPPUNIT_ASSERT(brandsVec == sortedBrandsVec);
    }
    void singleBrandInProgram()
    {
      Logger _logger("atseintl.Pricing.Shopping.IBF.BrandedFaresComparator");
      std::vector<QualifiedBrand> brandProgramVec;
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo6));

      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo1));
      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo3));

      BrandCode sortedBrands[] = { "SV" , "FL", "DT" };
      std::vector<BrandCode> sortedBrandsVec(sortedBrands, sortedBrands + 3 );
      BrandCode brands[] = { "SV" , "DT" , "FL" };
      std::vector<BrandCode> brandsVec(brands, brands + 3);

      BrandedFaresComparator brandComparator(brandProgramVec, _logger);
      CPPUNIT_ASSERT(brandsVec != sortedBrandsVec);
      std::sort(brandsVec.begin(), brandsVec.end(), brandComparator);
      CPPUNIT_ASSERT(brandsVec == sortedBrandsVec);
    }
    void brandsNotInTrx()
    {
      // Brands that are not in the vector that is sent to the comparator's constructor
      // should be sorted alphabetically and should appear at the end
      Logger _logger("atseintl.Pricing.Shopping.IBF.BrandedFaresComparator");
      std::vector<QualifiedBrand> brandProgramVec;
      brandProgramVec.push_back(std::make_pair(&program1, &brandInfo6));

      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo1));
      brandProgramVec.push_back(std::make_pair(&program2, &brandInfo3));

      BrandCode sortedBrands[] = { "SV" , "FL", "DT", "XX" , "XY" , "ZZ" };
      std::vector<BrandCode> sortedBrandsVec(sortedBrands, sortedBrands + 6 );
      BrandCode brands[] = { "FL" , "ZZ" , "SV" , "XY" , "XX", "DT"};
      std::vector<BrandCode> brandsVec(brands, brands + 6);

      BrandedFaresComparator brandComparator(brandProgramVec, _logger);
      CPPUNIT_ASSERT(brandsVec != sortedBrandsVec);
      std::sort(brandsVec.begin(), brandsVec.end(), brandComparator);
      CPPUNIT_ASSERT(brandsVec == sortedBrandsVec);
    }
private:
  BrandInfo brandInfo1, brandInfo2, brandInfo3, brandInfo4, brandInfo5, brandInfo6;
  BrandProgram program1, program2;
  };
CPPUNIT_TEST_SUITE_REGISTRATION(BrandedFaresComparatorTest);
}
