//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include <boost/assign/std/set.hpp>

#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/test/ProcessTagInfoMock.h"
#include "Diagnostic/Diag689Collector.h"
#include "RexPricing/StopoverConnectionValidator.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class StopoverConnectionValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StopoverConnectionValidatorTest);

  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testConstructor_diagnostic);
  CPPUNIT_TEST(testSetDiagnostic);
  //CPPUNIT_TEST(testMatch_diagnostic);

  CPPUNIT_TEST(testGetSegmentsWithinFareComponents_oneFcWithOneSeg);
  CPPUNIT_TEST(testGetSegmentsWithinFareComponents_oneFcWithTwoSeg);
  CPPUNIT_TEST(testGetSegmentsWithinFareComponents_oneFcWithThreeSeg);
  CPPUNIT_TEST(testGetSegmentsWithinFareComponents_twoFcWithFourSegsEach);
  CPPUNIT_TEST(testGetSegmentsWithinFareComponents_twoFcWithTwoSeg);

  CPPUNIT_TEST(testMatchImpl_badByte);
  CPPUNIT_TEST(testMatchImpl_blankConx);
  CPPUNIT_TEST(testMatchImpl_blankStop);
  CPPUNIT_TEST(testMatchImpl_identicalConx);
  CPPUNIT_TEST(testMatchImpl_identicalStop);
  CPPUNIT_TEST(testMatchImpl_stop);
  CPPUNIT_TEST(testMatchImpl_failStopLoc);
  CPPUNIT_TEST(testMatchImpl_failStopType);
  CPPUNIT_TEST(testMatchImpl_conx);
  CPPUNIT_TEST(testMatchImpl_failConxLoc);
  CPPUNIT_TEST(testMatchImpl_fcNotFound);
  CPPUNIT_TEST(testMatchImpl_destChanged);
  CPPUNIT_TEST(testMatchImpl_inbound);
  CPPUNIT_TEST(testMatchImpl_failInbount);
  CPPUNIT_TEST(testMatchImpl_stopTypeChanged);

  CPPUNIT_TEST(testInit_noSegments);
  CPPUNIT_TEST(testInit_someSegements);

  CPPUNIT_TEST(testMatch_diagnosticPass);
  CPPUNIT_TEST(testMatch_diagnosticFailConnections);
  CPPUNIT_TEST(testMatch_diagnosticFailStopovers);
  CPPUNIT_TEST(testMatch_diagnosticFailBothStopovers);
  CPPUNIT_TEST(testMatch_diagnosticFailBothConnections);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    create<TestConfigInitializer>();
    create<RootLoggerGetOff>();

    _trx = create<RexPricingTrx>();

    _excFarePath = create<FarePath>();
    _trx->exchangeItin().push_back(create<ExcItin>());
    _trx->exchangeItin().front()->rexTrx() = _trx;
    _trx->exchangeItin().front()->farePath().push_back(_excFarePath);

    _farePath = create<FarePath>();
    _farePath->itin() = create<Itin>();

    _trx->itin().push_back(_farePath->itin());

    Diag689Collector* dc = 0;
    _validator = _memHandle.create<StopoverConnectionValidator>(*_trx, *_farePath, dc);
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  template <typename T>
  T* create()
  {
    return _memHandle.create<T>();
  }

  std::string getDiagString()
  {
    _validator->_diag->flushMsg();
    return _validator->_diag->str();
  }

  Diag689Collector* createDiagnostic()
  {
    Diag689Collector* diag = create<Diag689Collector>();
    diag->activate();
    diag->trx() = _trx;
    return diag;
  }

  void testConstructor()
  {
    CPPUNIT_ASSERT_EQUAL(static_cast<const RexBaseTrx*>(_trx), &_validator->_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<const FarePath*>(_farePath), &_validator->_farePath);
    CPPUNIT_ASSERT_EQUAL(static_cast<Diag689Collector*>(0), _validator->_diag);
  }

  void testConstructor_diagnostic()
  {
    Diag689Collector* dc = createDiagnostic();
    StopoverConnectionValidator val(*_trx, *_farePath, dc);
    CPPUNIT_ASSERT_EQUAL(static_cast<const RexBaseTrx*>(_trx), &_validator->_trx);
    CPPUNIT_ASSERT_EQUAL(static_cast<const FarePath*>(_farePath), &val._farePath);
    CPPUNIT_ASSERT_EQUAL(dc, val._diag);
  }

  void testSetDiagnostic()
  {
    Diag689Collector* dc = createDiagnostic();
    _validator->setDiagnostic(dc);
    CPPUNIT_ASSERT_EQUAL(dc, _validator->_diag);
  }

  void testMatch_diagnostic()
  {
    _validator->setDiagnostic(createDiagnostic());

//    CPPUNIT_ASSERT(_validator->match(*perm));
//    CPPUNIT_ASSERT(!getDiagString().empty());
  }

  enum SegType
  {
    STOP = 0,
    CONX
  };

  TravelSeg* createSeg(const LocCode& board, const LocCode& off, SegType type)
  {
    AirSeg* seg = create<AirSeg>();
    seg->boardMultiCity() = board;
    seg->offMultiCity() = off;
    seg->stopOver() = type == STOP;
    return seg;
  }

  template<int size>
  FareUsage* createFareUsage(TravelSeg* (&segs)[size])
  {
    FareUsage* fu = create<FareUsage>();
    for (TravelSeg** c = segs; c != segs+size; ++c)
      fu->travelSeg() += *c;
    return fu;
  }

  template<int size>
  PricingUnit* createPricingUnit(TravelSeg* (&segs)[size])
  {
    PricingUnit* pu = create<PricingUnit>();
    pu->fareUsage() += createFareUsage(segs);
    return pu;
  }

  template<int size1, int size2>
  PricingUnit* createPricingUnit(TravelSeg* (&segs1)[size1], TravelSeg* (&segs2)[size2])
  {
    PricingUnit* pu = createPricingUnit(segs1);
    pu->fareUsage() += createFareUsage(segs2);
    return pu;
  }

  void testGetSegmentsWithinFareComponents_oneFcWithOneSeg()
  {
    TravelSeg* fc1[] = {createSeg("A", "B", CONX)};
    _farePath->pricingUnit() += createPricingUnit(fc1);

    std::vector<TravelSeg*> segments, expect;
    _validator->getSegmentsWithinFareComponents(*_farePath, segments);

    CPPUNIT_ASSERT_EQUAL(expect, segments);
  }

  void testGetSegmentsWithinFareComponents_oneFcWithTwoSeg()
  {
    TravelSeg* fc1[] = {createSeg("A", "B", CONX),
        createSeg("B", "C", CONX)};
    _farePath->pricingUnit() += createPricingUnit(fc1);

    std::vector<TravelSeg*> segments, expect;
    _validator->getSegmentsWithinFareComponents(*_farePath, segments);

    expect += fc1[0];
    CPPUNIT_ASSERT_EQUAL(expect, segments);
  }

  void testGetSegmentsWithinFareComponents_oneFcWithThreeSeg()
  {
    TravelSeg* fc1[] = {createSeg("A", "B", CONX),
        createSeg("B", "C", CONX),
        createSeg("C", "D", CONX)};
    _farePath->pricingUnit() += createPricingUnit(fc1);

    std::vector<TravelSeg*> segments, expect;
    _validator->getSegmentsWithinFareComponents(*_farePath, segments);

    expect += fc1[0], fc1[1];
    CPPUNIT_ASSERT_EQUAL(expect, segments);
  }

  void testGetSegmentsWithinFareComponents_twoFcWithFourSegsEach()
  {
    TravelSeg* fc1[] = {createSeg("A", "B", CONX),
        createSeg("B", "C", CONX),
        createSeg("C", "D", CONX),
        createSeg("D", "E", CONX)};
    TravelSeg* fc2[] = {createSeg("E", "C", CONX),
        createSeg("C", "D", CONX),
        createSeg("D", "B", CONX),
        createSeg("B", "A", CONX)};
    _farePath->pricingUnit() += createPricingUnit(fc1), createPricingUnit(fc2);

    std::vector<TravelSeg*> segments, expect;
    _validator->getSegmentsWithinFareComponents(*_farePath, segments);

    expect += fc1[0], fc1[1], fc1[2],
        fc2[0], fc2[1], fc2[2];
    CPPUNIT_ASSERT_EQUAL(expect, segments);
  }

  void testGetSegmentsWithinFareComponents_twoFcWithTwoSeg()
  {
    TravelSeg* fc1[] = {createSeg("A", "B", CONX),
                        createSeg("B", "C", CONX)};
    TravelSeg* fc2[] = {createSeg("C", "D", CONX),
        createSeg("D", "E", CONX)};
    _farePath->pricingUnit() += createPricingUnit(fc1, fc2);

    std::vector<TravelSeg*> segments, expect;
    _validator->getSegmentsWithinFareComponents(*_farePath, segments);

    expect += fc1[0], fc2[0];
    CPPUNIT_ASSERT_EQUAL(expect, segments);
  }

  typedef StopoverConnectionValidator::LocationsSet LocationsSet;

  enum { FAIL = 0, PASS = 1 };

  void assertMatch(const Indicator& byte, bool expect,
      const LocationsSet& expectStopovers, const LocationsSet& expectConnections)
  {
    LocationsSet unmatchedStopovers, unmatchedConnections;
    CPPUNIT_ASSERT_EQUAL(expect, _validator->matchImpl(byte, unmatchedStopovers, unmatchedConnections));

    CPPUNIT_ASSERT_EQUAL(expectStopovers, unmatchedStopovers);
    CPPUNIT_ASSERT_EQUAL(expectConnections, unmatchedConnections);
  }

  typedef StopoverConnectionValidator::ValidationStatusCache::Status Status;

  void assertValidationCacheState(const Status& determined, bool expect,
                                  const Status& notDeterminedA,
                                  const Status& notDeterminedB)
  {
    CPPUNIT_ASSERT(determined.isDetermined);
    CPPUNIT_ASSERT_EQUAL(determined.value, expect);
    CPPUNIT_ASSERT(!notDeterminedA.isDetermined);
    CPPUNIT_ASSERT(!notDeterminedB.isDetermined);
  }

  void testMatchImpl_badByte()
  {
    LocationsSet expectStopovers, expectConnections;
    assertMatch('!', FAIL, expectStopovers, expectConnections);
    CPPUNIT_ASSERT(!_validator->_statusCache->connections.isDetermined);
    CPPUNIT_ASSERT(!_validator->_statusCache->stopovers.isDetermined);
    CPPUNIT_ASSERT(!_validator->_statusCache->both.isDetermined);
  }

  void testMatchImpl_blankConx()
  {
    _validator->_excSegs += createSeg("A", "B", CONX), createSeg("B", "C", CONX);
    _validator->_newSegs += createSeg("D", "E", CONX), createSeg("E", "F", CONX);

    LocationsSet expectStopovers, expectConnections;
    assertMatch(ProcessTagPermutation::STOPCONN_BLANK, PASS, expectStopovers, expectConnections);
    CPPUNIT_ASSERT(!_validator->_statusCache->connections.isDetermined);
    CPPUNIT_ASSERT(!_validator->_statusCache->stopovers.isDetermined);
    CPPUNIT_ASSERT(!_validator->_statusCache->both.isDetermined);
  }

  void testMatchImpl_blankStop()
  {
    _validator->_excSegs += createSeg("A", "B", STOP), createSeg("B", "C", STOP);
    _validator->_newSegs += createSeg("D", "E", STOP), createSeg("E", "F", STOP);

    LocationsSet expectStopovers, expectConnections;
    assertMatch(ProcessTagPermutation::STOPCONN_BLANK, PASS, expectStopovers, expectConnections);
    CPPUNIT_ASSERT(!_validator->_statusCache->connections.isDetermined);
    CPPUNIT_ASSERT(!_validator->_statusCache->stopovers.isDetermined);
    CPPUNIT_ASSERT(!_validator->_statusCache->both.isDetermined);
  }

  void testMatchImpl_identicalConx()
  {
    _validator->_excSegs += createSeg("A", "B", CONX), createSeg("B", "C", CONX);
    _validator->_newSegs += createSeg("A", "B", CONX), createSeg("B", "C", CONX);

    LocationsSet expectStopovers, expectConnections;
    assertMatch(ProcessTagPermutation::STOPCONN_B, PASS, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->both, PASS,
                               _validator->_statusCache->connections,
                               _validator->_statusCache->stopovers);
  }

  void testMatchImpl_identicalStop()
  {
    _validator->_excSegs += createSeg("A", "B", STOP), createSeg("B", "C", STOP);
    _validator->_newSegs += createSeg("A", "B", STOP), createSeg("B", "C", STOP);

    LocationsSet expectStopovers, expectConnections;
    assertMatch(ProcessTagPermutation::STOPCONN_B, PASS, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->both, PASS,
                               _validator->_statusCache->connections,
                               _validator->_statusCache->stopovers);
  }

  void testMatchImpl_stop()
  {
    _validator->_excSegs += createSeg("A", "B", STOP), createSeg("B", "C", STOP);
    _validator->_newSegs += createSeg("A", "B", STOP), createSeg("B", "C", STOP);

    LocationsSet expectStopovers, expectConnections;
    assertMatch(ProcessTagPermutation::STOPCONN_S, PASS, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->stopovers, PASS,
                               _validator->_statusCache->connections,
                               _validator->_statusCache->both);
  }

  void testMatchImpl_failStopLoc()
  {
    _validator->_excSegs += createSeg("A", "B", STOP), createSeg("B", "C", STOP);
    _validator->_newSegs += createSeg("A", "B", STOP), createSeg("B", "D", STOP);

    LocationsSet expectStopovers, expectConnections;
    expectStopovers += "C", "D";
    assertMatch(ProcessTagPermutation::STOPCONN_S, FAIL, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->stopovers, FAIL,
                               _validator->_statusCache->connections,
                               _validator->_statusCache->both);
  }

  void testMatchImpl_failStopType()
  {
    _validator->_excSegs += createSeg("A", "B", STOP), createSeg("B", "C", STOP);
    _validator->_newSegs += createSeg("A", "B", STOP), createSeg("B", "C", CONX);

    LocationsSet expectStopovers, expectConnections;
    expectStopovers += "C";
    assertMatch(ProcessTagPermutation::STOPCONN_S, FAIL, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->stopovers, FAIL,
                               _validator->_statusCache->connections,
                               _validator->_statusCache->both);
  }

  void testMatchImpl_conx()
  {
    _validator->_excSegs += createSeg("A", "B", CONX), createSeg("B", "C", CONX);
    _validator->_newSegs += createSeg("A", "B", CONX), createSeg("B", "C", CONX);

    LocationsSet expectStopovers, expectConnections;
    assertMatch(ProcessTagPermutation::STOPCONN_C, PASS, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->connections, PASS,
                               _validator->_statusCache->both,
                               _validator->_statusCache->stopovers);
  }

  void testMatchImpl_failConxLoc()
  {
    _validator->_excSegs += createSeg("A", "B", CONX), createSeg("B", "C", CONX);
    _validator->_newSegs += createSeg("A", "B", CONX), createSeg("B", "D", CONX);

    LocationsSet expectStopovers, expectConnections;
    expectConnections += "C", "D";
    assertMatch(ProcessTagPermutation::STOPCONN_C, FAIL, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->connections, FAIL,
                               _validator->_statusCache->both,
                               _validator->_statusCache->stopovers);
  }

  void testMatchImpl_failConxType()
  {
    _validator->_excSegs += createSeg("A", "B", CONX), createSeg("B", "C", CONX);
    _validator->_newSegs += createSeg("A", "B", CONX), createSeg("B", "D", STOP);

    LocationsSet expectStopovers, expectConnections;
    expectConnections += "C";
    assertMatch(ProcessTagPermutation::STOPCONN_C, FAIL, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->connections, FAIL,
                               _validator->_statusCache->both,
                               _validator->_statusCache->stopovers);
  }

  void testMatchImpl_fcNotFound()
  {
    _validator->_excSegs += createSeg("A", "B", CONX),
        createSeg("B", "C", STOP),
        createSeg("C", "D", CONX);
    _validator->_newSegs += createSeg("A", "B", CONX),
        createSeg("B", "E", STOP),
        createSeg("E", "F", CONX);

    LocationsSet expectStopovers, expectConnections;
    expectStopovers += "C", "E";
    assertMatch(ProcessTagPermutation::STOPCONN_S, FAIL, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->stopovers, FAIL,
                               _validator->_statusCache->connections,
                               _validator->_statusCache->both);
  }

  void testMatchImpl_destChanged()
  {
    _validator->_excSegs += createSeg("A", "B", STOP),
        createSeg("B", "C", CONX),
        createSeg("C", "D", CONX);
    _validator->_newSegs += createSeg("A", "E", STOP),
        createSeg("E", "C", CONX),
        createSeg("C", "D", CONX),
        createSeg("D", "F", CONX);

    LocationsSet expectStopovers, expectConnections;
    expectConnections += "F";
    assertMatch(ProcessTagPermutation::STOPCONN_C, FAIL, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->connections, FAIL,
                               _validator->_statusCache->both,
                               _validator->_statusCache->stopovers);
  }

  void testMatchImpl_inbound()
  {
    _validator->_excSegs += createSeg("A", "B", CONX),
        createSeg("B", "C", STOP),
        createSeg("C", "D", CONX);
    _validator->_newSegs += createSeg("D", "E", CONX),
        createSeg("E", "B", CONX),
        createSeg("B", "A", CONX),
        createSeg("A", "B", CONX),
        createSeg("B", "C", STOP),
        createSeg("C", "D", CONX);

    LocationsSet expectStopovers, expectConnections;
    assertMatch(ProcessTagPermutation::STOPCONN_S, PASS, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->stopovers, PASS,
                               _validator->_statusCache->connections,
                               _validator->_statusCache->both);
  }

  void testMatchImpl_failInbount()
  {
    _validator->_excSegs += createSeg("A", "B", STOP),
        createSeg("B", "C", CONX),
        createSeg("C", "D", CONX);
    _validator->_newSegs += createSeg("D", "C", CONX),
        createSeg("C", "B", CONX),
        createSeg("B", "A", CONX),
        createSeg("A", "B", STOP),
        createSeg("B", "E", CONX),
        createSeg("E", "D", CONX);

    LocationsSet expectStopovers, expectConnections;
    expectConnections += "A", "B", "E";
    assertMatch(ProcessTagPermutation::STOPCONN_C, FAIL, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->connections, FAIL,
                               _validator->_statusCache->both,
                               _validator->_statusCache->stopovers);
  }

  void testMatchImpl_stopTypeChanged()
  {
    _validator->_excSegs += createSeg("A", "B", CONX),
        createSeg("B", "C", STOP),
        createSeg("C", "D", CONX);
    _validator->_newSegs += createSeg("A", "B", CONX),
        createSeg("B", "C", CONX),
        createSeg("C", "D", CONX);

    LocationsSet expectStopovers, expectConnections;
    expectConnections += "C";
    assertMatch(ProcessTagPermutation::STOPCONN_B, FAIL, expectStopovers, expectConnections);
    assertValidationCacheState(_validator->_statusCache->both, FAIL,
                               _validator->_statusCache->connections,
                               _validator->_statusCache->stopovers);
  }

  void testInit_noSegments()
  {
    _validator->init();

    std::vector<TravelSeg*> expect;
    CPPUNIT_ASSERT_EQUAL(expect, _validator->_excSegs);
    CPPUNIT_ASSERT_EQUAL(expect, _validator->_newSegs);
  }


  void testInit_someSegements()
  {

    TravelSeg* fc1[] = {createSeg("A", "B", CONX),
        createSeg("B", "C", CONX),
        createSeg("C", "D", CONX),
        createSeg("D", "E", CONX)};
    TravelSeg* fc2[] = {createSeg("E", "F", CONX),
        createSeg("F", "G", CONX),
        createSeg("G", "H", CONX),
        createSeg("H", "I", CONX)};

    _excFarePath->pricingUnit() += createPricingUnit(fc1), createPricingUnit(fc2);
    _farePath->pricingUnit() += createPricingUnit(fc1), createPricingUnit(fc2);

    _validator->init();

    std::vector<TravelSeg*> expect;
    expect += fc1[0], fc1[1], fc1[2], fc2[0], fc2[1], fc2[2];
    CPPUNIT_ASSERT_EQUAL(expect, _validator->_excSegs);
    CPPUNIT_ASSERT_EQUAL(expect, _validator->_newSegs);
  }

  ProcessTagPermutation* createProcessTagPermutation(Indicator ind)
  {
    ReissueSequence* sq = create<ReissueSequence>();
    sq->stopoverConnectInd() = ind;
    ProcessTagInfo* info = create<ProcessTagInfo>();
    info->reissueSequence()->orig() = sq;
    ProcessTagPermutation* perm = create<ProcessTagPermutation>();
    perm->processTags() += info;
    return perm;
  }

  void populateSegs(SegType seg)
  {
    _validator->_excSegs += createSeg("A", "B", STOP),
        createSeg("B", "C", CONX),
        createSeg("C", "D", STOP);
    _validator->_newSegs += createSeg("A", "B", STOP),
        createSeg("B", "C", seg),
        createSeg("C", "D", STOP);
  }

  void testMatch_diagnosticPass()
  {
    populateSegs(STOP);
    _validator->setDiagnostic(createDiagnostic());
    _validator->match(*createProcessTagPermutation(ProcessTagPermutation::STOPCONN_BLANK));
    CPPUNIT_ASSERT_EQUAL(std::string("STOP/CONX CHECK:  \n"), getDiagString());
  }


  void testMatch_diagnosticFailConnections()
  {
    populateSegs(STOP);
    _validator->setDiagnostic(createDiagnostic());
    _validator->match(*createProcessTagPermutation(ProcessTagPermutation::STOPCONN_C));
    CPPUNIT_ASSERT_EQUAL(std::string("STOP/CONX CHECK: C\n"
        "  UNMATCHED CONX: C \n"), getDiagString());
  }

  void testMatch_diagnosticFailStopovers()
  {
    populateSegs(STOP);
    _validator->setDiagnostic(createDiagnostic());
    _validator->match(*createProcessTagPermutation(ProcessTagPermutation::STOPCONN_S));
    CPPUNIT_ASSERT_EQUAL(std::string("STOP/CONX CHECK: S\n"
        "  UNMATCHED STPS: C \n"), getDiagString());
  }

  void testMatch_diagnosticFailBothStopovers()
  {
    populateSegs(STOP);
    _validator->setDiagnostic(createDiagnostic());
    _validator->match(*createProcessTagPermutation(ProcessTagPermutation::STOPCONN_B));
    CPPUNIT_ASSERT_EQUAL(std::string("STOP/CONX CHECK: B\n"
        "  UNMATCHED CONX: C \n"), getDiagString());
  }

  void testMatch_diagnosticFailBothConnections()
  {
    populateSegs(CONX);
    _validator->_newSegs += createSeg("D", "E", STOP);
    _validator->setDiagnostic(createDiagnostic());
    _validator->match(*createProcessTagPermutation(ProcessTagPermutation::STOPCONN_B));
    CPPUNIT_ASSERT_EQUAL(std::string("STOP/CONX CHECK: B\n"
        "  UNMATCHED STPS: E \n"), getDiagString());
  }

protected:
  FarePath* _farePath, *_excFarePath;
  RexPricingTrx* _trx;
  TestMemHandle _memHandle;
  StopoverConnectionValidator* _validator;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StopoverConnectionValidatorTest);

std::ostream&
operator<<(std::ostream& os, const TravelSeg* seg)
{
  return os << seg->boardMultiCity() << "-" << seg->offMultiCity() << " ";
}

} // tse

