// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include <stdexcept>

#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/Common/Types.h"
#include "Common/SafeEnumToString.h"

namespace tax
{

class SafeEnumToStringTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SafeEnumToStringTest);
  CPPUNIT_TEST(testTaxPointTagSale);
  CPPUNIT_TEST(testTaxPointTagDeparture);
  CPPUNIT_TEST(testTaxPointTagArrival);
  CPPUNIT_TEST(testTaxPointTagDelivery);
  CPPUNIT_TEST(testTaxPointTagInvalid);
  CPPUNIT_TEST(testOpenSegmentIndicatorFixed);
  CPPUNIT_TEST(testOpenSegmentIndicatorDateFixed);
  CPPUNIT_TEST(testOpenSegmentIndicatorOpen);
  CPPUNIT_TEST(testOpenSegmentIndicatorInvalid);
  CPPUNIT_TEST(testStopoverTimeUnitBlank);
  CPPUNIT_TEST(testStopoverTimeUnitMinutes);
  CPPUNIT_TEST(testStopoverTimeUnitHours);
  CPPUNIT_TEST(testStopoverTimeUnitDays);
  CPPUNIT_TEST(testStopoverTimeUnitMonths);
  CPPUNIT_TEST(testStopoverTimeUnitHoursSameDay);
  CPPUNIT_TEST(testStopoverTimeUnitInvalid);
  CPPUNIT_TEST(testTaxOrChargeTagCharge);
  CPPUNIT_TEST(testTaxOrChargeTagTax);
  CPPUNIT_TEST(testTaxOrChargeTagBlank);
  CPPUNIT_TEST(testRefundableTagNo);
  CPPUNIT_TEST(testRefundableTagYes);
  CPPUNIT_TEST(testRefundableTagNoButReuseable);
  CPPUNIT_TEST(testRefundableTagPartial);
  CPPUNIT_TEST(testRefundableTagBlank);
  CPPUNIT_TEST(testAccountableDocTagNonTkdOrSale);
  CPPUNIT_TEST(testAccountableDocTagNotReported);
  CPPUNIT_TEST(testAccountableDocTagReportBundle);
  CPPUNIT_TEST(testAccountableDocTagReportSeparate);
  CPPUNIT_TEST(testAccountableDocTagBlank);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void testTaxPointTagSale()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("SALE"), safeEnumToString(type::TaxPointTag::Sale));
  }

  void testTaxPointTagDeparture()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("DEPARTURE"), safeEnumToString(type::TaxPointTag::Departure));
  }

  void testTaxPointTagArrival()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("ARRIVAL"), safeEnumToString(type::TaxPointTag::Arrival));
  }

  void testTaxPointTagDelivery()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("DELIVERY"), safeEnumToString(type::TaxPointTag::Delivery));
  }

  void testTaxPointTagInvalid()
  {
    CPPUNIT_ASSERT_EQUAL(std::string(""), safeEnumToString(static_cast<type::TaxPointTag>('.')));
  }

  void testOpenSegmentIndicatorFixed()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("FIXED"), safeEnumToString(type::OpenSegmentIndicator::Fixed));
  }

  void testOpenSegmentIndicatorDateFixed()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("DATEFIXED"),
                         safeEnumToString(type::OpenSegmentIndicator::DateFixed));
  }

  void testOpenSegmentIndicatorOpen()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("OPEN"), safeEnumToString(type::OpenSegmentIndicator::Open));
  }

  void testOpenSegmentIndicatorInvalid()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"),
                         safeEnumToString(static_cast<type::OpenSegmentIndicator>('.')));
  }

  void testStopoverTimeUnitBlank()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("BLANK"), safeEnumToString(type::StopoverTimeUnit::Blank));
  }

  void testStopoverTimeUnitMinutes()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("MINS"), safeEnumToString(type::StopoverTimeUnit::Minutes));
  }

  void testStopoverTimeUnitHours()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("HOURS"), safeEnumToString(type::StopoverTimeUnit::Hours));
  }

  void testStopoverTimeUnitDays()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("DAYS"), safeEnumToString(type::StopoverTimeUnit::Days));
  }

  void testStopoverTimeUnitMonths()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("MONTHS"), safeEnumToString(type::StopoverTimeUnit::Months));
  }

  void testStopoverTimeUnitHoursSameDay()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("SAME DAY HOURS"),
                         safeEnumToString(type::StopoverTimeUnit::HoursSameDay));
  }

  void testStopoverTimeUnitInvalid()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"), safeEnumToString(static_cast<type::StopoverTimeUnit>('.')));
  }

  void testTaxOrChargeTagCharge()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("CHARGE"), safeEnumToString(type::TaxOrChargeTag::Charge));
  }

  void testTaxOrChargeTagTax()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("TAX"), safeEnumToString(type::TaxOrChargeTag::Tax));
  }

  void testTaxOrChargeTagBlank()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"), safeEnumToString(type::TaxOrChargeTag::Blank));
  }

  void testRefundableTagNo()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NO"), safeEnumToString(type::RefundableTag::No));
  }

  void testRefundableTagYes()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("YES"), safeEnumToString(type::RefundableTag::Yes));
  }

  void testRefundableTagNoButReuseable()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NO BUT RE-USEABLE"), safeEnumToString(type::RefundableTag::NoButReuseable));
  }

  void testRefundableTagPartial()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("PARTIAL"), safeEnumToString(type::RefundableTag::Partial));
  }

  void testRefundableTagBlank()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"), safeEnumToString(type::RefundableTag::Blank));
  }

  void testAccountableDocTagNonTkdOrSale()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NON TKD/SALE"),
                         safeEnumToString(type::AccountableDocTag::NonTkdOrSale));
  }

  void testAccountableDocTagNotReported()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("NOT REPORTED"),
                         safeEnumToString(type::AccountableDocTag::NotReported));
  }

  void testAccountableDocTagReportBundle()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("REPORT/BUNDLE"),
                         safeEnumToString(type::AccountableDocTag::ReportBundle));
  }

  void testAccountableDocTagReportSeparate()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("REPORT/SEPARATE"),
                         safeEnumToString(type::AccountableDocTag::ReportSeparate));
  }

  void testAccountableDocTagBlank()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("UNKNOWN"),
                         safeEnumToString(type::AccountableDocTag::Blank));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SafeEnumToStringTest);

} // namespace tax
