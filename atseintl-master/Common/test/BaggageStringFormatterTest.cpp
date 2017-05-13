#include "test/include/CppUnitHelperMacros.h"

#include "Common/BaggageStringFormatter.h"

namespace tse
{
class BaggageStringFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageStringFormatterTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}
};
CPPUNIT_TEST_SUITE_REGISTRATION(BaggageStringFormatterTest);
} // tse
