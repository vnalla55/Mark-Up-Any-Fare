//----------------------------------------------------------------------------
//  Copyright Sabre 2004, 2009
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
#include <boost/assign/std/vector.hpp>

#include "BookingCode/FareBookingCodeValidator.h"
#include "Common/ClassOfService.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/AdvResTktInfo.h"
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DST.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TSIInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Rules/AdvanceResTkt.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestFareUsageFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"

namespace tse
{

using boost::assign::operator+=;

class AdvResTktTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AdvResTktTest);

  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testInitialize);
  CPPUNIT_TEST(testValidateFareMarketShouldPassFareMarketCoverItin);
  CPPUNIT_TEST(testValidateFareMarketShouldPassEachSectorXOrigMatchFMAndItinNoTktRest);
  CPPUNIT_TEST(testValidateFareMarketShouldFailOrigFMSameOrigItin);
  CPPUNIT_TEST(testValidateFareMarketShouldSoftPassReservedAllowed);
  CPPUNIT_TEST(testValidateFareMarketShouldSoftPassTSIEqual1);
  CPPUNIT_TEST(testValidateFareMarketShouldFailOnTicketing);
  CPPUNIT_TEST(testValidateFareMarketShouldPassAtPU);
  CPPUNIT_TEST(testValidateFareMarketShouldFailOnTicketing15MinAdvTicketing);
  CPPUNIT_TEST(testValidateFareMarketShouldSoftPassAtPU);
  CPPUNIT_TEST(testValidateFareMarketShouldFailOrigOfFMAndItinNotEqualPU);
  CPPUNIT_TEST(testValidateFareMarketShouldSoftPassOrginFMNotOrginItin);
  CPPUNIT_TEST(testValidateFareMarketShouldFailTktTSIEqual3);
  CPPUNIT_TEST(testValidateFareMarketShouldSoftPassRevalidOfPUForLastTktDate);
  CPPUNIT_TEST(testValidateFareMarketShouldSoftPassTktTSIEqual0);
  CPPUNIT_TEST(testValidate_FailOnInvalidRulInfo);
  CPPUNIT_TEST(testValidate_SoftpassAR_EXC_TRX_NewItinPhase);
  CPPUNIT_TEST(testPU);
  CPPUNIT_TEST(testWPNC);
  CPPUNIT_TEST(testValidateResTkt_RexAdjustTicketDT);
  CPPUNIT_TEST(testValidateResTkt_DoNotAdjustTicketDT);
  CPPUNIT_TEST(testGetLimitDateTime);
  CPPUNIT_TEST(testGetLimitDateTimeFailsWhenMonthIsOutOfRange);
  CPPUNIT_TEST(testGetLimitDateTimeForMinutesWhenReferenceTimeIsNotIgnored);
  CPPUNIT_TEST(testGetLimitDateTimeForHoursWhenReferenceTimeIsNotIgnored);
  CPPUNIT_TEST(testGetLimitDateTimeForDaysWhenReferenceTimeIsNotIgnored);
  CPPUNIT_TEST(testGetLimitDateTimeForMonthsWhenReferenceTimeIsNotIgnored);
  CPPUNIT_TEST(testValidateAdvanceTktTime);
  CPPUNIT_TEST(testDetermineTicketDate_TktLater);
  CPPUNIT_TEST(testDetermineTicketDate_TktEarlier);
  CPPUNIT_TEST(testDetermineTicketDate_notNewItin);

  CPPUNIT_TEST(testUpdateRebookStatusReturnTrueWhenAllFareUsagePass);
  CPPUNIT_TEST(testUpdateRebookStatusReturnFalseWhenFirstFareUsageFail);
  CPPUNIT_TEST(testUpdateRebookStatusReturnFalseWhenSecondFareUsageFail);
  CPPUNIT_TEST(testUpdateRebookStatusReturnSKIPwhenSegmentStatusNotInitialized);
  CPPUNIT_TEST(testUpdateRebookStatusReturnPASSwhenSegmentRebooked);
  CPPUNIT_TEST(testUpdateRebookStatusReturnFAILwhenSegmentNotRebooked);

  CPPUNIT_TEST(testCheckAvailReturnTrueWhenBookingCodeAvailable);
  CPPUNIT_TEST(testCheckAvailReturnFalseWhenBookingCodeNotAvailable);

  CPPUNIT_TEST(testPartOfLocalJourneyReturnFalseForNonPricingTrx);
  CPPUNIT_TEST(testPartOfLocalJourneyReturnFalseWhenJourneyNotActive);
  CPPUNIT_TEST(testPartOfLocalJourneyReturnFalseWhenJourneyNotApply);
  CPPUNIT_TEST(testPartOfLocalJourneyReturnFalseWhenSegArunk);
  CPPUNIT_TEST(testPartOfLocalJourneyReturnFalseWhenNotLocalJourneyCarrier);
  CPPUNIT_TEST(testPartOfLocalJourneyReturnFalseWhenNotWPNCentry);
  CPPUNIT_TEST(testPartOfLocalJourneyReturnFalseWhenSegNotFoundInFlowMarket);
  CPPUNIT_TEST(testPartOfLocalJourneyReturnTrueWhenSegFoundInFlowMarket);

  CPPUNIT_TEST(testGetFlowMarketReturnZeroWhenTravelSegNotFoundInFareMarket);
  CPPUNIT_TEST(testGetFlowAvailReturnZeroWhenTravelSegNotFound);
  CPPUNIT_TEST(testGetFlowAvailReturnZeroWhenTravelSegNotFoundAndArunkInFareMarket);

  CPPUNIT_TEST(testIsHistorical_TYO_0955);
  CPPUNIT_SKIP_TEST(testIsHistorical_TYO_1020);

  CPPUNIT_TEST(validateCheckConfirmedStatusNotPermitted_Permited);
  CPPUNIT_TEST(validateCheckConfirmedStatusNotPermitted_NotConfirmed);
  CPPUNIT_TEST(validateCheckConfirmedStatusNotPermitted_Confirmed_SameDay);
  CPPUNIT_TEST(validateCheckConfirmedStatusNotPermitted_Confirmed_DiffDay);

  CPPUNIT_TEST(testDeleteTvlSegIgnoreCat5_true_ifexist_KeepFare_ignoreCat5);
  CPPUNIT_TEST(testDeleteTvlSegIgnoreCat5_false_ifnoexist_KeepFare_ignoreCat5);

  CPPUNIT_TEST(testGetPrevExchangeOrOriginTktIssueDT_NotReissueExchange);
  CPPUNIT_TEST(testGetPrevExchangeOrOriginTktIssueDT_NoPreviousExchangeDT_IsSet);
  CPPUNIT_TEST(
      testGetPrevExchangeOrOriginTktIssueDT_NotReissueExchangeSet_PreviousExchangeDT_IsSet);
  CPPUNIT_TEST(testGetPrevExchangeOrOriginTktIssueDT_AllSet);

  CPPUNIT_TEST(testDetermineTicketDateForRex_NotForPricingUnit);
  CPPUNIT_TEST(testDetermineTicketDateForRex_NotForPricingUnit_NewTicketIssueDate);
  CPPUNIT_TEST(testDetermineTicketDateForRex_ApplyReissueExchange);
  CPPUNIT_TEST(testDetermineTicketDateForRex_PreviousExchangeDT);
  CPPUNIT_TEST(testDetermineTicketDateForRex_NotForPricingUnit_NewTicketIssueDate);
  CPPUNIT_TEST(testDetermineTicketDateForRex_ApplyReissueExchange_NewTicketIssueDate);
  CPPUNIT_SKIP_TEST(testCalculatUTCBetweenAgentLocationAndDepartureCity);
  CPPUNIT_TEST(testIsFlownSectorForNewItinPass);
  CPPUNIT_TEST(testIsFlownSectorForNewItinFail);
  CPPUNIT_TEST(testCat31BookingDateDeterminationYes);
  CPPUNIT_TEST(testCat31BookingDateDeterminationNo);
  CPPUNIT_TEST(testGetLimitDateTimeWhenTKTMustbeIssueTheHourReservationIsDone);
  CPPUNIT_TEST(testValidateFareMarketRtwFailBeforeTurnaround);
  CPPUNIT_TEST(testValidateFareMarketRtwFailTurnaround);
  CPPUNIT_TEST(testValidateFareMarketRtwPass);
  CPPUNIT_TEST(testValidateFareMarketRtwPassArunkBeforeTurnaround);
  CPPUNIT_TEST(testValidateFareMarketRtwFailRequireAllSegsConf);

  CPPUNIT_TEST_SUITE_END();

