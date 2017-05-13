//-----------------------------------------------------------------------------
//
//  File:     Diag199CollectorTest.cpp
//
//  Author :  Grzegorz Wanke
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag199Collector.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/MarketCriteria.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Agent.h"
#include "DBAccess/Customer.h"

using namespace tse;
using namespace std;

namespace tse
{
class Diag199CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag199CollectorTest);
  CPPUNIT_TEST(testAddBilling);
  CPPUNIT_TEST(testAddRefundDates);
  CPPUNIT_TEST(testAddPricingDates);
  CPPUNIT_TEST(testPrintBrandingInfo);
  CPPUNIT_TEST(testGetFMValidatingCarriers);
  CPPUNIT_TEST(testPbbHasBrandCode);
  CPPUNIT_TEST(testPbbNoBrandCode);
  CPPUNIT_TEST(testShoppingContextFirstSegment);
  CPPUNIT_TEST(testShoppingContextSecondSegment);
  CPPUNIT_TEST(testShoppingContextTwoSegments);
  CPPUNIT_TEST(testAddDiscountInfo_discountInconsistency);
  CPPUNIT_TEST(testAddDiscountInfo_plusUpInconsistency);
  CPPUNIT_TEST(testAddDiscountInfo_discountAmount);
  CPPUNIT_TEST(testAddDiscountInfo_plusUpPercentage);
  CPPUNIT_TEST(testAddDiscountInfo_discountPercentage);
  CPPUNIT_TEST(testAddDiscountInfo_plusUpPercentage);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _collector = new Diag199Collector;
    _collector->_active = true;
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    Customer* customer = _memHandle.create<Customer>();
    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = customer;
    _request->ticketingAgent() = agent;
    _trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);
    _trx->setRequest(_request);
    _trx->setOptions(_memHandle.create<PricingOptions>());
    sfo = getLoc("SFO");
    dfw = getLoc("DFW");
    _fm = createFareMarket(sfo, dfw, "VA");
    AirSeg* airSeg = makeAirSeg("SFO", "DFW", "VA");
    std::vector<TravelSeg*> travelSegs;
    travelSegs.push_back(airSeg);
    _fm->travelSeg() = travelSegs;
    _request->ticketingDT() = DateTime(2015, 11, 25);
  }

  void tearDown() { delete _collector; _memHandle.clear();}

  Billing createBilling()
  {
    Billing billing;
    billing.userPseudoCityCode() = "HDQ";
    billing.userStation() = "5642";
    billing.userBranch() = "6173";
    billing.partitionID() = "AA";
    billing.userSetAddress() = "B37CCF";
    billing.serviceName() = "INTLWPI1";
    billing.aaaCity() = "HDQ";
    billing.aaaSine() = "H3R";
    billing.actionCode() = "WPBET";
    billing.transactionID() = 99;
    return billing;
  }

  void testAddBilling()
  {
    _collector->addBilling(createBilling());
    string expected(" BILLING INFORMATION\n"
                    "     USER PCC      : HDQ\n"
                    "     USER STATION  : 5642\n"
                    "     USER BRANCH   : 6173\n"
                    "     PARTITION     : AA\n"
                    "     SET ADDR      : B37CCF\n"
                    "     SERVICE NAME  : INTLWPI1\n"
                    "     AAA CITY      : HDQ\n"
                    "     AAA SINE      : H3R\n"
                    "     ACTION CODE   : WPBET\n"
                    "     TRANSACTION ID: 99\n");

    CPPUNIT_ASSERT_EQUAL(expected, _collector->str());
  }

  void testAddRefundDates()
  {
    RefundPricingTrx trx;
    trx.displayOnly() = 0;
    trx.setTravelDate(DateTime(2008, 6, 6));
    RexBaseRequest request;
    request.setTicketingDT(DateTime(2008, 6, 7));
    trx.setRequest(&request);
    trx.setOriginalTktIssueDT() = DateTime(2008, 6, 8);
    _collector->addRefundDates(trx);
    string expected(" \n DISPLAY ONLY  : 0\n"
                    " TRAVEL DATE   : 2008-06-06\n"
                    " REFUND DATE   : 2008-06-07\n"
                    " TICKETING DATE: 2008-06-08\n \n");

    CPPUNIT_ASSERT_EQUAL(expected, _collector->str());
  }
  void testAddPricingDates()
  {
    PricingTrx trx;
    trx.displayOnly() = 0;
    trx.setTravelDate(DateTime(2008, 6, 6));
    trx.bookingDate() = DateTime(2008, 6, 7);
    PricingRequest request;
    trx.setRequest(&request);
    trx.ticketingDate() = DateTime(2008, 6, 8);

    _collector->addPricingDates(trx);
    string expected(" \n DISPLAY ONLY  : 0"
                    "\n TRAVEL DATE   : 2008-06-06"
                    "\n BOOKING DATE  : 2008-06-07"
                    "\n TICKETING DATE: 2008-06-08\n \n");

    CPPUNIT_ASSERT_EQUAL(expected, _collector->str());
  }

  void testPrintBrandingInfo()
  {
    PricingTrx trx;
    FareMarket fm;

    _collector->printBrandingInfo(trx, fm);
    CPPUNIT_ASSERT(_collector->str().empty());

    MarketResponse marketResponse;
    std::vector<MarketResponse*> v;
    v.push_back(&marketResponse);

    PricingTrx::BrandedMarketMap bmm;
    bmm[5] = v;
    trx.brandedMarketMap() = bmm;

    fm.marketIDVec().push_back(5);
    marketResponse.setMarketID() = 15;

    MarketCriteria marketCriteria;
    marketCriteria.departureAirportCode() = "AUS";
    marketCriteria.arrivalAirportCode() = "SEA";

    marketResponse.marketCriteria() = &marketCriteria;
    marketResponse.carrier() = "ZZ";

    BrandProgram brandProgram;
    brandProgram.programID() = 55;
    brandProgram.programCode() = "ALPHA";
    brandProgram.programName() = "BETA";
    brandProgram.programDescription() = "GAMMA";

    BrandInfo brand;
    brand.brandCode() = "BBETA";
    brand.brandName() = "BGAMMA";

    brandProgram.brandsData().push_back(&brand);

    marketResponse.brandPrograms().push_back(&brandProgram);

    _collector->printBrandingInfo(trx, fm);

    string expected(" *** BRAND INFORMATION ***\n"
                    " BRAND MARKET 15: AUS-ZZ-SEA\n"
                    "   BRAND PROGRAMS:\n"
                    "     ID: 7       CODE: ALPHA\n"
                    "     NAME: BETA\n"
                    "     DESCRIPTION: GAMMA\n"
                    "     BRANDS:\n"
                    "       BBETA - BGAMMA\n\n");

    CPPUNIT_ASSERT_EQUAL(expected, _collector->str());
  }

  void testGetFMValidatingCarriers()
  {
    FareMarket fm;
    PricingTrx trx;
    fm.validatingCarriers().push_back("AA");

    string s = _collector->getFMValidatingCarriers(trx, fm);
    CPPUNIT_ASSERT(s == "AA");

    fm.validatingCarriers().push_back("BB");
    s = _collector->getFMValidatingCarriers(trx, fm);
    CPPUNIT_ASSERT(s == "AA,BB");

    fm.validatingCarriers().clear();
    trx.setValidatingCxrGsaApplicable(false);

    s = _collector->getFMValidatingCarriers(trx, fm);
    CPPUNIT_ASSERT(s.empty());

    trx.setValidatingCxrGsaApplicable(true);
    fm.validatingCarriers().clear();
    Itin itin1, itin2;

    ValidatingCxrGSAData v;
    ValidatingCxrDataMap vcm;
    vcx::ValidatingCxrData vcd;

    vcm["AA"] = vcd;
    v.validatingCarriersData() = vcm;
    itin1.validatingCxrGsaData() = &v;

    trx.itin().push_back(&itin1);

    s = _collector->getFMValidatingCarriers(trx, fm);
    CPPUNIT_ASSERT(s == "AA");

    trx.itin().push_back(&itin2);

    s = _collector->getFMValidatingCarriers(trx, fm);
    CPPUNIT_ASSERT(s.empty());

    trx.itin().clear();

    vcm["BB"] = vcd;
    v.validatingCarriersData() = vcm;
    itin1.validatingCxrGsaData() = &v;
    trx.itin().push_back(&itin1);

    s = _collector->getFMValidatingCarriers(trx, fm);
    CPPUNIT_ASSERT(s.empty());

  }

  void testPbbHasBrandCode()
  {
    _collector->addFareMarket(*_trx, *_fm);
    CPPUNIT_ASSERT(_collector->str().find("BRAND CODE         : FL") != std::string::npos);
  }

  void testPbbNoBrandCode()
  {
    _fm->travelSeg()[0]->setBrandCode("");
    _collector->addFareMarket(*_trx, *_fm);
    CPPUNIT_ASSERT(_collector->str().find("BRAND CODE         :") == std::string::npos);
  }

  void testShoppingContextFirstSegment()
  {
    _trx->getMutableFixedLegs().push_back(true);
    _trx->getMutableFixedLegs().push_back(false);

    Itin *itin = _memHandle.create<Itin>();
    _trx->itin().push_back(itin);

    makeShoppingContextSegments(itin);

    skipper::FareComponentShoppingContext *context =
      _memHandle.create<skipper::FareComponentShoppingContext>();
    context->fareBasisCode = "VSP4MAE";
    context->fareCalcFareAmt = "868.49";
    context->brandCode = "EF";

    _trx->getMutableFareComponentShoppingContexts()[itin->travelSeg()[0]->pnrSegment()] = context;

    _collector->displayContextShoppingInfo(*_trx);
    std::string expected =
      "\n"
      " ---------------------------------------------------------- \n"
      " *** CONTEXT SHOPPING FARE COMPONENT ***\n"
      " FARE BASIS CODE      : VSP4MAE\n"
      " FARE CALC FARE AMOUNT: 868.49\n"
      " BRAND CODE           : EF\n"
      "\n"
      " *** VCTR ***\n"
      " N/A\n"
      "\n"
      " *** TRAVEL SEGMENTS *** \n"
      " 1 VA123 Y  23SEP SYDMEL 1015AM 1250PM O-MEL CAB:8 OK/ FDM   \n"
      " ---------------------------------------------------------- \n"
      " *** CONTEXT SHOPPING FIXED LEGS ***\n"
      " LEG 1: FIXED\n"
      " LEG 2: SHOPPED\n"
      " ---------------------------------------------------------- \n";

    CPPUNIT_ASSERT_EQUAL(expected, _collector->str());
  }

  void testShoppingContextSecondSegment()
  {
    _trx->getMutableFixedLegs().push_back(false);
    _trx->getMutableFixedLegs().push_back(true);

    Itin *itin = _memHandle.create<Itin>();
    _trx->itin().push_back(itin);

    makeShoppingContextSegments(itin);

    skipper::FareComponentShoppingContext *context =
      _memHandle.create<skipper::FareComponentShoppingContext>();
    context->fareBasisCode = "VSP4MAE";
    context->fareCalcFareAmt = "868.49";
    context->brandCode = "EF";
    VCTR vctr;
    vctr.vendor() = "1S";
    vctr.carrier() = "VA";
    vctr.tariff() = 1234;
    vctr.rule() = "0000";
    context->vctr = vctr;

    _trx->getMutableFareComponentShoppingContexts()[itin->travelSeg()[1]->pnrSegment()] = context;

    _collector->displayContextShoppingInfo(*_trx);
    std::string expected =
      "\n"
      " ---------------------------------------------------------- \n"
      " *** CONTEXT SHOPPING FARE COMPONENT ***\n"
      " FARE BASIS CODE      : VSP4MAE\n"
      " FARE CALC FARE AMOUNT: 868.49\n"
      " BRAND CODE           : EF\n"
      "\n"
      " *** VCTR ***\n"
      " VENDOR: 1S  CARRIER: VA  TARIFF: 1234  RULE: 0000\n"
      "\n"
      " *** TRAVEL SEGMENTS *** \n"
      " 3 VA127 Y  30SEP MELSYD 155PM  440PM  O-SYD CAB:8 OK/ FDM   \n"
      " ---------------------------------------------------------- \n"
      " *** CONTEXT SHOPPING FIXED LEGS ***\n"
      " LEG 1: SHOPPED\n"
      " LEG 2: FIXED\n"
      " ---------------------------------------------------------- \n";

    CPPUNIT_ASSERT_EQUAL(expected, _collector->str());
  }

  void testShoppingContextTwoSegments()
  {
    _trx->getMutableFixedLegs().push_back(true);
    _trx->getMutableFixedLegs().push_back(false);

    Itin *itin = _memHandle.create<Itin>();
    _trx->itin().push_back(itin);

    makeShoppingContextSegments(itin, true);

    skipper::FareComponentShoppingContext *context =
      _memHandle.create<skipper::FareComponentShoppingContext>();
    context->fareBasisCode = "VSP4MAE";
    context->fareCalcFareAmt = "868.49";
    context->brandCode = "EF";

    _trx->getMutableFareComponentShoppingContexts()[itin->travelSeg()[0]->pnrSegment()] = context;
    _trx->getMutableFareComponentShoppingContexts()[itin->travelSeg()[1]->pnrSegment()] = context;

    _collector->displayContextShoppingInfo(*_trx);
    std::string expected =
      "\n"
      " ---------------------------------------------------------- \n"
      " *** CONTEXT SHOPPING FARE COMPONENT ***\n"
      " FARE BASIS CODE      : VSP4MAE\n"
      " FARE CALC FARE AMOUNT: 868.49\n"
      " BRAND CODE           : EF\n"
      "\n"
      " *** VCTR ***\n"
      " N/A\n"
      "\n"
      " *** TRAVEL SEGMENTS *** \n"
      " 1 VA123 Y  23SEP SYDPER 1015AM 1250PM O-PER CAB:8 OK/ FDM   \n"
      " 2 VA126 Y  23SEP PERMEL 155PM  440PM  O-MEL CAB:8 OK/ FDM   \n"
      " ---------------------------------------------------------- \n"
      " *** CONTEXT SHOPPING FIXED LEGS ***\n"
      " LEG 1: FIXED\n"
      " LEG 2: SHOPPED\n"
      " ---------------------------------------------------------- \n";

    CPPUNIT_ASSERT_EQUAL(expected, _collector->str());
  }

  void makeShoppingContextSegments(Itin* itin, bool outboundConnection=false)
  {
    AirSeg* seg = nullptr;

    if (outboundConnection)
    {
      seg = _memHandle.create<AirSeg>();
      itin->travelSeg().push_back(seg);
      seg->pnrSegment() = 1;
      seg->carrier() = "VA";
      seg->flightNumber() = 123;
      seg->setBookingCode("Y");
      seg->departureDT() = DateTime(2015, 9, 23, 10, 15);
      seg->origAirport() = "SYD";
      seg->destAirport() = "PER";
      seg->stopOver() = true;
      seg->arrivalDT() = DateTime(2015, 9, 23, 12, 50);
      seg->bookedCabin().setEconomyClass();
      seg->resStatus() = "OK";
      seg->geoTravelType() = GeoTravelType::ForeignDomestic;

      seg = _memHandle.create<AirSeg>();
      itin->travelSeg().push_back(seg);
      seg->pnrSegment() = 2;
      seg->carrier() = "VA";
      seg->flightNumber() = 126;
      seg->setBookingCode("Y");
      seg->departureDT() = DateTime(2015, 9, 23, 13, 55);
      seg->origAirport() = "PER";
      seg->destAirport() = "MEL";
      seg->stopOver() = true;
      seg->arrivalDT() = DateTime(2015, 9, 23, 16, 40);
      seg->bookedCabin().setEconomyClass();
      seg->resStatus() = "OK";
      seg->geoTravelType() = GeoTravelType::ForeignDomestic;
    }
    else
    {
      seg = _memHandle.create<AirSeg>();
      itin->travelSeg().push_back(seg);
      seg->pnrSegment() = 1;
      seg->carrier() = "VA";
      seg->flightNumber() = 123;
      seg->setBookingCode("Y");
      seg->departureDT() = DateTime(2015, 9, 23, 10, 15);
      seg->origAirport() = "SYD";
      seg->destAirport() = "MEL";
      seg->stopOver() = true;
      seg->arrivalDT() = DateTime(2015, 9, 23, 12, 50);
      seg->bookedCabin().setEconomyClass();
      seg->resStatus() = "OK";
      seg->geoTravelType() = GeoTravelType::ForeignDomestic;
    }

    seg = _memHandle.create<AirSeg>();
    itin->travelSeg().push_back(seg);
    seg->pnrSegment() = 3;
    seg->carrier() = "VA";
    seg->flightNumber() = 127;
    seg->setBookingCode("Y");
    seg->departureDT() = DateTime(2015, 9, 30, 13, 55);
    seg->origAirport() = "MEL";
    seg->destAirport() = "SYD";
    seg->stopOver() = true;
    seg->arrivalDT() = DateTime(2015, 9, 30, 16, 40);
    seg->bookedCabin().setEconomyClass();
    seg->resStatus() = "OK";
    seg->geoTravelType() = GeoTravelType::ForeignDomestic;
  }

  FareMarket*
  createFareMarket(const Loc* origin, const Loc* destination, CarrierCode goveringCarrier)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->origin() = origin;
    fm->destination() = destination;
    fm->governingCarrier() = goveringCarrier;
    return fm;
  }

  const Loc* getLoc(const LocCode& locCode)
  {
    return TestLocFactory::create("/vobs/atseintl/test/testdata/data/Loc" + locCode + ".xml");
  }

  AirSeg* makeAirSeg(const LocCode& boardMultiCity,
                     const LocCode& offMultiCity,
                     const CarrierCode& carrier,
                     LocCode origAirport = "",
                     LocCode destAirport = "")
  {
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc(boardMultiCity, DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc(offMultiCity, DateTime::localTime());

    if ("" == origAirport)
      origAirport = boardMultiCity;
    if ("" == destAirport)
      destAirport = offMultiCity;

    AirSeg* as = _memHandle.create<AirSeg>();
    as->origin() = const_cast<Loc*>(loc1);
    as->destination() = const_cast<Loc*>(loc2);
    as->boardMultiCity() = boardMultiCity;
    as->offMultiCity() = offMultiCity;
    as->origAirport() = origAirport;
    as->destAirport() = destAirport;
    as->carrier() = carrier;
    as->setBrandCode("FL");
    return as;
  }

  void addOneFareMarketWithTwoAirSegments()
  {
    FareMarket* fareMarket = createFareMarket(getLoc("SFO"), getLoc("LON"), "EY");

    AirSeg* seg1 = makeAirSeg("SFO", "LON", "EY");
    seg1->segmentOrder() = 1;
    fareMarket->travelSeg().push_back(seg1);

    AirSeg* seg2 = makeAirSeg("LON", "JFK", "EY");
    seg2->segmentOrder() = 2;
    fareMarket->travelSeg().push_back(seg2);

    _trx->fareMarket().push_back(fareMarket);
  }

  std::string getFailedFareMarketInDiag()
  {
    return std::string("***************** START DIAG 199 ************************** \n"
                       "NUMBER OF FARE MARKETS IN TRX: 1\n"
                       " \n"
                       " ---------------------------------------------------------- \n"
                       " *** FAILED FARE MARKET 1 ***\n"
                       " ---------------------------------------------------------- \n"
                       " ORIGIN-DESTINATION : SFO-LON\n"
                       " BOARD-OFF CITIES   : -\n"
                       " BOARD-OFF AIRPORTS : SFO-JFK\n"
                       " GOVERNING CARRIER  : EY\n"
                       " VALIDATING CARRIERS: \n"
                       " GLOBAL DIRECTION   : XX   -UNIVERSAL\n"
                       " GEO  TRAVEL  TYPE  : UNKNOWN\n"
                       " DIRECTION          : UNKNOWN\n"
                       " FLT TRACK INDICATOR: FALSE\n"
                       " BRAND CODE         : FL         \n"
                       " LEG INDEX          : 0\n"
                       " TRAVEL DATE        : N/A\n"
                       " \n"
                       " FAILED FARE MARKET DUE TO DISCOUNT OR MARK UP INCONSISTENCY !!!\n"
                       "\n"
                       " *** TRAVEL SEGMENTS *** \n"
                       " 0 EY0      N/A   SFOLON N/A    N/A    X-LON CAB:  / UNK   \n"
                       " 0 EY0      N/A   LONJFK N/A    N/A    X-JFK CAB:  / UNK   \n"
                       " *** PRIMARY SECTOR ***\n"
                       " PRIMARY SECTOR IS NULL \n"
                       "\n"
                       " *** SIDE TRIPS ***\n"
                       " NUMBER OF SIDE TRIPS: 0\n"
                       " \n"
                       " *** CUSTOMER/CARRIER PREFERENCE DATA ***\n"
                       " SOLO CUSTOMER: YES SOLO ACTIVE: NO  \n"
                       "  JOURNEY ACTIVE: NO   FLOW FARE MARKET: NO  MARRIED: NO\n"
                       " 0 EY0     JOURNEY TYPE:000    SOLO CXR:000  AVL BREAK:UNK \n"
                       " 0 EY0     JOURNEY TYPE:000    SOLO CXR:000  AVL BREAK:UNK \n"
                       " \n"
                       " *** ATAE CLASS OF SERVICES ***\n"
                       " 0 EY0      N/A   SFOLON N/A    N/A    X-LON CAB:  / UNK   \n"
                       "   THRU AVAIL:  \n"
                       "   SOLO AVAIL:  \n"
                       " \n"
                       " 0 EY0      N/A   LONJFK N/A    N/A    X-JFK CAB:  / UNK   \n"
                       "   THRU AVAIL:  \n"
                       "   SOLO AVAIL:  \n"
                       " \n"
                       " \n"
                       " ---------------------------------------------------------- \n"
                       "\n"
                       "\n"
                       " *** ALL TRAVEL SEGMENTS ***\n"
                       " \n"
                       "\n"
                       " ---------------------------------------------------------- \n"
                       "\n"
                       " BILLING INFORMATION\n"
                       "     USER PCC      : HDQ\n"
                       "     USER STATION  : 5642\n"
                       "     USER BRANCH   : 6173\n"
                       "     PARTITION     : AA\n"
                       "     SET ADDR      : B37CCF\n"
                       "     SERVICE NAME  : INTLWPI1\n"
                       "     AAA CITY      : HDQ\n"
                       "     AAA SINE      : H3R\n"
                       "     ACTION CODE   : WPBET\n"
                       "     TRANSACTION ID: 99\n"
                       " \n"
                       " \n"
                       " DISPLAY ONLY  : 0\n"
                       " TRAVEL DATE   : N/A\n"
                       " BOOKING DATE  : N/A\n"
                       " TICKETING DATE: 2015-11-25\n"
                       " \n"
                       "***************** END   DIAG 199 ************************** \n");
  }

  void testAddDiscountInfo_discountInconsistency()
  {
    _trx->getRequest()->addDiscountAmountNew(1, 1, 12.34, "USD");
    _trx->getRequest()->addDiscountAmountNew(2, 2, 23.45, "USD");

    addOneFareMarketWithTwoAirSegments();
    Billing billing = createBilling();
    _trx->billing() = &billing;

    *_collector << *_trx;
    CPPUNIT_ASSERT_EQUAL(getFailedFareMarketInDiag(),
                         _collector->str());
  }

  void testAddDiscountInfo_plusUpInconsistency()
  {
    _trx->getRequest()->addDiscountAmountNew(1, 1, -12.34, "USD");
    _trx->getRequest()->addDiscountAmountNew(2, 2, -23.45, "USD");

    addOneFareMarketWithTwoAirSegments();
    Billing billing = createBilling();
    _trx->billing() = &billing;

    *_collector << *_trx;
    CPPUNIT_ASSERT_EQUAL(getFailedFareMarketInDiag(),
                         _collector->str());
  }

  void testAddDiscountInfo_discountAmount()
  {
    _trx->getRequest()->addDiscountAmountNew(1, 1, 12.34, "USD");
    _trx->getRequest()->addDiscountAmountNew(1, 2, 12.34, "USD");

    addOneFareMarketWithTwoAirSegments();

    _collector->addDiscountInfo(*_trx, *_trx->fareMarket()[0]);

    CPPUNIT_ASSERT_EQUAL(std::string(" DISCOUNT AMOUNT: 12.34 USD FOR SEGMENTS: 1-2\n"
                                     "\n"),
                         _collector->str());
  }

  void testAddDiscountInfo_plusUpAmount()
  {
    _trx->getRequest()->addDiscountAmountNew(1, 1, -12.34, "USD");
    _trx->getRequest()->addDiscountAmountNew(1, 2, -12.34, "USD");

    addOneFareMarketWithTwoAirSegments();

    _collector->addDiscountInfo(*_trx, *_trx->fareMarket()[0]);

    CPPUNIT_ASSERT_EQUAL(std::string(" MARK UP AMOUNT: 12.34 USD FOR SEGMENTS: 1-2\n"
                                     "\n"),
                         _collector->str());
  }

  void testAddDiscountInfo_discountPercentage()
  {
    _trx->getRequest()->addDiscountPercentage(1, 50);
    _trx->getRequest()->addDiscountPercentage(2, 50);

    addOneFareMarketWithTwoAirSegments();

    _collector->addDiscountInfo(*_trx, *_trx->fareMarket()[0]);

    CPPUNIT_ASSERT_EQUAL(std::string(" DISCOUNT PERCENTAGE: 50 PERCENT.\n"
                                     "\n"),
                         _collector->str());
  }

  void testAddDiscountInfo_plusUpPercentage()
  {
    _trx->getRequest()->addDiscountPercentage(1, -50);
    _trx->getRequest()->addDiscountPercentage(2, -50);

    addOneFareMarketWithTwoAirSegments();

    _collector->addDiscountInfo(*_trx, *_trx->fareMarket()[0]);

    CPPUNIT_ASSERT_EQUAL(std::string(" MARK UP PERCENTAGE: 50 PERCENT.\n"
                                     "\n"),
                         _collector->str());
  }

private:
  Diag199Collector* _collector;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  FareMarket* _fm;
  const tse::Loc* sfo;
  const tse::Loc* dfw;

};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag199CollectorTest);
}
