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
#ifndef CONFIG_HIERARCHY_REFACTOR

#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/DynamicConfigurableFallbackValue.h"
#include "DataModel/PricingTrx.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestLogger.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
namespace fallback
{
class DynamicConfigurableFallbackValueTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DynamicConfigurableFallbackValueTest);

  CPPUNIT_TEST(testCreate);

  CPPUNIT_TEST(testSet);
  CPPUNIT_TEST(testSet_fromConfigOn);
  CPPUNIT_TEST(testSet_fromConfigOff);
  CPPUNIT_TEST(testSet_fromConfigBadValue);
  CPPUNIT_TEST(testSet_fromConfigEmpty);
  CPPUNIT_TEST(testSet_fromConfigDynamicLoadable);

  CPPUNIT_TEST(testConfigure);

  CPPUNIT_TEST_SUITE_END();

protected:
  DynamicConfigurableFallbackValue* _fallback;
  TestMemHandle _memH;
  TestLogger* _clogger;
  TestLogger* _slogger;
  PricingTrx* _trx;

  static const std::string OPTION;
  static const bool DEFAULT_VALUE;
  static const char YES, NO;

public:
  void setUp()
  {
    _memH(new TestConfigInitializer);
    _slogger = _memH(new TestLogger("atseintl.Server.TseServer"));
    _clogger = _memH(new TestLogger("atseintl.Common.ConfigurableValue"));
    _trx = _memH(new PricingTrx);

    _fallback = _memH(new DynamicConfigurableFallbackValue(OPTION, DEFAULT_VALUE));
  }

  void tearDown()
  {
    _memH.clear();
  }

  void testCreate()
  {
    CPPUNIT_ASSERT(!_fallback->_isConfigured);
    CPPUNIT_ASSERT_EQUAL(OPTION, _fallback->_option);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_VALUE, _fallback->_value);
    CPPUNIT_ASSERT_EQUAL(DynamicConfigurableFallbackValue::CONFIG_SECTION, _fallback->_section);
  }

  void testSet()
  {
    _fallback->set(true);

    CPPUNIT_ASSERT(_fallback->_isConfigured);
    CPPUNIT_ASSERT(_fallback->_value);
  }

  void setOption(tse::ConfigMan& config, char value, const std::string& option = OPTION)
  {
    config.setValue(option, value, DynamicConfigurableFallbackValue::CONFIG_SECTION);
  }

  void setDynamicLoadableItems() { setOption(Global::config(), YES); }

  void testSet_fromConfigOn()
  {
    tse::ConfigMan config;
    setOption(config, YES);

    _fallback->set(&config);

    CPPUNIT_ASSERT(_fallback->get(_trx));
    CPPUNIT_ASSERT_EQUAL(std::string("INFO - Setting OPTION - On\n"), _slogger->str());
    CPPUNIT_ASSERT_EQUAL(std::string(), _clogger->str());
  }

  void testSet_fromConfigOff()
  {
    tse::ConfigMan config;
    setOption(config, NO);

    _fallback->_value = true;
    _fallback->set(&config);

    CPPUNIT_ASSERT(!_fallback->get(_trx));
    CPPUNIT_ASSERT_EQUAL(std::string("INFO - Setting OPTION - Off\n"), _slogger->str());
    CPPUNIT_ASSERT_EQUAL(std::string(), _clogger->str());
  }

  void testSet_fromConfigBadValue()
  {
    tse::ConfigMan config;
    setOption(config, 'B');

    _fallback->set(&config);

    CPPUNIT_ASSERT_EQUAL(DEFAULT_VALUE, _fallback->get(_trx));
    CPPUNIT_ASSERT_EQUAL(std::string(), _slogger->str());
    CPPUNIT_ASSERT_EQUAL(std::string("ERROR - Invalid value B in FALLBACK_SECTION:OPTION. "
                                     "Using default data instead.\n"),
                         _clogger->str());
  }

  void testSet_fromConfigEmpty()
  {
    tse::ConfigMan config;

    _fallback->set(&config);

    CPPUNIT_ASSERT_EQUAL(DEFAULT_VALUE, _fallback->get(_trx));
    CPPUNIT_ASSERT_EQUAL(std::string(), _slogger->str());
    CPPUNIT_ASSERT(_clogger->str().find("ERROR - Missing value in FALLBACK_SECTION:OPTION. "
                                        "Using default data instead.\n") != std::string::npos);
  }

  void testConfigure()
  {
    const std::string otherOption = "OTHER_OPTION";
    DynamicConfigurableFallbackValue otherValue(otherOption, true);

    tse::ConfigMan config;
    setOption(config, YES);
    setOption(config, NO, otherOption);

    configure(config);

    CPPUNIT_ASSERT(_fallback->get(_trx));
    CPPUNIT_ASSERT(!otherValue.get(_trx));
    CPPUNIT_ASSERT_EQUAL(std::string("INFO - Setting OPTION - On\n"
                                     "INFO - Setting OTHER_OPTION - Off\n"),
                         _slogger->str());
  }

  void testSet_fromConfigDynamicLoadable()
  {
    tse::ConfigMan config;
    setOption(config, YES);
    setDynamicLoadableItems();

    _fallback->set(&config);

    CPPUNIT_ASSERT(_fallback->get(_trx));
    CPPUNIT_ASSERT_EQUAL(std::string("INFO - Setting OPTION - On\n"), _slogger->str());
    CPPUNIT_ASSERT_EQUAL(std::string(), _clogger->str());
  }
};

const std::string DynamicConfigurableFallbackValueTest::OPTION = "OPTION";

const bool DynamicConfigurableFallbackValueTest::DEFAULT_VALUE = false;

const char DynamicConfigurableFallbackValueTest::YES = 'Y',
           DynamicConfigurableFallbackValueTest::NO = 'N';

CPPUNIT_TEST_SUITE_REGISTRATION(DynamicConfigurableFallbackValueTest);

} // fallback

} // tse

#endif