private:
  AdvanceResTkt* _advanceResTkt;
  AdvResTktInfo* _advResTktInfo;
  FareMarket* _fm;
  PaxTypeFare* _paxTypeFare;
  PricingTrx* _pricingTrx;
  Itin* _itin;
  PricingOptions _options;
  PricingRequest _request;
  PricingUnit* _pricingUnit;
  DiagManager* _diagManager;
  CarrierPreference _carrierPref;
  ClassOfService _cos;
  TestMemHandle _memHandle;
  Agent _agent;
  AirSeg* _aSeg;
  DateTime _ticketDT;
  DateTime _departureDT;

  void resetRule(AdvResTktInfo& advResTktInfo)
  {
    advResTktInfo.vendor() = "ATP";
    advResTktInfo.unavailtag() = ' ';
    advResTktInfo.permitted() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.lastResTod() = 0;
    advResTktInfo.ticketed() = ' ';
    advResTktInfo.standby() = ' ';
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.confirmedSector() = ' ';
    advResTktInfo.geoTblItemNo() = 0;

    advResTktInfo.tktTSI() = 0;
    advResTktInfo.advTktExcpUnit() = ' ';
    advResTktInfo.advTktUnit() = "";
    advResTktInfo.advTktPeriod() = "";
    advResTktInfo.advTktTod() = 0;
    advResTktInfo.advTktBoth() = AdvanceResTkt::notApply;
    advResTktInfo.advTktOpt() = AdvanceResTkt::notApply;
    advResTktInfo.advTktdepart() = 0;
    advResTktInfo.advTktDepartUnit() = ' ';
  }

  void validateAdvanceTktTime_setup()
  {
    _advanceResTkt->_advTktUnit = AdvanceResTkt::MONTH_UNIT_INDICATOR;
    _advanceResTkt->_ignoreTktAfterResRestriction = true;
    _advanceResTkt->_advTktOpt = AdvanceResTkt::earlierTime;
    _advanceResTkt->_ignoreTktDeforeDeptRestriction = false;
    _advanceResTkt->_advTktDepart = 1;
    _advanceResTkt->_advTktDepartUnit = AdvanceResTkt::MONTH_UNIT_INDICATOR;
    _advanceResTkt->_advTktBoth = AdvanceResTkt::notApply;
    _advanceResTkt->_tktToDateOverride = DateTime(2003, 12, 19, 23, 59, 0);
    _advanceResTkt->_diagFromToDate = true;

    _advResTktInfo = _memHandle.create<AdvResTktInfo>();
    _advResTktInfo->waiverTktDate() = DateTime(2003, 12, 18, 23, 59, 0);
    _advResTktInfo->advTktExcpUnit() = AdvanceResTkt::notApply;
    _advResTktInfo->tktTSI() = 1;

    pricingTrxSetUp();

    _pricingTrx->diagnostic().diagnosticType() = Diagnostic305;
    _pricingTrx->diagnostic().activate();

    _diagManager = _memHandle(new DiagManager(*_pricingTrx, Diagnostic305));
  }

  void pricingTrxSetUp()
  {
    _pricingTrx = _memHandle.create<PricingTrx>();
    _options.journeyActivatedForPricing() = true;
    _options.applyJourneyLogic() = true;
    _pricingTrx->setOptions(&_options);
    _request.lowFareRequested() = 'T'; // WPNC entry
    _pricingTrx->setRequest(&_request);
    _carrierPref.localMktJourneyType() = YES; // local journey carrier
    _request.ticketingAgent() = &_agent;
    _agent.agentLocation() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");
  }

  void prepareItinAndFareMarkets()
  {
    AirSeg* a1 = _memHandle.create<AirSeg>();
    AirSeg* a2 = _memHandle.create<AirSeg>();
    AirSeg* a3 = _memHandle.create<AirSeg>();
    a1->carrier() = a2->carrier() = a3->carrier() = "CO";
    a1->setBookingCode("Y");
    a2->setBookingCode("Y");
    a3->setBookingCode("Y");

    _itin = _memHandle.create<Itin>();
    _itin->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3;

    FareMarket* fm1 = _memHandle.create<FareMarket>();
    fm1->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3;

    FareMarket* fm2 = _memHandle.create<FareMarket>();
    fm2->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2;
    fm2->setFlowMarket(true);

    FareMarket* fm3 = _memHandle.create<FareMarket>();
    fm3->travelSeg() += (TravelSeg*)a2, (TravelSeg*)a3;
    fm3->setFlowMarket(true);

    FareMarket* fm4 = _memHandle.create<FareMarket>();
    fm4->travelSeg() += (TravelSeg*)a1;

    FareMarket* fm5 = _memHandle.create<FareMarket>();
    fm5->travelSeg() += (TravelSeg*)a2;

    _itin->fareMarket() += fm1, fm2, fm3, fm4, fm5;
    _itin->flowFareMarket() += fm2, fm3;
    _pricingTrx->itin().push_back(_itin);
  }

  void populateBookingCodeYinCosVec(std::vector<ClassOfService*>& cosVec)
  {
    _cos.bookingCode() = "Y";
    _cos.numSeats() = 7;
    cosVec += &_cos;
    _request.wpas() = 'T'; // Just to make sure that PaxTypeUtil::numSeatsForFare() return zero
  }

  PaxTypeFare* preparePaxTypeFare()
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(_memHandle.create<Fare>());
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->fareClass() = "12HENS";
    fi->vendor() = "ATP";
    ptf->fare()->setFareInfo(fi);
    return ptf;
  }

  FareUsage* prepareFareUsage(FareMarket* fm)
  {
    PaxTypeFare* ptf = preparePaxTypeFare();
    ptf->segmentStatus().resize(1);
    ptf->fareMarket() = fm;
    std::vector<ClassOfService*>* cosVec = _memHandle.create<std::vector<ClassOfService*> >();
    populateBookingCodeYinCosVec(*cosVec);
    ptf->fareMarket()->classOfServiceVec() += cosVec;
    FareUsage* fu = _memHandle.create<FareUsage>();
    fu->paxTypeFare() = ptf;
    fu->travelSeg().insert(fu->travelSeg().end(), fm->travelSeg().begin(), fm->travelSeg().end());
    return fu;
  }

  void commonRTWPreparation()
  {
    // Clear Any stored segments before running test
    TestAirSegFactory::clearCache();
    _pricingTrx = _memHandle.create<PricingTrx>();
    _pricingTrx->setOptions(_memHandle.create<PricingOptions>());
    _pricingTrx->diagnostic().diagnosticType() = Diagnostic305;
    _pricingTrx->diagnostic().activate();

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegHKGJNB.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegJNBSYD.xml");
    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegSYDLAX.xml");
    AirSeg* airSeg3 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAXFRA.xml");
    AirSeg* airSeg4 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegFRAPEK.xml");
    _fm = _memHandle.create<FareMarket>();
    _fm->travelSeg().push_back(airSeg0);
    _fm->travelSeg().push_back(airSeg1);
    _fm->travelSeg().push_back(airSeg2);
    _fm->travelSeg().push_back(airSeg3);
    _fm->travelSeg().push_back(airSeg4);
    _fm->direction() = FMDirection::OUTBOUND;
    _fm->origin() = airSeg0->origin();
    _fm->destination() = airSeg4->destination();
    _paxTypeFare = preparePaxTypeFare();
    _paxTypeFare->fareMarket() = _fm;

    // Create the itin
    _itin = _memHandle.create<Itin>();

    _itin->travelSeg().push_back(airSeg0);
    _itin->travelSeg().push_back(airSeg1);
    _itin->travelSeg().push_back(airSeg2);
    _itin->travelSeg().push_back(airSeg3);
    _itin->travelSeg().push_back(airSeg4);

    // RTW turnaround point
    _itin->furthestPointSegmentOrder() = 3;
    airSeg2->furthestPoint() = true;

    _pricingTrx->itin().push_back(_itin);

    PricingRequest* request = _memHandle.create<PricingRequest>();
    request->ticketingDT() = airSeg0->departureDT() - TimeDuration(73, 0, 0);
    _pricingTrx->setRequest(request);
    request->ticketingAgent() = &_agent;
    _agent.agentLocation() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");

    // RTW requests
    _pricingTrx->getOptions()->setRtw(true);

    PaxTypeFare ptf;
    _advanceResTkt->setRuleDataAccess(
        _memHandle(new RuleControllerDataAccessMock(ptf, _pricingTrx, _itin)));

    _advResTktInfo = _memHandle.create<AdvResTktInfo>();
    resetRule(*_advResTktInfo);

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(73, 0, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();
    airSeg2->bookingDT() = airSeg0->bookingDT();
    airSeg3->bookingDT() = airSeg0->bookingDT();
    airSeg4->bookingDT() = airSeg0->bookingDT();
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _advanceResTkt = _memHandle.create<AdvanceResTkt>();
  }

  void tearDown() { _memHandle.clear(); }

  void testCreate() { CPPUNIT_ASSERT(_advanceResTkt->_newTicketIssueDate.isEmptyDate()); }

  void testInitialize()
  {
    _advanceResTkt->_newTicketIssueDate = DateTime(2010, 10, 10);
    _advanceResTkt->initialize(*_memHandle(new PricingTrx),
                               *_memHandle(new AdvResTktInfo),
                               *_memHandle(new PaxTypeFare),
                               0,
                               0);
    CPPUNIT_ASSERT(_advanceResTkt->_newTicketIssueDate.isEmptyDate());
  }

  void setupValidateTestData()
  {
    // Clear Any stored segments before running test
    TestAirSegFactory::clearCache();
    _pricingTrx = _memHandle.create<PricingTrx>();
    _pricingTrx->setOptions(&_options);
    _pricingTrx->diagnostic().diagnosticType() = Diagnostic305;
    _pricingTrx->diagnostic().activate();
    _pricingTrx->setRequest(&_request);
    _request.ticketingAgent() = &_agent;
    _agent.agentLocation() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");
  }

  void testValidateFareMarketShouldPassFareMarketCoverItin()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();
    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    //--------------------------------------------------------
    // Should PASS, Although a Cat5 for PU, but we have FareMarket cover
    // itin travel segments
    fm->direction() = FMDirection::OUTBOUND;
    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == PASS);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  // if no ticketing restriction, we have PASS , otherwise SOFTPASS
  void testValidateFareMarketShouldPassEachSectorXOrigMatchFMAndItinNoTktRest()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;
    //--------------------------------------------------------
    // Must reserve 3 days before departure
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.lastResUnit() = "Dd";
    advResTktInfo.lastResPeriod() = "003";

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(96, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();
    // made reservation 96 hours 5 minutes before departure of PU orig,
    // Because orig of FM is same of orig of itin, it should be orig of PU as
    // well, eachSector is 'X' so we only care about this FC
    // if no ticketing restriction, we have PASS , otherwise SOFTPASS

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == PASS);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldFailOrigFMSameOrigItin()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(96, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();
    // made reservation 96 hours 5 minutes before departure of PU orig,
    // Because orig of FM is same of orig of itin, it should be orig of PU as
    // well, eachSector is 'X' so we only care about this FC
    // if no ticketing restriction, we have PASS , otherwise SOFTPASS

    //--------------------------------------------------------
    // Reserve allowed after 3 days before departure
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.firstResUnit() = "Dd";
    advResTktInfo.firstResPeriod() = "003";

    // Should FAIL
    // Because orig of FM is same of orig of itin, it should be orig of PU as
    // well
    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == FAIL);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldSoftPassReservedAllowed()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    // Should SOFTPASS
    // Because orig of FM is not orig of itin, it may not be orig of PU
    FareMarket* fm = _memHandle.create<FareMarket>();
    CPPUNIT_ASSERT(fm != 0);
    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg1->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(96, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();

    // made reservation 96 hours 5 minutes before departure of PU orig,
    // Because orig of FM is same of orig of itin, it should be orig of PU as
    // well, eachSector is 'X' so we only care about this FC
    // if no ticketing restriction, we have PASS , otherwise SOFTPASS
    //--------------------------------------------------------
    // Reserve allowed after 3 days before departure
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.firstResUnit() = "Dd";
    advResTktInfo.firstResPeriod() = "003";

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == SOFTPASS);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldSoftPassTSIEqual1()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(96, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();
    // made reservation 96 hours 5 minutes before departure of PU orig,
    // Because orig of FM is same of orig of itin, it should be orig of PU as
    // well, eachSector is 'X' so we only care about this FC
    // if no ticketing restriction, we have PASS , otherwise SOFTPASS

    //--------------------------------------------------------
    // Reserve allowed after 3 days before departure
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.firstResUnit() = "Dd";
    advResTktInfo.firstResPeriod() = "003";

    //--------------------------------------------------------
    // Should SOFTPASS
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.resTSI() = 1; // origin of Pricing Unit

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == SOFTPASS);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldFailOnTicketing()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // Reservation made before third SUN in front FC orig departure
    // Ticketing must be made 24 hours after reservation
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "Hh";
    advResTktInfo.advTktPeriod() = "24";

    // should fail on ticketing
    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();
    _request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(28, 5, 0);

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == FAIL);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldPassAtPU()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // Reservation made before third SUN in front FC orig departure
    // Ticketing must be made 24 hours after reservation
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "Hh";
    advResTktInfo.advTktPeriod() = "24";

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();
    _request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(28, 5, 0);

    // should softpass (Pass at PU)
    advResTktInfo.confirmedSector() = AdvanceResTkt::allSector; // out/inbound
    _request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(2, 15, 0);

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == SOFTPASS);
    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldFailOnTicketing15MinAdvTicketing()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // Reservation made before third SUN in front FC orig departure
    // Ticketing must be made 24 hours after reservation
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    // 15minutes after reservation
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "Nn";
    advResTktInfo.advTktPeriod() = "015";

    advResTktInfo.confirmedSector() = AdvanceResTkt::allSector; // out/inbound

    // should fail on ticketing
    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();
    _request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(0, 35, 0);

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == FAIL);
    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldSoftPassAtPU()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // Reservation made before third SUN in front FC orig departure
    // Ticketing must be made 24 hours after reservation
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    // 15minutes after reservation
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "Nn";
    advResTktInfo.advTktPeriod() = "015";

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();

    // should softpass (Pass at PU)
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.confirmedSector() = AdvanceResTkt::allSector; // out/inbound
    _request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(0, 12, 0);

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == SOFTPASS);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldFailOrigOfFMAndItinNotEqualPU()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // First time ticketing can be made 3 days before departure
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "";
    advResTktInfo.advTktPeriod() = "";
    advResTktInfo.advTktOpt() = AdvanceResTkt::firstTimePermit;
    advResTktInfo.advTktdepart() = 3;
    advResTktInfo.advTktDepartUnit() = 'D';
    advResTktInfo.confirmedSector() = AdvanceResTkt::allSector; // out/inbound

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();

    _request.ticketingDT() = airSeg0->departureDT() - TimeDuration(240, 5, 0);

    // Should FAIL
    // Because orig of FM is same of orig of itin, it should be orig of PU as
    // well
    paxTypeFare->fareMarket() = fm;
    advResTkt.initialize(*_pricingTrx, advResTktInfo, *paxTypeFare, 0, &itin);

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == FAIL);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldSoftPassOrginFMNotOrginItin()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // First time ticketing can be made 3 days before departure
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "";
    advResTktInfo.advTktPeriod() = "";
    advResTktInfo.advTktOpt() = AdvanceResTkt::firstTimePermit;
    advResTktInfo.advTktdepart() = 3;
    advResTktInfo.advTktDepartUnit() = 'D';
    advResTktInfo.confirmedSector() = AdvanceResTkt::allSector; // out/inbound

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();

    _request.ticketingDT() = airSeg0->departureDT() - TimeDuration(240, 5, 0);

    // Should FAIL
    // Because orig of FM is same of orig of itin, it should be orig of PU as
    // well
    paxTypeFare->fareMarket() = fm;
    advResTkt.initialize(*_pricingTrx, advResTktInfo, *paxTypeFare, 0, &itin);

    FareMarket* fm2 = _memHandle.create<FareMarket>();
    CPPUNIT_ASSERT(fm2 != 0);

    fm2->travelSeg().push_back(airSeg1);
    fm2->origin() = airSeg1->origin();
    fm2->destination() = airSeg1->destination();

    // Should SOFTPASS
    // Because orig of FM2 is not orig of itin, it may not be orig of PU
    paxTypeFare->fareMarket() = fm2;
    advResTkt.initialize(*_pricingTrx, advResTktInfo, *paxTypeFare, 0, &itin);

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm2));
    CPPUNIT_ASSERT(ret == SOFTPASS);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldFailTktTSIEqual3()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // First time ticketing can be made 3 days before departure
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "";
    advResTktInfo.advTktPeriod() = "";
    advResTktInfo.advTktOpt() = AdvanceResTkt::firstTimePermit;
    advResTktInfo.advTktdepart() = 3;
    advResTktInfo.advTktDepartUnit() = 'D';
    advResTktInfo.confirmedSector() = AdvanceResTkt::allSector; // out/inbound

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();

    _request.ticketingDT() = airSeg0->departureDT() - TimeDuration(240, 5, 0);

    paxTypeFare->fareMarket() = fm;
    advResTkt.initialize(*_pricingTrx, advResTktInfo, *paxTypeFare, 0, &itin);

    FareMarket* fm2 = _memHandle.create<FareMarket>();
    CPPUNIT_ASSERT(fm2 != 0);

    fm2->travelSeg().push_back(airSeg1);
    fm2->origin() = airSeg1->origin();
    fm2->destination() = airSeg1->destination();

    paxTypeFare->fareMarket() = fm2;
    advResTkt.initialize(*_pricingTrx, advResTktInfo, *paxTypeFare, 0, &itin);

    // Fare Component Scope, should fail
    advResTktInfo.tktTSI() = 3;

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm2));
    CPPUNIT_ASSERT(ret == FAIL);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldSoftPassRevalidOfPUForLastTktDate()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // First time ticketing can be made 3 days before departure
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "";
    advResTktInfo.advTktPeriod() = "";
    advResTktInfo.advTktOpt() = AdvanceResTkt::firstTimePermit;
    advResTktInfo.advTktdepart() = 3;
    advResTktInfo.advTktDepartUnit() = 'D';
    advResTktInfo.confirmedSector() = AdvanceResTkt::allSector; // out/inbound

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();

    paxTypeFare->fareMarket() = fm;
    advResTkt.initialize(*_pricingTrx, advResTktInfo, *paxTypeFare, 0, &itin);

    // Fare Component Scope, should fail
    advResTktInfo.tktTSI() = 3;

    // should not fail
    _request.ticketingDT() = airSeg0->departureDT() - TimeDuration(24, 5, 0);

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == SOFTPASS); // SOFTPASS because revalid at PU for lastTktDT

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketShouldSoftPassTktTSIEqual0()
  {
    setupValidateTestData();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector* diag = factory->create(*_pricingTrx);
    diag->enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");

    PaxTypeFare* paxTypeFare = preparePaxTypeFare();

    FareMarket* fm = _memHandle.create<FareMarket>();

    fm->direction() = FMDirection::OUTBOUND;
    fm->travelSeg().push_back(airSeg0);
    fm->travelSeg().push_back(airSeg1);
    fm->origin() = airSeg0->origin();
    fm->destination() = airSeg1->destination();
    paxTypeFare->fareMarket() = fm;
    _pricingTrx->fareMarket().push_back(fm);
    _pricingTrx->getRequest()->ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    _pricingTrx->itin().push_back(&itin);

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    resetRule(advResTktInfo);

    airSeg0->resStatus() = CONFIRM_RES_STATUS;
    airSeg1->resStatus() = CONFIRM_RES_STATUS;

    fm->direction() = FMDirection::OUTBOUND;

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // First time ticketing can be made 3 days before departure
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "";
    advResTktInfo.advTktPeriod() = "";
    advResTktInfo.advTktOpt() = AdvanceResTkt::firstTimePermit;
    advResTktInfo.advTktdepart() = 3;
    advResTktInfo.advTktDepartUnit() = 'D';
    advResTktInfo.confirmedSector() = AdvanceResTkt::allSector; // out/inbound

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();

    paxTypeFare->fareMarket() = fm;
    advResTkt.initialize(*_pricingTrx, advResTktInfo, *paxTypeFare, 0, &itin);

    _request.ticketingDT() = airSeg0->departureDT() - TimeDuration(24, 5, 0);

    // should SOFTPASS
    advResTktInfo.tktTSI() = 0;

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = advResTkt.validate(*_pricingTrx, itin, *paxTypeFare, &advResTktInfo, *fm));
    CPPUNIT_ASSERT(ret == SOFTPASS);

    diag->flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidate_FailOnInvalidRulInfo()
  {
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);
    Itin itin;
    PaxTypeFare paxTypeFare;
    AdvanceResTkt advResTkt;
    // pass in a non AdvResTktInfo object
    BlackoutInfo ruleItemInfo;
    FareMarket fm;

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, itin, paxTypeFare, &ruleItemInfo, fm));
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_SoftpassAR_EXC_TRX_NewItinPhase()
  {
    RexPricingTrx trx;
    trx.setRequest(&_request);
    _request.ticketingAgent() = &_agent;
    _agent.agentLocation() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");
    trx.trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;
    Itin itin;
    PaxTypeFare paxTypeFare;
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;
    FareMarket fm;

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, itin, paxTypeFare, &advResTktInfo, fm));
    CPPUNIT_ASSERT_EQUAL(SOFTPASS, ret);
  }

  void verifyDiagnostic(const std::string& diagnostic)
  {
    // cout << diagnostic << endl;
    CPPUNIT_ASSERT(!diagnostic.empty());
  }

  class RuleControllerDataAccessMock : public RuleControllerDataAccess
  {
  public:
    RuleControllerDataAccessMock(PaxTypeFare& ptf, PricingTrx* trx, Itin* itin) : _ptf(ptf), _trx(trx), _itin(itin) {}
    virtual Itin* itin() { return _itin; }
    virtual PaxTypeFare& paxTypeFare() const { return _ptf; }
    virtual PricingTrx& trx() { return *_trx; }

  protected:
    PricingTrx* _trx;
    Itin* _itin;
    PaxTypeFare& _ptf;
  };

  void testPU()
  {
    // Clear Any stored segments before running test
    TestAirSegFactory::clearCache();
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);
    trx.diagnostic().diagnosticType() = Diagnostic305;
    trx.diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    FareMarket* fm0 = _memHandle.create<FareMarket>();
    fm0->travelSeg().push_back(airSeg0);
    fm0->direction() = FMDirection::OUTBOUND;
    fm0->origin() = airSeg0->origin();
    fm0->destination() = airSeg0->destination();
    PaxTypeFare* paxTypeFare0 = preparePaxTypeFare();
    paxTypeFare0->fareMarket() = fm0;
    FareUsage* fu0 = _memHandle.create<FareUsage>();
    fu0->paxTypeFare() = paxTypeFare0;
    fu0->inbound() = false;

    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    fm1->travelSeg().push_back(airSeg1);
    fm1->direction() = FMDirection::INBOUND;
    fm1->origin() = airSeg1->origin();
    fm1->destination() = airSeg1->destination();
    PaxTypeFare* paxTypeFare1 = preparePaxTypeFare();
    paxTypeFare1->fareMarket() = fm1;
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    fu1->paxTypeFare() = paxTypeFare1;
    fu1->inbound() = true;

    CPPUNIT_ASSERT(fu0 != 0);
    CPPUNIT_ASSERT(fu1 != 0);

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->fareUsage().push_back(fu0);
    pu->fareUsage().push_back(fu1);
    pu->travelSeg().push_back(airSeg0);
    pu->travelSeg().push_back(airSeg1);
    pu->turnAroundPoint() = airSeg1;

    // We need FarePath and PricingUnit for GeoItemTbl validation, although
    // we do not declaim to support GeoItemTbl on accompanied travel yet
    FarePath* fp0 = _memHandle.create<FarePath>();
    CPPUNIT_ASSERT(fp0 != 0);
    fp0->pricingUnit().push_back(pu);

    // Create the itin
    PricingRequest request;
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);

    trx.itin().push_back(&itin);

    request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(72, 5, 0);
    trx.setRequest(&request);
    request.ticketingAgent() = &_agent;
    _agent.agentLocation() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    PaxTypeFare ptf;
    advResTkt.setRuleDataAccess(_memHandle(new RuleControllerDataAccessMock(ptf, &trx, &itin)));

    resetRule(advResTktInfo);

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(72, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();

    //--------------------------------------------------------
    // Should PASS,
    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == PASS);

    //--------------------------------------------------------
    // Should PASS
    // Must reserve 3 days before departure
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.lastResUnit() = "Dd";
    advResTktInfo.lastResPeriod() = "003";

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(96, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();
    // made reservation 72 hours 5 minutes before departure

    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == PASS);

    //--------------------------------------------------------
    // Should FAIL
    // Reserve allowed after 3 days before departure
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.firstResUnit() = "Dd";
    advResTktInfo.firstResPeriod() = "003";

    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == FAIL);

    //--------------------------------------------------------
    // Should PASS
    // advResTktInfo.eachSector() = ' ';
    // advResTktInfo.resTSI() = 1; // origin of Pricing Unit

    // CPPUNIT_ASSERT_NO_THROW( ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0) );
    // CPPUNIT_ASSERT(ret == PASS);

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // Reservation made before third SUN in front FC orig departure
    // Ticketing must be made 24 hours after reservation
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "Hh";
    advResTktInfo.advTktPeriod() = "24";

    // should fail on ticketing
    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(480, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();
    request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(28, 5, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == FAIL);

    // should Pass at PU
    request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(2, 15, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == PASS);

    //--------------------------------------------------------
    // First time ticketing can be made 3 days before departure
    advResTktInfo.advTktUnit() = "";
    advResTktInfo.advTktPeriod() = "";
    advResTktInfo.advTktOpt() = AdvanceResTkt::firstTimePermit;
    advResTktInfo.advTktdepart() = 3;
    advResTktInfo.advTktDepartUnit() = 'D';

    // should fail
    request.ticketingDT() = airSeg0->departureDT() - TimeDuration(240, 5, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == FAIL);

    // should PASS
    request.ticketingDT() = airSeg0->departureDT() - TimeDuration(24, 5, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == PASS);

    // If Reservation made 20 days before departure, ticketing must be made
    // 24 hours after reservation
    resetRule(advResTktInfo);
    advResTktInfo.advTktExcpUnit() = 'D';
    advResTktInfo.advTktExcpTime() = 20;
    advResTktInfo.advTktUnit() = "Hh";
    advResTktInfo.advTktPeriod() = "24";

    // should SKIP, ticketing restriction not apply
    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(12 * 24, 5, 0);
    request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(48, 0, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == SKIP);

    // should fail, when ticketing restriction apply
    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(22 * 24, 5, 0);
    request.ticketingDT() = airSeg0->bookingDT() + TimeDuration(48, 0, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == STOP);

    // Byte 35 (EACH) should not effect measure ticket restriction from
    // departure of PU
    resetRule(advResTktInfo);
    advResTktInfo.advTktOpt() = 'R';
    advResTktInfo.advTktdepart() = 7;
    advResTktInfo.advTktDepartUnit() = 'D';
    advResTktInfo.eachSector() = 'X';
    advResTkt.initialize(trx, advResTktInfo, *fu1->paxTypeFare(), pu, &itin);

    request.ticketingDT() = airSeg0->departureDT() - TimeDuration(2 * 24, 5, 0);
    airSeg1->departureDT() = request.ticketingDT() + TimeDuration(8 * 24, 5, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu1));
    CPPUNIT_ASSERT(ret == STOP);

    request.ticketingDT() = airSeg0->departureDT() - TimeDuration(7 * 24, 5, 0);
    airSeg1->departureDT() = request.ticketingDT() + TimeDuration(8 * 24, 5, 0);
    advResTkt.initialize(trx, advResTktInfo, *fu1->paxTypeFare(), pu, &itin);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu1));
    CPPUNIT_ASSERT(ret == PASS);

    //------------------------------------------------------
    // OpenSegment
    //------------------------------------------------------

    diag.flushMsg();
    verifyDiagnostic(trx.diagnostic().toString());
  }

  void testWPNC()
  {
    // Clear Any stored segments before running test
    TestAirSegFactory::clearCache();
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);
    trx.diagnostic().diagnosticType() = Diagnostic305;
    trx.diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic305);

    AirSeg* airSeg0 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegDFWJFK.xml");
    airSeg0->setBookingCode("V");

    FareMarket* fm0 = _memHandle.create<FareMarket>();
    fm0->travelSeg().push_back(airSeg0);
    fm0->direction() = FMDirection::OUTBOUND;
    fm0->origin() = airSeg0->origin();
    fm0->destination() = airSeg0->destination();
    PaxTypeFare* paxTypeFare0 = preparePaxTypeFare();
    paxTypeFare0->fareMarket() = fm0;
    FareUsage* fu0 = _memHandle.create<FareUsage>();
    fu0->paxTypeFare() = paxTypeFare0;
    fu0->inbound() = false;

    AirSeg* airSeg1 =
        TestAirSegFactory::create("/vobs/atseintl/Rules/test/AdvResTktData/AirSegJFKDFW.xml");
    airSeg1->setBookingCode("V");

    FareMarket* fm1 = _memHandle.create<FareMarket>();
    fm1->travelSeg().push_back(airSeg1);
    fm1->direction() = FMDirection::INBOUND;
    fm1->origin() = airSeg1->origin();
    fm1->destination() = airSeg1->destination();
    PaxTypeFare* paxTypeFare1 = preparePaxTypeFare();
    paxTypeFare1->fareMarket() = fm1;
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    fu1->paxTypeFare() = paxTypeFare1;
    fu1->inbound() = true;

    CPPUNIT_ASSERT(fu0 != 0);
    CPPUNIT_ASSERT(fu1 != 0);

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    pu->fareUsage().push_back(fu0);
    pu->fareUsage().push_back(fu1);
    pu->travelSeg().push_back(airSeg0);
    pu->travelSeg().push_back(airSeg1);
    pu->turnAroundPoint() = airSeg1;

    // We need FarePath and PricingUnit for GeoItemTbl validation
    FarePath* fp0 = _memHandle.create<FarePath>();
    CPPUNIT_ASSERT(fp0 != 0);
    fp0->pricingUnit().push_back(pu);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);

    trx.itin().push_back(&itin);

    // WPNC request
    PricingRequest request;
    request.lowFareNoAvailability() = 'T';
    request.ticketingDT() = DateTime::localTime();
    airSeg0->bookingDT() = request.ticketingDT() - TimeDuration(72, 5, 0);
    trx.setRequest(&request);
    request.ticketingAgent() = &_agent;
    _agent.agentLocation() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");

    // Rule Item Info
    AdvanceResTkt advResTkt;
    AdvResTktInfo advResTktInfo;

    PaxTypeFare ptf;
    advResTkt.setRuleDataAccess(_memHandle(new RuleControllerDataAccessMock(ptf, &trx, &itin)));

    advResTktInfo.vendor() = "ATP";
    advResTktInfo.unavailtag() = ' ';
    // advResTktInfo.waiverResDate()
    // advResTktInfo.waiverTktDate()
    advResTktInfo.permitted() = ' ';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.lastResTod() = 0;
    advResTktInfo.ticketed() = ' ';
    advResTktInfo.standby() = ' ';
    advResTktInfo.eachSector() = ' ';
    advResTktInfo.confirmedSector() = ' ';
    advResTktInfo.geoTblItemNo() = 0;

    advResTktInfo.tktTSI() = 0;
    advResTktInfo.advTktExcpUnit() = ' ';
    advResTktInfo.advTktExcpTime() = 0;
    advResTktInfo.advTktUnit() = "";
    advResTktInfo.advTktPeriod() = "";
    advResTktInfo.advTktTod() = 0;
    advResTktInfo.advTktBoth() = AdvanceResTkt::notApply;
    advResTktInfo.advTktOpt() = AdvanceResTkt::notApply;
    advResTktInfo.advTktdepart() = 0;
    advResTktInfo.advTktDepartUnit() = ' ';

    airSeg0->bookingDT() = airSeg0->departureDT() - TimeDuration(72, 5, 0);
    airSeg1->bookingDT() = airSeg0->bookingDT();

    // Initialize booking flags
    FareBookingCodeValidator fbcv0(trx, *fm0, &itin);
    fbcv0.SetBkgSegmStatus(*paxTypeFare0, FareBookingCodeValidator::INIT);

    FareBookingCodeValidator fbcv1(trx, *fm1, &itin);
    fbcv1.SetBkgSegmStatus(*paxTypeFare1, FareBookingCodeValidator::INIT);

    if (fu0->segmentStatus().empty())
      fu0->segmentStatus().insert(fu0->segmentStatus().end(),
                                  fu0->paxTypeFare()->segmentStatus().begin(),
                                  fu0->paxTypeFare()->segmentStatus().end());

    //--------------------------------------------------------
    // Should PASS with WPNC
    // Ticketing 24 hours after reserve
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.advTktUnit() = "Hh";
    advResTktInfo.advTktPeriod() = "24";

    Record3ReturnTypes ret = PASS;

    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == PASS);
    // verify that paxTypeFare0 should be set flag as rebooked
    CPPUNIT_ASSERT(fu0->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
    CPPUNIT_ASSERT(fu0->segmentStatus()[0]._bkgCodeReBook == "V");

    // Now we already have Rebook set by BookingCodeValidation
    // We should pass with nothing should be changed about rebook
    fu0->segmentStatus()[0]._bkgCodeReBook = "K";
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == PASS);
    // verify that paxTypeFare0 should be set flag as rebooked
    CPPUNIT_ASSERT(fu0->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
    CPPUNIT_ASSERT(fu0->segmentStatus()[0]._bkgCodeReBook == "K");

    // Now we apply restriction to OUTBOUND FareUsage while validating INBOUND
    // FareUsage, this should let Outbound marked as rebook needed
    advResTktInfo.eachSector() = ' ';
    paxTypeFare0->segmentStatus().clear();
    fbcv0.SetBkgSegmStatus(*paxTypeFare0, FareBookingCodeValidator::INIT);
    fu0->segmentStatus().clear();
    if (fu0->segmentStatus().empty())
      fu0->segmentStatus().insert(fu0->segmentStatus().end(),
                                  fu0->paxTypeFare()->segmentStatus().begin(),
                                  fu0->paxTypeFare()->segmentStatus().end());
    fu1->segmentStatus().clear();
    if (fu1->segmentStatus().empty())
      fu1->segmentStatus().insert(fu1->segmentStatus().end(),
                                  fu1->paxTypeFare()->segmentStatus().begin(),
                                  fu1->paxTypeFare()->segmentStatus().end());
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu1));
    CPPUNIT_ASSERT(ret == PASS);
    // verify that paxTypeFare0 should be set flag as rebooked
    CPPUNIT_ASSERT(fu0->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
    CPPUNIT_ASSERT(fu0->segmentStatus()[0]._bkgCodeReBook == "V");
    CPPUNIT_ASSERT(!fu1->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));

    /////////////////////////////////////////////////////////
    // Ticketing Restriction
    /////////////////////////////////////////////////////////
    //--------------------------------------------------------
    // Reservation made before third SUN in front FC orig departure
    // Ticketing must be made 24 hours after reservation
    advResTktInfo.eachSector() = 'X';
    advResTktInfo.resTSI() = 0;
    advResTktInfo.lastResUnit() = "3";
    advResTktInfo.lastResPeriod() = "SUN";
    advResTktInfo.lastResTod() = 720;
    advResTktInfo.firstResUnit() = "";
    advResTktInfo.firstResPeriod() = "";
    advResTktInfo.advTktUnit() = "Hh";
    advResTktInfo.advTktPeriod() = "24";

    paxTypeFare0->segmentStatus().clear();
    fbcv0.SetBkgSegmStatus(*paxTypeFare0, FareBookingCodeValidator::INIT);
    fu0->segmentStatus().clear();
    if (fu0->segmentStatus().empty())
      fu0->segmentStatus().insert(fu0->segmentStatus().end(),
                                  fu0->paxTypeFare()->segmentStatus().begin(),
                                  fu0->paxTypeFare()->segmentStatus().end());
    // should PASS on ticketing when validate on fu1, but only set paxTypeFare1 booking flag
    airSeg1->bookingDT() = request.ticketingDT() - TimeDuration(28, 5, 0);
    airSeg1->departureDT() = airSeg1->bookingDT() + TimeDuration(532, 5, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu1));
    // CPPUNIT_ASSERT(ret == PASS);
    // CPPUNIT_ASSERT( !fu0->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)
    // );
    // CPPUNIT_ASSERT( fu1->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)
    // );
    // CPPUNIT_ASSERT( fu1->segmentStatus()[0]._bkgCodeReBook == "V" );

    // no flag set
    paxTypeFare0->segmentStatus().clear();
    fbcv0.SetBkgSegmStatus(*paxTypeFare0, FareBookingCodeValidator::INIT);
    paxTypeFare1->segmentStatus().clear();
    fbcv1.SetBkgSegmStatus(*paxTypeFare1, FareBookingCodeValidator::INIT);
    fu0->segmentStatus().clear();
    if (fu0->segmentStatus().empty())
      fu0->segmentStatus().insert(fu0->segmentStatus().end(),
                                  fu0->paxTypeFare()->segmentStatus().begin(),
                                  fu0->paxTypeFare()->segmentStatus().end());
    fu1->segmentStatus().clear();
    if (fu1->segmentStatus().empty())
      fu1->segmentStatus().insert(fu1->segmentStatus().end(),
                                  fu1->paxTypeFare()->segmentStatus().begin(),
                                  fu1->paxTypeFare()->segmentStatus().end());

    // rebook would not help
    airSeg0->bookingDT() = request.ticketingDT() - TimeDuration(2, 15, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == FAIL);
    CPPUNIT_ASSERT(!fu0->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
    CPPUNIT_ASSERT(!fu1->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));

    //--------------------------------------------------------
    // First time ticketing can be made 3 days before departure
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.advTktUnit() = "";
    advResTktInfo.advTktPeriod() = "";
    advResTktInfo.advTktOpt() = AdvanceResTkt::firstTimePermit;
    advResTktInfo.advTktdepart() = 3;
    advResTktInfo.advTktDepartUnit() = 'D';

    // should fail, rebooking should not help
    airSeg0->departureDT() = request.ticketingDT() + TimeDuration(240, 5, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == FAIL);

    // should PASS
    airSeg0->departureDT() = request.ticketingDT() + TimeDuration(24, 5, 0);
    CPPUNIT_ASSERT_NO_THROW(ret = advResTkt.validate(trx, &advResTktInfo, *fp0, *pu, *fu0));
    CPPUNIT_ASSERT(ret == PASS);

    //------------------------------------------------------
    // OpenSegment
    // TODO
    //------------------------------------------------------

    diag.flushMsg();
    verifyDiagnostic(trx.diagnostic().toString());
  }

  void testValidateResTkt_RexAdjustTicketDT()
  {
    validateAdvanceTktTime_setup();

    // should be adjusted inside logic
    const DateTime tktDT = DateTime(2033, 12, 20, 23, 59, 0);
    // because of rex transaction
    createRexPricingTrx();

    bool whateverValue = false;

    Record3ReturnTypes r3rt = _advanceResTkt->validateResTkt(tktDT, // whatever
                                                             tktDT,
                                                             /*needResDTCheck = */ false,
                                                             /*needTktDTCheck = */ true,
                                                             *_advResTktInfo,
                                                             /*needResTktSameDayChk = */ false,
                                                             /*needResTktSameTimeChk = */ false,
                                                             tktDT, // whatever
                                                             tktDT, // whatever
                                                             tktDT, // whatever
                                                             _pricingUnit, // pu pointer not needed
                                                             *_diagManager,
                                                             whateverValue,
                                                             whateverValue,
                                                             whateverValue,
                                                             whateverValue,
                                                             tktDT,
                                                             *_pricingTrx); // whatever );//whatever

    // to ease diagnose of method flow
    _diagManager->collector().flushMsg();

    std::string diagOutput = _pricingTrx->diagnostic().toString();
    verifyDiagnostic(diagOutput);

    std::string adjustedDateStr = diagOutput.substr(15, 19);
    std::string expectedDateStr = "2003-12-19T23:59:00";

    CPPUNIT_ASSERT_EQUAL(expectedDateStr, adjustedDateStr);
    CPPUNIT_ASSERT_EQUAL(FAIL, r3rt);
  }

  void testValidateResTkt_DoNotAdjustTicketDT()
  {
    validateAdvanceTktTime_setup();

    // should not be adjusted inside logic
    const DateTime tktDT = DateTime(2033, 12, 20, 23, 59, 0);

    bool whateverValue = false;

    Record3ReturnTypes r3rt = _advanceResTkt->validateResTkt(tktDT, // whatever
                                                             tktDT,
                                                             /*needResDTCheck = */ false,
                                                             /*needTktDTCheck = */ true,
                                                             *_advResTktInfo,
                                                             /*needResTktSameDayChk = */ false,
                                                             /*needResTktSameTimeChk = */ false,
                                                             tktDT, // whatever
                                                             tktDT, // whatever
                                                             tktDT, // whatever
                                                             _pricingUnit, // pu pointer not needed
                                                             *_diagManager,
                                                             whateverValue,
                                                             whateverValue,
                                                             whateverValue,
                                                             whateverValue,
                                                             tktDT,
                                                             *_pricingTrx); // whatever

    // to ease diagnose of method flow
    _diagManager->collector().flushMsg();

    verifyDiagnostic(_pricingTrx->diagnostic().toString());

    std::string adjustedDateStr = _pricingTrx->diagnostic().toString().substr(15, 19);
    std::string expectedDateStr = "2033-12-20T23:59:00";

    CPPUNIT_ASSERT_EQUAL(expectedDateStr, adjustedDateStr);
    CPPUNIT_ASSERT_EQUAL(FAIL, r3rt);
  }

  void testValidateAdvanceTktTime()
  {
    validateAdvanceTktTime_setup();

    _pricingUnit = _memHandle.create<PricingUnit>();
    const DateTime ticketDT = DateTime(2003, 12, 19, 23, 59, 0);
    const DateTime departureDT = DateTime(2004, 01, 20, 23, 59, 0);
    bool mayPassAfterRebook = false;

    Record3ReturnTypes r3rt =
        _advanceResTkt->validateAdvanceTktTime(_pricingUnit, // pu pointer not needed
                                               ticketDT,
                                               ticketDT, // whatever
                                               departureDT,
                                               *_advResTktInfo,
                                               *_diagManager,
                                               mayPassAfterRebook);

    // to ease diagnose of method flow
    _diagManager->collector().flushMsg();
    verifyDiagnostic(_pricingTrx->diagnostic().toString());

    CPPUNIT_ASSERT_EQUAL(PASS, r3rt);
  }

  void testGetLimitDateTime()
  {
    validateAdvanceTktTime_setup();

    DateTime limitTimeReturn;
    const DateTime referenceDT = DateTime(2003, 12, 18, 23, 59, 0);
    const Indicator monthInd = AdvanceResTkt::MONTH_UNIT_INDICATOR;

    CPPUNIT_ASSERT(_advanceResTkt->getLimitDateTime(limitTimeReturn,
                                                    referenceDT,
                                                    300, // 1-1440
                                                    3, // period
                                                    monthInd,
                                                    AdvanceResTkt::BEFORE_REF_TIME,
                                                    false));

    // to ease diagnose of method flow
    _diagManager->collector().flushMsg();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _pricingTrx->diagnostic().toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(2003, 9, 18, 5, 0, 0), limitTimeReturn);
  }

  void testGetLimitDateTimeFailsWhenMonthIsOutOfRange()
  {
    DateTime limitTimeReturn;
    const DateTime referenceDT = DateTime(2003, 1, 30, 23, 59, 0);
    const Indicator monthInd = AdvanceResTkt::MONTH_UNIT_INDICATOR;

    CPPUNIT_ASSERT(_advanceResTkt->getLimitDateTime(limitTimeReturn,
                                                    referenceDT,
                                                    300,
                                                    11, // newMonth = 12
                                                    monthInd,
                                                    AdvanceResTkt::AFTER_REF_TIME,
                                                    false));
  }

  void testGetLimitDateTimeForMinutesWhenReferenceTimeIsNotIgnored()
  {
    validateAdvanceTktTime_setup();

    DateTime limitTimeReturn;
    const DateTime referenceDT = DateTime(2003, 12, 18, 23, 59, 0);
    const Indicator dayInd = AdvanceResTkt::MINUTE_UNIT_INDICATOR;

    CPPUNIT_ASSERT(_advanceResTkt->getLimitDateTime(limitTimeReturn,
                                                    referenceDT,
                                                    300, // 1-1440
                                                    3, // period
                                                    dayInd,
                                                    AdvanceResTkt::BEFORE_REF_TIME,
                                                    false,
                                                    true));

    // to ease diagnose of method flow
    _diagManager->collector().flushMsg();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _pricingTrx->diagnostic().toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(2003, 12, 18, 23, 56, 0), limitTimeReturn);
  }

  void testGetLimitDateTimeForHoursWhenReferenceTimeIsNotIgnored()
  {
    validateAdvanceTktTime_setup();

    DateTime limitTimeReturn;
    const DateTime referenceDT = DateTime(2003, 12, 18, 23, 59, 0);
    const Indicator dayInd = AdvanceResTkt::HOUR_UNIT_INDICATOR;

    CPPUNIT_ASSERT(_advanceResTkt->getLimitDateTime(limitTimeReturn,
                                                    referenceDT,
                                                    300, // 1-1440
                                                    3, // period
                                                    dayInd,
                                                    AdvanceResTkt::BEFORE_REF_TIME,
                                                    false,
                                                    true));

    // to ease diagnose of method flow
    _diagManager->collector().flushMsg();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _pricingTrx->diagnostic().toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(2003, 12, 18, 20, 59, 0), limitTimeReturn);
  }

  void testGetLimitDateTimeForDaysWhenReferenceTimeIsNotIgnored()
  {
    validateAdvanceTktTime_setup();

    DateTime limitTimeReturn;
    const DateTime referenceDT = DateTime(2003, 12, 18, 23, 59, 0);
    const Indicator dayInd = AdvanceResTkt::DAY_UNIT_INDICATOR;

    CPPUNIT_ASSERT(_advanceResTkt->getLimitDateTime(limitTimeReturn,
                                                    referenceDT,
                                                    300, // 1-1440
                                                    3, // period
                                                    dayInd,
                                                    AdvanceResTkt::BEFORE_REF_TIME,
                                                    false,
                                                    true));

    // to ease diagnose of method flow
    _diagManager->collector().flushMsg();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _pricingTrx->diagnostic().toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(2003, 12, 15, 23, 59, 0), limitTimeReturn);
  }

  void testGetLimitDateTimeForMonthsWhenReferenceTimeIsNotIgnored()
  {
    validateAdvanceTktTime_setup();

    DateTime limitTimeReturn;
    const DateTime referenceDT = DateTime(2003, 12, 18, 23, 59, 0);
    const Indicator monthInd = AdvanceResTkt::MONTH_UNIT_INDICATOR;

    CPPUNIT_ASSERT(_advanceResTkt->getLimitDateTime(limitTimeReturn,
                                                    referenceDT,
                                                    300, // 1-1440
                                                    3, // period
                                                    monthInd,
                                                    AdvanceResTkt::BEFORE_REF_TIME,
                                                    false,
                                                    true));

    // to ease diagnose of method flow
    _diagManager->collector().flushMsg();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _pricingTrx->diagnostic().toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(2003, 9, 18, 23, 59, 0), limitTimeReturn);
  }

  void testDetermineTicketDate_TktLater()
  {
    AdvanceResTkt advanceResTkt;
    advanceResTkt._pricingTrx = _memHandle.create<RexPricingTrx>();

    const DateTime ticketDT = DateTime(2005, 12, 18, 23, 59, 0);
    const DateTime departureDT = DateTime(2003, 12, 18, 23, 59, 0);

    const DateTime& adjustedDT = advanceResTkt.determineTicketDate(ticketDT, departureDT);

    CPPUNIT_ASSERT_EQUAL(departureDT, adjustedDT);
  }

  void testDetermineTicketDate_TktEarlier()
  {
    AdvanceResTkt advanceResTkt;
    advanceResTkt._pricingTrx = _memHandle.create<RexPricingTrx>();

    const DateTime ticketDT = DateTime(2001, 12, 18, 23, 59, 0);
    const DateTime departureDT = DateTime(2003, 12, 18, 23, 59, 0);

    const DateTime& adjustedDT = advanceResTkt.determineTicketDate(ticketDT, departureDT);

    CPPUNIT_ASSERT_EQUAL(ticketDT, adjustedDT);
  }

  void testDetermineTicketDate_notNewItin()
  {
    AdvanceResTkt advanceResTkt;
    RexPricingTrx rexTrx;
    RexPricingOptions opt;
    rexTrx.setOptions(&opt);
    rexTrx.setAnalyzingExcItin(true);
    advanceResTkt._pricingTrx = &rexTrx;

    const DateTime ticketDT = DateTime(2005, 12, 18, 23, 59, 0);
    const DateTime departureDT = DateTime(2003, 12, 18, 23, 59, 0);

    const DateTime& adjustedDT = advanceResTkt.determineTicketDate(ticketDT, departureDT);

    CPPUNIT_ASSERT_EQUAL(ticketDT, adjustedDT);
  }

  void testUpdateRebookStatusReturnTrueWhenAllFareUsagePass()
  {
    pricingTrxSetUp();
    prepareItinAndFareMarkets();
    FareUsage* fu1 = prepareFareUsage(_itin->fareMarket()[3]);
    FareUsage* fu2 = prepareFareUsage(_itin->fareMarket()[4]);

    std::map<const TravelSeg*, bool> segRebookStatus;
    segRebookStatus.insert(std::make_pair(_itin->travelSeg()[0], true));
    segRebookStatus.insert(std::make_pair(_itin->travelSeg()[1], true));

    PricingUnit pu;
    pu.fareUsage() += fu1, fu2;
    FarePath fp;
    fp.pricingUnit() += &pu;

    CPPUNIT_ASSERT(_advanceResTkt->updateRebookStatus(*_pricingTrx, fp, *fu1, segRebookStatus));
  }

  void testUpdateRebookStatusReturnFalseWhenFirstFareUsageFail()
  {
    pricingTrxSetUp();
    prepareItinAndFareMarkets();
    FareUsage* fu1 = prepareFareUsage(_itin->fareMarket()[3]);
    FareUsage* fu2 = prepareFareUsage(_itin->fareMarket()[4]);

    std::map<const TravelSeg*, bool> segRebookStatus;
    segRebookStatus.insert(std::make_pair(_itin->travelSeg()[0], true));
    _itin->travelSeg()[0]->setBookingCode("L"); // this will make sure first Fu fails
    segRebookStatus.insert(std::make_pair(_itin->travelSeg()[1], true));

    PricingUnit pu;
    pu.fareUsage() += fu1, fu2;
    FarePath fp;
    fp.pricingUnit() += &pu;

    CPPUNIT_ASSERT(!_advanceResTkt->updateRebookStatus(*_pricingTrx, fp, *fu1, segRebookStatus));
    CPPUNIT_ASSERT(!fu1->failedCat5InAnotherFu());
  }

  void testUpdateRebookStatusReturnFalseWhenSecondFareUsageFail()
  {
    pricingTrxSetUp();
    prepareItinAndFareMarkets();
    FareUsage* fu1 = prepareFareUsage(_itin->fareMarket()[3]);
    FareUsage* fu2 = prepareFareUsage(_itin->fareMarket()[4]);

    std::map<const TravelSeg*, bool> segRebookStatus;
    segRebookStatus.insert(std::make_pair(_itin->travelSeg()[0], true));
    segRebookStatus.insert(std::make_pair(_itin->travelSeg()[1], true));
    _itin->travelSeg()[1]->setBookingCode("L"); // this will make sure Second Fu fails

    PricingUnit pu;
    pu.fareUsage() += fu1, fu2;
    FarePath fp;
    fp.pricingUnit() += &pu;

    CPPUNIT_ASSERT(!_advanceResTkt->updateRebookStatus(*_pricingTrx, fp, *fu1, segRebookStatus));
    CPPUNIT_ASSERT(fu1->failedCat5InAnotherFu());
  }

  void testUpdateRebookStatusReturnSKIPwhenSegmentStatusNotInitialized()
  {
    pricingTrxSetUp();
    prepareItinAndFareMarkets();
    FareUsage* fu = prepareFareUsage(_itin->fareMarket()[1]);
    const std::map<const TravelSeg*, bool> segRebookStatus;
    CPPUNIT_ASSERT_EQUAL(SKIP,
                         _advanceResTkt->updateRebookStatus(
                             *_pricingTrx, _itin, *(fu->paxTypeFare()), fu, segRebookStatus));
  }

  void testUpdateRebookStatusReturnPASSwhenSegmentRebooked()
  {
    pricingTrxSetUp();
    prepareItinAndFareMarkets();
    FareUsage* fu = prepareFareUsage(_itin->fareMarket()[3]);

    std::map<const TravelSeg*, bool> segRebookStatus;
    segRebookStatus.insert(std::make_pair(_itin->travelSeg()[0], true));

    CPPUNIT_ASSERT_EQUAL(PASS,
                         _advanceResTkt->updateRebookStatus(
                             *_pricingTrx, _itin, *(fu->paxTypeFare()), fu, segRebookStatus));
    CPPUNIT_ASSERT_EQUAL(
        true, fu->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
    CPPUNIT_ASSERT_EQUAL(_itin->travelSeg()[0]->getBookingCode(),
                         fu->segmentStatus()[0]._bkgCodeReBook);
  }

  void testUpdateRebookStatusReturnFAILwhenSegmentNotRebooked()
  {
    pricingTrxSetUp();
    prepareItinAndFareMarkets();
    FareUsage* fu = prepareFareUsage(_itin->fareMarket()[3]);

    std::map<const TravelSeg*, bool> segRebookStatus;
    segRebookStatus.insert(std::make_pair(_itin->travelSeg()[0], true));
    _itin->travelSeg()[0]->setBookingCode("L");

    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _advanceResTkt->updateRebookStatus(
                             *_pricingTrx, _itin, *(fu->paxTypeFare()), fu, segRebookStatus));
    CPPUNIT_ASSERT_EQUAL(
        false, fu->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
  }

  void testCheckAvailReturnTrueWhenBookingCodeAvailable()
  {
    pricingTrxSetUp();
    AirSeg airSeg;
    airSeg.setBookingCode("Y");
    std::vector<ClassOfService*> cosVec;
    populateBookingCodeYinCosVec(cosVec);
    PaxTypeFare ptf;
    CPPUNIT_ASSERT(
        _advanceResTkt->checkAvail(static_cast<TravelSeg&>(airSeg), cosVec, *_pricingTrx, ptf));
  }

  void testCheckAvailReturnFalseWhenBookingCodeNotAvailable()
  {
    pricingTrxSetUp();
    AirSeg airSeg;
    airSeg.setBookingCode("L");
    std::vector<ClassOfService*> cosVec;
    populateBookingCodeYinCosVec(cosVec);
    PaxTypeFare ptf;
    CPPUNIT_ASSERT(
        !_advanceResTkt->checkAvail(static_cast<TravelSeg&>(airSeg), cosVec, *_pricingTrx, ptf));
  }

  void testPartOfLocalJourneyReturnFalseForNonPricingTrx()
  {
    pricingTrxSetUp();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    AirSeg airSeg;
    CPPUNIT_ASSERT(!_advanceResTkt->partOfLocalJourney(*_pricingTrx, (TravelSeg*)&airSeg));
  }

  void testPartOfLocalJourneyReturnFalseWhenJourneyNotActive()
  {
    pricingTrxSetUp();
    _options.journeyActivatedForPricing() = false;
    AirSeg airSeg;
    CPPUNIT_ASSERT(!_advanceResTkt->partOfLocalJourney(*_pricingTrx, (TravelSeg*)&airSeg));
  }

  void testPartOfLocalJourneyReturnFalseWhenJourneyNotApply()
  {
    pricingTrxSetUp();
    _options.applyJourneyLogic() = false;
    AirSeg airSeg;
    CPPUNIT_ASSERT(!_advanceResTkt->partOfLocalJourney(*_pricingTrx, (TravelSeg*)&airSeg));
  }

  void testPartOfLocalJourneyReturnFalseWhenSegArunk()
  {
    pricingTrxSetUp();
    ArunkSeg tvlSeg;
    CPPUNIT_ASSERT(!_advanceResTkt->partOfLocalJourney(*_pricingTrx, (TravelSeg*)&tvlSeg));
  }

  void testPartOfLocalJourneyReturnFalseWhenNotLocalJourneyCarrier()
  {
    pricingTrxSetUp();
    AirSeg airSeg;
    _carrierPref.localMktJourneyType() = NO;
    airSeg.carrierPref() = &_carrierPref;
    CPPUNIT_ASSERT(!_advanceResTkt->partOfLocalJourney(*_pricingTrx, (TravelSeg*)&airSeg));
  }

  void testPartOfLocalJourneyReturnFalseWhenNotWPNCentry()
  {
    pricingTrxSetUp();
    _request.lowFareRequested() = 'F'; //  Not WPNC entry
    AirSeg airSeg;
    airSeg.carrierPref() = &_carrierPref;
    CPPUNIT_ASSERT(!_advanceResTkt->partOfLocalJourney(*_pricingTrx, (TravelSeg*)&airSeg));
  }

  void testPartOfLocalJourneyReturnFalseWhenSegNotFoundInFlowMarket()
  {
    pricingTrxSetUp();
    prepareItinAndFareMarkets();
    AirSeg airSeg;
    airSeg.carrierPref() = &_carrierPref;
    CPPUNIT_ASSERT(!_advanceResTkt->partOfLocalJourney(*_pricingTrx, (TravelSeg*)&airSeg));
  }

  void testPartOfLocalJourneyReturnTrueWhenSegFoundInFlowMarket()
  {
    pricingTrxSetUp();
    prepareItinAndFareMarkets();
    dynamic_cast<AirSeg*>(_itin->travelSeg()[1])->carrierPref() = &_carrierPref;
    CPPUNIT_ASSERT(_advanceResTkt->partOfLocalJourney(*_pricingTrx, _itin->travelSeg()[1]));
  }

  void testGetFlowMarketReturnZeroWhenTravelSegNotFoundInFareMarket()
  {
    pricingTrxSetUp();
    prepareItinAndFareMarkets();
    AirSeg airSegNotFound;
    CPPUNIT_ASSERT_EQUAL(
        (const tse::FareMarket*)(FareMarket*)0,
        _advanceResTkt->getFlowMarket(*_pricingTrx, _itin, (TravelSeg*)&airSegNotFound));
  }

  void testGetFlowAvailReturnZeroWhenTravelSegNotFound()
  {
    AirSeg airSegInFareMarket, airSegNotInFareMarket;
    FareMarket fm;
    fm.travelSeg() += (TravelSeg*)&airSegInFareMarket;
    std::vector<ClassOfService*> cosVec1;
    fm.classOfServiceVec() += &cosVec1;
    CPPUNIT_ASSERT_EQUAL((std::vector<ClassOfService*>*)0,
                         _advanceResTkt->getFlowAvail((TravelSeg*)&airSegNotInFareMarket, &fm));
  }

  void testGetFlowAvailReturnZeroWhenTravelSegNotFoundAndArunkInFareMarket()
  {
    AirSeg airSeg1, airSeg2;
    ArunkSeg arunk;
    FareMarket fm;
    fm.travelSeg() += (TravelSeg*)&arunk, (TravelSeg*)&airSeg1;

    std::vector<ClassOfService*> emptyCosVec;
    fm.classOfServiceVec() += &emptyCosVec;

    CPPUNIT_ASSERT_EQUAL((std::vector<ClassOfService*>*)0,
                         _advanceResTkt->getFlowAvail((TravelSeg*)&airSeg2, &fm));
  }
  void testIsHistorical_TYO_0955()
  {
    pricingTrxSetUp();
    // if TUL is 09:55, TYO is 23:55 - not hist
    _pricingTrx->getRequest()->ticketingDT() = DateTime(9999, 05, 10, 9, 55, 01);
    CPPUNIT_ASSERT(!_advanceResTkt->isHistorical(*_pricingTrx));
  }
  void testIsHistorical_TYO_1020()
  {
    pricingTrxSetUp();
    // if TUL is 10:20, TYO is 00:20 - hist
    _pricingTrx->getRequest()->ticketingDT() = DateTime(2009, 05, 10, 10, 20, 01);
    CPPUNIT_ASSERT(_advanceResTkt->isHistorical(*_pricingTrx));
  }

  void checkConfirmedStatusNotPermitted_setup()
  {
    _advResTktInfo = _memHandle.create<AdvResTktInfo>();
    _aSeg = _memHandle.create<AirSeg>();
    _aSeg->resStatus() = "OK";
    _advResTktInfo->permitted() = 'X';
    _ticketDT = DateTime(2009, 12, 01, 12, 23, 03);
    _departureDT = DateTime(2009, 12, 01, 18, 31, 53);
  }

  void validateCheckConfirmedStatusNotPermitted_Permited()
  {
    checkConfirmedStatusNotPermitted_setup();
    _advResTktInfo->permitted() = ' ';
    CPPUNIT_ASSERT(!_advanceResTkt->checkConfirmedStatusNotPermitted(
        *_aSeg, *_advResTktInfo, _ticketDT, _departureDT));
  }
  void validateCheckConfirmedStatusNotPermitted_NotConfirmed()
  {
    checkConfirmedStatusNotPermitted_setup();
    _aSeg->resStatus() = "NS";
    CPPUNIT_ASSERT(_advanceResTkt->checkConfirmedStatusNotPermitted(
        *_aSeg, *_advResTktInfo, _ticketDT, _departureDT));
  }
  void validateCheckConfirmedStatusNotPermitted_Confirmed_SameDay()
  {
    checkConfirmedStatusNotPermitted_setup();
    CPPUNIT_ASSERT(_advanceResTkt->checkConfirmedStatusNotPermitted(
        *_aSeg, *_advResTktInfo, _ticketDT, _departureDT));
  }
  void validateCheckConfirmedStatusNotPermitted_Confirmed_DiffDay()
  {
    checkConfirmedStatusNotPermitted_setup();
    _departureDT = DateTime(2009, 12, 02, 02, 11, 28);
    CPPUNIT_ASSERT(!_advanceResTkt->checkConfirmedStatusNotPermitted(
        *_aSeg, *_advResTktInfo, _ticketDT, _departureDT));
  }

  void preparePUAndTwoFareUsagesForRex()
  {
    _pricingTrx = _memHandle.create<RexPricingTrx>();
    (static_cast<RexPricingTrx*>(_pricingTrx))->trxPhase() = RexPricingTrx::PRICE_NEWITIN_PHASE;

    prepareItinAndFareMarkets();
    FareUsage* fu1 = prepareFareUsage(_itin->fareMarket()[2]);
    FareUsage* fu2 = prepareFareUsage(_itin->fareMarket()[3]);
    _pricingUnit = _memHandle.create<PricingUnit>();
    _pricingUnit->fareUsage() += fu1, fu2;
  }

  void addToApplTvlSeg(RuleUtil::TravelSegWrapperVector& applTravelSegment, const FareUsage& fu)
  {
    std::vector<TravelSeg*>::const_iterator tvlI = fu.travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tvlIEnd = fu.travelSeg().end();
    for (; tvlI != tvlIEnd; ++tvlI)
    {
      RuleUtil::TravelSegWrapper* tsw = _memHandle.create<RuleUtil::TravelSegWrapper>();
      tsw->travelSeg() = *tvlI;
      tsw->origMatch() = true;
      tsw->destMatch() = false;
      applTravelSegment.push_back(tsw);
    }
  }

  void setFareUsageIgnoreCat5(FareUsage& fareUsage)
  {
    fareUsage.categoryIgnoredForKeepFare().insert(5);
  }

  void testDeleteTvlSegIgnoreCat5_true_ifexist_KeepFare_ignoreCat5()
  {
    preparePUAndTwoFareUsagesForRex();
    RuleUtil::TravelSegWrapperVector applTravelSegment, newApplTvlSegs;
    addToApplTvlSeg(applTravelSegment, *_pricingUnit->fareUsage()[0]);
    addToApplTvlSeg(applTravelSegment, *_pricingUnit->fareUsage()[1]);

    setFareUsageIgnoreCat5(*(_pricingUnit->fareUsage()[1]));

    CPPUNIT_ASSERT(_advanceResTkt->deleteTvlSegIgnoreCat5(
        *_pricingTrx, _pricingUnit, applTravelSegment, newApplTvlSegs));

    CPPUNIT_ASSERT_EQUAL(applTravelSegment.size(),
                         (_pricingUnit->fareUsage()[0]->travelSeg().size() +
                          _pricingUnit->fareUsage()[1]->travelSeg().size()));
    CPPUNIT_ASSERT_EQUAL(newApplTvlSegs.size(), _pricingUnit->fareUsage()[0]->travelSeg().size());
  }

  void testDeleteTvlSegIgnoreCat5_false_ifnoexist_KeepFare_ignoreCat5()
  {
    preparePUAndTwoFareUsagesForRex();
    RuleUtil::TravelSegWrapperVector applTravelSegment, newApplTvlSegs;
    addToApplTvlSeg(applTravelSegment, *_pricingUnit->fareUsage()[0]);
    addToApplTvlSeg(applTravelSegment, *_pricingUnit->fareUsage()[1]);

    CPPUNIT_ASSERT(!_advanceResTkt->deleteTvlSegIgnoreCat5(
        *_pricingTrx, _pricingUnit, applTravelSegment, newApplTvlSegs));

    CPPUNIT_ASSERT_EQUAL(applTravelSegment.size(),
                         (_pricingUnit->fareUsage()[0]->travelSeg().size() +
                          _pricingUnit->fareUsage()[1]->travelSeg().size()));
    CPPUNIT_ASSERT(0 == newApplTvlSegs.size());
  }

  void testGetPrevExchangeOrOriginTktIssueDT_NotReissueExchange()
  {
    RexPricingTrx& rTrx = createRexPricingTrx();
    rTrx.previousExchangeDT() = DateTime::emptyDate();

    CPPUNIT_ASSERT_EQUAL(rTrx.originalTktIssueDT(),
                         _advanceResTkt->getPrevExchangeOrOriginTktIssueDT(rTrx));
  }

  void testGetPrevExchangeOrOriginTktIssueDT_NoPreviousExchangeDT_IsSet()
  {
    RexPricingTrx& rTrx = createRexPricingTrx();
    rTrx.previousExchangeDT() = DateTime::emptyDate();

    CPPUNIT_ASSERT_EQUAL(rTrx.originalTktIssueDT(),
                         _advanceResTkt->getPrevExchangeOrOriginTktIssueDT(rTrx));
  }

  void testGetPrevExchangeOrOriginTktIssueDT_NotReissueExchangeSet_PreviousExchangeDT_IsSet()
  {
    RexPricingTrx& rTrx = createRexPricingTrx();
    rTrx.setRexPrimaryProcessType(' ');

    CPPUNIT_ASSERT_EQUAL(rTrx.originalTktIssueDT(),
                         _advanceResTkt->getPrevExchangeOrOriginTktIssueDT(rTrx));
  }

  void testGetPrevExchangeOrOriginTktIssueDT_AllSet()
  {
    RexPricingTrx& rTrx = createRexPricingTrx();

    CPPUNIT_ASSERT_EQUAL(rTrx.previousExchangeDT(),
                         _advanceResTkt->getPrevExchangeOrOriginTktIssueDT(rTrx));
  }

  RexPricingTrx& createRexPricingTrx()
  {
    RexPricingTrx& trx = *_memHandle(new RexPricingTrx);
    trx.setOptions(_memHandle(new PricingOptions));
    trx.setAnalyzingExcItin(false);
    trx.setOriginalTktIssueDT() = DateTime(2005, 5, 5);
    trx.previousExchangeDT() = DateTime(2006, 6, 6);
    trx.setRexPrimaryProcessType('A');
    _advanceResTkt->_pricingTrx = &trx;
    return trx;
  }

  void testDetermineTicketDateForRex_NotForPricingUnit()
  {
    RexPricingTrx& trx = createRexPricingTrx();
    DateTime tktDT(2007, 7, 7), departureDT(2008, 8, 8);

    CPPUNIT_ASSERT_EQUAL(trx.originalTktIssueDT(),
                         _advanceResTkt->determineTicketDateForRex(trx, false, tktDT, departureDT));
  }

  void testDetermineTicketDateForRex_ApplyReissueExchange()
  {
    RexPricingTrx& trx = createRexPricingTrx();
    trx.setRexPrimaryProcessType(' ');
    DateTime tktDT(trx.originalTktIssueDT()), departureDT(2003, 3, 3);

    CPPUNIT_ASSERT_EQUAL(departureDT,
                         _advanceResTkt->determineTicketDateForRex(trx, true, tktDT, departureDT));
  }

  void testDetermineTicketDateForRex_PreviousExchangeDT()
  {
    RexPricingTrx& trx = createRexPricingTrx();
    DateTime tktDT(trx.originalTktIssueDT()), departureDT(2008, 8, 8);

    CPPUNIT_ASSERT_EQUAL(trx.previousExchangeDT(),
                         _advanceResTkt->determineTicketDateForRex(trx, true, tktDT, departureDT));
  }

  void testDetermineTicketDateForRex_NotForPricingUnit_NewTicketIssueDate()
  {
    RexPricingTrx& trx = createRexPricingTrx();
    DateTime tktDT(trx.originalTktIssueDT()), departureDT(2008, 8, 8);
    _advanceResTkt->_newTicketIssueDate = DateTime(2009, 9, 9);

    CPPUNIT_ASSERT_EQUAL(_advanceResTkt->_newTicketIssueDate,
                         _advanceResTkt->determineTicketDateForRex(trx, false, tktDT, departureDT));
  }

  void testDetermineTicketDateForRex_ApplyReissueExchange_NewTicketIssueDate()
  {
    RexPricingTrx& trx = createRexPricingTrx();
    trx.setRexPrimaryProcessType(' ');
    DateTime tktDT(trx.originalTktIssueDT()), departureDT(2008, 8, 8);
    _advanceResTkt->_newTicketIssueDate = DateTime(2007, 7, 7);

    CPPUNIT_ASSERT_EQUAL(_advanceResTkt->_newTicketIssueDate,
                         _advanceResTkt->determineTicketDateForRex(trx, true, tktDT, departureDT));
  }
  void testCalculatUTCBetweenAgentLocationAndDepartureCity()
  {
    pricingTrxSetUp();
    setupValidateTestData();
    prepareItinAndFareMarkets();
    const PricingTrx& trx = *_pricingTrx;
    dynamic_cast<AirSeg*>(_itin->travelSeg()[1])->origAirport() = "SAN";
    dynamic_cast<AirSeg*>(_itin->travelSeg()[1])->carrierPref() = &_carrierPref;
    const TravelSeg& trvseg = *_itin->travelSeg()[1];
    short utcOffset = _advanceResTkt->checkUTC(trx, trvseg);
    short utcMatch = -120;
    CPPUNIT_ASSERT(utcMatch == utcOffset);
  }

  void testIsFlownSectorForNewItinPass()
  {
    AirSeg seg;
    seg.unflown() = false;
    RexPricingTrx rexTrx;
    rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    CPPUNIT_ASSERT(_advanceResTkt->isFlownSectorForNewItin(rexTrx, seg));

    RefundPricingTrx refundTrx;
    refundTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    CPPUNIT_ASSERT(_advanceResTkt->isFlownSectorForNewItin(refundTrx, seg));
  }

  class AdvanceResTktStub : public AdvanceResTkt
  {
  public:
    explicit AdvanceResTktStub(const DateTime& tktFromDateOverride = DateTime::emptyDate())
    {
      _tktFromDateOverride = tktFromDateOverride;
    }

  private:
    virtual void updateBookingDate(DateTime& bookingDT, PricingTrx& trx) const {}
  };

  void callBookingDateDetermination(const AdvanceResTkt& art, DateTime& bookingDT)
  {
    PricingTrx trx;
    AirSeg tvlSeg;
    tvlSeg.bookingDT() = DateTime(2006, 6, 6);
    bool isRebooked = false;

    art.getBookingDate(trx, tvlSeg, bookingDT, isRebooked, 0);
  }

  void testCat31BookingDateDeterminationYes()
  {
    DateTime bookingDT;
    callBookingDateDetermination(AdvanceResTktStub(DateTime(2007, 7, 7)), bookingDT);
    CPPUNIT_ASSERT_EQUAL(DateTime(2007, 7, 7), bookingDT);
  }

  void testCat31BookingDateDeterminationNo()
  {
    DateTime bookingDT;
    callBookingDateDetermination(AdvanceResTktStub(), bookingDT);
    CPPUNIT_ASSERT_EQUAL(DateTime(2006, 6, 6), bookingDT);
  }

  void testIsFlownSectorForNewItinFail()
  {
    AirSeg seg;
    seg.unflown() = false;
    RexPricingTrx rexTrx;
    rexTrx.trxPhase() = RexBaseTrx::MATCH_EXC_RULE_PHASE;
    CPPUNIT_ASSERT(!_advanceResTkt->isFlownSectorForNewItin(rexTrx, seg));

    seg.unflown() = true;
    RefundPricingTrx refundTrx;
    refundTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    CPPUNIT_ASSERT(!_advanceResTkt->isFlownSectorForNewItin(refundTrx, seg));
  }

  void testGetLimitDateTimeWhenTKTMustbeIssueTheHourReservationIsDone()
  {
    validateAdvanceTktTime_setup();

    DateTime limitTimeReturn;
    const DateTime referenceDT = DateTime(2013, 9, 05, 04, 19, 0);
    const Indicator dayInd = AdvanceResTkt::HOUR_UNIT_INDICATOR;

    CPPUNIT_ASSERT(_advanceResTkt->getLimitDateTime(limitTimeReturn,
                                                    referenceDT,
                                                    -1, // 1-1440
                                                    0, // period
                                                    dayInd,
                                                    AdvanceResTkt::AFTER_REF_TIME,
                                                    true,
                                                    false));

    // to ease diagnose of method flow
    _diagManager->collector().flushMsg();
    CPPUNIT_ASSERT_EQUAL(std::string(""), _pricingTrx->diagnostic().toString());
    CPPUNIT_ASSERT_EQUAL(DateTime(2013, 9, 05, 05, 18, 0), limitTimeReturn);
  }

  void testValidateFareMarketRtwFailBeforeTurnaround()
  {
    commonRTWPreparation();

    _pricingTrx->itin()[0]->travelSeg()[1]->resStatus() = "";

    //--------------------------------------------------------
    // Should FAIL,
    // reservation status empty for segment before turnaround
    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _advanceResTkt->validate(*_pricingTrx, *_itin, *_paxTypeFare, _advResTktInfo, *_fm));
    CPPUNIT_ASSERT(ret == FAIL);

    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketRtwFailTurnaround()
  {
    commonRTWPreparation();

    //--------------------------------------------------------
    // Should FAIL,
    // reservation status empty for turnaround segment
    _pricingTrx->itin()[0]->travelSeg()[2]->resStatus() = "";

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _advanceResTkt->validate(*_pricingTrx, *_itin, *_paxTypeFare, _advResTktInfo, *_fm));
    CPPUNIT_ASSERT(ret == FAIL);

    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketRtwPass()
  {
    commonRTWPreparation();

    //--------------------------------------------------------
    // Should PASS,
    // reservation status empty for segment after turnaround
    _pricingTrx->itin()[0]->travelSeg()[3]->resStatus() = "";

    Record3ReturnTypes ret = FAIL;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _advanceResTkt->validate(*_pricingTrx, *_itin, *_paxTypeFare, _advResTktInfo, *_fm));
    CPPUNIT_ASSERT(ret == PASS);

    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketRtwPassArunkBeforeTurnaround()
  {
    commonRTWPreparation();

    //--------------------------------------------------------
    // Should PASS,
    // arunk as turnaround segment
    _pricingTrx->itin()[0]->travelSeg()[2] = _memHandle.create<ArunkSeg>();
    _pricingTrx->itin()[0]->travelSeg()[2]->resStatus() = "";

    Record3ReturnTypes ret = FAIL;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _advanceResTkt->validate(*_pricingTrx, *_itin, *_paxTypeFare, _advResTktInfo, *_fm));
    CPPUNIT_ASSERT(ret == PASS);

    verifyDiagnostic(_pricingTrx->diagnostic().toString());
  }

  void testValidateFareMarketRtwFailRequireAllSegsConf()
  {
    commonRTWPreparation();

    //--------------------------------------------------------
    // Should FAIL,
    // because of open segment (even though it's after turnaround)
    _pricingTrx->itin()[0]->travelSeg()[2] = _memHandle.create<AirSeg>();
    _pricingTrx->itin()[0]->travelSeg()[2]->segmentType() = Open;
    _pricingTrx->itin()[0]->travelSeg()[2]->resStatus() = "";
    _advResTktInfo->confirmedSector() = AdvanceResTkt::allSector;

    CPPUNIT_ASSERT_EQUAL(
        PASS, _advanceResTkt->validate(*_pricingTrx, *_itin, *_paxTypeFare, _advResTktInfo, *_fm));
  }

  class MyDataHandle : public DataHandleMock
  {
  public:
    bool getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                                const DSTGrpCode& dstgrp2,
                                short& utcoffset,
                                const DateTime& dateTime1,
                                const DateTime& dateTime2)
    {
      utcoffset = 0;
      return true;
    }

    const TSIInfo* getTSI(int key)
    {
      if (key == 1)
        return getTsi(1, 'S');
      else if (key == 3)
        return getTsi(3, 'F');

      return DataHandleMock::getTSI(key);
    }

  private:
    DST* getDst(DSTGrpCode grp, short utcoffset, short adjmin = 0)
    {
      DSTAdjMinutes* dstmin = new DSTAdjMinutes;
      dstmin->_dstAdjmin = adjmin;
      dstmin->_dstStart = DateTime(1980, 1, 1);
      dstmin->_dstFinish = DateTime(2200, 1, 1);

      DST* ret = _memHandle.create<DST>();
      ret->dstgroup() = grp;
      ret->utcoffset() = utcoffset;
      ret->dstAdjMinutes().push_back(dstmin);

      return ret;
    }

    TSIInfo* getTsi(int tsi, char scope)
    {
      TSIInfo* ret = _memHandle.create<TSIInfo>();
      ret->tsi() = tsi;
      ret->geoRequired() = ' ';
      ret->geoNotType() = ' ';
      ret->geoOut() = ' ';
      ret->geoItinPart() = ' ';
      ret->geoCheck() = 'O';
      ret->loopDirection() = 'F';
      ret->loopOffset() = 0;
      ret->loopToSet() = 0;
      ret->loopMatch() = 'O';
      ret->scope() = scope;
      ret->type() = 'O';

      return ret;
    }

    TestMemHandle _memHandle;
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(AdvResTktTest);

} // tse
