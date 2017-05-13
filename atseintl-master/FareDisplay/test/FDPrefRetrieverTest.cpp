//----------------------------------------------------------------------------
//	File: FDPrefRetrieverTest.cpp
//
//	Author: Gern Blanston
//  	created:      03/01/2007
//  	description:  unit test class for methods in FDPrefRetriever
//  	              used to select the FD preferences
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

#include "FareDisplay/test/FDPrefRetrieverTest.h"

namespace tse
{

bool
FakeFDPrefRetriever::retrieveData(const Indicator& userApplType,
                                  const UserApplCode& userAppl,
                                  const Indicator& pCCType,
                                  const PseudoCityCode& pCC,
                                  const TJRGroup& tjrGroup)
{
  //  printf("\n:%c:%s::%c:%s::%d\n", userApplType,   userAppl.c_str(),
  //                                  pCCType, pCC.c_str(), tjrGroup);
  _found = CustomUserTest::getRecType(userApplType, userAppl, pCCType, pCC, tjrGroup);
  // for now, not processing return list
  // just want to know which kind of recs found
  return (_found != REC_NOT_FOUND);
}

// unit tests in base class
// will call this to exercise desired module
void
FDPrefRetrieverTest::retrieveAndAssert(UserRecTypes expected)
{
  FakeFDPrefRetriever* fdPrefRetriever = new FakeFDPrefRetriever(*_trx);

  fdPrefRetriever->retrieve();
  CPPUNIT_ASSERT_EQUAL(expected, fdPrefRetriever->_found);
  delete fdPrefRetriever;
}

} // tse
