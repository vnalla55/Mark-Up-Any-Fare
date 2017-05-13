#include "test/include/CppUnitHelperMacros.h"
#include "ServiceFees/OCFees.h"

namespace tse
{

class OCFeesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OCFeesTest);

  CPPUNIT_TEST(testOcTaxItem);
  CPPUNIT_TEST(testNoTaxItemInOcFees);
  CPPUNIT_TEST(testTwoTaxItemsInOcFees);

  CPPUNIT_TEST_SUITE_END();

public:

  void testOcTaxItem()
  {
    OCFees::TaxItem item;
    item.setTaxCode("NZ");
    item.setTaxAmount(10.10);

    CPPUNIT_ASSERT(item.getTaxCode() == "NZ" && item.getTaxAmount() == 10.10);
  }

  void testNoTaxItemInOcFees()
  {
    OCFees ocfees;

    CPPUNIT_ASSERT(ocfees.getTaxes().size() == 0);
  }

  void testTwoTaxItemsInOcFees()
  {
    OCFees ocfees;
    OCFees::TaxItem item1;
    OCFees::TaxItem item2;
    ocfees.addTax(item1);
    ocfees.addTax(item2);

    CPPUNIT_ASSERT(ocfees.getTaxes().size() == 2);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OCFeesTest);
}
