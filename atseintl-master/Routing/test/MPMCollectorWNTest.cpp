#include "Routing/MPMCollectorWN.h"
#include "Routing/MileageRoute.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Routing/Collector.h"
#include <cstdlib>
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{

namespace
{
class MockMPMCollectorWN : public MPMCollectorWN
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
class MyDataHandle : public DataHandleMock
{
  std::vector<TpdPsr*> _ret;

public:
  const std::vector<TpdPsr*>& getTpdPsr(Indicator applInd,
                                        const CarrierCode& carrier,
                                        Indicator area1,
                                        Indicator area2,
                                        const DateTime& date)
  {
    if (carrier == "**")
      return _ret;
    return DataHandleMock::getTpdPsr(applInd, carrier, area1, area2, date);
  }
};
}

class MPMCollectorWNTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MPMCollectorWNTest);
  CPPUNIT_TEST(testCollectMileage);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _memHandle.create<TestConfigInitializer>();
  }
  void tearDown() { _memHandle.clear(); }

  void testCollectMileage()
  {
    const Collector<MockMPMCollectorWN>& collector(
        tse::Singleton<Collector<MockMPMCollectorWN> >::instance());
    DataHandle dataHandle;
    MileageRouteItem item1, item2, item3, item4;
    std::string nul("NUL");
    item1.city1() = _memHandle.create<Loc>();
    item1.city2() = _memHandle.create<Loc>();
    item1.city1()->loc() = item1.city2()->loc() = nul;
    item1.mpmGlobalDirection() = GlobalDirection::XX;
    item2.city1() = _memHandle.create<Loc>();
    item2.city2() = _memHandle.create<Loc>();
    item2.city1()->loc() = nul;
    item2.city2()->loc() = "100";
    item2.mpmGlobalDirection() = GlobalDirection::XX;
    item3.city1() = _memHandle.create<Loc>();
    item3.city2() = _memHandle.create<Loc>();
    item3.city1()->loc() = nul;
    item3.city2()->loc() = "100";
    item3.mpmGlobalDirection() = GlobalDirection::EH;
    item4.city1() = _memHandle.create<Loc>();
    item4.city2() = _memHandle.create<Loc>();
    item4.city1()->loc() = nul;
    item4.city2()->loc() = "1100";
    item4.mpmGlobalDirection() = EH;
    item1.mpm() = item2.mpm() = item3.mpm() = item4.mpm() = 10;
    MileageRoute route;
    route.dataHandle() = &dataHandle;
    route.mileageRouteItems().push_back(item1);
    route.mileageRouteItems().push_back(item2);
    route.mileageRouteItems().push_back(item3);
    route.mileageRouteItems().push_back(item4);
    CPPUNIT_ASSERT(collector.collectMileage(route));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(10), route.mileageRouteItems()[0].mpm());
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), route.mileageRouteItems()[1].mpm());
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), route.mileageRouteItems()[2].mpm());
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(1100), route.mileageRouteItems()[3].mpm());
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MPMCollectorWNTest);
}
