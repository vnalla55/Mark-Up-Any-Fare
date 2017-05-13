#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/TseCodeTypes.h"
#include "DataModel/ExcItin.h"
#include "ItinAnalyzer/ExcItinUtil.h"
#include "ItinAnalyzer/test/TravelSegmentTestUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;

class ExcItinUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ExcItinUtilTest);

  CPPUNIT_TEST(testDetermineChangesIdentical);
  CPPUNIT_TEST(testDetermineChangesDifferentBoardOff);
  CPPUNIT_TEST(testDetermineChangesDifferentCarrier);
  CPPUNIT_TEST(testDetermineChangesDifferentDate);
  CPPUNIT_TEST(testDetermineChangesDifferentDateTime);
  CPPUNIT_TEST(testDetermineChangesDifferentFlight);
  CPPUNIT_TEST(testDetermineChangesDifferentCabin);
  CPPUNIT_TEST(testDetermineChangesDifferentCabinWhenOpenSeg);
  CPPUNIT_TEST(testDetermineChangesDifferentBkgCode);
  CPPUNIT_TEST(testDetermineChangesDifferentBkgCodeWhenOpenSeg);
  CPPUNIT_TEST(testDetermineChangesArunkSeg);
  CPPUNIT_TEST(testDetermineChangesArunkSegBis);
  CPPUNIT_TEST(testDetermineChangesAddRemoveSeg);

  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToNewOpen);
  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToMultiNewOpen);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToNewOpen);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToMultiNewOpen);
  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToNewOpenDifferentCarrier);
  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToMultiNewOpenDifferentCarrier);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToNewOpenDifferentCarrier);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToMultiNewOpenDifferentCarrier);
  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToNewOpenDifferentBkgCode);
  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToMultiNewOpenDifferentBkgCode);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToNewOpenDifferentBkgCode);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToMultiNewOpenDifferentBkgCode);

  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToNewOpenDifferentDepDateAndBkgCode);
  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToMultiNewOpenDifferentDepDateAndBkgCode);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToNewOpenDifferentDepDateAndBkgCode);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToMultiNewOpenDifferentDepDateAndBkgCode);

  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToNewOpenDifferentDifferentDepTime);
  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToNewOpenDifferentDifferentDepDate);

  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToArunk);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToMultiArunk);
  CPPUNIT_TEST(testDetermineChangesMatchMultiExcOpenToArunkAndAir);
  CPPUNIT_TEST(testDetermineChangesMatchExcOpenToNewAir);
  CPPUNIT_TEST(testDetermineChangesMatchExcAirToNewOpen);
  CPPUNIT_TEST(testDetermineChangesMatchDuplicateSegments);

  CPPUNIT_TEST(testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedTrue);
  CPPUNIT_TEST(testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneChanged);
  CPPUNIT_TEST(testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneDeleted);
  CPPUNIT_TEST(testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneNewSegmentAdded);
  CPPUNIT_TEST(testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneOpenConfirmed);
  CPPUNIT_TEST(
      testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneInventoryChangedOpenSegments);
  CPPUNIT_TEST(
      testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneInventoryChangedAirSegments);

  CPPUNIT_TEST_SUITE_END();

