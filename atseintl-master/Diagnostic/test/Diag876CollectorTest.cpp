#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesConcur.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag876Collector.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class Diag876CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag876CollectorTest);
  CPPUNIT_TEST(testShouldDisplay_S6_NoParam);
  CPPUNIT_TEST(testShouldDisplay_S6_SQ12345_Pass);
  CPPUNIT_TEST(testShouldDisplay_S6_SQ54321_Fail);
  CPPUNIT_TEST(testShouldDisplay_S5_NoParam);
  CPPUNIT_TEST(testShouldDisplay_S5_CXDL_Pass);
  CPPUNIT_TEST(testShouldDisplay_S5_CXLO_Fail);
  CPPUNIT_TEST(testShouldDisplay_S5_SGGL_Pass);
  CPPUNIT_TEST(testShouldDisplay_S5_SGBG_Fail);
  CPPUNIT_TEST(testShouldDisplay_S5_SCQWLPass);
  CPPUNIT_TEST(testShouldDisplay_S5_SCPOI_Fail);
  CPPUNIT_TEST(testShouldDisplay_TS_FMKRKLON_Pass);
  CPPUNIT_TEST(testShouldDisplay_TS_FMKRKDFW_Fail);
  CPPUNIT_TEST(testShouldDisplay_TS_FMDFWLON_Fail);
  CPPUNIT_TEST(testShouldDisplay_SG_NoParam);
  CPPUNIT_TEST(testShouldDisplay_SG_SGAB_Pass);
  CPPUNIT_TEST(testShouldDisplay_SG_SGDL_Fail);
  CPPUNIT_TEST(testShouldDisplay_CX_NoParam);
  CPPUNIT_TEST(testShouldDisplay_CX_CXAA_Pass);
  CPPUNIT_TEST(testShouldDisplay_CX_CXBB_Fail);
  CPPUNIT_TEST(testPrintHeader_NeedValidation);
  CPPUNIT_TEST(testPrintHeader_NoValidation);
  CPPUNIT_TEST(testPrintNoOCFeesFound);
  CPPUNIT_TEST(testPrintS5_Short);
  CPPUNIT_TEST(testPrintS5_Long);
  CPPUNIT_TEST(testPrintS6_Short);
  CPPUNIT_TEST(testPrintS6_Long);
  CPPUNIT_TEST(testPrintS5Concur_1);
  CPPUNIT_TEST(testPrintS5Concur_2);
  CPPUNIT_TEST(testPrintS5Concur_X);
  CPPUNIT_TEST(testPrintS6Found_True);
  CPPUNIT_TEST(testPrintS6Found_False);
  CPPUNIT_TEST(testPrintS6ValidationHeader_Marketing);
  CPPUNIT_TEST(testPrintS6ValidationHeader_Operating);
  CPPUNIT_TEST(testPrintS6Validation);
  CPPUNIT_TEST(testPrintS6ValidationNoPass);
  CPPUNIT_TEST(testPrintNoS6InDB);
  CPPUNIT_TEST(testAddStarLine);
  CPPUNIT_TEST(testShouldDisplay_typeA);
  CPPUNIT_TEST(testShouldDisplay_typeB);
  CPPUNIT_TEST(testShouldDisplay_typeC);
  CPPUNIT_TEST(testShouldDisplay_typeE);
  CPPUNIT_TEST(testShouldDisplay_typeF);
  CPPUNIT_TEST(testShouldDisplay_typeM);
  CPPUNIT_TEST(testShouldDisplay_typeT);
  CPPUNIT_TEST(testShouldDisplay_typeP);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _diag = _memHandle.create<Diag876Collector>();
    _trx = _memHandle.create<PricingTrx>();
    _diag->initTrx(*_trx);
    _diag->activate();
  }
  void tearDown() { _memHandle.clear(); }
  void testShouldDisplay_S6_NoParam() { CPPUNIT_ASSERT(_diag->shouldDisplay(*createS6())); }
  void testShouldDisplay_S6_SQ12345_Pass()
  {
    _trx->diagnostic().diagParamMap()["SQ"] = "12345";
    CPPUNIT_ASSERT(_diag->shouldDisplay(*createS6()));
  }
  void testShouldDisplay_S6_SQ54321_Fail()
  {
    _trx->diagnostic().diagParamMap()["SQ"] = "54321";
    CPPUNIT_ASSERT(!_diag->shouldDisplay(*createS6()));
  }
  void testShouldDisplay_S5_NoParam() { CPPUNIT_ASSERT(_diag->shouldDisplay(*createSCI())); }
  void testShouldDisplay_S5_CXDL_Pass()
  {
    _trx->diagnostic().diagParamMap()["CX"] = "DL";
    CPPUNIT_ASSERT(_diag->shouldDisplay(*createSCI()));
  }
  void testShouldDisplay_S5_CXLO_Fail()
  {
    _trx->diagnostic().diagParamMap()["CX"] = "LO";
    CPPUNIT_ASSERT(!_diag->shouldDisplay(*createSCI()));
  }
  void testShouldDisplay_S5_SGGL_Pass()
  {
    _trx->diagnostic().diagParamMap()["SG"] = "GL";
    CPPUNIT_ASSERT(_diag->shouldDisplay(*createSCI()));
  }
  void testShouldDisplay_S5_SGBG_Fail()
  {
    _trx->diagnostic().diagParamMap()["SG"] = "BG";
    CPPUNIT_ASSERT(!_diag->shouldDisplay(*createSCI()));
  }
  void testShouldDisplay_S5_SCQWLPass()
  {
    _trx->diagnostic().diagParamMap()["SC"] = "QWL";
    CPPUNIT_ASSERT(_diag->shouldDisplay(*createSCI()));
  }
  void testShouldDisplay_S5_SCPOI_Fail()
  {
    _trx->diagnostic().diagParamMap()["SC"] = "POI";
    CPPUNIT_ASSERT(!_diag->shouldDisplay(*createSCI()));
  }
  void testShouldDisplay_TS_FMKRKLON_Pass()
  {
    _trx->diagnostic().diagParamMap()["FM"] = "KRKLON";
    CPPUNIT_ASSERT(_diag->shouldDisplay(createAS("KRK", "WAW"), createAS("WAW", "LON")));
  }
  void testShouldDisplay_TS_FMKRKDFW_Fail()
  {
    _trx->diagnostic().diagParamMap()["FM"] = "KRKDFW";
    CPPUNIT_ASSERT(!_diag->shouldDisplay(createAS("KRK", "WAW"), createAS("WAW", "LON")));
  }
  void testShouldDisplay_TS_FMDFWLON_Fail()
  {
    _trx->diagnostic().diagParamMap()["FM"] = "DFWLON";
    CPPUNIT_ASSERT(!_diag->shouldDisplay(createAS("KRK", "WAW"), createAS("WAW", "LON")));
  }
  void testShouldDisplay_SG_NoParam() { CPPUNIT_ASSERT(_diag->shouldDisplay(createSG())); }
  void testShouldDisplay_SG_SGAB_Pass()
  {
    _trx->diagnostic().diagParamMap()["SG"] = "AB";
    CPPUNIT_ASSERT(_diag->shouldDisplay(createSG()));
  }
  void testShouldDisplay_SG_SGDL_Fail()
  {
    _trx->diagnostic().diagParamMap()["SG"] = "DL";
    CPPUNIT_ASSERT(!_diag->shouldDisplay(createSG()));
  }
  void testShouldDisplay_CX_NoParam() { CPPUNIT_ASSERT(_diag->shouldDisplay("ZX")); }
  void testShouldDisplay_CX_CXAA_Pass()
  {
    _trx->diagnostic().diagParamMap()["CX"] = "AA";
    CPPUNIT_ASSERT(_diag->shouldDisplay("AA"));
  }
  void testShouldDisplay_CX_CXBB_Fail()
  {
    _trx->diagnostic().diagParamMap()["CX"] = "BB";
    CPPUNIT_ASSERT(!_diag->shouldDisplay("AA"));
  }
  void testPrintHeader_NeedValidation()
  {
    FarePath fP; 
    _diag->printHeader(*createCXSet("AA"),
                       *createCXSet("BB", "CC"),
                       createAS("KRK", "LON"),
                       createAS("LON", "DFW"),
                       true,
                       fP);
    CPPUNIT_ASSERT_EQUAL(
        std::string("***********     OPTIONAL SERVICES S6 VALIDATION     ***********\n"
                    " MARKETING CARRIERS: AA \n"
                    " OPERATING CARRIERS: BB CC \n"
                    "  PORTION OF TRAVEL: KRK - DFW\n"
                    "      ** S6 VALIDATION IS NEEDED\n"
                    "***************************************************************\n"),
        _diag->str());
  }
  void testPrintHeader_NoValidation()
  {
    FarePath fP; 
    _diag->printHeader(*createCXSet("DD"),
                       *createCXSet("EE", "FF", "GG"),
                       createAS("NYC", "AMS"),
                       createAS("AMS", "TYO"),
                       false,
                       fP);
    CPPUNIT_ASSERT_EQUAL(
        std::string("***********     OPTIONAL SERVICES S6 VALIDATION     ***********\n"
                    " MARKETING CARRIERS: DD \n"
                    " OPERATING CARRIERS: EE FF GG \n"
                    "  PORTION OF TRAVEL: NYC - TYO\n"
                    "      ** S6 VALIDATION IS NOT NEEDED\n"
                    "***************************************************************\n"),
        _diag->str());
  }
  void testPrintNoOCFeesFound()
  {
    _diag->printNoOCFeesFound("AA", "BG");
    CPPUNIT_ASSERT_EQUAL(
        std::string("NO OCFEES FOUND FOR CARRIER: AA AND GROUP CODE: BG\n"
                    "***************************************************************\n"),
        _diag->str());
  }
  void testPrintS5_Short()
  {
    _diag->printS5(*createSCI(), "   MSG");
    CPPUNIT_ASSERT_EQUAL(std::string("S5 RECORD:\n"
                                     " CARRIER:    DL       VENDOR:      ATP     CONCUR:      1\n"
                                     " SRVTYPE:    OC       SRVGROUP:    GL\n"
                                     " SRVSUBTYPE: QWL      SRVSUBGROUP: RT\n"
                                     " SERVICE: MERCHANT \n"
                                     "   MSG\n"),
                         _diag->str());
  }
  void testPrintS5_Long()
  {
    _trx->diagnostic().diagParamMap()["DD"] = "INFO";
    _diag->printS5(*createSCI(), "   MSG2");
    CPPUNIT_ASSERT_EQUAL(std::string("S5 RECORD:\n"
                                     " VENDOR               - ATP\n"
                                     " CARRIER              - DL\n"
                                     " SERVICE TYPE CODE    - OC\n"
                                     " SERVICE SUBTYPE CODE - QWL\n"
                                     " SERVICE GROUP        - GL\n"
                                     " SERVICE SUBGROUP     - RT\n"
                                     " CONCUR               - 1\n"
                                     " SERVICE              - MERCHANT \n"
                                     "   MSG2\n"),
                         _diag->str());
  }
  void testPrintS6_Short()
  {
    _diag->printS6(*createS6(), "   MSG3");
    CPPUNIT_ASSERT_EQUAL(std::string("  S6 RECORD:\n"
                                     "   CARRIER:    LO       SEQ:      12345\n"
                                     "   SRVTYPE:    OC       SRVGROUP:    SG\n"
                                     "   SRVSUBTYPE: ABC      SRVSUBGROUP: AD\n"
                                     "    ASSESED CXR: BA  MKGOPER: O   CONCUR: Y\n"
                                     "                                                     MSG3\n"),
                         _diag->str());
  }
  void testPrintS6_Long()
  {
    _trx->diagnostic().diagParamMap()["DD"] = "INFO";
    _diag->printS6(*createS6(), "   MSG4");
    CPPUNIT_ASSERT_EQUAL(std::string("  S6 RECORD:\n"
                                     "   VENDOR               - ATP\n"
                                     "   CARRIER              - LO\n"
                                     "   CREATEDATE           - 12/13/2011\n"
                                     "   EXPIREDATE           - 01/02/2021\n"
                                     "   EFFDATE              - 07/08/2031\n"
                                     "   DISCDATE             - 03/04/2041\n"
                                     "   SEQ NO               - 12345\n"
                                     "   SERVICE TYPE CODE    - OC\n"
                                     "   SERVICE SUBTYPE CODE - ABC\n"
                                     "   SERVICE GROUP        - SG\n"
                                     "   SERVICE SUBGROUP     - AD\n"
                                     "   ASSEESSED CARRIER    - BA\n"
                                     "   MKGOPER FARE OWNER   - O\n"
                                     "   CONCUR               - Y\n"
                                     "                                                     MSG4\n"),
                         _diag->str());
  }
  /*
           1         2         3         4         5         6
  1234567890123456789012345678901234567890123456789012345678901234
                                 *** S5 CONCUR NOT ALLOWED ***\n
                            *** NO CONCURRENCE IS REQUIRED ***\n
                                          ** S6 FOUND FOR S5**\n
                              *** NO S5 PASS S6 VALIDATION ***\n
                          ** NO S6 RECORDS FOUND IN DATABASE **\n
  ********************************************\n
  */
  void testPrintS5Concur_1()
  {
    _diag->printS5Concur('1');
    CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
  }
  void testPrintS5Concur_2()
  {
    _diag->printS5Concur('2');
    CPPUNIT_ASSERT_EQUAL(
        std::string("                               *** S5 CONCUR NOT ALLOWED ***\n"),
        _diag->str());
  }
  void testPrintS5Concur_X()
  {
    _diag->printS5Concur('X');
    CPPUNIT_ASSERT_EQUAL(
        std::string("                          *** NO CONCURRENCE IS REQUIRED ***\n"),
        _diag->str());
  }
  void testPrintS6Found_True()
  {
    _diag->printS6Found(true);
    CPPUNIT_ASSERT_EQUAL(
        std::string("                                         ** S6 FOUND FOR S5**\n"),
        _diag->str());
  }
  void testPrintS6Found_False()
  {
    _diag->printS6Found(false);
    CPPUNIT_ASSERT_EQUAL(std::string("\n"), _diag->str());
  }
  void testPrintS6ValidationHeader_Marketing()
  {
    _diag->printS6ValidationHeader(true);
    CPPUNIT_ASSERT_EQUAL(
        std::string("MARKETING CARRIER THAT CAN BE ASSESED:\n"
                    " CX VENDR TYPE GROP SUBTYPE SUBGROP CONCUR               STATUS\n"
                    "***************************************************************\n"),
        _diag->str());
  }
  void testPrintS6ValidationHeader_Operating()
  {
    _diag->printS6ValidationHeader(false);
    CPPUNIT_ASSERT_EQUAL(
        std::string("OPERATING CARRIER THAT CAN BE ASSESED:\n"
                    " CX VENDR TYPE GROP SUBTYPE SUBGROP CONCUR               STATUS\n"
                    "***************************************************************\n"),
        _diag->str());
  }
  void testPrintS6Validation()
  {
    _diag->printS6Validation(*createSCI(), " MSG5");
    CPPUNIT_ASSERT_EQUAL(
        std::string(" DL   ATP   OC   GL     QWL      RT      1                 MSG5\n"),
        _diag->str());
  }
  void testPrintS6ValidationNoPass()
  {
    _diag->printS6ValidationNoPass();
    CPPUNIT_ASSERT_EQUAL(
        std::string("                            *** NO S5 PASS S6 VALIDATION ***\n"),
        _diag->str());
  }
  void testPrintNoS6InDB()
  {
    _diag->printNoS6InDB();
    CPPUNIT_ASSERT_EQUAL(
        std::string("                        ** NO S6 RECORDS FOUND IN DATABASE **\n"),
        _diag->str());
  }
  void testAddStarLine()
  {
    _diag->addStarLine(45);
    CPPUNIT_ASSERT_EQUAL(std::string("*********************************************\n"),
                         _diag->str());
  }

  void testShouldDisplay_typeA()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = BAGGAGE_ALLOWANCE;
    CPPUNIT_ASSERT(!_diag->shouldDisplay(s5));
  }

  void testShouldDisplay_typeB()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = CARRY_ON_ALLOWANCE;
    CPPUNIT_ASSERT(!_diag->shouldDisplay(s5));
  }

  void testShouldDisplay_typeC()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = BAGGAGE_CHARGE;
    CPPUNIT_ASSERT(!_diag->shouldDisplay(s5));
  }

  void testShouldDisplay_typeE()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = BAGGAGE_EMBARGO;
    CPPUNIT_ASSERT(!_diag->shouldDisplay(s5));
  }

  void testShouldDisplay_typeF()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    CPPUNIT_ASSERT(_diag->shouldDisplay(s5));
  }

  void testShouldDisplay_typeM()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = MERCHANDISE_SERVICE;
    CPPUNIT_ASSERT(_diag->shouldDisplay(s5));
  }

  void testShouldDisplay_typeT()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = TICKET_RELATED_SERVICE;
    CPPUNIT_ASSERT(_diag->shouldDisplay(s5));
  }

  void testShouldDisplay_typeP()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = PREPAID_BAGGAGE;
    CPPUNIT_ASSERT(_diag->shouldDisplay(s5));
  }

