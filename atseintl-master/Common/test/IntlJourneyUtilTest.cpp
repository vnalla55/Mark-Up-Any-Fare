#include <boost/assign/std/vector.hpp>
#include <iostream>
#include <vector>

#include "Common/ClassOfService.h"
#include "Common/IntlJourneyUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestDataBuilders.h"
#include "test/include/TestMemHandle.h"

using namespace boost::assign;

namespace tse
{
class IntlJourneyUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IntlJourneyUtilTest);

  CPPUNIT_TEST(testProcessTsAlreadyInJourney_True);
  CPPUNIT_TEST(testProcessTsAlreadyInJourney_False);

  CPPUNIT_TEST(testDetermineRemainingSegs_All);
  CPPUNIT_TEST(testDetermineRemainingSegs_Some);
  CPPUNIT_TEST(testDetermineRemainingSegs_None);

  CPPUNIT_TEST(testGetInterlineKey_startSegNotFound);
  CPPUNIT_TEST(testGetInterlineKey_oneSeg);
  CPPUNIT_TEST(testGetInterlineKey_twoSeg_sameCxr);
  CPPUNIT_TEST(testGetInterlineKey_twoSeg_sameCxr_firstStopOver);
  CPPUNIT_TEST(testGetInterlineKey_twoSeg_diffCxr);
  CPPUNIT_TEST(testGetInterlineKey_twoSeg_diffCxr_firstStopOver);
  CPPUNIT_TEST(testGetInterlineKey_twoSeg_back_sameCxr);
  CPPUNIT_TEST(testGetInterlineKey_twoSeg_back_sameCxr_firstSo);
  CPPUNIT_TEST(testGetInterlineKey_max3Segs);
  CPPUNIT_TEST(testGetInterlineKey_3Segs_Start2_FwSame_BwDiff);
  CPPUNIT_TEST(testGetInterlineKey_3Segs_Start2_FwSame_BwSame);
  CPPUNIT_TEST(testGetInterlineKey_3Segs_Start3_BwSame_BwSame);
  CPPUNIT_TEST(testGetInterlineKey_3Segs_Start3_BwSame_BwDiff);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start2_FwSame_FwSame);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start2_Stopover3);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start2_Stopover1_Stopover3);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start2_FwDiff_BwDiff);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start2_FwDiff_FwSame);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start2_FwSame_BwDiff);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start3_FwDiff_BwDiff);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start2_FwSame_BwSame);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start3_FwSame_BwSame);
  CPPUNIT_TEST(testGetInterlineKey_4Segs_Start3_FwSame_BwSame_FirstDiff);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _itin;
  AirSegBuilder* _asBuilder;

