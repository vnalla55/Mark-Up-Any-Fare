#include "test/include/CppUnitHelperMacros.h"
#include "Routing/IsInLoc.h"
#include "DBAccess/TpdPsrViaGeoLoc.h"
#include "Routing/MileageRouteItem.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using namespace std;

// note that the target class file of this test is
// defined in TicketedPointDeduction.h/.cpp
class IsInLocTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IsInLocTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testOperatorReturnTrueWhen_Direct_Service_Req_Loc1);
  CPPUNIT_TEST(testOperatorReturnFalseWhen_Direct_Service_Req_Loc1_and_stopoverNotAllowed);
  CPPUNIT_TEST(testOpeartorReturnTrueWhen_Direct_Service_And_NoStop_Loc1);
  CPPUNIT_TEST(testOpeartorReturnFalseWhen_Direct_Service_And_NoStop_Loc1);
  CPPUNIT_TEST(testOpeartorReturnTrueWhen_NoStop_Loc1);
  CPPUNIT_TEST(testOpeartorReturnFalseWhen_NoStop_Loc1_Fail_stopover);
  CPPUNIT_TEST(testOpeartorReturnFalseWhen_NoStop_Loc1_Fail_occurrence);
  CPPUNIT_TEST(testOpeartorReturnFalseWhen_NoStop_Loc1_Fail_occurrence_origLoc2);
  CPPUNIT_TEST(testOpeartorReturnTrueWhen_Direct_Service_Req_Loc2);
  CPPUNIT_TEST(testOpeartorReturnFalseWhen_Direct_Service_Req_Loc2_and_stopoverNotAllowed);
  CPPUNIT_TEST(testOpeartorReturnTrueWhen_Direct_Service_And_NoStop_Loc2);
  CPPUNIT_TEST(testOpeartorReturnFalseWhen_Direct_Service_And_NoStop_Loc2);
  CPPUNIT_TEST(testOpeartorReturnTrueWhen_NoStop_Loc2);
  CPPUNIT_TEST(testOpeartorReturnFalseWhen_NoStop_Loc2_Fail_stopover);
  CPPUNIT_TEST(testOpeartorReturnFalseWhen_NoStop_Loc2_Fail_occurrence);
  CPPUNIT_TEST(testOpeartorReturnFalseWhen_NoStop_Loc2_Fail_occurrence_origLoc2);

  CPPUNIT_TEST_SUITE_END();

