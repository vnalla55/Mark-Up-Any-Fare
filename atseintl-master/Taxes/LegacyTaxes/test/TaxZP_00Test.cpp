#include "Taxes/LegacyTaxes/TaxZP_00.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Common/TseEnums.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "DataModel/PricingTrx.h"

#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class TaxZP_00Test : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(TaxZP_00Test);
  CPPUNIT_TEST(testLocateUS1TaxEmptyResponse);
  CPPUNIT_TEST(testLocateUS1TaxEmptyUS1InResponse);
  CPPUNIT_TEST(testLocateUS1TaxUS1InResponse);
  CPPUNIT_TEST_SUITE_END();

public:
  void testLocateUS1TaxEmptyResponse()
  {
    PricingTrx trx;
    TaxCodeReg taxCodeReg;
    TaxResponse response;
    TaxZP_00 zp;

    CPPUNIT_ASSERT(!zp.validateUS1(trx, response, taxCodeReg));
  }

  void testLocateUS1TaxEmptyUS1InResponse()
  {
    TaxItem taxItem;
    PricingTrx trx;
    TaxCodeReg taxCodeReg;
    taxItem.taxAmount() = 0;
    // TaxCodeReg taxcodeReg;
    // taxItem.taxCodeReg() = &taxcodeReg;
    // taxItem.taxCodeReg()->taxCode()="US1";
    taxItem.taxCode() = "US1";
    std::vector<TaxItem*> taxV;
    taxV.push_back(&taxItem);

    TaxResponse response;
    response.taxItemVector() = taxV;
    TaxZP_00 zp;
    CPPUNIT_ASSERT(zp.validateUS1(trx, response, taxCodeReg));
  }

  void testLocateUS1TaxUS1InResponse()
  {
    PricingTrx trx;
    TaxCodeReg taxCodeReg;
    TaxItem taxItem;
    taxItem.taxAmount() = 10;
    // TaxCodeReg taxcodeReg;
    // taxItem.taxCodeReg() = &taxcodeReg;
    // taxItem.taxCodeReg()->taxCode()="US1";
    taxItem.taxCode() = "US1";
    std::vector<TaxItem*> taxV;
    taxV.push_back(&taxItem);

    TaxResponse response;
    response.taxItemVector() = taxV;
    TaxZP_00 zp;
    CPPUNIT_ASSERT(zp.validateUS1(trx, response, taxCodeReg));
  }

private:
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxZP_00Test);
}
