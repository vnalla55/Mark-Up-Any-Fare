//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#include "Common/TseCodeTypes.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "DataModel/TicketingCxrDisplayRequest.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/STLTicketingCxrDisplayResponseFormatter.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

namespace tse
{
class STLTicketingCxrDisplayResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(STLTicketingCxrDisplayResponseFormatterTest);
  CPPUNIT_TEST(testTicketingDisplayResponse_PPR);
  CPPUNIT_TEST(testTicketingDisplayResponse_3PT);
  CPPUNIT_TEST(testTicketingDisplayResponse_STD);
  CPPUNIT_TEST(testTicketingDisplayResponseEmpty);
  CPPUNIT_TEST(testTicketingDisplayResponseVcxValidatingCxr);
  CPPUNIT_TEST(testTicketingDisplayResponseVcxGsa);
  CPPUNIT_TEST(testTicketingDisplayResponseVcxNeutral);
  CPPUNIT_TEST(testTicketingDisplayResponseVcxAll);
  CPPUNIT_TEST(testFormatCarriers_Empty);
  CPPUNIT_TEST(testFormatCarriers_AlphaOrder);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCxr_ElectronicReq);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCxr_ElectronicPref);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCxr_PaperPref);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCxr_NoCarriers);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCarrier_empty);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCarrier_ElectronicPref);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCarrier_ElectronicReq);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCarrier_PaperPref);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCarrier_PaperReq);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCarrier_AllTicketTypes);
  CPPUNIT_TEST(testFormatTicketingCxrValidatingCarrier_AlphaOrder);
  CPPUNIT_TEST(testFormatTicketingCxrGeneralSalesAgent_Empty);
  CPPUNIT_TEST(testFormatTicketingCxrGeneralSalesAgent_AlphaOrder);
  CPPUNIT_TEST(testFormatTicketingCxrNeutralValidatingCxr_Empty);
  CPPUNIT_TEST(testFormatTicketingCxrNeutralValidatingCxr_AlphaOrder);
  CPPUNIT_TEST_SUITE_END();

