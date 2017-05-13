#include "Routing/MPMRetriever.h"
#include <cstdlib>
#include "Routing/MileageRouteItem.h"
#include "Routing/Retriever.h"
#include "DBAccess/Loc.h"
#include "DBAccess/DataHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

namespace
{
class MockMPMRetriever : public MPMRetriever
{
private:
  bool getMileage(MileageRouteItem& item, DataHandle& dataHandle) const
  {
    if (item.city1()->loc() == "MIL")
    {
      item.mpm() = 1000;
      return true;
    }
    short mpm = atoi(item.city1()->loc().c_str()) + atoi(item.city2()->loc().c_str());
    switch (mpm)
    {
    case 30:
    case 90:
    case 300:
      item.mpm() = mpm;
      return true;
    };
    return false;
  }
  bool getMileageSubstitution(MileageRouteItem& item, DataHandle& dataHandle) const
  {
    bool result(false);
    if (item.city1()->loc() == "SUB")
    {
      item.city1()->loc() = "040";
      result = true;
    }
    if (item.city2()->loc() == "SUB")
    {
      item.city2()->loc() = "050";
      result = true;
    }
    return result;
  }
  bool getAdditionalMileage(MileageRouteItem& item, DataHandle& dataHandle) const
  {
    uint16_t mpm(0);
    if (item.city1()->loc() == "ADD")
    {
      mpm += 100;
    }
    if (item.city2()->loc() == "ADD")
    {
      mpm += 200;
    }
    if (mpm == 300)
    {
      item.mpm() = mpm;
      return true;
    }
    return false;
  }
};
}

class MPMRetrieverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MPMRetrieverTest);
  CPPUNIT_TEST(testRetrieve);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void testRetrieve()
  {
    const Retriever<MockMPMRetriever>& retriever(
        tse::Singleton<Retriever<MockMPMRetriever> >::instance());
    DataHandle dataHandle;
    MileageRouteItem item;
    item.city1() = _memHandle.create<Loc>();
    item.city2() = _memHandle.create<Loc>();
    std::string nul("NUL");
    item.mpm() = 5;
    item.city1()->loc() = item.city2()->loc() = "NUL";
    CPPUNIT_ASSERT(!retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(5), item.mpm());
    CPPUNIT_ASSERT_EQUAL(nul, static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(nul, static_cast<std::string>(item.city2()->loc()));
    item.city1()->loc() = item.city2()->loc() = "MIL";
    CPPUNIT_ASSERT(retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(1000), item.mpm());
    CPPUNIT_ASSERT_EQUAL(std::string("MIL"), static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(std::string("MIL"), static_cast<std::string>(item.city2()->loc()));
    item.city1()->loc() = "NUL";
    item.city2()->loc() = "SUB";
    CPPUNIT_ASSERT(!retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(1000), item.mpm());
    CPPUNIT_ASSERT_EQUAL(nul, static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(std::string("SUB"), static_cast<std::string>(item.city2()->loc()));
    item.city1()->loc() = "SUB";
    CPPUNIT_ASSERT(retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(90), item.mpm());
    CPPUNIT_ASSERT_EQUAL(std::string("SUB"), static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(std::string("SUB"), static_cast<std::string>(item.city2()->loc()));
    item.city1()->loc() = "NUL";
    item.city2()->loc() = "ADD";
    CPPUNIT_ASSERT(!retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(90), item.mpm());
    CPPUNIT_ASSERT_EQUAL(nul, static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(std::string("ADD"), static_cast<std::string>(item.city2()->loc()));
    item.city1()->loc() = "ADD";
    CPPUNIT_ASSERT(retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(300), item.mpm());
    CPPUNIT_ASSERT_EQUAL(std::string("ADD"), static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(std::string("ADD"), static_cast<std::string>(item.city2()->loc()));
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MPMRetrieverTest);
}
