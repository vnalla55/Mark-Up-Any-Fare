//----------------------------------------------------------------------------
//  Copyright Sabre 2009
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
#include "Diagnostic/Diag941Collector.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/LegsBuilder.h"
#include "test/testdata/TestShoppingTrxFactory.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class ItinStatisticMock : public ItinStatistic
{
public:
  ItinStatisticMock(ShoppingTrx& trx) : ItinStatistic(trx) {}

  void setAdditionalNSCount(size_t online, size_t interline)
  {
    _additionalOnlineNonStopsCount = online;
    _additionalInterlineNonStopsCount = interline;
  }
};

// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 06, 01);
static DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 6),  DT(obDate, 7) },  // 1h
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }, // 1h
  { 1, 1, "DL", "DFW", "JFK", "DL", DT(ibDate, 12), DT(ibDate, 13) }  // 1h
};
#undef DT

// =================================
// TEST
// =================================

class Diag941CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag941CollectorTest);
  CPPUNIT_TEST(testNonStopHeader);
  CPPUNIT_TEST(testNonStopActions);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);

    Diagnostic& root = _trx->diagnostic();
    _dc.trx() = _trx;
    _dc.rootDiag() = &root;
    _dc.diagnosticType() = Diagnostic941;
    _dc.enable(Diagnostic941);
    _dc.activate();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testNonStopHeader()
  {
    shpq::DiversityTestUtil util(_trx->diversity());
    util.setMaxInterlineNonStopCount(111);
    util.setMaxOnlineNonStopCount(222);
    util.setNonStopOptionsCount(300);

    _dc.printNonStopsHeader();
    CPPUNIT_ASSERT_EQUAL(size_t(0u), _dc.str().size());

    _dc._diagNonStops = true;
    _dc.printNonStopsHeader();

    std::string expected;
    expected += "Non-stop statistic legend:\n";
    expected += "\tO - number of additional online non-stops\n";
    expected += "\tI - number of additional interline non-stops\n";
    expected += "\tT - total number of additional non-stops\n\n";
    expected += "Non-stop diversity:\n";
    expected += "\tNumber of additional non-stop options to generate: 300\n";
    expected += "\tMax number of non-stop options: 333\n";
    expected += "\tMax number of online non-stop options: 222\n";
    expected += "\tMax number of interline non-stop options: 111\n\n";

    CPPUNIT_ASSERT_EQUAL(expected, _dc.str());
  }

  void testNonStopActions()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();

    ItinStatisticMock stats(*_trx);
    stats.setAdditionalNSCount(10, 20);

    SopIdVec combination = {0,1};

    _dc.printNonStopAction(Diag941Collector::ADD_NS, combination, stats);
    CPPUNIT_ASSERT_EQUAL(size_t(0u), _dc.str().size());

    _dc._diagNonStops = true;
    _dc.printNonStopAction(Diag941Collector::ADD_NS, combination, stats);
    CPPUNIT_ASSERT_EQUAL(size_t(0u), _dc.str().size());

    _dc.flushNonStopActions();
    std::string output = _dc.str();
    CPPUNIT_ASSERT(output.find("ADD_NS") != std::string::npos);
    CPPUNIT_ASSERT(output.find("0x1") != std::string::npos);
    CPPUNIT_ASSERT(output.find("O[10] I[20] T[30]") != std::string::npos);

    _dc.printNonStopAction(Diag941Collector::ADD_ANS, combination, stats);
    _dc.printNonStopAction(Diag941Collector::SWAPPER_ADD_ANS, combination, stats);
    _dc.printNonStopAction(Diag941Collector::SWAPPER_REM_NS, combination, stats);
    _dc.printNonStopAction(Diag941Collector::SWAPPER_REM_ANS, combination, stats);
    _dc.flushNonStopActions();
    output = _dc.str();
    CPPUNIT_ASSERT(output.find("ADD_ANS") != std::string::npos);
    CPPUNIT_ASSERT(output.find("SWAPPER_ADD_ANS") != std::string::npos);
    CPPUNIT_ASSERT(output.find("SWAPPER_REM_NS") != std::string::npos);
    CPPUNIT_ASSERT(output.find("SWAPPER_REM_ANS") != std::string::npos);
  }

private:
  ShoppingTrx* _trx;
  Diag941Collector _dc;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag941CollectorTest);
}
