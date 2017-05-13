//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include <boost/range.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseEnums.h"
#include "DataModel/FlexFares/ValidationStatus.h"
#include "Rules/CategoryValidationObserver.h"
#include "Rules/RuleValidationContext.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <memory>

namespace tse
{
// ==================================
// TEST
// ==================================
class CategoryValidationObserverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CategoryValidationObserverTest);
  CPPUNIT_TEST(testCategoryValidation);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testCategoryValidation()
  {
    RuleValidationMonitor rvm;
    CategoryValidationObserver cvo(rvm);

    RuleValidationContext rvc;
    rvc._contextType = RuleValidationContext::FARE_MARKET;

    std::shared_ptr<flexFares::ValidationStatus> vs(new flexFares::ValidationStatus());

    const uint16_t catNumber = PENALTIES_RULE;
    const Record3ReturnTypes ret = PASS;

    Record3ReturnTypes flexRuleNoAdvance =
        vs->getStatusForAttribute<flexFares::NO_ADVANCE_PURCHASE>();
    Record3ReturnTypes flexRuleNoMinMax = vs->getStatusForAttribute<flexFares::NO_MIN_MAX_STAY>();

    cvo.update(rvc, vs, catNumber, ret);

    CPPUNIT_ASSERT_EQUAL(vs->getStatusForAttribute<flexFares::NO_PENALTIES>(), PASS);
    CPPUNIT_ASSERT_EQUAL(vs->getStatusForAttribute<flexFares::NO_ADVANCE_PURCHASE>(),
                         flexRuleNoAdvance);
    CPPUNIT_ASSERT_EQUAL(vs->getStatusForAttribute<flexFares::NO_MIN_MAX_STAY>(), flexRuleNoMinMax);
  }
}; // CategoryValidationObserverTest

CPPUNIT_TEST_SUITE_REGISTRATION(CategoryValidationObserverTest);
} // tse
