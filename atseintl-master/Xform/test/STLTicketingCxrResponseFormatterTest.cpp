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
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TicketingCxrRequest.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/STLTicketingCxrResponseFormatter.h"
#include "Xform/XformUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

namespace tse
{
class STLTicketingCxrResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(STLTicketingCxrResponseFormatterTest);
  CPPUNIT_TEST(testPrepareValidationResult_Valid);
  CPPUNIT_TEST(testPrepareValidationResult_ValidSingleGSASwap);
  CPPUNIT_TEST(testPrepareValidationResult_ValidMultipleGSASwap);
  CPPUNIT_TEST(testPrepareValidationResult_NotValid);
  CPPUNIT_TEST(testPrepareTicketType_EtktPref);
  CPPUNIT_TEST(testPrepareTicketType_EtktReq);
  CPPUNIT_TEST(testPrepareTicketType_PaperTktPref);
  CPPUNIT_TEST(testPrepareTicketType_PaperTktReq);
  CPPUNIT_TEST(testPrepareMessage);
  CPPUNIT_TEST(testPrepareTrailerMessage_ValidMsg);
  CPPUNIT_TEST(testPrepareTrailerMessage_ValidOverrideMsg);
  CPPUNIT_TEST(testPrepareTrailerMessage_ValidSingleGSASwapMsg);
  CPPUNIT_TEST(testPrepareTrailerMessage_ValidMultipleGSASwapMsg);
  CPPUNIT_TEST(testPrepareTrailerMessage_AlternateValidCxrMsg);
  CPPUNIT_TEST(testPrepareTrailerMessage_OptionalValidMsg); // 5
  CPPUNIT_TEST(testPrepareTrailerMessage_RexAndOverrideInterlineErrMsg);
  CPPUNIT_TEST(testPrepareTrailerMessage_SettlementPlanOverrideErrMsg);
  CPPUNIT_TEST(testPrepareTrailerMessage_NoParticipationErrMsg); // 10
  CPPUNIT_TEST(testPrepareTrailerMessage_PaperTktOverrideErrMsg);
  CPPUNIT_TEST(testPrepareTrailerMessage_NoValidTktAgmtFoundErrMsg); // 13
  CPPUNIT_TEST_SUITE_END();

private:
  mutable TestMemHandle _memH;
  XMLConstruct* _construct;
  TicketingCxrTrx* _tcsTrx;
  TicketingCxrRequest* _tcsReq;
  STLTicketingCxrResponseFormatter* _formatter;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _construct = _memH.insert(new XMLConstruct);
    _tcsReq = _memH.insert(new TicketingCxrRequest);
    _tcsTrx = _memH.insert(new TicketingCxrTrx);
    _tcsTrx->setRequest(_tcsReq);
    _formatter = _memH.insert(new STLTicketingCxrResponseFormatter);
  }

  void tearDown() { _memH.clear(); }

  // Tests for ValidationResult
  void testPrepareValidationResult_Valid()
  {
    std::string res("<ValidationResult>Valid</ValidationResult>");
    _tcsTrx->setValidationResult(vcx::VALID);
    _formatter->prepareValidationResult(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareValidationResult_ValidSingleGSASwap()
  {
    std::string res("<ValidationResult>ValidSingleGSASwap</ValidationResult>");
    _tcsTrx->setValidationResult(vcx::VALID_SINGLE_GSA_SWAP);
    _formatter->prepareValidationResult(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareValidationResult_ValidMultipleGSASwap()
  {
    std::string res("<ValidationResult>ValidMultipleGSASwap</ValidationResult>");
    _tcsTrx->setValidationResult(vcx::VALID_MULTIPLE_GSA_SWAP);
    _formatter->prepareValidationResult(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareValidationResult_NotValid()
  {
    std::string res("<ValidationResult>NotValid</ValidationResult>");
    _tcsTrx->setValidationResult(vcx::NOT_VALID);
    _formatter->prepareValidationResult(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  //@todo Write test for system error.

  // Ticket Type
  void testPrepareTicketType_EtktPref()
  {
    std::string res("<TicketType>ETKTPREF</TicketType>");
    _tcsTrx->setTicketType(vcx::ETKT_PREF);
    _formatter->prepareTicketType(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareTicketType_EtktReq()
  {
    std::string res("<TicketType>ETKTREQ</TicketType>");
    _tcsTrx->setTicketType(vcx::ETKT_REQ);
    _formatter->prepareTicketType(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareTicketType_PaperTktPref()
  {
    std::string res("<TicketType>PTKTPREF</TicketType>");
    _tcsTrx->setTicketType(vcx::PAPER_TKT_PREF);
    _formatter->prepareTicketType(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareTicketType_PaperTktReq()
  {
    std::string res("<TicketType>PTKTREQ</TicketType>");
    _tcsTrx->setTicketType(vcx::PAPER_TKT_REQ);
    _formatter->prepareTicketType(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  ////

  void testPrepareMessage()
  {
    std::string res("<Message type=\"type\" code=\"9\">message</Message>");
    TicketingCxrResponseUtil::prepareMessage(*_construct, "type", 9, "message");
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(_construct->getXMLData(), res);
  }

  void testPrepareTrailerMessage_ValidMsg()
  {
    std::string res("<Message type=\"General\" code=\"1\">VALIDATING CARRIER - </Message>");
    _tcsTrx->setValidationStatus(vcx::VALID_MSG);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(_construct->getXMLData(), res);
  }
  void testPrepareTrailerMessage_ValidOverrideMsg()
  {
    std::string res(
        "<Message type=\"General\" code=\"2\">VALIDATING CARRIER SPECIFIED - </Message>");
    _tcsTrx->setValidationStatus(vcx::VALID_OVERRIDE);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(_construct->getXMLData(), res);
  }

  //@todo need to modify test for XX, ZZ  replacement
  void testPrepareTrailerMessage_ValidSingleGSASwapMsg()
  {
    std::string res("<Message type=\"General\" code=\"3\">VALIDATING CARRIER - |XX| PER GSA "
                    "AGREEMENT WITH |ZZ|</Message>");
    _tcsTrx->setValidationStatus(vcx::VALID_SINGLE_GSA);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(_construct->getXMLData(), res);
  }

  void testPrepareTrailerMessage_ValidMultipleGSASwapMsg()
  {
    std::string res("<Message type=\"General\" code=\"4\">VALIDATING CARRIER - </Message>");
    _tcsTrx->setValidationStatus(vcx::VALID_MULTIPLE_GSA);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(_construct->getXMLData(), res);
  }
  void testPrepareTrailerMessage_AlternateValidCxrMsg()
  {
    std::string res(
        "<Message type=\"General\" code=\"5\">ALTERNATE VALIDATING CARRIER/S - </Message>");
    _tcsTrx->setValidationStatus(vcx::ALTERNATE_CXR);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    std::cout << __LINE__ << ": " << _construct->getXMLData() << std::endl;
    CPPUNIT_ASSERT_EQUAL(_construct->getXMLData(), res);
  }
  void testPrepareTrailerMessage_OptionalValidMsg()
  {
    std::string res(
        "<Message type=\"General\" code=\"6\">OPTIONAL VALIDATING CARRIERS - </Message>");
    _tcsTrx->setValidationStatus(vcx::OPTIONAL_CXR);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(_construct->getXMLData(), res);
  }
  void testPrepareTrailerMessage_RexAndOverrideInterlineErrMsg()
  {
    std::string res("<Message type=\"General\" code=\"7\"> HAS NO INTERLINE TICKETING AGREEMENT "
                    "WITH </Message>");
    _tcsTrx->setValidationStatus(vcx::HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareTrailerMessage_SettlementPlanOverrideErrMsg()
  {
    std::string res("<Message type=\"General\" code=\"8\">INVALID SETTLEMENT METHOD FOR POINT OF SALE</Message>");
    _tcsTrx->setValidationStatus(vcx::INVALID_SETTLEMENTPLAN_ERR);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareTrailerMessage_NoParticipationErrMsg()
  {
    std::string res(
        "<Message type=\"General\" code=\"9\"> NOT VALID FOR SETTLEMENT METHOD</Message>");
    _tcsTrx->setValidationStatus(vcx::CXR_DOESNOT_PARTICIPATES_IN_SETTLEMENTPLAN);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareTrailerMessage_PaperTktOverrideErrMsg()
  {
    std::string res("<Message type=\"General\" code=\"10\">PAPER TICKET NOT PERMITTED</Message>");
    _tcsTrx->setValidationStatus(vcx::PAPER_TKT_OVERRIDE_ERR);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
  void testPrepareTrailerMessage_NoValidTktAgmtFoundErrMsg()
  {
    std::string res(
        "<Message type=\"General\" code=\"11\">NO VALID TICKETING AGREEMENTS FOUND</Message>");
    _tcsTrx->setValidationStatus(vcx::NO_VALID_TKT_AGMT_FOUND);
    _formatter->prepareTrailerMessage(*_tcsTrx, *_construct);
    CPPUNIT_ASSERT(_construct->isWellFormed());
    CPPUNIT_ASSERT_EQUAL(res, _construct->getXMLData());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(STLTicketingCxrResponseFormatterTest);
}
