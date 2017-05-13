//-----------------------------------------------------------------------------
//
//  File:     CheckTrxErrorTest.cpp
//
//  Author :  Grzegorz Wanke
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "RexPricing/CheckTrxError.h"

#include "DataModel/PaxTypeFare.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/AirSeg.h"
#include "Rules/RuleConst.h"
#include "Common/ErrorResponseException.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "DataModel/ReissueOptionsMap.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{
class CheckTrxErrorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CheckTrxErrorTest);
  CPPUNIT_TEST(setUpFareCompAsOutboundMatchedFare);
  CPPUNIT_TEST(setUpFareCompAsOutboundMatchedRules);
  CPPUNIT_TEST(testFareCompMatchedReissueRules_Outbound);
  CPPUNIT_TEST(testFareCompMatchedReissueRules_Inbound);
  CPPUNIT_TEST(testFareCompMatchedReissueRules_NoMatch);
  CPPUNIT_TEST(testFareCompMatchedFare_Outbound);
  CPPUNIT_TEST(testFareCompMatchedFare_Inbound);
  CPPUNIT_TEST(testFareCompMatchedFare_NoMatch);
  CPPUNIT_TEST(testCheckPermutation_Throw);
  CPPUNIT_TEST(testCheckPermutation_NoThrow);
  CPPUNIT_TEST(testProcessRepriceExcPhase_NoMatchFare);
  CPPUNIT_TEST(testProcessRepriceExcPhase_NoMatchRules);
  CPPUNIT_TEST(testProcessRepriceExcPhase_Match);
  CPPUNIT_TEST(testProcessRepriceExcPhase_NoMatchFares_TakesPrecedence);
  CPPUNIT_TEST(testProcess_NoExcItin);
  CPPUNIT_TEST(testProcess_NoError);
  CPPUNIT_TEST(testProcessPaxTypeFareForReissue_MatchedR3);
  CPPUNIT_TEST(testProcessPaxTypeFareForReissue_NoR3);
  CPPUNIT_TEST(testProcessPaxTypeFareForRefund_MatchedR3);
  CPPUNIT_TEST(testProcessPaxTypeFareForRefund_NoR3);
  CPPUNIT_TEST(testProcessPaxTypeFareForRefund_qualifyingCatFailed);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.insert(new RefundPricingTrx);
    _checkTrxError = _memHandle.insert(new CheckTrxError(*_trx));
    _fc = _memHandle.insert(new FareCompInfo);
    _ptf = _memHandle.insert(new PaxTypeFare);
    _fm = _memHandle.insert(new FareMarket);
    AirSeg* seg = _memHandle.insert(new AirSeg);
    seg->departureDT() = DateTime(2016, 5, 13, 0, 0, 0);
    _fm->travelSeg().push_back(seg);
    _ptf->fareMarket() = _fm;
  }

  void tearDown() { _memHandle.clear(); }

  void setUpFareCompAsOutboundMatchedFare()
  {
    _ptf->setCategoryValid(RuleConst::VOLUNTARY_REFUNDS_RULE, false);
    _fc->matchedFares().push_back(_ptf);
    _ptf->fareMarket() = _fm;
    _fc->fareMarket() = _fm;
  }

  void setUpFareCompAsOutboundMatchedRules()
  {
    _ptf->setCategoryValid(RuleConst::VOLUNTARY_REFUNDS_RULE);
    _fc->matchedFares().push_back(_ptf);
  }

  void setUpFareCompAsOutboundMatchedAll()
  {
    // call order important!
    setUpFareCompAsOutboundMatchedFare();
    setUpFareCompAsOutboundMatchedRules();
  }

  void testFareCompMatchedReissueRules_Outbound()
  {
    setUpFareCompAsOutboundMatchedRules();

    CPPUNIT_ASSERT_EQUAL(RuleConst::VOLUNTARY_REFUNDS_RULE, _checkTrxError->_subjectCategory);
    CPPUNIT_ASSERT(_checkTrxError->fareCompMatchedReissueRules(*_fc));
  }

  void testFareCompMatchedReissueRules_Inbound()
  {
    _ptf->setCategoryValid(RuleConst::VOLUNTARY_REFUNDS_RULE);
    _fc->matchedFares().push_back(_ptf);

    CPPUNIT_ASSERT_EQUAL(RuleConst::VOLUNTARY_REFUNDS_RULE, _checkTrxError->_subjectCategory);
    CPPUNIT_ASSERT(_checkTrxError->fareCompMatchedReissueRules(*_fc));
  }

  void testFareCompMatchedReissueRules_NoMatch()
  {
    CPPUNIT_ASSERT(!_checkTrxError->fareCompMatchedReissueRules(*_fc));
  }

  void testFareCompMatchedFare_Outbound()
  {
    setUpFareCompAsOutboundMatchedFare();

    CPPUNIT_ASSERT(_checkTrxError->fareCompMatchedFare(*_fc));
  }

  void testFareCompMatchedFare_Inbound()
  {
    _fc->matchedFares().push_back(_ptf);
    _ptf->fareMarket() = _fm;
    _fc->fareMarket() = _fm;

    CPPUNIT_ASSERT(_checkTrxError->fareCompMatchedFare(*_fc));
  }

  void testFareCompMatchedFare_NoMatch()
  {
    CPPUNIT_ASSERT(!_checkTrxError->fareCompMatchedFare(*_fc));
  }

  void setUpRexTrx()
  {
    _trx = _memHandle.insert(new RexPricingTrx);
    _checkTrxError = _memHandle.insert(new CheckTrxError(*_trx));
  }

  ErrorResponseException::ErrorResponseCode
  getExecutedMethodError(void (CheckTrxError::*pt2ConstMethod)(void) const)
  {
    try { (_checkTrxError->*pt2ConstMethod)(); }
    catch (const ErrorResponseException& catched) { return catched.code(); }

    return ErrorResponseException::NO_ERROR;
  }

  void testCheckPermutation_Throw()
  {
    setUpRexTrx();

    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::REISSUE_RULES_FAIL,
                         getExecutedMethodError(&CheckTrxError::checkPermutations));
  }

  void testCheckPermutation_NoThrow()
  {
    setUpRexTrx();
    ProcessTagPermutation permutation;
    static_cast<RexPricingTrx&>(*_trx).processTagPermutations().push_back(&permutation);

    CPPUNIT_ASSERT_NO_THROW(_checkTrxError->checkPermutations());
  }

  void setUpExcItin()
  {
    _excItin = _memHandle.insert(new ExcItin);
    _excItin->fareComponent().push_back(_fc);
    _trx->exchangeItin().push_back(_excItin);
  }

  void testProcessRepriceExcPhase_NoMatchFare()
  {
    setUpExcItin();

    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_FARE,
                         getExecutedMethodError(&CheckTrxError::processRepriceExcPhase));
  }

  void testProcessRepriceExcPhase_NoMatchRules()
  {
    setUpExcItin();
    setUpFareCompAsOutboundMatchedFare();

    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_REFUND_RULES,
                         getExecutedMethodError(&CheckTrxError::processRepriceExcPhase));
  }

  void testProcessRepriceExcPhase_Match()
  {
    setUpExcItin();
    setUpFareCompAsOutboundMatchedAll();

    CPPUNIT_ASSERT_NO_THROW(_checkTrxError->processRepriceExcPhase());
  }

  void testProcessRepriceExcPhase_NoMatchFares_TakesPrecedence()
  {
    // first matched fare, do not matched rules,
    // second not matched fare - UNABLE_TO_MATCH_FARE takes precedence

    setUpExcItin();
    setUpFareCompAsOutboundMatchedFare();

    FareCompInfo secondFC;
    _excItin->fareComponent().push_back(&secondFC);

    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_FARE,
                         getExecutedMethodError(&CheckTrxError::processRepriceExcPhase));
  }

  void testProcess_NoExcItin()
  {
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_FARE,
                         getExecutedMethodError(&CheckTrxError::process));
  }

  void testProcess_NoError()
  {
    setUpExcItin();
    setUpFareCompAsOutboundMatchedAll();
    _trx->trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;

    CPPUNIT_ASSERT_NO_THROW(_checkTrxError->process());
  }

  void setUpRec3_NotMatched()
  {
    _roMap = _memHandle.insert(new ReissueOptions);
    static_cast<RexPricingTrx&>(*_trx).reissueOptions() = *_roMap;
  }

  void setUpRec3_Matched()
  {
    setUpRec3_NotMatched();
    _rec3 = _memHandle.insert(new VoluntaryChangesInfo);
    static_cast<RexPricingTrx&>(*_trx).reissueOptions().insertOption(_ptf, _rec3);
  }

  ErrorResponseException::ErrorResponseCode
  getProcessPaxTypeFareError(void (CheckTrxError::*pt2processPaxTypeFare)(const PaxTypeFare* ptf)
                             const)
  {
    try { (_checkTrxError->*pt2processPaxTypeFare)(_ptf); }
    catch (const ErrorResponseException& catched) { return catched.code(); }

    return ErrorResponseException::NO_ERROR;
  }

  void testProcessPaxTypeFareForReissue_MatchedR3()
  {
    setUpRexTrx();
    setUpRec3_Matched();

    CPPUNIT_ASSERT_NO_THROW(_checkTrxError->processPaxTypeFareForReissue(_ptf));
  }

  void testProcessPaxTypeFareForReissue_NoR3()
  {
    setUpRexTrx();
    setUpExcItin();
    setUpRec3_NotMatched();
    setUpFareCompAsOutboundMatchedFare();

    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::REISSUE_RULES_FAIL,
                         getProcessPaxTypeFareError(&CheckTrxError::processPaxTypeFareForReissue));
  }

  void setUpRefundRec3()
  {
    _refundRec3 = _memHandle.insert(new VoluntaryRefundsInfo);
    static_cast<RefundPricingTrx&>(*_trx).insertOption(_ptf, _refundRec3);
  }

  void testProcessPaxTypeFareForRefund_MatchedR3()
  {
    setUpRefundRec3();
    CPPUNIT_ASSERT_NO_THROW(_checkTrxError->processPaxTypeFareForRefund(_ptf));
  }

  void testProcessPaxTypeFareForRefund_NoR3()
  {
    static_cast<RefundPricingTrx&>(*_trx).reachedR3Validation().insert(_ptf);
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::REFUND_RULES_FAIL,
                         getProcessPaxTypeFareError(&CheckTrxError::processPaxTypeFareForRefund));
  }

  void testProcessPaxTypeFareForRefund_qualifyingCatFailed()
  {
    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::UNABLE_TO_MATCH_REFUND_RULES,
                         getProcessPaxTypeFareError(&CheckTrxError::processPaxTypeFareForRefund));
  }

private:
  TestMemHandle _memHandle;

  CheckTrxError* _checkTrxError;
  RexBaseTrx* _trx;
  FareCompInfo* _fc;
  PaxTypeFare* _ptf;
  FareMarket* _fm;
  ExcItin* _excItin;
  VoluntaryChangesInfo* _rec3;
  VoluntaryRefundsInfo* _refundRec3;
  ReissueOptions* _roMap;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CheckTrxErrorTest);
}
