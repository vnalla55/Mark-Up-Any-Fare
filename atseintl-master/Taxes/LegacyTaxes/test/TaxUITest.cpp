#include <iostream>
#include <string>
#include <time.h>
#include <vector>

#include "Common/TseCodeTypes.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxUI.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class TaxUITest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUITest);
  CPPUNIT_TEST(testApplyTaxOnTaxEmptyResponse);
  CPPUNIT_TEST(testApplyTaxOnTaxFR1QW);
  CPPUNIT_TEST(testApplyTaxOnTaxFR1US1);
  CPPUNIT_TEST(testIsNonFrOrYqTaxFR1EmptyAmt);
  CPPUNIT_TEST(testIsNonFrOrYqTaxFR1);
  CPPUNIT_TEST(testIsNonFrOrYqTaxUS1);
  CPPUNIT_TEST(testApplyTaxOnTaxYQYRNoFR1);
  CPPUNIT_TEST(testApplyTaxOnTaxYQYR);
  CPPUNIT_TEST(testApplyTaxOnTaxCalculationDetails);
  CPPUNIT_TEST(testApplyTaxOnTaxExternalYQYR);
  CPPUNIT_TEST_SUITE_END();

public:
  void testApplyTaxOnTaxEmptyResponse()
  {
    TaxUI taxUI;
    PricingTrx trx;
    PricingRequest request;
    trx.setRequest(&request);
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;

    taxUI.applyTaxOnTax(trx, taxResponse, taxCodeReg);
    CPPUNIT_ASSERT(!taxUI._taxAmount);
  }

  void testApplyTaxOnTaxFR1QW()
  {
    TaxUI taxUI;
    taxUI._taxableFare = 90;
    PricingTrx trx;
    PricingRequest request;
    trx.setRequest(&request);
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;
    taxCodeReg.taxAmt() = 4;
    std::vector<TaxItem*> taxV;
    taxResponse.taxItemVector() = taxV;

    TaxItem taxItem1;
    taxItem1.taxAmount() = 10;
    // TaxCodeReg  taxCodeReg1;
    // taxCodeReg1.taxCode() = "FR1";
    // taxCodeReg1.taxAmt() = 10.0;
    // taxItem1.taxCodeReg() = &taxCodeReg1;
    taxItem1.taxCode() = "FR1";
    taxItem1.taxAmt() = 10.0;
    taxV.push_back(&taxItem1);

    taxResponse.taxItemVector().push_back(&taxItem1);

    TaxItem taxItem2;
    taxItem2.taxAmount() = 10;
    // TaxCodeReg  taxCodeReg2;
    // taxCodeReg2.taxCode() = "QW";
    // taxCodeReg2.taxAmt() = 10.0;
    // taxItem2.taxCodeReg() = &taxCodeReg2;
    taxItem2.taxCode() = "QW";
    taxItem2.taxAmt() = 10.0;
    taxResponse.taxItemVector().push_back(&taxItem2);
    taxV.push_back(&taxItem2);

    taxUI.applyTaxOnTax(trx, taxResponse, taxCodeReg);
    CPPUNIT_ASSERT(taxUI._taxAmount);
  }

  void testApplyTaxOnTaxFR1US1()
  {
    TaxUI taxUI;
    taxUI._taxableFare = 90;
    PricingTrx trx;
    PricingRequest request;
    trx.setRequest(&request);
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;
    taxCodeReg.taxAmt() = 4;
    std::vector<TaxItem*> taxV;
    taxResponse.taxItemVector() = taxV;

    TaxItem taxItem1;
    taxItem1.taxAmount() = 10;

    // TaxCodeReg  taxCodeReg1;
    // taxCodeReg1.taxCode() = "FR2";
    // taxCodeReg1.taxAmt() = 10.0;
    // taxItem1.taxCodeReg() = &taxCodeReg1;
    taxItem1.taxCode() = "FR2";
    taxItem1.taxAmt() = 10.0;

    taxV.push_back(&taxItem1);
    taxResponse.taxItemVector().push_back(&taxItem1);

    TaxItem taxItem2;
    taxItem2.taxAmount() = 10;
    // TaxCodeReg taxCodeReg2;
    // taxCodeReg2.taxCode() = "US1";
    // taxCodeReg2.taxAmt() = 10.0;
    // taxItem2.taxCodeReg() = &taxCodeReg2;
    taxItem2.taxCode() = "US1";
    taxItem2.taxAmt() = 10.0;
    taxV.push_back(&taxItem2);

    taxUI.applyTaxOnTax(trx, taxResponse, taxCodeReg);
    CPPUNIT_ASSERT(!taxUI._taxAmount);
  }

  void testIsNonFrOrYqTaxFR1EmptyAmt()
  {
    TaxUI ui;
    TaxItem taxItem;
    taxItem.taxAmount() = 0;
    // TaxCodeReg taxcodeReg;
    // taxItem.taxCodeReg() = &taxcodeReg;
    taxItem.taxCode() = "FR1";
    CPPUNIT_ASSERT(!ui.isNonFrOrYqTax(&taxItem));
  }

  void testIsNonFrOrYqTaxFR1()
  {
    TaxUI ui;
    TaxItem taxItem;
    taxItem.taxAmount() = 10;
    // TaxCodeReg taxcodeReg;
    // taxItem.taxCodeReg() = &taxcodeReg;
    taxItem.taxCode() = "FR1";
    CPPUNIT_ASSERT(!ui.isNonFrOrYqTax(&taxItem));
  }

  void testIsNonFrOrYqTaxUS1()
  {
    TaxUI ui;
    TaxItem taxItem;
    taxItem.taxAmount() = 10;
    // TaxCodeReg taxcodeReg;
    // taxItem.taxCodeReg() = &taxcodeReg;
    taxItem.taxCode() = "US1";
    CPPUNIT_ASSERT(ui.isNonFrOrYqTax(&taxItem));
  }

  void testApplyTaxOnTaxYQYRNoFR1()
  {
    PricingTrx trx;
    TaxResponse taxResponse;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "UI";
    taxCodeReg.taxAmt() = 0.1;

    // TaxCodeReg taxCodeRegYQF;
    // taxCodeRegYQF.taxCode() = "YQF";

    TaxItem yqyrItem;
    // yqyrItem.taxCodeReg()= &taxCodeRegYQF;
    yqyrItem.taxCode() = "YQF";
    yqyrItem.taxAmount() = 100;
    taxResponse.taxItemVector().push_back(&yqyrItem);

    taxCodeReg.taxOnTaxCode().push_back(YQF);

    TaxUI tax;

    tax.applyTaxOnTax(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(tax.taxAmount() == 0);
  }

  void testApplyTaxOnTaxYQYR()
  {
    PricingTrx trx;
    TaxResponse taxResponse;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "UI";
    taxCodeReg.taxAmt() = 0.1;

    // TaxCodeReg taxCodeRegFR1;
    // taxCodeRegFR1.taxCode() = "FR1";
    // taxCodeRegFR1.taxAmt() = 0.1;

    TaxItem fr1Item;
    // fr1Item.taxCodeReg()= &taxCodeRegFR1;
    fr1Item.taxCode() = "FR1";
    fr1Item.taxAmt() = 0.1;
    fr1Item.taxAmount() = 100;
    taxResponse.taxItemVector().push_back(&fr1Item);

    // TaxCodeReg taxCodeRegYQF;
    // taxCodeRegYQF.taxCode() = "YQF";

    TaxItem yqyrItem;
    // yqyrItem.taxCodeReg()= &taxCodeRegYQF;
    yqyrItem.taxCode() = "YQF";
    yqyrItem.taxAmount() = 100;
    taxResponse.taxItemVector().push_back(&yqyrItem);

    taxCodeReg.taxOnTaxCode().push_back(YQF);

    TaxUI tax;

    tax.applyTaxOnTax(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(tax.taxAmount() == taxCodeReg.taxAmt() * yqyrItem.taxAmount() +
                                          taxCodeReg.taxAmt() * fr1Item.taxAmount());
  }
  void testApplyTaxOnTaxCalculationDetails()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;

    TaxItem fr1Item;
    fr1Item.taxCode() = "FR1";
    fr1Item.taxAmount() = 100;
    taxResponse.taxItemVector().push_back(&fr1Item);

    TaxItem yqfItem;

    yqfItem.taxCode() = "YQF";
    yqfItem.taxAmount() = 100;
    taxResponse.taxItemVector().push_back(&yqfItem);

    TaxUI tax;

    tax.applyTaxOnTax(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT(tax.calculationDetails().taxableTaxes.size() == 2);
    CPPUNIT_ASSERT(tax.calculationDetails().taxableTaxes.begin()->first == "FR1");
    CPPUNIT_ASSERT(tax.calculationDetails().taxableTaxes.begin()->second == 100);
    CPPUNIT_ASSERT(tax.calculationDetails().taxableTaxSumAmount == 200);
  }
  void testApplyTaxOnTaxExternalYQYR()
  {
    TaxTrx trx;
    trx.setShoppingPath(true);
    TaxResponse taxResponse;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "UI";
    taxCodeReg.taxAmt() = 0.1;

    TaxItem fr1Item;
    fr1Item.taxCode() = "FR1";
    fr1Item.taxAmt() = 0.1;
    fr1Item.taxAmount() = 100;
    taxResponse.taxItemVector().push_back(&fr1Item);

    TaxItem yqyrItem;
    yqyrItem.taxCode() = "YQF";
    yqyrItem.taxAmount() = 100;
    const NationCode nation("FR");
    Itin itin;
    FarePath farePath;
    farePath.getMutableExternalTaxes().push_back(&yqyrItem);
    farePath.itin() = &itin;
    itin.farePath().push_back(&farePath);
    taxResponse.farePath() = &farePath;

    taxCodeReg.taxOnTaxCode().push_back(YQF);

    TaxUI tax;

    tax.applyTaxOnTax(trx, taxResponse, taxCodeReg);

    CPPUNIT_ASSERT_EQUAL(taxCodeReg.taxAmt() * yqyrItem.taxAmount() +
                         taxCodeReg.taxAmt() * fr1Item.taxAmount(),
                         tax.taxAmount());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxUITest);
}
