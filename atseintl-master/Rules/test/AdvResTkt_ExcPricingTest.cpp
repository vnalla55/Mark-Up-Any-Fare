#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Agent.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DBAccess/DataHandle.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseEnums.h"
#include "Rules/AdvanceResTkt.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include <iostream>

using namespace tse;
using namespace std;

namespace tse
{
class AdvResTkt_ExcPricingTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AdvResTkt_ExcPricingTest);
  CPPUNIT_TEST(testIgnoreTktAfterRes_UnusedTktPUOutboundChanged);
  CPPUNIT_TEST(testIgnoreTktAfterRes_UnusedTktPUOutboundUnchanged);
  CPPUNIT_TEST(testIgnoreTktAfterRes_PartialUsedTkt);
  CPPUNIT_TEST(testvalidateAdvanceTktTime_IgnoreTktAfterResRestriction);
  CPPUNIT_TEST(testOptionB_PU_ToDate_PartialFlown);
  CPPUNIT_TEST(testOptionB_PU_ToDate_Unflown);
  CPPUNIT_TEST(testOptionB_PU_FromDate_FCUnchanged);
  CPPUNIT_TEST(testOptionB_PU_FromDate_FCChanged);
  CPPUNIT_TEST(testSubscriberUseD93TicketDT);
  CPPUNIT_TEST(testNonSubscriberDoNotUseD93TicketDT);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    // TestFixture::setUp();

    _dataHandle = &(_excTrx.dataHandle());
    _request.ticketingAgent() = &_tktAgent;
    _excTrx.setRequest(&_request);
    _excTrx.setOptions(&_options);
    _excTrx.reqType() = "FE"; // default for testing

    buildTravelPoints();
    buildTravelSegs();
    buildAllTestFareMarkets();
    buildAllPaxTypeFaresAndFareUsages();

    _dataHandle->get(pu12o_23i);

    _fu12->inbound() = false;
    pu12o_23i->fareUsage().push_back(_fu12);
    _fu23->inbound() = true;
    pu12o_23i->fareUsage().push_back(_fu23);

    _excTrx.exchangeItin().push_back(&_excItin);
  }

  void tearDown()
  {
    // TestFixture::tearDown();
  }

  void buildTravelPoints()
  {
    _loc1 = "ABC";
    _loc2 = "BCD";
    _loc3 = "CDE";
  }

  void buildTravelSegs()
  {
    _tvlSeg12 = new AirSeg();
    _tvlSeg12->pnrSegment() = 1;
    _tvlSeg12->origAirport() = _loc1;
    _tvlSeg12->destAirport() = _loc2;

    _tvlSeg23 = new AirSeg();
    _tvlSeg23->pnrSegment() = 2;
    _tvlSeg23->origAirport() = _loc2;
    _tvlSeg23->destAirport() = _loc3;
  }

  void buildAllTestFareMarkets()
  {
    _dataHandle->get(_fm12);
    _fm12->travelSeg().push_back(_tvlSeg12);

    _dataHandle->get(_fm23);
    _fm23->travelSeg().push_back(_tvlSeg23);
  }

  void buildAllPaxTypeFaresAndFareUsages()
  {
    _dataHandle->get(_ptf12);
    _ptf12->fareMarket() = _fm12;
    _dataHandle->get(_fu12);
    _fu12->paxTypeFare() = _ptf12;

    _dataHandle->get(_ptf23);
    _ptf23->fareMarket() = _fm23;
    _dataHandle->get(_fu23);
    _fu23->paxTypeFare() = _ptf23;
  }

  void reset(AdvResTktInfo& advResTktInfo)
  {
    advResTktInfo.advTktdepart() = 0;
    advResTktInfo.geoTblItemNo() = 0;
    advResTktInfo.firstResTod() = 0;
    advResTktInfo.lastResTod() = 0;
    advResTktInfo.advTktTod() = 0;
    advResTktInfo.advTktExcpTime() = 0;
    advResTktInfo.tktTSI() = 0;
    advResTktInfo.resTSI() = 0;
    advResTktInfo.firstResPeriod() = "  ";
    advResTktInfo.firstResUnit() = "  ";
    advResTktInfo.lastResPeriod() = "  ";
    advResTktInfo.lastResUnit() = "  ";
    advResTktInfo.permitted() = ' ';
    advResTktInfo.ticketed() = ' ';
    advResTktInfo.standby() = ' ';
    advResTktInfo.confirmedSector() = ' ';
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.advTktPeriod() = "  ";
    advResTktInfo.advTktUnit() = "  ";
    advResTktInfo.advTktOpt() = ' ';
    advResTktInfo.advTktDepartUnit() = ' ';
    advResTktInfo.advTktBoth() = ' ';
    advResTktInfo.advTktExcpUnit() = ' ';
  }

  void testIgnoreTktAfterRes_UnusedTktPUOutboundChanged()
  {
    _tvlSeg12->unflown() = true;
    _tvlSeg12->changeStatus() = TravelSeg::CHANGED;
    _tvlSeg23->unflown() = true;
    _tvlSeg23->changeStatus() = TravelSeg::UNCHANGED;

    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;
    Itin newItin;

    newItin.travelSeg().push_back(_tvlSeg12);
    newItin.travelSeg().push_back(_tvlSeg23);
    pu12o_23i->travelSeg().push_back(_tvlSeg12);
    pu12o_23i->travelSeg().push_back(_tvlSeg23);

    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT_EQUAL(false, advResTkt._ignoreTktAfterResRestriction);
  }

  void testIgnoreTktAfterRes_UnusedTktPUOutboundUnchanged()
  {
    _tvlSeg12->unflown() = true;
    _tvlSeg12->changeStatus() = TravelSeg::UNCHANGED;
    _tvlSeg23->unflown() = true;
    _tvlSeg23->changeStatus() = TravelSeg::UNCHANGED;

    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;
    reset(advResTktInfo);
    Itin newItin;

    newItin.travelSeg().push_back(_tvlSeg12);
    newItin.travelSeg().push_back(_tvlSeg23);
    pu12o_23i->travelSeg().push_back(_tvlSeg12);
    pu12o_23i->travelSeg().push_back(_tvlSeg23);

    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT_EQUAL(true, advResTkt._ignoreTktAfterResRestriction);
  }

  void testIgnoreTktAfterRes_PartialUsedTkt()
  {
    _tvlSeg12->unflown() = false;
    _tvlSeg12->changeStatus() = TravelSeg::UNCHANGED;
    _tvlSeg23->unflown() = true;
    _tvlSeg23->changeStatus() = TravelSeg::UNCHANGED;

    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;
    reset(advResTktInfo);
    Itin newItin;

    newItin.travelSeg().push_back(_tvlSeg12);
    newItin.travelSeg().push_back(_tvlSeg23);
    pu12o_23i->travelSeg().push_back(_tvlSeg12);
    pu12o_23i->travelSeg().push_back(_tvlSeg23);

    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT_EQUAL(true, advResTkt._ignoreTktAfterResRestriction);

    _tvlSeg23->changeStatus() = TravelSeg::CHANGED;
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT_EQUAL(true, advResTkt._ignoreTktAfterResRestriction);
  }

  void testvalidateAdvanceTktTime_IgnoreTktAfterResRestriction()
  {
    DiagManager diag(_excTrx, Diagnostic305);

    DateTime bookDT(2007, 9, 11, 7, 37, 0);
    DateTime ticketDT = bookDT.addDays(5);
    DateTime departureDT(2007, 10, 1, 8, 20, 0);

    AdvResTktInfo advResTktInfo;
    reset(advResTktInfo);

    advResTktInfo.advTktUnit() = "Hh";
    advResTktInfo.advTktPeriod() = "24";

    bool mayPassAfterRebook = false;

    AdvanceResTkt advResTkt;
    advResTkt._ignoreTktAfterResRestriction = false;

    PricingUnit pu;

    CPPUNIT_ASSERT_EQUAL(
        FAIL,
        advResTkt.validateAdvanceTktTime(
            &pu, ticketDT, bookDT, departureDT, advResTktInfo, diag, mayPassAfterRebook));

    advResTkt._ignoreTktAfterResRestriction = true;
    CPPUNIT_ASSERT_EQUAL(
        PASS,
        advResTkt.validateAdvanceTktTime(
            &pu, ticketDT, bookDT, departureDT, advResTktInfo, diag, mayPassAfterRebook));
  }

  void testOptionB_PU_ToDate_PartialFlown()
  {
    _excTrx.getOptions()->AdvancePurchaseOption() = 'T';

    Itin newItin;

    newItin.travelSeg().push_back(_tvlSeg12);
    newItin.travelSeg().push_back(_tvlSeg23);

    AdvResTktInfo advResTktInfo;
    reset(advResTktInfo);

    advResTktInfo.advTktUnit() = "Hh";
    advResTktInfo.advTktPeriod() = "24";

    _tvlSeg12->departureDT() = DateTime(2007, 10, 1, 8, 20, 0);
    _tvlSeg23->departureDT() = DateTime(2007, 10, 5, 8, 20, 0);

    _tvlSeg12->unflown() = false;
    _tvlSeg23->unflown() = true;

    AdvanceResTkt advResTkt;

    _tvlSeg12->changeStatus() = TravelSeg::UNCHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::CHANGED;

    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf12, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(DateTime::emptyDate() == advResTkt._tktToDateOverride);

    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(_tvlSeg23->departureDT() == advResTkt._tktToDateOverride);

    _tvlSeg12->changeStatus() = TravelSeg::UNCHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::UNCHANGED;

    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf12, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(DateTime::emptyDate() == advResTkt._tktToDateOverride);

    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(DateTime::emptyDate() == advResTkt._tktToDateOverride);
  }

  void testOptionB_PU_ToDate_Unflown()
  {
    _excTrx.getOptions()->AdvancePurchaseOption() = 'T';

    Itin newItin;

    newItin.travelSeg().push_back(_tvlSeg12);
    newItin.travelSeg().push_back(_tvlSeg23);
    pu12o_23i->travelSeg().push_back(_tvlSeg12);
    pu12o_23i->travelSeg().push_back(_tvlSeg23);

    AdvResTktInfo advResTktInfo;
    reset(advResTktInfo);

    advResTktInfo.advTktUnit() = "Hh";
    advResTktInfo.advTktPeriod() = "24";

    _tvlSeg12->departureDT() = DateTime(2007, 10, 1, 8, 20, 0);
    _tvlSeg23->departureDT() = DateTime(2007, 10, 5, 8, 20, 0);

    AdvanceResTkt advResTkt;

    // TO Date
    _tvlSeg12->unflown() = true;
    _tvlSeg23->unflown() = true;

    _tvlSeg12->changeStatus() = TravelSeg::UNCHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::CHANGED;

    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf12, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(DateTime::emptyDate() == advResTkt._tktToDateOverride);

    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(_tvlSeg23->departureDT() == advResTkt._tktToDateOverride);

    _tvlSeg12->changeStatus() = TravelSeg::CHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::UNCHANGED;

    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf12, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(_tvlSeg12->departureDT() == advResTkt._tktToDateOverride);

    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(DateTime::emptyDate() == advResTkt._tktToDateOverride);

    _tvlSeg12->changeStatus() = TravelSeg::UNCHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::UNCHANGED;

    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf12, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(DateTime::emptyDate() == advResTkt._tktToDateOverride);
    advResTkt._tktToDateOverride = DateTime::emptyDate();
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(DateTime::emptyDate() == advResTkt._tktToDateOverride);

    _tvlSeg12->changeStatus() = TravelSeg::CHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::CHANGED;

    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf12, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(_tvlSeg12->departureDT() == advResTkt._tktToDateOverride);
    advResTkt.initialize(_excTrx, advResTktInfo, *_ptf23, pu12o_23i, &newItin);
    CPPUNIT_ASSERT(_tvlSeg23->departureDT() == advResTkt._tktToDateOverride);
  }

  void testOptionB_PU_FromDate_FCUnchanged()
  {
    _excTrx.getOptions()->AdvancePurchaseOption() = 'T';

    DateTime tktDT[2];

    _excTrx.setOriginalTktIssueDT() = DateTime(2007, 9, 9, 2, 10, 0);
    _excTrx.getRequest()->ticketingDT() = DateTime(2007, 9, 13, 2, 10, 0);

    _tvlSeg12->departureDT() = DateTime(2007, 10, 1, 8, 20, 0);
    _tvlSeg23->departureDT() = DateTime(2007, 10, 5, 8, 20, 0);

    AdvanceResTkt advResTkt;

    _tvlSeg12->changeStatus() = TravelSeg::UNCHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::UNCHANGED;

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm12, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.originalTktIssueDT() == tktDT[0]);

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm23, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.originalTktIssueDT() == tktDT[0]);
  }

  void testOptionB_PU_FromDate_FCChanged()
  {
    _excTrx.getOptions()->AdvancePurchaseOption() = 'T';

    DateTime tktDT[2];

    _excTrx.setOriginalTktIssueDT() = DateTime(2007, 9, 9, 2, 10, 0);
    _excTrx.getRequest()->ticketingDT() = DateTime(2007, 9, 13, 2, 10, 0);

    _tvlSeg12->departureDT() = DateTime(2007, 10, 1, 8, 20, 0);
    _tvlSeg23->departureDT() = DateTime(2007, 10, 5, 8, 20, 0);

    AdvanceResTkt advResTkt;

    _tvlSeg12->changeStatus() = TravelSeg::CHANGED;
    _tvlSeg12->changeStatus() = TravelSeg::CHANGED;

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm12, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.getRequest()->ticketingDT() == tktDT[0]);

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm23, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.getRequest()->ticketingDT() == tktDT[0]);
  }

  void testSubscriberUseD93TicketDT()
  {
    _tktAgent.tvlAgencyPCC() = "ABC";

    DateTime tktDT[2];

    _excTrx.setOriginalTktIssueDT() = DateTime(2007, 9, 9, 2, 10, 0);
    _excTrx.purchaseDT() = DateTime(2007, 9, 19, 2, 0, 0);
    _excTrx.getRequest()->ticketingDT() = DateTime(2007, 9, 13, 2, 10, 0);

    _tvlSeg12->departureDT() = DateTime(2007, 10, 1, 8, 20, 0);
    _tvlSeg23->departureDT() = DateTime(2007, 10, 5, 8, 20, 0);

    AdvanceResTkt advResTkt;

    _tvlSeg12->changeStatus() = TravelSeg::UNCHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::UNCHANGED;

    _excTrx.getOptions()->AdvancePurchaseOption() = 'T';

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm12, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.purchaseDT() == tktDT[0]);

    _excTrx.getOptions()->AdvancePurchaseOption() = 'F';

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm12, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.purchaseDT() == tktDT[0]);

    _tvlSeg12->changeStatus() = TravelSeg::CHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::CHANGED;

    _excTrx.getOptions()->AdvancePurchaseOption() = 'T';

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm12, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.purchaseDT() == tktDT[0]);

    _excTrx.getOptions()->AdvancePurchaseOption() = 'F';

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm12, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.purchaseDT() == tktDT[0]);

    _tktAgent.tvlAgencyPCC() = "";
  }

  void testNonSubscriberDoNotUseD93TicketDT()
  {
    _tktAgent.tvlAgencyPCC() = "";

    DateTime tktDT[2];

    _excTrx.setOriginalTktIssueDT() = DateTime(2007, 9, 9, 2, 10, 0);
    _excTrx.purchaseDT() = DateTime(2007, 9, 19, 2, 0, 0);
    _excTrx.getRequest()->ticketingDT() = DateTime(2007, 9, 13, 2, 10, 0);

    _tvlSeg12->departureDT() = DateTime(2007, 10, 1, 8, 20, 0);
    _tvlSeg23->departureDT() = DateTime(2007, 10, 5, 8, 20, 0);

    Itin newItin;

    newItin.travelSeg().push_back(_tvlSeg12);
    newItin.travelSeg().push_back(_tvlSeg23);
    _excTrx.newItin().clear();
    _excTrx.newItin().push_back(&newItin);

    _excTrx.getOptions()->AdvancePurchaseOption() = 'T';

    AdvanceResTkt advResTkt;

    _tvlSeg12->changeStatus() = TravelSeg::UNCHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::UNCHANGED;

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm12, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.purchaseDT() != tktDT[0]);

    _tvlSeg12->changeStatus() = TravelSeg::CHANGED;
    _tvlSeg23->changeStatus() = TravelSeg::CHANGED;

    CPPUNIT_ASSERT_EQUAL(1, advResTkt.getEligibleTktDates(_excTrx, *_fm12, pu12o_23i, tktDT));
    CPPUNIT_ASSERT(_excTrx.purchaseDT() != tktDT[0]);
  }

private:
  ExchangePricingTrx _excTrx;
  ExcItin _excItin;
  Agent _tktAgent;
  PricingRequest _request;
  PricingOptions _options;
  DataHandle* _dataHandle;
  LocCode _loc1, _loc2, _loc3;
  TravelSeg* _tvlSeg12, *_tvlSeg23;

  FareMarket* _fm12, *_fm23;
  PaxTypeFare* _ptf12, *_ptf23;
  FareUsage* _fu12, *_fu23;
  PricingUnit* pu12o_23i;
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(AdvResTkt_ExcPricingTest);
