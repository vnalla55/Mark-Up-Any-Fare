#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/DefaultGroupingStrategy.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseEnums.h"
#include "Common/Config/ConfigMan.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareDisplayRequest.h"

#include "FareDisplay/GroupingStrategy.h"
#include "FareDisplay/StrategyBuilder.h"
#include "DataModel/Agent.h"
#include "DataModel/Itin.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;
namespace tse
{
class DefaultGroupingStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DefaultGroupingStrategyTest);
  CPPUNIT_TEST(testApply_NoTrx);
  CPPUNIT_TEST(testApply_Ascending);
  CPPUNIT_TEST(testApply_Descending);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _groupingStr = _memHandle.create<DefaultGroupingStrategy>();
    _groupingStr->_trx = _memHandle.create<FareDisplayTrx>();
    _groupingStr->_trx->setOptions(_memHandle.create<FareDisplayOptions>());
    _groupingStr->_trx->setRequest(_memHandle.create<FareDisplayRequest>());
    _groupingStr->_trx->itin().push_back(_memHandle.create<Itin>());
    _groupingStr->_trx->itin().front()->geoTravelType() = GeoTravelType::International;
    _groupingStr->_trx->getOptions()->sortAscending() = true;
  }
  void tearDown() { _memHandle.clear(); }
  void testApply_NoTrx()
  {
    // if No trx then fail appy
    _groupingStr->_trx = 0;
    CPPUNIT_ASSERT(!_groupingStr->apply());
  }
  void testApply_Ascending()
  {
    // Default goup strategy is GROUP_BY_FARE_AMOUNT, sort ascending
    CPPUNIT_ASSERT(_groupingStr->apply());
    CPPUNIT_ASSERT(_groupingStr->groups().size() == 1);
    CPPUNIT_ASSERT(_groupingStr->groups().front()->groupType() == Group::GROUP_BY_FARE_AMOUNT);
    CPPUNIT_ASSERT(_groupingStr->groups().front()->sortType() ==
                   DefaultGroupingStrategy::ASCENDING);
  }
  void testApply_Descending()
  {
    // Default goup strategy is GROUP_BY_FARE_AMOUNT, sort descending
    _groupingStr->_trx->getOptions()->sortAscending() = false;
    _groupingStr->_trx->getOptions()->sortDescending() = true;
    CPPUNIT_ASSERT(_groupingStr->apply());
    CPPUNIT_ASSERT(_groupingStr->groups().size() == 1);
    CPPUNIT_ASSERT(_groupingStr->groups().front()->groupType() == Group::GROUP_BY_FARE_AMOUNT);
    CPPUNIT_ASSERT(_groupingStr->groups().front()->sortType() ==
                   DefaultGroupingStrategy::DESCENDING);
  }

private:
  TestMemHandle _memHandle;
  GroupingStrategy* _groupingStr;
};
CPPUNIT_TEST_SUITE_REGISTRATION(DefaultGroupingStrategyTest);
}
