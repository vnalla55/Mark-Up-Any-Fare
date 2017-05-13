#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/set.hpp>
#include "Rules/Tab988Merger.h"
#include "Rules/RuleUtil.h"
#include "DataModel/AirSeg.h"
#include "ItinAnalyzer/ExcItinUtil.h"
#include "DBAccess/GeoRuleItem.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/Agent.h"
#include "DBAccess/DST.h"
#include "test/include/CppUnitHelperMacros.h"
#include "Rules/test/TestCommon.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;
using namespace boost::assign;

namespace tse
{
class Diag986Collector;

class Tab988MergerTest : public CppUnit::TestFixture
{
  enum SeqByteIndicator
  {
    PORTION,
    FORCED_CNX,
    FIRST_BREAK,
    PORTION_OUTBOUND,
    FLIGHT_NO
  };

  static const Indicator NO_RESTR_1ST_BREAK = ' ';
  static const Indicator RESTR_1ST_BREAK = 'Y';

  static const Indicator NO_RESTRICTION = ' ';
  static const Indicator FIRST_FC = 'F';
  static const Indicator ORIG_TO_STOPOVER = 'O';

  CPPUNIT_TEST_SUITE(Tab988MergerTest);

  CPPUNIT_TEST(testMergePortion_EmptyReissueSequenceVect);
  CPPUNIT_TEST(testMergePortion_EmptyGeoTblItemNo);
  CPPUNIT_TEST(testMergePortion_SetGeoTblItemNo_EmptyPortionInd);
  CPPUNIT_TEST(testMergePortion_AllTraveSegmentsCanBeChanged);
  CPPUNIT_TEST(testMergePortion_FirstTraveSegmentCanNOTBeChanged);
  CPPUNIT_TEST(testMergePortion_FirstTraveSegmentCanNOTBeChanged2xSEQ);
  CPPUNIT_TEST(testMergePortion_FirstTraveSegmentCanNOTBeChanged3xSEQ);
  CPPUNIT_TEST(testMergePortion_FirstAndThirdTraveSegmentCanNOTBeChanged);
  CPPUNIT_TEST(testMergePortion_FirstTraveSegmentCanNOTBeChangedThirdCan1);
  CPPUNIT_TEST(testMergePortion_FirstTraveSegmentCanNOTBeChangedThirdCan2);
  CPPUNIT_TEST(testMergePortion_ThirdTraveSegmentCanNOTBeChangedFirstCan1);
  CPPUNIT_TEST(testMergePortion_ThirdTraveSegmentCanNOTBeChangedFirstCan2);
  CPPUNIT_TEST(testMergePortion_FirstTraveSegmentCanNOTBeChangedOnlyInFirstSEQ);
  CPPUNIT_TEST(testMergePortion_FirstTraveSegmentCanNOTBeChangedOnlyInSecondSEQ);
  CPPUNIT_TEST(testMergePortion_ThreeDiffCanNOTChange);
  CPPUNIT_TEST(testMergePortion_OneWithoutConstrains);
  CPPUNIT_TEST(testMergePortion_OneToMerge);

  CPPUNIT_TEST(testCollectForcedConnectionStopovers_NoRestriction);
  CPPUNIT_TEST(testCollectForcedConnectionStopovers_ForcedCnxOneSeq);
  CPPUNIT_TEST(testCollectForcedConnectionStopovers_ForcedStopoverOneSeq);
  CPPUNIT_TEST(testCollectForcedConnectionStopovers_ForcedCnxAndStopoverOneSeq);
  CPPUNIT_TEST(testCollectForcedConnectionStopovers_ForcedCnxForcedBoth);
  CPPUNIT_TEST(testCollectForcedConnectionStopovers_ForcedCnxAndNoRestriction);
  CPPUNIT_TEST(testCollectFirstBreakRest_EmptyTbl988);
  CPPUNIT_TEST(testCollectFirstBreakRest_OneSeqNoRestriction);
  CPPUNIT_TEST(testCollectFirstBreakRest_TwoSeqNoRestriction);
  CPPUNIT_TEST(testCollectFirstBreakRest_OneSeqFirstBreakRestr);
  CPPUNIT_TEST(testCollectFirstBreakRest_TwoSeqFirstBreakRestr);
  CPPUNIT_TEST(testCollectFirstBreakRest_FirstBreakRestrAndNoRestr);

