#include "Common/DateTime.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDisplayPref.h"
#include "DBAccess/FareInfo.h"
#include "FareDisplay/ScheduleCountMgr.cpp"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/testdata/TestFactoryManager.h"

using namespace std;
namespace tse
{
class ScheduleCountMgrTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ScheduleCountMgrTest);
  CPPUNIT_TEST(testisCarrierHasFare);
  CPPUNIT_TEST_SUITE_END();

public:
  void testisCarrierHasFare()
  {
    PaxTypeFare pFare1, pFare2;
    Fare fare1, fare2;
    FareInfo info1, info2;

    info1.carrier() = "AA";
    info2.carrier() = "BA";

    fare1.setFareInfo(&info1);
    fare2.setFareInfo(&info2);

    pFare1.setFare(&fare1);
    pFare2.setFare(&fare2);

    std::vector<PaxTypeFare*> fares;

    CPPUNIT_ASSERT(!ScheduleCountMgr::isCarrierHasFare(fares, "AA"));
    fares.push_back(&pFare1);
    CPPUNIT_ASSERT(ScheduleCountMgr::isCarrierHasFare(fares, "AA"));
    fares.push_back(&pFare2);
    CPPUNIT_ASSERT(ScheduleCountMgr::isCarrierHasFare(fares, "BA"));
    CPPUNIT_ASSERT(!ScheduleCountMgr::isCarrierHasFare(fares, "DL"));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ScheduleCountMgrTest);
}
