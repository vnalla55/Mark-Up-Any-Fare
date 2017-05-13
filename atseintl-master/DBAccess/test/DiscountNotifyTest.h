#ifndef DiscountNotifyTest_H
#define DiscountNotifyTest_H

#include <cppunit/extensions/HelperMacros.h>

namespace tse
{
class DiscountNotifyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiscountNotifyTest);
  CPPUNIT_TEST(testTranslate);
  CPPUNIT_TEST_SUITE_END();

public:
  void testTranslate();
};

} // namespace tse

#endif // DiscountNotifyTest_H
