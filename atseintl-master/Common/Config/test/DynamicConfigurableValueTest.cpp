//----------------------------------------------------------------------------
//  Copyright Sabre 2011
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

#ifdef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/DynamicConfigurableValue.h"
#include "DataModel/PricingTrx.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

namespace
{
const bool DO_NOT_CONFIGURE_CV = false;
const char VALUE_TRUE = 'T';
const char VALUE_FALSE = 'N';
}

using ::testing::HasSubstr;
using ::testing::Not;

class DynamicConfigurableValueTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _dcv = _memHandle(new DynamicConfigurableValue<bool>("SECTION", "OPTION", false));
    _memHandle.create<TestConfigInitializer>();
     Global::configUpdateInProgress() = false;

    _trx = _memHandle(new PricingTrx);

    _logger = _memHandle(new TestLogger("atseintl.Common.ConfigurableValue"));
    _tseServerlogger = _memHandle(new TestLogger("atseintl.Server.TseServer"));

  }

  void TearDown()  { _memHandle.clear(); }

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  TestLogger* _logger;
  TestLogger* _tseServerlogger;

  DynamicConfigurableValue<bool>* _dcv;
};

TEST_F(DynamicConfigurableValueTest, isDynamic)
{
  ASSERT_TRUE(_dcv->isDynamic());
}

TEST_F(DynamicConfigurableValueTest, configure_MissingValue)
{
  TestConfigInitializer::setValue("DIFFERENT OPTION", VALUE_TRUE, "SECTION");

  Global::configBundle().load(std::memory_order_relaxed)->
      update(*Global::dynamicCfg(), "SECTION", "OPTION");

  ASSERT_THAT(_logger->str(), HasSubstr("ERROR - Missing value in SECTION:OPTION."
              " Using default data instead."));
}

TEST_F(DynamicConfigurableValueTest, configure_InvalidValue)
{
  TestConfigInitializer::setValue("OPTION", "ABC", "SECTION");

  ASSERT_THAT(_logger->str(), HasSubstr("ERROR - Invalid value ABC in SECTION:OPTION."
               " Using default data instead."));
}

TEST_F(DynamicConfigurableValueTest, configure_ValidValue)
{
  TestConfigInitializer::setValue("OPTION", VALUE_TRUE, "SECTION");

  ASSERT_THAT(_logger->str(), Not(HasSubstr("ERROR")));
}

TEST_F(DynamicConfigurableValueTest, getValue_classMemberValueReturned)
{
  TestConfigInitializer::setValue("OPTION", VALUE_TRUE, "SECTION");

  ASSERT_TRUE(_dcv->getValue(_trx));
}

TEST_F(DynamicConfigurableValueTest, getConfigMsg_dynamicOnlyTrue)
{
  TestConfigInitializer::setValue("OPTION", VALUE_TRUE, "SECTION");

  ASSERT_STREQ("SECTION/OPTION/Y", _dcv->getConfigMsg(_trx, true).c_str());
}

TEST_F(DynamicConfigurableValueTest, getConfigMsg_dynamicOnlyFalse)
{
  TestConfigInitializer::setValue("OPTION", VALUE_TRUE, "SECTION");

  ASSERT_STREQ("SECTION/OPTION/Y", _dcv->getConfigMsg(_trx, false).c_str());
}


} // ns tse

#endif
