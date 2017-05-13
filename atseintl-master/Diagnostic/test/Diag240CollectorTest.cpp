#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag240Collector.h"

#include "DBAccess/FareFocusRuleInfo.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diagnostic.h"
#include "Common/TseEnums.h"

using namespace std;

namespace tse
{

class Diag240CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag240CollectorTest);
  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintDiagSecurityHShakeNotFound);
  CPPUNIT_TEST(testPrintDiagFareFocusRulesNotFound);
  CPPUNIT_TEST(testPrintFareFocusLookup);
  CPPUNIT_TEST(testDisplayStatus);
  CPPUNIT_TEST(testPrintFareFocusLookupRC);
  CPPUNIT_TEST(testPrintFareFocusRuleStatus);
  CPPUNIT_TEST(testPrintFareFocusRuleInfo);

  CPPUNIT_TEST_SUITE_END();

private:
  Diag240Collector* _collector;
  Diagnostic* _diagroot;
  PricingTrx* _trx;
  DataHandle _dataHandle;
  TestMemHandle _memHandle;
  FareFocusRuleInfo* _ffri;


public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _ffri = _memHandle.create<FareFocusRuleInfo>();
    try
    {
      _diagroot = _memHandle(new Diagnostic(Diagnostic240));
      _diagroot->activate();
      _collector = _memHandle(new Diag240Collector(*_diagroot));
      _collector->enable(Diagnostic240);
    }
    catch (...) { CPPUNIT_ASSERT(false); }

    _dataHandle.get(_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void createFFRuleInfo()
  {
    _ffri->fareFocusRuleId() = 77777;
    _ffri->statusCode() = 'A';
    _ffri->sourcePCC() = "A1B1";
    _ffri->securityItemNo() = 123;
    _ffri->vendor() = "ATP";
    _ffri->carrier() = "LH";
    _ffri->ruleCode() = "ABCD";
    _ffri->ruleTariff() = 100;
    _ffri->fareClassItemNo() = 9988;
    _ffri->fareType() = "XPX";
    _ffri->bookingCodeItemNo() = 12;
    _ffri->owrt() = '2';
    _ffri->displayType() = 'T';
    _ffri->loc1().locType() = 'C';
    _ffri->loc1().loc() = "DFW";
    _ffri->loc2().locType() = 'C';
    _ffri->loc2().loc() = "FRA";
    _ffri->directionality() = 'B';
    _ffri->publicPrivateIndicator() = 'Y';
//    _ffri->effDate() 
//    _ffri->discDate()
    _ffri->securityItemNo() = 123;
    _ffri->fareClassItemNo() = 6;
    _ffri->bookingCodeItemNo() =55;
    _ffri->travelDayTimeApplItemNo() =0;

  }

  void testPrintHeader()
  {
    _collector->printHeader();
    string response = "************** FARE FOCUS RULES DIAGNOSTIC ********************\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testPrintDiagSecurityHShakeNotFound()
  {
    Diagnostic diagroot(Diagnostic240);
    diagroot.activate();
    Diag240Collector diag(diagroot);
    diag.enable(Diagnostic240);
    diag.printDiagSecurityHShakeNotFound();
    string actual = diag.str();

    string expected = " \nNO FARE FOCUS PROCESSING - SECURITY HANDSHAKE NOT FOUND\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintDiagFareFocusRulesNotFound()
  {
    Diagnostic diagroot(Diagnostic240);
    diagroot.activate();
    Diag240Collector diag(diagroot);
    diag.enable(Diagnostic240);
    diag.printDiagFareFocusRulesNotFound();
    string actual = diag.str();
    string expected = " \nNO FARE FOCUS PROCESSING - FARE FOCUS RULES ARE NOT FOUND\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintFareFocusLookup()
  {
    Diagnostic diagroot(Diagnostic240);
    diagroot.activate();
    Diag240Collector diag(diagroot);
    diag.enable(Diagnostic240);

    uint64_t fareFocusRuleId = 1234;
    StatusFFRuleValidation rc = FAIL_FF_RULES_SECURITY_HANDSHAKE;

    diag.printFareFocusLookup(fareFocusRuleId, rc);
    string actual = diag.str();
    string expected = "  FARE FOCUS RULE ID: 1234  STATUS- NOT MATCH SECURITY HANDSHAKE\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testDisplayStatus()
  {
    Diagnostic diagroot(Diagnostic240);
    diagroot.activate();
    Diag240Collector diag(diagroot);
    diag.enable(Diagnostic240);

    StatusFFRuleValidation rc = FAIL_FF_VENDOR;

    diag.displayStatus(rc);
    string actual = diag.str();
    string expected = "NOT MATCH VENDOR\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintFareFocusLookupRC()
  {
    Diagnostic diagroot(Diagnostic240);
    diagroot.activate();
    Diag240Collector diag(diagroot);
    diag.enable(Diagnostic240);

    StatusFFRuleValidation rc = FAIL_FF_SECURITY;

    diag.printFareFocusLookup(rc);
    string actual = diag.str();
    string expected = "NOT MATCH FF SECURITY\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);

  }

  void testPrintFareFocusRuleStatus()
  {
    Diagnostic diagroot(Diagnostic240);
    diagroot.activate();
    Diag240Collector diag(diagroot);
    diag.enable(Diagnostic240);

    StatusFFRuleValidation rc = FAIL_FF_FARE_TYPE;
    diag.printFareFocusRuleStatus(rc);
    string actual = diag.str();

    string expected = "***                  RULE STATUS : ";
    expected += "NOT MATCH FARE TYPE\n";
    expected += " \n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintFareFocusRuleInfo()
  {
    Diagnostic diagroot(Diagnostic240);
    diagroot.activate();
    Diag240Collector diag(diagroot);
    diag.enable(Diagnostic240);

    createFFRuleInfo();
    diag.printFareFocusRuleInfo(_ffri);
    string actual = diag.str();

    string expected = "--------------- FARE FOCUS RULE DATA  -------------------------\n";
    expected +="FARE FOCUS RULE ID : 77777";
    expected +="\n";
    expected +="EFF/DISC DATES     : ";
    expected +="N/A  N/A";
    expected +="\n";
    expected +="STATUS             : A";
    expected +="\n";
    expected +="SOURCE PCC         : A1B1";
    expected +="\n";
    expected +="VENDOR CODE        : ATP";
    expected +="\n";
    expected +="CARRIER CODE       : LH";
    expected +="\n";
    expected +="RULE               : ABCD";
    expected +="\n";
    expected +="RULE TARIFF        : 100";
    expected +="\n";
    expected +="FARE TYPE          : XPX";
    expected +="\n";
    expected +="OWRT INDICATOR     : R";
    expected +="\n";
    expected +="CAT35 DISPLAY TYPE : T";
    expected +="\n";
    expected +="PUBLIC INDICATOR   : Y";
    expected +="\n";
    expected +="GEO LOC1/LOC2      : C DFW   C FRA";
    expected +="\n";
    expected +="DIRECTIONALITY     : B";
    expected +="\n";
    expected +="SECURITY TABLE     : 123";
    expected +="\n";
    expected +="FARE CLASS TABLE   : 6";
    expected +="\n";
    expected +="BOOKING CLASS TABLE: 55";
    expected +="\n";
    expected +="TRAVEL RANGE TABLE : 0";
    expected +="\n";


    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag240CollectorTest);
} // tse
