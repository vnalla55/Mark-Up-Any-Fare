#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "Diagnostic/DiagCollector.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/HighRTChecker.h"
#include "MinFares/OJMMinimumFare.h"
#include "MinFares/RTMinimumFare.h"
#include "Rules/RuleConst.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace tse
{

class OJMAndRTMinimumFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OJMAndRTMinimumFareTest);
  CPPUNIT_TEST(test_04);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testCalculateFareAmtReturnFalseWhenNoFare);
  CPPUNIT_TEST(testCalculateFareAmtReturnTrueWhenFaresPresent);
  CPPUNIT_TEST(testCalculateFareAmtCheckCalcResult);
  CPPUNIT_TEST(testCalculateFareAmtCheckCalcResultForZeroAmts);
  CPPUNIT_TEST(testCalculateFareAmtCheckIfDifferentialNotAdded);
  CPPUNIT_TEST(testCalculateFareAmtCheckIfMileageSurchargeAdded);
  CPPUNIT_TEST(testProcessNetFaresDontAddPlusUpWhenNetRemitFp);
  CPPUNIT_TEST(testProcessNetFaresDontAddPlusUpWhenIsNotWpNet);
  CPPUNIT_TEST(testProcessNetFaresAddPlusUp);
  CPPUNIT_TEST(testProcessNetFaresCheckPlusUpAmt);
  CPPUNIT_TEST(testProcessNetFaresCheckPlusUpNullWhenDifferenceIsZero);
  CPPUNIT_TEST(testIsSameCabinReturnTrueWhenBothFirst);
  CPPUNIT_TEST(testIsSameCabinReturnTrueWhenFirstAndPremiumFirst);
  CPPUNIT_TEST(testIsSameCabinReturnFalseWhenFirstAndEconomy);
  CPPUNIT_TEST(testIsSameCabinReturnFalseWhenFirstAndPremiumBusiness);
  CPPUNIT_TEST(testIsSameCabinReturnTrueWhenBothBusiness);
  CPPUNIT_TEST(testIsSameCabinReturnTrueWhenBothPremiumBusiness);
  CPPUNIT_TEST(testIsSameCabinReturnTrueWhenBusinessAndPremiumBusiness);
  CPPUNIT_TEST(testIsSameCabinReturnFalseWhenBusinessAndEconomy);
  CPPUNIT_TEST(testIsSameCabinReturnFalseWhenBusinessAndPremiumFirst);
  CPPUNIT_TEST(testIsSameCabinReturnFalseWhenBusinessAndPremiumEconomy);
  CPPUNIT_TEST(testIsSameCabinReturnTrueWhenBothEconomy);
  CPPUNIT_TEST(testIsSameCabinReturnTrueWhenBothPremiumEconomy);
  CPPUNIT_TEST(testIsSameCabinReturnTrueWhenEconomyAndPremiumEconomy);
  CPPUNIT_TEST(testIsSameCabinReturnFalseWhenEconomyAndFirst);
  CPPUNIT_TEST(testIsSameCabinReturnFalseWhenEconomyAndPremiumFirst);
  CPPUNIT_TEST(testIsSameCabinReturnFalseWhenPremiumEconomyAndBusiness);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _pu = _memHandle.create<PricingUnit>();
    _farePath = _memHandle.create<FarePath>();
    _itin = _memHandle.create<Itin>();
    _farePath->itin() = _itin;
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
  }


  void setUpOjm()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic767;
    _trx->diagnostic().activate();

    _diag = DCFactory::instance()->create(*_trx);
    if (_diag)
    {
      _diag->enable(Diagnostic767);
    }

    _pu->puType() = PricingUnit::Type::OPENJAW;
    _ojm = _memHandle.create<OJMMinimumFare>(*_trx, *_farePath);
  }

  void setUpRt()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic768;
    _trx->diagnostic().activate();

    _diag = DCFactory::instance()->create(*_trx);
    if (_diag)
    {
      _diag->enable(Diagnostic768);
    }

    _pu->puType() = PricingUnit::Type::ROUNDTRIP;
    _rtm = _memHandle.create<RTMinimumFare>(*_trx, *_farePath);
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void test_04()
  {
    DateTime now(time(NULL));

    const Loc* loc[5];
    loc[0] = _trx->dataHandle().getLoc("SIN", now);
    loc[1] = _trx->dataHandle().getLoc("LHR", now);
    loc[2] = _trx->dataHandle().getLoc("LON", now);
    loc[3] = _trx->dataHandle().getLoc("CDG", now);
    loc[4] = _trx->dataHandle().getLoc("LHR", now);

    Agent agent;
    agent.agentLocation() = loc[0];

    CarrierCode crx[2] = { "BA", "AA" };

    AirSeg airSeg[4];
    ArunkSeg arnkSeg;
    setAirSeg(airSeg[0], 1, GeoTravelType::International, loc[0], loc[1], crx[0]); // MANORD
    setAirSeg(airSeg[1], 2, GeoTravelType::Domestic, loc[1], loc[2], crx[1]); // ORDDFW
    setArunkSeg(arnkSeg, 3, GeoTravelType::Domestic, loc[2], loc[3]); // DFWJFK
    setAirSeg(airSeg[2], 4, GeoTravelType::International, loc[3], loc[4], crx[1]); // JFKLHR
    setAirSeg(airSeg[3], 5, GeoTravelType::Domestic, loc[4], loc[0], crx[1]); // LHRMAN

    PricingRequest request;
    request.ticketingAgent() = &agent;
    _trx->setRequest(&request);

    _itin->travelSeg().push_back(&airSeg[0]);
    _itin->travelSeg().push_back(&airSeg[1]);
    _itin->travelSeg().push_back(&arnkSeg);
    _itin->travelSeg().push_back(&airSeg[2]);
    _itin->travelSeg().push_back(&airSeg[3]);
    _itin->intlSalesIndicator() = Itin::SITI;

    _trx->travelSeg().push_back(&airSeg[0]);
    _trx->travelSeg().push_back(&airSeg[1]);
    _trx->travelSeg().push_back(&arnkSeg);
    _trx->travelSeg().push_back(&airSeg[2]);
    _trx->travelSeg().push_back(&airSeg[3]);
    _trx->itin().push_back(_itin);

    PaxType paxType;
    paxType.paxType() = "ADT";

    ///////////////////////////////////////////////////////////////////////////////
    // TariffCrossRefInfo
    TariffCrossRefInfo tariff[2];

    tariff[0]._fareTariffCode = "TAFP";
    tariff[0]._tariffCat = 0;
    tariff[0]._ruleTariff = 304;
    tariff[0]._routingTariff1 = 99;
    tariff[0]._routingTariff2 = 96;

    tariff[1]._fareTariffCode = "TAFP";
    tariff[1]._tariffCat = 0;
    tariff[1]._ruleTariff = 304;
    tariff[1]._routingTariff1 = 99;
    tariff[1]._routingTariff2 = 96;

    ///////////////////////////////////////////////////////////////////////////////
    // FareInfo
    FareInfo fareInfo[2];

    fareInfo[0]._vendor = Vendor::ATPCO;
    fareInfo[0]._carrier = "AA";
    fareInfo[0]._market1 = "SIN";
    fareInfo[0]._market2 = "LON";
    fareInfo[0]._fareClass = "Y";
    fareInfo[0]._fareTariff = 1;
    fareInfo[0]._fareAmount = 1260;
    fareInfo[0]._originalFareAmount = 1260;
    fareInfo[0]._currency = "BGP";
    fareInfo[0]._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo[0]._ruleNumber = "2000";
    fareInfo[0]._routingNumber = "XXXX";
    fareInfo[0]._directionality = TO;
    fareInfo[0]._globalDirection = GlobalDirection::PA;

    fareInfo[1]._vendor = Vendor::ATPCO;
    fareInfo[1]._carrier = "AA";
    fareInfo[1]._market1 = "PAR";
    fareInfo[1]._market2 = "SIN";
    fareInfo[1]._fareClass = "Y";
    fareInfo[1]._fareTariff = 1;
    fareInfo[1]._fareAmount = 1260;
    fareInfo[1]._originalFareAmount = 1260;
    fareInfo[1]._currency = "BGP";
    fareInfo[1]._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo[1]._ruleNumber = "2000";
    fareInfo[1]._routingNumber = "XXXX";
    fareInfo[1]._directionality = TO;
    fareInfo[1]._globalDirection = GlobalDirection::PA;

    ///////////////////////////////////////////////////////////////////////////////
    // Fare
    Fare fare[2];
    fare[0].nucFareAmount() = 722.57;
    fare[1].nucFareAmount() = 695.30;

    ///////////////////////////////////////////////////////////////////////////////
    // FareClassAppInfo
    FareClassAppInfo appInfo[2];
    appInfo[0]._fareType = "BU";
    appInfo[0]._pricingCatType = 'N';
    appInfo[1]._fareType = "BU";
    appInfo[1]._pricingCatType = 'N';

    ///////////////////////////////////////////////////////////////////////////////
    // FareClassAppSegInfo
    FareClassAppSegInfo appSegInfo[2];
    appSegInfo[0]._paxType = paxType.paxType();
    appSegInfo[1]._paxType = paxType.paxType();

    ///////////////////////////////////////////////////////////////////////////////
    // FareMarket
    FareMarket fareMarket[2];

    fare[0].initialize(Fare::FS_International, &fareInfo[0], fareMarket[0], &tariff[0]);

    fare[1].initialize(Fare::FS_International, &fareInfo[1], fareMarket[1], &tariff[1]);

    fareMarket[0].travelSeg().push_back(&airSeg[0]);
    fareMarket[0].travelSeg().push_back(&airSeg[1]);
    fareMarket[0].origin() = loc[0];
    fareMarket[0].destination() = loc[2];
    fareMarket[0].setGlobalDirection(fareInfo[0]._globalDirection);
    fareMarket[0].governingCarrier() = fareInfo[0]._carrier;

    fareMarket[1].travelSeg().push_back(&airSeg[2]);
    fareMarket[1].travelSeg().push_back(&airSeg[3]);
    fareMarket[1].origin() = loc[3];
    fareMarket[1].destination() = loc[0];
    fareMarket[1].setGlobalDirection(fareInfo[1]._globalDirection);
    fareMarket[1].governingCarrier() = fareInfo[1]._carrier;

    ///////////////////////////////////////////////////////////////////////////////
    // PaxTypeFare
    PaxTypeFare paxTypeFare[2];
    paxTypeFare[0].initialize(&fare[0], &paxType, &fareMarket[0]);
    paxTypeFare[0].fareClassAppInfo() = &appInfo[0];
    paxTypeFare[0].fareClassAppSegInfo() = &appSegInfo[0];

    paxTypeFare[1].initialize(&fare[1], &paxType, &fareMarket[1]);
    paxTypeFare[1].fareClassAppInfo() = &appInfo[1];
    paxTypeFare[1].fareClassAppSegInfo() = &appSegInfo[1];

    // setup for same cabin
    paxTypeFare[0].cabin().setBusinessClass();
    paxTypeFare[1].cabin().setBusinessClass();

    ///////////////////////////////////////////////////////////////////////////////
    // PaxTypeBucket
    PaxTypeBucket paxTypeCortege[2];
    paxTypeCortege[0].requestedPaxType() = &paxType;
    paxTypeCortege[1].requestedPaxType() = &paxType;

    paxTypeCortege[0].paxTypeFare().push_back(&paxTypeFare[0]);
    paxTypeCortege[1].paxTypeFare().push_back(&paxTypeFare[1]);

    fareMarket[0].paxTypeCortege().push_back(paxTypeCortege[0]);
    fareMarket[1].paxTypeCortege().push_back(paxTypeCortege[1]);

    ///////////////////////////////////////////////////////////////////////////////
    // FareUsage
    FareUsage fareUsage[2];

    // MAN-ORD-DFW / JFK
    fareUsage[0].travelSeg().push_back(&airSeg[0]);
    fareUsage[0].travelSeg().push_back(&airSeg[1]);
    fareUsage[0].travelSeg().push_back(&arnkSeg);
    fareUsage[0].paxTypeFare() = &paxTypeFare[0];

    // JFK-LHR-MAN
    fareUsage[1].travelSeg().push_back(&airSeg[2]);
    fareUsage[1].travelSeg().push_back(&airSeg[3]);
    fareUsage[1].paxTypeFare() = &paxTypeFare[1];

    ///////////////////////////////////////////////////////////////////////////////
    // Pricing Unit
    _pu->fareUsage().push_back(&fareUsage[0]);
    _pu->fareUsage().push_back(&fareUsage[1]);
    _pu->puFareType() = PricingUnit::NL;

    ///////////////////////////////////////////////////////////////////////////////
    // FarePath
    _farePath->pricingUnit().push_back(_pu);
    _farePath->paxType() = &paxType;

    ///////////////////////////////////////////////////////////////////////////////
    // Trx.FareMarket
    _trx->fareMarket().push_back(&fareMarket[0]);
    _trx->fareMarket().push_back(&fareMarket[1]);

    _itin->setTravelDate(DateTime::emptyDate());

    CPPUNIT_ASSERT_MESSAGE("No PU in FarePath", _farePath->pricingUnit().size() > 0);
    CPPUNIT_ASSERT_MESSAGE("No AirSeg in Itin", _farePath->itin()->travelSeg().size() > 0);
    CPPUNIT_ASSERT_MESSAGE("No FareUsage in PU", _pu->fareUsage().size() > 0);

    setUpOjm();
    CPPUNIT_ASSERT(_ojm->process(*_pu) >= 0.0);

    setUpRt();
    CPPUNIT_ASSERT(_rtm->process(*_pu) >= 0.0);
  }

  void testProcess()
  {
    DateTime now(time(NULL));

    const Loc* loc[5];
    loc[0] = _trx->dataHandle().getLoc("MAN", now);
    loc[1] = _trx->dataHandle().getLoc("ORD", now);
    loc[2] = _trx->dataHandle().getLoc("DFW", now);
    loc[3] = _trx->dataHandle().getLoc("JFK", now);
    loc[4] = _trx->dataHandle().getLoc("LHR", now);

    Agent agent;
    agent.agentLocation() = loc[0];

    CarrierCode crx[2] = { "BA", "AA" };

    AirSeg airSeg[4];
    ArunkSeg arnkSeg;
    setAirSeg(airSeg[0], 1, GeoTravelType::International, loc[0], loc[1], crx[0]); // MANORD
    setAirSeg(airSeg[1], 2, GeoTravelType::Domestic, loc[1], loc[2], crx[1]); // ORDDFW
    setArunkSeg(arnkSeg, 3, GeoTravelType::Domestic, loc[2], loc[3]); // DFWJFK
    setAirSeg(airSeg[2], 4, GeoTravelType::International, loc[3], loc[4], crx[1]); // JFKLHR
    setAirSeg(airSeg[3], 5, GeoTravelType::Domestic, loc[4], loc[0], crx[1]); // LHRMAN

    PricingRequest request;
    request.ticketingAgent() = &agent;
    _trx->setRequest(&request);

    _itin->travelSeg().push_back(&airSeg[0]);
    _itin->travelSeg().push_back(&airSeg[1]);
    _itin->travelSeg().push_back(&arnkSeg);
    _itin->travelSeg().push_back(&airSeg[2]);
    _itin->travelSeg().push_back(&airSeg[3]);
    _itin->intlSalesIndicator() = Itin::SITI;

    _trx->travelSeg().push_back(&airSeg[0]);
    _trx->travelSeg().push_back(&airSeg[1]);
    _trx->travelSeg().push_back(&arnkSeg);
    _trx->travelSeg().push_back(&airSeg[2]);
    _trx->travelSeg().push_back(&airSeg[3]);
    _trx->itin().push_back(_itin);

    PaxType paxType;
    paxType.paxType() = "ADT";

    ///////////////////////////////////////////////////////////////////////////////
    // TariffCrossRefInfo
    TariffCrossRefInfo tariff[2];

    tariff[0]._fareTariffCode = "TAFP";
    tariff[0]._tariffCat = 0;
    tariff[0]._ruleTariff = 304;
    tariff[0]._routingTariff1 = 99;
    tariff[0]._routingTariff2 = 96;

    tariff[1]._fareTariffCode = "TAFP";
    tariff[1]._tariffCat = 0;
    tariff[1]._ruleTariff = 304;
    tariff[1]._routingTariff1 = 99;
    tariff[1]._routingTariff2 = 96;

    ///////////////////////////////////////////////////////////////////////////////
    // FareInfo
    FareInfo fareInfo[2];

    fareInfo[0]._vendor = Vendor::ATPCO;
    fareInfo[0]._carrier = "AA";
    fareInfo[0]._market1 = "DFW";
    fareInfo[0]._market2 = "MAN";
    fareInfo[0]._fareClass = "Y";
    fareInfo[0]._fareTariff = 1;
    fareInfo[0]._fareAmount = 1260;
    fareInfo[0]._originalFareAmount = 1260;
    fareInfo[0]._currency = "BGP";
    fareInfo[0]._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo[0]._ruleNumber = "2000";
    fareInfo[0]._routingNumber = "XXXX";
    fareInfo[0]._directionality = TO;
    fareInfo[0]._globalDirection = GlobalDirection::PA;

    fareInfo[1]._vendor = Vendor::ATPCO;
    fareInfo[1]._carrier = "AA";
    fareInfo[1]._market1 = "DFW";
    fareInfo[1]._market2 = "MAN";
    fareInfo[1]._fareClass = "Y";
    fareInfo[1]._fareTariff = 1;
    fareInfo[1]._fareAmount = 1260;
    fareInfo[1]._originalFareAmount = 1260;
    fareInfo[1]._currency = "BGP";
    fareInfo[1]._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo[1]._ruleNumber = "2000";
    fareInfo[1]._routingNumber = "XXXX";
    fareInfo[1]._directionality = TO;
    fareInfo[1]._globalDirection = GlobalDirection::PA;

    ///////////////////////////////////////////////////////////////////////////////
    // Fare
    Fare fare[2];
    fare[0].nucFareAmount() = 1;
    fare[1].nucFareAmount() = 1;

    ///////////////////////////////////////////////////////////////////////////////
    // FareClassAppInfo
    FareClassAppInfo appInfo[2];
    appInfo[0]._fareType = "BU";
    appInfo[0]._pricingCatType = 'N';
    appInfo[1]._fareType = "BU";
    appInfo[1]._pricingCatType = 'N';

    ///////////////////////////////////////////////////////////////////////////////
    // FareClassAppSegInfo
    FareClassAppSegInfo appSegInfo[2];
    appSegInfo[0]._paxType = paxType.paxType();
    appSegInfo[1]._paxType = paxType.paxType();

    ///////////////////////////////////////////////////////////////////////////////
    // FareMarket
    FareMarket fareMarket[2];

    fare[0].initialize(Fare::FS_International, &fareInfo[0], fareMarket[0], &tariff[0]);

    fare[1].initialize(Fare::FS_International, &fareInfo[1], fareMarket[1], &tariff[1]);

    fareMarket[0].travelSeg().push_back(&airSeg[0]);
    fareMarket[0].travelSeg().push_back(&airSeg[1]);
    fareMarket[0].origin() = loc[0];
    fareMarket[0].destination() = loc[2];
    fareMarket[0].setGlobalDirection(fareInfo[0]._globalDirection);
    fareMarket[0].governingCarrier() = fareInfo[0]._carrier;

    fareMarket[1].travelSeg().push_back(&airSeg[2]);
    fareMarket[1].travelSeg().push_back(&airSeg[3]);
    fareMarket[1].origin() = loc[3];
    fareMarket[1].destination() = loc[0];
    fareMarket[1].setGlobalDirection(fareInfo[1]._globalDirection);
    fareMarket[1].governingCarrier() = fareInfo[1]._carrier;

    ///////////////////////////////////////////////////////////////////////////////
    // PaxTypeFare
    PaxTypeFare paxTypeFare[2];
    paxTypeFare[0].initialize(&fare[0], &paxType, &fareMarket[0]);
    paxTypeFare[0].fareClassAppInfo() = &appInfo[0];
    paxTypeFare[0].fareClassAppSegInfo() = &appSegInfo[0];

    paxTypeFare[1].initialize(&fare[1], &paxType, &fareMarket[1]);
    paxTypeFare[1].fareClassAppInfo() = &appInfo[1];
    paxTypeFare[1].fareClassAppSegInfo() = &appSegInfo[1];

    // setup for mixed cabin
    paxTypeFare[0].cabin().setBusinessClass();
    paxTypeFare[1].cabin().setFirstClass();

    ///////////////////////////////////////////////////////////////////////////////
    // PaxTypeBucket
    PaxTypeBucket paxTypeCortege[2];
    paxTypeCortege[0].requestedPaxType() = &paxType;
    paxTypeCortege[1].requestedPaxType() = &paxType;

    paxTypeCortege[0].paxTypeFare().push_back(&paxTypeFare[0]);
    paxTypeCortege[1].paxTypeFare().push_back(&paxTypeFare[1]);

    fareMarket[0].paxTypeCortege().push_back(paxTypeCortege[0]);
    fareMarket[1].paxTypeCortege().push_back(paxTypeCortege[1]);

    ///////////////////////////////////////////////////////////////////////////////
    // FareUsage
    FareUsage fareUsage[2];

    // MAN-ORD-DFW / JFK
    fareUsage[0].travelSeg().push_back(&airSeg[0]);
    fareUsage[0].travelSeg().push_back(&airSeg[1]);
    fareUsage[0].travelSeg().push_back(&arnkSeg);
    fareUsage[0].paxTypeFare() = &paxTypeFare[0];

    // JFK-LHR-MAN
    fareUsage[1].travelSeg().push_back(&airSeg[2]);
    fareUsage[1].travelSeg().push_back(&airSeg[3]);
    fareUsage[1].paxTypeFare() = &paxTypeFare[1];

    ///////////////////////////////////////////////////////////////////////////////
    // Pricing Unit
    _pu->fareUsage().push_back(&fareUsage[0]);
    _pu->fareUsage().push_back(&fareUsage[1]);
    _pu->puFareType() = PricingUnit::NL;

    ///////////////////////////////////////////////////////////////////////////////
    // FarePath
    _farePath->pricingUnit().push_back(_pu);
    _farePath->paxType() = &paxType;

    ///////////////////////////////////////////////////////////////////////////////
    // Trx.FareMarket
    _trx->fareMarket().push_back(&fareMarket[0]);
    _trx->fareMarket().push_back(&fareMarket[1]);

    _itin->setTravelDate(DateTime::emptyDate());

    CPPUNIT_ASSERT_MESSAGE("No PU in FarePath", _farePath->pricingUnit().size() > 0);
    CPPUNIT_ASSERT_MESSAGE("No AirSeg in Itin", _farePath->itin()->travelSeg().size() > 0);
    CPPUNIT_ASSERT_MESSAGE("No FareUsage in PU", _pu->fareUsage().size() > 0);

    // Leave it out until I setup all the paxtypefares
    setUpOjm();
    CPPUNIT_ASSERT(!_ojm->process(*_pu));

    setUpRt();
    CPPUNIT_ASSERT(!_rtm->process(*_pu));
  }

  void testCalculateFareAmtReturnFalseWhenNoFare()
  {
    MoneyAmount fareAmt;
    addFusToPu();
    _fu1->paxTypeFare() = NULL;
    CPPUNIT_ASSERT(!HighRTChecker::calculateFareAmt(*_pu, fareAmt));
  }

  void testCalculateFareAmtReturnTrueWhenFaresPresent()
  {
    MoneyAmount fareAmt;
    addFusToPu();
    CPPUNIT_ASSERT(HighRTChecker::calculateFareAmt(*_pu, fareAmt));
  }

  void testCalculateFareAmtCheckCalcResult()
  {
    MoneyAmount fareAmt = 0;
    addFusToPu();
    addFaresWithAmtsToPtf(1000, 55);
    HighRTChecker::calculateFareAmt(*_pu, fareAmt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1055, fareAmt, EPSILON);
  }

  void testCalculateFareAmtCheckCalcResultForZeroAmts()
  {
    MoneyAmount fareAmt = 0;
    addFusToPu();
    addFaresWithAmtsToPtf(0, 0);
    HighRTChecker::calculateFareAmt(*_pu, fareAmt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, fareAmt, EPSILON);
  }

  void testCalculateFareAmtCheckIfDifferentialNotAdded()
  {
    MoneyAmount fareAmt = 0;
    addFusToPu();
    addFaresWithAmtsToPtf(0, 0);
    _fu1->differentialAmt() = 100;
    HighRTChecker::calculateFareAmt(*_pu, fareAmt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, fareAmt, EPSILON);
  }

  void testCalculateFareAmtCheckIfMileageSurchargeAdded()
  {
    MoneyAmount fareAmt = 0;
    addFusToPu();
    addFaresWithAmtsToPtf(0, 0);
    _fare1->mileageSurchargeAmt() = 777;
    HighRTChecker::calculateFareAmt(*_pu, fareAmt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(777, fareAmt, EPSILON);
  }

  void testProcessNetFaresDontAddPlusUpWhenNetRemitFp()
  {
    setUpOjm();
    _ojm->_isNetRemitFp = true;
    _ojm->processNetFares(*_pu);
    CPPUNIT_ASSERT(NULL == _pu->hrtojNetPlusUp());

    setUpRt();
    _rtm->_isNetRemitFp = true;
    _rtm->processNetFares(*_pu);
    CPPUNIT_ASSERT(NULL == _pu->hrtojNetPlusUp());
  }

  void testProcessNetFaresDontAddPlusUpWhenIsNotWpNet()
  {
    addFusToPu();
    addFaresWithAmtsToPtf(10, 10);

    setUpOjm();
    _ojm->processNetFares(*_pu);
    CPPUNIT_ASSERT(NULL == _pu->hrtojNetPlusUp());

    setUpRt();
    _rtm->processNetFares(*_pu);
    CPPUNIT_ASSERT(NULL == _pu->hrtojNetPlusUp());
  }

  void testProcessNetFaresAddPlusUp()
  {
    addFusToPu();
    addFaresWithAmtsToPtf(10, 10, true, 20, 30);
    createFareMarketes();

    setUpOjm();
    _ojm->processNetFares(*_pu);
    CPPUNIT_ASSERT(NULL != _pu->hrtojNetPlusUp());

    setUpRt();
    _rtm->processNetFares(*_pu);
    CPPUNIT_ASSERT(NULL != _pu->hrtojNetPlusUp());
  }

  void testProcessNetFaresCheckPlusUpAmt()
  {
    addFusToPu();
    addFaresWithAmtsToPtf(10, 10, true, 200, 300); // 2*300=600; 200+300=500; 600-500=100
    createFareMarketes();

    setUpOjm();
    _ojm->processNetFares(*_pu);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, _pu->hrtojNetPlusUp()->plusUpAmount, EPSILON);

    setUpRt();
    _rtm->processNetFares(*_pu);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, _pu->hrtojNetPlusUp()->plusUpAmount, EPSILON);
  }

  void testProcessNetFaresCheckPlusUpNullWhenDifferenceIsZero()
  {
    addFusToPu();
    addFaresWithAmtsToPtf(10, 10, true, 200, 200);
    createFareMarketes();

    setUpOjm();
    _ojm->processNetFares(*_pu);
    CPPUNIT_ASSERT(NULL == _pu->hrtojNetPlusUp());

    setUpRt();
    _rtm->processNetFares(*_pu);
    CPPUNIT_ASSERT(NULL == _pu->hrtojNetPlusUp());
  }

  void testIsSameCabinReturnTrueWhenBothFirst()
  {
    addFusToPu();
    _ptf1->cabin().setFirstClass();
    _ptf2->cabin().setFirstClass();
    CPPUNIT_ASSERT(HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnTrueWhenFirstAndPremiumFirst()
  {
    addFusToPu();
    _ptf1->cabin().setFirstClass();
    _ptf2->cabin().setPremiumFirstClass();
    CPPUNIT_ASSERT(HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnFalseWhenFirstAndEconomy()
  {
    addFusToPu();
    _ptf1->cabin().setFirstClass();
    _ptf2->cabin().setEconomyClass();
    CPPUNIT_ASSERT(!HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnFalseWhenFirstAndPremiumBusiness()
  {
    addFusToPu();
    _ptf1->cabin().setFirstClass();
    _ptf2->cabin().setBusinessClass();
    CPPUNIT_ASSERT(!HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnTrueWhenBothBusiness()
  {
    addFusToPu();
    _ptf1->cabin().setBusinessClass();
    _ptf2->cabin().setBusinessClass();
    CPPUNIT_ASSERT(HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnTrueWhenBothPremiumBusiness()
  {
    addFusToPu();
    _ptf1->cabin().setPremiumBusinessClass();
    _ptf2->cabin().setPremiumBusinessClass();
    CPPUNIT_ASSERT(HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnTrueWhenBusinessAndPremiumBusiness()
  {
    addFusToPu();
    _ptf1->cabin().setBusinessClass();
    _ptf2->cabin().setPremiumBusinessClass();
    CPPUNIT_ASSERT(HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnFalseWhenBusinessAndEconomy()
  {
    addFusToPu();
    _ptf1->cabin().setBusinessClass();
    _ptf2->cabin().setEconomyClass();
    CPPUNIT_ASSERT(!HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnFalseWhenBusinessAndPremiumFirst()
  {
    addFusToPu();
    _ptf1->cabin().setBusinessClass();
    _ptf2->cabin().setPremiumFirstClass();
    CPPUNIT_ASSERT(!HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnFalseWhenBusinessAndPremiumEconomy()
  {
    addFusToPu();
    _ptf1->cabin().setBusinessClass();
    _ptf2->cabin().setPremiumEconomyClass();
    CPPUNIT_ASSERT(!HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnTrueWhenBothEconomy()
  {
    addFusToPu();
    _ptf1->cabin().setEconomyClass();
    _ptf2->cabin().setEconomyClass();
    CPPUNIT_ASSERT(HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnTrueWhenBothPremiumEconomy()
  {
    addFusToPu();
    _ptf1->cabin().setPremiumEconomyClass();
    _ptf2->cabin().setPremiumEconomyClass();
    CPPUNIT_ASSERT(HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnTrueWhenEconomyAndPremiumEconomy()
  {
    addFusToPu();
    _ptf1->cabin().setEconomyClass();
    _ptf2->cabin().setPremiumEconomyClass();
    CPPUNIT_ASSERT(HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnFalseWhenEconomyAndFirst()
  {
    addFusToPu();
    _ptf1->cabin().setEconomyClass();
    _ptf2->cabin().setFirstClass();
    CPPUNIT_ASSERT(!HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnFalseWhenEconomyAndPremiumFirst()
  {
    addFusToPu();
    _ptf1->cabin().setEconomyClass();
    _ptf2->cabin().setPremiumFirstClass();
    CPPUNIT_ASSERT(!HighRTChecker::isSameCabin(*_pu));
  }

  void testIsSameCabinReturnFalseWhenPremiumEconomyAndBusiness()
  {
    addFusToPu();
    _ptf1->cabin().setPremiumEconomyClass();
    _ptf2->cabin().setBusinessClass();
    CPPUNIT_ASSERT(!HighRTChecker::isSameCabin(*_pu));
  }

protected:
  void setAirSeg(AirSeg& seg,
                 int segOrder,
                 GeoTravelType geoTravelType,
                 const Loc* origin,
                 const Loc* destination,
                 const std::string& carrier)
  {
    seg.segmentOrder() = segOrder;
    seg.pnrSegment() = segOrder;
    seg.geoTravelType() = geoTravelType;
    seg.origin() = origin;
    seg.destination() = destination;
    seg.carrier() = carrier;
  }

  void setArunkSeg(ArunkSeg& seg,
                   int segOrder,
                   GeoTravelType geoTravelType,
                   const Loc* origin,
                   const Loc* destination)
  {
    seg.segmentOrder() = segOrder;
    seg.pnrSegment() = segOrder;
    seg.geoTravelType() = geoTravelType;
    seg.origin() = origin;
    seg.destination() = destination;
  }

  void addFusToPu()
  {
    _fu1 = _memHandle.create<FareUsage>();
    _fu2 = _memHandle.create<FareUsage>();
    _ptf1 = _memHandle.create<PaxTypeFare>();
    _ptf2 = _memHandle.create<PaxTypeFare>();
    _fu1->paxTypeFare() = _ptf1;
    _fu2->paxTypeFare() = _ptf2;
    _pu->fareUsage() += _fu1, _fu2;
  }

  void addFaresWithAmtsToPtf(const MoneyAmount& amt1,
                             const MoneyAmount& amt2,
                             bool addNet = false,
                             const MoneyAmount& netAmt1 = 0.0,
                             const MoneyAmount& netAmt2 = 0.0)
  {
    createFareWithAmt(amt1, _ptf1, _fare1);
    createFareWithAmt(amt2, _ptf2, _fare2);
    if (addNet)
    {
      addNetFaresToPaxTypes(_ptf1, netAmt1);
      addNetFaresToPaxTypes(_ptf2, netAmt2);
    }
  }
  void createFareWithAmt(const MoneyAmount& amt, PaxTypeFare* ptf, Fare*& fare)
  {
    fare = _memHandle.create<Fare>();
    fare->nucFareAmount() = amt;
    ptf->setFare(fare);
    ptf->status().set(PaxTypeFare::PTF_Negotiated);
  }

  void addNetFaresToPaxTypes(PaxTypeFare* ptf, const MoneyAmount& amt)
  {
    NegPaxTypeFareRuleData* netFare = _memHandle.create<NegPaxTypeFareRuleData>();
    netFare->nucNetAmount() = amt;
    PaxTypeFare::PaxTypeFareAllRuleData* allRuleData =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    allRuleData->fareRuleData = netFare;
    (*ptf->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allRuleData;
  }
  void createFareMarketes()
  {
    _fareMarket1 = _memHandle.create<FareMarket>();
    _fareMarket2 = _memHandle.create<FareMarket>();
    _ptf1->fareMarket() = _fareMarket1;
    _ptf2->fareMarket() = _fareMarket2;
  }

private:
  PricingTrx* _trx;
  DiagCollector* _diag;
  TestMemHandle _memHandle;
  PricingUnit* _pu;
  FarePath* _farePath;
  Itin* _itin;
  OJMMinimumFare* _ojm;
  RTMinimumFare* _rtm;
  FareUsage* _fu1;
  FareUsage* _fu2;
  PaxTypeFare* _ptf1;
  PaxTypeFare* _ptf2;
  Fare* _fare1;
  Fare* _fare2;
  FareMarket* _fareMarket1;
  FareMarket* _fareMarket2;
  PricingRequest* _request;
};
CPPUNIT_TEST_SUITE_REGISTRATION(OJMAndRTMinimumFareTest);
}
