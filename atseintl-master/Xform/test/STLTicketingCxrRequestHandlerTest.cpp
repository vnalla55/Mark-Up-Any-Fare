//----------------------------------------------------------------------------
//  Copyright Sabre 2014
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//----------------------------------------------------------------------------
#include "Xform/STLTicketingCxrRequestHandler.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TicketingCxrRequest.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

namespace tse
{
class STLTicketingCxrRequestHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(STLTicketingCxrRequestHandlerTest);
  CPPUNIT_TEST(testParseSettlementPlan);
  CPPUNIT_TEST(testParseValidatingCxr);
  CPPUNIT_TEST(testParseTicketType);
  CPPUNIT_TEST(testParseTicketDate);
  CPPUNIT_TEST(testParsePosCountry);
  CPPUNIT_TEST(testParseBillingInformation);
  CPPUNIT_TEST(testParser);
  CPPUNIT_TEST_SUITE_END();

private:
  STLTicketingCxrRequestHandler* _handler;
  TestMemHandle _memHandle;
  Trx* _trx;
  DataHandle _dataHandle;
  TicketingCxrRequest* _tcsReq;
  TicketingCxrTrx* _tcsTrx;
  DateTime _todayDate;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _tcsTrx = _memHandle.insert(new TicketingCxrTrx);
    _trx = 0;
    _handler = _memHandle.insert(new STLTicketingCxrRequestHandler(_trx));
    _tcsReq = _memHandle.insert(new TicketingCxrRequest);
    _todayDate = DateTime::localTime();
  }

  void tearDown() { _memHandle.clear(); }

  void testParseSettlementPlan()
  {
    std::string req = "<SettlementPlan>ABC</SettlementPlan>";
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, req));
    TicketingCxrTrx* trx = dynamic_cast<TicketingCxrTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(SettlementPlanType("ABC"), trx->getRequest()->getSettlementPlan());
  }
  void testParseValidatingCxr()
  {
    std::string req = "<ValidatingCxr>LH</ValidatingCxr>";
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, req));
    TicketingCxrTrx* trx = dynamic_cast<TicketingCxrTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LH"), trx->getRequest()->getValidatingCxr());
  }
  void testParseTicketType()
  {
    std::string req = "<TicketType>ETKTPREF</TicketType>";
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, req));
    TicketingCxrTrx* trx = dynamic_cast<TicketingCxrTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(vcx::TicketType(vcx::ETKT_PREF), trx->getRequest()->getTicketType());
  }
  void testParseTicketDate()
  {
    std::string req = "<TicketDate requestDate=\"2011-01-24\" requestTimeOfDay=\"1234\"/>";
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, req));
    TicketingCxrTrx* trx = dynamic_cast<TicketingCxrTrx*>(_trx);
    CPPUNIT_ASSERT_EQUAL(std::string("2011-Jan-24 20:34:00"),
                         trx->getRequest()->getTicketDate().toSimpleString());
  }
  void testParsePosCountry()
  {
    std::string req =
        "<POS company=\"AA\" duty=\"1\" lniata=\"ABCD02\" multiHost=\"1S\" sine=\"361567\">";
    req += "<Actual city=\"DFW\" country=\"US\" number=\"88856111\" province=\"TX\"/>";
    req += "<Home city=\"DFW\" country=\"US\" number=\"88856111\" province=\"TX\"/>";
    req += "<Pcc>80K2</Pcc>";
    req += "</POS>";
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, req));
    TicketingCxrTrx* trx = dynamic_cast<TicketingCxrTrx*>(_trx);
    CPPUNIT_ASSERT("US" == trx->getRequest()->getPosCountry());
    CPPUNIT_ASSERT("80K2" == trx->getRequest()->getPcc());
    CPPUNIT_ASSERT("1S" == trx->getRequest()->getMultiHost());
  }

  void testParseBillingInformation()
  {
    std::string req =
      "<BillingInformation actionCode=\"WTI\" businessFunction=\"XXX1\" parentServiceName=\"VALDTCXR\" parentTransactionID=\"1112\" sourceOfRequest=\"PPSS\" userBranch=\"XXX2\" userSetAddress=\"3D35F7\" userStation=\"0\"/>";
    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, req));
    TicketingCxrTrx* trx = dynamic_cast<TicketingCxrTrx*>(_trx);
    CPPUNIT_ASSERT(trx);
    CPPUNIT_ASSERT(trx->billing());

    Billing& billing = *trx->billing();
    CPPUNIT_ASSERT_EQUAL(std::string("WTI"), billing.actionCode());
    CPPUNIT_ASSERT_EQUAL(std::string("XXX1"), billing.clientServiceName());
    CPPUNIT_ASSERT_EQUAL(std::string("VALDTCXR"), billing.parentServiceName());
    CPPUNIT_ASSERT(1112 == billing.parentTransactionID());
    CPPUNIT_ASSERT_EQUAL(std::string("PPSS"), billing.requestPath());
    CPPUNIT_ASSERT_EQUAL(std::string("XXX2"), billing.userBranch());
    CPPUNIT_ASSERT_EQUAL(std::string("3D35F7"), billing.userSetAddress());
    CPPUNIT_ASSERT_EQUAL(std::string("0"), billing.userStation());
  }

  void testParser()
  {
    std::string req = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
    req += "<TicketingCxrServiceRQ version=\"0.0.0\" "
           "xmlns=\"http://stl.sabre.com/AirPricing/vcx/v0\">";
    req += "  <POS company=\"AA\" duty=\"1\" lniata=\"ABCD02\" multiHost=\"1S\" sine=\"361567\">";
    req += "    <Actual city=\"DFW\" country=\"US\" number=\"88856111\" province=\"TX\"/>";
    req += "    <Home city=\"DFW\" country=\"US\" number=\"88856111\" province=\"TX\"/>";
    req += "    <Pcc>80K2</Pcc>";
    req += "  </POS>";
    req += "  <SettlementPlan>ABC</SettlementPlan>";
    req += "  <ValidatingCxr>LH</ValidatingCxr>";
    req += "  <ParticipatingCxr>AA</ParticipatingCxr>";
    req += "  <ParticipatingCxr>AB</ParticipatingCxr>";
    req += "  <ParticipatingCxr>AC</ParticipatingCxr>";
    req += "  <TicketType>ETKTPREF</TicketType>";
    req += "  <TicketDate requestDate=\"2011-01-24\" requestTimeOfDay=\"1234\"/>";
    req += "</TicketingCxrServiceRQ>";

    CPPUNIT_ASSERT_NO_THROW(_handler->parse(_dataHandle, req));

    TicketingCxrTrx* trx = dynamic_cast<TicketingCxrTrx*>(_trx);
    CPPUNIT_ASSERT("US" == trx->getRequest()->getPosCountry());
    CPPUNIT_ASSERT("80K2" == trx->getRequest()->getPcc());
    CPPUNIT_ASSERT("1S" == trx->getRequest()->getMultiHost());
    CPPUNIT_ASSERT("ABC" == trx->getRequest()->getSettlementPlan());
    CPPUNIT_ASSERT("LH" == trx->getRequest()->getValidatingCxr());

    const std::vector<vcx::ParticipatingCxr>& crs = trx->getRequest()->participatingCxrs();
    CPPUNIT_ASSERT("AA" == crs[0].cxrName);
    CPPUNIT_ASSERT("AB" == crs[1].cxrName);
    CPPUNIT_ASSERT("AC" == crs[2].cxrName);

    CPPUNIT_ASSERT(vcx::ETKT_PREF == trx->getRequest()->getTicketType());
    CPPUNIT_ASSERT("2011-Jan-24 20:34:00" == trx->getRequest()->getTicketDate().toSimpleString());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(STLTicketingCxrRequestHandlerTest);
}
