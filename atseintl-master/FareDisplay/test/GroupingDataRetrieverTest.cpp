#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "FareDisplay/GroupingDataRetriever.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/Itin.h"
#include "DataModel/Billing.h"
#include "FareDisplay/Group.h"
#include "DataModel/Agent.h"
#include "Common/DateTime.h"

namespace tse
{
class GroupingDataRetrieverTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(GroupingDataRetrieverTest);
  CPPUNIT_TEST(testGroupingDataRetriever);
  CPPUNIT_TEST(testGroupHeader);
  CPPUNIT_TEST_SUITE_END();

public:
  class OurDataHandleMock : public DataHandleMock
  {
  public:
    OurDataHandleMock()
    {
      static FareDisplaySort fds;
      fds.fareDisplayType() = 'S';
      fds.domIntlAppl() = 'I';
      _vecFds.push_back(&fds);
    }
    ~OurDataHandleMock() {}

    const std::vector<FareDisplaySort*>& getFareDisplaySort(const Indicator& userApplType,
                                                            const UserApplCode& userAppl,
                                                            const Indicator& pseudoCityType,
                                                            const PseudoCityCode& pseudoCity,
                                                            const TJRGroup& tjrGroup,
                                                            const DateTime& travelDate)
    {
      return _vecFds;
    }

  private:
    std::vector<FareDisplaySort*> _vecFds;

  }; // class OurDataHandleMock

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<OurDataHandleMock>();

    static FareDisplayTrx fTrx;
    static FareDisplayOptions options;
    static Itin itin;
    static Billing billing;
    static Agent agent;
    static FareDisplayRequest request;

    itin.geoTravelType() = GeoTravelType::International;
    billing.partitionID() = "1S";
    request.ticketingAgent() = &agent;

    fTrx.fdResponse() = &_response;
    fTrx.setOptions(&options);
    fTrx.billing() = &billing;
    fTrx.setRequest(&request);
    fTrx.itin().push_back(&itin);
    fTrx.preferredCarriers().insert("AA");

    GroupingDataRetriever retriever(fTrx);
    retriever.getGroupAndSortPref(_groups);
  }

  void tearDown() { _memHandle.clear(); }

  void testGroupingDataRetriever() { CPPUNIT_ASSERT(!_groups.empty()); }

  void testGroupHeader() { CPPUNIT_ASSERT(!_response.groupHeaders().empty()); }

private:
  TestMemHandle _memHandle;

  std::vector<Group*> _groups;
  FareDisplayResponse _response;

}; // class GroupingDataRetrieverTest

CPPUNIT_TEST_SUITE_REGISTRATION(GroupingDataRetrieverTest);
}
