#include "Common/NoPNRTravelSegmentTimeUpdater.h"

#include <string>
#include <time.h>
#include <iostream>
#include <vector>
#include <unistd.h>

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseCodeTypes.h"
#include "test/include/MockTseServer.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestXMLHelper.h"

#include "DataModel/NoPNRPricingTrx.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"

// some extension to CPPUNIT ASSERS

#define ASSERT_DONT_THROW_YET(X)                                                                   \
  try { CPPUNIT_ASSERT(X); }                                                                       \
  catch (...) { TOTAL_ERRORS << "l. " << __LINE__ << ": " << #X << std::endl; }

#define LATE_ASSERT std::ostringstream TOTAL_ERRORS;

#define END_LATE_ASSERT                                                                            \
  if (!TOTAL_ERRORS.str().empty())                                                                 \
    CPPUNIT_FAIL(std::string("FAILED TEST CONDITIONS: \n") + TOTAL_ERRORS.str());

using namespace std;

namespace tse
{

class NoPNRTravelSegmentTimeUpdaterTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(NoPNRTravelSegmentTimeUpdaterTest);

  CPPUNIT_TEST(test_construction);
  CPPUNIT_TEST(test_updateSegmentDates_all_open);
  CPPUNIT_TEST(test_updateSegmentDates_first_closed);
  CPPUNIT_TEST(test_updateSegmentDates_last_open);
  CPPUNIT_TEST(test_update_segments_no_time_travels);

  CPPUNIT_TEST_SUITE_END();

  void test_construction()
  {
    NoPNRPricingTrx trx;
    PricingUnit pu;
    NoPNRTravelSegmentTimeUpdater theUpdater = NoPNRTravelSegmentTimeUpdater(pu, trx);

    // should be marked as NoPNR trx
    CPPUNIT_ASSERT(theUpdater._wqTrx);
    // travel segments should be pointed to
    CPPUNIT_ASSERT(theUpdater._travelSegmentsVector == &pu.travelSeg());
    // trx object should also be pointed to
    CPPUNIT_ASSERT(&trx == theUpdater._trx);
    // _alreadyProcessed shouldn't be set yet
    CPPUNIT_ASSERT(!theUpdater._alreadyProcessed);

    NoPNRTravelSegmentTimeUpdater theOtherUpdater =
        NoPNRTravelSegmentTimeUpdater(trx.travelSeg(), trx);
    // should be marked as NoPNR trx
    CPPUNIT_ASSERT(theOtherUpdater._wqTrx);
    // travel segments should be pointed to
    CPPUNIT_ASSERT(theOtherUpdater._travelSegmentsVector == &trx.travelSeg());
    // trx object should also be pointed to
    CPPUNIT_ASSERT(&trx == theOtherUpdater._trx);
    // _alreadyProcessed shouldn't be set yet
    CPPUNIT_ASSERT(!theOtherUpdater._alreadyProcessed);

    PricingTrx somePricingTrx;

    NoPNRTravelSegmentTimeUpdater thirdUpdater =
        NoPNRTravelSegmentTimeUpdater(somePricingTrx.travelSeg(), somePricingTrx);
    // sholdn't be marked as NoPNR trx
    CPPUNIT_ASSERT(!thirdUpdater._wqTrx);

    AirSeg theSeg;
    DateTime dummyDt;
    theSeg.hasEmptyDate() = true;

    thirdUpdater.updateOpenDateIfNeccesary(&theSeg, dummyDt);
    // shouldn't be processed at all - this isn't NoPNR trx
    CPPUNIT_ASSERT(!thirdUpdater._alreadyProcessed);
  }

  void test_updateSegmentDates_all_open()
  {
    LATE_ASSERT;

    NoPNRPricingTrx trx;
    std::vector<TravelSeg*> segVec;

    NoPNRTravelSegmentTimeUpdater theUpdater(segVec, trx);

    AirSeg seg1, seg2, seg3;

    // first - try 'all segment dates empty' case

    trx.itin().push_back(new Itin());
    trx.itin().front()->dateType() = Itin::NoDate;

    seg1.hasEmptyDate() = true;
    seg2.hasEmptyDate() = true;
    seg3.hasEmptyDate() = true;

    seg1.departureDT() = seg1.arrivalDT() = DateTime(2000, 10, 10);
    seg1.bookingDT() = DateTime(2000, 11, 10);
    seg2.departureDT() = seg2.arrivalDT() = DateTime(2000, 10, 10);
    seg2.bookingDT() = DateTime(2000, 11, 10);
    seg3.departureDT() = seg3.arrivalDT() = DateTime(2000, 10, 10);
    seg3.bookingDT() = DateTime(2000, 11, 10);

    // fill the vector
    segVec.push_back(&seg1);
    segVec.push_back(&seg2);
    segVec.push_back(&seg3);

    // not processed yet
    ASSERT_DONT_THROW_YET(theUpdater._updatedDates.size() == 0);

    theUpdater.updateSegmentDates(*theUpdater._travelSegmentsVector);
    theUpdater._alreadyProcessed = true;

    // all segments should be updated
    // all segments are open - all should be updated
    ASSERT_DONT_THROW_YET(theUpdater._updatedDates.size() == 3);

    DateTime compareDate = DateTime(2000, 11, 10);
    DateTime updatedDate = DateTime(1935, 8, 1);

    theUpdater.findAndUpdateSegment(&seg1, updatedDate);
    ASSERT_DONT_THROW_YET(compareDate == updatedDate);

    compareDate = compareDate.addDays(1);
    updatedDate = DateTime(1935, 8, 1);
    theUpdater.findAndUpdateSegment(&seg2, updatedDate);
    ASSERT_DONT_THROW_YET(compareDate == updatedDate);

    compareDate = compareDate.addDays(1);
    updatedDate = DateTime(1935, 8, 1);
    theUpdater.findAndUpdateSegment(&seg3, updatedDate);
    ASSERT_DONT_THROW_YET(compareDate == updatedDate);

    END_LATE_ASSERT;
  }

  void test_updateSegmentDates_first_closed()
  {
    LATE_ASSERT;

    NoPNRPricingTrx trx;
    std::vector<TravelSeg*> segVec;

    NoPNRTravelSegmentTimeUpdater theUpdater(segVec, trx);

    AirSeg seg1, seg2, seg3;

    // first - try 'all segment dates empty' case

    trx.itin().push_back(new Itin());
    trx.itin().front()->dateType() = Itin::PartialDate;

    seg1.hasEmptyDate() = false; // date specified
    seg2.hasEmptyDate() = true;
    seg3.hasEmptyDate() = true;

    seg1.departureDT() = seg1.arrivalDT() = DateTime(2000, 10, 10);
    seg2.departureDT() = seg2.arrivalDT() = DateTime(2000, 1, 1);
    seg3.departureDT() = seg3.arrivalDT() = DateTime(2000, 1, 1);

    // fill the vector
    segVec.push_back(&seg1);
    segVec.push_back(&seg2);
    segVec.push_back(&seg3);

    ASSERT_DONT_THROW_YET(theUpdater._updatedDates.size() == 0);

    theUpdater.updateSegmentDates(*theUpdater._travelSegmentsVector);
    theUpdater._alreadyProcessed = true;

    // 2 segments should be updated
    ASSERT_DONT_THROW_YET(theUpdater._updatedDates.size() == 2);

    DateTime updatedDate = DateTime(1935, 8, 1);

    theUpdater.findAndUpdateSegment(&seg1, updatedDate);
    ASSERT_DONT_THROW_YET(updatedDate == DateTime(1935, 8, 1)); // shouldn't be changed !

    DateTime compareDate = DateTime(2000, 10, 10).addDays(1);
    updatedDate = DateTime(1935, 8, 1);
    theUpdater.findAndUpdateSegment(&seg2, updatedDate);
    ASSERT_DONT_THROW_YET(compareDate == updatedDate);

    compareDate = compareDate.addDays(1);
    updatedDate = DateTime(1935, 8, 1);
    theUpdater.findAndUpdateSegment(&seg3, updatedDate);
    ASSERT_DONT_THROW_YET(compareDate == updatedDate);

    END_LATE_ASSERT;
  }

  void test_updateSegmentDates_last_open()
  {
    LATE_ASSERT;

    NoPNRPricingTrx trx;
    std::vector<TravelSeg*> segVec;
    NoPNRTravelSegmentTimeUpdater theUpdater(segVec, trx);
    AirSeg seg1, seg2, seg3;

    trx.itin().push_back(new Itin());
    trx.itin().front()->dateType() = Itin::PartialDate;

    seg1.hasEmptyDate() = false; // date specified
    seg2.hasEmptyDate() = false;
    seg3.hasEmptyDate() = true;

    seg1.departureDT() = seg1.arrivalDT() = DateTime(2000, 10, 10);
    seg2.departureDT() = seg2.arrivalDT() = DateTime(2005, 10, 10);
    seg3.departureDT() = seg3.arrivalDT() = DateTime(2000, 1, 1);

    // fill the vector
    segVec.push_back(&seg1);
    segVec.push_back(&seg2);
    segVec.push_back(&seg3);

    ASSERT_DONT_THROW_YET(theUpdater._updatedDates.size() == 0);

    theUpdater.updateSegmentDates(*theUpdater._travelSegmentsVector);
    theUpdater._alreadyProcessed = true;

    ASSERT_DONT_THROW_YET(theUpdater._updatedDates.size() == 1);

    DateTime updatedDate = DateTime(1935, 8, 1);
    theUpdater.findAndUpdateSegment(&seg1, updatedDate);
    ASSERT_DONT_THROW_YET(updatedDate == DateTime(1935, 8, 1)); // shouldn't be changed !

    theUpdater.findAndUpdateSegment(&seg2, updatedDate);
    ASSERT_DONT_THROW_YET(updatedDate == DateTime(1935, 8, 1)); // shouldn't be changed !

    DateTime compareDate = seg2.departureDT().addDays(1);
    updatedDate = DateTime(1935, 8, 1);
    theUpdater.findAndUpdateSegment(&seg3, updatedDate);
    ASSERT_DONT_THROW_YET(compareDate == updatedDate);

    END_LATE_ASSERT;
  }

  void test_update_segments_no_time_travels()
  {
    LATE_ASSERT;

    NoPNRPricingTrx trx;
    std::vector<TravelSeg*> segVec;
    NoPNRTravelSegmentTimeUpdater theUpdater(segVec, trx);
    AirSeg seg1, seg2, seg3, seg4;

    trx.itin().push_back(new Itin());
    trx.itin().front()->dateType() = Itin::PartialDate;

    seg1.hasEmptyDate() = false;
    seg2.hasEmptyDate() = true;
    seg3.hasEmptyDate() = true;
    seg4.hasEmptyDate() = false;

    seg1.departureDT() = DateTime(2000, 1, 1);
    seg4.departureDT() = DateTime(2000, 1, 2);
    seg2.departureDT() = DateTime(1935, 10, 10);
    seg3.departureDT() = DateTime(1935, 10, 10);

    // fill the vector
    segVec.push_back(&seg1);
    segVec.push_back(&seg2);
    segVec.push_back(&seg3);
    segVec.push_back(&seg4);

    theUpdater.updateSegmentDates(*theUpdater._travelSegmentsVector);
    theUpdater._alreadyProcessed = true;

    // all segments should be updated
    ASSERT_DONT_THROW_YET(theUpdater._updatedDates.size() == 2);

    DateTime updatedDate = DateTime(1935, 8, 1);
    theUpdater.findAndUpdateSegment(&seg2, updatedDate);
    ASSERT_DONT_THROW_YET(updatedDate == seg1.departureDT().addDays(1));

    updatedDate = DateTime(1935, 8, 1);
    theUpdater.findAndUpdateSegment(&seg3, updatedDate);
    ASSERT_DONT_THROW_YET(updatedDate == seg1.departureDT().addDays(1));

    seg1.hasEmptyDate() = false;
    seg2.hasEmptyDate() = true;
    seg3.hasEmptyDate() = false;

    seg1.departureDT() = DateTime(2000, 1, 1);
    seg2.departureDT() = DateTime(1935, 10, 10);
    seg3.departureDT() = seg1.departureDT();

    // fill the vector
    segVec.clear();
    segVec.push_back(&seg1);
    segVec.push_back(&seg2);
    segVec.push_back(&seg3);

    theUpdater = NoPNRTravelSegmentTimeUpdater(segVec, trx);
    theUpdater.updateSegmentDates(*theUpdater._travelSegmentsVector);
    theUpdater._alreadyProcessed = true;

    ASSERT_DONT_THROW_YET(theUpdater._updatedDates.size() == 1);

    theUpdater.findAndUpdateSegment(&seg2, updatedDate);

    ASSERT_DONT_THROW_YET(updatedDate == seg1.departureDT() && updatedDate == seg3.departureDT());

    END_LATE_ASSERT;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(NoPNRTravelSegmentTimeUpdaterTest);

} // namespace tse

#undef ASSERT_DONT_THROW_YET
#undef LATE_ASSERT
#undef END_LATE_ASSERT
