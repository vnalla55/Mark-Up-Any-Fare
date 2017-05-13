// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2016
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
#ifdef CONFIG_HIERARCHY_REFACTOR

#include <gtest/gtest.h>

#include "test/include/GtestHelperMacros.h"

#include "Common/Config/ConfigBundle.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Config/DynamicConfigurableValue.h"

namespace tse
{
namespace
{
constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();
}

class ConfigBundleTest : public ::testing::Test
{
public:
  void SetUp()
  {
    allocateAllConfigBundles();
  }

protected:
  DynamicConfigurableValue<uint32_t> dummy{"DUMMY", "VALUE"};
};

TEST_F(ConfigBundleTest, test_allocateAllConfigBundles_single)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");

  allocateAllConfigBundles();

  EXPECT_NE(INVALID_INDEX, value.index());
}

TEST_F(ConfigBundleTest, test_allocateAllConfigBundles_many_sameType_diffAllocation)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<std::string> value2("SEC2", "NAM2");

  allocateAllConfigBundles();

  EXPECT_NE(INVALID_INDEX, value.index());
  EXPECT_NE(INVALID_INDEX, value2.index());
  EXPECT_NE(value.index(), value2.index());
}

TEST_F(ConfigBundleTest, test_allocateAllConfigBundles_many_sameType_sameAllocation)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<std::string> value2("SEC", "NAM");

  allocateAllConfigBundles();

  EXPECT_NE(INVALID_INDEX, value.index());
  EXPECT_EQ(value.index(), value2.index());
}

TEST_F(ConfigBundleTest, test_allocateAllConfigBundles_many_diffType)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<bool> value2("SEC2", "NAM2");

  allocateAllConfigBundles();

  EXPECT_NE(INVALID_INDEX, value.index());
  EXPECT_NE(INVALID_INDEX, value2.index());
}

TEST_F(ConfigBundleTest, test_size)
{
  const uint32_t sizeBefore = DynamicConfigurableValuesBundlePool<std::string>::size();

  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  EXPECT_EQ(1, DynamicConfigurableValuesBundlePool<std::string>::size() - sizeBefore);
}

TEST_F(ConfigBundleTest, test_size_many_sameType_diffAllocation)
{
  const uint32_t sizeBefore = DynamicConfigurableValuesBundlePool<std::string>::size();

  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<std::string> value2("SEC2", "NAM2");
  allocateAllConfigBundles();

  EXPECT_EQ(2, DynamicConfigurableValuesBundlePool<std::string>::size() - sizeBefore);
}

TEST_F(ConfigBundleTest, test_size_many_sameType_sameAllocation)
{
  const uint32_t sizeBefore = DynamicConfigurableValuesBundlePool<std::string>::size();

  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<std::string> value2("SEC", "NAM");
  allocateAllConfigBundles();

  EXPECT_EQ(1, DynamicConfigurableValuesBundlePool<std::string>::size() - sizeBefore);
}

TEST_F(ConfigBundleTest, test_size_many_diffType)
{
  const uint32_t sizeBefore = DynamicConfigurableValuesBundlePool<std::string>::size();
  const uint32_t size2Before = DynamicConfigurableValuesBundlePool<bool>::size();

  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<bool> value2("SEC2", "NAM2");
  allocateAllConfigBundles();

  EXPECT_EQ(1, DynamicConfigurableValuesBundlePool<std::string>::size() - sizeBefore);
  EXPECT_EQ(1, DynamicConfigurableValuesBundlePool<bool>::size() - size2Before);
}

TEST_F(ConfigBundleTest, test_bool_emptyBundle)
{
  ConfigBundle empty;

  EXPECT_FALSE(static_cast<bool>(empty));
  EXPECT_FALSE(!!empty);
}

TEST_F(ConfigBundleTest, test_bool_allocatedBundle)
{
  ConfigBundle bundle;
  bundle.allocate();

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_TRUE(!!bundle);
}

