// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/ErrorResponseException.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/SurfaceSeg.h"
#include "DBAccess/FareCalcConfig.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"
#include "Xform/BaggageResponseFormatter.h"

#define XMLNS BaggageResponseFormatter::XML_NAMESPACE_TEXT

namespace tse
{

using boost::assign::operator+=;

class BaggageResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageResponseFormatterTest);

  CPPUNIT_TEST(test_buildPXI_Without_Age);
  CPPUNIT_TEST(test_buildPXI_With_Age);
  CPPUNIT_TEST(test_buildPXI_With_Age_NonUsDot);
  CPPUNIT_TEST(test_buildDCL_Plain_Text);
  CPPUNIT_TEST(test_buildDCL_ADD_Tag_SATT);
  CPPUNIT_TEST(test_buildDCL_ADD_Tag_SATF);
  CPPUNIT_TEST(test_buildSEG);
  CPPUNIT_TEST(test_buildSEG_Empty_Allowance);
  CPPUNIT_TEST(test_buildSEG_Nil_Allowance);
  CPPUNIT_TEST(test_buildITN_One_Segment);
  CPPUNIT_TEST(test_buildITN_Two_Segments);
  CPPUNIT_TEST(test_buildITN_Surface_Segment);
  CPPUNIT_TEST(test_buildITN_No_Allowance);
  CPPUNIT_TEST(test_formatResponse_No_FarePath);
  CPPUNIT_TEST(test_formatResponse_One_FarePath);
  CPPUNIT_TEST(test_formatResponse_Two_FarePaths);
  CPPUNIT_TEST(test_formatResponse_Diag);
  CPPUNIT_TEST(test_formatResponse_Diag854);
  CPPUNIT_TEST(test_formatResponse_Diag_Error);
  CPPUNIT_TEST(test_buildErrorAndDiagnosticElements_DiagnosticNone);
  CPPUNIT_TEST(test_buildErrorAndDiagnosticElements_No_Data);
  CPPUNIT_TEST(test_buildErrorAndDiagnosticElements_Diag_Data);
  CPPUNIT_TEST(test_buildErrorAndDiagnosticElements_Diag854);
  CPPUNIT_TEST(test_buildErrorAndDiagnosticElements_Error_Code);
  CPPUNIT_TEST(test_buildErrorAndDiagnosticElements_Error_Code_Diag);
  CPPUNIT_TEST(test_buildErrorAndDiagnosticElements_Error_Code_Diag_Empty);
  CPPUNIT_TEST(test_buildQ00);
  CPPUNIT_TEST(test_buildFFY);

  CPPUNIT_TEST_SUITE_END();

private:
  BaggageResponseFormatter* _bgFormatter;
  XMLConstruct* _construct;
  BaggageTrx* _bgTrx;
  AncRequest* _ancRq;
  Itin* _itin;
  FarePath* _farePath;
  TestMemHandle _memH;
  Agent* _pAgent;
  Billing* _pBilling;

public:
  void setUp()
  {
    Message::fillMsgErrCodeMap();

    _memH.create<TestConfigInitializer>(); // base PricingResponseFormatter needs it
    _bgFormatter = _memH.create<BaggageResponseFormatter>();
    _construct = _memH.create<XMLConstruct>();

    _pAgent = _memH.insert(new Agent);
    _pBilling = _memH.insert(new Billing);

    _ancRq = _memH.create<AncRequest>();
    _ancRq->ticketingAgent() = _pAgent;
    _ancRq->majorSchemaVersion() = 1;
    _ancRq->minorSchemaVersion() = 0;
    _ancRq->revisionSchemaVersion() = 1;

    _itin = _memH.create<Itin>();
    _itin->setItinOrderNum(1);
    _itin->setBaggageTripType(BaggageTripType::TO_FROM_US);
    _farePath = _memH.create<FarePath>();
    _farePath->paxType() = createPaxType();
    _itin->farePath().push_back(_farePath);
    _farePath->itin() = _itin;

    _bgTrx = _memH.create<BaggageTrx>();
    _bgTrx->billing() = _pBilling;
    _bgTrx->itin().push_back(_itin);
    _bgTrx->diagnostic().diagnosticType() = DiagnosticNone;
    _bgTrx->setRequest(_ancRq);
    _bgTrx->setOptions(_memH.create<PricingOptions>());
    _bgTrx->fareCalcConfig() = _memH.create<FareCalcConfig>();

  }

  void tearDown() { _memH.clear(); }