public:
  bool cond;
  bool origLoc1;
  TpdPsrViaGeoLoc loc;
  MileageRouteItem item;
  TestMemHandle _memHandle;

  void setUp()
  {
    cond = false;
    origLoc1 = false;
    loc.reqDirectSvcBtwViaAndLoc1() = 'N';
    loc.reqDirectSvcBtwViaAndLoc2() = 'N';
    loc.noStopBtwViaAndLoc1() = 'N';
    loc.noStopBtwViaAndLoc2() = 'N';
    loc.stopoverNotAllowed() = 'N';
    Loc* city1(_memHandle.create<Loc>());
    Loc* city2(_memHandle.create<Loc>());
    item.city1() = city1;
    item.city2() = city2;
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    TpdPsrViaGeoLoc loc;
    bool cond(false);
    bool origLoc1(false);
    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc.conditional());
  }
  void testOperatorReturnTrueWhen_Direct_Service_Req_Loc1()
  {
    origLoc1 = true;
    loc.reqDirectSvcBtwViaAndLoc1() = 'Y';

    MileageRouteItem item;
    item.isDirectFromRouteBegin() = true;
    item.isStopover() = true;

    Loc city1;
    Loc city2;
    item.city1() = &city1;
    item.city2() = &city2;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(isInLoc(item));
  }
  void testOperatorReturnFalseWhen_Direct_Service_Req_Loc1_and_stopoverNotAllowed()
  {
    origLoc1 = true;
    loc.reqDirectSvcBtwViaAndLoc1() = 'Y';
    loc.stopoverNotAllowed() = 'Y';

    item.isDirectFromRouteBegin() = true;
    item.isStopover() = true;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
  void testOpeartorReturnTrueWhen_Direct_Service_And_NoStop_Loc1()
  {
    origLoc1 = true;
    loc.reqDirectSvcBtwViaAndLoc1() = 'Y';
    loc.noStopBtwViaAndLoc1() = 'Y';

    item.isDirectFromRouteBegin() = true;
    item.isFirstOccurrenceFromRouteBegin() = true;
    item.isStopover() = false;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(isInLoc(item));
  }
  void testOpeartorReturnFalseWhen_Direct_Service_And_NoStop_Loc1()
  {
    origLoc1 = true;
    loc.reqDirectSvcBtwViaAndLoc1() = 'Y';
    loc.noStopBtwViaAndLoc1() = 'Y';

    item.isDirectFromRouteBegin() = true;
    item.isFirstOccurrenceFromRouteBegin() = true;
    item.isStopover() = true;

    IsInLoc isInLoc(loc, cond, origLoc1);
    // bool rc =isInLoc(item);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
  void testOpeartorReturnTrueWhen_NoStop_Loc1()
  {
    origLoc1 = true;
    loc.noStopBtwViaAndLoc1() = 'Y';

    item.isDirectFromRouteBegin() = false;
    item.isFirstOccurrenceFromRouteBegin() = true;
    item.isStopover() = false;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(isInLoc(item));
  }
  void testOpeartorReturnFalseWhen_NoStop_Loc1_Fail_stopover()
  {
    origLoc1 = true;
    loc.noStopBtwViaAndLoc1() = 'Y';

    item.isDirectFromRouteBegin() = false;
    item.isFirstOccurrenceFromRouteBegin() = true;
    item.isStopover() = true;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
  void testOpeartorReturnFalseWhen_NoStop_Loc1_Fail_occurrence()
  {
    origLoc1 = true;
    loc.noStopBtwViaAndLoc1() = 'Y';

    item.isDirectFromRouteBegin() = false;
    item.isFirstOccurrenceFromRouteBegin() = false;
    item.isStopover() = false;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
  void testOpeartorReturnFalseWhen_NoStop_Loc1_Fail_occurrence_origLoc2()
  {
    loc.noStopBtwViaAndLoc1() = 'Y';

    item.isDirectFromRouteBegin() = false;
    item.isFirstOccurrenceFromRouteBegin() = true;
    item.isStopover() = false;
    item.isLastOccurrenceToRouteEnd() = false;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
  void testOpeartorReturnTrueWhen_Direct_Service_Req_Loc2()
  {
    origLoc1 = true;
    loc.reqDirectSvcBtwViaAndLoc2() = 'Y';

    item.isDirectToRouteEnd() = true;
    item.isStopover() = true;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(isInLoc(item));
  }
  void testOpeartorReturnFalseWhen_Direct_Service_Req_Loc2_and_stopoverNotAllowed()
  {
    origLoc1 = true;
    loc.reqDirectSvcBtwViaAndLoc2() = 'Y';
    loc.stopoverNotAllowed() = 'Y';

    item.isDirectToRouteEnd() = true;
    item.isStopover() = true;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
  void testOpeartorReturnTrueWhen_Direct_Service_And_NoStop_Loc2()
  {
    origLoc1 = true;
    loc.reqDirectSvcBtwViaAndLoc2() = 'Y';
    loc.noStopBtwViaAndLoc1() = 'Y';

    item.isDirectToRouteEnd() = true;
    item.isLastOccurrenceToRouteEnd() = true;
    item.isStopover() = false;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(isInLoc(item));
  }
  void testOpeartorReturnFalseWhen_Direct_Service_And_NoStop_Loc2()
  {
    origLoc1 = true;
    loc.reqDirectSvcBtwViaAndLoc2() = 'Y';
    loc.noStopBtwViaAndLoc2() = 'Y';

    item.isDirectToRouteEnd() = true;
    item.isLastOccurrenceToRouteEnd() = true;
    item.isStopover() = true;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
  void testOpeartorReturnTrueWhen_NoStop_Loc2()
  {
    origLoc1 = true;
    loc.noStopBtwViaAndLoc2() = 'Y';

    item.isDirectToRouteEnd() = false;
    item.isLastOccurrenceToRouteEnd() = true;
    item.isStopover() = false;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(isInLoc(item));
  }
  void testOpeartorReturnFalseWhen_NoStop_Loc2_Fail_stopover()
  {
    origLoc1 = true;
    loc.noStopBtwViaAndLoc2() = 'Y';

    item.isDirectToRouteEnd() = false;
    item.isLastOccurrenceToRouteEnd() = true;
    item.isStopover() = true;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
  void testOpeartorReturnFalseWhen_NoStop_Loc2_Fail_occurrence()
  {
    origLoc1 = true;
    loc.noStopBtwViaAndLoc2() = 'Y';

    item.isDirectToRouteEnd() = false;
    item.isLastOccurrenceToRouteEnd() = false;
    item.isStopover() = false;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
  void testOpeartorReturnFalseWhen_NoStop_Loc2_Fail_occurrence_origLoc2()
  {
    loc.noStopBtwViaAndLoc2() = 'Y';

    item.isDirectFromRouteBegin() = false;
    item.isFirstOccurrenceFromRouteBegin() = false;
    item.isStopover() = false;
    item.isLastOccurrenceToRouteEnd() = true;

    IsInLoc isInLoc(loc, cond, origLoc1);
    CPPUNIT_ASSERT(!isInLoc(item));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(IsInLocTest);

} // namespace TSE
