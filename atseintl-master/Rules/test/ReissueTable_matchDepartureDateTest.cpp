#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/ExchShopCalendarUtils.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"

#include "DBAccess/ReissueSequence.h"
#include "Rules/ReissueTable.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

#include <iostream>

namespace tse
{
namespace
{
bool operator==(const ExchShopCalendar::DateRange& lhs, const ExchShopCalendar::DateRange& rhs)
{
  return lhs.firstDate.getOnlyDate() == rhs.firstDate.getOnlyDate() &&
         lhs.lastDate.getOnlyDate() == rhs.lastDate.getOnlyDate();
}
}
using namespace boost::assign;

class ReissueTable_matchDepartureDateTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ReissueTable_matchDepartureDateTest);

  CPPUNIT_TEST(testDepartureDateNoApply);
  CPPUNIT_TEST(testDepartureDateTheSameDateNoMatch);
  CPPUNIT_TEST(testDepartureDateTheSameDateMatch);
  CPPUNIT_TEST(testDepartureDateDifferDateNoMatchSameDate);
  CPPUNIT_TEST(testDepartureDateDifferDateNoMatchNewDateEarlier);
  CPPUNIT_TEST(testDepartureDateDifferDateMatch);

  CPPUNIT_TEST(testDepartureDateTheSameDateAndOpenOpenSegs);
  CPPUNIT_TEST(testDepartureDateDifferDateAndOpenOpenSegs);
  CPPUNIT_TEST(testDepartureDateTheSameDateAndOpenAirSegs);
  CPPUNIT_TEST(testDepartureDateDifferDateAndOpenAirSegs);
  CPPUNIT_TEST(testDepartureDateDifferDateAndOpenOpenWithoutDateSegs);
  CPPUNIT_TEST(testDepartureDateDifferDateAndOpenWithoutDateAirSegs);
  CPPUNIT_TEST(testDepartureDateTheSameDateAndAirOpenWithoutDateSegs);
  CPPUNIT_TEST(testDepartureDateTheSameDateAndAirOpenWithDateSegs);
  CPPUNIT_TEST(testDepartureDateDifferDateAndAirOpenWithoutDateSegs);
  CPPUNIT_TEST(testDepartureDateDifferDateAndAirOpenWithDateSegs);

  CPPUNIT_TEST(testDepartureDateNoFCInNewItin);
  CPPUNIT_TEST(testDepartureDateIncorrectByte59);

  CPPUNIT_TEST(testDepartureDateTheSameSegmentTwice);
  CPPUNIT_TEST(testDepartureDateTheSameSegmentTwice_ChangeToOpen);

  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_SegmantAB);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_SegmantAC);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_SegmantAD);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_SegmantBC);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_SegmantBD);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_SegmantCD);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_SegmantDC);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_SegmantAE);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_SegmantEF);

  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_TwoSegmantsAB1);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_TwoSegmantsAB2);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_TwoSegmantsAB_CHANGED);

  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_TwoSegmantsAC);
  CPPUNIT_TEST(testFindPotentialFCBeginsInNewItin_TwoSegmantsAC_CHANGED);

  CPPUNIT_TEST(testCalendar_matchDepartureDate_SameDate);
  CPPUNIT_TEST(testCalendar_matchDepartureDate_LaterDate);
  CPPUNIT_TEST(testCalendar_matchDepartureDate_MaxRange);
  CPPUNIT_TEST(testCalendar_matchDepartureDate_MinRange);
  CPPUNIT_TEST(testCalendar_matchDepartureDate_wrongFCbegin);
  CPPUNIT_TEST(testCalendar_matchDepartureDate_rangeTest_sameDate);
  CPPUNIT_TEST(testCalendar_matchDepartureDate_rangeTest_laterDate);
  CPPUNIT_TEST(testCalendar_matchDepartureDate_rangeTest_noMatch);
  CPPUNIT_TEST(testCalendar_matchDepartureDate_rangeTest_laterDateEdgeCase);
  CPPUNIT_TEST(testCalendar_matchDepartureDate_rangeTest_earlierDateEdgeCase);

  CPPUNIT_TEST_SUITE_END();

