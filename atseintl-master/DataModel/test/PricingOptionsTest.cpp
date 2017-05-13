#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PricingOptions.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class PricingOptionsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingOptionsTest);

  CPPUNIT_TEST(testGetGroupCodesWhenServiceGroupsEmpty);
  CPPUNIT_TEST(testGetGroupCodesWhenServiceGroupsHasOneElement);
  CPPUNIT_TEST(testGetGroupCodesWhenServiceGroupsHasManyElements);
  CPPUNIT_TEST(testIsOcOrBaggageDataRequested_empty);
  CPPUNIT_TEST(testIsOcOrBaggageDataRequested_AncillaryData);
  CPPUNIT_TEST(testIsOcOrBaggageDataRequested_AncillaryData_all);
  CPPUNIT_TEST(testIsOcOrBaggageDataRequested_CatalogData);
  CPPUNIT_TEST(testIsOcOrBaggageDataRequested_DisclosureData);

  CPPUNIT_TEST(testIsServiceTypeRequested_found);
  CPPUNIT_TEST(testIsServiceTypeRequested_NotFound);
  CPPUNIT_TEST(testIsServiceTypeRequested_empty);



  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  PricingOptions* _pricingOpts;

public:
  void setUp() { _pricingOpts = _memHandle.create<PricingOptions>(); }

  void tearDown() { _memHandle.clear(); }

  void testGetGroupCodesWhenServiceGroupsEmpty()
  {
    std::vector<RequestedOcFeeGroup> requestedGroups;
    std::vector<ServiceGroup> groupCodes;

    _pricingOpts->getGroupCodes(requestedGroups, groupCodes);

    CPPUNIT_ASSERT(groupCodes.empty());
  }

  void testGetGroupCodesWhenServiceGroupsHasOneElement()
  {
    std::vector<RequestedOcFeeGroup> requestedGroups;
    RequestedOcFeeGroup requestedOcFeeGroup;
    requestedOcFeeGroup.groupCode() = "ML";
    requestedGroups.push_back(requestedOcFeeGroup);

    std::vector<ServiceGroup> groupCodes;

    _pricingOpts->getGroupCodes(requestedGroups, groupCodes);

    CPPUNIT_ASSERT_EQUAL(size_t(1), groupCodes.size());
    CPPUNIT_ASSERT(groupCodes[0] == "ML");
  }

  void testGetGroupCodesWhenServiceGroupsHasManyElements()
  {
    std::vector<RequestedOcFeeGroup> requestedGroups;
    RequestedOcFeeGroup requestedOcFeeGroup1, requestedOcFeeGroup2;
    requestedOcFeeGroup1.groupCode() = "ML";
    requestedGroups.push_back(requestedOcFeeGroup1);
    requestedOcFeeGroup2.groupCode() = "BG";
    requestedGroups.push_back(requestedOcFeeGroup2);

    std::vector<ServiceGroup> groupCodes;

    _pricingOpts->getGroupCodes(requestedGroups, groupCodes);

    CPPUNIT_ASSERT_EQUAL(size_t(2), groupCodes.size());
    CPPUNIT_ASSERT(groupCodes[0] == "ML");
    CPPUNIT_ASSERT(groupCodes[1] == "BG");
  }


  void testIsOcOrBaggageDataRequested_empty()
  {
    CPPUNIT_ASSERT(!_pricingOpts->isOcOrBaggageDataRequested(RequestedOcFeeGroup::AncillaryData));
    CPPUNIT_ASSERT(!_pricingOpts->isOcOrBaggageDataRequested(RequestedOcFeeGroup::CatalogData));
    CPPUNIT_ASSERT(!_pricingOpts->isOcOrBaggageDataRequested(RequestedOcFeeGroup::DisclosureData));
  }

  void testIsOcOrBaggageDataRequested_AncillaryData()
  {
    RequestedOcFeeGroup rf;
    rf.setRequestedInformation(RequestedOcFeeGroup::AncillaryData);
    _pricingOpts->_serviceGroupsVec.push_back(rf);

    CPPUNIT_ASSERT(_pricingOpts->isOcOrBaggageDataRequested(RequestedOcFeeGroup::AncillaryData));
  }

  void testIsOcOrBaggageDataRequested_AncillaryData_all()
  {
    _pricingOpts->_processAllGroups = true;
    CPPUNIT_ASSERT(_pricingOpts->isOcOrBaggageDataRequested(RequestedOcFeeGroup::AncillaryData));
  }

  void testIsOcOrBaggageDataRequested_CatalogData()
  {
    RequestedOcFeeGroup rf;
     rf.setRequestedInformation(RequestedOcFeeGroup::CatalogData);
     _pricingOpts->_serviceGroupsVec.push_back(rf);
     CPPUNIT_ASSERT(_pricingOpts->isOcOrBaggageDataRequested(RequestedOcFeeGroup::CatalogData));
  }

  void testIsOcOrBaggageDataRequested_DisclosureData()
  {
    RequestedOcFeeGroup rf;
     rf.setRequestedInformation(RequestedOcFeeGroup::DisclosureData);
     _pricingOpts->_serviceGroupsVec.push_back(rf);
     CPPUNIT_ASSERT(_pricingOpts->isOcOrBaggageDataRequested(RequestedOcFeeGroup::DisclosureData));
  }

  void testIsServiceTypeRequested_found()
  {
    RequestedOcFeeGroup rf;
     rf.addAncillaryServiceType('C');
     rf.addAncillaryServiceType('B');
     _pricingOpts->_serviceGroupsVec.push_back(rf);

     CPPUNIT_ASSERT(_pricingOpts->isServiceTypeRequested('C'));
  }

  void testIsServiceTypeRequested_NotFound()
  {
    RequestedOcFeeGroup rf;
     rf.addAncillaryServiceType('C');
     rf.addAncillaryServiceType('B');
     _pricingOpts->_serviceGroupsVec.push_back(rf);

     CPPUNIT_ASSERT(!_pricingOpts->isServiceTypeRequested('E'));
  }

  void testIsServiceTypeRequested_empty()
  {
    CPPUNIT_ASSERT(!_pricingOpts->isServiceTypeRequested('C'));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingOptionsTest);
}
