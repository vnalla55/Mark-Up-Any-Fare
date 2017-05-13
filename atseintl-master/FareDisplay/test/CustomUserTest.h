//----------------------------------------------------------------------------
//	File: CustomUserTest.h
//
//	Author: Gern Blanston
//
//  Copyright Sabre 2007
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
#pragma once

#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/Customer.h"
#include "Common/Global.h"

#include "test/include/TestConfigInitializer.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#define BRANCH_PCC "BRAN"
#define HOME_PCC "HOME"
#define UNKNOWN_PCC "XXXX"
#define MULTI_PCC ""
#define MULTI_APPL "JJ"
#define TARGET_GROUP 7

namespace tse
{
enum UserRecTypes
{
  REC_FOR_BRANCH,
  REC_FOR_HOME,
  REC_FOR_GROUP,
  REC_FOR_MULTI,
  REC_FOR_CRS,
  REC_FOR_ANYBODY,
  REC_NOT_FOUND
};

class CustomUserTest : public CppUnit::TestFixture
{
public:
  CustomUserTest();
  virtual ~CustomUserTest() {};

  void initAgent(Agent& agent,
                 const std::string& branch,
                 const std::string& home,
                 const std::string& crx,
                 Customer& customer,
                 const TJRGroup& group = 0);

  static UserRecTypes getRecType(const Indicator& userApplType,
                                 const UserApplCode& userAppl,
                                 const Indicator& pCCType,
                                 const PseudoCityCode& pCC,
                                 const TJRGroup& tjrGroup);
  void setUp();
  void tearDown();
  void simRollback();
  virtual void retrieveAndAssert(UserRecTypes expected) = 0;

  void testBranchAgent();
  void testHomeAgent();
  void testGroupAgent();
  void testCrsAgent();
  void testUnknownAgent();

  void testCrsMulti();
  void testCrsNoPartition();
  void testCrsUnknown();
  void testCrsNoTjr();

  void testRollbackBranch();
  void testRollbackGroup();
  void testRollbackMulti();

protected:
  FareDisplayTrx* _trx{nullptr};

  Agent _branchAgent;
  Agent _homeAgent;
  Agent _groupAgent;
  Agent _crsAgent;
  Agent _unknownAgent;
  Agent _crsMulti;
  Agent _crsUnknown;
  Agent _crsNoTjr;

  TestConfigInitializer* _testConfigInitializer{nullptr};
  Customer _branchCustomer;
  Customer _homeCustomer;
  Customer _groupCustomer;
  Customer _crsCustomer;
  Customer _unknownCustomer;
  Customer _crsMultiCustomer;
  Customer _crsUnknownCustomer;
  Customer _crsNoTjrCustomer;
};
}
