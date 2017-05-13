//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/SvcFeesDiagCollector.h"

#include "DBAccess/SvcFeesAccCodeInfo.h"
#include "DBAccess/SvcFeesTktDesignatorInfo.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Rules/RuleConst.h"

using namespace std;

namespace tse
{
class SvcFeesDiagCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SvcFeesDiagCollectorTest);

  CPPUNIT_TEST(testConstructor);

  CPPUNIT_TEST(testOperatorSvcFeesAccCodeInfo);
  CPPUNIT_TEST(testOperatorSvcFeesAccCodeInfo_NotActive);

  CPPUNIT_TEST(testOperatorSvcFeesTktDesignatorInfo);
  CPPUNIT_TEST(testOperatorSvcFeesTktDesignatorInfo_NotActive);

  CPPUNIT_TEST(testPrintValidationStatus_Pass);
  CPPUNIT_TEST(testPrintValidationStatus_Fail);
  CPPUNIT_TEST(testPrintValidationStatus_NotActive);

  CPPUNIT_TEST(testPrintAccountCodeTable172Header);
  CPPUNIT_TEST(testPrintAccountCodeTable172Header_NotActive);

  CPPUNIT_TEST(testPrintTktDesignatorTable173Header);
  CPPUNIT_TEST(testPrintTktDesignatorTable173Header_NotActive);

  CPPUNIT_TEST(testPrintTktDesignatorFailInfo_InputMissing);
  CPPUNIT_TEST(testPrintTktDesignatorFailInfo_DesEmpty);
  CPPUNIT_TEST(testPrintTktDesignatorFailInfo_Unknown);
  CPPUNIT_TEST(testPrintTktDesignatorFailInfo_OutputMissing);
  CPPUNIT_TEST(testPrintTktDesignatorFailInfo_NotActive);

  CPPUNIT_TEST(testPrintSecurityTable183Info_Fail_View_Tkt_Ind);
  CPPUNIT_TEST(testPrintSecurityTable183Info_PASS);

  CPPUNIT_TEST_SUITE_END();