private:
  AirSeg* createAirSeg(uint16_t order = 1)
  {
    AirSeg* airSeg = _memH.create<AirSeg>();
    airSeg->boardMultiCity() = "PAR";
    airSeg->origAirport() = "ORY";
    airSeg->offMultiCity() = "LON";
    airSeg->destAirport() = "LHR";
    airSeg->pnrSegment() = 123;
    airSeg->setMarketingCarrierCode("LH");
    airSeg->setOperatingCarrierCode("AA");
    airSeg->segmentOrder() = order;
    return airSeg;
  }

  PaxType* createPaxType(const Code<3>& paxTypeCode = "ADT", uint16_t age = 0, uint16_t number = 1)
  {
    PaxType* paxType = _memH.create<PaxType>();
    paxType->paxType() = paxTypeCode;
    paxType->age() = age;
    paxType->number() = number;
    return paxType;
  }

  void addTextToDiagnostic(const std::string& text)
  {
    _bgTrx->diagnostic().activate();
    DiagCollector* dc = _memH.insert(new DiagCollector(_bgTrx->diagnostic()));
    dc->activate();
    *dc << text;
    dc->flushMsg();
  }

  void test_buildPXI_With_Age()
  {
    FarePath farePath;
    farePath.paxType() = createPaxType("ADT", 10);
    farePath.itin() = _itin;

    _bgFormatter->buildPXI(*_construct, &farePath, *_bgTrx);
    const std::string expectedXML("<PXI B70=\"A10\" USI=\"T\" Q0U=\"1\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPXI_With_Age_NonUsDot()
  {
    FarePath farePath;
    farePath.paxType() = createPaxType("ADT", 10);
    farePath.itin() = _itin;
    _itin->setBaggageTripType(BaggageTripType::OTHER);

    _bgFormatter->buildPXI(*_construct, &farePath, *_bgTrx);
    const std::string expectedXML("<PXI B70=\"A10\" USI=\"F\" Q0U=\"1\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPXI_Without_Age()
  {
    FarePath farePath;
    farePath.paxType() = createPaxType();
    farePath.itin() = _itin;

    _bgFormatter->buildPXI(*_construct, &farePath, *_bgTrx);
    const std::string expectedXML("<PXI B70=\"ADT\" USI=\"T\" Q0U=\"1\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDCL_Plain_Text()
  {
    _farePath->baggageResponse() = "test message";

    _bgFormatter->buildDCL(*_construct, _farePath, *_bgTrx);
    const std::string expectedXML("<DCL><MSG N06=\"X\" Q0K=\"0\" S18=\"test message\"/></DCL>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDCL_ADD_Tag_SATT()
  {
    _farePath->baggageResponse() = "<ADD>";
    _ancRq->setSpecificAgencyText(true);

    _bgFormatter->buildDCL(*_construct, _farePath, *_bgTrx);
    const std::string expectedXML(
        "<DCL>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"ADDITIONAL ALLOWANCES AND/OR DISCOUNTS "
        "MAY APPLY\"/>"
        "</DCL>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildDCL_ADD_Tag_SATF()
  {
    _farePath->baggageResponse() = "<ADD>";
    _ancRq->setSpecificAgencyText(false);

    _bgFormatter->buildDCL(*_construct, _farePath, *_bgTrx);
    const std::string expectedXML("<DCL>"
                                  "<MSG N06=\"X\" Q0K=\"0\" S18=\"ADDITIONAL ALLOWANCES AND/OR "
                                  "DISCOUNTS MAY APPLY DEPENDING ON\"/>"
                                  "<MSG N06=\"X\" Q0K=\"0\" S18=\"FLYER-SPECIFIC FACTORS /E.G. "
                                  "FREQUENT FLYER STATUS/MILITARY/\"/>"
                                  "<MSG N06=\"X\" Q0K=\"0\" S18=\"CREDIT CARD FORM OF PAYMENT/EARLY "
                                  "PURCHASE OVER INTERNET,ETC./\"/>"
                                  "</DCL>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSEG()
  {
    const std::string allowance("01P");

    _bgFormatter->buildSEG(*_construct, createAirSeg(), allowance);
    const std::string expectedXML(
        "<SEG A11=\"PAR\" C6I=\"ORY\" A12=\"LON\" A02=\"LHR\" Q0Z=\"123\" "
        "N0D=\"P\" B20=\"01\" B00=\"LH\" B01=\"AA\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSEG_Empty_Allowance()
  {
    const std::string allowance("");

    _bgFormatter->buildSEG(*_construct, createAirSeg(), allowance);
    const std::string expectedXML(
        "<SEG A11=\"PAR\" C6I=\"ORY\" A12=\"LON\" A02=\"LHR\" Q0Z=\"123\" "
        "B00=\"LH\" B01=\"AA\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildSEG_Nil_Allowance()
  {
    const std::string allowance("NIL");

    _bgFormatter->buildSEG(*_construct, createAirSeg(), allowance);
    const std::string expectedXML(
        "<SEG A11=\"PAR\" C6I=\"ORY\" A12=\"LON\" A02=\"LHR\" Q0Z=\"123\" "
        "N0D=\" \" B20=\"NI\" B00=\"LH\" B01=\"AA\"/>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildITN_One_Segment()
  {
    _farePath->baggageResponse() = "TEST RESPONSE";

    AirSeg* airSeg = createAirSeg();
    _itin->travelSeg() += airSeg;
    _farePath->mutableBaggageAllowance().insert(std::make_pair(airSeg, "01P"));

    _bgFormatter->buildITN(*_construct, 1, _farePath, *_bgTrx);
    const std::string expectedXML(
        "<ITN Q00=\"1\">"
        "<PXI B70=\"ADT\" USI=\"T\" Q0U=\"1\"/>"
        "<DCL><MSG N06=\"X\" Q0K=\"0\" S18=\"TEST RESPONSE\"/></DCL>"
        "<SEG A11=\"PAR\" C6I=\"ORY\" A12=\"LON\" A02=\"LHR\" Q0Z=\"123\" "
        "N0D=\"P\" B20=\"01\" B00=\"LH\" B01=\"AA\"/>"
        "</ITN>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildITN_Two_Segments()
  {
    _farePath->baggageResponse() = "TEST RESPONSE";

    AirSeg* airSeg1 = createAirSeg(1);
    _itin->travelSeg() += airSeg1;
    _farePath->mutableBaggageAllowance().insert(std::make_pair(airSeg1, "01P"));

    AirSeg* airSeg2 = createAirSeg(2);
    _itin->travelSeg() += airSeg2;
    _farePath->mutableBaggageAllowance().insert(std::make_pair(airSeg2, "20K"));

    _bgFormatter->buildITN(*_construct, 1, _farePath, *_bgTrx);
    const std::string expectedXML(
        "<ITN Q00=\"1\">"
        "<PXI B70=\"ADT\" USI=\"T\" Q0U=\"1\"/>"
        "<DCL><MSG N06=\"X\" Q0K=\"0\" S18=\"TEST RESPONSE\"/></DCL>"
        "<SEG A11=\"PAR\" C6I=\"ORY\" A12=\"LON\" A02=\"LHR\" Q0Z=\"123\" "
        "N0D=\"P\" B20=\"01\" B00=\"LH\" B01=\"AA\"/>"
        "<SEG A11=\"PAR\" C6I=\"ORY\" A12=\"LON\" A02=\"LHR\" Q0Z=\"123\" "
        "N0D=\"K\" B20=\"20\" B00=\"LH\" B01=\"AA\"/>"
        "</ITN>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildITN_Surface_Segment()
  {
    _farePath->baggageResponse() = "TEST RESPONSE";

    SurfaceSeg surfaceSeg;
    _itin->travelSeg() += &surfaceSeg;
    AirSeg* airSeg = createAirSeg();
    _itin->travelSeg() += airSeg;
    _farePath->mutableBaggageAllowance().insert(std::make_pair(airSeg, "01P"));

    _bgFormatter->buildITN(*_construct, 1, _farePath, *_bgTrx);
    const std::string expectedXML(
        "<ITN Q00=\"1\">"
        "<PXI B70=\"ADT\" USI=\"T\" Q0U=\"1\"/>"
        "<DCL><MSG N06=\"X\" Q0K=\"0\" S18=\"TEST RESPONSE\"/></DCL>"
        "<SEG A11=\"PAR\" C6I=\"ORY\" A12=\"LON\" A02=\"LHR\" Q0Z=\"123\" "
        "N0D=\"P\" B20=\"01\" B00=\"LH\" B01=\"AA\"/>"
        "</ITN>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildITN_No_Allowance()
  {
    _farePath->baggageResponse() = "TEST RESPONSE";

    AirSeg* airSeg = createAirSeg();
    _itin->travelSeg() += airSeg;

    _bgFormatter->buildITN(*_construct, 1, _farePath, *_bgTrx);
    const std::string expectedXML(
        "<ITN Q00=\"1\">"
        "<PXI B70=\"ADT\" USI=\"T\" Q0U=\"1\"/>"
        "<DCL><MSG N06=\"X\" Q0K=\"0\" S18=\"TEST RESPONSE\"/></DCL>"
        "<SEG A11=\"PAR\" C6I=\"ORY\" A12=\"LON\" A02=\"LHR\" Q0Z=\"123\" B00=\"LH\" B01=\"AA\"/>"
        "</ITN>");
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatResponse_No_FarePath()
  {
    _itin->farePath().clear();

    const std::string expectedXML("<BaggageResponse xmlns=\"" + XMLNS + "\"/>");
    CPPUNIT_ASSERT_EQUAL(
        expectedXML, _bgFormatter->formatResponse("", *_bgTrx, ErrorResponseException::NO_ERROR));
  }

  void test_formatResponse_One_FarePath()
  {
    _farePath->baggageResponse() = "TEST RESPONSE";

    _ancRq->paxToOriginalItinMap().insert(std::make_pair(_farePath->paxType(), _itin));
    AirSeg* airSeg = createAirSeg();
    _itin->travelSeg() += airSeg;
    _farePath->mutableBaggageAllowance().insert(std::make_pair(airSeg, "01P"));

    const std::string expectedXML(
        "<BaggageResponse xmlns=\"" + XMLNS +
        "\">"
        "<ITN Q00=\"1\">"
        "<PXI B70=\"ADT\" USI=\"T\" Q0U=\"1\"/>"
        "<DCL>"
        "<MSG N06=\"X\" Q0K=\"0\" S18=\"TEST RESPONSE\"/>"
        "</DCL>"
        "<SEG A11=\"PAR\" C6I=\"ORY\" A12=\"LON\" A02=\"LHR\" Q0Z=\"123\" "
        "N0D=\"P\" B20=\"01\" B00=\"LH\" B01=\"AA\"/>"
        "</ITN>"
        "</BaggageResponse>");
    CPPUNIT_ASSERT_EQUAL(
        expectedXML, _bgFormatter->formatResponse("", *_bgTrx, ErrorResponseException::NO_ERROR));
  }

  void test_formatResponse_Two_FarePaths()
  {
    _itin->farePath().clear();

    Itin itin1;
    itin1.setItinOrderNum(1);
    FarePath farePath1;
    farePath1.paxType() = createPaxType("ADT", 0, 1);
    farePath1.itin() = &itin1;
    _itin->farePath() += &farePath1;
    _ancRq->paxToOriginalItinMap().insert(std::make_pair(farePath1.paxType(), &itin1));

    Itin itin2;
    itin2.setItinOrderNum(2);
    FarePath farePath2;
    farePath2.paxType() = createPaxType("INF", 0, 2);
    farePath2.itin() = &itin2;
    _itin->farePath() += &farePath2;
    _ancRq->paxToOriginalItinMap().insert(std::make_pair(farePath2.paxType(), &itin2));

    const std::string expectedXML(
        "<BaggageResponse xmlns=\"" + XMLNS +
        "\">"
        "<ITN Q00=\"1\"><PXI B70=\"ADT\" USI=\"F\" Q0U=\"1\"/><DCL/></ITN>"
        "<ITN Q00=\"2\"><PXI B70=\"INF\" USI=\"F\" Q0U=\"2\"/><DCL/></ITN>"
        "</BaggageResponse>");
    CPPUNIT_ASSERT_EQUAL(
        expectedXML, _bgFormatter->formatResponse("", *_bgTrx, ErrorResponseException::NO_ERROR));
  }

  void test_formatResponse_Diag()
  {
    _bgTrx->diagnostic().diagnosticType() = Diagnostic194;

    const std::string expectedXML(
        "<BaggageResponse xmlns=\"" + XMLNS +
        "\">"
        "<MSG N06=\"X\" Q0K=\"3\" S18=\"DIAGNOSTIC 194 RETURNED NO DATA\"/>"
        "</BaggageResponse>");
    CPPUNIT_ASSERT_EQUAL(
        expectedXML, _bgFormatter->formatResponse("", *_bgTrx, ErrorResponseException::NO_ERROR));
  }

  void test_formatResponse_Diag854()
  {
    _bgTrx->diagnostic().diagnosticType() = Diagnostic854;
    _ancRq->paxToOriginalItinMap().insert(std::make_pair(_farePath->paxType(), _itin));

    const std::string response(
        _bgFormatter->formatResponse("", *_bgTrx, ErrorResponseException::NO_ERROR));
    CPPUNIT_ASSERT(std::string::npos != response.find("<BaggageResponse xmlns=\"" + XMLNS + "\">"));
    CPPUNIT_ASSERT(std::string::npos !=
                   response.find("<MSG N06=\"X\" Q0K=\"3\" S18=\"HOSTNAME/PORT:"));
    CPPUNIT_ASSERT(std::string::npos != response.find("<MSG N06=\"X\" Q0K=\"3\" S18=\"BASELINE:"));
    CPPUNIT_ASSERT(std::string::npos != response.find("<MSG N06=\"X\" Q0K=\"3\" S18=\"DATABASE:"));
    CPPUNIT_ASSERT(std::string::npos !=
                   response.find("<ITN Q00=\"1\"><PXI B70=\"ADT\" USI=\"T\" Q0U=\"1\"/><DCL/></ITN>"
                                 "</BaggageResponse>"));
  }

  void test_formatResponse_Diag_Error()
  {
    _bgTrx->diagnostic().diagnosticType() = Diagnostic194;

    const std::string expectedXML("<BaggageResponse xmlns=\"" + XMLNS +
                                  "\">"
                                  "<MSG N06=\"E\" Q0K=\"38\"/>"
                                  "</BaggageResponse>");
    CPPUNIT_ASSERT_EQUAL(
        expectedXML,
        _bgFormatter->formatResponse("", *_bgTrx, ErrorResponseException::SYSTEM_ERROR));
  }

  void test_buildErrorAndDiagnosticElements_DiagnosticNone()
  {
    const std::string expectedXML("");
    _bgFormatter->buildErrorAndDiagnosticElements(
        "", *_bgTrx, ErrorResponseException::NO_ERROR, *_construct);
    CPPUNIT_ASSERT_EQUAL(std::string(), _construct->getXMLData());
  }

  void test_buildErrorAndDiagnosticElements_No_Data()
  {
    _bgTrx->diagnostic().diagnosticType() = Diagnostic194;

    const std::string expectedXML(
        "<MSG N06=\"X\" Q0K=\"3\" S18=\"DIAGNOSTIC 194 RETURNED NO DATA\"/>");
    _bgFormatter->buildErrorAndDiagnosticElements(
        "", *_bgTrx, ErrorResponseException::NO_ERROR, *_construct);
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildErrorAndDiagnosticElements_Diag_Data()
  {
    _bgTrx->diagnostic().diagnosticType() = Diagnostic194;
    addTextToDiagnostic("TEST DATA");

    const std::string expectedXML("<MSG N06=\"X\" Q0K=\"3\" S18=\"TEST DATA\"/>");
    _bgFormatter->buildErrorAndDiagnosticElements(
        "", *_bgTrx, ErrorResponseException::NO_ERROR, *_construct);
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildErrorAndDiagnosticElements_Diag854()
  {
    _bgTrx->diagnostic().diagnosticType() = Diagnostic854;

    _bgFormatter->buildErrorAndDiagnosticElements(
        "", *_bgTrx, ErrorResponseException::NO_ERROR, *_construct);
    CPPUNIT_ASSERT(std::string::npos !=
                   _construct->getXMLData().find("<MSG N06=\"X\" Q0K=\"3\" S18=\"HOSTNAME/PORT:"));
    CPPUNIT_ASSERT(std::string::npos !=
                   _construct->getXMLData().find("<MSG N06=\"X\" Q0K=\"3\" S18=\"BASELINE:"));
    CPPUNIT_ASSERT(std::string::npos !=
                   _construct->getXMLData().find("<MSG N06=\"X\" Q0K=\"3\" S18=\"DATABASE:"));
  }

  void test_buildErrorAndDiagnosticElements_Error_Code()
  {
    const std::string errorString("UNKNOWN ERROR");

    const std::string expectedXML("<MSG N06=\"E\" Q0K=\"0\" S18=\"UNKNOWN ERROR\"/>");
    _bgFormatter->buildErrorAndDiagnosticElements(
        errorString, *_bgTrx, ErrorResponseException::UNKNOWN_EXCEPTION, *_construct);
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildErrorAndDiagnosticElements_Error_Code_Diag()
  {
    const std::string errorString("UNKNOWN ERROR");
    _bgTrx->diagnostic().diagnosticType() = Diagnostic194;
    addTextToDiagnostic("TEST DATA");

    const std::string expectedXML("<MSG N06=\"X\" Q0K=\"3\" S18=\"TEST DATA\"/>"
                                  "<MSG N06=\"E\" Q0K=\"0\" S18=\"UNKNOWN ERROR\"/>");
    _bgFormatter->buildErrorAndDiagnosticElements(
        errorString, *_bgTrx, ErrorResponseException::UNKNOWN_EXCEPTION, *_construct);
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildErrorAndDiagnosticElements_Error_Code_Diag_Empty()
  {
    const std::string errorString("UNKNOWN ERROR");
    _bgTrx->diagnostic().diagnosticType() = Diagnostic194;
    addTextToDiagnostic("");

    const std::string expectedXML("<MSG N06=\"E\" Q0K=\"0\" S18=\"UNKNOWN ERROR\"/>");
    _bgFormatter->buildErrorAndDiagnosticElements(
        errorString, *_bgTrx, ErrorResponseException::UNKNOWN_EXCEPTION, *_construct);
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildQ00()
  {
    _bgFormatter->buildQ00(*_construct, 1);
    std::string expected = "<Q00>1</Q00>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildFFY()
  {
    std::set<int> segmentNumbers;
    segmentNumbers.insert(1);
    segmentNumbers.insert(2);

    BaggageResponseFormatter::FFInfo ffInfo("A", 3, "LO", segmentNumbers);

    _bgFormatter->buildFFY(*_construct, ffInfo);
    std::string expected = "<FFY BPT=\"A\" Q7D=\"3\" B00=\"LO\"><Q00>1</Q00><Q00>2</Q00></FFY>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaggageResponseFormatterTest);

} // tse
