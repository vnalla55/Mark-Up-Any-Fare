#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag870Collector.h"

#include "DBAccess/TicketingFeesInfo.h"
#include "DBAccess/SvcFeesAccCodeInfo.h"
#include "DBAccess/SvcFeesTktDesignatorInfo.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diagnostic.h"
#include "Common/TseEnums.h"

#include "test/testdata/TestTicketingFeesInfoFactory.h"
#include "test/testdata/TestSvcFeesAccCodeInfoFactory.h"
#include "test/testdata/TestSvcFeesTktDesignatorInfoFactory.h"
#include "test/testdata/TestSvcFeesSecurityInfoFactory.h"

using namespace std;

namespace tse
{

class Diag870CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag870CollectorTest);
  CPPUNIT_TEST(testPrintS4TicketingInfoGeneralData);
  CPPUNIT_TEST(testPrintFistPartDetailedRequest);
  CPPUNIT_TEST(testPrintTktDesignPartDetailedRequest);
  CPPUNIT_TEST(testPrintFinalPartDetailedRequest);
  CPPUNIT_TEST(testPrintAccountCodeTable172Header);
  CPPUNIT_TEST(testPrintAccountCodeTable172Info);
  CPPUNIT_TEST(testPrintTktDesignatorTable173Info);
  CPPUNIT_TEST(testPrintSecurityTable183Header);
  CPPUNIT_TEST(testPrintSecurityTable183Info);
  CPPUNIT_TEST(testPrintCanNotCollectPrintINTERNAL_ERROR);
  CPPUNIT_TEST(testPrintCanNotCollectPrintTJR_NOT_APPLY_TKTFEE);
  CPPUNIT_TEST(testPrintCanNotCollectPrintOB_NOT_ACTIVE_YET);
  CPPUNIT_TEST(testPrintCanNotCollectPrintVALIDATING_CXR_EMPTY);
  CPPUNIT_TEST(testPrintCanNotCollectPrintALL_SEGS_OPEN);
  CPPUNIT_TEST(testPrintCanNotCollectPrintNOT_ALL_SEGS_CONFIRM);
  CPPUNIT_TEST(testPrintCanNotCollectPrintNON_CREDIT_CARD_FOP);
  CPPUNIT_TEST(testPrintCxrNotActive);
  CPPUNIT_TEST(testPrintFopBinNotFound);
  CPPUNIT_TEST(testPrintFopBinNumber);
  CPPUNIT_TEST(testPrintDiagHeaderAmountFopResidualInd);

  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag870Collector diag(diagroot);

      std::string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void setUp() {}
  void tearDown() {}

  void testPrintS4TicketingInfoGeneralData()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);

    TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoOne.xml");
    StatusS4Validation status = PASS_S4;
    diag.printS4GeneralInfo(tInfo, status);

    string expected;
    string actual = diag.str();

    expected += " OBFCA      1002   ADT   800.00EUR    123***     PASS\n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintFistPartDetailedRequest()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);

    // TicketingFeesInfo* tInfo =
    // TestTicketingFeesInfoFactory::create("/vobs/atseintl/test/testdata/data/TicketingInfoOne.xml");
    TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoThree.xml");

    diag.printFistPartDetailedRequest(tInfo);

    string expected;
    string actual = diag.str();

    expected += "------------------- OB FEES DETAILED INFO --------------------\n";
    expected += "SVC CODE      : OBFCA     ";
    expected += "CXR CODE:  AF         ";
    expected += "PAX TYPE: ADT\n";
    expected += "SEQ NO        : 100200 \n";
    expected += "FOP BIN       : 456**  \n";
    expected += "TKT EFF DATE  : N/A";
    expected += "     TKT DISC DATE : N/A\n";
    expected += "JOURNEY IND   : Y\n";
    expected += "     LOC1 TYPE: C ";
    expected += "  LOC1 : AEP     ";
    expected += "  LOC1 ZONE : 123     \n";

    expected += "     LOC2 TYPE: C ";
    expected += "  LOC2 : DFW     ";
    expected += "  LOC2 ZONE : 456     \n";

    expected += "WHOLLY WITHIN\n";
    expected += "     LOC TYPE : C ";
    expected += "  LOC  : FRA     ";
    expected += "  LOC  ZONE : \n";

    expected += "VIA  LOC TYPE : C ";
    expected += "  LOC  : LON     ";
    expected += "  LOC  ZONE : 789     \n";
    expected += "  \n";

    expected += "FARE IND      : A";
    expected += "   PRIMARY CXR: AA";
    expected += "    FARE BASIS: Y26B\n";
    expected += "FEE AMOUNT    :   20.00USD\n";
    expected += "FEE PERCENT   : ";
    expected += "    MAX FEE AMOUNT: \n";

    expected += "IATA INTERLINE: N";
    expected += "   COMMISSION : N";
    expected += "   REFUND/REISSUE: N\n";
    expected += "NO CHARGE     : X";
    expected += "   TAX INCLUDE:  \n";

    expected += "ACC CODE T172 : 34567   \n";
    expected += "TKT DSGN T173 : 123456  \n";
    expected += "SECURITY T183 : 9876543";
    expected += "   PRIVATE IND: P \n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintTktDesignPartDetailedRequest()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);

    TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoThree.xml");

    diag.printTktDesignatorTable173Header(tInfo->svcFeesTktDsgnTblItemNo());

    string expected;
    string actual = diag.str();

    expected += " .....................................................\n";
    expected += " * TKT DESIGNATOR T173 ITEM NO: 123456   DETAIL INFO *\n";
    expected += " .....................................................\n";
    expected += "  SEQ NO           TKT DSGN CODE   STATUS\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintFinalPartDetailedRequest()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);

    // TicketingFeesInfo* tInfo =
    // TestTicketingFeesInfoFactory::create("/vobs/atseintl/test/testdata/data/TicketingInfoThree.xml");
    StatusS4Validation status = PASS_S4;
    diag.printFinalPartDetailedRequest(status);

    string expected;
    string actual = diag.str();
    expected += "                                   S4 STATUS : PASS\n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintAccountCodeTable172Header()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    string expected;
    expected += " *....................................................*\n";
    expected += " *  ACC CODE T172 ITEM NO : 1234567 DETAIL INFO\n";
    expected += " *....................................................*\n";
    expected += " SEQ NO   ACCOUNT CODE           STATUS\n";
    diag.printAccountCodeTable172Header(1234567);
    string actual = diag.str();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintAccountCodeTable172Info()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);

    SvcFeesAccCodeInfo* accInfo = TestSvcFeesAccCodeInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/AccCodeInfoOne.xml");
    *((SvcFeesDiagCollector*)&diag) << *accInfo;
    diag.printValidationStatus(false);

    string expected;
    string actual = diag.str();
    expected += " 101020   AAA00                  FAIL\n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintTktDesignatorTable173Info()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);

    SvcFeesTktDesignatorInfo* accInfo = TestSvcFeesTktDesignatorInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TktDesignatorInfoOne.xml");
    *((SvcFeesDiagCollector*)&diag) << *accInfo;
    diag.printValidationStatus(true);

    string expected;
    string actual = diag.str();

    expected += "   101020            ID200         PASS\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintSecurityTable183Header()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);

    string expected;
    expected += " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
    expected += " * SECURITY T183 ITEM NO : 101     DETAIL INFO       * \n";
    expected += " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n";
    expected += " SEQ NO TVL GDS DUTY GEO LOC TYPE CODE VIEW STATUS \n";
    diag.printSecurityTable183Header(101);
    string actual = diag.str();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintSecurityTable183Info()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);

    SvcFeesSecurityInfo* accInfo = TestSvcFeesSecurityInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/SecurityInfoOne.xml");
    StatusT183Security status = PASS_T183;
    accInfo->viewBookTktInd() = '1';
    diag.printSecurityTable183Info(accInfo, status);

    string expected;
    string actual = diag.str();
    expected += " 100200  X  1S  DFW  C DFW   T IAD      1   PASS \n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintCanNotCollectPrintINTERNAL_ERROR()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printCanNotCollect(INTERNAL_ERROR);
    string actual = diag.str();

    string expected = " *** NOT PROCESSED - INTERNAL ERROR\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintCanNotCollectPrintTJR_NOT_APPLY_TKTFEE()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printCanNotCollect(TJR_NOT_APPLY_TKTFEE);
    string actual = diag.str();

    string expected = " *** NOT PROCESSED - TJR OBFEES NOT APPLY SET TO Y\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintCanNotCollectPrintOB_NOT_ACTIVE_YET()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printCanNotCollect(OB_NOT_ACTIVE_YET);
    string actual = diag.str();

    string expected = " *** NOT PROCESSED - TICKETING DATE BEFORE OB ACTIVATION DATE\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintCanNotCollectPrintVALIDATING_CXR_EMPTY()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printCanNotCollect(VALIDATING_CXR_EMPTY);
    string actual = diag.str();

    string expected = " *** NOT PROCESSED - VALIDATING CXR EMPTY\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintCanNotCollectPrintALL_SEGS_OPEN()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printCanNotCollect(ALL_SEGS_OPEN);
    string actual = diag.str();

    string expected = " *** NOT PROCESSED - ALL SEGMENTS OPEN\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintCanNotCollectPrintNOT_ALL_SEGS_CONFIRM()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printCanNotCollect(NOT_ALL_SEGS_CONFIRM);
    string actual = diag.str();

    string expected = " *** NOT PROCESSED - ALL SEGMENTS NOT CONFIRMED\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintCanNotCollectPrintNON_CREDIT_CARD_FOP()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printCanNotCollect(NON_CREDIT_CARD_FOP);
    string actual = diag.str();

    string expected = " *** NOT PROCESSED - NON CREDIT CARD FOP\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintCxrNotActive()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    const CarrierCode& cxr = "CO";
    diag.printCxrNotActive(cxr);
    string actual = diag.str();

    string expected = " *** NOT PROCESSED - OB FEE PROCESSING NOT ACTIVE FOR CO ***\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintFopBinNotFound()
  {
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printFopBinNotFound();
    string actual = diag.str();

    string expected = " *** NO S4 SEQUENCE FOUND FOR BIN NUMBER\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintFopBinNumber()
  {
    std::vector<FopBinNumber> fopBin;
    fopBin.push_back("123456");
    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printFopBinNumber(fopBin);
    string actual = diag.str();

    string expected = "                  FOP BIN NUMBER - 123456\n";
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }

  void testPrintDiagHeaderAmountFopResidualInd()
  {
    MoneyAmount mAmt = 9.11;
    bool chargeResInd = true;
    CurrencyCode cc = "GBP";

    Diagnostic diagroot(Diagnostic870);
    diagroot.activate();
    Diag870Collector diag(diagroot);
    diag.enable(Diagnostic870);
    diag.printAmountFopResidualInd(cc, mAmt, chargeResInd);
    string actual = diag.str();

    string expected;
    expected += "                  CHARGE AMOUNT  -     9.11 GBP\n";
    expected += "                  RESIDUAL IND   - TRUE\n";

    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag870CollectorTest);
} // tse