protected:
  AirSeg* createAS(LocCode orig, LocCode dest)
  {
    AirSeg* ret = _memHandle.create<AirSeg>();
    ret->origAirport() = orig;
    ret->destAirport() = dest;
    return ret;
  }
  OptionalServicesConcur* createS6()
  {
    OptionalServicesConcur* ret = _memHandle.create<OptionalServicesConcur>();
    ret->vendor() = "ATP";
    ret->carrier() = "LO";
    ret->seqNo() = 12345;
    ret->createDate() = DateTime(2011, 12, 13, 14, 15, 16);
    ret->expireDate() = DateTime(2021, 1, 2, 3, 4, 5);
    ret->effDate() = DateTime(2031, 7, 8, 9, 10, 11);
    ret->discDate() = DateTime(2041, 3, 4, 5, 6, 7);
    ret->serviceTypeCode() = "OC";
    ret->serviceSubTypeCode() = "ABC";
    ret->serviceGroup() = "SG";
    ret->serviceSubGroup() = "AD";
    ret->accessedCarrier() = "BA";
    ret->mkgOperFareOwner() = 'O';
    ret->concur() = 'Y';
    return ret;
  }
  SubCodeInfo* createSCI()
  {
    SubCodeInfo* ret = _memHandle.create<SubCodeInfo>();
    ret->vendor() = "ATP";
    ret->carrier() = "DL";
    ret->serviceTypeCode() = "OC";
    ret->serviceSubTypeCode() = "QWL";
    ret->createDate() = DateTime(2051, 12, 11, 10, 9, 8);
    ret->expireDate() = DateTime(2061, 7, 6, 5, 4, 3);
    ret->serviceGroup() = "GL";
    ret->serviceSubGroup() = "RT";
    ret->concur() = '1';
    ret->fltTktMerchInd() = MERCHANDISE_SERVICE;
    return ret;
  }
  ServiceFeesGroup* createSG()
  {
    ServiceFeesGroup* ret = _memHandle.create<ServiceFeesGroup>();
    ret->groupCode() = "AB";
    return ret;
  }
  std::set<CarrierCode>* createCXSet(const char* cxr1, const char* cxr2 = 0, const char* cxr3 = 0)
  {
    std::set<CarrierCode>* ret = _memHandle.create<std::set<CarrierCode> >();
    ret->insert(cxr1);
    if (cxr2)
      ret->insert(cxr2);
    if (cxr3)
      ret->insert(cxr3);
    return ret;
  }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Diag876Collector* _diag;
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag876CollectorTest);
}