private:
  class CustomValidator : public ExchShopCalendar::R3ValidationResult
  {
  public:
    CustomValidator(const std::vector<PricingTrx::OriginDestination>& ondVec,
                    const Itin& excItin,
                    const Itin& newItin,
                    DataHandle& dh)
      : ExchShopCalendar::R3ValidationResult(ondVec, excItin, newItin, dh)
    {
    }

    int32_t getOndIndexForSeg(const TravelSeg& seg) const override { return 0; }

  private:
    bool
    isSameCity(const TravelSeg* lhs, const TravelSeg* rhs, DataHandle& dh, bool isBoarding) const
        override
    {
      auto func = [&](const TravelSeg& seg)
      { return (isBoarding ? seg.boardMultiCity() : seg.offMultiCity()); };

      return func(*lhs) == func(*rhs);
    }
  };

  ReissueTable* _reissueTable;
  RexPricingTrx* _trx;
  ReissueSequence* _r3;
  FareMarket* _fareMarket;
  Itin* _newItin, *_dummyItin;
  TestMemHandle _memH;
  CustomValidator* _customValidator;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _memH.create<RootLoggerGetOff>();

    _trx = _memH.create<RexPricingTrx>();

    _dummyItin = _memH.create<Itin>();

    _newItin = _memH.create<Itin>();
    _trx->itin().push_back(_newItin);

    _r3 = _memH.create<ReissueSequence>();
    _fareMarket = _memH.create<FareMarket>();

    _reissueTable = _memH.insert(new ReissueTable(*_trx, _newItin, (DiagCollector*)0));
  }

  void tearDown() { _memH.clear(); }

  void testDepartureDateNoApply()
  {
    _r3->dateInd() = ReissueTable::NOT_APPLY;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 13, 0, 0));

    _fareMarket->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 10, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 10, 14, 35, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 13, 0, 0));

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateTheSameDateNoMatch()
  {
    _r3->dateInd() = ReissueTable::MATCH_SAME_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 40, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 10, 14, 25, 0));

    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateTheSameDateMatch()
  {
    _r3->dateInd() = ReissueTable::MATCH_SAME_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 15, 25, 0));

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateDifferDateNoMatchSameDate()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 16, 25, 0));

    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateDifferDateNoMatchNewDateEarlier()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 12, 25, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 5, 14, 25, 0));

    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateDifferDateMatch()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 4, 16, 25, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateTheSameDateAndOpenOpenSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_SAME_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Open, DateTime(2001, 2, 3, 14, 20, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Open, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateDifferDateAndOpenOpenSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Open, DateTime(2001, 2, 4, 10, 0, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Open, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateDifferDateAndOpenOpenWithoutDateSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Open, DateTime()),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Open, DateTime());

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateDifferDateAndOpenWithoutDateAirSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 1, 26, 19, 0, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Open, DateTime());

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateTheSameDateAndOpenAirSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_SAME_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 4, 10, 0, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Open, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateDifferDateAndOpenAirSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 10, 45, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Open, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateTheSameDateAndAirOpenWithoutDateSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_SAME_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Open, DateTime()),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateTheSameDateAndAirOpenWithDateSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_SAME_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Open, DateTime(2001, 2, 3, 17, 25, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateDifferDateAndAirOpenWithoutDateSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Open, DateTime()),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateDifferDateAndAirOpenWithDateSegs()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Open, DateTime(2001, 2, 10, 14, 0, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateNoFCInNewItin()
  {
    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "DFW", 2, Air, DateTime(2001, 2, 10, 14, 15, 0)),
        createSeg("DFW", "NYC", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateIncorrectByte59()
  {
    _r3->dateInd() = 'A';

    _newItin->travelSeg() += createSeg("NYC", "CHI", 1, Air, DateTime(2001, 1, 25, 9, 0, 0)),
        createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 15, 25, 0)),
        createSeg("LAX", "CHI", 3, Air, DateTime(2001, 2, 15, 8, 0, 0));

    _fareMarket->travelSeg() += createSeg("CHI", "LAX", 2, Air, DateTime(2001, 2, 3, 14, 25, 0));

    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateTheSameSegmentTwice()
  {
    _r3->dateInd() = ReissueTable::MATCH_SAME_DEPARTURE_DATE;

    _newItin->travelSeg() += createSeg("HOU", "MSY", 1, Air, DateTime(2009, 6, 1, 8, 25, 0)),
        createSeg("MSY", "HOU", 2, Air, DateTime(2009, 6, 8, 9, 20, 0)),
        createSeg("HOU", "MSY", 3, Air, DateTime(2009, 6, 10, 8, 25, 0));

    _fareMarket->travelSeg() = _newItin->travelSeg();

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void testDepartureDateTheSameSegmentTwice_ChangeToOpen()
  {
    _r3->dateInd() = ReissueTable::MATCH_SAME_DEPARTURE_DATE;

    _newItin->travelSeg() +=
        createSeg("HOU", "MSY", 1, Open, DateTime(2009, 6, 1, 8, 25, 0), TravelSeg::CHANGED),
        createSeg("MSY", "HOU", 2, Air, DateTime(2009, 6, 8, 9, 20, 0), TravelSeg::CHANGED),
        createSeg("HOU", "MSY", 3, Air, DateTime(2009, 6, 10, 8, 25, 0), TravelSeg::CHANGED);

    _fareMarket->travelSeg() +=
        createSeg("HOU", "MSY", 1, Air, DateTime(2009, 6, 1, 8, 25, 0), TravelSeg::CHANGED),
        createSeg("MSY", "HOU", 2, Air, DateTime(2009, 6, 8, 9, 20, 0), TravelSeg::CHANGED),
        createSeg("HOU", "MSY", 3, Air, DateTime(2009, 6, 10, 8, 25, 0), TravelSeg::CHANGED);

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3));
  }

  void assertPiece(const std::pair<std::vector<TravelSeg*>::const_iterator,
                                   std::vector<TravelSeg*>::const_iterator>& result,
                   TravelSeg* begin,
                   TravelSeg* end)
  {
    CPPUNIT_ASSERT_EQUAL(begin, *result.first);
    CPPUNIT_ASSERT_EQUAL(end, *result.second);
  }

  std::vector<TravelSeg*> populateABCDSegments()
  {
    std::vector<TravelSeg*> seg;
    seg += createSeg("A", "B", 1), createSeg("B", "C", 2), createSeg("C", "D", 3);
    return seg;
  }

  std::vector<TravelSeg*> populateABABSegments()
  {
    std::vector<TravelSeg*> seg;
    seg += createSeg("A", "B", 1), createSeg("B", "A", 2), createSeg("A", "B", 3);
    return seg;
  }

  void testFindPotentialFCBeginsInNewItin_SegmantAB()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABCDSegments();
    expect += segs[0];
    _reissueTable->findPotentialFCBeginsInNewItin("A", "B", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_SegmantAC()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABCDSegments();
    expect += segs[0];
    _reissueTable->findPotentialFCBeginsInNewItin("A", "C", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_SegmantAD()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABCDSegments();
    expect += segs[0];
    _reissueTable->findPotentialFCBeginsInNewItin("A", "D", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_SegmantBC()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABCDSegments();
    expect += segs[1];
    _reissueTable->findPotentialFCBeginsInNewItin("B", "C", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_SegmantBD()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABCDSegments();
    expect += segs[1];
    _reissueTable->findPotentialFCBeginsInNewItin("B", "D", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_SegmantCD()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABCDSegments();
    expect += segs[2];
    _reissueTable->findPotentialFCBeginsInNewItin("C", "D", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_SegmantDC()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABCDSegments();

    _reissueTable->findPotentialFCBeginsInNewItin("D", "C", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_SegmantAE()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABCDSegments();

    _reissueTable->findPotentialFCBeginsInNewItin("A", "E", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_SegmantEF()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABCDSegments();

    _reissueTable->findPotentialFCBeginsInNewItin("E", "F", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_TwoSegmantsAB1()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABABSegments();
    expect += segs[0], segs[2];
    _reissueTable->findPotentialFCBeginsInNewItin("A", "B", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_TwoSegmantsAB_CHANGED()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABABSegments();
    segs[2]->changeStatus() = TravelSeg::CHANGED;
    expect += segs[2];
    _reissueTable->findPotentialFCBeginsInNewItin("A", "B", TravelSeg::CHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_TwoSegmantsAB2()
  {
    std::vector<TravelSeg*> result, expect, segs = populateABABSegments();
    segs.insert(segs.begin(), createSeg("C", "A", 1));
    segs += createSeg("B", "C", 2);
    expect += segs[1], segs[3];
    _reissueTable->findPotentialFCBeginsInNewItin("A", "B", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_TwoSegmantsAC()
  {
    std::vector<TravelSeg*> result, expect, segs;
    segs += createSeg("A", "B", 1), createSeg("B", "C", 2), createSeg("C", "D", 3),
        createSeg("D", "A", 4), createSeg("A", "C", 5);
    expect += segs[0], segs[4];
    _reissueTable->findPotentialFCBeginsInNewItin("A", "C", TravelSeg::UNCHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void testFindPotentialFCBeginsInNewItin_TwoSegmantsAC_CHANGED()
  {
    std::vector<TravelSeg*> result, expect, segs;
    segs += createSeg("A", "B", 1), createSeg("B", "C", 2), createSeg("C", "D", 3),
        createSeg("D", "A", 4), createSeg("A", "C", 5, TravelSeg::CHANGED);
    expect += segs[4];
    _reissueTable->findPotentialFCBeginsInNewItin("A", "C", TravelSeg::CHANGED, segs, result);
    CPPUNIT_ASSERT(expect == result);
  }

  void prepareTrxForCalendar(Indicator dateIndicator,
                             const std::vector<AirSeg*>& previousTicketSegs,
                             const std::vector<AirSeg*>& newTicketSegs,
                             int shiftBefore,
                             int shiftAfter)
  {
    _r3->dateInd() = dateIndicator;
    for (auto seg : previousTicketSegs)
      _fareMarket->travelSeg() += seg;
    for (auto seg : newTicketSegs)
      _newItin->travelSeg() += seg;

    PricingTrx::OriginDestination ond;
    ond.calDaysAfter = shiftBefore;
    ond.calDaysBefore = shiftAfter;
    ond.travelDate = newTicketSegs.front()->departureDT();
    _trx->orgDest.push_back(ond);
    _trx->setTrxType(PricingTrx::RESHOP_TRX);

    const std::vector<PricingTrx::OriginDestination> onds{_trx->originDest().front()};
    _newItin->fareMarket().push_back(_fareMarket);
    _customValidator =
        _memH.create<CustomValidator>(onds, *_newItin, *_newItin, *_memH.create<DataHandle>());
  }

  void testCalendar_matchDepartureDate_SameDate()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_SAME_DEPARTURE_DATE,
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          3,
                          3);
    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));

    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;
    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));
  }

  void testCalendar_matchDepartureDate_LaterDate()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_SAME_DEPARTURE_DATE,
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 20))},
                          3,
                          3);
    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));

    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;
    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));
  }

  void testCalendar_matchDepartureDate_MaxRange()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_SAME_DEPARTURE_DATE,
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 18))},
                          3,
                          3);
    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));

    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;
    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));
  }

  void testCalendar_matchDepartureDate_MinRange()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_SAME_DEPARTURE_DATE,
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 12))},
                          3,
                          3);
    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));

    _r3->dateInd() = ReissueTable::MATCH_LATER_DEPARTURE_DATE;
    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));
  }

  void testCalendar_matchDepartureDate_wrongFCbegin()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_SAME_DEPARTURE_DATE,
                          {createSeg("ATH", "NYC", 1, Air, DateTime(2016, 6, 15)),
                           createSeg("NYC", "ATH", 2, Air, DateTime(2016, 6, 20))},
                          {createSeg("NYC", "ATH", 2, Air, DateTime(2016, 6, 15))},
                          3,
                          3);
    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));
  }

  bool datecmp(const DateTime dt,
               const char* strdt,
               DateFormat format = DateFormat::YYYYMMDD,
               const char* separator = "-")
  {
    return dt.dateToString(format, separator) == strdt;
  }

  void testCalendar_matchDepartureDate_rangeTest_sameDate()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_SAME_DEPARTURE_DATE,
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          3,
                          3);

    _customValidator->addDateRange({DateTime(2016, 6, 12), DateTime(2016, 6, 18)}, 0);

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));

    auto dr = _reissueTable->_dateIndRange[ReissueTable::MATCH_SAME_DEPARTURE_DATE];
    auto& ond = _trx->orgDest.front();
    auto result = ExchShopCalendar::DateRange{ond.travelDate, ond.travelDate};

    CPPUNIT_ASSERT(dr == result);
  }

  void testCalendar_matchDepartureDate_rangeTest_laterDate()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_LATER_DEPARTURE_DATE,
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          3,
                          3);

    _customValidator->addDateRange({DateTime(2016, 6, 12), DateTime(2016, 6, 18)}, 0);
    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));

    auto dr = _reissueTable->_dateIndRange[ReissueTable::MATCH_LATER_DEPARTURE_DATE];
    auto& ond = _trx->orgDest.front();
    auto result = ExchShopCalendar::DateRange{ond.travelDate.addDays(1), ond.travelDate.addDays(3)};

    CPPUNIT_ASSERT(dr == result);
  }

  void testCalendar_matchDepartureDate_rangeTest_earlierDateEdgeCase()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_LATER_DEPARTURE_DATE,
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 15))},
                          {createSeg("NYC", "ATH", 1, Air, DateTime(2016, 6, 13))},
                          3,
                          3);

    _customValidator->addDateRange({DateTime(2016, 6, 12), DateTime(2016, 6, 18)}, 0);

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));

    auto dr = _reissueTable->_dateIndRange[ReissueTable::MATCH_LATER_DEPARTURE_DATE];
    auto& ond = _trx->orgDest.front();
    auto result = ExchShopCalendar::DateRange{ond.travelDate.addDays(3), ond.travelDate.addDays(3)};

    CPPUNIT_ASSERT(dr == result);
  }

  void testCalendar_matchDepartureDate_rangeTest_noMatch()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_SAME_DEPARTURE_DATE,
                          {createSeg("FOO", "BAR", 1, Air, DateTime(2016, 6, 15))},
                          {createSeg("FOO", "BAR", 1, Air, DateTime(2016, 6, 20))},
                          3,
                          3);

    _customValidator->addDateRange({DateTime(2016, 6, 12), DateTime(2016, 6, 18)}, 0);

    CPPUNIT_ASSERT(!_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));
    CPPUNIT_ASSERT(_reissueTable->_dateIndRange.size() == 0);
  }

  void testCalendar_matchDepartureDate_rangeTest_laterDateEdgeCase()
  {
    prepareTrxForCalendar(ReissueTable::MATCH_LATER_DEPARTURE_DATE,
                          {createSeg("FOO", "BAR", 1, Air, DateTime(2016, 6, 15))},
                          {createSeg("FOO", "BAR", 1, Air, DateTime(2016, 6, 18))},
                          3,
                          3);

    _customValidator->addDateRange({DateTime(2016, 6, 15), DateTime(2016, 6, 21)}, 0);

    CPPUNIT_ASSERT(_reissueTable->matchDepartureDate(*_fareMarket, *_r3, _customValidator));

    auto dr = _reissueTable->_dateIndRange[ReissueTable::MATCH_LATER_DEPARTURE_DATE];
    auto& ond = _trx->orgDest.front();
    auto result =
        ExchShopCalendar::DateRange{ond.travelDate.subtractDays(2), ond.travelDate.addDays(3)};

    CPPUNIT_ASSERT(dr == result);
  }

  AirSeg* createSeg(const LocCode& board,
                    const LocCode& off,
                    const int32_t legId,
                    TravelSeg::ChangeStatus status = TravelSeg::UNCHANGED)
  {
    AirSeg* seg = _memH.create<AirSeg>();
    CPPUNIT_ASSERT(off != board);
    seg->boardMultiCity() = seg->origAirport() = board;
    seg->offMultiCity() = seg->destAirport() = off;
    seg->changeStatus() = status;
    seg->legId() = legId;
    return seg;
  }

  AirSeg* createSeg(const LocCode& board,
                    const LocCode& off,
                    const int32_t legId,
                    const TravelSegType& type,
                    const DateTime& depDateTime,
                    TravelSeg::ChangeStatus status = TravelSeg::UNCHANGED)
  {
    AirSeg* seg = createSeg(board, off, legId, status);
    if (type == Air)
      CPPUNIT_ASSERT(depDateTime != DateTime());
    seg->segmentType() = type;
    seg->hasEmptyDate() = (depDateTime == DateTime());
    seg->departureDT() = depDateTime;
    return seg;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ReissueTable_matchDepartureDateTest);

} // tse
