
#include "DBAccess/CarrierPreference.h"
#include "Taxes/LegacyTaxes/ServiceFeeYQ.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

namespace YQYR
{

class ServiceFeeYQTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ServiceFeeYQTest);
  CPPUNIT_TEST(testIsNonRefundable_empty);
  CPPUNIT_TEST(testIsNonRefundable_YQ);
  CPPUNIT_TEST(testIsNonRefundable_YR);
  CPPUNIT_TEST(testIsNonRefundable_YQYR);
  CPPUNIT_TEST(testIsNonRefundable_YQI);
  CPPUNIT_TEST(testIsNonRefundable_YRF);
  CPPUNIT_TEST_SUITE_END();

public:
  void testIsNonRefundable_empty()
  {
    CarrierPreference pref;
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable(TaxCode(), pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YQI", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YQF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YRI", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YRF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YC", pref));
  }

  void testIsNonRefundable_YQ()
  {
    CarrierPreference pref;
    pref.setNonRefundableYQCode("YQ");
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable(TaxCode(), pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YQI", pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YQF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YRI", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YRF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YC", pref));
  }

  void testIsNonRefundable_YR()
  {
    CarrierPreference pref;
    pref.setNonRefundableYRCode("YR");
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable(TaxCode(), pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YQI", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YQF", pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YRI", pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YRF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YC", pref));
  }

  void testIsNonRefundable_YQYR()
  {
    CarrierPreference pref;
    pref.setNonRefundableYQCode("YQ");
    pref.setNonRefundableYRCode("YR");
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable(TaxCode(), pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YQI", pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YQF", pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YRI", pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YRF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YC", pref));
  }

  void testIsNonRefundable_YQI()
  {
    CarrierPreference pref;
    pref.setNonRefundableYQCode("YQI");
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable(TaxCode(), pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YQI", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YQF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YRI", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YRF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YC", pref));
  }

  void testIsNonRefundable_YRF()
  {
    CarrierPreference pref;
    pref.setNonRefundableYRCode("YRF");
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable(TaxCode(), pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YQI", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YQF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YRI", pref));
    CPPUNIT_ASSERT(ServiceFee::isNonRefundable("YRF", pref));
    CPPUNIT_ASSERT(!ServiceFee::isNonRefundable("YC", pref));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServiceFeeYQTest);

} // YQYR

} // tse
