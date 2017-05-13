//----------------------------------------------------------------------------
//
// Copyright Sabre 2013
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/DateTime.h"
#include "Common/TrxUtil.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/FarePath.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "FreeBagService/BaggageTicketingDateScope.h"
#include "Rules/RuleConst.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{

using boost::assign::operator +=;

class BaggageTicketingDateScopeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageTicketingDateScopeTest);

  CPPUNIT_TEST(test_isDateChangeRequired_changeRequired_1);
  CPPUNIT_TEST(test_isDateChangeRequired_changeRequired_2);
  CPPUNIT_TEST(test_recognize_reissue_for_PE_and_no_N25);
  CPPUNIT_TEST(test_recognize_reissue_for_PE_and_no_N25_checkD92);
  CPPUNIT_TEST(test_itinWithAllExchanges);
  CPPUNIT_TEST(test_itinWithOneReissue);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _farePath = _memHandle.create<FarePath>();
    _rexTrx = _memHandle.create<RexPricingTrx>();

    _d07Date = _memHandle.insert(new DateTime(2013, 10, 12));
    _d92Date = _memHandle.insert(new DateTime(2013, 10, 01));
    _d95Date = _memHandle.insert(new DateTime(2013, 9, 11));
    _dummyDate = _memHandle.insert(new DateTime(1999, 1, 1));

    _rexTrx->setRexPrimaryProcessType('A');
    _rexTrx->trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;

    _rexTrx->prepareRequest();
    _rexTrx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    static_cast<RexPricingRequest*>(_rexTrx->getRequest())->setTicketingDT(*_dummyDate);

    _rexTrx->dataHandle().setTicketDate(*_dummyDate);
    _rexTrx->ticketingDate() = *_dummyDate;

    _rexTrx->currentTicketingDT() = *_d07Date;
    _rexTrx->previousExchangeDT() = *_d95Date;
    _rexTrx->setOriginalTktIssueDT() = *_d92Date;
  }

  void tearDown() { _memHandle.clear(); }

  void test_isDateChangeRequired_changeRequired_1()
  {
    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->dataHandle().ticketDate());

    BaggageTicketingDateScope dateSetter(*_rexTrx, _farePath);

    CPPUNIT_ASSERT_EQUAL(*_d07Date, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_d07Date, _rexTrx->dataHandle().ticketDate());
  }

  void test_isDateChangeRequired_changeRequired_2()
  {
    _farePath->exchangeReissue() = REISSUE;

    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->dataHandle().ticketDate());

    BaggageTicketingDateScope dateSetter(*_rexTrx, _farePath);

    CPPUNIT_ASSERT_EQUAL(*_d95Date, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_d95Date, _rexTrx->dataHandle().ticketDate());
  }

  void test_recognize_reissue_for_PE_and_no_N25()
  {
    _farePath->exchangeReissue() = EXCHANGE;
    _rexTrx->reqType() = PARTIAL_EXCHANGE;
    Indicator blankN25 = RuleConst::BLANK;
    _rexTrx->setRexPrimaryProcessType(blankN25);

    BaggageTicketingDateScope dateSetter(*_rexTrx, _farePath);
    CPPUNIT_ASSERT(dateSetter.isItReissue(*_farePath));

    Indicator validN25 = 'A';
    _rexTrx->setRexPrimaryProcessType(validN25);
    BaggageTicketingDateScope dateSetter2(*_rexTrx, _farePath);
    CPPUNIT_ASSERT(!dateSetter2.isItReissue(*_farePath));
  }

  void test_recognize_reissue_for_PE_and_no_N25_checkD92()
  {
    _farePath->exchangeReissue() = EXCHANGE;
    _rexTrx->reqType() = PARTIAL_EXCHANGE;
    Indicator blankN25 = RuleConst::BLANK;
    _rexTrx->setRexPrimaryProcessType(blankN25);

    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->dataHandle().ticketDate());

    BaggageTicketingDateScope dateSetter(*_rexTrx, _farePath);

    CPPUNIT_ASSERT_EQUAL(*_d92Date, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_d92Date, _rexTrx->dataHandle().ticketDate());
  }

  void test_itinWithAllExchanges()
  {
    FarePath fp1, fp2;
    fp1.exchangeReissue() = EXCHANGE;
    fp2.exchangeReissue() = EXCHANGE;
    Itin itin;
    itin.farePath() += &fp1, &fp2;

    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->dataHandle().ticketDate());

    {
      BaggageTicketingDateScope dateSetter(*_rexTrx, &itin);
      CPPUNIT_ASSERT_EQUAL(*_d07Date, _rexTrx->ticketingDate());
      CPPUNIT_ASSERT_EQUAL(*_d07Date, _rexTrx->dataHandle().ticketDate());
    }

    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->dataHandle().ticketDate());
  }

  void test_itinWithOneReissue()
  {
    FarePath fp1, fp2;
    fp1.exchangeReissue() = EXCHANGE;
    fp2.exchangeReissue() = REISSUE;
    Itin itin;
    itin.farePath() += &fp1, &fp2;

    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->dataHandle().ticketDate());

    {
      BaggageTicketingDateScope dateSetter(*_rexTrx, &itin);
      CPPUNIT_ASSERT_EQUAL(*_d95Date, _rexTrx->ticketingDate());
      CPPUNIT_ASSERT_EQUAL(*_d95Date, _rexTrx->dataHandle().ticketDate());
    }

    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->ticketingDate());
    CPPUNIT_ASSERT_EQUAL(*_dummyDate, _rexTrx->dataHandle().ticketDate());
  }

  TestMemHandle _memHandle;
  FarePath* _farePath;
  RexPricingTrx* _rexTrx;
  DateTime* _d92Date;
  DateTime* _d95Date;
  DateTime* _d07Date;
  DateTime* _dummyDate;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaggageTicketingDateScopeTest);

}
