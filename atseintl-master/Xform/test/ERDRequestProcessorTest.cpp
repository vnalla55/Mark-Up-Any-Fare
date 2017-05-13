//----------------------------------------------------------------------------
//
//      File: ERDRequestProcessorTest.cpp
//      Description: Unit test for ERDRequestProcessor class
//      Created: October 10, 2008
//      Authors: Konrad Koch
//
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

#include "Common/NonFatalErrorResponseException.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ERDFareComp.h"
#include "DataModel/ERDFltSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DBAccess/Customer.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Xform/ERDRequestProcessor.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<Customer*>& getCustomer(const PseudoCityCode& key)
  {
    std::vector<Customer*>* ret = _memHandle.create<std::vector<Customer*> >();
    Customer* c = _memHandle.create<Customer>();
    ret->push_back(c);
    c->pseudoCity() = key;
    c->arcNo() = "9999999";
    c->lbtCustomerGroup() = "9999999";
    c->branchAccInd() = 'N';
    c->curConvInd() = 'N';
    c->cadSubscriberInd() = 'N';
    c->webSubscriberInd() = 'N';
    c->btsSubscriberInd() = 'N';
    c->sellingFareInd() = 'N';
    c->tvlyInternetSubriber() = 'N';
    c->availabilityIgRul2St() = 'Y';
    c->availabilityIgRul3St() = 'Y';
    c->availIgRul2StWpnc() = 'Y';
    c->activateJourneyPricing() = 'Y';
    c->activateJourneyShopping() = 'Y';
    c->doNotApplySegmentFee() = 'N';
    c->optInAgency() = 'Y';
    c->privateFareInd() = 'Y';
    c->doNotApplyObTktFees() = 'N';
    if (key == "5KAD")
    {
      c->homePseudoCity() = "5KAD";
      c->homeArcNo() = "9999999";
      c->requestCity() = "SIN";
      c->aaCity() = "SIN";
      c->defaultCur() = "SGD";
      c->agencyName() = "FARES AND PRICING 1";
      c->channelId() = 'T';
      c->crsCarrier() = "1B";
      c->ssgGroupNo() = 16;
      c->eTicketCapable() = 'N';
      c->hostName() = "ABAC";
      c->fareQuoteCur() = 'Y';
      return *ret;
    }
    else if (key == "W0H3")
    {
      c->homePseudoCity() = "W0H3";
      c->homeArcNo() = "4553728";
      c->requestCity() = "DFW";
      c->aaCity() = "SAT";
      c->defaultCur() = "USD";
      c->agencyName() = "TRAVELOCITY.COM";
      c->channelId() = 'N';
      c->crsCarrier() = "1S";
      c->ssgGroupNo() = 9;
      c->eTicketCapable() = 'Y';
      c->hostName() = "SABR";
      c->fareQuoteCur() = 'N';
      return *ret;
    }
    return DataHandleMock::getCustomer(key);
  }
  const std::vector<FareCalcConfig*>& getFareCalcConfig(const Indicator& userApplType,
                                                        const UserApplCode& userAppl,
                                                        const PseudoCityCode& pseudoCity)
  {
    if (pseudoCity == "5KAD" || pseudoCity == "W0H3")
      return DataHandleMock::getFareCalcConfig('C', "", "");
    return DataHandleMock::getFareCalcConfig(userApplType, userAppl, pseudoCity);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "WAW")
      return "WAW";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "")
      return 0;
    return DataHandleMock::getLoc(locCode, date);
  }
};
}
class ERDRequestProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ERDRequestProcessorTest);
  CPPUNIT_TEST(testInvalidRequest);
  CPPUNIT_TEST(testPassBasicRequest);
  CPPUNIT_TEST(testFailBasicRequest);
  CPPUNIT_TEST(testPassFareBasisRequest);
  CPPUNIT_TEST(testFailFareBasisRequest);
  CPPUNIT_TEST(testPassSegmentRequest);
  CPPUNIT_TEST(testFailSegmentRequest);
  CPPUNIT_TEST(testPassFareBasisSegmentRequest);
  CPPUNIT_TEST(testFailFareBasisSegmentRequest);
  CPPUNIT_TEST(testPassPtcRequestSabre);
  CPPUNIT_TEST(testFailPtcRequestSabre);
  CPPUNIT_TEST(testFailPtcRequestSabre2);
  CPPUNIT_TEST(testPassPtcRequestAbacus);
  CPPUNIT_TEST(testFailPtcRequestAbacus2);
  CPPUNIT_TEST(testPassCategories);
  CPPUNIT_TEST(testFailCategories);
  CPPUNIT_TEST(testPassCategoriesForERDFromSWS);
  CPPUNIT_TEST(testPassMenu);
  CPPUNIT_TEST(testPassHeader);
  CPPUNIT_TEST(testPassRouting);
  CPPUNIT_TEST(testPassIc);
  CPPUNIT_TEST(testFailMenuAndHeader);
  CPPUNIT_TEST(testFailHeaderAndRouting);
  CPPUNIT_TEST(testFailIcAndRouting);
  CPPUNIT_TEST(testFailMenuAndCategories);
  CPPUNIT_TEST(testFailHeaderAndAlphaCodes);
  CPPUNIT_TEST(testFailMenuAndMx);
  CPPUNIT_TEST(testPassSimilarChildTypes);
  CPPUNIT_TEST(testPassDtsSection);
  CPPUNIT_TEST(testPassDtsSectionNoLineRequested);
  CPPUNIT_TEST(testFailDtsSectionInvalidLine);
  CPPUNIT_TEST(testFailInvalidDtsSection);
  CPPUNIT_TEST(testFailEmptyDtsSection);
  CPPUNIT_TEST(testFailLineRequestedInEmptyDtsSection);
  CPPUNIT_TEST(testPassFareBreaks);
  CPPUNIT_TEST(testFailFareBreaks);
  CPPUNIT_TEST(testIsWqrd);
  CPPUNIT_TEST(testErrorFromPSS);
  CPPUNIT_TEST(testArnkInWqrd);
  CPPUNIT_TEST(testArnkInWprd);
  CPPUNIT_TEST(testAllSegmentsAreArnks);
  CPPUNIT_TEST(testInternationalRoundingCalculation1);
  CPPUNIT_TEST(testInternationalRoundingCalculation2);
  CPPUNIT_TEST(testInternationalRoundingCalculation3);
  CPPUNIT_TEST(testInternationalRoundingCalculation4);
  CPPUNIT_TEST(testInternationalRoundingCalculation5);
  CPPUNIT_TEST(testInternationalRoundingCalculationChild);
  CPPUNIT_TEST(testInternationalRoundingCalculationRussiaPoland);
  CPPUNIT_TEST(testInternationalRoundingCalculationRussiaRussia);
  CPPUNIT_TEST_SUITE_END();