TEST_F(ConfigBundleTest, test_basicOps_emptyBundle)
{
  ConfigBundle empty;

  ConfigBundle another = empty;
  EXPECT_FALSE(static_cast<bool>(another));

  empty = another;
  EXPECT_FALSE(static_cast<bool>(empty));

  empty = std::move(another);
  EXPECT_FALSE(static_cast<bool>(empty));
  EXPECT_FALSE(static_cast<bool>(another));

  ConfigBundle yetAnother{std::move(empty)};
  EXPECT_FALSE(static_cast<bool>(yetAnother));
  EXPECT_FALSE(static_cast<bool>(empty));
}

TEST_F(ConfigBundleTest, test_basicOps_allocatedBundle)
{
  ConfigBundle bundle;
  bundle.allocate();

  ConfigBundle another = bundle;
  EXPECT_TRUE(static_cast<bool>(another));
  EXPECT_EQ(&bundle.get<uint32_t>(dummy.index()), &another.get<uint32_t>(dummy.index()));

  bundle = another;
  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_EQ(&bundle.get<uint32_t>(dummy.index()), &another.get<uint32_t>(dummy.index()));

  uint32_t* before = &bundle.get<uint32_t>(dummy.index());
  bundle.clear();
  bundle = std::move(another);
  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_FALSE(static_cast<bool>(another));
  EXPECT_EQ(before, &bundle.get<uint32_t>(dummy.index()));

  ConfigBundle yetAnother{std::move(bundle)};
  EXPECT_TRUE(static_cast<bool>(yetAnother));
  EXPECT_FALSE(static_cast<bool>(bundle));
  EXPECT_EQ(before, &yetAnother.get<uint32_t>(dummy.index()));
}

TEST_F(ConfigBundleTest, test_clear_emptyBundle)
{
  ConfigBundle empty;
  empty.clear();

  EXPECT_FALSE(static_cast<bool>(empty));
}

TEST_F(ConfigBundleTest, test_clear_allocatedBundle)
{
  ConfigBundle bundle;
  bundle.allocate();
  bundle.clear();

  EXPECT_FALSE(static_cast<bool>(bundle));
}

TEST_F(ConfigBundleTest, test_get)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  ConfigBundle bundle;
  bundle.allocate();

  bundle.get<std::string>(value.index()) = "abc";
  EXPECT_EQ("abc", bundle.get<std::string>(value.index()));
  EXPECT_EQ("abc", static_cast<const ConfigBundle&>(bundle).get<std::string>(value.index()));
}

TEST_F(ConfigBundleTest, test_get_many_sameTypes)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<std::string> value2("SEC2", "NAM2");
  allocateAllConfigBundles();

  ConfigBundle bundle;
  bundle.allocate();

  bundle.get<std::string>(value.index()) = "abc";
  bundle.get<std::string>(value2.index()) = "def";

  EXPECT_EQ("abc", bundle.get<std::string>(value.index()));
  EXPECT_EQ("abc", static_cast<const ConfigBundle&>(bundle).get<std::string>(value.index()));

  EXPECT_EQ("def", bundle.get<std::string>(value2.index()));
  EXPECT_EQ("def", static_cast<const ConfigBundle&>(bundle).get<std::string>(value2.index()));
}

TEST_F(ConfigBundleTest, test_get_many_diffTypes)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<bool> value2("SEC2", "NAM2");
  allocateAllConfigBundles();

  ConfigBundle bundle;
  bundle.allocate();

  bundle.get<std::string>(value.index()) = "abc";
  bundle.get<bool>(value2.index()) = true;

  EXPECT_EQ("abc", bundle.get<std::string>(value.index()));
  EXPECT_EQ("abc", static_cast<const ConfigBundle&>(bundle).get<std::string>(value.index()));

  EXPECT_EQ(true, bundle.get<bool>(value2.index()));
  EXPECT_EQ(true, static_cast<const ConfigBundle&>(bundle).get<bool>(value2.index()));
}

TEST_F(ConfigBundleTest, test_clone)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  ConfigBundle bundle;
  bundle.allocate();

  bundle.get<std::string>(value.index()) = "abc";

  ConfigBundle another = bundle.clone();

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_TRUE(static_cast<bool>(another));
  EXPECT_NE(&bundle.get<uint32_t>(dummy.index()), &another.get<uint32_t>(dummy.index()));
  EXPECT_EQ("abc", another.get<std::string>(value.index()));

  another.get<std::string>(value.index()) = "def";

  EXPECT_EQ("abc", bundle.get<std::string>(value.index()));
  EXPECT_EQ("def", another.get<std::string>(value.index()));
}

