//----------------------------------------------------------------------------
//  Copyright Sabre 2008
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

#include "test/include/CppUnitHelperMacros.h"

#include "Common/BaggageTripType.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FareCalcConsts.h"
#include "Xform/AltPricingResponseFormatter.h"
#include "Xform/PricingResponseXMLTags.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace tse;

class MockAltPricingResponseFormatter : public AltPricingResponseFormatter
{
  friend class AirPricingResponseFormatterTest;

public:
  void
  getPsgOptions(PricingTrx& trx, FareCalcCollector* fareCalcCollector, XMLConstruct& altConstruct)
  {
    AltPricingResponseFormatter::getPsgOptions(trx, fareCalcCollector, altConstruct);
  }

  void prepareOBFeesInfo(const PricingTrx& pricingTrx, XMLConstruct& construct)
  {
    AltPricingResponseFormatter::prepareOBFeesInfo(pricingTrx, construct);
  }
};

namespace
{
class FarePathBuilder
{
  FarePath* _farePath;
  TestMemHandle* _memHandle;

public:
  FarePathBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _farePath = _memHandle->create<FarePath>();
  }

  FarePathBuilder& withBaggageResponse(const std::string& baggageResponse)
  {
    _farePath->baggageResponse() = baggageResponse;
    return *this;
  }

  FarePath build() { return *_farePath; }
};

class CalcTotalsBuilder
{
  CalcTotals* _calcTotals;
  TestMemHandle* _memHandle;

public:
  CalcTotalsBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _calcTotals = _memHandle->create<CalcTotals>();
  }

  CalcTotalsBuilder& withFarePath(FarePath& farePath)
  {
    _calcTotals->farePath = &farePath;
    return *this;
  }

  CalcTotalsBuilder& withRequestedPaxType(const std::string& paxType)
  {
    _calcTotals->requestedPaxType = paxType;
    return *this;
  }

  CalcTotalsBuilder& withWpaInfo(int psgDetailRefNo, const std::string& wpnDetailResponse)
  {
    _calcTotals->wpaInfo.psgDetailRefNo = psgDetailRefNo;
    _calcTotals->wpaInfo.wpnDetailResponse = wpnDetailResponse;
    return *this;
  }

  CalcTotals build() { return *_calcTotals; }
};
}

namespace tse
{
class AltPricingResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AltPricingResponseFormatterTest);
  CPPUNIT_TEST(testPXXResponse);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_without_Itin);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_US_DOT_100);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_US_DOT_100_WPA);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_US_DOT_101);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_US_DOT_101_WPA);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_NON_US_DOT_100);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_NON_US_DOT_100_WPA);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_NON_US_DOT_101);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_NON_US_DOT_101_WPA);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_NON_US_DOT_110);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_NON_US_DOT_110_WPA);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_NON_US_DOT_111);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_NON_US_DOT_111_WPA);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_ISRTrue);
  CPPUNIT_TEST(testAddAdditionalPaxInfo_ISRFalse);
  CPPUNIT_TEST(testPrepareOBFeesInfo);
  CPPUNIT_TEST(testPrepareUncompressedResponseText_Baggage);
  CPPUNIT_TEST(testPrepareUncompressedResponseText_Baggage_MPax);
  CPPUNIT_TEST(testPrepareUncompressedResponseText_ServiceFeeTemplate);
  CPPUNIT_TEST_SUITE_END();

