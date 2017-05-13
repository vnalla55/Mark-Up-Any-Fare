#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "Diagnostic/Diag888Collector.h"

#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/BrandedFare.h"
#include "DBAccess/BrandedFareSeg.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/SvcFeesAccCodeInfo.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "DBAccess/SvcFeesFeatureInfo.h"
#include "DBAccess/TaxText.h"
#include "Rules/RuleConst.h"

using namespace std;

namespace tse
{
using boost::assign::operator+=;

class Diag888CollectorDataHandleMock : public DataHandleMock
{
  TestMemHandle _memHandle;

  const TaxText* getTaxText(const VendorCode& vendor, int itemNo)
  {
    TaxText* ret(0);
    ret = _memHandle.create<TaxText>();
    ret->txtMsgs() += "EXEMPTIONS-";
    return ret;
  }

  const std::vector<SvcFeesAccCodeInfo*>&
  getSvcFeesAccountCode(const VendorCode& vc, const int itemNo)
  {
    std::vector<SvcFeesAccCodeInfo*>* vectAcc =
        _memHandle.create<std::vector<SvcFeesAccCodeInfo*> >();
    SvcFeesAccCodeInfo* ret = _memHandle.create<SvcFeesAccCodeInfo>();
    ret->seqNo() = 10000;
    ret->itemNo() = 3882;
    ret->accountCode() = "ACC11";
    vectAcc->push_back(ret);
    return *vectAcc;
  }

  const std::vector<SvcFeesSecurityInfo*>&
  getSvcFeesSecurity(const VendorCode& vc, const int itemNo)
  {
    std::vector<SvcFeesSecurityInfo*>* vectSec = _memHandle.create<vector<SvcFeesSecurityInfo*> >();
    SvcFeesSecurityInfo* ret = _memHandle.create<SvcFeesSecurityInfo>();
    ret->itemNo() = 9548;
    ret->seqNo() = 1;
    ret->carrierGdsCode() = "1S";
    ret->viewBookTktInd() = '1';
    vectSec->push_back(ret);
    return *vectSec;
  }

  const std::vector<SvcFeesFareIdInfo*>& getSvcFeesFareIds(const VendorCode& vc, long long itemNo)
  {
    std::vector<SvcFeesFareIdInfo*>* vectFar = _memHandle.create<vector<SvcFeesFareIdInfo*> >();
    SvcFeesFareIdInfo* ret = _memHandle.create<SvcFeesFareIdInfo>();
    ret->vendor() = Vendor::ATPCO;
    ret->itemNo() = 8888;
    ret->seqNo() = 199;
    ret->fareApplInd() = ' ';
    ret->owrt() = '1';
    ret->ruleTariff() = 555;
    ret->ruleTariffInd() = "PUB";
    ret->rule() = "345";
    ret->fareClass() = "GOGO";
    ret->fareType() = "BUR";
    ret->paxType() = "ABC";
    ret->routing() = 1991;
    ret->bookingCode1() = "FN";
    ret->bookingCode2() = "A";
    ret->source() = 'A';
    ret->minFareAmt1() = 50;
    ret->maxFareAmt1() = 255;
    ret->cur1() = "USD";
    ret->noDec1() = 2;
    ret->minFareAmt2() = 20;
    ret->maxFareAmt2() = 777;
    ret->cur2() = "AUD";
    ret->noDec2() = 3;
    vectFar->push_back(ret);
    return *vectFar;
  }

  const std::vector<SvcFeesFeatureInfo*>& getSvcFeesFeature(const VendorCode& vc, long long itemNo)
  {
    std::vector<SvcFeesFeatureInfo*>* vectFar = _memHandle.create<vector<SvcFeesFeatureInfo*> >();
    SvcFeesFeatureInfo* ret = _memHandle.create<SvcFeesFeatureInfo>();
    ret->vendor() = Vendor::ATPCO;
    ret->itemNo() = 19876;
    ret->seqNo() = 1;
    ret->carrier() = "DL";
    ret->fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    ret->serviceTypeCode() = "OC";
    ret->serviceSubTypeCode() = "0XX";
    ret->segmentAppl1() = 'C';
    ret->segmentAppl2() = 'F';
    ret->segmentAppl3() = 'N';
    ret->segmentAppl4() = 'A';
    ret->segmentAppl5() = 'N';
    ret->segmentAppl6() = 'N';
    ret->segmentAppl7() = ' ';
    ret->segmentAppl8() = 'N';
    ret->segmentAppl9() = 'N';
    ret->segmentAppl10() = ' ';
    vectFar->push_back(ret);
    return *vectFar;
  }

public:
  Diag888CollectorDataHandleMock() {}
};

class Diag888CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag888CollectorTest);
  CPPUNIT_TEST(testPrintS8CommonHeader);
  CPPUNIT_TEST(testPrintS8Banner);
  CPPUNIT_TEST(testPrintS8NotProcessed);

  CPPUNIT_TEST(testPrintS8NotFound_ATP);
  CPPUNIT_TEST(testPrintS8NotFound_MMGR);
  CPPUNIT_TEST(testPrintS8FareMarket);
  CPPUNIT_TEST(testPrintS8BrandedFaresContent);
  CPPUNIT_TEST(testPrintS8BrandedFaresDetailContent_with_Three_segements);
  CPPUNIT_TEST(testPrintS8BrandedFaresDetailContent_with_Three_segements_plus_DetailedTables);

  CPPUNIT_TEST_SUITE_END();

