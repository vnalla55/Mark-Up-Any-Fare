#include "Common/XMLConstruct.h"
#include "DSS/DSSRequest.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DBAccess/Customer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

#include <boost/regex.hpp>

#include <set>

namespace tse
{
class DSSRequestTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DSSRequestTest);
  CPPUNIT_TEST(testGetCarrierString);
  CPPUNIT_TEST(testAddCarrierListRecord);
  CPPUNIT_TEST(testSetWeb_Tvly);
  CPPUNIT_TEST(testSetWeb_NotTvly);
  CPPUNIT_TEST(testSetIIC_Tvly);
  CPPUNIT_TEST(testSetIIC_NotTvly);
  CPPUNIT_TEST(testSetIOD_Tvly);
  CPPUNIT_TEST(testSetIOD_NotTvly);
  CPPUNIT_TEST(testIsTravelocity_true);
  CPPUNIT_TEST(testIsTravelocity_NoCustomer);
  CPPUNIT_TEST(testIsTravelocity_NotTvly);
  CPPUNIT_TEST(testAddFlightCountRecord);
  CPPUNIT_TEST(testAddStopRecord);
  CPPUNIT_TEST(testAddSegmentRecord_Tvly);
  CPPUNIT_TEST(testAddSegmentRecord_NotTvly);
  CPPUNIT_TEST(testAddConnectTimeRecord);
  CPPUNIT_TEST(testAddEntryTypeRecord);
  CPPUNIT_TEST(testAddFlightRecord);
  CPPUNIT_TEST(testAddTimeRecord);
  CPPUNIT_TEST(testAddDateRecord_Tvly);
  CPPUNIT_TEST(testAddDateRecord_NotTvly);
  CPPUNIT_TEST(testAddDateRecord_NotTvly_EmptyDate);
  CPPUNIT_TEST(testAddTravelSegRecord);
  CPPUNIT_TEST(testAddAgentRecord_Tvly);
  CPPUNIT_TEST(testAddAgentRecord_NotTvly);
  CPPUNIT_TEST(testAddAgentRecord_NoCustomer);
  CPPUNIT_TEST(testAddBillingRecord);
  CPPUNIT_TEST(testBuild);
  CPPUNIT_TEST(testBuildXray);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetCarrierString()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("AA LO"), _dss->getCarrierString(_carrierList));
  }
  void testAddCarrierListRecord()
  {
    _dss->addCarrierListRecord(*_con, _carrierList);
    CPPUNIT_ASSERT_EQUAL(std::string("<CRR CRL=\"AA LO\"/>"), _con->getXMLData());
  }
  void testSetWeb_Tvly() { CPPUNIT_ASSERT_EQUAL(std::string("true"), _dss->setWEB(true)); }
  void testSetWeb_NotTvly() { CPPUNIT_ASSERT_EQUAL(std::string("false"), _dss->setWEB(false)); }
  void testSetIIC_Tvly() { CPPUNIT_ASSERT_EQUAL(std::string("true"), _dss->setIIC(true)); }
  void testSetIIC_NotTvly() { CPPUNIT_ASSERT_EQUAL(std::string("false"), _dss->setIIC(false)); }
  void testSetIOD_Tvly() { CPPUNIT_ASSERT_EQUAL(std::string("true"), _dss->setIOD(true)); }
  void testSetIOD_NotTvly() { CPPUNIT_ASSERT_EQUAL(std::string("false"), _dss->setIOD(false)); }
  void testIsTravelocity_true() { CPPUNIT_ASSERT_EQUAL(true, _dss->isTravelocity()); }
  void testIsTravelocity_NoCustomer()
  {
    _trx->getRequest()->ticketingAgent()->agentTJR() = 0;
    CPPUNIT_ASSERT_EQUAL(false, _dss->isTravelocity());
  }
  void testIsTravelocity_NotTvly()
  {
    _cus->tvlyLocation() = 'N';
    CPPUNIT_ASSERT_EQUAL(false, _dss->isTravelocity());
  }
  void testAddFlightCountRecord()
  {
    _dss->addFlightCountRecord(true, *_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<FCT IIC=\"true\" IOD=\"true\"/>"), _con->getXMLData());
  }
  void testAddStopRecord()
  {
    _dss->addStopRecord(*_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<STP TMN=\"0\" TMX=\"15\"/>"), _con->getXMLData());
  }
  void testAddSegmentRecord_Tvly()
  {
    _dss->addSegmentRecord(*_con, true);
    CPPUNIT_ASSERT_EQUAL(std::string("<SEG SMN=\"0\" SMX=\"3\"/>"), _con->getXMLData());
  }
  void testAddSegmentRecord_NotTvly()
  {
    _dss->addSegmentRecord(*_con, false);
    CPPUNIT_ASSERT_EQUAL(std::string("<SEG SMN=\"0\" SMX=\"2\"/>"), _con->getXMLData());
  }
  void testAddConnectTimeRecord()
  {
    _dss->addConnectTimeRecord(*_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<CNX CMN=\"0\" CMX=\"0\"/>"), _con->getXMLData());
  }
  void testAddEntryTypeRecord()
  {
    _dss->addEntryTypeRecord(*_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<ENT TYP=\"FD\"/>"), _con->getXMLData());
  }
  void testAddFlightRecord()
  {
    _dss->addFlightRecord(*_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<FLT SOL=\"100000\"/>"), _con->getXMLData());
  }
  void testAddTimeRecord()
  {
    _dss->addTimeRecord(*_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<TIM BK2=\"360\" OUT=\"1080\" TGT=\"360\"/>"),
                         _con->getXMLData());
  }
  void testAddDateRecord_Tvly()
  {
    _dss->addDateRecord(DateTime(2030, 4, 5, 12, 13, 0), *_req, *_opt, true, *_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<DAT BK1=\"2\" OUT=\"1\" TG1=\"2030-08-10\"/>"),
                         _con->getXMLData());
  }
  void testAddDateRecord_NotTvly()
  {
    _dss->addDateRecord(DateTime(2030, 4, 5, 12, 13, 0), *_req, *_opt, false, *_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<DAT BK1=\"0\" OUT=\"0\" TG1=\"2030-04-05\"/>"),
                         _con->getXMLData());
  }
  void testAddDateRecord_NotTvly_EmptyDate()
  {
    _dss->addDateRecord(DateTime::emptyDate(), *_req, *_opt, false, *_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<DAT BK1=\"0\" OUT=\"0\" TG1=\"2030-08-08\"/>"),
                         _con->getXMLData());
  }
  void testAddTravelSegRecord()
  {
    _dss->addTravelSegRecord(*createTS("AAA", "BBB", true, false), *_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<ODI BRD=\"AAA\" BTP=\"C\" OFF=\"BBB\" OTP=\"A\"/>"),
                         _con->getXMLData());
  }
  void testAddAgentRecord_Tvly()
  {
    _dss->addAgentRecord(*_req->ticketingAgent(), true, *_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<UID UDD=\"2\" OWN=\"UN\" CTY=\"BOM\" WEB=\"true\"/><AGI/>"),
                         _con->getXMLData());
  }
  void testAddAgentRecord_NotTvly()
  {
    _dss->addAgentRecord(*_req->ticketingAgent(), false, *_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<UID UDD=\"2\" OWN=\"UN\" CTY=\"BOM\" WEB=\"false\"/><AGI/>"),
                         _con->getXMLData());
  }
  void testAddAgentRecord_NoCustomer()
  {
    _req->ticketingAgent()->agentTJR() = 0;
    _dss->addAgentRecord(*_req->ticketingAgent(), true, *_con);
    CPPUNIT_ASSERT_EQUAL(std::string("<UID UDD=\"34\" OWN=\"UN\" CTY=\"BOM\" WEB=\"true\"/><AGI/>"),
                         _con->getXMLData());
  }
  void testAddBillingRecord()
  {
    _dss->addBillingRecord(*_trx->billing(), *_con);
    CPPUNIT_ASSERT_EQUAL(
        std::string("<BIL TXN=\"12345\" UCD=\"ADI\" AAA=\"MIL\" UST=\"SABR\" UBR=\"PIWO\" "
                    "ASI=\"SIN\" AKD=\"DRINK\" USA=\"WIELICKA\" PID=\"C:\" CSV=\"SGSCHEDS\"/>"),
        _con->getXMLData());
  }
  void testBuild()
  {
    std::string req = "";
    _dss->build(*_trx, _carrierList, req);
    CPPUNIT_ASSERT_EQUAL(
        std::string("<DSS><BIL TXN=\"12345\" UCD=\"ADI\" AAA=\"MIL\" UST=\"SABR\" UBR=\"PIWO\" "
                    "ASI=\"SIN\" AKD=\"DRINK\" USA=\"WIELICKA\" PID=\"C:\" CSV=\"SGSCHEDS\"/><UID "
                    "UDD=\"2\" OWN=\"UN\" CTY=\"BOM\" WEB=\"true\"/><AGI/><ODI BRD=\"WAW\" "
                    "BTP=\"A\" OFF=\"KRK\" OTP=\"C\"/><DAT BK1=\"2\" OUT=\"1\" "
                    "TG1=\"2030-08-10\"/><TIM BK2=\"360\" OUT=\"1080\" TGT=\"360\"/><FLT "
                    "SOL=\"100000\"/><ENT TYP=\"FD\"/><CNX CMN=\"0\" CMX=\"0\"/><SEG SMN=\"0\" "
                    "SMX=\"3\"/><STP TMN=\"0\" TMX=\"15\"/><CRR CRL=\"AA LO\"/><FCT IIC=\"true\" "
                    "IOD=\"true\"/></DSS>"),
        req);
  }

  void testBuildXray()
  {
    _trx->assignXrayJsonMessage(xray::JsonMessagePtr(new xray::JsonMessage("mid", "cid")));
    _trx->getXrayJsonMessage()->setId("id");
    std::string req = "";
    _dss->build(*_trx, _carrierList, req);
    CPPUNIT_ASSERT(boost::regex_search(
        req,
        boost::regex("<DSS><XRA MID=\"[a-zA-Z0-9]+\" CID=\"cid.id\"/><BIL TXN=\"12345\" "
                     "UCD=\"ADI\" AAA=\"MIL\" UST=\"SABR\" UBR=\"PIWO\" "
                     "ASI=\"SIN\" AKD=\"DRINK\" USA=\"WIELICKA\" PID=\"C:\" CSV=\"SGSCHEDS\"/><UID "
                     "UDD=\"2\" OWN=\"UN\" CTY=\"BOM\" WEB=\"true\"/><AGI/><ODI BRD=\"WAW\" "
                     "BTP=\"A\" OFF=\"KRK\" OTP=\"C\"/><DAT BK1=\"2\" OUT=\"1\" "
                     "TG1=\"2030-08-10\"/><TIM BK2=\"360\" OUT=\"1080\" TGT=\"360\"/><FLT "
                     "SOL=\"100000\"/><ENT TYP=\"FD\"/><CNX CMN=\"0\" CMX=\"0\"/><SEG SMN=\"0\" "
                     "SMX=\"3\"/><STP TMN=\"0\" TMX=\"15\"/><CRR CRL=\"AA LO\"/><FCT IIC=\"true\" "
                     "IOD=\"true\"/></DSS>")));
  }
  TravelSeg* createTS(const char* orig, const char* dest, bool origCity, bool destCity)
  {
    TravelSeg* seg = _memHandle.create<AirSeg>();
    Loc* l = _memHandle.create<Loc>();
    l->loc() = orig;
    l->cityInd() = origCity;
    seg->origin() = l;
    l = _memHandle.create<Loc>();
    l->loc() = dest;
    l->cityInd() = destCity;
    seg->destination() = l;
    seg->departureDT() = DateTime(2035, 12, 24, 16, 20, 0);
    return seg;
  }
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _carrierList.insert("AA");
    _carrierList.insert("LO");
    _carrierList.insert("YY");
    _trx = _memHandle.create<FareDisplayTrx>();
    Itin* it = _memHandle.create<Itin>();
    _trx->itin().push_back(it);
    _trx->travelSeg().push_back(createTS("WAW", "KRK", false, true));
    it->geoTravelType() = GeoTravelType::International;
    _req = _memHandle.create<FareDisplayRequest>();
    _trx->setRequest(_req);
    _opt = _memHandle.create<FareDisplayOptions>();
    _trx->setOptions(_opt);
    Agent* ag = _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    ag->cxrCode() = "UN";
    ag->agentCity() = "BOM";
    _cus = _trx->getRequest()->ticketingAgent()->agentTJR() = _memHandle.create<Customer>();
    _cus->tvlyLocation() = 'Y';
    _cus->ssgGroupNo() = 2;
    _req->preferredTravelDate() = DateTime(2030, 8, 10, 12, 20, 0);
    _req->dateRangeLower() = DateTime(2030, 8, 8, 10, 0, 0);
    _req->dateRangeUpper() = DateTime(2030, 8, 12, 9, 0, 0);
    Billing* b = _trx->billing() = _memHandle.create<Billing>();
    b->userPseudoCityCode() = "ADI";
    b->aaaCity() = "MIL";
    b->userStation() = "SABR";
    b->userBranch() = "PIWO";
    b->aaaSine() = "SIN";
    b->actionCode() = "DRINK";
    b->userSetAddress() = "WIELICKA";
    b->partitionID() = "C:";
    _dss = _memHandle.insert(new DSSRequest(*_trx));
    _con = _memHandle.create<XMLConstruct>();
  }
  void tearDown()
  {
    _carrierList.clear();
    _memHandle.clear();
  }

private:
  std::set<CarrierCode> _carrierList;
  TestMemHandle _memHandle;
  FareDisplayTrx* _trx;
  FareDisplayRequest* _req;
  FareDisplayOptions* _opt;
  DSSRequest* _dss;
  XMLConstruct* _con;
  Customer* _cus;
};
CPPUNIT_TEST_SUITE_REGISTRATION(DSSRequestTest);
}
