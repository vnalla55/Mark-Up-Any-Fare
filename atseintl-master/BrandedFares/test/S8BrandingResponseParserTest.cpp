//-------------------------------------------------------------------
//
//  File:        S8BrandingResponseParserTest.cpp
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

#include "test/include/CppUnitHelperMacros.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "test/include/TestMemHandle.h"
#include "BrandedFares/S8BrandingResponseParser.h"
#include "BrandedFares/RequestSource.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/BSDiagnostics.h"
#include "BrandedFares/RuleExecution.h"
#include "BrandedFares/BrandingResponse.h"
#include "BrandedFares/S8BrandingSecurity.h"
#include "BrandedFares/RuleExecution.h"
#include "BrandedFares/MarketRule.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandInfo.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;
namespace tse
{
FALLBACKVALUE_DECL(fallbackUseOnlyFirstProgram);

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

  const std::vector<SvcFeesFareIdInfo*>&
  getSvcFeesFareIds(const VendorCode& vendor, long long itemNo)

  {
    std::vector<SvcFeesFareIdInfo*>* vectSvcFeesFareIdInfo = _memHandle.create<std::vector<SvcFeesFareIdInfo*> >();
    SvcFeesFareIdInfo* svcFeesFareIdInfo = _memHandle.create<SvcFeesFareIdInfo>();
    svcFeesFareIdInfo->vendor() = Vendor::ATPCO;
    svcFeesFareIdInfo->itemNo() = 12441;
    svcFeesFareIdInfo->seqNo() = 1001;
    svcFeesFareIdInfo->ruleTariff() = -1;
    svcFeesFareIdInfo->routing() = 99999;
    svcFeesFareIdInfo->bookingCode1() = "U";
    vectSvcFeesFareIdInfo->push_back(svcFeesFareIdInfo);
    return *vectSvcFeesFareIdInfo;
  }
};
}

