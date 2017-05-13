#ifndef EXCHANGE_PRICING_TRX_TEST_H
#define EXCHANGE_PRICING_TRX_TEST_H

#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>

namespace tse
{
class ExchangePricingTrx;
class RexPricingTrx;
class PricingRequest;
class PricingOptions;
class ExcItin;
class Itin;
class TrxAborter;
class FareCompInfo;
class SurchargeOverride;
class DifferentialOverride;
class StopoverOverride;
class PlusUpOverride;
class MileageTypeData;
class AirSeg;
class Billing;
class FareMarket;
class AirSeg;
class PaxType;

class ExchangePricingTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ExchangePricingTrxTest);

  CPPUNIT_TEST(testDefaultInitialization);
  CPPUNIT_TEST(testSetRequestType);
  CPPUNIT_TEST(testExchangeTrxInitializedWithRexTrxRedirectExType);
  CPPUNIT_TEST(testExchangeTrxInitializedFareCompInfoEqualsRexTrxFareCompInfo);
  CPPUNIT_TEST(testTag10ExchangeTrxInitializedTicketDTWithCurrentTktDT);
  CPPUNIT_TEST(testPEExchangeTrxInitializedTicketDTWithPurchaseDT);
  CPPUNIT_TEST(testFEExchangeTrxInitializedTicketDTWithPurchaseDT);
  CPPUNIT_TEST(testPEExchangeTrxInitializedTicketDTWithCurrentTktDTIfPurchaseDTMissing);
  CPPUNIT_TEST(testFEExchangeTrxInitializedTicketDTWithCurrentTktDTIfPurchaseDTMissing);
  CPPUNIT_TEST(testInitializeOverridesCopyFromARtoPORTtrx);
  CPPUNIT_TEST(testInitialize);
  CPPUNIT_TEST(testInitializeWhenDiagnosticIsActive);
  CPPUNIT_TEST(testInitializeWhenDiagnosticIsInActive);

  CPPUNIT_TEST(testExchangeApplyCurrentDTAsTicketDT);
  CPPUNIT_TEST(testReissueApplyHistoricalDTAsTicketDT);
  CPPUNIT_TEST(testReissueApplyHistoricalDTAsPreviousEchangeDT);
  CPPUNIT_TEST(testExchangeSetBsrRoeDateAsCurrentDT);
  CPPUNIT_TEST(testReissueSetBsrRoeDateAsHistoricalD95Present);
  CPPUNIT_TEST(testReissueSetBsrRoeDateAsHistoricalD95Empty);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testDefaultInitialization();
  void testSetRequestType();
  void testExchangeTrxInitializedWithRexTrxRedirectExType();
  void testExchangeTrxInitializedFareCompInfoEqualsRexTrxFareCompInfo();
  void testTag10ExchangeTrxInitializedTicketDTWithCurrentTktDT();
  void testPEExchangeTrxInitializedTicketDTWithPurchaseDT();
  void testFEExchangeTrxInitializedTicketDTWithPurchaseDT();
  void testPEExchangeTrxInitializedTicketDTWithCurrentTktDTIfPurchaseDTMissing();
  void testFEExchangeTrxInitializedTicketDTWithCurrentTktDTIfPurchaseDTMissing();
  void testInitializeOverridesCopyFromARtoPORTtrx();
  void testInitialize();
  void testInitializeWhenDiagnosticIsActive();
  void testInitializeWhenDiagnosticIsInActive();

  void testExchangeApplyCurrentDTAsTicketDT();
  void testReissueApplyHistoricalDTAsPreviousEchangeDT();
  void testReissueApplyHistoricalDTAsTicketDT();
  void testExchangeSetBsrRoeDateAsCurrentDT();
  void testReissueSetBsrRoeDateAsHistoricalD95Present();
  void testReissueSetBsrRoeDateAsHistoricalD95Empty();

  void setsUpDates();

private:
  ExchangePricingTrx* _trx;
  RexPricingTrx* _rexTrx;
  PricingRequest* _req;
  PricingOptions* _options;
  ExcItin* _excItin;
  FareCompInfo* _fareCompInfo;
  Itin* _itin;
  TrxAborter* _aborter;
  SurchargeOverride* _sur1;
  SurchargeOverride* _sur2;
  DifferentialOverride* _diff1;
  DifferentialOverride* _diff2;
  StopoverOverride* _sto1;
  StopoverOverride* _sto2;
  PlusUpOverride* _pup1;
  PlusUpOverride* _pup2;
  MileageTypeData* _mil1;
  MileageTypeData* _mil2;
  AirSeg* _tvlSeg;

  void populateExchangeOverridesInRexTrx();
  Billing* _billing;
  FareMarket* _fareMarket;
  PaxType* _paxType;
};
} // end namespace
#endif // EXCHANGE_PRICING_TRX_TEST_H
