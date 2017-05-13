// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include <boost/range.hpp>

#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/Diversity.h"
#include "Pricing/RequiredNonStopsCalculator.h"

namespace tse
{

// =================================
// LEGS DATA
// =================================

DateTime date1 = DateTime(2013, 06, 01);
DateTime date2 = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,           arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(date1, 10), DT(date1, 12) },
  { 0, 1, "AA", "JFK", "DFW", "AA", DT(date1, 10), DT(date1, 12) },
  { 0, 2, "LH", "JFK", "SAO", "LH", DT(date1, 10), DT(date1, 12) },
  { 0, 2, "LH", "SAO", "DFW", "LH", DT(date1, 12), DT(date1, 14) },
  { 1, 0, "LH", "DFW", "LAX", "LH", DT(date2, 10), DT(date2, 12) },
  { 1, 1, "AA", "DFW", "LAX", "AA", DT(date2, 10), DT(date2, 12) },
  { 1, 2, "AA", "DFW", "LAX", "AA", DT(date2, 10), DT(date2, 12) }
};
#undef DT

// ==================================
// TEST
// ==================================

class RequiredNonStopsCalculatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RequiredNonStopsCalculatorTest);
  CPPUNIT_TEST(testAllSolutions);
  CPPUNIT_TEST(testOnlineSolutions);
  CPPUNIT_TEST(testInterlineSolutions);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  ShoppingTrx::FlightMatrix _fliMatrix;
  ShoppingTrx::EstimateMatrix _estMatrix;
  RequiredNonStopsCalculator* _reqNSCalc;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _reqNSCalc = _memHandle.create<RequiredNonStopsCalculator>();
    CPPUNIT_ASSERT(_trx);
    CPPUNIT_ASSERT(_reqNSCalc);

    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();

    addToFlightMatrix(0, 0); // LH non-stop
    addToFlightMatrix(1, 1); // AA non-stop
    addToEstimateMatrix(0, 0); // AA non-stop
    addToFlightMatrix(0, 1); // interline non-stop
    addToFlightMatrix(1, 0); // interline non-stop
    addToFlightMatrix(2, 0); // not a non-stop
  }

  void tearDown()
  {
    _memHandle.clear();
    _fliMatrix.clear();
    _estMatrix.clear();
  }

  void testAllSolutions()
  {
    shpq::DiversityTestUtil divUtil(_trx->diversity());
    divUtil.setMaxOnlineNonStopCount(10u);
    divUtil.setMaxInterlineNonStopCount(20u);
    divUtil.setNonStopOptionsCount(100u);

    _reqNSCalc->init(*_trx);
    _reqNSCalc->countAlreadyGeneratedNS(*_trx, _fliMatrix);
    _reqNSCalc->countAlreadyGeneratedNS(*_trx, _estMatrix);
    std::size_t reqNSCount = _reqNSCalc->calcRequiredNSCount(*_trx);

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(25u), reqNSCount);

    divUtil.setNonStopOptionsCount(24u);
    reqNSCount = _reqNSCalc->calcRequiredNSCount(*_trx);

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(24u), reqNSCount);
  }

  void testOnlineSolutions()
  {
    shpq::DiversityTestUtil divUtil(_trx->diversity());
    divUtil.setMaxOnlineNonStopCount(10u);
    divUtil.setMaxInterlineNonStopCount(20u);
    divUtil.setNonStopOptionsCount(100u);

    _trx->onlineSolutionsOnly() = true;
    _reqNSCalc->init(*_trx);
    _reqNSCalc->countAlreadyGeneratedNS(*_trx, _fliMatrix);
    _reqNSCalc->countAlreadyGeneratedNS(*_trx, _estMatrix);
    std::size_t reqNSCount = _reqNSCalc->calcRequiredNSCount(*_trx);

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(7u), reqNSCount);
  }

  void testInterlineSolutions()
  {
    shpq::DiversityTestUtil divUtil(_trx->diversity());
    divUtil.setMaxOnlineNonStopCount(10u);
    divUtil.setMaxInterlineNonStopCount(20u);
    divUtil.setNonStopOptionsCount(100u);

    _trx->interlineSolutionsOnly() = true;
    _reqNSCalc->init(*_trx);
    _reqNSCalc->countAlreadyGeneratedNS(*_trx, _fliMatrix);
    _reqNSCalc->countAlreadyGeneratedNS(*_trx, _estMatrix);
    std::size_t reqNSCount = _reqNSCalc->calcRequiredNSCount(*_trx);

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(18u), reqNSCount);
  }

  void addToFlightMatrix(int obSop, int ibSop)
  {
    SopIdVec sops;
    sops.push_back(obSop);
    sops.push_back(ibSop);
    _fliMatrix.insert(std::make_pair(sops, (GroupFarePath*)0));
  }

  void addToEstimateMatrix(int obSop, int ibSop)
  {
    SopIdVec sops;
    sops.push_back(obSop);
    sops.push_back(ibSop);
    _estMatrix.insert(std::make_pair(sops, ShoppingTrx::EstimatedSolution()));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RequiredNonStopsCalculatorTest);
}