TEST_F(ConfigBundleTest, test_makeUnique_alreadyUnique)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  ConfigBundle bundle;
  bundle.allocate();

  bundle.get<std::string>(value.index()) = "abc";
  uint32_t* before = &bundle.get<uint32_t>(dummy.index());

  bundle.makeUnique();

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_EQ(before, &bundle.get<uint32_t>(dummy.index()));
  EXPECT_EQ("abc", bundle.get<std::string>(value.index()));
}

TEST_F(ConfigBundleTest, test_makeUnique_notUnique)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  ConfigBundle bundle;
  bundle.allocate();

  ConfigBundle bundle2 = bundle;

  bundle2.get<std::string>(value.index()) = "abc";
  uint32_t* before = &bundle2.get<uint32_t>(dummy.index());

  bundle2.makeUnique();

  EXPECT_TRUE(static_cast<bool>(bundle2));
  EXPECT_NE(before, &bundle2.get<uint32_t>(dummy.index()));
  EXPECT_EQ("abc", bundle2.get<std::string>(value.index()));
}

TEST_F(ConfigBundleTest, test_fill_notAllocated)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  ConfigMan config;
  config.setValue("NAM", "123", "SEC", true);

  ConfigBundle bundle;

  bundle.fill(config);

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_EQ("123", bundle.get<std::string>(value.index()));
}

TEST_F(ConfigBundleTest, test_fill_allocated)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  ConfigMan config;
  config.setValue("NAM", "123", "SEC", true);

  ConfigBundle bundle;
  bundle.allocate();

  bundle.fill(config);

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_EQ("123", bundle.get<std::string>(value.index()));
}

TEST_F(ConfigBundleTest, test_fill_twice)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  ConfigMan config;
  config.setValue("NAM", "123", "SEC", true);

  ConfigBundle bundle;
  bundle.allocate();

  bundle.fill(config);
  config.setValue("NAM", "456", "SEC", true);
  bundle.fill(config);

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_EQ("456", bundle.get<std::string>(value.index()));
}

TEST_F(ConfigBundleTest, test_update_invalidName)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  ConfigMan config;
  config.setValue("NAM", "123", "SEC", true);

  ConfigBundle bundle;
  bundle.allocate();

  bundle.update(config, "INVALID", "NAME");

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_EQ("", bundle.get<std::string>(value.index()));
}

TEST_F(ConfigBundleTest, test_update_validName)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  allocateAllConfigBundles();

  ConfigMan config;
  config.setValue("NAM", "123", "SEC", true);

  ConfigBundle bundle;
  bundle.allocate();

  bundle.update(config, "SEC", "NAM");

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_EQ("123", bundle.get<std::string>(value.index()));
}

TEST_F(ConfigBundleTest, test_update_many_sameType)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<std::string> value2("SEC2", "NAM2");
  allocateAllConfigBundles();

  ConfigMan config;
  config.setValue("NAM", "123", "SEC", true);

  ConfigBundle bundle;
  bundle.allocate();

  bundle.update(config, "SEC", "NAM");

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_EQ("123", bundle.get<std::string>(value.index()));
  EXPECT_EQ("", bundle.get<std::string>(value2.index()));
}

TEST_F(ConfigBundleTest, test_update_many_diffType)
{
  DynamicConfigurableValue<std::string> value("SEC", "NAM");
  DynamicConfigurableValue<bool> value2("SEC2", "NAM2");
  allocateAllConfigBundles();

  ConfigMan config;
  config.setValue("NAM", "123", "SEC", true);

  ConfigBundle bundle;
  bundle.allocate();

  bundle.update(config, "SEC", "NAM");

  EXPECT_TRUE(static_cast<bool>(bundle));
  EXPECT_EQ("123", bundle.get<std::string>(value.index()));
  EXPECT_EQ(false, bundle.get<bool>(value2.index()));
}

}

#endif