private:
  ExcItin* _excItin;
  Itin* _newItin;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    // Ensure using old cabin values to avoid conflicts with AllTestSuites
    _excItin = _memHandle.create<ExcItin>();
    _excItin->setItinIndex(0);
    _newItin = _memHandle.create<Itin>();
  }

  void tearDown()
  {
    releaseTravelSegments(_excItin->travelSeg());
    releaseTravelSegments(_newItin->travelSeg());
    _memHandle.clear();
  }

  void testDetermineChangesIdentical()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createArunkSegment("KTW", "KRK"), createAirSegment("FRA", "KTW", "LH", "2007-12-21"),
        createOpenSegment("WAW", "MOW", "LO", "2007-12-30"),
        createOpenSegment("MOW", "WAW", "LO", "2007-12-30");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createArunkSegment("KTW", "KRK"), createAirSegment("FRA", "KTW", "LH", "2007-12-21"),
        createOpenSegment("WAW", "MOW", "LO", "2007-12-30"),
        createOpenSegment("MOW", "WAW", "LO", "2007-12-30"),
        ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(5, TravelSeg::UNCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesDifferentBoardOff()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-01-20");

    _newItin->travelSeg() += createAirSegment("DFW", "PAR", "AA", "2007-01-20");
    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesDifferentCarrier()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-01-20");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "LH", "2007-01-20");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesDifferentBkgCode()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 70, "Y"),
        createAirSegment("WAW", "KRK", "LO", "2007-04-04", "11:11", 50, "I");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 70, "V"),
        createAirSegment("WAW", "KRK", "LO", "2007-04-04", "11:11", 50, "I");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect;
    expect += TravelSeg::INVENTORYCHANGED, TravelSeg::UNCHANGED;

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesDifferentBkgCodeWhenOpenSeg()
  {
    _excItin->travelSeg() += createOpenSegment("KTW", "FRA", "LH", "2007-12-26", "10:15", 80, "Y"),
        createAirSegment("DFW", "NYC", "AA", "2007-12-20", "08:25", 70, "V"),
        createOpenSegment("FRA", "KRK", "LO", "2007-02-20", "11:12", 70, "H");

    _newItin->travelSeg() += createAirSegment("KTW", "FRA", "LH", "2007-12-26", "10:15", 80, "V"),
        createAirSegment("FRA", "KRK", "LO", "2007-02-20", "11:12", 70, "H");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc, expect_new;
    expect_exc += TravelSeg::INVENTORYCHANGED, TravelSeg::CHANGED, TravelSeg::CONFIRMOPENSEGMENT;
    expect_new += TravelSeg::INVENTORYCHANGED, TravelSeg::CONFIRMOPENSEGMENT;

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesDifferentDate()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-22");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesDifferentDateTime()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "09:22");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::UNCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesDifferentCabin()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 70, "Y");

    _newItin->travelSeg() += createAirSegment(
        "DFW", "FRA", "AA", "2007-12-20", "08:25", 70, "X", CabinType::BUSINESS_CLASS);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesDifferentCabinWhenOpenSeg()
  {
    _excItin->travelSeg() += createOpenSegment(
        "KTW", "FRA", "LH", "2007-12-25", "11:11", 80, BookingCode('X'), CabinType::FIRST_CLASS);

    _newItin->travelSeg() += createAirSegment(
        "KTW", "FRA", "LH", "2007-12-26", "11:11", 80, BookingCode('Y'), CabinType::BUSINESS_CLASS);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
  }

  void testDetermineChangesDifferentFlight()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 70);

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesArunkSeg()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "LH", "2007-12-20"),
        createArunkSegment("FRA", "BER"), createAirSegment("BER", "WAW", "LH", "2007-12-22");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "LH", "2007-12-20"),
        createArunkSegment("FRA", "MUC"), createAirSegment("MUC", "WAW", "LH", "2007-12-23");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect;
    expect += TravelSeg::UNCHANGED, TravelSeg::CHANGED, TravelSeg::CHANGED;

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesArunkSegBis()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "LH", "2007-12-20"),
        createArunkSegment("FRA", "BER"), createAirSegment("BER", "WAW", "LH", "2007-12-22");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "LH", "2007-12-20"),
        createAirSegment("FRA", "BER", "LH", "2007-12-21"),
        createAirSegment("BER", "WAW", "LH", "2007-12-22");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect;
    expect += TravelSeg::UNCHANGED, TravelSeg::CHANGED, TravelSeg::UNCHANGED;

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesAddRemoveSeg()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createAirSegment("FRA", "KTW", "LH", "2007-12-21"), createArunkSegment("KTW", "WAW"),
        createOpenSegment("WAW", "MOW", "LO", "2007-12-30"),
        createOpenSegment("MOW", "KRK", "LO", "2007-12-30");

    _newItin->travelSeg() += createAirSegment("LAX", "DFW", "AA", "2007-12-18"),
        createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createAirSegment("FRA", "KTW", "LH", "2007-12-21"), createArunkSegment("KTW", "WAW"),
        createOpenSegment("WAW", "KRK", "LO", "2007-12-30"),
        createAirSegment("KTW", "GDN", "LO", "2007-12-25"),
        createAirSegment("GDN", "KTW", "LO", "2007-12-27");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc, expect_new;
    expect_exc += TravelSeg::UNCHANGED, TravelSeg::UNCHANGED, TravelSeg::UNCHANGED,
        TravelSeg::CONFIRMOPENSEGMENT, TravelSeg::CONFIRMOPENSEGMENT;
    expect_new += TravelSeg::CHANGED, TravelSeg::UNCHANGED, TravelSeg::UNCHANGED,
        TravelSeg::UNCHANGED, TravelSeg::CONFIRMOPENSEGMENT, TravelSeg::CHANGED, TravelSeg::CHANGED;

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToNewOpen()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80);

    _newItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::UNCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToMultiNewOpen()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80);

    _newItin->travelSeg() += createOpenSegment("DFW", "KRK", "AA", "2007-12-20", "08:25", 80),
        createOpenSegment("KRK", "FRA", "AA", "2007-12-20", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc(1, TravelSeg::CONFIRMOPENSEGMENT),
        expect_new(2, TravelSeg::CONFIRMOPENSEGMENT);

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToNewOpen()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80),
        createOpenSegment("FRA", "KRK", "AA", "2007-12-20", "08:25", 80);

    _newItin->travelSeg() += createOpenSegment("DFW", "KRK", "AA", "2007-12-20", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc(2, TravelSeg::CONFIRMOPENSEGMENT),
        expect_new(1, TravelSeg::CONFIRMOPENSEGMENT);

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToMultiNewOpen()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80),
        createOpenSegment("FRA", "KRK", "AA", "2007-12-20", "08:25", 80);

    _newItin->travelSeg() += createOpenSegment("DFW", "MOW", "AA", "2007-12-20", "08:25", 80),
        createOpenSegment("MOW", "KRK", "AA", "2007-12-20", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(2, TravelSeg::CONFIRMOPENSEGMENT);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToNewOpenDifferentCarrier()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80);

    _newItin->travelSeg() += createOpenSegment("DFW", "FRA", "LH", "2007-12-20", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToMultiNewOpenDifferentCarrier()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80);

    _newItin->travelSeg() += createOpenSegment("DFW", "KRK", "AA", "2007-12-20", "08:25", 80),
        createOpenSegment("KRK", "FRA", "LH", "2007-12-20", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc(1, TravelSeg::CHANGED),
        expect_new(2, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToNewOpenDifferentCarrier()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "LH", "2007-12-20", "08:25", 80),
        createOpenSegment("FRA", "KRK", "AA", "2007-12-20", "08:25", 80);

    _newItin->travelSeg() += createOpenSegment("DFW", "KRK", "AA", "2007-12-20", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc(2, TravelSeg::CHANGED),
        expect_new(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToMultiNewOpenDifferentCarrier()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80),
        createOpenSegment("FRA", "KRK", "AA", "2007-12-20", "08:25", 80);

    _newItin->travelSeg() += createOpenSegment("DFW", "MOW", "LH", "2007-12-20", "08:25", 80),
        createOpenSegment("MOW", "KRK", "LH", "2007-12-20", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(2, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToNewOpenDifferentBkgCode()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    _newItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "N");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::INVENTORYCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToMultiNewOpenDifferentBkgCode()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    _newItin->travelSeg() += createOpenSegment("DFW", "KRK", "AA", "2007-12-20", "08:25", 80, "Y"),
        createOpenSegment("KRK", "FRA", "AA", "2007-12-20", "08:25", 80, "N");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc(1, TravelSeg::INVENTORYCHANGED),
        expect_new(2, TravelSeg::INVENTORYCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToNewOpenDifferentBkgCode()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "N"),
        createOpenSegment("FRA", "KRK", "AA", "2007-12-20", "08:25", 80, "Y");

    _newItin->travelSeg() += createOpenSegment("DFW", "KRK", "AA", "2007-12-20", "08:25", 80, "Y");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc(2, TravelSeg::INVENTORYCHANGED),
        expect_new(1, TravelSeg::INVENTORYCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToMultiNewOpenDifferentBkgCode()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y"),
        createOpenSegment("FRA", "KRK", "AA", "2007-12-20", "08:25", 80, "N");

    _newItin->travelSeg() += createOpenSegment("DFW", "MOW", "AA", "2007-12-20", "08:25", 80, "N"),
        createOpenSegment("MOW", "KRK", "AA", "2007-12-20", "08:25", 80, "Y");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(2, TravelSeg::INVENTORYCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToNewOpenDifferentDepDateAndBkgCode()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-10", "08:25", 80, "Y");

    _newItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "N");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::INVENTORYCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToMultiNewOpenDifferentDepDateAndBkgCode()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-10", "08:25", 80, "Y");

    _newItin->travelSeg() += createOpenSegment("DFW", "KRK", "AA", "2007-12-20", "08:25", 80, "N"),
        createOpenSegment("KRK", "FRA", "AA", "2007-12-20", "08:25", 80, "N");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc(1, TravelSeg::INVENTORYCHANGED),
        expect_new(2, TravelSeg::INVENTORYCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToNewOpenDifferentDepDateAndBkgCode()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y"),
        createOpenSegment("FRA", "KRK", "AA", "2007-12-10", "08:25", 80, "N");

    _newItin->travelSeg() += createOpenSegment("DFW", "KRK", "AA", "2007-12-20", "08:25", 80, "Y");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc(2, TravelSeg::INVENTORYCHANGED),
        expect_new(1, TravelSeg::INVENTORYCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToMultiNewOpenDifferentDepDateAndBkgCode()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y"),
        createOpenSegment("FRA", "KRK", "AA", "2007-12-20", "08:25", 80, "N");

    _newItin->travelSeg() += createOpenSegment("DFW", "MOW", "AA", "2007-12-20", "08:25", 80, "N"),
        createOpenSegment("MOW", "KRK", "AA", "2007-12-10", "08:25", 80, "N");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(2, TravelSeg::INVENTORYCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToNewOpenDifferentDifferentDepTime()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    _newItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "10:25", 80, "Y");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::UNCHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToNewOpenDifferentDifferentDepDate()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    _newItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-10", "08:25", 80, "Y");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CONFIRMOPENSEGMENT);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToArunk()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");
    _newItin->travelSeg() += createArunkSegment("DFW", "FRA");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToArunkAndAir()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "MOW", "AA", "2007-12-20", "08:25", 80, "Y"),
        createOpenSegment("MOW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    _newItin->travelSeg() += createArunkSegment("DFW", "KTW"),
        createOpenSegment("KTW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(2, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchMultiExcOpenToMultiArunk()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "MOW", "AA", "2007-12-20", "08:25", 80, "Y"),
        createOpenSegment("MOW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    _newItin->travelSeg() += createArunkSegment("DFW", "KTW"), createArunkSegment("KTW", "FRA");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(2, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcOpenToNewAir()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CONFIRMOPENSEGMENT);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchExcAirToNewOpen()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    _newItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect(1, TravelSeg::CHANGED);

    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  void testDetermineChangesMatchDuplicateSegments()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y"),
        createOpenSegment("DFW", "FRA", "AA", "2007-12-21", "08:10", 70, "N"),
        createOpenSegment("FRA", "KRK", "LH", "2007-12-21", "08:10", 70, "N"),
        createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y"),
        createAirSegment("DFW", "FRA", "AA", "2007-12-10", "08:25", 70, "Y"),
        createAirSegment("DFW", "FRA", "AA", "2007-12-10", "08:25", 80, "Y"),
        createOpenSegment("DFW", "FRA", "LH", "2007-12-21", "08:10", 70, "N"),
        createOpenSegment("KTW", "FRA", "LH", "2007-12-21", "08:10", 80, "N"),
        createAirSegment("PAR", "ROM", "LH", "2007-12-21", "08:10", 70, "N"),
        createOpenSegment("WAW", "MOW", "LH", "2007-12-21", "08:10", 70, "Y"),
        createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y"),
        createOpenSegment("GLA", "MOW", "LH", "2007-12-21", "08:10", 70, "N");

    _newItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y"),
        createOpenSegment("FRA", "DFW", "LH", "2007-12-21", "08:10", 70, "N"),
        createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y"),
        createAirSegment("KTW", "FRA", "LH", "2007-12-21", "08:10", 70, "N"),
        createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "N"),
        createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y"),
        createAirSegment("DFW", "FRA", "AA", "2007-12-10", "08:25", 80, "Y"),
        createOpenSegment("DFW", "KTW", "LH", "2007-12-21", "08:10", 70, "N"),
        createOpenSegment("DFW", "KTW", "LH", "2007-12-21", "08:10", 70, "N"),
        createOpenSegment("KTW", "FRA", "LH", "2007-12-21", "08:10", 70, "Y"),
        createOpenSegment("PAR", "WAW", "LH", "2007-12-21", "08:10", 70, "N"),
        createOpenSegment("WAW", "MOW", "LH", "2007-12-21", "08:10", 70, "Y"),
        createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);

    std::vector<TravelSeg::ChangeStatus> expect_exc, expect_new;

    expect_exc += TravelSeg::UNCHANGED, TravelSeg::CONFIRMOPENSEGMENT, TravelSeg::CHANGED,
        TravelSeg::UNCHANGED, TravelSeg::CHANGED, TravelSeg::UNCHANGED, TravelSeg::INVENTORYCHANGED,
        TravelSeg::CHANGED, TravelSeg::CHANGED, TravelSeg::UNCHANGED, TravelSeg::UNCHANGED,
        TravelSeg::CHANGED;

    expect_new += TravelSeg::CHANGED, TravelSeg::CHANGED, TravelSeg::UNCHANGED, TravelSeg::CHANGED,
        TravelSeg::CONFIRMOPENSEGMENT, TravelSeg::UNCHANGED, TravelSeg::UNCHANGED,
        TravelSeg::CHANGED, TravelSeg::INVENTORYCHANGED, TravelSeg::INVENTORYCHANGED,
        TravelSeg::CHANGED, TravelSeg::UNCHANGED, TravelSeg::UNCHANGED;

    CPPUNIT_ASSERT_EQUAL(expect_exc, getTravelSegmentsStatus(_excItin->travelSeg()));
    CPPUNIT_ASSERT_EQUAL(expect_new, getTravelSegmentsStatus(_newItin->travelSeg()));
  }

  enum
  {
    FLOWN,
    UNFLOWN
  };

  void testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedTrue()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createArunkSegment("KTW", "KRK"), createAirSegment("FRA", "KTW", "LH", "2007-12-21"),
        createOpenSegment("WAW", "MOW", "LO", "2007-12-30"),
        createOpenSegment("MOW", "WAW", "LO", "2007-12-30");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createArunkSegment("KTW", "KRK"), createAirSegment("FRA", "KTW", "LH", "2007-12-21"),
        createOpenSegment("WAW", "MOW", "LO", "2007-12-30"),
        createOpenSegment("MOW", "WAW", "LO", "2007-12-30");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);
    ExcItinUtil::CheckSegmentsStatus(_excItin, _newItin);

    CPPUNIT_ASSERT(_excItin->allSegmentsUnchangedOrConfirmed());
    CPPUNIT_ASSERT(!_excItin->someSegmentsChanged());
    CPPUNIT_ASSERT(!_excItin->sameSegmentsInventoryChanged());
    CPPUNIT_ASSERT(!_excItin->someSegmentsConfirmed());
  }

  void testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneChanged()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createArunkSegment("KTW", "KRK"), createAirSegment("FRA", "KTW", "LH", "2007-12-21"),
        createOpenSegment("WAW", "MOW", "LO", "2007-12-30"),
        createOpenSegment("MOW", "WAW", "LO", "2007-12-30");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createArunkSegment("KTW", "KRK"), createAirSegment("FRA", "KTW", "LH", "2007-12-21"),
        createOpenSegment("WAW", "MOW", "LO", "2007-12-30"),
        createOpenSegment("MOW", "KRK", "LH", "2007-12-30");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);
    ExcItinUtil::CheckSegmentsStatus(_excItin, _newItin);

    CPPUNIT_ASSERT(!_excItin->allSegmentsUnchangedOrConfirmed());
    CPPUNIT_ASSERT(_excItin->someSegmentsChanged());
    CPPUNIT_ASSERT(!_excItin->sameSegmentsInventoryChanged());
    CPPUNIT_ASSERT(!_excItin->someSegmentsConfirmed());
  }

  void testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneDeleted()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createArunkSegment("KTW", "KRK"), createAirSegment("FRA", "KTW", "LH", "2007-12-21"),
        createOpenSegment("WAW", "MOW", "LO", "2007-12-30"),
        createOpenSegment("MOW", "WAW", "LO", "2007-12-30");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createArunkSegment("KTW", "KRK"), createAirSegment("FRA", "KTW", "LH", "2007-12-21"),
        createOpenSegment("WAW", "MOW", "LO", "2007-12-30");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);
    ExcItinUtil::CheckSegmentsStatus(_excItin, _newItin);

    CPPUNIT_ASSERT(!_excItin->allSegmentsUnchangedOrConfirmed());
    CPPUNIT_ASSERT(_excItin->someSegmentsChanged());
    CPPUNIT_ASSERT(!_excItin->sameSegmentsInventoryChanged());
    CPPUNIT_ASSERT(!_excItin->someSegmentsConfirmed());
  }

  void testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneNewSegmentAdded()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20"),
        createAirSegment("FRA", "KTW", "AA", "2007-12-20");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);
    ExcItinUtil::CheckSegmentsStatus(_excItin, _newItin);

    CPPUNIT_ASSERT(!_excItin->allSegmentsUnchangedOrConfirmed());
    CPPUNIT_ASSERT(_excItin->someSegmentsChanged());
    CPPUNIT_ASSERT(!_excItin->sameSegmentsInventoryChanged());
    CPPUNIT_ASSERT(!_excItin->someSegmentsConfirmed());
  }

  void testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneOpenConfirmed()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80);

    _newItin->travelSeg() += createOpenSegment("DFW", "KRK", "AA", "2007-12-20", "08:25", 80),
        createOpenSegment("KRK", "FRA", "AA", "2007-12-21", "08:25", 80);

    ExcItinUtil::DetermineChanges(_excItin, _newItin);
    ExcItinUtil::CheckSegmentsStatus(_excItin, _newItin);

    CPPUNIT_ASSERT(_excItin->allSegmentsUnchangedOrConfirmed());
    CPPUNIT_ASSERT(!_excItin->someSegmentsChanged());
    CPPUNIT_ASSERT(!_excItin->sameSegmentsInventoryChanged());
    CPPUNIT_ASSERT(_excItin->someSegmentsConfirmed());
  }

  void
  testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneInventoryChangedOpenSegments()
  {
    _excItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "Y");
    _newItin->travelSeg() += createOpenSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 80, "N");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);
    ExcItinUtil::CheckSegmentsStatus(_excItin, _newItin);

    CPPUNIT_ASSERT(!_excItin->allSegmentsUnchangedOrConfirmed());
    CPPUNIT_ASSERT(!_excItin->someSegmentsChanged());
    CPPUNIT_ASSERT(_excItin->sameSegmentsInventoryChanged());
    CPPUNIT_ASSERT(!_excItin->someSegmentsConfirmed());
  }

  void testCheckSegmentsStatusCheckIfAllSegmentsUnchangedOrConfirmedOneInventoryChangedAirSegments()
  {
    _excItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 70, "Y"),
        createAirSegment("WAW", "KRK", "LO", "2007-04-04", "11:11", 50, "I");

    _newItin->travelSeg() += createAirSegment("DFW", "FRA", "AA", "2007-12-20", "08:25", 70, "V"),
        createAirSegment("WAW", "KRK", "LO", "2007-04-04", "11:11", 50, "I");

    ExcItinUtil::DetermineChanges(_excItin, _newItin);
    ExcItinUtil::CheckSegmentsStatus(_excItin, _newItin);

    CPPUNIT_ASSERT(!_excItin->allSegmentsUnchangedOrConfirmed());
    CPPUNIT_ASSERT(!_excItin->someSegmentsChanged());
    CPPUNIT_ASSERT(_excItin->sameSegmentsInventoryChanged());
    CPPUNIT_ASSERT(!_excItin->someSegmentsConfirmed());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExcItinUtilTest);

} // tse
