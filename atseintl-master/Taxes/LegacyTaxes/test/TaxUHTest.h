#ifndef TAX_UH_TEST_H
#define TAX_UH_TEST_H

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

namespace tse
{

class PricingTrx;
class TaxResponse;

class TaxUHTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(TaxUHTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxUH_FromSVONoSU);
  CPPUNIT_TEST(testTaxUH_FromSVOWithSU);
  CPPUNIT_TEST(testTaxUH_NotSVO);
  CPPUNIT_TEST(testTaxUH_Loc2Blank);
  CPPUNIT_TEST_SUITE_END();

public:
  /**
   * Test the constructor.
   **/
  void testConstructor();
  void testTaxUH_FromSVONoSU();
  void testTaxUH_FromSVOWithSU();
  void testTaxUH_NotSVO();
  void testTaxUH_Loc2Blank();

  void setUp();
  void tearDown();

private:
  TestMemHandle _memHandle;
  tse::PricingTrx* _trx;
  tse::TaxResponse* _taxResponse;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUHTest);

}
#endif // TAX_UH_TEST.H
