#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/CabinGroup.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayRequest.h"
#include "FareDisplay/Group.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class CabinGroupTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CabinGroupTest);
  CPPUNIT_TEST(testInitializeCabinGroup);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _fTrx = _memHandle.create<FareDisplayTrx>();
    _fTrx->fdResponse() = _memHandle.create<FareDisplayResponse>();
    _fTrx->setRequest(_memHandle.create<FareDisplayRequest>());
    _group = _memHandle.create<Group>();
  }

  void tearDown() { _memHandle.clear(); }

  void testInitializeCabinGroup()
  {
    std::vector<Group*> groups;
    CabinGroup cabinGr;
    cabinGr.initializeCabinGroup(*_fTrx, groups);

    CPPUNIT_ASSERT(!groups.empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)groups.size());
    CPPUNIT_ASSERT(groups[0]->groupType() == Group::GROUP_BY_CABIN);
    CPPUNIT_ASSERT(!_fTrx->fdResponse()->groupHeaders().empty());
    CPPUNIT_ASSERT(_fTrx->fdResponse()->groupHeaders()[0] == Group::GROUP_BY_CABIN);
  }

private:
  FareDisplayTrx* _fTrx;
  Group* _group;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(CabinGroupTest);
}
