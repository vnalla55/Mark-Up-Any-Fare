#include <fstream>

#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "Xform/XformTaxXML.h"
#include "DBAccess/Currency.h"
#include "DBAccess/MultiTransport.h"
#include "DataModel/TaxTrx.h"

using namespace std;
using namespace boost::assign;

namespace tse
{

class Customer;
class Nation;
class Trx;

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

  MultiTransport* getMT(LocCode city, LocCode loc)
  {
    MultiTransport* ret = _memHandle.create<MultiTransport>();
    ret->multitranscity() = city;
    ret->multitransLoc() = loc;
    return ret;
  }

public:
  const std::vector<Customer*>& getCustomer(const PseudoCityCode& key)
  {
    std::vector<Customer*>& ret = *_memHandle.create<std::vector<Customer*> >();
    if (key == "MEX" || key == "DFW" || key == "HDQ" || key == "LHR" || key == "")
      return ret;

    return DataHandleMock::getCustomer(key);
  }

  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "")
      return 0;
    return DataHandleMock::getLoc(locCode, date);
  }

  const LocCode getMultiTransportCity(const LocCode& locCode)
  {
    if (locCode == "DFW")
      return "DFW";

    return DataHandleMock::getMultiTransportCity(locCode);
  }

  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    std::vector<MultiTransport*>& ret = *_memHandle.create<std::vector<MultiTransport*> >();
    if (locCode == "DFW")
    {
      ret += getMT("DFW", "DAL"), getMT("DFW", "DFW"), getMT("DFW", "QDF");
      return ret;
    }

    return DataHandleMock::getMultiTransportCity(locCode, carrierCode, tvlType, tvlDate);
  }
  const Nation* getNation(const NationCode& nationCode, const DateTime& date)
  {
    return DataHandleMock::getNation(nationCode, date);
  }

};
} // namespace

class XformTaxXMLTest : public CppUnit::TestFixture
{
  class MockXformTaxXML : public XformTaxXML
  {
  public:
    MockXformTaxXML(const std::string& name, tse::ConfigMan& config)
      : XformTaxXML(name, config), _parseOverwritten(false), _parse(false)
    {
    }

    void setParse(bool newParse)
    {
      _parse = newParse;
      _parseOverwritten = true;
    }

    virtual bool parse(const char*& content, DataHandle& dataHandle, Trx*& trx, bool throttle)
    {
      if (_parseOverwritten)
        return _parse;
      else
        return XformTaxXML::parse(content, dataHandle, trx, throttle);
    }

  private:
    bool _parseOverwritten;
    bool _parse;
  };

  CPPUNIT_TEST_SUITE(XformTaxXMLTest);

  CPPUNIT_TEST(testTaxTrx);
  CPPUNIT_TEST(testPFCTrx);
  CPPUNIT_TEST(testParsingReturnsNoTrx);
  CPPUNIT_TEST(testTaxInfoTrx);
  CPPUNIT_TEST(testTaxInfoTrxNoHeader);
  CPPUNIT_TEST(testNewTaxTrx);
  CPPUNIT_TEST(testNewTaxTrxNoHeader);
  CPPUNIT_TEST(testNewTaxTrxXmlHeader);
  CPPUNIT_TEST(testXrayTagParse);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    std::string serverName = "TestServer";
    _trx = 0;

    CPPUNIT_ASSERT(initializeConfig() == true);

