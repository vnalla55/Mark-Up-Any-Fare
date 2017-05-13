// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/DynamicConfigLoader.h"
#include "Common/Config/DynamicConfigurableString.h"

namespace tse
{

class DynamicConfigLoaderTest : public ::testing::Test
{
public:
  Global::configPtr original, config;
  TestMemHandle _memHandle;

  void SetUp()
  {
    original.reset(new ConfigMan);

    original->setValue("AAA", "BBB", "SECTION");
    original->setValue("CCC", "DDD", "SECTION");

    config = original;

    _memHandle.create<DynamicConfigurableString>("SECTION", "AAA", "");
    _memHandle(new TestConfigInitializer);
    Global::config().clear();
    TestConfigInitializer::setValue("AAA", "BBB", "SECTION");

    DynamicConfigLoader::findLoadableItems();
  }

  void TearDown()
  {
    _memHandle.clear();
  }
};

TEST_F(DynamicConfigLoaderTest, test_overrideConfigValue)
{
  std::string value;
  ASSERT_TRUE(DynamicConfigLoader::overrideConfigValue(config, "SECTION", "AAA", "XYZ"));

  ASSERT_EQ(original.get(), config.get());

  config->getValue("AAA", value, "SECTION");
  ASSERT_EQ("XYZ", value);

  config->getValue("CCC", value, "SECTION");
  ASSERT_EQ("DDD", value);
}

TEST_F(DynamicConfigLoaderTest, test_overrideConfigValue_dynamic)
{
  std::string value;
  Global::dynamicCfg() = original;

  ASSERT_TRUE(DynamicConfigLoader::overrideConfigValue(config, "SECTION", "AAA", "XYZ"));

  ASSERT_NE(original.get(), config.get());

  config->getValue("AAA", value, "SECTION");
  ASSERT_EQ("XYZ", value);

  original->getValue("AAA", value, "SECTION");
  ASSERT_EQ("BBB", value);

  config->getValue("CCC", value, "SECTION");
  ASSERT_EQ("DDD", value);
}

TEST_F(DynamicConfigLoaderTest, test_overrideConfigValue_newDynamic)
{
  std::string value;
  Global::newDynamicCfg() = original;

  ASSERT_TRUE(DynamicConfigLoader::overrideConfigValue(config, "SECTION", "AAA", "XYZ"));

  ASSERT_NE(original.get(), config.get());

  config->getValue("AAA", value, "SECTION");
  ASSERT_EQ("XYZ", value);

  original->getValue("AAA", value, "SECTION");
  ASSERT_EQ("BBB", value);

  config->getValue("CCC", value, "SECTION");
  ASSERT_EQ("DDD", value);
}

TEST_F(DynamicConfigLoaderTest, test_overrideConfigValue_notDynamicLoadable)
{
  std::string value;
  ASSERT_FALSE(DynamicConfigLoader::overrideConfigValue(config, "SECTION", "CCC", "XYZ"));

  ASSERT_EQ(original.get(), config.get());

  config->getValue("AAA", value, "SECTION");
  ASSERT_EQ("BBB", value);

  config->getValue("CCC", value, "SECTION");
  ASSERT_EQ("DDD", value);
}

TEST_F(DynamicConfigLoaderTest, test_overrideConfigValue_noSuchOption)
{
  std::string value;
  ASSERT_FALSE(DynamicConfigLoader::overrideConfigValue(config, "SECTION", "QQQ", "XYZ"));

  ASSERT_EQ(original.get(), config.get());

  config->getValue("AAA", value, "SECTION");
  ASSERT_EQ("BBB", value);

  config->getValue("CCC", value, "SECTION");
  ASSERT_EQ("DDD", value);

  std::vector<ConfigMan::NameValue> namesValues;
  config->getValues(namesValues, "SECTION");
  ASSERT_EQ(size_t(2), namesValues.size());
}

}
