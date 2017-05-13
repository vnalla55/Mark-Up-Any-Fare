#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "Diagnostic/Diag898Collector.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/RuleExecution.h"
#include "BrandedFares/S8BrandingSecurity.h"
#include "BrandedFares/MarketRule.h"
#include "BrandedFares/S8BrandingResponseParser.h"
#include "BrandedFares/BSDiagnostics.h"

using namespace std;

namespace tse
{
using boost::assign::operator+=;

class Diag898CollectorDataHandleMock : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  Diag898CollectorDataHandleMock() {}
};

class Diag898CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag898CollectorTest);
  CPPUNIT_TEST(testPrintS8FareMarket);
  CPPUNIT_TEST(testPrintS8CommonHeader);
  CPPUNIT_TEST(testPrintS8DetailHeader);
  CPPUNIT_TEST(testPrintSeparator);
  CPPUNIT_TEST(testPrintRuleExecutionContent);
  CPPUNIT_TEST(testPrintDetailRuleExecutionContent);
  CPPUNIT_TEST(testPrintS8BrandingSecurity);
  CPPUNIT_TEST(testPrintMarketRuleDirectionOI);
  CPPUNIT_TEST(testPrintMarketRuleDirectionOT);
  CPPUNIT_TEST(testPrintMarketRuleDirectionIN);
  CPPUNIT_TEST(testPrintMarketRuleGlobalDirection);
  CPPUNIT_TEST(testIsDdInfo);
  CPPUNIT_TEST(testShouldDisplayCarrierMatched);
  CPPUNIT_TEST(testShouldDisplayCarrierNotMatched);
  CPPUNIT_TEST(testShouldDisplayProgramCode);
  CPPUNIT_TEST(testMatchFareMarketMatched);
  CPPUNIT_TEST(testMatchFareMarketNotMatched);
  CPPUNIT_TEST(testShouldDisplayPassedMatched);
  CPPUNIT_TEST(testShouldDisplayPassedNotMatched);
  CPPUNIT_TEST(testPrintDataNotFound);
  CPPUNIT_TEST(testPrintBrandedDataError);
  CPPUNIT_TEST(testMatchGlobalDirectionPass);
  CPPUNIT_TEST(testMatchGlobalDirectionFail);
  CPPUNIT_TEST(testMatchMarketDirectionOI);
  CPPUNIT_TEST(testMatchMarketDirectionOT);
  CPPUNIT_TEST(testMatchMarketDirectionIN);
  CPPUNIT_TEST(testMatchMarketDirectionBlank);
  CPPUNIT_TEST(testMatchAirportDirectionOI);
  CPPUNIT_TEST(testMatchAirportDirectionOT);
  CPPUNIT_TEST(testMatchAirportDirectionIN);
  CPPUNIT_TEST(testSetStatusForMarketFailureFailGeography);
  CPPUNIT_TEST(testSetStatusForMarketFailureFailDirection);
  CPPUNIT_TEST(testSetStatusForMarketFailureFailGlobalDirection);
  CPPUNIT_TEST(testPrintDiagnostic);
  CPPUNIT_TEST(testPrintDiagnosticDetail);
  CPPUNIT_TEST(testDisplayVendorATP);
  CPPUNIT_TEST(testDisplayVendorATPDetail);
  CPPUNIT_TEST(testDisplayVendorMMGR);
  CPPUNIT_TEST(testDisplayVendorMMGRDetail);
  CPPUNIT_TEST(testDisplayVendorUSOC);
  CPPUNIT_TEST(testDisplayVendorUSOCDetail);
  CPPUNIT_TEST(testDisplayVendorSITA);
  CPPUNIT_TEST(testDisplayVendorSITADetail);
  CPPUNIT_TEST(testGetStatusStrFailPsgrType);
  CPPUNIT_TEST(testGetStatusStrFailAccCode);
  CPPUNIT_TEST(testGetStatusStrFailMarket);
  CPPUNIT_TEST(testGetStatusStrFailPcc);
  CPPUNIT_TEST(testGetStatusStrFailCarrierGds);
  CPPUNIT_TEST(testGetStatusStrFailAgentLoc);
  CPPUNIT_TEST(testGetStatusStrFailDeptCode);
  CPPUNIT_TEST(testGetStatusStrFailOfficeDesig);
  CPPUNIT_TEST(testGetStatusStrFailSecurity);
  CPPUNIT_TEST(testGetStatusStrFailViewBook);
  CPPUNIT_TEST(testSetStatusPassSeclected);
  CPPUNIT_TEST(testSetStatusStr);
  CPPUNIT_TEST(testMatchGeoAirportFail);
  CPPUNIT_TEST(testMatchGeoAirportPass);
  CPPUNIT_TEST(testPrintBlankLine);
  CPPUNIT_TEST_SUITE_END();

