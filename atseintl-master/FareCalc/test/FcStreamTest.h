#ifndef FC_STREAM_TEST_H
#define FC_STREAM_TEST_H

#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/DataHandle.h"

namespace tse
{
class Itin;
class FareUsage;

class FcStreamTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FcStreamTest);
  CPPUNIT_TEST(testCollect);
  CPPUNIT_TEST(testForEachFareUsage);
  CPPUNIT_TEST(testSplit);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testCollect();
  void testForEachFareUsage();
  void testSplit();

  DataHandle _dataHandle;
  Itin* _itin;
  std::vector<FareUsage*> _fuList;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FcStreamTest);

} // tse

#endif // FC_STREAM_TEST_H
