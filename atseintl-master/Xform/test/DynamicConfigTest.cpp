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
#include "Common/Config/DynamicConfigurableFlag.h"
#include "DataModel/PricingTrx.h"
#include "Xform/DynamicConfig.h"

namespace tse
{

class DynamicConfigTest : public ::testing::Test
{
public:
  Global::configPtr config;
  Trx* _trx;
  DynamicConfigInput* _input;
  DynamicConfigHandler* _handler;
  TestMemHandle _memHandle;

  void SetUp() {
    config.reset(new ConfigMan);

    config->setValue("AAA", "BBB", "SECTION");
    config->setValue("AAA_BBB_CCC", "BBB", "FALLBACK_SECTION");
    config->setValue("CCC", "DDD", "SECTION");

    Global::dynamicCfg().reset();
    Global::newDynamicCfg().reset();

    _memHandle.create<DynamicConfigurableFlagOn>("SECTION", "AAA", "");
    _memHandle.create<DynamicConfigurableFlagOn>("FALLBACK_SECTION", "AAA_BBB_CCC", "");
    _memHandle(new TestConfigInitializer);
    Global::config().clear();

    TestConfigInitializer::setValue("DYNAMIC_CONFIG_OVERRIDE_ENABLED", "Y", "TSE_SERVER");

    TestConfigInitializer::setValue("AAA", "BBB", "SECTION");

    TestConfigInitializer::setValue("AAA_BBB_CCC", "BBB", "FALLBACK_SECTION");

    DynamicConfigLoader::findLoadableItems();

    _trx = _memHandle(new PricingTrx);
    _trx->dynamicCfg() = config;
    _input = _memHandle(new DynamicConfigInput);
    _handler = _memHandle(new DynamicConfigHandler(*_trx));
  }

  void TearDown() {
    _memHandle.clear();
  }
};

TEST_F(DynamicConfigTest, test_process_ok)
{
  std::string value;

  _input->setName("SECTION::AAA");
  _input->setValue("XYZ");
  _handler->process(*_input);

  config->getValue("AAA", value, "SECTION");
  ASSERT_EQ("XYZ", value);
}

TEST_F(DynamicConfigTest, test_process_ok_default_section)
{
  std::string value;

  _input->setName("AAA_BBB_CCC");
  _input->setValue("XYZ");
  _handler->process(*_input);

  config->getValue("AAA_BBB_CCC", value, "FALLBACK_SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_substitute1)
{
  std::string value;

  _input->setName("AAA BBB CCC");
  _input->setValue("XYZ");
  _handler->process(*_input);

  config->getValue("AAA_BBB_CCC", value, "FALLBACK_SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_substitute2)
{
  std::string value;

  _input->setName("FALLBACK SECTION/AAA BBB_CCC");
  _input->setValue("XYZ");
  _handler->process(*_input);

  config->getValue("AAA_BBB_CCC", value, "FALLBACK_SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_valueInName)
{
  std::string value;

  _input->setName("FALLBACK SECTION/AAA BBB CCC/XYZ");
  _handler->process(*_input);

  config->getValue("AAA_BBB_CCC", value, "FALLBACK_SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_noSubstitute)
{
  std::string value;

  _input->setName("AAA_BBB_CCC");
  _input->setValue("XYZ");
  _input->setSubstitute("N");
  _handler->process(*_input);

  config->getValue("AAA_BBB_CCC", value, "FALLBACK_SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_noSubstitute_backslash)
{
  std::string value;

  _input->setName("FALLBACK_SECTION\\AAA_BBB_CCC");
  _input->setValue("XYZ");
  _input->setSubstitute("F");
  _handler->process(*_input);

  config->getValue("AAA_BBB_CCC", value, "FALLBACK_SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_colons)
{
  std::string value;

  _input->setName("FALLBACK_SECTION::AAA_BBB_CCC");
  _input->setValue("XYZ");
  _handler->process(*_input);

  config->getValue("AAA_BBB_CCC", value, "FALLBACK_SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_backslash)
{
  std::string value;

  _input->setName("FALLBACK_SECTION\\AAA_BBB_CCC");
  _input->setValue("XYZ");
  _handler->process(*_input);

  config->getValue("AAA_BBB_CCC", value, "FALLBACK_SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_optional)
{
  std::string value;

  _input->setName("SECTION::AAA");
  _input->setValue("XYZ");
  _input->setOptional("T");
  _handler->process(*_input);

  config->getValue("AAA", value, "SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_noOptional)
{
  std::string value;

  _input->setName("SECTION::AAA");
  _input->setValue("XYZ");
  _input->setOptional("F");
  _handler->process(*_input);

  config->getValue("AAA", value, "SECTION");
  ASSERT_EQ("XYZ", value);
  ASSERT_TRUE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_empty)
{
  std::string value;

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_noValue)
{
  std::string value;

  _input->setName("FALLBACK SECTION\\AAA BBB CCC");

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_twoValues)
{
  std::string value;

  _input->setName("FALLBACK SECTION\\AAA BBB CCC\\XYZ");
  _input->setValue("XYZ");

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_invalidSubstitute1)
{
  std::string value;

  _input->setName("FALLBACK_SECTION\\AAA_BBB_CCC");
  _input->setValue("XYZ");
  _input->setSubstitute("");

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_invalidSubstitute2)
{
  std::string value;

  _input->setName("FALLBACK_SECTION\\AAA_BBB_CCC");
  _input->setValue("XYZ");
  _input->setSubstitute("n");

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_invalidOptional1)
{
  std::string value;

  _input->setName("FALLBACK_SECTION\\AAA_BBB_CCC");
  _input->setValue("XYZ");
  _input->setOptional("");

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_invalidOptional2)
{
  std::string value;

  _input->setName("FALLBACK_SECTION\\AAA_BBB_CCC");
  _input->setValue("XYZ");
  _input->setOptional("7");

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_noSubstitute_slash)
{
  std::string value;

  _input->setName("FALLBACK_SECTION/AAA_BBB_CCC");
  _input->setValue("XYZ");
  _input->setSubstitute("N");

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_noSubstitute_space)
{
  std::string value;

  _input->setName("FALLBACK_SECTION\\AAA BBB CCC");
  _input->setValue("XYZ");
  _input->setSubstitute("F");

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_ok_error_optional_prevent)
{
  std::string value;

  _input->setOptional("Y");

  _handler->process(*_input);
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

TEST_F(DynamicConfigTest, test_process_error_noOptional)
{
  std::string value;

  _input->setOptional("N");

  ASSERT_ANY_THROW(_handler->process(*_input));
  ASSERT_FALSE(_trx->isDynamicCfgOverriden());
}

}
