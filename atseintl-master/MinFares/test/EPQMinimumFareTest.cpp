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
#include "MinFares/EPQMinimumFare.h"
#include "Diagnostic/DCFactory.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "test/include/MockGlobal.h"
#include "DataModel/Agent.h"
#include "Common/TrxUtil.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DBAccess/DiskCache.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

using namespace std;

namespace tse
{

class EPQMinimumFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EPQMinimumFareTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testProcess()
  {
    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic711;
    trx.diagnostic().activate();

    Loc loc;
    loc.loc() = "FRA";
    loc.cityInd() = true;
    loc.nation() = "DE";
    loc.area() = "2";
    loc.subarea() = EUROPE;

    Agent agent;
    agent.agentLocation() = &loc;

    PricingRequest request;
    request.ticketingAgent() = &agent;

    trx.setRequest(&request);
    trx.diagnostic().diagnosticType() = Diagnostic711;
    trx.diagnostic().activate();

    PaxType paxType;
    paxType.paxType() = "ADT";

    Itin itin;
    trx.itin().push_back(&itin);

    Loc loc1;
    loc1.loc() = "DFW";
    loc.cityInd() = true;
    loc1.nation() = "US";
    loc1.area() = "1";
    loc1.subarea() = NORTH_AMERICA;

    Loc loc2;
    loc2.loc() = "LON";
    loc.cityInd() = true;
    loc2.nation() = "GB";
    loc2.area() = "2";
    loc2.subarea() = EUROPE;

    Loc loc3;
    loc3.loc() = "FRA";
    loc.cityInd() = true;
    loc3.nation() = "DE";
    loc3.area() = "2";
    loc3.subarea() = EUROPE;

    Loc loc4;
    loc4.loc() = "YVR";
    loc.cityInd() = true;
    loc4.nation() = "CA";
    loc4.area() = "1";
    loc4.subarea() = NORTH_AMERICA;

    Loc loc5;
    loc5.loc() = "NYC";
    loc.cityInd() = true;
    loc5.nation() = "US";
    loc5.area() = "1";
    loc5.subarea() = NORTH_AMERICA;

    AirSeg trvlSeg1;
    trvlSeg1.origin() = &loc1;
    trvlSeg1.destination() = &loc2;
    trvlSeg1.stopOver() = false;

    AirSeg trvlSeg2;
    trvlSeg2.origin() = &loc2;
    trvlSeg2.destination() = &loc3;
    trvlSeg2.stopOver() = false;

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);

    FarePath farePath;
    farePath.itin() = &itin;

    Fare fare;
    fare.nucFareAmount() = 230.00;

    FareInfo fareInfo;
    fareInfo._carrier = "LH";
    fareInfo._market1 = "DFW";
    fareInfo._market2 = "FRA";
    fareInfo._fareClass = "WMLUQOW";
    fareInfo._fareAmount = 200.00;
    fareInfo._currency = "GBP";
    fareInfo._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo._ruleNumber = "5135";
    fareInfo._routingNumber = "XXXX";
    fareInfo._directionality = FROM;
    fareInfo._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";

    FareMarket fareMarket;
    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.origin() = &loc1;
    fareMarket.boardMultiCity() = loc1.loc();
    fareMarket.destination() = &loc3;
    fareMarket.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection = GlobalDirection::AT;
    fareMarket.setGlobalDirection(globleDirection);
    fareMarket.governingCarrier() = "LH";

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

    FareUsage fu;
    fu.travelSeg().push_back(&trvlSeg1);
    fu.travelSeg().push_back(&trvlSeg2);
    fu.paxTypeFare() = &paxTypeFare;

    PricingUnit pu;
    pu.fareUsage().push_back(&fu);
    pu.puType() = PricingUnit::Type::ONEWAY;

    farePath.pricingUnit().push_back(&pu);

    itin.intlSalesIndicator() = Itin::SOTI;

    const Loc* saleLoc = TrxUtil::saleLoc(trx);

    EPQMinimumFare epq;
    CPPUNIT_ASSERT(!epq.process(trx, farePath));

    // std::string str = trx.diagnostic().toString();
    // std::cout << str.c_str() << std::endl;

    CPPUNIT_ASSERT(!epq.isSITI(itin));
    CPPUNIT_ASSERT(!epq.isSIUsTerritories(*saleLoc));
    CPPUNIT_ASSERT(!epq.isSIAndOriginInArea1(*saleLoc, loc1));
    CPPUNIT_ASSERT(!epq.isOriginInWesternAfrica(*saleLoc, loc1));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EPQMinimumFareTest);
}
