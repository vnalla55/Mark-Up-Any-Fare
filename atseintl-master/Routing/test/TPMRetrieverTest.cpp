#include "Routing/TPMRetriever.h"
#include <cstdlib>
#include "Routing/MileageRouteItem.h"
#include "Routing/Retriever.h"
#include "DBAccess/Loc.h"
#include "DBAccess/DataHandle.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

namespace
{
class MockTPMRetriever : public TPMRetriever
{
private:
  bool getMileage(MileageRouteItem& item, DataHandle& dataHandle) const
  {
    if (item.city1()->loc() == "MIL")
    {
      item.tpm() = 1000;
      return true;
    }
    short tpm = atoi(item.city1()->loc().c_str()) + atoi(item.city2()->loc().c_str());
    switch (tpm)
    {
    case 30:
    case 90:
    case 300:
      item.tpm() = tpm;
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
  bool getConstructed(MileageRouteItem& item, DataHandle& dataHandle) const
  {
    if (item.city1()->loc() == "CON")
    {
      item.tpm() = 100;
      return true;
    }
    return false;
  }
};
}

class TPMRetrieverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TPMRetrieverTest);
  CPPUNIT_TEST(testRetrieve);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void testRetrieve()
  {
    const Retriever<MockTPMRetriever>& retriever(
        tse::Singleton<Retriever<MockTPMRetriever> >::instance());
    DataHandle dataHandle;
    MileageRouteItem item;
    item.city1() = _memHandle.create<Loc>();
    item.city2() = _memHandle.create<Loc>();
    std::string nul("NUL");
    item.tpm() = 5;
    item.city1()->loc() = item.city2()->loc() = "NUL";
    CPPUNIT_ASSERT(!retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(5), item.tpm());
    CPPUNIT_ASSERT_EQUAL(nul, static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(nul, static_cast<std::string>(item.city2()->loc()));
    item.city1()->loc() = item.city2()->loc() = "MIL";
    CPPUNIT_ASSERT(retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(1000), item.tpm());
    CPPUNIT_ASSERT_EQUAL(std::string("MIL"), static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(std::string("MIL"), static_cast<std::string>(item.city2()->loc()));
    item.city1()->loc() = "NUL";
    item.city2()->loc() = "SUB";
    CPPUNIT_ASSERT(!retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(1000), item.tpm());
    CPPUNIT_ASSERT_EQUAL(nul, static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(std::string("SUB"), static_cast<std::string>(item.city2()->loc()));
    item.city1()->loc() = "SUB";
    CPPUNIT_ASSERT(retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(90), item.tpm());
    CPPUNIT_ASSERT_EQUAL(std::string("SUB"), static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(std::string("SUB"), static_cast<std::string>(item.city2()->loc()));
    item.city1()->loc() = "CON";
    CPPUNIT_ASSERT(retriever.retrieve(item, dataHandle));
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(100), item.tpm());
    CPPUNIT_ASSERT_EQUAL(std::string("CON"), static_cast<std::string>(item.city1()->loc()));
    CPPUNIT_ASSERT_EQUAL(std::string("SUB"), static_cast<std::string>(item.city2()->loc()));
  }
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TPMRetrieverTest);
}
