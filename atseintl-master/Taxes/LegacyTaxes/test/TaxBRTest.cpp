#include "Taxes/LegacyTaxes/TaxSP9500.h"

#include "Common/TseEnums.h"

#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"

#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxRequest.h"

#include "Taxes/LegacyTaxes/TaxItem.h"

#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class TaxBRTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(TaxBRTest);
  CPPUNIT_TEST(testMultiCityMirrorImage);
  CPPUNIT_TEST(testNotMultiCityMirrorImage);
  CPPUNIT_TEST(testMultiCityStopOver);
  CPPUNIT_TEST(testNotMultiCityStopOver);
  CPPUNIT_TEST_SUITE_END();

public:
  AirSeg* travelSegFrom;
  AirSeg* travelSegTo;
  Loc* origin;
  Loc* destination;

  void setUp()
  {
    origin = new Loc();
    destination = new Loc();

    travelSegFrom = new AirSeg();
    travelSegFrom->origin() = origin;
    travelSegFrom->destination() = destination;

    travelSegTo = new AirSeg();
    travelSegTo->origin() = origin;
    travelSegTo->destination() = destination;
  }

  void tearDown()
  {
    delete origin;
    delete destination;
    delete travelSegFrom;
    delete travelSegTo;
  }

  void testMultiCityMirrorImage()
  {
    travelSegFrom->origAirport() = "BBB";
    travelSegFrom->boardMultiCity() = "JFK";
    travelSegTo->destAirport() = "AAA";
    travelSegTo->offMultiCity() = "JFK";

    TaxSP9500 tax;

    CPPUNIT_ASSERT(tax.isMultiCityMirrorImage(travelSegFrom, travelSegTo));
  }

  void testNotMultiCityMirrorImage()
  {
    travelSegFrom->origAirport() = "BBB";
    travelSegFrom->boardMultiCity() = "JFK";
    travelSegTo->destAirport() = "AAA";
    travelSegTo->offMultiCity() = "DFW";

    TaxSP9500 tax;

    CPPUNIT_ASSERT(!tax.isMultiCityMirrorImage(travelSegFrom, travelSegTo));
  }

  void testMultiCityStopOver()
  {
    travelSegFrom->destAirport() = "BBB";
    travelSegFrom->offMultiCity() = "JFK";
    travelSegTo->origAirport() = "AAA";
    travelSegTo->boardMultiCity() = "JFK";

    TaxSP9500 tax;

    CPPUNIT_ASSERT(tax.isMultiCityStopOver(travelSegFrom, travelSegTo));
  }

  void testNotMultiCityStopOver()
  {
    travelSegFrom->destAirport() = "BBB";
    travelSegFrom->offMultiCity() = "DFW";
    travelSegTo->origAirport() = "AAA";
    travelSegTo->boardMultiCity() = "JFK";

    TaxSP9500 tax;

    CPPUNIT_ASSERT(!tax.isMultiCityStopOver(travelSegFrom, travelSegTo));
  }

private:
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxBRTest);
}
