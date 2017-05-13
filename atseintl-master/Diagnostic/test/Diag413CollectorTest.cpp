#include "Common/ClassOfService.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag413Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/testdata/TestFareMarketFactory.h"

namespace tse
{
class Diag413CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag413CollectorTest);
  CPPUNIT_TEST(testStreamingOperatorFareMarket);
  CPPUNIT_TEST(testStreamingOperatorFareMarket2);
  CPPUNIT_TEST(testStreamingOperatorFareMarket3);
  CPPUNIT_TEST(testLineSkip);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag413Collector diag(diagroot);

      std::string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(std::string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  //---------------------------------------------------------------------
  // testStreamingOperatorFareMarket()
  //---------------------------------------------------------------------
  void testStreamingOperatorFareMarket()
  {

    Diagnostic diagroot(Diagnostic413);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag413Collector diag(diagroot);
    diag.enable(Diagnostic413);

    CPPUNIT_ASSERT(diag.isActive());

    FareMarket* fMkt = TestFareMarketFactory::create(
        "/vobs/atseintl/Fares/test/data/FareCurrencySelection/IntlPrime/fareMarket.xml");

    diag << *fMkt;

    std::string expected;
    std::string actual = diag.str();

    expected += "---------------------------------------------------------------\n";
    expected += "\n                  DIFFERENTIALS DIAGNOSTIC \n\n";
    //   expected += " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
    expected += "\nTHROUGH MARKET  LHR-BA-BOM    INTERNATIONAL\n";
    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }

  //---------------------------------------------------------------------
  // testStreamingOperatorFareMarket2()
  //---------------------------------------------------------------------
  void testStreamingOperatorFareMarket2()
  {

    Diagnostic diagroot(Diagnostic413);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag413Collector diag(diagroot);
    diag.enable(Diagnostic413);

    CPPUNIT_ASSERT(diag.isActive());
    FareMarket* fMkt = TestFareMarketFactory::create(
        "/vobs/atseintl/Fares/test/data/DRVController/fareMarket2.xml");

    diag << *fMkt;

    std::string expected;
    std::string actual = diag.str();

    expected += "---------------------------------------------------------------\n";
    expected += "\n                  DIFFERENTIALS DIAGNOSTIC \n\n";
    //   expected += " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
    expected += "\nTHROUGH MARKET  DFW-AA-NYC    US/CA\n";

    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++
  //---------------------------------------------------------------------
  // testStreamingOperatorFareMarket3()
  //---------------------------------------------------------------------
  void testStreamingOperatorFareMarket3()
  {

    Diagnostic diagroot(Diagnostic413);
    diagroot.activate();

    // First test an 'empty' FareMarket
    Diag413Collector diag(diagroot);
    diag.enable(Diagnostic413);

    CPPUNIT_ASSERT(diag.isActive());

    //  FareMarket fm;
    // string expected("");
    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    dest.loc() = "LAX";

    FareMarket fareMarket;
    AirSeg mockAirSeg;
    mockAirSeg.origin() = &origin;
    mockAirSeg.destination() = &dest;

    mockAirSeg.boardMultiCity() = "DFW";
    mockAirSeg.offMultiCity() = "LAX";

    //   fareMarket.travelSeg().push_back(&mockAirSeg);

    CarrierCode bcCarrier = "AA"; // The BookingCode objects carrier.

    fareMarket.origin() = &origin;
    fareMarket.destination() = &dest;

    fareMarket.governingCarrier() = bcCarrier;

    // fareMarket.set_TravelBoundary(FMTravelBoundary::TravelWithinUSCA);  // Domestic

    fareMarket.travelBoundary() = FMTravelBoundary::TravelWithinUSCA; // International

    ClassOfService cos;
    CabinType cabin;

    cos.bookingCode() = "Y";
    cos.numSeats() = 10;
    cos.cabin() = cabin;

    ClassOfService cos1;

    cos1.bookingCode() = "F";
    cos1.numSeats() = 1;
    cos1.cabin() = cabin;

    ClassOfService cos2;

    cos2.bookingCode() = "A";
    cos2.numSeats() = 0;
    cos2.cabin() = cabin;

    std::vector<ClassOfService*> _cos;

    _cos.push_back(&cos);
    _cos.push_back(&cos1);
    _cos.push_back(&cos2);

    fareMarket.classOfServiceVec().push_back(&_cos);

    diag << fareMarket;

    std::string expected;
    std::string actual = diag.str();

    expected += "---------------------------------------------------------------\n";
    expected += "\n                  DIFFERENTIALS DIAGNOSTIC \n\n";
    expected += " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";

    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }

  //---------------------------------------------------------------------
  // testLineSkip()
  //---------------------------------------------------------------------
  void testLineSkip()
  {
    Diagnostic diagroot(Diagnostic413);
    diagroot.activate();
    Diag413Collector diag(diagroot);

    diag.enable(Diagnostic413);
    CPPUNIT_ASSERT(diag.isActive());
    diag.lineSkip(0);
    diag.lineSkip(1);
    diag.lineSkip(2);
    diag.lineSkip(3);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag413CollectorTest);
}
