#include "test/include/CppUnitHelperMacros.h"

#include "AddonConstruction/AtpcoConstructedFare.h"
#include "DBAccess/AddonFareInfo.h"
#include "AddonConstruction/ConstructionJob.h"
#include "DBAccess/TSEDateInterval.h"
#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/AddonFareCortege.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class ConstructedFareTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ConstructedFareTest);

  CPPUNIT_TEST(testisValidFailNotValidFlag);
  CPPUNIT_TEST(testisValidFailFareCortegeNotSet);
  CPPUNIT_TEST(testisValidFailDoubleEnded);
  CPPUNIT_TEST(testisValidPassDoubleEnded);
  CPPUNIT_TEST(testisValidPassNotDoubleEnded);
  CPPUNIT_TEST(testClone);

  CPPUNIT_TEST_SUITE_END();
  ConstructedFare* _cf;
  TestMemHandle _memHandle;

public:
  void setUp() { _cf = _memHandle.create<AtpcoConstructedFare>(); }

  void tearDown() { _memHandle.clear(); }

  void testisValidFailNotValidFlag()
  {
    _cf->valid() = false;
    CPPUNIT_ASSERT_EQUAL(false, _cf->isValid());
  }

  void testisValidFailFareCortegeNotSet()
  {
    _cf->valid() = true;
    CPPUNIT_ASSERT_EQUAL(false, _cf->isValid());
  }

  void testisValidFailDoubleEnded()
  {
    _cf->valid() = true;
    _cf->isDoubleEnded() = true;
    _cf->origAddon() = new AddonFareCortege;
    CPPUNIT_ASSERT_EQUAL(false, _cf->isValid());
  }

  void testisValidPassDoubleEnded()
  {
    _cf->valid() = true;
    _cf->isDoubleEnded() = true;
    _cf->origAddon() = new AddonFareCortege;
    _cf->destAddon() = new AddonFareCortege;
    CPPUNIT_ASSERT_EQUAL(true, _cf->isValid());
  }

  void testisValidPassNotDoubleEnded()
  {
    _cf->valid() = true;
    _cf->isDoubleEnded() = false;
    _cf->destAddon() = new AddonFareCortege;
    CPPUNIT_ASSERT_EQUAL(true, _cf->isValid());
  }

  void testClone()
  {
    _cf->origAddon() = new AddonFareCortege;
    _cf->destAddon() = new AddonFareCortege;
    _cf->market1() = "ABC";
    _cf->gateway2() = "NYC";
    AtpcoConstructedFare cf;
    _cf->clone(cf);
    CPPUNIT_ASSERT_EQUAL(_cf->market1(), cf.market1());
    CPPUNIT_ASSERT_EQUAL(_cf->gateway2(), cf.gateway2());
    CPPUNIT_ASSERT_EQUAL(_cf->origAddon(), cf.origAddon());
    CPPUNIT_ASSERT_EQUAL(_cf->destAddon(), cf.destAddon());
    CPPUNIT_ASSERT_EQUAL(_cf->specifiedFare(), cf.specifiedFare());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ConstructedFareTest);
}
