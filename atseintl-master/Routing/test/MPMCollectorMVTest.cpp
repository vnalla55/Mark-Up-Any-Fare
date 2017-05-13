#include "Routing/MPMCollectorMV.h"
#include "Routing/MileageRoute.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Routing/Collector.h"
#include <cstdlib>
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

namespace
{
class MockMPMCollectorMV : public MPMCollectorMV
{
private:
  bool getMPM(MileageRouteItem& item, DataHandle& dataHandle) const
  {
    if (item.city2()->loc() == "NUL")
    {
      return false;
    }
    item.mpm() = atoi(item.city2()->loc().c_str());
    return true;
  }
};
}
class MPMCollectorMVTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MPMCollectorMVTest);
  CPPUNIT_TEST(testCollectMileage);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }
  void testCollectMileage()
  {
    const Collector<MockMPMCollectorMV>& collector(
        tse::Singleton<Collector<MockMPMCollectorMV> >::instance());
    DataHandle dataHandle;
    MileageRouteItem item1, item2;
    std::string nul("NUL");
    item1.city1() = _memHandle.create<Loc>();
    item1.city2() = _memHandle.create<Loc>();
    item1.city1()->loc() = item1.city2()->loc() = nul;
    item2.city1() = _memHandle.create<Loc>();
    item2.city2() = _memHandle.create<Loc>();
    item2.city1()->loc() = item2.city2()->loc() = nul;
    MileageRoute route;
    route.mileageRouteItems().push_back(item1);
    route.mileageRouteItems().push_back(item2);
    route.mileageRouteMPM() = 10;
    CPPUNIT_ASSERT(!collector.collectMileage(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(10), route.mileageRouteMPM());
    route.globalDirection() = GlobalDirection::XX;
    route.mileageRouteItems().clear();
    item2.city2()->loc() = "100";
    route.mileageRouteItems().push_back(item1);
    route.mileageRouteItems().push_back(item2);
    CPPUNIT_ASSERT(collector.collectMileage(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), route.mileageRouteMPM());
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MPMCollectorMVTest);
}
