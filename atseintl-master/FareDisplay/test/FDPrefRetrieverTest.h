//----------------------------------------------------------------------------
//	File: FDPrefRetrieverTest.h
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

#ifndef FDPREFRETRIEVER_TEST_H
#define FDPREFRETRIEVER_TEST_H

#include "FareDisplay/test/CustomUserTest.h"
#include "ItinAnalyzer/FDPrefRetriever.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class FakeFDPrefRetriever : public FDPrefRetriever
{
public:
  FakeFDPrefRetriever(FareDisplayTrx& trx) : FDPrefRetriever(trx), _found(REC_NOT_FOUND) {}

  bool getPrefData(FareDisplayTrx& trx,
                   const Indicator& userApplType,
                   const UserApplCode& userAppl,
                   const Indicator& pCCType,
                   const PseudoCityCode& pCC)
  {
    return retrieveData(userApplType, userAppl, pCCType, pCC, 0);
  }

  bool retrieveData(const Indicator& userApplType,
                    const UserApplCode& userAppl,
                    const Indicator& pCCType,
                    const PseudoCityCode& pCC,
                    const TJRGroup& tjrGroup);

  UserRecTypes _found;
};

class FDPrefRetrieverTest : public CustomUserTest
{
  CPPUNIT_TEST_SUITE(FDPrefRetrieverTest);
  // most are defined in base class
  CPPUNIT_TEST(testBranchAgent);
  CPPUNIT_TEST(testHomeAgent);
  CPPUNIT_TEST(testGroupAgent);
  CPPUNIT_TEST(testCrsAgent);
  CPPUNIT_TEST(testUnknownAgent);

  CPPUNIT_TEST(testCrsMulti);
  CPPUNIT_TEST(testCrsNoPartition);
  CPPUNIT_TEST(testCrsUnknown);
  CPPUNIT_TEST(testCrsNoTjr);

  CPPUNIT_TEST(testRollbackBranch);
  CPPUNIT_TEST(testRollbackGroup);
  CPPUNIT_TEST(testRollbackMulti);
  CPPUNIT_TEST_SUITE_END();

public:
  void retrieveAndAssert(UserRecTypes expected);
};

CPPUNIT_TEST_SUITE_REGISTRATION(FDPrefRetrieverTest);
}
#endif
