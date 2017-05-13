#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/set.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include "Common/Config/ConfigMan.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "Shopping/RexConstrainsConsolidator.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"

#include <boost/range/adaptor/map.hpp>

using namespace std;
using namespace boost::assign;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
public:
  MyDataHandle(TestMemHandle& dataHandle) : _dataHandle(dataHandle) {}

  const std::vector<MultiAirportCity*>& getMultiAirportCity(const LocCode& city)
  {
    std::vector<MultiAirportCity*>* dummyObj;
    dummyObj = _dataHandle.create<std::vector<MultiAirportCity*> >();
    return *dummyObj;
  }

protected:
  TestMemHandle& _dataHandle;
};
}

class RexConstrainsConsolidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexConstrainsConsolidatorTest);

  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_test);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_segVectIsEmpty);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_ONDIsEmpty);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_oneFlownSeg);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_oneUnflownSeg);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_allFlown);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_allUnflown);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_flownUnflown);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_flownUnflown2);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_multipleOND_breakInFirstOND);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_multipleOND_breakInSecondOND);
  CPPUNIT_SKIP_TEST(testdetachFlownToNewLeg_shoppedUnshopped);

  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_test);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_noOND);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_noFC);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_noExcTvlSeg);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_oneOND);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_twoOND);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_twoOND_firstIsOneSeg);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_twoOND_lastIsOneSeg);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_maxNoOfOND);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_cannotMatchOND);
  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_cannotMatchSecondOND);

  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_test);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_portionOnly);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_outboundPortionOnly);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_unshoppedFlightsOnly);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_allFltRestWithBkg);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_flightNoRest_allFltRest);

  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_R3MergedData_empty);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_govCxrIsPrefIn1FM);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_govCxrIsPrefIn1FM_addGov);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_govCxrIsPrefIn2FM);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_firstFMCxrListEmpty);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_secondFMCxrListEmpty);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_userCxrList_empty);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_userCxrList_R3cxrList_empty);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_userCxrList_R3cxrList_exc);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_userCxrList_R3cxrList_pref);

  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test1);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test2);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test3);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test4);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test5);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test6);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test7);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test8);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test9);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test10);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test11);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test12);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test13);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test14);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test15);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test16);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_carrierRest_test17);

  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_skipFM_fltRest);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_skipFM_carrierRest);
  CPPUNIT_SKIP_TEST(testCreateSODForONDInfo_test);
  CPPUNIT_SKIP_TEST(testCreateSODForONDInfo_cxrListEmpty);

  CPPUNIT_SKIP_TEST(testMergeOADCrxListWithPSSCrxList_fareByteCxrApplNotSet);
  CPPUNIT_SKIP_TEST(testMergeOADCrxListWithPSSCrxList_fareByteCxrApplEmpty);
  CPPUNIT_SKIP_TEST(testMergeOADCrxListWithPSSCrxList_govIsNotExcluded);
  CPPUNIT_SKIP_TEST(testMergeOADCrxListWithPSSCrxList_removeUsrFromGov);

  CPPUNIT_SKIP_TEST(testmatchONDInfotoExcOND_multiAirport);
  CPPUNIT_SKIP_TEST(testConsolideConstrainsForONDInfo_multiAirport);

  CPPUNIT_SKIP_TEST(testGetFlownExcONDTvlSegPos_test);
  CPPUNIT_SKIP_TEST(testGetFlownExcONDTvlSegPos_emptyNewItinSegs);
  CPPUNIT_SKIP_TEST(testGetFlownExcONDTvlSegPos_emptyExcItinSegs);
  CPPUNIT_SKIP_TEST(testGetFlownExcONDTvlSegPos_wrongFirstTvlSegPosNumber);
  CPPUNIT_SKIP_TEST(testGetFlownExcONDTvlSegPos_wrongLastTvlSegPosNumber);
  CPPUNIT_SKIP_TEST(testGetFlownExcONDTvlSegPos_matchFirstSeg);
  CPPUNIT_SKIP_TEST(testGetFlownExcONDTvlSegPos_matchAll);
  CPPUNIT_SKIP_TEST(testGetFlownExcONDTvlSegPos_matchLast);
  CPPUNIT_SKIP_TEST(ondHasPrimarySectorInExcFlownPart_noFM);
  CPPUNIT_SKIP_TEST(ondHasPrimarySectorInExcFlownPart_primarySegOutOfVect);
  CPPUNIT_SKIP_TEST(ondHasPrimarySectorInExcFlownPart_primarySegNotFound);
  CPPUNIT_SKIP_TEST(ondHasPrimarySectorInExcFlownPart_primarySegFoundOnFront);
  CPPUNIT_SKIP_TEST(ondHasPrimarySectorInExcFlownPart_primarySegFoundInMiddle);
  CPPUNIT_SKIP_TEST(ondHasPrimarySectorInExcFlownPart_primarySegFoundOnEnd);
  CPPUNIT_SKIP_TEST(ondHasPrimarySectorInExcFlownPart_primarySegFoundOnAll);
  CPPUNIT_SKIP_TEST(findFMforSegment_FMnotFound);
  CPPUNIT_SKIP_TEST(findFMforSegment_FMfoundOnFront);
  CPPUNIT_SKIP_TEST(findFMforSegment_FMfoundOnBack);
  CPPUNIT_SKIP_TEST(findFMforOND_outOfVect);
  CPPUNIT_SKIP_TEST(findFMforOND_foundOnFront0);
  CPPUNIT_SKIP_TEST(findFMforOND_foundOnFront1);
  CPPUNIT_SKIP_TEST(findFMforOND_foundOnFront2);
  CPPUNIT_SKIP_TEST(findFMforOND_foundOnBack0);
  CPPUNIT_SKIP_TEST(findFMforOND_foundOnBack1);
  CPPUNIT_SKIP_TEST(findFMforOND_foundOnBack2);
  CPPUNIT_SKIP_TEST(findFMforOND_foundOnAll);
  CPPUNIT_SKIP_TEST(isONDinExcFMWithFlownGov_test);
  CPPUNIT_SKIP_TEST(isONDinExcFMWithFlownGov_firstONDhasGov);
  CPPUNIT_SKIP_TEST(isONDinExcFMWithFlownGov_secondONDhasGov);
  CPPUNIT_SKIP_TEST(testSetGovCxr_ondCxrEmpty);
  CPPUNIT_SKIP_TEST(testSetGovCxr_diffPrefCxr);
  CPPUNIT_SKIP_TEST(testSetGovCxr_interPrefCxr);
  CPPUNIT_SKIP_TEST(testSetGovCxr_diffExcCxr);
  CPPUNIT_SKIP_TEST(testSetGovCxr_diffExcCxrDiffCxr);
  CPPUNIT_SKIP_TEST(testSetUsrCxr_ondCxrEmpty);
  CPPUNIT_SKIP_TEST(testSetUsrCxr_copyUsrCxr);
  CPPUNIT_SKIP_TEST(routingHasChanged_noChange);
  CPPUNIT_SKIP_TEST(routingHasChanged_change);
  CPPUNIT_SKIP_TEST(routingHasChanged_noChangeMultiAirport);
  CPPUNIT_SKIP_TEST(routingHasChanged_change_LessNewFlights);
  CPPUNIT_SKIP_TEST(routingHasChanged_change_LessExcFlights);
  CPPUNIT_SKIP_TEST(matchFMsToONDInfoWithoutNoMatched_routingNOTChanged);
  CPPUNIT_SKIP_TEST(matchFMsToONDInfoWithoutNoMatched_routingChanged);
  CPPUNIT_SKIP_TEST(createOndInfoNotMatched_routingNOTChanged);
  CPPUNIT_SKIP_TEST(createOndInfoNotMatched_routingChanged);
  CPPUNIT_SKIP_TEST(createFmNotMatched_routingNOTChanged);
  CPPUNIT_SKIP_TEST(createFmNotMatched_routingChanged);
  CPPUNIT_SKIP_TEST(doesAnyFlownFCHasSameCxrAsUnmappedFC_routingChanged_sameGovCxr);
  CPPUNIT_SKIP_TEST(doesAnyFlownFCHasSameCxrAsUnmappedFC_routingChanged_diffGovCxr);
  CPPUNIT_SKIP_TEST(doesAnyFlownFCHasSameCxrAsUnmappedFC_unflown);

  CPPUNIT_SKIP_TEST(testCheckRestrictionNeeded);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    //_memHandle.create<TestConfigInitializer>();
    //_trx = _memHandle.create<RexShoppingTrx>();
    //_trx->diagnostic().diagnosticType() = Diagnostic987;
    //_trx->diagnostic().activate();
    //_trx->diagnostic().diagParamMap().insert(make_pair("DD", "DETAILS"));

    //_rexConstrainsConsolidator = _memHandle.insert(new RexConstrainsConsolidator(*_trx));
    //_newItin = _memHandle.create<Itin>();
    //_trx->newItin().push_back(_newItin);
    //_trx->itin().push_back(_newItin);
    //_newItinSegs = &(_newItin->travelSeg());
    //_excItin = _memHandle.create<ExcItin>();
    //_trx->exchangeItin().push_back(_excItin);
    //_excItinSegs = &(_excItin->travelSeg());
    //_trx->setShopOnlyCurrentItin(false);
    //_memHandle.create<MyDataHandle>(_memHandle);
  }

  void tearDown() { _memHandle.clear(); }

  void testdetachFlownToNewLeg_test()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "LAX", "2009-09-01 09:00");

    std::vector<RexConstrainsConsolidator::ONDInf> expectedONDInfo;
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("DFW", "JFK", "2009-09-01 09:00");
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("JFK", "LAX", "2009-09-02 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertONDInfo(expectedONDInfo, _rexConstrainsConsolidator->_ondInfo);
  }

  void testdetachFlownToNewLeg_segVectIsEmpty()
  {
    cleanStructures();
    addOdThruFM("DFW", "LAX", "2009-09-01 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    CPPUNIT_ASSERT(_rexConstrainsConsolidator->_ondInfo.empty());
  }

  void testdetachFlownToNewLeg_ONDIsEmpty()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    CPPUNIT_ASSERT(_rexConstrainsConsolidator->_ondInfo.empty());
  }

  void testdetachFlownToNewLeg_oneFlownSeg()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "JFK", "2009-09-01 09:00");

    std::vector<RexConstrainsConsolidator::ONDInf> expectedONDInfo;
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("DFW", "JFK", "2009-09-01 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertONDInfo(expectedONDInfo, _rexConstrainsConsolidator->_ondInfo);
  }

  void testdetachFlownToNewLeg_oneUnflownSeg()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "JFK", "2009-09-01 09:00");

    std::vector<RexConstrainsConsolidator::ONDInf> expectedONDInfo;
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("DFW", "JFK", "2009-09-01 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertONDInfo(expectedONDInfo, _rexConstrainsConsolidator->_ondInfo);
  }

  void testdetachFlownToNewLeg_allFlown()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", false, "2009-09-03 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "MIA", "2009-09-01 09:00");

    std::vector<RexConstrainsConsolidator::ONDInf> expectedONDInfo;
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("DFW", "MIA", "2009-09-01 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertONDInfo(expectedONDInfo, _rexConstrainsConsolidator->_ondInfo);
  }

  void testdetachFlownToNewLeg_allUnflown()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "MIA", "2009-09-01 09:00");

    std::vector<RexConstrainsConsolidator::ONDInf> expectedONDInfo;
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("DFW", "MIA", "2009-09-01 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertONDInfo(expectedONDInfo, _rexConstrainsConsolidator->_ondInfo);
  }

  void testdetachFlownToNewLeg_flownUnflown()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "MIA", "2009-09-01 09:00");

    std::vector<RexConstrainsConsolidator::ONDInf> expectedONDInfo;
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("DFW", "JFK", "2009-09-01 09:00");
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("JFK", "MIA", "2009-09-02 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertONDInfo(expectedONDInfo, _rexConstrainsConsolidator->_ondInfo);
  }

  void testdetachFlownToNewLeg_flownUnflown2()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "MIA", "2009-09-01 09:00");

    std::vector<RexConstrainsConsolidator::ONDInf> expectedONDInfo;
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("DFW", "LAX", "2009-09-01 09:00");
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("LAX", "MIA", "2009-09-03 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertONDInfo(expectedONDInfo, _rexConstrainsConsolidator->_ondInfo);
  }

  void testdetachFlownToNewLeg_multipleOND_breakInFirstOND()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "MIA", "2009-09-01 09:00");
    addOdThruFM("MIA", "DFW", "2009-09-04 09:00");

    std::vector<RexConstrainsConsolidator::ONDInf> expectedONDInfo;
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("DFW", "LAX", "2009-09-01 09:00");
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("LAX", "MIA", "2009-09-03 09:00");
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("MIA", "DFW", "2009-09-04 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertONDInfo(expectedONDInfo, _rexConstrainsConsolidator->_ondInfo);
  }

  void testdetachFlownToNewLeg_multipleOND_breakInSecondOND()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", false, "2009-09-03 09:00");
    addSegment(_newItinSegs, "MIA", "KRK", false, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "MIA", "2009-09-01 09:00");
    addOdThruFM("MIA", "DFW", "2009-09-04 09:00");

    std::vector<RexConstrainsConsolidator::ONDInf> expectedONDInfo;
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("DFW", "MIA", "2009-09-01 09:00");
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("MIA", "KRK", "2009-09-04 09:00");
    expectedONDInfo += RexConstrainsConsolidator::ONDInf("KRK", "DFW", "2009-09-05 09:00");

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertONDInfo(expectedONDInfo, _rexConstrainsConsolidator->_ondInfo);
  }

  void testdetachFlownToNewLeg_shoppedUnshopped()
  {
    cleanStructures();
    bool shopped = true;
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", shopped);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", false, "2009-09-03 09:00", shopped);
    addSegment(_newItinSegs, "MIA", "KRK", false, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", shopped);
    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    addOdThruFM("DFW", "DFW", "2009-09-01 09:00");

    std::vector<int> expectedUnshoppedForFirstOND;
    expectedUnshoppedForFirstOND += 2, 4;
    std::vector<int> expectedUnshoppedForSecondOND;
    expectedUnshoppedForSecondOND += 5;

    _rexConstrainsConsolidator->detachFlownToNewLeg();

    assertUnshopped(expectedUnshoppedForFirstOND,
                    _rexConstrainsConsolidator->_ondInfo.front()->unshoppedFlights);
    assertUnshopped(expectedUnshoppedForSecondOND,
                    _rexConstrainsConsolidator->_ondInfo.back()->unshoppedFlights);
  }

  void testmatchONDInfotoExcOND_test()
  {
    setData();

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;
    ondInfoToFM[_firstONDInfo] += _firstFareMarket;
    ondInfoToFM[_secondONDInfo] += _firstFareMarket;
    ondInfoToFM[_thirdONDInfo] += _secondFareMarket;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_noOND()
  {
    setData();
    _rexConstrainsConsolidator->_ondInfo.clear();

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_noFC()
  {
    setData();
    _excItin->fareComponent().clear();

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_noExcTvlSeg()
  {
    setData();
    _excItinSegs->clear();
    _rexConstrainsConsolidator->_excItinSeg.clear();

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_oneOND()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    addOdThruFM("DFW", "DFW", "2009-09-01 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;
    ondInfoToFM[_firstONDInfo] += _firstFareMarket, _secondFareMarket;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_twoOND()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    addOdThruFM("DFW", "LAX", "2009-09-01 09:00");
    addOdThruFM("LAX", "DFW", "2009-09-03 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;
    ondInfoToFM[_firstONDInfo] += _firstFareMarket;
    ondInfoToFM[_secondONDInfo] += _firstFareMarket, _secondFareMarket;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_twoOND_firstIsOneSeg()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    addOdThruFM("DFW", "JFK", "2009-09-01 09:00");
    addOdThruFM("JFK", "DFW", "2009-09-02 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;
    ondInfoToFM[_firstONDInfo] += _firstFareMarket;
    ondInfoToFM[_secondONDInfo] += _firstFareMarket, _secondFareMarket;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_twoOND_lastIsOneSeg()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    addOdThruFM("DFW", "FRA", "2009-09-01 09:00");
    addOdThruFM("FRA", "DFW", "2009-09-06 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;
    ondInfoToFM[_firstONDInfo] += _firstFareMarket, _secondFareMarket;
    ondInfoToFM[_secondONDInfo] += _secondFareMarket;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_maxNoOfOND()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    addOdThruFM("DFW", "JFK", "2009-09-01 09:00");
    addOdThruFM("JFK", "LAX", "2009-09-02 09:00");
    addOdThruFM("LAX", "MIA", "2009-09-03 09:00");
    addOdThruFM("MIA", "KRK", "2009-09-04 09:00");
    addOdThruFM("KRK", "FRA", "2009-09-05 09:00");
    addOdThruFM("FRA", "DFW", "2009-09-06 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _thirdONDInfo = _rexConstrainsConsolidator->_ondInfo[2];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;
    ondInfoToFM[_firstONDInfo] += _firstFareMarket;
    ondInfoToFM[_secondONDInfo] += _firstFareMarket;
    ondInfoToFM[_thirdONDInfo] += _firstFareMarket;
    ondInfoToFM[_rexConstrainsConsolidator->_ondInfo[3]] += _secondFareMarket;
    ondInfoToFM[_rexConstrainsConsolidator->_ondInfo[4]] += _secondFareMarket;
    ondInfoToFM[_rexConstrainsConsolidator->_ondInfo[5]] += _secondFareMarket;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_cannotMatchOND()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "PAR", true, "2009-09-06 09:00");
    addOdThruFM("DFW", "PAR", "2009-09-01 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;
    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testmatchONDInfotoExcOND_cannotMatchSecondOND()
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    addSegment(_newItinSegs, "FRA", "PAR", true, "2009-09-06 09:00");
    addOdThruFM("DFW", "LAX", "2009-09-01 09:00");
    addOdThruFM("LAX", "PAR", "2009-09-03 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", true, "2009-09-01 09:00");
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", true, "2009-09-02 09:00");
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00");
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00");
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00");
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00");
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;
    ondInfoToFM[_firstONDInfo] += _firstFareMarket;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testConsolideConstrainsForONDInfo_test()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexShoppingTrx::PortionMergeTvlVectType firstPortion;
    firstPortion += 1, 2, 3;
    addPortionToOADRsponse(_firstFareMarket, firstPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 4, 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<int> firstONDPortion;
    firstONDPortion += 1, 2;
    addFltRestWithBkg(firstCons, firstONDPortion);
    std::set<int> firstONDUnshFltRest;
    firstONDUnshFltRest += 1, 2;
    addUnshoppedFltRest(firstCons, firstONDUnshFltRest);
    addFltRest(firstCons, firstONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    std::set<int> secondONDPortion;
    secondONDPortion += 3;
    addFltRestWithBkg(secondCons, secondONDPortion);
    std::set<int> secondONDUnshFltRest;
    secondONDUnshFltRest += 3;
    addUnshoppedFltRest(secondCons, secondONDUnshFltRest);
    addFltRest(secondCons, secondONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<int> thirdONDPortion;
    thirdONDPortion += 4, 5;
    addFltRestWithBkg(thirdCons, thirdONDPortion);
    std::set<int> thirdONDUnshFltRest;
    thirdONDUnshFltRest += 4, 5, 6;
    addUnshoppedFltRest(thirdCons, thirdONDUnshFltRest);
    addFltRest(thirdCons, thirdONDUnshFltRest);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_portionOnly()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexShoppingTrx::PortionMergeTvlVectType firstPortion;
    firstPortion += 1, 3;
    addPortionToOADRsponse(_firstFareMarket, firstPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<int> firstONDPortion;
    firstONDPortion += 1;
    addFltRestWithBkg(firstCons, firstONDPortion);
    std::set<int> firstONDUnshFltRest;
    firstONDUnshFltRest += 1, 2;
    addUnshoppedFltRest(firstCons, firstONDUnshFltRest);
    addFltRest(firstCons, firstONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    std::set<int> secondONDPortion;
    secondONDPortion += 3;
    addFltRestWithBkg(secondCons, secondONDPortion);
    std::set<int> secondONDUnshFltRest;
    secondONDUnshFltRest += 3;
    addUnshoppedFltRest(secondCons, secondONDUnshFltRest);
    addFltRest(secondCons, secondONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<int> thirdONDPortion;
    thirdONDPortion += 5;
    addFltRestWithBkg(thirdCons, thirdONDPortion);
    std::set<int> thirdONDUnshFltRest;
    thirdONDUnshFltRest += 4, 5, 6;
    addUnshoppedFltRest(thirdCons, thirdONDUnshFltRest);
    addFltRest(thirdCons, thirdONDUnshFltRest);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_outboundPortionOnly()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexShoppingTrx::PortionMergeTvlVectType firstOutboundPortion;
    firstOutboundPortion += 1;
    addOutboundPortionToOADRsponse(_firstFareMarket, firstOutboundPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondOutboundPortion;
    secondOutboundPortion += 5;
    addOutboundPortionToOADRsponse(_secondFareMarket, secondOutboundPortion);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<int> firstONDPortion;
    firstONDPortion += 1;
    addFltRestWithBkg(firstCons, firstONDPortion);
    std::set<int> firstONDUnshFltRest;
    firstONDUnshFltRest += 1, 2;
    addUnshoppedFltRest(firstCons, firstONDUnshFltRest);
    addFltRest(firstCons, firstONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    std::set<int> secondONDUnshFltRest;
    secondONDUnshFltRest += 3;
    addUnshoppedFltRest(secondCons, secondONDUnshFltRest);
    addFltRest(secondCons, secondONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<int> thirdONDPortion;
    thirdONDPortion += 5;
    addFltRestWithBkg(thirdCons, thirdONDPortion);
    std::set<int> thirdONDUnshFltRest;
    thirdONDUnshFltRest += 4, 5, 6;
    addUnshoppedFltRest(thirdCons, thirdONDUnshFltRest);
    addFltRest(thirdCons, thirdONDUnshFltRest);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_unshoppedFlightsOnly()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<int> firstONDUnshFltRest;
    firstONDUnshFltRest += 1, 2;
    addUnshoppedFltRest(firstCons, firstONDUnshFltRest);
    addFltRest(firstCons, firstONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    std::set<int> secondONDUnshFltRest;
    secondONDUnshFltRest += 3;
    addUnshoppedFltRest(secondCons, secondONDUnshFltRest);
    addFltRest(secondCons, secondONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<int> thirdONDUnshFltRest;
    thirdONDUnshFltRest += 4, 5, 6;
    addUnshoppedFltRest(thirdCons, thirdONDUnshFltRest);
    addFltRest(thirdCons, thirdONDUnshFltRest);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_allFltRestWithBkg()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    // portion
    RexShoppingTrx::PortionMergeTvlVectType firstPortion;
    firstPortion += 1, 3;
    addPortionToOADRsponse(_firstFareMarket, firstPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    // outbound portion
    RexShoppingTrx::PortionMergeTvlVectType firstOutboundPortion;
    firstOutboundPortion += 2;
    addOutboundPortionToOADRsponse(_firstFareMarket, firstOutboundPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondOutboundPortion;
    secondOutboundPortion += 4;
    addOutboundPortionToOADRsponse(_secondFareMarket, secondOutboundPortion);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<int> firstONDPortion;
    firstONDPortion += 1, 2;
    addFltRestWithBkg(firstCons, firstONDPortion);
    std::set<int> firstONDUnshFltRest;
    firstONDUnshFltRest += 1, 2;
    addUnshoppedFltRest(firstCons, firstONDUnshFltRest);
    addFltRest(firstCons, firstONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    std::set<int> secondONDPortion;
    secondONDPortion += 3;
    addFltRestWithBkg(secondCons, secondONDPortion);
    std::set<int> secondONDUnshFltRest;
    secondONDUnshFltRest += 3;
    addUnshoppedFltRest(secondCons, secondONDUnshFltRest);
    addFltRest(secondCons, secondONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<int> thirdONDPortion;
    thirdONDPortion += 4, 5;
    addFltRestWithBkg(thirdCons, thirdONDPortion);
    std::set<int> thirdONDUnshFltRest;
    thirdONDUnshFltRest += 4, 5, 6;
    addUnshoppedFltRest(thirdCons, thirdONDUnshFltRest);
    addFltRest(thirdCons, thirdONDUnshFltRest);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_flightNoRest_allFltRest()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    bool flightNoRest = true;
    setFlightNumnerRest(_firstFareMarket, flightNoRest);
    setFlightNumnerRest(_secondFareMarket, !flightNoRest);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<int> firstONDUnshFltRest;
    firstONDUnshFltRest += 1, 2;
    addUnshoppedFltRest(firstCons, firstONDUnshFltRest);
    addFltRest(firstCons, firstONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    std::set<int> secondONDUnshFltRest;
    secondONDUnshFltRest += 3;
    addUnshoppedFltRest(secondCons, secondONDUnshFltRest);
    addFltRest(secondCons, secondONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<int> thirdONDUnshFltRest;
    thirdONDUnshFltRest += 4, 5, 6;
    addUnshoppedFltRest(thirdCons, thirdONDUnshFltRest);
    addFltRest(thirdCons, thirdONDUnshFltRest);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = true;
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    addCxrCons(firstCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    addCxrCons(thirdCons, secondCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_R3MergedData_empty()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_govCxrIsPrefIn1FM()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = true;
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    std::set<CarrierCode> cxrList;
    cxrList += "AA", "BB";
    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    addCxrCons(firstCons, cxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, cxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    addCxrCons(thirdCons, secondCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_govCxrIsPrefIn1FM_addGov()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = false;
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    std::set<CarrierCode> cxrList;
    cxrList += "AA", "BB", "G1";
    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    addCxrCons(firstCons, cxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, cxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    addCxrCons(thirdCons, secondCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_govCxrIsPrefIn2FM()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = false;
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    std::set<CarrierCode> cxrList;
    cxrList += "CC", "DD";
    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    addCxrCons(firstCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    addCxrCons(thirdCons, cxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_firstFMCxrListEmpty()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = true;
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    addCxrCons(thirdCons, secondCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_secondFMCxrListEmpty()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    bool excludedCxrList = true;
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    addCxrCons(firstCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_userCxrList_empty()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = true;
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    addCxrCons(firstCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    addCxrCons(thirdCons, secondCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_userCxrList_R3cxrList_empty()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = false;
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    bool userExcludedCxrList = false;
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "CC";
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "CC";
    bool usrExcludedCxrList = true;
    addUserCxrList(userCxrList, userExcludedCxrList);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, !excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, !excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<CarrierCode> secondConsCxrList;
    secondConsCxrList += "CC", "DD";
    addCxrCons(thirdCons, secondConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_userCxrList_R3cxrList_exc()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, !excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "CC";
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "CC";
    bool usrExcludedCxrList = true;
    addUserCxrList(userCxrList, excludedCxrList);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "BB";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<CarrierCode> secondConsCxrList;
    secondConsCxrList += "CC", "DD";
    addCxrCons(thirdCons, secondConsCxrList, usrCxrList, !excludedCxrList, usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_userCxrList_R3cxrList_pref()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, !excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "CC";
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "CC";
    bool usrExcludedCxrList = true;
    addUserCxrList(userCxrList, !excludedCxrList);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "AA", "BB";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<CarrierCode> secondConsCxrList;
    secondConsCxrList += "CC", "DD";
    addCxrCons(thirdCons, secondConsCxrList, usrCxrList, !excludedCxrList, !usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test1()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB", "G1";
    bool excludedCxrList = true;
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, !excludedCxrList);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "AA", "BB", "G1";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, !excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, !excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test2()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, !excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA";
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA";
    bool usrExcludedCxrList = true;
    addUserCxrList(userCxrList, !excludedCxrList);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "AA";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, !excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, !excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test3()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "CC", "DD";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "BB";
    addUserCxrList(userCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "BB";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "CC", "DD";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test4()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "AA", "BB";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test5()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "BB";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, !excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA";
    addUserCxrList(userCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "BB";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, !excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, !excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test6()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA";
    addUserCxrList(userCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test7()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "BB";
    addUserCxrList(userCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "BB";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test8()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA";
    addUserCxrList(userCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "BB";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test9()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    bool excludedCxrList = true;
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA";
    addUserCxrList(userCxrList, !excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test10()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "BB";
    addUserCxrList(userCxrList, !excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "BB";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "AA";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<CarrierCode> secondConsCxrList;
    addCxrCons(thirdCons, secondConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test11()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB", "CC";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "BB";
    addUserCxrList(userCxrList, !excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "BB";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "AA", "BB", "CC";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<CarrierCode> secondConsCxrList;
    addCxrCons(thirdCons, secondConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test12()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "CC";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "BB";
    addUserCxrList(userCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "BB";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "CC";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<CarrierCode> secondConsCxrList;
    addCxrCons(thirdCons, secondConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test13()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "G1";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, !excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "AA", "G1";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, !excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, !excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test14()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "CC";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "CC";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test15()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "CC";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, !excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "BB";
    addUserCxrList(userCxrList, !excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "BB";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "CC";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, !excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, !excludedCxrList, !usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<CarrierCode> secondConsCxrList;
    addCxrCons(thirdCons, secondConsCxrList, usrCxrList, excludedCxrList, !usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test16()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    bool excludedCxrList = true;
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA";
    addUserCxrList(userCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_carrierRest_test17()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "CC";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA";
    addUserCxrList(userCxrList, excludedCxrList);
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA";
    bool usrExcludedCxrList = true;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<CarrierCode> firstConsCxrList;
    firstConsCxrList += "CC";
    addCxrCons(firstCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<CarrierCode> secondConsCxrList;
    addCxrCons(thirdCons, secondConsCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_skipFM_fltRest()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    // portion
    RexShoppingTrx::PortionMergeTvlVectType firstPortion;
    firstPortion += 1, 3;
    addPortionToOADRsponse(_firstFareMarket, firstPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    // outbound portion
    RexShoppingTrx::PortionMergeTvlVectType firstOutboundPortion;
    firstOutboundPortion += 2;
    addOutboundPortionToOADRsponse(_firstFareMarket, firstOutboundPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondOutboundPortion;
    secondOutboundPortion += 4;
    addOutboundPortionToOADRsponse(_secondFareMarket, secondOutboundPortion);

    // add FM to skip
    _rexConstrainsConsolidator->_fmToSkipConsRest += _firstFareMarket;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<int> firstONDUnshFltRest;
    firstONDUnshFltRest += 1, 2;
    addUnshoppedFltRest(firstCons, firstONDUnshFltRest);
    addFltRest(firstCons, firstONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    std::set<int> secondONDUnshFltRest;
    secondONDUnshFltRest += 3;
    addUnshoppedFltRest(secondCons, secondONDUnshFltRest);
    addFltRest(secondCons, secondONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<int> thirdONDPortion;
    thirdONDPortion += 4, 5;
    addFltRestWithBkg(thirdCons, thirdONDPortion);
    std::set<int> thirdONDUnshFltRest;
    thirdONDUnshFltRest += 4, 5, 6;
    addUnshoppedFltRest(thirdCons, thirdONDUnshFltRest);
    addFltRest(thirdCons, thirdONDUnshFltRest);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testConsolideConstrainsForONDInfo_skipFM_carrierRest()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, firstBreakIsRestricted);
    std::set<CarrierCode> firstCxrList;
    firstCxrList += "AA", "BB";
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = true;
    setCarrierListInOADRsponse(_firstFareMarket, firstCxrList, excludedCxrList);
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    std::set<CarrierCode> usrCxrList;
    bool usrExcludedCxrList = true;

    // add FM to skip
    _rexConstrainsConsolidator->_fmToSkipConsRest += _secondFareMarket;

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    addCxrCons(firstCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    addCxrCons(secondCons, firstCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testCreateSODForONDInfo_test()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    // portion
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    // outbound portion
    RexShoppingTrx::PortionMergeTvlVectType firstOutboundPortion;
    firstOutboundPortion += 3;
    addOutboundPortionToOADRsponse(_firstFareMarket, firstOutboundPortion);

    // flight no
    bool flightNoRest = true;
    setFlightNumnerRest(_firstFareMarket, flightNoRest);
    setFlightNumnerRest(_secondFareMarket, !flightNoRest);

    // forced connections
    RexShoppingTrx::ForcedConnectionSet forcedConnections;
    forcedConnections += "JFK", "LAX", "KRK", "FRA", "DFW";
    setFrcCnnxInOADRsponse(_firstFareMarket, forcedConnections);

    // carriers
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> secondCxrList;
    secondCxrList += "CC", "DD";
    bool excludedCxrList = false;
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);
    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");
    bool userExcludedCxrList = false;
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "CC";
    addUserCxrList(userCxrList, userExcludedCxrList);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();
    _rexConstrainsConsolidator->createSODForONDInfo();

    _rexConstrainsConsolidator->printConsolidatedConstrainsWithSOD();

    bool change = true;
    bool exact_cxr = true;
    bool preferred_cxr = true;
    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", true);
    oadConsRest += firstCons;
    RexShoppingTrx::SubOriginDestination* sod1ond1 =
        createSOD("DFW", "LAX", "2009-09-01 09:00", !change, exact_cxr, !preferred_cxr);
    std::set<int> sod1ond1flt;
    sod1ond1flt += 1, 2;
    addFlt(sod1ond1, sod1ond1flt);
    firstCons->sod.push_back(sod1ond1);

    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    RexShoppingTrx::SubOriginDestination* sod1ond2 =
        createSOD("LAX", "MIA", "2009-09-03 09:00", !change, exact_cxr, !preferred_cxr);
    std::set<int> sod1ond2flt;
    sod1ond2flt += 3;
    addFlt(sod1ond2, sod1ond2flt);
    secondCons->sod.push_back(sod1ond2);

    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    RexShoppingTrx::SubOriginDestination* sod1ond3 =
        createSOD("MIA", "KRK", "2009-09-04 09:00", change, exact_cxr, preferred_cxr);
    thirdCons->sod.push_back(sod1ond3);
    RexShoppingTrx::SubOriginDestination* sod2ond3 =
        createSOD("KRK", "FRA", "2009-09-05 09:00", !change, exact_cxr, preferred_cxr);
    std::set<int> sod2ond3flt;
    sod2ond3flt += 5;
    addFlt(sod2ond3, sod2ond3flt);
    thirdCons->sod.push_back(sod2ond3);
    RexShoppingTrx::SubOriginDestination* sod3ond3 =
        createSOD("FRA", "DFW", "2009-09-06 09:00", change, exact_cxr, preferred_cxr);
    thirdCons->sod.push_back(sod3ond3);

    assertSODinOAD(oadConsRest, _trx->oadConsRest());
  }

  void testCreateSODForONDInfo_cxrListEmpty()
  {
    setData(true);
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    // portion
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    // outbound portion
    RexShoppingTrx::PortionMergeTvlVectType firstOutboundPortion;
    firstOutboundPortion += 3;
    addOutboundPortionToOADRsponse(_firstFareMarket, firstOutboundPortion);

    // flight no
    bool flightNoRest = true;
    setFlightNumnerRest(_firstFareMarket, flightNoRest);
    setFlightNumnerRest(_secondFareMarket, !flightNoRest);

    // forced connections
    RexShoppingTrx::ForcedConnectionSet forcedConnections;
    forcedConnections += "JFK", "LAX", "KRK", "FRA", "DFW";
    setFrcCnnxInOADRsponse(_firstFareMarket, forcedConnections);

    // carriers
    setFMToFirstFMinPU(_firstFareMarket, _firstFareMarket);
    setFMToFirstFMinPU(_firstFareMarket, _secondFareMarket);
    bool firstBreakIsRestricted = true;
    setFirstBreakRestInOADRsponse(_firstFareMarket, !firstBreakIsRestricted);
    setFirstBreakRestInOADRsponse(_secondFareMarket, !firstBreakIsRestricted);
    std::set<CarrierCode> secondCxrList;
    bool excludedCxrList = false;
    setCarrierListInOADRsponse(_secondFareMarket, secondCxrList, excludedCxrList);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();
    _rexConstrainsConsolidator->createSODForONDInfo();

    bool change = true;
    bool exact_cxr = true;
    bool preferred_cxr = true;
    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFW", "LAX", "2009-09-01 09:00", true);
    oadConsRest += firstCons;
    RexShoppingTrx::SubOriginDestination* sod1ond1 =
        createSOD("DFW", "LAX", "2009-09-01 09:00", !change, !exact_cxr, !preferred_cxr);
    std::set<int> sod1ond1flt;
    sod1ond1flt += 1, 2;
    addFlt(sod1ond1, sod1ond1flt);
    firstCons->sod.push_back(sod1ond1);

    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIA", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    RexShoppingTrx::SubOriginDestination* sod1ond2 =
        createSOD("LAX", "MIA", "2009-09-03 09:00", !change, !exact_cxr, !preferred_cxr);
    std::set<int> sod1ond2flt;
    sod1ond2flt += 3;
    addFlt(sod1ond2, sod1ond2flt);
    secondCons->sod.push_back(sod1ond2);

    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIA", "DFW", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    RexShoppingTrx::SubOriginDestination* sod1ond3 =
        createSOD("MIA", "KRK", "2009-09-04 09:00", change, !exact_cxr, preferred_cxr);
    thirdCons->sod.push_back(sod1ond3);
    RexShoppingTrx::SubOriginDestination* sod2ond3 =
        createSOD("KRK", "FRA", "2009-09-05 09:00", !change, !exact_cxr, preferred_cxr);
    std::set<int> sod2ond3flt;
    sod2ond3flt += 5;
    addFlt(sod2ond3, sod2ond3flt);
    thirdCons->sod.push_back(sod2ond3);
    RexShoppingTrx::SubOriginDestination* sod3ond3 =
        createSOD("FRA", "DFW", "2009-09-06 09:00", change, !exact_cxr, preferred_cxr);
    thirdCons->sod.push_back(sod3ond3);

    assertSODinOAD(oadConsRest, _trx->oadConsRest());
  }

  void testMergeOADCrxListWithPSSCrxList_fareByteCxrApplNotSet()
  {
    bool userExcludedCxrList = false;
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "CC";
    addUserCxrList(userCxrList, userExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* cons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);

    for (auto& constr : cons->sodConstraints | boost::adaptors::map_values)
    {
      _rexConstrainsConsolidator->mergeOADCrxListWithPSSCrxList(constr);
      CPPUNIT_ASSERT(constr.fareByteCxrAppl != nullptr);
      CPPUNIT_ASSERT(constr.fareByteCxrAppl->govCxr.cxrList.empty());
      CPPUNIT_ASSERT(!constr.fareByteCxrAppl->usrCxr.cxrList.empty());
      CPPUNIT_ASSERT(constr.fareByteCxrAppl->usrCxr.excluded == false);
    }
  }

  void testMergeOADCrxListWithPSSCrxList_fareByteCxrApplEmpty()
  {
    bool userExcludedCxrList = true;
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "CC";
    addUserCxrList(userCxrList, userExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* cons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    std::set<CarrierCode> consCxrList;
    bool excludedCxrList = true;
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "BB";
    bool usrExcludedCxrList = true;
    addCxrCons(cons, consCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    for (auto& constr : cons->sodConstraints | boost::adaptors::map_values)
    {
      const RexShoppingTrx::FareByteCxrData* oldFareByteCxrData = constr.fareByteCxrAppl;
      _rexConstrainsConsolidator->mergeOADCrxListWithPSSCrxList(constr);
      CPPUNIT_ASSERT(constr.fareByteCxrAppl == oldFareByteCxrData);
      CPPUNIT_ASSERT(constr.fareByteCxrAppl->govCxr.cxrList.empty());
      CPPUNIT_ASSERT(!constr.fareByteCxrAppl->usrCxr.cxrList.empty());
      CPPUNIT_ASSERT(constr.fareByteCxrAppl->govCxr.excluded == true);
    }
  }

  void testMergeOADCrxListWithPSSCrxList_govIsNotExcluded()
  {
    bool userExcludedCxrList = true;
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "CC";
    addUserCxrList(userCxrList, userExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* cons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    std::set<CarrierCode> consCxrList;
    consCxrList += "AA", "BB";
    bool excludedCxrList = false;
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "BB";
    bool usrExcludedCxrList = true;
    addCxrCons(cons, consCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    for (auto& constr : cons->sodConstraints | boost::adaptors::map_values)
    {
      const RexShoppingTrx::FareByteCxrData* oldFareByteCxrData = constr.fareByteCxrAppl;
      _rexConstrainsConsolidator->mergeOADCrxListWithPSSCrxList(constr);
      std::set<CarrierCode> expectedGovCxrList;
      expectedGovCxrList += "AA", "BB";
      CPPUNIT_ASSERT(constr.fareByteCxrAppl == oldFareByteCxrData);
      assertCxrList(constr.fareByteCxrAppl->govCxr.cxrList, expectedGovCxrList);
      CPPUNIT_ASSERT(!constr.fareByteCxrAppl->usrCxr.cxrList.empty());
      CPPUNIT_ASSERT(constr.fareByteCxrAppl->govCxr.excluded == false);
    }
  }

  void testMergeOADCrxListWithPSSCrxList_removeUsrFromGov()
  {
    bool userExcludedCxrList = true;
    std::set<CarrierCode> userCxrList;
    userCxrList += "AA", "CC";
    addUserCxrList(userCxrList, userExcludedCxrList);
    RexShoppingTrx::OADConsolidatedConstrains* cons =
        addCons("DFW", "LAX", "2009-09-01 09:00", false);
    std::set<CarrierCode> consCxrList;
    consCxrList += "AA", "BB";
    bool excludedCxrList = true;
    std::set<CarrierCode> usrCxrList;
    usrCxrList += "AA", "BB";
    bool usrExcludedCxrList = true;
    addCxrCons(cons, consCxrList, usrCxrList, excludedCxrList, usrExcludedCxrList);

    for (auto& constr : cons->sodConstraints | boost::adaptors::map_values)
    {
      const RexShoppingTrx::FareByteCxrData* oldFareByteCxrData = constr.fareByteCxrAppl;
      _rexConstrainsConsolidator->mergeOADCrxListWithPSSCrxList(constr);
      std::set<CarrierCode> expectedGovCxrList;
      expectedGovCxrList += "BB";
      CPPUNIT_ASSERT(constr.fareByteCxrAppl == oldFareByteCxrData);
      assertCxrList(constr.fareByteCxrAppl->govCxr.cxrList, expectedGovCxrList);
      CPPUNIT_ASSERT(!constr.fareByteCxrAppl->usrCxr.cxrList.empty());
      CPPUNIT_ASSERT(constr.fareByteCxrAppl->govCxr.excluded == true);
    }
  }

  void testmatchONDInfotoExcOND_multiAirport()
  {
    cleanStructures();
    bool allShop = true;
    addSegment(_newItinSegs, "DFW", "JFK", "DFM", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", "LAX", "MIM", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", "MIM", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", "FRA", "DFM", true, "2009-09-06 09:00", allShop);
    addOdThruFM("DFM", "MIM", "2009-09-01 09:00");
    addOdThruFM("MIM", "DFM", "2009-09-04 09:00");
    AirSeg* airSeg1 =
        addSegment(_excItinSegs, "DFW", "JFK", "DFM", "JFK", false, "2009-09-01 09:00", allShop);
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    AirSeg* airSeg3 =
        addSegment(_excItinSegs, "LAX", "MIA", "LAX", "MIM", true, "2009-09-03 09:00", allShop);
    AirSeg* airSeg4 =
        addSegment(_excItinSegs, "MIA", "KRK", "MIM", "KRK", true, "2009-09-04 09:00", allShop);
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    AirSeg* airSeg6 =
        addSegment(_excItinSegs, "FRA", "DFW", "FRA", "DFM", true, "2009-09-06 09:00", allShop);
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _thirdONDInfo = _rexConstrainsConsolidator->_ondInfo[2];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexConstrainsConsolidator::ONDInfoToFM ondInfoToFM;
    ondInfoToFM[_firstONDInfo] += _firstFareMarket;
    ondInfoToFM[_secondONDInfo] += _firstFareMarket;
    ondInfoToFM[_thirdONDInfo] += _secondFareMarket;

    assertONDInfoToFM(ondInfoToFM, _rexConstrainsConsolidator->_ondInfoToFM);
  }

  void testConsolideConstrainsForONDInfo_multiAirport()
  {
    cleanStructures();
    bool allShop = false;
    addSegment(_newItinSegs, "DFW", "JFK", "DFM", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", "LAX", "MIM", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", "MIM", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", "FRA", "DFM", true, "2009-09-06 09:00", allShop);
    addOdThruFM("DFM", "MIM", "2009-09-01 09:00");
    addOdThruFM("MIM", "DFM", "2009-09-04 09:00");
    AirSeg* airSeg1 =
        addSegment(_excItinSegs, "DFW", "JFK", "DFM", "JFK", false, "2009-09-01 09:00", allShop);
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    AirSeg* airSeg3 =
        addSegment(_excItinSegs, "LAX", "MIA", "LAX", "MIM", true, "2009-09-03 09:00", allShop);
    AirSeg* airSeg4 =
        addSegment(_excItinSegs, "MIA", "KRK", "MIM", "KRK", true, "2009-09-04 09:00", allShop);
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    AirSeg* airSeg6 =
        addSegment(_excItinSegs, "FRA", "DFW", "FRA", "DFM", true, "2009-09-06 09:00", allShop);
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _thirdONDInfo = _rexConstrainsConsolidator->_ondInfo[2];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));

    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexShoppingTrx::PortionMergeTvlVectType firstPortion;
    firstPortion += 1, 2, 3;
    addPortionToOADRsponse(_firstFareMarket, firstPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 4, 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    RexShoppingTrx::OADConsolidatedConstrainsVect oadConsRest;
    RexShoppingTrx::OADConsolidatedConstrains* firstCons =
        addCons("DFM", "LAX", "2009-09-01 09:00", false);
    oadConsRest += firstCons;
    std::set<int> firstONDPortion;
    firstONDPortion += 1, 2;
    addFltRestWithBkg(firstCons, firstONDPortion);
    std::set<int> firstONDUnshFltRest;
    firstONDUnshFltRest += 1, 2;
    addUnshoppedFltRest(firstCons, firstONDUnshFltRest);
    addFltRest(firstCons, firstONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* secondCons =
        addCons("LAX", "MIM", "2009-09-03 09:00", false);
    oadConsRest += secondCons;
    std::set<int> secondONDPortion;
    secondONDPortion += 3;
    addFltRestWithBkg(secondCons, secondONDPortion);
    std::set<int> secondONDUnshFltRest;
    secondONDUnshFltRest += 3;
    addUnshoppedFltRest(secondCons, secondONDUnshFltRest);
    addFltRest(secondCons, secondONDUnshFltRest);
    RexShoppingTrx::OADConsolidatedConstrains* thirdCons =
        addCons("MIM", "DFM", "2009-09-04 09:00", false);
    oadConsRest += thirdCons;
    std::set<int> thirdONDPortion;
    thirdONDPortion += 4, 5;
    addFltRestWithBkg(thirdCons, thirdONDPortion);
    std::set<int> thirdONDUnshFltRest;
    thirdONDUnshFltRest += 4, 5, 6;
    addUnshoppedFltRest(thirdCons, thirdONDUnshFltRest);
    addFltRest(thirdCons, thirdONDUnshFltRest);

    assertONDConsRest(oadConsRest, _trx->oadConsRest());
  }

  void testGetFlownExcONDTvlSegPos_test()
  {
    cleanStructures();
    bool allShop = true;
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    const int firstONDNewTvlSegPos = 1;
    const int lastONDNewTvlSegPos = 2;
    int firstONDExcTvlSegPos = 0;
    int lastONDExcTvlSegPos = 0;
    bool matched = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    matched = _rexConstrainsConsolidator->getFlownExcONDTvlSegPos(
        firstONDNewTvlSegPos, lastONDNewTvlSegPos, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedMatched = true;
    int expectedFirstONDExcTvlSegPos = 0;
    int expectedLastONDExcTvlSegPos = 1;

    CPPUNIT_ASSERT(matched == expectedMatched);
    CPPUNIT_ASSERT(firstONDExcTvlSegPos == expectedFirstONDExcTvlSegPos);
    CPPUNIT_ASSERT(lastONDExcTvlSegPos == expectedLastONDExcTvlSegPos);
  }

  void testGetFlownExcONDTvlSegPos_emptyNewItinSegs()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    const int firstONDNewTvlSegPos = 1;
    const int lastONDNewTvlSegPos = 2;
    int firstONDExcTvlSegPos = 0;
    int lastONDExcTvlSegPos = 0;
    bool matched = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    matched = _rexConstrainsConsolidator->getFlownExcONDTvlSegPos(
        firstONDNewTvlSegPos, lastONDNewTvlSegPos, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedMatched = false;
    int expectedFirstONDExcTvlSegPos = 0;
    int expectedLastONDExcTvlSegPos = 0;

    CPPUNIT_ASSERT(matched == expectedMatched);
    CPPUNIT_ASSERT(firstONDExcTvlSegPos == expectedFirstONDExcTvlSegPos);
    CPPUNIT_ASSERT(lastONDExcTvlSegPos == expectedLastONDExcTvlSegPos);
  }

  void testGetFlownExcONDTvlSegPos_emptyExcItinSegs()
  {
    cleanStructures();
    bool allShop = true;
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    const int firstONDNewTvlSegPos = 1;
    const int lastONDNewTvlSegPos = 2;
    int firstONDExcTvlSegPos = 0;
    int lastONDExcTvlSegPos = 0;
    bool matched = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    matched = _rexConstrainsConsolidator->getFlownExcONDTvlSegPos(
        firstONDNewTvlSegPos, lastONDNewTvlSegPos, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedMatched = false;
    int expectedFirstONDExcTvlSegPos = 0;
    int expectedLastONDExcTvlSegPos = 0;

    CPPUNIT_ASSERT(matched == expectedMatched);
    CPPUNIT_ASSERT(firstONDExcTvlSegPos == expectedFirstONDExcTvlSegPos);
    CPPUNIT_ASSERT(lastONDExcTvlSegPos == expectedLastONDExcTvlSegPos);
  }

  void testGetFlownExcONDTvlSegPos_wrongFirstTvlSegPosNumber()
  {
    cleanStructures();
    bool allShop = true;
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    const int firstONDNewTvlSegPos = 0;
    const int lastONDNewTvlSegPos = 2;
    int firstONDExcTvlSegPos = 0;
    int lastONDExcTvlSegPos = 0;
    bool matched = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    matched = _rexConstrainsConsolidator->getFlownExcONDTvlSegPos(
        firstONDNewTvlSegPos, lastONDNewTvlSegPos, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedMatched = false;
    int expectedFirstONDExcTvlSegPos = 0;
    int expectedLastONDExcTvlSegPos = 0;

    CPPUNIT_ASSERT(matched == expectedMatched);
    CPPUNIT_ASSERT(firstONDExcTvlSegPos == expectedFirstONDExcTvlSegPos);
    CPPUNIT_ASSERT(lastONDExcTvlSegPos == expectedLastONDExcTvlSegPos);
  }

  void testGetFlownExcONDTvlSegPos_wrongLastTvlSegPosNumber()
  {
    cleanStructures();
    bool allShop = true;
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    const int firstONDNewTvlSegPos = 1;
    const int lastONDNewTvlSegPos = 7;
    int firstONDExcTvlSegPos = 0;
    int lastONDExcTvlSegPos = 0;
    bool matched = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    matched = _rexConstrainsConsolidator->getFlownExcONDTvlSegPos(
        firstONDNewTvlSegPos, lastONDNewTvlSegPos, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedMatched = false;
    int expectedFirstONDExcTvlSegPos = 0;
    int expectedLastONDExcTvlSegPos = 0;

    CPPUNIT_ASSERT(matched == expectedMatched);
    CPPUNIT_ASSERT(firstONDExcTvlSegPos == expectedFirstONDExcTvlSegPos);
    CPPUNIT_ASSERT(lastONDExcTvlSegPos == expectedLastONDExcTvlSegPos);
  }

  void testGetFlownExcONDTvlSegPos_matchFirstSeg()
  {
    cleanStructures();
    bool allShop = true;
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    const int firstONDNewTvlSegPos = 1;
    const int lastONDNewTvlSegPos = 1;
    int firstONDExcTvlSegPos = 0;
    int lastONDExcTvlSegPos = 0;
    bool matched = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    matched = _rexConstrainsConsolidator->getFlownExcONDTvlSegPos(
        firstONDNewTvlSegPos, lastONDNewTvlSegPos, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedMatched = true;
    int expectedFirstONDExcTvlSegPos = 0;
    int expectedLastONDExcTvlSegPos = 0;

    CPPUNIT_ASSERT(matched == expectedMatched);
    CPPUNIT_ASSERT(firstONDExcTvlSegPos == expectedFirstONDExcTvlSegPos);
    CPPUNIT_ASSERT(lastONDExcTvlSegPos == expectedLastONDExcTvlSegPos);
  }

  void testGetFlownExcONDTvlSegPos_matchAll()
  {
    cleanStructures();
    bool allShop = true;
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    const int firstONDNewTvlSegPos = 1;
    const int lastONDNewTvlSegPos = 6;
    int firstONDExcTvlSegPos = 0;
    int lastONDExcTvlSegPos = 0;
    bool matched = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    matched = _rexConstrainsConsolidator->getFlownExcONDTvlSegPos(
        firstONDNewTvlSegPos, lastONDNewTvlSegPos, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedMatched = true;
    int expectedFirstONDExcTvlSegPos = 0;
    int expectedLastONDExcTvlSegPos = 5;

    CPPUNIT_ASSERT(matched == expectedMatched);
    CPPUNIT_ASSERT(firstONDExcTvlSegPos == expectedFirstONDExcTvlSegPos);
    CPPUNIT_ASSERT(lastONDExcTvlSegPos == expectedLastONDExcTvlSegPos);
  }

  void testGetFlownExcONDTvlSegPos_matchLast()
  {
    cleanStructures();
    bool allShop = true;
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    const int firstONDNewTvlSegPos = 6;
    const int lastONDNewTvlSegPos = 6;
    int firstONDExcTvlSegPos = 0;
    int lastONDExcTvlSegPos = 0;
    bool matched = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    matched = _rexConstrainsConsolidator->getFlownExcONDTvlSegPos(
        firstONDNewTvlSegPos, lastONDNewTvlSegPos, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedMatched = true;
    int expectedFirstONDExcTvlSegPos = 5;
    int expectedLastONDExcTvlSegPos = 5;

    CPPUNIT_ASSERT(matched == expectedMatched);
    CPPUNIT_ASSERT(firstONDExcTvlSegPos == expectedFirstONDExcTvlSegPos);
    CPPUNIT_ASSERT(lastONDExcTvlSegPos == expectedLastONDExcTvlSegPos);
  }

  void ondHasPrimarySectorInExcFlownPart_noFM()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    FareMarket* fareMarket = 0;

    const int firstONDExcTvlSegPos = 0;
    const int lastONDExcTvlSegPos = 0;
    bool found = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    found = _rexConstrainsConsolidator->ondHasPrimarySectorInExcFlownPart(
        fareMarket, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedFound = false;

    CPPUNIT_ASSERT(found == expectedFound);
  }

  void ondHasPrimarySectorInExcFlownPart_primarySegOutOfVect()
  {
    cleanStructures();
    bool allShop = true;

    TravelSeg* seg = addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    FareMarket* fareMarket = FM(seg);

    const int firstONDExcTvlSegPos = 0;
    const int lastONDExcTvlSegPos = 5;
    bool found = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    found = _rexConstrainsConsolidator->ondHasPrimarySectorInExcFlownPart(
        fareMarket, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedFound = false;

    CPPUNIT_ASSERT(found == expectedFound);
  }

  void ondHasPrimarySectorInExcFlownPart_primarySegNotFound()
  {
    cleanStructures();
    bool allShop = true;

    TravelSeg* seg = addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    FareMarket* fareMarket = FM(seg);

    const int firstONDExcTvlSegPos = 1;
    const int lastONDExcTvlSegPos = 5;
    bool found = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    found = _rexConstrainsConsolidator->ondHasPrimarySectorInExcFlownPart(
        fareMarket, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedFound = false;

    CPPUNIT_ASSERT(found == expectedFound);
  }

  void ondHasPrimarySectorInExcFlownPart_primarySegFoundOnFront()
  {
    cleanStructures();
    bool allShop = true;

    TravelSeg* seg = addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    FareMarket* fareMarket = FM(seg);

    const int firstONDExcTvlSegPos = 0;
    const int lastONDExcTvlSegPos = 0;
    bool found = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    found = _rexConstrainsConsolidator->ondHasPrimarySectorInExcFlownPart(
        fareMarket, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedFound = true;

    CPPUNIT_ASSERT(found == expectedFound);
  }

  void ondHasPrimarySectorInExcFlownPart_primarySegFoundInMiddle()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    TravelSeg* seg = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    FareMarket* fareMarket = FM(seg);

    const int firstONDExcTvlSegPos = 3;
    const int lastONDExcTvlSegPos = 3;
    bool found = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    found = _rexConstrainsConsolidator->ondHasPrimarySectorInExcFlownPart(
        fareMarket, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedFound = true;

    CPPUNIT_ASSERT(found == expectedFound);
  }

  void ondHasPrimarySectorInExcFlownPart_primarySegFoundOnEnd()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    TravelSeg* seg = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    FareMarket* fareMarket = FM(seg);

    const int firstONDExcTvlSegPos = 5;
    const int lastONDExcTvlSegPos = 5;
    bool found = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    found = _rexConstrainsConsolidator->ondHasPrimarySectorInExcFlownPart(
        fareMarket, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedFound = true;

    CPPUNIT_ASSERT(found == expectedFound);
  }

  void ondHasPrimarySectorInExcFlownPart_primarySegFoundOnAll()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    TravelSeg* seg = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    FareMarket* fareMarket = FM(seg);

    const int firstONDExcTvlSegPos = 0;
    const int lastONDExcTvlSegPos = 5;
    bool found = false;

    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    found = _rexConstrainsConsolidator->ondHasPrimarySectorInExcFlownPart(
        fareMarket, firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    bool expectedFound = true;

    CPPUNIT_ASSERT(found == expectedFound);
  }

  void findFMforSegment_FMnotFound()
  {
    setData(true);

    TravelSeg* airSegFormNewItin = (*_newItinSegs)[0];
    const FareMarket* fareMarket = _rexConstrainsConsolidator->findFMforSegment(airSegFormNewItin);

    CPPUNIT_ASSERT(fareMarket == 0);
  }

  void findFMforSegment_FMfoundOnFront()
  {
    setData(true);

    TravelSeg* airSegFormNewItin = (*_excItinSegs)[0];
    const FareMarket* fareMarket = _rexConstrainsConsolidator->findFMforSegment(airSegFormNewItin);

    CPPUNIT_ASSERT(fareMarket == _firstFareMarket);
  }

  void findFMforSegment_FMfoundOnBack()
  {
    setData(true);

    TravelSeg* airSegFormNewItin = (*_excItinSegs)[4];
    const FareMarket* fareMarket = _rexConstrainsConsolidator->findFMforSegment(airSegFormNewItin);

    CPPUNIT_ASSERT(fareMarket == _secondFareMarket);
  }

  void findFMforOND_outOfVect()
  {
    setData(true);

    const int firstONDExcTvlSegPos = -1;
    const int lastONDExcTvlSegPos = 6;

    const FareMarket* fareMarket =
        _rexConstrainsConsolidator->findFMforOND(firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    CPPUNIT_ASSERT(fareMarket == 0);
  }

  void findFMforOND_foundOnFront0()
  {
    setData(true);

    const int firstONDExcTvlSegPos = 0;
    const int lastONDExcTvlSegPos = 0;

    const FareMarket* fareMarket =
        _rexConstrainsConsolidator->findFMforOND(firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    CPPUNIT_ASSERT(fareMarket == _firstFareMarket);
  }

  void findFMforOND_foundOnFront1()
  {
    setData(true);

    const int firstONDExcTvlSegPos = 0;
    const int lastONDExcTvlSegPos = 1;

    const FareMarket* fareMarket =
        _rexConstrainsConsolidator->findFMforOND(firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    CPPUNIT_ASSERT(fareMarket == _firstFareMarket);
  }

  void findFMforOND_foundOnFront2()
  {
    setData(true);

    const int firstONDExcTvlSegPos = 0;
    const int lastONDExcTvlSegPos = 2;

    const FareMarket* fareMarket =
        _rexConstrainsConsolidator->findFMforOND(firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    CPPUNIT_ASSERT(fareMarket == _firstFareMarket);
  }

  void findFMforOND_foundOnBack0()
  {
    setData(true);

    const int firstONDExcTvlSegPos = 3;
    const int lastONDExcTvlSegPos = 3;

    const FareMarket* fareMarket =
        _rexConstrainsConsolidator->findFMforOND(firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    CPPUNIT_ASSERT(fareMarket == _secondFareMarket);
  }

  void findFMforOND_foundOnBack1()
  {
    setData(true);

    const int firstONDExcTvlSegPos = 3;
    const int lastONDExcTvlSegPos = 4;

    const FareMarket* fareMarket =
        _rexConstrainsConsolidator->findFMforOND(firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    CPPUNIT_ASSERT(fareMarket == _secondFareMarket);
  }

  void findFMforOND_foundOnBack2()
  {
    setData(true);

    const int firstONDExcTvlSegPos = 3;
    const int lastONDExcTvlSegPos = 5;

    const FareMarket* fareMarket =
        _rexConstrainsConsolidator->findFMforOND(firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    CPPUNIT_ASSERT(fareMarket == _secondFareMarket);
  }

  void findFMforOND_foundOnAll()
  {
    setData(true);

    const int firstONDExcTvlSegPos = 0;
    const int lastONDExcTvlSegPos = 5;

    const FareMarket* fareMarket =
        _rexConstrainsConsolidator->findFMforOND(firstONDExcTvlSegPos, lastONDExcTvlSegPos);

    CPPUNIT_ASSERT(fareMarket == _firstFareMarket);
  }

  void isONDinExcFMWithFlownGov_test()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexShoppingTrx::PortionMergeTvlVectType firstPortion;
    firstPortion += 1, 2, 3;
    addPortionToOADRsponse(_firstFareMarket, firstPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 4, 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    std::vector<const FareMarket*> fmWithFlownGov;
    _rexConstrainsConsolidator->createFmWithFlownGovSet(fmWithFlownGov);
    bool found = _rexConstrainsConsolidator->isONDinExcFMWithFlownGov(fmWithFlownGov,
                                                                      _trx->oadConsRest()[0]);

    CPPUNIT_ASSERT(found == false);
  }

  void isONDinExcFMWithFlownGov_firstONDhasGov()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexShoppingTrx::PortionMergeTvlVectType firstPortion;
    firstPortion += 1, 2, 3;
    addPortionToOADRsponse(_firstFareMarket, firstPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 4, 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    _firstFareMarket->primarySector() = (*_excItinSegs)[1];

    std::vector<const FareMarket*> fmWithFlownGov;
    _rexConstrainsConsolidator->createFmWithFlownGovSet(fmWithFlownGov);
    bool found = _rexConstrainsConsolidator->isONDinExcFMWithFlownGov(fmWithFlownGov,
                                                                      _trx->oadConsRest()[0]);

    CPPUNIT_ASSERT(found == false);
  }

  void isONDinExcFMWithFlownGov_secondONDhasGov()
  {
    setData();
    _rexConstrainsConsolidator->matchONDInfotoExcOND();

    RexShoppingTrx::PortionMergeTvlVectType firstPortion;
    firstPortion += 1, 2, 3;
    addPortionToOADRsponse(_firstFareMarket, firstPortion);
    RexShoppingTrx::PortionMergeTvlVectType secondPortion;
    secondPortion += 4, 5;
    addPortionToOADRsponse(_secondFareMarket, secondPortion);

    _rexConstrainsConsolidator->consolideConstrainsForONDInfo();

    _firstFareMarket->primarySector() = (*_excItinSegs)[1];
    _secondFareMarket->primarySector() = (*_excItinSegs)[4];

    std::vector<const FareMarket*> fmWithFlownGov;
    _rexConstrainsConsolidator->createFmWithFlownGovSet(fmWithFlownGov);
    bool found = _rexConstrainsConsolidator->isONDinExcFMWithFlownGov(fmWithFlownGov,
                                                                      _trx->oadConsRest()[1]);

    CPPUNIT_ASSERT(found == true);
  }

  void testSetGovCxr_ondCxrEmpty()
  {
    RexShoppingTrx::FareByteCxrData* sodFareByteCxrData = 0;

    _rexConstrainsConsolidator->setGovCxr(sodFareByteCxrData, 0);

    CPPUNIT_ASSERT(sodFareByteCxrData == 0);
  }

  void testSetGovCxr_diffPrefCxr()
  {
    RexShoppingTrx::FareByteCxrData ondFareByteCxrData;
    ondFareByteCxrData.govCxr.excluded = false;
    ondFareByteCxrData.govCxr.cxrList += "AA";
    ondFareByteCxrData.usrCxr.excluded = false;
    ondFareByteCxrData.usrCxr.cxrList += "BB";
    RexShoppingTrx::FareByteCxrData* sodFareByteCxrData;

    _rexConstrainsConsolidator->setGovCxr(sodFareByteCxrData, &ondFareByteCxrData);

    CPPUNIT_ASSERT(_trx->isShopOnlyCurrentItin());
    assertCxrList(ondFareByteCxrData.govCxr.cxrList, sodFareByteCxrData->govCxr.cxrList);
    CPPUNIT_ASSERT(ondFareByteCxrData.govCxr.excluded == sodFareByteCxrData->govCxr.excluded);
  }

  void testSetGovCxr_interPrefCxr()
  {
    RexShoppingTrx::FareByteCxrData ondFareByteCxrData;
    ondFareByteCxrData.govCxr.excluded = false;
    ondFareByteCxrData.govCxr.cxrList += "AA", "BB";
    ondFareByteCxrData.usrCxr.excluded = false;
    ondFareByteCxrData.usrCxr.cxrList += "BB";
    RexShoppingTrx::FareByteCxrData* sodFareByteCxrData;

    _rexConstrainsConsolidator->setGovCxr(sodFareByteCxrData, &ondFareByteCxrData);

    CPPUNIT_ASSERT(!_trx->isShopOnlyCurrentItin());
    assertCxrList(ondFareByteCxrData.govCxr.cxrList, sodFareByteCxrData->govCxr.cxrList);
    CPPUNIT_ASSERT(ondFareByteCxrData.govCxr.excluded == sodFareByteCxrData->govCxr.excluded);
  }

  void testSetGovCxr_diffExcCxr()
  {
    RexShoppingTrx::FareByteCxrData ondFareByteCxrData;
    ondFareByteCxrData.govCxr.excluded = false;
    ondFareByteCxrData.govCxr.cxrList += "AA";
    ondFareByteCxrData.usrCxr.excluded = true;
    ondFareByteCxrData.usrCxr.cxrList += "AA", "BB";
    RexShoppingTrx::FareByteCxrData* sodFareByteCxrData;

    _rexConstrainsConsolidator->setGovCxr(sodFareByteCxrData, &ondFareByteCxrData);

    CPPUNIT_ASSERT(_trx->isShopOnlyCurrentItin());
    assertCxrList(ondFareByteCxrData.govCxr.cxrList, sodFareByteCxrData->govCxr.cxrList);
    CPPUNIT_ASSERT(ondFareByteCxrData.govCxr.excluded == sodFareByteCxrData->govCxr.excluded);
  }

  void testSetGovCxr_diffExcCxrDiffCxr()
  {
    RexShoppingTrx::FareByteCxrData ondFareByteCxrData;
    ondFareByteCxrData.govCxr.excluded = false;
    ondFareByteCxrData.govCxr.cxrList += "AA";
    ondFareByteCxrData.usrCxr.excluded = true;
    ondFareByteCxrData.usrCxr.cxrList += "CC", "BB";
    RexShoppingTrx::FareByteCxrData* sodFareByteCxrData;

    _rexConstrainsConsolidator->setGovCxr(sodFareByteCxrData, &ondFareByteCxrData);

    CPPUNIT_ASSERT(!_trx->isShopOnlyCurrentItin());
    assertCxrList(ondFareByteCxrData.govCxr.cxrList, sodFareByteCxrData->govCxr.cxrList);
    CPPUNIT_ASSERT(ondFareByteCxrData.govCxr.excluded == sodFareByteCxrData->govCxr.excluded);
  }

  void testSetUsrCxr_ondCxrEmpty()
  {
    RexShoppingTrx::FareByteCxrData* sodFareByteCxrData = 0;

    _rexConstrainsConsolidator->setUsrCxr(sodFareByteCxrData, 0);

    CPPUNIT_ASSERT(sodFareByteCxrData == 0);
  }

  void testSetUsrCxr_copyUsrCxr()
  {
    RexShoppingTrx::FareByteCxrData ondFareByteCxrData;
    ondFareByteCxrData.govCxr.excluded = false;
    ondFareByteCxrData.govCxr.cxrList += "AA";
    ondFareByteCxrData.usrCxr.excluded = true;
    ondFareByteCxrData.usrCxr.cxrList += "CC", "BB";
    RexShoppingTrx::FareByteCxrData* sodFareByteCxrData = 0;

    _rexConstrainsConsolidator->setUsrCxr(sodFareByteCxrData, &ondFareByteCxrData);

    assertCxrList(ondFareByteCxrData.usrCxr.cxrList, sodFareByteCxrData->usrCxr.cxrList);
    CPPUNIT_ASSERT(ondFareByteCxrData.usrCxr.excluded == sodFareByteCxrData->usrCxr.excluded);
    CPPUNIT_ASSERT(sodFareByteCxrData->govCxr.cxrList.empty());
  }

  void routingHasChanged_noChange()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    bool routingChanged = false;
    routingChanged = _rexConstrainsConsolidator->routingHasChanged();

    CPPUNIT_ASSERT(routingChanged == false);
  }

  void routingHasChanged_change()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DEN", true, "2009-09-06 09:00", allShop);

    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    bool routingChanged = false;
    routingChanged = _rexConstrainsConsolidator->routingHasChanged();

    CPPUNIT_ASSERT(routingChanged == true);
  }

  void routingHasChanged_noChangeMultiAirport()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "EWR", true, "2009-09-06 09:00", allShop);

    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "NYC", true, "2009-09-06 09:00", allShop);

    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    bool routingChanged = false;
    routingChanged = _rexConstrainsConsolidator->routingHasChanged();

    CPPUNIT_ASSERT(routingChanged == true);
  }

  void routingHasChanged_change_LessNewFlights()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);

    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "DFW", true, "2009-09-05 09:00", allShop);

    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    bool routingChanged = false;
    routingChanged = _rexConstrainsConsolidator->routingHasChanged();

    CPPUNIT_ASSERT(routingChanged == true);
  }

  void routingHasChanged_change_LessExcFlights()
  {
    cleanStructures();
    bool allShop = true;

    addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_excItinSegs, "KRK", "DFW", true, "2009-09-05 09:00", allShop);

    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DEN", true, "2009-09-06 09:00", allShop);

    _rexConstrainsConsolidator->copySegWithoutARUNKs();

    bool routingChanged = false;
    routingChanged = _rexConstrainsConsolidator->routingHasChanged();

    CPPUNIT_ASSERT(routingChanged == true);
  }

  void matchFMsToONDInfoWithoutNoMatched_routingNOTChanged()
  {
    setData();
    RexConstrainsConsolidator::ExcSegPosToFMMap excSegPosToFM;
    _rexConstrainsConsolidator->createExcSegPosToFMMap(excSegPosToFM);
    _rexConstrainsConsolidator->matchONDInfoToExcSegPos();

    _rexConstrainsConsolidator->matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);

    CPPUNIT_ASSERT(!_rexConstrainsConsolidator->_ondInfoToFMWithoutNoMatched.empty());
    std::vector<const FareMarket*> fmList1;
    fmList1 += _firstFareMarket;
    std::vector<const FareMarket*> fmList2;
    fmList2 += _secondFareMarket;
    assertFMList(_rexConstrainsConsolidator->_ondInfoToFMWithoutNoMatched[_firstONDInfo], fmList1);
    assertFMList(_rexConstrainsConsolidator->_ondInfoToFMWithoutNoMatched[_secondONDInfo], fmList1);
    assertFMList(_rexConstrainsConsolidator->_ondInfoToFMWithoutNoMatched[_thirdONDInfo], fmList2);
  }

  void matchFMsToONDInfoWithoutNoMatched_routingChanged()
  {
    setDataWithRoutingChange();
    RexConstrainsConsolidator::ExcSegPosToFMMap excSegPosToFM;
    _rexConstrainsConsolidator->createExcSegPosToFMMap(excSegPosToFM);
    _rexConstrainsConsolidator->matchONDInfoToExcSegPos();

    _rexConstrainsConsolidator->matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);

    CPPUNIT_ASSERT(_rexConstrainsConsolidator->_ondInfoToFMWithoutNoMatched.count(_firstONDInfo) !=
                   0);
    CPPUNIT_ASSERT(_rexConstrainsConsolidator->_ondInfoToFMWithoutNoMatched.count(_secondONDInfo) !=
                   0);
    CPPUNIT_ASSERT(_rexConstrainsConsolidator->_ondInfoToFMWithoutNoMatched.count(_thirdONDInfo) ==
                   0);

    std::vector<const FareMarket*> fmList;
    fmList += _firstFareMarket;
    assertFMList(_rexConstrainsConsolidator->_ondInfoToFMWithoutNoMatched[_firstONDInfo], fmList);
  }

  void createOndInfoNotMatched_routingNOTChanged()
  {
    setData();
    RexConstrainsConsolidator::ExcSegPosToFMMap excSegPosToFM;
    _rexConstrainsConsolidator->createExcSegPosToFMMap(excSegPosToFM);
    _rexConstrainsConsolidator->matchONDInfoToExcSegPos();
    _rexConstrainsConsolidator->matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);

    _rexConstrainsConsolidator->createOndInfoNotMatched();

    CPPUNIT_ASSERT(_rexConstrainsConsolidator->_ondInfoNotMatched.empty());
  }

  void createOndInfoNotMatched_routingChanged()
  {
    setDataWithRoutingChange();
    RexConstrainsConsolidator::ExcSegPosToFMMap excSegPosToFM;
    _rexConstrainsConsolidator->createExcSegPosToFMMap(excSegPosToFM);
    _rexConstrainsConsolidator->matchONDInfoToExcSegPos();
    _rexConstrainsConsolidator->matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);

    _rexConstrainsConsolidator->createOndInfoNotMatched();

    CPPUNIT_ASSERT(_rexConstrainsConsolidator->_ondInfoNotMatched.count(_firstONDInfo) == 0);
    CPPUNIT_ASSERT(_rexConstrainsConsolidator->_ondInfoNotMatched.count(_secondONDInfo) == 0);
    CPPUNIT_ASSERT(_rexConstrainsConsolidator->_ondInfoNotMatched.count(_thirdONDInfo) != 0);
  }

  void createFmNotMatched_routingNOTChanged()
  {
    setData();
    RexConstrainsConsolidator::ExcSegPosToFMMap excSegPosToFM;
    _rexConstrainsConsolidator->createExcSegPosToFMMap(excSegPosToFM);
    _rexConstrainsConsolidator->matchONDInfoToExcSegPos();
    _rexConstrainsConsolidator->matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);
    _rexConstrainsConsolidator->createOndInfoNotMatched();

    _rexConstrainsConsolidator->createFmNotMatched();

    CPPUNIT_ASSERT(0 != std::count(_rexConstrainsConsolidator->_fmNotMatched.begin(),
                                   _rexConstrainsConsolidator->_fmNotMatched.end(),
                                   _firstFareMarket));
    CPPUNIT_ASSERT(0 == std::count(_rexConstrainsConsolidator->_fmNotMatched.begin(),
                                   _rexConstrainsConsolidator->_fmNotMatched.end(),
                                   _secondFareMarket));
  }

  void createFmNotMatched_routingChanged()
  {
    setDataWithRoutingChange();
    RexConstrainsConsolidator::ExcSegPosToFMMap excSegPosToFM;
    _rexConstrainsConsolidator->createExcSegPosToFMMap(excSegPosToFM);
    _rexConstrainsConsolidator->matchONDInfoToExcSegPos();
    _rexConstrainsConsolidator->matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);
    _rexConstrainsConsolidator->createOndInfoNotMatched();

    _rexConstrainsConsolidator->createFmNotMatched();

    CPPUNIT_ASSERT(0 != std::count(_rexConstrainsConsolidator->_fmNotMatched.begin(),
                                   _rexConstrainsConsolidator->_fmNotMatched.end(),
                                   _firstFareMarket));
    CPPUNIT_ASSERT(0 != std::count(_rexConstrainsConsolidator->_fmNotMatched.begin(),
                                   _rexConstrainsConsolidator->_fmNotMatched.end(),
                                   _secondFareMarket));
  }

  void doesAnyFlownFCHasSameCxrAsUnmappedFC_routingChanged_sameGovCxr()
  {
    setDataWithRoutingChange();
    RexConstrainsConsolidator::ExcSegPosToFMMap excSegPosToFM;
    _rexConstrainsConsolidator->createExcSegPosToFMMap(excSegPosToFM);
    _rexConstrainsConsolidator->matchONDInfoToExcSegPos();
    _rexConstrainsConsolidator->matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);
    _rexConstrainsConsolidator->createOndInfoNotMatched();
    _rexConstrainsConsolidator->createFmNotMatched();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G1");

    bool addCxrRestToUnmappedONDInfos =
        _rexConstrainsConsolidator->doesAnyFlownFCHasSameCxrAsUnmappedFC();

    CPPUNIT_ASSERT(addCxrRestToUnmappedONDInfos == true);
  }

  void doesAnyFlownFCHasSameCxrAsUnmappedFC_routingChanged_diffGovCxr()
  {
    setDataWithRoutingChange();
    RexConstrainsConsolidator::ExcSegPosToFMMap excSegPosToFM;
    _rexConstrainsConsolidator->createExcSegPosToFMMap(excSegPosToFM);
    _rexConstrainsConsolidator->matchONDInfoToExcSegPos();
    _rexConstrainsConsolidator->matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);
    _rexConstrainsConsolidator->createOndInfoNotMatched();
    _rexConstrainsConsolidator->createFmNotMatched();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->printONDInfoNotMatched();
    _rexConstrainsConsolidator->printFMNotMatched();

    bool addCxrRestToUnmappedONDInfos =
        _rexConstrainsConsolidator->doesAnyFlownFCHasSameCxrAsUnmappedFC();

    CPPUNIT_ASSERT(addCxrRestToUnmappedONDInfos == true);
  }

  void doesAnyFlownFCHasSameCxrAsUnmappedFC_unflown()
  {
    setDataWithRoutingChange_unflown();
    RexConstrainsConsolidator::ExcSegPosToFMMap excSegPosToFM;
    _rexConstrainsConsolidator->createExcSegPosToFMMap(excSegPosToFM);
    _rexConstrainsConsolidator->matchONDInfoToExcSegPos();
    _rexConstrainsConsolidator->matchFMsToONDInfoWithoutNoMatched(excSegPosToFM);
    _rexConstrainsConsolidator->createOndInfoNotMatched();
    _rexConstrainsConsolidator->createFmNotMatched();

    setGovCarrier(_firstFareMarket, "G1");
    setGovCarrier(_secondFareMarket, "G2");

    _rexConstrainsConsolidator->printONDInfoNotMatched();
    _rexConstrainsConsolidator->printFMNotMatched();

    bool addCxrRestToUnmappedONDInfos =
        _rexConstrainsConsolidator->doesAnyFlownFCHasSameCxrAsUnmappedFC();

    CPPUNIT_ASSERT(addCxrRestToUnmappedONDInfos == false);
  }

  void testCheckRestrictionNeeded()
  {
    cleanStructures();

    typedef RexShoppingTrx::OADConsolidatedConstrains Constrains;
    typedef RexConstrainsConsolidator::ONDInf ONDInfo;

    std::vector<Constrains> oadConsRest(2);
    std::vector<ONDInfo> ondInfo(2);

    oadConsRest[0].origAirport = ondInfo[0].origAirport = "SVO";
    oadConsRest[0].destAirport = ondInfo[0].destAirport = "DEL";

    oadConsRest[1].origAirport = ondInfo[1].origAirport = "DEL";
    oadConsRest[1].destAirport = ondInfo[1].destAirport = "BLR";

    FareMarket fareMarket;
    fareMarket.primarySector() =
        addSegment(&fareMarket.travelSeg(), "SVO", "DEL", true, "2014-09-01 09:00");
    addSegment(&fareMarket.travelSeg(), "DEL", "BLR", true, "2014-09-05 09:00");

    fareMarket.boardMultiCity() = "SVO";
    fareMarket.offMultiCity() = "BLR";

    _rexConstrainsConsolidator->_ondInfoToFM[&ondInfo[0]].push_back(&fareMarket);
    _rexConstrainsConsolidator->_ondInfoToFM[&ondInfo[1]].push_back(&fareMarket);

    _rexConstrainsConsolidator->checkRestrictionNeeded(oadConsRest[0], &ondInfo[0]);

    CPPUNIT_ASSERT(oadConsRest[0].cxrRestrictionNeeded == true);

    _rexConstrainsConsolidator->checkRestrictionNeeded(oadConsRest[1], &ondInfo[1]);

    CPPUNIT_ASSERT(oadConsRest[1].cxrRestrictionNeeded == false);
  }
  // tests end

protected:
  RexShoppingTrx* _trx;
  Itin* _newItin;
  std::vector<TravelSeg*>* _newItinSegs;
  ExcItin* _excItin;
  std::vector<TravelSeg*>* _excItinSegs;
  RexConstrainsConsolidator* _rexConstrainsConsolidator;
  FareMarket* _firstFareMarket;
  FareMarket* _secondFareMarket;
  RexConstrainsConsolidator::ONDInf* _firstONDInfo;
  RexConstrainsConsolidator::ONDInf* _secondONDInfo;
  RexConstrainsConsolidator::ONDInf* _thirdONDInfo;
  TestMemHandle _memHandle;

  void cleanStructures()
  {
    _newItin->travelSeg().clear();
    _excItin->travelSeg().clear();
    _rexConstrainsConsolidator->_ondInfo.clear();
    _rexConstrainsConsolidator->_ondInfoToFM.clear();
    _rexConstrainsConsolidator->_ondInfoToExcSegPosSet.clear();
    _rexConstrainsConsolidator->_fmToExcSegPosSet.clear();
    _trx->originDest().clear();
    _excItin->fareComponent().clear();
    _rexConstrainsConsolidator->_excItinSeg.clear();
    _rexConstrainsConsolidator->_newItinSeg.clear();
    _rexConstrainsConsolidator->_ondInfoToExcSegPosWithoutNoMatchedSet.clear();
    _rexConstrainsConsolidator->_ondInfoNotMatched.clear();
    _rexConstrainsConsolidator->_fmNotMatched.clear();
    _rexConstrainsConsolidator->_ondInfoToFMWithoutNoMatched.clear();
    _rexConstrainsConsolidator->_addCxrRestToUnmappedONDInfos = true;
  }

  AirSeg* addSegment(std::vector<TravelSeg*>* tvlSeg,
                     const LocCode& origAirport,
                     const LocCode& destAirport,
                     bool unflown,
                     const std::string& depDateTime,
                     bool shopped = false,
                     bool stopOver = false)
  {
    const TravelSeg::ChangeStatus changeStatus = TravelSeg::CHANGED;
    const TravelSegType segType = Air;

    AirSeg* seg = _memHandle.create<AirSeg>();

    seg->origAirport() = origAirport;
    seg->destAirport() = destAirport;
    seg->segmentType() = Air;
    seg->stopOver() = stopOver;
    seg->carrier() = "AA";
    if (segType == Air)
      seg->flightNumber() = 123;
    seg->unflown() = unflown;
    if (!depDateTime.empty())
      seg->departureDT() = DateTime(const_cast<std::string&>(depDateTime));
    seg->boardMultiCity() = origAirport;
    seg->offMultiCity() = destAirport;
    seg->changeStatus() = changeStatus;
    seg->isShopped() = shopped;
    seg->pnrSegment() = tvlSeg->size() + 1;
    tvlSeg->push_back(seg);

    return seg;
  }

  AirSeg* addSegment(std::vector<TravelSeg*>* tvlSeg,
                     const LocCode& origAirport,
                     const LocCode& destAirport,
                     const LocCode& boardMultiCity,
                     const LocCode& offMultiCity,
                     bool unflown,
                     const std::string& depDateTime,
                     bool shopped = false,
                     bool stopOver = false)
  {
    const TravelSeg::ChangeStatus changeStatus = TravelSeg::CHANGED;
    const TravelSegType segType = Air;

    AirSeg* seg = _memHandle.create<AirSeg>();

    seg->origAirport() = origAirport;
    seg->destAirport() = destAirport;
    seg->segmentType() = Air;
    seg->stopOver() = stopOver;
    seg->carrier() = "AA";
    if (segType == Air)
      seg->flightNumber() = 123;
    seg->unflown() = unflown;
    if (!depDateTime.empty())
      seg->departureDT() = DateTime(const_cast<std::string&>(depDateTime));
    seg->boardMultiCity() = boardMultiCity;
    seg->offMultiCity() = offMultiCity;
    seg->changeStatus() = changeStatus;
    seg->isShopped() = shopped;
    seg->pnrSegment() = tvlSeg->size() + 1;
    tvlSeg->push_back(seg);

    return seg;
  }

  void addOdThruFM(const LocCode& origAirport,
                   const LocCode& destAirport,
                   const std::string& depDateTime)
  {
    PricingTrx::OriginDestination thruFMForMip;
    thruFMForMip.boardMultiCity = origAirport;
    thruFMForMip.offMultiCity = destAirport;
    thruFMForMip.travelDate = DateTime(const_cast<std::string&>(depDateTime));
    _trx->originDest().push_back(thruFMForMip);
  }

  FareMarket* FM(const LocCode& origAirport, const LocCode& destAirport)
  {
    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->boardMultiCity() = origAirport;
    Loc* locFrom = _memHandle.create<Loc>();
    locFrom->loc() = origAirport;
    const_cast<Loc*&>(fareMarket->origin()) = locFrom;
    fareMarket->offMultiCity() = destAirport;
    Loc* locTo = _memHandle.create<Loc>();
    locTo->loc() = destAirport;
    const_cast<Loc*&>(fareMarket->destination()) = locTo;
    return fareMarket;
  }

  FareMarket* FM(TravelSeg* primarySegment)
  {
    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->primarySector() = primarySegment;
    return fareMarket;
  }

  FareCompInfo* FCInfo(FareMarket* fareMarket)
  {
    FareCompInfo* fareCompInfo = _memHandle.create<FareCompInfo>();
    fareCompInfo->fareMarket() = fareMarket;
    return fareCompInfo;
  }

  RexConstrainsConsolidator::ONDInf*
  addONDInfo(const LocCode& boardMCity, const LocCode& offMCity, const std::string& depDateTime)
  {
    RexConstrainsConsolidator::ONDInf* ondInfo =
        _memHandle.insert(new RexConstrainsConsolidator::ONDInf(boardMCity, offMCity, depDateTime));
    _rexConstrainsConsolidator->_ondInfo.push_back(ondInfo);
    return ondInfo;
  }

  void
  addPortionToOADRsponse(FareMarket* fareMarket, RexShoppingTrx::PortionMergeTvlVectType& portion)
  {
    for (auto& oadResponse : _trx->oadResponse()[fareMarket])
      oadResponse.portion.insert(oadResponse.portion.end(), portion.begin(), portion.end());
  }

  void addOutboundPortionToOADRsponse(FareMarket* fareMarket,
                                      RexShoppingTrx::PortionMergeTvlVectType& outboundPortion)
  {
    for (auto& oadResponse : _trx->oadResponse()[fareMarket])
    {
      oadResponse.outboundPortion.insert(oadResponse.outboundPortion.end(), outboundPortion.begin(),
                                         outboundPortion.end());
    }
  }

  RexShoppingTrx::OADConsolidatedConstrains* addCons(const LocCode& origAirport,
                                                     const LocCode& destAirport,
                                                     const std::string& depDateTime,
                                                     bool unflown)
  {
    RexShoppingTrx::OADConsolidatedConstrains* newCons =
        _memHandle.create<RexShoppingTrx::OADConsolidatedConstrains>();
    newCons->origAirport = origAirport;
    newCons->destAirport = destAirport;
    newCons->travelDate = DateTime(const_cast<std::string&>(depDateTime));
    newCons->unflown = unflown;
    newCons->sodConstraints.insert(std::make_pair(0, RexShoppingTrx::SODConstraint{}));

    return newCons;
  }

  void addFltRestWithBkg(RexShoppingTrx::OADConsolidatedConstrains* newCons,
                         std::set<int>& restFlightsWithBkg)
  {
    for (std::size_t i = 0; i < newCons->sod.size(); ++i)
      newCons->sodConstraints[i].restFlightsWithBkg.insert(restFlightsWithBkg.begin(),
                                                           restFlightsWithBkg.end());
  }

  void addFltRest(RexShoppingTrx::OADConsolidatedConstrains* newCons, std::set<int>& restFlights)
  {
    for (std::size_t i = 0; i < newCons->sod.size(); ++i)
      newCons->sodConstraints[i].restFlights.insert(restFlights.begin(), restFlights.end());
  }

  void addUnshoppedFltRest(RexShoppingTrx::OADConsolidatedConstrains* newCons,
                           std::set<int>& unshoppedFlights)
  {
    newCons->unshoppedFlights.insert(unshoppedFlights.begin(), unshoppedFlights.end());
  }

  void setUnshoppedFlight(RexConstrainsConsolidator::ONDInf* ondInfo,
                          const std::vector<int>& unshoppedFlights)
  {
    ondInfo->unshoppedFlights.insert(
        ondInfo->unshoppedFlights.end(), unshoppedFlights.begin(), unshoppedFlights.end());
  }

  void setFlightNumnerRest(FareMarket* fareMarket, bool rest)
  {
    for (auto& oadResponse : _trx->oadResponse()[fareMarket])
      oadResponse.flightNumberRestriction = rest;
  }

  void setFrcCnnxInOADRsponse(FareMarket* fareMarket,
                              RexShoppingTrx::ForcedConnectionSet& forcedConnections)
  {
    for (auto& oadResponse : _trx->oadResponse()[fareMarket])
      oadResponse.forcedConnections.insert(forcedConnections.begin(), forcedConnections.end());
  }

  void addFrcCnx(RexShoppingTrx::OADConsolidatedConstrains* newCons,
                 RexShoppingTrx::ForcedConnectionSet& forcedConnection)
  {
    newCons->forcedConnection.insert(forcedConnection.begin(), forcedConnection.end());
  }

  void setFMToFirstFMinPU(const FareMarket* firstFareMarket, const FareMarket* fareMarket)
  {
    _trx->fMToFirstFMinPU()[firstFareMarket] += fareMarket;
  }

  void setFirstBreakRestInOADRsponse(FareMarket* fareMarket, bool firstBreakRest)
  {
    for (auto& oadResponse : _trx->oadResponse()[fareMarket])
      oadResponse.firstBreakRest = firstBreakRest;
  }

  void setCarrierListInOADRsponse(const FareMarket* fareMarket,
                                  std::set<CarrierCode>& cxrList,
                                  bool excluded)
  {
    for (auto& oadResponse : _trx->oadResponse()[fareMarket])
    {
      oadResponse.fareByteCxrAppl.cxrList.insert(cxrList.begin(), cxrList.end());
      oadResponse.fareByteCxrAppl.excluded = excluded;
    }
  }

  void setGovCarrier(FareMarket* fareMarket, const CarrierCode cxr)
  {
    fareMarket->governingCarrier() = cxr;
  }

  void addCxrCons(RexShoppingTrx::OADConsolidatedConstrains* firstCons,
                  std::set<CarrierCode>& govCxrList,
                  std::set<CarrierCode>& usrCxrList,
                  bool govExcluded,
                  bool usrExcluded)
  {
    RexShoppingTrx::FareByteCxrData* fareByteCxrData =
        _memHandle.create<RexShoppingTrx::FareByteCxrData>();
    fareByteCxrData->govCxr.cxrList.insert(govCxrList.begin(), govCxrList.end());
    fareByteCxrData->usrCxr.cxrList.insert(usrCxrList.begin(), usrCxrList.end());
    fareByteCxrData->govCxr.excluded = govExcluded;
    fareByteCxrData->usrCxr.excluded = usrExcluded;

    for (std::size_t i = 0; i < firstCons->sod.size(); ++i)
      firstCons->sodConstraints[i].fareByteCxrAppl = fareByteCxrData;
  }

  void addUserCxrList(std::set<CarrierCode>& cxrList, bool excluded)
  {
    RexShoppingTrx::FareByteCxrApplData& fareByteCxrApplData = _trx->cxrListFromPSS();
    fareByteCxrApplData.cxrList.insert(cxrList.begin(), cxrList.end());
    fareByteCxrApplData.excluded = excluded;
  }

  RexShoppingTrx::SubOriginDestination* createSOD(const LocCode& origAirport,
                                                  const LocCode& destAirport,
                                                  const std::string& depDateTime,
                                                  bool change,
                                                  bool exact_cxr,
                                                  bool preferred_cxr)
  {
    RexShoppingTrx::SubOriginDestination* newSOD =
        _memHandle.create<RexShoppingTrx::SubOriginDestination>();
    newSOD->origAirport = origAirport;
    newSOD->destAirport = destAirport;
    newSOD->travelDate = DateTime(const_cast<std::string&>(depDateTime));
    newSOD->change = change;
    newSOD->exact_cxr = exact_cxr;
    newSOD->preferred_cxr = preferred_cxr;

    return newSOD;
  }

  void addFlt(RexShoppingTrx::SubOriginDestination* sod, std::set<int>& flights)
  {
    sod->flights.insert(flights.begin(), flights.end());
  }

  void addBKCCanChange(RexShoppingTrx::SubOriginDestination* sod, std::set<int>& bkcCanChange)
  {
    sod->bkcCanChange.insert(bkcCanChange.begin(), bkcCanChange.end());
  }

  void addFrcCnx(RexShoppingTrx::SubOriginDestination* sod,
                 RexShoppingTrx::ForcedConnectionSet& forcedConnections)
  {
    sod->forcedConnections.insert(forcedConnections.begin(), forcedConnections.end());
  }

  // helpers
  void setData(bool allShop = false)
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);
    addOdThruFM("DFW", "MIA", "2009-09-01 09:00");
    addOdThruFM("MIA", "DFW", "2009-09-04 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _thirdONDInfo = _rexConstrainsConsolidator->_ondInfo[2];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));
  }

  void assertONDInfo(std::vector<RexConstrainsConsolidator::ONDInf>& expectedONDInfo,
                     const std::vector<RexConstrainsConsolidator::ONDInf*>& actualONDInfo)
  {
    CPPUNIT_ASSERT(expectedONDInfo.size() == actualONDInfo.size());
    for (size_t i = 0; i < actualONDInfo.size(); i++)
      CPPUNIT_ASSERT(expectedONDInfo[i] == *(actualONDInfo[i]));
  }

  void assertUnshopped(const std::vector<int>& expectedUnshopped,
                       const std::vector<int>& actualUnshopped)
  {
    CPPUNIT_ASSERT(expectedUnshopped.size() == actualUnshopped.size());
    for (size_t i = 0; i < actualUnshopped.size(); i++)
      CPPUNIT_ASSERT(expectedUnshopped[i] == actualUnshopped[i]);
  }

  void assertONDInfoToFM(RexConstrainsConsolidator::ONDInfoToFM& expectedONDInfoToFM,
                         RexConstrainsConsolidator::ONDInfoToFM& actualONDInfoToFM)
  {
    CPPUNIT_ASSERT(expectedONDInfoToFM.size() == actualONDInfoToFM.size());
    RexConstrainsConsolidator::ONDInfoToFM::const_iterator ondInfoToFMIter =
        expectedONDInfoToFM.begin();
    for (; ondInfoToFMIter != expectedONDInfoToFM.end(); ++ondInfoToFMIter)
    {
      CPPUNIT_ASSERT(actualONDInfoToFM.count(ondInfoToFMIter->first) != 0);
      CPPUNIT_ASSERT(ondInfoToFMIter->second.size() ==
                     actualONDInfoToFM[ondInfoToFMIter->first].size());

      std::vector<const FareMarket*>::const_iterator fmIter = ondInfoToFMIter->second.begin();
      for (; fmIter != ondInfoToFMIter->second.end(); ++fmIter)
      {
        CPPUNIT_ASSERT(0 != std::count(actualONDInfoToFM[ondInfoToFMIter->first].begin(),
                                       actualONDInfoToFM[ondInfoToFMIter->first].end(),
                                       *fmIter));
      }
    }
  }

  void assertONDConsRest(RexShoppingTrx::OADConsolidatedConstrainsVect& expectedOADConsRest,
                         RexShoppingTrx::OADConsolidatedConstrainsVect& actualOADConsRest)
  {
    CPPUNIT_ASSERT(expectedOADConsRest.size() == actualOADConsRest.size());
    RexShoppingTrx::OADConsolidatedConstrainsVect::const_iterator actualOADConsRestIter =
        actualOADConsRest.begin();
    for (; actualOADConsRestIter != actualOADConsRest.end(); ++actualOADConsRestIter)
    {
      bool found = false;
      RexShoppingTrx::OADConsolidatedConstrainsVect::const_iterator expectedOADConsRestIter =
          expectedOADConsRest.begin();
      for (; expectedOADConsRestIter != expectedOADConsRest.end(); ++expectedOADConsRestIter)
      {
        if ((*expectedOADConsRestIter)->origAirport == (*actualOADConsRestIter)->origAirport &&
            (*expectedOADConsRestIter)->destAirport == (*actualOADConsRestIter)->destAirport &&
            (*expectedOADConsRestIter)->travelDate.get64BitRepDateOnly() ==
                (*actualOADConsRestIter)->travelDate.get64BitRepDateOnly())
        {
          CPPUNIT_ASSERT((*expectedOADConsRestIter)->unshoppedFlights.size() ==
                         (*actualOADConsRestIter)->unshoppedFlights.size());
          std::set<int>::const_iterator actUnshoppedFlightsIter =
              (*actualOADConsRestIter)->unshoppedFlights.begin();
          for (; actUnshoppedFlightsIter != (*actualOADConsRestIter)->unshoppedFlights.end();
               ++actUnshoppedFlightsIter)
            CPPUNIT_ASSERT(
                (*expectedOADConsRestIter)->unshoppedFlights.count(*actUnshoppedFlightsIter) != 0);

          for (std::size_t i = 0; i < (*expectedOADConsRestIter)->sodConstraints.size(); ++i)
          {
            CPPUNIT_ASSERT(
                (*expectedOADConsRestIter)->sodConstraints[i].restFlightsWithBkg.size() ==
                (*actualOADConsRestIter)->sodConstraints[i].restFlightsWithBkg.size());

            auto actRestFlightsIter =
                (*actualOADConsRestIter)->sodConstraints[i].restFlightsWithBkg.begin();
            for (; actRestFlightsIter !=
                   (*actualOADConsRestIter)->sodConstraints[i].restFlightsWithBkg.end();
                 ++actRestFlightsIter)
            {
              CPPUNIT_ASSERT((*expectedOADConsRestIter)
                                 ->sodConstraints[i]
                                 .restFlightsWithBkg.count(*actRestFlightsIter) != 0);
            }

            CPPUNIT_ASSERT((*expectedOADConsRestIter)->sodConstraints[i].restFlights.size() ==
                           (*actualOADConsRestIter)->sodConstraints[i].restFlights.size());
            actRestFlightsIter = (*actualOADConsRestIter)->sodConstraints[i].restFlights.begin();
            for (; actRestFlightsIter !=
                   (*actualOADConsRestIter)->sodConstraints[i].restFlights.end();
                 ++actRestFlightsIter)
            {
              CPPUNIT_ASSERT((*expectedOADConsRestIter)
                                 ->sodConstraints[i]
                                 .restFlights.count(*actRestFlightsIter) != 0);
            }
          }

          CPPUNIT_ASSERT((*expectedOADConsRestIter)->forcedConnection.size() ==
                         (*actualOADConsRestIter)->forcedConnection.size());
          RexShoppingTrx::ForcedConnectionSet::const_iterator actRestFrcCnxIter =
              (*actualOADConsRestIter)->forcedConnection.begin();
          for (; actRestFrcCnxIter != (*actualOADConsRestIter)->forcedConnection.end();
               ++actRestFrcCnxIter)
            CPPUNIT_ASSERT((*expectedOADConsRestIter)->forcedConnection.count(*actRestFrcCnxIter) !=
                           0);

          for (std::size_t i = 0; i < (*expectedOADConsRestIter)->sodConstraints.size(); ++i)
          {
            if ((*expectedOADConsRestIter)->sodConstraints[i].fareByteCxrAppl &&
                (*actualOADConsRestIter)->sodConstraints[i].fareByteCxrAppl)
            {
              CPPUNIT_ASSERT((*expectedOADConsRestIter)
                                 ->sodConstraints[i]
                                 .fareByteCxrAppl->govCxr.cxrList.size() ==
                             (*actualOADConsRestIter)
                                 ->sodConstraints[i]
                                 .fareByteCxrAppl->govCxr.cxrList.size());
              for (auto it = (*expectedOADConsRestIter)
                                 ->sodConstraints[i]
                                 .fareByteCxrAppl->govCxr.cxrList.begin();
                   it !=
                   (*expectedOADConsRestIter)
                       ->sodConstraints[i]
                       .fareByteCxrAppl->govCxr.cxrList.end();
                   it++)
                CPPUNIT_ASSERT((*actualOADConsRestIter)
                                   ->sodConstraints[i]
                                   .fareByteCxrAppl->govCxr.cxrList.count(*it));
              CPPUNIT_ASSERT(
                  (*expectedOADConsRestIter)->sodConstraints[i].fareByteCxrAppl->govCxr.excluded ==
                  (*actualOADConsRestIter)->sodConstraints[i].fareByteCxrAppl->govCxr.excluded);

              CPPUNIT_ASSERT((*expectedOADConsRestIter)
                                 ->sodConstraints[i]
                                 .fareByteCxrAppl->usrCxr.cxrList.size() ==
                             (*actualOADConsRestIter)
                                 ->sodConstraints[i]
                                 .fareByteCxrAppl->usrCxr.cxrList.size());
              for (auto it = (*expectedOADConsRestIter)
                                 ->sodConstraints[i]
                                 .fareByteCxrAppl->usrCxr.cxrList.begin();
                   it !=
                   (*expectedOADConsRestIter)
                       ->sodConstraints[i]
                       .fareByteCxrAppl->usrCxr.cxrList.end();
                   it++)
                CPPUNIT_ASSERT((*actualOADConsRestIter)
                                   ->sodConstraints[i]
                                   .fareByteCxrAppl->usrCxr.cxrList.count(*it));
              CPPUNIT_ASSERT(
                  (*expectedOADConsRestIter)->sodConstraints[i].fareByteCxrAppl->usrCxr.excluded ==
                  (*actualOADConsRestIter)->sodConstraints[i].fareByteCxrAppl->usrCxr.excluded);
            }
          }

          found = true;
          break;
        }
      }
      CPPUNIT_ASSERT(found);
    }
  }

  void assertSODinOAD(RexShoppingTrx::OADConsolidatedConstrainsVect& expectedOADConsRest,
                      RexShoppingTrx::OADConsolidatedConstrainsVect& actualOADConsRest)
  {
    CPPUNIT_ASSERT(expectedOADConsRest.size() == actualOADConsRest.size());
    RexShoppingTrx::OADConsolidatedConstrainsVect::const_iterator actualOADConsRestIter =
        actualOADConsRest.begin();
    for (; actualOADConsRestIter != actualOADConsRest.end(); ++actualOADConsRestIter)
    {
      bool found = false;
      RexShoppingTrx::OADConsolidatedConstrainsVect::const_iterator expectedOADConsRestIter =
          expectedOADConsRest.begin();
      for (; expectedOADConsRestIter != expectedOADConsRest.end(); ++expectedOADConsRestIter)
      {
        if ((*expectedOADConsRestIter)->origAirport == (*actualOADConsRestIter)->origAirport &&
            (*expectedOADConsRestIter)->destAirport == (*actualOADConsRestIter)->destAirport &&
            (*expectedOADConsRestIter)->travelDate.get64BitRepDateOnly() ==
                (*actualOADConsRestIter)->travelDate.get64BitRepDateOnly())
        {
          CPPUNIT_ASSERT((*expectedOADConsRestIter)->sod.size() ==
                         (*actualOADConsRestIter)->sod.size());
          std::vector<RexShoppingTrx::SubOriginDestination*>::const_iterator actSODIter =
              (*actualOADConsRestIter)->sod.begin();
          for (; actSODIter != (*actualOADConsRestIter)->sod.end(); ++actSODIter)
          {
            bool sodFound = false;
            std::vector<RexShoppingTrx::SubOriginDestination*>::const_iterator expSODIter =
                (*expectedOADConsRestIter)->sod.begin();
            for (; expSODIter != (*expectedOADConsRestIter)->sod.end(); ++expSODIter)
            {
              if ((*expSODIter)->origAirport == (*actSODIter)->origAirport &&
                  (*expSODIter)->destAirport == (*actSODIter)->destAirport &&
                  (*expSODIter)->travelDate.get64BitRepDateOnly() ==
                      (*actSODIter)->travelDate.get64BitRepDateOnly())
              {
                CPPUNIT_ASSERT((*expSODIter)->flights.size() == (*actSODIter)->flights.size());
                for (std::set<int>::const_iterator i = (*expSODIter)->flights.begin();
                     i != (*expSODIter)->flights.end();
                     i++)
                  CPPUNIT_ASSERT((*actSODIter)->flights.count(*i));

                CPPUNIT_ASSERT((*expSODIter)->bkcCanChange.size() ==
                               (*actSODIter)->bkcCanChange.size());
                for (std::set<int>::const_iterator i = (*expSODIter)->bkcCanChange.begin();
                     i != (*expSODIter)->bkcCanChange.end();
                     i++)
                  CPPUNIT_ASSERT((*actSODIter)->bkcCanChange.count(*i));

                CPPUNIT_ASSERT((*expSODIter)->forcedConnections.size() ==
                               (*actSODIter)->forcedConnections.size());
                RexShoppingTrx::ForcedConnectionSet::const_iterator expRestFrcCnxIter =
                    (*expSODIter)->forcedConnections.begin();
                for (; expRestFrcCnxIter != (*expSODIter)->forcedConnections.end();
                     ++expRestFrcCnxIter)
                  CPPUNIT_ASSERT((*actSODIter)->forcedConnections.count(*expRestFrcCnxIter) != 0);

                CPPUNIT_ASSERT((*expSODIter)->change == (*actSODIter)->change);
                CPPUNIT_ASSERT((*expSODIter)->exact_cxr == (*actSODIter)->exact_cxr);
                CPPUNIT_ASSERT((*expSODIter)->preferred_cxr == (*actSODIter)->preferred_cxr);

                sodFound = true;
                break;
              }
            }
            CPPUNIT_ASSERT(sodFound);
          }

          found = true;
          break;
        }
      }
      CPPUNIT_ASSERT(found);
    }
  }

  void assertCxrList(std::set<CarrierCode>& actualCxrList, std::set<CarrierCode>& expectedCxrList)
  {
    CPPUNIT_ASSERT(actualCxrList.size() == expectedCxrList.size());
    for (std::set<CarrierCode>::const_iterator i = actualCxrList.begin(); i != actualCxrList.end();
         i++)
      CPPUNIT_ASSERT(expectedCxrList.count(*i));
  }

  void setDataWithRoutingChange(bool allShop = false)
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DEN", true, "2009-09-06 09:00", allShop);
    addOdThruFM("DFW", "MIA", "2009-09-01 09:00");
    addOdThruFM("MIA", "DEN", "2009-09-04 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", false, "2009-09-01 09:00", allShop);
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", false, "2009-09-02 09:00", allShop);
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _thirdONDInfo = _rexConstrainsConsolidator->_ondInfo[2];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));
  }

  void setDataWithRoutingChange_unflown(bool allShop = false)
  {
    cleanStructures();
    addSegment(_newItinSegs, "DFW", "JFK", true, "2009-09-01 09:00", allShop);
    addSegment(_newItinSegs, "JFK", "LAX", true, "2009-09-02 09:00", allShop);
    addSegment(_newItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    addSegment(_newItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    addSegment(_newItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    addSegment(_newItinSegs, "FRA", "DEN", true, "2009-09-06 09:00", allShop);
    addOdThruFM("DFW", "MIA", "2009-09-01 09:00");
    addOdThruFM("MIA", "DEN", "2009-09-04 09:00");
    AirSeg* airSeg1 = addSegment(_excItinSegs, "DFW", "JFK", true, "2009-09-01 09:00", allShop);
    AirSeg* airSeg2 = addSegment(_excItinSegs, "JFK", "LAX", true, "2009-09-02 09:00", allShop);
    AirSeg* airSeg3 = addSegment(_excItinSegs, "LAX", "MIA", true, "2009-09-03 09:00", allShop);
    AirSeg* airSeg4 = addSegment(_excItinSegs, "MIA", "KRK", true, "2009-09-04 09:00", allShop);
    AirSeg* airSeg5 = addSegment(_excItinSegs, "KRK", "FRA", true, "2009-09-05 09:00", allShop);
    AirSeg* airSeg6 = addSegment(_excItinSegs, "FRA", "DFW", true, "2009-09-06 09:00", allShop);
    _rexConstrainsConsolidator->copySegWithoutARUNKs();
    _rexConstrainsConsolidator->detachFlownToNewLeg();
    _firstONDInfo = _rexConstrainsConsolidator->_ondInfo[0];
    _secondONDInfo = _rexConstrainsConsolidator->_ondInfo[1];
    _thirdONDInfo = _rexConstrainsConsolidator->_ondInfo[2];
    _firstFareMarket = FM("DFW", "MIA");
    std::vector<TravelSeg*>* firstFareMarketTvlSeg = &(_firstFareMarket->travelSeg());
    firstFareMarketTvlSeg->push_back(airSeg1);
    firstFareMarketTvlSeg->push_back(airSeg2);
    firstFareMarketTvlSeg->push_back(airSeg3);
    _excItin->fareComponent().push_back(FCInfo(_firstFareMarket));
    _secondFareMarket = FM("MIA", "DFW");
    std::vector<TravelSeg*>* secondFareMarketTvlSeg = &(_secondFareMarket->travelSeg());
    secondFareMarketTvlSeg->push_back(airSeg4);
    secondFareMarketTvlSeg->push_back(airSeg5);
    secondFareMarketTvlSeg->push_back(airSeg6);
    _excItin->fareComponent().push_back(FCInfo(_secondFareMarket));
  }

  void assertFMList(std::vector<const FareMarket*>& actualFMList,
                    std::vector<const FareMarket*>& expectedFMList)
  {
    CPPUNIT_ASSERT(actualFMList.size() == expectedFMList.size());
    for (std::vector<const FareMarket*>::const_iterator i = actualFMList.begin();
         i != actualFMList.end();
         i++)
      CPPUNIT_ASSERT(0 != std::count(expectedFMList.begin(), expectedFMList.end(), *i));
  }

  // helpers end
};
CPPUNIT_TEST_SUITE_REGISTRATION(RexConstrainsConsolidatorTest);
}
