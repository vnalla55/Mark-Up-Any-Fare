/*
 * OptionalServicesGroupCodeFilterTest.cpp
 *
 *  Created on: Dec 17, 2014
 *      Author: SG0221190
 */

#include <set>
#include <utility>
#include <vector>

#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/ServiceGroupInfo.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"

#include "ServiceFees/OptionalServicesGroupCodeFilter.h"

namespace tse
{

class OptionalServicesGroupCodeFilterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OptionalServicesGroupCodeFilterTest);

  CPPUNIT_TEST(test_NoGroups_PACS);
  CPPUNIT_TEST(test_NoGroups_PANCS);
  CPPUNIT_TEST(test_EmptyOCF_PACS);
  CPPUNIT_TEST(test_EmptyOCF_PANCS);
  CPPUNIT_TEST(test_OneActive_PACS_BG);
  CPPUNIT_TEST(test_OneActive_PACS_BG_ProcessAllGroups);
  CPPUNIT_TEST(test_OneActive_PACS_SA);
  CPPUNIT_TEST(test_OneActive_PANCS_BG);
  CPPUNIT_TEST(test_OneActive_PANCS_SA);
  CPPUNIT_TEST(test_OneInactive_PACS);
  CPPUNIT_TEST(test_OneInactive_PANCS);
  CPPUNIT_TEST(test_Invalid_PACS);
  CPPUNIT_TEST(test_Invalid_PANCS);
  CPPUNIT_TEST(test_PANCS_MISC6_0);
  CPPUNIT_TEST(test_PANCS_MISC6_1);

  CPPUNIT_TEST_SUITE_END();

