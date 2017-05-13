//----------------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Common/Config/ConfigurableValue.h"
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
}

using ::testing::HasSubstr;
using ::testing::Not;

class ConfigurableValueTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle(new PricingTrx);
    _logger = _memHandle(new TestLogger("atseintl.Common.ConfigurableValue"));

    _cv = _memHandle(new ConfigurableValue<uint32_t>("SECTION", "OPTION"));
  }

  void TearDown()  { _memHandle.clear(); }

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  TestLogger* _logger;
  ConfigurableValue<uint32_t>* _cv;
};

TEST_F(ConfigurableValueTest, isDynamic)
{
  ASSERT_FALSE(_cv->isDynamic());
}

TEST_F(ConfigurableValueTest, configure_MissingValue)
{
  TestConfigInitializer::setValue("DIFFERENT OPTION", 123, "SECTION", true, DO_NOT_CONFIGURE_CV);

  _cv->configure();

  ASSERT_TRUE(_cv->isDefault());
  ASSERT_THAT(_logger->str(), HasSubstr("ERROR - Missing value in SECTION:OPTION."
              " Using default data instead."));
}

TEST_F(ConfigurableValueTest, configure_InvalidValue)
{
  TestConfigInitializer::setValue("OPTION", "ABC", "SECTION", true, DO_NOT_CONFIGURE_CV);

  _cv->configure();

  ASSERT_TRUE(_cv->isDefault());
  ASSERT_THAT(_logger->str(), HasSubstr("ERROR - Invalid value ABC in SECTION:OPTION."
               " Using default data instead."));
}

TEST_F(ConfigurableValueTest, configure_ValidValue)
{
  TestConfigInitializer::setValue("OPTION", 123, "SECTION", true, DO_NOT_CONFIGURE_CV);

  _cv->configure();

  ASSERT_FALSE(_cv->isDefault());
  ASSERT_THAT(_logger->str(), Not(HasSubstr("ERROR")));
}

TEST_F(ConfigurableValueTest, getValue_noConfiguredBeforeUsing)
{
  TestConfigInitializer::setValue("OPTION", 123, "SECTION", true, DO_NOT_CONFIGURE_CV);

  ASSERT_TRUE(_cv->isDefault());
  ASSERT_EQ(uint32_t(), _cv->getValue());
  ASSERT_EQ(uint32_t(), _cv->getValue(_trx));
}

TEST_F(ConfigurableValueTest, getValue_ConfiguredBeforeUsing)
{
  TestConfigInitializer::setValue("OPTION", 123, "SECTION");

  ASSERT_FALSE(_cv->isDefault());
  ASSERT_EQ(123, _cv->getValue());
  ASSERT_EQ(123, _cv->getValue(_trx));
}

TEST_F(ConfigurableValueTest, getConfigMsg_dynamicOnlyTrue)
{
  TestConfigInitializer::setValue("OPTION", 123, "SECTION");

  ASSERT_STREQ("", _cv->getConfigMsg(_trx, true).c_str());
}

TEST_F(ConfigurableValueTest, getConfigMsg_dynamicOnlyFlase)
{
  TestConfigInitializer::setValue("OPTION", 321, "SECTION");

  ASSERT_STREQ("SECTION/OPTION/321", _cv->getConfigMsg(_trx, false).c_str());
}

} // ns tse

#endif
