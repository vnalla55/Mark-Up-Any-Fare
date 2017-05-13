//----------------------------------------------------------------------------
//	File: InclCdRetrieverTest.cpp
//
//	Author: Gern Blanston
//	created:      03/01/2007
//	description:  unit test class for methods in FareDisplayUtil
//	              used to select the FD preferences
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

#include "FareDisplay/test/InclCdRetrieverTest.h"

namespace tse
{

CPPUNIT_TEST_SUITE_REGISTRATION(InclCdRetrieverTest);

bool
FakeInclCdRetriever::retrieveData(const Indicator& userApplType,
                                  const UserApplCode& userAppl,
                                  const Indicator& pCCType,
                                  const PseudoCityCode& pCC,
                                  const TJRGroup& tjrGroup)
{
  //  printf("\n:%c:%s::%c:%s::%d\n", userApplType,   userAppl.c_str(),
  //                                  pCCType, pCC.c_str(), tjrGroup);
  _found = CustomUserTest::getRecType(userApplType, userAppl, pCCType, pCC, 0);
  // for now, not processing return list
  // just want to know which kind of recs found
  if (_found != REC_NOT_FOUND)
    _dummyRetVec.push_back(&_dummyRec);

  return !_dummyRetVec.empty();
}

// unit tests in base class
// will call this to exercise desired module
void
InclCdRetrieverTest::retrieveAndAssert(UserRecTypes expected)
{
  // use arbitrary inclusion code when testing which key is used to get rec
  InclusionCode dummyInclCd("ABC");
  _inclCdRetr.reset(new FakeInclCdRetriever(*_trx, dummyInclCd));
  _inclCdRetr->retrieve();
  CPPUNIT_ASSERT_EQUAL(expected, _inclCdRetr->_found);
}

} // tse