public:
  void addToAllGroupCodes(const char* groupCode,
                          DateTime effDate, DateTime discDate)
  {
    ServiceGroupInfo* sgi;
    sgi = _memHandle.create<ServiceGroupInfo>();
    sgi->svcGroup() = groupCode;
    sgi->effDate() = effDate;
    sgi->discDate() = discDate;
    _allGroupCodes->push_back(sgi);
  }

  void setUp()
  {
    _server = _memHandle.create<MockTseServer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _request->ticketingDT() = DateTime(2014, 6, 1);
    _request->validatingCarrier() = "AA";
    _request->ticketingAgent() = _memHandle.create<Agent>();
    _trx->billing() = _memHandle.create<Billing>();
    _trx->setOptions(_memHandle.create<PricingOptions>());

    _allGroupCodes = _memHandle.create< std::vector<ServiceGroupInfo*> >();
    addToAllGroupCodes("BG", DateTime(2014, 1, 1), DateTime(2014, 12, 1));
    addToAllGroupCodes("ML", DateTime(2014, 1, 1), DateTime(2014, 12, 1));
    addToAllGroupCodes("SA", DateTime(2014, 1, 1), DateTime(2014, 12, 1));
    addToAllGroupCodes("99", DateTime(2014, 1, 1), DateTime(2014, 12, 1));
    addToAllGroupCodes("PT", DateTime(2014, 1, 1), DateTime(2014, 12, 1));
    _grActive = createGroupSet("BG ML SA 99 PT");
  }

  void tearDown() { _memHandle.clear(); }

  typedef std::set<ServiceGroup> GroupSet;

  GroupSet* createGroupSet(const char* groupCodes)
  {
    GroupSet* groupSet = _memHandle.create<GroupSet>();
    std::istringstream splitter(groupCodes);
    std::string gc;
    while (!splitter.eof())
    {
      splitter >> gc;
      groupSet->insert(gc);
    }
    return groupSet;
  }

  void addRequestedGroup(const char* groupCode,
                         RequestedOcFeeGroup::RequestedInformation reqInfo)
  {
    RequestedOcFeeGroup rqGroup;
    rqGroup.groupCode() = groupCode;
    rqGroup.setRequestedInformation(reqInfo);
    _trx->getOptions()->serviceGroupsVec().push_back(rqGroup);
  }

  void printSet(const char* header, int line, const GroupSet& set)
  {
    std::cout << "\n" << header << "[" << set.size() << ":L" << line << "]:";
    for (GroupSet::const_iterator iGrp = set.begin(); iGrp != set.end(); ++iGrp)
    {
      std::cout << " " << *iGrp;
    }
    std::cout.flush();
  }

  #define xstr(s) str(s)
  #define str(s) #s
  #define PRINT_SET(_set_) do{PrintSet(xstr(_set_), __LINE__, _set_);}while(0)

  void test_NoGroups_PACS()
  {
    _trx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("SA"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)2, filter.grNotActive().size());
    CPPUNIT_ASSERT(filter.isInSetNotActive("99"));
    CPPUNIT_ASSERT(filter.isInSetNotActive("ML"));
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)2, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
  }

  void test_NoGroups_PANCS()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->billing()->requestPath() = ANCS_PO_ATSE_PATH;
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)5, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("SA"));
    CPPUNIT_ASSERT(filter.isInSetValid("BG"));
    CPPUNIT_ASSERT(filter.isInSetValid("PT"));
    CPPUNIT_ASSERT(filter.isInSetValid("99"));
    CPPUNIT_ASSERT(filter.isInSetValid("ML"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotProcessed().size());
  }

  void test_EmptyOCF_PACS()
  {
    _trx->getOptions()->isProcessAllGroups() = true;
    _trx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    addRequestedGroup("SA", RequestedOcFeeGroup::NoData);
    addRequestedGroup("PT", RequestedOcFeeGroup::NoData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("SA"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)4, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_EmptyOCF_PANCS()
  {
    _trx->getOptions()->isProcessAllGroups() = true;
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->billing()->requestPath() = ANCS_PO_ATSE_PATH;
    addRequestedGroup("SA", RequestedOcFeeGroup::NoData);
    addRequestedGroup("PT", RequestedOcFeeGroup::NoData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)5, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("SA"));
    CPPUNIT_ASSERT(filter.isInSetValid("BG"));
    CPPUNIT_ASSERT(filter.isInSetValid("PT"));
    CPPUNIT_ASSERT(filter.isInSetValid("99"));
    CPPUNIT_ASSERT(filter.isInSetValid("ML"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotProcessed().size());
  }

  void test_OneActive_PACS_BG()
  {
    _trx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    addRequestedGroup("BG", RequestedOcFeeGroup::NoData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grValid().size());
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)5, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("SA"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_OneActive_PACS_BG_ProcessAllGroups()
  {
    _trx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    addRequestedGroup("BG", RequestedOcFeeGroup::NoData);
    OptionalServicesGroupCodeFilter filter(*_trx, true, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("SA"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)2, filter.grNotActive().size());
    CPPUNIT_ASSERT(filter.isInSetNotActive("99"));
    CPPUNIT_ASSERT(filter.isInSetNotActive("ML"));
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)2, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
  }

  void test_OneActive_PACS_SA()
  {
    _trx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    addRequestedGroup("SA", RequestedOcFeeGroup::NoData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("SA"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)4, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_OneActive_PANCS_BG()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->billing()->requestPath() = ANCS_PO_ATSE_PATH;
    addRequestedGroup("BG", RequestedOcFeeGroup::AncillaryData);
    addRequestedGroup("SA", RequestedOcFeeGroup::NoData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("BG"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)4, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("SA"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_OneActive_PANCS_SA()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->billing()->requestPath() = ANCS_PO_ATSE_PATH;
    addRequestedGroup("BG", RequestedOcFeeGroup::NoData);
    addRequestedGroup("SA", RequestedOcFeeGroup::AncillaryData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("SA"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)4, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_OneInactive_PACS()
  {
    _trx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    addRequestedGroup("SB", RequestedOcFeeGroup::NoData);
    addToAllGroupCodes("SB", DateTime(2014, 1, 1), DateTime(2014, 12, 1));
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grValid().size());
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grNotActive().size());
    CPPUNIT_ASSERT(filter.isInSetNotActive("SB"));
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)5, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("SA"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_OneInactive_PANCS()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->billing()->requestPath() = ANCS_PO_ATSE_PATH;
    addRequestedGroup("SB", RequestedOcFeeGroup::NoData);
    addToAllGroupCodes("SB", DateTime(2014, 1, 1), DateTime(2014, 12, 1));
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grValid().size());
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grNotActive().size());
    CPPUNIT_ASSERT(filter.isInSetNotActive("SB"));
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)5, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("SA"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_Invalid_PACS()
  {
    _trx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    addRequestedGroup("OO", RequestedOcFeeGroup::NoData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grValid().size());
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grNotValid().size());
    CPPUNIT_ASSERT(filter.isInSetNotValid("OO"));
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)5, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("SA"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_Invalid_PANCS()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->billing()->requestPath() = ANCS_PO_ATSE_PATH;
    addRequestedGroup("OO", RequestedOcFeeGroup::AncillaryData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grValid().size());
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grNotValid().size());
    CPPUNIT_ASSERT(filter.isInSetNotValid("OO"));
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)5, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("SA"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_PANCS_MISC6_0()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->billing()->requestPath() = ANCS_PO_ATSE_PATH;
    _trx->billing()->actionCode() = "MISC6";
    addRequestedGroup("BG", RequestedOcFeeGroup::NoData);
    addRequestedGroup("SA", RequestedOcFeeGroup::NoData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grValid().size());
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)5, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("SA"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("BG"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_PANCS_MISC6_1()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->billing()->requestPath() = ANCS_PO_ATSE_PATH;
    _trx->billing()->actionCode() = "MISC6";
    addRequestedGroup("BG", RequestedOcFeeGroup::AncillaryData);
    addRequestedGroup("SA", RequestedOcFeeGroup::NoData);
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("BG"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)4, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("SA"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

  void test_PANCS_EmptyOCF()
  {
    _trx->modifiableActivationFlags().setAB240(true);
    _trx->getOptions()->isProcessAllGroups() = true;
    _trx->billing()->requestPath() = ANCS_PO_ATSE_PATH;
    OptionalServicesGroupCodeFilter filter(*_trx, false, *_allGroupCodes, *_grActive);
    // Valid groups
    CPPUNIT_ASSERT_EQUAL((size_t)1, filter.grValid().size());
    CPPUNIT_ASSERT(filter.isInSetValid("BG"));
    // NotValid groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotValid().size());
    // NotActive groups
    CPPUNIT_ASSERT_EQUAL((size_t)0, filter.grNotActive().size());
    // NotProcessed groups
    CPPUNIT_ASSERT_EQUAL((size_t)4, filter.grNotProcessed().size());
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("SA"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("PT"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("99"));
    CPPUNIT_ASSERT(filter.isInSetNotProcessed("ML"));
  }

protected:
  TestMemHandle _memHandle;
  TseServer* _server;
  PricingTrx* _trx;
  PricingRequest* _request;
  std::vector<ServiceGroupInfo*>* _allGroupCodes;
  GroupSet* _grActive;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionalServicesGroupCodeFilterTest);

} // namespace tse
