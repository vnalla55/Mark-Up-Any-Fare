//----------------------------------------------------------------------------
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/TrxUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Customer.h"
#include "Rules/PricingUnitRuleController.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;

class PricingUnitRuleControllerStub : public PricingUnitRuleController
{
public:
  PricingUnitRuleControllerStub(CategoryPhase phase)
    : PricingUnitRuleController(phase), _failFare(false), _failFareInCat5BecauseOfAnotherFu(false)
  {
  }

  bool validate(PricingTrx& trx, PricingUnit& pricingUnit, FareUsage& fareUsage, Itin& itin)
  {
    if (_failFare)
    {
      if (_failFareInCat5BecauseOfAnotherFu)
        fareUsage.failedCat5InAnotherFu() = true;
      return false;
    }
    return true;
  }

  bool _failFare;
  bool _failFareInCat5BecauseOfAnotherFu;
};

//----------------------------------------------------------------------------
class PricingUnitRuleControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingUnitRuleControllerTest);
  CPPUNIT_TEST(testValidateReturnTrueWhenAllFareUsagePass);
  CPPUNIT_TEST(testValidateReturnFailWhenFareUsageFails);
  CPPUNIT_TEST(testValidateReturnFailAndFailedFareUsageNotSetWhenFailedByAnotherFuSet);
  CPPUNIT_TEST(testGetPTFsForFMS);
  CPPUNIT_TEST_SUITE_END();

protected:
  PricingUnitRuleControllerStub* _purc;
  PricingTrx _trx;
  PricingUnit _pu;
  FareUsage _fu;
  Itin _itin;
  FareUsage* _failedFareUsage;
  TestMemHandle _memHandle;

public:
  //----------------------------------------------------------------------------
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _pu.fareUsage() += &_fu;
    CategoryPhase phase = FCORuleValidation;
    _purc = _memHandle.create<PricingUnitRuleControllerStub>(phase);
    _failedFareUsage = 0;
  }

  //----------------------------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

  //----------------------------------------------------------------------------
  void testValidateReturnTrueWhenAllFareUsagePass()
  {
    FareUsage* expected = 0;
    CPPUNIT_ASSERT(validate());
    CPPUNIT_ASSERT_EQUAL(expected, _failedFareUsage);
  }

  //----------------------------------------------------------------------------
  void testValidateReturnFailWhenFareUsageFails()
  {
    _purc->_failFare = true;
    CPPUNIT_ASSERT(!validate());
    CPPUNIT_ASSERT_EQUAL(&_fu, _failedFareUsage);
  }

  //----------------------------------------------------------------------------
  void testValidateReturnFailAndFailedFareUsageNotSetWhenFailedByAnotherFuSet()
  {
    _purc->_failFare = true;
    _purc->_failFareInCat5BecauseOfAnotherFu = true;
    FareUsage* expected = 0;
    CPPUNIT_ASSERT(!validate());
    CPPUNIT_ASSERT_EQUAL(expected, _failedFareUsage);
  }

  //----------------------------------------------------------------------------
  void testGetPTFsForFMS()
  {
    RexPricingTrx rexTrx;
    FareMarket fm1, fm2;
    PaxTypeFare ptf1, ptf2;
    ExcItin excItin;
    RexBaseRequest rexBaseRequest;
    Agent agent;
    Customer cus;
    cus.crsCarrier() = "1B";
    cus.hostName() = "ABAC";
    agent.agentTJR() = &cus;

    rexBaseRequest.ticketingAgent() = &agent;

    TrxUtil::enableAbacus();

    rexTrx.setRequest(&rexBaseRequest);
    rexTrx.setExcTrxType(PricingTrx::AR_EXC_TRX);
    rexTrx.trxPhase() = RexPricingTrx::REPRICE_EXCITIN_PHASE;
    rexTrx.exchangeItin().push_back(&excItin);

    excItin.addSiblingMarket(&fm1, &fm2);

    fm1.allPaxTypeFare().push_back(&ptf1);
    fm2.allPaxTypeFare().push_back(&ptf2);

    std::vector<PaxTypeFare*> ptfVec = fm1.allPaxTypeFare();
    _purc->preparePtfVector(ptfVec, rexTrx, fm1);

    CPPUNIT_ASSERT_EQUAL(&ptf1, ptfVec[0]);
    CPPUNIT_ASSERT_EQUAL(&ptf2, ptfVec[1]);
  }

  bool validate()
  {
    // unable to figure out how to resolve validate(..., FareUsage*&, ...)  without base class scope
    return _purc->PricingUnitRuleController::validate(_trx, _pu, _failedFareUsage, _itin);
    //  return _purc->validate(_trx, _pu, _failedFareUsage, _itin);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PricingUnitRuleControllerTest);
}
