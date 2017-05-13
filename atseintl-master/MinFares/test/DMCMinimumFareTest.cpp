#include "test/include/CppUnitHelperMacros.h"
#include "MinFares/DMCMinimumFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/DiskCache.h"
#include "test/include/MockGlobal.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/PaxType.h"
#include "Common/TseConsts.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DataModel/FareMarket.h"
#include "Common/Vendor.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"
#include "MinFares/test/MinFareDataHandleTest.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
using namespace std;

class DMCMinimumFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DMCMinimumFareTest);
  CPPUNIT_TEST(testQualify);
  CPPUNIT_TEST(testCompareAndSaveFare_1);
  CPPUNIT_TEST(testCompareAndSaveFare_2);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcess_MHException);
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
    // Trx
    _trx = _memHandle.create<PricingTrx>();
    _trx->diagnostic().diagnosticType() = Diagnostic762;
    _trx->diagnostic().activate();

    DCFactory* factory = DCFactory::instance();
    _diag = factory->create(*_trx);
    if (_diag)
    {
      _diag->enable(Diagnostic762);
      // if need to see the fare selection diag
      //_diag->enable(Diagnostic762, Diagnostic718);
    }
    _dmc = _memHandle.create<DMCMinimumFare>();
    _memHandle.create<MinFareDataHandleTest>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testQualify()
  {
    PricingUnit pu;

    CPPUNIT_ASSERT(_dmc->qualify(pu) == false);

    pu.puType() = PricingUnit::Type::ONEWAY;
    CPPUNIT_ASSERT(_dmc->qualify(pu) == true);

    pu.puType() = PricingUnit::Type::OPENJAW;
    pu.puFareType() = PricingUnit::NL;
    CPPUNIT_ASSERT(_dmc->qualify(pu) == true);
  }

  void testCompareAndSaveFare_1()
  {
    PaxTypeFare paxTypeFare;
    const LocCode boardPoint = "DFW";
    const LocCode offPoint = "JFK";

    MinFarePlusUpItem plusUpItem;
    plusUpItem.baseAmount = 100;
    plusUpItem.plusUpAmount = 10;

    // Setup PaxTypeFare
    Fare fare1;
    PaxTypeFare ptfFare1;
    FareInfo fareInfo1;
    FareMarket fareMarket1;
    fareMarket1.boardMultiCity() = boardPoint;
    fareMarket1.offMultiCity() = offPoint;
    fare1.initialize(Fare::FS_Domestic, &fareInfo1, fareMarket1);
    fareInfo1.currency() = "NUC";
    ptfFare1.fareMarket() = &fareMarket1;
    ptfFare1.setFare(&fare1);
    ptfFare1.nucFareAmount() = 80;

    // Setup PaxTypeFare
    Fare fare2;
    PaxTypeFare ptfFare2;
    FareInfo fareInfo2;
    FareMarket fareMarket2;
    fareMarket2.boardMultiCity() = boardPoint;
    fareMarket2.offMultiCity() = offPoint;
    fare2.initialize(Fare::FS_Domestic, &fareInfo2, fareMarket2);
    fareInfo2.currency() = "NUC";
    ptfFare2.fareMarket() = &fareMarket2;
    ptfFare2.setFare(&fare2);
    ptfFare2.nucFareAmount() = 40;

    PtfPair ptfPair(&ptfFare1, &ptfFare2);

    _dmc->_plusUp = &plusUpItem;
    int rc = _dmc->compareAndSaveFare(paxTypeFare, ptfPair, plusUpItem, true, true);

    CPPUNIT_ASSERT(rc == true);
    CPPUNIT_ASSERT(plusUpItem.baseAmount == 100);
    CPPUNIT_ASSERT(plusUpItem.plusUpAmount == 20);
  }

  void testCompareAndSaveFare_2()
  {
    PaxTypeFare thruFare, interFare;
    FareMarket interFareMarket;
    MinFarePlusUpItem curPlusUp;

    Fare fare;
    FareInfo fareInfo;
    fare.initialize(Fare::FS_Domestic, &fareInfo, interFareMarket);
    fareInfo.currency() = "NUC";
    interFare.setFare(&fare);

    interFare.nucFareAmount() = 120;
    interFare.fareMarket() = &interFareMarket;
    interFareMarket.boardMultiCity() = "DFW";
    interFareMarket.offMultiCity() = "JFK";

    curPlusUp.baseAmount = 100;
    curPlusUp.plusUpAmount = 10;

    _dmc->_plusUp = &curPlusUp;

    bool rc = _dmc->compareAndSaveFare(thruFare, interFare, curPlusUp, true, true);

    CPPUNIT_ASSERT(rc == true);
    CPPUNIT_ASSERT(curPlusUp.baseAmount == 100);
    CPPUNIT_ASSERT(curPlusUp.plusUpAmount == 20);
  }

  void testProcess()
  {
    Loc loc[3];
    setLoc(loc[0], "DEL", "IN", IATA_AREA3, "31");
    setLoc(loc[1], "TYO", "JP", IATA_AREA3, "33");
    setLoc(loc[2], "LAX", "US", IATA_AREA1, NORTH_AMERICA);

    CarrierCode crx[2] = { "JL", "KE" };

    Agent agent;
    agent.agentLocation() = &loc[0];

    PricingRequest request;
    request.ticketingAgent() = &agent;
    _trx->setRequest(&request);
    _trx->setTravelDate(DateTime::localTime());

    AirSeg airSeg[2];
    setAirSeg(airSeg[0], 0, GeoTravelType::International, &loc[0], &loc[1], crx[0]); // DELTYO
    setAirSeg(airSeg[1], 1, GeoTravelType::International, &loc[1], &loc[2], crx[1]); // TYOLAX
    airSeg[0].boardMultiCity() = loc[0].loc();
    airSeg[0].offMultiCity() = loc[1].loc();
    airSeg[1].boardMultiCity() = loc[1].loc();
    airSeg[1].offMultiCity() = loc[2].loc();

    Itin itin;
    itin.travelSeg().push_back(&airSeg[0]);
    itin.travelSeg().push_back(&airSeg[1]);
    itin.intlSalesIndicator() = Itin::SITI;

    _trx->travelSeg().push_back(&airSeg[0]);
    _trx->travelSeg().push_back(&airSeg[1]);
    _trx->itin().push_back(&itin);

    PaxType paxType;
    paxType.paxType() = "ADT";

    ///////////////////////////////////////////////////////////////////////////////
    // DEL-LAX

    Fare fare1;
    fare1.nucFareAmount() = 2582.45;

    FareInfo fareInfo1;
    fareInfo1._carrier = crx[1];
    fareInfo1._market1 = "DEL";
    fareInfo1._market2 = "LAX";
    fareInfo1._fareClass = "C";
    fareInfo1._fareAmount = 100.00;
    fareInfo1._currency = "BGP";
    fareInfo1._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo1._ruleNumber = "2000";
    fareInfo1._routingNumber = "XXXX";
    fareInfo1._directionality = FROM;
    fareInfo1._globalDirection = GlobalDirection::PA;
    fareInfo1._vendor = Vendor::ATPCO;
    fareInfo1._vendor = "   ";

    TariffCrossRefInfo tariff1;
    tariff1._fareTariffCode = "TAFPBA";
    tariff1._tariffCat = 0;
    tariff1._ruleTariff = 304;
    tariff1._routingTariff1 = 99;
    tariff1._routingTariff2 = 96;

    FareMarket fareMarket1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket1, &tariff1);

    fareMarket1.travelSeg().push_back(&airSeg[0]);
    fareMarket1.travelSeg().push_back(&airSeg[1]);
    fareMarket1.origin() = &loc[0];
    fareMarket1.destination() = &loc[2];
    fareMarket1.boardMultiCity() = loc[0].loc();
    fareMarket1.offMultiCity() = loc[2].loc();
    fareMarket1.setGlobalDirection(fareInfo1._globalDirection);
    fareMarket1.governingCarrier() = fareInfo1._carrier;

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket1);

    FareClassAppInfo appInfo1;
    appInfo1._fareType = "BU";
    appInfo1._pricingCatType = 'N';
    paxTypeFare1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo appSegInfo1;
    appSegInfo1._paxType = paxType.paxType();
    paxTypeFare1.fareClassAppSegInfo() = &appSegInfo1;
    paxTypeFare1.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege1;
    paxTypeCortege1.requestedPaxType() = &paxType;
    paxTypeCortege1.paxTypeFare().push_back(&paxTypeFare1);

    ///////////////////////////////////////////////////////////////////////////////
    // DEL-TYO
    Fare fare2;
    fare2.nucFareAmount() = 916.40;

    FareInfo fareInfo2;
    fareInfo2._carrier = crx[0];
    fareInfo2._market1 = "DEL";
    fareInfo2._market2 = "TYO";
    fareInfo2._fareClass = "C";
    fareInfo2._fareAmount = 80.00;
    fareInfo2._currency = "BGP";
    fareInfo2._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo2._ruleNumber = "2000";
    fareInfo2._routingNumber = "XXXX";
    fareInfo2._directionality = FROM;
    fareInfo2._globalDirection = GlobalDirection::EH;
    fareInfo2._vendor = Vendor::ATPCO;
    fareInfo2._vendor = "   ";

    TariffCrossRefInfo tariff2;
    tariff2._fareTariffCode = "TAFPBA";
    tariff2._tariffCat = 0;
    tariff2._ruleTariff = 304;
    tariff2._routingTariff1 = 99;
    tariff2._routingTariff2 = 96;

    FareMarket fareMarket2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2, &tariff2);

    fareMarket2.travelSeg().push_back(&airSeg[0]);
    fareMarket2.origin() = &loc[0];
    fareMarket2.destination() = &loc[1];
    fareMarket2.boardMultiCity() = loc[0].loc();
    fareMarket2.offMultiCity() = loc[1].loc();

    fareMarket2.setGlobalDirection(fareInfo2._globalDirection);
    fareMarket2.governingCarrier() = fareInfo2._carrier;

    PaxTypeFare paxTypeFare2;
    paxTypeFare2.initialize(&fare2, &paxType, &fareMarket2);

    FareClassAppInfo appInfo2;
    appInfo2._fareType = "BU";
    appInfo2._pricingCatType = 'N';
    paxTypeFare2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo appSegInfo2;
    appSegInfo2._paxType = paxType.paxType();
    paxTypeFare2.fareClassAppSegInfo() = &appSegInfo2;
    paxTypeFare2.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege2;
    paxTypeCortege2.requestedPaxType() = &paxType;
    paxTypeCortege2.paxTypeFare().push_back(&paxTypeFare2);

    ///////////////////////////////////////////////////////////////////////////////
    // TYO-LAX
    Fare fare3;
    fare3.nucFareAmount() = 2582.45;

    FareInfo fareInfo3;
    fareInfo3._carrier = crx[1];
    fareInfo3._market1 = "TYO";
    fareInfo3._market2 = "LAX";
    fareInfo3._fareClass = "C";
    fareInfo3._fareAmount = 100.00;
    fareInfo3._currency = "BGP";
    fareInfo3._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo3._ruleNumber = "2000";
    fareInfo3._routingNumber = "XXXX";
    fareInfo3._directionality = FROM;
    fareInfo3._globalDirection = GlobalDirection::PA;
    fareInfo3._vendor = Vendor::ATPCO;
    fareInfo3._vendor = "   ";

    TariffCrossRefInfo tariff3;
    tariff3._fareTariffCode = "TAFPBA";
    tariff3._tariffCat = 0;
    tariff3._ruleTariff = 304;
    tariff3._routingTariff1 = 99;
    tariff3._routingTariff2 = 96;

    FareMarket fareMarket3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fareMarket3, &tariff3);

    fareMarket3.travelSeg().push_back(&airSeg[1]);
    fareMarket3.origin() = &loc[1];
    fareMarket3.destination() = &loc[2];
    fareMarket3.boardMultiCity() = loc[1].loc();
    fareMarket3.offMultiCity() = loc[2].loc();

    fareMarket3.setGlobalDirection(fareInfo3._globalDirection);
    fareMarket3.governingCarrier() = fareInfo3._carrier;

    PaxTypeFare paxTypeFare3;
    paxTypeFare3.initialize(&fare3, &paxType, &fareMarket3);

    FareClassAppInfo appInfo3;
    appInfo3._fareType = "BU";
    appInfo3._pricingCatType = 'N';
    paxTypeFare3.fareClassAppInfo() = &appInfo3;

    FareClassAppSegInfo appSegInfo3;
    appSegInfo3._paxType = paxType.paxType();
    paxTypeFare3.fareClassAppSegInfo() = &appSegInfo3;
    paxTypeFare3.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege3;
    paxTypeCortege3.requestedPaxType() = &paxType;
    paxTypeCortege3.paxTypeFare().push_back(&paxTypeFare3);

    ///////////////////////////////////////////////////////////////////////////////
    // LAX-DEL
    Fare fare4;
    fare4.nucFareAmount() = 2438.00;

    FareInfo fareInfo4;
    fareInfo4._carrier = crx[1];
    fareInfo4._market1 = "DEL";
    fareInfo4._market2 = "LAX";
    fareInfo4._fareClass = "C2";
    fareInfo4._fareAmount = 100.00;
    fareInfo4._currency = "BGP";
    fareInfo4._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo4._ruleNumber = "2000";
    fareInfo4._routingNumber = "XXXX";
    fareInfo4._directionality = TO;
    fareInfo4._globalDirection = GlobalDirection::PA;
    fareInfo4._vendor = Vendor::ATPCO;
    fareInfo4._vendor = "   ";

    TariffCrossRefInfo tariff4;
    tariff4._fareTariffCode = "TAFPBA";
    tariff4._tariffCat = 0;
    tariff4._ruleTariff = 304;
    tariff4._routingTariff1 = 99;
    tariff4._routingTariff2 = 96;

    fare4.initialize(Fare::FS_International, &fareInfo4, fareMarket1, &tariff4);

    PaxTypeFare paxTypeFare4;
    paxTypeFare4.initialize(&fare4, &paxType, &fareMarket1);

    FareClassAppInfo appInfo4;
    appInfo4._fareType = "BU";
    appInfo4._pricingCatType = 'N';
    paxTypeFare4.fareClassAppInfo() = &appInfo4;

    FareClassAppSegInfo appSegInfo4;
    appSegInfo4._paxType = paxType.paxType();
    paxTypeFare4.fareClassAppSegInfo() = &appSegInfo4;
    paxTypeFare4.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege4;
    paxTypeCortege4.requestedPaxType() = &paxType;
    paxTypeCortege4.paxTypeFare().push_back(&paxTypeFare4);

    paxTypeCortege1.paxTypeFare().push_back(&paxTypeFare4);
    fareMarket1.paxTypeCortege().push_back(paxTypeCortege1);

    ///////////////////////////////////////////////////////////////////////////////
    // TYO-DEL
    Fare fare5;
    fare5.nucFareAmount() = 2632.17;

    FareInfo fareInfo5;
    fareInfo5._carrier = crx[0];
    fareInfo5._market1 = "DEL";
    fareInfo5._market2 = "TYO";
    fareInfo5._fareClass = "C";
    fareInfo5._fareAmount = 80.00;
    fareInfo5._currency = "BGP";
    fareInfo5._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo5._ruleNumber = "2000";
    fareInfo5._routingNumber = "XXXX";
    fareInfo5._directionality = TO;
    fareInfo5._globalDirection = GlobalDirection::EH;
    fareInfo5._vendor = Vendor::ATPCO;
    fareInfo5._vendor = "   ";

    TariffCrossRefInfo tariff5;
    tariff5._fareTariffCode = "TAFPBA";
    tariff5._tariffCat = 0;
    tariff5._ruleTariff = 304;
    tariff5._routingTariff1 = 99;
    tariff5._routingTariff2 = 96;

    fare5.initialize(Fare::FS_International, &fareInfo5, fareMarket2, &tariff5);

    PaxTypeFare paxTypeFare5;
    paxTypeFare5.initialize(&fare5, &paxType, &fareMarket2);

    FareClassAppInfo appInfo5;
    appInfo5._fareType = "BU";
    appInfo5._pricingCatType = 'N';
    paxTypeFare5.fareClassAppInfo() = &appInfo5;

    FareClassAppSegInfo appSegInfo5;
    appSegInfo5._paxType = paxType.paxType();
    paxTypeFare5.fareClassAppSegInfo() = &appSegInfo5;
    paxTypeFare5.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege5;
    paxTypeCortege5.requestedPaxType() = &paxType;
    paxTypeCortege5.paxTypeFare().push_back(&paxTypeFare5);

    paxTypeCortege2.paxTypeFare().push_back(&paxTypeFare5);
    fareMarket2.paxTypeCortege().push_back(paxTypeCortege2);

    ///////////////////////////////////////////////////////////////////////////////
    // LAX-TYO
    Fare fare6;
    fare6.nucFareAmount() = 2190.00;

    FareInfo fareInfo6;
    fareInfo6._carrier = crx[1];
    fareInfo6._market1 = "TYO";
    fareInfo6._market2 = "LAX";
    fareInfo6._fareClass = "C";
    fareInfo6._fareAmount = 100.00;
    fareInfo6._currency = "BGP";
    fareInfo6._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo6._ruleNumber = "2000";
    fareInfo6._routingNumber = "XXXX";
    fareInfo6._directionality = TO;
    fareInfo6._globalDirection = GlobalDirection::PA;
    fareInfo6._vendor = Vendor::ATPCO;
    fareInfo6._vendor = "   ";

    TariffCrossRefInfo tariff6;
    tariff6._fareTariffCode = "TAFPBA";
    tariff6._tariffCat = 0;
    tariff6._ruleTariff = 304;
    tariff6._routingTariff1 = 99;
    tariff6._routingTariff2 = 96;

    fare6.initialize(Fare::FS_International, &fareInfo6, fareMarket3, &tariff6);

    PaxTypeFare paxTypeFare6;
    paxTypeFare6.initialize(&fare6, &paxType, &fareMarket3);

    FareClassAppInfo appInfo6;
    appInfo6._fareType = "BU";
    appInfo6._pricingCatType = 'N';
    paxTypeFare6.fareClassAppInfo() = &appInfo6;

    FareClassAppSegInfo appSegInfo6;
    appSegInfo6._paxType = paxType.paxType();
    paxTypeFare6.fareClassAppSegInfo() = &appSegInfo6;
    paxTypeFare6.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege6;
    paxTypeCortege6.requestedPaxType() = &paxType;
    paxTypeCortege6.paxTypeFare().push_back(&paxTypeFare6);

    paxTypeCortege3.paxTypeFare().push_back(&paxTypeFare6);
    fareMarket3.paxTypeCortege().push_back(paxTypeCortege3);

    ///////////////////////////////////////////////////////////////////////////////
    // FareUsage
    FareUsage fareUsage;
    fareUsage.travelSeg().push_back(&airSeg[0]);
    fareUsage.travelSeg().push_back(&airSeg[1]);
    fareUsage.paxTypeFare() = &paxTypeFare1;

    ///////////////////////////////////////////////////////////////////////////////
    // Pricing Unit
    PricingUnit pu;
    pu.fareUsage().push_back(&fareUsage);
    pu.puType() = PricingUnit::Type::ONEWAY;
    pu.puFareType() = PricingUnit::NL;
    // pu.puFareType() = PricingUnit::SP;

    ///////////////////////////////////////////////////////////////////////////////
    // FarePath
    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    farePath.paxType() = &paxType;

    ///////////////////////////////////////////////////////////////////////////////
    // Trx.FareMarket
    _trx->fareMarket().push_back(&fareMarket1);
    _trx->fareMarket().push_back(&fareMarket2);
    _trx->fareMarket().push_back(&fareMarket3);

    CPPUNIT_ASSERT_MESSAGE("No PU in FarePath", farePath.pricingUnit().size() > 0);
    CPPUNIT_ASSERT_MESSAGE("No AirSeg in Itin", farePath.itin()->travelSeg().size() > 0);
    CPPUNIT_ASSERT_MESSAGE("No FareUsage in PU", pu.fareUsage().size() > 0);

    CPPUNIT_ASSERT(_dmc->process(*_trx, farePath, pu) >= 0.0);
  }

  void testProcess_MHException()
  {
    DateTime now(time(NULL));

    const Loc* loc[3];
    loc[0] = _trx->dataHandle().getLoc("SIN", now);
    loc[1] = _trx->dataHandle().getLoc("KUL", now);
    loc[2] = _trx->dataHandle().getLoc("BWN", now);

    CarrierCode crx[2] = { "MH", "MH" };

    Agent agent;
    agent.agentLocation() = loc[0];

    PricingRequest request;
    request.ticketingAgent() = &agent;
    _trx->setRequest(&request);

    AirSeg airSeg[2];
    setAirSeg(airSeg[0], 0, GeoTravelType::International, loc[0], loc[1], crx[0]); // SINKUL
    setAirSeg(airSeg[1], 1, GeoTravelType::International, loc[1], loc[2], crx[1]); // KULBWN

    Itin itin;
    itin.travelSeg().push_back(&airSeg[0]);
    itin.travelSeg().push_back(&airSeg[1]);
    itin.intlSalesIndicator() = Itin::SITI;

    _trx->travelSeg().push_back(&airSeg[0]);
    _trx->travelSeg().push_back(&airSeg[1]);
    _trx->itin().push_back(&itin);

    PaxType paxType;
    paxType.paxType() = "ADT";

    ///////////////////////////////////////////////////////////////////////////////
    // DEL-LAX

    Fare fare1;
    fare1.nucFareAmount() = 2582.45;

    FareInfo fareInfo1;
    fareInfo1._carrier = crx[1];
    fareInfo1._market1 = "DEL";
    fareInfo1._market2 = "LAX";
    fareInfo1._fareClass = "C";
    fareInfo1._fareAmount = 100.00;
    fareInfo1._currency = "BGP";
    fareInfo1._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo1._ruleNumber = "2000";
    fareInfo1._routingNumber = "XXXX";
    fareInfo1._directionality = FROM;
    fareInfo1._globalDirection = GlobalDirection::PA;
    fareInfo1._vendor = Vendor::ATPCO;
    fareInfo1._vendor = "   ";

    TariffCrossRefInfo tariff1;
    tariff1._fareTariffCode = "TAFPBA";
    tariff1._tariffCat = 0;
    tariff1._ruleTariff = 304;
    tariff1._routingTariff1 = 99;
    tariff1._routingTariff2 = 96;

    FareMarket fareMarket1;
    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket1, &tariff1);

    fareMarket1.travelSeg().push_back(&airSeg[0]);
    fareMarket1.travelSeg().push_back(&airSeg[1]);
    fareMarket1.origin() = loc[0];
    fareMarket1.destination() = loc[2];
    fareMarket1.boardMultiCity() = loc[0]->loc();
    fareMarket1.offMultiCity() = loc[2]->loc();

    fareMarket1.setGlobalDirection(fareInfo1._globalDirection);
    fareMarket1.governingCarrier() = fareInfo1._carrier;

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket1);

    FareClassAppInfo appInfo1;
    appInfo1._fareType = "BU";
    appInfo1._pricingCatType = 'N';
    paxTypeFare1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo appSegInfo1;
    appSegInfo1._paxType = paxType.paxType();
    paxTypeFare1.fareClassAppSegInfo() = &appSegInfo1;
    paxTypeFare1.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege1;
    paxTypeCortege1.requestedPaxType() = &paxType;
    paxTypeCortege1.paxTypeFare().push_back(&paxTypeFare1);

    ///////////////////////////////////////////////////////////////////////////////
    // DEL-TYO
    Fare fare2;
    fare2.nucFareAmount() = 916.40;

    FareInfo fareInfo2;
    fareInfo2._carrier = crx[0];
    fareInfo2._market1 = "DEL";
    fareInfo2._market2 = "TYO";
    fareInfo2._fareClass = "C";
    fareInfo2._fareAmount = 80.00;
    fareInfo2._currency = "BGP";
    fareInfo2._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo2._ruleNumber = "2000";
    fareInfo2._routingNumber = "XXXX";
    fareInfo2._directionality = FROM;
    fareInfo2._globalDirection = GlobalDirection::EH;
    fareInfo2._vendor = Vendor::ATPCO;
    fareInfo2._vendor = "   ";

    TariffCrossRefInfo tariff2;
    tariff2._fareTariffCode = "TAFPBA";
    tariff2._tariffCat = 0;
    tariff2._ruleTariff = 304;
    tariff2._routingTariff1 = 99;
    tariff2._routingTariff2 = 96;

    FareMarket fareMarket2;
    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2, &tariff2);

    fareMarket2.travelSeg().push_back(&airSeg[0]);
    fareMarket2.origin() = loc[0];
    fareMarket2.destination() = loc[1];
    fareMarket2.setGlobalDirection(fareInfo2._globalDirection);
    fareMarket2.governingCarrier() = fareInfo2._carrier;

    PaxTypeFare paxTypeFare2;
    paxTypeFare2.initialize(&fare2, &paxType, &fareMarket2);

    FareClassAppInfo appInfo2;
    appInfo2._fareType = "BU";
    appInfo2._pricingCatType = 'N';
    paxTypeFare2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo appSegInfo2;
    appSegInfo2._paxType = paxType.paxType();
    paxTypeFare2.fareClassAppSegInfo() = &appSegInfo2;
    paxTypeFare2.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege2;
    paxTypeCortege2.requestedPaxType() = &paxType;
    paxTypeCortege2.paxTypeFare().push_back(&paxTypeFare2);

    ///////////////////////////////////////////////////////////////////////////////
    // TYO-LAX
    Fare fare3;
    fare3.nucFareAmount() = 2582.45;

    FareInfo fareInfo3;
    fareInfo3._carrier = crx[1];
    fareInfo3._market1 = "TYO";
    fareInfo3._market2 = "LAX";
    fareInfo3._fareClass = "C";
    fareInfo3._fareAmount = 100.00;
    fareInfo3._currency = "BGP";
    fareInfo3._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo3._ruleNumber = "2000";
    fareInfo3._routingNumber = "XXXX";
    fareInfo3._directionality = FROM;
    fareInfo3._globalDirection = GlobalDirection::PA;
    fareInfo2._vendor = Vendor::ATPCO;
    fareInfo2._vendor = "   ";

    TariffCrossRefInfo tariff3;
    tariff3._fareTariffCode = "TAFPBA";
    tariff3._tariffCat = 0;
    tariff3._ruleTariff = 304;
    tariff3._routingTariff1 = 99;
    tariff3._routingTariff2 = 96;

    FareMarket fareMarket3;
    fare3.initialize(Fare::FS_International, &fareInfo3, fareMarket3, &tariff3);

    fareMarket3.travelSeg().push_back(&airSeg[1]);
    fareMarket3.origin() = loc[1];
    fareMarket3.destination() = loc[2];
    fareMarket3.boardMultiCity() = loc[1]->loc();
    fareMarket3.offMultiCity() = loc[2]->loc();

    fareMarket3.setGlobalDirection(fareInfo3._globalDirection);
    fareMarket3.governingCarrier() = fareInfo3._carrier;

    PaxTypeFare paxTypeFare3;
    paxTypeFare3.initialize(&fare3, &paxType, &fareMarket3);

    FareClassAppInfo appInfo3;
    appInfo3._fareType = "BU";
    appInfo3._pricingCatType = 'N';
    paxTypeFare3.fareClassAppInfo() = &appInfo3;

    FareClassAppSegInfo appSegInfo3;
    appSegInfo3._paxType = paxType.paxType();
    paxTypeFare3.fareClassAppSegInfo() = &appSegInfo3;
    paxTypeFare3.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege3;
    paxTypeCortege3.requestedPaxType() = &paxType;
    paxTypeCortege3.paxTypeFare().push_back(&paxTypeFare3);

    ///////////////////////////////////////////////////////////////////////////////
    // LAX-DEL
    Fare fare4;
    fare4.nucFareAmount() = 2438.00;

    FareInfo fareInfo4;
    fareInfo4._carrier = crx[1];
    fareInfo4._market1 = "DEL";
    fareInfo4._market2 = "LAX";
    fareInfo4._fareClass = "C2";
    fareInfo4._fareAmount = 100.00;
    fareInfo4._currency = "BGP";
    fareInfo4._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo4._ruleNumber = "2000";
    fareInfo4._routingNumber = "XXXX";
    fareInfo4._directionality = TO;
    fareInfo4._globalDirection = GlobalDirection::PA;
    fareInfo4._vendor = Vendor::ATPCO;
    fareInfo4._vendor = "   ";

    TariffCrossRefInfo tariff4;
    tariff4._fareTariffCode = "TAFPBA";
    tariff4._tariffCat = 0;
    tariff4._ruleTariff = 304;
    tariff4._routingTariff1 = 99;
    tariff4._routingTariff2 = 96;

    fare4.initialize(Fare::FS_International, &fareInfo4, fareMarket1, &tariff4);

    PaxTypeFare paxTypeFare4;
    paxTypeFare4.initialize(&fare4, &paxType, &fareMarket1);

    FareClassAppInfo appInfo4;
    appInfo4._fareType = "BU";
    appInfo4._pricingCatType = 'N';
    paxTypeFare4.fareClassAppInfo() = &appInfo4;

    FareClassAppSegInfo appSegInfo4;
    appSegInfo4._paxType = paxType.paxType();
    paxTypeFare4.fareClassAppSegInfo() = &appSegInfo4;
    paxTypeFare4.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege4;
    paxTypeCortege4.requestedPaxType() = &paxType;
    paxTypeCortege4.paxTypeFare().push_back(&paxTypeFare4);

    paxTypeCortege1.paxTypeFare().push_back(&paxTypeFare4);
    fareMarket1.paxTypeCortege().push_back(paxTypeCortege1);

    ///////////////////////////////////////////////////////////////////////////////
    // TYO-DEL
    Fare fare5;
    fare5.nucFareAmount() = 2632.17;

    FareInfo fareInfo5;
    fareInfo5._carrier = crx[0];
    fareInfo5._market1 = "DEL";
    fareInfo5._market2 = "TYO";
    fareInfo5._fareClass = "C";
    fareInfo5._fareAmount = 80.00;
    fareInfo5._currency = "BGP";
    fareInfo5._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo5._ruleNumber = "2000";
    fareInfo5._routingNumber = "XXXX";
    fareInfo5._directionality = TO;
    fareInfo5._globalDirection = GlobalDirection::EH;
    fareInfo5._vendor = Vendor::ATPCO;
    fareInfo5._vendor = "   ";

    TariffCrossRefInfo tariff5;
    tariff5._fareTariffCode = "TAFPBA";
    tariff5._tariffCat = 0;
    tariff5._ruleTariff = 304;
    tariff5._routingTariff1 = 99;
    tariff5._routingTariff2 = 96;

    fare5.initialize(Fare::FS_International, &fareInfo5, fareMarket2, &tariff5);

    PaxTypeFare paxTypeFare5;
    paxTypeFare5.initialize(&fare5, &paxType, &fareMarket2);

    FareClassAppInfo appInfo5;
    appInfo5._fareType = "BU";
    appInfo5._pricingCatType = 'N';
    paxTypeFare5.fareClassAppInfo() = &appInfo5;

    FareClassAppSegInfo appSegInfo5;
    appSegInfo5._paxType = paxType.paxType();
    paxTypeFare5.fareClassAppSegInfo() = &appSegInfo5;
    paxTypeFare5.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege5;
    paxTypeCortege5.requestedPaxType() = &paxType;
    paxTypeCortege5.paxTypeFare().push_back(&paxTypeFare5);

    paxTypeCortege2.paxTypeFare().push_back(&paxTypeFare5);
    fareMarket2.paxTypeCortege().push_back(paxTypeCortege2);

    ///////////////////////////////////////////////////////////////////////////////
    // LAX-TYO
    Fare fare6;
    fare6.nucFareAmount() = 2190.00;

    FareInfo fareInfo6;
    fareInfo6._carrier = crx[1];
    fareInfo6._market1 = "TYO";
    fareInfo6._market2 = "LAX";
    fareInfo6._fareClass = "C";
    fareInfo6._fareAmount = 100.00;
    fareInfo6._currency = "BGP";
    fareInfo6._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo6._ruleNumber = "2000";
    fareInfo6._routingNumber = "XXXX";
    fareInfo6._directionality = TO;
    fareInfo6._globalDirection = GlobalDirection::PA;
    fareInfo6._vendor = Vendor::ATPCO;
    fareInfo6._vendor = "   ";

    TariffCrossRefInfo tariff6;
    tariff6._fareTariffCode = "TAFPBA";
    tariff6._tariffCat = 0;
    tariff6._ruleTariff = 304;
    tariff6._routingTariff1 = 99;
    tariff6._routingTariff2 = 96;

    fare6.initialize(Fare::FS_International, &fareInfo6, fareMarket3, &tariff6);

    PaxTypeFare paxTypeFare6;
    paxTypeFare6.initialize(&fare6, &paxType, &fareMarket3);

    FareClassAppInfo appInfo6;
    appInfo6._fareType = "BU";
    appInfo6._pricingCatType = 'N';
    paxTypeFare6.fareClassAppInfo() = &appInfo6;

    FareClassAppSegInfo appSegInfo6;
    appSegInfo6._paxType = paxType.paxType();
    paxTypeFare6.fareClassAppSegInfo() = &appSegInfo6;
    paxTypeFare6.cabin().setBusinessClass();

    PaxTypeBucket paxTypeCortege6;
    paxTypeCortege6.requestedPaxType() = &paxType;
    paxTypeCortege6.paxTypeFare().push_back(&paxTypeFare6);

    paxTypeCortege3.paxTypeFare().push_back(&paxTypeFare6);
    fareMarket3.paxTypeCortege().push_back(paxTypeCortege3);

    ///////////////////////////////////////////////////////////////////////////////
    // FareUsage
    FareUsage fareUsage[2];
    fareUsage[0].travelSeg().push_back(&airSeg[0]);
    fareUsage[1].travelSeg().push_back(&airSeg[1]);

    fareUsage[0].paxTypeFare() = &paxTypeFare1;
    fareUsage[1].paxTypeFare() = &paxTypeFare2;

    ///////////////////////////////////////////////////////////////////////////////
    // Pricing Unit
    PricingUnit pu[2];
    pu[0].fareUsage().push_back(&fareUsage[0]);
    pu[0].puType() = PricingUnit::Type::ONEWAY;
    pu[0].puFareType() = PricingUnit::NL;
    pu[0].geoTravelType() = GeoTravelType::International;

    pu[1].fareUsage().push_back(&fareUsage[1]);
    pu[1].puType() = PricingUnit::Type::ONEWAY;
    pu[1].puFareType() = PricingUnit::NL;
    pu[1].geoTravelType() = GeoTravelType::International;

    ///////////////////////////////////////////////////////////////////////////////
    // FarePath
    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu[0]);
    farePath.pricingUnit().push_back(&pu[1]);
    farePath.paxType() = &paxType;

    ///////////////////////////////////////////////////////////////////////////////
    // Trx.FareMarket
    _trx->fareMarket().push_back(&fareMarket1);
    _trx->fareMarket().push_back(&fareMarket2);
    _trx->fareMarket().push_back(&fareMarket3);

    // Excluded from DMC processing due to MH exception
    CPPUNIT_ASSERT(_dmc->qualifyMHException(*_diag, *_trx, farePath));
  }

protected:
  void setLoc(Loc& loc,
              const string& city,
              const string& nation,
              const string& area,
              const string& subarea = "")
  {
    loc.loc() = city;
    loc.cityInd() = true;
    loc.nation() = nation;
    loc.area() = area;
    if (subarea != "")
    {
      loc.subarea() = subarea;
    }
  }

  void setAirSeg(AirSeg& seg,
                 int segOrder,
                 GeoTravelType geoTravelType,
                 const Loc* origin,
                 const Loc* destination,
                 const string& carrier)
  {
    static DateTime now(time(NULL));
    seg.segmentOrder() = segOrder;
    seg.pnrSegment() = segOrder;
    seg.geoTravelType() = geoTravelType;
    seg.origin() = origin;
    seg.destination() = destination;
    seg.carrier() = carrier;
    seg.departureDT() = now;
  }

private:
  PricingTrx* _trx;
  DiagCollector* _diag;
  TestMemHandle _memHandle;
  DMCMinimumFare* _dmc;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DMCMinimumFareTest);
}
