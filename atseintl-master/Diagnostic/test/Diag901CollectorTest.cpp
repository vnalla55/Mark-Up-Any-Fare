#include <ctime>
#include <iostream>
#include <unistd.h>
#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareMarket.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag901Collector.h"
#include "DataModel/Trx.h"

using namespace std;
namespace tse
{
class Diag901CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag901CollectorTest);
  CPPUNIT_TEST(testStreamingOperatorFareMarket);
  CPPUNIT_TEST_SUITE_END();

public:
  void testStreamingOperatorFareMarket()
  {
    Diagnostic diagroot(AllFareDiagnostic);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag901Collector diag(diagroot);
    diag.enable(AllFareDiagnostic);

    CPPUNIT_ASSERT(diag.isActive());

    FareMarket fm;
    diag << fm;
    CPPUNIT_ASSERT(diag.str() == "");
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag901CollectorTest);
}