private:
  SvcFeesDiagCollector* _collector;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    try
    {
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic877));
      _diagroot->activate();
      _collector = _memHandle.insert(new SvcFeesDiagCollector(*_diagroot));
      _collector->enable(Diagnostic877);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

  // tests
  void testConstructor()
  {
    try
    {
      SvcFeesDiagCollector diag;
      CPPUNIT_ASSERT(diag.str().empty());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testOperatorSvcFeesAccCodeInfo()
  {
    SvcFeesAccCodeInfo accCode;
    accCode.seqNo() = 5;
    accCode.accountCode() = "ACC01";
    _collector->operator<<(accCode);
    CPPUNIT_ASSERT_EQUAL(string(" 5        ACC01               "), _collector->str());
  }

  void testOperatorSvcFeesAccCodeInfo_NotActive()
  {
    _collector->_active = false;
    _collector->operator<<(SvcFeesAccCodeInfo());
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testOperatorSvcFeesTktDesignatorInfo()
  {
    SvcFeesTktDesignatorInfo tktDes;
    tktDes.seqNo() = 5;
    tktDes.tktDesignator() = "TKTDES";
    _collector->operator<<(tktDes);
    CPPUNIT_ASSERT_EQUAL(string("   5                 TKTDES     "), _collector->str());
  }

  void testOperatorSvcFeesTktDesignatorInfo_NotActive()
  {
    _collector->_active = false;
    _collector->operator<<(SvcFeesTktDesignatorInfo());
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testPrintValidationStatus_Pass()
  {
    _collector->printValidationStatus(true);
    CPPUNIT_ASSERT_EQUAL(string("   PASS\n"), _collector->str());
  }

  void testPrintValidationStatus_Fail()
  {
    _collector->printValidationStatus(false);
    CPPUNIT_ASSERT_EQUAL(string("   FAIL\n"), _collector->str());
  }

  void testPrintValidationStatus_NotActive()
  {
    _collector->_active = false;
    _collector->printValidationStatus(false);
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testPrintAccountCodeTable172Header()
  {
    string result = " *....................................................*\n"
                    " *  ACC CODE T172 ITEM NO : 7 DETAIL INFO\n"
                    " *....................................................*\n"
                    " SEQ NO   ACCOUNT CODE           STATUS\n";
    _collector->printAccountCodeTable172Header(7);
    CPPUNIT_ASSERT_EQUAL(result, _collector->str());
  }

  void testPrintAccountCodeTable172Header_NotActive()
  {
    _collector->_active = false;
    _collector->printAccountCodeTable172Header(7);
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testPrintTktDesignatorTable173Header()
  {
    string result = " .....................................................\n"
                    " * TKT DESIGNATOR T173 ITEM NO: 7        DETAIL INFO *\n"
                    " .....................................................\n"
                    "  SEQ NO           TKT DSGN CODE   STATUS\n";
    _collector->printTktDesignatorTable173Header(7);
    CPPUNIT_ASSERT_EQUAL(result, _collector->str());
  }

  void testPrintTktDesignatorTable173Header_NotActive()
  {
    _collector->_active = false;
    _collector->printTktDesignatorTable173Header(7);
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testPrintTktDesignatorFailInfo_InputMissing()
  {
    _collector->printTktDesignatorFailInfo(FAIL_NO_INPUT);
    CPPUNIT_ASSERT_EQUAL(string("                  INPUT DESIGNATOR MISSING\n"), _collector->str());
  }

  void testPrintTktDesignatorFailInfo_DesEmpty()
  {
    _collector->printTktDesignatorFailInfo(FAIL_ON_BLANK);
    CPPUNIT_ASSERT_EQUAL(string("                       DESIGNATOR IS EMPTY\n"), _collector->str());
  }

  void testPrintTktDesignatorFailInfo_OutputMissing()
  {
    _collector->printTktDesignatorFailInfo(FAIL_NO_OUTPUT);
    CPPUNIT_ASSERT_EQUAL(string("                  OUTPUT DESIGNATOR MISSING\n"),
                         _collector->str());
  }

  void testPrintTktDesignatorFailInfo_Unknown()
  {
    _collector->printTktDesignatorFailInfo((StatusT173) - 1);
    CPPUNIT_ASSERT_EQUAL(string("                  UNKNOWN STATUS\n"), _collector->str());
  }

  void testPrintTktDesignatorFailInfo_NotActive()
  {
    _collector->_active = false;
    _collector->printTktDesignatorFailInfo(StatusT173());
    CPPUNIT_ASSERT_EQUAL(string(""), _collector->str());
  }

  void testPrintSecurityTable183Info_Fail_View_Tkt_Ind()
  {
    SvcFeesSecurityInfo feeSec;
    buildSecurInfo(feeSec);
    feeSec.viewBookTktInd() = '2';
    _collector->printSecurityTable183Header(12345);
    _collector->printSecurityTable183Info(&feeSec, FAIL_VIEW_IND);
    CPPUNIT_ASSERT_EQUAL(string(" *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                " * SECURITY T183 ITEM NO : 12345   DETAIL INFO       * \n"
                                " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                " SEQ NO TVL GDS DUTY GEO LOC TYPE CODE VIEW STATUS \n"
                                " 1       X  1S  *    N US    T W0H3     2   FAIL VIEW BOOK \n"),
                         _collector->str());
  }

  void testPrintSecurityTable183Info_PASS()
  {
    SvcFeesSecurityInfo feeSec;
    buildSecurInfo(feeSec);
    feeSec.viewBookTktInd() = '1';
    _collector->printSecurityTable183Header(12345);
    _collector->printSecurityTable183Info(&feeSec, PASS_T183);
    CPPUNIT_ASSERT_EQUAL(string(" *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                " * SECURITY T183 ITEM NO : 12345   DETAIL INFO       * \n"
                                " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                " SEQ NO TVL GDS DUTY GEO LOC TYPE CODE VIEW STATUS \n"
                                " 1       X  1S  *    N US    T W0H3     1   PASS \n"),
                         _collector->str());
  }

  void buildSecurInfo(SvcFeesSecurityInfo& feeSec)
  {
    feeSec.seqNo() = 1;
    feeSec.travelAgencyInd() = 'X';
    feeSec.carrierGdsCode() = "1S";
    feeSec.dutyFunctionCode() = "*";
    feeSec.loc().locType() = LOCTYPE_NATION;
    feeSec.loc().loc() = "US";
    feeSec.code().locType() = RuleConst::TRAVEL_AGENCY;
    feeSec.code().loc() = "W0H3";
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SvcFeesDiagCollectorTest);
} // tse
