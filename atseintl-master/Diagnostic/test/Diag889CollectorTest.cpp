#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "Diagnostic/Diag889Collector.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"

using namespace std;

namespace tse
{
using boost::assign::operator+=;

class Diag889CollectorDataHandleMock : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  Diag889CollectorDataHandleMock() {}
};

class Diag889CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag889CollectorTest);
  CPPUNIT_TEST(testPrintT189Banner);
  CPPUNIT_TEST(testPrintT189BannerWithDisableSoftPass);
  CPPUNIT_TEST(testPrintPaxTypeFareCommonHeader);
  CPPUNIT_TEST(testDisplayT189Secondary);
  CPPUNIT_TEST(testPrintSeqNoAndStatus);
  CPPUNIT_TEST(testPrintTravelInfo);
  CPPUNIT_TEST(testPrintDetailBrandProgramDataSourceS8);
  CPPUNIT_TEST(testPrintDetailBrandProgramDataSourceCBAS);
  CPPUNIT_TEST(testPrintBrand);
  CPPUNIT_TEST(testPrintT189NotFound);
  CPPUNIT_TEST(testPrintBrandProgram);
  CPPUNIT_TEST(testPrintBrandT189Item);
  CPPUNIT_TEST(testPrintDataNotFound);
  CPPUNIT_TEST(testPrintCompleteBrand);
  CPPUNIT_TEST(testPrintBrandProgramNotFound);
  CPPUNIT_TEST(testPrintBrandNotFound);
  CPPUNIT_TEST(testPrintFareMarketNotMatched);
  CPPUNIT_TEST(testPrintNoFaresFound);
  CPPUNIT_TEST(testPrintT189SecondaryItemNoAndSeqNo);
  CPPUNIT_TEST(testPrintT189NotExist);
  CPPUNIT_TEST(testPrintT189SecondaryDetailInfo);
  CPPUNIT_TEST(testPrintFareNotValid);
  CPPUNIT_TEST(testStreamingOperatorPaxTypeFare);
  CPPUNIT_TEST(testDisplayStatusSoftpassRBD);
  CPPUNIT_TEST(testDisplayStatusFailFareClass);
  CPPUNIT_TEST(testDisplayStatusFailFareType);
  CPPUNIT_TEST(testDisplayStatusFailPaxType);
  CPPUNIT_TEST(testDisplayStatusFailRouting);
  CPPUNIT_TEST(testDisplayStatusFailPrimeRBD);
  CPPUNIT_TEST(testDisplayStatusFailRange1Currency);
  CPPUNIT_TEST(testDisplayStatusFailRange1Decimal);
  CPPUNIT_TEST(testDisplayStatusFailRange1MinAmount);
  CPPUNIT_TEST(testDisplayStatusFailRange1MaxAmount);
  CPPUNIT_TEST(testDisplayStatusFailRange2Currency);
  CPPUNIT_TEST(testDisplayStatusFailRange2Decimal);
  CPPUNIT_TEST(testDisplayStatusFailRange2MinAmount);
  CPPUNIT_TEST(testDisplayStatusFailRange2MaxAmount);
  CPPUNIT_TEST(testDisplayStatusApplNegative);
  CPPUNIT_TEST(testDisplayStatusFailOwrt);
  CPPUNIT_TEST(testDisplayStatusFailSource);
  CPPUNIT_TEST(testDisplayStatusFailRuleTariff);
  CPPUNIT_TEST(testDisplayStatusFailRule);
  CPPUNIT_TEST(testDisplayStatusFailSecT189);
  CPPUNIT_TEST(testDisplayStatusNoStatus);
  CPPUNIT_TEST(testPrintDetailInfo);
  CPPUNIT_TEST(testPrintCompleteBrandProgram);
  CPPUNIT_TEST(testFareMarketPrintDataNotFound);
  CPPUNIT_TEST(testFareMarketPrintBrandProgramNotFound);
  CPPUNIT_TEST(testPrintCarrierNotMatched);
  CPPUNIT_TEST(testPrintBlankLine);
  CPPUNIT_TEST(testPrintSize);
  CPPUNIT_TEST(testPrintBrandSizeAndCurrentBrand);
  CPPUNIT_TEST(testPrintCurrentBrand);
  CPPUNIT_TEST(testPrintFailGlobalDirection);
  CPPUNIT_TEST(testPrintFailDirectionality_3_AND_TO);
  CPPUNIT_TEST(testPrintFailDirectionality_3_AND_UNKNOWN);
  CPPUNIT_TEST(testPrintFailDirectionality_3_AND_BOTH);
  CPPUNIT_TEST(testPrintFailDirectionality_4_AND_FROM);
  CPPUNIT_TEST(testPrintBrandFilterMatched);
  CPPUNIT_TEST(testPrintBrandFilterNotMatched);
  CPPUNIT_TEST(testFareIsValidForPbbYes);
  CPPUNIT_TEST(testFareIsValidForPbbNo);
  CPPUNIT_TEST(testPrintTable189Header);
  CPPUNIT_TEST(testPrintT189SecondaryStatusPass);
  CPPUNIT_TEST(testPrintT189SecondaryStatusFail);
  CPPUNIT_TEST(testPrintNoProgramFound);
  CPPUNIT_TEST(testPrintMatchBasedOnBookingCodeHeaderPrimary);
  CPPUNIT_TEST(testPrintMatchBasedOnBookingCodeHeaderSecondary);
  CPPUNIT_TEST(testPrintMatchBasedOnFareBasisCodeHeader);
  CPPUNIT_TEST(testPrintMatchBasedOnExcludedFareBasisCodeHeader);
  CPPUNIT_TEST(testPrintMatchBasedOnFareBasisCodeInvalid);
  CPPUNIT_TEST(testPrintMatchBasedOnFareBasisCodeValid);
  CPPUNIT_TEST(testPrintMatchBasedOnBookingCodePrimaryInvalid);
  CPPUNIT_TEST(testPrintMatchBasedOnBookingCodePrimaryValid);
  CPPUNIT_TEST(testPrintMatchBasedOnBookingCodeSecondaryInalid);
  CPPUNIT_TEST(testPrintMatchBasedOnBookingCodeSecondaryValid);
  CPPUNIT_TEST(testPrintMatchBasedOnBookingCodeSecondaryValidEmpty);
  CPPUNIT_TEST(testPrintMatchBasedOnBookingCodePrimaryInvalidEmpty);
  CPPUNIT_TEST(testPrintValidateCbasResultFail);
  CPPUNIT_TEST(testPrintValidateCbasResultHardPass);
  CPPUNIT_TEST(testPrintValidateCbasResultSoftPass);
  CPPUNIT_TEST_SUITE_END();