private:
  FareCalcCollector _fcc;
  PricingTrx* _trx;
  PaxType* _adt;
  PaxType* _cnn;
  PaxType* _neg;
  TestMemHandle _memHandle;
  AltPricingResponseFormatter* _formatter;
  XMLConstruct* _construct;
  PricingRequest* _pricingRequest;
  Agent* _agent;
  PricingOptions* _options;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _adt = getPaxType("ADT");
    _cnn = getPaxType("CNN");
    _neg = getPaxType("NEG");
    _trx = _memHandle.create<AltPricingTrx>();
    _pricingRequest = _memHandle.create<PricingRequest>();
    Agent* agent = _memHandle.create<Agent>();
    TrxUtil::enableAbacus();
    Customer* customer = _memHandle.create<Customer>();
    customer->crsCarrier() = "1B";
    customer->hostName() = "ABAC";
    agent->agentTJR() = customer;
    _pricingRequest->ticketingAgent() = agent;
    _trx->setRequest(_pricingRequest);

    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);

    _trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();
    _trx->fareCalcConfig()->warningMessages() = 'Y';

    Billing* billing = _memHandle.create<Billing>();
    _trx->billing() = billing;

    _formatter = _memHandle.create<AltPricingResponseFormatter>();
    _construct = _memHandle.create<XMLConstruct>();
  }

  void tearDown() { _memHandle.clear(); }

  PaxType* getPaxType(PaxTypeCode pax)
  {
    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->paxType() = pax;
    return paxType;
  }

  void setSchemaVersion(uint16_t major, uint16_t minor, uint16_t update)
  {
    PricingRequest* pricingRequest = _trx->getRequest();
    if (!pricingRequest)
      pricingRequest = _memHandle.create<PricingRequest>();

    pricingRequest->majorSchemaVersion() = major;
    pricingRequest->minorSchemaVersion() = minor;
    pricingRequest->revisionSchemaVersion() = update;

    _trx->setRequest(pricingRequest);
  }

  void setUsDot(bool isUsDot)
  {
    if (_trx->itin().empty())
      _trx->itin().push_back(_memHandle.create<Itin>());
    _trx->itin().front()->setBaggageTripType(isUsDot ? BaggageTripType::TO_FROM_US
                                                     : BaggageTripType::OTHER);
  }

  CalcTotals*
  getCalcTotals(PaxType* pax, int refNo, bool noMatch, std::string detailResponse = "Mock Response")
  {
    CalcTotals* calcTotals = _memHandle.create<CalcTotals>();

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->noMatchOption() = noMatch;
    farePath->paxType() = pax;

    calcTotals->farePath = farePath;
    calcTotals->wpaInfo.psgDetailRefNo = refNo;
    calcTotals->wpaInfo.wpnDetailResponse = detailResponse;
    calcTotals->requestedPaxType = pax->paxType();

    return calcTotals;
  }

  std::string getPXXResponse()
  {
    MockAltPricingResponseFormatter formatter;
    XMLConstruct construct;

    formatter.getPsgOptions(*_trx, &_fcc, construct);

    std::string data = construct.getXMLData();

    return data;
  }

  void testResponseString(std::string expected)
  {
    std::string response = getPXXResponse();

    CPPUNIT_ASSERT_EQUAL(expected, response);
  }

  void orderPaxInTrx(PaxType* p1, PaxType* p2, PaxType* p3)
  {
    p1->inputOrder() = 0;
    p2->inputOrder() = 1;
    p3->inputOrder() = 2;

    _trx->paxType().clear();

    _trx->paxType().push_back(p1);
    _trx->paxType().push_back(p2);
    _trx->paxType().push_back(p3);
  }

  void testPXXResponse()
  {
    orderPaxInTrx(_adt, _cnn, _neg);

    // test single passenger
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 1, false));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"1\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 2, false));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"2\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 3, true));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"3\" S81=\"3\"/></PXX>");

    _fcc.passengerCalcTotals().clear();

    // test multiple passengers
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 1, true));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"1\" S81=\"T\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 2, false));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"2\" S81=\"1\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 3, false));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"3\" S81=\"1\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 4, true));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"4\" S81=\"1/4\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 5, true));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"5\" S81=\"1/4/5\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 6, true));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"6\" S81=\"1/4-6\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 7, false));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"7\" S81=\"1/4-6\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 8, false));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"8\" S81=\"1/4-6\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 9, true));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"9\" S81=\"1/4-6/9\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_cnn, 10, true));
    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"9\" S81=\"1/4-6/9\"/><PXI B70=\"CNN\" "
                       "Q0S=\"10\" S81=\"T\"/></PXX>");

    _fcc.passengerCalcTotals().pop_back();
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_cnn, 10, false));
    testResponseString(
        "<PXX><PXI B70=\"ADT\" Q0S=\"9\" S81=\"1/4-6/9\"/><PXI B70=\"CNN\" Q0S=\"10\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_cnn, 14, false));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_cnn, 12, false));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_cnn, 13, true));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_cnn, 11, false));

    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"9\" S81=\"1/4-6/9\"/><PXI B70=\"CNN\" "
                       "Q0S=\"14\" S81=\"13\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_neg, 20, true));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_neg, 19, false));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_neg, 16, true));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_neg, 15, true));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_neg, 18, true));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_neg, 17, true));

    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"9\" S81=\"1/4-6/9\"/><PXI B70=\"CNN\" "
                       "Q0S=\"14\" S81=\"13\"/><PXI B70=\"NEG\" Q0S=\"20\" "
                       "S81=\"15-18/20\"/></PXX>");

    orderPaxInTrx(_cnn, _neg, _adt);
    testResponseString("<PXX><PXI B70=\"CNN\" Q0S=\"14\" S81=\"13\"/><PXI B70=\"NEG\" Q0S=\"20\" "
                       "S81=\"15-18/20\"/><PXI B70=\"ADT\" Q0S=\"9\" S81=\"1/4-6/9\"/></PXX>");

    orderPaxInTrx(_neg, _adt, _cnn);
    testResponseString("<PXX><PXI B70=\"NEG\" Q0S=\"20\" S81=\"15-18/20\"/><PXI B70=\"ADT\" "
                       "Q0S=\"9\" S81=\"1/4-6/9\"/><PXI B70=\"CNN\" Q0S=\"14\" S81=\"13\"/></PXX>");

    _fcc.passengerCalcTotals().clear();

    // test all match
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 3, true));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 1, true));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 5, true));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 4, true));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 2, true));

    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"5\" S81=\"T\"/></PXX>");

    _fcc.passengerCalcTotals().clear();

    // test no match
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 4, false));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 3, false));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 2, false));
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_adt, 1, false));

    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"4\"/></PXX>");

    // Add a pax with no fares (empty wpnDetailResponse)
    _fcc.passengerCalcTotals().push_back(getCalcTotals(_cnn, 1, false, ""));

    testResponseString("<PXX><PXI B70=\"ADT\" Q0S=\"4\"/><PXI B70=\"CNN\" Q0S=\"0\"/></PXX>");

    _fcc.passengerCalcTotals().push_back(getCalcTotals(_neg, 1, false, ""));
    testResponseString("<PXX><PXI B70=\"NEG\" Q0S=\"0\"/><PXI B70=\"ADT\" Q0S=\"4\"/><PXI "
                       "B70=\"CNN\" Q0S=\"0\"/></PXX>");
  }

  void testAddAdditionalPaxInfo_without_Itin()
  {
    setSchemaVersion(1, 0, 0);
    _trx->itin().erase(_trx->itin().begin(), _trx->itin().end());

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("S86=\"baggageResponse\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("USI="));
  }

  void testAddAdditionalPaxInfo_US_DOT_100()
  {
    setSchemaVersion(1, 0, 0);
    setUsDot(true);
    _trx->altTrxType() = PricingTrx::WP;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("S86=\"baggageResponse\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("USI=\"T\""));
  }

  void testAddAdditionalPaxInfo_US_DOT_100_WPA()
  {
    setSchemaVersion(1, 0, 0);
    setUsDot(true);
    _trx->altTrxType() = PricingTrx::WPA;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("S86=\"baggageResponse\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("USI=\"T\""));
  }

  void testAddAdditionalPaxInfo_US_DOT_101()
  {
    setSchemaVersion(1, 0, 1);
    setUsDot(true);
    _trx->altTrxType() = PricingTrx::WP;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("S86=\"baggageResponse\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("USI=\"T\""));
  }

  void testAddAdditionalPaxInfo_US_DOT_101_WPA()
  {
    setSchemaVersion(1, 0, 1);
    setUsDot(true);
    _trx->altTrxType() = PricingTrx::WPA;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("S86=\"baggageResponse\""));
    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"T\""));
  }

  void testAddAdditionalPaxInfo_NON_US_DOT_100()
  {
    setSchemaVersion(1, 0, 0);
    setUsDot(false);
    _trx->altTrxType() = PricingTrx::WP;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("S86=\"baggageResponse\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("USI=\"F\""));
  }

  void testAddAdditionalPaxInfo_NON_US_DOT_100_WPA()
  {
    setSchemaVersion(1, 0, 0);
    setUsDot(false);
    _trx->altTrxType() = PricingTrx::WPA;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("S86=\"baggageResponse\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("USI=\"F\""));
  }

  void testAddAdditionalPaxInfo_NON_US_DOT_101()
  {
    setSchemaVersion(1, 0, 1);
    setUsDot(false);
    _trx->altTrxType() = PricingTrx::WP;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos ==
                   _construct->getXMLData().find("S86=\"ATTN*baggageResponse\\n\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("USI=\"F\""));
  }

  void testAddAdditionalPaxInfo_NON_US_DOT_101_WPA()
  {
    setSchemaVersion(1, 0, 1);
    setUsDot(false);
    _trx->altTrxType() = PricingTrx::WPA;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos !=
                   _construct->getXMLData().find("S86=\"ATTN*baggageResponse\\n\""));
    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"F\""));
  }

  void testAddAdditionalPaxInfo_NON_US_DOT_110()
  {
    setSchemaVersion(1, 1, 0);
    setUsDot(false);
    _trx->altTrxType() = PricingTrx::WP;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos ==
                   _construct->getXMLData().find("S86=\"ATTN*baggageResponse\\n\""));
    CPPUNIT_ASSERT(std::string::npos == _construct->getXMLData().find("USI=\"F\""));
  }

  void testAddAdditionalPaxInfo_NON_US_DOT_110_WPA()
  {
    setSchemaVersion(1, 1, 0);
    setUsDot(false);
    _trx->altTrxType() = PricingTrx::WPA;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos !=
                   _construct->getXMLData().find("S86=\"ATTN*baggageResponse\\n\""));
    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"F\""));
  }

  void testAddAdditionalPaxInfo_NON_US_DOT_111()
  {
    setSchemaVersion(1, 1, 1);
    setUsDot(false);
    _trx->altTrxType() = PricingTrx::WP;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos !=
                   _construct->getXMLData().find("S86=\"ATTN*baggageResponse\\n\""));
    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"F\""));
  }

  void testAddAdditionalPaxInfo_NON_US_DOT_111_WPA()
  {
    setSchemaVersion(1, 1, 1);
    setUsDot(false);
    _trx->altTrxType() = PricingTrx::WPA;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle)
                               .withFarePath(farePath)
                               .withRequestedPaxType("AAA")
                               .withWpaInfo(3, "detailWpaInfo")
                               .build();
    uint16_t paxNumber = 1; // not used
    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    std::string expectedResult;

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("S84=\"detailWpaInfo\""));
    CPPUNIT_ASSERT(std::string::npos !=
                   _construct->getXMLData().find("S86=\"ATTN*baggageResponse\\n\""));
    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"F\""));
  }

  void testAddAdditionalPaxInfo_ISRTrue()
  {
    _trx->altTrxType() = PricingTrx::WPA;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    farePath.intlSurfaceTvlLimit() = true;
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle).withFarePath(farePath).build();

    uint16_t paxNumber = 1; // not used

    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("ISR=\"T\""));
  }

  void testAddAdditionalPaxInfo_ISRFalse()
  {
    _trx->altTrxType() = PricingTrx::WPA;

    FarePath farePath = FarePathBuilder(&_memHandle).withBaggageResponse("baggageResponse").build();
    farePath.intlSurfaceTvlLimit() = false;
    CalcTotals calcTotal = CalcTotalsBuilder(&_memHandle).withFarePath(farePath).build();

    uint16_t paxNumber = 1; // not used

    _construct->openElement(xml2::PassengerInfo);
    _formatter->addAdditionalPaxInfo(*_trx, calcTotal, paxNumber, *_construct);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("ISR=\"F\""));
  }

  void testPrepareOBFeesInfo()
  {
    PricingRequest req;
    req.formOfPayment() = "123456";
    req.formOfPaymentCard() = 'T';
    req.collectOBFee() = 'T';
    req.paymentAmountFop() = 20.20;
    req.chargeResidualInd() = true;
    _trx->setRequest(&req);

    MockAltPricingResponseFormatter formatter;
    formatter.prepareOBFeesInfo(*_trx, *_construct);

    CPPUNIT_ASSERT(_construct->getXMLData().find(xml2::FormOfPayment) != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find(xml2::CardChargeAmount) != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find(xml2::ResidualSpecifiedChargeAmount) !=
                   std::string::npos);
  }

  void testPrepareUncompressedResponseText_Baggage()
  {
    std::string rsp = "BASE FARE: USD666\n\n" + FreeBaggageUtil::BaggageTagHead + "001\n"
                                                                                  "AAA BBB CCC\n";
    _formatter->prepareUncompressedResponseText(*_trx, rsp);

    const std::string exp = "BASE FARE: USD666\n\nAAA BBB CCC\n";
    CPPUNIT_ASSERT_EQUAL(exp, rsp);
  }

  void testPrepareUncompressedResponseText_Baggage_MPax()
  {
    std::string rsp = "ABC\n\n"
                      "BAGGAGETEXT001\n"
                      "DEF\n\n"
                      "BAGGAGETEXT002\n"
                      "GHI\n";

    _formatter->prepareUncompressedResponseText(*_trx, rsp);

    const std::string exp = "ABC\n\nDEF\n\nGHI\n";
    CPPUNIT_ASSERT_EQUAL(exp, rsp);
  }

  void testPrepareUncompressedResponseText_ServiceFeeTemplate()
  {
    _options->setServiceFeesTemplateRequested();
    std::string rsp =
        "ABC\n" +
        FareCalcConsts::SERVICE_FEE_AMOUNT_LINE + "\n" +
        "XYZ\n" +
        FareCalcConsts::SERVICE_FEE_TOTAL_LINE + "\n" +
        "DEF\n";
    _formatter->prepareUncompressedResponseText(*_trx, rsp);

    const std::string exp = "ABC\nDEF\n";
    CPPUNIT_ASSERT_EQUAL(exp, rsp);
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(AltPricingResponseFormatterTest);
}