private:
  Diag888Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;
  Diag888CollectorDataHandleMock* _dataHandleMock;
  PricingTrx* _trx;
  BrandedFare* _brandedFare;
  PricingRequest* _req;

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diag888Collector diag;
      CPPUNIT_ASSERT_EQUAL(string(""), diag.str());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void setUp()
  {
    try
    {
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic888));
      _diagroot->activate();
      _diag = _memHandle.insert(new Diag888Collector(*_diagroot));
      _diag->enable(Diagnostic888);
      _dataHandleMock = _memHandle.create<Diag888CollectorDataHandleMock>();
      _brandedFare = _memHandle.create<BrandedFare>();
      _trx = _memHandle.create<PricingTrx>();
      _diag->trx() = _trx;
      _req = _memHandle.create<PricingRequest>();
      _trx->setRequest(_req);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

  void createPartiallyBrandedFare()
  {
    _brandedFare->seqNo() = 1010101;
    _brandedFare->segCount() = 3;
    _brandedFare->vendor() = "ATP";
    _brandedFare->carrier() = "YY";
    _brandedFare->psgType() = "GPY";
    _brandedFare->programCode() = "CPPUNIT TS";
  }

  void createFullBrandedFare()
  {
    _brandedFare->source() = 'T';
    _brandedFare->tvlFirstYear() = 13;
    _brandedFare->tvlFirstMonth() = 10;
    _brandedFare->tvlFirstDay() = 29;
    _brandedFare->publicPrivateInd() = 'P';
    _brandedFare->svcFeesAccountCodeTblItemNo() = 3882;
    _brandedFare->svcFeesSecurityTblItemNo() = 9548;
    _brandedFare->directionality() = '3';
    _brandedFare->loc1ZoneTblItemNo() = "123456";
    _brandedFare->loc2ZoneTblItemNo() = "234567";
    _brandedFare->effDate() = DateTime(2013, 5, 1);
    _brandedFare->discDate() = DateTime(2013, 11, 9);
    _brandedFare->locKey1().loc() = LOC_NYC;
    _brandedFare->locKey1().locType() = LOCTYPE_CITY;
    strToGlobalDirection(_brandedFare->globalInd(), "AP");
    _brandedFare->oneMatrix() = 'Z';
    _brandedFare->programText() = "CPPUNIT TEST FOR BRANDED FARES";
    _brandedFare->svcFeesFeatureTblItemNo() = 19876;
    _brandedFare->taxTextTblItemNo() = 555555;

    _brandedFare->segments()[0]->segNo() = 1;
    _brandedFare->segments()[0]->tier() = 1;
    _brandedFare->segments()[0]->brandName() = "APPLE 1";
    _brandedFare->segments()[0]->svcFeesFareIdTblItemNo() = 321321;
    _brandedFare->segments()[0]->taxTextTblItemNo() = 12;

    BrandedFareSeg* seg1(new BrandedFareSeg);
    _brandedFare->segments().push_back(seg1);
    _brandedFare->segments()[1]->segNo() = 10;
    _brandedFare->segments()[1]->tier() = 3;
    _brandedFare->segments()[1]->brandName() = "CUCUMBER";
    _brandedFare->segments()[1]->svcFeesFareIdTblItemNo() = 4343;
    _brandedFare->segments()[1]->taxTextTblItemNo() = 12;
    BrandedFareSeg* seg2(new BrandedFareSeg);
    _brandedFare->segments().push_back(seg2);
    _brandedFare->segments()[2]->segNo() = 20;
    _brandedFare->segments()[2]->tier() = 5;
    _brandedFare->segments()[2]->brandName() = "PRINTER";
    _brandedFare->segments()[2]->svcFeesFareIdTblItemNo() = 4666;
    _brandedFare->segments()[2]->taxTextTblItemNo() = 12;
  }

  void testPrintS8CommonHeader()
  {
    _diag->printS8CommonHeader();
    CPPUNIT_ASSERT_EQUAL(string("V CXR   SEQ      PAX  PROGRAM      BRANDS\n"), _diag->str());
  }

  void testPrintS8Banner()
  {
    _diag->printS8Banner();
    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES - S8 ANALYSIS ******************\n"),
                         _diag->str());
  }

  void testPrintS8NotProcessed()
  {
    _diag->printS8NotProcessed();
    CPPUNIT_ASSERT_EQUAL(string("   DATA NOT PROCESSED\n"), _diag->str());
  }

  void testPrintS8NotFound_ATP()
  {
    VendorCode vc = "ATP";
    CarrierCode cc = "AY";

    _diag->printS8NotFound(vc, cc);
    CPPUNIT_ASSERT_EQUAL(string("A  AY      DATA NOT FOUND\n"), _diag->str());
  }

  void testPrintS8NotFound_MMGR()
  {
    VendorCode vc = "MMGR";
    CarrierCode cc = "AY";

    _diag->printS8NotFound(vc, cc);
    CPPUNIT_ASSERT_EQUAL(string("M  AY      DATA NOT FOUND\n"), _diag->str());
  }

  void testPrintS8FareMarket()
  {
    FareMarket fm;
    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = "DFW";
    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = "RIC";
    fm.governingCarrier() = "AY";
    fm.origin() = origin;
    fm.destination() = destination;

    _diag->printS8FareMarket(fm);
    CPPUNIT_ASSERT_EQUAL(string("------------ FARE MARKET : DFW - RIC   CXR - AY -------------\n"),
                         _diag->str());
  }

  void testPrintS8BrandedFaresContent()
  {
    createPartiallyBrandedFare();
    _diag->printS8BrandedFaresContent(_brandedFare);

    CPPUNIT_ASSERT_EQUAL(string("A  YY   1010101  GPY  CPPUNIT TS    3 \n"), _diag->str());
  }

  void testPrintS8BrandedFaresDetailContent_with_Three_segements()
  {
    BrandedFare::dummyData(*_brandedFare);
    createFullBrandedFare();
    _brandedFare->segCount() = 3;

    _diag->printS8DetailContent(*_trx, _brandedFare);

    CPPUNIT_ASSERT_EQUAL(string("--------------- BRANDED FARES S8 DETAILED INFO ---------------\n"
                                " CARRIER: AA             VENDOR : ATP      SOURCE : T\n"
                                " SEQ NBR : 1111111111\n"
                                " S8 EFF DATE : 2013-05-01      TVL DATE START :   13-10-29\n"
                                " S8 DISC DATE: 2013-11-09      TVL DATE STOP  :    4-05-06\n"
                                "  \n"
                                "     PAX TYPE  : ADT\n"
                                " PRIVATE IND   : P \n"
                                " ACC CODE T172 : 3882   \n"
                                " SECURITY T183 : 9548   \n"
                                "DIRECTIONALITY : 3 \n"
                                "     LOC1 TYPE : C    LOC1 : NYC       LOC1 ZONE : 123456  \n"
                                "     LOC2 TYPE : B    LOC2 : NYC       LOC2 ZONE : 234567  \n"
                                " GLOBAL IND : AP          MATRIX : Z\n"
                                "  PROGRAM CODE : FFA        \n"
                                "  PROGRAM TEXT : CPPUNIT TEST FOR BRANDED FARES\n"
                                "ANCILLARY T166 : 19876  \n"
                                "TEXT T196 : 555555 \n"
                                "-------------------------------------------------------------\n"
                                " BRAND SEGMENTS : 3  \n"
                                "-------------------------------------------------------------\n"
                                " SEG NBR: 1     \n"
                                " TIER: 1       BRAND NAME:APPLE 1                        \n"
                                " FARE ID T189 : 321321 \n"
                                "    TEXT T196 : 12     \n"
                                "-------------------------------------------------------------\n"
                                " SEG NBR: 10    \n"
                                " TIER: 3       BRAND NAME:CUCUMBER                       \n"
                                " FARE ID T189 : 4343   \n"
                                "    TEXT T196 : 12     \n"
                                "-------------------------------------------------------------\n"
                                " SEG NBR: 20    \n"
                                " TIER: 5       BRAND NAME:PRINTER                        \n"
                                " FARE ID T189 : 4666   \n"
                                "    TEXT T196 : 12     \n"),
                         _diag->str());
  }

  void testPrintS8BrandedFaresDetailContent_with_Three_segements_plus_DetailedTables()
  {
    _trx->diagnostic().diagParamMap().insert(std::make_pair("TD", "YES"));
    _trx->diagnostic().diagParamMap().insert(std::make_pair("SQ", "100"));

    BrandedFare::dummyData(*_brandedFare);
    createFullBrandedFare();
    _brandedFare->segCount() = 3;

    _diag->printS8DetailContent(*_trx, _brandedFare);

    CPPUNIT_ASSERT_EQUAL(string("--------------- BRANDED FARES S8 DETAILED INFO ---------------\n"
                                " CARRIER: AA             VENDOR : ATP      SOURCE : T\n"
                                " SEQ NBR : 1111111111\n"
                                " S8 EFF DATE : 2013-05-01      TVL DATE START :   13-10-29\n"
                                " S8 DISC DATE: 2013-11-09      TVL DATE STOP  :    4-05-06\n"
                                "  \n"
                                "     PAX TYPE  : ADT\n"
                                " PRIVATE IND   : P \n"
                                " ACC CODE T172 : 3882   \n"
                                " SECURITY T183 : 9548   \n"
                                "DIRECTIONALITY : 3 \n"
                                "     LOC1 TYPE : C    LOC1 : NYC       LOC1 ZONE : 123456  \n"
                                "     LOC2 TYPE : B    LOC2 : NYC       LOC2 ZONE : 234567  \n"
                                " GLOBAL IND : AP          MATRIX : Z\n"
                                "  PROGRAM CODE : FFA        \n"
                                "  PROGRAM TEXT : CPPUNIT TEST FOR BRANDED FARES\n"
                                "ANCILLARY T166 : 19876  \n"
                                "TEXT T196 : 555555 \n"
                                " *....................................................*\n"
                                " *  ACC CODE T172 ITEM NO : 3882 DETAIL INFO\n"
                                " *....................................................*\n"
                                " SEQ NO   ACCOUNT CODE           STATUS\n"
                                " 10000    ACC11               \n"
                                " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                " * SECURITY T183 ITEM NO : 9548    DETAIL INFO       * \n"
                                " *- - - - - - - - - - - - - - - - - - - - - - - - - -* \n"
                                " SEQ NO TVL GDS DUTY GEO LOC TYPE CODE VIEW STATUS \n"
                                " 1          1S                          1   UNKNOWN STATUS \n"
                                "*-----------------------------------------------------*\n"
                                "*   FEE FEATURE T166 ITEM NO : 19876                  *\n"
                                "*-----------------------------------------------------*\n"
                                " VENDOR : ATP     SEQ NBR : 1         ITEM NO : 19876  \n"
                                " CARRIER  : DL \n"
                                " SERVICE  : FLIGHT   \n"
                                " SERVICE CODE : OC     SUB CODE : 0XX\n"
                                " SEGMENT APPLICATION : \n"
                                " 1: FOR CHARGE    2: FOR FREE      3: NOT OFFERED \n"
                                " 4: NOT APPL      5: NOT OFFERED   6: NOT OFFERED \n"
                                " 7:               8: NOT OFFERED   9: NOT OFFERED \n"
                                "10:             \n"
                                "-------------------------------------------------------------\n"
                                " BRAND SEGMENTS : 3  \n"
                                "-------------------------------------------------------------\n"
                                " SEG NBR: 1     \n"
                                " TIER: 1       BRAND NAME:APPLE 1                        \n"
                                " FARE ID T189 : 321321 \n"
                                "    TEXT T196 : 12     \n"
                                "*-----------------------------------------------------*\n"
                                "*   FARE IDENTIFICATION T189 ITEM NO : 321321         *\n"
                                "*-----------------------------------------------------*\n"
                                " VENDOR : ATP     SEQ NBR : 199       ITEM NO : 8888   \n"
                                " SOURCE : A\n"
                                "\n"
                                "FARE INFORMATION\n"
                                " APPL  : \n"
                                " OW/RT : 1 - ONE WAY MAY BE DOUBLED\n"
                                " RULETARIFF IND : PUB\n"
                                " RULE TARIFF : 555   RULE NUMBER : 345 \n"
                                " FARE CLASS  : GOGO    \n"
                                " FARE TYPE : BUR \n"
                                " PAX TYPE : ABC\n"
                                " ROUTING : 1991 \n"
                                " PRIME RBD1 : FN\n"
                                " PRIME RBD2 : A \n"
                                "\n"
                                " FARE RANGE AMOUNT\n"
                                " MIN 1 :      50.00USD   MIN 2 :     20.000AUD\n"
                                " MAX 1 :     255.00USD   MAX 2 :    777.000AUD\n"
                                "*------------------------------*\n"
                                "*-----------------------------------------------------*\n"
                                "*   TEXT TABLE T196 ITEM NO : 12                      *\n"
                                "*-----------------------------------------------------*\n"
                                "  EXEMPTIONS-\n"
                                "\n"
                                "-------------------------------------------------------------\n"
                                " SEG NBR: 10    \n"
                                " TIER: 3       BRAND NAME:CUCUMBER                       \n"
                                " FARE ID T189 : 4343   \n"
                                "    TEXT T196 : 12     \n"
                                "*-----------------------------------------------------*\n"
                                "*   FARE IDENTIFICATION T189 ITEM NO : 4343           *\n"
                                "*-----------------------------------------------------*\n"
                                " VENDOR : ATP     SEQ NBR : 199       ITEM NO : 8888   \n"
                                " SOURCE : A\n"
                                "\n"
                                "FARE INFORMATION\n"
                                " APPL  : \n"
                                " OW/RT : 1 - ONE WAY MAY BE DOUBLED\n"
                                " RULETARIFF IND : PUB\n"
                                " RULE TARIFF : 555   RULE NUMBER : 345 \n"
                                " FARE CLASS  : GOGO    \n"
                                " FARE TYPE : BUR \n"
                                " PAX TYPE : ABC\n"
                                " ROUTING : 1991 \n"
                                " PRIME RBD1 : FN\n"
                                " PRIME RBD2 : A \n"
                                "\n"
                                " FARE RANGE AMOUNT\n"
                                " MIN 1 :      50.00USD   MIN 2 :     20.000AUD\n"
                                " MAX 1 :     255.00USD   MAX 2 :    777.000AUD\n"
                                "*------------------------------*\n"
                                "*-----------------------------------------------------*\n"
                                "*   TEXT TABLE T196 ITEM NO : 12                      *\n"
                                "*-----------------------------------------------------*\n"
                                "  EXEMPTIONS-\n"
                                "\n"
                                "-------------------------------------------------------------\n"
                                " SEG NBR: 20    \n"
                                " TIER: 5       BRAND NAME:PRINTER                        \n"
                                " FARE ID T189 : 4666   \n"
                                "    TEXT T196 : 12     \n"
                                "*-----------------------------------------------------*\n"
                                "*   FARE IDENTIFICATION T189 ITEM NO : 4666           *\n"
                                "*-----------------------------------------------------*\n"
                                " VENDOR : ATP     SEQ NBR : 199       ITEM NO : 8888   \n"
                                " SOURCE : A\n"
                                "\n"
                                "FARE INFORMATION\n"
                                " APPL  : \n"
                                " OW/RT : 1 - ONE WAY MAY BE DOUBLED\n"
                                " RULETARIFF IND : PUB\n"
                                " RULE TARIFF : 555   RULE NUMBER : 345 \n"
                                " FARE CLASS  : GOGO    \n"
                                " FARE TYPE : BUR \n"
                                " PAX TYPE : ABC\n"
                                " ROUTING : 1991 \n"
                                " PRIME RBD1 : FN\n"
                                " PRIME RBD2 : A \n"
                                "\n"
                                " FARE RANGE AMOUNT\n"
                                " MIN 1 :      50.00USD   MIN 2 :     20.000AUD\n"
                                " MAX 1 :     255.00USD   MAX 2 :    777.000AUD\n"
                                "*------------------------------*\n"
                                "*-----------------------------------------------------*\n"
                                "*   TEXT TABLE T196 ITEM NO : 12                      *\n"
                                "*-----------------------------------------------------*\n"
                                "  EXEMPTIONS-\n"
                                "\n"),
                         _diag->str());
  }
  /*
        string expected;
        string actual = _diag->str ();

        expected += "A  YY   1010101  GPY  CPPUNIT TS    3 \n";

  printExpectedVsActualValues(expected, actual);

        CPPUNIT_ASSERT_EQUAL( expected, actual );
  */

  void printExpectedVsActualValues(string& expected, string& actual)
  {
    cout << std::endl;
    cout << "XXXXXX" << endl;
    cout << actual << endl;
    cout << "XXXXX" << endl;
    cout << "XXXXXX" << endl;
    cout << expected << endl;
    cout << "XXXXX" << endl;

    cout << "length of Expected: " << expected.size();
    cout << "length of Actual  : " << actual.size();
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag888CollectorTest);
} // tse
