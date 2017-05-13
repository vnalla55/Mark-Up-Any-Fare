//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag807Collector.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class Diag807CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag807CollectorTest);
  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintWithExemptionModificator);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _dc = _memHandle(new Diag807Collector);
    _dc->activate();

    PricingTrx* trx = _memHandle(new PricingTrx);
    _dc->trx() = trx;
  }

  void tearDown() { _memHandle.clear(); }

  void testPrintHeader()
  {
    std::string expected = "************************************************************\n"
                           "*    ATSE PFC AND TAX EXEMPTION PROCESS DIAGNOSTIC 807     *\n"
                           "************************************************************\n";

    _dc->printHeader();
    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
    _dc->clear();
  }

  void testPrintWithExemptionModificator()
  {
    std::string expected = "PFC/TAX EXEMPT NOT APPL WITH TAX EXEMPT MODIFIERS TE/TN/TX\n";
    _dc->printInfo(Diag807Collector::EXEMPTION_MODIFIERS_USED);
    CPPUNIT_ASSERT_EQUAL(expected, _dc->str());
    _dc->clear();
  }

private:
  Diag807Collector* _dc;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag807CollectorTest);
}