class S8BrandingResponseParserTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(S8BrandingResponseParserTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testPricingTrxBrandProgramVec);
  CPPUNIT_TEST(testPricingTrxBrandProgramVecOneProgramPerFeareMarket);
  CPPUNIT_TEST(testErrorResponseParse);
  CPPUNIT_TEST(testDiagnosticResponseParse);
  CPPUNIT_TEST(testDiagnosticResponseRuleExecutionParse);
  CPPUNIT_TEST(testProcessPassSecurity);
  CPPUNIT_TEST(testProcessFailSecurity);
  CPPUNIT_TEST(testCheckSecurityFailure);
  CPPUNIT_TEST(testAddPatternInfo);
  CPPUNIT_TEST(testSetPatternStatus);
  CPPUNIT_TEST(testCheckFailure);
  CPPUNIT_TEST(testCheckPatternFailure);
  CPPUNIT_TEST(testAddMarketRuleInfo);
  CPPUNIT_TEST(testCheckMarketFailure);
  CPPUNIT_TEST(testSetPatternStatusFailPassengerType);
  CPPUNIT_TEST(testSetPatternStatusFailMarket);
  CPPUNIT_TEST(testSetPatternStatusFailPCC);
  CPPUNIT_TEST(testSetPatternStatusFailCarrier);
  CPPUNIT_TEST(testSetPatternStatusFailSalesDate);
  CPPUNIT_TEST(testSetPatternStatusFailTravelDate);
  CPPUNIT_TEST(testGetSecurityStatusCarrierGds);
  CPPUNIT_TEST(testGetSecurityStatusDepartmentCode);
  CPPUNIT_TEST(testGetSecurityStatusOfficeDesignator);
  CPPUNIT_TEST(testGetSecurityStatusMarketLocation);
  CPPUNIT_TEST(testGetSecurityStatusIataNumber);
  CPPUNIT_TEST(testGetSecurityStatusViewBookTicket);
  CPPUNIT_TEST(testSetStatusFailPassengerType);
  CPPUNIT_TEST(testSetStatusFailAccountCode);
  CPPUNIT_TEST(testSetStatusFailMarket);
  CPPUNIT_TEST(testSetStatusFailPCC);
  CPPUNIT_TEST(testSetStatusFailCarrier);
  CPPUNIT_TEST(testSetStatusFailSalesDate);
  CPPUNIT_TEST(testSetStatusFailTravelDate);
  CPPUNIT_TEST(testGetStatusStrFailPassengerType);
  CPPUNIT_TEST(testGetStatusStrFailAccCode);
  CPPUNIT_TEST(testGetStatusStrFailMarket);
  CPPUNIT_TEST(testGetStatusStrFailPcc);
  CPPUNIT_TEST(testGetStatusStrFailCarrier);
  CPPUNIT_TEST(testGetStatusStrFailSalesDate);
  CPPUNIT_TEST(testGetStatusStrFailTravelDate);
  CPPUNIT_TEST(testGetStatusStrFailIataNumber);
  CPPUNIT_TEST(testGetStatusStrFailCarrierGds);
  CPPUNIT_TEST(testGetStatusStrFailAgentLoc);
  CPPUNIT_TEST(testGetStatusStrFailDeptCode);
  CPPUNIT_TEST(testGetStatusStrFailOfficeDesig);
  CPPUNIT_TEST(testGetStatusStrFailSecurity);
  CPPUNIT_TEST(testGetStatusStrFailViewBook);
  CPPUNIT_TEST(testParsePassengerType);
  CPPUNIT_TEST(testParseMarketLocation);
  CPPUNIT_TEST(testParseIataNumbers);
  CPPUNIT_TEST(testPopulateIataNumbersSecurity);
  CPPUNIT_TEST(testPopulateFareMarket);
  CPPUNIT_TEST(testCarrierFlightItemNumParse);
  CPPUNIT_TEST(testNoCarrierFlightItemNumParse);
  CPPUNIT_TEST(testResponseHasDataSource);
  CPPUNIT_TEST(testResponseHasNoDataSource);
  CPPUNIT_TEST(testResponseHasBrandText);
  CPPUNIT_TEST(testResponseHasNoBrandText);
  CPPUNIT_TEST(testResponseHasBookingCode);
  CPPUNIT_TEST(testResponseHasMultipleBookingCodes);
  CPPUNIT_TEST(testResponseHasNoBookingCode);
  CPPUNIT_TEST(testResponseHasFareBasisCode);
  CPPUNIT_TEST(testResponseHasNoFareBasisCode);
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
    _s8BrandingSecurity = _memHandle.create<S8BrandingSecurity>();
    _securityInfo = _memHandle.create<SecurityInfo>();
    buildCarrierGdsSecurityInfo();
    _ruleExecution = _memHandle.create<RuleExecution>();
    buildRuleExecution();
    _marketRule = _memHandle.create<MarketRule>();
    buildMarketRule();
    sfo = getLoc("SFO");
    dfw = getLoc("DFW");
    _fm1 = createFareMarket(sfo, dfw, "AA");
    _memHandle.create<MyDataHandle>();
  }

  const Loc* getLoc(const LocCode& locCode)
  {
    return TestLocFactory::create("/vobs/atseintl/test/testdata/data/Loc" + locCode + ".xml");
  }

  void tearDown() { _memHandle.clear(); }

  void testParse()
  {
    std::string xmlResponse =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><GetAirlineBrandsRS "
        "xmlns=\"http://services.sabre.com/Branding/V1\" "
        "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
        "xmlns:ns3=\"http://opentravel.org/common/v02\" "
        "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_00\" "
        "xmlns:ns5=\"http://services.sabre.com/STL/v02\"><BrandingResponse><ResponseSource "
        "pseudoCityCode=\"80K2\" iATANumber=\"9999999\" clientID=\"FQ\" requestType=\"\" "
        "requestingCarrierGDS=\"\" GEOLocation=\"MIA\" departmentCode=\"80K2\" "
        "officeDesignator=\"\"/><BrandingResults><CarrierBrandsData/></BrandingResults></"
        "BrandingResponse></GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();
    std::string clientId = s8BrandingResponseParser.requestSource()->clientID();
    std::string expectedClientId = "FQ";
    CPPUNIT_ASSERT_EQUAL(clientId, expectedClientId);
    CPPUNIT_ASSERT(!s8BrandingResponseParser.isError());
  }

  void testPricingTrxBrandProgramVec()
  {
    //remove this test when removing fallbackUseOnlyFirstProgram
    fallback::value::fallbackUseOnlyFirstProgram.set(true);

    std::string xmlResponse =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><ns5:GetAirlineBrandsRS "
        "xmlns=\"http://services.sabre.com/STL/v02\" "
        "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
        "xmlns:ns3=\"http://opentravel.org/common/v02\" "
        "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
        "xmlns:ns5=\"http://stl.sabre.com/Merchandising/"
        "v1\"><ns5:BrandingResponse><ns5:ResponseSource pseudoCityCode=\"80K2\" "
        "iATANumber=\"9999999\" clientID=\"FQ\" requestType=\"\" requestingCarrierGDS=\"1S\" "
        "GEOLocation=\"MIA\" departmentCode=\"80K2\" "
        "officeDesignator=\"\"/><ns5:BrandingResults><ns5:MarketResponse "
        "marketID=\"1\"><ns5:ProgramIdList><ns5:ProgramID>12</ns5:ProgramID><ns5:ProgramID>9</"
        "ns5:ProgramID></ns5:ProgramIdList><ns5:Carrier>VA</"
        "ns5:Carrier><ns5:MarketCriteria><ns5:DepartureAirportCode>MEL</"
        "ns5:DepartureAirportCode><ns5:ArrivalAirportCode>SYD</"
        "ns5:ArrivalAirportCode><ns5:PassengerTypes><ns5:Type>ADT</ns5:Type><ns5:Type>NEG</"
        "ns5:Type></ns5:PassengerTypes></ns5:MarketCriteria></"
        "ns5:MarketResponse><ns5:CarrierBrandsData><ns5:BrandProgram "
        "programID=\"12\"><ns5:Vendor>MMGR</ns5:Vendor><ns5:GlobalIndicator></"
        "ns5:GlobalIndicator><ns5:ProgramCode>VIRGINDMAU</"
        "ns5:ProgramCode><ns5:ProgramName>Direction BETWEEN AU TO "
        "AU</ns5:ProgramName><ns5:BrandsData><ns5:Brand><ns5:Identifier>34</"
        "ns5:Identifier><ns5:Code>DOGGONE</ns5:Code><ns5:Name>SNIFFLES IS OUR "
        "DOG</ns5:Name><ns5:Tier>1</ns5:Tier><ns5:PrimaryFareIDTable>38</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand></ns5:BrandsData><ns5:AccountCodeList><ns5:AccountCode>GSO01</ns5:AccountCode></"
        "ns5:AccountCodeList></ns5:BrandProgram><ns5:BrandProgram "
        "programID=\"9\"><ns5:Vendor>MMGR</ns5:Vendor><ns5:GlobalIndicator></"
        "ns5:GlobalIndicator><ns5:ProgramCode>ABCDEF.1</ns5:ProgramCode><ns5:ProgramName>Test my "
        "Merchandise N "
        "AU</ns5:ProgramName><ns5:PassengerType>ADT</"
        "ns5:PassengerType><ns5:BrandsData><ns5:Brand><ns5:Identifier>23</"
        "ns5:Identifier><ns5:Code>ALPHA</ns5:Code><ns5:Name>HEY DIDDLE "
        "DIDDLE</ns5:Name><ns5:Tier>1</ns5:Tier><ns5:PrimaryFareIDTable>27</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand><ns5:Brand><ns5:Identifier>24</ns5:Identifier><ns5:Code>BETA</"
        "ns5:Code><ns5:Name>AN APPLE A "
        "DAY</ns5:Name><ns5:Tier>2</ns5:Tier><ns5:PrimaryFareIDTable>28</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand><ns5:Brand><ns5:Identifier>25</ns5:Identifier><ns5:Code>CAPRICORN</"
        "ns5:Code><ns5:Name>HOT CROSS "
        "BREAD</ns5:Name><ns5:Tier>3</ns5:Tier><ns5:PrimaryFareIDTable>29</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand><ns5:Brand><ns5:Identifier>26</ns5:Identifier><ns5:Code>DELTA</"
        "ns5:Code><ns5:Name>ITSY BITSY "
        "SPIDER</ns5:Name><ns5:Tier>4</ns5:Tier><ns5:PrimaryFareIDTable>30</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand></ns5:BrandsData><ns5:AccountCodeList><ns5:AccountCode>GSO01</ns5:AccountCode></"
        "ns5:AccountCodeList></ns5:BrandProgram></ns5:CarrierBrandsData></ns5:BrandingResults></"
        "ns5:BrandingResponse></ns5:GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();
    CPPUNIT_ASSERT(!_trx->brandProgramVec().empty());
    CPPUNIT_ASSERT_EQUAL(5, (int)_trx->brandProgramVec().size());
    CPPUNIT_ASSERT(!s8BrandingResponseParser.isError());
  }

  void testPricingTrxBrandProgramVecOneProgramPerFeareMarket()
  {

    std::string xmlResponse =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><ns5:GetAirlineBrandsRS "
        "xmlns=\"http://services.sabre.com/STL/v02\" "
        "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
        "xmlns:ns3=\"http://opentravel.org/common/v02\" "
        "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
        "xmlns:ns5=\"http://stl.sabre.com/Merchandising/"
        "v1\"><ns5:BrandingResponse><ns5:ResponseSource pseudoCityCode=\"80K2\" "
        "iATANumber=\"9999999\" clientID=\"FQ\" requestType=\"\" requestingCarrierGDS=\"1S\" "
        "GEOLocation=\"MIA\" departmentCode=\"80K2\" "
        "officeDesignator=\"\"/><ns5:BrandingResults><ns5:MarketResponse "
        "marketID=\"1\"><ns5:ProgramIdList><ns5:ProgramID>12</ns5:ProgramID><ns5:ProgramID>9</"
        "ns5:ProgramID></ns5:ProgramIdList><ns5:Carrier>VA</"
        "ns5:Carrier><ns5:MarketCriteria><ns5:DepartureAirportCode>MEL</"
        "ns5:DepartureAirportCode><ns5:ArrivalAirportCode>SYD</"
        "ns5:ArrivalAirportCode><ns5:PassengerTypes><ns5:Type>ADT</ns5:Type><ns5:Type>NEG</"
        "ns5:Type></ns5:PassengerTypes></ns5:MarketCriteria></"
        "ns5:MarketResponse><ns5:CarrierBrandsData><ns5:BrandProgram "
        "programID=\"12\"><ns5:Vendor>MMGR</ns5:Vendor><ns5:GlobalIndicator></"
        "ns5:GlobalIndicator><ns5:ProgramCode>VIRGINDMAU</"
        "ns5:ProgramCode><ns5:ProgramName>Direction BETWEEN AU TO "
        "AU</ns5:ProgramName><ns5:BrandsData><ns5:Brand><ns5:Identifier>34</"
        "ns5:Identifier><ns5:Code>DOGGONE</ns5:Code><ns5:Name>SNIFFLES IS OUR "
        "DOG</ns5:Name><ns5:Tier>1</ns5:Tier><ns5:PrimaryFareIDTable>38</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand></ns5:BrandsData><ns5:AccountCodeList><ns5:AccountCode>GSO01</ns5:AccountCode></"
        "ns5:AccountCodeList></ns5:BrandProgram><ns5:BrandProgram "
        "programID=\"9\"><ns5:Vendor>MMGR</ns5:Vendor><ns5:GlobalIndicator></"
        "ns5:GlobalIndicator><ns5:ProgramCode>ABCDEF.1</ns5:ProgramCode><ns5:ProgramName>Test my "
        "Merchandise N "
        "AU</ns5:ProgramName><ns5:PassengerType>ADT</"
        "ns5:PassengerType><ns5:BrandsData><ns5:Brand><ns5:Identifier>23</"
        "ns5:Identifier><ns5:Code>ALPHA</ns5:Code><ns5:Name>HEY DIDDLE "
        "DIDDLE</ns5:Name><ns5:Tier>1</ns5:Tier><ns5:PrimaryFareIDTable>27</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand><ns5:Brand><ns5:Identifier>24</ns5:Identifier><ns5:Code>BETA</"
        "ns5:Code><ns5:Name>AN APPLE A "
        "DAY</ns5:Name><ns5:Tier>2</ns5:Tier><ns5:PrimaryFareIDTable>28</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand><ns5:Brand><ns5:Identifier>25</ns5:Identifier><ns5:Code>CAPRICORN</"
        "ns5:Code><ns5:Name>HOT CROSS "
        "BREAD</ns5:Name><ns5:Tier>3</ns5:Tier><ns5:PrimaryFareIDTable>29</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand><ns5:Brand><ns5:Identifier>26</ns5:Identifier><ns5:Code>DELTA</"
        "ns5:Code><ns5:Name>ITSY BITSY "
        "SPIDER</ns5:Name><ns5:Tier>4</ns5:Tier><ns5:PrimaryFareIDTable>30</"
        "ns5:PrimaryFareIDTable><ns5:SecondaryFareIDTable>0</ns5:SecondaryFareIDTable></"
        "ns5:Brand></ns5:BrandsData><ns5:AccountCodeList><ns5:AccountCode>GSO01</ns5:AccountCode></"
        "ns5:AccountCodeList></ns5:BrandProgram></ns5:CarrierBrandsData></ns5:BrandingResults></"
        "ns5:BrandingResponse></ns5:GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();
    CPPUNIT_ASSERT(!_trx->brandProgramVec().empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)_trx->brandProgramVec().size()); //first program has only one brand
    CPPUNIT_ASSERT(!s8BrandingResponseParser.isError());
  }

  void testErrorResponseParse()
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
    s8BrandingResponseParser.print();
    CPPUNIT_ASSERT(s8BrandingResponseParser.isError());
    CPPUNIT_ASSERT_EQUAL(std::string("Error"),
                         s8BrandingResponseParser.brandingResponse()->messageCode());
    CPPUNIT_ASSERT_EQUAL(std::string("2015"),
                         s8BrandingResponseParser.brandingResponse()->failCode());
    CPPUNIT_ASSERT_EQUAL(std::string("Invalid Office Designator - 80K2WC"),
                         s8BrandingResponseParser.brandingResponse()->messageText());
  }

  void testDiagnosticResponseParse()
  {

    std::string xmlResponse =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
        "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
        "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
        "xmlns:ns3=\"http://opentravel.org/common/v02\" "
        "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
        "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
        "pseudoCityCode=\"80K2\" iataNumber=\"9999999\" clientID=\"FQ\" requestType=\"D\" "
        "requestingCarrierGDS=\"1S\" geoLocation=\"MIA\" departmentCode=\"80K2\" dutyCode=\"*\" "
        "functionCode=\"B\"></ResponseSource> <BrandingResults> <MarketResponse marketID=\"1\"> "
        "<ProgramIdList> <ProgramID>132</ProgramID> </ProgramIdList> <Carrier>BA</Carrier> "
        "<MarketCriteria direction=\"OT\"> <DepartureDate>2014-01-15</DepartureDate> "
        "<DepartureAirportCode>LHR</DepartureAirportCode> "
        "<ArrivalAirportCode>NYC</ArrivalAirportCode> </MarketCriteria> <Diagnostics> "
        "<RuleExecution ruleId=\"132\"> <ProgramCode>BGI</ProgramCode> <Condition "
        "pattern=\"CollectionHelper.containsAnElementFrom('GSO01', "
        "accountCodes)\">true</Condition> <Condition pattern=\"carrierGds in "
        "('1S')\">true</Condition> <Condition pattern=\"isViewBookTicket == "
        "true\">true</Condition> <Condition pattern=\"carrier == 'BA'\">true</Condition> "
        "<Condition pattern=\"DateHelper.isDateInRange(salesDate, '4', '11', '2013', '29', '2', "
        "'2016')\">true</Condition> <Condition pattern=\"pcc in ('80K2')\">true</Condition> "
        "<Condition pattern=\"DateHelper.isDateInRange(travelDate, '4', '11', '2013', '29', '2', "
        "'2016')\">true</Condition> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LON', 'M', 'USNY', 'S', 'OI', '', '')\">true</Condition> "
        "</RuleExecution> <RuleExecution ruleId=\"251\"> <ProgramCode>BBO</ProgramCode> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LHR', 'M', 'YVR', 'M', 'OI', '', '')\">false</Condition> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '11', '11', '2013', '31', '12', "
        "'2014')\">true</Condition> <Condition pattern=\"carrier == 'BA'\">true</Condition> "
        "</RuleExecution> <RuleExecution ruleId=\"133\"> <ProgramCode>BOL</ProgramCode> <Condition "
        "pattern=\"carrier == 'BA'\">true</Condition> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '4', '11', '2013', '31', '12', "
        "'2014')\">true</Condition> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LHR', 'M', 'ORD', 'M', 'OI', '', '')\">false</Condition> "
        "</RuleExecution> <RuleExecution ruleId=\"252\"> <ProgramCode>BAL</ProgramCode> <Condition "
        "pattern=\"carrier == 'BA'\">true</Condition> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '11', '11', '2013', '31', '12', "
        "'9999')\">true</Condition> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LON', 'M', 'AR', 'N', 'OI', '', '')\">false</Condition>";
    xmlResponse +=
        " </RuleExecution> <RuleExecution ruleId=\"122\"> <ProgramCode>BEB</ProgramCode> "
        "<Condition pattern=\"DateHelper.isDateInRange(salesDate, '30', '10', '2013', '31', '12', "
        "'2014')\">true</Condition> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LON', 'M', 'SYD', 'M', 'OI', 'EH', '')\">false</Condition> <Condition "
        "pattern=\"carrier == 'BA'\">true</Condition> </RuleExecution> <RuleExecution "
        "ruleId=\"113\"> <ProgramCode>BQE</ProgramCode> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '30', '10', '2013', '31', '12', "
        "'2014')\">true</Condition> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LHR', 'M', 'USCA', 'S', 'OI', '', '')\">false</Condition> <Condition "
        "pattern=\"carrier == 'BA'\">true</Condition> </RuleExecution> <RuleExecution "
        "ruleId=\"123\"> <ProgramCode>BET</ProgramCode> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '30', '10', '2013', '31', '12', "
        "'2014')\">true</Condition> <Condition pattern=\"carrier == 'BA'\">true</Condition> "
        "<Condition pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LON', 'M', 'SYD', 'M', 'OI', 'TS', '')\">false</Condition> "
        "</RuleExecution> <RuleExecution ruleId=\"124\"> <ProgramCode>BES</ProgramCode> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LON', 'M', 'SYD', 'M', 'OI', 'AP', '')\">false</Condition> <Condition "
        "pattern=\"carrier == 'BA'\">true</Condition> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '30', '10', '2013', '1', '1', "
        "'2015')\">true</Condition> </RuleExecution> <RuleExecution ruleId=\"178\"> "
        "<ProgramCode>BWF</ProgramCode> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, '2', 'A', 'BR', 'N', 'OI', '', '')\">false</Condition> <Condition "
        "pattern=\"carrier == 'BA'\">true</Condition> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '6', '11', '2013', '31', '12', "
        "'2014')\">true</Condition> </RuleExecution> <RuleExecution ruleId=\"177\"> "
        "<ProgramCode>BD</ProgramCode>  <Condition pattern=\"carrier == 'BA'\">true</Condition> "
        "<Condition pattern=\"DateHelper.isDateInRange(salesDate, '6', '11', '2013', '31', '12', "
        "'2014')\">true</Condition> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, '2', 'A', '2', 'A', 'OT', '', '')\">false</Condition> </RuleExecution>";

    xmlResponse +=
        " <RuleExecution ruleId=\"176\"> <ProgramCode>BNM</ProgramCode> <Condition "
        "pattern=\"carrierGds in ('1S')\">true</Condition> <Condition pattern=\"isViewBookTicket "
        "== true\">true</Condition> <Condition pattern=\"carrier == 'BA'\">true</Condition> "
        "<Condition pattern=\"pcc in ('80K2')\">true</Condition> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '6', '11', '2013', '31', '12', "
        "'2014')\">true</Condition> <Condition pattern=\"DateHelper.isDateInRange(travelDate, '6', "
        "'11', '2013', '31', '12', '2014')\">true</Condition> </RuleExecution> <RuleExecution "
        "ruleId=\"101\"> <ProgramCode>BAW</ProgramCode> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LON', 'M', 'CA', 'N', 'OI', '', '')\">false</Condition> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '24', '10', '2013', '31', '12', "
        "'2014')\">true</Condition> <Condition pattern=\"carrier == 'BA'\">true</Condition> "
        "</RuleExecution> <RuleExecution ruleId=\"118\"> <ProgramCode>BWC</ProgramCode> <Condition "
        "pattern=\"DateHelper.isDateInRange(salesDate, '30', '10', '2013', '31', '12', "
        "'2014')\">true</Condition> <Condition pattern=\"carrier == 'BA'\">true</Condition> "
        "<Condition pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LON', 'M', 'SFO', 'M', 'OI', '', '')\">false</Condition> "
        "</RuleExecution> </Diagnostics> </MarketResponse> <CarrierBrandsData> <BrandProgram "
        "programID=\"132\"> <Vendor>MMGR</Vendor> <GlobalIndicator></GlobalIndicator> "
        "<ProgramCode>BGI</ProgramCode> <ProgramName>George the III</ProgramName> <BrandsData> "
        "<Brand> <Identifier>509</Identifier> <Code>BH</Code> <Name>BUNKER HILL</Name> "
        "<Tier>1</Tier> <PrimaryFareIDTable>522</PrimaryFareIDTable> "
        "<SecondaryFareIDTable>0</SecondaryFareIDTable> </Brand> <Brand> "
        "<Identifier>510</Identifier> <Code>S</Code> <Name>SARATOGA</Name> <Tier>2</Tier> "
        "<PrimaryFareIDTable>523</PrimaryFareIDTable> "
        "<SecondaryFareIDTable>0</SecondaryFareIDTable> </Brand> <Brand> "
        "<Identifier>511</Identifier> <Code>VF</Code> <Name>VALLEY FORGE</Name> <Tier>3</Tier> "
        "<PrimaryFareIDTable>524</PrimaryFareIDTable> "
        "<SecondaryFareIDTable>0</SecondaryFareIDTable> </Brand> <Brand> "
        "<Identifier>512</Identifier> <Code>LC</Code> <Name>LEXINGTON and CONCORD</Name> "
        "<Tier>4</Tier> <PrimaryFareIDTable>525</PrimaryFareIDTable> "
        "<SecondaryFareIDTable>0</SecondaryFareIDTable> </Brand> </BrandsData> <AccountCodeList> "
        "<AccountCode>GSO01</AccountCode> </AccountCodeList> <SystemCode></SystemCode> "
        "</BrandProgram> </CarrierBrandsData> </BrandingResults> </BrandingResponse> "
        "</GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    CPPUNIT_ASSERT(!marketResponse->bsDiagnostics()->ruleExecution().empty());
    CPPUNIT_ASSERT_EQUAL(13, (int)marketResponse->bsDiagnostics()->ruleExecution().size());
  }

  void testDiagnosticResponseRuleExecutionParse()
  {

    std::string xmlResponse =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
        "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
        "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
        "xmlns:ns3=\"http://opentravel.org/common/v02\" "
        "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
        "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
        "pseudoCityCode=\"80K2\" iataNumber=\"9999999\" clientID=\"FQ\" requestType=\"D\" "
        "requestingCarrierGDS=\"1S\" geoLocation=\"MIA\" departmentCode=\"80K2\" dutyCode=\"*\" "
        "functionCode=\"B\"></ResponseSource> <BrandingResults> <MarketResponse marketID=\"1\"> "
        "<ProgramIdList> <ProgramID>132</ProgramID> </ProgramIdList> <Carrier>BA</Carrier> "
        "<MarketCriteria direction=\"OT\"> <DepartureDate>2014-01-15</DepartureDate> "
        "<DepartureAirportCode>LHR</DepartureAirportCode> "
        "<ArrivalAirportCode>NYC</ArrivalAirportCode> </MarketCriteria> <Diagnostics> "
        "<RuleExecution ruleId=\"132\"> <ProgramCode>BGI</ProgramCode> <Condition "
        "pattern=\"CollectionHelper.containsAnElementFrom('GSO01', "
        "accountCodes)\">true</Condition> <Condition pattern=\"carrierGds in "
        "('1S')\">true</Condition> <Condition pattern=\"isViewBookTicket == "
        "true\">true</Condition> <Condition pattern=\"carrier == 'BA'\">true</Condition> "
        "<Condition pattern=\"DateHelper.isDateInRange(salesDate, '4', '11', '2013', '29', '2', "
        "'2016')\">true</Condition> <Condition pattern=\"pcc in ('80K2')\">true</Condition> "
        "<Condition pattern=\"DateHelper.isDateInRange(travelDate, '4', '11', '2013', '29', '2', "
        "'2016')\">true</Condition> <Condition "
        "pattern=\"marketLocationHelper.isMarketMatch(origin, destination, direction, "
        "rqGlobalIndicator, 'LON', 'M', 'USNY', 'S', 'OI', '', '')\">true</Condition> "
        "</RuleExecution>";

    xmlResponse += " </Diagnostics> </MarketResponse> <CarrierBrandsData> <BrandProgram "
                   "programID=\"132\"> <Vendor>MMGR</Vendor> <GlobalIndicator></GlobalIndicator> "
                   "<ProgramCode>BGI</ProgramCode> <ProgramName>George the III</ProgramName> "
                   "<BrandsData> <Brand> <Identifier>509</Identifier> <Code>BH</Code> <Name>BUNKER "
                   "HILL</Name> <Tier>1</Tier> <PrimaryFareIDTable>522</PrimaryFareIDTable> "
                   "<SecondaryFareIDTable>0</SecondaryFareIDTable> </Brand> <Brand> "
                   "<Identifier>510</Identifier> <Code>S</Code> <Name>SARATOGA</Name> "
                   "<Tier>2</Tier> <PrimaryFareIDTable>523</PrimaryFareIDTable> "
                   "<SecondaryFareIDTable>0</SecondaryFareIDTable> </Brand> <Brand> "
                   "<Identifier>511</Identifier> <Code>VF</Code> <Name>VALLEY FORGE</Name> "
                   "<Tier>3</Tier> <PrimaryFareIDTable>524</PrimaryFareIDTable> "
                   "<SecondaryFareIDTable>0</SecondaryFareIDTable> </Brand> <Brand> "
                   "<Identifier>512</Identifier> <Code>LC</Code> <Name>LEXINGTON and "
                   "CONCORD</Name> <Tier>4</Tier> <PrimaryFareIDTable>525</PrimaryFareIDTable> "
                   "<SecondaryFareIDTable>0</SecondaryFareIDTable> </Brand> </BrandsData> "
                   "<AccountCodeList> <AccountCode>GSO01</AccountCode> </AccountCodeList> "
                   "<SystemCode></SystemCode> </BrandProgram> </CarrierBrandsData> "
                   "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    CPPUNIT_ASSERT(!marketResponse->bsDiagnostics()->ruleExecution().empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)marketResponse->bsDiagnostics()->ruleExecution().size());
    RuleExecution* ruleExecution = marketResponse->bsDiagnostics()->ruleExecution().front();
    CPPUNIT_ASSERT_EQUAL(std::string("BGI"), std::string(ruleExecution->programCode().c_str()));
    CPPUNIT_ASSERT_EQUAL(std::string("132"), std::string(ruleExecution->ruleID().c_str()));
    CPPUNIT_ASSERT(ruleExecution->passengerType().empty());
  }

  void buildCarrierGdsSecurityInfo()
  {
    _securityInfo->_securityName = "CARRIER GDS";
    _securityInfo->_securityValue = "1S";
    _securityInfo->_status = PASS_S8;
    _s8BrandingSecurity->securityInfoVec().push_back(_securityInfo);
  }

  void testProcessPassSecurity()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.processPassSecurity(_securityInfo);
    CPPUNIT_ASSERT_EQUAL(
        s8BrandingResponseParser._securityStatus[S8BrandingResponseParser::CARRIER_GDS],
        S8BrandingResponseParser::SS_PASS);
  }

  void testProcessFailSecurity()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    _securityInfo->_status = FAIL_S8_CARRIER_GDS;
    s8BrandingResponseParser.processFailSecurity(_securityInfo);
    CPPUNIT_ASSERT_EQUAL(
        s8BrandingResponseParser._securityStatus[S8BrandingResponseParser::CARRIER_GDS],
        S8BrandingResponseParser::SS_FAIL);
  }

  void testCheckSecurityFailure()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    _securityInfo->_status = FAIL_S8_CARRIER_GDS;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.checkSecurityFailure();
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser._ruleExecution->status(), FAIL_S8_SECURITY);
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
    _ruleExecution->s8BrandingSecurity() = _s8BrandingSecurity;
  }

  void testAddPatternInfo()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::vector<std::string> patternValueVec;
    patternValueVec.push_back("80K2");
    patternValueVec.push_back("DQ73");
    _ruleExecution->pseudoCityCodes().clear();
    s8BrandingResponseParser.addPatternInfo(
        patternValueVec, "false", _ruleExecution->pseudoCityCodes());
    CPPUNIT_ASSERT_EQUAL(2, (int)_ruleExecution->pseudoCityCodes().size());
  }

  void testSetPatternStatus()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_PCC;
    StatusS8 status = PASS_S8;
    s8BrandingResponseParser.setPatternStatus(status);
    CPPUNIT_ASSERT_EQUAL(status, FAIL_S8_PCC);
  }

  void testCheckFailure()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::vector<PatternInfo*> patternValueVec;
    PatternInfo* patternInfoPCC = _memHandle.create<PatternInfo>();
    patternInfoPCC->_patternValue = "80K2";
    patternInfoPCC->_status = FAIL_S8_PCC;
    patternValueVec.push_back(patternInfoPCC);

    StatusS8 status = FAIL_S8_PCC;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.checkFailure(patternValueVec, status);

    CPPUNIT_ASSERT_EQUAL(_ruleExecution->status(), FAIL_S8_PCC);
  }

  void testCheckPatternFailure()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::vector<std::string> patternValueVec;
    patternValueVec.push_back("80K2");
    patternValueVec.push_back("DQ73");
    _ruleExecution->pseudoCityCodes().clear();
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.checkPatternFailure();
    CPPUNIT_ASSERT_EQUAL(_ruleExecution->status(), PASS_S8);
  }

  void buildMarketRule()
  {
    _marketRule->originLoc() = "LON";
    _marketRule->originLocType() = 'M';
    _marketRule->destinationLoc() = "USNY";
    _marketRule->destinationLocType() = 'S';
    _marketRule->direction() = "OI";
  }

  void testAddMarketRuleInfo()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::vector<MarketRule*> marketRuleVec;
    marketRuleVec.push_back(_marketRule);
    _ruleExecution->marketRules().clear();
    s8BrandingResponseParser.addMarketRuleInfo(
        marketRuleVec, "false", _ruleExecution->marketRules());
    CPPUNIT_ASSERT_EQUAL(1, (int)_ruleExecution->marketRules().size());
  }

  void testCheckMarketFailure()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::vector<MarketRuleInfo*> marketRuleInfoVec;
    MarketRuleInfo* marketRuleInfo = _memHandle.create<MarketRuleInfo>();
    marketRuleInfo->_marketRule = _marketRule;
    marketRuleInfo->_status = FAIL_S8_MARKET;
    marketRuleInfoVec.push_back(marketRuleInfo);

    StatusS8 status = FAIL_S8_MARKET;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.checkMarketFailure(marketRuleInfoVec, status);

    CPPUNIT_ASSERT_EQUAL(_ruleExecution->status(), FAIL_S8_MARKET);
  }

  void testSetPatternStatusFailPassengerType()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_PASSENGER_TYPE;
    StatusS8 status = PASS_S8;
    s8BrandingResponseParser.setPatternStatus(status);
    CPPUNIT_ASSERT_EQUAL(status, FAIL_S8_PASSENGER_TYPE);
  }

  void testSetPatternStatusFailMarket()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_MARKET;
    StatusS8 status = PASS_S8;
    s8BrandingResponseParser.setPatternStatus(status);
    CPPUNIT_ASSERT_EQUAL(status, FAIL_S8_MARKET);
  }

  void testSetPatternStatusFailPCC()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_PCC;
    StatusS8 status = PASS_S8;
    s8BrandingResponseParser.setPatternStatus(status);
    CPPUNIT_ASSERT_EQUAL(status, FAIL_S8_PCC);
  }

  void testSetPatternStatusFailCarrier()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_CARRIER;
    StatusS8 status = PASS_S8;
    s8BrandingResponseParser.setPatternStatus(status);
    CPPUNIT_ASSERT_EQUAL(status, FAIL_S8_CARRIER);
  }

  void testSetPatternStatusFailSalesDate()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_SALES_DATE;
    StatusS8 status = PASS_S8;
    s8BrandingResponseParser.setPatternStatus(status);
    CPPUNIT_ASSERT_EQUAL(status, FAIL_S8_SALES_DATE);
  }

  void testSetPatternStatusFailTravelDate()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_TRAVEL_DATE;
    StatusS8 status = PASS_S8;
    s8BrandingResponseParser.setPatternStatus(status);
    CPPUNIT_ASSERT_EQUAL(status, FAIL_S8_TRAVEL_DATE);
  }

  void testGetSecurityStatusCarrierGds()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._securityName = "CARRIER GDS";
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser.getSecurityStatus(), FAIL_S8_CARRIER_GDS);
  }

  void testGetSecurityStatusDepartmentCode()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._securityName = "DEPT CODE";
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser.getSecurityStatus(), FAIL_S8_DEPARTMENT_CODE);
  }

  void testGetSecurityStatusOfficeDesignator()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._securityName = "OFFICE DESIG";
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser.getSecurityStatus(), FAIL_S8_OFFICE_DESIGNATOR);
  }

  void testGetSecurityStatusMarketLocation()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._securityName = "MARKET LOC";
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser.getSecurityStatus(), FAIL_S8_AGENT_LOCATION);
  }

  void testGetSecurityStatusIataNumber()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._securityName = "IATA NUM";
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser.getSecurityStatus(), FAIL_S8_IATA_NUMBER);
  }

  void testGetSecurityStatusViewBookTicket()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._securityName = "VIEW BOOK";
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser.getSecurityStatus(), FAIL_S8_VIEW_BOOK_TICKET);
  }

  void testSetStatusFailPassengerType()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_PASSENGER_TYPE;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.setStatus();
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser._ruleExecution->status(), FAIL_S8_PASSENGER_TYPE);
  }

  void testSetStatusFailAccountCode()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_ACCOUNT_CODE;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.setStatus();
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser._ruleExecution->status(), FAIL_S8_ACCOUNT_CODE);
  }

  void testSetStatusFailMarket()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_MARKET;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.setStatus();
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser._ruleExecution->status(), FAIL_S8_MARKET);
  }

  void testSetStatusFailPCC()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_PCC;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.setStatus();
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser._ruleExecution->status(), FAIL_S8_PCC);
  }

  void testSetStatusFailCarrier()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_CARRIER;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.setStatus();
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser._ruleExecution->status(), FAIL_S8_CARRIER);
  }

  void testSetStatusFailSalesDate()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_SALES_DATE;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.setStatus();
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser._ruleExecution->status(), FAIL_S8_SALES_DATE);
  }

  void testSetStatusFailTravelDate()
  {
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser._s8PatternType = S8BrandingResponseParser::S8_PT_TRAVEL_DATE;
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.setStatus();
    CPPUNIT_ASSERT_EQUAL(s8BrandingResponseParser._ruleExecution->status(), FAIL_S8_TRAVEL_DATE);
  }

  void testGetStatusStrFailPassengerType()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_PASSENGER_TYPE, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL PSGR TYPE"), statusStr);
  }

  void testGetStatusStrFailAccCode()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_ACCOUNT_CODE, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL ACC CODE"), statusStr);
  }

  void testGetStatusStrFailMarket()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_MARKET, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL MARKET"), statusStr);
  }

  void testGetStatusStrFailPcc()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_PCC, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL PCC"), statusStr);
  }

  void testGetStatusStrFailCarrier()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_CARRIER, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL CARRIER"), statusStr);
  }

  void testGetStatusStrFailSalesDate()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_SALES_DATE, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL SALES DATE"), statusStr);
  }

  void testGetStatusStrFailTravelDate()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_TRAVEL_DATE, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL TRAVEL DATE"), statusStr);
  }

  void testGetStatusStrFailIataNumber()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_IATA_NUMBER, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL IATA NUM"), statusStr);
  }

  void testGetStatusStrFailCarrierGds()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_CARRIER_GDS, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL CARRIER GDS"), statusStr);
  }

  void testGetStatusStrFailAgentLoc()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_AGENT_LOCATION, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL AGENT LOC"), statusStr);
  }

  void testGetStatusStrFailDeptCode()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_DEPARTMENT_CODE, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL DEPT CODE"), statusStr);
  }

  void testGetStatusStrFailOfficeDesig()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_OFFICE_DESIGNATOR, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL OFFICE DESIG"), statusStr);
  }

  void testGetStatusStrFailSecurity()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_SECURITY, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL SECURITY"), statusStr);
  }

  void testGetStatusStrFailViewBook()
  {
    std::string statusStr;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.getStatusStr(FAIL_S8_VIEW_BOOK_TICKET, statusStr);
    CPPUNIT_ASSERT_EQUAL(string("FAIL VIEW BOOK"), statusStr);
  }

  void testParsePassengerType()
  {
    std::string patternValue = " 'ADT', passengerTypeCodes)";
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.parsePassengerType(patternValue);
    CPPUNIT_ASSERT_EQUAL(1, (int)s8BrandingResponseParser._passengerTypeVec.size());
    CPPUNIT_ASSERT_EQUAL(string("ADT"), s8BrandingResponseParser._passengerTypeVec.front());
  }

  void testParseMarketLocation()
  {
    std::string patternValue = " 'PL', 'N', '')";
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.parseMarketLocation(patternValue);
    CPPUNIT_ASSERT_EQUAL(string("PL|N"), s8BrandingResponseParser._securityValue);
  }

  void testParseIataNumbers()
  {
    std::string patternValue = "4553728, 4553729)";
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.parseIataNumbers(patternValue);
    CPPUNIT_ASSERT_EQUAL(2, (int)s8BrandingResponseParser._iataNumbers.size());
    CPPUNIT_ASSERT_EQUAL(string("4553728"), s8BrandingResponseParser._iataNumbers.front());
  }

  void testPopulateIataNumbersSecurity()
  {
    std::string condition = "false";
    std::string patternValue = "4553728, 4553729)";
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.parseIataNumbers(patternValue);
    s8BrandingResponseParser._ruleExecution = _ruleExecution;
    s8BrandingResponseParser.populateIataNumbersSecurity(condition);
    CPPUNIT_ASSERT_EQUAL(3,
                         (int)s8BrandingResponseParser._ruleExecution->s8BrandingSecurity()
                             ->securityInfoVec()
                             .size());
    CPPUNIT_ASSERT_EQUAL(string("CARRIER GDS"),
                         s8BrandingResponseParser._ruleExecution->s8BrandingSecurity()
                             ->securityInfoVec()
                             .front()
                             ->_securityName);
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

  void testPopulateFareMarket()
  {
    int index = 0;
    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    s8BrandingResponseParser.populateFareMarket(_fm1, index);
    CPPUNIT_ASSERT_EQUAL(1, (int)_fm1->brandProgramIndexVec().size());
  }

  void testCarrierFlightItemNumParse()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> <ProgramID>896</ProgramID> "
    "</ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>23</CarrierFlightItemNum> "
    "</Brand> <Brand> <Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>275</CarrierFlightItemNum> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>3142</CarrierFlightItemNum> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();
    BrandProgram* brandProgram = brandPrograms.front();
    BrandInfo* brandInfo = brandProgram->brandsData().front();
    CPPUNIT_ASSERT_EQUAL(23, brandInfo->getCarrierFlightItemNum()); 
  }

  void testNoCarrierFlightItemNumParse()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> <ProgramID>896</ProgramID> "
    "</ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();
    BrandProgram* brandProgram = brandPrograms.front();
    BrandInfo* brandInfo = brandProgram->brandsData().front();
    CPPUNIT_ASSERT_EQUAL(0, brandInfo->getCarrierFlightItemNum()); 
  }

  void testResponseHasDataSource()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList dataSource=\"CBS\"> "
    "<ProgramID>896</ProgramID> </ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>275</CarrierFlightItemNum> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>3142</CarrierFlightItemNum> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    CPPUNIT_ASSERT_EQUAL(BRAND_SOURCE_CBAS, marketResponse->dataSource());
  }

  void testResponseHasNoDataSource()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> "
    "<ProgramID>896</ProgramID> </ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>275</CarrierFlightItemNum> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>3142</CarrierFlightItemNum> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    CPPUNIT_ASSERT_EQUAL(BRAND_SOURCE_S8, marketResponse->dataSource());
  }

  void testResponseHasBrandText()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> <ProgramID>896</ProgramID> "
    "</ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <Text>TEXT</Text> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();
    BrandProgram* brandProgram = brandPrograms.front();
    BrandInfo* brandInfo = brandProgram->brandsData().front();
    CPPUNIT_ASSERT_EQUAL(std::string("TEXT"), brandInfo->brandText());
  }

  void testResponseHasNoBrandText()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> "
    "<ProgramID>896</ProgramID> </ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>275</CarrierFlightItemNum> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>3142</CarrierFlightItemNum> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();
    BrandProgram* brandProgram = brandPrograms.front();
    BrandInfo* brandInfo = brandProgram->brandsData().front();
    CPPUNIT_ASSERT(brandInfo->brandText().empty());
  }

  void testResponseHasBookingCode()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> <ProgramID>896</ProgramID> "
    "</ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> <BookingCodeList> <Code>Y</Code> </BookingCodeList>"
    "<SecondaryBookingCodeList> <Code>F</Code> </SecondaryBookingCodeList> </Brand> <Brand> "
    "<Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();
    BrandProgram* brandProgram = brandPrograms.front();
    BrandInfo* brandInfo = brandProgram->brandsData().front();
    CPPUNIT_ASSERT_EQUAL(BrandCode("CC"), brandInfo->brandCode());
    CPPUNIT_ASSERT_EQUAL(1, (int) brandInfo->primaryBookingCode().size());
    CPPUNIT_ASSERT_EQUAL(1, (int) brandInfo->secondaryBookingCode().size());
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), brandInfo->primaryBookingCode().front());
    CPPUNIT_ASSERT_EQUAL(BookingCode("F"), brandInfo->secondaryBookingCode().front());
  }

  void testResponseHasMultipleBookingCodes()
  {

    std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> <ProgramID>896</ProgramID> "
    "</ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> <BookingCodeList> <Code>Y</Code> <Code>Z</Code> </BookingCodeList>"
    "<SecondaryBookingCodeList> <Code>F</Code> <Code>G</Code> <Code>H</Code> </SecondaryBookingCodeList> </Brand> <Brand> "
    "<Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();
    BrandProgram* brandProgram = brandPrograms.front();
    BrandInfo* brandInfo = brandProgram->brandsData().front();
    CPPUNIT_ASSERT_EQUAL(BrandCode("CC"), brandInfo->brandCode());
    CPPUNIT_ASSERT_EQUAL(2, (int) brandInfo->primaryBookingCode().size());
    CPPUNIT_ASSERT_EQUAL(3, (int) brandInfo->secondaryBookingCode().size());
    CPPUNIT_ASSERT_EQUAL(BookingCode("Y"), brandInfo->primaryBookingCode()[0]);
    CPPUNIT_ASSERT_EQUAL(BookingCode("Z"), brandInfo->primaryBookingCode()[1]);
    CPPUNIT_ASSERT_EQUAL(BookingCode("F"), brandInfo->secondaryBookingCode()[0]);
    CPPUNIT_ASSERT_EQUAL(BookingCode("G"), brandInfo->secondaryBookingCode()[1]);
    CPPUNIT_ASSERT_EQUAL(BookingCode("H"), brandInfo->secondaryBookingCode()[2]);
  }

  void testResponseHasNoBookingCode()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> "
    "<ProgramID>896</ProgramID> </ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>275</CarrierFlightItemNum> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>3142</CarrierFlightItemNum> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();
    BrandProgram* brandProgram = brandPrograms.front();
    BrandInfo* brandInfo = brandProgram->brandsData().front();
    CPPUNIT_ASSERT(brandInfo->primaryBookingCode().empty());
    CPPUNIT_ASSERT(brandInfo->secondaryBookingCode().empty());
  }

  void testResponseHasFareBasisCode()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> "
    "<ProgramID>896</ProgramID> </ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> <FareBasisCodes> <FareBasisCode includeInd=\"true\"> "
    "RHKL</FareBasisCode> <FareBasisCode includeInd=\"false\">QZDAPU</FareBasisCode> </FareBasisCodes> "
    "</Brand> <Brand> <Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>275</CarrierFlightItemNum> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>3142</CarrierFlightItemNum> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();
    BrandProgram* brandProgram = brandPrograms.front();
    BrandInfo* brandInfo = brandProgram->brandsData().front();
    CPPUNIT_ASSERT_EQUAL(1, (int) brandInfo->includedFareBasisCode().size());
    CPPUNIT_ASSERT_EQUAL(1, (int) brandInfo->excludedFareBasisCode().size());
    CPPUNIT_ASSERT_EQUAL(FareBasisCode("RHKL"), brandInfo->includedFareBasisCode().front());
    CPPUNIT_ASSERT_EQUAL(FareBasisCode("QZDAPU"), brandInfo->excludedFareBasisCode().front());

  }

  void testResponseHasNoFareBasisCode()
  {

  std::string xmlResponse =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> <GetAirlineBrandsRS "
    "xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
    "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
    "xmlns:ns3=\"http://opentravel.org/common/v02\" "
    "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_01\" "
    "xmlns:ns5=\"http://stl.sabre.com/Merchandising/v0\"> <BrandingResponse> <ResponseSource "
    "pseudoCityCode=\"DQ73\" iataNumber=\"9999999\" clientID=\"FQ\" requestingCarrierGDS=\"1S\" "
    "geoLocation=\"LHR\" departmentCode=\"DQ73\" dutyCode=\"*\" functionCode=\"B\"></ResponseSource> "
    "<BrandingResults> <MarketResponse marketID=\"1\"> <ProgramIdList> "
    "<ProgramID>896</ProgramID> </ProgramIdList> <Carrier>BA</Carrier> <MarketCriteria direction=\"OT\"> "
    "<DepartureDate>2014-05-28</DepartureDate> <DepartureAirportCode>LHR</DepartureAirportCode> "
    "<ArrivalAirportCode>JNB</ArrivalAirportCode> </MarketCriteria> </MarketResponse> "
    "<CarrierBrandsData> <BrandProgram programID=\"896\"> <Vendor>MMGR</Vendor> "
    "<GlobalIndicator></GlobalIndicator> <ProgramCode>BD</ProgramCode> "
    "<ProgramName>DIAMOND</ProgramName> <SystemCode>H</SystemCode> <BrandsData> <Brand> "
    "<Identifier>3516</Identifier> <Code>CC</Code> <Name>Cricket Team Cape Cobras</Name> "
    "<Tier>1</Tier> <PrimaryFareIDTable>4090</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "</Brand> <Brand> <Identifier>3517</Identifier> <Code>CV</Code> "
    "<Name>Cricket Team Titans</Name> <Tier>2</Tier> <PrimaryFareIDTable>4091</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>275</CarrierFlightItemNum> "
    "</Brand> <Brand> <Identifier>3518</Identifier> <Code>CL</Code> "
    "<Name>Cricket Team LIONS</Name> <Tier>3</Tier> <PrimaryFareIDTable>4092</PrimaryFareIDTable> "
    "<SecondaryFareIDTable>0</SecondaryFareIDTable> "
    "<CarrierFlightItemNum>3142</CarrierFlightItemNum> "
    "</Brand> </BrandsData> </BrandProgram> </CarrierBrandsData> "
    "</BrandingResults> </BrandingResponse> </GetAirlineBrandsRS>";

    S8BrandingResponseParser s8BrandingResponseParser(*_trx);
    std::map<int, std::vector<FareMarket*> > marketIDFareMarketMap;
    IXMLUtils::stripnamespaces(xmlResponse);
    s8BrandingResponseParser.parse(xmlResponse);
    s8BrandingResponseParser.process(marketIDFareMarketMap);
    s8BrandingResponseParser.print();

    int marketID = 1;
    PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
        _trx->brandedMarketMap().find(marketID);
    MarketResponse* marketResponse = marketResponseMapBeg->second.front();
    MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();
    BrandProgram* brandProgram = brandPrograms.front();
    BrandInfo* brandInfo = brandProgram->brandsData().front();
    CPPUNIT_ASSERT(brandInfo->includedFareBasisCode().empty());
    CPPUNIT_ASSERT(brandInfo->excludedFareBasisCode().empty());
  }


  PricingTrx* _trx;
  TestMemHandle _memHandle;
  S8BrandingSecurity* _s8BrandingSecurity;
  SecurityInfo* _securityInfo;
  RuleExecution* _ruleExecution;
  MarketRule* _marketRule;
  FareMarket* _fm1;
  AirSeg* _seg1;
  const tse::Loc* sfo;
  const tse::Loc* dfw;
};
CPPUNIT_TEST_SUITE_REGISTRATION(S8BrandingResponseParserTest);
}