private:
  ERDRequestProcessor* _processor;
  FareDisplayTrx* _trx;
  Itin* _itin;
  FareDisplayOptions* _options;
  FareDisplayRequest* _request;
  TestMemHandle _memHandle;

  ERDFareComp* createFareComponent(const CarrierCode& carrier,
                                   const FareClassCode& fareBasis,
                                   const FareClassCode& fareClass,
                                   const LocCode& departureAirport,
                                   const LocCode& arrivalAirport,
                                   const MoneyAmount fareAmount,
                                   const CurrencyCode& fareCurrency,
                                   const PaxTypeCode& paxTypeCode,
                                   uint16_t paxTypeNumber)
  {
    ERDFareComp* erdFareComp = _memHandle.create<ERDFareComp>();

    if (erdFareComp)
    {
      erdFareComp->vendor() = "ATP";
      erdFareComp->trueGoverningCarrier() = carrier;
      erdFareComp->governingCarrier() = carrier;
      erdFareComp->fareBasis() = fareBasis;
      erdFareComp->fareClass() = fareClass;
      erdFareComp->departureAirport() = departureAirport;
      erdFareComp->arrivalAirport() = arrivalAirport;
      erdFareComp->fareAmount() = fareAmount;
      erdFareComp->fareCurrency() = fareCurrency;
      erdFareComp->paxTypeCode() = paxTypeCode;
      erdFareComp->requestedPaxTypeCode() = paxTypeCode;
      erdFareComp->paxTypeNumber() = paxTypeNumber;
    }
    _trx->getOptions()->erdFareComps().push_back(erdFareComp);

    return erdFareComp;
  }

  ERDFltSeg* createFlightSegment(ERDFareComp* erdFareComp,
                                 uint16_t itinSegNumber,
                                 GeoTravelType geoTravelType = GeoTravelType::ForeignDomestic)
  {
    if (!erdFareComp)
      return 0;

    ERDFltSeg* erdFltSeg = _memHandle.create<ERDFltSeg>();

    if (erdFltSeg)
    {
      erdFltSeg->fareComponent() = erdFareComp;
      erdFltSeg->itinSegNumber() = itinSegNumber;
      erdFltSeg->geoTravelType() = geoTravelType;
      erdFltSeg->departureAirport() = erdFareComp->departureAirport();
      erdFltSeg->arrivalAirport() = erdFareComp->arrivalAirport();
    }
    // Add segment to fare component
    erdFareComp->segments().push_back(erdFltSeg);

    return erdFltSeg;
  }

  void setNonAbacusPCC()
  {
    DataHandle dataHandle;
    const std::vector<Customer*> hdq = dataHandle.getCustomer("W0H3");
    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = hdq[0];
    agent->mainTvlAgencyPCC() = "W0H3";
    _trx->getRequest()->ticketingAgent() = agent;
    TrxUtil::disableAbacus();

    CPPUNIT_ASSERT(!agent->abacusUser());
  }

  void setAbacusPCC()
  {
    DataHandle dataHandle;
    const std::vector<Customer*> abacus = dataHandle.getCustomer("5KAD");
    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = abacus[0];
    agent->mainTvlAgencyPCC() = "5KAD";
    _trx->getRequest()->ticketingAgent() = agent;
    TrxUtil::enableAbacus();

    CPPUNIT_ASSERT(agent->abacusUser());
  }

  void setupItinerary1()
  {
    ERDFareComp* erdFareComp;

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "KRK", "WAW", 100, "PLN", "ADT", 1);
    createFlightSegment(erdFareComp, 1);
  }

  void setupItinerary2()
  {
    ERDFareComp* erdFareComp;

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "KRK", "WAW", 100, "PLN", "ADT", 1);
    createFlightSegment(erdFareComp, 1);

    erdFareComp = createFareComponent("LO", "TEST2", "TEST2", "WAW", "DFW", 500, "PLN", "ADT", 1);
    createFlightSegment(erdFareComp, 2, GeoTravelType::International);
  }

  void setupItinerary3()
  {
    ERDFareComp* erdFareComp;

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "KRK", "WAW", 100, "PLN", "ADT", 1);
    createFlightSegment(erdFareComp, 1);

    erdFareComp = createFareComponent("LO", "TEST2", "TEST2", "WAW", "DFW", 500, "PLN", "INS", 2);
    createFlightSegment(erdFareComp, 2, GeoTravelType::International);
  }

  void setupItinerary4()
  {
    ERDFareComp* erdFareComp;

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "KRK", "WAW", 100, "PLN", "ADT", 1);
    createFlightSegment(erdFareComp, 1);

    erdFareComp = createFareComponent("LO", "TEST2", "TEST2", "WAW", "DFW", 500, "PLN", "INS", 2);
    createFlightSegment(erdFareComp, 1, GeoTravelType::International);
  }

  void setupItinerary5()
  {
    ERDFareComp* erdFareComp;

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "KRK", "WAW", 100, "PLN", "ADT", 1);
    createFlightSegment(erdFareComp, 1);

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "KRK", "WAW", 100, "PLN", "ADT", 1);
    createFlightSegment(erdFareComp, 2);
  }

  void setupChildItinerary()
  {
    ERDFareComp* erdFareComp;

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "KRK", "WAW", 100, "PLN", "X05", 1);
    createFlightSegment(erdFareComp, 1);

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "KRK", "WAW", 100, "PLN", "X07", 2);
    createFlightSegment(erdFareComp, 1);

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "KRK", "WAW", 100, "PLN", "XNN", 3);
    createFlightSegment(erdFareComp, 1);
  }

  void setupRussiaPolandItinerary()
  {
    ERDFareComp* erdFareComp;

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "SVO", "HTA", 100, "PLN", "X05", 1);
    createFlightSegment(erdFareComp, 1, GeoTravelType::International);

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "HTA", "WAW", 100, "PLN", "X07", 2);
    createFlightSegment(erdFareComp, 1, GeoTravelType::International);
  }

  void setupRussiaRussiaItinerary()
  {
    ERDFareComp* erdFareComp;

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "SVO", "HTA", 100, "PLN", "X05", 1);
    createFlightSegment(erdFareComp, 1, GeoTravelType::International);

    erdFareComp = createFareComponent("LO", "TEST1", "TEST1", "HTA", "SVO", 100, "PLN", "X07", 2);
    createFlightSegment(erdFareComp, 1, GeoTravelType::International);
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _itin = _memHandle.create<Itin>();
    _request = _memHandle.create<FareDisplayRequest>();
    _options = _memHandle.create<FareDisplayOptions>();

    AirSeg* airSeg;
    _trx->dataHandle().get(airSeg);
    DateTime tvlDate(DateTime::localTime());
    airSeg->departureDT() = tvlDate;

    _trx->itin().push_back(_itin);
    _trx->setOptions(_options);
    _trx->setRequest(_request);
    _trx->travelSeg().push_back(airSeg);

    _processor = _memHandle.insert(new ERDRequestProcessor(*_trx));
  }

  void tearDown() { _memHandle.clear(); }

  void testInvalidRequest()
  {
    // In case of no data, we should throw an exception
    _trx->getOptions()->erdFareComps().clear();
    CPPUNIT_ASSERT_THROW(_processor->process(), ErrorResponseException);
  }

  void testPassBasicRequest()
  {
    setAbacusPCC();
    setupItinerary1();

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_trx->getRequest()->fareBasisCode() == "TEST1");
    CPPUNIT_ASSERT(_trx->getOptions()->carrierCode() == "LO");
  }

  void testFailBasicRequest()
  {
    setAbacusPCC();
    setupItinerary2();

    CPPUNIT_ASSERT(!_processor->process());
  }

  void testPassFareBasisRequest()
  {
    setAbacusPCC();
    setupItinerary2();
    _request->fareBasisCode() = "TEST1";

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_trx->getRequest()->fareBasisCode() == "TEST1");
    CPPUNIT_ASSERT(_trx->getOptions()->carrierCode() == "LO");
  }

  void testFailFareBasisRequest()
  {
    setAbacusPCC();
    setupItinerary2();
    _request->fareBasisCode() = "TES";

    CPPUNIT_ASSERT(!_processor->process());
  }

  void testPassSegmentRequest()
  {
    setAbacusPCC();
    setupItinerary2();
    _options->requestedSegments().push_back(1);

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_trx->getRequest()->fareBasisCode() == "TEST1");
    CPPUNIT_ASSERT(_trx->getOptions()->carrierCode() == "LO");
  }

  void testFailSegmentRequest()
  {
    setAbacusPCC();
    setupItinerary2();
    _options->requestedSegments().push_back(3);

    CPPUNIT_ASSERT(!_processor->process());
  }

  void testPassFareBasisSegmentRequest()
  {
    setAbacusPCC();
    setupItinerary2();
    _request->fareBasisCode() = "TEST1";
    _options->requestedSegments().push_back(1);

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_trx->getRequest()->fareBasisCode() == "TEST1");
    CPPUNIT_ASSERT(_trx->getOptions()->carrierCode() == "LO");
  }

  void testFailFareBasisSegmentRequest()
  {
    setAbacusPCC();
    setupItinerary2();
    _request->fareBasisCode() = "TEST1";
    _options->requestedSegments().push_back(2);

    CPPUNIT_ASSERT(!_processor->process());
  }

  void testPassPtcRequestSabre()
  {
    setNonAbacusPCC();
    setupItinerary3();

    _options->requestedPaxTypeCode() = "ADT";
    _options->requestedSegments().push_back(1);

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_trx->getRequest()->fareBasisCode() == "TEST1");
    CPPUNIT_ASSERT(_trx->getOptions()->carrierCode() == "LO");
  }

  void testFailPtcRequestSabre()
  {
    setNonAbacusPCC();
    setupItinerary3();

    _options->requestedPaxTypeCode() = "ADT";
    _options->requestedSegments().push_back(2);

    CPPUNIT_ASSERT(!_processor->process());
  }

  void testFailPtcRequestSabre2()
  {
    setNonAbacusPCC();
    setupItinerary3();

    _options->requestedPaxTypeCode() = "UNN";
    _options->requestedSegments().push_back(1);

    CPPUNIT_ASSERT(!_processor->process());
  }

  void testPassPtcRequestAbacus()
  {
    setAbacusPCC();
    setupItinerary3();

    _options->requestedPaxTypeNumber() = 1;
    _options->requestedSegments().push_back(1);

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_trx->getRequest()->fareBasisCode() == "TEST1");
    CPPUNIT_ASSERT(_trx->getOptions()->carrierCode() == "LO");
  }

  void testFailPtcRequestAbacus()
  {
    setAbacusPCC();
    setupItinerary3();

    _options->requestedPaxTypeNumber() = 1;
    _options->requestedSegments().push_back(2);

    CPPUNIT_ASSERT(!_processor->process());
  }

  void testFailPtcRequestAbacus2()
  {
    setAbacusPCC();
    setupItinerary3();

    _options->requestedPaxTypeNumber() = 3;
    _options->requestedSegments().push_back(1);

    CPPUNIT_ASSERT(!_processor->process());
  }

  void testPassCategories()
  {
    setAbacusPCC();
    setupItinerary1();

    _options->ruleCategories().push_back(1);
    _options->ruleCategories().push_back(49);
    _options->requestedSegments().push_back(1);

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_trx->getOptions()->ruleCategories().size() == 2);
    CPPUNIT_ASSERT(_trx->getOptions()->ruleCategories()[0] == 1);
    CPPUNIT_ASSERT(_trx->getOptions()->ruleCategories()[1] == 49);
  }

  void testFailCategories()
  {
    setAbacusPCC();
    setupItinerary1();

    _options->ruleCategories().push_back(1);
    _options->ruleCategories().push_back(99);
    _options->requestedSegments().push_back(1);

    CPPUNIT_ASSERT(!_processor->process());
    CPPUNIT_ASSERT(_trx->getOptions()->ruleCategories().size() == 2);
    CPPUNIT_ASSERT(_trx->getOptions()->ruleCategories()[0] == 1);
    CPPUNIT_ASSERT(_trx->getOptions()->ruleCategories()[1] == 99);
  }

  void testPassCategoriesForERDFromSWS()
  {
    setAbacusPCC();
    setupItinerary1();

    Billing bill;
    _trx->billing() = &bill;
    _trx->getRequest()->requestType() = ENHANCED_RD_REQUEST;
    _trx->billing()->requestPath() = SWS_PO_ATSE_PATH;

    _options->ruleCategories().push_back(1);
    _options->ruleCategories().push_back(99);
    _options->requestedSegments().push_back(1);

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_trx->getOptions()->ruleCategories().size() == 2);
    CPPUNIT_ASSERT(_trx->getOptions()->ruleCategories()[0] == 1);
    CPPUNIT_ASSERT(_trx->getOptions()->ruleCategories()[1] == 99);
  }

  void testPassMenu()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->ruleMenuDisplay() = 'T';
    CPPUNIT_ASSERT(_processor->process());
  }

  void testPassHeader()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->headerDisplay() = 'T';
    CPPUNIT_ASSERT(_processor->process());
  }

  void testPassRouting()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->routingDisplay() = 'T';
    CPPUNIT_ASSERT(_processor->process());
  }

  void testPassIc()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->IntlConstructionDisplay() = 'T';
    CPPUNIT_ASSERT(_processor->process());
  }

  void testFailMenuAndHeader()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->ruleMenuDisplay() = 'T';
    _options->headerDisplay() = 'T';
    CPPUNIT_ASSERT(!_processor->process());
  }

  void testFailHeaderAndRouting()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->routingDisplay() = 'T';
    _options->headerDisplay() = 'T';
    CPPUNIT_ASSERT(!_processor->process());
  }

  void testFailIcAndRouting()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->routingDisplay() = 'T';
    _options->IntlConstructionDisplay() = 'T';
    CPPUNIT_ASSERT(!_processor->process());
  }

  void testFailMenuAndCategories()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->ruleCategories().push_back(1);
    _options->ruleMenuDisplay() = 'T';
    CPPUNIT_ASSERT(!_processor->process());
  }

  void testFailHeaderAndAlphaCodes()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->alphaCodes().push_back("DS");
    _options->headerDisplay() = 'T';
    CPPUNIT_ASSERT(!_processor->process());
  }

  void testFailMenuAndMx()
  {
    setupItinerary1();
    setAbacusPCC();

    _options->combScoreboardDisplay() = 'T';
    _options->headerDisplay() = 'T';
    CPPUNIT_ASSERT(!_processor->process());
  }

  void testPassSimilarChildTypes()
  {
    setAbacusPCC();
    setupChildItinerary();

    CPPUNIT_ASSERT(_processor->process());
  }

  void testPassDtsSection()
  {
    std::string sampleDtsText = "<SUM>1</SUM><SUM>2</SUM><SUM>3</SUM>";
    _options->lineNumber() = 3;
    _options->requestedLineNumber() = 3;

    CPPUNIT_ASSERT(_processor->prepareDTS(sampleDtsText));
    CPPUNIT_ASSERT(sampleDtsText == "<SUM>3</SUM>");
  }

  void testPassDtsSectionNoLineRequested()
  {
    std::string sampleDtsText = "<SUM>1</SUM><SUM>2</SUM><SUM>3</SUM>";
    _options->lineNumber() = 3;
    _options->requestedLineNumber() = -1;

    CPPUNIT_ASSERT(_processor->prepareDTS(sampleDtsText));
    CPPUNIT_ASSERT(sampleDtsText == "<SUM>1</SUM>");
  }

  void testFailDtsSectionInvalidLine()
  {
    std::string sampleDtsText = "<SUM>1</SUM><SUM>2</SUM><SUM>3</SUM>";
    _options->lineNumber() = 2;
    _options->requestedLineNumber() = 3;

    CPPUNIT_ASSERT_THROW(_processor->prepareDTS(sampleDtsText), NonFatalErrorResponseException);
  }

  void testFailInvalidDtsSection()
  {
    std::string sampleDtsText = "<SUM>1</SUM><SUM>2</SUM><SUM>3</SUM>";
    _options->lineNumber() = 4;
    _options->requestedLineNumber() = 4;

    CPPUNIT_ASSERT_THROW(_processor->prepareDTS(sampleDtsText), NonFatalErrorResponseException);
  }

  void testFailEmptyDtsSection()
  {
    std::string sampleDtsText = "";
    _options->lineNumber() = 0;
    _options->requestedLineNumber() = -1;

    CPPUNIT_ASSERT(!_processor->prepareDTS(sampleDtsText));
  }

  void testFailLineRequestedInEmptyDtsSection()
  {
    std::string sampleDtsText = "";
    _options->lineNumber() = 0;
    _options->requestedLineNumber() = 1;

    CPPUNIT_ASSERT_THROW(_processor->prepareDTS(sampleDtsText), NonFatalErrorResponseException);
  }

  void testPassFareBreaks()
  {
    setAbacusPCC();
    setupItinerary5();

    _trx->getOptions()->erdFareComps().front()->fareBreak() = 'F';
    _trx->getOptions()->erdFareComps().front()->ruleNumber() = "AAA";
    _trx->getOptions()->erdFareComps().back()->ruleNumber() = "BBB";

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_trx->getOptions()->erdFareComps().front()->fareBreak() == 'T');
    CPPUNIT_ASSERT(_trx->getOptions()->erdFareComps().front()->ruleNumber() == "BBB");
  }

  void testFailFareBreaks()
  {
    setAbacusPCC();
    setupItinerary5();

    _trx->getOptions()->erdFareComps().front()->fareBreak() = 'F';
    _trx->getOptions()->erdFareComps().front()->ruleNumber() = "AAA";
    _trx->getOptions()->erdFareComps().front()->ruleNumber() = "YYY";
    _trx->getOptions()->erdFareComps().back()->ruleNumber() = "BBB";
    _trx->getOptions()->erdFareComps().back()->arrivalCity() = "ZZZ";

    CPPUNIT_ASSERT(!_processor->process());
    CPPUNIT_ASSERT(_trx->getOptions()->erdFareComps().front()->fareBreak() != 'T');
    CPPUNIT_ASSERT(_trx->getOptions()->erdFareComps().front()->ruleNumber() != "BBB");
  }

  void testIsWqrd()
  {
    setAbacusPCC();
    setupItinerary5();

    _trx->getOptions()->isWqrd() = true;
    _options->requestedSegments().push_back(1);
    _options->requestedSegments().push_back(2);
    CPPUNIT_ASSERT(_processor->process());

    _options->requestedSegments().push_back(3);
    CPPUNIT_ASSERT(!_processor->process());
  }

  void testErrorFromPSS()
  {
    setAbacusPCC();
    setupItinerary1();

    _trx->getOptions()->errorFromPSS() = 'P';
    CPPUNIT_ASSERT(!_processor->process());
    _trx->getOptions()->errorFromPSS() = 'Q';
    CPPUNIT_ASSERT(!_processor->process());
    _trx->getOptions()->errorFromPSS() = 'S';
    CPPUNIT_ASSERT(!_processor->process());
    _trx->getOptions()->errorFromPSS() = 'F';
    CPPUNIT_ASSERT(!_processor->process());
    _trx->getOptions()->errorFromPSS() = 'C';
    CPPUNIT_ASSERT(!_processor->process());
    _trx->getOptions()->errorFromPSS() = 'M';
    CPPUNIT_ASSERT(!_processor->process());
    _trx->getOptions()->errorFromPSS() = 'I';
    CPPUNIT_ASSERT(!_processor->process());
    _trx->getOptions()->errorFromPSS() = 'X';
    CPPUNIT_ASSERT(!_processor->process());
    _trx->getOptions()->errorFromPSS() = 'A';
    CPPUNIT_ASSERT(!_processor->process());

    _trx->getOptions()->errorFromPSS() = ' ';
    CPPUNIT_ASSERT(_processor->process());
  }

  void testArnkInWqrd()
  {
    setAbacusPCC();
    setupItinerary5();

    _trx->getOptions()->isWqrd() = true;
    _options->requestedSegments().push_back(1);
    _options->requestedSegments().push_back(2);
    _trx->getOptions()->erdFareComps().front()->segments().front()->itinSegNumber() = 0;

    CPPUNIT_ASSERT(_processor->process());
  }

  void testArnkInWprd()
  {
    setAbacusPCC();
    setupItinerary5();

    _options->requestedSegments().push_back(1);
    _options->requestedSegments().push_back(2);
    _options->surfaceSegments().push_back(1);

    CPPUNIT_ASSERT(_processor->process());
  }

  void testAllSegmentsAreArnks()
  {
    setAbacusPCC();
    setupItinerary5();

    _options->requestedSegments().push_back(1);
    _options->requestedSegments().push_back(2);
    _options->surfaceSegments().push_back(1);
    _options->surfaceSegments().push_back(2);

    CPPUNIT_ASSERT(!_processor->process());
  }

  void testInternationalRoundingCalculation1()
  {
    setAbacusPCC();
    setupItinerary1();

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(!_itin->useInternationalRounding());
  }

  void testInternationalRoundingCalculation2()
  {
    setAbacusPCC();
    setupItinerary2();

    CPPUNIT_ASSERT(!_processor->process());
    CPPUNIT_ASSERT(!_itin->useInternationalRounding());
  }

  void testInternationalRoundingCalculation3()
  {
    setAbacusPCC();
    setupItinerary3();

    CPPUNIT_ASSERT(!_processor->process());
    CPPUNIT_ASSERT(!_itin->useInternationalRounding());
  }

  void testInternationalRoundingCalculation4()
  {
    setAbacusPCC();
    setupItinerary4();

    CPPUNIT_ASSERT(!_processor->process());
    CPPUNIT_ASSERT(!_itin->useInternationalRounding());
  }

  void testInternationalRoundingCalculation5()
  {
    setAbacusPCC();
    setupItinerary5();

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(!_itin->useInternationalRounding());
  }

  void testInternationalRoundingCalculationChild()
  {
    setAbacusPCC();
    setupChildItinerary();

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(!_itin->useInternationalRounding());
  }

  void testInternationalRoundingCalculationRussiaPoland()
  {
    setAbacusPCC();
    setupRussiaPolandItinerary();

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(_itin->useInternationalRounding());
  }

  void testInternationalRoundingCalculationRussiaRussia()
  {
    setAbacusPCC();
    setupRussiaRussiaItinerary();

    CPPUNIT_ASSERT(_processor->process());
    CPPUNIT_ASSERT(!_itin->useInternationalRounding());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ERDRequestProcessorTest);
}
