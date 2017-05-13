#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Category15.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"

#include "test/include/CppUnitHelperMacros.h"

#include <ctime>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

namespace tse
{

class TaxDisplayCAT15Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT15Test);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testGroup1);
  CPPUNIT_TEST(testGroup2);
  CPPUNIT_TEST(testGroup3);
  CPPUNIT_TEST(testGroup4);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCreation()
  {
    std::unique_ptr<Category15> cat15(new Category15);
    CPPUNIT_ASSERT(0 != cat15);
  }

  void testGroup1()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;

    Category15 cat15;

    taxCodeReg.interlinableTaxInd() = Indicator('N');
    cat15.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("     NO MISCELANEOUS INFORMATION RESTRICTIONS APPLY.\n"),
                         cat15.subCat1());

    taxCodeReg.interlinableTaxInd() = Indicator('Y');
    cat15.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TAX AMOUNT IS INTERLINEABLE.\n"), cat15.subCat1());
  }

  void testGroup2()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;

    Category15 cat15;

    taxCodeReg.feeInd() = Indicator('N');
    cat15.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(EMPTY_STRING(), cat15.subCat2());

    taxCodeReg.feeInd() = Indicator('Y');
    cat15.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TAX DISPLAYS AS FEE IN FT MESSAGE.\n"), cat15.subCat2());
  }

  void testGroup3()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;

    Category15 cat15;

    taxCodeReg.showseparateInd() = Indicator('N');
    cat15.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(EMPTY_STRING(), cat15.subCat3());

    taxCodeReg.showseparateInd() = Indicator('Y');
    cat15.build(trx, taxDisplayItem);
    CPPUNIT_ASSERT_EQUAL(std::string("* TAX NOT COMBINED WITH XT LOGIC.\n"), cat15.subCat3());
  }

  void testGroup4()
  {
    TaxTrx trx;

    TaxRequest request;
    trx.setRequest(&request);

    DateTime dt;
    trx.getRequest()->ticketingDT() = dt.localTime();

    Itin itin;
    trx.itin().push_back(&itin);

    TaxResponse taxResponse;
    trx.itin().front()->mutableTaxResponses().push_back(&taxResponse);

    TaxDisplayItem taxDisplayItem;
    TaxCodeReg taxCodeReg;
    taxDisplayItem.taxCodeReg() = &taxCodeReg;

    Category15 cat15;
    // TODO: why there is no ASSERT here??
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT15Test);
}
