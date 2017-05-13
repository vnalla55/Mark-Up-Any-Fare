#include "test/include/CppUnitHelperMacros.h"
#include <vector>

#include "Common/DateTime.h"
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Rules/BlackoutDates.h"
#include "Rules/BlackoutDates.h"
#include "Rules/DatePredicates.h"
#include "Rules/RuleItem.h"

#include "test/include/TestMemHandle.h"

namespace tse
{

class BlackoutDatesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BlackoutDatesTest);

  CPPUNIT_TEST(testInitialize_RangeDates);
  CPPUNIT_TEST(testInitialize_SpecifiedDates);
  CPPUNIT_TEST(testInitialize_RangeDates_AnyYear);
  CPPUNIT_TEST(testInitialize_SpecifiedDates_AnyYear);
  CPPUNIT_TEST(testInitialize_RangeDates_International);
  CPPUNIT_TEST(testInitialize_SpecifiedDates_International);

  CPPUNIT_TEST(testProcess_AnyYearRangeDates_InRangeRightBoundary);
  CPPUNIT_TEST(testProcess_AnyYearRangeDates_OutRangeRightBoundary);
  CPPUNIT_TEST(testProcess_AnyYearRangeDates_InRangeLeftBoundary);
  CPPUNIT_TEST(testProcess_AnyYearRangeDates_OutRangeLeftBoundary);

  CPPUNIT_TEST(testProcess_RangeDates_InRangeRightBoundary);
  CPPUNIT_TEST(testProcess_RangeDates_OutRangeRightBoundary);
  CPPUNIT_TEST(testProcess_RangeDates_InRangeLeftBoundary);
  CPPUNIT_TEST(testProcess_RangeDates_OutRangeLeftBoundary);

  CPPUNIT_TEST(testProcess_AnyYearSpecifiedDates_FirstDate);
  CPPUNIT_TEST(testProcess_AnyYearSpecifiedDates_SecondDate);
  CPPUNIT_TEST(testProcess_AnyYearSpecifiedDates_Between);

  CPPUNIT_TEST(testProcess_SpecifiedDates_FirstDate);
  CPPUNIT_TEST(testProcess_SpecifiedDates_SecondDate);
  CPPUNIT_TEST(testProcess_SpecifiedDates_Between);

  CPPUNIT_TEST(testProcess_IntlRestriction_RangeDates_Domestic);
  CPPUNIT_TEST(testProcess_IntlRestriction_RangeDates_Mixed);
  CPPUNIT_TEST(testProcess_IntlRestriction_RangeDates_International);

  CPPUNIT_TEST(testGetTextOnlyPredicate_Unavailable);
  CPPUNIT_TEST(testGetTextOnlyPredicate_TextOnly);
  CPPUNIT_TEST(testGetTextOnlyPredicate_NoPredicate);

  CPPUNIT_TEST(testGetSameSegmentsPredicate_Range);
  CPPUNIT_TEST(testGetSameSegmentsPredicate_Dates);

  CPPUNIT_TEST(testGetContainsInternationalPredicate_Applicable);
  CPPUNIT_TEST(testGetContainsInternationalPredicate_NotApplicable);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memH(new PricingTrx);
    _record3 = _memH(new BlackoutInfo);
    _validator = _memH(new BlackoutDates);
  }

  void tearDown() { _memH.clear(); }

  void populateDate(std::string date, int& year, int& month, int& day)
  {
    std::string::size_type pos = date.find("0000");
    if (pos != std::string::npos)
      date.replace(pos, 4, "1899");

    DateTime dt(date, 0);
    year = dt.year() - 1900;
    month = dt.month();
    day = dt.day();
  }

  void populateRecord3(char intlRest, char appl, const char* startTime, const char* stopTime)
  {
    _record3->intlRest() = intlRest;
    _record3->blackoutAppl() = appl;

    _record3->overrideDateTblItemNo() = 0;
    _record3->geoTblItemNoBetween() = 0;
    _record3->geoTblItemNoAnd() = 0;

    populateDate(
        startTime, _record3->tvlStartYear(), _record3->tvlStartMonth(), _record3->tvlStartDay());

    populateDate(
        stopTime, _record3->tvlStopYear(), _record3->tvlStopMonth(), _record3->tvlStopDay());
  }

  Loc* createLoc(const LocCode& location, const NationCode& nation)
  {
    Loc* loc = _memH(new Loc);
    loc->loc() = location;
    loc->nation() = nation;
    return loc;
  }

  AirSeg* createSegment(const Loc* dest, const Loc* orig, const std::string& departure)
  {
    AirSeg* seg = _memH(new AirSeg);
    seg->destination() = dest;
    seg->origin() = orig;
    seg->departureDT() = DateTime(departure, 0);

    return seg;
  }

  AirSeg* createNycLonSegment(const std::string& departure)
  {
    return createSegment(createLoc("NYC", "US"), createLoc("LON", "UK"), departure);
  }

  AirSeg* createLonNycSegment(const std::string& departure)
  {
    return createSegment(createLoc("LON", "UK"), createLoc("NYC", "US"), departure);
  }

  AirSeg* createNycLaxSegment(const std::string& departure)
  {
    return createSegment(createLoc("NYC", "US"), createLoc("LAX", "US"), departure);
  }

  AirSeg* createLaxNycSegment(const std::string& departure)
  {
    return createSegment(createLoc("LAX", "US"), createLoc("NYC", "US"), departure);
  }

  PaxTypeFare& createPaxTypeFare()
  {
    PaxTypeFare* ptf = _memH(new PaxTypeFare);
    ptf->fareMarket() = _memH(new FareMarket);
    return *ptf;
  }

  void populateR3RangeDates(const char* startTime, const char* stopTime)
  {
    populateRecord3(BlackoutDates::BLANK, BlackoutDates::RANGE, startTime, stopTime);
    _validator->initialize(*_trx, _record3);
  }

  void populateR3SpecifiedDates(const char* startTime, const char* stopTime)
  {
    populateRecord3(BlackoutDates::BLANK, BlackoutDates::DATES, startTime, stopTime);
    _validator->initialize(*_trx, _record3);
  }

  void populateR3InternationalRangeDates(const char* startTime, const char* stopTime)
  {
    populateRecord3(BlackoutDates::INT_DONT_APPLY, BlackoutDates::RANGE, startTime, stopTime);
    _validator->initialize(*_trx, _record3);
  }

  void testInitialize_RangeDates()
  {
    populateR3RangeDates("2008-08-01", "2008-08-15");

    CPPUNIT_ASSERT_EQUAL(std::string("SameSegment\n"
                                     " +And\n"
                                     " | +Not\n"
                                     " | | +IsDateBetween (2008-8-1 -> 2008-8-15)\n"),
                         _validator->_root->toString());
  }

  void testInitialize_SpecifiedDates()
  {
    populateR3SpecifiedDates("2008-03-09", "2008-03-11");

    CPPUNIT_ASSERT_EQUAL(std::string("SameSegment\n"
                                     " +And\n"
                                     " | +Not\n"
                                     " | | +IsDate (2008-3-9, 2008-3-11)\n"),
                         _validator->_root->toString());
  }

  void testInitialize_RangeDates_AnyYear()
  {
    populateR3RangeDates("0000-03-09", "0000-08-09");

    CPPUNIT_ASSERT_EQUAL(std::string("SameSegment\n"
                                     " +And\n"
                                     " | +Not\n"
                                     " | | +IsDateBetween (-1-3-9 -> -1-8-9)\n"),
                         _validator->_root->toString());
  }

  void testInitialize_SpecifiedDates_AnyYear()
  {
    populateR3SpecifiedDates("0000-03-09", "0000-03-11");

    CPPUNIT_ASSERT_EQUAL(std::string("SameSegment\n"
                                     " +And\n"
                                     " | +Not\n"
                                     " | | +IsDate (-1-3-9, -1-3-11)\n"),
                         _validator->_root->toString());
  }

  void testInitialize_RangeDates_International()
  {
    populateR3InternationalRangeDates("2008-10-01", "2008-10-31");

    CPPUNIT_ASSERT_EQUAL(std::string("IfThenElse\n"
                                     " +ContainsInternational\n"
                                     " +Pass\n"
                                     " +SameSegment\n"
                                     " | +And\n"
                                     " | | +Not\n"
                                     " | | | +IsDateBetween (2008-10-1 -> 2008-10-31)\n"),
                         _validator->_root->toString());
  }

  void testInitialize_SpecifiedDates_International()
  {
    populateRecord3(
        BlackoutDates::INT_DONT_APPLY, BlackoutDates::DATES, "2008-10-01", "2008-10-31");
    _validator->initialize(*_trx, _record3);

    CPPUNIT_ASSERT_EQUAL(std::string("IfThenElse\n"
                                     " +ContainsInternational\n"
                                     " +Pass\n"
                                     " +SameSegment\n"
                                     " | +And\n"
                                     " | | +Not\n"
                                     " | | | +IsDate (2008-10-1, 2008-10-31)\n"),
                         _validator->_root->toString());
  }

  void testProcess_AnyYearRangeDates_OutRangeRightBoundary()
  {
    populateR3RangeDates("0000-03-09", "0000-08-09");
    ;

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2008-08-10"));

    CPPUNIT_ASSERT_EQUAL(PASS, _validator->process(ptf, *_trx));
  }

  void testProcess_AnyYearRangeDates_InRangeRightBoundary()
  {
    populateR3RangeDates("0000-03-09", "0000-08-09");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2007-08-09"));

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator->process(ptf, *_trx));
  }

  void testProcess_AnyYearRangeDates_InRangeLeftBoundary()
  {
    populateR3RangeDates("0000-03-09", "0000-08-09");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2006-03-09"));

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator->process(ptf, *_trx));
  }

  void testProcess_AnyYearRangeDates_OutRangeLeftBoundary()
  {
    populateR3RangeDates("0000-03-09", "0000-08-09");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2005-03-08"));

    CPPUNIT_ASSERT_EQUAL(PASS, _validator->process(ptf, *_trx));
  }

  void testProcess_RangeDates_OutRangeRightBoundary()
  {
    populateR3RangeDates("2008-08-01", "2008-08-15");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2008-08-16"));

    CPPUNIT_ASSERT_EQUAL(PASS, _validator->process(ptf, *_trx));
  }

  void testProcess_RangeDates_InRangeRightBoundary()
  {
    populateR3RangeDates("2008-08-01", "2008-08-15");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2008-08-15"));

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator->process(ptf, *_trx));
  }

  void testProcess_RangeDates_InRangeLeftBoundary()
  {
    populateR3RangeDates("2008-08-01", "2008-08-15");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2008-08-01"));

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator->process(ptf, *_trx));
  }

  void testProcess_RangeDates_OutRangeLeftBoundary()
  {
    populateR3RangeDates("2008-08-01", "2008-08-15");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2008-07-31"));

    CPPUNIT_ASSERT_EQUAL(PASS, _validator->process(ptf, *_trx));
  }

  void testProcess_AnyYearSpecifiedDates_Between()
  {
    populateR3SpecifiedDates("0000-03-09", "0000-03-11");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2007-03-10"));

    CPPUNIT_ASSERT_EQUAL(PASS, _validator->process(ptf, *_trx));
  }

  void testProcess_AnyYearSpecifiedDates_FirstDate()
  {
    populateR3SpecifiedDates("0000-03-09", "0000-03-11");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2006-03-09"));

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator->process(ptf, *_trx));
  }

  void testProcess_AnyYearSpecifiedDates_SecondDate()
  {
    populateR3SpecifiedDates("0000-03-09", "0000-03-11");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2009-03-11"));

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator->process(ptf, *_trx));
  }

  void testProcess_SpecifiedDates_Between()
  {
    populateR3SpecifiedDates("2008-03-09", "2008-03-11");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2008-03-10"));

    CPPUNIT_ASSERT_EQUAL(PASS, _validator->process(ptf, *_trx));
  }

  void testProcess_SpecifiedDates_FirstDate()
  {
    populateR3SpecifiedDates("2008-03-09", "2008-03-11");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2008-03-09"));

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator->process(ptf, *_trx));
  }

  void testProcess_SpecifiedDates_SecondDate()
  {
    populateR3SpecifiedDates("2008-03-09", "2008-03-11");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2008-03-11"));

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator->process(ptf, *_trx));
  }

  void testProcess_IntlRestriction_RangeDates_Domestic()
  {
    populateR3InternationalRangeDates("2008-10-01", "2008-10-31");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLaxSegment("2008-10-25"));
    ptf.fareMarket()->travelSeg().push_back(createLaxNycSegment("2008-10-25"));

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator->process(ptf, *_trx));
  }

  void testProcess_IntlRestriction_RangeDates_Mixed()
  {
    populateR3InternationalRangeDates("2008-10-01", "2008-10-31");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLaxSegment("2008-10-25"));
    ptf.fareMarket()->travelSeg().push_back(createLonNycSegment("2008-10-29"));

    CPPUNIT_ASSERT_EQUAL(PASS, _validator->process(ptf, *_trx));
  }

  void testProcess_IntlRestriction_RangeDates_International()
  {
    populateR3InternationalRangeDates("2008-10-01", "2008-10-31");

    PaxTypeFare& ptf = createPaxTypeFare();
    ptf.fareMarket()->travelSeg().push_back(createNycLonSegment("2008-10-25"));
    ptf.fareMarket()->travelSeg().push_back(createLonNycSegment("2008-10-27"));

    CPPUNIT_ASSERT_EQUAL(PASS, _validator->process(ptf, *_trx));
  }

  static const DateTime TICKETING_DATE;

  std::vector<TravelSeg*>& createSegments(const DateTime& dt)
  {
    AirSeg* seg = _memH.create<AirSeg>();
    seg->departureDT() = dt;
    std::vector<TravelSeg*>* segments = _memH(new std::vector<TravelSeg*>(1, seg));
    return *segments;
  }

  void populateDates(const DateTime& start, const DateTime& stop, BlackoutInfo& info)
  {
    info.tvlStartYear() = start.year() - 1900;
    info.tvlStartMonth() = start.month();
    info.tvlStartDay() = start.day();
    info.tvlStopYear() = stop.year() - 1900;
    info.tvlStopMonth() = stop.month();
    info.tvlStopDay() = stop.day();
  }

  void testGetTextOnlyPredicate_NoPredicate()
  {
    BlackoutInfo info;
    info.unavailtag() = BlackoutDates::BLANK;

    Predicate* p = _validator->getTextOnlyPredicate(info);

    CPPUNIT_ASSERT(!p);
  }

  void testGetTextOnlyPredicate_Unavailable()
  {
    BlackoutInfo info;
    info.unavailtag() = BlackoutDates::UNAVAILABLE;

    Predicate* p = _validator->getTextOnlyPredicate(info);

    CPPUNIT_ASSERT(p);
    CPPUNIT_ASSERT_EQUAL(std::string("TextOnly\n"), p->toString());
    CPPUNIT_ASSERT_EQUAL(FAIL, (p->test(createSegments(TICKETING_DATE), *_trx)).valid);
  }

  void testGetTextOnlyPredicate_TextOnly()
  {
    BlackoutInfo info;
    info.unavailtag() = BlackoutDates::TEXT_ONLY;

    Predicate* p = _validator->getTextOnlyPredicate(info);

    CPPUNIT_ASSERT(p);
    CPPUNIT_ASSERT_EQUAL(std::string("TextOnly\n"), p->toString());
    CPPUNIT_ASSERT_EQUAL(SKIP, (p->test(createSegments(TICKETING_DATE), *_trx)).valid);
  }

  void testGetSameSegmentsPredicate_Range()
  {
    BlackoutInfo info;
    populateDates(TICKETING_DATE, TICKETING_DATE.addYears(1), info);
    info.blackoutAppl() = BlackoutDates::RANGE;

    Predicate* p = _validator->getSameSegmentsPredicate(info);

    CPPUNIT_ASSERT(p);
    CPPUNIT_ASSERT_EQUAL(std::string("SameSegment\n"
                                     " +And\n"
                                     " | +Not\n"
                                     " | | +IsDateBetween (2011-6-1 -> 2012-6-1)\n"),
                         p->toString());
    CPPUNIT_ASSERT_EQUAL(FAIL, (p->test(createSegments(TICKETING_DATE), *_trx)).valid);
    CPPUNIT_ASSERT_EQUAL(PASS,
                         (p->test(createSegments(TICKETING_DATE.subtractDays(1)), *_trx)).valid);
  }

  void testGetSameSegmentsPredicate_Dates()
  {
    BlackoutInfo info;
    populateDates(TICKETING_DATE, TICKETING_DATE.addYears(1), info);
    info.blackoutAppl() = BlackoutDates::DATES;

    Predicate* p = _validator->getSameSegmentsPredicate(info);

    CPPUNIT_ASSERT(p);
    CPPUNIT_ASSERT_EQUAL(std::string("SameSegment\n"
                                     " +And\n"
                                     " | +Not\n"
                                     " | | +IsDate (2011-6-1, 2012-6-1)\n"),
                         p->toString());
    CPPUNIT_ASSERT_EQUAL(FAIL, (p->test(createSegments(TICKETING_DATE), *_trx)).valid);
    CPPUNIT_ASSERT_EQUAL(PASS, (p->test(createSegments(TICKETING_DATE.addDays(1)), *_trx)).valid);
  }

  void testGetContainsInternationalPredicate_NotApplicable()
  {
    BlackoutInfo info;
    populateDates(TICKETING_DATE, TICKETING_DATE.addYears(1), info);
    info.blackoutAppl() = BlackoutDates::DATES;
    info.intlRest() = BlackoutDates::BLANK;

    Predicate* p = _validator->getContainsInternationalPredicate(info);

    CPPUNIT_ASSERT(!p);
  }

  void testGetContainsInternationalPredicate_Applicable()
  {
    BlackoutInfo info;
    populateDates(TICKETING_DATE, TICKETING_DATE.addYears(1), info);
    info.blackoutAppl() = BlackoutDates::DATES;
    info.intlRest() = BlackoutDates::INT_DONT_APPLY;

    Predicate* p = _validator->getContainsInternationalPredicate(info);

    CPPUNIT_ASSERT(p);
    CPPUNIT_ASSERT_EQUAL(std::string("IfThenElse\n"
                                     " +ContainsInternational\n"
                                     " +Pass\n"
                                     " +SameSegment\n"
                                     " | +And\n"
                                     " | | +Not\n"
                                     " | | | +IsDate (2011-6-1, 2012-6-1)\n"),
                         p->toString());
  }

protected:
  TestMemHandle _memH;
  PricingTrx* _trx;
  BlackoutInfo* _record3;
  BlackoutDates* _validator;
};

const DateTime
BlackoutDatesTest::TICKETING_DATE(2011, 6, 1);

CPPUNIT_TEST_SUITE_REGISTRATION(BlackoutDatesTest);

} // tse
