#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/ExcItin.h"
#include "DataModel/AirSeg.h"
#include "Common/DateTime.h"
#include "DataModel/Billing.h"
#include "Common/TrxUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexPricingRequest.h"
#include "DBAccess/Loc.h"
#include "test/include/TestFallbackUtil.h"
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

namespace tse
{
using namespace boost::assign;

class RPTDataHandleMock : public DataHandleMock
{
public:
  bool getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                              const DSTGrpCode& dstgrp2,
                              short& utcoffset,
                              const DateTime& dateTime1,
                              const DateTime& dateTime2)
  {
    return (utcoffset = 6);
  }
};

std::ostream& operator<<(std::ostream& os, const std::vector<RexPricingTrx::DateSeqType>& seq)
{
  std::vector<RexPricingTrx::DateSeqType>::const_iterator i = seq.begin();
  for (; i != seq.end(); ++i)
    os << "(" << i->first << "," << i->second.toSimpleString() << ") ";
  return os;
}

namespace
{
static const std::vector<FarePath*>::size_type one = 1;

static const std::vector<ProcessTagPermutation*>::size_type onep = 1;
static const std::vector<ProcessTagPermutation*>::size_type two = 2;
static const std::vector<ProcessTagPermutation*>::size_type three = 3;
}

class RexPricingTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexPricingTrxTest);
  CPPUNIT_TEST(testDefaultValuesAreInitiallyPopulated);
  CPPUNIT_TEST(testSetExchangeAccompPaxType);
  CPPUNIT_TEST(testUnflownItineraryValidOneYearFromOriginalTicketDate);
  CPPUNIT_TEST(testPartialFlownItineraryValidOneYearFromFirstSegment);
  CPPUNIT_TEST(testForFullyFlownItineraryValidityDateIsNotSet);
  CPPUNIT_TEST(testTicketDateFromCurrentWhenAnalyzeExchangeItinNotSet);
  CPPUNIT_TEST(testTicketDateFromOriginalWhenAnalyzeExchangeItinSet);

  CPPUNIT_TEST(testSetFareApplicationDTWhenAnalysingNewItin);
  CPPUNIT_TEST(testSetFareApplicationDTWhenAnalysingExchangeItin);
  CPPUNIT_TEST(testFareApplicationDTWhenAnalysingNewItin);
  CPPUNIT_TEST(testFareApplicationDTWhenAnalysingExchangeItin);

  CPPUNIT_TEST(testMarkFareRetrievalMethodHistorical);
  CPPUNIT_TEST(testMarkFareRetrievalMethodKeep);
  CPPUNIT_TEST(testMarkFareRetrievalMethodTvlCommence);
  CPPUNIT_TEST(testMarkFareRetrievalMethodCurrent);
  CPPUNIT_TEST(testMarkAllFareRetrievalMethod);

  CPPUNIT_TEST(testNeedRetrieveHistoricalFare);
  CPPUNIT_TEST(testNeedRetrieveKeepFare);
  CPPUNIT_TEST(testNeedRetrieveTvlCommenceFare);
  CPPUNIT_TEST(testNeedRetrieveCurrentFare);
  CPPUNIT_TEST(testNeedRetrieveAllFare);
  CPPUNIT_TEST(testNeedRetrieveFare);

  CPPUNIT_TEST(testTicketingDate);
  CPPUNIT_TEST(testSetAnalyzingExcItin);

  CPPUNIT_TEST(testSetupDateSeq_O);
  CPPUNIT_TEST(testSetupDateSeq_OC);
  CPPUNIT_TEST(testSetupDateSeq_OR);
  CPPUNIT_TEST(testSetupDateSeq_OCR);
  CPPUNIT_TEST(testSetupDateSeq_ORC);
  CPPUNIT_TEST(testSetupDateSeq_ORC2);
  CPPUNIT_TEST(testSetupDateSeq_O2);
  CPPUNIT_TEST(testSetupDateSeq_OR2);
  CPPUNIT_TEST(testRSVStatus);

  CPPUNIT_TEST(testBuildFlownSegRetrievalMap);

  CPPUNIT_TEST(testSetActionCodeForRexPricingRedirectedToPortExchange);

  CPPUNIT_TEST(testSetAdjustedTravelDate_N);
  CPPUNIT_TEST(testAdjustedTravelDate_N);
  CPPUNIT_TEST(testSetAdjustedTravelDate_Y_HasUnflown);
  CPPUNIT_TEST(testSetAdjustedTravelDate_Y);
  CPPUNIT_TEST(testAdjustedTravelDate_Y);

  CPPUNIT_TEST(testTravelDate);
  CPPUNIT_TEST(testSetUpRexSkipSecurity_Airline);
  CPPUNIT_TEST(testSetUpRexSkipSecurity_Subscriber);

  CPPUNIT_TEST(testSetupMipDateSeq_fcDateEqOrig);
  CPPUNIT_TEST(testSetupMipDateSeq_fcDateDiffThanOrig);
  CPPUNIT_TEST(testSetupMipDateSeq_fcDateEmpty);
  CPPUNIT_TEST(testSetupMipDateSeq_twoFcDates);
  CPPUNIT_TEST(testSetupMipDateSeq_twoFcDatesEqual);

  CPPUNIT_TEST(testTktValidityDateWhenPreviousExchDateAndNOExchReissueSetOriginTktDate);
  CPPUNIT_TEST(testTktValidityDateWhenPreviousExchDateAndExchReissueSetPrevExchDate);

  CPPUNIT_TEST(testNewItinSecondROEConversionDateGetsInitializedToEmptyDate);
  CPPUNIT_TEST(testNewItinSecondROEConversionDateGettterSetter);

  CPPUNIT_TEST(testValidSolutionsPlacement1);
  CPPUNIT_TEST(testValidSolutionsPlacement2);
  CPPUNIT_TEST(testValidSolutionsPlacement3);
  CPPUNIT_TEST(testValidSolutionsPlacement4);
  CPPUNIT_TEST(testValidSolutionsPlacement5);
  CPPUNIT_TEST(testValidSolutionsPlacement6);
  CPPUNIT_TEST(testValidSolutionsPlacement7);
  CPPUNIT_TEST(testValidSolutionsPlacement8);
  CPPUNIT_TEST(testValidSolutionsPlacement9);
  CPPUNIT_TEST(testValidSolutionsPlacement10);
  CPPUNIT_TEST(testValidSolutionsPlacement11);
  CPPUNIT_TEST(testValidSolutionsPlacement12);
  CPPUNIT_TEST(testValidSolutionsPlacement13);
  CPPUNIT_TEST(testValidSolutionsPlacement14);
  CPPUNIT_TEST(testValidSolutionsPlacement15);
  CPPUNIT_TEST(testValidSolutionsPlacement16);
  CPPUNIT_TEST(testValidSolutionsPlacement17);
  CPPUNIT_TEST(testValidSolutionsPlacement18);
  CPPUNIT_TEST(testValidSolutionsPlacement19);
  CPPUNIT_TEST(testValidSolutionsPlacement20);
  CPPUNIT_TEST(testValidSolutionsPlacement21);
  CPPUNIT_TEST(testValidSolutionsPlacement22);
  CPPUNIT_TEST(testValidSolutionsPlacement23);
  CPPUNIT_TEST(testValidSolutionsPlacement24);
  CPPUNIT_TEST(testValidSolutionsPlacement25);

  CPPUNIT_TEST(testDivideOne);
  CPPUNIT_TEST(testDivideMixed7);
  CPPUNIT_TEST(testDivideMixedK7);
  CPPUNIT_TEST(testDivideMixed4KX79);

  CPPUNIT_TEST(testUseHistoricalRoeDateExchangeMixedFares);
  CPPUNIT_TEST(testUseHistoricalRoeDateExchangeHistoricalFares);
  CPPUNIT_TEST(testUseHistoricalRoeDateExchangeCurrentFares);
  CPPUNIT_TEST(testUseHistoricalRoeDateReissueMixedFares);
  CPPUNIT_TEST(testUseHistoricalRoeDateReissueHistoricalFares);
  CPPUNIT_TEST(testUseHistoricalRoeDateReissueCurrentFares);
  CPPUNIT_TEST(testROEDateSetterTurnOnSecondDateIndicatorWhenFirstROEDateNotHistorical);
  CPPUNIT_TEST(testROEDateSetterDontTurnOnSecondDateIndicatorWhenFirstROEDateIsHistorical);
  CPPUNIT_TEST(testROEDateSetterTurnOnSecondDateIndicatorWhenFirstROEDateNotCurrent);
  CPPUNIT_TEST(testROEDateSetterDontTurnOnSecondDateIndicatorWhenFirstROEDateCurrent);

  CPPUNIT_TEST(testAreHistoricalFaresNeededForHIP_success);
  CPPUNIT_TEST(testAreHistoricalFaresNeededForHIP_noParentMarket);
  CPPUNIT_TEST(testAreHistoricalFaresNeededForHIP_noStopover);

  CPPUNIT_TEST(testUtcAdjustementPrevious);
  CPPUNIT_TEST(testUtcAdjustementOriginal);

  CPPUNIT_TEST(testRRFPPUrefundable);
  CPPUNIT_TEST(testRRFPPUnonRefundable);
  CPPUNIT_TEST(testRRFPPUindeterminate);
  CPPUNIT_TEST(testRRFPPUtrxState);
  CPPUNIT_TEST(testRRFPPUplusUpNeededF);
  CPPUNIT_TEST(testRRFPPUplusUpNeededT);
  CPPUNIT_TEST(testSetExcTktNonRefundableNotMatched);

  CPPUNIT_TEST_SUITE_END();

