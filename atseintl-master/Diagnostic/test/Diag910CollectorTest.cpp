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
#include "test/include/CppUnitHelperMacros.h"

#include "Diagnostic/Diag910Collector.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class Diag910CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag910CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testStreamWrongTrxType);
  CPPUNIT_TEST(testStreamEmptyShoppingTrx);
  CPPUNIT_TEST(testOutputHeader);
  CPPUNIT_TEST_SUITE_END();

  Diag910Collector* _diag;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _diag = _memHandle.create<Diag910Collector>();
    _diag->rootDiag() = _memHandle.create<Diagnostic>();
    _diag->activate();
  }

  void tearDown() { _memHandle.clear(); }

  std::string getDiagString()
  {
    _diag->flushMsg();
    return _diag->str();
  }

  void testConstructor()
  {
    try { CPPUNIT_ASSERT_EQUAL(std::string(""), getDiagString()); }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testStreamWrongTrxType()
  {
    PricingTrx trx;

    (*_diag) << trx;

    std::string expected = "DIAG 910 REQUIRES ShoppingTrx\n";
    CPPUNIT_ASSERT_EQUAL(expected, getDiagString());
  }

  void testStreamEmptyShoppingTrx()
  {
    ShoppingTrx trx;
    Itin itin;
    trx.journeyItin() = &itin;

    (*_diag) << trx;
    std::string expected = "(NO VALID FARE PATHS)\n";
    CPPUNIT_ASSERT(getDiagString().find(expected) != std::string::npos);
  }

  void testOutputHeader()
  {
    _diag->printHeader();

    std::string expected = "SEQ LEG PU CX/FARE BASIS           /TOTAL AMOUNT/ AMT CUR/GI\n";
    CPPUNIT_ASSERT_EQUAL(expected, getDiagString());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag910CollectorTest);

} // tse
