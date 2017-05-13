//-------------------------------------------------------------------
//
//  File:        CommissionsTest.cpp
//  Created:     Jan 21, 2009
//  Authors:     Mauricio Dantas
//
//  Description:
//
//  Updates:
//
//  Copyright Sabre 2004, 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "Rules/Commissions.h"

#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "DBAccess/Customer.h"
#include "DBAccess/NegFareRest.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/CollectedNegFareData.h"
#include "DBAccess/CommissionCap.h"
#include "DBAccess/NUCInfo.h"

namespace tse
{
FALLBACKVALUE_DECL(fallbackCommissionManagement);
FALLBACKVALUE_DECL(fallbackCommissionCapRemoval);

class CommissionsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CommissionsTest);

  CPPUNIT_TEST(testGetCommissions);
  CPPUNIT_TEST(testGetRegularCommissions);

  CPPUNIT_TEST(testGetRuleCommission_RulePercent10);
  CPPUNIT_TEST(testGetRuleCommission_1Seg_RuleCommAmt100_OW);
  CPPUNIT_TEST(testGetRuleCommission_1Seg_RuleCommAmt100_RT);
  CPPUNIT_TEST(testGetRuleCommission_2Seg_RuleCommAmt200_OW);
  CPPUNIT_TEST(testGetRuleCommission_2Seg_RuleCommAmt200_RT);

  CPPUNIT_TEST(testIsITBTFare_1Seg_Blank);
  CPPUNIT_TEST(testIsITBTFare_1Seg_IT);
  CPPUNIT_TEST(testIsITBTFare_1Seg_BT);
  CPPUNIT_TEST(testIsITBTFare_2Seg_CouponInd1_Blank);
  CPPUNIT_TEST(testIsITBTFare_2Seg_CouponInd1_IT);
  CPPUNIT_TEST(testIsITBTFare_2Seg_CouponInd1_BT);
  CPPUNIT_TEST(testIsITBTFare_2Seg_CouponInd2_Blank);
  CPPUNIT_TEST(testIsITBTFare_2Seg_CouponInd2_IT);
  CPPUNIT_TEST(testIsITBTFare_2Seg_CouponInd2_BT);

  CPPUNIT_TEST(testCalculateNetTicketingCommission_SellingFare_RulePercent10);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_SellingFare_RuleCommAmt0_NoAgentComm);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_SellingFare_RuleCommAmt100);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_SellingFare_InputCommPrc10);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_SellingFare_InputCommAmt100);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_RulePercent10_NonITBTFare);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_RulePercent10_ITBTFare);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_RuleCommAmt0_NonITBTFare_NoAgentComm);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_RuleCommAmt0_ITBTFare_NoAgentComm);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_RuleCommAmt100_NonITBTFare);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_RuleCommAmt100_ITBTFare);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_InputCommAmt100_NonITBTFare);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_InputCommAmt100_ITBTFare);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_InputCommPrc10_NonITBTFare);
  CPPUNIT_TEST(testCalculateNetTicketingCommission_OtherFare_InputCommPrc10_ITBTFare);

  CPPUNIT_TEST(testGetNetRemitCommission_RuleAmtWithConversion);
  CPPUNIT_TEST(testGetNetRemitCommission_RuleAmtWithoutConversion);

  CPPUNIT_TEST(testGetCat35TFSFCommission_RuleAmtWithConversion);
  CPPUNIT_TEST(testGetCat35TFSFCommission_RuleAmtWithoutConversion);
  CPPUNIT_TEST(testIsNetRemitAllowedFail);
  CPPUNIT_TEST(testIsNetRemitAllowedPass);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    fallback::value::fallbackCommissionManagement.set(true);
    fallback::value::fallbackCommissionCapRemoval.set(true);
    _memHandle.create<CommissionsDataHandle>();
    initTrx();
    _commission = _memHandle.insert(new Commissions(*_trx));
  }

  void tearDown() { _memHandle.clear(); }

  void testGetCommissions()
  {
    _trx->getOptions()->cat35Net() = 'F';
    _commission->_totalSellAmt = 100;
    _commission->getCommissions(*_farePath);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, _farePath->commissionAmount(), 0.01);
  }

  void testGetRegularCommissions()
  {
    _commission->getCommissions(*_farePath);
    _commission->_totalSellAmt = 100;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, _farePath->commissionAmount(), 0.01);
  }

  void testGetRuleCommission_RulePercent10()
  {
    PaxTypeFare ptf;
    NegFareRest negFareRest;
    NegPaxTypeFareRuleData negPaxTypeFare;

    negFareRest.commPercent() = 10;

    _commission->getRuleCommission(&ptf, &negFareRest, &negPaxTypeFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, _commission->rulePercent(), 0.01);
  }

  void testGetRuleCommission_1Seg_RuleCommAmt100_OW()
  {
    MockPaxTypeFare ptf(false);
    NegFareRest negFareRest;
    NegPaxTypeFareRuleData negPaxTypeFare;

    negFareRest.commPercent() = RuleConst::PERCENT_NO_APPL;
    negFareRest.commAmt1() = 100;
    negFareRest.cur1() = NUC;
    negPaxTypeFare.calculatedNegCurrency() = NUC;

    _commission->_baseFareCurrency = NUC;
    _commission->_paymentCurrency = NUC;

    _commission->getRuleCommission(&ptf, &negFareRest, &negPaxTypeFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, _commission->ruleCommAmt(), 0.01);
  }

  void testGetRuleCommission_1Seg_RuleCommAmt100_RT()
  {
    MockPaxTypeFare ptf(true);
    NegFareRest negFareRest;
    NegPaxTypeFareRuleData negPaxTypeFare;

    negFareRest.commPercent() = RuleConst::PERCENT_NO_APPL;
    negFareRest.commAmt1() = 100;
    negFareRest.cur1() = NUC;
    negPaxTypeFare.calculatedNegCurrency() = NUC;

    _commission->_baseFareCurrency = NUC;
    _commission->_paymentCurrency = NUC;

    _commission->getRuleCommission(&ptf, &negFareRest, &negPaxTypeFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, _commission->ruleCommAmt(), 0.01);
  }

  void testGetRuleCommission_2Seg_RuleCommAmt200_OW()
  {
    MockPaxTypeFare ptf(false);
    NegFareRest negFareRest;
    NegPaxTypeFareRuleData negPaxTypeFare;

    negFareRest.commPercent() = RuleConst::PERCENT_NO_APPL;
    negFareRest.commAmt2() = 200;
    negFareRest.cur2() = NUC;
    negPaxTypeFare.calculatedNegCurrency() = NUC;

    _commission->_baseFareCurrency = NUC;
    _commission->_paymentCurrency = NUC;

    _commission->getRuleCommission(&ptf, &negFareRest, &negPaxTypeFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, _commission->ruleCommAmt(), 0.01);
  }

  void testGetRuleCommission_2Seg_RuleCommAmt200_RT()
  {
    MockPaxTypeFare ptf(true);
    NegFareRest negFareRest;
    NegPaxTypeFareRuleData negPaxTypeFare;

    negFareRest.commPercent() = RuleConst::PERCENT_NO_APPL;
    negFareRest.commAmt2() = 200;
    negFareRest.cur2() = NUC;
    negPaxTypeFare.calculatedNegCurrency() = NUC;

    _commission->_baseFareCurrency = NUC;
    _commission->_paymentCurrency = NUC;

    _commission->getRuleCommission(&ptf, &negFareRest, &negPaxTypeFare);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, _commission->ruleCommAmt(), 0.01);
  }

  void testIsITBTFare_1Seg_Blank()
  {
    NegFareRest negFareRest;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    negFareRest.fareBoxText1() = "";

    CPPUNIT_ASSERT(!_commission->isITBTFare(&negFareRest));
  }

  void testIsITBTFare_1Seg_IT()
  {
    NegFareRest negFareRest;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::IT_TICKET;

    CPPUNIT_ASSERT(_commission->isITBTFare(&negFareRest));
  }

  void testIsITBTFare_1Seg_BT()
  {
    NegFareRest negFareRest;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::BT_TICKET;

    CPPUNIT_ASSERT(_commission->isITBTFare(&negFareRest));
  }

  void testIsITBTFare_2Seg_CouponInd1_Blank()
  {
    NegFareRest negFareRest;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.couponInd1() = NegotiatedFareRuleUtil::PSG_COUPON;
    negFareRest.fareBoxText1() = "";

    CPPUNIT_ASSERT(!_commission->isITBTFare(&negFareRest));
  }

  void testIsITBTFare_2Seg_CouponInd1_IT()
  {
    NegFareRest negFareRest;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.couponInd1() = NegotiatedFareRuleUtil::PSG_COUPON;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::IT_TICKET;

    CPPUNIT_ASSERT(_commission->isITBTFare(&negFareRest));
  }

  void testIsITBTFare_2Seg_CouponInd1_BT()
  {
    NegFareRest negFareRest;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.couponInd1() = NegotiatedFareRuleUtil::PSG_COUPON;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::BT_TICKET;

    CPPUNIT_ASSERT(_commission->isITBTFare(&negFareRest));
  }

  void testIsITBTFare_2Seg_CouponInd2_Blank()
  {
    NegFareRest negFareRest;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.couponInd2() = NegotiatedFareRuleUtil::PSG_COUPON;
    negFareRest.fareBoxText2() = "";

    CPPUNIT_ASSERT(!_commission->isITBTFare(&negFareRest));
  }

  void testIsITBTFare_2Seg_CouponInd2_IT()
  {
    NegFareRest negFareRest;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.couponInd2() = NegotiatedFareRuleUtil::PSG_COUPON;
    negFareRest.fareBoxText2() = NegotiatedFareRuleUtil::IT_TICKET;

    CPPUNIT_ASSERT(_commission->isITBTFare(&negFareRest));
  }

  void testIsITBTFare_2Seg_CouponInd2_BT()
  {
    NegFareRest negFareRest;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::TWO_SEGMENTS;
    negFareRest.couponInd2() = NegotiatedFareRuleUtil::PSG_COUPON;
    negFareRest.fareBoxText2() = NegotiatedFareRuleUtil::BT_TICKET;

    CPPUNIT_ASSERT(_commission->isITBTFare(&negFareRest));
  }

  void testCalculateNetTicketingCommission_SellingFare_RulePercent10()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::SELLING_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = 10;
    _commission->_totalSellAmt = 500;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        50.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_SellingFare_RuleCommAmt0_NoAgentComm()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::SELLING_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_ruleCommAmt = 0;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_SellingFare_RuleCommAmt100()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::SELLING_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_ruleCommAmt = 100;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_SellingFare_InputCommPrc10()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::SELLING_FARE);
    NegFareRest negFareRest;

    _commission->_totalSellAmt = 500;
    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_commType = Commissions::PERCENT_COMM_TYPE;
    _commission->_inputCommPrc = 10;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        50.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_SellingFare_InputCommAmt100()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::SELLING_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_commType = Commissions::AMOUNT_COMM_TYPE;
    _commission->_inputCommAmt = 100;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_RulePercent10_NonITBTFare()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = 10;
    _commission->_totalSellAmt = 500;
    _commission->_markUpCommission = 100;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        150.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_RulePercent10_ITBTFare()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = 10;
    _commission->_totalNetAmt = 500;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::IT_TICKET;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        50.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_RuleCommAmt0_NonITBTFare_NoAgentComm()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_ruleCommAmt = 0;
    _commission->_markUpCommission = 100;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_RuleCommAmt0_ITBTFare_NoAgentComm()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_ruleCommAmt = 0;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::IT_TICKET;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_RuleCommAmt100_NonITBTFare()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_ruleCommAmt = 100;
    _commission->_markUpCommission = 100;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        200.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_RuleCommAmt100_ITBTFare()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_ruleCommAmt = 100;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::IT_TICKET;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_InputCommAmt100_NonITBTFare()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_commType = Commissions::AMOUNT_COMM_TYPE;
    _commission->_inputCommAmt = 100;
    _commission->_markUpCommission = 100;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_InputCommAmt100_ITBTFare()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_commType = Commissions::AMOUNT_COMM_TYPE;
    _commission->_inputCommAmt = 100;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::IT_TICKET;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_InputCommPrc10_NonITBTFare()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_totalSellAmt = 100;
    _commission->_commType = Commissions::PERCENT_COMM_TYPE;
    _commission->_inputCommPrc = 10;
    _commission->_markUpCommission = 100;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        110.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testCalculateNetTicketingCommission_OtherFare_InputCommPrc10_ITBTFare()
  {
    MockPaxTypeFare paxTypeFare(RuleConst::NET_SUBMIT_FARE);
    NegFareRest negFareRest;

    _commission->_rulePercent = RuleConst::PERCENT_NO_APPL;
    _commission->_totalNetAmt = 100;
    _commission->_commType = Commissions::PERCENT_COMM_TYPE;
    _commission->_inputCommPrc = 10;

    negFareRest.noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    negFareRest.fareBoxText1() = NegotiatedFareRuleUtil::IT_TICKET;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        10.0, _commission->calculateNetTicketingCommission(&paxTypeFare, &negFareRest), 0.01);
  }

  void testGetNetRemitCommission_RuleAmtWithConversion()
  {
    _commission->_paymentCurrency = "NUC";
    _commission->_ruleCommAmt = 7.0;
    _commission->getNetRemitCommission(*_farePath, *_farePath->collectedNegFareData(), false);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, _commission->_ruleCommAmt, 0.01);
  }

  void testGetNetRemitCommission_RuleAmtWithoutConversion()
  {
    _farePath->collectedNegFareData()->currency() = "NUC";

    _commission->_paymentCurrency = "NUC";
    _commission->_ruleCommAmt = 7.0;
    _commission->getNetRemitCommission(*_farePath, *_farePath->collectedNegFareData(), false);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, _commission->_ruleCommAmt, 0.01);
  }

  void testGetCat35TFSFCommission_RuleAmtWithConversion()
  {
    _commission->_paymentCurrency = "NUC";
    _commission->_ruleCommAmt = 7.0;
    _commission->getCat35TFSFCommission(*_farePath, *_farePath->collectedNegFareData());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, _commission->_ruleCommAmt, 0.01);
  }

  void testGetCat35TFSFCommission_RuleAmtWithoutConversion()
  {
    _farePath->collectedNegFareData()->currency() = "NUC";

    _commission->_paymentCurrency = "NUC";
    _commission->_ruleCommAmt = 7.0;
    _commission->getCat35TFSFCommission(*_farePath, *_farePath->collectedNegFareData());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, _commission->_ruleCommAmt, 0.01);
  }

  void testIsNetRemitAllowedFail()
  {
    Agent* agent = _memHandle.create<Agent>();
    Customer * customer = _memHandle.create<Customer>();
    agent->agentTJR() = customer;
    _trx->getRequest()->ticketingAgent() = agent;
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1J";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "AXES";
    _farePath->collectedNegFareData()->bspMethod() = RuleConst::NRR_METHOD_3;
    bool response = _commission->isNetRemitAllowed(*_farePath->collectedNegFareData());
    CPPUNIT_ASSERT_EQUAL(false, response);
  }

  void testIsNetRemitAllowedPass()
  {
    Agent* agent = _memHandle.create<Agent>();
    Customer * customer = _memHandle.create<Customer>();
    agent->agentTJR() = customer;
    _trx->getRequest()->ticketingAgent() = agent;
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";
    _farePath->collectedNegFareData()->bspMethod() = RuleConst::NRR_METHOD_3;
    bool response = _commission->isNetRemitAllowed(*_farePath->collectedNegFareData());
    CPPUNIT_ASSERT_EQUAL(true, response);
  }

  /*
  void testIsAgencyCommissionsApplicable_Pricing_True()
  {
    CPPUNIT_ASSERT(_agencyCommission->isAgencyCommissionsApplicable(*_farePath));
  }
  void testIsAgencyCommissionsApplicable_MIP_True()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    CPPUNIT_ASSERT(_agencyCommission->isAgencyCommissionsApplicable(*_farePath));
  }
  void testIsAgencyCommissionsApplicable_IS_True()
  {
    _trx->setTrxType(PricingTrx::IS_TRX);
    CPPUNIT_ASSERT(_agencyCommission->isAgencyCommissionsApplicable(*_farePath));
  }
  void testIsAgencyCommissionsApplicable_No_PNR_False()
  {
    _trx->noPNRPricing() = true;
    CPPUNIT_ASSERT(!_agencyCommission->isAgencyCommissionsApplicable(*_farePath));
  }
  void testIsAgencyCommissionsApplicable_Exchange_False()
  {
    _trx->setExcTrxType(PricingTrx::AR_EXC_TRX);
    CPPUNIT_ASSERT(!_agencyCommission->isAgencyCommissionsApplicable(*_farePath));
  }
  void testIsAgencyCommissionsApplicable_CollNegFareAmountNonZero_False()
  {
    MoneyAmount sourceFee = 100;
    _farePath->collectedNegFareData()->comAmount() = sourceFee;
    CPPUNIT_ASSERT(!_agencyCommission->isAgencyCommissionsApplicable(*_farePath));
  }
  */

  /*
  void testDoesCat35CommissionExists()
  {
    CPPUNIT_ASSERT(!_commission->doesCat35CommissionExists(*_farePath));

    MoneyAmount sourceFee = 0;
    Percent percent = 10;
    _farePath->collectedNegFareData()->comAmount() = sourceFee;
    _farePath->collectedNegFareData()->comPercent() = percent;
    CPPUNIT_ASSERT(_commission->doesCat35CommissionExists(*_farePath));

    sourceFee = 100;
    percent = 0;
    _farePath->collectedNegFareData()->comAmount() = sourceFee;
    _farePath->collectedNegFareData()->comPercent() = percent;
    CPPUNIT_ASSERT(!_agencyCommission->isAgencyCommissionsApplicable(*_farePath));
  }

  void testIsAgencyCommissionsApplicableWhenAgentCommissionExist()
  {
    CPPUNIT_ASSERT(_commission->isTicketingAgentSpecifiedCommission());

    _trx->getRequest()->ticketingAgent()->agentCommissionType() = "CMP";
    CPPUNIT_ASSERT(!_commission->isTicketingAgentSpecifiedCommission());
  }

  void testIsAgencyCommissionsApplicableForCarrierPartition()
  {
    CPPUNIT_ASSERT(!_commission->isRequestFromTravelAgent());

    _trx.getRequest()->ticketingAgent()->tvlAgencyPCC()="4ABC";
    CPPUNIT_ASSERT(_commission->isRequestFromTravelAgent());
  }
  */

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  Commissions* _commission;

  void initTrx()
  {
    _trx = _memHandle.create<PricingTrx>();
    _farePath = _memHandle.create<FarePathMock>();

    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->cat35Net() = 'T';

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(_memHandle.create<AirSeg>());
    itin->farePath().push_back(_farePath);
    _farePath->itin() = itin;
    _trx->itin().push_back(itin);

    _farePath->collectedNegFareData() = _memHandle.create<CollectedNegFareData>();
  }

  class MockPaxTypeFare : public PaxTypeFare
  {
  public:
    MockPaxTypeFare(bool roundTrip) { init(roundTrip); }

    MockPaxTypeFare(const Indicator& displayCatType) { init(false, displayCatType); }

  protected:
    void init(bool roundTrip, const Indicator& displayCatType = RuleConst::NET_SUBMIT_FARE)
    {
      if (roundTrip)
        fareInfo_.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;

      fare_.setFareInfo(&fareInfo_);
      _fare = &fare_;

      fareClassAppInfo_._displayCatType = displayCatType;
      _fareClassAppInfo = &fareClassAppInfo_;
    }

  protected:
    FareInfo fareInfo_;
    Fare fare_;
    FareClassAppInfo fareClassAppInfo_;
  };

  class FarePathMock : public FarePath
  {
  public:
    FarePathMock() {}
    bool applyNonIATAR(const PricingTrx& trx) const { return false; }
  };

  class CommissionsDataHandle : public DataHandleMock
  {
  public:
    CommissionsDataHandle()
    {
      _commisions.push_back(&_commision);
      _commision.amt() = 100;

      _nucInfo._nucFactor = 0.5;
    }

    const std::vector<CommissionCap*>&
    getCommissionCap(const CarrierCode& carrier, const CurrencyCode& cur, const DateTime& date)
    {
      return _commisions;
    }

    NUCInfo*
    getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
    {
      return &_nucInfo;
    }

  protected:
    std::vector<CommissionCap*> _commisions;
    CommissionCap _commision;
    NUCInfo _nucInfo;
  };
};
CPPUNIT_TEST_SUITE_REGISTRATION(CommissionsTest);
}
