#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "FareDisplay/DefaultGroupingStrategy.h"
#include "FareDisplay/PreferredGroupingStrategy.h"
#include "FareDisplay/OverrideGroupingStrategy.h"
#include "FareDisplay/FDConsts.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "FareDisplay/StrategyBuilder.h"
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/GroupingStrategy.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  std::vector<FareDisplaySort*> _ret;

public:
  const std::vector<FareDisplaySort*>& getFareDisplaySort(const Indicator& userApplType,
                                                          const UserApplCode& userAppl,
                                                          const Indicator& pseudoCityType,
                                                          const PseudoCityCode& pseudoCity,
                                                          const TJRGroup& tjrGroup,
                                                          const DateTime& travelDate)
  {
    return _ret;
  }
};
}
class StrategyBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StrategyBuilderTest);
  CPPUNIT_TEST(testSelectStrategy);
  CPPUNIT_TEST(testBuildStrategy);
  CPPUNIT_TEST(testGetGroupingData);
  CPPUNIT_TEST(testInvokeCabinGroup);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fdTrx.setOptions(&_options);

    _request.ticketingAgent() = &_agent;
    _fdTrx.setRequest(&_request);
    _fdTrx.fdResponse() = _memHandle.create<FareDisplayResponse>();

    _fdTrx.billing() = &_billing;

    _itin.geoTravelType() = GeoTravelType::International;
    _fdTrx.itin().push_back(&_itin);

    _gs = NULL;
  }

  void tearDown() { _memHandle.clear(); }

  void testGetGroupingData()
  {
    MyDataHandle mdh;
    std::vector<Group*> groups(0);

    bool ret = _sb.getGroupingData(_fdTrx, groups);
    CPPUNIT_ASSERT(ret == false);
  }

  void testBuildStrategy()
  {
    MyDataHandle mdh;
    PaxTypeFare ptFare1, ptFare2;
    Fare fare1, fare2;
    FareInfo fareInfo1, fareInfo2;

    fare1.setFareInfo(&fareInfo1);
    fare2.setFareInfo(&fareInfo2);

    ptFare1.setFare(&fare1);
    ptFare2.setFare(&fare2);

    fareInfo1.carrier() = "AA";
    fareInfo2.carrier() = "YY";

    _fdTrx.allPaxTypeFare().push_back(&ptFare1);
    _fdTrx.allPaxTypeFare().push_back(&ptFare2);

    _gs = _sb.buildStrategy(_fdTrx);
  }

  void testSelectStrategy()
  {
    MyDataHandle mdh;
    _options.sortAscending() = 'Y';

    bool ret = _sb.selectStrategy(_fdTrx, _gs);
    CPPUNIT_ASSERT(ret == true);
    CPPUNIT_ASSERT(dynamic_cast<DefaultGroupingStrategy*>(_gs));

    _fdTrx.getRequest()->displayPassengerTypes().push_back("ADT");
    bool ret2 = _sb.selectStrategy(_fdTrx, _gs);
    CPPUNIT_ASSERT(ret2 == true);
    CPPUNIT_ASSERT(dynamic_cast<OverrideGroupingStrategy*>(_gs));

    _fdTrx.getRequest()->displayPassengerTypes().clear();
    _options.sortAscending() = 0;
    _request.requestedInclusionCode() = ALL_INCLUSION_CODE;
    ret = _sb.selectStrategy(_fdTrx, _gs);
    CPPUNIT_ASSERT(ret == true);
    CPPUNIT_ASSERT(dynamic_cast<PreferredGroupingStrategy*>(_gs));

    _fdTrx.getRequest()->displayPassengerTypes().push_back("ADT");
    ret = _sb.selectStrategy(_fdTrx, _gs);
    CPPUNIT_ASSERT(ret == true);
    CPPUNIT_ASSERT(dynamic_cast<PreferredGroupingStrategy*>(_gs));
  }

  void testInvokeCabinGroup()
  {
    std::vector<Group*> groups;

    _sb.invokeCabinGroup(_fdTrx, groups);

    CPPUNIT_ASSERT(!groups.empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)groups.size());
    CPPUNIT_ASSERT(groups[0]->groupType() == Group::GROUP_BY_CABIN);
    CPPUNIT_ASSERT(!_fdTrx.fdResponse()->groupHeaders().empty());
    CPPUNIT_ASSERT(_fdTrx.fdResponse()->groupHeaders()[0] == Group::GROUP_BY_CABIN);
  }

private:
  FareDisplayTrx _fdTrx;
  Itin _itin;
  GroupingStrategy* _gs;
  FareDisplayOptions _options;
  FareDisplayRequest _request;
  Agent _agent;
  Billing _billing;
  StrategyBuilder _sb;
};
CPPUNIT_TEST_SUITE_REGISTRATION(StrategyBuilderTest);
}
