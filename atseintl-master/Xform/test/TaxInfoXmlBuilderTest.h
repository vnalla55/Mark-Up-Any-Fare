#ifndef TAX_INFO_XML_BUILDER_TEST_H
#define TAX_INFO_XML_BUILDER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class TaxInfoXmlBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxInfoXmlBuilderTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testBuildMisingTaxCodes);
  CPPUNIT_TEST(testBuildPfcForDFW);
  CPPUNIT_TEST(testBuildPfcForKRK);
  CPPUNIT_TEST(testBuildPfcForNYC);
  CPPUNIT_TEST(testBuildPfcForDFWJFK);
  CPPUNIT_TEST(testBuildPfcForDFWKRKJFKNYC);
  CPPUNIT_TEST(testBuildPfcNoAirports);
  CPPUNIT_TEST(testBuildZpForDFW);
  CPPUNIT_TEST(testBuildZpForKRK);
  CPPUNIT_TEST(testBuildZpForNYC);
  CPPUNIT_TEST(testBuildZpForNYCJFK);
  CPPUNIT_TEST(testBuildZpForDFWJFK);
  CPPUNIT_TEST(testBuildZpForABIDFW);
  CPPUNIT_TEST(testBuildZpForDFWKRKJFKNYC);
  CPPUNIT_TEST(testBuildZpForDFWORDDENLAX);
  CPPUNIT_TEST(testBuildZpNoAirports);
  CPPUNIT_TEST(testBuildZpPfc);

  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor();
  void testBuildMisingTaxCodes();
  void testBuildPfcForDFW();
  void testBuildPfcForKRK();
  void testBuildPfcForNYC();
  void testBuildPfcForDFWJFK();
  void testBuildPfcForDFWKRKJFKNYC();
  void testBuildPfcNoAirports();
  void testBuildZpForDFW();
  void testBuildZpForKRK();
  void testBuildZpForNYC();
  void testBuildZpForNYCJFK();
  void testBuildZpForDFWJFK();
  void testBuildZpForABIDFW();
  void testBuildZpForDFWKRKJFKNYC();
  void testBuildZpForDFWORDDENLAX();
  void testBuildZpNoAirports();
  void testBuildZpPfc();
};

#endif
