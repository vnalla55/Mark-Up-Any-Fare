//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include <map>

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Trx.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "MinFares/MatchDefaultLogicTable.h"
#include "MinFares/MatchExceptionTable.h"
#include "MinFares/MatchRuleLevelExclTable.h"
#include "MinFares/MinFareLogic.h"
#include "Rules/RuleUtil.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class MinFareLogicTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MinFareLogicTest);
  CPPUNIT_TEST(test_isMixedCabin_1);
  CPPUNIT_SKIP_TEST(testGetTableItem);
  CPPUNIT_TEST(testARNKsegment);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _trx = _memHandle.create<PricingTrx>(); }

  void tearDown() { _memHandle.clear(); }

  void test_isMixedCabin_1()
  {
    PricingUnit pu;
    FareUsage fareUsage[5];
    PaxTypeFare paxTypeFare[5];

    for (int i = 0; i < 5; i++)
      fareUsage[i].paxTypeFare() = &paxTypeFare[i];

    for (int i = 0; i < 5; i++)
      pu.fareUsage().push_back(&fareUsage[i]);

    paxTypeFare[0].cabin().setUndefinedClass();
    paxTypeFare[1].cabin().setFirstClass();
    paxTypeFare[2].cabin().setBusinessClass();
    paxTypeFare[3].cabin().setEconomyClass();
    paxTypeFare[4].cabin().setUnknownClass();

    ///////////////////////////////////////////////////////////////////////////
    CabinType lowestCabin;
    MinFareLogic::isMixedCabin(pu, lowestCabin);
    CPPUNIT_ASSERT(lowestCabin.isEconomyClass());
  }

  void testGetTableItem()
  {
    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic709;
    trx.diagnostic().activate();

    PaxType paxType;
    paxType.paxType() = ADULT;

    const Loc* loc = trx.dataHandle().getLoc("BRU", DateTime::localTime());
    const Loc* loc1 = trx.dataHandle().getLoc("RIO", DateTime::localTime());

    Agent agent;
    agent.agentLocation() = loc;

    PricingRequest request;
    request.ticketingAgent() = &agent;
    trx.setRequest(&request);

    trx.setTravelDate(DateTime::localTime());

    AirSeg travelSeg;
    travelSeg.segmentOrder() = 0;
    travelSeg.geoTravelType() = GeoTravelType::International;
    travelSeg.origin() = loc;
    travelSeg.destination() = loc1;
    travelSeg.departureDT() = DateTime::localTime();
    travelSeg.carrier() = "RG";

    Itin itin;
    itin.travelSeg().push_back(&travelSeg);
    itin.intlSalesIndicator() = Itin::SITI;
    itin.setTravelDate(DateTime::localTime());

    FarePath farePath;
    farePath.itin() = &itin;
    trx.itin().push_back(&itin);
    trx.travelSeg().push_back(&travelSeg);

    Fare fare;
    fare.nucFareAmount() = 3364.38;

    FareInfo fareInfo;
    fareInfo._carrier = travelSeg.carrier();
    fareInfo._market1 = loc->loc();
    fareInfo._market2 = loc1->loc();
    fareInfo._fareClass = "CRGBE";
    fareInfo._fareAmount = 3000.00;
    fareInfo._currency = "GBP";
    fareInfo._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo._ruleNumber = "2011";
    fareInfo._routingNumber = "XXXX";
    fareInfo._directionality = FROM;
    fareInfo._globalDirection = GlobalDirection::AT;
    fareInfo._vendor = "ATP";

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "CRGBE";
    tariffRefInfo._ruleTariff = 27;

    FareMarket fareMarket;
    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareMarket.travelSeg().push_back(&travelSeg);
    fareMarket.origin() = loc;
    fareMarket.destination() = loc1;
    fareMarket.boardMultiCity() = loc->loc();
    fareMarket.offMultiCity() = loc1->loc();

    GlobalDirection globleDirection = GlobalDirection::AT;
    fareMarket.setGlobalDirection(globleDirection);
    fareMarket.governingCarrier() = "RG";

    PaxTypeFare paxTypeFare;
    paxTypeFare.initialize(&fare, &paxType, &fareMarket);

    FareClassAppInfo appInfo;
    appInfo._fareType = "EU";
    appInfo._pricingCatType = 'N';
    paxTypeFare.fareClassAppInfo() = &appInfo;

    FareClassAppSegInfo fareClassAppSegInfo;
    fareClassAppSegInfo._paxType = "ADT";
    paxTypeFare.fareClassAppSegInfo() = &fareClassAppSegInfo;
    paxTypeFare.cabin().setEconomyClass();

    const PaxTypeFare* baseFareForCat17 = 0;

    int32_t tbl996ItemNo = RuleUtil::getCat17Table996ItemNo(trx, paxTypeFare, baseFareForCat17);
    CPPUNIT_ASSERT(tbl996ItemNo > 0);

    std::multimap<uint16_t, const MinFareRuleLevelExcl*> ruleMap;
    const MinFareRuleLevelExcl* ruleLevelExcl =
        MinFareLogic::getRuleLevelExcl(trx, itin, paxTypeFare, HIP, ruleMap, DateTime::emptyDate());
    CPPUNIT_ASSERT(ruleLevelExcl == 0);

    std::multimap<uint16_t, const MinFareAppl*> applMap;
    std::map<uint16_t, const MinFareDefaultLogic*> defaultLogicMap;
    const MinFareAppl* appl = MinFareLogic::getApplication(
        trx, farePath, itin, paxTypeFare, HIP, applMap, defaultLogicMap, DateTime::emptyDate());
    CPPUNIT_ASSERT(appl != 0);

    const MinFareDefaultLogic* defaultLogic =
        MinFareLogic::getDefaultLogic(HIP, trx, itin, paxTypeFare, defaultLogicMap);
    CPPUNIT_ASSERT(defaultLogic != 0);

    std::string str = trx.diagnostic().toString();
  }

  void testARNKsegment()
  {

    DateTime now(time(NULL));

    const Loc* BJS, *SHA, *SGN;
    SHA = _trx->dataHandle().getLoc("SHA", now);
    SGN = _trx->dataHandle().getLoc("SGN", now);
    BJS = _trx->dataHandle().getLoc("BJS", now);

    TravelSeg* segARNK;
    segARNK = _memHandle.create<ArunkSeg>();

    // international ARNK segment china-vietnam
    segARNK->origin() = SHA;
    segARNK->destination() = SGN;

    std::vector<TravelSeg*> travelSegVec;
    travelSegVec.clear();
    travelSegVec.push_back(segARNK);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;

    CPPUNIT_ASSERT(!MinFareLogic::isDomestic(itin, travelSegVec, false));

    // domestic ARNK segment all in china
    segARNK->destination() = BJS;

    CPPUNIT_ASSERT(MinFareLogic::isDomestic(itin, travelSegVec, false));
  }

private:
  PricingTrx* _trx;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MinFareLogicTest);
}
