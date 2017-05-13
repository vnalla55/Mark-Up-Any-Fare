#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>

#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "ItinAnalyzer/CouponMatcher.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;

class CouponMatcherTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CouponMatcherTest);

  CPPUNIT_TEST(testGetFirstUnflownSegment_noSegements);
  CPPUNIT_TEST(testGetFirstUnflownSegment_firstSegment);
  CPPUNIT_TEST(testGetFirstUnflownSegment_lastSegment);
  CPPUNIT_TEST(testGetFirstUnflownSegment_fullyFlown);

  CPPUNIT_TEST(testMatchUnflown_InternationalPass);
  CPPUNIT_TEST(testMatchUnflown_InternationalWithAddedSegmentPass);
  CPPUNIT_TEST(testMatchUnflown_SingleInternationalFlightPass);
  CPPUNIT_TEST(testMatchUnflown_SingleDomesticToSingleInternationFail);
  CPPUNIT_TEST(testMatchUnflown_SingleDomesticFlightPass);
  CPPUNIT_TEST(testMatchUnflown_DomesticToInternationalFail);
  CPPUNIT_TEST(testMatchUnflown_MatchAllInternationalFlightsPass);
  CPPUNIT_TEST(testMatchUnflown_SingleInternationalToSingleDomesticPass);
  CPPUNIT_TEST(testMatchUnflown_DomesticAndInternationalFlightsNoNeedToMapDomestic1Pass);
  CPPUNIT_TEST(testMatchUnflown_DomesticAndInternationalFlightsNoNeedToMapDomestic2Pass);
  CPPUNIT_TEST(testMatchUnflown_DomesticAndInternationalFlightsNoNeedToMapDomestic3Pass);
  CPPUNIT_TEST(testMatchUnflown_DomesticAndInternationalFlightsMapDomesticFail);
  CPPUNIT_TEST(
      testMatchUnflown_DomesticAndInternationalFlightsNoNeedToMapDomesticExchangeItinIsLongerPass);
  CPPUNIT_TEST(testMatchUnflown_DomesticAndInternationalFlightsSomeCouponsFlownFail);
  CPPUNIT_TEST(testMatchUnflown_MatchInternationalFlightsToAnyInternationalSectorPass);
  CPPUNIT_TEST(testMatchUnflown_DomesticAndInternationalFlightsNoMatchForDomesticFail);
  CPPUNIT_TEST(testMatchUnflown_DomesticAndInternationalOnlyOneSectorUnflownFail);
  CPPUNIT_TEST(testMatchUnflown_DomesticAndInternationalOnlyOneSectorUnflownPass);
  CPPUNIT_TEST(testMatchUnflown_OneInternationalManyDomesticOnNewPass);
  CPPUNIT_TEST(testMatchUnflown_DomesticInUsChangedToFlightToCanadaFail);
  CPPUNIT_TEST(testMatchUnflown_DomesticChangedToDomesticPass);
  CPPUNIT_TEST(testMatchUnflown_DomesticChangedToInternationalFail);
  CPPUNIT_TEST(testMatchUnflown_DomesticAndInternationalChangedToInternationalPass);
  CPPUNIT_TEST(testMatchUnflown_DomesticJira_AAP235Pass);

  CPPUNIT_TEST(testGetTravelType_transborder);
  CPPUNIT_TEST(testGetTravelType_international);
  CPPUNIT_TEST(testGetTravelType_domestic);
  CPPUNIT_TEST(testGetTravelType_foreignDomestic);

  CPPUNIT_TEST(testRemoveFirst_match);
  CPPUNIT_TEST(testRemoveFirst_noMatch);

  CPPUNIT_TEST_SUITE_END();

