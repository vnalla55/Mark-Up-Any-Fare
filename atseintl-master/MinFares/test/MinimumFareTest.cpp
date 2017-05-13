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
#include "MinFares/MinimumFare.h"
#include "MinFares/HIPMinimumFare.h"

#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "Diagnostic/DCFactory.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "MinFares/MinFareChecker.h"
#include "test/include/MockGlobal.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "Common/TseConsts.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/MinFareAppl.h"
#include "MinFares/MinFareFareSelection.h"
#include "DBAccess/DiskCache.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

using namespace std;

namespace tse
{

class MinimumFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MinimumFareTest);
  CPPUNIT_TEST(testMatching);
  CPPUNIT_TEST(testAdjustRexDate_retrieval);
  CPPUNIT_TEST(testAdjustRexDate_travel);
  CPPUNIT_TEST(testRestorePricingDates);
  CPPUNIT_TEST(testCtmExemptedWhenCtmCheckNotApply);
  CPPUNIT_TEST(testCtmNotExemptedWhenCtmCheckApplyOnTktPoint);
  CPPUNIT_TEST(testDmcExemptedWhenDmcCheckNotApply);
  CPPUNIT_TEST(testDmcNotExemptedWhenDmcCheckApplyOnTktPoint);
  CPPUNIT_TEST(testCpmExemptedWhenCpmCheckNotApply);
  CPPUNIT_TEST(testCpmNotExemptedWhenCpmCheckApplyOnTktPoint);
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
  PricingTrx* _trx;
  MinimumFare* _minFare;
  MinFareAppl* _minFareAppl;
  AirSeg* _airSeg;
  Itin* _itin;
  vector<TravelSeg*> _thruTvlSegs;
  vector<TravelSeg*> _intermTvlSegs;
  vector<TravelSeg*>::iterator _intermBeginIter, _intermEndIter;

  void testMatching()
  {

    PricingTrx trx;

    PricingRequest request;

    request.globalDirection() = GlobalDirection::WH; // Western Hemisphere

    trx.setRequest(&request); // Set the request pointer (so pass address).

    // Set the agent location
    Agent agent;
    Loc loca;
    loca.loc() = "LON";
    loca.nation() = "GP";

    agent.agentLocation() = &loca; // Sold in GP (Guadeloupe)

    request.ticketingAgent() = &agent;
    // End building the agent location

    trx.diagnostic().diagnosticType() = Diagnostic701;
    trx.diagnostic().activate();

    Loc loc1;
    LocCode locCode1 = "BKK";
    loc1.loc() = locCode1;
    loc1.cityInd() = true;
    loc1.nation() = "TH";
    loc1.area() = IATA_AREA3;

    Loc loc2;
    LocCode locCode2 = "TYO";
    loc2.loc() = locCode2;
    loc2.cityInd() = true;
    loc2.nation() = "JP";
    loc2.area() = IATA_AREA3;

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
    airSeg1.segmentOrder() = 0;
    airSeg1.pnrSegment() = 0;
    airSeg1.geoTravelType() = GeoTravelType::International;
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    airSeg1.carrier() = cc1;
    airSeg1.stopOver() = true;

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

    fareMarket1.travelSeg().push_back(&airSeg1);
    fareMarket1.travelSeg().push_back(&airSeg2);

    Fare fare1;
    fare1.nucFareAmount() = 1845.26;

    FareInfo fareInfo1;
    fareInfo1._carrier = "KL";
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

    fareMarket1.origin() = &loc1;
    fareMarket1.destination() = &loc3;

    GlobalDirection globleDirection1 = GlobalDirection::AT;
    fareMarket1.setGlobalDirection(globleDirection1);
    fareMarket1.governingCarrier() = "KL";

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

    //=== setup the second set of Fare, FareMarket and PaxTypeFare ===
    FareMarket fareMarket2;

    fareMarket2.travelSeg().push_back(&airSeg1);

    Fare fare2;
    fare2.nucFareAmount() = 4000.22;

    FareInfo fareInfo2;
    fareInfo2._carrier = "KL";
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

    fareMarket2.origin() = &loc1;
    fareMarket2.destination() = &loc2;

    GlobalDirection globleDirection2 = GlobalDirection::EH;
    fareMarket2.setGlobalDirection(globleDirection2);
    fareMarket2.governingCarrier() = "KL";

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

    Fare fare3;
    fare3.nucFareAmount() = 3333.33;

    FareInfo fareInfo3;
    fareInfo3._carrier = "KL";
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

    fareMarket3.origin() = &loc2;
    fareMarket3.destination() = &loc3;

    GlobalDirection globleDirection3 = GlobalDirection::WH;
    fareMarket3.setGlobalDirection(globleDirection3);
    fareMarket3.governingCarrier() = "KL";

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
    PricingUnit::PUFareType nl = PricingUnit::NL;

    PricingUnit pu;
    pu.fareUsage().push_back(&fareUsage);
    pu.puType() = ow;
    pu.puFareType() = nl;

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

    try { hip.process(farePath, ruleLevelMap, applMap, defaultLogicMap); }
    catch (ErrorResponseException& ex) {}
  }

  void testAdjustRexDate_retrieval()
  {
    DateTime retrievalDate(2008, 01, 02);
    DateTime ticketingDate(2008, 01, 03);

    RexPricingTrx trx;
    trx.dataHandle().setTicketDate(ticketingDate);

    MinimumFare mf;
    mf.adjustRexPricingDates(trx, retrievalDate);

    CPPUNIT_ASSERT(trx.dataHandle().ticketDate() == retrievalDate);
  }

  void testAdjustRexDate_travel()
  {
    DateTime travelDate(2008, 01, 02);
    DateTime afterTravelDate(2008, 01, 03);

    RexPricingTrx trx;

    MinimumFare mf(travelDate);

    mf.adjustRexPricingDates(trx, afterTravelDate);

    CPPUNIT_ASSERT(mf.travelDate() == afterTravelDate);
  }

  void testRestorePricingDates()
  {
    DateTime rexRetrievalDate(2008, 01, 02);
    DateTime orginalTicketingDate(2008, 01, 03);

    RexPricingTrx trx;
    trx.dataHandle().setTicketDate(rexRetrievalDate);

    MinimumFare mf;
    mf.restorePricingDates(trx, orginalTicketingDate);

    CPPUNIT_ASSERT(trx.dataHandle().ticketDate() == orginalTicketingDate);
  }

  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = new PricingTrx();
    _itin = new Itin();
    _minFare = new MinimumFare();
    _minFareAppl = new MinFareAppl();
    _airSeg = new AirSeg();
    _intermTvlSegs.push_back(_airSeg);
    _intermBeginIter = _intermEndIter = _intermTvlSegs.begin();
  }

  void tearDown()
  {
    _memHandle.clear();
    delete _trx;
    delete _itin;
    delete _minFare;
    delete _minFareAppl;
    _intermTvlSegs.clear();
    delete _airSeg;
  }

  void testCtmExemptedWhenCtmCheckNotApply()
  {
    _minFareAppl->ctmCheckAppl() = NO;
    CPPUNIT_ASSERT(_minFare->checkIntermediateExclusion(*_trx,
                                                        CTM,
                                                        *_minFareAppl,
                                                        0,
                                                        true,
                                                        _thruTvlSegs,
                                                        *_itin,
                                                        0,
                                                        0,
                                                        MinFareFareSelection::OUTBOUND,
                                                        _intermBeginIter,
                                                        _intermEndIter));
  }

  void testCtmNotExemptedWhenCtmCheckApplyOnTktPoint()
  {
    _minFareAppl->ctmCheckAppl() = YES;
    _minFareAppl->ctmStopTktInd() = MinimumFare::TICKET_POINT;
    CPPUNIT_ASSERT(!_minFare->checkIntermediateExclusion(*_trx,
                                                         CTM,
                                                         *_minFareAppl,
                                                         0,
                                                         true,
                                                         _thruTvlSegs,
                                                         *_itin,
                                                         0,
                                                         0,
                                                         MinFareFareSelection::OUTBOUND,
                                                         _intermBeginIter,
                                                         _intermEndIter));
  }

  void testDmcExemptedWhenDmcCheckNotApply()
  {
    _minFareAppl->dmcCheckAppl() = NO;
    CPPUNIT_ASSERT(_minFare->checkIntermediateExclusion(*_trx,
                                                        DMC,
                                                        *_minFareAppl,
                                                        0,
                                                        true,
                                                        _thruTvlSegs,
                                                        *_itin,
                                                        0,
                                                        0,
                                                        MinFareFareSelection::OUTBOUND,
                                                        _intermBeginIter,
                                                        _intermEndIter));
  }

  void testDmcNotExemptedWhenDmcCheckApplyOnTktPoint()
  {
    _minFareAppl->dmcCheckAppl() = YES;
    _minFareAppl->dmcStopTktInd() = MinimumFare::TICKET_POINT;
    CPPUNIT_ASSERT(!_minFare->checkIntermediateExclusion(*_trx,
                                                         DMC,
                                                         *_minFareAppl,
                                                         0,
                                                         true,
                                                         _thruTvlSegs,
                                                         *_itin,
                                                         0,
                                                         0,
                                                         MinFareFareSelection::OUTBOUND,
                                                         _intermBeginIter,
                                                         _intermEndIter));
  }

  void testCpmExemptedWhenCpmCheckNotApply()
  {
    _minFareAppl->cpmCheckAppl() = NO;
    CPPUNIT_ASSERT(_minFare->checkIntermediateExclusion(*_trx,
                                                        CPM,
                                                        *_minFareAppl,
                                                        0,
                                                        true,
                                                        _thruTvlSegs,
                                                        *_itin,
                                                        0,
                                                        0,
                                                        MinFareFareSelection::OUTBOUND,
                                                        _intermBeginIter,
                                                        _intermEndIter));
  }

  void testCpmNotExemptedWhenCpmCheckApplyOnTktPoint()
  {
    _minFareAppl->cpmCheckAppl() = YES;
    _minFareAppl->cpmStopTktInd() = MinimumFare::TICKET_POINT;
    CPPUNIT_ASSERT(!_minFare->checkIntermediateExclusion(*_trx,
                                                         CPM,
                                                         *_minFareAppl,
                                                         0,
                                                         true,
                                                         _thruTvlSegs,
                                                         *_itin,
                                                         0,
                                                         0,
                                                         MinFareFareSelection::OUTBOUND,
                                                         _intermBeginIter,
                                                         _intermEndIter));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MinimumFareTest);
}
