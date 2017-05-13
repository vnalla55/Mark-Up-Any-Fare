#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "Common/Config/ConfigurableValuesPool.h"
#include "Common/Config/DynamicConfigLoader.h"
#include "Common/Config/DynamicConfigurableDate.h"
#include "Server/TseServer.h"

namespace tse
{
class TseServerMock : public TseServer
{
public:
  static tse::ConfigMan* getConfig() { return Global::_configMan; }
  static void setConfig(tse::ConfigMan* configMan) { Global::_configMan = configMan; }
};

namespace
{
bool
isEqual(const tse::ConfigMan::NameValue& nv1, const tse::ConfigMan::NameValue& nv2)
{
  return ((nv1.group == nv2.group) && (nv1.name == nv2.name) && (nv2.value == nv2.value));
}
}

class DynamicConfigLoaderTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(DynamicConfigLoaderTest);
  CPPUNIT_TEST(testLoadableItems_noDynamic);
  CPPUNIT_TEST(testLoadableItems_dynamic);
  CPPUNIT_TEST(testLoadableItems_2_dynamic);
  CPPUNIT_TEST(testGenerateConfig);
  CPPUNIT_TEST(testUpdateDynamicEntries_noDynamic_noChange);
  CPPUNIT_TEST(testUpdateDynamicEntries_noDynamic_change);
  CPPUNIT_TEST(testUpdateDynamicEntries_dynamic_noChange);
  CPPUNIT_TEST(testUpdateDynamicEntries_dynamic_change);
  CPPUNIT_TEST(testUpdateDynamicConfig);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    TseServerMock::setConfig(&_cfg);
    TestConfigInitializer::setValue("MIN_CONFIG_UPDATE_INTERVAL", "0", "TSE_SERVER");
    _cfgFile = "tseserver.cfg";
    _testCfgFile = "tseserver.test.cfg";

