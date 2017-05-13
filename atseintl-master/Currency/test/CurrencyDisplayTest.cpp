#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/StopWatch.h"
#include "Currency/CurrencyService.h"
#include "Currency/LocalCurrencyDisplay.h"
#include "Currency/test/MockDataHandle.h"
#include "DataModel/Agent.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/StaticObjectPool.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"

#include <iostream>
#include <string>

#include <time.h>

using namespace std;

namespace tse
{

class CurrencyDisplayTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CurrencyDisplayTest);
  CPPUNIT_TEST(testDisplayAllMultiplicativeRates);
  CPPUNIT_TEST(testDisplayAllReciprocalRates);
  CPPUNIT_TEST(testDisplaySpecifiedRates);
  CPPUNIT_TEST(testDisplayHistoricalAndFutureRates);
  CPPUNIT_TEST(testConvertDirectGBPToBRL);
  CPPUNIT_TEST(testConvertDirectBRLToGBP);
  CPPUNIT_TEST(testConvertDirectGBPToCRC);
  CPPUNIT_TEST(testConvertDirectBRLToGBPWithCountryOverride);
  CPPUNIT_TEST(testConvertDirectBRLToGBPWithPastDate);
  CPPUNIT_TEST(testDisplaySOSToGBP);
  CPPUNIT_TEST(testDisplayFutureRates);
  CPPUNIT_TEST(testConvertUSDToJPY);
  CPPUNIT_TEST(testConvertUSDToEUR);
  CPPUNIT_TEST(testGetFutureRate);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _server = _memHandle.create<MockTseServer>();
    _mockDataHandle = _memHandle.create<CurrencyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void testDisplayAllMultiplicativeRates()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;
      CurrencyService* currencyService = new CurrencyService("CURRENCY_SVC", *_server);

      trx.sourceCurrency() = "CUR";
      trx.commandType() = 'D';
      trx.eprBDK() = 'T';
      trx.reciprocal() = 'T';
      trx.pssLocalDate() = DateTime::localTime();

      if ((trx.eprBDK() == 'T' && trx.reciprocal() == 'T') ||
          (trx.eprBDK() == 'F' && !(trx.reciprocal() == 'T')))
      {
        LOG4CXX_DEBUG(_logger, "Formatting multiplicative rates");
      }
      else
        LOG4CXX_DEBUG(_logger, "Formatting Reciprocal rates");

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Retrieving All Rates DC*CUR ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode rio("RIO");

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      NationCode nationCode;

      if (loc)
      {
        nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;

      trx.setRequest(&request);

      int argc = 0;
      char** argv = 0;

      currencyService->initialize(argc, argv);

      LocalCurrencyDisplay currencyDisplay(trx);
      bool displayRC = currencyDisplay.displayAllRates();

      //   bool displayRC = trx.process(*currencyService);

      CPPUNIT_ASSERT(displayRC != false);

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      RoundingFactor factor;
      CurrencyNoDec noDec;
      RoundingRule rule;

      CurrencyCode curCode("USD");

      bool taxRC = currencyDisplay.getTaxRounding(curCode, travelDate, factor, noDec, rule);

      LOG4CXX_DEBUG(_logger, "Return from getTaxRounding: " << taxRC);

      delete currencyService;

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

  void testDisplayAllReciprocalRates()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;
      CurrencyService* currencyService = new CurrencyService("CURRENCY_SVC", *_server);

      trx.sourceCurrency() = "CUR";
      trx.commandType() = 'D';
      trx.eprBDK() = 'T';
      trx.pssLocalDate() = DateTime::localTime();

      if ((trx.eprBDK() == 'T' && trx.reciprocal() == 'T') ||
          (trx.eprBDK() == 'F' && !(trx.reciprocal() == 'T')))
      {
        LOG4CXX_DEBUG(_logger, "Formatting multiplicative rates");
      }
      else
        LOG4CXX_DEBUG(_logger, "Formatting Reciprocal rates");

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Retrieving All Rates DC*CUR ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode rio("RIO");

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      NationCode nationCode;

      if (loc)
      {
        nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;

      trx.setRequest(&request);

      LocalCurrencyDisplay currencyDisplay(trx);
      bool displayRC = currencyDisplay.displayAllRates();

      // bool displayRC = trx.process(*currencyService);

      CPPUNIT_ASSERT(displayRC != false);

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      RoundingFactor factor;
      CurrencyNoDec noDec;
      RoundingRule rule;

      CurrencyCode curCode("USD");

      bool taxRC = currencyDisplay.getTaxRounding(curCode, travelDate, factor, noDec, rule);

      LOG4CXX_DEBUG(_logger, "Return from getTaxRounding: " << taxRC);

      delete currencyService;

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
  void testDisplaySpecifiedRates()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;

      trx.sourceCurrency() = "GBP";
      trx.commandType() = 'D';
      trx.eprBDK() = 'T';
      trx.reciprocal() = 'T';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Display Specified Rates DC*GBP ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode rio("RIO");

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;

      trx.setRequest(&request);

      LocalCurrencyDisplay currencyDisplay(trx);

      CurrencyCode sourceCurrency("GBP");

      bool displayRC = currencyDisplay.displayRate(sourceCurrency);

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

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

  void testDisplayHistoricalAndFutureRates()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;

      trx.sourceCurrency() = "CAD";
      DateTime pastDate(2004, tse::May, 1);
      trx.baseDT() = pastDate;
      trx.commandType() = 'D';
      trx.eprBDK() = 'T';
      trx.reciprocal() = 'T';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Display Historical Rates DC*GBP ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode rio("RIO");

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;

      trx.setRequest(&request);

      LocalCurrencyDisplay currencyDisplay(trx);

      CurrencyCode sourceCurrency("GBP");

      bool displayRC = currencyDisplay.displayRate(sourceCurrency);

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

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

  void testConvertDirectGBPToBRL()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;
      CurrencyService* currencyService = new CurrencyService("CURRENCY_SVC", *_server);

      trx.sourceCurrency() = "GBP";
      trx.amount() = 500.00;
      trx.targetCurrency() = "BRL";
      DateTime currentDate = DateTime::localTime();
      trx.baseDT() = currentDate;
      trx.commandType() = 'C';
      trx.eprBDK() = 'F';
      trx.reciprocal() = 'F';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing DC'GBP500/BRL ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode rio("RIO");

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      request.ticketingAgent()->agentCity() = rio;

      trx.setRequest(&request);

      bool displayRC = trx.process(*currencyService);

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

      delete currencyService;

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

  void testConvertDirectBRLToGBP()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;

      trx.sourceCurrency() = "BRL";
      trx.amount() = 500.00;
      trx.targetCurrency() = "GBP";
      DateTime currentDate = DateTime::localTime();
      trx.baseDT() = currentDate;
      trx.commandType() = 'C';
      trx.eprBDK() = 'F';
      trx.reciprocal() = 'F';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing DC'BRL500/GBP ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode rio("RIO");

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      request.ticketingAgent()->agentCity() = rio;

      trx.setRequest(&request);

      LocalCurrencyDisplay currencyDisplay(trx);

      bool displayRC = currencyDisplay.convert();

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

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

  void testConvertDirectGBPToCRC()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;

      trx.sourceCurrency() = "GBP";
      trx.amount() = 500.00;
      trx.targetCurrency() = "CRC";
      DateTime currentDate = DateTime::localTime();
      trx.baseDT() = currentDate;
      trx.commandType() = 'C';
      trx.eprBDK() = 'F';
      trx.reciprocal() = 'F';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing DC'GBP500/CRC ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode rio("RIO");

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      request.ticketingAgent()->agentCity() = rio;

      trx.setRequest(&request);

      LocalCurrencyDisplay currencyDisplay(trx);

      bool displayRC = currencyDisplay.convert();

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

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
  void testConvertDirectBRLToGBPWithCountryOverride()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;

      trx.sourceCurrency() = "BRL";
      trx.amount() = 500.00;
      trx.targetCurrency() = "GBP";
      DateTime currentDate = DateTime::localTime();
      trx.baseDT() = currentDate;
      trx.commandType() = 'C';
      trx.eprBDK() = 'F';
      trx.reciprocal() = 'F';
      trx.baseCountry() = "JP";
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing DC'BRL500/GBP with Country Override JP ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode rio("RIO");

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      request.ticketingAgent()->agentCity() = rio;

      trx.setRequest(&request);

      LocalCurrencyDisplay currencyDisplay(trx);

      bool displayRC = currencyDisplay.convert();

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

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

  void testConvertDirectBRLToGBPWithPastDate()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;

      DateTime pastDate(2004, tse::May, 1);

      trx.sourceCurrency() = "BRL";
      trx.amount() = 500.00;
      trx.targetCurrency() = "GBP";
      trx.baseDT() = pastDate;
      trx.commandType() = 'C';
      trx.eprBDK() = 'F';
      trx.reciprocal() = 'F';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing DC'BRL500/GBP with PastDate 5/01/2004 ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode rio("RIO");

      LOG4CXX_DEBUG(_logger, "Location Code: " << rio);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      request.ticketingAgent()->agentCity() = rio;

      trx.setRequest(&request);

      LocalCurrencyDisplay currencyDisplay(trx);

      bool displayRC = currencyDisplay.convert();

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

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

  void testDisplaySOSToGBP()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;

      DateTime date = DateTime::localTime();

      trx.sourceCurrency() = "SOS";
      trx.amount() = 50000.00;
      trx.targetCurrency() = "GBP";
      trx.baseDT() = date;
      trx.commandType() = 'D';
      trx.eprBDK() = 'F';
      trx.reciprocal() = 'F';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Display SOS - GBP Rate");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode mgq("MGQ");

      LOG4CXX_DEBUG(_logger, "Location Code: " << mgq);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(mgq, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for MGQ is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      request.ticketingAgent()->agentCity() = mgq;

      trx.setRequest(&request);

      // CurrencyCode sourceCurrency("SOS");

      CurrencyService* currencyService = new CurrencyService("CURRENCY_SVC", *_server);

      bool displayRC = trx.process(*currencyService);

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

      delete currencyService;

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

  void testDisplayFutureRates()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;

      trx.sourceCurrency() = "ARS";
      // DateTime pastDate(2004,tse::May,1);
      // trx.baseDT()         = pastDate;
      trx.commandType() = 'D';
      // trx.eprBDK()         = 'T';
      // trx.reciprocal()     = 'T';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Display Future Rates DC*ARS ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode dfw("DFW");

      LOG4CXX_DEBUG(_logger, "Location Code: " << dfw);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(dfw, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for DFW is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;

      trx.setRequest(&request);

      LocalCurrencyDisplay currencyDisplay(trx);

      CurrencyCode sourceCurrency("ARS");

      bool displayRC = currencyDisplay.displayRate(sourceCurrency);

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

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

  void testConvertUSDToJPY()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;
      CurrencyService* currencyService = new CurrencyService("CURRENCY_SVC", *_server);

      trx.sourceCurrency() = "USD";
      trx.amount() = 500.00;
      trx.targetCurrency() = "JPY";
      DateTime currentDate = DateTime::localTime();
      trx.baseDT() = currentDate;
      trx.commandType() = 'C';
      trx.eprBDK() = 'F';
      trx.reciprocal() = 'F';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing DC'USD500/JPY ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode dfw("DFW");

      LOG4CXX_DEBUG(_logger, "Location Code: " << dfw);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(dfw, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for DFW is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      request.ticketingAgent()->agentCity() = dfw;

      trx.setRequest(&request);

      bool displayRC = trx.process(*currencyService);

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

      delete currencyService;

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

  void testConvertUSDToEUR()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      PricingRequest request;
      CurrencyService* currencyService = new CurrencyService("CURRENCY_SVC", *_server);

      trx.sourceCurrency() = "USD";
      trx.amount() = 500.00;
      trx.targetCurrency() = "EUR";
      DateTime currentDate = DateTime::localTime();
      trx.baseDT() = currentDate;
      trx.commandType() = 'C';
      trx.eprBDK() = 'F';
      trx.reciprocal() = 'F';
      trx.pssLocalDate() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing DC'USD500/EUR ");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LocCode dfw("DFW");

      LOG4CXX_DEBUG(_logger, "Location Code: " << dfw);

      DateTime travelDate = DateTime::localTime();

      const tse::Loc* loc = dataHandle.getLoc(dfw, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for DFW is: " << nationCode);
      }

      LOG4CXX_DEBUG(_logger, "Before agent get");
      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      LOG4CXX_DEBUG(_logger, "After agent get");

      agent->agentLocation() = loc;
      trx.setRequest(&request);
      request.ticketingAgent() = agent;
      request.ticketingAgent()->agentCity() = dfw;

      trx.setRequest(&request);

      bool displayRC = trx.process(*currencyService);

      string responseStr = trx.response().str();

      LOG4CXX_DEBUG(_logger, responseStr);

      CPPUNIT_ASSERT(displayRC != false);

      delete currencyService;

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

  void testGetFutureRate()
  {
    try
    {
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Get Future Rate");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyTrx trx;
      trx.pssLocalDate() = DateTime::localTime();
      LocalCurrencyDisplay currencyDisplay(trx);
      Year theYear(2005);
      Month theMonth(6);
      Day theDay(3);
      DateTime expirationDate(theYear, theMonth, theDay);
      DateTime effectiveDate;

      CurrencyCode primeCurrency("BRL");
      CurrencyCode currency("USD");
      CurrencyNoDec rateNoDec = 0;
      ExchRate futureRate = 0;
      Indicator rateType;

      bool futureRateRC = currencyDisplay.getFutureBSRRate(
          primeCurrency, currency, expirationDate, effectiveDate, rateNoDec, rateType, futureRate);

      if (futureRateRC)
      {
        LOG4CXX_DEBUG(_logger, "Future Rate " << futureRate);
        LOG4CXX_DEBUG(_logger, "rateNoDec " << rateNoDec);
      }
      else
        LOG4CXX_DEBUG(_logger, "Future Rate not available ");

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
  MockTseServer* _server;
  TestMemHandle _memHandle;
  static log4cxx::LoggerPtr _logger;
};
log4cxx::LoggerPtr
CurrencyDisplayTest::_logger(
    log4cxx::Logger::getLogger("atseintl.Currency.test.CurrencyDisplayTest"));

CPPUNIT_TEST_SUITE_REGISTRATION(CurrencyDisplayTest);
}
