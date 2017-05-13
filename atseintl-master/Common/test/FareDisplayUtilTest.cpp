#include "Common/FareDisplayUtil.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/FareDisplayPref.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <iostream>
#include <time.h>

using namespace std;
namespace tse
{

class FareDisplayUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayUtilTest);
  CPPUNIT_TEST(testGetFareDisplayTrxShouldCastToCorrectType);
  CPPUNIT_TEST(testGetFareDisplayTrxShouldFailCastOfIncorrectType);
  CPPUNIT_TEST(testSetDisplayOnlyShouldNotModifyPaxTypeFareIfNotAFareDisplayTrx);
  CPPUNIT_TEST(testSetDisplayOnlyShouldNotSetDisplayOnlyIfFlagSetToFalse);
  CPPUNIT_TEST(testSetDisplayOnlyShouldSetDisplayOnlyIfFlagSetToTrue);
  CPPUNIT_TEST(testFormatMonthWithMonthValueZero);
  CPPUNIT_TEST(testFormatMonthWithMonthValueNegative_BadData);
  CPPUNIT_TEST(testFormatMonthWithMonthValue_BadData);
  CPPUNIT_TEST(testFormatMonthDDMM);
  CPPUNIT_TEST(testFormatMonthDDMMY);
  CPPUNIT_TEST(testFormatMonthDDMMM);
  CPPUNIT_TEST(testFormatMonthDDMMMYorDDMMMYY);
  CPPUNIT_TEST(testFormatMonthUnknown);
  CPPUNIT_TEST(testFormatYearWithYearValueZeroForEachFormat);
  CPPUNIT_TEST(testFormatYearForDDMMandDDMMM);
  CPPUNIT_TEST(testFormatYearForDDMMMY);
  CPPUNIT_TEST(testFormatYearForDDMMY);
  CPPUNIT_TEST(testFormatYearForDDMMMYY);
  CPPUNIT_TEST(testSplitLinesBasic);
  CPPUNIT_TEST(testSplitLinesNoTerminators);
  CPPUNIT_TEST(testSplitLinesNoTerminatorsBarelyEnds);
  CPPUNIT_TEST(testSplitLinesWithTerminators);
  CPPUNIT_TEST(testSplitLinesWithTerminators2);
  CPPUNIT_TEST(testSplitLinesMultipleTerminators);
  CPPUNIT_TEST(testIsWebUserShouldReturnFalseIfNoAgentTJR);
  CPPUNIT_TEST(testIsWebUserShouldReturnFalseIfTvlyLocationEqualsNo);
  CPPUNIT_TEST(testIsWebUserShouldReturnTrueIfTvlyLocationEqualsYes);
  CPPUNIT_TEST(testIsAbacusUserShouldReturnTrueIfAgentCxrCodeIsABACUS_MULTIHOST_ID);
  CPPUNIT_TEST(testIsAxessUserShouldReturnFalseIfNoAgent);
  CPPUNIT_TEST(testIsAxessUserShouldReturnFalseIfAgentCxrCodeNotAXESS_MULTIHOST_ID);
  CPPUNIT_TEST(testPreventHistoricalBrandingWhenIsShopperRequest);
  CPPUNIT_TEST(testIsBrandServiceEnabledWhenNotOptOut);
  CPPUNIT_TEST(testNoBrandingWhenHistoricalRequest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }
  void tearDown() { _memHandle.clear(); }

  void testGetFareDisplayTrxShouldCastToCorrectType()
  {
    PricingTrx* pricingTrx = _memHandle.create<FareDisplayTrx>();
    FareDisplayTrx* trx;
    bool ret = FareDisplayUtil::getFareDisplayTrx(pricingTrx, trx);

    CPPUNIT_ASSERT_EQUAL(true, ret);
    CPPUNIT_ASSERT(pricingTrx == trx);
  }

  void testGetFareDisplayTrxShouldFailCastOfIncorrectType()
  {
    PricingTrx* pricingTrx = _memHandle.create<PricingTrx>();
    FareDisplayTrx* trx;
    bool ret = FareDisplayUtil::getFareDisplayTrx(pricingTrx, trx);

    CPPUNIT_ASSERT_EQUAL(false, ret);
    CPPUNIT_ASSERT(trx == 0);
  }

  void testSetDisplayOnlyShouldNotModifyPaxTypeFareIfNotAFareDisplayTrx()
  {
    PricingTrx* pricingTrx = _memHandle.create<PricingTrx>();
    PaxTypeFare paxTypeFare;
    FareDisplayInfo fareDisplayInfo;
    paxTypeFare.fareDisplayInfo() = &fareDisplayInfo;
    fareDisplayInfo.displayOnly() = false;
    fareDisplayInfo.isMinMaxFare() = false;
    bool isDisplayOnly = true;
    bool isMinMaxFare = true;

    FareDisplayUtil::setDisplayOnly(*pricingTrx, paxTypeFare, isDisplayOnly, isMinMaxFare);

    CPPUNIT_ASSERT_EQUAL(false, fareDisplayInfo.displayOnly());
    CPPUNIT_ASSERT_EQUAL(false, fareDisplayInfo.isMinMaxFare());
  }

  void testSetDisplayOnlyShouldNotSetDisplayOnlyIfFlagSetToFalse()
  {
    PricingTrx* pricingTrx = _memHandle.create<FareDisplayTrx>();
    PaxTypeFare paxTypeFare;
    FareDisplayInfo fareDisplayInfo;
    paxTypeFare.fareDisplayInfo() = &fareDisplayInfo;
    fareDisplayInfo.displayOnly() = false;
    fareDisplayInfo.isMinMaxFare() = false;
    bool isDisplayOnly = false;
    bool isMinMaxFare = true;

    FareDisplayUtil::setDisplayOnly(*pricingTrx, paxTypeFare, isDisplayOnly, isMinMaxFare);

    CPPUNIT_ASSERT_EQUAL(false, fareDisplayInfo.displayOnly());
    CPPUNIT_ASSERT_EQUAL(true, fareDisplayInfo.isMinMaxFare());
  }

  void testSetDisplayOnlyShouldSetDisplayOnlyIfFlagSetToTrue()
  {
    PricingTrx* pricingTrx = _memHandle.create<FareDisplayTrx>();
    PaxTypeFare paxTypeFare;
    FareDisplayInfo fareDisplayInfo;
    paxTypeFare.fareDisplayInfo() = &fareDisplayInfo;
    fareDisplayInfo.displayOnly() = false;
    fareDisplayInfo.isMinMaxFare() = false;
    bool isDisplayOnly = true;
    bool isMinMaxFare = false;

    FareDisplayUtil::setDisplayOnly(*pricingTrx, paxTypeFare, isDisplayOnly, isMinMaxFare);

    CPPUNIT_ASSERT_EQUAL(true, fareDisplayInfo.displayOnly());
    CPPUNIT_ASSERT_EQUAL(false, fareDisplayInfo.isMinMaxFare());
  }

  void testFormatMonthWithMonthValueZero()
  {
    std::ostringstream stringStream;
    int32_t month = 0;
    Indicator format = '1';
    FareDisplayUtil::formatMonth(month, stringStream, format);

    CPPUNIT_ASSERT_EQUAL((std::string) " ", stringStream.str());
  }

  void testFormatMonthWithMonthValueNegative_BadData()
  {
    std::ostringstream stringStream;
    int32_t month = -2;
    Indicator format = '1';
    FareDisplayUtil::formatMonth(month, stringStream, format);

    CPPUNIT_ASSERT_EQUAL((std::string) " ", stringStream.str());
  }

  void testFormatMonthWithMonthValue_BadData()
  {
    std::ostringstream stringStream;
    int32_t month = 14;
    Indicator format = '1';
    FareDisplayUtil::formatMonth(month, stringStream, format);

    CPPUNIT_ASSERT_EQUAL((std::string) " ", stringStream.str());
  }

  void testFormatMonthDDMM()
  {
    std::ostringstream stringStream;
    int32_t month = 1;
    Indicator format = '1';
    FareDisplayUtil::formatMonth(month, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "JA", stringStream.str());

    stringStream.str("");
    month = 10;
    FareDisplayUtil::formatMonth(month, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "OC", stringStream.str());
  }

  void testFormatMonthDDMMY()
  {
    std::ostringstream stringStream;
    int32_t month = 1;
    Indicator format = '3';
    FareDisplayUtil::formatMonth(month, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "JA", stringStream.str());

    stringStream.str("");
    month = 10;
    FareDisplayUtil::formatMonth(month, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "OC", stringStream.str());
  }

  void testFormatMonthDDMMM()
  {
    std::ostringstream stringStream;
    int32_t month = 1;
    Indicator format = '2';
    FareDisplayUtil::formatMonth(month, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "JAN", stringStream.str());

    stringStream.str("");
    month = 10;
    FareDisplayUtil::formatMonth(month, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "OCT", stringStream.str());
  }

  void testFormatMonthDDMMMYorDDMMMYY()
  {
    std::ostringstream stringStream;
    int32_t month = 1;
    Indicator format = '4';
    FareDisplayUtil::formatMonth(month, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "JAN", stringStream.str());

    stringStream.str("");
    month = 10;
    FareDisplayUtil::formatMonth(month, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "OCT", stringStream.str());
  }

  void testFormatMonthUnknown()
  {
    std::ostringstream stringStream;
    int32_t month = 1;
    Indicator format = '5'; // in format year this is DDMMMYY
    FareDisplayUtil::formatMonth(month, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "JAN", stringStream.str());
  }

  void testFormatYearWithYearValueZeroForEachFormat()
  {
    std::ostringstream stringStream;
    int32_t year = 0;
    Indicator format = '1'; // DDMM
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "", stringStream.str());

    format = '2'; // DDMMM
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "", stringStream.str());

    format = '3'; // DDMMY
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "0", stringStream.str());

    // Must reset stream
    stringStream.str("");
    format = '4'; // DDMMMY
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "0", stringStream.str());

    // Must reset stream
    stringStream.str("");
    format = '5'; // DDMMMYY
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) " 0", stringStream.str());
  }

  void testFormatYearForDDMMandDDMMM()
  {
    std::ostringstream stringStream;
    int32_t year = 1;
    Indicator format = '1'; // DDMM
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "", stringStream.str());

    format = '2'; // DDMMM
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "", stringStream.str());
  }

  void testFormatYearForDDMMY()
  {
    std::ostringstream stringStream;
    int32_t year = 1;
    Indicator format = '3'; // DDMMY
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "1", stringStream.str());

    // Must reset stream
    stringStream.str("");
    year = 110;
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "110", stringStream.str());

    // Must reset stream
    stringStream.str("");
    year = 2008;
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "2008", stringStream.str());
  }

  void testFormatYearForDDMMMY()
  {
    std::ostringstream stringStream;
    int32_t year = 1;
    Indicator format = '4'; // DDMMMY
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "1", stringStream.str());

    // Must reset stream
    stringStream.str("");
    year = 110;
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "110", stringStream.str());

    // Must reset stream
    stringStream.str("");
    year = 2008;
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "2008", stringStream.str());
  }

  void testFormatYearForDDMMMYY()
  {
    std::ostringstream stringStream;
    int32_t year = 1;
    Indicator format = '5'; // DDMMMYY
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) " 1", stringStream.str());

    // Must reset stream
    stringStream.str("");
    year = 110;
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "110", stringStream.str());

    // Must reset stream
    stringStream.str("");
    year = 2008;
    FareDisplayUtil::formatYear(year, stringStream, format);
    CPPUNIT_ASSERT_EQUAL((std::string) "2008", stringStream.str());
  }

  void testSplitLinesBasic()
  {
    string text = "SentenceThe quick brown fox\n";
    string message = FareDisplayUtil::splitLines(text, text.length(), "\n", 8, "Test Message:");
    CPPUNIT_ASSERT_EQUAL((std::string) "Sentence\nTest Message:The quick brown fox\n", message);
  }

  void testSplitLinesNoTerminators()
  {
    const string text =
      "Lorem ipsum dolor sit amet, consectetur adipisicing elit,\n"
      "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
      "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
      "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor\n"
      "in reprehenderit in voluptate velit esse cillum dolore eu fugiat\n"
      "nulla pariatur. Excepteur sint occaecat cupidatat non proident,\n"
      "sunt in culpa qui officia deserunt mollit anim id est laborum.";

    const string expected =
      "Lorem ipsum dolor sit amet, consectetur \n"
      "prefix:adipisicing elit,\nsed do eiusmod tempor incididunt ut labore\n"
      "prefix: et dolore magna aliqua.\nUt enim ad minim veniam, quis nostr\n"
      "prefix:ud exercitation ullamco laboris\nnisi ut aliquip ex ea commod\n"
      "prefix:o consequat. Duis aute irure dolor\nin reprehenderit in volup\n"
      "prefix:tate velit esse cillum dolore eu fugiat\nnulla pariatur. Exce\n"
      "prefix:pteur sint occaecat cupidatat non proident,\nsunt in culpa qu\n"
      "prefix:i officia deserunt mollit anim id est laborum.";

    const string message = FareDisplayUtil::splitLines(text, 60, "", 40, "prefix:");

    CPPUNIT_ASSERT_EQUAL(expected, message);
  }

  void testSplitLinesNoTerminatorsBarelyEnds()
  {
    const string text =
      "Lorem ipsum dolor sit amet, consectetur adipisicing elit,\n"
      "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
      "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
      "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor\n"
      "in reprehenderit in voluptate velit esse cillum dolore eu fugiat\n"
      "nulla pariatur. Excepteur sint occaecat cupidatat non proident,\n"
      "sunt in culpa qui officia deserunt mollit anim id est laborum."
      "It barely ends";

    const string expected =
      "Lorem ipsum dolor sit amet, consectetur \n"
      "prefix:adipisicing elit,\nsed do eiusmod tempor incididunt ut labore\n"
      "prefix: et dolore magna aliqua.\nUt enim ad minim veniam, quis nostr\n"
      "prefix:ud exercitation ullamco laboris\nnisi ut aliquip ex ea commod\n"
      "prefix:o consequat. Duis aute irure dolor\nin reprehenderit in volup\n"
      "prefix:tate velit esse cillum dolore eu fugiat\nnulla pariatur. Exce\n"
      "prefix:pteur sint occaecat cupidatat non proident,\nsunt in culpa qu\n"
      "prefix:i officia deserunt mollit anim id est laborum.It barely ends";

    const string message = FareDisplayUtil::splitLines(text, 60, "", 40, "prefix:");

    CPPUNIT_ASSERT_EQUAL(expected, message);
  }

  void testSplitLinesWithTerminators()
  {
    const string text =
      "Lorem ipsum dolor sit amet, consectetur adipisicing elit,\n"
      "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
      "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
      "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor\n"
      "in reprehenderit in voluptate velit esse cillum dolore eu fugiat\n"
      "nulla pariatur. Excepteur sint occaecat cupidatat non proident,\n"
      "sunt in culpa qui officia deserunt mollit anim id est laborum.";

    const string expected =
      "Lorem ipsum dolor sit amet, consectetur \n"
      "-> adipisicing elit,\n\n"
      "-> sed do eiusmod tempor incididunt ut labore et dolo\n"
      "-> re magna aliqua.\n\n"
      "-> Ut enim ad minim veniam, quis nostrud exercitation\n"
      "->  ullamco laboris\n\n"
      "-> nisi ut aliquip ex ea commodo consequat. Duis aute\n"
      "->  irure dolor\n\n"
      "-> in reprehenderit in voluptate velit esse cillum do\n"
      "-> lore eu fugiat\n\n"
      "-> nulla pariatur. Excepteur sint occaecat cupidatat \n"
      "-> non proident,\n\n"
      "-> sunt in culpa qui officia deserunt mollit anim id \n"
      "-> est laborum.";

    const string message = FareDisplayUtil::splitLines(text, 50, "\n", 40, "-> ");

    CPPUNIT_ASSERT_EQUAL(expected, message);
  }

  void testSplitLinesWithTerminators2()
  {
    const string text =
      "Lorem ipsum dolor sit amet, consectetur adipisicing elit,\n"
      "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
      "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
      "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor\n"
      "in reprehenderit in voluptate velit esse cillum dolore eu fugiat\n"
      "nulla pariatur. Excepteur sint occaecat cupidatat non proident,\n"
      "sunt in culpa qui officia deserunt mollit anim id est laborum.";

    const string expected =
      "Lorem ipsum dolor sit amet,\n"
      " consectetur adipisicing elit,\n"
      "\nsed do eiusmod tempor incididunt ut labore et dol\n"
      "ore magna aliqua.\nUt enim ad minim veniam,\n"
      " quis nostrud exercitation ullamco laboris\nnisi ut\n"
      " aliquip ex ea commodo consequat. Duis aute irure \n"
      "dolor\nin reprehenderit in voluptate velit esse cil\n"
      "lum dolore eu fugiat\nnulla pariatur. Excepteur sin\n"
      "t occaecat cupidatat non proident,\n"
      "\nsunt in culpa qui officia deserunt mollit anim id\n"
      " est laborum.";

    const string message = FareDisplayUtil::splitLines(text, 50, ",", 40);

    CPPUNIT_ASSERT_EQUAL(expected, message);
  }

  void testSplitLinesMultipleTerminators()
  {
    const string text =
      "Lorem ipsum dolor sit amet, consectetur adipisicing elit,\n"
      "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
      "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
      "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor\n"
      "in reprehenderit in voluptate velit esse cillum dolore eu fugiat\n"
      "nulla pariatur. Excepteur sint occaecat cupidatat non proident,\n"
      "sunt in culpa qui officia deserunt mollit anim id est laborum.";

    const string expected =
      "Lorem ipsum dolor sit amet, consectetur \n"
      "!adipisicing elit,\nsed do eiusmod tempor \n"
      "!incididunt ut labore et dolore magna aliqua.\nUt \n"
      "!enim ad minim veniam, quis nostrud exercitation \n"
      "!ullamco laboris\nnisi ut aliquip ex ea commodo \n"
      "!consequat. Duis aute irure dolor\nin reprehenderit \n"
      "!in voluptate velit esse cillum dolore eu \n"
      "!fugiat\nnulla pariatur. Excepteur sint occaecat \n"
      "!cupidatat non proident,\nsunt in culpa qui officia \n"
      "!deserunt mollit anim id est laborum.";

    const string message = FareDisplayUtil::splitLines(text, 50, " ,.", 40, "!");

    CPPUNIT_ASSERT_EQUAL(expected, message);
  }

  void testIsWebUserShouldReturnFalseIfNoAgentTJR()
  {
    FareDisplayTrx trx;
    FareDisplayRequest request;
    trx.setRequest(&request);
    Agent agent;
    agent.agentTJR() = 0;
    trx.getRequest()->ticketingAgent() = &agent;

    CPPUNIT_ASSERT_EQUAL(false, FareDisplayUtil::isWebUser(trx));
  }

  void testIsWebUserShouldReturnFalseIfTvlyLocationEqualsNo()
  {
    FareDisplayTrx trx;
    FareDisplayRequest request;
    trx.setRequest(&request);
    Agent agent;
    Customer agentTJR;
    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingAgent()->agentTJR() = &agentTJR;
    trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() = NO;

    CPPUNIT_ASSERT_EQUAL(false, FareDisplayUtil::isWebUser(trx));
  }

  void testIsWebUserShouldReturnTrueIfTvlyLocationEqualsYes()
  {
    FareDisplayTrx trx;
    FareDisplayRequest request;
    trx.setRequest(&request);
    Agent agent;
    Customer agentTJR;
    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingAgent()->agentTJR() = &agentTJR;
    trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() = YES;

    CPPUNIT_ASSERT_EQUAL(true, FareDisplayUtil::isWebUser(trx));
  }

  void testIsAbacusUserShouldReturnTrueIfAgentCxrCodeIsABACUS_MULTIHOST_ID()
  {
    FareDisplayTrx trx;
    FareDisplayRequest request;
    trx.setRequest(&request);
    Agent agent;
    agent.cxrCode() = ABACUS_MULTIHOST_ID;
    trx.getRequest()->ticketingAgent() = &agent;
  }

  void testIsAxessUserShouldReturnFalseIfNoAgent()
  {
    FareDisplayTrx trx;
    FareDisplayRequest request;
    trx.setRequest(&request);

    CPPUNIT_ASSERT_EQUAL(false, FareDisplayUtil::isAxessUser(trx));
  }

  void testIsAxessUserShouldReturnFalseIfAgentCxrCodeNotAXESS_MULTIHOST_ID()
  {
    FareDisplayTrx trx;
    FareDisplayRequest request;
    trx.setRequest(&request);
    Agent agent;
    trx.getRequest()->ticketingAgent() = &agent;

    CPPUNIT_ASSERT_EQUAL(false, FareDisplayUtil::isAxessUser(trx));
  }

  class FareDisplayTrx_StubShopperRequest : public FareDisplayTrx
  {
  public:
    bool _isShopperRequest;
    bool _isBrandGroupingOptOut;
    bool isShopperRequest() const { return _isShopperRequest; }
    bool isBrandGroupingOptOut() const { return _isBrandGroupingOptOut; }
  };

  void testPreventHistoricalBrandingWhenIsShopperRequest()
  {
    FareDisplayTrx_StubShopperRequest trx;
    trx._isShopperRequest = true;
    FareDisplayRequest request;
    trx.setRequest(&request);
    DateTime historicalDate = DateTime(2008, 1, 21);
    trx.getRequest()->ticketingDT() = historicalDate;
    MockGlobal::setAllowHistorical(true);
    enableBrandGrouping();
    CPPUNIT_ASSERT(trx.isShopperRequest());
    CPPUNIT_ASSERT(!FareDisplayUtil::isBrandGrouping(trx));
  }

  void enableBrandGrouping()
  {
    TestConfigInitializer::setValue("BRAND_GROUPING", string("Y"), "FAREDISPLAY_SVC");
    CPPUNIT_ASSERT(FareDisplayUtil::isBrandGroupingEnabled());
  }

  void testIsBrandServiceEnabledWhenNotOptOut()
  {
    FareDisplayTrx_StubShopperRequest trx;
    trx._isShopperRequest = false;
    trx._isBrandGroupingOptOut = false;
    FareDisplayOptions options;
    FareDisplayRequest request;
    trx.setRequest(&request);
    trx.setOptions(&options);
    DateTime futureDate = DateTime(2099, 1, 21);
    trx.getRequest()->ticketingDT() = futureDate;
    MockGlobal::setAllowHistorical(true);
    enableBrandGrouping();
    CPPUNIT_ASSERT(!trx.isShopperRequest());
    CPPUNIT_ASSERT(FareDisplayUtil::isBrandGrouping(trx));
  }
  void testNoBrandingWhenHistoricalRequest()
  {
    FareDisplayTrx_StubShopperRequest trx;
    trx._isShopperRequest = false;
    FareDisplayOptions options;
    FareDisplayRequest request;
    trx.setRequest(&request);
    trx.setOptions(&options);
    DateTime historicalDate = DateTime(2008, 1, 21);
    trx.getRequest()->ticketingDT() = historicalDate;
    MockGlobal::setAllowHistorical(false);
    enableBrandGrouping();
    CPPUNIT_ASSERT(FareDisplayUtil::isBrandGrouping(trx));
  }
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayUtilTest);
}
