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
#include "test/include/TestConfigInitializer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "MinFares/CTMMinimumFare.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "Diagnostic/DCFactory.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"
#include "Common/TseConsts.h"
#include "MinFares/test/MinFareDataHandleTest.h"

namespace tse
{

class CTMMinimumFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CTMMinimumFareTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testTktDateAfterEliminationDate);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _farePath = _memHandle.create<FarePath>();
    _pricingUnit = _memHandle.create<PricingUnit>();
    _pricingUnit->puFareType() = PricingUnit::SP;
    _itin = _memHandle.create<Itin>();
    AirSeg* airSeg = makeAirSeg("SFO", "DFW", "AA");
    _itin->travelSeg().push_back(airSeg);
    _farePath->itin() = _itin;
    _ctmMinimumFare = _memHandle.create<CTMMinimumFare>(
        *_trx, *_farePath, *_pricingUnit, _ruleLevelMap, _applMap, _defaultLogicMap);
  }

  void tearDown() { _memHandle.clear(); }

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
    return as;
  }

  void testProcess()
  {
    MinFareDataHandleTest mdh;
    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic719;
    trx.diagnostic().activate();

    Loc loc;
    loc.loc() = "DFW";
    loc.nation() = "US";
    loc.subarea() = IATA_AREA1;
    loc.area() = "1";

    Loc loc1;
    loc1.loc() = "LON";
    loc1.nation() = "GB";
    loc1.subarea() = EUROPE;
    loc1.area() = "2";

    Loc loc2;
    loc2.loc() = "PAR";
    loc2.nation() = "FR";
    loc2.subarea() = EUROPE;
    loc2.area() = "2";

    Loc loc3;
    loc3.loc() = "DFW";
    loc3.nation() = "US";
    loc3.subarea() = IATA_AREA1;
    loc3.area() = "1";

    Itin itin;
    //    TSEDateTime departDT = TseUtil::getTravelDate(trx);
    //    TSEDateTime departDT;

    Agent agent;
    agent.agentLocation() = &loc;

    PricingRequest request;
    request.ticketingAgent() = &agent;

    trx.setRequest(&request);

    AirSeg travelSeg;
    travelSeg.segmentOrder() = 0;
    travelSeg.geoTravelType() = GeoTravelType::International;
    travelSeg.origin() = &loc;
    travelSeg.destination() = &loc1;
    travelSeg.boardMultiCity() = loc.loc();
    travelSeg.offMultiCity() = loc1.loc();
    travelSeg.departureDT() = DateTime::localTime();
    travelSeg.carrier() = "BA";

    AirSeg travelSeg1;
    travelSeg1.segmentOrder() = 1;
    travelSeg1.geoTravelType() = GeoTravelType::International;
    travelSeg1.origin() = &loc1;
    travelSeg1.destination() = &loc2;
    travelSeg1.boardMultiCity() = loc1.loc();
    travelSeg1.offMultiCity() = loc2.loc();
    //    travelSeg1.departureDT() = departDT;
    travelSeg1.carrier() = "BA";

    AirSeg travelSeg2;
    travelSeg2.segmentOrder() = 2;
    travelSeg2.geoTravelType() = GeoTravelType::International;
    travelSeg2.origin() = &loc2;
    travelSeg2.destination() = &loc3;
    travelSeg2.boardMultiCity() = loc2.loc();
    travelSeg2.offMultiCity() = loc3.loc();
    //    travelSeg2.departureDT() = departDT;
    travelSeg2.carrier() = "BA";

    itin.travelSeg().push_back(&travelSeg);
    itin.travelSeg().push_back(&travelSeg1);
    itin.travelSeg().push_back(&travelSeg2);

    trx.travelSeg().push_back(&travelSeg);
    trx.travelSeg().push_back(&travelSeg1);
    trx.travelSeg().push_back(&travelSeg2);

    trx.itin().push_back(&itin);

    PaxType paxType;
    paxType.paxType() = "ADT";

    Fare fare;
    fare.nucFareAmount() = 1200.00;

    FareInfo fareInfo;
    fareInfo._carrier = "BA";
    fareInfo._market1 = "DFW";
    fareInfo._market2 = "LON";
    fareInfo._fareClass = "Y";
    fareInfo._fareAmount = 1200.00;
    fareInfo._currency = "GBP";
    fareInfo._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo._ruleNumber = "5135";
    fareInfo._routingNumber = "XXXX";
    fareInfo._directionality = FROM;
    fareInfo._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";

    FareMarket fareMarket;
    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareMarket.travelSeg().push_back(&travelSeg);
    fareMarket.origin() = &loc;
    fareMarket.destination() = &loc1;
    fareMarket.boardMultiCity() = loc.loc();
    fareMarket.offMultiCity() = loc1.loc();

    GlobalDirection globleDirection = GlobalDirection::AT;
    fareMarket.setGlobalDirection(globleDirection);
    fareMarket.governingCarrier() = "BA";

    PaxTypeFare paxTypeFare;
    paxTypeFare.initialize(&fare, &paxType, &fareMarket);

    FareClassAppInfo appInfo;
    appInfo._fareType = "XEX";
    appInfo._pricingCatType = 'S';
    paxTypeFare.fareClassAppInfo() = &appInfo;

    FareClassAppSegInfo fareClassAppSegInfo;
    fareClassAppSegInfo._paxType = "ADT";
    paxTypeFare.fareClassAppSegInfo() = &fareClassAppSegInfo;
    paxTypeFare.cabin().setBusinessClass();

    // =============================================================
    Fare fare1;
    fare1.nucFareAmount() = 800.00;

    FareInfo fareInfo1;
    fareInfo1._carrier = "BA";
    fareInfo1._market1 = "LON";
    fareInfo1._market2 = "DFW";
    fareInfo1._fareClass = "YO1";
    fareInfo1._fareAmount = 800.00;
    fareInfo1._currency = "GBP";
    fareInfo1._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo1._ruleNumber = "5135";
    fareInfo1._routingNumber = "XXXX";
    fareInfo1._directionality = TO;
    fareInfo1._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo1;
    tariffRefInfo1._fareTariffCode = "TAFPBA";

    FareMarket fareMarket1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket1, &tariffRefInfo1);

    fareMarket1.travelSeg().push_back(&travelSeg1);
    fareMarket1.travelSeg().push_back(&travelSeg2);
    fareMarket1.origin() = &loc1;
    fareMarket1.destination() = &loc3;
    fareMarket1.boardMultiCity() = loc1.loc();
    fareMarket1.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection1 = GlobalDirection::AT;
    fareMarket1.setGlobalDirection(globleDirection1);
    fareMarket1.governingCarrier() = "BA";

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket1);

    FareClassAppInfo appInfo1;
    appInfo1._fareType = "XEX";
    appInfo1._pricingCatType = 'S';
    paxTypeFare1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo fareClassAppSegInfo1;
    fareClassAppSegInfo1._paxType = "ADT";
    paxTypeFare1.fareClassAppSegInfo() = &fareClassAppSegInfo1;
    paxTypeFare1.cabin().setEconomyClass();

    //=======================================================================
    Fare fare2;
    fare2.nucFareAmount() = 2200.00;

    FareInfo fareInfo2;
    fareInfo2._carrier = "BA";
    fareInfo2._market1 = "LON";
    fareInfo2._market2 = "PAR";
    fareInfo2._fareClass = "YO2";
    fareInfo2._fareAmount = 2200.00;
    fareInfo2._currency = "USD";
    fareInfo2._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo2._ruleNumber = "5135";
    fareInfo2._routingNumber = "XXXX";
    fareInfo2._directionality = FROM;
    fareInfo2._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo2;
    tariffRefInfo2._fareTariffCode = "TAFPBA";

    FareMarket fareMarket2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket, &tariffRefInfo2);

    fareMarket2.travelSeg().push_back(&travelSeg1);
    fareMarket2.origin() = &loc1;
    fareMarket2.destination() = &loc2;
    fareMarket2.boardMultiCity() = loc1.loc();
    fareMarket2.offMultiCity() = loc2.loc();

    GlobalDirection globleDirection2 = GlobalDirection::AT;
    fareMarket2.setGlobalDirection(globleDirection2);
    fareMarket2.governingCarrier() = "BA";

    PaxTypeFare paxTypeFare2;
    paxTypeFare2.initialize(&fare2, &paxType, &fareMarket2);

    FareClassAppInfo appInfo2;
    appInfo2._fareType = "EU";
    appInfo2._pricingCatType = 'N';
    paxTypeFare2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo fareClassAppSegInfo2;
    fareClassAppSegInfo2._paxType = "ADT";
    paxTypeFare2.fareClassAppSegInfo() = &fareClassAppSegInfo2;
    paxTypeFare2.cabin().setEconomyClass();

    //================================================================================
    Fare fare3;
    fare3.nucFareAmount() = 3300.00;

    FareInfo fareInfo3;
    fareInfo3._carrier = "BA";
    fareInfo3._market1 = "PAR";
    fareInfo3._market2 = "DFW";
    fareInfo3._fareClass = "YO3";
    fareInfo3._fareAmount = 3300.00;
    fareInfo3._currency = "GBP";
    fareInfo3._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo3._ruleNumber = "5135";
    fareInfo3._routingNumber = "XXXX";
    fareInfo3._directionality = TO;
    fareInfo3._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo3;
    tariffRefInfo3._fareTariffCode = "TAFPBA";

    FareMarket fareMarket3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fareMarket3, &tariffRefInfo3);

    fareMarket3.travelSeg().push_back(&travelSeg2);
    fareMarket3.origin() = &loc2;
    fareMarket3.destination() = &loc3;
    fareMarket3.boardMultiCity() = loc2.loc();
    fareMarket3.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection3 = GlobalDirection::AT;
    fareMarket3.setGlobalDirection(globleDirection3);
    fareMarket3.governingCarrier() = "BA";

    PaxTypeFare paxTypeFare3;
    paxTypeFare3.initialize(&fare3, &paxType, &fareMarket3);

    FareClassAppInfo appInfo3;
    appInfo3._fareType = "PGV";
    appInfo3._pricingCatType = 'S';
    //    appInfo3._fareType = "EU";
    //    appInfo3._pricingCatType = 'N';
    paxTypeFare3.fareClassAppInfo() = &appInfo3;

    FareClassAppSegInfo fareClassAppSegInfo3;
    fareClassAppSegInfo3._paxType = "ADT";
    paxTypeFare3.fareClassAppSegInfo() = &fareClassAppSegInfo3;
    paxTypeFare3.cabin().setEconomyClass();

    //==============================================================
    Fare fare4;
    fare4.nucFareAmount() = 4440.00;

    FareInfo fareInfo4;
    fareInfo4._carrier = "BA";
    fareInfo4._market1 = "DFW";
    fareInfo4._market2 = "PAR";
    fareInfo4._fareClass = "WMLUQOW";
    fareInfo4._fareAmount = 4440.00;
    fareInfo4._currency = "GBP";
    fareInfo4._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo4._ruleNumber = "5135";
    fareInfo4._routingNumber = "XXXX";
    fareInfo4._directionality = FROM;
    fareInfo4._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo4;
    tariffRefInfo4._fareTariffCode = "TAFPBA";

    FareMarket fareMarket4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fareMarket4, &tariffRefInfo4);

    fareMarket4.travelSeg().push_back(&travelSeg);
    fareMarket4.travelSeg().push_back(&travelSeg1);
    //    fareMarket4.travelSeg().push_back(&travelSeg2);
    fareMarket4.origin() = &loc;
    fareMarket4.destination() = &loc2;
    fareMarket4.boardMultiCity() = loc.loc();
    fareMarket4.offMultiCity() = loc2.loc();

    GlobalDirection globleDirection4 = GlobalDirection::AT;
    fareMarket4.setGlobalDirection(globleDirection4);
    fareMarket4.governingCarrier() = "BA";

    PaxTypeFare paxTypeFare4;
    paxTypeFare4.initialize(&fare4, &paxType, &fareMarket4);

    FareClassAppInfo appInfo4;
    //    appInfo4._fareType = "EU";
    //    appInfo4._pricingCatType = 'N';
    appInfo4._fareType = "XEX";
    appInfo4._pricingCatType = 'S';
    paxTypeFare4.fareClassAppInfo() = &appInfo4;

    FareClassAppSegInfo fareClassAppSegInfo4;
    fareClassAppSegInfo4._paxType = "ADT";
    paxTypeFare4.fareClassAppSegInfo() = &fareClassAppSegInfo4;
    paxTypeFare4.cabin().setEconomyClass();

    // Build FarePath
    FareUsage fareUsage;
    fareUsage.travelSeg().push_back(&travelSeg);
    fareUsage.paxTypeFare() = &paxTypeFare;

    FareUsage fareUsage1;
    fareUsage1.travelSeg().push_back(&travelSeg1);
    fareUsage1.travelSeg().push_back(&travelSeg2);
    fareUsage1.paxTypeFare() = &paxTypeFare1;

    //    FareUsage fareUsage2;
    //    fareUsage2.travelSeg().push_back(&travelSeg2);
    //    fareUsage2.paxTypeFare() = &paxTypeFare2;

    PricingUnit::Type ct = PricingUnit::Type::CIRCLETRIP;
    PricingUnit::PUFareType nl = PricingUnit::NL;

    PricingUnit pu;
    pu.fareUsage().push_back(&fareUsage);
    pu.fareUsage().push_back(&fareUsage1);
    pu.puType() = ct;
    pu.puFareType() = nl;
    pu.travelSeg().push_back(&travelSeg);
    pu.travelSeg().push_back(&travelSeg1);
    pu.travelSeg().push_back(&travelSeg2);
    //    PricingUnit pu1;
    //    pu1.fareUsage().push_back(&fareUsage1);
    //    pu1.puType() = ct;
    //    pu1.puFareType() = nl;

    //    PricingUnit pu2;
    //    pu2.fareUsage().push_back(&fareUsage2);
    //    pu2.puType() = ow;
    //    pu2.puFareType() = nl;

    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    //    farePath.pricingUnit().push_back(&pu1);
    //    farePath.pricingUnit().push_back(&pu2);
    farePath.paxType() = &paxType;

    PaxTypeBucket paxTypeCortege2;
    paxTypeCortege2.requestedPaxType() = &paxType;
    paxTypeCortege2.paxTypeFare().push_back(&paxTypeFare2);
    fareMarket2.paxTypeCortege().push_back(paxTypeCortege2);

    PaxTypeBucket paxTypeCortege3;
    paxTypeCortege3.requestedPaxType() = &paxType;
    paxTypeCortege3.paxTypeFare().push_back(&paxTypeFare3);
    fareMarket3.paxTypeCortege().push_back(paxTypeCortege3);

    PaxTypeBucket paxTypeCortege4;
    paxTypeCortege4.requestedPaxType() = &paxType;
    paxTypeCortege4.paxTypeFare().push_back(&paxTypeFare4);
    fareMarket4.paxTypeCortege().push_back(paxTypeCortege4);

    trx.fareMarket().push_back(&fareMarket);
    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket3);
    trx.fareMarket().push_back(&fareMarket4);

    std::multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;
    std::multimap<uint16_t, const MinFareAppl*> applMap;
    std::map<uint16_t, const MinFareDefaultLogic*> defaultLogicMap;

    //**************************************************************
    CTMMinimumFare ctm(trx, farePath, pu, ruleLevelMap, applMap, defaultLogicMap);
    try { CPPUNIT_ASSERT(ctm.process() >= 0.0); }
    catch (ErrorResponseException& ex)
    {
      // std::cout << ex.message() << std::endl;
    }
    // std::string str = trx.diagnostic().toString();
    // std::cout << str.c_str() << std::endl;
  }

  void testTktDateAfterEliminationDate()
  {
    _trx->getRequest()->ticketingDT() = DateTime(2013, 11, 9);
    CPPUNIT_ASSERT(!_ctmMinimumFare->qualifyCtmCheck());
  }

protected:
  PricingTrx* _trx;
  PricingRequest* _request;
  FarePath* _farePath;
  PricingUnit* _pricingUnit;
  Itin* _itin;
  CTMMinimumFare* _ctmMinimumFare;
  std::multimap<uint16_t, const MinFareRuleLevelExcl*> _ruleLevelMap;
  std::multimap<uint16_t, const MinFareAppl*> _applMap;
  std::map<uint16_t, const MinFareDefaultLogic*> _defaultLogicMap;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CTMMinimumFareTest);
}
