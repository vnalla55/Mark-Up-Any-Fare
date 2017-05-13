#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include <string>
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxRequest.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"
#include "Common/DateTime.h"
#include "Taxes/LegacyTaxes/Category1.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class TaxDisplayCAT1Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT1Test);
  CPPUNIT_TEST(testBuildSubCat1TaxAmtPercent);
  CPPUNIT_TEST(testBuildSubCat1TaxAmtFlat);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx.setRequest(&_request);
    _trx.getRequest()->ticketingDT() = _dt.localTime();
    _taxDisplayItem.taxCodeReg() = &_taxCodeReg;
  }

  void testBuildSubCat1TaxAmtPercent()
  {
    _taxCodeReg.taxCur() = "USD";
    _taxCodeReg.taxType() = 'P';
    _taxCodeReg.taxAmtInt().amt() = 75;
    _taxCodeReg.taxAmtInt().nodec() = 2;
    _taxCodeReg.seqNo() = 9999;

    _category1.build(_trx, _taxDisplayItem);

    CPPUNIT_ASSERT_EQUAL(_category1.subCat1(), std::string("USD  75 PERCENT  SEQ: 9999\n"));
  }

  void testBuildSubCat1TaxAmtFlat()
  {
    _taxCodeReg.taxCur() = "USD";
    _taxCodeReg.taxType() = 'F';
    _taxCodeReg.taxAmt() = 320.75;
    _taxCodeReg.specialProcessNo() = 0;
    _taxCodeReg.seqNo() = 400;

    _category1.build(_trx, _taxDisplayItem);

    CPPUNIT_ASSERT_EQUAL(_category1.subCat1(), std::string("USD  320.75  SEQ: 400\n"));
  }

private:
  TaxTrx _trx;
  TaxCodeReg _taxCodeReg;
  TaxRequest _request;
  TaxDisplayItem _taxDisplayItem;
  DateTime _dt;
  Category1 _category1;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT1Test);
}
