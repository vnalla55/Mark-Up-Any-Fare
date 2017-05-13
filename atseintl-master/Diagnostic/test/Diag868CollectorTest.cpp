#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag868Collector.h"
#include "DBAccess/FareFocusSecurityInfo.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diagnostic.h"
#include "Common/TseEnums.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/AdjustedSellingCalcData.h"

using namespace std;

namespace tse
{
class Diag868CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag868CollectorTest);
  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintDiagSecurityHShakeNotFound);
  CPPUNIT_TEST(testPrintDiagFareRetailerRulesNotFound);
  CPPUNIT_TEST(testDisplayStatus);
  CPPUNIT_TEST(testPrintFareRetailerRuleStatus);
  CPPUNIT_TEST(testPrintFareRetailerRuleLookupInfo);
  CPPUNIT_TEST(testPrintFareRetailerRuleInfo);
  CPPUNIT_TEST(testPrintFareCreation);
  CPPUNIT_TEST(testPrintMinimumAmount);

  CPPUNIT_TEST_SUITE_END();

private:
  Diag868Collector* _collector;
  Diagnostic* _diagroot;
  PricingTrx* _trx;
  ConfigMan* _configMan;
  DataHandle _dataHandle;
  TestMemHandle _memHandle;
  FareRetailerRuleInfo* _frri;
  FareRetailerRuleLookupInfo* _frrl;
  FareFocusSecurityInfo* _ffsi;

  PaxTypeFare _ptf;
  Fare _fare;
  FareInfo _fareInfo;
  AdjustedSellingCalcData _calcData;

