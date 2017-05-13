//----------------------------------------------------------------------------
//	File: InclCdRetrieverTest.h
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

#include "Common/FareDisplayUtil.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "FareDisplay/test/CustomUserTest.h"
#include "ItinAnalyzer/InclusionCodeRetriever.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>

namespace tse
{

class FakeInclCdRetriever : public InclusionCodeRetriever
{

public:
  virtual ~FakeInclCdRetriever() {}
  FakeInclCdRetriever(FareDisplayTrx& trx, InclusionCode& inclusionCode)
    : InclusionCodeRetriever(trx), _found(REC_NOT_FOUND)
  {
    trx.getRequest()->inclusionCode() = inclusionCode;
  }

  bool retrieveData(const Indicator& userApplType,
                    const UserApplCode& userAppl,
                    const Indicator& pCCType,
                    const PseudoCityCode& pCC,
                    const TJRGroup& tjrGroup);

  UserRecTypes _found;
  std::vector<FareDisplayInclCd*> _dummyRetVec;
  FareDisplayInclCd _dummyRec;
};

class InclCdRetrieverTest : public CustomUserTest
{
  CPPUNIT_TEST_SUITE(InclCdRetrieverTest);
  // most are defined in base class
  CPPUNIT_TEST(testBranchAgent);
  CPPUNIT_TEST(testHomeAgent);
  CPPUNIT_SKIP_TEST(testGroupAgent); // no groups for incl code
  CPPUNIT_TEST(testCrsAgent);
  CPPUNIT_TEST(testUnknownAgent);

  CPPUNIT_TEST(testCrsMulti);
  CPPUNIT_TEST(testCrsNoPartition);
  CPPUNIT_TEST(testCrsUnknown);
  CPPUNIT_TEST(testCrsNoTjr);
  CPPUNIT_TEST_SUITE_END();

public:
  void retrieveAndAssert(UserRecTypes expected);

  std::unique_ptr<FakeInclCdRetriever> _inclCdRetr;
};
}
