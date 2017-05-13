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
#include "MinFares/BHCMinimumFare.h"

#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "MinFares/HIPMinimumFare.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingRequest.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class BHCMinimumFareTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(BHCMinimumFareTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST_SUITE_END();

  void testProcess()
  {
    PricingTrx trx;

    trx.diagnostic().diagnosticType() = Diagnostic718;
    trx.diagnostic().activate();

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

    PaxType paxType;
    paxType.paxType() = "ADT";

    Loc loc1;
    loc1.loc() = "AAA";
    loc1.nation() = "US";
    loc1.subarea() = NORTH_AMERICA;
    loc1.cityInd() = true;
    loc1.area() = IATA_AREA1;

    Loc loc2;
    loc2.loc() = "BBB";
    loc2.nation() = "CA";
    loc2.subarea() = NORTH_AMERICA;
    loc2.cityInd() = true;
    loc2.area() = IATA_AREA1;

    Loc loc3;
    loc3.loc() = "CCC";
    loc3.nation() = "FR";
    loc3.subarea() = EUROPE;
    loc3.cityInd() = true;
    loc3.area() = IATA_AREA2;

    Loc loc4;
    loc4.loc() = "DDD";
    loc4.nation() = "US";
    loc4.subarea() = NORTH_AMERICA;
    loc4.cityInd() = true;
    loc4.area() = IATA_AREA1;

    AirSeg airSeg1;
    airSeg1.segmentOrder() = 0;
    airSeg1.pnrSegment() = 0;
    airSeg1.geoTravelType() = GeoTravelType::International;
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    airSeg1.carrier() = "BA";
    airSeg1.stopOver() = true;
    airSeg1.departureDT() = DateTime::localTime();

    AirSeg airSeg2;
    airSeg2.segmentOrder() = 1;
    airSeg2.pnrSegment() = 1;
    airSeg2.geoTravelType() = GeoTravelType::International;
    airSeg2.origin() = &loc2;
    airSeg2.destination() = &loc3;
    airSeg2.carrier() = "BA";
    airSeg2.stopOver() = true;

    DateTime newDate1(airSeg1.departureDT().year(),
                      airSeg1.departureDT().month(),
                      airSeg1.departureDT().day(),
                      airSeg1.departureDT().hours() + 24,
                      airSeg1.departureDT().minutes(),
                      airSeg1.departureDT().seconds());
    airSeg2.departureDT() = newDate1;

    AirSeg airSeg3;
    airSeg3.segmentOrder() = 2;
    airSeg3.pnrSegment() = 2;
    airSeg3.geoTravelType() = GeoTravelType::International;
    airSeg3.origin() = &loc3;
    airSeg3.destination() = &loc4;
    airSeg3.carrier() = "BA";

    DateTime newDate2(airSeg2.departureDT().year(),
                      airSeg2.departureDT().month(),
                      airSeg2.departureDT().day(),
                      airSeg2.departureDT().hours() + 24,
                      airSeg2.departureDT().minutes(),
                      airSeg2.departureDT().seconds());
    airSeg3.departureDT() = newDate2;

    Itin itin;
    itin.travelSeg().push_back(&airSeg1);
    itin.travelSeg().push_back(&airSeg2);
    itin.travelSeg().push_back(&airSeg3);
    itin.intlSalesIndicator() = Itin::SITI;
    trx.itin().push_back(&itin);

    trx.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg2);
    trx.travelSeg().push_back(&airSeg3);

    FareMarket faremkt1;
    faremkt1.origin() = &loc1;
    faremkt1.destination() = &loc3;
    faremkt1.boardMultiCity() = loc1.loc();
    faremkt1.offMultiCity() = loc3.loc();
    faremkt1.governingCarrier() = "BA";
    faremkt1.travelSeg().push_back(&airSeg1);
    faremkt1.travelSeg().push_back(&airSeg2);

    PaxTypeFare paxtf1;
    //    paxtf1.fareMarket() = &faremkt1;

    Fare fare1;
    fare1.nucFareAmount() = 1700.00;

    FareInfo fareInfo1;
    fareInfo1._carrier = "BA";
    fareInfo1._market1 = "AAA";
    fareInfo1._market2 = "CCC";
    fareInfo1._fareClass = "WMLUQOW";
    fareInfo1._fareAmount = 1700.00;
    fareInfo1._currency = "NUC";
    fareInfo1._owrt = ONE_WAY_MAY_BE_DOUBLED;
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

    fare1.initialize(Fare::FS_International, &fareInfo1, faremkt1, &tariffRefInfo1);

    FareClassAppInfo appInfo1;
    // For Normal Fare
    // appInfo1._fareType = "EU";
    // appInfo1._pricingCatType = 'N';
    // For special Fare
    appInfo1._fareType = "XPN";
    appInfo1._pricingCatType = 'S';
    paxtf1.fareTypeDesignator().setFTDUnknown();

    paxtf1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo fareClassAppSegInfo1;
    fareClassAppSegInfo1._paxType = "ADT";
    paxtf1.fareClassAppSegInfo() = &fareClassAppSegInfo1;
    paxtf1.initialize(&fare1, &paxType, &faremkt1);
    //    fare1.setFareInfo(&fareInfo1);
    //    paxtf1.setFare(&fare1);
    paxtf1.cabin().setEconomyClass();

    PaxTypeBucket paxTypeCortege1;
    paxTypeCortege1.requestedPaxType() = &paxType;
    paxTypeCortege1.paxTypeFare().push_back(&paxtf1);
    faremkt1.paxTypeCortege().push_back(paxTypeCortege1);
    faremkt1.governingCarrier() = "BA";

    //=== end setup the first set of Fare, FareMarket and PaxTypeFare ===

    //=== setup the second set of Fare, FareMarket and PaxTypeFare ===

    FareMarket faremkt2;

    faremkt2.origin() = &loc1;
    faremkt2.destination() = &loc2;
    faremkt2.boardMultiCity() = loc1.loc();
    faremkt2.offMultiCity() = loc2.loc();
    faremkt2.governingCarrier() = "BA";
    faremkt2.travelSeg().push_back(&airSeg1);

    PaxTypeFare paxtf2;
    paxtf2.fareMarket() = &faremkt2;

    Fare fare2;
    fare2.nucFareAmount() = 600.00;

    FareInfo fareInfo2;
    fareInfo2._carrier = "BA";
    fareInfo2._market1 = "AAA";
    fareInfo2._market2 = "BBB";
    fareInfo2._fareClass = "YLAP   ";
    fareInfo2._fareAmount = 600.00;
    fareInfo2._currency = "NUC";
    fareInfo2._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo2._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo2._ruleNumber = "5135";
    fareInfo2._routingNumber = "XXXX";
    fareInfo2._directionality = FROM;
    fareInfo2._globalDirection = WH;

    TariffCrossRefInfo tariffRefInfo2;
    tariffRefInfo2._fareTariffCode = "TAFPBA";
    tariffRefInfo2._tariffCat = 0;
    tariffRefInfo2._ruleTariff = 304;
    tariffRefInfo2._routingTariff1 = 99;
    tariffRefInfo2._routingTariff2 = 96;

    fare2.initialize(Fare::FS_International, &fareInfo2, faremkt2, &tariffRefInfo2);

    FareClassAppInfo appInfo2;
    // appInfo2._fareType = "EU";
    // appInfo2._pricingCatType = 'N';
    appInfo2._fareType = "XPN";
    appInfo2._pricingCatType = 'S';
    paxtf2.fareTypeDesignator().setFTDUnknown();
    paxtf2.fareClassAppInfo() = &appInfo2;

    FareClassAppSegInfo fareClassAppSegInfo2;
    fareClassAppSegInfo2._paxType = "ADT";
    paxtf2.fareClassAppSegInfo() = &fareClassAppSegInfo2;
    paxtf2.initialize(&fare2, &paxType, &faremkt2);
    //    fare2.setFareInfo(&fareInfo2);
    //    paxtf2.setFare(&fare2);
    paxtf2.cabin().setEconomyClass();

    PaxTypeBucket paxTypeCortege2;
    paxTypeCortege2.requestedPaxType() = &paxType;
    paxTypeCortege2.paxTypeFare().push_back(&paxtf2);
    faremkt2.paxTypeCortege().push_back(paxTypeCortege2);

    faremkt2.governingCarrier() = "BA";

    //=== end setup the second set of Fare, FareMarket and PaxTypeFare ===

    //=== setup the third set of Fare, FareMarket and PaxTypeFare ===

    FareMarket faremkt3;

    faremkt3.origin() = &loc1;
    faremkt3.destination() = &loc4;
    faremkt3.governingCarrier() = "BA";
    faremkt3.boardMultiCity() = loc1.loc();
    faremkt3.offMultiCity() = loc4.loc();
    faremkt3.travelSeg().push_back(&airSeg1);
    faremkt3.travelSeg().push_back(&airSeg2);
    faremkt3.travelSeg().push_back(&airSeg3);

    PaxTypeFare paxtf3;
    paxtf3.fareMarket() = &faremkt3;
    paxtf3.cabin().setEconomyClass();

    //--- Mock Fare
    Fare fare3;
    fare3.nucFareAmount() = 1400.00;

    FareInfo fareInfo3;
    fareInfo3._carrier = "BA";
    fareInfo3._market1 = "AAA";
    fareInfo3._market2 = "DDD";
    fareInfo3._fareClass = "YLA3   ";
    fareInfo3._fareAmount = 1400.00;
    fareInfo3._currency = "NUC";
    fareInfo3._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo3._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo3._ruleNumber = "5135";
    fareInfo3._routingNumber = "XXXX";
    fareInfo3._directionality = FROM;
    fareInfo3._globalDirection = WH;

    TariffCrossRefInfo tariffRefInfo3;
    tariffRefInfo3._fareTariffCode = "TAFPBA";
    tariffRefInfo3._tariffCat = 0;
    tariffRefInfo3._ruleTariff = 304;
    tariffRefInfo3._routingTariff1 = 99;
    tariffRefInfo3._routingTariff2 = 96;

    fare3.initialize(Fare::FS_International, &fareInfo3, faremkt3, &tariffRefInfo3);

    FareClassAppInfo appInfo3;
    // appInfo3._fareType = "EU";
    // appInfo3._pricingCatType = 'N';
    appInfo3._fareType = "XPN";
    appInfo3._pricingCatType = 'S';
    paxtf3.fareTypeDesignator().setFTDUnknown();
    paxtf3.fareClassAppInfo() = &appInfo3;

    FareClassAppSegInfo fareClassAppSegInfo3;
    fareClassAppSegInfo3._paxType = "ADT";
    paxtf3.fareClassAppSegInfo() = &fareClassAppSegInfo3;
    paxtf3.initialize(&fare3, &paxType, &faremkt3);

    //    fare3.setFareInfo(&fareInfo3);

    //    paxtf3.setFare(&fare3);

    PaxTypeBucket paxTypeCortege3;
    paxTypeCortege3.requestedPaxType() = &paxType;
    paxTypeCortege3.paxTypeFare().push_back(&paxtf3);
    faremkt3.paxTypeCortege().push_back(paxTypeCortege3);

    faremkt3.governingCarrier() = "BA";

    //=== end setup the third set of Fare, FareMarket and PaxTypeFare ===

    //=== setup the forth set of Fare, FareMarket and PaxTypeFare ===

    FareMarket faremkt4;

    faremkt4.origin() = &loc2;
    faremkt4.destination() = &loc3;
    faremkt4.boardMultiCity() = loc2.loc();
    faremkt4.offMultiCity() = loc3.loc();
    faremkt4.governingCarrier() = "BA";
    faremkt4.travelSeg().push_back(&airSeg2);

    PaxTypeFare paxtf4;
    paxtf4.fareMarket() = &faremkt4;
    paxtf4.cabin().setEconomyClass();

    //--- Mock Fare
    Fare fare4;
    fare4.nucFareAmount() = 1750.00;

    FareInfo fareInfo4;
    fareInfo4._carrier = "BA";
    fareInfo4._market1 = "BBB";
    fareInfo4._market2 = "CCC";
    fareInfo4._fareClass = "YLA3   ";
    fareInfo4._fareAmount = 1750.00;
    fareInfo4._currency = "NUC";
    fareInfo4._owrt = ONE_WAY_MAY_BE_DOUBLED;
    fareInfo4._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo4._ruleNumber = "5135";
    fareInfo4._routingNumber = "XXXX";
    fareInfo4._directionality = FROM;
    fareInfo4._globalDirection = WH;

    TariffCrossRefInfo tariffRefInfo4;
    tariffRefInfo4._fareTariffCode = "TAFPBA";
    tariffRefInfo4._tariffCat = 0;
    tariffRefInfo4._ruleTariff = 304;
    tariffRefInfo4._routingTariff1 = 99;
    tariffRefInfo4._routingTariff2 = 96;

    fare4.initialize(Fare::FS_International, &fareInfo4, faremkt4, &tariffRefInfo4);

    FareClassAppInfo appInfo4;
    // appInfo3._fareType = "EU";
    // appInfo3._pricingCatType = 'N';
    appInfo4._fareType = "XPN";
    appInfo4._pricingCatType = 'S';
    paxtf4.fareTypeDesignator().setFTDUnknown();
    paxtf4.fareClassAppInfo() = &appInfo4;

    FareClassAppSegInfo fareClassAppSegInfo4;
    fareClassAppSegInfo4._paxType = "ADT";
    paxtf4.fareClassAppSegInfo() = &fareClassAppSegInfo4;
    paxtf4.initialize(&fare4, &paxType, &faremkt4);

    PaxTypeBucket paxTypeCortege4;
    paxTypeCortege4.requestedPaxType() = &paxType;
    paxTypeCortege4.paxTypeFare().push_back(&paxtf4);
    faremkt4.paxTypeCortege().push_back(paxTypeCortege4);

    faremkt4.governingCarrier() = "BA";

    //=== end setup the forth set of Fare, FareMarket and PaxTypeFare ===

    FareUsage fareUsage;
    fareUsage.travelSeg().push_back(&airSeg1);
    fareUsage.travelSeg().push_back(&airSeg2);
    fareUsage.travelSeg().push_back(&airSeg3);
    fareUsage.paxTypeFare() = &paxtf3;

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

    trx.fareMarket().push_back(&faremkt1);
    trx.fareMarket().push_back(&faremkt2);
    trx.fareMarket().push_back(&faremkt3);
    trx.fareMarket().push_back(&faremkt4);

    //--- HIP plus up in the thru PaxTypeFare ---
    //    paxtf3.hipAmount() = 50.00;
    //    paxtf3.hipBoardPoint() = loc1.loc();
    //    paxtf3.hipOffPoint() = loc3.loc();

    std::multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;
    std::multimap<uint16_t, const MinFareAppl*> applMap;
    std::map<uint16_t, const MinFareDefaultLogic*> defaultLogicMap;

    itin.setTravelDate(DateTime::emptyDate());

    HIPMinimumFare hip(trx);

    CPPUNIT_ASSERT(hip.process(farePath, ruleLevelMap, applMap, defaultLogicMap) >= 0.0);

    //    BHCMinimumFare bhc(normalExemptSet);
    //    CPPUNIT_ASSERT(bhc.process(trx, paxType, paxtf3));

    //##BASE FARE NUC  2050.00 AAA-CCC AAA-DDD PLUS UP   150.00 ##
    // std::cout << hip._diag.str() << std::endl;
    // std::string str = trx.diagnostic().toString();
    // std::cout << str.c_str() << std::endl;
  }
  TestMemHandle _memHandle;
  ConfigMan* _configMan;

public:

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _configMan = _memHandle.create<ConfigMan>();
  }

  void tearDown() { _memHandle.clear(); }

};
CPPUNIT_TEST_SUITE_REGISTRATION(BHCMinimumFareTest);
}
