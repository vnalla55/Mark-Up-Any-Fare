#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag220Collector.h"

#include "DataModel/PricingTrx.h"
#include "DataModel/AirSeg.h"

namespace tse
{
class Diag220CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag220CollectorTest);
  CPPUNIT_TEST(testPaxTypeDisplay);
  CPPUNIT_TEST_SUITE_END();

  //---------------------------------------------------------------------
  // testPaxTypeDisplay()
  //---------------------------------------------------------------------
  void testPaxTypeDisplay()
  {
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);
    PaxType pt;
    trx.paxType().push_back(&pt);
    pt.paxType() = "REQ";
    pt.actualPaxType().insert(std::make_pair("CR", &trx.paxType()));
    Itin itin;
    trx.itin().push_back(&itin);
    AirSeg seg;
    itin.travelSeg().push_back(&seg);
    seg.carrier() = "CR";

    Diagnostic diagroot(Diagnostic220);
    diagroot.activate();

    // First try with a 'blank' object to make sure we dont
    Diag220Collector diag(diagroot);

    diag.enable(Diagnostic220);
    CPPUNIT_ASSERT(diag.isActive());

    diag.displayPaxTypes(trx);

    std::string expected = "REQUESTED PASSENGER TYPE - REQ\n"
                           "CR REQ      \n \n";

    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag220CollectorTest);
}
