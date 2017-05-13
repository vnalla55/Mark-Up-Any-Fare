//----------------------------------------------------------------------------
//	File: Grouping_RetrieveSort.cpp
//
//	Author: Gern Blanston
//  	created:      02/01/2007
//  	description:  this is a unit test class for GroupingDataRetriever
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
#include "FareDisplay/GroupingDataRetriever.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace tse;
using namespace std;

namespace utest_GroupingDataRetriever
{

class FakeGroupingDataRetriever : public GroupingDataRetriever
{
  vector<FareDisplaySort*> _retSort;

public:
  FakeGroupingDataRetriever(FareDisplayTrx& trx) : GroupingDataRetriever(trx), _found(REC_NOT_FOUND)
  {
  }

  virtual bool initializeGroups(std::vector<Group*>& groups) { return true; }
  virtual void setHeader(std::vector<Group*>&) {}

  bool retrieveData(const Indicator& userApplType,
                    const UserApplCode& userAppl,
                    const Indicator& pCCType,
                    const PseudoCityCode& pCC,
                    const TJRGroup& tjrGroup)
  {
    _found = CustomUserTest::getRecType(userApplType, userAppl, pCCType, pCC, tjrGroup);
    // for now, not processing return list
    // just want to know which kind of recs found
    return (_found != REC_NOT_FOUND);
  }

  UserRecTypes _found;
};

class Grouping_RetrieveSort : public CustomUserTest
{
  CPPUNIT_TEST_SUITE(Grouping_RetrieveSort);
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
  virtual void retrieveAndAssert(UserRecTypes expected)
  {
    std::vector<Group*> dummy;
    FakeGroupingDataRetriever* grpRetriever = new FakeGroupingDataRetriever(*_trx);

    grpRetriever->getGroupAndSortPref(dummy);
    CPPUNIT_ASSERT_EQUAL(expected, grpRetriever->_found);
    delete grpRetriever;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Grouping_RetrieveSort);

} // end namespace
