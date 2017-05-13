//----------------------------------------------------------------------------
//	File: FDHeaderMsgCtrl_retrieve.cpp
//
//	Author: Gern Blanston
//  	created:      03/01/2007
//  	description:  unit test class for methods in FDHeaderMsgController
//  	              used to select the header messages.  Does NOT test
//  	              how msgs are processed, ony which ones are retrieved.
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
#include "FareDisplay/FDHeaderMsgController.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace tse;
using namespace std;

namespace utest_FDHeaderMsgController
{

class FakeFDHeaderMsgController : public FDHeaderMsgController
{
public:
  FakeFDHeaderMsgController(FareDisplayTrx& trx, const std::set<CarrierCode>& prefCxr)
    : FDHeaderMsgController(trx, prefCxr)
  {
  }

  virtual bool retrieveData( // FareDisplayTrx&       trx,
      const Indicator& userApplType,
      const UserApplCode& userAppl,
      const Indicator& pseudoCityType,
      const PseudoCityCode& pseudoCity,
      const TJRGroup& tjrGroup)
  {
    /*
      printf("\n:%c:%s:     :%c:%s:%d:\n",
                userApplType,   userAppl.c_str(),
                pseudoCityType, pseudoCity.c_str(),     tjrGroup);
    */
    UserRecTypes temp =
        CustomUserTest::getRecType(userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
    if (temp != REC_NOT_FOUND)
      _found.push_back(temp);

    // not processing list, just checking which kind of recs found
    return true;
  }

  // stubs for what's not tested
  bool eliminateRows(const std::vector<const tse::FDHeaderMsg*>& fdHeaderMsgDataList,
                     std::vector<const tse::FDHeaderMsg*>& fdFilteredHdrMsg,
                     FareDisplayTrx& trx)
  {
    return true;
  }

  std::vector<UserRecTypes> _found;
};

} // end namespace

using namespace utest_FDHeaderMsgController;

class FDHeaderMsgCtrl_retrieve : public CustomUserTest
{
  CPPUNIT_TEST_SUITE(FDHeaderMsgCtrl_retrieve);
  // most all in base class
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
  void retrieveAndAssert(UserRecTypes expected)
  {
    std::set<CarrierCode> cxrList;
    _fdHeaderMsgCtrl = new FakeFDHeaderMsgController(*_trx, cxrList);
    _fdHeaderMsgCtrl->getHeaderMsgs(_dummyResults);
    setExpected(expected);

    bool isSame = (_expected == _fdHeaderMsgCtrl->_found);
    if (!isSame)
    {
      cout << "\nExpected:\t";
      copy(_expected.begin(), _expected.end(), ostream_iterator<UserRecTypes>(cout, " "));
      cout << "\nFound:\t";
      copy(_fdHeaderMsgCtrl->_found.begin(),
           _fdHeaderMsgCtrl->_found.end(),
           ostream_iterator<UserRecTypes>(cout, " "));
      cout << endl;
      cout.flush();
    }
    CPPUNIT_ASSERT(isSame);
    delete _fdHeaderMsgCtrl;
  }
  void setExpected(UserRecTypes user)
  {
    switch (user)
    {
    case REC_FOR_BRANCH:
      _expected.push_back(REC_FOR_BRANCH);
      _expected.push_back(REC_FOR_HOME);
      _expected.push_back(REC_FOR_CRS);
      _expected.push_back(REC_FOR_ANYBODY);
      break;
    case REC_FOR_HOME:
      _expected.push_back(REC_FOR_HOME);
      _expected.push_back(REC_FOR_CRS);
      _expected.push_back(REC_FOR_ANYBODY);
      break;
    case REC_FOR_GROUP:
      _expected.push_back(REC_FOR_GROUP);
      _expected.push_back(REC_FOR_CRS);
      _expected.push_back(REC_FOR_ANYBODY);
      break;
    case REC_FOR_MULTI:
      _expected.push_back(REC_FOR_MULTI);
      _expected.push_back(REC_FOR_CRS);
      _expected.push_back(REC_FOR_ANYBODY);
      break;
    case REC_FOR_CRS:
      _expected.push_back(REC_FOR_CRS);
      _expected.push_back(REC_FOR_ANYBODY);
      break;

    default:
      _expected.push_back(REC_FOR_ANYBODY);
    }
  }

  FakeFDHeaderMsgController* _fdHeaderMsgCtrl;
  std::vector<tse::FDHeaderMsgText*> _dummyResults;
  std::vector<UserRecTypes> _expected;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FDHeaderMsgCtrl_retrieve);
