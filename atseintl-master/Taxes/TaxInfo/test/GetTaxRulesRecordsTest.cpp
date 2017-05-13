// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include "Taxes/TaxInfo/GetTaxRulesRecords.h"
#include "DBAccess/TaxRulesRecord.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class GetTaxRulesRecordsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GetTaxRulesRecordsTest);
  CPPUNIT_TEST(translateTaxCodeBasic);
  CPPUNIT_TEST(translateTaxDoubleAnsForUS);
  CPPUNIT_TEST(travelDateRange_happyPath);
  CPPUNIT_TEST(travelDateRange_happyPath_2digitYear);
  CPPUNIT_TEST(travelDateRange_missingData);
  CPPUNIT_TEST(travelDateRange_deduceYear);
  CPPUNIT_TEST(travelDateRange_ExceptOneWeek_invertedDays);
  CPPUNIT_TEST(travelDateRange_invertedMonths);
  CPPUNIT_TEST_SUITE_END();

  std::pair<DateTime, DateTime> ans;

public:

  void translateTaxCodeBasic()
  {
    std::vector<TaxCodeAndType> ans = translateToAtpcoCodes("UK1");
    CPPUNIT_ASSERT_EQUAL(size_t(1), ans.size());
    CPPUNIT_ASSERT_EQUAL(TaxCode("UK"), ans[0].code);
    CPPUNIT_ASSERT_EQUAL(TaxCode("001"), ans[0].type);

    ans = translateToAtpcoCodes("PL");
    CPPUNIT_ASSERT_EQUAL(size_t(1), ans.size());
    CPPUNIT_ASSERT_EQUAL(TaxCode("PL"), ans[0].code);
    CPPUNIT_ASSERT_EQUAL(TaxCode("001"), ans[0].type);

    ans = translateToAtpcoCodes("NZ3");
    CPPUNIT_ASSERT_EQUAL(size_t(1), ans.size());
    CPPUNIT_ASSERT_EQUAL(TaxCode("NZ"), ans[0].code);
    CPPUNIT_ASSERT_EQUAL(TaxCode("003"), ans[0].type);
  }

  void translateTaxDoubleAnsForUS()
  {
    std::vector<TaxCodeAndType> ans = translateToAtpcoCodes("US1");
    CPPUNIT_ASSERT_EQUAL(size_t(2), ans.size());
    CPPUNIT_ASSERT_EQUAL(TaxCode("US"), ans[0].code);
    CPPUNIT_ASSERT_EQUAL(TaxCode("003"), ans[0].type);
    CPPUNIT_ASSERT_EQUAL(TaxCode("US"), ans[1].code);
    CPPUNIT_ASSERT_EQUAL(TaxCode("004"), ans[1].type);
  }

  void travelDateRange_happyPath()
  {
    const DateTime noDate = DateTime::emptyDate();

    TaxRulesRecord rulesRec;
    rulesRec.tvlFirstYear() = 2014;
    rulesRec.tvlFirstMonth() = 1;
    rulesRec.tvlFirstDay() = 1;
    rulesRec.tvlLastYear() = 2014;
    rulesRec.tvlLastMonth() = 12;
    rulesRec.tvlLastDay() = 31;

    ans = makeTravelRange(rulesRec, noDate);
    CPPUNIT_ASSERT_EQUAL(DateTime(2014, 1, 1), ans.first);
    CPPUNIT_ASSERT_EQUAL(DateTime(2014, 12, 31), ans.second);
  }

  void travelDateRange_happyPath_2digitYear()
  {
    const DateTime noDate = DateTime::emptyDate();

    TaxRulesRecord rulesRec;
    rulesRec.tvlFirstYear() = 14;
    rulesRec.tvlFirstMonth() = 1;
    rulesRec.tvlFirstDay() = 1;
    rulesRec.tvlLastYear() = 14;
    rulesRec.tvlLastMonth() = 12;
    rulesRec.tvlLastDay() = 31;

    ans = makeTravelRange(rulesRec, noDate);
    CPPUNIT_ASSERT_EQUAL(DateTime(2014, 1, 1), ans.first);
    CPPUNIT_ASSERT_EQUAL(DateTime(2014, 12, 31), ans.second);
  }

  void travelDateRange_missingData()
  {
    const DateTime noDate = DateTime::emptyDate();
    DateTime tktDate(2014, 6, 10);

    TaxRulesRecord rulesRec;

    ans = makeTravelRange(rulesRec, tktDate);
    CPPUNIT_ASSERT_EQUAL(noDate, ans.first);
    CPPUNIT_ASSERT_EQUAL(noDate, ans.second);

    rulesRec.tvlFirstYear() = 2014;
    rulesRec.tvlFirstMonth() = 1;
    rulesRec.tvlFirstDay() = 1;
    rulesRec.tvlLastYear() = 2014;
    rulesRec.tvlLastMonth() = 12;
    rulesRec.tvlLastDay() = 0;      // bad day

    ans = makeTravelRange(rulesRec, tktDate);
    CPPUNIT_ASSERT_EQUAL(DateTime(2014, 1, 1), ans.first);
    CPPUNIT_ASSERT_EQUAL(noDate, ans.second);
  }

  void travelDateRange_deduceYear()
  {
    TaxRulesRecord rulesRec;
    rulesRec.tvlFirstYear() = 0;
    rulesRec.tvlFirstMonth() = 2;
    rulesRec.tvlFirstDay() = 15;
    rulesRec.tvlLastYear() = 0;
    rulesRec.tvlLastMonth() = 6;
    rulesRec.tvlLastDay() = 16;

    ans = makeTravelRange(rulesRec, DateTime(2013, 12, 31)); // M and D is irrelevant
    CPPUNIT_ASSERT_EQUAL(DateTime(2013, 2, 15), ans.first);
    CPPUNIT_ASSERT_EQUAL(DateTime(2013, 6, 16), ans.second);
  }

  void travelDateRange_invertedMonths()
  {
    TaxRulesRecord rulesRec;
    rulesRec.tvlFirstYear() = 0;
    rulesRec.tvlFirstMonth() = 10;
    rulesRec.tvlFirstDay() = 15;
    rulesRec.tvlLastYear() = 0;
    rulesRec.tvlLastMonth() = 6; // last month before first month
    rulesRec.tvlLastDay() = 16;

    ans = makeTravelRange(rulesRec, DateTime(2013, 5, 31));
    CPPUNIT_ASSERT_EQUAL(DateTime(2012, 10, 15), ans.first);
    CPPUNIT_ASSERT_EQUAL(DateTime(2013, 6, 16), ans.second);

    ans = makeTravelRange(rulesRec, DateTime(2013, 11, 30));
    CPPUNIT_ASSERT_EQUAL(DateTime(2013, 10, 15), ans.first);
    CPPUNIT_ASSERT_EQUAL(DateTime(2014, 6, 16), ans.second);

  }

  void travelDateRange_ExceptOneWeek_invertedDays()
  {
    TaxRulesRecord rulesRec;
    rulesRec.tvlFirstYear() = 0;
    rulesRec.tvlFirstMonth() = 6;
    rulesRec.tvlFirstDay() = 20;
    rulesRec.tvlLastYear() = 0;
    rulesRec.tvlLastMonth() = 6; // same month
    rulesRec.tvlLastDay() = 10;  // inverted days

    ans = makeTravelRange(rulesRec, DateTime(2013, 6, 5));
    CPPUNIT_ASSERT_EQUAL(DateTime(2012, 6, 20), ans.first);
    CPPUNIT_ASSERT_EQUAL(DateTime(2013, 6, 10), ans.second);

    ans = makeTravelRange(rulesRec, DateTime(2013, 6, 25));
    CPPUNIT_ASSERT_EQUAL(DateTime(2013, 6, 20), ans.first);
    CPPUNIT_ASSERT_EQUAL(DateTime(2014, 6, 10), ans.second);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(GetTaxRulesRecordsTest);

} // namespace tse