private:
  Diag889Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;
  Diag889CollectorDataHandleMock* _dataHandleMock;
  PricingTrx* _trx;
  PricingRequest* _req;
  SvcFeesFareIdInfo* _svcFeesFareIdInfo;
  std::vector<SvcFeesFareIdInfo*>* _svcFeesFareIdInfoVector;
  FareMarket* _fm1;
  const tse::Loc* sfo;
  const tse::Loc* dfw;
  BrandInfo* _brand1;
  BrandProgram* _bProgram1;
  PaxTypeFare* _ptf1;

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diag889Collector diag;
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
      _memHandle.create<TestConfigInitializer>();
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic889));
      _diagroot->activate();
      _diag = _memHandle.insert(new Diag889Collector(*_diagroot));
      _diag->enable(Diagnostic889);
      _dataHandleMock = _memHandle.create<Diag889CollectorDataHandleMock>();
      _trx = _memHandle.create<PricingTrx>();
      _diag->trx() = _trx;
      _req = _memHandle.create<PricingRequest>();
      _trx->setRequest(_req);
      _svcFeesFareIdInfoVector = _memHandle.create<vector<SvcFeesFareIdInfo*> >();
      buildSvcFeesFareIdInfo();
      sfo = getLoc("SFO");
      dfw = getLoc("DFW");
      _fm1 = _memHandle.create<FareMarket>();
      createFareMarket(sfo, dfw, "AA");
      buildBrandProgram();
      FareClassCode fareClass = "GOGO";
      Fare* f1 = createFare(
          _fm1, Fare::FS_Domestic, GlobalDirection::US, ONE_WAY_MAY_BE_DOUBLED, "", fareClass);
      _ptf1 = createPaxTypeFare(f1, *_fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

  void buildSvcFeesFareIdInfo()
  {
    _svcFeesFareIdInfo = _memHandle.create<SvcFeesFareIdInfo>();
    _svcFeesFareIdInfo->vendor() = Vendor::ATPCO;
    _svcFeesFareIdInfo->itemNo() = 8888;
    _svcFeesFareIdInfo->seqNo() = 199;
    _svcFeesFareIdInfo->fareApplInd() = ' ';
    _svcFeesFareIdInfo->owrt() = '1';
    _svcFeesFareIdInfo->ruleTariff() = 555;
    _svcFeesFareIdInfo->ruleTariffInd() = "PUB";
    _svcFeesFareIdInfo->rule() = "345";
    _svcFeesFareIdInfo->fareClass() = "GOGO";
    _svcFeesFareIdInfo->fareType() = "BUR";
    _svcFeesFareIdInfo->paxType() = "ABC";
    _svcFeesFareIdInfo->routing() = 1991;
    _svcFeesFareIdInfo->bookingCode1() = "FN";
    _svcFeesFareIdInfo->bookingCode2() = "A";
    _svcFeesFareIdInfo->source() = 'A';
    _svcFeesFareIdInfo->minFareAmt1() = 50;
    _svcFeesFareIdInfo->maxFareAmt1() = 255;
    _svcFeesFareIdInfo->cur1() = "USD";
    _svcFeesFareIdInfo->noDec1() = 2;
    _svcFeesFareIdInfo->minFareAmt2() = 20;
    _svcFeesFareIdInfo->maxFareAmt2() = 777;
    _svcFeesFareIdInfo->cur2() = "AUD";
    _svcFeesFareIdInfo->noDec2() = 3;
    _svcFeesFareIdInfoVector->push_back(_svcFeesFareIdInfo);
  }

  const Loc* getLoc(const LocCode& locCode)
  {
    return TestLocFactory::create("/vobs/atseintl/test/testdata/data/Loc" + locCode + ".xml");
  }

  void createFareMarket(const Loc* origin, const Loc* destination, CarrierCode goveringCarrier)
  {
    _fm1->origin() = origin;
    _fm1->destination() = destination;
    _fm1->governingCarrier() = goveringCarrier;
  }

  Fare* createFare(FareMarket* fm1,
                   Fare::FareState state,
                   GlobalDirection gd,
                   Indicator owrt,
                   CurrencyCode currency,
                   FareClassCode& fareClass)
  {
    Fare* f1 = _memHandle.create<Fare>();
    FareInfo* fi1 = _memHandle.create<FareInfo>();
    TariffCrossRefInfo* tcri1 = _memHandle.create<TariffCrossRefInfo>();

    fi1->_globalDirection = gd;
    fi1->_owrt = owrt;
    fi1->_currency = currency;
    fi1->_fareClass = fareClass;
    fi1->_vendor = Vendor::ATPCO;
    fi1->_ruleNumber = "0";
    fi1->market1() = fm1->origin()->loc();
    fi1->market2() = fm1->destination()->loc();
    f1->initialize(state, fi1, *fm1, tcri1);
    return f1;
  }

  PaxTypeFare* createPaxTypeFare(
      Fare* f1, FareMarket& fm1, PaxTypeCode paxTypeCode, VendorCode vendorCode, Indicator adultInd)
  {
    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    PaxType* pt1 = _memHandle.create<PaxType>();
    PaxTypeInfo* pti1 = _memHandle.create<PaxTypeInfo>();

    pt1->paxType() = paxTypeCode;
    pt1->vendorCode() = vendorCode;
    pti1->adultInd() = adultInd;
    pt1->paxTypeInfo() = pti1;
    ptf1->initialize(f1, pt1, &fm1);
    return ptf1;
  }

  void buildBrandProgram()
  {
    _brand1 = _memHandle.create<BrandInfo>();
    _brand1->brandCode() = "app";
    _brand1->brandName() = "apple";
    _brand1->tier() = 99;
    _brand1->fareIDdataPrimaryT189() = *_svcFeesFareIdInfoVector;
    _bProgram1 = _memHandle.create<BrandProgram>();
    _bProgram1->programCode() = "us";
    _bProgram1->programName() = "domestic us";
    _bProgram1->passengerType().push_back("RUG");
    _bProgram1->programID() = "areaone";
  }

  void testPrintT189Banner()
  {
    _diag->printT189Banner();
    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES T189 ANALYSIS ***************\n"),
                         _diag->str());
  }

  void testPrintT189BannerWithDisableSoftPass()
  {
    TestConfigInitializer::setValue("DISABLE_SOFT_PASS", "Y", "TN_PATH");
    _diag->printT189Banner();
    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES T189 ANALYSIS ***************\n"
                                "SOFT PASSES DISABLED IN TN_SHOPPING\n"),
                         _diag->str());
  }

  void testPrintPaxTypeFareCommonHeader()
  {
    _diag->printPaxTypeFareCommonHeader();

    CPPUNIT_ASSERT_EQUAL(string("  GI V RULE FARE CLS TRF O O    AMT   CUR PAX PAX ROUT FARE\n"
                                "                     NUM R I              REQ ACT      TYPE\n"
                                "***********************************************************\n"),
                         _diag->str());
  }

  void testDisplayT189Secondary()
  {
    _diag->displayT189Secondary(_svcFeesFareIdInfoVector->front());
    CPPUNIT_ASSERT_EQUAL(string("S T189 ITEM NO :    8888 SEQ NO  :     199\n"), _diag->str());
  }

  void testPrintSeqNoAndStatus()
  {
    _diag->printSeqNoAndStatus(_svcFeesFareIdInfoVector->front(), PASS_T189);
    CPPUNIT_ASSERT_EQUAL(string("P   8888        199      "), _diag->str());
  }

  void testPrintTravelInfo()
  {
    _diag->printTravelInfo(_fm1);
    CPPUNIT_ASSERT_EQUAL(string("PORTION OF TRAVEL  :  -    GOV CARRIER  : AA \n"
                                "***********************************************************\n"),
                         _diag->str());
  }

  void testPrintDetailBrandProgramDataSourceS8()
  {
    _bProgram1->dataSource() = BRAND_SOURCE_S8;
    _diag->printDetailBrandProgram(_bProgram1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM CODE : US          VENDOR : \n"
                                "BRAND DATA SOURCE: S8\n"), _diag->str());
  }

  void testPrintDetailBrandProgramDataSourceCBAS()
  {
    _bProgram1->dataSource() = BRAND_SOURCE_CBAS;
    _diag->printDetailBrandProgram(_bProgram1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM CODE : US          VENDOR : \n"
                                "BRAND DATA SOURCE: CB\n"), _diag->str());
  }

  void testPrintBrand()
  {
    _diag->printBrand(_brand1);
    CPPUNIT_ASSERT_EQUAL(string("-----------------------------------------------------------\n"
                                "BRAND CODE : APP\n"
                                "BRAND NAME : APPLE                           TIER : 99     \n"),
                         _diag->str());
  }

  void testPrintT189NotFound()
  {
    _diag->printT189NotFound(_brand1);
    CPPUNIT_ASSERT_EQUAL(string("BRAND   : APP  P T189 ITEMNO : NOT FOUND\n"), _diag->str());
  }

  void testPrintBrandProgram()
  {
    _diag->printBrandProgram(_bProgram1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM : US         \n"), _diag->str());
  }

  void testPrintBrandT189Item()
  {
    _diag->printBrandT189Item(_svcFeesFareIdInfoVector->front(), _brand1);
    CPPUNIT_ASSERT_EQUAL(string("BRAND   : APP        \n"), _diag->str());
  }

  void testPrintDataNotFound()
  {
    _diag->printDataNotFound();
    CPPUNIT_ASSERT_EQUAL(string("BRAND DATA NOT FOUND\n"), _diag->str());
  }

  void testPrintCompleteBrand()
  {
    _diag->printCompleteBrand(_brand1);
    CPPUNIT_ASSERT_EQUAL(string("-----------------------------------------------------------\n"
                                "BRAND CODE : APP\n"
                                "BRAND NAME : APPLE                          \n"
                                "TIER NUMBER : 99     \n"
                                "PRIMARYFAREIDTABLE : \n"
                                "SECONDARYFAREIDTABLE : \n"),
                         _diag->str());
  }

  void testPrintBrandProgramNotFound()
  {
    _diag->printBrandProgramNotFound();
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM : NOT FOUND\n"), _diag->str());
  }

  void testPrintBrandNotFound()
  {
    _diag->printBrandNotFound();
    CPPUNIT_ASSERT_EQUAL(string("BRAND   : NOT FOUND P T189 ITEMNO : NOT FOUND\n"), _diag->str());
  }

  void testPrintFareMarketNotMatched()
  {
    _diag->printFareMarketNotMatched(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW NOT MATCHED\n"), _diag->str());
  }

  void testPrintNoFaresFound()
  {
    _diag->printNoFaresFound(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("NO FARES FOUND FOR MARKET : SFO-DFW\n"), _diag->str());
  }

  void testPrintT189SecondaryItemNoAndSeqNo()
  {
    _diag->printT189SecondaryItemNoAndSeqNo(_svcFeesFareIdInfo, true);
    CPPUNIT_ASSERT_EQUAL(string("S   8888        199      SOFTPASS SECOND RBD\n"),
                         _diag->str());
  }

  void testPrintT189NotExist()
  {
    _diag->printT189NotExist(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW NO FAREID T189 DATA EXIST\n"), _diag->str());
  }

  void testPrintT189SecondaryDetailInfo()
  {
    _diag->printT189SecondaryDetailInfo(_svcFeesFareIdInfo, true);
    CPPUNIT_ASSERT_EQUAL(string(" VENDOR : ATP     SEQ NBR : 199       ITEM NO : 8888   \n"
                                " SOURCE : A\n"
                                "\nFARE INFORMATION\n"
                                " PRIME RBD1 : FN\n"
                                " PRIME RBD2 : A \n"
                                " STATUS : SOFTPASS SECOND RBD\n"),
                         _diag->str());
  }

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

  void testPrintFareNotValid()
  {
    _diag->printFareNotValid();
    CPPUNIT_ASSERT_EQUAL(string("FARE NOT VALID FOR PRICING - SEE DIAG 499/DDALLFARES\n"),
                         _diag->str());
  }

  void testStreamingOperatorPaxTypeFare()
  {
    *_diag << *_ptf1;
    CPPUNIT_ASSERT_EQUAL(string("P US A 0    GOGO     0   X I         UNKADT      UNK\n"),
                         _diag->str());
  }

  void testDisplayStatusSoftpassRBD()
  {
    StatusT189 status = SOFTPASS_RBD;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("SOFTPASS RBD"), _diag->str());
  }

  void testDisplayStatusFailFareClass()
  {
    StatusT189 status = FAIL_FARECLASS;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL FARECLASS"), _diag->str());
  }

  void testDisplayStatusFailFareType()
  {
    StatusT189 status = FAIL_FARETYPE;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL FARETYPE"), _diag->str());
  }

  void testDisplayStatusFailPaxType()
  {
    StatusT189 status = FAIL_PAXTYPE;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL PAXTYPE"), _diag->str());
  }

  void testDisplayStatusFailRouting()
  {
    StatusT189 status = FAIL_ROUTING;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL ROUTING"), _diag->str());
  }

  void testDisplayStatusFailPrimeRBD()
  {
    StatusT189 status = FAIL_PRIME_RBD;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL PRIME RBD"), _diag->str());
  }

  void testDisplayStatusFailRange1Currency()
  {
    StatusT189 status = FAIL_RANGE1_CURR;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RANGE1 CURRENCY"), _diag->str());
  }

  void testDisplayStatusFailRange1Decimal()
  {
    StatusT189 status = FAIL_RANGE1_DECIMAL;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RANGE1 DECIMAL"), _diag->str());
  }

  void testDisplayStatusFailRange1MinAmount()
  {
    StatusT189 status = FAIL_RANGE1_MIN;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RANGE1 MIN AMOUNT"), _diag->str());
  }

  void testDisplayStatusFailRange1MaxAmount()
  {
    StatusT189 status = FAIL_RANGE1_MAX;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RANGE1 MAX AMOUNT"), _diag->str());
  }

  void testDisplayStatusFailRange2Currency()
  {
    StatusT189 status = FAIL_RANGE2_CURR;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RANGE2 CURRENCY"), _diag->str());
  }

  void testDisplayStatusFailRange2Decimal()
  {
    StatusT189 status = FAIL_RANGE2_DECIMAL;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RANGE2 DECIMAL"), _diag->str());
  }

  void testDisplayStatusFailRange2MinAmount()
  {
    StatusT189 status = FAIL_RANGE2_MIN;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RANGE2 MIN AMOUNT"), _diag->str());
  }

  void testDisplayStatusFailRange2MaxAmount()
  {
    StatusT189 status = FAIL_RANGE2_MAX;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RANGE2 MAX AMOUNT"), _diag->str());
  }

  void testDisplayStatusApplNegative()
  {
    StatusT189 status = APPL_NEGATIVE;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("APPL NEGATIVE"), _diag->str());
  }

  void testDisplayStatusFailOwrt()
  {
    StatusT189 status = FAIL_OWRT;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL OWRT"), _diag->str());
  }

  void testDisplayStatusFailSource()
  {
    StatusT189 status = FAIL_SOURCE;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL SOURCE"), _diag->str());
  }

  void testDisplayStatusFailRuleTariff()
  {
    StatusT189 status = FAIL_RULE_TARIFF;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RULE TARIFF"), _diag->str());
  }

  void testDisplayStatusFailRule()
  {
    StatusT189 status = FAIL_RULE;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL RULE"), _diag->str());
  }

  void testDisplayStatusFailSecT189()
  {
    StatusT189 status = FAIL_SEC_T189;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string("FAIL SEC T189"), _diag->str());
  }

  void testDisplayStatusNoStatus()
  {
    StatusT189 status = NO_STATUS;
    _diag->displayStatus(status);
    CPPUNIT_ASSERT_EQUAL(string(" "), _diag->str());
  }

  void testPrintDetailInfo()
  {
    StatusT189 status = PASS_T189;
    _diag->printDetailInfo(_svcFeesFareIdInfo, status);
    CPPUNIT_ASSERT_EQUAL(string("-----------------------------------------------------------\n"
                                " VENDOR : ATP     SEQ NBR : 199       ITEM NO : 8888   \n"
                                " SOURCE : A\n"
                                "\nFARE INFORMATION\n"
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
                                "\n FARE RANGE AMOUNT\n"
                                " MIN 1 :      50.00USD   MIN 2 :     20.000AUD\n"
                                " MAX 1 :     255.00USD   MAX 2 :    777.000AUD\n\n"),
                         _diag->str());
  }

  void testPrintCompleteBrandProgram()
  {
    _diag->printCompleteBrandProgram(_bProgram1);

    CPPUNIT_ASSERT_EQUAL(string("PROGRAM CODE : US         \n"
                                "PROGRAM NAME : DOMESTIC US                    \n"
                                "PROGRAM DESC : \n"
                                "VENDOR : \n"
                                "PSG TYPE : RUG \n"
                                "DIRECTION : \n"
                                "GLOBAL DIRECTION : \n"
                                "ORIGINLOC : \n"
                                "SEQ NO  : \n"
                                "EFFECTIVE DATE : N/A\n"
                                "DISCONTINUE DATE : N/A\n"
                                "ACCOUNT CODE : \n"),
                         _diag->str());
  }

  void testFareMarketPrintDataNotFound()
  {
    _diag->printDataNotFound(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW BRAND DATA NOT FOUND\n"), _diag->str());
  }

  void testFareMarketPrintBrandProgramNotFound()
  {
    _diag->printBrandProgramNotFound(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW PROGRAM : NOT FOUND\n"), _diag->str());
  }

  void testPrintCarrierNotMatched()
  {
    _diag->printCarrierNotMatched(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW CARRIER NOT MATCHED\n"), _diag->str());
  }

  void testPrintBlankLine()
  {
    _diag->printBlankLine();
    CPPUNIT_ASSERT_EQUAL(string("                                                           \n"),
                         _diag->str());
  }

  void testPrintSize()
  {
    _diag->printSize("NUMBER OF PROGRAMS: ", 1);
    CPPUNIT_ASSERT_EQUAL(string("NUMBER OF PROGRAMS: 1\n"), _diag->str());
  }

  void testPrintBrandSizeAndCurrentBrand()
  {
    _diag->printBrandSizeAndCurrentBrand(_brand1, 2);
    CPPUNIT_ASSERT_EQUAL(string("NUMBER OF BRANDS : 2\nCURRENT BRAND CODE : APP        \n"),
                         _diag->str());
  }

  void testPrintCurrentBrand()
  {
    _diag->printCurrentBrand(_brand1);
    CPPUNIT_ASSERT_EQUAL(string("CURRENT BRAND CODE : APP        \n"), _diag->str());
  }

  void testPrintFailGlobalDirection()
  {
    _bProgram1->globalDirection() = GlobalDirection::AT;
    _diag->printFailGlobalDirection(*_bProgram1, *_ptf1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM GLOBAL DIR : AT  DOES NOT MATCH FARE GLOBAL DIR : US\n"),
                         _diag->str());
  }

  void testPrintFailDirectionality_3_AND_TO()
  {
    _bProgram1->direction() = "OT";

    _diag->printFailDirectionality(*_bProgram1, *_ptf1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM DIR : OUTBOUND  DOES NOT MATCH FARE DIR : TO\n"),
                         _diag->str());

    _diag->clear();
    _bProgram1->originsRequested().clear();
    _bProgram1->originsRequested().insert(_ptf1->origin());
    _diag->printFailDirectionality(*_bProgram1, *_ptf1, true, Direction::ORIGINAL);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM DIR : OUTBOUND  / ORIGINAL (FROM) DOES NOT MATCH FARE DIR : TO\n"),
                             _diag->str());
  }

  void testPrintFailDirectionality_3_AND_UNKNOWN()
  {
    _bProgram1->direction() = "OT";
    FareInfo fareInfo;
    fareInfo.directionality() = TERMINATE;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    _diag->printFailDirectionality(*_bProgram1, *_ptf1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM DIR : OUTBOUND  DOES NOT MATCH FARE DIR : UNKNOWN\n"),
                         _diag->str());
  }

  void testPrintFailDirectionality_3_AND_BOTH()
  {
    _bProgram1->direction() = "OT";
    FareInfo fareInfo;
    fareInfo.directionality() = BOTH;
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    _diag->printFailDirectionality(*_bProgram1, *_ptf1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM DIR : OUTBOUND  DOES NOT MATCH FARE DIR : BOTH\n"),
                         _diag->str());
  }

  void testPrintFailDirectionality_4_AND_FROM()
  {
    _bProgram1->direction() = "IN";
    FareInfo fareInfo;
    fareInfo.directionality() = FROM;
    fareInfo.market1() = _fm1->origin()->loc();
    fareInfo.market2() = _fm1->destination()->loc();
    Fare fare;
    TariffCrossRefInfo tCRInfo;
    fare.initialize(Fare::FS_International, &fareInfo, *_fm1, &tCRInfo);
    _ptf1->setFare(&fare);
    _diag->printFailDirectionality(*_bProgram1, *_ptf1);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM DIR : INBOUND   DOES NOT MATCH FARE DIR : FROM\n"),
                         _diag->str());

    _diag->clear();
    _bProgram1->originsRequested().clear();
    _bProgram1->originsRequested().insert(_ptf1->destination());
    _diag->printFailDirectionality(*_bProgram1, *_ptf1, true, Direction::REVERSED);
    CPPUNIT_ASSERT_EQUAL(string("PROGRAM DIR : INBOUND   / REVERSED (TO)   DOES NOT MATCH FARE DIR : FROM\n"),
                         _diag->str());
  }

  void testPrintBrandFilterMatched()
  {
    _diag->printBrandFilter(*_fm1, _brand1->brandCode(), true);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW MATCHED      BRAND CODE : APP        \n"),
                         _diag->str());
  }

  void testPrintBrandFilterNotMatched()
  {
    _diag->printBrandFilter(*_fm1, _brand1->brandCode(), false);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW NOT MATCHED  BRAND CODE : APP        \n"),
                         _diag->str());
  }

  void testFareIsValidForPbbYes()
  {
    _diag->printIsValidForBranding(*_ptf1);
    CPPUNIT_ASSERT_EQUAL(string("FARE IS VALID FOR PBB :  YES\n"),
                         _diag->str());
  }

  void testFareIsValidForPbbNo()
  {
    _ptf1->setIsValidForBranding(false);
    _diag->printIsValidForBranding(*_ptf1);
    CPPUNIT_ASSERT_EQUAL(string("FARE IS VALID FOR PBB :  NO\n"),
                         _diag->str());
  }

  void testPrintTable189Header()
  {
    _diag->printTable189Header();
    CPPUNIT_ASSERT_EQUAL(string("P/S T189 ITEMNO SEQ NUM  STATUS\n"),
                         _diag->str());
  }

  void testPrintT189SecondaryStatusPass()
  {
    _diag->printT189SecondaryStatus(*_diag, true);
    CPPUNIT_ASSERT_EQUAL(string("SOFTPASS SECOND RBD\n"),
                         _diag->str());
  }

  void testPrintT189SecondaryStatusFail()
  {
    _diag->printT189SecondaryStatus(*_diag, false);
    CPPUNIT_ASSERT_EQUAL(string("FAIL SECOND RBD\n"),
                         _diag->str());
  }

  void testPrintNoProgramFound()
  {
    _diag->printNoProgramFound(*_fm1);
    CPPUNIT_ASSERT_EQUAL(string("FARE MARKET SFO-DFW NO PROGRAMS FOUND\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnBookingCodeHeaderPrimary()
  {
    _diag->printMatchBasedOnBookingCodeHeaderPrimary();
    CPPUNIT_ASSERT_EQUAL(string("PRIME RBD     STATUS\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnBookingCodeHeaderSecondary()
  {
    _diag->printMatchBasedOnBookingCodeHeaderSecondary();
    CPPUNIT_ASSERT_EQUAL(string("SECOND RBD    STATUS\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnFareBasisCodeHeader()
  {
    _diag->printMatchBasedOnFareBasisCodeHeader();
    CPPUNIT_ASSERT_EQUAL(string("FARE BASIS CODE      STATUS\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnExcludedFareBasisCodeHeader()
  {
    _diag->printMatchBasedOnExcludedFareBasisCodeHeader();
    CPPUNIT_ASSERT_EQUAL(string("MATCHING EXCLUDED FARE BASIS CODES:\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnFareBasisCodeInvalid()
  {
    FareBasisCode fareBasisCode("FBC0");
    _diag->printMatchBasedOnFareBasisCode(fareBasisCode, false);
    CPPUNIT_ASSERT_EQUAL(string("FBC0                 NO MATCH\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnFareBasisCodeValid()
  {
    FareBasisCode fareBasisCode("FBC1");
    _diag->printMatchBasedOnFareBasisCode(fareBasisCode, true);
    CPPUNIT_ASSERT_EQUAL(string("FBC1                 MATCH\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnBookingCodePrimaryInvalid()
  {
    BookingCode bookingCode("B0");
    _diag->printMatchBasedOnBookingCodeHeaderPrimary();
    _diag->printMatchBasedOnBookingCode(bookingCode, false);
    CPPUNIT_ASSERT_EQUAL(string("PRIME RBD     STATUS\n"
                                "B0            FAIL\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnBookingCodePrimaryValid()
  {
    BookingCode bookingCode("B1");
    _diag->printMatchBasedOnBookingCodeHeaderPrimary();
    _diag->printMatchBasedOnBookingCode(bookingCode, true);
    CPPUNIT_ASSERT_EQUAL(string("PRIME RBD     STATUS\n"
                                "B1            PASS\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnBookingCodeSecondaryInalid()
  {
    BookingCode bookingCode("B2");
    _diag->printMatchBasedOnBookingCodeHeaderSecondary();
    _diag->printMatchBasedOnBookingCode(bookingCode, false);
    CPPUNIT_ASSERT_EQUAL(string("SECOND RBD    STATUS\n"
                                "B2            FAIL\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnBookingCodeSecondaryValid()
  {
    BookingCode bookingCode("B3");
    _diag->printMatchBasedOnBookingCodeHeaderSecondary();
    _diag->printMatchBasedOnBookingCode(bookingCode, true);
    CPPUNIT_ASSERT_EQUAL(string("SECOND RBD    STATUS\n"
                                "B3            PASS\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnBookingCodeSecondaryValidEmpty()
  {
    BookingCode bookingCode("");
    _diag->printMatchBasedOnBookingCodeHeaderSecondary();
    _diag->printMatchBasedOnBookingCode(bookingCode, true);
    CPPUNIT_ASSERT_EQUAL(string("SECOND RBD    STATUS\n"
                                "              PASS\n"),
                         _diag->str());
  }

  void testPrintMatchBasedOnBookingCodePrimaryInvalidEmpty()
  {
    BookingCode bookingCode("");
    _diag->printMatchBasedOnBookingCodeHeaderPrimary();
    _diag->printMatchBasedOnBookingCode(bookingCode, false);
    CPPUNIT_ASSERT_EQUAL(string("PRIME RBD     STATUS\n"
                                "              FAIL\n"),
                         _diag->str());
  }

  void testPrintValidateCbasResultFail()
  {
    _diag->printValidateCbasResult(PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT_EQUAL(string("CBAS VALIDATION RESULT: FAIL\n"),
                         _diag->str());
  }

  void testPrintValidateCbasResultHardPass()
  {
    _diag->printValidateCbasResult(PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT_EQUAL(string("CBAS VALIDATION RESULT: HARD PASS\n"),
                         _diag->str());
  }

  void testPrintValidateCbasResultSoftPass()
  {
    _diag->printValidateCbasResult(PaxTypeFare::BS_SOFT_PASS);
    CPPUNIT_ASSERT_EQUAL(string("CBAS VALIDATION RESULT: SOFT PASS\n"),
                         _diag->str());
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag889CollectorTest);
} // tse
