//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TrxUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/test/ProcessTagInfoMock.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NUCInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag689Collector.h"
#include "RexPricing/NewTicketEqualOrHigherValidator.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class NewTicketEqualOrHigherValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NewTicketEqualOrHigherValidatorTest);

  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testConstructor_diagnostic);
  CPPUNIT_TEST(testSetDiagnostic);
  CPPUNIT_TEST(testMatch_diagnostic);

  CPPUNIT_TEST(testIsNewTicketAmountEqualOrHigher_equal);
  CPPUNIT_TEST(testIsNewTicketAmountEqualOrHigher_higher);
  CPPUNIT_TEST(testIsNewTicketAmountEqualOrHigher_less);

  CPPUNIT_TEST(testIsNewNonrefTicketAmountEqualOrHigher_isSubtractForNonRefundable_equal);
  CPPUNIT_TEST(testIsNewNonrefTicketAmountEqualOrHigher_isSubtractForNonRefundable_higher);
  CPPUNIT_TEST(testIsNewNonrefTicketAmountEqualOrHigher_isSubtractForNonRefundable_less);
  CPPUNIT_TEST(testIsNewNonrefTicketAmountEqualOrHigher_noSubtractForNonRefundable_equal);
  CPPUNIT_TEST(testIsNewNonrefTicketAmountEqualOrHigher_noSubtractForNonRefundable_higher);
  CPPUNIT_TEST(testIsNewNonrefTicketAmountEqualOrHigher_noSubtractForNonRefundable_less);

  CPPUNIT_TEST(testGetFarePath_cat35TFSFE_disabled);
  CPPUNIT_TEST(testGetFarePath_noNetSellingIndicator);
  CPPUNIT_TEST(testGetFarePath_noFormOfPaymentCard);
  CPPUNIT_TEST(testGetFarePath_noCat35Sell);
  CPPUNIT_TEST(testGetFarePath_noItBt);
  CPPUNIT_TEST(testGetFarePath_isItBt);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    create<TestConfigInitializer>();
    create<RootLoggerGetOff>();

    RexPricingRequest* request = create<RexPricingRequest>();
    request->ticketingAgent() = create<Agent>();
    _trx = create<MockRexPricingTrx>();

    _trx->setTotalBaseFareAmount(Money(100, "CAD"));
    _trx->setExcNonRefAmount(Money(50, "CAD"));

    _trx->exchangeItin().push_back(create<ExcItin>());
    _trx->exchangeItin().front()->rexTrx() = _trx;
    _trx->exchangeItin().front()->calculationCurrency() = NUC;
    _trx->exchangeItin().front()->farePath().push_back(create<FarePath>());
    //_trx->totalFareCalcAmount() = 100;
    _trx->setRequest(request);

    _farePath = create<FarePath>();
    _farePath->itin() = create<Itin>();
    _farePath->itin()->originationCurrency() = USD;
    _farePath->itin()->calculationCurrency() = NUC;
    _farePath->itin()->useInternationalRounding() = true;
    _farePath->setTotalNUCAmount(200);

    _trx->itin().push_back(_farePath->itin());
    _tagInfo = 0;

    Diag689Collector* dc = 0;
    _validator = _memHandle.create<NewTicketEqualOrHigherValidator>(*_trx, *_farePath, dc);
  }

  void tearDown()
  {
    _memHandle.clear();
    delete[] _tagInfo;
  }

  template <typename T>
  T* create()
  {
    return _memHandle.create<T>();
  }

  std::string getDiagString()
  {
    _validator->_diag->flushMsg();
    return _validator->_diag->str();
  }

  Diag689Collector* createDiagnostic()
  {
    Diag689Collector* diag = create<Diag689Collector>();
    diag->activate();
    diag->trx() = _trx;
    return diag;
  }

  template <int size>
  void setupProcessTags(Indicator (&ind)[size],
                        ProcessTagInfoMock* (ProcessTagInfoMock::*set_method)(Indicator),
                        ProcessTagPermutation& perm)
  {
    _tagInfo = new ProcessTagInfoMock[size];

    std::transform(_tagInfo,
                   _tagInfo + size,
                   ind,
                   std::back_inserter(perm.processTags()),
                   std::mem_fun_ref(set_method));
  }

  template <int size>
  ProcessTagPermutation* createProcessTagPermutation(Indicator (&ind)[size])
  {
    ProcessTagPermutation* perm = create<ProcessTagPermutation>();
    setupProcessTags(ind, &ProcessTagInfoMock::setRS<&ReissueSequence::ticketEqualOrHigher>, *perm);
    return perm;
  }

  void testConstructor()
  {
    CPPUNIT_ASSERT_EQUAL(static_cast<RexBaseTrx*>(_trx), &_validator->_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<const FarePath*>(_farePath), &_validator->_farePath);
    CPPUNIT_ASSERT_EQUAL(static_cast<Diag689Collector*>(0), _validator->_diag);

    CPPUNIT_ASSERT_EQUAL(Money(50, "CAD"), _validator->_excNonrefAmount);
    CPPUNIT_ASSERT_EQUAL(Money(0, "CAD"), _validator->_newNonrefAmount);
    CPPUNIT_ASSERT_EQUAL(Money(400, "CAD"), _validator->_newBaseAmount);

    CPPUNIT_ASSERT(!_validator->_stateCache->baseFareStatus.isDetermined);
    CPPUNIT_ASSERT(!_validator->_stateCache->nonrefFareStatus.isDetermined);
  }

  void testConstructor_diagnostic()
  {
    Diag689Collector* dc = createDiagnostic();
    NewTicketEqualOrHigherValidator val(*_trx, *_farePath, dc);
    CPPUNIT_ASSERT_EQUAL(static_cast<RexBaseTrx*>(_trx), &_validator->_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<const FarePath*>(_farePath), &val._farePath);
    CPPUNIT_ASSERT_EQUAL(dc, val._diag);

    CPPUNIT_ASSERT_EQUAL(Money(50, "CAD"), _validator->_excNonrefAmount);
    CPPUNIT_ASSERT_EQUAL(Money(0, "CAD"), _validator->_newNonrefAmount);
    CPPUNIT_ASSERT_EQUAL(Money(400, "CAD"), _validator->_newBaseAmount);

    CPPUNIT_ASSERT(!_validator->_stateCache->baseFareStatus.isDetermined);
    CPPUNIT_ASSERT(!_validator->_stateCache->nonrefFareStatus.isDetermined);
  }

  void testSetDiagnostic()
  {
    Diag689Collector* dc = createDiagnostic();
    _validator->setDiagnostic(dc);
    CPPUNIT_ASSERT_EQUAL(dc, _validator->_diag);
  }

  void testMatch_diagnostic()
  {
    _validator->setDiagnostic(createDiagnostic());

    Indicator ind[] = { ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_B };

    ProcessTagPermutation* perm = createProcessTagPermutation(ind);

    CPPUNIT_ASSERT(_validator->match(*perm));
    CPPUNIT_ASSERT(!getDiagString().empty());
  }

  void populateTotalBaseFareAmounts(const MoneyAmount& excAmnt, const MoneyAmount& newAmnt)
  {
    _trx->setTotalBaseFareAmount(Money(excAmnt, USD));
    _validator->_newBaseAmount = Money(newAmnt, USD);
  }

  typedef NewTicketEqualOrHigherValidator::ValidationStateCache::Status Status;
  void
  assertValidationCacheState(const Status& determined, bool expect, const Status& notDetermined)
  {
    CPPUNIT_ASSERT(determined.isDetermined);
    CPPUNIT_ASSERT_EQUAL(determined.value, expect);
    CPPUNIT_ASSERT(!notDetermined.isDetermined);
  }

  void testIsNewTicketAmountEqualOrHigher_equal()
  {
    populateTotalBaseFareAmounts(100.0, 100.0);

    CPPUNIT_ASSERT(_validator->isNewTicketAmountEqualOrHigher());
    assertValidationCacheState(
        _validator->_stateCache->baseFareStatus, true, _validator->_stateCache->nonrefFareStatus);
  }

  void testIsNewTicketAmountEqualOrHigher_higher()
  {
    populateTotalBaseFareAmounts(100.0, 100.0 + 2 * EPSILON);

    CPPUNIT_ASSERT(_validator->isNewTicketAmountEqualOrHigher());
    assertValidationCacheState(
        _validator->_stateCache->baseFareStatus, true, _validator->_stateCache->nonrefFareStatus);
  }

  void testIsNewTicketAmountEqualOrHigher_less()
  {
    populateTotalBaseFareAmounts(100.0, 100.0 - EPSILON);

    CPPUNIT_ASSERT(!_validator->isNewTicketAmountEqualOrHigher());
    assertValidationCacheState(
        _validator->_stateCache->baseFareStatus, false, _validator->_stateCache->nonrefFareStatus);
  }

  enum
  {
    SUBTRACT_FOR_NONREFUNDABLE_PASS = 1,
    SUBTRACT_FOR_NONREFUNDABLE_FAIL = 0
  };

  void populateNonrefAmounts(const MoneyAmount& excAmnt, const MoneyAmount& newAmnt)
  {
    _validator->_excNonrefAmount = Money(excAmnt, USD);
    _validator->_newNonrefAmount = Money(newAmnt, USD);
  }

  void testIsNewNonrefTicketAmountEqualOrHigher_isSubtractForNonRefundable_equal()
  {
    populateNonrefAmounts(100.0, 100.0);

    CPPUNIT_ASSERT(
        _validator->isNewNonrefTicketAmountEqualOrHigher(SUBTRACT_FOR_NONREFUNDABLE_PASS));
    assertValidationCacheState(
        _validator->_stateCache->nonrefFareStatus, true, _validator->_stateCache->baseFareStatus);
  }

  void testIsNewNonrefTicketAmountEqualOrHigher_isSubtractForNonRefundable_higher()
  {
    populateNonrefAmounts(100.0, 100.0 + 2 * EPSILON);

    CPPUNIT_ASSERT(
        _validator->isNewNonrefTicketAmountEqualOrHigher(SUBTRACT_FOR_NONREFUNDABLE_PASS));
    assertValidationCacheState(
        _validator->_stateCache->nonrefFareStatus, true, _validator->_stateCache->baseFareStatus);
  }

  void testIsNewNonrefTicketAmountEqualOrHigher_isSubtractForNonRefundable_less()
  {
    populateNonrefAmounts(100.0, 100.0 - EPSILON);

    CPPUNIT_ASSERT(
        _validator->isNewNonrefTicketAmountEqualOrHigher(SUBTRACT_FOR_NONREFUNDABLE_PASS));
    assertValidationCacheState(
        _validator->_stateCache->nonrefFareStatus, true, _validator->_stateCache->baseFareStatus);
  }

  void testIsNewNonrefTicketAmountEqualOrHigher_noSubtractForNonRefundable_equal()
  {
    populateNonrefAmounts(100.0, 100.0);

    CPPUNIT_ASSERT(
        _validator->isNewNonrefTicketAmountEqualOrHigher(SUBTRACT_FOR_NONREFUNDABLE_FAIL));
    assertValidationCacheState(
        _validator->_stateCache->nonrefFareStatus, true, _validator->_stateCache->baseFareStatus);
  }

  void testIsNewNonrefTicketAmountEqualOrHigher_noSubtractForNonRefundable_higher()
  {
    populateNonrefAmounts(100.0, 100.0 + 2 * EPSILON);

    CPPUNIT_ASSERT(
        _validator->isNewNonrefTicketAmountEqualOrHigher(SUBTRACT_FOR_NONREFUNDABLE_FAIL));
    assertValidationCacheState(
        _validator->_stateCache->nonrefFareStatus, true, _validator->_stateCache->baseFareStatus);
  }

  void testIsNewNonrefTicketAmountEqualOrHigher_noSubtractForNonRefundable_less()
  {
    populateNonrefAmounts(100.0, 100.0 - EPSILON);

    CPPUNIT_ASSERT(
        !_validator->isNewNonrefTicketAmountEqualOrHigher(SUBTRACT_FOR_NONREFUNDABLE_FAIL));
    assertValidationCacheState(
        _validator->_stateCache->nonrefFareStatus, false, _validator->_stateCache->baseFareStatus);
  }

  enum
  {
    TICKETENTRY_YES = 1, TICKETENTRY_NO = 0,
    NETSELLINGINDICATOR_YES = 1, NETSELLINGINDICATOR_NO = 0,
    PAYMENTCARD_YES = 1, PAYMENTCARD_NO = 0,
    CAT35SELL_YES = 1, CAT35SELL_NO = 0,
    ITBT_YES = 1, ITBT_NO = 0
  };

  void populateFarePath(bool isItBt)
  {
    NegFareRest* rest = _memHandle.create<NegFareRest>();
    rest->noSegs() = NegotiatedFareRuleUtil::ONE_SEGMENT;
    if (isItBt)
      rest->fareBoxText1() = NegotiatedFareRuleUtil::IT_TICKET;

    NegPaxTypeFareRuleData* data = _memHandle.create<NegPaxTypeFareRuleData>();
    data->ruleItemInfo() = rest;

    PaxTypeFare::PaxTypeFareAllRuleData*
      allData = _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    allData->fareRuleData = data;

    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    (*ptf->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allData;
    ptf->status().set(PaxTypeFare::PTF_Negotiated);

    FareClassAppInfo* info = _memHandle.create<FareClassAppInfo>();
    info->_displayCatType = RuleConst::NET_SUBMIT_FARE;
    ptf->fareClassAppInfo() = info;

     FareUsage* fu = _memHandle.create<FareUsage>();
    fu->paxTypeFare() = ptf;
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->fareUsage().push_back(fu);
    _farePath->pricingUnit().push_back(pu);
  }

  void setupGetFarePath(const std::string& pcc, bool isNetSellingIndicator, bool isTicketEntry,
                        bool isFormOfPaymentCard, bool isCat35Sell, bool isItBt)
  {
    Agent* agent = _trx->getRequest()->ticketingAgent() = create<Agent>();
    agent->tvlAgencyPCC() = pcc;
    agent->agentLocation() = _memHandle.create<Loc>();
    static_cast<RexPricingOptions*>(_trx->getOptions())->setNetSellingIndicator(isNetSellingIndicator);
    _trx->getRequest()->ticketEntry() = isTicketEntry ? 'T' : 'N';
    _trx->getRequest()->formOfPaymentCard() = isFormOfPaymentCard ? 'T' : 'N';
    _trx->getOptions()->cat35Sell() = isCat35Sell ? 'T' : 'N';
    populateFarePath(isItBt);
  }


  void testGetFarePath_cat35TFSFE_disabled()
  {
    setupGetFarePath(AIRLINE_PCC, NETSELLINGINDICATOR_YES,
                     TICKETENTRY_YES, PAYMENTCARD_YES, CAT35SELL_YES, ITBT_YES);

    CPPUNIT_ASSERT(!TrxUtil::isCat35TFSFEnabled(*_trx));

    CPPUNIT_ASSERT_EQUAL(static_cast<const FarePath*>(_farePath),
                         &_validator->getFarePath(*_farePath));
  }

  void testGetFarePath_noNetSellingIndicator()
  {
    setupGetFarePath(AGENT_PCC, NETSELLINGINDICATOR_NO,
                     TICKETENTRY_YES, PAYMENTCARD_YES, CAT35SELL_YES, ITBT_YES);

    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));

    CPPUNIT_ASSERT_EQUAL(static_cast<const FarePath*>(_farePath),
                         &_validator->getFarePath(*_farePath));
  }

  void testGetFarePath_noFormOfPaymentCard()
  {
    setupGetFarePath(AGENT_PCC, NETSELLINGINDICATOR_YES,
                     TICKETENTRY_YES, PAYMENTCARD_NO, CAT35SELL_YES, ITBT_YES);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));

    CPPUNIT_ASSERT_EQUAL(static_cast<const FarePath*>(_farePath),
                         &_validator->getFarePath(*_farePath));
  }

  void testGetFarePath_noCat35Sell()
  {
    setupGetFarePath(AGENT_PCC, NETSELLINGINDICATOR_YES,
                     TICKETENTRY_YES, PAYMENTCARD_YES, CAT35SELL_NO, ITBT_YES);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));

    CPPUNIT_ASSERT_EQUAL(static_cast<const FarePath*>(_farePath),
                         &_validator->getFarePath(*_farePath));
  }

  void testGetFarePath_noItBt()
  {
    setupGetFarePath(AGENT_PCC, NETSELLINGINDICATOR_YES,
                     TICKETENTRY_NO, PAYMENTCARD_YES, CAT35SELL_YES, ITBT_NO);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));

    CPPUNIT_ASSERT_EQUAL(static_cast<const FarePath*>(_farePath),
                         &_validator->getFarePath(*_farePath));
  }

  void testGetFarePath_isItBt()
  {
    setupGetFarePath(AGENT_PCC, NETSELLINGINDICATOR_YES,
                     TICKETENTRY_NO, PAYMENTCARD_YES, CAT35SELL_YES, ITBT_YES);
    CPPUNIT_ASSERT(TrxUtil::isCat35TFSFEnabled(*_trx));

    CPPUNIT_ASSERT(_farePath != &_validator->getFarePath(*_farePath));
  }

private:
  class MockRexPricingTrx : public RexPricingTrx
  {
  public:
    MockRexPricingTrx()
    {
      setOptions(&_op);
      setRequest(&_rq);
    }

    virtual Money
    convertCurrency(const Money& source, const CurrencyCode& targetCurr, bool rounding) const
    {
      if ((source.code() == NUC && targetCurr == USD) ||
          (source.code() == USD && targetCurr == NUC))
        return Money(source.value(), targetCurr);

      return Money(source.value() * 2, targetCurr);
    }
  protected:
    RexPricingRequest _rq;
    RexPricingOptions _op;
  };

  static const char* const AGENT_PCC;
  static const char* const AIRLINE_PCC;

  FarePath* _farePath;
  RexPricingTrx* _trx;
  TestMemHandle _memHandle;
  ProcessTagInfoMock* _tagInfo;
  NewTicketEqualOrHigherValidator* _validator;
};

CPPUNIT_TEST_SUITE_REGISTRATION(NewTicketEqualOrHigherValidatorTest);

const char* const NewTicketEqualOrHigherValidatorTest::AGENT_PCC = "80K2";
const char* const NewTicketEqualOrHigherValidatorTest::AIRLINE_PCC = "";

} // tse
