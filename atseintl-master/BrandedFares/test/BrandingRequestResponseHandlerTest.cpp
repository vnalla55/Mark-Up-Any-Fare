//-------------------------------------------------------------------
//
//  File:        BrandingRequestResponseHandlerTest.cpp
//  Created:     April 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareDisplayTrx.h"
#include "Common/TrxUtil.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "BrandedFares/BrandingService.h"
#include "BrandedFares/BrandingRequestResponseHandler.h"
#include "BrandedFares/S8BrandingResponseParser.h"
#include "BrandedFares/BrandingResponse.h"
#include "BrandedFares/BrandingCriteria.h"
#include "Diagnostic/Diag888Collector.h"
#include "Diagnostic/Diag890Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/BrandedFare.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

#include <regex>

using namespace std;
namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

  const std::vector<BrandedFare*>&
  getBrandedFare(const VendorCode& vendor, const CarrierCode& carrier)

  {
    std::vector<BrandedFare*>* vectBrandedFare = _memHandle.create<std::vector<BrandedFare*> >();
    BrandedFare* brandedFare = _memHandle.create<BrandedFare>();
    brandedFare->seqNo() = 1234;
    brandedFare->segCount() = 2;
    brandedFare->vendor() = "ATP";
    brandedFare->psgType() = "ADT";
    brandedFare->carrier() = "VA";
    brandedFare->programCode() = "US";
    brandedFare->source() = 'T';
    brandedFare->tvlFirstYear() = 13;
    brandedFare->tvlFirstMonth() = 10;
    brandedFare->tvlFirstDay() = 29;
    brandedFare->publicPrivateInd() = 'P';
    brandedFare->svcFeesAccountCodeTblItemNo() = 3882;
    brandedFare->svcFeesSecurityTblItemNo() = 9548;
    brandedFare->directionality() = '3';
    brandedFare->loc1ZoneTblItemNo() = "123456";
    brandedFare->loc2ZoneTblItemNo() = "234567";
    brandedFare->effDate() = DateTime(2013, 5, 1);
    brandedFare->discDate() = DateTime(2013, 11, 9);
    brandedFare->locKey1().loc() = LOC_NYC;
    brandedFare->locKey1().locType() = LOCTYPE_CITY;
    strToGlobalDirection(brandedFare->globalInd(), "AP");
    brandedFare->oneMatrix() = 'Z';
    brandedFare->programText() = "CPPUNIT TEST FOR BRANDED FARES";
    brandedFare->svcFeesFeatureTblItemNo() = 19876;
    brandedFare->taxTextTblItemNo() = 555555;
    BrandedFareSeg* seg1(new BrandedFareSeg);
    brandedFare->segments().push_back(seg1);
    brandedFare->segments()[0]->segNo() = 10;
    brandedFare->segments()[0]->tier() = 3;
    brandedFare->segments()[0]->brandName() = "CUCUMBER";
    brandedFare->segments()[0]->svcFeesFareIdTblItemNo() = 4343;
    brandedFare->segments()[0]->taxTextTblItemNo() = 12;

    BrandedFareSeg* seg2(new BrandedFareSeg);
    brandedFare->segments().push_back(seg2);
    brandedFare->segments()[1]->segNo() = 20;
    brandedFare->segments()[1]->tier() = 5;
    brandedFare->segments()[1]->brandName() = "PRINTER";
    brandedFare->segments()[1]->svcFeesFareIdTblItemNo() = 4666;
    brandedFare->segments()[1]->taxTextTblItemNo() = 12;

    vectBrandedFare->push_back(brandedFare);
    return *vectBrandedFare;
  }
};
}

class BrandingRequestResponseHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandingRequestResponseHandlerTest);
  CPPUNIT_TEST(testFareDisplayXmlRequestPass);
  CPPUNIT_TEST(testFareDisplayXmlRequestPassMultiAccCorpId);

  CPPUNIT_TEST(testNotFareDisplayTrxFail);

  CPPUNIT_TEST(testCreateDiag_False);
  CPPUNIT_TEST(testCreateDiag890_True);
  CPPUNIT_TEST(testCreateDiag990_False);

  CPPUNIT_TEST(testCreateDiag898_False);
  CPPUNIT_TEST(testCreateDiag898_True);
  CPPUNIT_TEST(testIsError_False);
  CPPUNIT_TEST(testIsError_True);
  CPPUNIT_TEST(testBrandedFaresDiagnostic888BrandCodeEmpty);
  CPPUNIT_TEST(testBrandedFaresDiagnostic888BrandCodeNotEmpty);
  CPPUNIT_TEST(testBrandedFaresDiagnostic888BrandCodeEmptyDetailInfo);
  CPPUNIT_TEST(testBrandedFaresDiagnostic888BrandCodeNotEmptyDetailInfo);

  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _options = _memHandle.create<PricingOptions>();
    _request = _memHandle.create<FareDisplayRequest>();
    _trx->setRequest(_request);
    _paxTypeADT = _memHandle.create<PaxType>();
    _paxTypeADT->paxType() = ADULT;
    _locSYD = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    _locMEL = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEL.xml");
    _locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _brrh = _memHandle.create<BrandingRequestResponseHandler>(*_trx);
    _brrh->setClientId(BR_CLIENT_PBB);
    _memHandle.create<MyDataHandle>();
    sfo = getLoc("SFO");
    dfw = getLoc("DFW");
    _fm1 = createFareMarket(sfo, dfw, "AA");
    _brandedFare = _memHandle.create<BrandedFare>();
  }

  const Loc* getLoc(const LocCode& locCode)
  {
    return TestLocFactory::create("/vobs/atseintl/test/testdata/data/Loc" + locCode + ".xml");
  }

  void tearDown() { _memHandle.clear(); }

  void createItin(const Loc* stop1, const Loc* stop2)
  {
    AirSeg* air1 = createSegment();
    air1->pnrSegment() = 1;
    air1->segmentOrder() = 1;
    air1->origin() = stop1;
    air1->origAirport() = stop1->loc();
    air1->destination() = stop2;
    air1->destAirport() = stop2->loc();

    _trx->travelSeg().push_back(air1);

    TrxUtil::buildFareMarket(*_trx, _trx->travelSeg());

    _pricingUnit = _memHandle.create<PricingUnit>();
    _farePath->pricingUnit().push_back(_pricingUnit);

    _fareUsage = _memHandle.create<FareUsage>();
    _pricingUnit->fareUsage().push_back(_fareUsage);

    _fareUsage->travelSeg().push_back(air1);
  }

  AirSeg* createSegment(const Loc* origin = NULL, const Loc* destination = NULL)
  {
    if (!_farePath)
    {
      _farePath = _memHandle.create<FarePath>();
      _farePath->paxType() = _paxTypeADT;
    }
    if (!_itin)
      _itin = _memHandle.create<Itin>();
    _farePath->itin() = _itin;

    AirSeg* air = _memHandle.create<AirSeg>();
    air->origin() = origin;
    air->destination() = destination;

    _itin->travelSeg().push_back(air);

    return air;
  }

  Agent* createAgent()
  {
    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = _locDFW;
    agent->agentCity() = "DFW";
    agent->tvlAgencyPCC() = "HDQ";
    agent->mainTvlAgencyPCC() = "HDQ";
    agent->tvlAgencyIATA() = "XYZ";
    agent->homeAgencyIATA() = "XYZ";
    agent->agentFunctions() = "XYZ";
    agent->agentDuty() = "XYZ";
    agent->airlineDept() = "XYZ";
    agent->cxrCode() = "AA";
    agent->currencyCodeAgent() = "USD";
    agent->coHostID() = 9;
    agent->agentCommissionType() = "PERCENT";
    agent->agentCommissionAmount() = 10;

    return agent;
  }

  FareMarket*
  createFareMarket(const Loc* origin, const Loc* destination, CarrierCode goveringCarrier)
  {
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    fm1->origin() = origin;
    fm1->destination() = destination;
    fm1->governingCarrier() = goveringCarrier;
    _seg1 = _memHandle.create<AirSeg>();
    _seg1->origAirport() = fm1->origin()->loc();
    _seg1->destAirport() = fm1->destination()->loc();
    fm1->travelSeg().push_back(_seg1);
    return fm1;
  }

  void getXML(std::string& xmlRequest, DateTime travelDate)
  {
    xmlRequest = R"(<\?xml version="1\.0" encoding="UTF-8"\?><GetAirlineBrandsRQ )"
                 R"(xmlns="http://stl\.sabre\.com/Merchandising/v1" version="1\.0\.0" )"
                 R"(xmlns:ANCS="http://stl\.sabre\.com/Merchandising/v0" )"
                 R"(xmlns:xsi="http://www\.w3\.org/2001/XMLSchema-instance" )"
                 R"(xsi:schemaLocation="http://stl\.sabre\.com/Merchandising/v1 )"
                 R"(\.\./AirlineBranding_1_0_0\.xsd"><BrandingRequest><RequestSource )"
                 R"(dutyCode="XYZ" functionCode="X" geoLocation="DFW" clientID="FQ" )"
                 R"(action="FQ" iataNumber="XYZ" departmentCode="XYZ" pseudoCityCode="HDQ" )"
                 R"(requestType="D" requestingCarrierGDS="1B"/><XRA MID="\w+" )"
                 R"(CID="cid\.id"/><BrandingCriteria><MarketRequest marketID="1">)"
                 R"(<MarketCriteria direction="OT"><DepartureDate>)";
    xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
    xmlRequest += R"(</DepartureDate><DepartureAirportCode>SYD</DepartureAirportCode>)"
                  R"(<ArrivalAirportCode>MEL</ArrivalAirportCode><PassengerTypes><Type>ADT</Type>)"
                  R"(</PassengerTypes></MarketCriteria><ANCS:CarrierList><ANCS:Carrier></ANCS:Carrier>)"
                  R"(</ANCS:CarrierList></MarketRequest><AccountCodeList><AccountCode>JKL</)"
                  R"(AccountCode><AccountCode>GHI</AccountCode></AccountCodeList><SalesDate>.+)";
  }

  void getXMLMultiAccCorpId(std::string& xmlRequest, DateTime travelDate)
  {
    xmlRequest = R"(<\?xml version="1\.0" encoding="UTF-8"\?><GetAirlineBrandsRQ )"
                 R"(xmlns="http://stl\.sabre\.com/Merchandising/v1" version="1\.0\.0" )"
                 R"(xmlns:ANCS="http://stl\.sabre\.com/Merchandising/v0" )"
                 R"(xmlns:xsi="http://www\.w3\.org/2001/XMLSchema-instance" )"
                 R"(xsi:schemaLocation="http://stl\.sabre\.com/Merchandising/v1 )"
                 R"(\.\./AirlineBranding_1_0_0\.xsd"><BrandingRequest><RequestSource )"
                 R"(dutyCode="XYZ" functionCode="X" geoLocation="DFW" clientID="FQ" )"
                 R"(action="FQ" iataNumber="XYZ" departmentCode="XYZ" pseudoCityCode="HDQ" )"
                 R"(requestType="D" requestingCarrierGDS="1B"/><XRA MID="\w+" )"
                 R"(CID="cid\.id"/><BrandingCriteria><MarketRequest marketID="1">)"
                 R"(<MarketCriteria direction="OT"><DepartureDate>)";
    xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
    xmlRequest += R"(</DepartureDate><DepartureAirportCode>SYD</DepartureAirportCode>)"
                  R"(<ArrivalAirportCode>MEL</ArrivalAirportCode><PassengerTypes><Type>ADT</Type>)"
                  R"(</PassengerTypes></MarketCriteria><ANCS:CarrierList><ANCS:Carrier></ANCS:Carrier>)"
                  R"(</ANCS:CarrierList></MarketRequest><AccountCodeList><AccountCode>TST01)"
                  R"(</AccountCode><AccountCode>TST02</AccountCode><AccountCode>TST03</AccountCode>)"
                  R"(<AccountCode>A</AccountCode><AccountCode>B</AccountCode></AccountCodeList>)"
                  R"(<SalesDate>.+)";
  }

  void testFareDisplayXmlRequestPass()
  {
    createItin(_locSYD, _locMEL);
    _trx->getRequest()->ticketingAgent() = createAgent();

    _trx->assignXrayJsonMessage(xray::JsonMessagePtr(new xray::JsonMessage("mid", "cid")));
    _trx->getXrayJsonMessage()->setId("id");

    _trx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    DateTime travelDate = DateTime::localTime();
    _trx->setTravelDate(travelDate);
    _trx->paxType().push_back(_paxTypeADT);
    Billing billing;
    _trx->billing() = &billing;
    _trx->billing()->partitionID() = RuleConst::SABRE1B;
    _trx->getRequest()->diagnosticNumber() = Diagnostic898;
    _trx->getRequest()->isMultiAccCorpId() = false;
    _trx->getRequest()->accountCode() = "GHI";
    _trx->getRequest()->corporateID() = "JKL";

    BrandingService bs(*_trx);
    bs.getOnlyXmlData() = true;
    bool rc = bs.getBrandedFares();

    CPPUNIT_ASSERT(rc == true);

    std::string xmlRequest;
    getXML(xmlRequest, travelDate);
    CPPUNIT_ASSERT_EQUAL(2, (int)bs.brandingCriteria()->accountCodes().size());
    CPPUNIT_ASSERT(std::regex_match(bs.xmlData(), std::regex(xmlRequest)));
  }

  void testFareDisplayXmlRequestPassMultiAccCorpId()
  {
    createItin(_locSYD, _locMEL);
    _trx->getRequest()->ticketingAgent() = createAgent();

    _trx->assignXrayJsonMessage(xray::JsonMessagePtr(new xray::JsonMessage("mid", "cid")));
    _trx->getXrayJsonMessage()->setId("id");

    _trx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    DateTime travelDate = DateTime::localTime();
    _trx->setTravelDate(travelDate);
    _trx->paxType().push_back(_paxTypeADT);
    Billing billing;
    _trx->billing() = &billing;
    _trx->billing()->partitionID() = RuleConst::SABRE1B;
    _trx->getRequest()->diagnosticNumber() = Diagnostic898;
    _trx->getRequest()->isMultiAccCorpId() = true;
    _trx->getRequest()->corpIdVec().push_back("TST01");
    _trx->getRequest()->corpIdVec().push_back("TST02");
    _trx->getRequest()->corpIdVec().push_back("TST03");
    _trx->getRequest()->accCodeVec().push_back("A");
    _trx->getRequest()->accCodeVec().push_back("B");

    BrandingService bs(*_trx);
    bs.getOnlyXmlData() = true;
    bool rc = bs.getBrandedFares();

    CPPUNIT_ASSERT(rc == true);

    std::string xmlRequest;
    getXMLMultiAccCorpId(xmlRequest, travelDate);
    CPPUNIT_ASSERT_EQUAL(5, (int)bs.brandingCriteria()->accountCodes().size());
    CPPUNIT_ASSERT(std::regex_match(bs.xmlData(), std::regex(xmlRequest)));
  }

  void testNotFareDisplayTrxFail()
  {
    BrandingService bs(*_trx);
    bool rc = bs.getBrandedFares();
    CPPUNIT_ASSERT(rc == false);
  }

  void createDiag(DiagnosticTypes diagType = Diagnostic890)
  {
    _trx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      _trx->diagnostic().activate();
    }
  }

  void testCreateDiag_False()
  {
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT(!_brrh->createDiag());
  }

  void testCreateDiag890_True()
  {
    createDiag();
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    bool status = _brrh->createDiag();
    CPPUNIT_ASSERT(status);
  }

  void testCreateDiag990_False()
  {
    DiagnosticTypes diagType = Diagnostic990;
    createDiag(diagType);
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT(!_brrh->createDiag());
  }

  void testCreateDiag898_False() { CPPUNIT_ASSERT(!_brrh->createDiag898()); }

  void testCreateDiag898_True()
  {
    createDiag(Diagnostic898);
    bool status = _brrh->createDiag898();
    CPPUNIT_ASSERT(status);
  }

  void testIsError_False()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    CPPUNIT_ASSERT(!_brrh->isError(s8BrandingResponseParser));
  }

  void testIsError_True()
  {
    std::string xmlResponse =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><GetAirlineBrandsRS "
        "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
        "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
        "xmlns:ns3=\"http://opentravel.org/common/v02\" "
        "xmlns:ns4=\"http://stl.sabre.com/Merchandising/v0\" "
        "xmlns:ns5=\"http://services.sabre.com/STL_Payload/v02_01\"><BrandingResponse><Message "
        "messageCode=\"Error\" failCode=\"2015\" messageText=\"Invalid Office Designator - "
        "80K2WC\"/></BrandingResponse></GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.parse(xmlResponse);
    CPPUNIT_ASSERT(_brrh->isError(s8BrandingResponseParser));
  }

  void testBrandedFaresDiagnostic888BrandCodeEmpty()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic888;
    _trx->fareMarket().push_back(_fm1);
    _brrh->brandedFaresDiagnostic888();

    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                                "------------ FARE MARKET : SFO - DFW   CXR - AA -------------\n"
                                "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"
                                "A  VA   1234     ADT  US            2 \n"),
                         _brrh->_diag888->str());
  }

  void testBrandedFaresDiagnostic888BrandCodeNotEmpty()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic888;
    _trx->diagnostic().diagParamMap()["BC"] = "APP";
    _trx->fareMarket().push_back(_fm1);
    _brrh->brandedFaresDiagnostic888();

    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                                "------------ FARE MARKET : SFO - DFW   CXR - AA -------------\n"
                                "V CXR   SEQ      PAX  PROGRAM      BRANDS\n"),
                         _brrh->_diag888->str());
  }

  void testBrandedFaresDiagnostic888BrandCodeEmptyDetailInfo()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic888;
    _trx->diagnostic().diagParamMap()["DD"] = "INFO";
    _trx->fareMarket().push_back(_fm1);
    _brrh->brandedFaresDiagnostic888();

    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                                "------------ FARE MARKET : SFO - DFW   CXR - AA -------------\n"
                                "--------------- BRANDED FARES S8 DETAILED INFO ---------------\n"
                                " CARRIER: VA             VENDOR : ATP      SOURCE : T\n"
                                " SEQ NBR : 1234   \n"
                                " S8 EFF DATE : 2013-05-01      TVL DATE START :   13-10-29\n"
                                " S8 DISC DATE: 2013-11-09      TVL DATE STOP  : \n"
                                "  \n"
                                "     PAX TYPE  : ADT\n"
                                " PRIVATE IND   : P \n"
                                " ACC CODE T172 : 3882   \n"
                                " SECURITY T183 : 9548   \n"
                                "DIRECTIONALITY : 3 \n"
                                "     LOC1 TYPE : C    LOC1 : NYC       LOC1 ZONE : 123456  \n"
                                "     LOC2 TYPE :      LOC2 :           LOC2 ZONE : 234567  \n"
                                " GLOBAL IND : AP          MATRIX : Z\n"
                                "  PROGRAM CODE : US         \n"
                                "  PROGRAM TEXT : CPPUNIT TEST FOR BRANDED FARES\n"
                                "ANCILLARY T166 : 19876  \n"
                                "TEXT T196 : 555555 \n"
                                "-------------------------------------------------------------\n"
                                " BRAND SEGMENTS : 2  \n"
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
                         _brrh->_diag888->str());
  }

  void testBrandedFaresDiagnostic888BrandCodeNotEmptyDetailInfo()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic888;
    _trx->diagnostic().diagParamMap()["BC"] = "APP";
    _trx->diagnostic().diagParamMap()["DD"] = "INFO";
    _trx->fareMarket().push_back(_fm1);
    _brrh->brandedFaresDiagnostic888();

    CPPUNIT_ASSERT_EQUAL(string("*************** BRANDED FARES - S8 ANALYSIS ******************\n"
                                "------------ FARE MARKET : SFO - DFW   CXR - AA -------------\n"),
                         _brrh->_diag888->str());
  }

protected:
  const Loc* _locSYD;
  const Loc* _locMEL;
  const Loc* _locDFW;

  PricingOptions* _options;
  FareDisplayRequest* _request;
  PricingTrx* _trx;
  FarePath* _farePath;
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;
  std::vector<PaxType*> _paxTypes;
  PaxType* _paxTypeADT;
  Itin* _itin;
  BrandingRequestResponseHandler* _brrh;
  TestMemHandle _memHandle;
  FareMarket* _fm1;
  const tse::Loc* sfo;
  const tse::Loc* dfw;
  AirSeg* _seg1;
  BrandedFare* _brandedFare;
};
CPPUNIT_TEST_SUITE_REGISTRATION(BrandingRequestResponseHandlerTest);
}