    // to check cast error for negative values
    ::setenv("TSE_CONFIG_MAN_NO_ASSERTION", "1", 0);
  }

  void tearDown() { unlink(_testCfgFile.c_str()); }

  tse::ConfigMan::NameValue
  createNameValue(const std::string& group, const std::string& name, const std::string& value)
  {
    tse::ConfigMan::NameValue nv;
    nv.group = group;
    nv.name = name;
    nv.value = value;
    return nv;
  }

  void getDynamicLoaderInstance(std::string& cfgFile)
  {
    std::string cfgFileOverrideGroup = "TSE_SERVER";
    std::vector<tse::ConfigMan::NameValue> cfgOverrides;

    DynamicConfigLoader* loader(
        new DynamicConfigLoader(cfgFile,
                                cfgFileOverrideGroup,
                                "OVERRIDE_CFGS",
                                "|",
                                cfgOverrides,
                                TseServer::loadConfigWithOverrides,
                                0));
    DynamicConfigLoader::_instance = loader;
    _dynamicConfigLoader = DynamicConfigLoader::_instance;
  }

  void testGenerateConfig()
  {
    _cfg.read(_cfgFile);
    DynamicConfigurableEffectiveDate specialFareTypeGroupDate(
        "PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", DateTime());

    std::vector<tse::ConfigMan::NameValue> nv;

    nv.push_back(
        createNameValue("PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "1888-01-10"));

    nv.push_back(createNameValue("TSE_SERVER", "OVERRIDE_CFGS", ""));

    _cfg.setValues(nv, true);

    getDynamicLoaderInstance(_cfgFile);

    std::vector<tse::ConfigMan::NameValue> nv1;

    CPPUNIT_ASSERT(_cfg.getValues(nv1));
    CPPUNIT_ASSERT(nv1.size());

    std::vector<tse::ConfigMan::NameValue> nv2;
    tse::ConfigMan& dynamicCfg = *Global::dynamicCfg();
    CPPUNIT_ASSERT(dynamicCfg.getValues(nv2));

    CPPUNIT_ASSERT(nv2.size());

    CPPUNIT_ASSERT_EQUAL(nv1.size(), nv2.size());

    bool equal = std::equal(nv1.begin(), nv1.end(), nv2.begin(), isEqual);
    CPPUNIT_ASSERT(equal);
  }

  void testLoadableItems_noDynamic()
  {
    std::vector<ConfigMan::NameValue> nv;
    _cfg.clear();
    getDynamicLoaderInstance(_testCfgFile);

    nv.push_back(
        createNameValue("PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "2010-01-10"));
    _cfg.setValues(nv);

    _dynamicConfigLoader->findLoadableItems();

    CPPUNIT_ASSERT(!DynamicConfigLoader::isDynamicLoadable(
                       "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "PRICING_SVC"));
  }

  void testLoadableItems_dynamic()
  {
    std::vector<ConfigMan::NameValue> nv;
    _cfg.clear();
    getDynamicLoaderInstance(_testCfgFile);

    nv.push_back(
        createNameValue("PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "2010-01-10"));
    _cfg.setValues(nv);

    {
      DynamicConfigurableEffectiveDate specialFareTypeGroupDate(
          "PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", DateTime());

      _dynamicConfigLoader->findLoadableItems();

      CPPUNIT_ASSERT(DynamicConfigLoader::isDynamicLoadable(
          "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "PRICING_SVC"));
    }

    _dynamicConfigLoader->findLoadableItems();

    CPPUNIT_ASSERT(!DynamicConfigLoader::isDynamicLoadable(
                       "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "PRICING_SVC"));
  }

  void testLoadableItems_2_dynamic()
  {
    std::vector<ConfigMan::NameValue> nv;
    _cfg.clear();
    getDynamicLoaderInstance(_testCfgFile);

    nv.push_back(
        createNameValue("PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "2010-01-10"));
    nv.push_back(createNameValue(
        "PRICING_SVC", "MIN_FARE_SPECIAL_FARE_HIP_SKIP_NORMAL_ACTIVE_DATE", "2015-11-11"));
    _cfg.setValues(nv);

    {
      DynamicConfigurableEffectiveDate specialFareTypeGroupDate(
          "PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", DateTime());

      _dynamicConfigLoader->findLoadableItems();

      CPPUNIT_ASSERT(DynamicConfigLoader::isDynamicLoadable(
          "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "PRICING_SVC"));
      CPPUNIT_ASSERT(!DynamicConfigLoader::isDynamicLoadable(
                         "MIN_FARE_SPECIAL_FARE_HIP_SKIP_NORMAL_ACTIVE_DATE", "PRICING_SVC"));

      {
        DynamicConfigurableEffectiveDate minFareSpecialFareHipSkipNormalDate(
            "PRICING_SVC", "MIN_FARE_SPECIAL_FARE_HIP_SKIP_NORMAL_ACTIVE_DATE", DateTime());

        _dynamicConfigLoader->findLoadableItems();

        CPPUNIT_ASSERT(DynamicConfigLoader::isDynamicLoadable(
            "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "PRICING_SVC"));
        CPPUNIT_ASSERT(DynamicConfigLoader::isDynamicLoadable(
            "MIN_FARE_SPECIAL_FARE_HIP_SKIP_NORMAL_ACTIVE_DATE", "PRICING_SVC"));
      }
    }

    _dynamicConfigLoader->findLoadableItems();

    CPPUNIT_ASSERT(!DynamicConfigLoader::isDynamicLoadable(
                       "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "PRICING_SVC"));
    CPPUNIT_ASSERT(!DynamicConfigLoader::isDynamicLoadable(
                       "MIN_FARE_SPECIAL_FARE_HIP_SKIP_NORMAL_ACTIVE_DATE", "PRICING_SVC"));
  }

  void testUpdateDynamicEntries_noDynamic_noChange()
  {
    tse::ConfigMan sourceCfg;
    tse::ConfigMan destCfg;

    TseServerMock::setConfig(&sourceCfg);

    sourceCfg.read(_cfgFile);
    destCfg.read(_cfgFile);

    getDynamicLoaderInstance(_testCfgFile);
    _dynamicConfigLoader->findLoadableItems();

    std::string output;
    int count = _dynamicConfigLoader->updateDynamicEntries(sourceCfg, destCfg, output);

    CPPUNIT_ASSERT_EQUAL(0, count);
  }

  void testUpdateDynamicEntries_noDynamic_change()
  {
    tse::ConfigMan sourceCfg;
    tse::ConfigMan destCfg;

    TseServerMock::setConfig(&sourceCfg);

    sourceCfg.read(_cfgFile);
    destCfg.read(_cfgFile);

    sourceCfg.setValue(
        "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "2012-04-01", "PRICING_SVC", true);

    getDynamicLoaderInstance(_testCfgFile);
    _dynamicConfigLoader->findLoadableItems();

    std::string output;
    int count = _dynamicConfigLoader->updateDynamicEntries(sourceCfg, destCfg, output);

    CPPUNIT_ASSERT_EQUAL(0, count);

    std::string newValue;
    destCfg.getValue("SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", newValue, "PRICING_SVC");
    CPPUNIT_ASSERT_EQUAL(std::string("2010-04-01"), newValue);
  }

  void testUpdateDynamicEntries_dynamic_noChange()
  {
    tse::ConfigMan sourceCfg;
    tse::ConfigMan destCfg;

    TseServerMock::setConfig(&sourceCfg);

    sourceCfg.read(_cfgFile);
    destCfg.read(_cfgFile);

    DynamicConfigurableEffectiveDate specialFareTypeGroupDate(
        "PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", DateTime());

    getDynamicLoaderInstance(_testCfgFile);
    _dynamicConfigLoader->findLoadableItems();

    std::string output;
    int count = _dynamicConfigLoader->updateDynamicEntries(sourceCfg, destCfg, output);

    CPPUNIT_ASSERT_EQUAL(0, count);
  }

  void testUpdateDynamicEntries_dynamic_change()
  {
    tse::ConfigMan sourceCfg;
    tse::ConfigMan destCfg;

    TseServerMock::setConfig(&sourceCfg);

    sourceCfg.read(_cfgFile);
    destCfg.read(_cfgFile);

    sourceCfg.setValue(
        "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "2012-04-01", "PRICING_SVC", true);

    DynamicConfigurableEffectiveDate specialFareTypeGroupDate(
        "PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", DateTime());

    getDynamicLoaderInstance(_testCfgFile);
    _dynamicConfigLoader->findLoadableItems();

    std::string output;
    int count = _dynamicConfigLoader->updateDynamicEntries(sourceCfg, destCfg, output);

    CPPUNIT_ASSERT_EQUAL(1, count);

    std::string newValue;
    destCfg.getValue("SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", newValue, "PRICING_SVC");
    CPPUNIT_ASSERT_EQUAL(std::string("2012-04-01"), newValue);
  }

  void testUpdateDynamicConfig()
  {
    _cfg.clear();
    _cfg.read(_cfgFile);

    DynamicConfigurableEffectiveDate specialFareTypeGroupDate(
        "PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", DateTime());

    std::vector<tse::ConfigMan::NameValue> nv;

    nv.push_back(
        createNameValue("PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "1888-01-10"));

    nv.push_back(createNameValue("TSE_SERVER", "OVERRIDE_CFGS", ""));

    _cfg.setValues(nv, true);
    _cfg.write(_testCfgFile);

    getDynamicLoaderInstance(_testCfgFile);
    CPPUNIT_ASSERT(_dynamicConfigLoader->findLoadableItems());

    std::string output;

    Global::configUpdateInProgress() = true;
    CPPUNIT_ASSERT(!_dynamicConfigLoader->updateDynamicConfig(output));

    _dynamicConfigLoader->generateNewConfig(_cfg, Global::dynamicCfg());

    Global::configUpdateInProgress() = false;
    CPPUNIT_ASSERT(!_dynamicConfigLoader->updateDynamicConfig(output));
    CPPUNIT_ASSERT(!Global::configUpdateInProgress());

    nv.clear();
    nv.push_back(
        createNameValue("PRICING_SVC", "SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", "2222-01-10"));
    _cfg.setValues(nv, true);
    _cfg.write(_testCfgFile);

    CPPUNIT_ASSERT(_dynamicConfigLoader->updateDynamicConfig(output));
    CPPUNIT_ASSERT(Global::configUpdateInProgress());
    CPPUNIT_ASSERT(Global::newDynamicCfg().use_count() > 0);

    tse::ConfigMan& newCfg = *Global::newDynamicCfg();

    std::string newValue;
    newCfg.getValue("SPECIAL_FARE_TYPE_GROUP_ACTIVATION_DATE", newValue, "PRICING_SVC");
    CPPUNIT_ASSERT_EQUAL(std::string("2222-01-10"), newValue);
  }

  // Member variables
  DynamicConfigLoader* _dynamicConfigLoader;
  tse::ConfigMan _cfg;
  std::string _cfgFile;
  std::string _testCfgFile;
};

} // namespace tse

CPPUNIT_TEST_SUITE_REGISTRATION(tse::DynamicConfigLoaderTest);
