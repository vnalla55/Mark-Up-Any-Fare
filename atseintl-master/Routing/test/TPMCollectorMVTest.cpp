#include "Routing/TPMCollectorMV.h"
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

class MockTPMCollectorMV : public TPMCollectorMV
{
private:
  bool getTPM(MileageRouteItem& item, DataHandle& dataHandle) const
  {
    item.tpm() = atoi(item.city1()->loc().c_str());
    return true;
  }
  bool getSurfaceSector(MileageRouteItem& item, DataHandle& dataHandle) const
  {
    if (item.city2()->loc() == "SUR")
    {
      item.tpmSurfaceSectorExempt() = true;
      return true;
    }
    return false;
  }
  bool getSouthAtlantic(MileageRoute& route) const { return false; }
};
}
class TPMCollectorMVTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TPMCollectorMVTest);
  CPPUNIT_TEST(testCollectMileage);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void testCollectMileage()
  {
    const Collector<MockTPMCollectorMV>& collector(
        tse::Singleton<Collector<MockTPMCollectorMV> >::instance());
    DataHandle dataHandle;
    MileageRouteItem item1, item2, item3;
    std::string nul("NUL");
    item1.city1() = _memHandle.create<Loc>();
    item1.city2() = _memHandle.create<Loc>();
    item1.city1()->loc() = "100";
    item1.city2()->loc() = "SUR";
    item2.city1() = _memHandle.create<Loc>();
    item2.city2() = _memHandle.create<Loc>();
    item2.city1()->loc() = "200";
    item2.city2()->loc() = "SUR";
    item2.isSurface() = true;
    item3.city1() = _memHandle.create<Loc>();
    item3.city2() = _memHandle.create<Loc>();
    item3.city1()->loc() = "300";
    item3.city2()->loc() = nul;
    item1.tpm() = item2.tpm() = item3.tpm() = 10;
    MileageRoute route;
    route.mileageRouteItems().push_back(item1);
    route.mileageRouteItems().push_back(item2);
    route.mileageRouteItems().push_back(item3);
    route.dataHandle() = &dataHandle;
    CPPUNIT_ASSERT(collector.collectMileage(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), route.mileageRouteItems()[0].tpm());
    CPPUNIT_ASSERT(!route.mileageRouteItems()[0].tpmSurfaceSectorExempt());
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(200), route.mileageRouteItems()[1].tpm());
    CPPUNIT_ASSERT(route.mileageRouteItems()[1].tpmSurfaceSectorExempt());
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(300), route.mileageRouteItems()[2].tpm());
    CPPUNIT_ASSERT(!route.mileageRouteItems()[2].tpmSurfaceSectorExempt());
  }
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TPMCollectorMVTest);
}