public:

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _itin = _memHandle.create<Itin>();
    _asBuilder = _memHandle(new AirSegBuilder(_memHandle));
  }

  void tearDown() { _memHandle.clear(); }

  void addToCosAndKey(TravelSeg* ts, IntlJourneyUtil::CosAndKey& cosAndKey)
  {
    if (!cosAndKey.first)
      cosAndKey.first = _memHandle(new IntlJourneyUtil::JourneyCosList);
    cosAndKey.first->resize(cosAndKey.first->size() + 1);
    cosAndKey.second.push_back(ts);
  }

  void testProcessTsAlreadyInJourney_True()
  {
    AirSeg as1, as2, as3, as4;
    IntlJourneyUtil::Journeys journeys(2u);
    addToCosAndKey(&as1, journeys[0]);
    addToCosAndKey(&as2, journeys[0]);
    addToCosAndKey(&as3, journeys[1]);
    addToCosAndKey(&as4, journeys[1]);
    CPPUNIT_ASSERT(IntlJourneyUtil::processTsAlreadyInJourney(journeys, &as4, *_itin));
  }

  void testProcessTsAlreadyInJourney_False()
  {
    AirSeg as1, as2, as3, as4;
    IntlJourneyUtil::Journeys journeys(2u);
    addToCosAndKey(&as1, journeys[0]);
    addToCosAndKey(&as2, journeys[0]);
    addToCosAndKey(&as3, journeys[1]);
    CPPUNIT_ASSERT(!IntlJourneyUtil::processTsAlreadyInJourney(journeys, &as4, *_itin));
  }

  void assertSegVec(const std::vector<TravelSeg*>& actualSegVec,
                    const std::vector<TravelSeg*>& expected)
  {
    CPPUNIT_ASSERT_EQUAL(expected.size(), actualSegVec.size());
    for (size_t pos = 0; pos < expected.size(); ++pos)
    {
      std::ostringstream os;
      os << "Segment mismatch at position " << pos;
      CPPUNIT_ASSERT_EQUAL_MESSAGE(os.str(), expected[pos], actualSegVec[pos]);
    }
  }

  void testDetermineRemainingSegs_All()
  {
    AirSeg* as1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    ArunkSeg arunk;
    AirSeg* as2 = (*_asBuilder).withLocs("CCC", "DDD").withCxr("AA").build();
    IntlJourneyUtil::Journeys journeys(1u);
    addToCosAndKey(as1, journeys[0]);
    addToCosAndKey(&arunk, journeys[0]);
    addToCosAndKey(as2, journeys[0]);
    _itin->travelSeg() += as1, &arunk, as2;

    std::vector<TravelSeg*> outputSegs;
    IntlJourneyUtil::determineRemainingSegs("BB", journeys, *_itin, outputSegs);

    assertSegVec(outputSegs, _itin->travelSeg());
  }

  void testDetermineRemainingSegs_Some()
  {
    AirSeg* as1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* as2 = (*_asBuilder).withLocs("CCC", "DDD").withCxr("BB").build();
    IntlJourneyUtil::Journeys journeys(1u);
    addToCosAndKey(as1, journeys[0]);
    addToCosAndKey(as2, journeys[0]);
    _itin->travelSeg() += as1, as2;

    std::vector<TravelSeg*> outputSegs;
    IntlJourneyUtil::determineRemainingSegs("AA", journeys, *_itin, outputSegs);

    std::vector<TravelSeg*> expected(1u, as2);
    assertSegVec(outputSegs, expected);
  }

  void testDetermineRemainingSegs_None()
  {
    AirSeg* as1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BA").build();
    AirSeg* as2 = (*_asBuilder).withLocs("CCC", "DDD").withCxr("AA").build();
    IntlJourneyUtil::Journeys journeys(1u);
    addToCosAndKey(as1, journeys[0]);
    addToCosAndKey(as2, journeys[0]);
    _itin->travelSeg() += as1, as2;

    std::vector<TravelSeg*> outputSegs;
    IntlJourneyUtil::determineRemainingSegs("AA", journeys, *_itin, outputSegs);
    CPPUNIT_ASSERT(outputSegs.empty());
  }

  void testGetInterlineKey_startSegNotFound()
  {
    AirSeg* startSeg = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").build();
    std::vector<TravelSeg*> inputSegs;
    inputSegs += (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    inputSegs += (*_asBuilder).withLocs("CCC", "DDD").withCxr("BB").build();
    inputSegs += (*_asBuilder).withLocs("DDD", "EEE").withCxr("BB").build();

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(!IntlJourneyUtil::getInterlineKey(startSeg, inputSegs, outputSegs));
  }

  void testGetInterlineKey_oneSeg()
  {
    AirSeg* startSeg = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").build();
    std::vector<TravelSeg*> inputSegs;
    inputSegs += startSeg;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(startSeg, inputSegs, outputSegs));

    assertSegVec(outputSegs, inputSegs);
  }

  void testGetInterlineKey_twoSeg_sameCxr()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("AA").build();
    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg1, inputSegs, outputSegs));
    assertSegVec(outputSegs, inputSegs);
  }

  void testGetInterlineKey_twoSeg_sameCxr_firstStopOver()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").withLegId(0).build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("AA").withLegId(1).build();
    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg1, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected(1u, airSeg1);
    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_twoSeg_diffCxr()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg1, inputSegs, outputSegs));
    assertSegVec(outputSegs, inputSegs);
  }

  void testGetInterlineKey_twoSeg_diffCxr_firstStopOver()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").withLegId(0).build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").withLegId(1).build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg1, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected(1u, airSeg1);
    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_twoSeg_back_sameCxr()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("AA").build();
    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));
    assertSegVec(outputSegs, inputSegs);
  }

  void testGetInterlineKey_twoSeg_back_sameCxr_firstSo()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").withLegId(0).build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("AA").withLegId(1).build();
    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected(1u, airSeg2);
    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_max3Segs()
  {
    AirSeg* startSeg = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").build();
    std::vector<TravelSeg*> inputSegs;
    inputSegs += startSeg;
    inputSegs += (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    inputSegs += (*_asBuilder).withLocs("CCC", "DDD").withCxr("BB").build();
    inputSegs += (*_asBuilder).withLocs("DDD", "EEE").withCxr("BB").build();

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(startSeg, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected(inputSegs.begin(), inputSegs.begin() + 3);
    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_3Segs_Start2_FwSame_BwDiff()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg1, airSeg2, airSeg3;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_3Segs_Start2_FwSame_BwSame()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg1, airSeg2, airSeg3;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_3Segs_Start3_BwSame_BwSame()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg3, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg1, airSeg2, airSeg3;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_3Segs_Start3_BwSame_BwDiff()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg3, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg1, airSeg2, airSeg3;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start2_FwSame_FwSame()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("AA").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("AA").build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("AA").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg2, airSeg3, airSeg4;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start2_Stopover3()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").withLegId(0).build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("AA").withLegId(0).build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("AA").withLegId(0).build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("AA").withLegId(1).build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg1, airSeg2, airSeg3;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start2_Stopover1_Stopover3()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").withLegId(0).build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("AA").withLegId(1).build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("AA").withLegId(1).build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("AA").withLegId(2).build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg2, airSeg3;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start2_FwDiff_BwDiff()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("AA").build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("AA").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg1, airSeg2, airSeg3;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start2_FwDiff_FwSame()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("AA").build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg2, airSeg3, airSeg4;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start2_FwSame_BwDiff()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("AA").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("BB").build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg1, airSeg2, airSeg3;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start3_FwDiff_BwDiff()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("AA").build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg3, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg2, airSeg3, airSeg4;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start2_FwSame_BwSame()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("AA").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("BB").build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg2, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg1, airSeg2, airSeg3;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start3_FwSame_BwSame()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("AA").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("AA").build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg3, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg2, airSeg3, airSeg4;

    assertSegVec(outputSegs, expected);
  }

  void testGetInterlineKey_4Segs_Start3_FwSame_BwSame_FirstDiff()
  {
    AirSeg* airSeg1 = (*_asBuilder).withLocs("AAA", "BBB").withCxr("BB").build();
    AirSeg* airSeg2 = (*_asBuilder).withLocs("BBB", "CCC").withCxr("BB").build();
    AirSeg* airSeg3 = (*_asBuilder).withLocs("CCC", "BBB").withCxr("AA").build();
    AirSeg* airSeg4 = (*_asBuilder).withLocs("BBB", "AAA").withCxr("BB").build();

    std::vector<TravelSeg*> inputSegs;
    inputSegs += airSeg1, airSeg2, airSeg3, airSeg4;

    std::vector<TravelSeg*> outputSegs;
    CPPUNIT_ASSERT(IntlJourneyUtil::getInterlineKey(airSeg3, inputSegs, outputSegs));

    std::vector<TravelSeg*> expected;
    expected += airSeg2, airSeg3, airSeg4;

    assertSegVec(outputSegs, expected);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(IntlJourneyUtilTest);
}
