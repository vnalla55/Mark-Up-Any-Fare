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
#include "Diagnostic/Diag854Collector.h"
#include "test/include/CppUnitHelperMacros.h"
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

using namespace std;
namespace tse
{
class Diag854CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag854CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(DiagnosticNone);
      Diag854Collector diag(diagroot);

      string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  //  *********************************************
  //  Create objects and call diag854Collector
  //  *********************************************

  void testProcess()
  {
    Diagnostic diagroot(Diagnostic854);
    diagroot.activate();
    Diag854Collector diag(diagroot);
    diag.enable(Diagnostic854);
    CPPUNIT_ASSERT(diag.isActive());

    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic854;
    trx.diagnostic().activate();
    // const FareCalcConfig* fcc = new FareCalcConfig();

    FareMarket* fm = new tse::FareMarket();
    trx.fareMarket().push_back(fm);

    PricingRequest req;
    trx.setRequest(&req);
    Agent agent;
    Itin itin;
    trx.itin().push_back(&itin);
    trx.getRequest()->ticketingAgent() = &agent;
    PaxType paxType;
    trx.paxType().push_back(&paxType);
    paxType.number() = 2; // Number of passengers; only entry used.
    AirSeg travelSeg;
    trx.travelSeg().push_back(&travelSeg);
    itin.travelSeg().push_back(&travelSeg);

    Loc dfw;
    Loc lax;
    LocCode dfwLoc("DFW");
    LocCode laxLoc("LAX");
    dfw.loc() = dfwLoc; //
    dfw.nation() = "US";
    lax.loc() = laxLoc; //
    lax.nation() = "US";

    travelSeg.segmentOrder() = 0;
    travelSeg.geoTravelType() = GeoTravelType::ForeignDomestic;
    travelSeg.origin() = &dfw;
    travelSeg.destination() = &lax;
    travelSeg.origAirport() = "DFW";
    travelSeg.destAirport() = "LAX";

    DateTime departdt(2004, 6, 20, 14, 40, 0);
    travelSeg.departureDT() = departdt;
    DateTime arrivaldt(2004, 6, 22, 16, 45, 23);
    travelSeg.arrivalDT() = arrivaldt;

    trx.setOptions(new PricingOptions());

    // currently (23Feb2005), Diag854 has no effective code
    // for now, just keep the compiler happy
    //     diag.process( trx, fcc );

    string str = diag.str();
    /*****
     //std::cout << "X" << std::endl;
     //std::cout << str << std::endl;
     //std::cout << "X" << std::endl;

     string expected;
     expected += "1.    0  20JUN DFW  1440 1645 2 22JUN  \n";
     expected += "\n";
     expected += "\n";
     expected += "         1         2         3         4         5         6\n";
     expected += "123456789012345678901234567890123456789012345678901234567890123\n";
     expected += "PSGR TYPE  DEF\n";
     expected += "     CXR RES DATE  FARE BASIS      NVB   NVA     BG\n";
     expected += " DFW\n";
     expected += "       20JUN MYFBASIS\n";
     expected += "\n";
     expected += "         1         2         3         4         5         6\n";
     expected += "123456789012345678901234567890123456789012345678901234567890123\n";
     expected += "20JUN DEPARTURE DATE-----LAST DAY TO PURCHASE 01JAN \n";
     expected += "01JAN TKT/TL01JAN70\n";
     expected += "       BASE FARE      EQUIV AMT      TAXES             TOTAL\n";
     expected += " 1-      0.00                                 0.00ADT \n";


     //std::cout << "Length of expected = " << expected.size() << std::endl;
     //std::cout << "Length of actual  = " << str.size() << std::endl;

     //for( int i = 0; i < str.size(); i++ )
     //{
     //    std::cout << str.substr(i,1);
     //    if( str.substr(i,1) != expected.substr(i,1) )
     //    {
     //       std::cout << "XX Difference detected" << std::endl;
     //       break;
     //    }
     //}

     CPPUNIT_ASSERT_EQUAL( expected, str );
     ****/
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag854CollectorTest);
}
