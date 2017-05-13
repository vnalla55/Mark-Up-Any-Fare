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
#include "MinFares/HIPMinimumFare.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/DiskCache.h"
#include "Diagnostic/DCFactory.h"
#include "DBAccess/DiscountInfo.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "MinFares/MinFareChecker.h"
#include "MinFares/MinimumFare.h"
#include "test/include/MockGlobal.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingRequest.h"
#include "Rules/RuleConst.h"
#include "DataModel/PricingTrx.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"
#include "Common/TseConsts.h"

//--- for Fare By Rule creation ---
#include "DBAccess/FareByRuleItemInfo.h"

#include "DBAccess/FareByRuleApp.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

using namespace std;

namespace tse
{

class HIPMinimumFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(HIPMinimumFareTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testPromoFareProcess);
  CPPUNIT_TEST(testCat25FareProcess);
  CPPUNIT_TEST(testHipExemptedWhenMinFareApplMissing);
  CPPUNIT_TEST(testHipExemptedWhenHipCheckNotApply);
  CPPUNIT_TEST(testHipNotExemptedWhenHipCheckApply);

  CPPUNIT_TEST(testSkipProcessForSpecialOnly_Normal_NoApplyDefault_NoSpec);
  CPPUNIT_TEST(testSkipProcessForSpecialOnly_Normal_NoApplyDefault_Spec);
  CPPUNIT_TEST(testSkipProcessForSpecialOnly_Normal_ApplyDefault_NoSpec);
  CPPUNIT_TEST(testSkipProcessForSpecialOnly_Normal_ApplyDefault_Spec);
  CPPUNIT_TEST(testSkipProcessForSpecialOnly_NotNormal_ApplyDefault_Spec);
  CPPUNIT_TEST_SUITE_END();

public:
  PricingTrx* _trx;
  HIPMinimumFare* _hip;
  MinFareAppl* _minFareAppl;
  PaxTypeFare* _paxTypeFare;
  TestMemHandle _memHandle;


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

  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->diagnostic().diagnosticType() = Diagnostic718;
    _trx->diagnostic().activate();

    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _minFareAppl = _memHandle.create<MinFareAppl>();
    _hip = _memHandle.insert(new HIPMinimumFare(*_trx));
    _hip->_matchedApplItem = _minFareAppl;
  }

  void tearDown() { _memHandle.clear(); }

  void testHipExemptedWhenMinFareApplMissing()
  {
    _hip->_matchedApplItem = 0;
    CPPUNIT_ASSERT(_hip->isExemptedByApplication(*_paxTypeFare));

    if (_hip->_diag)
      _hip->_diag->flushMsg();

    string diagStr = "FAILED MATCHING IN APPLICATION TABLE - RETURN\n";
    CPPUNIT_ASSERT_EQUAL(diagStr, _trx->diagnostic().toString());
  }

  void testHipExemptedWhenHipCheckNotApply()
  {
    _minFareAppl->hipCheckAppl() = NO;
    _minFareAppl->seqNo() = 1000;
    CPPUNIT_ASSERT(_hip->isExemptedByApplication(*_paxTypeFare));

    if (_hip->_diag)
      _hip->_diag->flushMsg();

    string diagStr = "EXEMPT BY APPLICATION TABLE - 1000\n";
    CPPUNIT_ASSERT_EQUAL(diagStr, _trx->diagnostic().toString());
  }

  void testHipNotExemptedWhenHipCheckApply()
  {
    _minFareAppl->hipCheckAppl() = YES;
    CPPUNIT_ASSERT(!_hip->isExemptedByApplication(*_paxTypeFare));
  }

  void testProcess()
  {

    PricingTrx trx;

    PricingRequest request;

    request.globalDirection() = GlobalDirection::WH; // Western Hemisphere

    trx.setRequest(&request); // Set the request pointer (so pass address).
    trx.setTravelDate(DateTime::localTime());

    // Set the agent location
    Agent agent;
    Loc loca;
    loca.loc() = "LON";
    loca.nation() = "GP";

    agent.agentLocation() = &loca; // Sold in GP (Guadeloupe)

    request.ticketingAgent() = &agent;
    // End building the agent location

    trx.diagnostic().diagnosticType() = Diagnostic718;
    trx.diagnostic().activate();

    Loc loc1;
    LocCode locCode1 = "BKK";
    loc1.loc() = locCode1;
    loc1.cityInd() = true;
    loc1.nation() = "TH";
    loc1.area() = IATA_AREA3;
    loc1.subarea() = NATION_THAILAND;

    Loc loc2;
    LocCode locCode2 = "TYO";
    loc2.loc() = locCode2;
    loc2.cityInd() = true;
    loc2.nation() = "JP";
    loc2.area() = IATA_AREA3;
    loc2.subarea() = "JP";

    Loc loc3;
    LocCode locCode3 = "NYC";
    loc3.loc() = locCode3;
    loc3.cityInd() = true;
    loc3.nation() = "US";
    loc3.area() = IATA_AREA1;
    loc3.subarea() = NORTH_AMERICA;

    CarrierCode cc1 = "BA";
    CarrierCode cc2 = "BA";

    AirSeg airSeg1;
    airSeg1.pnrSegment() = 0;
    airSeg1.geoTravelType() = GeoTravelType::International;
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    airSeg1.boardMultiCity() = loc1.loc();
    airSeg1.offMultiCity() = loc2.loc();

    airSeg1.carrier() = cc1;
    airSeg1.stopOver() = true;
    airSeg1.departureDT() = DateTime::localTime();

    AirSeg airSeg2;
    airSeg2.pnrSegment() = 1;
    airSeg2.geoTravelType() = GeoTravelType::International;
    airSeg2.origin() = &loc2;
    airSeg2.destination() = &loc3;
    airSeg2.boardMultiCity() = loc2.loc();
    airSeg2.offMultiCity() = loc3.loc();
    airSeg2.carrier() = cc2;

    DateTime newDate1(airSeg1.departureDT().year(),
                      airSeg1.departureDT().month(),
                      airSeg1.departureDT().day(),
                      airSeg1.departureDT().hours() + 24,
                      airSeg1.departureDT().minutes(),
                      airSeg1.departureDT().seconds());
    airSeg2.departureDT() = newDate1;

    Itin itin;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.intlSalesIndicator() = Itin::SITI;

    trx.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg2);
    trx.itin().push_back(&itin);

    FareMarket fareMarket1;

    fareMarket1.travelSeg().push_back(&airSeg1);
    fareMarket1.travelSeg().push_back(&airSeg2);
    fareMarket1.origin() = &loc1;
    fareMarket1.destination() = &loc3;
    fareMarket1.boardMultiCity() = loc1.loc();
    fareMarket1.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection1 = GlobalDirection::AT;
    fareMarket1.setGlobalDirection(globleDirection1);
    fareMarket1.governingCarrier() = "BA";

    Fare fare1;
    fare1.nucFareAmount() = 1845.26;

    FareInfo fareInfo1;
    fareInfo1._carrier = "BA";
    fareInfo1._market1 = "BKK";
    fareInfo1._market2 = "NYC";
    fareInfo1._fareClass = "YO1";
    fareInfo1._fareAmount = 600.00;
    fareInfo1._currency = "GBP";
    fareInfo1._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo1._ruleNumber = "2000";
    fareInfo1._routingNumber = "XXXX";
    fareInfo1._directionality = FROM;
    fareInfo1._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo1;
    tariffRefInfo1._fareTariffCode = "TAFPBA";
    tariffRefInfo1._tariffCat = 0;
    tariffRefInfo1._ruleTariff = 304;
    tariffRefInfo1._routingTariff1 = 99;
    tariffRefInfo1._routingTariff2 = 96;

    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket1, &tariffRefInfo1);

    PaxType paxType;
    PaxTypeCode paxTypeCode = "ADT";
    paxType.paxType() = paxTypeCode;
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

    PaxTypeBucket paxTypeCortege1;
    paxTypeCortege1.requestedPaxType() = &paxType;
    paxTypeCortege1.paxTypeFare().push_back(&paxTypeFare1);
    fareMarket1.paxTypeCortege().push_back(paxTypeCortege1);

    //=== end setup the first set of Fare, FareMarket and PaxTypeFare ===

    // set another thru fare

    Fare fare11;
    fare11.nucFareAmount() = 2000.20;

    FareInfo fareInfo11;
    fareInfo11._carrier = "BA";
    fareInfo11._market1 = "BKK";
    fareInfo11._market2 = "NYC";
    fareInfo11._fareClass = "Y11";
    fareInfo11._fareAmount = 600.00;
    fareInfo11._currency = "GBP";
    fareInfo11._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo11._ruleNumber = "2000";
    fareInfo11._routingNumber = "XXXX";
    fareInfo11._directionality = FROM;
    fareInfo11._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo11;
    tariffRefInfo11._fareTariffCode = "TAFPBA";
    tariffRefInfo11._tariffCat = 0;
    tariffRefInfo11._ruleTariff = 304;
    tariffRefInfo11._routingTariff1 = 99;
    tariffRefInfo11._routingTariff2 = 96;

    fare11.initialize(Fare::FS_International, &fareInfo11, fareMarket1, &tariffRefInfo11);

    PaxType paxType11;
    PaxTypeCode paxTypeCode11 = "ADT";
    paxType11.paxType() = paxTypeCode11;
    PaxTypeFare paxTypeFare11;
    paxTypeFare11.initialize(&fare11, &paxType11, &fareMarket1);

    FareClassAppInfo appInfo11;
    appInfo11._fareType = "XEX";
    appInfo11._pricingCatType = 'S';
    paxTypeFare11.fareClassAppInfo() = &appInfo11;

    FareClassAppSegInfo fareClassAppSegInfo11;
    fareClassAppSegInfo11._paxType = "ADT";
    paxTypeFare11.fareClassAppSegInfo() = &fareClassAppSegInfo11;
    paxTypeFare11.cabin().setEconomyClass();

    PaxTypeBucket paxTypeCortege11;
    paxTypeCortege11.requestedPaxType() = &paxType;
    paxTypeCortege11.paxTypeFare().push_back(&paxTypeFare11);
    fareMarket1.paxTypeCortege().push_back(paxTypeCortege11);

    //=== end setup the 1.1 set of Fare, FareMarket and PaxTypeFare ===

    //=== setup the second set of Fare, FareMarket and PaxTypeFare ===
    FareMarket fareMarket2;
    fareMarket2.travelSeg().push_back(&airSeg1);
    fareMarket2.origin() = &loc1;
    fareMarket2.destination() = &loc2;
    fareMarket2.boardMultiCity() = loc1.loc();
    fareMarket2.offMultiCity() = loc2.loc();

    GlobalDirection globleDirection2 = GlobalDirection::EH;
    fareMarket2.setGlobalDirection(globleDirection2);
    fareMarket2.governingCarrier() = "BA";

    Fare fare2;
    fare2.nucFareAmount() = 2200.02;

    FareInfo fareInfo2;
    fareInfo2._carrier = "BA";
    fareInfo2._market1 = "BKK";
    fareInfo2._market2 = "TYO";
    fareInfo2._fareClass = "YO2";
    fareInfo2._fareAmount = 600.00;
    fareInfo2._currency = "GBP";
    fareInfo2._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo2._ruleNumber = "5135";
    fareInfo2._routingNumber = "XXXX";
    fareInfo2._directionality = FROM;
    fareInfo2._globalDirection = GlobalDirection::WH;

    TariffCrossRefInfo tariffRefInfo2;
    tariffRefInfo2._fareTariffCode = "TAFPBA";
    tariffRefInfo2._tariffCat = 0;
    tariffRefInfo2._ruleTariff = 304;

    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2, &tariffRefInfo2);

    paxType.paxType() = paxTypeCode;
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

    PaxTypeBucket paxTypeCortege2;
    paxTypeCortege2.requestedPaxType() = &paxType;
    paxTypeCortege2.paxTypeFare().push_back(&paxTypeFare2);
    fareMarket2.paxTypeCortege().push_back(paxTypeCortege2);

    //=== end setup the second set of Fare, FareMarket and PaxTypeFare ===

    //=== setup the third set of Fare, FareMarket and PaxTypeFare ===

    FareMarket fareMarket3;
    fareMarket3.travelSeg().push_back(&airSeg2);
    fareMarket3.origin() = &loc2;
    fareMarket3.destination() = &loc3;
    fareMarket3.boardMultiCity() = loc2.loc();
    fareMarket3.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection3 = GlobalDirection::WH;
    fareMarket3.setGlobalDirection(globleDirection3);
    fareMarket3.governingCarrier() = "BA";

    Fare fare3;
    fare3.nucFareAmount() = 1333.33;

    FareInfo fareInfo3;
    fareInfo3._carrier = "BA";
    fareInfo3._market1 = "TYO";
    fareInfo3._market2 = "NYC";
    fareInfo3._fareClass = "YO3";
    fareInfo3._fareAmount = 600.00;
    fareInfo3._currency = "GBP";
    fareInfo3._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo3._ruleNumber = "5135";
    fareInfo3._routingNumber = "XXXX";
    fareInfo3._directionality = FROM;
    fareInfo3._globalDirection = GlobalDirection::WH;

    TariffCrossRefInfo tariffRefInfo3;
    tariffRefInfo3._fareTariffCode = "TAFPBA";
    tariffRefInfo3._tariffCat = 0;
    tariffRefInfo3._ruleTariff = 304;

    fare3.initialize(Fare::FS_International, &fareInfo3, fareMarket3, &tariffRefInfo3);

    paxType.paxType() = paxTypeCode;
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

    PaxTypeBucket paxTypeCortege3;
    paxTypeCortege3.requestedPaxType() = &paxType;
    paxTypeCortege3.paxTypeFare().push_back(&paxTypeFare3);
    fareMarket3.paxTypeCortege().push_back(paxTypeCortege3);

    //=== end setup the third set of Fare, FareMarket and PaxTypeFare ===

    FareUsage fareUsage;
    fareUsage.travelSeg().push_back(&airSeg1);
    fareUsage.travelSeg().push_back(&airSeg2);
    fareUsage.paxTypeFare() = &paxTypeFare11;

    PricingUnit::Type ow = PricingUnit::Type::ONEWAY;
    //    PricingUnit::PUFareType nl = PricingUnit::NL;
    PricingUnit::PUFareType spcl = PricingUnit::SP;

    PricingUnit pu;
    pu.fareUsage().push_back(&fareUsage);
    pu.puType() = ow;
    pu.puFareType() = spcl;

    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    farePath.paxType() = &paxType;

    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket3);

    //---------------------------------------------------
    multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;
    multimap<uint16_t, const MinFareAppl*> applMap;
    map<uint16_t, const MinFareDefaultLogic*> defaultLogicMap;

    HIPMinimumFare hip(trx);

    CPPUNIT_ASSERT(hip.process(farePath, ruleLevelMap, applMap, defaultLogicMap) >= 0.0);
  }

  void testPromoFareProcess()
  {

    PricingTrx trx;

    PricingRequest request;

    request.globalDirection() = GlobalDirection::WH; // Western Hemisphere

    trx.setRequest(&request); // Set the request pointer (so pass address).
    trx.setTravelDate(DateTime::localTime());

    // Set the agent location
    Agent agent;
    Loc loca;
    loca.loc() = "LON";
    loca.nation() = "GP";

    agent.agentLocation() = &loca; // Sold in GP (Guadeloupe)

    request.ticketingAgent() = &agent;
    // End building the agent location

    trx.diagnostic().diagnosticType() = Diagnostic718;
    trx.diagnostic().activate();

    Loc loc1;
    LocCode locCode1 = "BKK";
    loc1.loc() = locCode1;
    loc1.cityInd() = true;
    loc1.nation() = "TH";
    loc1.area() = IATA_AREA3;
    loc1.subarea() = NATION_THAILAND;

    Loc loc2;
    LocCode locCode2 = "TYO";
    loc2.loc() = locCode2;
    loc2.cityInd() = true;
    loc2.nation() = "JP";
    loc2.area() = IATA_AREA3;
    loc2.subarea() = "JP";

    Loc loc3;
    LocCode locCode3 = "NYC";
    loc3.loc() = locCode3;
    loc3.cityInd() = true;
    loc3.nation() = "US";
    loc3.area() = IATA_AREA1;
    loc3.subarea() = NORTH_AMERICA;

    CarrierCode cc1 = "BA";
    CarrierCode cc2 = "BA";

    AirSeg airSeg1;
    airSeg1.pnrSegment() = 0;
    airSeg1.geoTravelType() = GeoTravelType::International;
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    airSeg1.boardMultiCity() = loc1.loc();
    airSeg1.offMultiCity() = loc2.loc();
    airSeg1.carrier() = cc1;
    airSeg1.stopOver() = true;
    airSeg1.departureDT() = DateTime::localTime();

    AirSeg airSeg2;
    airSeg2.pnrSegment() = 1;
    airSeg2.geoTravelType() = GeoTravelType::International;
    airSeg2.origin() = &loc2;
    airSeg2.destination() = &loc3;
    airSeg2.boardMultiCity() = loc2.loc();
    airSeg2.offMultiCity() = loc3.loc();
    airSeg2.carrier() = cc2;

    DateTime newDate1(airSeg1.departureDT().year(),
                      airSeg1.departureDT().month(),
                      airSeg1.departureDT().day(),
                      airSeg1.departureDT().hours() + 24,
                      airSeg1.departureDT().minutes(),
                      airSeg1.departureDT().seconds());
    airSeg2.departureDT() = newDate1;

    Itin itin;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.intlSalesIndicator() = Itin::SITI;

    trx.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg2);
    trx.itin().push_back(&itin);

    FareMarket fareMarket1;
    fareMarket1.travelSeg().push_back(&airSeg1);
    fareMarket1.travelSeg().push_back(&airSeg2);
    fareMarket1.origin() = &loc1;
    fareMarket1.destination() = &loc3;
    fareMarket1.boardMultiCity() = loc1.loc();
    fareMarket1.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection1 = GlobalDirection::AT;
    fareMarket1.setGlobalDirection(globleDirection1);
    fareMarket1.governingCarrier() = "BA";

    Fare fare1;
    fare1.nucFareAmount() = 1845.26;

    FareInfo fareInfo1;
    fareInfo1._carrier = "BA";
    fareInfo1._market1 = "BKK";
    fareInfo1._market2 = "NYC";
    fareInfo1._fareClass = "PROMOYO1";
    fareInfo1._fareAmount = 600.00;
    fareInfo1._currency = "GBP";
    fareInfo1._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo1._ruleNumber = "2000";
    fareInfo1._routingNumber = "XXXX";
    fareInfo1._directionality = FROM;
    fareInfo1._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo1;
    tariffRefInfo1._fareTariffCode = "TAFPBA";
    tariffRefInfo1._tariffCat = 0;
    tariffRefInfo1._ruleTariff = 304;
    tariffRefInfo1._routingTariff1 = 99;
    tariffRefInfo1._routingTariff2 = 96;

    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket1, &tariffRefInfo1);

    PaxType paxType;
    PaxTypeCode paxTypeCode = "ADT";
    paxType.paxType() = paxTypeCode;
    PaxTypeFare paxTypeFare1;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket1);

    FareClassAppInfo appInfo1;
    appInfo1._fareType = "PGV";
    appInfo1._pricingCatType = 'S';
    paxTypeFare1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo fareClassAppSegInfo1;
    fareClassAppSegInfo1._paxType = "ADT";
    paxTypeFare1.fareClassAppSegInfo() = &fareClassAppSegInfo1;
    paxTypeFare1.fareTypeDesignator().setFTDPromotional();
    paxTypeFare1.cabin().setEconomyClass();

    PaxTypeBucket paxTypeCortege1;
    paxTypeCortege1.requestedPaxType() = &paxType;
    paxTypeCortege1.paxTypeFare().push_back(&paxTypeFare1);

    //=== end setup the first set of Fare, FareMarket and PaxTypeFare ===

    // set another thru fare

    Fare fare11;
    fare11.nucFareAmount() = 1500.20;

    FareInfo fareInfo11;
    fareInfo11._carrier = "BA";
    fareInfo11._market1 = "BKK";
    fareInfo11._market2 = "NYC";
    fareInfo11._fareClass = "Y11";
    fareInfo11._fareAmount = 600.00;
    fareInfo11._currency = "GBP";
    fareInfo11._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo11._ruleNumber = "2000";
    fareInfo11._routingNumber = "XXXX";
    fareInfo11._directionality = FROM;
    fareInfo11._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo11;
    tariffRefInfo11._fareTariffCode = "TAFPBA";
    tariffRefInfo11._tariffCat = 0;
    tariffRefInfo11._ruleTariff = 304;
    tariffRefInfo11._routingTariff1 = 99;
    tariffRefInfo11._routingTariff2 = 96;

    fare11.initialize(Fare::FS_International, &fareInfo11, fareMarket1, &tariffRefInfo11);

    PaxType paxType11;
    PaxTypeCode paxTypeCode11 = "ADT";
    paxType11.paxType() = paxTypeCode11;
    PaxTypeFare paxTypeFare11;
    paxTypeFare11.initialize(&fare11, &paxType11, &fareMarket1);

    FareClassAppInfo appInfo11;
    appInfo11._fareType = "EU";
    appInfo11._pricingCatType = 'N';
    paxTypeFare11.fareClassAppInfo() = &appInfo11;

    FareClassAppSegInfo fareClassAppSegInfo11;
    fareClassAppSegInfo11._paxType = "ADT";
    paxTypeFare11.fareClassAppSegInfo() = &fareClassAppSegInfo11;
    paxTypeFare11.cabin().setEconomyClass();

    paxTypeCortege1.paxTypeFare().push_back(&paxTypeFare11);
    fareMarket1.paxTypeCortege().push_back(paxTypeCortege1);

    //=== end setup the 1.1 set of Fare, FareMarket and PaxTypeFare ===

    //=== setup the second set of Fare, FareMarket and PaxTypeFare ===
    FareMarket fareMarket2;
    fareMarket2.travelSeg().push_back(&airSeg1);
    fareMarket2.origin() = &loc1;
    fareMarket2.destination() = &loc2;
    fareMarket2.boardMultiCity() = loc1.loc();
    fareMarket2.offMultiCity() = loc2.loc();

    GlobalDirection globleDirection2 = GlobalDirection::EH;
    fareMarket2.setGlobalDirection(globleDirection2);
    fareMarket2.governingCarrier() = "BA";

    Fare fare2;
    fare2.nucFareAmount() = 2200.02;

    FareInfo fareInfo2;
    fareInfo2._carrier = "BA";
    fareInfo2._market1 = "BKK";
    fareInfo2._market2 = "TYO";
    fareInfo2._fareClass = "YO2";
    fareInfo2._fareAmount = 600.00;
    fareInfo2._currency = "GBP";
    fareInfo2._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo2._ruleNumber = "5135";
    fareInfo2._routingNumber = "XXXX";
    fareInfo2._directionality = FROM;
    fareInfo2._globalDirection = GlobalDirection::WH;

    TariffCrossRefInfo tariffRefInfo2;
    tariffRefInfo2._fareTariffCode = "TAFPBA";
    tariffRefInfo2._tariffCat = 0;
    tariffRefInfo2._ruleTariff = 304;

    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2, &tariffRefInfo2);

    paxType.paxType() = paxTypeCode;
    PaxTypeFare paxTypeFare2;
    paxTypeFare2.initialize(&fare2, &paxType, &fareMarket2);

    FareClassAppInfo appInfo2;
    appInfo2._fareType = "XEF";
    appInfo2._pricingCatType = 'N';
    paxTypeFare2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo fareClassAppSegInfo2;
    fareClassAppSegInfo2._paxType = "ADT";
    paxTypeFare2.fareClassAppSegInfo() = &fareClassAppSegInfo2;
    paxTypeFare2.cabin().setEconomyClass();

    PaxTypeBucket paxTypeCortege2;
    paxTypeCortege2.requestedPaxType() = &paxType;
    paxTypeCortege2.paxTypeFare().push_back(&paxTypeFare2);
    fareMarket2.paxTypeCortege().push_back(paxTypeCortege2);

    //=== end setup the second set of Fare, FareMarket and PaxTypeFare ===

    //=== setup the third set of Fare, FareMarket and PaxTypeFare ===

    FareMarket fareMarket3;
    fareMarket3.travelSeg().push_back(&airSeg2);
    fareMarket3.origin() = &loc2;
    fareMarket3.destination() = &loc3;
    fareMarket3.boardMultiCity() = loc2.loc();
    fareMarket3.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection3 = GlobalDirection::WH;
    fareMarket3.setGlobalDirection(globleDirection3);
    fareMarket3.governingCarrier() = "BA";

    Fare fare3;
    fare3.nucFareAmount() = 1333.33;

    FareInfo fareInfo3;
    fareInfo3._carrier = "BA";
    fareInfo3._market1 = "TYO";
    fareInfo3._market2 = "NYC";
    fareInfo3._fareClass = "YO3";
    fareInfo3._fareAmount = 600.00;
    fareInfo3._currency = "GBP";
    fareInfo3._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo3._ruleNumber = "5135";
    fareInfo3._routingNumber = "XXXX";
    fareInfo3._directionality = FROM;
    fareInfo3._globalDirection = GlobalDirection::WH;

    TariffCrossRefInfo tariffRefInfo3;
    tariffRefInfo3._fareTariffCode = "TAFPBA";
    tariffRefInfo3._tariffCat = 0;
    tariffRefInfo3._ruleTariff = 304;

    fare3.initialize(Fare::FS_International, &fareInfo3, fareMarket3, &tariffRefInfo3);

    paxType.paxType() = paxTypeCode;
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

    PaxTypeBucket paxTypeCortege3;
    paxTypeCortege3.requestedPaxType() = &paxType;
    paxTypeCortege3.paxTypeFare().push_back(&paxTypeFare3);
    fareMarket3.paxTypeCortege().push_back(paxTypeCortege3);

    //=== end setup the third set of Fare, FareMarket and PaxTypeFare ===

    FareUsage fareUsage;
    fareUsage.travelSeg().push_back(&airSeg1);
    fareUsage.travelSeg().push_back(&airSeg2);
    fareUsage.paxTypeFare() = &paxTypeFare1;

    PricingUnit::Type ow = PricingUnit::Type::ONEWAY;
    //    PricingUnit::PUFareType nl = PricingUnit::NL;
    PricingUnit::PUFareType spcl = PricingUnit::SP;

    PricingUnit pu;
    pu.fareUsage().push_back(&fareUsage);
    pu.puType() = ow;
    pu.puFareType() = spcl;

    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    farePath.paxType() = &paxType;

    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket3);

    //---------------------------------------------------
    multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;
    multimap<uint16_t, const MinFareAppl*> applMap;
    map<uint16_t, const MinFareDefaultLogic*> defaultLogicMap;

    HIPMinimumFare hip(trx);

    CPPUNIT_ASSERT(hip.process(farePath, ruleLevelMap, applMap, defaultLogicMap) >= 0.0);
  }

  void testCat25FareProcess()
  {
    PricingTrx trx;

    PricingRequest request;

    request.globalDirection() = GlobalDirection::WH; // Western Hemisphere

    trx.setRequest(&request); // Set the request pointer (so pass address).

    trx.setTravelDate(DateTime::localTime());

    // Set the agent location
    Agent agent;
    Loc loca;
    loca.loc() = "LON";
    loca.nation() = "GP";

    agent.agentLocation() = &loca; // Sold in GP (Guadeloupe)

    request.ticketingAgent() = &agent;
    DateTime travelDate = DateTime::localTime();
    request.ticketingDT() = travelDate;

    // End building the agent location

    trx.diagnostic().diagnosticType() = Diagnostic718;
    trx.diagnostic().activate();

    Loc loc1;
    LocCode locCode1 = "BKK";
    loc1.loc() = locCode1;
    loc1.cityInd() = true;
    loc1.nation() = "TH";
    loc1.area() = IATA_AREA3;
    loc1.subarea() = NATION_THAILAND;

    Loc loc2;
    LocCode locCode2 = "TYO";
    loc2.loc() = locCode2;
    loc2.cityInd() = true;
    loc2.nation() = "JP";
    loc2.area() = IATA_AREA3;
    loc2.subarea() = "JP";

    Loc loc3;
    LocCode locCode3 = "NYC";
    loc3.loc() = locCode3;
    loc3.cityInd() = true;
    loc3.nation() = "US";
    loc3.area() = IATA_AREA1;
    loc3.subarea() = NORTH_AMERICA;

    CarrierCode cc1 = "BA";
    CarrierCode cc2 = "BA";

    AirSeg airSeg1;
    airSeg1.pnrSegment() = 0;
    airSeg1.geoTravelType() = GeoTravelType::International;
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    airSeg1.boardMultiCity() = loc1.loc();
    airSeg1.offMultiCity() = loc2.loc();
    airSeg1.carrier() = cc1;
    airSeg1.stopOver() = true;
    airSeg1.departureDT() = DateTime::localTime();

    AirSeg airSeg2;
    airSeg2.pnrSegment() = 1;
    airSeg2.geoTravelType() = GeoTravelType::International;
    airSeg2.origin() = &loc2;
    airSeg2.destination() = &loc3;
    airSeg2.boardMultiCity() = loc2.loc();
    airSeg2.offMultiCity() = loc3.loc();
    airSeg2.carrier() = cc2;

    DateTime newDate1(airSeg1.departureDT().year(),
                      airSeg1.departureDT().month(),
                      airSeg1.departureDT().day(),
                      airSeg1.departureDT().hours() + 24,
                      airSeg1.departureDT().minutes(),
                      airSeg1.departureDT().seconds());
    airSeg2.departureDT() = newDate1;

    Itin itin;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.intlSalesIndicator() = Itin::SITI;

    trx.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg2);
    trx.itin().push_back(&itin);

    FareMarket fareMarket1;

    fareMarket1.travelSeg().push_back(&airSeg1);
    fareMarket1.travelSeg().push_back(&airSeg2);
    fareMarket1.origin() = &loc1;
    fareMarket1.destination() = &loc3;
    fareMarket1.boardMultiCity() = loc1.loc();
    fareMarket1.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection1 = GlobalDirection::AT;
    fareMarket1.setGlobalDirection(globleDirection1);
    fareMarket1.governingCarrier() = "BA";

    Fare fare1;
    fare1.nucFareAmount() = 1845.26;

    FareInfo fareInfo1;
    fareInfo1._carrier = "BA";
    fareInfo1._market1 = "BKK";
    fareInfo1._market2 = "NYC";
    fareInfo1._fareClass = "YO1";
    fareInfo1._fareAmount = 600.00;
    fareInfo1._currency = "GBP";
    fareInfo1._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo1._ruleNumber = "2000";
    fareInfo1._routingNumber = "XXXX";
    fareInfo1._directionality = FROM;
    fareInfo1._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo1;
    tariffRefInfo1._fareTariffCode = "TAFPBA";
    tariffRefInfo1._tariffCat = 0;
    tariffRefInfo1._ruleTariff = 304;
    tariffRefInfo1._routingTariff1 = 99;
    tariffRefInfo1._routingTariff2 = 96;

    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket1, &tariffRefInfo1);

    PaxType paxType;
    PaxTypeCode paxTypeCode = "ADT";
    paxType.paxType() = paxTypeCode;
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

    //=== end setup the first set of Fare, FareMarket and PaxTypeFare ===

    // set another thru fare
    Fare fare11;
    fare11.nucFareAmount() = 2000.20;

    FareInfo fareInfo11;
    fareInfo11._carrier = "BA";
    fareInfo11._market1 = "BKK";
    fareInfo11._market2 = "NYC";
    fareInfo11._fareClass = "Y11";
    fareInfo11._fareAmount = 600.00;
    fareInfo11._currency = "GBP";
    fareInfo11._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo11._ruleNumber = "2000";
    fareInfo11._routingNumber = "XXXX";
    fareInfo11._directionality = FROM;
    fareInfo11._globalDirection = GlobalDirection::AT;

    TariffCrossRefInfo tariffRefInfo11;
    tariffRefInfo11._fareTariffCode = "TAFPBA";
    tariffRefInfo11._tariffCat = 0;
    tariffRefInfo11._ruleTariff = 304;
    tariffRefInfo11._routingTariff1 = 99;
    tariffRefInfo11._routingTariff2 = 96;

    fare11.initialize(Fare::FS_International, &fareInfo11, fareMarket1, &tariffRefInfo11);

    PaxType paxType11;
    PaxTypeCode paxTypeCode11 = "ADT";
    paxType11.paxType() = paxTypeCode11;

    PaxTypeFare paxTypeFare11;
    paxTypeFare11.initialize(&fare11, &paxType11, &fareMarket1);

    FareClassAppInfo appInfo11;
    appInfo11._fareType = "XEX";
    appInfo11._pricingCatType = 'S';
    paxTypeFare11.fareClassAppInfo() = &appInfo11;

    FareClassAppSegInfo fareClassAppSegInfo11;
    fareClassAppSegInfo11._paxType = "ADT";
    paxTypeFare11.fareClassAppSegInfo() = &fareClassAppSegInfo11;
    paxTypeFare11.cabin().setEconomyClass();

    PaxTypeBucket paxTypeCortege1;
    paxTypeCortege1.requestedPaxType() = &paxType;
    paxTypeCortege1.paxTypeFare().push_back(&paxTypeFare1);
    paxTypeCortege1.paxTypeFare().push_back(&paxTypeFare11);
    fareMarket1.paxTypeCortege().push_back(paxTypeCortege1);

    FareByRuleApp fbrApp;
    fbrApp.vendor() = "ATP";
    fbrApp.carrier() = "AA";
    fbrApp.segCnt() = 1;
    fbrApp.primePaxType() = "ADT";

    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.fareInd() = RuleConst::ADD_SPECIFIED_TO_CALCULATED;
    fbrItemInfo.percent() = 100;
    fbrItemInfo.specifiedFareAmt1() = 23.00;
    fbrItemInfo.specifiedCur1() = "GBP";

    FBRPaxTypeFareRuleData fbrPaxTypeFareRuleData;
    fbrPaxTypeFareRuleData.ruleItemInfo() = &fbrItemInfo;
    fbrPaxTypeFareRuleData.fbrApp() = &fbrApp;
    PaxTypeFare basePaxTypeFare;
    fbrPaxTypeFareRuleData.baseFare() = &basePaxTypeFare;
    TariffCrossRefInfo baseTariffRefInfo;
    baseTariffRefInfo._tariffCat = 0; // public tariff
    Fare baseFare;
    FareInfo baseFareInfo;
    baseFare.initialize(Fare::FS_International, &baseFareInfo, fareMarket1, &baseTariffRefInfo);
    basePaxTypeFare.initialize(&baseFare, &paxType, &fareMarket1);

    DataHandle dataHandle;
    paxTypeFare11.setRuleData(RuleConst::FARE_BY_RULE, dataHandle, &fbrPaxTypeFareRuleData, true);

    paxTypeFare11.status().set(PaxTypeFare::PTF_FareByRule);

    //=== end setup the 1.1 set of Fare, FareMarket and PaxTypeFare ===

    //=== setup the second set of Fare, FareMarket and PaxTypeFare ===
    FareMarket fareMarket2;

    fareMarket2.travelSeg().push_back(&airSeg1);
    fareMarket2.origin() = &loc1;
    fareMarket2.destination() = &loc2;
    fareMarket2.boardMultiCity() = loc1.loc();
    fareMarket2.offMultiCity() = loc2.loc();

    GlobalDirection globleDirection2 = GlobalDirection::EH;
    fareMarket2.setGlobalDirection(globleDirection2);
    fareMarket2.governingCarrier() = "BA";

    Fare fare2;
    fare2.nucFareAmount() = 2200.02;

    FareInfo fareInfo2;
    fareInfo2._carrier = "BA";
    fareInfo2._market1 = "BKK";
    fareInfo2._market2 = "TYO";
    fareInfo2._fareClass = "YO2";
    fareInfo2._fareAmount = 600.00;
    fareInfo2._currency = "GBP";
    fareInfo2._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo2._ruleNumber = "5135";
    fareInfo2._routingNumber = "XXXX";
    fareInfo2._directionality = FROM;
    fareInfo2._globalDirection = GlobalDirection::WH;

    TariffCrossRefInfo tariffRefInfo2;
    tariffRefInfo2._fareTariffCode = "TAFPBA";
    tariffRefInfo2._tariffCat = 0;
    tariffRefInfo2._ruleTariff = 304;

    fare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2, &tariffRefInfo2);

    paxType.paxType() = paxTypeCode;
    PaxTypeFare paxTypeFare2;
    paxTypeFare2.initialize(&fare2, &paxType, &fareMarket2);
    fareMarket2.allPaxTypeFare().push_back(&paxTypeFare2);

    FareClassAppInfo appInfo2;
    appInfo2._fareType = "EU";
    appInfo2._pricingCatType = 'N';
    paxTypeFare2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo fareClassAppSegInfo2;
    fareClassAppSegInfo2._paxType = "ADT";
    paxTypeFare2.fareClassAppSegInfo() = &fareClassAppSegInfo2;
    paxTypeFare2.cabin().setEconomyClass();

    PaxTypeBucket paxTypeCortege2;
    paxTypeCortege2.requestedPaxType() = &paxType;
    paxTypeCortege2.paxTypeFare().push_back(&paxTypeFare2);
    fareMarket2.paxTypeCortege().push_back(paxTypeCortege2);

    //=== end setup the second set of Fare, FareMarket and PaxTypeFare ===

    //=== setup the third set of Fare, FareMarket and PaxTypeFare ===

    FareMarket fareMarket3;

    fareMarket3.travelSeg().push_back(&airSeg2);
    fareMarket3.origin() = &loc2;
    fareMarket3.destination() = &loc3;
    fareMarket3.boardMultiCity() = loc2.loc();
    fareMarket3.offMultiCity() = loc3.loc();

    GlobalDirection globleDirection3 = GlobalDirection::WH;
    fareMarket3.setGlobalDirection(globleDirection3);
    fareMarket3.governingCarrier() = "BA";

    Fare fare3;
    fare3.nucFareAmount() = 1333.33;

    FareInfo fareInfo3;
    fareInfo3._carrier = "BA";
    fareInfo3._market1 = "TYO";
    fareInfo3._market2 = "NYC";
    fareInfo3._fareClass = "YO3";
    fareInfo3._fareAmount = 600.00;
    fareInfo3._currency = "GBP";
    fareInfo3._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo3._ruleNumber = "5135";
    fareInfo3._routingNumber = "XXXX";
    fareInfo3._directionality = FROM;
    fareInfo3._globalDirection = GlobalDirection::WH;

    TariffCrossRefInfo tariffRefInfo3;
    tariffRefInfo3._fareTariffCode = "TAFPBA";
    tariffRefInfo3._tariffCat = 0;
    tariffRefInfo3._ruleTariff = 304;

    fare3.initialize(Fare::FS_International, &fareInfo3, fareMarket3, &tariffRefInfo3);

    paxType.paxType() = paxTypeCode;
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

    PaxTypeBucket paxTypeCortege3;
    paxTypeCortege3.requestedPaxType() = &paxType;
    paxTypeCortege3.paxTypeFare().push_back(&paxTypeFare3);
    fareMarket3.paxTypeCortege().push_back(paxTypeCortege3);

    //=== end setup the third set of Fare, FareMarket and PaxTypeFare ===

    FareUsage fareUsage;
    fareUsage.travelSeg().push_back(&airSeg1);
    fareUsage.travelSeg().push_back(&airSeg2);
    fareUsage.paxTypeFare() = &paxTypeFare11;

    PricingUnit::Type ow = PricingUnit::Type::ONEWAY;
    //    PricingUnit::PUFareType nl = PricingUnit::NL;
    PricingUnit::PUFareType spcl = PricingUnit::SP;

    PricingUnit pu;
    pu.fareUsage().push_back(&fareUsage);
    pu.puType() = ow;
    pu.puFareType() = spcl;

    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu);
    farePath.paxType() = &paxType;

    trx.fareMarket().push_back(&fareMarket1);
    trx.fareMarket().push_back(&fareMarket2);
    trx.fareMarket().push_back(&fareMarket3);

    //---------------------------------------------------
    multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;
    multimap<uint16_t, const MinFareAppl*> applMap;
    map<uint16_t, const MinFareDefaultLogic*> defaultLogicMap;

    HIPMinimumFare hip(trx);

    CPPUNIT_ASSERT(hip.process(farePath, ruleLevelMap, applMap, defaultLogicMap) >= 0.0);

    //--- Make the thru fare become double discount fare ---

    DiscountInfo discountInfo;
    discountInfo.discPercent() = 50;
    discountInfo.farecalcInd() = RuleConst::ADD_SPECIFIED_TO_CALCULATED;
    discountInfo.cur1() = "GBP";
    discountInfo.cur2() = "GBP";
    discountInfo.fareAmt1() = 10.00;
    discountInfo.fareAmt2() = 8.00;

    //--- set thru fare amount ---

    PaxTypeFareRuleData ruleData;
    ruleData.ruleItemInfo() = &discountInfo;
    paxTypeFare11.setRuleData(19, dataHandle, &ruleData, true);
    paxTypeFare11.status().set(PaxTypeFare::PTF_Discounted);

    paxTypeFare11.nucFareAmount() = 500.00;
    fareUsage.paxTypeFare() = &paxTypeFare11;

    HIPMinimumFare hipDoubleDiscount(trx);

    CPPUNIT_ASSERT(hipDoubleDiscount.process(farePath, ruleLevelMap, applMap, defaultLogicMap) >=
                   0.0);
  }
  void testSkipProcessForSpecialOnly_Normal_NoApplyDefault_NoSpec()
  {
    initMinFareApplications(true, false, false, true);
    CPPUNIT_ASSERT(false == _hip->skipProcessForSpecialOnly());
  }
  void testSkipProcessForSpecialOnly_Normal_NoApplyDefault_Spec()
  {
    initMinFareApplications(true, false, true, false);
    CPPUNIT_ASSERT(true == _hip->skipProcessForSpecialOnly());
  }
  void testSkipProcessForSpecialOnly_Normal_ApplyDefault_NoSpec()
  {
    initMinFareApplications(true, true, true, false);
    CPPUNIT_ASSERT(false == _hip->skipProcessForSpecialOnly());
  }
  void testSkipProcessForSpecialOnly_Normal_ApplyDefault_Spec()
  {
    initMinFareApplications(true, true, false, true);
    CPPUNIT_ASSERT(true == _hip->skipProcessForSpecialOnly());
  }
  void testSkipProcessForSpecialOnly_NotNormal_ApplyDefault_Spec()
  {
    initMinFareApplications(false, false, true, false);
    CPPUNIT_ASSERT(false == _hip->skipProcessForSpecialOnly());
  }
  void initMinFareApplications(bool procesNormal,
                               bool applyDefaultLogic,
                               bool specOnly,
                               bool defSepcOnly)
  {
    MinFareDefaultLogic* defLogic = _memHandle.create<MinFareDefaultLogic>();
    _hip->_matchedDefaultItem = defLogic;

    _hip->_hipProcess =
        procesNormal ? MinimumFare::NORMAL_HIP_PROCESSING : MinimumFare::DEFAULT_HIP_PROCESSING;
    _minFareAppl->applyDefaultLogic() = applyDefaultLogic ? MinimumFare::YES : MinimumFare::NO;
    _minFareAppl->spclHipSpclOnlyInd() = specOnly ? MinimumFare::YES : MinimumFare::NO;
    defLogic->spclHipSpclOnlyInd() = defSepcOnly ? MinimumFare::YES : MinimumFare::NO;
  }

  void
  initAirSeg(AirSeg& travelSeg, GeoTravelType geoTravelType, Loc* orig, Loc* dest, CarrierCode cxr)
  {
    travelSeg.geoTravelType() = geoTravelType;
    travelSeg.origin() = orig;
    travelSeg.destination() = dest;
    travelSeg.boardMultiCity() = orig->loc();
    travelSeg.offMultiCity() = dest->loc();
    travelSeg.carrier() = cxr;
    travelSeg.stopOver() = true;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(HIPMinimumFareTest);
}
