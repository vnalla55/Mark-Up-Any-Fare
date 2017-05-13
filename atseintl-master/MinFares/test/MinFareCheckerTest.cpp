//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/Fare.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "MinFares/MinFareChecker.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "DataModel/FarePath.h"
#include "DBAccess/FareInfo.h"

using namespace std;

namespace tse
{

class MinFareCheckerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MinFareCheckerTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testIATAMandatePhase2);
  CPPUNIT_TEST(testProcessForDummy);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testProcess()
  {
    MinFareChecker checker;

    PricingTrx trx;

    // We need to set up the Request for our testing purposes.
    PricingRequest request;

    request.globalDirection() = GlobalDirection::WH; // Western Hemisphere

    trx.setTravelDate(DateTime::localTime());

    // request.ticketingDateTime = 0;
    Agent agent;
    Loc loca;
    loca.loc() = "LON";
    loca.nation() = "GP";

    agent.agentLocation() = &loca; // Sold in GP (Guadeloupe)

    // Build Request
    request.ticketingAgent() = &agent;
    // request.fareClassCode() =

    trx.setRequest(&request); // Set the request pointer (so pass address).

    trx.diagnostic().diagnosticType() = Diagnostic700;
    trx.diagnostic().activate();

    FarePath farePath;
    FareMarket faremkt;
    //===================================
    Loc loc1;
    LocCode locCode1 = "BKK";
    loc1.loc() = locCode1;

    Loc loc2;
    LocCode locCode2 = "TYO";
    loc2.loc() = locCode2;

    Loc loc3;
    LocCode locCode3 = "BKK";
    loc3.loc() = locCode3;

    CarrierCode cc1 = "BA";
    CarrierCode cc2 = "BA";

    AirSeg airSeg1;
    airSeg1.segmentOrder() = 0;
    airSeg1.pnrSegment() = 0;
    airSeg1.geoTravelType() = GeoTravelType::International;
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    airSeg1.carrier() = cc1;

    AirSeg airSeg2;
    airSeg2.segmentOrder() = 1;
    airSeg2.pnrSegment() = 1;
    airSeg2.geoTravelType() = GeoTravelType::International;
    airSeg2.origin() = &loc2;
    airSeg2.destination() = &loc3;
    airSeg2.carrier() = cc2;

    Itin itin;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.intlSalesIndicator() = Itin::SITI;

    trx.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg2);
    trx.itin().push_back(&itin);

    FareMarket fareMarket1;
    fareMarket1.origin() = &loc1;
    fareMarket1.destination() = &loc2;

    GlobalDirection globleDirection1 = GlobalDirection::AT;
    fareMarket1.setGlobalDirection(globleDirection1);
    fareMarket1.governingCarrier() = "BA";

    FareMarket fareMarket2;
    fareMarket2.origin() = &loc2;
    fareMarket2.destination() = &loc3;

    GlobalDirection globleDirection2 = GlobalDirection::PA;
    fareMarket2.setGlobalDirection(globleDirection2);
    fareMarket2.governingCarrier() = "BA";

    fareMarket1.travelSeg().push_back(&airSeg1);
    fareMarket2.travelSeg().push_back(&airSeg2);

    //===================================

    PaxTypeFare paxtf;
    paxtf.fareMarket() = &fareMarket1;

    PaxTypeFare paxtf1;
    paxtf1.fareMarket() = &fareMarket2;

    Fare fare1;
    FareInfo fareInfo1;
    fareInfo1._globalDirection = AT;
    fareInfo1._vendor = "ATP";
    fareInfo1._carrier = "BA";
    fareInfo1._ruleNumber = "5135";
    fareInfo1._fareTariff = 1;

    TariffCrossRefInfo tariff;
    tariff._fareTariffCode = "TAFP";
    tariff._tariffCat = 0;
    tariff._ruleTariff = 304;
    tariff._routingTariff1 = 99;
    tariff._routingTariff2 = 96;

    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket1, &tariff);

    Fare fare2;

    FareInfo fareInfo2;
    fareInfo2._globalDirection = GlobalDirection::PA;
    fareInfo2._vendor = "ATP";
    fareInfo2._carrier = "BA";
    fareInfo2._ruleNumber = "5135";
    fareInfo2._fareTariff = 1;

    TariffCrossRefInfo tariffRefInfo2;

    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2, &tariff);

    PaxType paxType;
    PaxTypeCode paxTypeCode = "ADT";
    paxType.paxType() = paxTypeCode;
    paxtf.initialize(&fare1, &paxType, &fareMarket1);
    paxtf.setFare(&fare1);

    paxtf1.initialize(&fare2, &paxType, &fareMarket2);
    paxtf1.setFare(&fare2);

    FareUsage fu;
    fu.paxTypeFare() = &paxtf;
    fu.inbound() = false;
    fu.travelSeg().push_back(&airSeg1);

    FareUsage fu1;
    fu1.paxTypeFare() = &paxtf1;
    fu1.inbound() = true;
    fu1.travelSeg().push_back(&airSeg2);

    PricingUnit::Type oj = PricingUnit::Type::OPENJAW;
    PricingUnit::PUFareType sp = PricingUnit::SP;

    PricingUnit pu;
    pu.fareUsage().push_back(&fu);
    pu.fareUsage().push_back(&fu1);

    pu.geoTravelType() = GeoTravelType::International;
    pu.travelSeg().push_back(&airSeg1);
    pu.travelSeg().push_back(&airSeg2);
    pu.isSideTripPU() = false;
    pu.puFareType() = sp;
    pu.puType() = oj;

    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    farePath.paxType() = &paxType;

    PricingOptions options;
    trx.setOptions(&options);

    std::string str;

    try
    {
      CPPUNIT_ASSERT(checker.process(trx, farePath));
      str = trx.diagnostic().toString();
      // std::cout << "------------str: " << str << std::endl;
    }
    catch (ErrorResponseException& ex) { str = ex.message() + "\n"; }

    string expected = "PROCESSED EPQ CHECK";
    CPPUNIT_ASSERT(str.find(expected) != string::npos);

    expected = "PROCESSED HIP CHECK";
    CPPUNIT_ASSERT(str.find(expected) != string::npos);

    expected = "PROCESSED HIGHEST RT OJ CHECK";
    CPPUNIT_ASSERT(str.find(expected) != string::npos);
  }

  void testIATAMandatePhase2()
  {
    MinFareChecker checker;

    PricingTrx trx;

    // We need to set up the Request for our testing purposes.
    PricingRequest request;

    request.globalDirection() = GlobalDirection::WH; // Western Hemisphere

    trx.setTravelDate(DateTime::localTime());

    // request.ticketingDateTime = 0;
    Agent agent;
    Loc loca;
    loca.loc() = "LON";
    loca.nation() = "GP";

    agent.agentLocation() = &loca; // Sold in GP (Guadeloupe)

    // Build Request
    request.ticketingAgent() = &agent;
    // request.fareClassCode() =

    trx.setRequest(&request); // Set the request pointer (so pass address).

    trx.diagnostic().diagnosticType() = Diagnostic700;
    trx.diagnostic().activate();

    FarePath farePath;
    FareMarket faremkt;
    //===================================
    Loc loc1;
    LocCode locCode1 = "BKK";
    loc1.loc() = locCode1;

    Loc loc2;
    LocCode locCode2 = "TYO";
    loc2.loc() = locCode2;

    Loc loc3;
    LocCode locCode3 = "BKK";
    loc3.loc() = locCode3;

    CarrierCode cc1 = "BA";
    CarrierCode cc2 = "BA";

    AirSeg airSeg1;
    airSeg1.segmentOrder() = 0;
    airSeg1.pnrSegment() = 0;
    airSeg1.geoTravelType() = GeoTravelType::International;
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    airSeg1.carrier() = cc1;

    AirSeg airSeg2;
    airSeg2.segmentOrder() = 1;
    airSeg2.pnrSegment() = 1;
    airSeg2.geoTravelType() = GeoTravelType::International;
    airSeg2.origin() = &loc2;
    airSeg2.destination() = &loc3;
    airSeg2.carrier() = cc2;

    Itin itin;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.intlSalesIndicator() = Itin::SITI;

    trx.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg2);
    trx.itin().push_back(&itin);

    FareMarket fareMarket1;
    fareMarket1.origin() = &loc1;
    fareMarket1.destination() = &loc2;

    GlobalDirection globleDirection1 = GlobalDirection::AT;
    fareMarket1.setGlobalDirection(globleDirection1);
    fareMarket1.governingCarrier() = "BA";

    FareMarket fareMarket2;
    fareMarket2.origin() = &loc2;
    fareMarket2.destination() = &loc3;

    GlobalDirection globleDirection2 = GlobalDirection::PA;
    fareMarket2.setGlobalDirection(globleDirection2);
    fareMarket2.governingCarrier() = "BA";

    fareMarket1.travelSeg().push_back(&airSeg1);
    fareMarket2.travelSeg().push_back(&airSeg2);

    //===================================

    PaxTypeFare paxtf;
    paxtf.fareMarket() = &fareMarket1;

    PaxTypeFare paxtf1;
    paxtf1.fareMarket() = &fareMarket2;

    Fare fare1;
    FareInfo fareInfo1;
    fareInfo1._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo1;

    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket1, &tariffRefInfo1);

    Fare fare2;

    FareInfo fareInfo2;
    fareInfo2._globalDirection = GlobalDirection::PA;

    TariffCrossRefInfo tariffRefInfo2;

    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2, &tariffRefInfo2);

    PaxType paxType;
    PaxTypeCode paxTypeCode = "ADT";
    paxType.paxType() = paxTypeCode;
    paxtf.initialize(&fare1, &paxType, &fareMarket1);
    paxtf.setFare(&fare1);

    paxtf1.initialize(&fare2, &paxType, &fareMarket2);
    paxtf1.setFare(&fare2);

    FareUsage fu;
    fu.paxTypeFare() = &paxtf;
    fu.inbound() = false;
    fu.travelSeg().push_back(&airSeg1);

    FareUsage fu1;
    fu1.paxTypeFare() = &paxtf1;
    fu1.inbound() = true;
    fu1.travelSeg().push_back(&airSeg2);

    PricingUnit::Type oj = PricingUnit::Type::OPENJAW;
    PricingUnit::PUFareType sp = PricingUnit::SP;

    PricingUnit pu;
    pu.fareUsage().push_back(&fu);
    pu.fareUsage().push_back(&fu1);

    pu.puType() = oj;
    pu.puFareType() = sp;
    pu.geoTravelType() = GeoTravelType::International;
    pu.travelSeg().push_back(&airSeg1);
    pu.travelSeg().push_back(&airSeg2);

    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    farePath.paxType() = &paxType;

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic700);

    std::string response;
    try
    {
      checker.processNormalOpenJaw(trx, farePath, pu, 1, diag);
      response = diag.str();
      diag.flushMsg();
    }
    catch (ErrorResponseException& ex) { response = ex.message(); }

    string expected = "CPM CHECK DOES NOT APPLY";

    CPPUNIT_ASSERT(response.find(expected) != string::npos);
  }

  void testProcessForDummy()
  {
    PricingRequest pricingRequest;

    ExchangePricingTrx trx;
    trx.setRequest(&pricingRequest);

    PricingOptions options;
    trx.setOptions(&options);

    Itin itin;

    AirSeg trvSeg;

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(&trvSeg);

    // dummy fare have basis code and amount specified
    fareMarket.fareBasisCode() = "AAA";
    fareMarket.fareCalcFareAmt() = "123";

    std::map<const TravelSeg*, uint16_t> dummyFCSegs;
    dummyFCSegs.insert(std::make_pair(&trvSeg, 1));

    trx.exchangeOverrides().dummyFCSegs() = dummyFCSegs;

    PaxTypeFare paxTypeFare;
    paxTypeFare.fareMarket() = &fareMarket;

    FareUsage fareUsage;
    fareUsage.paxTypeFare() = &paxTypeFare;

    PricingUnit pu;
    pu.fareUsage().push_back(&fareUsage);
    pu.exemptMinFare() = false;

    std::vector<PricingUnit*> puVec;
    puVec.push_back(&pu);

    FarePath farePath;
    farePath.pricingUnit() = puVec;

    MinFareChecker minFareChecker;

    CPPUNIT_ASSERT(minFareChecker.process(trx, farePath));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MinFareCheckerTest);
}