private:
  mutable TestMemHandle _memH;
  XMLConstruct* _construct;
  TicketingCxrDisplayTrx* _trx;
  TicketingCxrDisplayRequest* _request;
  STLTicketingCxrDisplayResponseFormatter* _formatter;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _construct = _memH.insert(new XMLConstruct);
    _request = _memH.insert(new TicketingCxrDisplayRequest);
    _trx = _memH.insert(new TicketingCxrDisplayTrx);
    _trx->getRequest()=_request;
    _formatter = _memH.insert(new STLTicketingCxrDisplayResponseFormatter);
  }

  void tearDown() { _memH.clear(); }

  //tests
  void testTicketingDisplayResponse_PPR()
  {
    std::string expected="<InterlineAgreementDisplay carrierName=\"AA\"><InterlineAgreements agreementType=\"PPR\"><Carrier code=\"5T\"/></InterlineAgreements></InterlineAgreementDisplay>";

    _trx->getRequest()->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    TicketingCxrDisplayResponse resp;
    resp.primeHost()="1S";
    resp.country()="US";
    resp.validatingCxr()="AA";
    InterlineAgreements& agmts = resp.interlineAgreements();
    agmts["PPR"].insert("5T");
    _trx->getResponse()=resp;
    _formatter->formatTicketingCxrDisplayResponse(*_construct, *_trx);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testTicketingDisplayResponse_3PT()
  {
    std::string expected="<InterlineAgreementDisplay carrierName=\"AA\"><InterlineAgreements agreementType=\"3PT\"><Carrier code=\"AX\"/></InterlineAgreements></InterlineAgreementDisplay>";

    _trx->getRequest()->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    TicketingCxrDisplayResponse resp;
    resp.primeHost()="1S";
    resp.country()="US";
    resp.validatingCxr()="AA";
    InterlineAgreements& agmts = resp.interlineAgreements();
    agmts["3PT"].insert("AX");
    _trx->getResponse()=resp;
    _formatter->formatTicketingCxrDisplayResponse(*_construct, *_trx);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testTicketingDisplayResponse_STD()
  {
    std::string expected="<InterlineAgreementDisplay carrierName=\"AA\"><InterlineAgreements agreementType=\"STD\"><Carrier code=\"3M\"/></InterlineAgreements></InterlineAgreementDisplay>";

    _trx->getRequest()->setRequestType(vcx::DISPLAY_INTERLINE_AGMT);
    TicketingCxrDisplayResponse resp;
    resp.primeHost()="1S";
    resp.country()="US";
    resp.validatingCxr()="AA";
    InterlineAgreements& agmts = resp.interlineAgreements();
    agmts["STD"].insert("3M");
    _trx->getResponse()=resp;
    _formatter->formatTicketingCxrDisplayResponse(*_construct, *_trx);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testTicketingDisplayResponseEmpty()
  {
    std::string expected =
        "<ValidatingCxrDisplay settlementPlanCode=\"ARC\" settlementPlanName=\"AIRLINE REPORTING CORPORATION\"/>";

    _trx->getRequest()->setRequestType(vcx::DISPLAY_VCXR);
    TicketingCxrDisplayResponse resp;
    resp.primeHost()="1S";
    resp.country()="US";
    ValidatingCxrDisplayMap& valCxrMap = resp.validatingCxrDisplayMap();
    TicketingCxrValidatingCxrDisplay tvcx;
    valCxrMap["ARC"]= tvcx;
    _trx->getResponse()=resp;
    _formatter->formatTicketingCxrDisplayResponse(*_construct, *_trx);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testTicketingDisplayResponseVcxValidatingCxr()
  {
    std::string expected =
        "<ValidatingCxrDisplay settlementPlanCode=\"ARC\" settlementPlanName=\"AIRLINE REPORTING CORPORATION\"><ValidatingCxrs ticketType=\"ETKTPREF\"><Carrier code=\"BB\"/></ValidatingCxrs></ValidatingCxrDisplay>";
    _trx->getRequest()->setRequestType(vcx::DISPLAY_VCXR);
    TicketingCxrDisplayResponse resp;
    resp.primeHost()="1S";
    resp.country()="US";
    ValidatingCxrDisplayMap& valCxrMap = resp.validatingCxrDisplayMap();
    valCxrMap["ARC"].validatingCarrierMap()[vcx::ETKT_PREF].insert("BB");
    _trx->getResponse()=resp;
    _formatter->formatTicketingCxrDisplayResponse(*_construct, *_trx);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testTicketingDisplayResponseVcxGsa()
  {
    std::string expected =
        "<ValidatingCxrDisplay settlementPlanCode=\"ARC\" settlementPlanName=\"AIRLINE REPORTING CORPORATION\"><GeneralSalesAgents carrierName=\"AA\"><Carrier code=\"BB\"/></GeneralSalesAgents></ValidatingCxrDisplay>";
    _trx->getRequest()->setRequestType(vcx::DISPLAY_VCXR);
    TicketingCxrDisplayResponse resp;
    resp.primeHost()="1S";
    resp.country()="US";
    ValidatingCxrDisplayMap& valCxrMap = resp.validatingCxrDisplayMap();
    valCxrMap["ARC"].generalSalesAgentMap()["AA"].insert("BB");
    _trx->getResponse()=resp;
    _formatter->formatTicketingCxrDisplayResponse(*_construct, *_trx);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testTicketingDisplayResponseVcxNeutral()
  {
    std::string expected =
        "<ValidatingCxrDisplay settlementPlanCode=\"ARC\" settlementPlanName=\"AIRLINE REPORTING CORPORATION\"><NeutralValidatingCxrs><Carrier code=\"AC\"/></NeutralValidatingCxrs></ValidatingCxrDisplay>";

    _trx->getRequest()->setRequestType(vcx::DISPLAY_VCXR);
    TicketingCxrDisplayResponse resp;
    resp.primeHost()="1S";
    resp.country()="US";
    ValidatingCxrDisplayMap& valCxrMap = resp.validatingCxrDisplayMap();
    valCxrMap["ARC"].neutralValidatingCxr().insert("AC");
    _trx->getResponse()=resp;
    _formatter->formatTicketingCxrDisplayResponse(*_construct, *_trx);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testTicketingDisplayResponseVcxAll()
  {
    std::string expected =
        "<ValidatingCxrDisplay settlementPlanCode=\"ARC\" settlementPlanName=\"AIRLINE REPORTING CORPORATION\"><ValidatingCxrs ticketType=\"ETKTPREF\"><Carrier code=\"BB\"/></ValidatingCxrs><GeneralSalesAgents carrierName=\"AA\"><Carrier code=\"BB\"/></GeneralSalesAgents><NeutralValidatingCxrs><Carrier code=\"AC\"/></NeutralValidatingCxrs></ValidatingCxrDisplay>";

    _trx->getRequest()->setRequestType(vcx::DISPLAY_VCXR);
    TicketingCxrDisplayResponse resp;
    resp.primeHost()="1S";
    resp.country()="US";
    ValidatingCxrDisplayMap& valCxrMap = resp.validatingCxrDisplayMap();
    valCxrMap["ARC"].validatingCarrierMap()[vcx::ETKT_PREF].insert("BB");
    valCxrMap["ARC"].generalSalesAgentMap()["AA"].insert("BB");
    valCxrMap["ARC"].neutralValidatingCxr().insert("AC");
    _trx->getResponse()=resp;
    _formatter->formatTicketingCxrDisplayResponse(*_construct, *_trx);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatCarriers_Empty()
  {
    const std::string expected = "";
    std::set<CarrierCode> carriers;
    _formatter->formatCarriers(*_construct, carriers);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatCarriers_AlphaOrder()
  {
    const std::string expected = "<Carrier code=\"8A\"/>"
                                 "<Carrier code=\"AA\"/>"
                                 "<Carrier code=\"N8\"/>"
                                 "<Carrier code=\"ZZ\"/>";
    std::set<CarrierCode> carriers;
    carriers.insert( "ZZ" );
    carriers.insert( "AA" );
    carriers.insert( "8A" );
    carriers.insert( "N8" );
    _formatter->formatCarriers(*_construct, carriers);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrValidatingCxr_ElectronicReq()
  {
    const vcx::TicketType ticketType = vcx::ETKT_REQ;
    std::set<CarrierCode> carriers;
    carriers.insert( "ER" );
    _formatter->formatTicketingCxrValidatingCxr(*_construct, ticketType, carriers);
    const std::string expected = "<ValidatingCxrs ticketType=\"ETKTREQ\">"
                                 "<Carrier code=\"ER\"/>"
                                 "</ValidatingCxrs>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrValidatingCxr_ElectronicPref()
  {
    const vcx::TicketType ticketType = vcx::ETKT_PREF;
    std::set<CarrierCode> carriers;
    carriers.insert( "EP" );
    _formatter->formatTicketingCxrValidatingCxr(*_construct, ticketType, carriers);
    const std::string expected = "<ValidatingCxrs ticketType=\"ETKTPREF\">"
                                 "<Carrier code=\"EP\"/>"
                                 "</ValidatingCxrs>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrValidatingCxr_PaperPref()
  {
    const vcx::TicketType ticketType = vcx::PAPER_TKT_PREF;
    std::set<CarrierCode> carriers;
    carriers.insert( "PP" );
    _formatter->formatTicketingCxrValidatingCxr(*_construct, ticketType, carriers);
    const std::string expected = "<ValidatingCxrs ticketType=\"PTKTPREF\">"
                                 "<Carrier code=\"PP\"/>"
                                 "</ValidatingCxrs>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  // This should never happen, but if it does, we want to make sure nothing bad will happen.
  void testFormatTicketingCxrValidatingCxr_NoCarriers()
  {
    const vcx::TicketType ticketType = vcx::ETKT_PREF;
    std::set<CarrierCode> carriers;
    _formatter->formatTicketingCxrValidatingCxr(*_construct, ticketType, carriers);
    const std::string expected = "<ValidatingCxrs ticketType=\"ETKTPREF\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrValidatingCarrier_empty()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    _formatter->formatTicketingCxrValidatingCarrier(*_construct, tcxDisplay);
    const std::string expected = "";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrValidatingCarrier_ElectronicPref()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_PREF].insert("AA");
    _formatter->formatTicketingCxrValidatingCarrier(*_construct, tcxDisplay);
    const std::string expected = "<ValidatingCxrs ticketType=\"ETKTPREF\">"
                                 "<Carrier code=\"AA\"/>"
                                 "</ValidatingCxrs>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrValidatingCarrier_ElectronicReq()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_REQ].insert("AA");
    _formatter->formatTicketingCxrValidatingCarrier(*_construct, tcxDisplay);
    const std::string expected = "<ValidatingCxrs ticketType=\"ETKTREQ\">"
                                 "<Carrier code=\"AA\"/>"
                                 "</ValidatingCxrs>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrValidatingCarrier_PaperPref()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    tcxDisplay.validatingCarrierMap()[vcx::PAPER_TKT_PREF].insert("AA");
    _formatter->formatTicketingCxrValidatingCarrier(*_construct, tcxDisplay);
    const std::string expected = "<ValidatingCxrs ticketType=\"PTKTPREF\">"
                                 "<Carrier code=\"AA\"/>"
                                 "</ValidatingCxrs>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  // A ticket type of paper required cannot be returned for a validating carrier.
  // See ValidatingCxrUtil::getTicketingMethod().
  // If this changes then this test should also change.
  void testFormatTicketingCxrValidatingCarrier_PaperReq()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    tcxDisplay.validatingCarrierMap()[vcx::PAPER_TKT_REQ].insert("AA");
    _formatter->formatTicketingCxrValidatingCarrier(*_construct, tcxDisplay);
    const std::string expected = "";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrValidatingCarrier_AllTicketTypes()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_PREF].insert("EP");
    tcxDisplay.validatingCarrierMap()[vcx::PAPER_TKT_PREF].insert("PP");
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_REQ].insert("ER");
    _formatter->formatTicketingCxrValidatingCarrier(*_construct, tcxDisplay);
    const std::string expected = "<ValidatingCxrs ticketType=\"ETKTREQ\">"
                                 "<Carrier code=\"ER\"/>"
                                 "</ValidatingCxrs>"
                                 "<ValidatingCxrs ticketType=\"ETKTPREF\">"
                                 "<Carrier code=\"EP\"/>"
                                 "</ValidatingCxrs>"
                                 "<ValidatingCxrs ticketType=\"PTKTPREF\">"
                                 "<Carrier code=\"PP\"/>"
                                 "</ValidatingCxrs>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrValidatingCarrier_AlphaOrder()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_PREF].insert("CC");
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_PREF].insert("BB");
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_PREF].insert("AA");
    tcxDisplay.validatingCarrierMap()[vcx::PAPER_TKT_PREF].insert("AA");
    tcxDisplay.validatingCarrierMap()[vcx::PAPER_TKT_PREF].insert("3U");
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_REQ].insert("RR");
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_REQ].insert("ZZ");
    tcxDisplay.validatingCarrierMap()[vcx::ETKT_REQ].insert("LL");
    _formatter->formatTicketingCxrValidatingCarrier(*_construct, tcxDisplay);
    const std::string expected = "<ValidatingCxrs ticketType=\"ETKTREQ\">"
                                 "<Carrier code=\"LL\"/>"
                                 "<Carrier code=\"RR\"/>"
                                 "<Carrier code=\"ZZ\"/>"
                                 "</ValidatingCxrs>"
                                 "<ValidatingCxrs ticketType=\"ETKTPREF\">"
                                 "<Carrier code=\"AA\"/>"
                                 "<Carrier code=\"BB\"/>"
                                 "<Carrier code=\"CC\"/>"
                                 "</ValidatingCxrs>"
                                 "<ValidatingCxrs ticketType=\"PTKTPREF\">"
                                 "<Carrier code=\"3U\"/>"
                                 "<Carrier code=\"AA\"/>"
                                 "</ValidatingCxrs>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrGeneralSalesAgent_Empty()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    _formatter->formatTicketingCxrGeneralSalesAgent(*_construct, tcxDisplay);
    const std::string expected = "";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrGeneralSalesAgent_AlphaOrder()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    const CarrierCode marketingCxr = "M1";
    tcxDisplay.generalSalesAgentMap()[marketingCxr].insert("ZZ");
    tcxDisplay.generalSalesAgentMap()[marketingCxr].insert("AA");
    tcxDisplay.generalSalesAgentMap()[marketingCxr].insert("2P");
    tcxDisplay.generalSalesAgentMap()[marketingCxr].insert("3Z");
    _formatter->formatTicketingCxrGeneralSalesAgent(*_construct, tcxDisplay);
    const std::string expected = "<GeneralSalesAgents carrierName=\"M1\">"
                                 "<Carrier code=\"2P\"/>"
                                 "<Carrier code=\"3Z\"/>"
                                 "<Carrier code=\"AA\"/>"
                                 "<Carrier code=\"ZZ\"/>"
                                 "</GeneralSalesAgents>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrNeutralValidatingCxr_Empty()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    _formatter->formatTicketingCxrNeutralValidatingCxr(*_construct, tcxDisplay);
    const std::string expected = "";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testFormatTicketingCxrNeutralValidatingCxr_AlphaOrder()
  {
    TicketingCxrValidatingCxrDisplay tcxDisplay;
    tcxDisplay.neutralValidatingCxr().insert("ZZ");
    tcxDisplay.neutralValidatingCxr().insert("AA");
    tcxDisplay.neutralValidatingCxr().insert("2P");
    tcxDisplay.neutralValidatingCxr().insert("3Z");
    _formatter->formatTicketingCxrNeutralValidatingCxr(*_construct, tcxDisplay);
    const std::string expected = "<NeutralValidatingCxrs>"
                                 "<Carrier code=\"2P\"/>"
                                 "<Carrier code=\"3Z\"/>"
                                 "<Carrier code=\"AA\"/>"
                                 "<Carrier code=\"ZZ\"/>"
                                 "</NeutralValidatingCxrs>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(STLTicketingCxrDisplayResponseFormatterTest);
}

