//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "Diagnostic/Diag853Collector.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diagnostic.h"
#include "Common/DateTime.h"
#include "DataModel/FarePath.h"
#include "DBAccess/FareCalcConfig.h"
#include <unistd.h>
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;
namespace tse
{
class Diag853CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag853CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

  class MyDataHandle : public DataHandleMock
  {
  public:
    const Loc* getLoc(const LocCode& locCode, const DateTime& date)
    {
      if (locCode == "")
        return 0;
      return DataHandleMock::getLoc(locCode, date);
    }
  };

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag853Collector diag(diagroot);

      string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testProcess()
  {
    MyDataHandle mdh;
    Diagnostic diagroot(Diagnostic853);
    diagroot.activate();
    Diag853Collector diag(diagroot);

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic853);

    CPPUNIT_ASSERT(diag.isActive());

    PricingTrx trx;
    PricingOptions options;
    PricingRequest request;
    Agent agent;
    trx.setOptions(&options);
    trx.setRequest(&request);
    trx.diagnostic().diagnosticType() = Diagnostic853;
    trx.diagnostic().activate();
    request.ticketingAgent() = &agent;

    const FareCalcConfig* fcc = new FareCalcConfig();

    diag.process(trx, fcc);
    string str = diag.str();
    // cout << str << std::endl;

    // std::string expected;

    // expected +="\n     *******  FareCalcConfig  *************** \n";
    // expected +="     userApplType:   \n";
    // expected +="     *******  End of Display *************** \n";

    // CPPUNIT_ASSERT_EQUAL( expected, str );
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag853CollectorTest);
}
