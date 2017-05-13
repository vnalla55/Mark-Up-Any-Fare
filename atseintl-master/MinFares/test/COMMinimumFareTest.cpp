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

#include "DataModel/Agent.h"
#include "MinFares/COMMinimumFare.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DiskCache.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"
#include "Common/TseConsts.h"
#include "DataModel/PaxType.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "test/include/MockGlobal.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "Common/DiagMonitor.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/CppUnitHelperMacros.h"
#include "MinFares/test/MinFareDataHandleTest.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class COMMinimumFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(COMMinimumFareTest);
  CPPUNIT_TEST(testQualifyComCheck);
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

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _memHandle.create<MinFareDataHandleTest>();
  }

  void tearDown() { _memHandle.clear(); }

  void testQualifyComCheck()
  {
    PricingTrx trx;
    FarePath farePath;
    Itin itin;
    itin.setTravelDate(DateTime::emptyDate());
    farePath.itin() = &itin;
    std::multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;
    std::multimap<uint16_t, const MinFareAppl*> applMap;
    std::map<uint16_t, const MinFareDefaultLogic*> defLogicMap;

    COMMinimumFare com(trx, farePath, ruleLevelMap, applMap, defLogicMap);
    std::vector<COMMinimumFare::ComProcInfo> comProcInfoList;
    bool rc = com.qualifyComCheck(comProcInfoList);

    CPPUNIT_ASSERT(rc == false); // Not qualify because fare path is empty

    DataHandle& dh = trx.dataHandle();
    {
      const Loc* tyo, *lax, *sin;

      tyo = TestLocFactory::create("testdata/Loc_TYO.xml");
      lax = TestLocFactory::create("testdata/Loc_LAX.xml");
      sin = TestLocFactory::create("testdata/Loc_SIN.xml");

      AirSeg* s1, *s2, *s3;
      dh.get(s1);
      dh.get(s2);
      dh.get(s3);

      s1->geoTravelType() = GeoTravelType::International;
      s1->origin() = tyo;
      s1->destination() = lax;
      s1->boardMultiCity() = tyo->loc();
      s1->offMultiCity() = lax->loc();
      s1->carrier() = "KE";

      s2->geoTravelType() = GeoTravelType::International;
      s2->origin() = lax;
      s2->destination() = tyo;
      s2->boardMultiCity() = lax->loc();
      s2->offMultiCity() = tyo->loc();
      s2->carrier() = "KE";

      s3->geoTravelType() = GeoTravelType::International;
      s3->origin() = tyo;
      s3->destination() = sin;
      s3->boardMultiCity() = tyo->loc();
      s3->offMultiCity() = sin->loc();
      s3->carrier() = "JL";

      PricingUnit* pu1, *pu2;
      dh.get(pu1);
      dh.get(pu2);
      pu1->travelSeg().push_back(s1); // TYO - LAX
      pu2->travelSeg().push_back(s2); // Lax - X/TYO - SIN
      pu2->travelSeg().push_back(s3);

      farePath.pricingUnit().push_back(pu1);
      farePath.pricingUnit().push_back(pu2);
    }
    rc = com.qualifyComCheck(comProcInfoList);
    CPPUNIT_ASSERT(rc);
    CPPUNIT_ASSERT(comProcInfoList.size() > 0);
  }

  void testProcess()
  {
    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic760;
    trx.diagnostic().activate();

    Loc loc;
    loc.loc() = "FRA";
    loc.nation() = "FR";
    loc.area() = "2";
    loc.subarea() = EUROPE;

    Loc loc1;
    loc1.loc() = "LON";
    loc1.nation() = "GB";
    loc1.area() = "2";
    loc1.subarea() = EUROPE;

    Loc loc2;
    loc2.loc() = "FRA";
    loc2.nation() = "FR";
    loc2.area() = "2";
    loc2.subarea() = EUROPE;

    Loc loc3;
    loc3.loc() = "PEK";
    loc3.nation() = "CN";
    loc3.area() = "1";
    loc3.subarea() = IATA_AREA1;

    Itin itin;
    //    TSEDateTime departDT = TseUtil::getTravelDate(trx);
    //    TSEDateTime departDT;

    Agent agent;
    agent.agentLocation() = &loc;

    PricingRequest request;
    request.ticketingAgent() = &agent;

    trx.setRequest(&request);

    AirSeg travelSeg;
    travelSeg.geoTravelType() = GeoTravelType::International;
    travelSeg.origin() = &loc;
    travelSeg.destination() = &loc1;
    travelSeg.boardMultiCity() = loc.loc();
    travelSeg.offMultiCity() = loc1.loc();
    travelSeg.departureDT() = DateTime::localTime();
    travelSeg.carrier() = "BA";

    AirSeg travelSeg1;
    travelSeg1.geoTravelType() = GeoTravelType::International;
    travelSeg1.origin() = &loc1;
    travelSeg1.destination() = &loc2;
    travelSeg1.boardMultiCity() = loc1.loc();
    travelSeg1.offMultiCity() = loc2.loc();
    //    travelSeg1.departureDT() = departDT;
    travelSeg1.carrier() = "BA";

    AirSeg travelSeg2;
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
    fareInfo._market1 = "FRA";
    fareInfo._market2 = "LON";
    fareInfo._fareClass = "Y";
    fareInfo._fareAmount = 800.00;
    fareInfo._currency = "GBP";
    fareInfo._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo._ruleNumber = "5135";
    fareInfo._routingNumber = "XXXX";
    fareInfo._directionality = FROM;
    fareInfo._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";
    tariffRefInfo._tariffCat = 0;
    tariffRefInfo._ruleTariff = 304;
    tariffRefInfo._routingTariff1 = 99;
    tariffRefInfo._routingTariff2 = 96;

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
    appInfo._fareType = "EU";
    appInfo._pricingCatType = 'N';
    paxTypeFare.fareClassAppInfo() = &appInfo;

    FareClassAppSegInfo fareClassAppSegInfo;
    fareClassAppSegInfo._paxType = "ADT";
    paxTypeFare.fareClassAppSegInfo() = &fareClassAppSegInfo;
    paxTypeFare.cabin().setEconomyClass();

    // =============================================================
    Fare fare1;
    fare1.nucFareAmount() = 800.00;

    FareInfo fareInfo1;
    fareInfo1._carrier = "BA";
    fareInfo1._market1 = "LON";
    fareInfo1._market2 = "PEK";
    fareInfo1._fareClass = "YO1";
    fareInfo1._fareAmount = 600.00;
    fareInfo1._currency = "GBP";
    fareInfo1._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo1._ruleNumber = "5135";
    fareInfo1._routingNumber = "XXXX";
    fareInfo1._directionality = FROM;
    fareInfo1._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo1;
    tariffRefInfo1._fareTariffCode = "TAFPBA";
    tariffRefInfo1._tariffCat = 0;
    tariffRefInfo1._ruleTariff = 304;
    tariffRefInfo1._routingTariff1 = 99;
    tariffRefInfo1._routingTariff2 = 96;

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
    appInfo1._fareType = "EU";
    appInfo1._pricingCatType = 'N';
    paxTypeFare1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo fareClassAppSegInfo1;
    fareClassAppSegInfo1._paxType = "ADT";
    paxTypeFare1.fareClassAppSegInfo() = &fareClassAppSegInfo1;
    paxTypeFare1.cabin().setEconomyClass();

    //=======================================================================
    Fare fare2;
    fare2.nucFareAmount() = 700.00;

    FareInfo fareInfo2;
    fareInfo2._carrier = "BA";
    fareInfo2._market1 = "FRA";
    fareInfo2._market2 = "PEK";
    fareInfo2._fareClass = "YO2";
    fareInfo2._fareAmount = 700.00;
    fareInfo2._currency = "USD";
    fareInfo2._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo2._ruleNumber = "5135";
    fareInfo2._routingNumber = "XXXX";
    fareInfo2._directionality = FROM;
    fareInfo2._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo2;
    tariffRefInfo2._fareTariffCode = "TAFPBA";
    tariffRefInfo2._tariffCat = 0;
    tariffRefInfo2._ruleTariff = 304;
    tariffRefInfo2._routingTariff1 = 99;
    tariffRefInfo2._routingTariff2 = 96;

    FareMarket fareMarket2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2, &tariffRefInfo2);

    fareMarket2.travelSeg().push_back(&travelSeg2);
    fareMarket2.origin() = &loc2;
    fareMarket2.destination() = &loc3;
    fareMarket2.boardMultiCity() = loc2.loc();
    fareMarket2.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection2 = GlobalDirection::AT;
    fareMarket2.setGlobalDirection(globleDirection2);
    fareMarket2.governingCarrier() = "BA";

    PaxTypeFare paxTypeFare2;
    paxTypeFare2.initialize(&fare2, &paxType, &fareMarket2);

    FareClassAppInfo appInfo2;
    //    appInfo2._fareType = "EU";
    //    appInfo2._pricingCatType = 'N';
    appInfo2._fareType = "PGV";
    appInfo2._pricingCatType = 'S';
    paxTypeFare2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo fareClassAppSegInfo2;
    fareClassAppSegInfo2._paxType = "ADT";
    paxTypeFare2.fareClassAppSegInfo() = &fareClassAppSegInfo2;
    paxTypeFare2.cabin().setEconomyClass();

    //================================================================================
    Fare fare3;
    fare3.nucFareAmount() = 333.00;

    FareInfo fareInfo3;
    fareInfo3._carrier = "BA";
    fareInfo3._market1 = "LON";
    fareInfo3._market2 = "FRA";
    fareInfo3._fareClass = "YO3";
    fareInfo3._fareAmount = 900.00;
    fareInfo3._currency = "GBP";
    fareInfo3._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo3._ruleNumber = "5135";
    fareInfo3._routingNumber = "XXXX";
    fareInfo3._directionality = TO;
    fareInfo3._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo3;
    tariffRefInfo3._fareTariffCode = "TAFPBA";
    tariffRefInfo3._tariffCat = 0;
    tariffRefInfo3._ruleTariff = 304;
    tariffRefInfo3._routingTariff1 = 99;
    tariffRefInfo3._routingTariff2 = 96;

    FareMarket fareMarket3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fareMarket3, &tariffRefInfo3);

    fareMarket3.travelSeg().push_back(&travelSeg1);
    fareMarket3.origin() = &loc1;
    fareMarket3.destination() = &loc2;
    fareMarket3.boardMultiCity() = loc1.loc();
    fareMarket3.offMultiCity() = loc2.loc();

    GlobalDirection globleDirection3 = GlobalDirection::AT;
    fareMarket3.setGlobalDirection(globleDirection3);
    fareMarket3.governingCarrier() = "BA";

    PaxTypeFare paxTypeFare3;
    paxTypeFare3.initialize(&fare3, &paxType, &fareMarket3);

    FareClassAppInfo appInfo3;
    appInfo3._fareType = "EU";
    appInfo3._pricingCatType = 'N';
    paxTypeFare3.fareClassAppInfo() = &appInfo3;

    FareClassAppSegInfo fareClassAppSegInfo3;
    fareClassAppSegInfo3._paxType = "ADT";
    paxTypeFare3.fareClassAppSegInfo() = &fareClassAppSegInfo3;
    paxTypeFare3.cabin().setEconomyClass();

    //==============================================================
    Fare fare4;
    fare4.nucFareAmount() = 1650.00;

    FareInfo fareInfo4;
    fareInfo4._carrier = "FR";
    fareInfo4._market1 = "FRA";
    fareInfo4._market2 = "PEK";
    fareInfo4._fareClass = "WMLUQOW";
    fareInfo4._fareAmount = 1300.00;
    fareInfo4._currency = "GBP";
    fareInfo4._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo4._ruleNumber = "5135";
    fareInfo4._routingNumber = "XXXX";
    fareInfo4._directionality = FROM;
    fareInfo4._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo4;
    tariffRefInfo4._fareTariffCode = "TAFPBA";
    tariffRefInfo4._tariffCat = 0;
    tariffRefInfo4._ruleTariff = 304;
    tariffRefInfo4._routingTariff1 = 99;
    tariffRefInfo4._routingTariff2 = 96;

    FareMarket fareMarket4;
    fare4.initialize(Fare::FS_International, &fareInfo4, fareMarket4, &tariffRefInfo4);

    fareMarket4.travelSeg().push_back(&travelSeg);
    fareMarket4.travelSeg().push_back(&travelSeg1);
    fareMarket4.travelSeg().push_back(&travelSeg2);
    fareMarket4.origin() = &loc;
    fareMarket4.destination() = &loc3;
    fareMarket4.boardMultiCity() = loc.loc();
    fareMarket4.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection4 = AT;
    fareMarket4.setGlobalDirection(globleDirection4);
    fareMarket4.governingCarrier() = "BA";

    PaxTypeFare paxTypeFare4;
    paxTypeFare4.initialize(&fare4, &paxType, &fareMarket4);

    FareClassAppInfo appInfo4;
    appInfo4._fareType = "EU";
    appInfo4._pricingCatType = 'N';
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

    PricingUnit::Type ow = PricingUnit::Type::ONEWAY;
    PricingUnit::PUFareType nl = PricingUnit::NL;

    PricingUnit pu;
    pu.fareUsage().push_back(&fareUsage);
    pu.puType() = ow;
    pu.puFareType() = nl;
    pu.travelSeg().push_back(&travelSeg);

    PricingUnit pu1;
    pu1.fareUsage().push_back(&fareUsage1);
    pu1.puType() = ow;
    pu1.puFareType() = nl;
    pu1.travelSeg().push_back(&travelSeg1);
    pu1.travelSeg().push_back(&travelSeg2);

    //    PricingUnit pu2;
    //    pu2.fareUsage().push_back(&fareUsage2);
    //    pu2.puType() = ow;
    //    pu2.puFareType() = nl;

    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    farePath.pricingUnit().push_back(&pu1);
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

    //**************************************************************
    std::multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;
    std::multimap<uint16_t, const MinFareAppl*> applMap;
    std::map<uint16_t, const MinFareDefaultLogic*> defLogicMap;

    COMMinimumFare com(trx, farePath, ruleLevelMap, applMap, defLogicMap);

    CPPUNIT_ASSERT(com.process() >= 0.0);
    // std::string str = trx.diagnostic().toString();

    fare3.nucFareAmount() = 1300.00;
    CPPUNIT_ASSERT(com.process() >= 0.0);
    // str = trx.diagnostic().toString();
    // std::cout << str.c_str() << std::endl;
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(COMMinimumFareTest);
}
