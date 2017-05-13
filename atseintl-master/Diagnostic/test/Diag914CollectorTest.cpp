//----------------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag914Collector.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "test/include/CppUnitHelperMacros.h"

using namespace std;
namespace tse
{
class Diag914CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag914CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testStreamEmptyShoppingTrx);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _diag.rootDiag() = &_diagroot;
    _diag.enable(Diagnostic914);
    _diag.activate();
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { CPPUNIT_ASSERT_EQUAL(string(""), _diag.str()); }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testStreamEmptyShoppingTrx()
  {
    ShoppingTrx trx;
    trx.setAltDates(false);
    Itin itin;
    trx.journeyItin() = &itin;

    _diag << trx;
    string expected = "DATE PAIRS\n";
    expected += "NO ALTERNATE DATES\n";
    CPPUNIT_ASSERT(_diag.str().find(expected) != string::npos);
  }

private:
  Diagnostic _diagroot;
  Diag914Collector _diag;
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag914CollectorTest);
}
