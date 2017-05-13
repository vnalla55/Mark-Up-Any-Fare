#ifndef __PRICING_UTIL_TEST_H__
#define __PRICING_UTIL_TEST_H__

#include "test/include/CppUnitHelperMacros.h"

#include "test/DBAccessMock/DataHandleMock.h"

#include <cppunit/TestFixture.h>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Pricing/NetRemitPricing.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/BankerSellRate.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "Common/TseConsts.h"
#include "DataModel/PaxType.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Customer.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include <string>

namespace tse
{
class Agent;
class Itin;
class PricingTrx;

namespace
{
class MyDataHandle : public DataHandleMock
{
private:
  TestMemHandle _memHandle;

public:
  const std::vector<BankerSellRate*>&
  getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
  {
    std::vector<BankerSellRate*>* ret = _memHandle.create<std::vector<BankerSellRate*> >();
    BankerSellRate* bsr = _memHandle.create<BankerSellRate>();
    bsr->primeCur() = primeCur;
    bsr->cur() = cur;
    bsr->rateType() = 'B';
    bsr->agentSine() = "FXR";
    bsr->rate() = 1.23456;
    bsr->rateNodec() = 2;

    ret->push_back(bsr);
    return *ret;
  }
};
}

class PricingUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingUtilTest);
  CPPUNIT_TEST(testDetermineCat27TourCodeEmptyFarePath);
  CPPUNIT_TEST(testDetermineCat27TourCodeOneEmptyPricingUnit);
  CPPUNIT_TEST(testDetermineCat27TourCodeOnePricingUnitEmptyFareUsage);
  CPPUNIT_TEST(testDetermineCat27TourCodeOnePricingUnitNoTourCode);
  CPPUNIT_TEST(testDetermineCat27TourCodeOnePricingUnitWithTourCode);
  CPPUNIT_TEST(testDetermineCat27TourCodeTwoPricingUnitsNoTours);
  CPPUNIT_TEST(testDetermineCat27TourCodeTwoPricingUnitsOneTour1);
  CPPUNIT_TEST(testDetermineCat27TourCodeTwoPricingUnitsOneTour2);
  CPPUNIT_TEST(testDetermineCat27TourCodeTwoPricingUnitsTwoTours1);
  CPPUNIT_TEST(testDetermineCat27TourCodeMultiplePricingUnits);

  CPPUNIT_TEST(testIsJLExemptAccntCode1);
  CPPUNIT_TEST(testIsJLExemptAccntCode2);
  CPPUNIT_TEST(testIsJLExemptAccntCode3);
  CPPUNIT_TEST(testIsJLExemptAccntCode4);
  CPPUNIT_TEST(testIsJLExemptAccntCode5);
  CPPUNIT_TEST(testIsJLExemptAccntCode6);
  CPPUNIT_TEST(testIsJLExemptTktDesig1);
  CPPUNIT_TEST(testIsJLExemptTktDesig2);
  CPPUNIT_TEST(testIsJLExemptTktDesig3);
  CPPUNIT_TEST(testIsJLExempt1);
  CPPUNIT_TEST(testIsJLExempt2);
  CPPUNIT_TEST(testIsJLExempt3);
  CPPUNIT_TEST(testIsJLExempt4);

  CPPUNIT_TEST(testPrintTaxItem605Enabled);
  CPPUNIT_TEST(testPrintTaxItem605Disabled);

  CPPUNIT_TEST(testIntersectCarrierList);

  CPPUNIT_TEST(testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_discount);
  CPPUNIT_TEST(testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_discount2);
  CPPUNIT_TEST(testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_plusUp);
  CPPUNIT_TEST(testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_plusUp2);
  CPPUNIT_TEST(testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_discount);
  CPPUNIT_TEST(testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_discount2);
  CPPUNIT_TEST(testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_plusUp);
  CPPUNIT_TEST(testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_plusUp2);

  CPPUNIT_TEST(testGetManualAdjustmentAmountsPerFUHelper);
  CPPUNIT_TEST(testGetAslMileageDiff);
  CPPUNIT_TEST(testConvertCurrencyForMsl);
  CPPUNIT_TEST(testAdjustedSellingCalcDataExists);
  CPPUNIT_TEST(testProcessAdjustedSellingFarePathASLPositive);
  CPPUNIT_TEST(testProcessAdjustedSellingFarePathASLNegative);
  CPPUNIT_TEST(testProcessAdjustedSellingFarePathMSLPositive);
  CPPUNIT_TEST(testProcessAdjustedSellingFarePathMSLNegative);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testDetermineCat27TourCodeEmptyFarePath();
  void testDetermineCat27TourCodeOneEmptyPricingUnit();
  void testDetermineCat27TourCodeOnePricingUnitEmptyFareUsage();
  void testDetermineCat27TourCodeOnePricingUnitNoTourCode();
  void testDetermineCat27TourCodeOnePricingUnitWithTourCode();
  void testDetermineCat27TourCodeTwoPricingUnitsNoTours();
  void testDetermineCat27TourCodeTwoPricingUnitsOneTour1();
  void testDetermineCat27TourCodeTwoPricingUnitsOneTour2();
  void testDetermineCat27TourCodeTwoPricingUnitsTwoTours1();
  void testDetermineCat27TourCodeTwoPricingUnitsTwoTours2();
  void testDetermineCat27TourCodeMultiplePricingUnits();

  void setUpForJLExempt();
  void testIsJLExemptAccntCode1();
  void testIsJLExemptAccntCode2();
  void testIsJLExemptAccntCode3();
  void testIsJLExemptAccntCode4();
  void testIsJLExemptAccntCode5();
  void testIsJLExemptAccntCode6();
  void testIsJLExemptTktDesig1();
  void testIsJLExemptTktDesig2();
  void testIsJLExemptTktDesig3();
  void testIsJLExempt1();
  void testIsJLExempt2();
  void testIsJLExempt3();
  void testIsJLExempt4();

  void setUpForPrintTaxItem(const DiagnosticTypes diagType);
  void testPrintTaxItem605Enabled();
  void testPrintTaxItem605Disabled();

  void testIntersectCarrierList();

  void testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_discount();
  void testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_discount2();
  void testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_plusUp();
  void testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_plusUp2();
  void testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_discount();
  void testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_discount2();
  void testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_plusUp();
  void testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_plusUp2();

  void testGetManualAdjustmentAmountsPerFUHelper();
  void testGetAslMileageDiff();
  void testConvertCurrencyForMsl();
  void testAdjustedSellingCalcDataExists();
  void testProcessAdjustedSellingFarePathASLPositive();
  void testProcessAdjustedSellingFarePathASLNegative();
  void testProcessAdjustedSellingFarePathMSLPositive();
  void testProcessAdjustedSellingFarePathMSLNegative();

private:
  void addTourCode(PaxTypeFare& fare, const std::string& tourCode);
  void addPricingUnit(FarePath& farePath, const std::string& tourCode);

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions* _options;
  FarePath* _farePath;
  FareInfo* _fareInfo;
  PaxTypeFare* _ptf;
  TravelSeg* _travelSeg;
  Fare* _fare;
  FareMarket* _fareMarket;
  Itin* _itin;
  FareUsage* _fareUsage;
  AirSeg* _airSeg1;
  AirSeg* _airSeg2;
  PricingUnit* _pricingUnit;
  Agent* _agent;
  Customer* _customer;
  TaxItem* _taxItem;
  DiagCollector* _diag;
  std::map<int16_t, Percent>* _discountPercentages;
  std::vector<DiscountAmount>* _discountAmounts;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingUtilTest);

} // tse

#endif //__PRICING_UTIL_TEST_H__
