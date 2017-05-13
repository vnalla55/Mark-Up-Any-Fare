//----------------------------------------------------------------------------
//	File: CustomUserTest.cpp
//
//	Author: Gern Blanston
//  	created:      02/01/2007
//  	description:  this is a base class for other unit tests
//  	              Many FD tables use similar logic for retrieving records
//  	              based on user info for custom features (FDPreferences, Grouping, ...)
//  	              This class allows such logic to tested uniformly
//
//  copyright sabre 2007
//
//          the copyright to the computer program(s) herein
//          is the property of sabre.
//          the program(s) may be used and/or copied only with
//          the written permission of sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/test/CustomUserTest.h"

namespace tse
{
CustomUserTest::CustomUserTest()
{
  initAgent(_branchAgent, BRANCH_PCC, HOME_PCC, SABRE_MULTIHOST_ID, _branchCustomer);
  initAgent(_homeAgent, HOME_PCC, HOME_PCC, SABRE_MULTIHOST_ID, _homeCustomer);
  initAgent(
      _groupAgent, UNKNOWN_PCC, UNKNOWN_PCC, SABRE_MULTIHOST_ID, _groupCustomer, TARGET_GROUP);
  initAgent(_crsAgent, UNKNOWN_PCC, UNKNOWN_PCC, SABRE_MULTIHOST_ID, _crsCustomer);
  initAgent(_unknownAgent, UNKNOWN_PCC, UNKNOWN_PCC, "XXX", _unknownCustomer);
  initAgent(_crsMulti, MULTI_PCC, MULTI_PCC, SABRE_MULTIHOST_ID, _crsMultiCustomer);
  initAgent(_crsUnknown, MULTI_PCC, MULTI_PCC, "XXX", _crsUnknownCustomer);
  initAgent(_crsNoTjr, MULTI_PCC, MULTI_PCC, "XXX", _crsNoTjrCustomer);
  _crsNoTjr.agentTJR() = 0;
}
void
CustomUserTest::initAgent(Agent& agent,
                          const std::string& branch,
                          const std::string& home,
                          const std::string& crx,
                          Customer& customer,
                          const TJRGroup& group)
{
  agent.tvlAgencyPCC() = branch;
  agent.mainTvlAgencyPCC() = home;
  agent.cxrCode() = crx;
  agent.agentTJR() = &customer;
  agent.agentTJR()->ssgGroupNo() = group;
}

// simulate the rec type the db routines will return
// doesn't return real data, just an indicator used solely within unit tests
UserRecTypes
CustomUserTest::getRecType(const Indicator& userApplType,
                           const UserApplCode& userAppl,
                           const Indicator& pCCType,
                           const PseudoCityCode& pCC,
                           const TJRGroup& tjrGroup)
{
  //  printf("\n:%c:%s::%c:%s::%d\n", userApplType,   userAppl.c_str(),
  //                                  pCCType, pCC.c_str(), tjrGroup);
  if (pCC == BRANCH_PCC && pCCType == PCCTYPE_BRANCH)
    return REC_FOR_BRANCH;

  if (pCC == HOME_PCC && pCCType == PCCTYPE_HOME)
    return REC_FOR_HOME;

  if (tjrGroup == TARGET_GROUP)
    return REC_FOR_GROUP;

  if (userAppl == MULTI_APPL && userApplType == MULTIHOST_USER_APPL)
    return REC_FOR_MULTI;

  if (userAppl == SABRE_USER && userApplType == CRS_USER_APPL)
    return REC_FOR_CRS;

  if (pCC == "" && pCCType == ' ' && userAppl == "" && userApplType == ' ')
    return REC_FOR_ANYBODY;

  return REC_NOT_FOUND;
}

void
CustomUserTest::setUp()
{
  _testConfigInitializer = new TestConfigInitializer;
  TestConfigInitializer::setValue("TJR_GROUP", "Y", "FAREDISPLAY_SVC");

  _trx = new FareDisplayTrx;
  _trx->setRequest(new FareDisplayRequest);
  _trx->setOptions(new FareDisplayOptions);
  _trx->billing() = new Billing;
}
void
CustomUserTest::tearDown()
{
  delete _trx->billing();
  delete _trx->getOptions();
  delete _trx->getRequest();
  delete _trx;
  delete _testConfigInitializer;
  _testConfigInitializer = nullptr;
}

void
CustomUserTest::simRollback()
{
  TestConfigInitializer::setValue("TJR_GROUP", "N", "FAREDISPLAY_SVC", true);
}

void
CustomUserTest::testBranchAgent()
{
  _trx->getRequest()->ticketingAgent() = &_branchAgent;
  retrieveAndAssert(REC_FOR_BRANCH);
}

void
CustomUserTest::testHomeAgent()
{
  _trx->getRequest()->ticketingAgent() = &_homeAgent;
  retrieveAndAssert(REC_FOR_HOME);
}
void
CustomUserTest::testGroupAgent()
{
  _trx->getRequest()->ticketingAgent() = &_groupAgent;
  retrieveAndAssert(REC_FOR_GROUP);
}

void
CustomUserTest::testCrsAgent()
{
  _trx->getRequest()->ticketingAgent() = &_crsAgent;
  retrieveAndAssert(REC_FOR_CRS);
}
void
CustomUserTest::testUnknownAgent()
{
  _trx->getRequest()->ticketingAgent() = &_unknownAgent;
  retrieveAndAssert(REC_FOR_ANYBODY);
}

//  MULTI HOST USER
void
CustomUserTest::testCrsMulti()
{
  _trx->getRequest()->ticketingAgent() = &_crsMulti;
  _trx->billing()->partitionID() = MULTI_APPL;
  retrieveAndAssert(REC_FOR_MULTI);
}

void
CustomUserTest::testCrsNoPartition()
{
  _trx->getRequest()->ticketingAgent() = &_crsMulti;
  retrieveAndAssert(REC_FOR_CRS);
}

void
CustomUserTest::testCrsUnknown()
{
  _trx->getRequest()->ticketingAgent() = &_crsUnknown;
  retrieveAndAssert(REC_FOR_ANYBODY);
}
void
CustomUserTest::testCrsNoTjr()
{
  _trx->getRequest()->ticketingAgent() = &_crsNoTjr;
  retrieveAndAssert(REC_FOR_ANYBODY);
}

// ROLLBACK TJR GROUP FEATURE
void
CustomUserTest::testRollbackBranch()
{
  simRollback();
  _trx->getRequest()->ticketingAgent() = &_branchAgent;
  retrieveAndAssert(REC_FOR_BRANCH);
}

void
CustomUserTest::testRollbackGroup()
{
  simRollback();
  _trx->getRequest()->ticketingAgent() = &_groupAgent;
  // with rollback, never expect REC_FOR_GROUP
  // retrieveAndAssert(REC_FOR_CRS);

  // After change TJR group is always used, expect REC_FOR_GROPU
  retrieveAndAssert(REC_FOR_GROUP);
}

void
CustomUserTest::testRollbackMulti()
{
  simRollback();
  _trx->getRequest()->ticketingAgent() = &_crsMulti;
  _trx->billing()->partitionID() = MULTI_APPL;
  retrieveAndAssert(REC_FOR_MULTI);
}

} // tse
