#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include <ctime>
#include <iostream>
#include <unistd.h>
#include "DataModel/ShoppingTrx.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag905Collector.h"
#include "DataModel/Trx.h"

using namespace std;
namespace tse
{
class Diag905CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag905CollectorTest);
  CPPUNIT_TEST(testStreamingOperatorShoppingTrx);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testStreamingOperatorShoppingTrx()
  {
    Diagnostic diagroot(AllFareDiagnostic);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag905Collector diag(diagroot);
    diag.enable(AllFareDiagnostic);

    CPPUNIT_ASSERT(diag.isActive());

    ShoppingTrx trx;
    trx.setOptions(new PricingOptions());
    trx.getOptions()->currencyOverride() = "USD";
    //    trx.getOptions()->opaqueEntry() = 'T';
    trx.getOptions()->noMinMaxStayRestr() = 'T';
    trx.getOptions()->onlineFares() = 'T';
    trx.getOptions()->publishedFares() = 'T';
    trx.getOptions()->privateFares() = 'T';
    trx.getOptions()->returnFareCalc() = 'T';

    trx.setRequest(new PricingRequest());
    trx.getRequest()->exemptAllTaxes() = 'T';
    trx.getRequest()->lowFareNoAvailability() = 'T';

    trx.getRequest()->ticketingAgent() = new Agent();
    trx.getRequest()->ticketingAgent()->agentCity() = "DFW";
    trx.billing() = new Billing();
    diag << trx;

    std::string expected;
    /*
     expected = "\n";
     expected += "******************************************************\n";
     expected += "ShoppingTrx contents after Shopping XML parse:\n";
     expected += "******************************************************\n";
     expected += "\n";
     expected += "Options:\n";
     expected += "  C40 currencyOverride = \n";
     expected += "  C45 alternateCurrency = \n";
     expected += "  POF web = \n";
     expected += "  P1X opaqueEntry = \n";
     expected += "  P1U noMinMaxStayRestr = \n";
     expected += "  P1W onlineFares = \n";
     expected += "  P1Y publishedFares = \n";
     expected += "  P1Z privateFares = \n";
     expected += "  P20 xoFares = \n";
     expected += "  P23 resTicketRestr = 0\n";
     expected += "  P31 returnFareCalc = \n";
     expected += "  PA1 noAdvPurchRestr = \n";
     expected += "  Q0S numberOfSolutions = 0\n";
     expected += "\n";
     expected += "Request:\n";
     expected += "  ticketingDT = 1970-Jan-01 00:00:00\n";
     expected += "  AC0 corporateID = \n";
     expected += "  A10 salePointOverride = \n";
     expected += "  A11 ticketPointOverride = \n";
     expected += "  D01 ticketDateOverride = 1970-Jan-01 00:00:00\n";
     expected += "  MTZ exemptAllTaxes = \n";
     expected += "  P1V lowFareNoAvailability = \n";
     expected += "  PA2 considerMultiAirport = 0\n";
     expected += "  P31 returnFareCalc = \n";
     expected += "  exemptSpecificTaxes = 0\n";
     expected += "\n";
     expected += "Legs:\n";
     expected += "\n";
     expected += "Passenger Types:\n";
     expected += "\n";
     expected += "Billing:\n";
     expected += "  A20 userPseudoCityCode = \n";
     expected += "  Q03 userStation = \n";
     expected += "  Q02 userBranch = \n";
     expected += "  A0E partitionID = \n";
     expected += "  AD0 userSetAddress = \n";
     expected += "  A22 aaaCity = \n";
     expected += "  AA0 aaaSine = \n";
     expected += "  C20 serviceName = \n";
     expected += "  A70 actionCode = \n";
     expected += "\n";
     expected += "Other:\n";
     expected += "  Q0A diagnostic = 0\n";
     expected += "\n";
     expected += "******************************************************\n";
     expected += "\n";
     expected += "\n";
     */

    // For this initial test, it should be enough to say we don't get an empty string as output.
    CPPUNIT_ASSERT(diag.str() != "");
    // CPPUNIT_ASSERT_EQUAL(diag.str(), expected );
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag905CollectorTest);
}