private:
  void initPaxTypeFare()
  {
    _fareInfo.carrier() = "AA";
    _fareInfo.originalFareAmount() = 100.00;
    _fareInfo.fareAmount() = 50.00;
    _fareInfo.currency() = "EUR";
    _fare.setFareInfo(&_fareInfo);
    _ptf.setFare(&_fare);
    _ptf.nucOriginalFareAmount() = 150.00;
    _ptf.nucFareAmount() = 300.00;
  }

  void initAdjustedSellingCalcData()
  {
    initPaxTypeFare();
    _calcData.setFareRetailerRuleId(1868);
    _calcData.setSourcePcc("VK40");
    _calcData.setCalculatedAmt(100.00);
    _calcData.setCalculatedNucAmt(200.00);
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _configMan = _memHandle.create<ConfigMan>();
    _frri = _memHandle.create<FareRetailerRuleInfo>();
    _frrl = _memHandle.create<FareRetailerRuleLookupInfo>();
    _ffsi = _memHandle.create<FareFocusSecurityInfo>();
    try
    {
      _diagroot = _memHandle(new Diagnostic(Diagnostic868));
      _diagroot->activate();
      _collector = _memHandle(new Diag868Collector(*_diagroot));
      _collector->enable(Diagnostic868);
    }
    catch (...) { CPPUNIT_ASSERT(false); }

    _dataHandle.get(_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void createFRRuleInfo()
  {
    _frri->fareRetailerRuleId() = 77777;
    _frri->applicationInd() = 'A';
    _frri->sourcePseudoCity() = "15X0";
    _frri->ownerPseudoCity() = "B4T0";
    _frri->securityItemNo() = 123;
    _frri->statusCd() = 'A';
    _frri->vendor() = "ATP";
    _frri->carrierItemNo() = 9008;
    _frri->ruleCdItemNo() = 96007;
    _frri->ruleTariff() = 21;
    _frri->fareClassItemNo() = 9988;
    _frri->fareType() = "XPX";
    _frri->bookingCdItemNo() = 12;
    _frri->owrt() = '2';
    _frri->displayCatType() = 'C';
    _frri->loc1().locType() = 'C';
    _frri->loc1().loc() = "DFW";
    _frri->loc2().locType() = 'C';
    _frri->loc2().loc() = "FRA";
    _frri->directionality() = 'B';
    _frri->publicPrivateInd() = 'Y';
    _frri->securityItemNo() = 123;
    _frri->fareClassItemNo() = 6;
    _frri->bookingCdItemNo() = 0;
    _frri->travelDayTimeApplItemNo() = 0;
  }

  void createFareFocusSecurityInfo()
  {
    _ffsi->securityItemNo() = 0;
  }

  void createFRRuleLookupInfo()
  {
    _frrl->applicationType() = 'N';
    _frrl->sourcePcc() = "15X0";
    _frrl->pcc() = "B4T0";
    FareRetailerRuleLookupId frr1(1102,20000000);
    FareRetailerRuleLookupId frr2(1103,40000000);
    _frrl->fareRetailerRuleLookupIds().push_back(frr1);
    _frrl->fareRetailerRuleLookupIds().push_back(frr2);
  }

  void testPrintHeader()
  {
    _collector->printHeader();
    string response = "************** FARE RETAILER RULE DIAGNOSTIC *******************\n";

    CPPUNIT_ASSERT_EQUAL(response, _collector->str());
  }

  void testPrintDiagSecurityHShakeNotFound()
  {
    Diagnostic diagroot(Diagnostic868);
    diagroot.activate();
    Diag868Collector diag(diagroot);
    diag.enable(Diagnostic868);
    diag.printDiagSecurityHShakeNotFound("ABCD");
    string actual = diag.str();
    string expected = "NO FRR PROCESSING FOR PCC ABCD - SECURITY HANDSHAKE NOT FOUND\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintDiagFareRetailerRulesNotFound()
  {
    Diagnostic diagroot(Diagnostic868);
    diagroot.activate();
    Diag868Collector diag(diagroot);
    diag.enable(Diagnostic868);
    diag.printDiagFareRetailerRulesNotFound();
    string actual = diag.str();
    string expected = " \nNO FARE RETAILER PROCESSING - FARE RETAILER RULES ARE NOT FOUND\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testDisplayStatus()
  {
    Diagnostic diagroot(Diagnostic868);
    diagroot.activate();
    Diag868Collector diag(diagroot);
    diag.enable(Diagnostic868);

    StatusFRRuleValidation rc = FAIL_FR_VENDOR;

    diag.displayStatus(*_trx, rc);
    string actual = diag.str();
    string expected = "NOT MATCH VENDOR\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintFareRetailerRuleStatus()
  {
    Diagnostic diagroot(Diagnostic868);
    diagroot.activate();
    Diag868Collector diag(diagroot);
    diag.enable(Diagnostic868);

    StatusFRRuleValidation rc = FAIL_FR_FARE_TYPE;
    diag.printFareRetailerRuleStatus(*_trx, rc);
    string actual = diag.str();

    string expected = "***                  RULE STATUS : ";
    expected += "NOT MATCH FARE TYPE\n";
    expected += " \n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintFareRetailerRuleInfo()
  {
    Diagnostic diagroot(Diagnostic868);
    diagroot.activate();
    Diag868Collector diag(diagroot);
    diag.enable(Diagnostic868);
    bool ddAll = false;
    createFRRuleInfo();
    diag.printFareRetailerRuleInfo(*_trx, _frri, _ffsi, ddAll);
    string actual = diag.str();

    string expected = "---------------- FARE RETAILER RULE DATA  ----------------------\n";

    expected +="APPLICATION IND      : A\n";
    expected +="EFF DATE             : N/A\n";
    expected +="DISC DATES           : N/A\n";
    expected +="STATUS               : A\n";
    expected +="FARE RET RULE ID     : 77777\n";
    expected +="SEQUENCE NUMBER      : 0\n";
    expected +="SOURCE PCC           : 15X0\n";
    expected +="VENDOR CODE          : ATP\n";
    expected +="RULE TARIFF          : 21\n";
    expected +="FARE TYPE            : XPX\n";
    expected +="OWRT INDICATOR       : R\n";
    expected +="CAT35 DISPLAY TYPE   : C\n";
    expected +="FARE DCT TABLE       : 0\n";
    expected +="PUBLIC INDICATOR     : Y\n";
    expected +="GEO LOC1/LOC2        : C DFW   C FRA\n";
    expected +="DIRECTIONALITY       : B\n";
    expected +="EXCLUDE LOC PAIR     : 0\n";
    expected +="GLOBAL DIRECTION     : \n";
    expected +="CARRIER TABLE        : 9008\n";
    expected +="RULE CODE TABLE      : 96007\n";
    expected +="SECURITY TABLE       : 123\n";
    expected +="FARE CLASS TABLE     : 6\n";
    expected +="EXCLUDE FARE CLASS   : 0\n";
    expected +="POS DAY TIME         : 0\n";
    expected +="ROUTING TABLE        : 0\n";
    expected +="TKT DESIGNATOR TABLE : 0\n";
    expected +="RESULTING TABLE      : 0\n";
    expected +="CALCULATION TABLE    : 0\n";
    expected +="BOOKING CODE TABLE   : 0\n";
    expected +="PAXTYPE CODE TABLE   : 0\n";
    expected +="ACCOUNT CODE TABLE   : 0\n";
    expected +="TRAVEL RANGE TABLE   : 0\n";
    expected +="EXCLUDE LOC PAIR     : 0\n";
    expected +="RETAILER CODE        : \n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }
  void testPrintFareRetailerRuleLookupInfo()
  {
    Diagnostic diagroot(Diagnostic868);
    diagroot.activate();
    Diag868Collector diag(diagroot);
    diag.enable(Diagnostic868);
    createFRRuleLookupInfo();
    diag.printFareRetailerRuleLookupInfo(_frrl);
    string actual = diag.str();

//    string expected = "=============== LOOKUP FARE RETAILER RULE DATA =========================";
    string expected = "FARE RETAILER RULE TYPE : N  SOURCE PCC : 15X0\n";
    expected += "  RULE ID:  1102   SEQNO:  20000000\n";
    expected += "  RULE ID:  1103   SEQNO:  40000000\n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }
  void testPrintFareCreation()
  {
    Diagnostic diagroot(Diagnostic868);
    diagroot.activate();
    Diag868Collector diag(diagroot);
    diag.enable(Diagnostic868);
    initPaxTypeFare();
    diag.printFareCreation(nullptr, &_ptf);
    string actual = diag.str();

    string fareBasis = _ptf.createFareBasis(nullptr, true) + "\n";
    string expected = "CREATED PAX TYPE FARE  : " + fareBasis;
    string fareAmount = "50.00 EUR\n";
    expected += "  BASE AMOUNT          : " + fareAmount;
    string originalFareAmount = "100.00 EUR\n";
    expected += "  ORIGINAL FARE AMOUNT : " + originalFareAmount;
    string nucFareAmount = "300.00 NUC \n";
    expected += "  NUC AMOUNT           : " + nucFareAmount;
    string nucOriginalFareAmount = "150.00 NUC \n";
    expected += "  ORIGINAL NUC AMOUNT  : " + nucOriginalFareAmount;

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }
  void testPrintMinimumAmount()
  {
    Diagnostic diagroot(Diagnostic868);
    diagroot.activate();
    Diag868Collector diag(diagroot);
    diag.enable(Diagnostic868);
    initAdjustedSellingCalcData();
    diag.printMinimumAmount(nullptr, &_ptf, &_calcData);
    string actual = diag.str();

    string expected = "\n--- MINIMUM AMOUNT SELECTED FOR ADJUSTED SELLING FARE ---\n";
    string fareBasis = _ptf.createFareBasis(nullptr, true) + "  ";
    expected += "FARE CLASS: " + fareBasis;
    string fareAsourcePCC = "VK40  ";
    expected += "SOURCE PCC: " + fareAsourcePCC;
    string FRRID = "1868\n";
    expected += "FRR ID: " + FRRID;
    string baseAmount = "100.00 EUR\n";
    expected += "  BASE AMOUNT     : " + baseAmount;
    string nucFareAmount = "200.00 NUC \n";
    expected += "  NUC FARE AMOUNT : " + nucFareAmount;

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag868CollectorTest);

} // tse
