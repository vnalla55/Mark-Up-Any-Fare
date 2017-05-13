//----------------------------------------------------------------------------
//	File: FDFareMarketRuleControllerTest_CWT.cpp
//
//	Author: Svetlana Tsarkova
//  	Created:      11/21/2007
//  	Description:  This is a unit test class for FDFareMarketRuleController:: isValidForCWTuser()
//
//  Copyright Sabre 2005,2009
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

#include <iostream>

#include "test/include/CppUnitHelperMacros.h"

#include "Common/PaxTypeUtil.h"
#include "Common/TSEException.h"
#include "DataModel/Agent.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "Rules/CategoryRuleItemSet.h"
#include "Rules/FDFareMarketRuleController.h"
#include "Rules/RuleConst.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FDFareMarketRuleControllerTest_CWT : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FDFareMarketRuleControllerTest_CWT);
  CPPUNIT_TEST(testIsValidForCWTuserPassCWTuser);
  CPPUNIT_TEST(testIsValidForCWTuserPassWhenNotCWTUser);
  CPPUNIT_TEST(testIsValidForCWTuserFailWhenNoAccountCode);

  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx _trx;
  PaxTypeFare* _paxTypeFare;
  FareClassAppSegInfo* _fareClassAppSegInfo;
  PricingRequest _request;
  Agent* _agent;
  Customer* _customer;
  TariffCrossRefInfo _tarCrosRef;
  Fare _fare;
  PaxType _paxType;
  TestMemHandle _memHandle;

public:
  // ------------------------------------------------------
  // @MethodName  setUp()
  // ------------------------------------------------------

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->paxType() = &_paxType;
    _tarCrosRef._tariffCat = 0;
    _fare.setTariffCrossRefInfo(&_tarCrosRef);
    _paxTypeFare->setFare(&_fare);
    _fareClassAppSegInfo = _memHandle.create<FareClassAppSegInfo>();

    _paxTypeFare->fareClassAppSegInfo() = _fareClassAppSegInfo;

    // Prepare Agent info
    _agent = _memHandle.create<Agent>();
    _customer = _memHandle.create<Customer>();
    _trx.setRequest(&_request);
    _trx.getRequest()->ticketingAgent() = _agent;

    // std::cout << "completed setup" << std::endl;
  }

  // ------------------------------------------------------
  // @MethodName  SurchargesRuleTest_NonSideTrip::tearDown()
  // ------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

  void setCWTuser()
  {
    _customer->ssgGroupNo() = Agent::CWT_GROUP_NUMBER;
    _agent->agentTJR() = _customer;
  }

  void setNotCWTuser()
  {
    _customer->ssgGroupNo() = 0;
    _agent->agentTJR() = _customer;
  }

  void setUsedCorpID() { _paxTypeFare->setMatchedCorpID(true); }

  void setPrivateTariff()
  {
    _tarCrosRef._tariffCat = RuleConst::PRIVATE_TARIFF;
    _fare.setTariffCrossRefInfo(&_tarCrosRef);
    _paxTypeFare->setFare(&_fare);
  }

  void setNationFR() { _paxTypeFare->setNationFRInCat35(true); }

  void setADTpaxType()
  {
    _paxType.paxType() = "ADT";
    _fareClassAppSegInfo->_paxType = "ADT";
  }

  void testIsValidForCWTuserPassCWTuser()
  {
    TestConfigInitializer::setValue("RULECATEGORY", "3|5|6|7|11", "FCO_RULE_VALIDATION");

    FDFareMarketRuleController fdFareMarketRC;

    setCWTuser();
    setUsedCorpID();
    setPrivateTariff();
    setNationFR();
    setADTpaxType();

    CPPUNIT_ASSERT(fdFareMarketRC.isValidForCWTuser(_trx, (*_paxTypeFare)));
  }

  void testIsValidForCWTuserPassWhenNotCWTUser()
  {
    TestConfigInitializer::setValue("RULECATEGORY", "3|5|6|7|11", "FCO_RULE_VALIDATION");

    FDFareMarketRuleController fdFareMarketRC;

    setNotCWTuser();
    CPPUNIT_ASSERT(fdFareMarketRC.isValidForCWTuser(_trx, (*_paxTypeFare)));
  }

  void testIsValidForCWTuserFailWhenNoAccountCode()
  {
    TestConfigInitializer::setValue("RULECATEGORY", "3|5|6|7|11", "FCO_RULE_VALIDATION");

    FDFareMarketRuleController fdFareMarketRC;

    setCWTuser();
    setPrivateTariff();
    setNationFR();
    setADTpaxType();

    CPPUNIT_ASSERT(!fdFareMarketRC.isValidForCWTuser(_trx, (*_paxTypeFare)));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FDFareMarketRuleControllerTest_CWT);

} // tse
