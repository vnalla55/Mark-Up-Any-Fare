#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PaxType.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "Diagnostic/Diag460Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/RoutingInfo.h"
#include "Routing/TravelRoute.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"

using namespace std;
namespace tse
{
class Diag460CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag460CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_SKIP_TEST(testDisplayHeader);
  CPPUNIT_SKIP_TEST(testDisplayRoutingStatus);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try
    {
      Diagnostic diagroot(Diagnostic460);
      Diag460Collector diag(diagroot);

      string str = diag.str();
      CPPUNIT_ASSERT_EQUAL(string(""), str);
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  /* public */
  //------------------------------------------------------------------
  // Test Empty Display - Header Only
  //------------------------------------------------------------------
  void testDisplayHeader()
  {
    Diagnostic diagroot(Diagnostic460);
    diagroot.activate();

    Diag460Collector diag(diagroot);

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic460);

    CPPUNIT_ASSERT(diag.isActive());

    diag.displayHeader();

    string str = diag.str();

    string expected;
    expected = "\nFARES VALIDATED BY ROUTINGS IN FARE MARKET                  \n";
    expected += "--------------------------------------------------------------\n";
    expected += " \n";
    expected += " FARE BASIS  V T  RTG  RTG  RTG O O      AMT CUR PAX  PASS SUR\n";
    expected += "             N P  NUM TRF1 TRF2 R I              TPE  FAIL    \n";
    expected += "------------ - - ---- ---- ---- - - -------- --- --- ---------\n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }

  //------------------------------------------------------------------------------
  // Display Routing Status
  //------------------------------------------------------------------------------
  void testDisplayRoutingStatus()
  {
    Diagnostic diagroot(Diagnostic460);
    diagroot.activate();

    Diag460Collector diag(diagroot);

    // Now, enable it and do the same streaming operation.
    diag.enable(Diagnostic460);

    CPPUNIT_ASSERT(diag.isActive());

    FareMarket fareMarket;
    PaxTypeFare paxTypeFare;
    PaxType paxType;
    Fare fare;
    paxTypeFare.initialize(&fare, &paxType, &fareMarket);
    const_cast<PaxTypeCode&>(paxTypeFare.fcasPaxType()) = std::string("ADT");

    FareClassAppInfo frClassAppInfo;
    FareClassAppSegInfo frClassAppSegInfo;
    TariffCrossRefInfo trfCrossRefInfo;

    Fare* fare1 = paxTypeFare.fare();
    FareInfo frInfo;
    fare1->setFareInfo(&frInfo);
    FareInfo* fareInfo = (FareInfo*)(fare1->fareInfo());
    fareInfo->_fareClass = "QES14QNR";
    fareInfo->_ruleNumber = "6059";
    fareInfo->_fareTariff = 77;
    fareInfo->_fareAmount = 106;
    fareInfo->_currency = "USD";
    fareInfo->_routingNumber = "0599";

    paxTypeFare.fareClassAppInfo() = const_cast<FareClassAppInfo*>(&frClassAppInfo);
    FareClassAppInfo* fareClassAppInfo =
        const_cast<FareClassAppInfo*>(paxTypeFare.fareClassAppInfo());
    fareClassAppInfo->_fareType = "ADT";

    fare1->setTariffCrossRefInfo(const_cast<TariffCrossRefInfo*>(&trfCrossRefInfo));
    TariffCrossRefInfo* tariffCrossRefInfo =
        const_cast<TariffCrossRefInfo*>(fare1->tariffCrossRefInfo());
    tariffCrossRefInfo->_routingTariff1 = 99;
    tariffCrossRefInfo->_routingTariff2 = 0;

    paxTypeFare.fareClassAppSegInfo() = const_cast<FareClassAppSegInfo*>(&frClassAppSegInfo);
    FareClassAppSegInfo* fareClassAppSegInfo =
        const_cast<FareClassAppSegInfo*>(paxTypeFare.fareClassAppSegInfo());
    fareClassAppSegInfo->_paxType = "ADT";
    fare1->nucFareAmount() = 106;

    paxTypeFare.status().setNull();
    paxTypeFare.setIsRouting(true);
    paxTypeFare.setRoutingProcessed(true);
    paxTypeFare.setRoutingValid(true);
    paxTypeFare.status().set(PaxTypeFare::PTF_Discounted);
    paxTypeFare.mileageSurchargePctg() = 25;

    fareInfo->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    fareInfo->_vendor = Vendor::ATPCO;
    fareInfo->_directionality = FROM;

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "BOS";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");

    // Push the paxTypeFare onto the allPaxTypeFareVector in fare Market
    std::vector<PaxTypeFare*> allPaxTypeFareVec;
    fareMarket.allPaxTypeFare().push_back(&paxTypeFare);

    FareClassCode fareClass, fareBasis;

    Diag460Collector::PaxTypeFareFilter filter(fareClass, fareBasis);
    diag.displayRoutingStatus(fareMarket, tvlRoute, filter);

    string str = diag.str();

    string expected;
    expected = "AA  BOS-AA-LON\n";
    // expected += " \n";
    expected += "\nFARES VALIDATED BY ROUTINGS IN FARE MARKET                  \n";
    expected += "--------------------------------------------------------------\n";
    expected += " \n";
    expected += " FARE BASIS  V T  RTG  RTG  RTG O O      AMT CUR PAX  PASS SUR\n";
    expected += "             N P  NUM TRF1 TRF2 R I              TPE  FAIL    \n";
    expected += "------------ - - ---- ---- ---- - - -------- --- --- ---------\n";
    expected += "**QES14QNR   A R 0599   99    0 R O   106.00 USD ADT  P   25M\n";
    expected += " \n";
    CPPUNIT_ASSERT_EQUAL(expected, str);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag460CollectorTest);
}
