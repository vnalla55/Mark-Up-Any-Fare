#ifndef TEST_FACTORY_TEST_H
#define TEST_FACTORY_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include "test/include/MockDataManager.h"

class TestFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFactoryTest);
  CPPUNIT_TEST(testTestAddonFareInfoFactory);
  CPPUNIT_TEST(testTestAirSegFactory);
  CPPUNIT_TEST(testTestCarrierPreferenceFactory);
  CPPUNIT_TEST(testTestCombinabilityRuleInfoFactory);
  //    CPPUNIT_TEST(testTestConstructedFareBucketFactory);
  CPPUNIT_TEST(testTestDifferentialDataFactory);
  CPPUNIT_TEST(testTestFareFactory);
  CPPUNIT_TEST(testTestFareClassAppInfoFactory);
  CPPUNIT_TEST(testTestFareClassAppSegInfoFactory);
  CPPUNIT_TEST(testTestFareInfoFactory);
  CPPUNIT_TEST(testTestFareMarketFactory);
  CPPUNIT_TEST(testTestFareUsageFactory);
  CPPUNIT_TEST(testTestLocFactory);
  CPPUNIT_TEST(testTestPaxTypeFactory);
  CPPUNIT_TEST(testTestPaxTypeBucketFactory);
  CPPUNIT_TEST(testTestPaxTypeFareFactory);
  CPPUNIT_TEST(testTestPaxTypeInfoFactory);
  CPPUNIT_TEST(testTestPfcItemFactory);
  CPPUNIT_TEST(testTestPricingUnitFactory);
  CPPUNIT_TEST(testTestTariffCrossRefInfoFactory);
  CPPUNIT_TEST(testTestTaxCodeRegFactory);
  CPPUNIT_TEST(testTestTaxItemFactory);
  CPPUNIT_TEST(testTestTaxRecordFactory);
  CPPUNIT_TEST(testTestTaxResponseFactory);
  CPPUNIT_TEST(testTestTaxResponseFactory);
  CPPUNIT_TEST_SUITE_END();

public:
  void testTestAddonFareInfoFactory();
  void testTestAirSegFactory();
  void testTestCarrierPreferenceFactory();
  void testTestCombinabilityRuleInfoFactory();
  // void testTestConstructedFareBucketFactory();
  void testTestDifferentialDataFactory();
  void testTestFareFactory();
  void testTestFareClassAppInfoFactory();
  void testTestFareClassAppSegInfoFactory();
  void testTestFareInfoFactory();
  void testTestFareMarketFactory();
  void testTestFareUsageFactory();
  void testTestLocFactory();
  void testTestPaxTypeFactory();
  void testTestPaxTypeBucketFactory();
  void testTestPaxTypeFareFactory();
  void testTestPaxTypeInfoFactory();
  void testTestPfcItemFactory();
  void testTestPricingUnitFactory();
  void testTestTariffCrossRefInfoFactory();
  void testTestTaxCodeRegFactory();
  void testTestTaxItemFactory();
  void testTestTaxRecordFactory();
  void testTestTaxResponseFactory();

  void setUp();
  void tearDown();
};

#endif
