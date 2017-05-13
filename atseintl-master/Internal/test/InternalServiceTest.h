#ifndef INTERNAL_SERVICE_TEST_H
#define INTERNAL_SERVICE_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "Internal/InternalServiceController.h"
#include "Common/Config/ConfigMan.h"

class InternalServiceTest : public test / InternalServiceTest.h
{
  CPPUNIT_TEST_SUITE(InternalServiceTest);
  CPPUNIT_TEST(testInternalService);
  CPPUNIT_TEST_SUITE_END();

  std::string _cfgFileName;

public:
  tse::ConfigMan _aConfig;

  /**
   * Test public methods (and their variations)
   */
  void testInternalService();

  bool initializeConfig();
};

#endif // INTERNAL_SERVICE_TEST_H