private:
  Diag898Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;
  Diag898CollectorDataHandleMock* _dataHandleMock;
  PricingTrx* _trx;
  PricingRequest* _req;
  MarketResponse* _mResponse;
  BrandProgram* _bProgram1;
  BrandProgram* _bProgram2;
  BrandInfo* _brand1;
  BrandInfo* _brand2;
  BrandInfo* _brand3;
  MarketCriteria* _mCriteria;
  RuleExecution* _ruleExecution;
  S8BrandingSecurity* _s8BrandingSecurity;
  SecurityInfo* _securityInfo;
  MarketRule* _marketRule;
  std::map<int, std::vector<FareMarket*> >* _marketIDFareMarketMap;
  FareMarket* _fm1;
  const tse::Loc* sfo;
  const tse::Loc* dfw;
  PaxTypeFare* _ptf1;
  std::vector<MarketResponse*>* _mR;
  BSDiagnostics* _bsDiagnostics;
  bool _firstMarket;

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diag898Collector diag;
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
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic898));
      _diagroot->activate();
      _diag = _memHandle.insert(new Diag898Collector(*_diagroot));
      _diag->enable(Diagnostic898);
      _dataHandleMock = _memHandle.create<Diag898CollectorDataHandleMock>();
      _trx = _memHandle.create<PricingTrx>();
      _diag->trx() = _trx;
      _diag->initTrx(*_trx);
      _req = _memHandle.create<PricingRequest>();
      _trx->setRequest(_req);
      _mResponse = _memHandle.create<MarketResponse>();
      _bProgram1 = _memHandle.create<BrandProgram>();
      _bProgram2 = _memHandle.create<BrandProgram>();
      _mR = _memHandle.create<vector<MarketResponse*> >();
      _bsDiagnostics = _memHandle.create<BSDiagnostics>();
      buildMarketResponse();

      _mCriteria = _memHandle.create<MarketCriteria>();
      buildMarketCriteria();
      _ruleExecution = _memHandle.create<RuleExecution>();
      buildRuleExecution();
      _mResponse->bsDiagnostics() = _bsDiagnostics;
      _mResponse->bsDiagnostics()->ruleExecution().push_back(_ruleExecution);
      _mR->push_back(_mResponse);
      _trx->brandedMarketMap().insert(make_pair(1, *_mR));

      _s8BrandingSecurity = _memHandle.create<S8BrandingSecurity>();
      _securityInfo = _memHandle.create<SecurityInfo>();
      buildSecurityInfo();
      _ruleExecution->s8BrandingSecurity() = _s8BrandingSecurity;
      _marketRule = _memHandle.create<MarketRule>();
      buildMarketRule();
      MarketRuleInfo* marketRuleInfo = _memHandle.create<MarketRuleInfo>();
      marketRuleInfo->_marketRule = _marketRule;
      marketRuleInfo->_status = PASS_S8;
      _ruleExecution->marketRules().push_back(marketRuleInfo);
      _marketIDFareMarketMap = _memHandle.create<std::map<int, std::vector<FareMarket*> > >();
      sfo = getLoc("SFO");
      dfw = getLoc("DFW");
      _fm1 = _memHandle.create<FareMarket>();
      createFareMarket(sfo, dfw, "AA");
      FareClassCode fareClass = "GOGO";
      Fare* f1 = createFare(
          _fm1, Fare::FS_Domestic, GlobalDirection::US, ONE_WAY_MAY_BE_DOUBLED, "", fareClass);
      _ptf1 = createPaxTypeFare(f1, *_fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
      _firstMarket = true;
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

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

  void buildMarketResponse()
  {

    _brand1 = _memHandle.create<BrandInfo>();
    _brand2 = _memHandle.create<BrandInfo>();
    _brand3 = _memHandle.create<BrandInfo>();

    _mResponse->carrier() = "BA";
    _mResponse->brandPrograms().push_back(_bProgram1);
    _mResponse->brandPrograms().push_back(_bProgram2);
    _bProgram1->brandsData().push_back(_brand1);
    _bProgram1->brandsData().push_back(_brand2);
    _bProgram2->brandsData().push_back(_brand3);
    // Program1
    _bProgram1->programCode() = "US";
    _bProgram1->programName() = "DOMESTIC US";
    _bProgram1->passengerType().push_back("RUG");
    _bProgram1->programID() = "AREAONE";
    // Brand1
    _brand1->brandCode() = "APP";
    _brand1->brandName() = "APPLE";
    _brand1->tier() = 99;
    //_brand1->fareIDdataPrimaryT189() = *_svcFeesFareIdInfoVector;

    // Brand2
    _brand2->brandCode() = "ABB";
    _brand2->brandName() = "ABBREVIATE";
    _brand2->tier() = 10;
    //_brand2->fareIDdataPrimaryT189() = *_svcFeesFareIdInfoVector;

    // Program2 - Brand3
    _bProgram2->programCode() = "AFA";
    _bProgram2->programName() = "FLIGHT ANYWHERE";
    _bProgram2->passengerType().push_back("FLK");
    _bProgram2->programID() = "PROGRAM2";
    _brand3->brandCode() = "FOR";
    _brand3->brandName() = "FOREVER";
    _brand3->tier() = 55;
    //_brand3->fareIDdataPrimaryT189() = *_svcFeesFareIdInfoVector;

    _mResponse->programIDList().push_back(_bProgram1->programID());
    _mResponse->programIDList().push_back(_bProgram2->programID());
    _mResponse->setMarketID() = 1;
  }

  void buildMarketCriteria()
  {
    _mCriteria->direction() = "OT";
    _mCriteria->arrivalAirportCode() = "NYC";
    _mCriteria->departureAirportCode() = "LHR";
    _mCriteria->globalDirection() = GlobalDirection::AT;
    _mResponse->marketCriteria() = _mCriteria;
  }

  void buildRuleExecution()
  {
    _ruleExecution->programCode() = "BGI";
    _ruleExecution->ruleID() = "132";
    PatternInfo* patternInfoPCC = _memHandle.create<PatternInfo>();
    patternInfoPCC->_patternValue = "80K2";
    patternInfoPCC->_status = PASS_S8;
    _ruleExecution->pseudoCityCodes().push_back(patternInfoPCC);
    PatternInfo* patternInfoAccountCode = _memHandle.create<PatternInfo>();
    patternInfoAccountCode->_patternValue = "GSO01";
    patternInfoAccountCode->_status = PASS_S8;
    _ruleExecution->accountCodes().push_back(patternInfoAccountCode);
    PatternInfo* patternInfoGoverningCarrier = _memHandle.create<PatternInfo>();
    patternInfoGoverningCarrier->_patternValue = "BA";
    patternInfoGoverningCarrier->_status = PASS_S8;
    _ruleExecution->governingCarrier().push_back(patternInfoGoverningCarrier);
    _ruleExecution->salesDateStart() = DateTime(2013, 11, 4);
    _ruleExecution->salesDateEnd() = DateTime(2016, 2, 29);
    _ruleExecution->travelDateStart() = DateTime(2013, 11, 4);
    _ruleExecution->travelDateEnd() = DateTime(2016, 2, 29);
  }

  void buildSecurityInfo()
  {
    _securityInfo->_securityName = "CARRIERGDS";
    _securityInfo->_securityValue = "1S";
    _securityInfo->_status = PASS_S8;
    _s8BrandingSecurity->securityInfoVec().push_back(_securityInfo);
  }

  void buildMarketRule()
  {
    _marketRule->originLoc() = "LON";
    _marketRule->originLocType() = 'M';
    _marketRule->destinationLoc() = "USNY";
    _marketRule->destinationLocType() = 'S';
    _marketRule->direction() = "OI";
  }

  void testPrintS8FareMarket()
  {
    _diag->printS8FareMarket(_mResponse);
    CPPUNIT_ASSERT_EQUAL(string("------------ FARE MARKET : LHR - NYC   CXR - BA ----------\n"),
                         _diag->str());
  }

  void testPrintS8CommonHeader()
  {
    _diag->printS8CommonHeader();
    CPPUNIT_ASSERT_EQUAL(string("V CXR   PROGRAMID      PROGRAM     PAX  STATUS\n"), _diag->str());
  }

  void testPrintS8DetailHeader()
  {
    _diag->printS8DetailHeader();
    CPPUNIT_ASSERT_EQUAL(string("--------------- BRANDED FARES 898 DETAILED INFO ----------\n"),
                         _diag->str());
  }

  void testPrintSeparator()
  {
    _diag->printSeparator();
    CPPUNIT_ASSERT_EQUAL(string("-----------------------------------------------------------\n"),
                         _diag->str());
  }

  void testPrintRuleExecutionContent()
  {
    _diag->printRuleExecutionContent(_ruleExecution, _mResponse, *_marketIDFareMarketMap);
    CPPUNIT_ASSERT_EQUAL(string("   BA   132              BGI            PASS              \n"),
                         _diag->str());
  }

  void testPrintDetailRuleExecutionContent()
  {
    _diag->printDetailRuleExecutionContent(_ruleExecution, _mResponse, *_marketIDFareMarketMap);
    CPPUNIT_ASSERT_EQUAL(string(" PROGRAM  : BGI        CARRIER : BA \n"
                                " PROGRAMID: 132            \n"
                                " EFF DATE : 2013-11-04    TRVL DATE START : 2013-11-04\n"
                                " DISC DATE: 2016-02-29    TRVL DATE STOP  : 2016-02-29\n"
                                " PAX TYPE : \n"
                                " MARKET   :  ORGN LOC: LON       LOC TYPE : M \n"
                                "             DEST LOC: USNY      LOC TYPE : S \n"
                                "             DIRECTION:          GLOBAL DIR : \n\n"
                                " ACC CODE : GSO01                \n"
                                " PCC      : 80K2 \n"
                                "**SECURITY: **\n"
                                "  CARRIERGDS  : 1S                   PASS              \n\n**\n"
                                " STATUS   : PASS              \n"),
                         _diag->str());
  }

  void testPrintS8BrandingSecurity()
  {
    _diag->printS8BrandingSecurity(_s8BrandingSecurity);
    CPPUNIT_ASSERT_EQUAL(string("  CARRIERGDS  : 1S                   PASS              \n"),
                         _diag->str());
  }

  void testPrintMarketRuleDirectionOI()
  {
    _diag->printMarketRule(_marketRule, _firstMarket);
    CPPUNIT_ASSERT_EQUAL(string(" ORGN LOC:      LON  LOC TYPE :  M\n"
                                "             DEST LOC:     USNY  LOC TYPE :  S\n"
                                "             DIRECTION:          GLOBAL DIR : \n"),
                         _diag->str());
  }

  void testPrintMarketRuleDirectionOT()
  {
    _marketRule->direction() = "OT";
    _diag->printMarketRule(_marketRule, _firstMarket);
    CPPUNIT_ASSERT_EQUAL(string(" ORGN LOC:      LON  LOC TYPE :  M\n"
                                "             DEST LOC:     USNY  LOC TYPE :  S\n"
                                "             DIRECTION: OUTBOUND GLOBAL DIR : \n"),
                         _diag->str());
  }

  void testPrintMarketRuleDirectionIN()
  {
    _marketRule->direction() = "IN";
    _diag->printMarketRule(_marketRule, _firstMarket);
    CPPUNIT_ASSERT_EQUAL(string(" ORGN LOC:      LON  LOC TYPE :  M\n"
                                "             DEST LOC:     USNY  LOC TYPE :  S\n"
                                "             DIRECTION:  INBOUND GLOBAL DIR : \n"),
                         _diag->str());
  }

  void testPrintMarketRuleGlobalDirection()
  {
    _marketRule->direction() = "IN";
    _marketRule->globalDirection() = GlobalDirection::AT;
    _diag->printMarketRule(_marketRule, _firstMarket);
    CPPUNIT_ASSERT_EQUAL(string(" ORGN LOC:      LON  LOC TYPE :  M\n"
                                "             DEST LOC:     USNY  LOC TYPE :  S\n"
                                "             DIRECTION:  INBOUND GLOBAL DIR :  AT\n"),
                         _diag->str());
  }

  void testIsDdInfo()
  {
    _trx->diagnostic().diagParamMap()["DD"] = "INFO";
    CPPUNIT_ASSERT(_diag->isDdInfo());
  }

  void testShouldDisplayCarrierMatched()
  {
    _trx->diagnostic().diagParamMap()["CX"] = "VA";
    PatternInfo* patternInfoGoverningCarrier = _memHandle.create<PatternInfo>();
    patternInfoGoverningCarrier->_patternValue = "VA";
    patternInfoGoverningCarrier->_status = PASS_S8;
    std::vector<PatternInfo*> cxrVec;
    cxrVec.push_back(patternInfoGoverningCarrier);
    CPPUNIT_ASSERT(_diag->shouldDisplay(cxrVec));
  }

  void testShouldDisplayCarrierNotMatched()
  {
    _trx->diagnostic().diagParamMap()["CX"] = "VA";
    PatternInfo* patternInfoGoverningCarrier = _memHandle.create<PatternInfo>();
    patternInfoGoverningCarrier->_patternValue = "AA";
    patternInfoGoverningCarrier->_status = PASS_S8;
    std::vector<PatternInfo*> cxrVec;
    cxrVec.push_back(patternInfoGoverningCarrier);
    CPPUNIT_ASSERT(!_diag->shouldDisplay(cxrVec));
  }

  void testShouldDisplayProgramCode()
  {
    _trx->diagnostic().diagParamMap()["PC"] = "PWH";
    ProgramCode programCode = "PWH";
    CPPUNIT_ASSERT(_diag->shouldDisplay(programCode));
  }

  void testMatchFareMarketMatched()
  {
    _trx->diagnostic().diagParamMap()["FM"] = "LHRNYC";
    CPPUNIT_ASSERT(_diag->matchFareMarket(_mResponse));
  }

  void testMatchFareMarketNotMatched()
  {
    _trx->diagnostic().diagParamMap()["FM"] = "JNBLON";
    CPPUNIT_ASSERT(!_diag->matchFareMarket(_mResponse));
  }

  void testShouldDisplayPassedMatched()
  {
    _trx->diagnostic().diagParamMap()["DD"] = "PASSED";
    CPPUNIT_ASSERT(_diag->shouldDisplay(PASS_S8));
  }

  void testShouldDisplayPassedNotMatched()
  {
    _trx->diagnostic().diagParamMap()["DD"] = "PASSED";
    CPPUNIT_ASSERT(!_diag->shouldDisplay(FAIL_S8_PCC));
  }

  void testPrintDataNotFound()
  {
    _diag->printDataNotFound();
    CPPUNIT_ASSERT_EQUAL(string("BRANDED DATA NOT FOUND\n"), _diag->str());
  }

  void testPrintBrandedDataError()
  {
    _diag->printBrandedDataError();
    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES 898 ANALYSIS ***************\n"
                                "BRANDED DATA NOT FOUND\n"),
                         _diag->str());
  }

  void testMatchGlobalDirectionPass()
  {
    CPPUNIT_ASSERT(_diag->matchGlobalDirection(_mCriteria, _marketRule));
  }
  void testMatchGlobalDirectionFail()
  {
    _marketRule->globalDirection() = GlobalDirection::EH;
    CPPUNIT_ASSERT(!_diag->matchGlobalDirection(_mCriteria, _marketRule));
  }

  void testMatchMarketDirectionOI()
  {
    CPPUNIT_ASSERT(!_diag->matchMarket(_mCriteria, _marketRule));
  }

  void testMatchMarketDirectionOT()
  {
    _marketRule->direction() = "OT";
    CPPUNIT_ASSERT(!_diag->matchMarket(_mCriteria, _marketRule));
  }

  void testMatchMarketDirectionIN()
  {
    _marketRule->direction() = "IN";
    CPPUNIT_ASSERT(!_diag->matchMarket(_mCriteria, _marketRule));
  }

  void testMatchMarketDirectionBlank()
  {
    _marketRule->direction() = "";
    CPPUNIT_ASSERT(!_diag->matchMarket(_mCriteria, _marketRule));
  }

  void testMatchAirportDirectionOI()
  {
    _marketRule->originLoc() = "LHR";
    _marketRule->originLocType() = 'M';
    _marketRule->destinationLoc() = "NYC";
    _marketRule->destinationLocType() = 'M';

    CPPUNIT_ASSERT(_diag->matchAirport(_mCriteria, _marketRule));
  }

  void testMatchAirportDirectionOT()
  {
    _marketRule->direction() = "OT";
    CPPUNIT_ASSERT(!_diag->matchAirport(_mCriteria, _marketRule));
  }

  void testMatchAirportDirectionIN()
  {
    _marketRule->direction() = "IN";
    CPPUNIT_ASSERT(!_diag->matchAirport(_mCriteria, _marketRule));
  }

  void testSetStatusForMarketFailureFailGeography()
  {
    std::string statusStr;
    _diag->setStatusForMarketFailure(_mCriteria, _marketRule, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL GEOGRAPHY"), statusStr);
  }

  void testSetStatusForMarketFailureFailDirection()
  {
    std::string statusStr;
    _mCriteria->arrivalAirportCode() = "SYD";
    _mCriteria->departureAirportCode() = "LON";
    _marketRule->direction() = "IN";
    _marketRule->destinationLoc() = "SYD";
    _marketRule->destinationLocType() = 'M';

    _diag->setStatusForMarketFailure(_mCriteria, _marketRule, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL DIRECTION"), statusStr);
  }

  void testSetStatusForMarketFailureFailGlobalDirection()
  {
    std::string statusStr;
    _marketRule->globalDirection() = GlobalDirection::EH;
    _diag->setStatusForMarketFailure(_mCriteria, _marketRule, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL GLOBAL DIRECTION"), statusStr);
  }

  void testPrintDiagnostic()
  {
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    _diag->printDiagnostic(marketIDFareMarketMap);

    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES 898 ANALYSIS ***************\n"
                                "------------ FARE MARKET : LHR - NYC   CXR - BA ----------\n"
                                "V CXR   PROGRAMID      PROGRAM     PAX  STATUS\n"
                                "   BA   132              BGI            PASS              \n"),
                         _diag->str());
  }

  void testPrintDiagnosticDetail()
  {
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    _trx->diagnostic().diagParamMap()["DD"] = "INFO";
    _diag->printDiagnostic(marketIDFareMarketMap);

    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES 898 ANALYSIS ***************\n"
                                "------------ FARE MARKET : LHR - NYC   CXR - BA ----------\n"
                                "--------------- BRANDED FARES 898 DETAILED INFO ----------\n"
                                " PROGRAM  : BGI        CARRIER : BA \n"
                                " PROGRAMID: 132            \n"
                                " EFF DATE : 2013-11-04    TRVL DATE START : 2013-11-04\n"
                                " DISC DATE: 2016-02-29    TRVL DATE STOP  : 2016-02-29\n"
                                " PAX TYPE : \n"
                                " MARKET   :  ORGN LOC: LON       LOC TYPE : M \n"
                                "             DEST LOC: USNY      LOC TYPE : S \n"
                                "             DIRECTION:          GLOBAL DIR : \n\n"
                                " ACC CODE : GSO01                \n"
                                " PCC      : 80K2 \n"
                                "**SECURITY: **\n"
                                "  CARRIERGDS  : 1S                   PASS              \n\n**\n"
                                " STATUS   : PASS              \n"
                                "-----------------------------------------------------------\n"),
                         _diag->str());
  }

  void testDisplayVendorATP()
  {
    _diag->displayVendor("ATP");
    CPPUNIT_ASSERT_EQUAL(string("A  "), _diag->str());
  }

  void testDisplayVendorATPDetail()
  {
    _diag->displayVendor("ATP", true);
    CPPUNIT_ASSERT_EQUAL(string(" A"), _diag->str());
  }

  void testDisplayVendorMMGR()
  {
    _diag->displayVendor("MMGR");
    CPPUNIT_ASSERT_EQUAL(string("M  "), _diag->str());
  }

  void testDisplayVendorMMGRDetail()
  {
    _diag->displayVendor("MMGR", true);
    CPPUNIT_ASSERT_EQUAL(string(" M"), _diag->str());
  }

  void testDisplayVendorUSOC()
  {
    _diag->displayVendor("USOC");
    CPPUNIT_ASSERT_EQUAL(string("C  "), _diag->str());
  }

  void testDisplayVendorUSOCDetail()
  {
    _diag->displayVendor("USOC", true);
    CPPUNIT_ASSERT_EQUAL(string(" C"), _diag->str());
  }

  void testDisplayVendorSITA()
  {
    _diag->displayVendor("SITA");
    CPPUNIT_ASSERT_EQUAL(string("   "), _diag->str());
  }

  void testDisplayVendorSITADetail()
  {
    _diag->displayVendor("SITA", true);
    CPPUNIT_ASSERT_EQUAL(string("  "), _diag->str());
  }

  void testGetStatusStrFailPsgrType()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_PASSENGER_TYPE, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL PSGR TYPE"), statusStr);
  }

  void testGetStatusStrFailAccCode()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_ACCOUNT_CODE, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL ACC CODE"), statusStr);
  }

  void testGetStatusStrFailMarket()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_MARKET, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL MARKET"), statusStr);
  }

  void testGetStatusStrFailPcc()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_PCC, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL PCC"), statusStr);
  }

  void testGetStatusStrFailCarrierGds()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_CARRIER_GDS, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL CARRIER GDS"), statusStr);
  }

  void testGetStatusStrFailAgentLoc()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_AGENT_LOCATION, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL AGENT LOC"), statusStr);
  }

  void testGetStatusStrFailDeptCode()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_DEPARTMENT_CODE, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL DEPT CODE"), statusStr);
  }

  void testGetStatusStrFailOfficeDesig()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_OFFICE_DESIGNATOR, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL OFFICE DESIG"), statusStr);
  }

  void testGetStatusStrFailSecurity()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_SECURITY, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL SECURITY"), statusStr);
  }

  void testGetStatusStrFailViewBook()
  {
    std::string statusStr;
    _diag->getStatusStr(FAIL_S8_VIEW_BOOK_TICKET, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL VIEW BOOK"), statusStr);
  }

  void testSetStatusPassSeclected()
  {
    std::string statusStr;
    _ruleExecution->ruleID() = "PROGRAM2";
    _diag->setStatusPassSeclected(_ruleExecution, _mResponse, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("PASS SELECTED"), statusStr);
  }

  void testSetStatusStr()
  {
    std::string statusStr;
    _ruleExecution->status() = FAIL_S8_MARKET;
    _diag->setStatusStr(_ruleExecution, _mResponse, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL GEOGRAPHY"), statusStr);
  }

  void testMatchGeoAirportFail()
  {
    CPPUNIT_ASSERT(!_diag->matchGeoAirport(_mCriteria, _marketRule));
  }

  void testMatchGeoAirportPass()
  {
    _mCriteria->arrivalAirportCode() = "SYD";
    _mCriteria->departureAirportCode() = "LON";

    _marketRule->destinationLoc() = "SYD";
    _marketRule->destinationLocType() = 'M';

    CPPUNIT_ASSERT(_diag->matchGeoAirport(_mCriteria, _marketRule));
  }

  void testPrintBlankLine()
  {
    _diag->printBlankLine();
    CPPUNIT_ASSERT_EQUAL(string("                                                           \n"),
                         _diag->str());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag898CollectorTest);
} // tse
