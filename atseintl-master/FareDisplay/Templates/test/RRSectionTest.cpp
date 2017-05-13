#include "FareDisplay/Templates/RRSection.h"

#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class RRSectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RRSectionTest);

  CPPUNIT_TEST(testGetFareCreationWhenOnlyPercentage);
  CPPUNIT_TEST(testGetFareCreationWhenOnlyRuleAmount);
  CPPUNIT_TEST(testGetFareCreation);
  CPPUNIT_TEST(testDisplayNETFareCreation);
  CPPUNIT_TEST(testDisplayNETFareCreationWhenNoDec);
  CPPUNIT_TEST(testDisplayAdjustedSellingLvlWhenNoDec);
  CPPUNIT_TEST(testDisplayAdjustedSellingLvl);
  CPPUNIT_TEST(testDisplayNetBussinesID);
  CPPUNIT_TEST(testDisplayNetBussinesIDWhenRedistributed);
  CPPUNIT_TEST(testDisplaySellingFareBussinesID);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _fareDispReq = _memHandle.create<FareDisplayRequest>();
    _trx->setRequest(_fareDispReq);
    _agent = _memHandle.create<Agent>();
    _fareDispReq->ticketingAgent() = _agent;

    _rrSection = _memHandle.insert(new RRSection(*_trx));
    _percent = _memHandle.create<std::string>();
    _fareAmt = _memHandle.create<std::string>();
    _currencyCode = _memHandle.create<CurrencyCode>();
    _negRuleData = _memHandle.create<NegPaxTypeFareRuleData>();
    _adjSellCalcData = _memHandle.create<AdjustedSellingCalcData>();
    _secRec = _memHandle.create<NegFareSecurityInfo>();

    _oss = _memHandle.create<std::ostringstream>();
  }

  void tearDown() { _memHandle.clear(); }

  void testGetFareCreationWhenOnlyPercentage()
  {
    *_percent = "15.20";
    bool isPositivePercentage = false;
    RRSection::FareCreation fareCreation(
        RuleConst::NF_CALC_PERCENT, *_percent, *_fareAmt, *_currencyCode, isPositivePercentage);

    *_oss << RRSection::FareCreation::BASE_FARE << SPACE << RRSection::FareCreation::MINUS_TEXT
          << SPACE << *_percent << SPACE << RRSection::FareCreation::PERCENT << PERIOD;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), fareCreation.getFareCreation());
  }

  void testGetFareCreationWhenOnlyRuleAmount()
  {
    *_percent = "";
    *_currencyCode = "USD";
    *_fareAmt = "10.50";
    bool isPositivePercentage = true;
    RRSection::FareCreation fareCreation(
        RuleConst::NF_ADD, *_percent, *_fareAmt, *_currencyCode, isPositivePercentage);

    *_oss << RRSection::FareCreation::BASE_FARE << SPACE << RRSection::FareCreation::PLUS_TEXT << SPACE
        << *_currencyCode << *_fareAmt << PERIOD;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), fareCreation.getFareCreation());
  }

  void testGetFareCreation()
  {
    *_percent = "5.20";
    *_currencyCode = "USD";
    *_fareAmt = "10.50";
    bool isPositivePercentage = false;
    RRSection::FareCreation fareCreation(
        RuleConst::NF_MINUS, *_percent, *_fareAmt, *_currencyCode, isPositivePercentage);

    *_oss << RRSection::FareCreation::BASE_FARE << SPACE << RRSection::FareCreation::MINUS_TEXT
          << SPACE << *_percent << SPACE << RRSection::FareCreation::PERCENT << SPACE
          << RRSection::FareCreation::MINUS_TEXT << SPACE << *_currencyCode << *_fareAmt << PERIOD;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), fareCreation.getFareCreation());
  }

  void testDisplayNETFareCreation()
  {
    *_percent = "5.20";
    *_currencyCode = "USD";
    *_fareAmt = "10.50";
    _negRuleData->calcInd() = RuleConst::NF_MINUS;
    _negRuleData->ruleAmt() = Money(10.5, *_currencyCode);
    _negRuleData->calculatedNegCurrency() = *_currencyCode;
    _negRuleData->noDecAmt() = 2;
    _negRuleData->percent() = 105.2;
    _negRuleData->noDecPercent() = 2;

    _rrSection->displayNETFareCreation(*_negRuleData);

    *_oss << RRSection::INDENTION << RRSection::FareCreation::BASE_FARE << SPACE
          << RRSection::FareCreation::PLUS_TEXT << SPACE << *_percent << SPACE
          << RRSection::FareCreation::PERCENT << SPACE << RRSection::FareCreation::MINUS_TEXT
          << SPACE << *_currencyCode << *_fareAmt << PERIOD << std::endl;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), _trx->response().str());
  }

  void testDisplayNETFareCreationWhenNoDec()
  {
    *_percent = "5";
    *_currencyCode = "USD";
    *_fareAmt = "10";
    _negRuleData->calcInd() = RuleConst::NF_MINUS;
    _negRuleData->ruleAmt() = Money(9.999999998, *_currencyCode);
    _negRuleData->calculatedNegCurrency() = *_currencyCode;
    _negRuleData->noDecAmt() = 0;
    _negRuleData->percent() = 105.0000001;
    _negRuleData->noDecPercent() = 0;

    _rrSection->displayNETFareCreation(*_negRuleData);

    *_oss << RRSection::INDENTION << RRSection::FareCreation::BASE_FARE << SPACE
          << RRSection::FareCreation::PLUS_TEXT << SPACE << *_percent << SPACE
          << RRSection::FareCreation::PERCENT << SPACE << RRSection::FareCreation::MINUS_TEXT
          << SPACE << *_currencyCode << *_fareAmt << PERIOD << std::endl;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), _trx->response().str());
  }

  void testDisplayAdjustedSellingLvlWhenNoDec()
  {
    *_percent = "4";
    *_currencyCode = "USD";
    *_fareAmt = "10";
    _adjSellCalcData->setCalcInd(RuleConst::NF_ADD);
    _adjSellCalcData->setRuleAmount(10.0000001);
    _adjSellCalcData->setNoDecAmt(2);
    _adjSellCalcData->setPercent(95.99999999);
    _adjSellCalcData->setNoDecPercent(2);
    _adjSellCalcData->setCalculatedASLCurrency(*_currencyCode);

    _rrSection->displayAdjustedSellingLvl(*_adjSellCalcData);

    *_oss << RRSection::INDENTION << RRSection::ASL_ADJUSTED_SELLING_LEVEL_CALCULATION << std::endl
          << RRSection::INDENTION << RRSection::FareCreation::BASE_FARE << SPACE
          << RRSection::FareCreation::MINUS_TEXT << SPACE << *_percent << SPACE
          << RRSection::FareCreation::PERCENT << SPACE << RRSection::FareCreation::PLUS_TEXT
          << SPACE << *_currencyCode << *_fareAmt << PERIOD << std::endl;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), _trx->response().str());
  }

  void testDisplayAdjustedSellingLvl()
  {
    *_percent = "4.80";
    *_currencyCode = "USD";
    *_fareAmt = "10.50";
    _adjSellCalcData->setCalcInd(RuleConst::NF_ADD);
    _adjSellCalcData->setRuleAmount(10.5);
    _adjSellCalcData->setNoDecAmt(2);
    _adjSellCalcData->setPercent(95.2);
    _adjSellCalcData->setNoDecPercent(2);
    _adjSellCalcData->setCalculatedASLCurrency(*_currencyCode);

    _rrSection->displayAdjustedSellingLvl(*_adjSellCalcData);

    *_oss << RRSection::INDENTION << RRSection::ASL_ADJUSTED_SELLING_LEVEL_CALCULATION << std::endl
          << RRSection::INDENTION << RRSection::FareCreation::BASE_FARE << SPACE
          << RRSection::FareCreation::MINUS_TEXT << SPACE << *_percent << SPACE
          << RRSection::FareCreation::PERCENT << SPACE << RRSection::FareCreation::PLUS_TEXT
          << SPACE << *_currencyCode << *_fareAmt << PERIOD << std::endl;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), _trx->response().str());
  }

  void testDisplayNetBussinesID()
  {
    uint64_t fareRetailerRuleId = 135792468;
    PseudoCityCode pcc = "80K2";
    Indicator fdCat35Type = RuleConst::NET_FARE;

    _rrSection->displayNetBussinesID(fareRetailerRuleId, pcc, fdCat35Type);

    *_oss << RRSection::INDENTION << pcc << SPACE << "FARE RETAILER NET LEVEL BUSINESS RULE ID"
        << SPACE << fareRetailerRuleId << std::endl;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), _trx->response().str());
  }

  void testDisplayNetBussinesIDWhenRedistributed()
  {
    uint64_t fareRetailerRuleId = 135792468;
    PseudoCityCode pcc = "80K2";
    Indicator fdCat35Type = RuleConst::REDISTRIBUTED_FARE;

    _rrSection->displayNetBussinesID(fareRetailerRuleId, pcc, fdCat35Type);

    *_oss << RRSection::INDENTION << pcc << SPACE << "FARE RETAILER REDISTRIBUTION BUSINESS RULE ID"
        << SPACE << fareRetailerRuleId << std::endl;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), _trx->response().str());
  }

  void testDisplaySellingFareBussinesID()
  {
    uint64_t fareRetailerRuleId = 135792468;
    PseudoCityCode pcc = "VK42";

    _rrSection->displaySellingFareBussinesID(fareRetailerRuleId, pcc);

    *_oss << RRSection::INDENTION << pcc << SPACE << "FARE RETAILER SELLING LEVEL BUSINESS RULE ID"
          << SPACE << fareRetailerRuleId << std::endl;

    CPPUNIT_ASSERT_EQUAL(_oss->str(), _trx->response().str());
  }

private:
  TestMemHandle _memHandle;
  std::ostringstream* _oss;
  RRSection* _rrSection;
  FareDisplayTrx* _trx;
  FareDisplayRequest* _fareDispReq;
  Agent* _agent;

  std::string* _percent;
  std::string* _fareAmt;
  CurrencyCode* _currencyCode;

  NegPaxTypeFareRuleData* _negRuleData;
  AdjustedSellingCalcData* _adjSellCalcData;
  NegFareSecurityInfo* _secRec;
};
CPPUNIT_TEST_SUITE_REGISTRATION(RRSectionTest);
}
