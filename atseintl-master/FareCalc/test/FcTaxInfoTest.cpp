//
// Copyright Sabre 2012-03-13
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

#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FcTaxInfo.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackFixForRTPricingInSplit);

class FcTaxInfoTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FcTaxInfoTest);
  CPPUNIT_TEST(testGroupTaxesByLeg);
  CPPUNIT_TEST(testGroupTaxesByFareUsage);
  CPPUNIT_TEST(testCalculateEstimatedBaseFaresForLegs_new);
  CPPUNIT_TEST(testCalculateRealBaseFaresForLegs);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx _trx;
  PricingRequest _request;
  PricingOptions _options;
  Agent _agent;
  Loc _loc;

  Itin itin;
  FarePath fp, fp_for_estimation;
  Fare f1, f2, f3;
  PaxTypeFare ptf1, ptf2, ptf3;
  PricingUnit pu1, pu2, pu3;
  FareUsage fu1, fu2, fu3;
  Loc loc;
  AirSeg s1, s2, s3, s4;

  TaxCodeReg tcr;
  TaxItem i1, i2, i3, i4, i5;
  PfcItem p1, p2, p3, p4, p5, p6;

public:
  FcTaxInfoTest()
  {
    using namespace boost::assign;

    _loc.nation() = "US";
    _trx.setRequest(&_request);
    _trx.setOptions(&_options);
    _request.ticketingAgent() = &_agent;
    _agent.agentLocation() = &_loc;
    _agent.currencyCodeAgent() = "USD";

    // setup itinerary structure
    s1.legId() = 0;
    s2.legId() = 0;
    s3.legId() = 1;
    s4.legId() = 1;
    s1.origin() = &loc;
    s2.origin() = &loc;
    s3.origin() = &loc;
    s4.origin() = &loc;
    s1.destination() = &loc;
    s2.destination() = &loc;
    s3.destination() = &loc;
    s4.destination() = &loc;
    s1.origAirport() = "ABC";
    s2.origAirport() = "DEF";
    s3.origAirport() = "GHI";
    s4.origAirport() = "DEF";

    fp.setTotalNUCAmount(200.0);
    fp.calculationCurrency() = "USD";
    fp.baseFareCurrency() = "USD";
    fp_for_estimation.setTotalNUCAmount(200.0);
    fp_for_estimation.calculationCurrency() = "USD";
    fp_for_estimation.baseFareCurrency() = "USD";
    ptf1.mileage() = 10;
    ptf2.mileage() = 20;
    ptf3.mileage() = 30;
    f1.nucFareAmount() = 50.0;
    f2.nucFareAmount() = 150.0;
    f3.nucFareAmount() = 200.0;
    ptf1.initialize(&f1, 0, 0);
    ptf2.initialize(&f2, 0, 0);
    ptf3.initialize(&f3, 0, 0);

    itin.farePath() += &fp;
    itin.travelSeg() += &s1, &s2, &s3, &s4;
    fp.itin() = &itin;

    fp.pricingUnit() += &pu1, &pu2;
    fp_for_estimation.pricingUnit() += &pu3;


    fu1.paxTypeFare() = &ptf1;
    fu2.paxTypeFare() = &ptf2;
    fu3.paxTypeFare() = &ptf3;

    pu1.fareUsage() += &fu1;
    pu2.fareUsage() += &fu2;
    pu3.fareUsage() += &fu3;

    fu1.accumulateSurchargeAmt(20.0);
    fu2.accumulateSurchargeAmt(70.0);
    fu3.accumulateSurchargeAmt(50);
    fu1.travelSeg() += &s1, &s2;
    fu2.travelSeg() += &s3, &s4;
    fu3.travelSeg() += &s1, &s2, &s3, &s4;

    // setup taxes
    // i1.taxCodeReg() = &tcr;
    // i2.taxCodeReg() = &tcr;
    // i3.taxCodeReg() = &tcr;
    // i4.taxCodeReg() = &tcr;
    // i5.taxCodeReg() = &tcr;

    i1.setFailCode(TaxItem::EXEMPT_ALL_TAXES);
    i2.setFailCode(TaxItem::EXEMPT_SPECIFIED_TAXES);
    i3.setFailCode(TaxItem::EXEMPT_ALL_TAXES);

    i1.setLegId(0);
    i2.setLegId(0);
    i3.setLegId(1);
    i4.setLegId(1);
    i5.setLegId(0);

    i1.setTravelSegStartIndex(0);
    i1.setTravelSegEndIndex(0); // s1 -> fu1
    i2.setTravelSegStartIndex(1);
    i2.setTravelSegEndIndex(1); // s2 -> fu1
    i3.setTravelSegStartIndex(2);
    i3.setTravelSegEndIndex(2); // s3 -> fu2
    i4.setTravelSegStartIndex(3);
    i4.setTravelSegEndIndex(3); // s4 -> fu2
    i5.setTravelSegStartIndex(1);
    i5.setTravelSegEndIndex(1); // s2 -> fu1

    p1.legId() = 0;
    p1.setPfcAirportCode("ABC"); // s1 -> fu1
    p2.legId() = 0;
    p2.setPfcAirportCode("DEF"); // s2 -> fu1
    p3.legId() = 1;
    p3.setPfcAirportCode("DEF"); // s4 -> fu2
    p4.legId() = 1;
    p4.setPfcAirportCode("GHI"); // s3 -> fu2
    p5.legId() = 1;
    p5.setPfcAirportCode("DEF"); // s4 -> fu2
    p6.legId() = 0;
    p6.setPfcAirportCode("DEF"); // s2 -> fu1
  }

  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testGroupTaxesByLeg()
  {
    using namespace boost::assign;

    TaxResponse tr;
    tr.taxItemVector() += &i1, &i2, &i3, &i4, &i5;
    tr.pfcItemVector() += &p1, &p2, &p3, &p4, &p5, &p6;

    CalcTotals totals;
    totals.convertedBaseFareCurrencyCode = "USD";
    totals.farePath = &fp;
    FareCalc::FcTaxInfo fcti(&tr);
    fcti._trx = &_trx;
    fcti._calcTotals = &totals;
    fcti._taxCurrencyCode = "USD";
    totals.getMutableFcTaxInfo() = fcti;

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), fcti.taxItems().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), fcti.pfcItems().size());

    FareCalc::SplitTaxInfo expectedGroupForLeg0;
    expectedGroupForLeg0.taxItems += &i1, &i2, &i5;
    expectedGroupForLeg0.pfcItems += &p1, &p2, &p6;

    FareCalc::SplitTaxInfo expectedGroupForLeg1;
    expectedGroupForLeg1.taxItems += &i3, &i4;
    expectedGroupForLeg1.pfcItems += &p3, &p4, &p5;

    FareCalc::FcTaxInfo::TaxesPerLeg result;
    fcti.getTaxesSplitByLeg(result);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
    CPPUNIT_ASSERT(expectedGroupForLeg0.taxItems == result[0].taxItems);
    CPPUNIT_ASSERT(expectedGroupForLeg0.pfcItems == result[0].pfcItems);
    CPPUNIT_ASSERT(expectedGroupForLeg1.taxItems == result[1].taxItems);
    CPPUNIT_ASSERT(expectedGroupForLeg1.pfcItems == result[1].pfcItems);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result[0].taxRecords.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result[1].taxRecords.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result[0].taxExempts.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result[1].taxExempts.size());
    CPPUNIT_ASSERT_EQUAL(10, result[0].mileage);
    CPPUNIT_ASSERT_EQUAL(20, result[1].mileage);
  }

  void testCalculateEstimatedBaseFaresForLegs_new()
  {
    fallback::value::fallbackFixForRTPricingInSplit.set(true);

    // This test is based on the invalid locations for used segments
    // so the mileage for all the segments is 0 ( as opposed to the mileage of fares )
    // PercentageComputator used in the calculateEstimatedBaseFaresForLegs
    // will split fares evenly among covered legs
    // More sophisticated scenarios can be found in PercentageComputatorTest.cpp file

    using namespace boost::assign;

    CalcTotals totals;
    totals.convertedBaseFare = 200.0;
    totals.convertedBaseFareCurrencyCode = "USD";
    totals.equivCurrencyCode = "USD";
    totals.farePath = &fp_for_estimation;

    TaxResponse tr;
    FareCalc::FcTaxInfo fcti(&tr);
    fcti._trx = &_trx;
    fcti._calcTotals = &totals;

    FareCalc::FcTaxInfo::TaxesPerLeg result;
    fcti.calculateEstimatedBaseFaresForLegs(result);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
    CPPUNIT_ASSERT_EQUAL(15, result[0].mileage);
    CPPUNIT_ASSERT_EQUAL(15, result[1].mileage);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(125.0, result[0].construction.value(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(125.0, result[1].construction.value(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), result[0].construction.code());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), result[1].construction.code());
  }

  void testCalculateRealBaseFaresForLegs()
  {
    fallback::value::fallbackFixForRTPricingInSplit.set(true);
    using namespace boost::assign;

    CalcTotals totals;
    totals.convertedBaseFare = 200.0;
    totals.convertedBaseFareCurrencyCode = "USD";
    totals.farePath = &fp;

    TaxResponse tr;
    FareCalc::FcTaxInfo fcti(&tr);
    fcti._trx = &_trx;
    fcti._calcTotals = &totals;

    FareCalc::FcTaxInfo::TaxesPerLeg result;
    fcti.calculateRealBaseFaresForLegs(result);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
    CPPUNIT_ASSERT_EQUAL(10, result[0].mileage);
    CPPUNIT_ASSERT_EQUAL(20, result[1].mileage);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(70.0, result[0].construction.value(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(220.0, result[1].construction.value(), EPSILON);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), result[0].construction.code());
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), result[1].construction.code());
  }

  void testGroupTaxesByFareUsage()
  {
    using namespace boost::assign;

    TaxResponse tr;
    tr.taxItemVector() += &i1, &i2, &i3, &i4, &i5;
    tr.pfcItemVector() += &p1, &p2, &p3, &p4, &p5, &p6;

    CalcTotals totals;
    totals.convertedBaseFareCurrencyCode = "USD";
    totals.farePath = &fp;
    FareCalc::FcTaxInfo fcti(&tr);
    fcti._trx = &_trx;
    fcti._calcTotals = &totals;
    fcti._taxCurrencyCode = "USD";
    totals.getMutableFcTaxInfo() = fcti;

    // test grouping
    FareCalc::SplitTaxInfo expectedGroupForFU1;
    FareCalc::SplitTaxInfo expectedGroupForFU2;

    expectedGroupForFU1.taxItems += &i1, &i2, &i5;
    expectedGroupForFU1.pfcItems += &p1, &p2, &p6;

    expectedGroupForFU2.taxItems += &i3, &i4;
    expectedGroupForFU2.pfcItems += &p3, &p4, &p5;

    FareCalc::FcTaxInfo::TaxesPerFareUsage result;
    fcti.getTaxesSplitByFareUsage(result);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result.size());
    CPPUNIT_ASSERT(expectedGroupForFU1.taxItems == result[&fu1].taxItems);
    CPPUNIT_ASSERT(expectedGroupForFU1.pfcItems == result[&fu1].pfcItems);
    CPPUNIT_ASSERT(expectedGroupForFU2.taxItems == result[&fu2].taxItems);
    CPPUNIT_ASSERT(expectedGroupForFU2.pfcItems == result[&fu2].pfcItems);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result[&fu1].taxRecords.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), result[&fu2].taxRecords.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result[&fu1].taxExempts.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), result[&fu2].taxExempts.size());
    CPPUNIT_ASSERT_EQUAL(10, result[&fu1].mileage);
    CPPUNIT_ASSERT_EQUAL(20, result[&fu2].mileage);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FcTaxInfoTest);
}
