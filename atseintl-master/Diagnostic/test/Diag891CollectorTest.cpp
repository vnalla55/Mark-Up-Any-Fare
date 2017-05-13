#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "Diagnostic/Diag891Collector.h"

using namespace std;

namespace tse
{

class Diag891CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag891CollectorTest);
  CPPUNIT_TEST(testConstructor);

  CPPUNIT_TEST_SUITE_END();

private:
  Diag891Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diag891Collector diag;
      CPPUNIT_ASSERT_EQUAL(string(""), diag.str());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void setUp()
  {
    try
    {
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic891));
      _diagroot->activate();
      _diag = _memHandle.insert(new Diag891Collector(*_diagroot));
      _diag->enable(Diagnostic891);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag891CollectorTest);
} // tse
