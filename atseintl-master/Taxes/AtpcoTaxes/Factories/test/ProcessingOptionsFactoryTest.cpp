// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TaxableUnitTagSet.h"
#include "DataModel/RequestResponse/InputProcessingOptions.h"
#include "DomainDataObjects/ProcessingOptions.h"
#include "Factories/ProcessingOptionsFactory.h"

namespace tax
{

class ProcessingOptionsFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ProcessingOptionsFactoryTest);
  CPPUNIT_TEST(tutFilterAll);
  CPPUNIT_TEST(tutFilterItinOnly);
  CPPUNIT_TEST(tutFilterOcOnly);
  CPPUNIT_TEST_SUITE_END();

  bool
  hasGroup(type::ProcessingGroup processingGroup, const std::vector<type::ProcessingGroup>& set)
  {
    return std::find(set.begin(), set.end(), processingGroup) != set.end();
  }

public:
  void tutFilterAll()
  {
    InputProcessingOptions opts;
    ProcessingOptions ans = ProcessingOptionsFactory().createFromInput(opts);
    const std::vector<type::ProcessingGroup>& groups = ans.getProcessingGroups();
    CPPUNIT_ASSERT(hasGroup(type::ProcessingGroup::OC, groups));
    CPPUNIT_ASSERT(hasGroup(type::ProcessingGroup::OB, groups));
    CPPUNIT_ASSERT(hasGroup(type::ProcessingGroup::ChangeFee, groups));
    CPPUNIT_ASSERT(hasGroup(type::ProcessingGroup::Itinerary, groups));
  }

  void tutFilterItinOnly()
  {
    InputProcessingOptions opts;
    InputApplyOn applGroup; applGroup._group = 3;
    opts.addApplicableGroup(applGroup);

    ProcessingOptions ans = ProcessingOptionsFactory().createFromInput(opts);
    const std::vector<type::ProcessingGroup>& groups = ans.getProcessingGroups();
    CPPUNIT_ASSERT(hasGroup(type::ProcessingGroup::Itinerary, groups));
  }

  void tutFilterOcOnly()
  {
    InputProcessingOptions opts;
    InputApplyOn applGroup; applGroup._group = 0;
    opts.addApplicableGroup(applGroup);

    ProcessingOptions ans = ProcessingOptionsFactory().createFromInput(opts);
    const std::vector<type::ProcessingGroup>& groups = ans.getProcessingGroups();
    CPPUNIT_ASSERT(hasGroup(type::ProcessingGroup::OC, groups));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProcessingOptionsFactoryTest);

}

