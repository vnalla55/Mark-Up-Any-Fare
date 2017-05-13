#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/IndustryFareAppl.h"
#include "DBAccess/IndustryFareBasisMod.h"
#include "DBAccess/Loc.h"
#include "Fares/IndustryFareController.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestFareMarketFactory.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  std::vector<const FareInfo*> _ret;

public:
  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "")
      return 0;
    return DataHandleMock::getLoc(locCode, date);
  }

  const std::vector<const FareInfo*>& getFaresByMarketCxr(const LocCode& market1,
                                                          const LocCode& market2,
                                                          const CarrierCode& cxr,
                                                          const DateTime& date)
  {
    if (market1 == "CVG" && market2 == "FRA" && cxr == "YY")
      return _ret;
    return DataHandleMock::getFaresByMarketCxr(market1, market2, cxr, date);
  }

  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "")
      return "";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
};
}

class IndustryFareControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IndustryFareControllerTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testMatchFares);
  CPPUNIT_TEST(testMatchFare);
  CPPUNIT_TEST(testDiagHeader);
  CPPUNIT_TEST(testDiagFare);
  CPPUNIT_TEST(testDiagFareAppl);
  CPPUNIT_TEST(testGovCarrierHasFares);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  //-----------------------------------------------------------------
  // testProcess()
  //-----------------------------------------------------------------
  void testProcess()
  {
    MyDataHandle mdh;
    PricingTrx trx;
    Itin itin;
    FareMarket fareMarket;

    PricingOptions options;
    trx.setOptions(&options);

    PricingRequest request;
    trx.setRequest(&request);

    Agent agent;
    request.ticketingAgent() = &agent;

    trx.diagnostic().diagnosticType() = Diagnostic277;
    trx.diagnostic().activate();

    Loc loc1;
    Loc loc2;

    loc1.loc() = "CVG";
    loc2.loc() = "FRA";

    loc1.nation() = ("US");
    loc2.nation() = ("FR");

    loc1.area() = ("1");
    loc2.area() = ("2");

    AirSeg airSeg1;
    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);
    airSeg1.pnrSegment() = 1;

    DateTime departDT;
    airSeg1.departureDT() = departDT;

    fareMarket.origin() = &loc1;
    fareMarket.destination() = &loc2;
    fareMarket.travelSeg().push_back(&airSeg1);
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.governingCarrier() = "DL";
    trx.travelSeg().push_back(&airSeg1);

    IndustryFareController idf(trx, itin, fareMarket);

    CPPUNIT_ASSERT(idf.process());
    // Note they dont get put in here yet, uncomment this later
    // CPPUNIT_ASSERT(!fareMarket.allPaxTypeFare().empty());
  }

  //-----------------------------------------------------------------
  // testMatchFares()
  //-----------------------------------------------------------------
  void testMatchFares() {}

  //-----------------------------------------------------------------
  // testMatchFare()
  //-----------------------------------------------------------------
  void testMatchFare()
  {
    MyDataHandle mdh;
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);
    Itin itin;
    FareMarket fareMarket;

    PricingRequest request;
    trx.setRequest(&request);

    Agent agent;
    request.ticketingAgent() = &agent;

    trx.diagnostic().diagnosticType() = Diagnostic277;
    trx.diagnostic().activate();

    Loc loc1;
    Loc loc2;

    loc1.nation() = ("US");
    loc2.nation() = ("GB");

    loc1.area() = ("1");
    loc2.area() = ("2");

    loc1.loc() = "MIA";
    loc2.loc() = "LON";

    AirSeg airSeg1;
    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    DateTime departDT;
    airSeg1.departureDT() = departDT;

    fareMarket.origin() = &loc1;
    fareMarket.destination() = &loc2;
    fareMarket.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg1);

    IndustryFareController idf(trx, itin, fareMarket);

    PaxTypeFare ptFare;
    IndustryFareAppl appl;
    IndustryFareAppl::ExceptAppl* except = 0;

    Fare fare;
    FareInfo fi;
    FareClassAppInfo fcai;

    fi._fareClass = "FCLS";
    fi._fareAmount = 100;
    fi._fareTariff = 200;
    fcai._fareType = "FTYPE";

    fi._carrier = "BA";
    fi._market1 = "DFW";
    fi._market2 = "LON";

    fare.setFareInfo(&fi);
    ptFare.fareClassAppInfo() = &fcai;
    ptFare.setFare(&fare);

    // Test 'Empty' first
    appl.fareTariff() = -1;
    appl.globalDir() = GlobalDirection::ZZ;
    appl.owrt() = ' ';
    appl.directionality() = ' ';

    CPPUNIT_ASSERT(idf.matchFare(ptFare, appl, except));

    // Now try a couple of directionality checks
    fi._market1 = "DFW";
    fi._market2 = "MIA";
    fi._directionality = BOTH;

    appl.directionality() = 'W'; // within
    appl.loc1().loc() = "US";
    appl.loc1().locType() = 'N';
    appl.userApplType() = 'C';

    CPPUNIT_ASSERT(IndustryFareController::MATCH_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    fi._market2 = "LON";
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));

    appl.directionality() = 'F'; // From

    appl.loc2().loc() = "GB";
    appl.loc2().locType() = 'N';
    CPPUNIT_ASSERT(IndustryFareController::MATCH_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    fi._market1 = "LON";
    fi._market2 = "DFW";
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));

    appl.directionality() = 'B';
    CPPUNIT_ASSERT(IndustryFareController::MATCH_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    fi._market1 = "DFW";
    fi._market2 = "LON";
    CPPUNIT_ASSERT(IndustryFareController::MATCH_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    fi._market1 = "XXX";
    fi._market2 = "XXX";
    appl.directionality() = ' ';
    CPPUNIT_ASSERT(IndustryFareController::MATCH_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    fi._market1 = "";
    fi._market2 = "";
    appl.loc1().loc() = "";
    appl.loc1().locType() = ' ';
    appl.loc2().loc() = "";
    appl.loc2().locType() = ' ';

    // Now start making it fail
    appl.vendor() = "XXX";
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));
    appl.vendor().clear();

    appl.fareTariff() = 100;
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));
    appl.fareTariff() = -1;

    appl.rule() = "AAA";
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));
    appl.rule().clear();

    appl.globalDir() = GlobalDirection::AT;
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));
    appl.globalDir() = GlobalDirection::ZZ;

    appl.loc1().loc() = "DFW";
    appl.loc1().locType() = 'C';
    appl.loc2().loc() = "LON";
    appl.loc2().locType() = 'C';

    appl.directionality() = 'W';
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));

    appl.directionality() = 'F';
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));

    appl.directionality() = 'B';
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));

    appl.directionality() = ' ';
    appl.loc1().loc() = "";
    appl.loc1().locType() = ' ';
    appl.loc2().loc() = "";
    appl.loc2().locType() = ' ';

    appl.cur() = "EUR";
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));
    appl.cur().clear();

    appl.fareType() = "XXX";
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));
    appl.fareType().clear();

    appl.fareClass() = "XXX";
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));
    appl.fareClass().clear();

    appl.footNote() = "XX";
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));
    appl.footNote().clear();

    appl.owrt() = 'O';
    CPPUNIT_ASSERT(IndustryFareController::NO_MATCH == idf.matchFare(ptFare, appl, except));
    appl.owrt() = ' ';

    appl.selectionType() = 'M';
    CPPUNIT_ASSERT(IndustryFareController::MATCH_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    appl.selectionType() = 'Y';
    appl.yyFareAppl() = '5';
    CPPUNIT_ASSERT(IndustryFareController::MATCH_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    appl.selectionType() = 'Y';
    appl.yyFareAppl() = '1';
    CPPUNIT_ASSERT(IndustryFareController::MATCH_DONT_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    appl.selectionType() = 'Y';
    appl.yyFareAppl() = '2';
    CPPUNIT_ASSERT(IndustryFareController::MATCH_DONT_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    appl.selectionType() = 'Y';
    appl.yyFareAppl() = '3';
    CPPUNIT_ASSERT(IndustryFareController::MATCH_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));

    appl.selectionType() = 'Y';
    appl.yyFareAppl() = '4';
    CPPUNIT_ASSERT(IndustryFareController::MATCH_CREATE_FARE ==
                   idf.matchFare(ptFare, appl, except));
  }

  //-----------------------------------------------------------------
  // testDiagHeader()
  //-----------------------------------------------------------------
  void testDiagHeader() {}

  //-----------------------------------------------------------------
  // testDiagFare()
  //-----------------------------------------------------------------
  void testDiagFare()
  {
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);
    Itin itin;
    FareMarket fareMarket;

    PricingRequest request;
    trx.setRequest(&request);

    Agent agent;
    request.ticketingAgent() = &agent;

    trx.diagnostic().diagnosticType() = Diagnostic277;
    trx.diagnostic().activate();

    Loc loc1;
    Loc loc2;

    loc1.nation() = ("US");
    loc2.nation() = ("GB");

    loc1.area() = ("1");
    loc2.area() = ("2");

    AirSeg airSeg1;
    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    DateTime departDT;
    airSeg1.departureDT() = departDT;

    fareMarket.origin() = &loc1;
    fareMarket.destination() = &loc2;
    fareMarket.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg1);

    fareMarket.direction() = FMDirection::OUTBOUND;

    IndustryFareController idf(trx, itin, fareMarket);

    PaxTypeFare ptFare;
    IndustryFareAppl appl;

    Fare fare;
    FareInfo fi;
    FareClassAppInfo fcai;

    fi._fareClass = "FCLS";
    fi._fareAmount = 100;
    fi._fareTariff = 200;
    fcai._fareType = "FTYPE";

    fi._carrier = "BA";
    fi._market1 = "DFW";
    fi._market2 = "LON";

    fare.setFareInfo(&fi);
    ptFare.fareClassAppInfo() = &fcai;
    ptFare.setFare(&fare);

    // Test 'Empty' first
    appl.fareTariff() = -1;
    appl.globalDir() = GlobalDirection::ZZ;
    appl.owrt() = ' ';

    idf.diagFare(ptFare, &appl);
  }

  //-----------------------------------------------------------------
  // testDiagFareAppl()
  //-----------------------------------------------------------------
  void testDiagFareAppl()
  {
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);
    Itin itin;
    FareMarket fareMarket;

    trx.diagnostic().diagnosticType() = Diagnostic272;
    trx.diagnostic().activate();

    PricingRequest request;
    trx.setRequest(&request);

    Agent agent;
    request.ticketingAgent() = &agent;

    Loc loc1;
    Loc loc2;

    loc1.nation() = ("US");
    loc2.nation() = ("GB");

    loc1.area() = ("1");
    loc2.area() = ("2");

    AirSeg airSeg1;
    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    DateTime departDT;
    airSeg1.departureDT() = departDT;

    fareMarket.origin() = &loc1;
    fareMarket.destination() = &loc2;
    fareMarket.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg1);

    IndustryFareController idf(trx, itin, fareMarket);

    PaxTypeFare ptFare;
    IndustryFareAppl appl;

    Fare fare;
    FareInfo fi;
    FareClassAppInfo fcai;

    fi._fareClass = "FCLS";
    fi._fareAmount = 100;
    fi._fareTariff = 200;
    fcai._fareType = "FTYPE";

    fi._carrier = "BA";
    fi._market1 = "DFW";
    fi._market2 = "LON";

    fare.setFareInfo(&fi);
    ptFare.fareClassAppInfo() = &fcai;
    ptFare.setFare(&fare);

    // Test 'Empty' first
    appl.fareTariff() = -1;
    appl.globalDir() = GlobalDirection::ZZ;
    appl.owrt() = ' ';

    IndustryFareBasisMod basis;
    basis.carrier() = "BA";
    basis.userAppl() = 1;
    basis.doNotChangeFareBasisInd() = 'Y';

    idf.diagFareAppl(appl, 0, &basis);
  }

  //-----------------------------------------------------------------
  // testGovCarrierHasFares()
  //-----------------------------------------------------------------
  void testGovCarrierHasFares()
  {
    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);
    Itin itin;
    FareMarket fareMarket;

    PricingRequest request;
    trx.setRequest(&request);

    Agent agent;
    request.ticketingAgent() = &agent;

    trx.diagnostic().diagnosticType() = Diagnostic277;
    trx.diagnostic().activate();

    Loc loc1;
    Loc loc2;

    loc1.nation() = ("US");
    loc2.nation() = ("GB");

    loc1.area() = ("1");
    loc2.area() = ("2");

    AirSeg airSeg1;
    airSeg1.origin() = (&loc1);
    airSeg1.destination() = (&loc2);

    DateTime departDT;
    airSeg1.departureDT() = departDT;

    fareMarket.origin() = &loc1;
    fareMarket.destination() = &loc2;
    fareMarket.travelSeg().push_back(&airSeg1);
    trx.travelSeg().push_back(&airSeg1);

    PaxTypeFare ptFare;
    Fare fare;
    FareInfo fi;
    FareClassAppInfo fcai;

    fi._fareClass = "1";
    fi._fareAmount = 100;
    fi._carrier = "AA";
    fi._market1 = "DFW";
    fi._market2 = "MAN";

    fcai._fareType = "ADD";

    TariffCrossRefInfo tcri;
    tcri._tariffCat = RuleConst::PRIVATE_TARIFF;

    fare.setFareInfo(&fi);
    ptFare.fareClassAppInfo() = &fcai;
    fare.setTariffCrossRefInfo(&tcri);
    ptFare.setFare(&fare);

    fareMarket.allPaxTypeFare().push_back(&ptFare);

    IndustryFareController idf(trx, itin, fareMarket);

    fareMarket.governingCarrier() = "UA";

    CPPUNIT_ASSERT(!idf.govCarrierHasFares(ptFare, ' '));

    fareMarket.governingCarrier() = "AA";
    tcri._tariffCat = 0;
    ptFare.setFare(&fare);

    CPPUNIT_ASSERT(idf.govCarrierHasFares(ptFare, ' '));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(IndustryFareControllerTest);

} // namespace
