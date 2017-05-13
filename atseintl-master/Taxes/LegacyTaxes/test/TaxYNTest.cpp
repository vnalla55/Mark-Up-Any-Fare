#ifndef TAX_YN_TEST_H
#define TAX_YN_TEST_H

#include "Taxes/LegacyTaxes/TaxYN.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

class TaxYNMock : public TaxYN
{
public:
  TaxYNMock() {}
  ~TaxYNMock() {}

  MoneyAmount fareAmountInNUC(const PricingTrx& trx, const TaxResponse& taxResponse)
  {
    return TaxYN::fareAmountInNUC(trx, taxResponse);
  }
  MoneyAmount discFactor(const PricingTrx& trx, int16_t segmentOrder)
  {
    return TaxYN::discFactor(trx, segmentOrder);
  }
};

class TaxYNTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxYNTest);
  CPPUNIT_TEST(testDiscFactorNoDisc);
  CPPUNIT_TEST(testDiscFactorDisc);
  CPPUNIT_TEST(testDiscFactorWrongDisc);
  CPPUNIT_TEST(testFareAmountInNUCEmptyPricingUnit);
  CPPUNIT_TEST(testFareAmountInNUCPMVException);
  CPPUNIT_TEST(testFareAmountInNUCEmptyFare);
  CPPUNIT_TEST_SUITE_END();

public:
  void testDiscFactorNoDisc();
  void testDiscFactorDisc();
  void testDiscFactorWrongDisc();
  void testFareAmountInNUCEmptyPricingUnit();
  void testFareAmountInNUCPMVException();
  void testFareAmountInNUCEmptyFare();

  void setUp()
  {
    _trx.setRequest(&_request);
    _taxResponse.farePath() = &_farePath;
    _fareUsage.travelSeg().push_back(&_travelSeg);
    _taxResponse.farePath()->pricingUnit().push_back(&_pricingUnit);
  }

private:
  PricingTrx _trx;
  PricingRequest _request;
  TaxResponse _taxResponse;
  PricingUnit _pricingUnit;
  FarePath _farePath;
  FareUsage _fareUsage;
  TaxYNMock _tax;
  AirSeg _travelSeg;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxYNTest);

void
TaxYNTest::testDiscFactorNoDisc()
{
  CPPUNIT_ASSERT_EQUAL(1.0, _tax.discFactor(_trx, 0));
}

void
TaxYNTest::testDiscFactorDisc()
{
  _request.discPercentages()[0] = 10;
  CPPUNIT_ASSERT_EQUAL(0.9, _tax.discFactor(_trx, 0));
}

void
TaxYNTest::testDiscFactorWrongDisc()
{
  _request.discPercentages()[1] = 110;
  CPPUNIT_ASSERT_EQUAL(1.0, _tax.discFactor(_trx, 1));
}

void
TaxYNTest::testFareAmountInNUCEmptyPricingUnit()
{
  _travelSeg.origAirport() = "DFW";
  PaxTypeFare paxTypeFare;
  _fareUsage.paxTypeFare() = &paxTypeFare;

  CPPUNIT_ASSERT_EQUAL(0.0, _tax.fareAmountInNUC(_trx, _taxResponse));
}

void
TaxYNTest::testFareAmountInNUCPMVException()
{
  _travelSeg.origAirport() = "PMV";
  _fareUsage.travelSeg().push_back(&_travelSeg);
  _pricingUnit.fareUsage().push_back(&_fareUsage);

  PaxTypeFare paxTypeFare;
  _fareUsage.paxTypeFare() = &paxTypeFare;

  CPPUNIT_ASSERT_EQUAL(0.0, _tax.fareAmountInNUC(_trx, _taxResponse));
}

void
TaxYNTest::testFareAmountInNUCEmptyFare()
{
  _travelSeg.origAirport() = "KRK";
  PaxTypeFare paxTypeFare;
  _fareUsage.paxTypeFare() = &paxTypeFare;

  CPPUNIT_ASSERT_EQUAL(0.0, _tax.fareAmountInNUC(_trx, _taxResponse));
}
}
#endif
