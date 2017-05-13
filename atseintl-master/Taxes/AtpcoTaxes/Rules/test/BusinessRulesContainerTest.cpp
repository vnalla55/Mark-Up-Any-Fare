// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include "DataModel/Services/RulesRecord.h"
#include "Rules/BusinessRulesContainer.h"

namespace tax
{

class BusinessRulesContainerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BusinessRulesContainerTest);

  CPPUNIT_TEST(testCreateRuleNames);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testCreateRuleNames()
  {
    RuleIdNames ruleIdNames;
    BusinessRulesContainer::createRuleNames(ruleIdNames);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(50), ruleIdNames.size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BusinessRulesContainerTest);

} // namespace tax
