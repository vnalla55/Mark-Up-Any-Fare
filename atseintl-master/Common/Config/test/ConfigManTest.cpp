#include <ostream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/ConfigMan.h"

namespace tse
{

namespace
{

class ConfigManType : public ConfigManTemplate<detail::EmptyMutex, detail::EmptyErrorHandler>
{
public:
  explicit ConfigManType(const std::string& defaultGroup = DefaultGroup())
    : ConfigManTemplate<detail::EmptyMutex, detail::EmptyErrorHandler>(defaultGroup)
  {
  }
};

struct ThrowErrorHandler
{
  static void onCastError(const std::string& /*name*/,
                          const std::string& /*value*/,
                          const std::string& /*group*/)
  {
    throw std::runtime_error("Cast error");
  }
};

std::string
ToString(const std::vector<tse::ConfigManType::NameValue>& vals)
{
  std::ostringstream out;
  for (std::vector<tse::ConfigManType::NameValue>::const_iterator i = vals.begin(); i != vals.end();
       ++i)
  {
    const tse::ConfigManType::NameValue& v = *i;
    out << "(" << v.name << "," << v.value << "," << v.group << ")";
  }

  return out.str();
}

std::string
ToString(const tse::ConfigManType::Values& values)
{
  std::ostringstream out;
  for (tse::ConfigManType::Values::const_iterator b = values.begin(); b != values.end(); ++b)
  {
    out << *b << ", ";
  }
  return out.str();
}

const char* const TestConfigData = "#comments \n"
                                   "[GRP1]\n"
                                   "KEY1=10\n"
                                   "KEY2 = FOO\n"
                                   "\n"
                                   "#comments 2\n"
                                   "[GRP2]\n"
                                   "KEY1=101\n"
                                   "KEY2 = FOO\n"
                                   "spaces  =    \"    \"\n"
                                   "newline=  Hello\\nWorld\n"
                                   "[GRP3]\n"
                                   "multiple  = 1,2,3,4,5\n"
                                   "[==GROUP==]\n"
                                   "KEY1=11 10\n"
                                   "KEY2=Hello\\,World\n"
                                   "KEY3=\"Hello, World\"\n";

} // unnamed namespace

class ConfigManTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ConfigManTest);
  CPPUNIT_TEST(testSetGetValueNoKey);
  CPPUNIT_TEST(testSetGetValue);
  CPPUNIT_TEST(testSetGetValueCaseCombination);
  CPPUNIT_TEST(testSetGetValueInsertExisting);
  CPPUNIT_TEST(testSetGetValueOverwrite);
  CPPUNIT_TEST(testSetGetValueUseDefaultGroup);
  CPPUNIT_TEST(testSetGetValueToInt);
  CPPUNIT_TEST(testSetGetValuesNoKey);
  CPPUNIT_TEST(testSetGetValues);
  CPPUNIT_TEST(testSetGetValuesOverwrite);
  CPPUNIT_TEST(testGetAllValues);
  CPPUNIT_TEST(testGetMetaTagRef);
  CPPUNIT_TEST(testGetMetaTagEnv);
  CPPUNIT_TEST(testGetMetaTagSys);
  CPPUNIT_TEST(testGetMetaTagFunc);
  CPPUNIT_TEST(testGetMetaTagRecursive);
  CPPUNIT_TEST(testGetFalseMetaTag);
  CPPUNIT_TEST(testEraseValue);
  CPPUNIT_TEST(testClear);
  CPPUNIT_TEST(testRead);
  CPPUNIT_TEST(testReadWrite);
  CPPUNIT_TEST(testReadFailed);
  CPPUNIT_TEST(testErrorHandler);
  CPPUNIT_TEST_SUITE_END();

  void testSetGetValueNoKey()
  {
    tse::ConfigManType cfg;
    std::string value;

    bool result = cfg.getValue("KEY1", value, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Successful get value for key: KEY1 from group: GRP1", !result);

    result = cfg.getValue("KEY1", value, "GRP4");
    CPPUNIT_ASSERT_MESSAGE("Successful get value for key: KEY1 from group: GRP4", !result);
  }

  void testSetGetValue()
  {
    tse::ConfigManType cfg;
    std::string value;

    bool result = cfg.setValue("KEY1", "KEY1_GRP1", "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY1 from group: GRP1", result);
    result = cfg.getValue("KEY1", value, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY1 from group: GRP1", result);
    CPPUNIT_ASSERT_EQUAL(std::string("KEY1_GRP1"), value);
    value.clear();

    result = cfg.setValue("KEY2", "KEY2_GRP1", "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY2 from group: GRP1", result);
    result = cfg.getValue("KEY2", value, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: GRP1", result);
    CPPUNIT_ASSERT_EQUAL(std::string("KEY2_GRP1"), value);
    value.clear();

    result = cfg.setValue("KEY1", "KEY1_GRP2", "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY1 from group: GRP2", result);
    result = cfg.getValue("KEY1", value, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY1 from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(std::string("KEY1_GRP2"), value);
    value.clear();

    result = cfg.setValue("KEY2", "KEY2_GRP2", "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY2 from group: GRP2", result);
    result = cfg.getValue("KEY2", value, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(std::string("KEY2_GRP2"), value);
    value.clear();

    result = cfg.setValue("KEY3", "KEY3_GRP2\n", "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY3 from group: GRP2", result);
    result = cfg.getValue("KEY3", value, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY3 from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(std::string("KEY3_GRP2\n"), value);
    value.clear();
  }

  void testSetGetValueCaseCombination()
  {
    tse::ConfigManType cfg;
    std::string value;

    bool result = cfg.setValue("FOO", 42);
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: FOO from group: DEFAULT", result);
    result = cfg.setValue("KEY1", "KEY1_GRP1", "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY1 from group: GRP1", result);
    result = cfg.setValue("KEY3", "KEY3_GRP2\n", "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY3 from group: GRP2", result);

    // find value by lower/upper cases combination
    result = cfg.getValue("fOO", value);
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: FOO from group: DEFAULT", result);
    CPPUNIT_ASSERT_EQUAL(std::string("42"), value);
    value.clear();

    result = cfg.getValue("kEy3", value, "grp2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY3 from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(std::string("KEY3_GRP2\n"), value);
    value.clear();

    result = cfg.getValue("key1", value, "grp1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY1 from group: GRP1", result);
    CPPUNIT_ASSERT_EQUAL(std::string("KEY1_GRP1"), value);
    value.clear();
  }

  void testSetGetValueInsertExisting()
  {
    tse::ConfigManType cfg;
    std::string value;

    bool result = cfg.setValue("KEY2", "KEY2_GRP1", "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY2 from group: GRP1", result);

    // insert value to existing key
    result = cfg.setValue("KEY2", 1, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY2 from group: GRP1", result);
    result = cfg.getValue("KEY2", value, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: GRP1", result);
    CPPUNIT_ASSERT_EQUAL(std::string("KEY2_GRP1"), value);
    value.clear();
  }

  void testSetGetValueOverwrite()
  {
    tse::ConfigManType cfg;
    std::string value;

    bool result = cfg.setValue("KEY2", "KEY2_GRP2", "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY2 from group: GRP2", result);

    // overwrite value
    result = cfg.setValue("KEY2", 1, "GRP2", true);
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY2 from group: GRP2", result);
    result = cfg.getValue("KEY2", value, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(std::string("1"), value);
    value.clear();
  }

  void testSetGetValueUseDefaultGroup()
  {
    tse::ConfigManType cfg;
    std::string value;

    bool result = cfg.setValue("FOO", 42);
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: FOO from group: DEFAULT", result);
    result = cfg.getValue("FOO", value);
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: FOO from group: DEFAULT", result);
    CPPUNIT_ASSERT_EQUAL(std::string("42"), value);
    value.clear();
  }

  void testSetGetValueToInt()
  {
    tse::ConfigManType cfg;
    int intValue;
    unsigned uintValue;

    bool result = cfg.setValue("FOO", 42);
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: FOO from group: DEFAULT", result);
    result = cfg.setValue("KEY2", 1, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY2 from group: GRP2", result);
    result = cfg.setValue("KEY1", "KEY1_GRP1", "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY1 from group: GRP1", result);

    result = cfg.setValue("KEY1Signed", "-100", "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot set value for key: KEY1Signed from group: GRP1", result);

    result = cfg.getValue("fOO", intValue);
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: FOO from group: DEFAULT", result);
    CPPUNIT_ASSERT_EQUAL(int(42), intValue);

    result = cfg.getValue("KEY2", intValue, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(int(1), intValue);

    result = cfg.getValue("key1", intValue, "grp1");
    CPPUNIT_ASSERT_MESSAGE("Successful value cast for key: KEY1 from group: GRP1", !result);

    result = cfg.getValue("KEY1Signed", uintValue, "grp1");
    CPPUNIT_ASSERT_MESSAGE("Successful value cast for key: KEY1Signed from group: GRP1", !result);
  }

  void testSetGetValuesNoKey()
  {
    tse::ConfigManType cfg;
    tse::ConfigManType::Values values;
    std::vector<tse::ConfigManType::NameValue> namesValues;

    bool result = cfg.getValues("KEY1", values, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Successful get values for key: KEY1 from group: GRP1", !result);

    // NOTE old implementation of ConfigMan always return true if group does not exist
    result = cfg.getValues(namesValues, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: GRP1", result);
  }

  void testSetGetValues()
  {
    tse::ConfigManType cfg;
    tse::ConfigManType::Values values;
    tse::ConfigManType::Values resultValues;
    std::vector<tse::ConfigManType::NameValue> namesValues;
    std::vector<tse::ConfigManType::NameValue> resultNamesValues;

    values.push_back("3");
    values.push_back("2");
    values.push_back("1");
    bool result = cfg.setValues("KEY1", values, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot set values for key: KEY1 from group: GRP1", result);
    result = cfg.getValues("KEY1", resultValues, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get values for key: KEY1 from group: GRP1", result);
    CPPUNIT_ASSERT_EQUAL(ToString(values), ToString(resultValues));
    result = cfg.getValues(resultNamesValues, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get values for key: KEY1 from group: GRP1", result);
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "3", "GRP1"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "2", "GRP1"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "1", "GRP1"));
    CPPUNIT_ASSERT_EQUAL(ToString(namesValues), ToString(resultNamesValues));
    resultValues.clear();
    values.clear();
    namesValues.clear();
    resultNamesValues.clear();

    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "0", "GRP1"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "-1", "GRP1"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "0", "GRP2"));
    result = cfg.setValues(namesValues);
    CPPUNIT_ASSERT_MESSAGE("Cannot set values for key: KEY1 from group: GRP1 and GRP2", result);
    result = cfg.getValues("KEY1", resultValues, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get values for key: KEY1 from group: GRP1", result);
    values.push_back("3");
    values.push_back("2");
    values.push_back("1");
    values.push_back("0");
    values.push_back("-1");
    CPPUNIT_ASSERT_EQUAL(ToString(values), ToString(resultValues));
    result = cfg.getValues(resultNamesValues, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get values for key: KEY1 from group: GRP1", result);
    namesValues.clear();
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "3", "GRP1"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "2", "GRP1"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "1", "GRP1"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "0", "GRP1"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "-1", "GRP1"));
    CPPUNIT_ASSERT_EQUAL(ToString(namesValues), ToString(resultNamesValues));
    resultValues.clear();
    values.clear();
    namesValues.clear();
    resultNamesValues.clear();

    result = cfg.getValues("KEY1", resultValues, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get values for key: KEY1 from group: GRP2", result);
    values.push_back("0");
    CPPUNIT_ASSERT_EQUAL(ToString(values), ToString(resultValues));
    result = cfg.getValues(resultNamesValues, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get values for key: KEY1 from group: GRP2", result);
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "0", "GRP2"));
    CPPUNIT_ASSERT_EQUAL(ToString(namesValues), ToString(resultNamesValues));
    resultValues.clear();
    values.clear();
    namesValues.clear();
    resultNamesValues.clear();
  }

  void testSetGetValuesOverwrite()
  {
    tse::ConfigManType cfg;
    tse::ConfigManType::Values values;
    tse::ConfigManType::Values resultValues;
    std::vector<tse::ConfigManType::NameValue> namesValues;
    std::vector<tse::ConfigManType::NameValue> resultNamesValues;

    values.push_back("3");
    values.push_back("2");
    values.push_back("1");
    bool result = cfg.setValues("KEY1", values, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot set values for key: KEY1 from group: GRP1", result);
    values.clear();

    // get values override
    values.push_back("100");
    result = cfg.setValues("KEY1", values, "GRP1", true);
    CPPUNIT_ASSERT_MESSAGE("Cannot set values for key: KEY1 from group: GRP1", result);
    result = cfg.getValues("KEY1", resultValues, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get values for key: KEY1 from group: GRP1", result);
    CPPUNIT_ASSERT_EQUAL(ToString(values), ToString(resultValues));
    values.clear();
    resultValues.clear();

    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "50", "GRP1"));
    result = cfg.setValues(namesValues, true);
    CPPUNIT_ASSERT_MESSAGE("Cannot set values for key: KEY1 from group: GRP1", result);
    result = cfg.getValues(resultNamesValues, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get values for key: KEY1 from group: GRP1", result);
    CPPUNIT_ASSERT_EQUAL(ToString(namesValues), ToString(resultNamesValues));
    namesValues.clear();
    resultNamesValues.clear();
  }

  void testGetAllValues()
  {
    tse::ConfigManType cfg;
    tse::ConfigManType::Values values;
    std::vector<tse::ConfigManType::NameValue> namesValues;
    std::vector<tse::ConfigManType::NameValue> resultNamesValues;

    cfg.setValue("Key1", 0, "Grp2");

    values.push_back("50");
    cfg.setValues("Key1", values, "Grp1");
    values.clear();

    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "150", "GRP3"));
    cfg.setValues(namesValues);
    namesValues.clear();

    bool result = cfg.getValues(resultNamesValues);
    CPPUNIT_ASSERT_MESSAGE("Cannot get all values", result);
    namesValues.clear();
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "50", "GRP1"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "0", "GRP2"));
    namesValues.push_back(tse::ConfigManType::NameValue("KEY1", "150", "GRP3"));
    CPPUNIT_ASSERT_EQUAL(ToString(namesValues), ToString(resultNamesValues));
  }

  void testGetMetaTagRef()
  {
    tse::ConfigManType cfg;
    std::string value;

    cfg.setValue("KEY0", 42);
    cfg.setValue("KEY1", 100, "GROUP");

    cfg.setValue("KEY_REF0", "<ref>KEY0</ref>", "GROUP");
    cfg.setValue("KEY_REF1", "<ref>GROUP:KEY1</ref>", "GROUP");
    cfg.setValue("KEY_REF2",
                 "Ultimate question of life the universe and everything: <ref>KEY0</ref>",
                 "GROUP");

    bool result = cfg.getValue("KEY_REF0", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_REF0 from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(std::string("42"), value);

    result = cfg.getValue("KEY_REF1", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_REF1 from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(std::string("100"), value);

    result = cfg.getValue("KEY_REF2", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_REF2 from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(std::string("Ultimate question of life the universe and everything: 42"),
                         value);
  }

  void testGetMetaTagEnv()
  {
    tse::ConfigManType cfg;
    std::string value;
    ::setenv("CONFIG_MAP_ENV", "Hello", 1);

    cfg.setValue("KEY_ENV", "<env>CONFIG_MAP_ENV</env>", "GROUP");

    bool result = cfg.getValue("KEY_ENV", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_ENV from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(std::string("Hello"), value);
  }

  void testGetMetaTagSys()
  {
    tse::ConfigManType cfg;
    std::string value;

    cfg.setValue("KEY_SYS", "<sys>echo \"Hello world\"</sys>", "GROUP");

    bool result = cfg.getValue("KEY_SYS", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_SYS from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(std::string("Hello world\n"), value);
  }

  void testGetMetaTagFunc()
  {
    tse::ConfigManType cfg;
    std::string value;

    cfg.setValue("KEY_PID", "<func>PID</func>", "GROUP");
    cfg.setValue("KEY_THREAD", "<func>THREAD</func>", "GROUP");
    cfg.setValue("KEY_CORES", "<func>CORES</func>", "GROUP");

    bool result = cfg.getValue("KEY_PID", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_SYS from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(boost::lexical_cast<std::string>(::getpid()), value);

    result = cfg.getValue("KEY_THREAD", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_THREAD from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(boost::lexical_cast<std::string>(::pthread_self()), value);

    result = cfg.getValue("KEY_CORES", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_CORES from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(boost::lexical_cast<std::string>(boost::thread::hardware_concurrency()),
                         value);
  }

  void testGetMetaTagRecursive()
  {
    tse::ConfigManType cfg;
    std::string value;

    cfg.setValue("KEY_REF_REC0", "<ref>GROUP:KEY_REF_REC2</ref>", "GROUP");
    cfg.setValue("KEY_REF_REC1", "<ref>GROUP:KEY_REF_REC0</ref>", "GROUP");
    cfg.setValue("KEY_REF_REC2", "<ref>GROUP:KEY_REF_REC1</ref>", "GROUP");

    // test infinite recursive
    bool result = cfg.getValue("KEY_REF_REC2", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Successful get value for key: KEY_REF_REC2 from group: GROUP", !result);
  }

  void testGetFalseMetaTag()
  {
    tse::ConfigManType cfg;
    std::string value;

    cfg.setValue("KEY_NOMETA1", "<func>PID</ref>", "GROUP");
    cfg.setValue("KEY_NOMETA2", "</env>PATH</env>", "GROUP");
    cfg.setValue("KEY_NOMETA3", "<func>PID</func", "GROUP");

    bool result = cfg.getValue("KEY_NOMETA1", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_NOMETA1 from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(std::string("<func>PID</ref>"), value);

    result = cfg.getValue("KEY_NOMETA2", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_NOMETA2 from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(std::string("</env>PATH</env>"), value);

    result = cfg.getValue("KEY_NOMETA3", value, "GROUP");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY_NOMETA3 from group: GROUP", result);
    CPPUNIT_ASSERT_EQUAL(std::string("<func>PID</func"), value);
  }

  void testEraseValue()
  {
    tse::ConfigManType cfg;
    std::string value;

    cfg.setValue("KEY1", "KEY1_GRP1", "GRP1");
    cfg.setValue("KEY2", "KEY2_GRP1", "GRP1");
    cfg.setValue("KEY1", "KEY2_GRP1", "GRP2");
    cfg.setValue("KEY1", "KEY2_GRP2", "GRP2");

    std::size_t nitems = cfg.eraseValue("Key2", "Grp1");
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), nitems);

    nitems = cfg.eraseValue("Key1", "GRP2");
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), nitems);

    bool result = cfg.getValue("KEY2", value, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Successful get value for key: KEY2 from group: GRP1", !result);

    result = cfg.getValue("KEY1", value, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Successful get value for key: KEY1 from group: GRP2", !result);

    result = cfg.getValue("KEY1", value, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY1 from group: GRP1", result);
  }

  void testClear()
  {
    tse::ConfigManType cfg;
    std::string value;

    cfg.setValue("KEY1", "KEY1_GRP1", "GRP1");
    cfg.clear();

    bool result = cfg.getValue("KEY1", value, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Successful get value for key: KEY1 from group: GRP1", !result);
  }

  void testRead()
  {
    std::istringstream in(TestConfigData);
    tse::ConfigManType cfg;
    std::string value;
    bool result = cfg.read(in);
    CPPUNIT_ASSERT_MESSAGE("Cannot read config data", result);

    result = cfg.getValue("KEY1", value, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY1 from group: GRP1", result);
    CPPUNIT_ASSERT_EQUAL(std::string("10"), value);

    result = cfg.getValue("KEY2", value, "GRP1");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: GRP1", result);
    CPPUNIT_ASSERT_EQUAL(std::string("FOO"), value);

    result = cfg.getValue("KEY1", value, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY1 from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(std::string("101"), value);

    result = cfg.getValue("KEY2", value, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(std::string("FOO"), value);

    result = cfg.getValue("SPACES", value, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: SPACES from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(std::string("    "), value);

    result = cfg.getValue("newLine", value, "GRP2");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: NEWLINE from group: GRP2", result);
    CPPUNIT_ASSERT_EQUAL(std::string("Hello\nWorld"), value);

    tse::ConfigManType::Values values;
    result = cfg.getValues("multiple", values, "GRP3");
    CPPUNIT_ASSERT_MESSAGE("Cannot get values for key: MULTIPLE from group: GRP2", result);
    tse::ConfigManType::Values expectedValues;
    expectedValues.push_back("1");
    expectedValues.push_back("2");
    expectedValues.push_back("3");
    expectedValues.push_back("4");
    expectedValues.push_back("5");
    CPPUNIT_ASSERT_EQUAL(ToString(expectedValues), ToString(values));

    result = cfg.getValue("KEY1", value, "==GROUP==");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY1 from group: ==GROUP==", result);
    // TODOCPPUNIT_ASSERT_EQUAL(std::string("11 10"), value);

    result = cfg.getValue("KEY2", value, "==GROUP==");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: ==GROUP==", result);
    CPPUNIT_ASSERT_EQUAL(std::string("Hello,World"), value);

    result = cfg.getValue("KEY3", value, "==GROUP==");
    CPPUNIT_ASSERT_MESSAGE("Cannot get value for key: KEY2 from group: ==GROUP==", result);
    CPPUNIT_ASSERT_EQUAL(std::string("Hello, World"), value);
  }

  void testReadWrite()
  {
    std::istringstream in(TestConfigData);
    tse::ConfigManType cfg;
    std::string value;
    bool result = cfg.read(in);
    CPPUNIT_ASSERT_MESSAGE("Cannot read config data", result);

    std::ostringstream out;
    cfg.write(out);

    // read again
    const std::string outString = out.str();
    in.str(outString);
    in.clear();
    result = cfg.read(in);
    CPPUNIT_ASSERT_MESSAGE("Cannot read config data", result);

    // write again
    out.str(std::string());
    out.clear();
    cfg.write(out);

    CPPUNIT_ASSERT_EQUAL(outString, out.str());
  }

  void testReadFailed()
  {
    std::istringstream in;

    in.str("[group end\n"
           "key=100");
    tse::ConfigManType cfg;
    bool result = cfg.read(in);
    CPPUNIT_ASSERT_MESSAGE("Malformed input read success", !result);

    in.str("[group end]\n"
           "key100");
    in.clear();
    result = cfg.read(in);
    CPPUNIT_ASSERT_MESSAGE("Malformed input read success", !result);

    in.str("group end]\n"
           "key100=100\n");
    in.clear();
    result = cfg.read(in);
    CPPUNIT_ASSERT_MESSAGE("Malformed input read success", !result);

    in.str("group end]\n"
           "key11=\"aaa\n"
           "key100=\n100");
    in.clear();
    result = cfg.read(in);
    CPPUNIT_ASSERT_MESSAGE("Malformed input read success", !result);
  }

  void testErrorHandler()
  {
    typedef ConfigManTemplate<detail::EmptyMutex, ThrowErrorHandler> Config;
    Config cfg;
    cfg.setValue("NON_INT", "value", "GRP1");
    cfg.setValue("NON_UINT", "-1", "GRP1");

    int intVal;
    CPPUNIT_ASSERT_THROW(cfg.getValue("NON_INT", intVal, "GRP1"), std::runtime_error);

    unsigned uintVal;
    CPPUNIT_ASSERT_THROW(cfg.getValue("NON_UINT", uintVal, "GRP1"), std::runtime_error);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigManTest);

} // namespace tse
