#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/ErrorResponseException.h"
#include "Common/Money.h"
#include "Common/StopWatch.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Currency/test/MockDataHandle.h"
#include "DataModel/Agent.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/StaticObjectPool.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <algorithm>
#include <string>

using namespace std;

namespace tse
{

class BSRCurrencyConverterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BSRCurrencyConverterTest);
  CPPUNIT_TEST(testDirectConversion);
  CPPUNIT_TEST(testTwoStepConversion);
  CPPUNIT_TEST(testWPConversion);
  CPPUNIT_TEST(testWPMConversion);
  CPPUNIT_TEST(testSingleWPRConversion);
  CPPUNIT_TEST(testDoubleWPRConversion);
  CPPUNIT_TEST(testDoubleWPRConversionWithException);
  CPPUNIT_TEST(testBRLToGBPConversion);
  CPPUNIT_TEST(testGBPToBRLConversion);
  CPPUNIT_TEST(testGetBSRRate);
  CPPUNIT_TEST(testApplyExchRate);
  CPPUNIT_TEST(testDeterminePricingCurrency);
  CPPUNIT_TEST(testApplyRounding);
  CPPUNIT_TEST(testConvertSOSToGBP);
  CPPUNIT_TEST(testBaseCurrencyConverter);
  CPPUNIT_TEST(testIndirectEquivAmtOverrideForGermany);
  CPPUNIT_TEST(testIndirectEquivAmtOverrideForParis);
  CPPUNIT_TEST(testConversionFacade);
  CPPUNIT_TEST(testZeroSourceAmt);
  CPPUNIT_TEST(testFacadeRound);
  CPPUNIT_TEST(testFacadeBSRConversionWithResults);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mockDataHandle = _memHandle.create<CurrencyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void testDirectConversion()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing BSR Direct Conversion");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      BSRCurrencyConverter bsrConverter;
      CarrierCode carrier("AA");
      PaxTypeCode paxType("ADT");
      BSRCollectionResults bsrResults(carrier, paxType);

      DataHandle dataHandle;
      LOG4CXX_DEBUG(_logger, "Created DataHandle");
      LocCode rio("RIO");
      DateTime travelDate = DateTime::localTime();
      LOG4CXX_DEBUG(_logger, "Created TravelDate");

      bool convertRC = false;

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;

      Money brl(100, "BRL");
      Money usd("USD"); // result should be 34.44

      CurrencyConversionRequest request1(
          usd, brl, travelDate, *(trx.getRequest()), trx.dataHandle());
      request1.setTrx(&trx);

      convertRC = bsrConverter.convert(request1, &bsrResults);

      LOG4CXX_DEBUG(_logger, "Converted USD amount in Brazilian BRL's : " << usd.value());

      CPPUNIT_ASSERT(convertRC != false);

      Money jpy(100, "JPY");
      Money usd2("USD"); // result should be .89

      LocCode nrt("NRT");
      LOG4CXX_DEBUG(_logger, "Testing BSR Direct Conversion");
      LOG4CXX_DEBUG(_logger, "Location Code: " << nrt);

      const Loc* loc2 = dataHandle.getLoc(nrt, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location vector");

      if (loc2)
      {

        NationCode nationCode = loc2->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for NRT is: " << nationCode);
      }

      // tse::Agent *agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc2;
      trx.getRequest()->ticketingAgent() = agent;

      CurrencyConversionRequest request2(usd2,
                                         jpy,
                                         travelDate,
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         false,
                                         CurrencyConversionRequest::DC_CONVERT);
      request2.setTrx(&trx);

      request2.salesLocCurrency() = "JPY";
      request2.commonSalesLocCurrency() = true;

      convertRC = bsrConverter.convert(request2, &bsrResults);

      LOG4CXX_DEBUG(_logger, "Converted USD amount in JPY " << usd2.value());

      CPPUNIT_ASSERT(convertRC != false);

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testTwoStepConversion()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      bool convertRC = false;
      PricingTrx trx;
      PricingRequest request;
      // CarrierCode carrier;

      BSRCurrencyConverter bsrConverter;
      BSRCollectionResults bsrResults;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing BSR Two Step Conversion");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      DataHandle dataHandle;
      DateTime travelDate = DateTime::localTime();

      LocCode msp("MSP");

      const tse::Loc* loc = dataHandle.getLoc(msp, travelDate);

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for MSP is: " << nationCode);
      }

      agent->agentLocation() = loc;
      request.ticketingAgent() = agent;
      trx.setRequest(&request);

      Money usd(1916.00, "USD"); // Result should be 1916.00
      Money gbp3("GBP");

      LOG4CXX_DEBUG(_logger, "Converting USD to British Pounds");

      CurrencyConversionRequest request1(gbp3,
                                         usd,
                                         travelDate,
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         false,
                                         CurrencyConversionRequest::DC_CONVERT);
      request1.setTrx(&trx);

      request1.salesLocCurrency() = "USD";
      request1.commonSalesLocCurrency() = true;

      convertRC = bsrConverter.convert(request1, &bsrResults);

      CPPUNIT_ASSERT(convertRC != false);

      LOG4CXX_DEBUG(_logger, "Converted USD amount in British Pounds: " << gbp3.value());

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testWPConversion()
  {
    try
    {
      bool convertRC = false;
      tse::StopWatch stopWatch;
      stopWatch.start();

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing WP BSR Conversion");
      LOG4CXX_DEBUG(_logger, "==========================\n");
      LOG4CXX_DEBUG(_logger, "\n");
      PricingTrx trx;
      PricingRequest request;

      BSRCurrencyConverter bsrConverter;
      BSRCollectionResults bsrResults;
      // CarrierCode carrier;

      DataHandle dataHandle;
      LocCode dfw("DFW");
      DateTime travelDate = DateTime::localTime();

      LOG4CXX_DEBUG(_logger, "Location Code: " << dfw);

      const tse::Loc* loc = dataHandle.getLoc(dfw, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for DFW is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      request.ticketingAgent() = agent;
      trx.setRequest(&request);

      Money gbp(1086.00, "GBP");
      Money usd("USD");

      LOG4CXX_DEBUG(_logger, "Converting British Pounds to USD");

      CurrencyConversionRequest request1(
          usd, gbp, travelDate, *(trx.getRequest()), trx.dataHandle());
      request1.setTrx(&trx);
      // convertRC = bsrConverter.convert(usd,gbp,trx,travelDate,carrier);

      convertRC = bsrConverter.convert(request1, &bsrResults);

      CPPUNIT_ASSERT(convertRC != false);

      LOG4CXX_DEBUG(_logger, "Converted GBP amount in USD : " << usd.value());

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testWPMConversion()
  {
    try
    {
      bool convertRC = false;
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;
      // CarrierCode carrier;

      BSRCurrencyConverter bsrConverter;
      BSRCollectionResults bsrResults;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing WPM Conversion");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      DataHandle dataHandle;
      LocCode rio("RIO");
      DateTime travelDate = DateTime::localTime();

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      request.ticketingAgent() = agent;
      trx.setRequest(&request);

      Money gbp(499.00, "GBP");
      Money crc("CRC");

      LOG4CXX_DEBUG(_logger, "Converting GBP to Costa Rica CRC");

      CurrencyConversionRequest request1(
          crc, gbp, travelDate, *(trx.getRequest()), trx.dataHandle());
      request1.setTrx(&trx);
      // convertRC = bsrConverter.convert(crc,gbp,trx,travelDate,carrier);

      convertRC = bsrConverter.convert(request1, &bsrResults);

      CPPUNIT_ASSERT(convertRC != false);

      LOG4CXX_DEBUG(_logger, "Converted GBP amount in Costa Rican CRC's : " << crc.value());

      LOG4CXX_DEBUG(_logger, "Testing BSR Two Step Conversion");

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testSingleWPRConversion()
  {
    try
    {
      bool convertRC = false;
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;
      // CarrierCode carrier("AA");

      BSRCurrencyConverter bsrConverter;
      BSRCollectionResults bsrResults;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Single WPR Conversion");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      DataHandle dataHandle;
      LocCode rio("RIO");
      DateTime travelDate = DateTime::localTime();

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      request.ticketingAgent() = agent;
      request.rateAmountOverride() = .199;
      trx.setRequest(&request);

      Money gbp(5482.00, "GBP");
      Money brl("BRL");

      LOG4CXX_DEBUG(_logger, "Converting GBP to BRL");

      CurrencyConversionRequest request1(
          brl, gbp, travelDate, *(trx.getRequest()), trx.dataHandle());
      request1.setTrx(&trx);

      // convertRC = bsrConverter.convert(brl,gbp,trx,travelDate,carrier);
      convertRC = bsrConverter.convert(request1, &bsrResults);

      CPPUNIT_ASSERT(convertRC != false);

      LOG4CXX_DEBUG(_logger, "Converted GBP amount in BRL's : " << brl.value());

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testDoubleWPRConversion()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;
      // CarrierCode carrier("AA");

      BSRCurrencyConverter bsrConverter;
      BSRCollectionResults bsrResults;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Double WPR Conversion");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      DataHandle dataHandle;
      LocCode rio("RIO");
      DateTime travelDate = DateTime::localTime();

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      request.ticketingAgent() = agent;
      trx.setRequest(&request);
      PricingOptions options;
      options.mOverride() = 'Y';
      options.currencyOverride() = "CRC";
      request.rateAmountOverride() = .6333;
      request.secondRateAmountOverride() = .007;

      trx.setRequest(&request);

      Money gbp(5482.00, "GBP");
      Money crc("CRC");

      LOG4CXX_DEBUG(_logger, "Converting GBP to CRC");

      CurrencyConversionRequest request1(crc,
                                         gbp,
                                         travelDate,
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         true,
                                         CurrencyConversionRequest::OTHER,
                                         false,
                                         &options);
      request1.setTrx(&trx);

      bsrConverter.convert(request1, &bsrResults);

      LOG4CXX_DEBUG(_logger, "Converted GBP amount in CRC's : " << crc.value());

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testDoubleWPRConversionWithException()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;
      // CarrierCode carrier("AA");

      BSRCurrencyConverter bsrConverter;
      BSRCollectionResults bsrResults;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Double WPR Conversion With Expected Override Exception");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      DataHandle dataHandle;
      LocCode rio("RIO");
      DateTime travelDate = DateTime::localTime();

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      PricingOptions options;
      agent->agentLocation() = loc;
      request.ticketingAgent() = agent;
      trx.setRequest(&request);
      options.mOverride() = 'Y';
      options.currencyOverride() = "BRL";
      request.rateAmountOverride() = .6333;
      request.secondRateAmountOverride() = .007;

      trx.setRequest(&request);
      trx.setOptions(&options);

      Money gbp(5482.00, "GBP");
      Money crc("CRC");

      LOG4CXX_DEBUG(_logger, "Converting GBP to CRC");

      CurrencyConversionRequest request1(crc,
                                         gbp,
                                         travelDate,
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         true,
                                         CurrencyConversionRequest::OTHER,
                                         false,
                                         &options);
      request1.setTrx(&trx);

      bsrConverter.convert(request1, &bsrResults);

      LOG4CXX_DEBUG(_logger, "Converted GBP amount in CRC's : " << crc.value());

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT(ex.code() == tse::ErrorResponseException::INVALID_INPUT);

      LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what());
    }
    catch (exception& ex)
    {
      CPPUNIT_FAIL("Unknown Exception");
      LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what());
    }
  }
  void testBRLToGBPConversion()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      bool convertRC = false;
      PricingTrx trx;
      PricingRequest request;
      // CarrierCode carrier;

      BSRCurrencyConverter bsrConverter;
      BSRCollectionResults bsrResults;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Conversion of BRL To GBP");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      DataHandle dataHandle;
      DateTime travelDate = DateTime::localTime();

      LocCode rio("RIO");

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      agent->agentLocation() = loc;
      request.ticketingAgent() = agent;
      trx.setRequest(&request);

      Money brl(500, "BRL");
      Money gbp("GBP");

      LOG4CXX_DEBUG(_logger, "Converting BRL to British Pounds");

      CurrencyConversionRequest request1(gbp,
                                         brl,
                                         travelDate,
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         false,
                                         CurrencyConversionRequest::DC_CONVERT);
      request1.setTrx(&trx);

      request1.salesLocCurrency() = "BRL";
      request1.commonSalesLocCurrency() = true;
      request1.conversionCurrency() = "USD";

      convertRC = bsrConverter.convert(request1, &bsrResults);

      CPPUNIT_ASSERT(convertRC != false);

      LOG4CXX_DEBUG(_logger, "Converted BRL amount in British Pounds: " << gbp.value());

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testGBPToBRLConversion()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      bool convertRC = false;
      PricingTrx trx;
      PricingRequest request;
      // CarrierCode carrier;

      BSRCurrencyConverter bsrConverter;
      BSRCollectionResults bsrResults;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Conversion of GBP To BRL");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      DataHandle dataHandle;
      DateTime travelDate = DateTime::localTime();

      LocCode rio("RIO");

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      agent->agentLocation() = loc;
      request.ticketingAgent() = agent;
      trx.setRequest(&request);

      Money brl("BRL");
      Money gbp(500, "GBP");

      LOG4CXX_DEBUG(_logger, "Converting GBP to BRL");

      CurrencyConversionRequest request1(brl,
                                         gbp,
                                         travelDate,
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         false,
                                         CurrencyConversionRequest::DC_CONVERT);
      request1.setTrx(&trx);

      request1.salesLocCurrency() = "BRL";
      request1.commonSalesLocCurrency() = true;
      request1.conversionCurrency() = "USD";

      convertRC = bsrConverter.convert(request1, &bsrResults);

      CPPUNIT_ASSERT(convertRC != false);

      LOG4CXX_DEBUG(_logger, "Converted GBP amount in BRL: " << brl.value());

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testGetBSRRate()
  {
    try
    {
      DateTime date = DateTime::localTime();
      CurrencyCode primeCurrency("BRL");
      CurrencyCode currency("GBP");
      ExchRate rate;
      CurrencyNoDec numDecimals;
      BSRCurrencyConverter bsrConverter;
      BSRCollectionResults bsrResults;
      Indicator exchangeRateType;
      tse::StopWatch stopWatch;
      stopWatch.start();

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Retrieval of BSR Rate");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      bool bsrRC =
          bsrConverter.bsrRate(primeCurrency, currency, date, rate, numDecimals, exchangeRateType);

      CPPUNIT_ASSERT(bsrRC != false);

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testApplyExchRate()
  {
    try
    {
      // CurrencyCode primeCurrency("BRL");
      // CurrencyCode currency("GBP");
      BSRCurrencyConverter bsrConverter;

      tse::StopWatch stopWatch;
      stopWatch.start();

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Application of BSR Rate");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      Money source(500, "BRL");
      Money target("GBP");
      ExchRate rate = .174584398;

      bool bsrRC = bsrConverter.applyExchRate(source, target, rate, BSRCurrencyConverter::MULTIPLY);

      CPPUNIT_ASSERT(bsrRC != false);

      Money source2(500, "GBP");
      Money target2("BRL");

      bsrRC = bsrConverter.applyExchRate(source2, target2, rate, BSRCurrencyConverter::DIVIDE);

      CPPUNIT_ASSERT(bsrRC != false);

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testDeterminePricingCurrency()
  {
    try
    {
      BSRCurrencyConverter bsrConverter;
      CurrencyTrx trx;

      tse::StopWatch stopWatch;
      stopWatch.start();

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Determine Pricing Currency");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      Money source(500, "BRL");
      Money target("GBP");

      DateTime travelDate = DateTime::localTime();

      LocCode rio("RIO");
      DataHandle dataHandle;

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }
      PricingRequest request;

      agent->agentLocation() = loc;
      request.ticketingAgent() = agent;
      trx.setRequest(&request);

      // This is certainly broken in currency conversion code. Just work-around it for now.
      TseCallableTrxTask::CurrentTrxSetter setTrx(&trx);

      CurrencyConversionRequest ccRequest(source,
                                          target,
                                          travelDate,
                                          *(trx.getRequest()),
                                          trx.dataHandle(),
                                          false,
                                          CurrencyConversionRequest::DC_CONVERT);

      ccRequest.salesLocCurrency() = "BRL";
      ccRequest.commonSalesLocCurrency() = true;

      CurrencyCode salesCurrency;
      CurrencyCode conversionCurrency;

      bool hasOverride = false;
      bool rc = bsrConverter.determinePricingCurrency(
          ccRequest, salesCurrency, conversionCurrency, hasOverride);

      CPPUNIT_ASSERT(!salesCurrency.empty() && rc);

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testApplyRounding()
  {
    try
    {
      BSRCurrencyConverter bsrConverter;

      tse::StopWatch stopWatch;
      stopWatch.start();

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Apply Rounding");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      CurrencyCode salesCurrency("BRL");
      DateTime date = DateTime::localTime();
      RoundingRule rule;
      RoundingFactor factor;
      CurrencyNoDec noDec;

      Money target(2693.949, "BRL");

      bool rc = bsrConverter.applyRounding(salesCurrency, target, date, rule, factor, noDec);

      LOG4CXX_DEBUG(_logger, "Rounded 2693.949 to: " << target.value());

      CPPUNIT_ASSERT(rc);

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testConvertSOSToGBP()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;

      BSRCurrencyConverter bsrConverter;
      CarrierCode carrier("AA");
      PaxTypeCode paxType("ADT");
      BSRCollectionResults bsrResults(carrier, paxType);

      bsrResults.collect() = true;

      DataHandle dataHandle;
      LocCode mgq("MGQ");
      DateTime travelDate = DateTime::localTime();

      bool convertRC = false;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Direct Conversion of Somalia Shillings to GBP's");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LOG4CXX_DEBUG(_logger, "Location Code: " << mgq);

      const tse::Loc* loc = dataHandle.getLoc(mgq, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for MGQ is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;

      Money sos(500, "SOS");
      Money gbp("GBP");

      CurrencyConversionRequest request1(gbp,
                                         sos,
                                         travelDate,
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         false,
                                         CurrencyConversionRequest::DC_CONVERT);
      request1.setTrx(&trx);

      request1.salesLocCurrency() = "SOS";
      request1.commonSalesLocCurrency() = true;
      request1.conversionCurrency() = "USD";

      convertRC = bsrConverter.convert(request1, &bsrResults);

      LOG4CXX_DEBUG(_logger, "Converted SOS amount in GBP's : " << gbp.value());

      CPPUNIT_ASSERT(convertRC != false);

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testBaseCurrencyConverter()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;

      CurrencyConverter currencyConverter;
      CarrierCode carrier("AA");
      PaxTypeCode paxType("ADT");
      BSRCollectionResults bsrResults(carrier, paxType);

      bsrResults.collect() = true;

      DataHandle dataHandle;
      LocCode mgq("MGQ");
      DateTime travelDate = DateTime::localTime();

      bool convertRC = false;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Base Currency Converter");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LOG4CXX_DEBUG(_logger, "Location Code: " << mgq);

      const tse::Loc* loc = dataHandle.getLoc(mgq, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for MGQ is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      Money sos(500, "SOS");
      Money gbp("GBP");

      CurrencyConversionRequest request1(gbp,
                                         sos,
                                         travelDate,
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         false,
                                         CurrencyConversionRequest::DC_CONVERT);
      request1.setTrx(&trx);

      request1.salesLocCurrency() = "SOS";
      request1.commonSalesLocCurrency() = true;
      request1.conversionCurrency() = "USD";

      convertRC = currencyConverter.convert(request1, &bsrResults);

      LOG4CXX_DEBUG(_logger, "Converted SOS amount in GBP's : " << gbp.value());

      CPPUNIT_ASSERT(convertRC != false);

      Money usd("USD");
      RoundingFactor usdRoundingFactor = 0;
      convertRC = currencyConverter.roundUp(usd, usdRoundingFactor);

      CPPUNIT_ASSERT(convertRC == false);

      convertRC = currencyConverter.roundDown(usd, usdRoundingFactor);

      CPPUNIT_ASSERT(convertRC == false);

      Money usd2(100.09, "USD");
      // RoundUnit roundUnit = 1.00;
      // RoundUnitNoDec roundUnitNoDec = 0;
      RoundingRule roundingRule = UP;
      RoundingFactor roundingFactor = 1.00;

      convertRC = currencyConverter.round(usd2, roundingFactor, roundingRule);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd2.value());

      CPPUNIT_ASSERT(convertRC != false);

      RoundingRule roundingRule2 = DOWN;
      convertRC = currencyConverter.round(usd2, roundingFactor, roundingRule2);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd2.value());

      CPPUNIT_ASSERT(convertRC != false);

      RoundingRule roundingRule3 = NEAREST;
      convertRC = currencyConverter.round(usd2, roundingFactor, roundingRule3);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd2.value());

      CPPUNIT_ASSERT(convertRC != false);

      Money nuc("NUC");
      Money usd4("USD");
      Agent agent2;

      DateTime ticketDate = DateTime::localTime();
      convertRC = currencyConverter.validateInput(nuc, usd4, agent2, ticketDate);

      CPPUNIT_ASSERT(convertRC != true);

      // Money brl("");
      Money usd41("USD");

      // convertRC = currencyConverter.validateInput(brl,usd41,agent2);

      // CPPUNIT_ASSERT(convertRC != true);

      Money brl2("BRL");

      convertRC = currencyConverter.validateInput(brl2, usd41, agent2, ticketDate);

      CPPUNIT_ASSERT(convertRC != true);

      Money usd5(100, "USD");

      convertRC = currencyConverter.validateInput(brl2, usd5, agent2, ticketDate);

      CPPUNIT_ASSERT(convertRC != true);

      convertRC = currencyConverter.validateInput(brl2, usd5);

      CPPUNIT_ASSERT(convertRC != true);

      convertRC = currencyConverter.validateInput(nuc, usd41);

      CPPUNIT_ASSERT(convertRC != true);

      convertRC = currencyConverter.validateInput(nuc, usd5);

      CPPUNIT_ASSERT(convertRC != false);

      RoundingRule roundingRule8 = NONE;
      Money usd8(100.225, "USD");
      // RoundUnit roundUnit8 = 1.00;
      // RoundUnitNoDec roundUnitNoDec8 = 2;

      convertRC = currencyConverter.round(usd8, roundingFactor, roundingRule8);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd8.value());

      Money jpy8(1000.46, "JPY");
      // RoundUnitNoDec roundUnitNoDec9 = 0;
      convertRC = currencyConverter.round(jpy8, roundingFactor, roundingRule8);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << jpy8.value());

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testIndirectEquivAmtOverrideForGermany()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing testIndirectEquivAmtOverrideForGermany");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      Itin itin;
      trx.itin().push_back(&itin);
      PaxType paxType;
      paxType.paxType() = "MIL";

      FarePath farePath;
      farePath.paxType() = &paxType;
      itin.farePath().push_back(&farePath);
      PricingUnit pricingUnit;
      farePath.pricingUnit().push_back(&pricingUnit);
      FareUsage fareUsage;

      PaxTypeFare paxTypeFare;
      paxTypeFare.paxType() = &paxType;

      Fare fare;

      FareInfo fareInfo;
      fareInfo._market1 = "FRA";
      fareInfo._market2 = "ORD";
      fareInfo._currency = "EUR";
      fareInfo._carrier = "AA";
      fareInfo._directionality = FROM;

      Fare mockFare;
      FareMarket fm1;
      mockFare.initialize(Fare::FS_International, &fareInfo, fm1);

      paxTypeFare.setFare(&mockFare);
      fareUsage.paxTypeFare() = &paxTypeFare;
      pricingUnit.fareUsage().push_back(&fareUsage);

      DataHandle dataHandle;
      LOG4CXX_DEBUG(_logger, "Created DataHandle");
      DateTime travelDate = DateTime::localTime();
      LOG4CXX_DEBUG(_logger, "Created TravelDate");

      LocCode fra("FRA");
      LOG4CXX_DEBUG(_logger, "Location Code: " << fra);

      const tse::Loc* loc = dataHandle.getLoc(fra, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for FRA is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      PricingOptions options;
      agent->agentLocation() = loc;
      trx.setRequest(&request);
      trx.setOptions(&options);

      request.ticketingAgent() = agent;

      BSRCurrencyConverter bsrConverter;
      /*
         Money eur(318,"EUR");
         Money usd("USD"); //result should be

         CurrencyConversionRequest
         ccRequest(usd,eur,travelDate,*(trx.getRequest()),trx.dataHandle());
      */

      bool rc = bsrConverter.hasIndirectEquivAmtOverride(trx);

      LOG4CXX_DEBUG(_logger, "hasIndirectEquivAmtOverride returned: " << rc);

      if (!(options.currencyOverride().empty()))
        LOG4CXX_DEBUG(_logger, "Equivalent currency override is: " << options.currencyOverride());

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testIndirectEquivAmtOverrideForParis()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing testIndirectEquivAmtOverrideForParis");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      Itin itin;
      trx.itin().push_back(&itin);
      PaxType paxType;
      paxType.paxType() = "MIL";

      FarePath farePath;
      farePath.paxType() = &paxType;
      itin.farePath().push_back(&farePath);
      PricingUnit pricingUnit;
      farePath.pricingUnit().push_back(&pricingUnit);
      FareUsage fareUsage;

      PaxTypeFare paxTypeFare;
      paxTypeFare.paxType() = &paxType;

      Fare fare;

      FareInfo fareInfo;
      fareInfo._market1 = "CDG";
      fareInfo._market2 = "ORD";
      fareInfo._currency = "USD";
      fareInfo._carrier = "AA";
      fareInfo._directionality = FROM;

      Fare mockFare;
      FareMarket fm2;
      mockFare.initialize(Fare::FS_International, &fareInfo, fm2);

      paxTypeFare.setFare(&mockFare);
      fareUsage.paxTypeFare() = &paxTypeFare;
      pricingUnit.fareUsage().push_back(&fareUsage);

      DataHandle dataHandle;
      LOG4CXX_DEBUG(_logger, "Created DataHandle");
      DateTime travelDate = DateTime::localTime();
      LOG4CXX_DEBUG(_logger, "Created TravelDate");

      LocCode par("PAR");
      LOG4CXX_DEBUG(_logger, "Location Code: " << par);

      const tse::Loc* loc = dataHandle.getLoc(par, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for PAR is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      PricingOptions options;
      trx.setOptions(&options);

      BSRCurrencyConverter bsrConverter;

      bool rc = bsrConverter.hasIndirectEquivAmtOverride(trx);

      LOG4CXX_DEBUG(_logger, "hasIndirectEquivAmtOverride returned: " << rc);

      if (!(options.currencyOverride().empty()))
      {
        LOG4CXX_DEBUG(_logger, "Equivalent currency override is: " << options.currencyOverride());
      }
      else
        LOG4CXX_DEBUG(_logger,
                      "Equivalent currency override is empty use default conversion process");

      stopWatch.stop();

      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testConversionFacade()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Conversion Facade");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      CurrencyConversionFacade ccFacade;
      // CarrierCode carrier("AA");
      // PaxTypeCode paxType("ADT");

      DataHandle dataHandle;
      LOG4CXX_DEBUG(_logger, "Created DataHandle");
      LocCode mad("MAD");
      DateTime ticketingDate = DateTime::localTime();
      LOG4CXX_DEBUG(_logger, "Created TicketingDate");

      LOG4CXX_DEBUG(_logger, "Location Code: " << mad);

      const tse::Loc* loc = dataHandle.getLoc(mad, ticketingDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for MAD is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      trx.getRequest()->ticketingDT() = ticketingDate;
      request.ticketingAgent() = agent;
      Itin itin;
      PricingOptions options;
      trx.setOptions(&options);
      trx.itin().push_back(&itin);

      Money eur(100, "EUR");
      Money usd("USD"); // result should be 34.44

      ccFacade.convert(usd, eur, trx);

      LOG4CXX_DEBUG(_logger, "Converted EUR amount in USD : " << usd.value());

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testZeroSourceAmt()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      PricingRequest request;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Zero Source Amount");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      BSRCurrencyConverter bsrConverter;
      CarrierCode carrier("AA");
      PaxTypeCode paxType("ADT");
      BSRCollectionResults bsrResults(carrier, paxType);

      DataHandle dataHandle;
      LOG4CXX_DEBUG(_logger, "Created DataHandle");
      LocCode rio("RIO");
      DateTime travelDate = DateTime::localTime();
      LOG4CXX_DEBUG(_logger, "Created TravelDate");

      bool convertRC = false;

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;

      Money brl(0.0, "BRL");
      Money usd("USD");

      CurrencyConversionRequest request1(
          usd, brl, travelDate, *(trx.getRequest()), trx.dataHandle());
      request1.setTrx(&trx);

      convertRC = bsrConverter.convert(request1, &bsrResults);

      LOG4CXX_DEBUG(_logger, "Converted USD amount in Brazilian BRL's : " << usd.value());

      CPPUNIT_ASSERT(convertRC != false);

      Money jpy(-0.001, "JPY");
      Money usd2("USD");

      LocCode nrt("NRT");
      LOG4CXX_DEBUG(_logger, "Testing BSR Direct Conversion");
      LOG4CXX_DEBUG(_logger, "Location Code: " << nrt);

      const Loc* loc2 = dataHandle.getLoc(nrt, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location vector");

      if (loc2)
      {

        NationCode nationCode = loc2->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for NRT is: " << nationCode);
      }

      // tse::Agent *agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc2;
      trx.getRequest()->ticketingAgent() = agent;

      CurrencyConversionRequest request2(usd2,
                                         jpy,
                                         travelDate,
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         false,
                                         CurrencyConversionRequest::DC_CONVERT);
      request2.setTrx(&trx);

      request2.salesLocCurrency() = "JPY";
      request2.commonSalesLocCurrency() = true;

      convertRC = bsrConverter.convert(request2, &bsrResults);

      LOG4CXX_DEBUG(_logger, "Converted USD amount in JPY " << usd2.value());

      CPPUNIT_ASSERT(convertRC != false);

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testFacadeRound()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      Itin itin;
      PricingRequest request;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Facade Round");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      trx.setRequest(&request);
      trx.ticketingDate() = DateTime::localTime();
      itin.useInternationalRounding() = false;
      trx.itin().push_back(&itin);
      MoneyAmount usdAmount = 212.56;
      Money usd(usdAmount, "USD");

      CurrencyConversionFacade ccFacade;

      bool roundRC = ccFacade.round(usd, trx, false);

      CPPUNIT_ASSERT(roundRC != false);

      Money nuc(usdAmount, "NUC");

      roundRC = ccFacade.round(nuc, trx, false);

      CPPUNIT_ASSERT(roundRC != false);

      Money brl(usdAmount, "BRL");

      roundRC = ccFacade.round(brl, trx, false);

      CPPUNIT_ASSERT(roundRC != false);

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testFacadeBSRConversionWithResults()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;
      Itin itin;
      PricingRequest request;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Facade Round");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      trx.setRequest(&request);
      trx.ticketingDate() = DateTime::localTime();
      itin.useInternationalRounding() = false;
      trx.itin().push_back(&itin);

      CurrencyConversionFacade ccFacade;
      CarrierCode carrier("AA");
      PaxTypeCode paxType("ADT");
      BSRCollectionResults bsrResults(carrier, paxType);

      DataHandle dataHandle;
      LOG4CXX_DEBUG(_logger, "Created DataHandle");
      LocCode rio("RIO");
      DateTime travelDate = DateTime::localTime();
      LOG4CXX_DEBUG(_logger, "Created TravelDate");

      bool convertRC = false;

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {

        NationCode nationCode = loc->nation();

        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      PricingOptions options;
      trx.setOptions(&options);
      request.ticketingAgent() = agent;

      Money brl(100, "BRL");
      Money usd("USD"); // result should be 34.44

      convertRC = ccFacade.convert(
          usd, brl, trx, true, CurrencyConversionRequest::OTHER, false, &bsrResults);

      CPPUNIT_ASSERT(convertRC != false);

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

private:
  CurrencyDataHandle* _mockDataHandle;
  TestMemHandle _memHandle;
  static log4cxx::LoggerPtr _logger;
};

log4cxx::LoggerPtr
BSRCurrencyConverterTest::_logger(
    log4cxx::Logger::getLogger("atseintl.Currency.test.BSRCurrencyConverterTest"));

CPPUNIT_TEST_SUITE_REGISTRATION(BSRCurrencyConverterTest);
}