public:
  RexPricingTrxTest()
  {
    log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle(new RexPricingTrx);
    _trx->newItin().push_back(new Itin);
    trx.setOptions(_memHandle.insert(new RexPricingOptions));
    _dataHandleMock = _memHandle.create<RPTDataHandleMock>();
    _rexRequest = _memHandle.create<RexPricingRequest>();
    _trx->setRequest(_rexRequest);
    _rexRequest->_originalTicketAgentLocation = _memHandle.create<Loc>();
    _rexRequest->_ticketingAgents[0] = _memHandle.create<Agent>();
    _rexRequest->_ticketingAgents[0]->agentCity() = "MOV";
    _rexRequest->_ticketingAgents[0]->agentLocation() = _memHandle.create<Loc>();
    _rexRequest->_prevTicketIssueAgent = _memHandle.create<Agent>();
    _rexRequest->_prevTicketIssueAgent->agentCity() = "MOV";
    _rexRequest->_prevTicketIssueAgent->agentLocation() = _memHandle.create<Loc>();
    // ROE migrated
    _pu1 = _memHandle(new PricingUnit);
    _fu11 = _memHandle(new FareUsage);
    _ptf11 = _memHandle(new PaxTypeFare);
    _fu12 = _memHandle(new FareUsage);
    _ptf12 = _memHandle(new PaxTypeFare);
    _ptf11->fareClassAppInfo() = _memHandle(new FareClassAppInfo);
    _ptf12->fareClassAppInfo() = _memHandle(new FareClassAppInfo);

    _fu11->paxTypeFare() = _ptf11;
    _fu12->paxTypeFare() = _ptf12;

    _pu1->fareUsage().push_back(_fu11);
    _pu1->fareUsage().push_back(_fu12);

    _fp = _memHandle(new FarePath);
    _fpValid = _memHandle(new bool(true));
    _fp->pricingUnit().push_back(_pu1);
  }

  void tearDown() { _memHandle.clear(); }

  void testDefaultValuesAreInitiallyPopulated()
  {
    CPPUNIT_ASSERT(trx.reqType() == "AR");
    CPPUNIT_ASSERT(trx.accompanyPaxType().empty());
    CPPUNIT_ASSERT(trx.exchangePaxType() == 0);
    CPPUNIT_ASSERT(trx.exchangeItin().empty());

    // CPPUNIT_ASSERT(trx.fareApplicationDT() == DateTime.emptyDate());
    CPPUNIT_ASSERT(trx.fareApplicationDT().isEmptyDate() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == true);
  }
  void testSetExchangeAccompPaxType()
  {
    PaxType pax;
    trx.paxType().push_back(&pax);
    trx.accompanyPaxType().push_back(&pax);
    CPPUNIT_ASSERT(trx.paxType().front() == &pax);
    CPPUNIT_ASSERT(trx.accompanyPaxType().front() == &pax);
  }

  void testUnflownItineraryValidOneYearFromOriginalTicketDate()
  {
    ExcItin itin;
    AirSeg unflown = createUnflownSegmentStarting(2004, 7, 1);
    itin.travelSeg().push_back(&unflown);

    trx.exchangeItin().push_back(&itin);
    trx.setOriginalTktIssueDT() = DateTime(2004, 2, 29);
    trx.setTktValidityDate();

    DateTime dateAcceptable(2005, 2, 28);
    CPPUNIT_ASSERT_EQUAL(dateAcceptable, itin.tktValidityDate());
  }

  inline AirSeg createUnflownSegmentStarting(int year, int month, int day)
  {
    AirSeg segment;
    segment.unflown() = true;
    segment.departureDT() = DateTime(year, month, day);
    return segment;
  }

  inline AirSeg createFlownSegmentStarting(int year, int month, int day)
  {
    AirSeg segment;
    segment.unflown() = false;
    segment.departureDT() = DateTime(year, month, day);
    return segment;
  }

  void testPartialFlownItineraryValidOneYearFromFirstSegment()
  {
    ExcItin itin;

    AirSeg flownSeg = createFlownSegmentStarting(2006, 9, 1);
    AirSeg unflownSeg = createUnflownSegmentStarting(2007, 7, 1);
    itin.travelSeg().push_back(&flownSeg);
    itin.travelSeg().push_back(&unflownSeg);

    trx.exchangeItin().push_back(&itin);
    trx.setOriginalTktIssueDT() = DateTime(2006, 8, 9);
    trx.setTktValidityDate();

    DateTime expected(2007, 9, 1);
    CPPUNIT_ASSERT_EQUAL(expected, itin.tktValidityDate());
  }

  void testForFullyFlownItineraryValidityDateIsNotSet()
  {
    ExcItin itin;
    AirSeg flownSeg = createFlownSegmentStarting(2006, 9, 1);
    AirSeg flownSeg1 = createFlownSegmentStarting(2007, 3, 1);
    itin.travelSeg().push_back(&flownSeg);
    itin.travelSeg().push_back(&flownSeg1);

    trx.exchangeItin().push_back(&itin);
    trx.setOriginalTktIssueDT() = DateTime(2006, 8, 9);
    DateTime original = itin.tktValidityDate();
    trx.setTktValidityDate();

    CPPUNIT_ASSERT_EQUAL(original, itin.tktValidityDate());
  }

  void setCurrentTicketingDate(DateTime& dt)
  {
    trx.currentTicketingDT() = dt;
    trx.setRequest(&request);
  }

  void testTicketDateFromCurrentWhenAnalyzeExchangeItinNotSet()
  {
    bool isAnalyzingExcItin = false;
    DateTime currentTicketingDate = DateTime(2007, 5, 14);
    setCurrentTicketingDate(currentTicketingDate);
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    CPPUNIT_ASSERT_EQUAL(currentTicketingDate, trx.ticketingDate());
  }

  void testTicketDateFromOriginalWhenAnalyzeExchangeItinSet()
  {
    bool isAnalyzingExcItin = false;
    DateTime originalTicketingDate = DateTime(2006, 10, 5);
    trx.setOriginalTktIssueDT() = originalTicketingDate;
    isAnalyzingExcItin = true;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    CPPUNIT_ASSERT_EQUAL(originalTicketingDate, trx.ticketingDate());
  }

  void testSetFareApplicationDTWhenAnalysingNewItin()
  {
    bool isAnalyzingExcItin = false;
    DateTime originalTktIssueDT = DateTime(2007, 4, 10);
    DateTime currentTicketingDT = DateTime(2007, 5, 10);

    trx.setOriginalTktIssueDT() = originalTktIssueDT;
    trx.currentTicketingDT() = currentTicketingDT;

    isAnalyzingExcItin = false;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    DateTime fareApplicationDTHistorical = originalTktIssueDT;
    trx.setFareApplicationDT(fareApplicationDTHistorical);

    CPPUNIT_ASSERT(trx.isAnalyzingExcItin() == false);
    CPPUNIT_ASSERT(trx.fareApplicationDT().isEmptyDate() == false);
    CPPUNIT_ASSERT_EQUAL(fareApplicationDTHistorical, trx.fareApplicationDT());
  }

  void testSetFareApplicationDTWhenAnalysingExchangeItin()
  {
    bool isAnalyzingExcItin = false;
    DateTime originalTktIssueDT = DateTime(2007, 4, 10);
    trx.setOriginalTktIssueDT() = originalTktIssueDT;

    isAnalyzingExcItin = true;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    DateTime fareApplicationDTHistorical = originalTktIssueDT;
    trx.setFareApplicationDT(fareApplicationDTHistorical);

    CPPUNIT_ASSERT(trx.isAnalyzingExcItin() == true);
    CPPUNIT_ASSERT(trx.fareApplicationDT().isEmptyDate() == false);
    CPPUNIT_ASSERT_EQUAL(fareApplicationDTHistorical, trx.fareApplicationDT());
  }

  void testFareApplicationDTWhenAnalysingNewItin()
  {
    bool isAnalyzingExcItin = false;
    DateTime originalTktIssueDT = DateTime(2007, 4, 10);
    trx.setOriginalTktIssueDT() = originalTktIssueDT;

    DateTime currentTicketingDT = DateTime(2007, 5, 10);
    trx.currentTicketingDT() = currentTicketingDT;

    isAnalyzingExcItin = false;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    DateTime fareApplicationDTHistorical = originalTktIssueDT;
    trx.fareApplicationDT() = fareApplicationDTHistorical;

    CPPUNIT_ASSERT(trx.isAnalyzingExcItin() == false);
    CPPUNIT_ASSERT(trx.fareApplicationDT().isEmptyDate() == false);
    CPPUNIT_ASSERT_EQUAL(fareApplicationDTHistorical, trx.fareApplicationDT());

    DateTime fareApplicationDTCurrent = currentTicketingDT;
    trx.fareApplicationDT() = fareApplicationDTCurrent;
    CPPUNIT_ASSERT(trx.fareApplicationDT().isEmptyDate() == false);
    CPPUNIT_ASSERT_EQUAL(fareApplicationDTCurrent, trx.fareApplicationDT());
  }

  void testFareApplicationDTWhenAnalysingExchangeItin()
  {
    bool isAnalyzingExcItin = false;
    DateTime originalTktIssueDT = DateTime(2007, 4, 10);
    trx.setOriginalTktIssueDT() = originalTktIssueDT;

    DateTime currentTicketingDT = DateTime(2007, 5, 10);
    trx.currentTicketingDT() = currentTicketingDT;

    isAnalyzingExcItin = true;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    DateTime fareApplicationDTHistorical = originalTktIssueDT;
    trx.fareApplicationDT() = fareApplicationDTHistorical;

    CPPUNIT_ASSERT(trx.isAnalyzingExcItin() == true);
    CPPUNIT_ASSERT(trx.fareApplicationDT().isEmptyDate() == false);
    CPPUNIT_ASSERT_EQUAL(fareApplicationDTHistorical, trx.fareApplicationDT());

    DateTime fareApplicationDTCurrent = currentTicketingDT;
    trx.fareApplicationDT() = fareApplicationDTCurrent;
    CPPUNIT_ASSERT(trx.fareApplicationDT().isEmptyDate() == false);
    CPPUNIT_ASSERT_EQUAL(fareApplicationDTCurrent, trx.fareApplicationDT());
  }

  void testMarkFareRetrievalMethodHistorical()
  {
    trx.fareRetrievalFlags().setNull();

    trx.markFareRetrievalMethodHistorical();
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievHistorical) == true);

    trx.markFareRetrievalMethodHistorical(false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievHistorical) == false);
  }

  void testMarkFareRetrievalMethodKeep()
  {
    trx.fareRetrievalFlags().setNull();

    trx.markFareRetrievalMethodKeep();
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievKeep) == true);

    trx.markFareRetrievalMethodKeep(false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievKeep) == false);
  }

  void testMarkFareRetrievalMethodTvlCommence()
  {
    trx.fareRetrievalFlags().setNull();

    trx.markFareRetrievalMethodTvlCommence();
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievTvlCommence) == true);

    trx.markFareRetrievalMethodTvlCommence(false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievTvlCommence) == false);
  }

  void testMarkFareRetrievalMethodCurrent()
  {
    trx.fareRetrievalFlags().setNull();

    trx.markFareRetrievalMethodCurrent();
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievCurrent) == true);

    trx.markFareRetrievalMethodCurrent(false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievCurrent) == false);
  }

  void testMarkAllFareRetrievalMethod()
  {
    trx.fareRetrievalFlags().setNull();

    trx.markAllFareRetrievalMethod();
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievHistorical) == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievKeep) == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievTvlCommence) == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievCurrent) == true);

    trx.markAllFareRetrievalMethod(false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievHistorical) == false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievKeep) == false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievTvlCommence) == false);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievCurrent) == false);
  }

  void testNeedRetrieveHistoricalFare()
  {
    trx.fareRetrievalFlags().setNull();

    trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);
    trx.fareRetrievalFlags().set(FareMarket::RetrievKeep);
    trx.fareRetrievalFlags().set(FareMarket::RetrievTvlCommence);
    trx.fareRetrievalFlags().set(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.needRetrieveHistoricalFare() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievHistorical) == true);
  }

  void testNeedRetrieveKeepFare()
  {
    trx.fareRetrievalFlags().setNull();

    trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);
    trx.fareRetrievalFlags().set(FareMarket::RetrievKeep);
    trx.fareRetrievalFlags().set(FareMarket::RetrievTvlCommence);
    trx.fareRetrievalFlags().set(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.needRetrieveKeepFare() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievKeep) == true);
  }

  void testNeedRetrieveTvlCommenceFare()
  {
    trx.fareRetrievalFlags().setNull();

    trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);
    trx.fareRetrievalFlags().set(FareMarket::RetrievKeep);
    trx.fareRetrievalFlags().set(FareMarket::RetrievTvlCommence);
    trx.fareRetrievalFlags().set(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.needRetrieveTvlCommenceFare() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievTvlCommence) == true);
  }

  void testNeedRetrieveCurrentFare()
  {
    trx.fareRetrievalFlags().setNull();

    trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);
    trx.fareRetrievalFlags().set(FareMarket::RetrievKeep);
    trx.fareRetrievalFlags().set(FareMarket::RetrievTvlCommence);
    trx.fareRetrievalFlags().set(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.needRetrieveCurrentFare() == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievCurrent) == true);
  }

  void testNeedRetrieveAllFare()
  {
    trx.fareRetrievalFlags().setNull();

    trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);
    trx.fareRetrievalFlags().set(FareMarket::RetrievKeep);
    trx.fareRetrievalFlags().set(FareMarket::RetrievTvlCommence);
    trx.fareRetrievalFlags().set(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isNull() == false);
    CPPUNIT_ASSERT(trx.needRetrieveAllFare() == true);

    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievHistorical) == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievKeep) == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievTvlCommence) == true);
    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isSet(FareMarket::RetrievCurrent) == true);

    CPPUNIT_ASSERT(trx.fareRetrievalFlags().isAllSet((FareMarket::FareRetrievalFlags)(
                       FareMarket::RetrievHistorical | FareMarket::RetrievKeep |
                       FareMarket::RetrievTvlCommence | FareMarket::RetrievCurrent)) == true);
  }

  void testNeedRetrieveFare()
  {
    trx.fareRetrievalFlags().setNull();
    CPPUNIT_ASSERT(trx.needRetrieveFare() == false);

    trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);
    CPPUNIT_ASSERT(trx.needRetrieveFare() == true);

    trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical);
    trx.fareRetrievalFlags().set(FareMarket::RetrievKeep);
    trx.fareRetrievalFlags().set(FareMarket::RetrievTvlCommence);
    trx.fareRetrievalFlags().set(FareMarket::RetrievCurrent);
    CPPUNIT_ASSERT(trx.needRetrieveFare() == true);

    trx.fareRetrievalFlags().set(FareMarket::RetrievHistorical, false);
    trx.fareRetrievalFlags().set(FareMarket::RetrievKeep, false);
    trx.fareRetrievalFlags().set(FareMarket::RetrievTvlCommence, false);
    trx.fareRetrievalFlags().set(FareMarket::RetrievCurrent, false);
    CPPUNIT_ASSERT(trx.needRetrieveFare() == false);
  }

  void testTicketingDate()
  {
    bool isAnalyzingExcItin = false;
    DateTime originalTktIssueDT = DateTime(2007, 4, 10);
    DateTime currentTicketingDT = DateTime(2007, 5, 10);

    DateTime fareApplicationDTHistorical = originalTktIssueDT;
    DateTime fareApplicationDTCurrent = currentTicketingDT;
    DateTime fareApplicationDT = DateTime::emptyDate();

    trx.setOriginalTktIssueDT() = originalTktIssueDT;
    trx.currentTicketingDT() = currentTicketingDT;

    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    CPPUNIT_ASSERT(trx.ticketingDate() == trx.currentTicketingDT());
    CPPUNIT_ASSERT(trx.fareApplicationDT() == trx.currentTicketingDT());

    fareApplicationDT = fareApplicationDTHistorical;
    trx.setFareApplicationDT(fareApplicationDT);
    CPPUNIT_ASSERT(trx.ticketingDate() == trx.fareApplicationDT());

    fareApplicationDT = fareApplicationDTCurrent;
    trx.setFareApplicationDT(fareApplicationDT);
    CPPUNIT_ASSERT(trx.ticketingDate() == trx.fareApplicationDT());

    isAnalyzingExcItin = true;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    CPPUNIT_ASSERT(trx.ticketingDate() == trx.originalTktIssueDT());
  }

  void testSetAnalyzingExcItin()
  {
    bool isAnalyzingExcItin = false;
    ExcItin excItin;
    Itin newItin;

    trx.exchangeItin().push_back(&excItin);
    trx.newItin().push_back(&newItin);

    DateTime originalTktIssueDT = DateTime(2007, 4, 10);
    DateTime currentTicketingDT = DateTime(2007, 5, 10);

    DateTime todayDT = DateTime::localTime();
    DateTime fareApplicationDT;

    trx.setOriginalTktIssueDT() = originalTktIssueDT;
    trx.currentTicketingDT() = currentTicketingDT;

    trx.setFareApplicationDT(DateTime::emptyDate());
    isAnalyzingExcItin = true;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    CPPUNIT_ASSERT((std::vector<Itin*>*)(&(trx.itin())) ==
                   (std::vector<Itin*>*)(&(trx.exchangeItin())));
    CPPUNIT_ASSERT(trx.isAnalyzingExcItin() == true);
    CPPUNIT_ASSERT(trx.dataHandle().ticketDate() == trx.originalTktIssueDT());
    CPPUNIT_ASSERT(trx.fareApplicationDT() == trx.originalTktIssueDT());

    fareApplicationDT = todayDT;
    trx.setFareApplicationDT(fareApplicationDT);
    isAnalyzingExcItin = true;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    CPPUNIT_ASSERT((std::vector<Itin*>*)(&(trx.itin())) ==
                   (std::vector<Itin*>*)(&(trx.exchangeItin())));
    CPPUNIT_ASSERT(trx.isAnalyzingExcItin() == true);
    CPPUNIT_ASSERT(trx.dataHandle().ticketDate() == trx.originalTktIssueDT());
    CPPUNIT_ASSERT(trx.fareApplicationDT() == trx.originalTktIssueDT());

    trx.setFareApplicationDT(DateTime::emptyDate());
    isAnalyzingExcItin = false;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    CPPUNIT_ASSERT(&(trx.itin()) == &(trx.newItin()));
    CPPUNIT_ASSERT(trx.isAnalyzingExcItin() == false);
    CPPUNIT_ASSERT(trx.dataHandle().ticketDate() == trx.currentTicketingDT());
    CPPUNIT_ASSERT(trx.fareApplicationDT() == trx.currentTicketingDT());

    trx.setFareApplicationDT(todayDT);
    isAnalyzingExcItin = false;
    trx.setAnalyzingExcItin(isAnalyzingExcItin);
    CPPUNIT_ASSERT(&(trx.itin()) == &(trx.newItin()));
    CPPUNIT_ASSERT(trx.isAnalyzingExcItin() == false);
    CPPUNIT_ASSERT(trx.dataHandle().ticketDate() == trx.currentTicketingDT());
    CPPUNIT_ASSERT(trx.fareApplicationDT() == trx.currentTicketingDT());
  }

  void testBuildFlownSegRetrievalMap()
  {
    AirSeg flownSeg1 = createFlownSegmentStarting(2008, 9, 1);
    flownSeg1.segmentOrder() = 1;
    AirSeg flownSeg2 = createFlownSegmentStarting(2008, 9, 5);
    flownSeg2.segmentOrder() = 2;

    FareMarket flownFm;
    flownFm.travelSeg().push_back(&flownSeg1);
    flownFm.travelSeg().push_back(&flownSeg2);

    FareMarket::FareRetrievalFlags flag = FareMarket::RetrievNone;
    trx.excFlownFcFareAppl().insert(std::make_pair(&flownFm, flag));

    std::map<int16_t, FareMarket::FareRetrievalFlags> segOrderFlagMap;
    trx.getFlownSegRetrievalMap(segOrderFlagMap);
    CPPUNIT_ASSERT(2 == segOrderFlagMap.size());
    CPPUNIT_ASSERT(1 == segOrderFlagMap.find(1)->first);
    CPPUNIT_ASSERT((FareMarket::FareRetrievalFlags)FareMarket::RetrievNone ==
                   segOrderFlagMap.find(1)->second);
  }

  void helperConfigAdjustTravelDate(const char value)
  {
    trx._dataHandle.setTicketDate(DateTime(2008, 8, 10));
    TestConfigInitializer::setValue("ADJUST_REX_TRAVEL_DATE", value, "REX_FARE_SELECTOR_SVC", true);
    (value == 'Y') ? CPPUNIT_ASSERT(TrxUtil::isAdjustRexTravelDateEnabled())
                   : CPPUNIT_ASSERT(!TrxUtil::isAdjustRexTravelDateEnabled());
  }

  void testSetAdjustedTravelDate_N()
  {
    helperConfigAdjustTravelDate('N');
    trx._travelDate = DateTime(2008, 8, 8);
    trx.setAdjustedTravelDate();
    CPPUNIT_ASSERT_EQUAL(DateTime(2008, 8, 8), trx._adjustedTravelDate);
  }

  void testSetAdjustedTravelDate_Y_HasUnflown()
  {
    helperConfigAdjustTravelDate('Y');
    trx._currentTicketingDT = DateTime(2008, 8, 8);
    AirSeg* as1 = new AirSeg;
    as1->departureDT() = DateTime(2008, 8, 9);
    trx._travelSeg.push_back(as1);
    trx.setAdjustedTravelDate();
    CPPUNIT_ASSERT_EQUAL(DateTime(2008, 8, 9), trx._adjustedTravelDate);
    delete as1;
  }

  void testSetAdjustedTravelDate_Y()
  {
    helperConfigAdjustTravelDate('Y');
    trx._currentTicketingDT = DateTime(2008, 8, 8);
    trx.setAdjustedTravelDate();
    CPPUNIT_ASSERT_EQUAL(DateTime(2008, 8, 8), trx._adjustedTravelDate);
  }

  void testAdjustedTravelDate_Y()
  {
    helperConfigAdjustTravelDate('Y');
    DateTime travelDate(2008, 8, 8);
    CPPUNIT_ASSERT_EQUAL(DateTime(2008, 8, 10), trx.adjustedTravelDate(travelDate));
  }

  void testAdjustedTravelDate_N()
  {
    helperConfigAdjustTravelDate('N');
    DateTime travelDate(2008, 8, 8);
    CPPUNIT_ASSERT_EQUAL(DateTime(2008, 8, 8), trx.adjustedTravelDate(travelDate));
  }

  void testTravelDate()
  {
    helperConfigAdjustTravelDate('Y');
    trx._adjustedTravelDate = DateTime(2008, 8, 8);
    CPPUNIT_ASSERT_EQUAL(DateTime(2008, 8, 10), trx.travelDate());
  }

  void helperSetupDate(const DateTime& orig, const DateTime& comm, const DateTime& reis)
  {
    trx.exchangeItin()[0]->travelCommenceDate() = comm;
    trx._originalTktIssueDT = orig;
    trx.lastTktReIssueDT() = reis;
  }
  void helperFillSeqOrig(std::vector<RexPricingTrx::DateSeqType>& seq, const DateTime& date)
  {
    seq.push_back(RexPricingTrx::DateSeqType(ONE_DAY_BEFORE_ORIG_TKT_DATE, date.subtractDays(1)));
    seq.push_back(RexPricingTrx::DateSeqType(TWO_DAYS_BEFORE_ORIG_TKT_DATE, date.subtractDays(2)));
    seq.push_back(RexPricingTrx::DateSeqType(ONE_DAY_AFTER_ORIG_TKT_DATE, date.addDays(1)));
    seq.push_back(RexPricingTrx::DateSeqType(TWO_DAYS_AFTER_ORIG_TKT_DATE, date.addDays(2)));
  }

  void helperFillSeqComm(std::vector<RexPricingTrx::DateSeqType>& seq, const DateTime& date)
  {
    seq.push_back(RexPricingTrx::DateSeqType(ONE_DAY_BEFORE_COMMENCE_DATE, date.subtractDays(1)));
    seq.push_back(RexPricingTrx::DateSeqType(TWO_DAYS_BEFORE_COMMENCE_DATE, date.subtractDays(2)));
    seq.push_back(RexPricingTrx::DateSeqType(ONE_DAY_AFTER_COMMENCE_DATE, date.addDays(1)));
    seq.push_back(RexPricingTrx::DateSeqType(TWO_DAYS_AFTER_COMMENCE_DATE, date.addDays(2)));
  }

  void helperFillSeqReis(std::vector<RexPricingTrx::DateSeqType>& seq, const DateTime& date)
  {
    seq.push_back(
        RexPricingTrx::DateSeqType(ONE_DAY_BEFORE_REISSUE_TKT_DATE, date.subtractDays(1)));
    seq.push_back(
        RexPricingTrx::DateSeqType(TWO_DAYS_BEFORE_REISSUE_TKT_DATE, date.subtractDays(2)));
    seq.push_back(RexPricingTrx::DateSeqType(ONE_DAY_AFTER_REISSUE_TKT_DATE, date.addDays(1)));
    seq.push_back(RexPricingTrx::DateSeqType(TWO_DAYS_AFTER_REISSUE_TKT_DATE, date.addDays(2)));
  }

  void testSetupDateSeq_O()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime::emptyDate(),
             reis = DateTime::emptyDate();

    helperSetupDate(orig, comm, reis);
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    helperFillSeqOrig(seq, orig);
    trx.setupDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testSetupDateSeq_OC()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 12),
             reis = DateTime::emptyDate();

    helperSetupDate(orig, comm, reis);
    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    seq.push_back(RexPricingTrx::DateSeqType(COMMENCE_DATE, comm));
    helperFillSeqOrig(seq, orig);
    helperFillSeqComm(seq, comm);
    trx.setupDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testSetupDateSeq_OR()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime::emptyDate(),
             reis = DateTime(2007, 4, 12);

    helperSetupDate(orig, comm, reis);
    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    seq.push_back(RexPricingTrx::DateSeqType(REISSUE_TICKET_DATE, reis));
    helperFillSeqOrig(seq, orig);
    helperFillSeqReis(seq, reis);
    trx.setupDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testSetupDateSeq_OCR()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 12),
             reis = DateTime(2007, 4, 14);

    helperSetupDate(orig, comm, reis);
    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    seq.push_back(RexPricingTrx::DateSeqType(COMMENCE_DATE, comm));
    seq.push_back(RexPricingTrx::DateSeqType(REISSUE_TICKET_DATE, reis));
    helperFillSeqOrig(seq, orig);
    helperFillSeqComm(seq, comm);
    helperFillSeqReis(seq, reis);
    trx.setupDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testSetupDateSeq_ORC()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 14),
             reis = DateTime(2007, 4, 12);

    helperSetupDate(orig, comm, reis);
    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    seq.push_back(RexPricingTrx::DateSeqType(REISSUE_TICKET_DATE, reis));
    seq.push_back(RexPricingTrx::DateSeqType(COMMENCE_DATE, comm));
    helperFillSeqOrig(seq, orig);
    helperFillSeqReis(seq, reis);
    helperFillSeqComm(seq, comm);
    trx.setupDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testSetupDateSeq_ORC2()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 8), reis = DateTime(2007, 4, 6);

    helperSetupDate(orig, comm, reis);
    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    seq.push_back(RexPricingTrx::DateSeqType(REISSUE_TICKET_DATE, reis));
    seq.push_back(RexPricingTrx::DateSeqType(COMMENCE_DATE, comm));
    helperFillSeqOrig(seq, orig);
    helperFillSeqReis(seq, reis);
    helperFillSeqComm(seq, comm);
    trx.setupDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testSetupDateSeq_O2()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 10),
             reis = DateTime(2007, 4, 10);

    helperSetupDate(orig, comm, reis);
    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    helperFillSeqOrig(seq, orig);
    trx.setupDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testSetupDateSeq_OR2()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 14),
             reis = DateTime(2007, 4, 14);

    helperSetupDate(orig, comm, reis);
    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    seq.push_back(RexPricingTrx::DateSeqType(REISSUE_TICKET_DATE, reis));
    helperFillSeqOrig(seq, orig);
    helperFillSeqReis(seq, reis);
    trx.setupDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testRSVStatus()
  {
    // RepriceSolutionValidator newer run
    CPPUNIT_ASSERT_EQUAL(false, trx.rsvCounterStatus()); // total:0 pass:0

    trx.incrementRSVCounter(false);
    CPPUNIT_ASSERT_EQUAL(true, trx.rsvCounterStatus()); // total:1 pass:0

    trx.incrementRSVCounter(true);
    CPPUNIT_ASSERT_EQUAL(false, trx.rsvCounterStatus()); // total:2 pass:1
  }

  void testSetActionCodeForRexPricingRedirectedToPortExchange()
  {
    Billing billing;
    std::string actionCode = "WFR";
    trx.billing() = &billing;
    trx.billing()->actionCode() = actionCode;
    trx.redirected() = true;
    trx.secondaryExcReqType() = FULL_EXCHANGE;

    trx.setActionCode();

    CPPUNIT_ASSERT_EQUAL(actionCode + FULL_EXCHANGE, trx.billing()->actionCode());
  }

  void helperSetUpRexSkipSecurity(const std::string& emptyMeansAirline)
  {
    _trx->setRequest(_memHandle.insert(new PricingRequest));
    _trx->getRequest()->ticketingAgent() = _memHandle.insert(new Agent);
    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = emptyMeansAirline;

    CPPUNIT_ASSERT(!_trx->skipSecurityForExcItin());
    _trx->setUpSkipSecurityForExcItin();
    CPPUNIT_ASSERT(_trx->skipSecurityForExcItin());
  }

  void testSetUpRexSkipSecurity_Airline()
  {
    TestConfigInitializer::setValue(
        "SKIP_CAT_31_AIRLINE_SECURITY_CHECK", "Y", "REX_FARE_SELECTOR_SVC");
    helperSetUpRexSkipSecurity("");
  }
  void testSetUpRexSkipSecurity_Subscriber()
  {
    TestConfigInitializer::setValue(
        "SKIP_CAT_31_SUBSCRIBER_SECURITY_CHECK", "Y", "REX_FARE_SELECTOR_SVC");
    helperSetUpRexSkipSecurity("ZYW");
  }
  void helperSetupFCInfo(DateTime& fcDate)
  {
    FareComponentInfo* fareComponentInfo = new FareComponentInfo;
    VCTRInfo* vctrInfo = new VCTRInfo;
    vctrInfo->retrievalDate() = fcDate;
    fareComponentInfo->vctrInfo() = vctrInfo;
    trx.excFareCompInfo().push_back(fareComponentInfo);
  }
  void testSetupMipDateSeq_fcDateEqOrig()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 10),
             reis = DateTime(2007, 4, 10), fc1Date = DateTime(2007, 4, 10);

    helperSetupDate(orig, comm, reis);
    helperSetupFCInfo(fc1Date);

    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    helperFillSeqOrig(seq, orig);
    trx.setupDateSeq();
    trx.setupMipDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }
  void testSetupMipDateSeq_fcDateDiffThanOrig()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 10),
             reis = DateTime(2007, 4, 10), fc1Date = DateTime(2007, 4, 11);

    helperSetupDate(orig, comm, reis);
    helperSetupFCInfo(fc1Date);

    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    seq.push_back(RexPricingTrx::DateSeqType(FARE_COMPONENT_DATE, fc1Date));
    helperFillSeqOrig(seq, orig);
    trx.setupDateSeq();
    trx.setupMipDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }
  void testSetupMipDateSeq_fcDateEmpty()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 10),
             reis = DateTime(2007, 4, 10), fc1Date = DateTime::emptyDate();

    helperSetupDate(orig, comm, reis);
    helperSetupFCInfo(fc1Date);

    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    helperFillSeqOrig(seq, orig);
    trx.setupDateSeq();
    trx.setupMipDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }
  void testSetupMipDateSeq_twoFcDates()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 10),
             reis = DateTime(2007, 4, 10), fc1Date = DateTime(2007, 4, 11),
             fc2Date = DateTime(2007, 4, 12);

    helperSetupDate(orig, comm, reis);
    helperSetupFCInfo(fc1Date);
    helperSetupFCInfo(fc2Date);

    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    seq.push_back(RexPricingTrx::DateSeqType(FARE_COMPONENT_DATE, fc1Date));
    seq.push_back(RexPricingTrx::DateSeqType(FARE_COMPONENT_DATE, fc2Date));
    helperFillSeqOrig(seq, orig);
    trx.setupDateSeq();
    trx.setupMipDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testSetupMipDateSeq_twoFcDatesEqual()
  {
    ExcItin excItin;
    trx.exchangeItin().push_back(&excItin);
    std::vector<RexPricingTrx::DateSeqType> seq;

    DateTime orig = DateTime(2007, 4, 10), comm = DateTime(2007, 4, 10),
             reis = DateTime(2007, 4, 10), fc1Date = DateTime(2007, 4, 11),
             fc2Date = DateTime(2007, 4, 11);

    helperSetupDate(orig, comm, reis);
    helperSetupFCInfo(fc1Date);
    helperSetupFCInfo(fc2Date);

    seq.clear();
    seq.push_back(RexPricingTrx::DateSeqType(ORIGINAL_TICKET_DATE, orig));
    seq.push_back(RexPricingTrx::DateSeqType(FARE_COMPONENT_DATE, fc1Date));
    helperFillSeqOrig(seq, orig);
    trx.setupDateSeq();
    trx.setupMipDateSeq();

    CPPUNIT_ASSERT_EQUAL(seq, trx._tktDateSeq);
  }

  void testTktValidityDateWhenPreviousExchDateAndNOExchReissueSetOriginTktDate()
  {
    ExcItin itin;
    AirSeg unflown = createUnflownSegmentStarting(2004, 7, 1);
    itin.travelSeg().push_back(&unflown);
    trx.exchangeItin().push_back(&itin);
    trx.setOriginalTktIssueDT() = DateTime(2009, 1, 2);
    trx.previousExchangeDT() = DateTime(2009, 1, 10);
    trx.setTktValidityDate();
    DateTime dateAcceptable(2010, 1, 2);
    CPPUNIT_ASSERT_EQUAL(dateAcceptable, itin.tktValidityDate());
  }

  void testTktValidityDateWhenPreviousExchDateAndExchReissueSetPrevExchDate()
  {
    ExcItin itin;
    AirSeg unflown = createUnflownSegmentStarting(2004, 7, 1);
    itin.travelSeg().push_back(&unflown);
    trx.exchangeItin().push_back(&itin);
    trx.setOriginalTktIssueDT() = DateTime(2009, 1, 2);
    trx.previousExchangeDT() = DateTime(2009, 1, 10);
    trx.setRexPrimaryProcessType('A');
    trx.setTktValidityDate();
    DateTime dateAcceptable(2010, 1, 10);
    CPPUNIT_ASSERT_EQUAL(dateAcceptable, itin.tktValidityDate());
  }

  void testNewItinSecondROEConversionDateGetsInitializedToEmptyDate()
  {
    CPPUNIT_ASSERT(trx.newItinSecondROEConversionDate().isEmptyDate());
    CPPUNIT_ASSERT(!trx.useSecondROEConversionDate());
  }

  void testNewItinSecondROEConversionDateGettterSetter()
  {
    DateTime testDate(2010, 3, 5);
    trx.newItinSecondROEConversionDate() = testDate;
    CPPUNIT_ASSERT_EQUAL(testDate, trx.newItinSecondROEConversionDate());
    trx.useSecondROEConversionDate() = true;
    CPPUNIT_ASSERT(trx.useSecondROEConversionDate());
  }

  std::pair<FarePath*, FarePath*> getSolutions(MoneyAmount rebook, MoneyAmount book)
  {
    std::pair<FarePath*, FarePath*> rebookBook =
        std::make_pair(static_cast<FarePath*>(0), static_cast<FarePath*>(0));

    if (rebook > EPSILON)
    {
      rebookBook.first = _memHandle(new FarePath);
      rebookBook.first->setTotalNUCAmount(rebook);
      rebookBook.first->rebookClassesExists() = true;
    }

    if (book > EPSILON)
    {
      rebookBook.second = _memHandle(new FarePath);
      rebookBook.second->setTotalNUCAmount(book);
      rebookBook.second->rebookClassesExists() = false;
    }

    return rebookBook;
  }

  std::pair<FarePath*, FarePath*> fillExistingSolutions(MoneyAmount rebook, MoneyAmount book)
  {
    std::pair<FarePath*, FarePath*> existing = getSolutions(rebook, book);

    if (existing.first)
      _trx->newItin().front()->farePath().push_back(existing.first);

    if (existing.second)
      _trx->newItin().front()->farePath().push_back(existing.second);

    return existing;
  }

  void testValidSolutionsPlacement1()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 0.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT(_trx->newItin().front()->farePath().empty());
  }
  void testValidSolutionsPlacement2()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(_trx->newItin().front()->farePath().front(), rebookBook.second);
    CPPUNIT_ASSERT_EQUAL(one, _trx->newItin().front()->farePath().size());
  }
  void testValidSolutionsPlacement3()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(1.0, 0.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(_trx->newItin().front()->farePath().front(), rebookBook.first);
    CPPUNIT_ASSERT_EQUAL(one, _trx->newItin().front()->farePath().size());
  }
  void testValidSolutionsPlacement4()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(1.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(_trx->newItin().front()->farePath().front(), rebookBook.first);
    CPPUNIT_ASSERT_EQUAL(_trx->newItin().front()->farePath().back(), rebookBook.second);
  }
  void testValidSolutionsPlacement5()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 0.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(1.0, 0.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(one, _trx->newItin().front()->farePath().size());
  }
  void testValidSolutionsPlacement6()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 0.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(0.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(one, _trx->newItin().front()->farePath().size());
  }
  void testValidSolutionsPlacement7()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 0.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(1.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement8()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(1.0, 0.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(rebookBook.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(one, _trx->newItin().front()->farePath().size());
  }
  void testValidSolutionsPlacement9()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(1.0, 0.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(0.0, 2.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(rebookBook.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement10()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(1.0, 0.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(2.0, 2.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(rebookBook.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement11()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 1.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(2.0, 0.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(rebookBook.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement12()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(rebookBook.second, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(one, _trx->newItin().front()->farePath().size());
  }
  void testValidSolutionsPlacement13()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 1.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(2.0, 2.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(rebookBook.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement14()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(1.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(rebookBook.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(rebookBook.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement15()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(1.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(rebookBook.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(rebookBook.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement16()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(1.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(rebookBook.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(rebookBook.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement17()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(2.0, 0.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(1.0, 0.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(one, _trx->newItin().front()->farePath().size());
  }
  void testValidSolutionsPlacement18()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(2.0, 0.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(0.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(rebookBook.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement19()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(2.0, 0.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(1.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement20()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 2.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(1.0, 0.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(rebookBook.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement21()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 2.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(0.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(one, _trx->newItin().front()->farePath().size());
  }
  void testValidSolutionsPlacement22()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(0.0, 2.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(1.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement23()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(2.0, 2.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(1.0, 0.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(rebookBook.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement24()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(2.0, 2.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(0.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(rebookBook.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().back());
  }
  void testValidSolutionsPlacement25()
  {
    std::pair<FarePath*, FarePath*> rebookBook = getSolutions(2.0, 2.0);
    std::pair<FarePath*, FarePath*> existing = fillExistingSolutions(1.0, 1.0);

    _trx->validSolutionsPlacement(rebookBook);

    CPPUNIT_ASSERT_EQUAL(existing.first, _trx->newItin().front()->farePath().front());
    CPPUNIT_ASSERT_EQUAL(existing.second, _trx->newItin().front()->farePath().back());
  }

  void AddProcessTag(ProcessTagPermutation& perm, ProcessTag tag)
  {
    ProcessTagInfo* pInfo = _memHandle(new ProcessTagInfo);
    ReissueSequence* seq = _memHandle(new ReissueSequence);
    pInfo->reissueSequence()->orig() = seq;
    seq->processingInd() = tag;
    perm.processTags().push_back(pInfo);
  }

  void addPermutation(const std::vector<ProcessTag>& ptis)
  {
    _trx->processTagPermutations().push_back(_memHandle(new ProcessTagPermutation));

    std::vector<ProcessTag>::const_iterator pti = ptis.begin();
    std::vector<ProcessTag>::const_iterator ptie = ptis.end();
    for (; pti != ptie; ++pti)
      AddProcessTag(*_trx->processTagPermutations().back(), *pti);
  }

  void testDivideOne()
  {
    addPermutation(boost::assign::list_of(KEEP_THE_FARES)(KEEP_THE_FARES));

    CPPUNIT_ASSERT_EQUAL(RexPricingTrx::NONE, _trx->tag1PricingSvcCallStatus());
    CPPUNIT_ASSERT_EQUAL(onep, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(!_trx->separateTag7Permutations());
    CPPUNIT_ASSERT_EQUAL(onep, _trx->processTagPermutations().size());
  }
  void testDivideMixed7()
  {
    addPermutation(boost::assign::list_of(KEEP_THE_FARES));
    addPermutation(boost::assign::list_of(REISSUE_DOWN_TO_LOWER_FARE));

    CPPUNIT_ASSERT_EQUAL(RexPricingTrx::NONE, _trx->tag1PricingSvcCallStatus());
    CPPUNIT_ASSERT_EQUAL(two, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(_trx->processTagPermutations().back()->hasTag7only());
    CPPUNIT_ASSERT(_trx->separateTag7Permutations());
    CPPUNIT_ASSERT_EQUAL(RexPricingTrx::TAG7PERMUTATION, _trx->tag1PricingSvcCallStatus());
    CPPUNIT_ASSERT_EQUAL(onep, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(_trx->processTagPermutations().front()->hasTag7only());
    _trx->_tag1PricingSvcCallStatus = RexPricingTrx::NONE;
    CPPUNIT_ASSERT_EQUAL(two, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(!_trx->processTagPermutations().front()->hasTag7only());
  }
  void testDivideMixedK7()
  {
    addPermutation(boost::assign::list_of(KEEP_THE_FARES)(REISSUE_DOWN_TO_LOWER_FARE));
    addPermutation(boost::assign::list_of(REISSUE_DOWN_TO_LOWER_FARE)(KEEP_THE_FARES));

    CPPUNIT_ASSERT_EQUAL(RexPricingTrx::NONE, _trx->tag1PricingSvcCallStatus());
    CPPUNIT_ASSERT_EQUAL(two, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(_trx->processTagPermutations().back()->hasTag7only());
    CPPUNIT_ASSERT(_trx->separateTag7Permutations());
    CPPUNIT_ASSERT_EQUAL(RexPricingTrx::TAG7PERMUTATION, _trx->tag1PricingSvcCallStatus());
    CPPUNIT_ASSERT_EQUAL(onep, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(_trx->processTagPermutations().front()->hasTag7only());
    _trx->_tag1PricingSvcCallStatus = RexPricingTrx::NONE;
    CPPUNIT_ASSERT_EQUAL(two, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(!_trx->processTagPermutations().front()->hasTag7only());
  }
  void testDivideMixed4KX79()
  {
    addPermutation(boost::assign::list_of(KEEP_THE_FARES)(KEEP_FARES_FOR_UNCHANGED_FC));
    addPermutation(boost::assign::list_of(REISSUE_DOWN_TO_LOWER_FARE)(REISSUE_DOWN_TO_LOWER_FARE));
    addPermutation(
        boost::assign::list_of(HISTORICAL_FARES_FOR_TRAVELED_FC)(REISSUE_DOWN_TO_LOWER_FARE));

    CPPUNIT_ASSERT_EQUAL(RexPricingTrx::NONE, _trx->tag1PricingSvcCallStatus());
    CPPUNIT_ASSERT_EQUAL(three, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(_trx->processTagPermutations()[1]->hasTag7only());
    CPPUNIT_ASSERT(_trx->separateTag7Permutations());
    CPPUNIT_ASSERT_EQUAL(RexPricingTrx::TAG7PERMUTATION, _trx->tag1PricingSvcCallStatus());
    CPPUNIT_ASSERT_EQUAL(onep, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(_trx->processTagPermutations().front()->hasTag7only());
    _trx->_tag1PricingSvcCallStatus = RexPricingTrx::NONE;
    CPPUNIT_ASSERT_EQUAL(three, _trx->processTagPermutations().size());
    CPPUNIT_ASSERT(!_trx->processTagPermutations().front()->hasTag7only());
    CPPUNIT_ASSERT(!_trx->processTagPermutations().back()->hasTag7only());
  }

  FareMarket::RetrievalInfo* createRetrievalInfo(FareMarket::FareRetrievalFlags flag)
  {
    FareMarket::RetrievalInfo* info = _memHandle.insert(new FareMarket::RetrievalInfo);
    info->_flag = flag;
    return info;
  }

  void testUseHistoricalRoeDateExchangeMixedFares()
  {
    RexPricingTrx trx;
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    fp.exchangeReissue() = EXCHANGE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievCurrent);
    ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
    CPPUNIT_ASSERT(!roeSetter.useHistoricalRoeDate());
  }

  void testUseHistoricalRoeDateExchangeHistoricalFares()
  {
    RexPricingTrx trx;
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    fp.exchangeReissue() = EXCHANGE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
    CPPUNIT_ASSERT(roeSetter.useHistoricalRoeDate());
  }

  void testUseHistoricalRoeDateExchangeCurrentFares()
  {
    RexPricingTrx trx;
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    fp.exchangeReissue() = EXCHANGE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievCurrent);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievCurrent);
    ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
    CPPUNIT_ASSERT(!roeSetter.useHistoricalRoeDate());
  }

  void testUseHistoricalRoeDateReissueMixedFares()
  {
    RexPricingTrx trx;
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    fp.exchangeReissue() = REISSUE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievCurrent);
    ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
    CPPUNIT_ASSERT(roeSetter.useHistoricalRoeDate());
  }

  void testUseHistoricalRoeDateReissueHistoricalFares()
  {
    RexPricingTrx trx;
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    fp.exchangeReissue() = REISSUE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
    CPPUNIT_ASSERT(roeSetter.useHistoricalRoeDate());
  }

  void testUseHistoricalRoeDateReissueCurrentFares()
  {
    RexPricingTrx trx;
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    fp.exchangeReissue() = REISSUE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievCurrent);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievCurrent);
    ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
    CPPUNIT_ASSERT(!roeSetter.useHistoricalRoeDate());
  }

  void testROEDateSetterTurnOnSecondDateIndicatorWhenFirstROEDateNotHistorical()
  {
    RexPricingTrx trx;
    trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    trx.setRexPrimaryProcessType('A');
    trx.setOriginalTktIssueDT() = DateTime(2009, 12, 12);
    trx.newItinROEConversionDate() = DateTime(2010, 3, 10);
    trx.newItinSecondROEConversionDate() = DateTime(2010, 3, 12);
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    // fp.exchangeReissue() = EXCHANGE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    {
      ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
      CPPUNIT_ASSERT(trx.useSecondROEConversionDate());
      CPPUNIT_ASSERT(fp.useSecondRoeDate());
    }
    // following is to test that ROEDateSetter destructor changed flag to false
    CPPUNIT_ASSERT(!trx.useSecondROEConversionDate());
    CPPUNIT_ASSERT(fp.useSecondRoeDate());
  }

  void testROEDateSetterDontTurnOnSecondDateIndicatorWhenFirstROEDateIsHistorical()
  {
    RexPricingTrx trx;
    trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    trx.setRexPrimaryProcessType('A');
    trx.setOriginalTktIssueDT() = DateTime(2009, 12, 12);
    trx.newItinROEConversionDate() = trx.originalTktIssueDT();
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    fp.exchangeReissue() = EXCHANGE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
    CPPUNIT_ASSERT(!trx.useSecondROEConversionDate());
    CPPUNIT_ASSERT(!fp.useSecondRoeDate());
  }

  void testROEDateSetterTurnOnSecondDateIndicatorWhenFirstROEDateNotCurrent()
  {
    RexPricingTrx trx;
    trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    trx.setRexPrimaryProcessType('A');
    trx.currentTicketingDT() = DateTime(2009, 12, 12);
    trx.newItinROEConversionDate() = DateTime(2009, 12, 13);
    trx.newItinSecondROEConversionDate() = DateTime(2010, 3, 12);
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    fp.exchangeReissue() = EXCHANGE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievCurrent);
    {
      ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
      CPPUNIT_ASSERT(trx.useSecondROEConversionDate());
      CPPUNIT_ASSERT(fp.useSecondRoeDate());
    }
    // follwing is to test that ROEDateSetter destructor changed flag to false
    CPPUNIT_ASSERT(!trx.useSecondROEConversionDate());
    CPPUNIT_ASSERT(fp.useSecondRoeDate());
  }

  void testROEDateSetterDontTurnOnSecondDateIndicatorWhenFirstROEDateCurrent()
  {
    RexPricingTrx trx;
    trx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    trx.setRexPrimaryProcessType('A');
    trx.currentTicketingDT() = DateTime(2009, 12, 12);
    trx.newItinROEConversionDate() = trx.currentTicketingDT();
    FarePath fp;
    fp.pricingUnit().push_back(_pu1);
    fp.exchangeReissue() = EXCHANGE;
    _ptf11->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievHistorical);
    _ptf12->retrievalInfo() = createRetrievalInfo(FareMarket::RetrievCurrent);
    ROEDateSetter roeSetter = ROEDateSetter(trx, fp);
    CPPUNIT_ASSERT(!trx.useSecondROEConversionDate());
    CPPUNIT_ASSERT(!fp.useSecondRoeDate());
  }
  void setupDataStructures(std::vector<PaxTypeFare*>& ptFares, PricingTrx& trx, FareMarket& fm)
  {
    PaxType paxType;
    trx.paxType().push_back(&paxType);
    trx.paxType().push_back(&paxType);

    PaxTypeBucket ptCortage;
    ptCortage.paxTypeFare().swap(ptFares);
    fm.paxTypeCortege().push_back(ptCortage);
  }

  FareMarket* setupHistoricalFaresForHIP(RexPricingTrx& trx,
                                         std::vector<TravelSeg*>& intermediateSegs,
                                         std::vector<TravelSeg*>& parentSegs)
  {
    FareMarket* intermediateFM = _memHandle.create<FareMarket>();
    FareMarket* parentFM = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    setupDataStructures(ptFares, trx, *parentFM);

    PaxTypeFare fare;
    trx.newItinKeepFares()[&fare] = parentFM;

    intermediateFM->travelSeg() = intermediateSegs;
    parentFM->travelSeg() = parentSegs;

    trx.newItin() += itin;
    trx.newItin().front()->fareMarket() += intermediateFM, parentFM;

    return intermediateFM;
  }

  void testAreHistoricalFaresNeededForHIP_noParentMarket()
  {
    RexPricingTrx* trx = _memHandle.create<RexPricingTrx>();
    std::vector<TravelSeg*> intermediateSegs, parentSegs;

    AirSeg a, b, c, d;

    intermediateSegs += &b, &c;
    parentSegs += &a, &b, &d;

    FareMarket* intermediateFM = setupHistoricalFaresForHIP(*trx, intermediateSegs, parentSegs);

    intermediateFM->travelSeg().back()->stopOver() = true;

    CPPUNIT_ASSERT(!trx->isFareMarketNeededForMinFares(*intermediateFM));
  }

  void testAreHistoricalFaresNeededForHIP_success()
  {
    RexPricingTrx* trx = _memHandle.create<RexPricingTrx>();
    std::vector<TravelSeg*> intermediateSegs, parentSegs;

    AirSeg a, b, c, d;

    intermediateSegs += &b, &c;
    parentSegs += &a, &b, &c, &d;

    FareMarket* intermediateFM = setupHistoricalFaresForHIP(*trx, intermediateSegs, parentSegs);

    intermediateFM->travelSeg().back()->stopOver() = true;

    CPPUNIT_ASSERT(trx->isFareMarketNeededForMinFares(*intermediateFM));
  }

  void testAreHistoricalFaresNeededForHIP_noStopover()
  {
    RexPricingTrx* trx = _memHandle.create<RexPricingTrx>();
    std::vector<TravelSeg*> intermediateSegs, parentSegs;

    AirSeg a, b, c, d;

    intermediateSegs += &b, &c;
    parentSegs += &a, &b, &c, &d;

    FareMarket* intermediateFM = setupHistoricalFaresForHIP(*trx, intermediateSegs, parentSegs);

    trx->newItin().front()->travelSeg() += &a, &b, &c, &d;

    intermediateFM->travelSeg().back()->stopOver() = false;

    CPPUNIT_ASSERT(!trx->isFareMarketNeededForMinFares(*intermediateFM));
  }

  void testUtcAdjustementPrevious()
  {
    _trx->_previousExchangeDT = DateTime(2006, 6, 6, 6, 0, 1);
    _trx->_originalTktIssueDT = DateTime(2006, 6, 16, 6, 0, 1);
    DateTime dateAcceptable(2006, 6, 6, 6, 6, 1);
    CPPUNIT_ASSERT_EQUAL(dateAcceptable, _trx->adjustToCurrentUtcZone());
  }

  void testUtcAdjustementOriginal()
  {
    _trx->_originalTktIssueDT = DateTime(2006, 6, 6, 6, 0, 1);
    DateTime dateAcceptable(2006, 6, 6, 6, 6, 1);
    CPPUNIT_ASSERT_EQUAL(dateAcceptable, _trx->adjustToCurrentUtcZone());
  }

  void raiiRexFarePathPlusUpsCall(boost::tribool expectedNonRefundable)
  {
    _trx->trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
    {
      RaiiRexFarePathPlusUps rfppu(*_trx, *_fp, *_fpValid);
    }

    if (!boost::indeterminate(expectedNonRefundable))
      CPPUNIT_ASSERT_EQUAL(expectedNonRefundable, _trx->excTktNonRefundable());
    else
      CPPUNIT_ASSERT(boost::indeterminate(_trx->excTktNonRefundable()));
  }

  void testRRFPPUrefundable()
  {
    CPPUNIT_ASSERT(boost::indeterminate(_trx->excTktNonRefundable()));
    raiiRexFarePathPlusUpsCall(true);
  }

  void testRRFPPUnonRefundable()
  {
    CPPUNIT_ASSERT(boost::indeterminate(_trx->excTktNonRefundable()));

    _fu11->isNonRefundable() = true;
    _fu12->isNonRefundable() = true;

    raiiRexFarePathPlusUpsCall(true);
  }

  void testRRFPPUindeterminate()
  {
    CPPUNIT_ASSERT(boost::indeterminate(_trx->excTktNonRefundable()));

    *_fpValid = false;
    raiiRexFarePathPlusUpsCall(boost::indeterminate);
  }

  void testRRFPPUtrxState()
  {
    _trx->setExcTktNonRefundable(true);
    raiiRexFarePathPlusUpsCall(true);
  }

  void testRRFPPUplusUpNeededF()
  {
    _trx->trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
    _trx->setExcTktNonRefundable(true);
    RaiiRexFarePathPlusUps rfppu(*_trx, *_fp, *_fpValid);
    CPPUNIT_ASSERT(!rfppu.plusUpsNeeded());
  }

  void testRRFPPUplusUpNeededT()
  {
    _trx->setExcTrxType(PricingTrx::AF_EXC_TRX);
    _trx->trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
    RaiiRexFarePathPlusUps rfppu(*_trx, *_fp, *_fpValid);
    CPPUNIT_ASSERT(rfppu.plusUpsNeeded());
  }

  void addSingleFcExcItin(std::string fcaFT = "")
  {
    _trx->exchangeItin().push_back(new ExcItin);
    _trx->exchangeItin().front()->fareComponent().push_back(new FareCompInfo);
    _trx->exchangeItin().front()->fareComponent().front()->matchedFares().push_back(
        FareCompInfo::MatchedFare(_ptf11));
    FareClassAppInfo* fcai = _memHandle(new FareClassAppInfo);
    fcai->_fareType = fcaFT;
    _ptf11->fareClassAppInfo() = fcai;
  }

  void testSetExcTktNonRefundableNotMatched()
  {
    _trx->exchangeItin().push_back(new ExcItin);
    _trx->exchangeItin().front()->fareComponent().push_back(new FareCompInfo);
    CPPUNIT_ASSERT(boost::indeterminate(_trx->excTktNonRefundable()));
  }

protected:
  // below objects keeps his state between tests!!!
  RexPricingTrx trx;
  PricingRequest request;

  RexPricingTrx* _trx;
  TestMemHandle _memHandle;
  RexPricingRequest* _rexRequest;
  DataHandleMock* _dataHandleMock;
  // ROE migration

  PricingUnit* _pu1;
  FareUsage* _fu11;
  FareUsage* _fu12;
  PaxTypeFare* _ptf11;
  PaxTypeFare* _ptf12;

  FarePath* _fp;
  bool* _fpValid;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RexPricingTrxTest);
}
