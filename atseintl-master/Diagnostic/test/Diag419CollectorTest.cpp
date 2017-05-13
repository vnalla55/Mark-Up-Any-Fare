#include <time.h>
#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/PaxType.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/DifferentialData.h"
#include "Common/ClassOfService.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag419Collector.h"
#include "Common/Vendor.h"
#include "DataModel/PricingTrx.h"
#include <unistd.h>

using namespace std;
namespace tse
{
class Diag419CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag419CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testStreamingOperatorPaxTypeFare);
  CPPUNIT_TEST(testFinalDiag);
  CPPUNIT_TEST(testShowFareType);
  CPPUNIT_TEST(testTravelSegmentHeader);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor(void)
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag419Collector diag(diagroot);

      string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTravelSegmentHeader(void)
  {
    Diagnostic diagroot(Diagnostic419);
    diagroot.activate();

    Diag419Collector diag(diagroot);
    diag.enable(Diagnostic419);
    string expected;
    expected = "  FARE BASIS   CXR V RULE TAR O O     FARE  CUR PAX BK C STAT\n";
    expected += "       CLASS              NUM R I    AMOUNT     TYP    B\n";

    diag.travelSegmentHeader();
    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }

  void testStreamingOperatorPaxTypeFare(void)
  {
    Diagnostic diagroot(Diagnostic419);
    diagroot.activate();

    Diag419Collector diag(diagroot);
    diag.enable(Diagnostic419);

    PaxTypeFare ptf;
    Fare fare;
    FareInfo fareInfo;
    ptf.status() = PaxTypeFare::PTF_Discounted;
    ptf.status().combine(PaxTypeFare::PTF_FareByRule);
    ptf.status().combine(PaxTypeFare::PTF_Negotiated);
    fareInfo.fareClass() = "Y26S";
    fareInfo.carrier() = "AA";
    fareInfo.vendor() = Vendor::SITA;
    fareInfo.ruleNumber() = "55";
    fareInfo.fareTariff() = 1;
    fareInfo.owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo.directionality() = FROM;

    fare.setFareInfo(&fareInfo);
    ptf.setFare(&fare);

    PaxTypeFare::SegmentStatus status;
    status._bkgCodeSegStatus = PaxTypeFare::BKSS_NOT_YET_PROCESSED;
    ptf.segmentStatus().push_back(status);

    diag << ptf;
    string expected = "Z@Y26S     AA S 55      1 R O  *       NOT YET PROCESSED \n";
    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }

  void testFinalDiag(void)
  {
    Diagnostic diagroot(Diagnostic419);
    diagroot.activate();

    Diag419Collector diag(diagroot);
    PricingTrx trx;

    //  diag.finalDiag (PricingTrx& trx, const FareMarket& mkt);
  }

  void testShowFareType(void)
  {
    Diagnostic diagroot(Diagnostic419);
    diagroot.activate();

    Diag419Collector diag(diagroot);
    PaxTypeFare pTf;
    Fare fareh;
    PaxType paxTypeh;
    FareInfo finfh;
    FareMarket fareMarketh;
    fareh.setFareInfo(&finfh);
    pTf.initialize(&fareh, &paxTypeh, &fareMarketh);
    pTf.status() = PaxTypeFare::PTF_Discounted;

    diag.showFareType(pTf);
    string expected;
    expected = "D ";
    CPPUNIT_ASSERT_EQUAL(expected, diag.str());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag419CollectorTest);
}
