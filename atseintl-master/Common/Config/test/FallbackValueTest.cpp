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

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestLogger.h"

#include "Common/Config/FallbackValue.h"

#include "Common/Config/ConfigMan.h"

namespace tse
{

namespace fallback
{

class FallbackValueTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FallbackValueTest);

  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testSet);
  CPPUNIT_TEST(testSet_fromConfigOn);
  CPPUNIT_TEST(testSet_fromConfigOff);
  CPPUNIT_TEST(testSet_fromConfigBadValue);
  CPPUNIT_TEST(testSet_fromConfigEmpty);

  CPPUNIT_TEST(testConfigure);

  CPPUNIT_TEST_SUITE_END();

protected:
  FallbackValue* _fallback;
  TestMemHandle _memH;
  TestLogger* _logger;
  RootLoggerGetOff _getOff;

  static const std::string OPTION;
  static const bool DEFAULT_VALUE;
  static const char YES, NO;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _logger = _memH(new TestLogger("atseintl.Server.TseServer"));

    _fallback = _memH(new FallbackValue(OPTION, DEFAULT_VALUE));
  }

  void tearDown() { _memH.clear(); }

  void testCreate()
  {
    CPPUNIT_ASSERT_EQUAL(OPTION, _fallback->_option);
    CPPUNIT_ASSERT_EQUAL(DEFAULT_VALUE, _fallback->_value);
  }

  void testSet()
  {
    _fallback->set(true);
    CPPUNIT_ASSERT(_fallback->_value);
  }

  void setOption(tse::ConfigMan& config, char value, const std::string& option = OPTION)
  {
    config.setValue(option, value, FallbackValue::CONFIG_SECTION);
  }

  void testSet_fromConfigOn()
  {
    tse::ConfigMan config;
    setOption(config, YES);
    _fallback->set(&config);

    CPPUNIT_ASSERT(_fallback->_value);
    CPPUNIT_ASSERT_EQUAL(std::string("INFO - Setting OPTION - On\n"), _logger->str());
  }

  void testSet_fromConfigOff()
  {
    tse::ConfigMan config;
    setOption(config, NO);
    _fallback->set(true);
    _fallback->set(&config);

    CPPUNIT_ASSERT(!_fallback->_value);
    CPPUNIT_ASSERT_EQUAL(std::string("INFO - Setting OPTION - Off\n"), _logger->str());
  }

  void testSet_fromConfigBadValue()
  {
    tse::ConfigMan config;
    setOption(config, 'B');
    _fallback->set(&config);

    CPPUNIT_ASSERT_EQUAL(DEFAULT_VALUE, _fallback->_value);
    CPPUNIT_ASSERT_EQUAL(std::string(), _logger->str());
  }

  void testSet_fromConfigEmpty()
  {
    tse::ConfigMan config;
    _fallback->set(&config);

    CPPUNIT_ASSERT_EQUAL(DEFAULT_VALUE, _fallback->_value);
    CPPUNIT_ASSERT_EQUAL(std::string(), _logger->str());
  }

  void testConfigure()
  {
    const std::string otherOption = "OTHER_OPTION";
    FallbackValue otherValue(otherOption, true);

    tse::ConfigMan config;
    setOption(config, YES);
    setOption(config, NO, otherOption);

    configure(config);

    CPPUNIT_ASSERT(_fallback->_value);
    CPPUNIT_ASSERT(!otherValue.get());
    CPPUNIT_ASSERT_EQUAL(std::string("INFO - Setting OPTION - On\n"
                                     "INFO - Setting OTHER_OPTION - Off\n"),
                         _logger->str());
  }
};

const std::string FallbackValueTest::OPTION = "OPTION";

const bool FallbackValueTest::DEFAULT_VALUE = false;

const char FallbackValueTest::YES = 'Y', FallbackValueTest::NO = 'N';

CPPUNIT_TEST_SUITE_REGISTRATION(FallbackValueTest);

} // fallback

} // tse

#endif
