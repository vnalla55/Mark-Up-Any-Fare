//
// Copyright Sabre 2012-04-18
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FcTaxInfo.h"
#include "Common/TseCodeTypes.h"
#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

namespace tse
{

class CalcTotalsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CalcTotalsTest);
  CPPUNIT_TEST(testGetTotalMileage);
  CPPUNIT_TEST(testGetTotalAmountPerPaxTaxAmtWhenZeroAmtDiffCurrency);
  CPPUNIT_TEST(testGetTotalAmountPerPaxTotalAmtWhenSameCurrency);
  CPPUNIT_TEST(testGetTotalAmountPerPaxTotalAmtWhenDiffCurrency);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetTotalMileage()
  {
    using namespace boost::assign;

    FarePath fp;
    PricingUnit pu1, pu2;
    PaxTypeFare ptf1, ptf2;
    FareUsage fu1, fu2;

    fp.pricingUnit() += &pu1, &pu2;

    fu1.paxTypeFare() = &ptf1;
    fu2.paxTypeFare() = &ptf2;

    pu1.fareUsage() += &fu1;
    pu2.fareUsage() += &fu2;

    ptf1.mileage() = 10;
    ptf2.mileage() = 20;

    CalcTotals totals;
    totals.farePath = &fp;

    CPPUNIT_ASSERT_EQUAL(30, totals.getTotalMileage());
  }
  void testGetTotalAmountPerPaxTaxAmtWhenZeroAmtDiffCurrency()
  {
    double taxAmt = 100.0;
    CalcTotals totals;

    totals.getMutableFcTaxInfo().taxAmount() = taxAmt;

    totals.convertedBaseFareCurrencyCode = "USD";
    totals.equivCurrencyCode = "GBP";
    totals.equivFareAmount = 0;
    totals.convertedBaseFare = 200.0;

    CPPUNIT_ASSERT_EQUAL(100.0, totals.getTotalAmountPerPax());
  }

  void testGetTotalAmountPerPaxTotalAmtWhenSameCurrency()
  {
    double taxAmt = 100.0;
    CalcTotals totals;

    totals.getMutableFcTaxInfo().taxAmount() = taxAmt;
    totals.convertedBaseFareCurrencyCode = "USD";
    totals.equivCurrencyCode = "USD";
    totals.equivFareAmount = 400.0;
    totals.convertedBaseFare = 200.0;

    CPPUNIT_ASSERT_EQUAL(300.0, totals.getTotalAmountPerPax());
  }

  void testGetTotalAmountPerPaxTotalAmtWhenDiffCurrency()
  {
    double taxAmt = 100.0;
    CalcTotals totals;

    totals.getMutableFcTaxInfo().taxAmount() = taxAmt;
    totals.convertedBaseFareCurrencyCode = "USD";
    totals.equivCurrencyCode = "GBP";
    totals.equivFareAmount = 400.0;
    totals.convertedBaseFare = 200.0;

    CPPUNIT_ASSERT_EQUAL(500.0, totals.getTotalAmountPerPax());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CalcTotalsTest);
}
