#include <string>
#include <time.h>
#include <iostream>
#include "DataModel/TaxResponse.h"
#include "Diagnostic/Diag803Collector.h"
#include "test/include/CppUnitHelperMacros.h"
#include <unistd.h>

using namespace std;
namespace tse
{
class Diag803CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag803CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testInactiveStreamingOperator);
  CPPUNIT_TEST(testStreamingOperator);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try
    {
      Diagnostic* diagroot = new Diagnostic(PFCRecSummaryDiagnostic);
      Diag803Collector diag(*diagroot);
      string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testInactiveStreamingOperator()
  {
    Diagnostic* diagroot = new Diagnostic(PFCRecSummaryDiagnostic);
    diagroot->activate();

    Diag803Collector diag(*diagroot);

    TaxResponse taxResponse;

    // First, we test being inactive. No changes are expected.

    diag << taxResponse;
    string str = diag.str();
    CPPUNIT_ASSERT_EQUAL(string(""), str);
  }

  void testStreamingOperator()
  {
    PricingTrx trx;
    trx.setValidatingCxrGsaApplicable(false);
    trx.setTrxType(PricingTrx::PRICING_TRX);
    PricingRequest request;
    trx.setRequest(&request);
    request.owPricingRTTaxProcess() = false;
    Diagnostic* diagroot = new Diagnostic(PFCRecSummaryDiagnostic);
    diagroot->activate();

    Diag803Collector diag(*diagroot);
    diag.trx() = &trx;

    // Now, enable it and do the same streaming operation.
    diag.enable(PFCRecSummaryDiagnostic);

    CPPUNIT_ASSERT(diag.isActive());

    TaxResponse taxResponse;
    FarePath dummyFarePath;
    taxResponse.farePath() = &dummyFarePath;

    diag << taxResponse;

    string str = diag.str();
    string expected;
    expected = " \n***********  PASSENGER FACILITY CHARGE  ***********\n";
    expected += "    ABIND  PFC AMT  CURR  SEGNO  PFC AIRPORT       \n";
    expected += "---------------------------------------------------\n \n";
    expected += "PAX TYPE \n";
    expected += "***************************************************\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag803CollectorTest);
}