private:
  CouponMatcher::TravelSegments* _excSegs;
  CouponMatcher::TravelSegments* _newSegs;
  CouponMatcher* _matcher;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _excSegs = _memHandle.create<CouponMatcher::TravelSegments>();
    _newSegs = _memHandle.create<CouponMatcher::TravelSegments>();
    _matcher = _memHandle.create<CouponMatcher>();
  }

  void tearDown() { _memHandle.clear(); }

  AirSeg* createSegment(bool unflown)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->unflown() = unflown;
    return seg;
  }

  template <int size>
  void populate(const bool (&tab)[size], CouponMatcher::TravelSegments& segs)
  {
    for (int i = 0; i < size; ++i)
      segs += createSegment(tab[i]);
  }

  enum
  {
    FLOWN = 0,
    UNFLOWN = 1
  };

  void testGetFirstUnflownSegment_noSegements()
  {
    // empty _excSegs
    CPPUNIT_ASSERT(_matcher->getFirstUnflownSegment(*_excSegs) == _excSegs->end());
  }

  void testGetFirstUnflownSegment_firstSegment()
  {
    bool status[] = { UNFLOWN, UNFLOWN, UNFLOWN };
    populate(status, *_excSegs);

    CPPUNIT_ASSERT(_matcher->getFirstUnflownSegment(*_excSegs) == _excSegs->begin());
  }

  void testGetFirstUnflownSegment_lastSegment()
  {
    bool status[] = { FLOWN, FLOWN, UNFLOWN };
    populate(status, *_excSegs);

    CPPUNIT_ASSERT(_matcher->getFirstUnflownSegment(*_excSegs) == _excSegs->end() - 1);
  }

  void testGetFirstUnflownSegment_fullyFlown()
  {
    bool status[] = { FLOWN, FLOWN, FLOWN };
    populate(status, *_excSegs);

    CPPUNIT_ASSERT(_matcher->getFirstUnflownSegment(*_excSegs) == _excSegs->end());
  }

  AirSeg* createSegment(const LocCode& board, const LocCode& off)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->origin() = createLoc(board);
    seg->destination() = createLoc(off);
    seg->origAirport() = seg->boardMultiCity() = board;
    seg->destAirport() = seg->offMultiCity() = off;

    seg->geoTravelType() = getTravelType(seg->origin()->nation(), seg->destination()->nation());

    return seg;
  }

  Loc* createLoc(const LocCode& loc)
  {
    CPPUNIT_ASSERT(loc != "");
    Loc* l = _memHandle.create<Loc>();
    l->loc() = loc;
    l->nation() = getNation(loc);
    return l;
  }

  GeoTravelType getTravelType(const NationCode& board, const NationCode& off)
  {
    if ((board == "US" && off == "CA") || (board == "CA" && off == "US"))
      return GeoTravelType::Transborder;

    if (board != off)
      return GeoTravelType::International;

    if ((board == "US") || (board == "CA"))
      return GeoTravelType::Domestic;

    return GeoTravelType::ForeignDomestic;
  }

  void testGetTravelType_transborder()
  {
    CPPUNIT_ASSERT(GeoTravelType::Transborder == getTravelType("US", "CA"));
    CPPUNIT_ASSERT(GeoTravelType::Transborder == getTravelType("CA", "US"));
  }

  void testGetTravelType_international()
  {
    CPPUNIT_ASSERT(GeoTravelType::International == getTravelType("US", "PL"));
    CPPUNIT_ASSERT(GeoTravelType::International == getTravelType("PL", "US"));
    CPPUNIT_ASSERT(GeoTravelType::International == getTravelType("CA", "PL"));
    CPPUNIT_ASSERT(GeoTravelType::International == getTravelType("PL", "CA"));
  }

  void testGetTravelType_domestic()
  {
    CPPUNIT_ASSERT(GeoTravelType::Domestic == getTravelType("US", "US"));
    CPPUNIT_ASSERT(GeoTravelType::Domestic == getTravelType("CA", "CA"));
  }

  void testGetTravelType_foreignDomestic()
  {
    CPPUNIT_ASSERT(GeoTravelType::ForeignDomestic == getTravelType("PL", "PL"));
    CPPUNIT_ASSERT(GeoTravelType::ForeignDomestic == getTravelType("GB", "GB"));
  }

  template <int size>
  bool find(const LocCode& loc, LocCode (&tab)[size])
  {
    return std::find(tab, tab + size, loc) != tab + size;
  }

  NationCode getNation(const LocCode& loc)
  {
    static LocCode us[] = { "NYC", "LAX", "MIA", "CHI", "DFW", "DEN" };
    if (find(loc, us))
      return "US";

    static LocCode gb[] = { "LON", "LHR", "MAN" };
    if (find(loc, gb))
      return "GB";

    static LocCode fr[] = { "PAR", "NCE", "XTU" };
    if (find(loc, fr))
      return "FR";

    static LocCode de[] = { "FRA", "MUC" };
    if (find(loc, de))
      return "DE";

    static LocCode pl[] = { "WAW", "KRK" };
    if (find(loc, pl))
      return "PL";

    static LocCode mx[] = { "MEX" };
    if (find(loc, mx))
      return "MX";

    static LocCode ca[] = { "YVR" };
    if (find(loc, ca))
      return "CA";

    static LocCode jp[] = { "NRT" };
    if (find(loc, jp))
      return "JP";

    CPPUNIT_FAIL(std::string("No nation for " + loc));
    return "";
  }

  template <int size>
  void populate(const LocCode (&tab)[size], CouponMatcher::TravelSegments& segs)
  {
    for (int i = 0; i < size - 1; ++i)
      segs += createSegment(tab[i], tab[i + 1]);
  }

  enum
  {
    FAIL = 0,
    PASS = 1
  };

  template <int size1, int size2>
  void assertMatchUnflown(bool expect,
                          const LocCode (&excJourney)[size1],
                          const LocCode (&newJourney)[size2])
  {
    populate(excJourney, *_excSegs);
    populate(newJourney, *_newSegs);
    CPPUNIT_ASSERT_EQUAL(
        expect,
        _matcher->matchUnflown(
            _excSegs->begin(), _excSegs->end(), _newSegs->begin(), _newSegs->end()));
  }

  void testMatchUnflown_InternationalPass()
  {
    LocCode excJourney[] = { "NYC", "LON" };
    LocCode newJourney[] = { "NYC", "LON" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_InternationalWithAddedSegmentPass()
  {
    LocCode excJourney[] = { "NYC", "LON" };
    LocCode newJourney[] = { "NYC", "LON", "PAR" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticToInternationalFail()
  {
    LocCode excJourney[] = { "FRA", "MUC" };
    LocCode newJourney[] = { "LON", "MUC" };
    assertMatchUnflown(FAIL, excJourney, newJourney);
  }

  void testMatchUnflown_SingleInternationalFlightPass()
  {
    LocCode excJourney[] = { "NYC", "LON" };
    LocCode newJourney[] = { "NYC", "PAR" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_SingleDomesticToSingleInternationFail()
  {
    LocCode excJourney[] = { "NYC", "LAX" };
    LocCode newJourney[] = { "NYC", "LON" };
    assertMatchUnflown(FAIL, excJourney, newJourney);
  }

  void testMatchUnflown_SingleDomesticFlightPass()
  {
    LocCode excJourney[] = { "NYC", "LAX" };
    LocCode newJourney[] = { "NYC", "MIA" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_MatchAllInternationalFlightsPass()
  {
    LocCode excJourney[] = { "CHI", "LHR", "WAW", "LHR" };
    LocCode newJourney[] = { "NYC", "LHR", "KRK", "LHR" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_SingleInternationalToSingleDomesticPass()
  {
    LocCode excJourney[] = { "NYC", "LHR" };
    LocCode newJourney[] = { "CHI", "LAX" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalFlightsNoNeedToMapDomestic1Pass()
  {
    LocCode excJourney[] = { "LAX", "CHI", "LHR", "CHI", "LAX" };
    LocCode newJourney[] = { "LAX", "NYC", "LHR", "CHI", "LAX" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalFlightsNoNeedToMapDomestic2Pass()
  {
    LocCode excJourney[] = { "LAX", "CHI", "LHR", "CHI", "LAX" };
    LocCode newJourney[] = { "LAX", "NYC", "LHR", "LAX" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalFlightsNoNeedToMapDomestic3Pass()
  {
    LocCode excJourney[] = { "LAX", "CHI", "LHR", "CHI", "LAX" };
    LocCode newJourney[] = { "LAX", "LHR", "LAX" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalFlightsMapDomesticFail()
  {
    LocCode excJourney[] = { "LAX", "CHI", "LHR", "CHI", "LAX" };
    LocCode newJourney[] = { "LAX", "LHR", "KRK", "LHR", "LAX" };
    assertMatchUnflown(FAIL, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalFlightsNoNeedToMapDomesticExchangeItinIsLongerPass()
  {
    LocCode excJourney[] = { "LAX", "CHI", "LHR", "PAR", "NCE", "PAR", "LHR", "CHI", "LAX" };
    LocCode newJourney[] = { "LAX", "CHI", "LHR", "PAR", "LHR", "CHI", "LAX" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalFlightsSomeCouponsFlownFail()
  {
    LocCode excJourney[] = { "LAX", "CHI", "LHR", "CHI", "LAX" };
    LocCode newJourney[] = { "LAX", "CHI", "LHR", "CHI", "MEX" };
    assertMatchUnflown(FAIL, excJourney, newJourney);
  }

  void testMatchUnflown_MatchInternationalFlightsToAnyInternationalSectorPass()
  {
    LocCode excJourney[] = { "CHI", "PAR", "WAW" };
    LocCode newJourney[] = { "KRK", "LHR", "NCE" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalFlightsNoMatchForDomesticFail()
  {
    LocCode excJourney[] = { "CHI", "PAR", "NCE" };
    LocCode newJourney[] = { "LHR", "WAW", "KRK", "PAR" };
    assertMatchUnflown(FAIL, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalOnlyOneSectorUnflownFail()
  {
    LocCode excJourney[] = { "LHR", "WAW", "KRK", "WAW" };
    LocCode newJourney[] = { "LHR", "WAW", "KRK", "PAR" };
    assertMatchUnflown(FAIL, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalOnlyOneSectorUnflownPass()
  {
    LocCode excJourney[] = { "LHR", "PAR", "KRK", "NCE" };
    LocCode newJourney[] = { "LHR", "PAR", "KRK", "PAR" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_OneInternationalManyDomesticOnNewPass()
  {
    LocCode excJourney[] = { "LAX", "CHI", "NYC", "LAX" };
    LocCode newJourney[] = { "LAX", "CHI", "NYC", "LAX", "LHR" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticInUsChangedToFlightToCanadaFail()
  {
    LocCode excJourney[] = { "NYC", "LAX" };
    LocCode newJourney[] = { "NYC", "YVR" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticChangedToDomesticPass()
  {
    LocCode excJourney[] = { "PAR", "NCE", "PAR", "LON", "CHI", "LAX" };
    LocCode newJourney[] = { "PAR", "XTU", "NCE", "PAR", "LON", "FRA", "CHI", "LAX" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticChangedToInternationalFail()
  {
    LocCode excJourney[] = { "LON", "CHI", "LAX" };
    LocCode newJourney[] = { "LON", "CHI", "MEX" };
    assertMatchUnflown(FAIL, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticAndInternationalChangedToInternationalPass()
  {
    LocCode excJourney[] = { "DFW", "CHI", "MAN" };
    LocCode newJourney[] = { "DFW", "NRT" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testMatchUnflown_DomesticJira_AAP235Pass()
  {
    LocCode excJourney[] = { "MEX", "DFW", "DEN", "DFW", "MEX" };
    LocCode newJourney[] = { "DEN", "DFW", "MEX", "DFW" };
    assertMatchUnflown(PASS, excJourney, newJourney);
  }

  void testRemoveFirst_match()
  {
    int tab[] = { 5, 4, 3, 2, 1, 0 };
    int* end = tab + 5;
    CPPUNIT_ASSERT(_matcher->removeFirst(tab, end, boost::bind(std::less<int>(), 4, _1)));
    CPPUNIT_ASSERT_EQUAL(tab + 4, end);
  }

  void testRemoveFirst_noMatch()
  {
    int tab[] = { 5, 4, 3, 2, 1, 0 };
    int* end = tab + 5;
    CPPUNIT_ASSERT(!_matcher->removeFirst(tab, end, boost::bind(std::less<int>(), 10, _1)));
    CPPUNIT_ASSERT_EQUAL(tab + 5, end);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CouponMatcherTest);

} // tse