    _aConfig.setValue("DEFAULT_CONFIG", "../taxRequest.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("DETAIL_CONFIG", "../detailXmlConfig.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("TAX_OTA_CONFIG", "../taxOTARequest.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("TAX_CONFIG", "../taxRequest.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("PFC_DISPLAY_CONFIG", "../pfcDisplayRequest.cfg", "DATAHANDLER_CONFIGS");
    _aConfig.setValue("TAX_INFO_CONFIG", "../taxInfoRequest.cfg", "DATAHANDLER_CONFIGS");

    _aConfig.setValue("TAX_DISPLAY_CONFIG", "../taxDisplayRequest.cfg", "DATAHANDLER_CONFIGS");

    _testServer = _memHandle.insert(new MockXformTaxXML(serverName, _aConfig));

    CPPUNIT_ASSERT(_testServer != 0);
    CPPUNIT_ASSERT(_testServer->initialize(0, (char**)0) == true);
  }

  void tearDown() { _memHandle.clear(); }

  //---------------------------------------------------------------------
  // tests
  //---------------------------------------------------------------------
  void testTaxTrx()
  {
    string requestXML(
        "XXXXXXXX<AirTaxRQ Version=\"2003A.TsabreXML1.0.2\"><POS><Source "
        "PseudoCityCode=\"UNKN\"><TPA_Extensions><UserInfo><Partition ID=\"S1\" /><Service "
        "Name=\"V2TAX\" /><AAACity Code=\"\" /><AgentSine Code=\"\" "
        "/></UserInfo></TPA_Extensions></Source></"
        "POS><ItineraryInfos><ItineraryInfo><ReservationItems><Item SalePseudoCityCode=\"DFW\" "
        "ValidatingCarrier=\"Y4\" TicketingCarrier=\"Y4\" RPH=\"1\"><FlightSegment "
        "ResBookDesigCode=\"J\" FlightNumber=\"755\" ArrivalDateTime=\"2013-03-22T07:00:00\" "
        "DepartureDateTime=\"2013-03-22T07:00:00\"><DepartureAirport LocationCode=\"MIA\" "
        "/><ArrivalAirport LocationCode=\"DFW\" /><Equipment AirEquipType=\"320\" "
        "/><MarketingAirline Code=\"Y4\" "
        "/></FlightSegment><AirFareInfo><PTC_FareBreakdown><PassengerType Code=\"ADT\" "
        "/><PassengerFare AncillaryServiceCode=\"TBBB\"><BaseFare CurrencyCode=\"USD\" "
        "Amount=\"100.0\" /><EquivFare CurrencyCode=\"USD\" Amount=\"0.0\" "
        "/></PassengerFare></PTC_FareBreakdown></AirFareInfo></Item></ReservationItems></"
        "ItineraryInfo></ItineraryInfos></AirTaxRQ>");
    _testServer->convert(_dataHandle, requestXML, _trx, false);
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(_trx);
    CPPUNIT_ASSERT(taxTrx != 0);
    CPPUNIT_ASSERT_EQUAL(std::string("2003A.TsabreXML1.0.2"), taxTrx->otaXmlVersion());
  }
  void testPFCTrx()
  {
    string requestXML("XXXXXXXX<PFCDisplayRequest><FRT S91=\"ATSEDBLD02A\" S92=\"22222\" /><AGI "
                      "A10=\"MIA\" A20=\"DFW\" A21=\"DFW\" AB0=\"9999999\" AB1=\"9999999\" "
                      "A90=\"YIJ\" N0G=\"*\" A80=\"80K2\" B00=\"1S\" C40=\"USD\" Q01=\"34\" />"
                      "<BIL "
                      "S0R=\"PSS\" A20=\"DFW\" Q03=\"925\" Q02=\"3470\" AE0=\"AA\" AD0=\"54B145\" "
                      "A22=\"DFW\" AA0=\"YIJ\" C20=\"INTDWPI1\" A70=\"PXC*L\" /><PRO PXT=\"C*\" "
                      "/><SGI Q0C=\"01\" A01=\"LAX\" /></PFCDisplayRequest>");
    _testServer->convert(_dataHandle, requestXML, _trx, false);
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(_trx);
    CPPUNIT_ASSERT(taxTrx != 0);
    CPPUNIT_ASSERT_EQUAL(PFC_DISPLAY_REQUEST, taxTrx->requestType());
  }
  void testParsingReturnsNoTrx()
  {
    _testServer->setParse(true);
    std::string request;
    _testServer->convert(_dataHandle, request, _trx, false);
  }
  void testTaxInfoTrx()
  {
    string requestXML(
        "Header"
        "<TaxInfoRequest>"
        "<AGI A10=\"WAW\" A21=\"WAW\" A80=\"WAW\" A90=\"GCM\" B00=\"1S\" C40=\"USD\" N0G=\"5\"/>"
        "<BIL A20=\"WAW\" A22=\"WAW\" A70=\"MISC+\" AA0=\"GCM\" AD0=\"12FC15\" AE0=\"AS\" "
             "C01=\"361471230821529348\" C20=\"TAXXINFO\" S0R=\"PSS\"/>"
        "<PRO D07=\"2014-04-16\" D54=\"1170\"/>"
        "<TAX BC0=\"XF\">"
        "<APT A01=\"SEA\"/>"
        "<APT A01=\"ANC\"/>"
        "</TAX>"
        "</TaxInfoRequest>");
    CPPUNIT_ASSERT(_testServer->convert(_dataHandle, requestXML, _trx, false));
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(_trx);
    CPPUNIT_ASSERT(taxTrx != 0);
    CPPUNIT_ASSERT_EQUAL(TAX_INFO_REQUEST, taxTrx->requestType());
  }
  void testTaxInfoTrxNoHeader()
  {
    string requestXML(
        "<TaxInfoRequest>"
        "<AGI A10=\"WAW\" A21=\"WAW\" A80=\"WAW\" A90=\"GCM\" B00=\"1S\" C40=\"USD\" N0G=\"5\"/>"
        "<BIL A20=\"WAW\" A22=\"WAW\" A70=\"MISC+\" AA0=\"GCM\" AD0=\"12FC15\" AE0=\"AS\" "
             "C01=\"361471230821529348\" C20=\"TAXXINFO\" S0R=\"PSS\"/>"
        "<PRO D07=\"2014-04-16\" D54=\"1170\"/>"
        "<TAX BC0=\"XF\">"
        "<APT A01=\"SEA\"/>"
        "<APT A01=\"ANC\"/>"
        "</TAX>"
        "</TaxInfoRequest>");
    CPPUNIT_ASSERT(_testServer->convert(_dataHandle, requestXML, _trx, false));
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(_trx);
    CPPUNIT_ASSERT(taxTrx != 0);
    CPPUNIT_ASSERT_EQUAL(TAX_INFO_REQUEST, taxTrx->requestType());
  }
  void testNewTaxTrx()
  {
    string requestXML(
        "Header"
        "<TaxRq>"
        "<SomeContent/>"
        "</TaxRq>");
    CPPUNIT_ASSERT(_testServer->convert(_dataHandle, requestXML, _trx, false));
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(_trx);
    CPPUNIT_ASSERT(taxTrx != 0);
    CPPUNIT_ASSERT_EQUAL(NEW_OTA_REQUEST, taxTrx->requestType());
  }
  void testNewTaxTrxNoHeader()
  {
    string requestXML(
        "<TAX>"
        "<SomeContent/>"
        "</TAX>");
    CPPUNIT_ASSERT(_testServer->convert(_dataHandle, requestXML, _trx, false));
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(_trx);
    CPPUNIT_ASSERT(taxTrx != 0);
    CPPUNIT_ASSERT_EQUAL(NEW_OTA_REQUEST, taxTrx->requestType());
  }
  void testNewTaxTrxXmlHeader() // it's not a requirement, but that's how it works
  {
    string requestXML(
        "XXXXXX"
        "<?xml header>"
        "<TAX>"
        "<SomeContent/>"
        "</TAX>");
    CPPUNIT_ASSERT(!_testServer->convert(_dataHandle, requestXML, _trx, false));
  }
  void testXrayTagParse()
  {
    TestConfigInitializer::setValue("FEATURE_ENABLE", "Y", "XRAY");

    string requestXML(
        "XXXXXXXX<AirTaxRQ Version=\"2003A.TsabreXML1.0.2\">"
        "<XRA MID=\"mid\" CID=\"cid\" /><POS><Source "
        "PseudoCityCode=\"UNKN\"><TPA_Extensions><UserInfo><Partition ID=\"S1\" /><Service "
        "Name=\"V2TAX\" /><AAACity Code=\"\" /><AgentSine Code=\"\" "
        "/></UserInfo></TPA_Extensions></Source></"
        "POS><ItineraryInfos><ItineraryInfo><ReservationItems><Item SalePseudoCityCode=\"DFW\" "
        "ValidatingCarrier=\"Y4\" TicketingCarrier=\"Y4\" RPH=\"1\"><FlightSegment "
        "ResBookDesigCode=\"J\" FlightNumber=\"755\" ArrivalDateTime=\"2013-03-22T07:00:00\" "
        "DepartureDateTime=\"2013-03-22T07:00:00\"><DepartureAirport LocationCode=\"MIA\" "
        "/><ArrivalAirport LocationCode=\"DFW\" /><Equipment AirEquipType=\"320\" "
        "/><MarketingAirline Code=\"Y4\" "
        "/></FlightSegment><AirFareInfo><PTC_FareBreakdown><PassengerType Code=\"ADT\" "
        "/><PassengerFare AncillaryServiceCode=\"TBBB\"><BaseFare CurrencyCode=\"USD\" "
        "Amount=\"100.0\" /><EquivFare CurrencyCode=\"USD\" Amount=\"0.0\" "
        "/></PassengerFare></PTC_FareBreakdown></AirFareInfo></Item></ReservationItems></"
        "ItineraryInfo></ItineraryInfos></AirTaxRQ>");
    _testServer->convert(_dataHandle, requestXML, _trx, false);
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(_trx);
    CPPUNIT_ASSERT(taxTrx != 0);
    CPPUNIT_ASSERT_EQUAL(std::string("mid"), taxTrx->getXrayJsonMessage()->getMid());
    CPPUNIT_ASSERT_EQUAL(std::string("cid"), taxTrx->getXrayJsonMessage()->getCid());
  }

private:
  std::string _cfgFileName;
  tse::ConfigMan _aConfig;
  MockXformTaxXML* _testServer;
  TestMemHandle _memHandle;
  DataHandle _dataHandle;
  Trx* _trx;

  bool initializeConfig()
  {
    _cfgFileName = "xmlConfig.cfg";

    if (!_aConfig.read(_cfgFileName))
    {
      return false; // failure
    }
    return true; // success
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(XformTaxXMLTest);
} // namespace tse
