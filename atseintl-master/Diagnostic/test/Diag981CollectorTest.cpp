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
#include "Diagnostic/Diag981Collector.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include <unistd.h>
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "Common/DateTime.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diagnostic.h"
#include "DBAccess/FareCalcConfig.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Response.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/FareMarket.h"
#include "test/testdata/TestLocFactory.h"

using namespace std;
namespace tse
{
class Diag981CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag981CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testStreamFareMarketNoFares);
  CPPUNIT_TEST(testStreamFareMarketNoFaresADTPaxType);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _diag.rootDiag() = &_diagroot;
    _diag.enable(Diagnostic981);
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

  void testStreamFareMarketNoFares()
  {
    FareMarket* fm = createBaseFareMarket();

    _diag << *fm;

    string expected = "** DIAG 981 FAREMARKET'S FARES ***\n";
    expected += " #  .IN.\n";
    expected += "Travel Date :  01JAN\n\n";
    expected += "NO FARES FOUND FOR MARKET : DALLAS FT WORTH-NEW YORK JFK\n\n";
    expected += "*** END OF DIAG 981 ***\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag.str());
  }

  void testStreamFareMarketNoFaresADTPaxType()
  {
    FareMarket* fm = createBaseFareMarket();
    PaxTypeBucket ptc;
    PaxType paxType;
    ptc.requestedPaxType() = &paxType;
    fm->paxTypeCortege().push_back(ptc);
    paxType.paxType() = "ADT";

    _diag << *fm;

    string expected = "** DIAG 981 FAREMARKET'S FARES ***\n";
    expected += " #  .IN.\n";
    expected += "Travel Date :  01JAN\n\n";
    expected += "NO FARES FOUND FOR MARKET : DFW-JFK. REQUESTED PAXTYPE : ADT\n\n";
    expected += "*** END OF DIAG 981 ***\n";
    CPPUNIT_ASSERT_EQUAL(expected, _diag.str());
  }

  FareMarket* createBaseFareMarket()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->direction() = FMDirection::INBOUND;
    fm->travelDate() = DateTime(2009, 1, 1);
    fm->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    fm->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    return fm;
  }

private:
  Diagnostic _diagroot;
  Diag981Collector _diag;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag981CollectorTest);
}