  CPPUNIT_TEST(testMergeFareByteCxrAppl);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_MoreApplicable);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_MoreRestricted);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_MoreApplicableMoreSEQ);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_MoreRestrictedMoreSEQ);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_OneResOneAppTwoSEQ);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_OneResOneAppTwoSEQRev);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_OneResOneAppThreeSEQ);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_OneResOneAppTwoSEQDiff);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_DiffRes);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_Empty);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_MoreSEQ);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_OnlyOneEmptyCxrApplTblItemNo);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_OneEmptyCxrApplTblItemNoWithRestricted);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_OneEmptyCxrApplTblItemNoWithApplicable);
  CPPUNIT_TEST(testMergeFareByteCxrAppl_OneEmptyCxrApplTblItemNo);

  CPPUNIT_TEST(testFlightNumberMerge);
  CPPUNIT_TEST(testFlightNumberMerge_empty);
  CPPUNIT_TEST(testFlightNumberMerge_allRestricted);
  CPPUNIT_TEST(testFlightNumberMerge_allNotRestricted);
  CPPUNIT_TEST(testFlightNumberMerge_allRestrictedWithOneIncorrect);
  CPPUNIT_TEST(testFlightNumberMerge_allNotRestrictedWithOneIncorrect);

  CPPUNIT_TEST(testCollectOutboundPortionRest_EmptyReissueSequenceVect);
  CPPUNIT_TEST(testCollectOutboundPortionRest_OneSeqRestrToStopover);
  CPPUNIT_TEST(testCollectOutboundPortionRest_OneSeqNoRestriction);
  CPPUNIT_TEST(testCollectOutboundPortionRest_OneSeqRestrFirstFC);
  CPPUNIT_TEST(testCollectOutboundPortionRest_MoreSeqRestrToStopover);
  CPPUNIT_TEST(testCollectOutboundPortionRest_MoreSeqNoRestriction);
  CPPUNIT_TEST(testCollectOutboundPortionRest_MoreSeqRestrFirstFC);
  CPPUNIT_TEST(testCollectOutboundPortionRest_NoRestAndRestFirstFCAndToStopover);
  CPPUNIT_TEST(testCollectOutboundPortionRest_NoRestAndRestFirstFC);
  CPPUNIT_TEST(testCollectOutboundPortionRest_NoRestAndRestToStopover);
  CPPUNIT_TEST(testCollectOutboundPortionRest_RestrFirstFCAndToStopover);

  CPPUNIT_TEST(matchCxrsFromPSS_test);
  CPPUNIT_TEST(matchCxrsFromPSS_tab990NotSet_GovCxrInExcluded);
  CPPUNIT_TEST(matchCxrsFromPSS_tab990NotSet_GovCxrNotInExcluded);
  CPPUNIT_TEST(matchCxrsFromPSS_tab990NotSet_GovCxrInApplicable);
  CPPUNIT_TEST(matchCxrsFromPSS_tab990NotSet_GovCxrNotInApplicable);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqApplicable_AllOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqApplicable_PartOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqApplicable_AllInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqApplicable_FewInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqApplicable_OneInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqApplicable_OneOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqRestricted_AllOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqRestricted_PartOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqRestricted_AllInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqRestricted_FewInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqRestricted_OneInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListExcluded_SeqRestricted_OneOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqApplicable_AllOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqApplicable_PartOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqApplicable_AllInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqApplicable_FewInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqApplicable_OneInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqApplicable_OneOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqRestricted_AllOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqRestricted_PartOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqRestricted_AllInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqRestricted_FewInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqRestricted_OneInSeq);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqRestricted_OneOutOfList);
  CPPUNIT_TEST(matchCxrsFromPSS_PSSListApplicable_SeqOnlySS);

  CPPUNIT_TEST(mergeDateRestrictions_empty);
  CPPUNIT_TEST(mergeDateRestrictions_not_apply);
  CPPUNIT_TEST(mergeDateRestrictions_same_and_later);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<RexShoppingTrx>();
    _trx->diagnostic().diagnosticType() = Diagnostic331;
    _trx->diagnostic().activate();
    //_trx->diagnostic().diagParamMap().insert(make_pair("DD", "PORTION"));
    //_trx->diagnostic().diagParamMap().insert(make_pair("DD", "FAREBYTE"));
    _trx->setRequest(_memHandle.create<RexPricingRequest>());
    Loc* loc = _memHandle.create<Loc>();
    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = loc;
    _trx->getRequest()->ticketingAgent() = agent;

    _trx->currentTicketingDT() = DateTime(time(0));

    _fareMarket.governingCarrier() = "GC";
    _newItin = _memHandle.create<Itin>();
    _exchangeItin = _memHandle.create<ExcItin>();
    _exchangeItin->fareMarket().push_back(&_fareMarket);
    _trx->exchangeItin().push_back(_exchangeItin);
    _trx->itin().push_back(_newItin);

    _pu = _memHandle.create<PricingUnit>();
    _pu->fareUsage().push_back(_memHandle.create<FareUsage>());
    _puFirstFCTvlSegs = &(_pu->fareUsage().front()->travelSeg());

    _t988 = _memHandle.insert(new ReissueTableOverride(*_trx, _exchangeItin, _pu));
    _fCSegs = &(_fareMarket.travelSeg());
    _newItinSegs = &(_newItin->travelSeg());
    _excItinSegs = &(_exchangeItin->travelSeg());

    addSegment(_excItinSegs, "DFW", "JFK", false, 111, "AA", "DFW", "NYC");
    addSegment(_excItinSegs, "JFK", "LHR", false, 112, "AA", "NYC", "LON");
    addSegment(_excItinSegs, "LHR", "CDG", false, 113, "AA", "LON", "PAR");
    addSegment(_excItinSegs, "CDG", "LHR", false, 114, "AA", "PAR", "LON");
    addSegment(_excItinSegs, "LHR", "JFK", false, 115, "AA", "LON", "NYC");
    addSegment(_excItinSegs, "JFK", "DFW", false, 116, "AA", "NYC", "DFW");
    _exchangeItin->farePath().push_back(FP(
        PU(FU((*_excItinSegs)[0], (*_excItinSegs)[1], (*_excItinSegs)[2]), FU((*_excItinSegs)[5])),
        PU(FU((*_excItinSegs)[3]), FU((*_excItinSegs)[4]))));
  }

  void tearDown()
  {
    // cout << endl << _trx->diagnostic().toString();
    TestCommon::freeVect(_t988Seqs);
    _memHandle.clear();
  }
  // tests

  void testCollectOutboundPortionRest_EmptyReissueSequenceVect()
  {
    std::vector<int> expected;
    _fCSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA", true);
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    assertVect(expected, dataItem.outboundPortion);
  }

  void testCollectOutboundPortionRest_OneSeqNoRestriction()
  {
    std::vector<int> expected;
    _fCSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA", true);
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover

    Indicator outboundPortionInd = NO_RESTRICTION;
    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, outboundPortionInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    assertVect(expected, dataItem.outboundPortion);
  }
  void testCollectOutboundPortionRest_MoreSeqNoRestriction()
  {
    std::vector<int> expected;
    _fCSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA", true);
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover

    Indicator outboundPortionInd = NO_RESTRICTION;
    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, outboundPortionInd);
    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, outboundPortionInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    assertVect(expected, dataItem.outboundPortion);
  }

  void testCollectOutboundPortionRest_OneSeqRestrToStopover()
  {
    std::vector<int> expected;
    _fCSegs->clear();
    _excItinSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA");
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover
    addSegment(_fCSegs, "DFW", "SFO");
    std::copy(_fCSegs->begin(), _fCSegs->end(), std::back_inserter(*_excItinSegs));

    Indicator outboundPortionInd = ORIG_TO_STOPOVER;
    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, outboundPortionInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    expected += 1, 2;
    assertVect(expected, dataItem.outboundPortion);
  }

  void testCollectOutboundPortionRest_MoreSeqRestrToStopover()
  {
    std::vector<int> expected;
    _fCSegs->clear();
    _excItinSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA");
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover
    addSegment(_fCSegs, "DFW", "SFO");
    std::copy(_fCSegs->begin(), _fCSegs->end(), std::back_inserter(*_excItinSegs));

    Indicator outboundPortionInd = ORIG_TO_STOPOVER;
    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, outboundPortionInd);
    _t988Seqs += SEQ988(12, PORTION_OUTBOUND, outboundPortionInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    expected += 1, 2;
    assertVect(expected, dataItem.outboundPortion);
  }
  void testCollectOutboundPortionRest_OneSeqRestrFirstFC()
  {
    std::vector<int> expected;
    _fCSegs->clear();
    _excItinSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA");
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover
    addSegment(_fCSegs, "DFW", "SFO");
    std::copy(_fCSegs->begin(), _fCSegs->end(), std::back_inserter(*_excItinSegs));

    Indicator outboundPortionInd = FIRST_FC;
    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, outboundPortionInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    expected += 1, 2, 3;
    assertVect(expected, dataItem.outboundPortion);
  }
  void testCollectOutboundPortionRest_MoreSeqRestrFirstFC()
  {
    std::vector<int> expected;
    _fCSegs->clear();
    _excItinSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA");
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover
    addSegment(_fCSegs, "DFW", "SFO");
    std::copy(_fCSegs->begin(), _fCSegs->end(), std::back_inserter(*_excItinSegs));

    Indicator outboundPortionInd = FIRST_FC;
    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, outboundPortionInd);
    _t988Seqs += SEQ988(12, PORTION_OUTBOUND, outboundPortionInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    expected += 1, 2, 3;
    assertVect(expected, dataItem.outboundPortion);
  }
  void testCollectOutboundPortionRest_RestrFirstFCAndToStopover()
  {
    std::vector<int> expected;
    _fCSegs->clear();
    _excItinSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA");
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover
    addSegment(_fCSegs, "DFW", "SFO");
    std::copy(_fCSegs->begin(), _fCSegs->end(), std::back_inserter(*_excItinSegs));

    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, FIRST_FC);
    _t988Seqs += SEQ988(12, PORTION_OUTBOUND, ORIG_TO_STOPOVER);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    expected += 1, 2;
    assertVect(expected, dataItem.outboundPortion);
  }
  void testCollectOutboundPortionRest_NoRestAndRestToStopover()
  {
    std::vector<int> expected;
    _fCSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA");
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover
    addSegment(_fCSegs, "DFW", "SFO");

    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, NO_RESTRICTION);
    _t988Seqs += SEQ988(12, PORTION_OUTBOUND, ORIG_TO_STOPOVER);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    assertVect(expected, dataItem.outboundPortion);
  }
  void testCollectOutboundPortionRest_NoRestAndRestFirstFC()
  {
    std::vector<int> expected;
    _fCSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA");
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover
    addSegment(_fCSegs, "DFW", "SFO");

    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, NO_RESTRICTION);
    _t988Seqs += SEQ988(12, PORTION_OUTBOUND, FIRST_FC);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    assertVect(expected, dataItem.outboundPortion);
  }
  void testCollectOutboundPortionRest_NoRestAndRestFirstFCAndToStopover()
  {
    std::vector<int> expected;
    _fCSegs->clear();

    addSegment(_fCSegs, "KTW", "FRA");
    addSegment(_fCSegs, "FRA", "DFW", true); // stopover
    addSegment(_fCSegs, "DFW", "SFO");
    _t988Seqs += SEQ988(11, PORTION_OUTBOUND, NO_RESTRICTION);
    _t988Seqs += SEQ988(12, PORTION_OUTBOUND, ORIG_TO_STOPOVER);
    _t988Seqs += SEQ988(13, PORTION_OUTBOUND, FIRST_FC);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectOutboundPortionRest(_t988Seqs, _fareMarket, dataItem);

    assertVect(expected, dataItem.outboundPortion);
  }
  //==============================================================
  //--------------------------------------------------------------
  void testCollectFirstBreakRest_EmptyTbl988()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectFirstBreakRest(_t988Seqs, dataItem);

    bool expectedStatus = false;
    CPPUNIT_ASSERT_EQUAL(expectedStatus, dataItem.firstBreakStatus);
  }

  void testCollectFirstBreakRest_OneSeqNoRestriction()
  {
    Indicator sameFareInd = NO_RESTR_1ST_BREAK;
    _t988Seqs += SEQ988(11, FIRST_BREAK, sameFareInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectFirstBreakRest(_t988Seqs, dataItem);

    bool expectedStatus = false;
    CPPUNIT_ASSERT_EQUAL(expectedStatus, dataItem.firstBreakStatus);
  }
  void testCollectFirstBreakRest_TwoSeqNoRestriction()
  {
    Indicator sameFareInd = NO_RESTR_1ST_BREAK;
    _t988Seqs += SEQ988(11, FIRST_BREAK, sameFareInd);
    _t988Seqs += SEQ988(11, FIRST_BREAK, sameFareInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectFirstBreakRest(_t988Seqs, dataItem);

    bool expectedStatus = false;
    CPPUNIT_ASSERT_EQUAL(expectedStatus, dataItem.firstBreakStatus);
  }

  void testCollectFirstBreakRest_OneSeqFirstBreakRestr()
  {
    Indicator sameFareInd = RESTR_1ST_BREAK;
    _t988Seqs += SEQ988(11, FIRST_BREAK, sameFareInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectFirstBreakRest(_t988Seqs, dataItem);

    bool expectedStatus = true;
    CPPUNIT_ASSERT_EQUAL(expectedStatus, dataItem.firstBreakStatus);
  }

  void testCollectFirstBreakRest_TwoSeqFirstBreakRestr()
  {
    Indicator sameFareInd = RESTR_1ST_BREAK;
    _t988Seqs += SEQ988(11, FIRST_BREAK, sameFareInd);
    _t988Seqs += SEQ988(11, FIRST_BREAK, sameFareInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectFirstBreakRest(_t988Seqs, dataItem);

    bool expectedStatus = true;
    CPPUNIT_ASSERT_EQUAL(expectedStatus, dataItem.firstBreakStatus);
  }

  void testCollectFirstBreakRest_FirstBreakRestrAndNoRestr()
  {
    Indicator sameFareInd = RESTR_1ST_BREAK;
    _t988Seqs += SEQ988(11, FIRST_BREAK, sameFareInd);

    sameFareInd = NO_RESTR_1ST_BREAK;
    _t988Seqs += SEQ988(12, FIRST_BREAK, sameFareInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectFirstBreakRest(_t988Seqs, dataItem);

    bool expectedStatus = false;
    CPPUNIT_ASSERT_EQUAL(expectedStatus, dataItem.firstBreakStatus);
  }

  void testCollectForcedConnectionStopovers_NoRestriction()
  {
    std::set<LocCode> expectedConnections;

    _excItinSegs->clear();
    addSegment(_excItinSegs, "KTW", "FRA");
    addSegment(_excItinSegs, "FRA", "DFW");

    Indicator stopoverConnInd = ' ';
    _t988Seqs += SEQ988(11, FORCED_CNX, stopoverConnInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectForcedConnections(_t988Seqs, dataItem);

    assertSet(expectedConnections, dataItem.forcedConnection);
  }

  void testCollectForcedConnectionStopovers_ForcedStopoverOneSeq()
  {
    std::set<LocCode> expectedConnections;

    _excItinSegs->clear();
    addSegment(_excItinSegs, "KTW", "FRA");
    addSegment(_excItinSegs, "FRA", "DFW", true);
    addSegment(_excItinSegs, "DFW", "NYC", true);
    addSegment(_excItinSegs, "NYC", "LAX");

    Indicator stopoverConnInd = 'S';
    _t988Seqs += SEQ988(11, FORCED_CNX, stopoverConnInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectForcedConnections(_t988Seqs, dataItem);

    assertSet(expectedConnections, dataItem.forcedConnection);
  }

  void testCollectForcedConnectionStopovers_ForcedCnxOneSeq()
  {
    std::set<LocCode> expectedConnections;

    _excItinSegs->clear();
    addSegment(_excItinSegs, "KTW", "FRA");
    addSegment(_excItinSegs, "FRA", "DFW", true);
    addSegment(_excItinSegs, "DFW", "NYC");

    Indicator stopoverConnInd = 'C';
    _t988Seqs += SEQ988(11, FORCED_CNX, stopoverConnInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectForcedConnections(_t988Seqs, dataItem);

    expectedConnections += "FRA", "NYC";
    assertSet(expectedConnections, dataItem.forcedConnection);
  }

  void testCollectForcedConnectionStopovers_ForcedCnxAndStopoverOneSeq()
  {
    std::set<LocCode> expectedConnections;

    _excItinSegs->clear();
    addSegment(_excItinSegs, "KTW", "FRA");
    addSegment(_excItinSegs, "FRA", "DFW", true);
    addSegment(_excItinSegs, "DFW", "NYC");

    Indicator stopoverConnInd = 'B';
    _t988Seqs += SEQ988(11, FORCED_CNX, stopoverConnInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectForcedConnections(_t988Seqs, dataItem);

    expectedConnections += "FRA", "NYC";
    assertSet(expectedConnections, dataItem.forcedConnection);
  }

  void testCollectForcedConnectionStopovers_ForcedCnxForcedBoth()
  {
    std::set<LocCode> expectedConnections;

    _excItinSegs->clear();
    addSegment(_excItinSegs, "KTW", "FRA");
    addSegment(_excItinSegs, "FRA", "DFW", true);
    addSegment(_excItinSegs, "DFW", "NYC");

    Indicator stopoverConnInd = 'C';
    _t988Seqs += SEQ988(11, FORCED_CNX, stopoverConnInd);
    stopoverConnInd = 'B';
    _t988Seqs += SEQ988(11, FORCED_CNX, stopoverConnInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectForcedConnections(_t988Seqs, dataItem);

    expectedConnections += "FRA", "NYC";
    assertSet(expectedConnections, dataItem.forcedConnection);
  }

  void testCollectForcedConnectionStopovers_ForcedCnxAndNoRestriction()
  {
    std::set<LocCode> expectedConnections;

    _excItinSegs->clear();
    addSegment(_excItinSegs, "KTW", "FRA");
    addSegment(_excItinSegs, "FRA", "DFW", true);
    addSegment(_excItinSegs, "DFW", "NYC");

    Indicator stopoverConnInd = 'C';
    _t988Seqs += SEQ988(11, FORCED_CNX, stopoverConnInd);

    stopoverConnInd = ' ';
    _t988Seqs += SEQ988(11, FORCED_CNX, stopoverConnInd);

    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->collectForcedConnections(_t988Seqs, dataItem);

    assertSet(expectedConnections, dataItem.forcedConnection);
  }

  void testMergePortion_EmptyReissueSequenceVect()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988->mergePortion(_t988Seqs, dataItem);
    CPPUNIT_ASSERT(dataItem.portionMerge.empty());
  }

  void testMergePortion_EmptyGeoTblItemNo()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988Seqs += SEQ(11, 0, 0);
    _t988->mergePortion(_t988Seqs, dataItem);
    CPPUNIT_ASSERT(dataItem.portionMerge.empty());
  }

  void testMergePortion_SetGeoTblItemNo_EmptyPortionInd()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    _t988Seqs += SEQ(11, TestCommon::JOURNEY, 0);
    _t988->mergePortion(_t988Seqs, dataItem);
    CPPUNIT_ASSERT(dataItem.portionMerge.empty());
  }

  void testMergePortion_AllTraveSegmentsCanBeChanged()
  {
    _t988Seqs +=
        SEQ(11, TestCommon::JOURNEY | TestCommon::CHECK_ORIG, 0, ReissueTable::FIRST_FLIGHT_COUPON);
    RexShoppingTrx::R3SeqsConstraint dataItem;

    _t988->mergePortion(_t988Seqs, dataItem);
    CPPUNIT_ASSERT(dataItem.portionMerge.empty());
  }

  void testMergePortion_FirstTraveSegmentCanNOTBeChanged()
  {
    _t988Seqs += SEQ(11,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COUPON);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 1;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_FirstTraveSegmentCanNOTBeChanged2xSEQ()
  {
    _t988Seqs += SEQ(11,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COUPON);
    _t988Seqs += SEQ(12,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COUPON);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 1;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_FirstTraveSegmentCanNOTBeChanged3xSEQ()
  {
    _t988Seqs += SEQ(11,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COUPON);
    _t988Seqs += SEQ(12,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COUPON);
    _t988Seqs += SEQ(13,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COUPON);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 1;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_FirstAndThirdTraveSegmentCanNOTBeChanged()
  {
    _t988Seqs +=
        SEQ(11,
            TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1 | TestCommon::SEG_3,
            0,
            ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 1, 3;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_FirstTraveSegmentCanNOTBeChangedThirdCan1()
  {
    _t988Seqs +=
        SEQ(11,
            TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1 | TestCommon::SEG_3,
            0,
            ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(12,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 1;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_FirstTraveSegmentCanNOTBeChangedThirdCan2()
  {
    _t988Seqs += SEQ(11,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs +=
        SEQ(12,
            TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1 | TestCommon::SEG_3,
            0,
            ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 1;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_ThirdTraveSegmentCanNOTBeChangedFirstCan1()
  {
    _t988Seqs += SEQ(11,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_3,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs +=
        SEQ(12,
            TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1 | TestCommon::SEG_3,
            0,
            ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 3;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_ThirdTraveSegmentCanNOTBeChangedFirstCan2()
  {
    _t988Seqs +=
        SEQ(11,
            TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1 | TestCommon::SEG_3,
            0,
            ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(12,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_3,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 3;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_FirstTraveSegmentCanNOTBeChangedOnlyInFirstSEQ()
  {
    _t988Seqs += SEQ(11,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(
        12, TestCommon::JOURNEY | TestCommon::CHECK_ORIG, 0, ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 1;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_FirstTraveSegmentCanNOTBeChangedOnlyInSecondSEQ()
  {
    _t988Seqs += SEQ(
        11, TestCommon::JOURNEY | TestCommon::CHECK_ORIG, 0, ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(12,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 1;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_ThreeDiffCanNOTChange()
  {
    _t988Seqs += SEQ(11,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(12,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_2,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(13,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_3,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;

    _t988->mergePortion(_t988Seqs, dataItem);
    CPPUNIT_ASSERT(dataItem.portionMerge.empty());
  }

  void testMergePortion_OneWithoutConstrains()
  {
    _t988Seqs += SEQ(11,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(12,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(
        13, TestCommon::JOURNEY | TestCommon::CHECK_ORIG, 0, ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(14,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 1;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergePortion_OneToMerge()
  {
    _t988Seqs +=
        SEQ(11,
            TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1 | TestCommon::SEG_2,
            0,
            ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs +=
        SEQ(12,
            TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_2 | TestCommon::SEG_3,
            0,
            ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs +=
        SEQ(13,
            TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_2 | TestCommon::SEG_3,
            0,
            ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs +=
        SEQ(14,
            TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_1 | TestCommon::SEG_2,
            0,
            ReissueTable::FIRST_FLIGHT_COMPONENT);
    _t988Seqs += SEQ(15,
                     TestCommon::JOURNEY | TestCommon::CHECK_ORIG | TestCommon::SEG_2,
                     0,
                     ReissueTable::FIRST_FLIGHT_COMPONENT);
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<uint8_t> expectedPortionMerge;
    expectedPortionMerge += 2;

    _t988->mergePortion(_t988Seqs, dataItem);
    assertUnchangableSegments(expectedPortionMerge, dataItem.portionMerge);
  }

  void testMergeFareByteCxrAppl()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "ZZ", "XX";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "ZZ", "BB", "XX";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB";
    addTable990(' ', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);
    _t988Seqs += SEQ(13, 3);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.restCxr += "XX", "ZZ";
    expectedCxrList.applCxr += "BB", "$$";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_MoreApplicable()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "ZZ", "XX";
    addTable990('X', tabCxrList);

    _t988Seqs += SEQ(11, 1);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.restCxr += "AA", "BB", "ZZ", "XX";
    expectedCxrList.applCxr += "$$";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_MoreRestricted()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "ZZ", "XX";
    addTable990(' ', tabCxrList);

    _t988Seqs += SEQ(11, 1);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.applCxr += "AA", "BB", "ZZ", "XX";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_MoreApplicableMoreSEQ()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "ZZ", "XX";
    addTable990(' ', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB", "ZZ";
    addTable990(' ', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB", "XX";
    addTable990(' ', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);
    _t988Seqs += SEQ(13, 3);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.applCxr += "AA", "BB", "ZZ", "XX";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_MoreRestrictedMoreSEQ()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "ZZ", "XX";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB", "ZZ";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB", "XX";
    addTable990('X', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);
    _t988Seqs += SEQ(13, 3);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.restCxr += "BB";
    expectedCxrList.applCxr += "$$";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_OneResOneAppTwoSEQ()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "AA";
    addTable990(' ', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.applCxr += "AA", "$$";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_OneResOneAppTwoSEQRev()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA";
    addTable990(' ', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "AA";
    addTable990('X', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.applCxr += "AA", "$$";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_OneResOneAppThreeSEQ()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "AA";
    addTable990(' ', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "AA";
    addTable990('X', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);
    _t988Seqs += SEQ(13, 3);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.applCxr += "AA", "$$";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_OneResOneAppTwoSEQDiff()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB";
    addTable990(' ', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.restCxr += "AA";
    expectedCxrList.applCxr += "BB", "$$";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_DiffRes()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB";
    addTable990('X', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.applCxr += "$$";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_Empty()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_MoreSEQ()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC", "DD";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB", "CC", "DD";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "CC", "DD";
    addTable990(' ', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "CC";
    addTable990(' ', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);
    _t988Seqs += SEQ(13, 3);
    _t988Seqs += SEQ(14, 4);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.restCxr += "BB";
    expectedCxrList.applCxr += "CC", "DD", "$$";
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_OnlyOneEmptyCxrApplTblItemNo()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    _t988Seqs += SEQ(11, 0);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.applCxr += "GC";
    expectedCxrList.govCxrPref = false;
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_OneEmptyCxrApplTblItemNoWithRestricted()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC", "DD";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB", "CC", "DD";
    addTable990('X', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);
    _t988Seqs += SEQ(13, 0);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.restCxr += "BB", "CC", "DD";
    expectedCxrList.applCxr += "GC", "$$";
    expectedCxrList.govCxrPref = false;
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_OneEmptyCxrApplTblItemNoWithApplicable()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "CC", "DD";
    addTable990(' ', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "CC";
    addTable990(' ', tabCxrList);

    _t988Seqs += SEQ(11, 1);
    _t988Seqs += SEQ(12, 2);
    _t988Seqs += SEQ(13, 0);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.applCxr += "CC", "DD", "GC";
    expectedCxrList.govCxrPref = false;
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testMergeFareByteCxrAppl_OneEmptyCxrApplTblItemNo()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC", "DD";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "BB", "CC", "DD";
    addTable990('X', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "CC", "DD";
    addTable990(' ', tabCxrList);

    tabCxrList.clear();
    tabCxrList += "CC";
    addTable990(' ', tabCxrList);

    _t988Seqs += SEQ(11, 0);
    _t988Seqs += SEQ(12, 2);
    _t988Seqs += SEQ(13, 3);
    _t988Seqs += SEQ(14, 4);

    RexShoppingTrx::FareByteCxrAppl expectedCxrList;
    expectedCxrList.restCxr += "BB";
    expectedCxrList.applCxr += "CC", "DD", "$$", "GC";
    expectedCxrList.govCxrPref = false;
    _t988->mergeFareByteCxrAppl(_t988Seqs, _fareMarket.governingCarrier(), dataItem);

    assertCxrVect(expectedCxrList, dataItem.fareByteCxrAppl);
  }

  void testFlightNumberMerge()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;

    _t988Seqs += SEQFLT(11, 'X');
    _t988Seqs += SEQFLT(12, 'X');
    _t988Seqs += SEQFLT(13, ' ');
    _t988Seqs += SEQFLT(14, 'X');

    _t988->mergeFlightNumber(_t988Seqs, dataItem);

    bool flightNoInAllSeqsCanNOTChange = dataItem.flightNumberRestriction;
    CPPUNIT_ASSERT(!flightNoInAllSeqsCanNOTChange);
  }

  void testFlightNumberMerge_empty()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;

    _t988->mergeFlightNumber(_t988Seqs, dataItem);

    bool flightNoInAllSeqsCanNOTChange = dataItem.flightNumberRestriction;
    CPPUNIT_ASSERT(!flightNoInAllSeqsCanNOTChange);
  }

  void testFlightNumberMerge_allRestricted()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;

    _t988Seqs += SEQ988(11, FLIGHT_NO, 'X');
    _t988Seqs += SEQ988(12, FLIGHT_NO, 'X');
    _t988Seqs += SEQ988(13, FLIGHT_NO, 'X');
    _t988Seqs += SEQ988(14, FLIGHT_NO, 'X');

    _t988->mergeFlightNumber(_t988Seqs, dataItem);

    bool flightNoInAllSeqsCanNOTChange = dataItem.flightNumberRestriction;
    CPPUNIT_ASSERT(flightNoInAllSeqsCanNOTChange);
  }

  void testFlightNumberMerge_allNotRestricted()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;

    _t988Seqs += SEQ988(11, FLIGHT_NO, ' ');
    _t988Seqs += SEQ988(12, FLIGHT_NO, ' ');
    _t988Seqs += SEQ988(13, FLIGHT_NO, ' ');
    _t988Seqs += SEQ988(14, FLIGHT_NO, ' ');

    _t988->mergeFlightNumber(_t988Seqs, dataItem);

    bool flightNoInAllSeqsCanNOTChange = dataItem.flightNumberRestriction;
    CPPUNIT_ASSERT(!flightNoInAllSeqsCanNOTChange);
  }

  void testFlightNumberMerge_allRestrictedWithOneIncorrect()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;

    _t988Seqs += SEQ988(11, FLIGHT_NO, 'X');
    _t988Seqs += SEQ988(12, FLIGHT_NO, '1');
    _t988Seqs += SEQ988(13, FLIGHT_NO, 'X');

    _t988->mergeFlightNumber(_t988Seqs, dataItem);

    bool flightNoInAllSeqsCanNOTChange = dataItem.flightNumberRestriction;
    CPPUNIT_ASSERT(!flightNoInAllSeqsCanNOTChange);
  }

  void testFlightNumberMerge_allNotRestrictedWithOneIncorrect()
  {
    RexShoppingTrx::R3SeqsConstraint dataItem;

    _t988Seqs += SEQ988(11, FLIGHT_NO, ' ');
    _t988Seqs += SEQ988(12, FLIGHT_NO, '1');
    _t988Seqs += SEQ988(13, FLIGHT_NO, ' ');

    _t988->mergeFlightNumber(_t988Seqs, dataItem);

    bool flightNoInAllSeqsCanNOTChange = dataItem.flightNumberRestriction;
    CPPUNIT_ASSERT(!flightNoInAllSeqsCanNOTChange);
  }

  void matchCxrsFromPSS_test()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(!sequensePASSED);
  }

  void matchCxrsFromPSS_tab990NotSet_GovCxrInExcluded()
  {
    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "GC", "BB";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 0)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_tab990NotSet_GovCxrNotInExcluded()
  {
    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "BB";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 0)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_tab990NotSet_GovCxrInApplicable()
  {
    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "GC", "BB";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 0)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_tab990NotSet_GovCxrNotInApplicable()
  {
    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "BB";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 0)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqApplicable_AllOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "DD", "EE";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqApplicable_PartOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC", "DD";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqApplicable_AllInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(!sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqApplicable_FewInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "BB", "CC";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(!sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqApplicable_OneInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "BB";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(!sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqApplicable_OneOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "DD";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqRestricted_AllOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "DD", "EE";
    addTable990('X', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqRestricted_PartOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC", "DD";
    addTable990('X', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqRestricted_AllInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC";
    addTable990('X', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqRestricted_FewInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB";
    addTable990('X', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqRestricted_OneInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA";
    addTable990('X', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListExcluded_SeqRestricted_OneOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "DD";
    addTable990('X', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = true;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqApplicable_AllOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "DD", "EE";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(!sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqApplicable_PartOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC", "DD";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqApplicable_AllInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqApplicable_FewInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqApplicable_OneInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "CC";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqApplicable_OneOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "DD";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(!sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqRestricted_AllOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "DD", "EE";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(!sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqRestricted_PartOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC", "DD";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqRestricted_AllInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB", "CC";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqRestricted_FewInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "AA", "BB";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqRestricted_OneInSeq()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "BB";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqRestricted_OneOutOfList()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "DD";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA", "BB", "CC";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(!sequensePASSED);
  }

  void matchCxrsFromPSS_PSSListApplicable_SeqOnlySS()
  {
    std::vector<CarrierCode> tabCxrList;

    tabCxrList += "$$";
    addTable990(' ', tabCxrList);

    std::vector<CarrierCode> pSSCxrList;
    pSSCxrList += "AA";
    bool excluded = false;
    addPSSCxrList(pSSCxrList, excluded);

    bool sequensePASSED = _t988->matchCxrsFromPSS(*_trx, _fareMarket, *(SEQ(11, 1)));

    CPPUNIT_ASSERT(sequensePASSED);
  }

  void mergeDateRestrictions_empty()
  {
    Tab988Merger::T988Seqs seqs;
    auto result = _t988->mergeDateRestrictions(seqs);
    CPPUNIT_ASSERT(result.empty());
  }

  void mergeDateRestrictions_not_apply()
  {
    Tab988Merger::T988Seqs seqs;

    ReissueSequence notApply;
    notApply.dateInd() = ReissueTable::NOT_APPLY;

    seqs.resize(2, &notApply);
    auto result = _t988->mergeDateRestrictions(seqs);

    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result[0].size() == 2);
    CPPUNIT_ASSERT(result[0].front()->dateInd() == ReissueTable::NOT_APPLY);
  }

  void mergeDateRestrictions_same_and_later()
  {
    Tab988Merger::T988Seqs seqs;

    ReissueSequence sameDate;
    sameDate.dateInd() = ReissueTable::MATCH_SAME_DEPARTURE_DATE;
    ReissueSequence laterDate;
    laterDate.dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    seqs.resize(2, &sameDate);
    seqs.resize(3, &laterDate);
    auto result = _t988->mergeDateRestrictions(seqs);

    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT(result[0].size() == 2);
    CPPUNIT_ASSERT(result[1].size() == 1);
    CPPUNIT_ASSERT(result[0].front()->dateInd() == ReissueTable::MATCH_SAME_DEPARTURE_DATE);
    CPPUNIT_ASSERT(result[1].front()->dateInd() == ReissueTable::MATCH_LATER_DEPARTURE_DATE);
  }
  // tests end

  // helpers
  void assertUnchangableSegments(const std::vector<uint8_t>& expectedPortionMerge,
                                 const RexShoppingTrx::PortionMergeTvlVectType& actualPortionMerge)
  {
    for (size_t i = 0; i < expectedPortionMerge.size(); i++)
      CPPUNIT_ASSERT(
          count(actualPortionMerge.begin(), actualPortionMerge.end(), expectedPortionMerge[i]));
  }

  void assertSet(const std::set<LocCode>& expectedSet, const std::set<LocCode>& actualSet)
  {
    for (std::set<LocCode>::const_iterator i = expectedSet.begin(); i != expectedSet.end(); i++)
      CPPUNIT_ASSERT(actualSet.count(*i));
  }

  void addTable990(unsigned char tblIndic, const std::vector<CarrierCode>& cxrList)
  {
    _t988->tabl990().push_back(std::make_pair(tblIndic, cxrList));
  }

  void assertCxrVect(const RexShoppingTrx::FareByteCxrAppl& expectedCarrierList,
                     const RexShoppingTrx::FareByteCxrAppl& actualCarrierList)
  {
    CPPUNIT_ASSERT(expectedCarrierList.restCxr.size() == actualCarrierList.restCxr.size());
    CPPUNIT_ASSERT(expectedCarrierList.applCxr.size() == actualCarrierList.applCxr.size());
    for (std::set<CarrierCode>::const_iterator i = expectedCarrierList.restCxr.begin();
         i != expectedCarrierList.restCxr.end();
         i++)
      CPPUNIT_ASSERT(actualCarrierList.restCxr.count(*i));
    for (std::set<CarrierCode>::const_iterator i = expectedCarrierList.applCxr.begin();
         i != expectedCarrierList.applCxr.end();
         i++)
      CPPUNIT_ASSERT(actualCarrierList.applCxr.count(*i));
    CPPUNIT_ASSERT(expectedCarrierList.govCxrPref == actualCarrierList.govCxrPref);
  }
  // helpers end
protected:
  RexShoppingTrx* _trx;
  Itin* _newItin;
  ExcItin* _exchangeItin;
  ReissueTableOverride* _t988;
  std::vector<ReissueSequence*> _t988Seqs;
  FareMarket _fareMarket;
  std::vector<TravelSeg*>* _fCSegs;
  std::vector<TravelSeg*>* _newItinSegs;
  std::vector<TravelSeg*>* _excItinSegs;
  PricingUnit* _pu;
  std::vector<TravelSeg*>* _puFirstFCTvlSegs;
  TestMemHandle _memHandle;

  FareUsage* FU(TravelSeg* p1, TravelSeg* p2 = NULL, TravelSeg* p3 = NULL)
  {
    return TestCommon::FU(p1, p2, p3);
  }

  PricingUnit* PU(FareUsage* p1, FareUsage* p2 = NULL) { return TestCommon::PU(p1, p2); }

  FarePath* FP(PricingUnit* p1, PricingUnit* p2 = NULL) { return TestCommon::FP(p1, p2); }

  ReissueSequence*
  SEQ(int r3ItemNo, int stsFrom, int stsTo, Indicator portionInd = ReissueTable::NOT_APPLY)
  {
    return TestCommon::SEQ(r3ItemNo, stsFrom, stsTo, portionInd);
  }

  ReissueSequence* SEQFLT(int r3ItemNo, Indicator flightNoInd)
  {
    ReissueSequence* r3 = new ReissueSequence;
    r3->itemNo() = r3ItemNo;
    r3->seqNo() = 1234;
    r3->flightNoInd() = flightNoInd;
    return r3;
  }

  ReissueSequence* SEQ(int r3ItemNo, int fareCxrApplTblItemNo)
  {
    ReissueSequence* r3 = new ReissueSequence;
    r3->itemNo() = r3ItemNo;
    r3->seqNo() = 1234;
    r3->fareCxrApplTblItemNo() = fareCxrApplTblItemNo;
    return r3;
  }

  ReissueSequence*
  SEQ988(int r3ItemNo, SeqByteIndicator byteInd, Indicator ind, int stsFrom = 0, int stsTo = 0)
  {
    ReissueSequence* seq = new ReissueSequence;
    seq->itemNo() = r3ItemNo;
    seq->seqNo() = r3ItemNo;
    switch (byteInd)
    {
    case PORTION:
      return TestCommon::SEQ(r3ItemNo, stsFrom, stsTo, ind);
    case FORCED_CNX:
      seq->stopoverConnectInd() = ind;
      return seq;
    case FIRST_BREAK:
      seq->firstBreakInd() = ind;
      return seq;
    case PORTION_OUTBOUND:
      seq->outboundInd() = ind;
      return seq;
    case FLIGHT_NO:
      seq->flightNoInd() = ind;
      return seq;
    }
    return 0;
  }

  void addSegment(std::vector<TravelSeg*>* tvlSeg,
                  const LocCode& board,
                  const LocCode& off,
                  bool stopOver = false,
                  int flightNo = 123,
                  const CarrierCode& cxr = "AA",
                  const LocCode& boardMCity = "",
                  const LocCode& offMCity = "",
                  const TravelSeg::ChangeStatus changeStatus = TravelSeg::CHANGED,
                  bool unflown = true,
                  const string& depDateTime = "2007-07-07 07:07",
                  const TravelSegType segType = Air)

  {
    TestCommon::addSegment(tvlSeg,
                           board,
                           off,
                           stopOver,
                           flightNo,
                           cxr,
                           boardMCity,
                           offMCity,
                           changeStatus,
                           unflown,
                           depDateTime,
                           segType);
  }

  void addPSSCxrList(std::vector<CarrierCode>& expectedSet, const bool excluded)
  {
    _trx->cxrListFromPSS().cxrList.insert(expectedSet.begin(), expectedSet.end());
    _trx->cxrListFromPSS().excluded = excluded;
  }
  void assertVect(const std::vector<int>& expected, const std::vector<int>& actual)
  {
    CPPUNIT_ASSERT(expected.size() == actual.size());
    for (size_t i = 0; i < expected.size() && i < actual.size(); i++)
      CPPUNIT_ASSERT(expected[i] == actual[i]);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Tab988MergerTest);
}
