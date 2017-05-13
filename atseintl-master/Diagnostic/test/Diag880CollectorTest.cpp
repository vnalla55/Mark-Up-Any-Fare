#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Diagnostic/Diag880Collector.h"
#include "test/testdata/TestLocFactory.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "Common/Money.h"

using namespace std;

namespace tse
{
class Diag880CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag880CollectorTest);
  CPPUNIT_TEST(testPrintHeader);
  CPPUNIT_TEST(testPrintSliceAndDiceMatrix);
  CPPUNIT_TEST(testPrintSolution);
  CPPUNIT_TEST(testPrintPiece);
  CPPUNIT_TEST(testPrintPieceNotFound);
  CPPUNIT_TEST(testPrintHardMatchFound);
  CPPUNIT_TEST(testPrintSoftMatchFound);
  CPPUNIT_TEST(testPrintResultWhenWinning);
  CPPUNIT_TEST(testPrintResultWhenNotWinning);
  CPPUNIT_TEST(testPrintResultFail);

  CPPUNIT_TEST_SUITE_END();

private:
  typedef std::pair<std::vector<TravelSeg*>::const_iterator,
                    std::vector<TravelSeg*>::const_iterator> TvlSegPair;
  Diag880Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;
  const Loc* _LOC_DFW;
  const Loc* _LOC_DEN;
  const Loc* _LOC_CHI;
  const Loc* _LOC_NYC;
  const Loc* _LOC_LON;

public:
  void setUp()
  {
    _diagroot = _memHandle.insert(new Diagnostic(Diagnostic880));
    _diagroot->activate();
    _diag = _memHandle.insert(new Diag880Collector(*_diagroot));
    _diag->enable(Diagnostic880);

    _LOC_DFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _LOC_DEN = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDEN.xml");
    _LOC_CHI = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCHI.xml");
    _LOC_NYC = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    _LOC_LON = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
  }

  void tearDown() { _memHandle.clear(); }

  void setupSeg(TravelSeg* seg, std::vector<TravelSeg*>& tvlSeg, const Loc* stop1, const Loc* stop2)
  {
    seg->pnrSegment() = (tvlSeg.empty() ? 1 : tvlSeg.back()->pnrSegment() + 1);
    seg->segmentOrder() = (tvlSeg.empty() ? 1 : tvlSeg.back()->segmentOrder() + 1);
    seg->origin() = stop1;
    seg->origAirport() = stop1->loc();
    seg->boardMultiCity() = stop1->loc();
    seg->destination() = stop2;
    seg->destAirport() = stop2->loc();
    seg->offMultiCity() = stop2->loc();

    tvlSeg.push_back(seg);
  }

  void createSeg(std::vector<TravelSeg*>& tvlSeg,
                 const Loc* stop1,
                 const Loc* stop2,
                 CarrierCode mktCxr,
                 CarrierCode operCxr)
  {
    AirSeg* air1 = _memHandle.create<AirSeg>();

    setupSeg(air1, tvlSeg, stop1, stop2);

    air1->setMarketingCarrierCode(mktCxr);
    air1->setOperatingCarrierCode(operCxr);
  }

  void createArunkSeg(std::vector<TravelSeg*>& tvlSeg, const Loc* stop1, const Loc* stop2)
  {
    ArunkSeg* arunk1 = _memHandle.create<ArunkSeg>();

    setupSeg(arunk1, tvlSeg, stop1, stop2);
  }

  void testPrintHeader()
  {
    ServiceGroup groupCode = "BG";
    ServiceSubTypeCode subCode = "0EF";
    PaxTypeCode paxTypeCode = "CNN";
    std::vector<TravelSeg*> tvlSeg;
    createSeg(tvlSeg, _LOC_DFW, _LOC_DEN, "AA", "NW");
    createArunkSeg(tvlSeg, _LOC_DEN, _LOC_NYC);
    createSeg(tvlSeg, _LOC_NYC, _LOC_LON, "LH", "UA");
    std::vector<TravelSeg*>::const_iterator first = tvlSeg.begin();
    const std::vector<TravelSeg*>::const_iterator end = tvlSeg.end();

    _diag->printHeader(groupCode, subCode, paxTypeCode, first, end, 0);

    CPPUNIT_ASSERT_EQUAL(string("*********************** SLICE AND DICE FOR ********************\n"
                                "GROUP: BG    SUBCODE: 0EF    REQUESTED PAXTYPE: CNN\n"
                                "----------------------- PORTION OF TRAVEL ---------------------\n"
                                "DFW-AA-DEN//NYC-LH-LON\n"
                                "------------------------ FLIGHT SEGMENTS ----------------------\n"
                                "DFW DEN    M-AA  O-NW\n"
                                "DEN NYC\n"
                                "NYC LON    M-LH  O-UA\n"),
                         _diag->str());
  }
  void testPrintSliceAndDiceMatrix()
  {
    TseUtil::SolutionSet solutions;

    std::vector<TravelSeg*> tvlSeg1;
    std::vector<TravelSeg*> tvlSeg2;
    std::vector<TvlSegPair*> routes1;
    createSeg(tvlSeg1, _LOC_DFW, _LOC_DEN, "AA", "NW");
    createSeg(tvlSeg1, _LOC_CHI, _LOC_NYC, "UA", "NW");
    createSeg(tvlSeg2, _LOC_NYC, _LOC_LON, "LH", "UA");
    TvlSegPair route1(tvlSeg1.begin(), tvlSeg1.end()); // fm1.boardMultiCity() = _LOC_DFW->loc();
                                                       // fm1.offMultiCity() = _LOC_NYC->loc();
    TvlSegPair route2(tvlSeg2.begin(), tvlSeg2.end()); // fm2.boardMultiCity() = _LOC_NYC->loc();
                                                       // fm2.offMultiCity() = _LOC_LON->loc();
    routes1.push_back(&route1);
    routes1.push_back(&route2);
    solutions.insert(TseUtil::Solution(0, routes1));

    std::vector<TravelSeg*> tvlSeg3;
    std::vector<TvlSegPair*> routes2;
    createSeg(tvlSeg3, _LOC_CHI, _LOC_NYC, "LH", "UA");
    createSeg(tvlSeg3, _LOC_NYC, _LOC_LON, "LH", "UA");
    TvlSegPair route3(tvlSeg3.begin(), tvlSeg3.end()); // fm3.boardMultiCity() = _LOC_CHI->loc();
                                                       // fm3.offMultiCity() = _LOC_LON->loc();
    routes2.push_back(&route3);
    solutions.insert(TseUtil::Solution(1, routes2));

    std::vector<TravelSeg*> tvlSeg4;
    std::vector<TvlSegPair*> routes3;
    createSeg(tvlSeg4, _LOC_CHI, _LOC_NYC, "LH", "UA");
    TvlSegPair route4(tvlSeg4.begin(), tvlSeg4.end()); // fm4.boardMultiCity() = _LOC_CHI->loc();
                                                       // fm4.offMultiCity() = _LOC_NYC->loc();
    routes3.push_back(&route4);
    solutions.insert(TseUtil::Solution(2, routes3));

    _diag->printSliceAndDiceMatrix(solutions, 1000);

    CPPUNIT_ASSERT_EQUAL(
        string("----------------------- SLICE-DICE MATRIX ---------------------\n"
               "---------------------- ALL SECTORS COVERED --------------------\n"
               "1. DFW NYC  NYC LON  \n"
               "----------------------1 SECTOR MISSING ------------------------\n"
               "2. CHI LON  \n"
               "----------------------2 SECTORS MISSING -----------------------\n"
               "3. CHI NYC  \n"
               "***************************************************************\n"),
        _diag->str());
  }

  void testPrintSolution()
  {
    std::vector<TvlSegPair*> routes;
    std::vector<TravelSeg*> tvlSeg1;
    std::vector<TravelSeg*> tvlSeg2;
    createSeg(tvlSeg1, _LOC_DFW, _LOC_DEN, "AA", "NW");
    createArunkSeg(tvlSeg1, _LOC_DEN, _LOC_CHI);
    createSeg(tvlSeg1, _LOC_CHI, _LOC_NYC, "UA", "NW");
    createSeg(tvlSeg2, _LOC_NYC, _LOC_LON, "LH", "UA");
    TvlSegPair route1(tvlSeg1.begin(), tvlSeg1.end());
    TvlSegPair route2(tvlSeg2.begin(), tvlSeg2.end());
    routes.push_back(&route1);
    routes.push_back(&route2);

    _diag->printSolution(routes, 1);

    CPPUNIT_ASSERT_EQUAL(string("1. DFW-AA-DEN//CHI-UA-NYC  NYC-LH-LON  \n \n"), _diag->str());
  }

  void testPrintPiece()
  {
    std::vector<TravelSeg*> tvlSeg;
    createSeg(tvlSeg, _LOC_DFW, _LOC_DEN, "AA", "NW");
    createArunkSeg(tvlSeg, _LOC_DEN, _LOC_CHI);
    createSeg(tvlSeg, _LOC_CHI, _LOC_NYC, "UA", "NW");
    TvlSegPair route(tvlSeg.begin(), tvlSeg.end());
    Money feeMoney(50.6, "NUC");
    CarrierCode carrierCode = "NW";
    bool isMarketing = false;
    DateTime tktDate;

    _diag->printPiece(route, feeMoney, carrierCode, isMarketing, tktDate);

    CPPUNIT_ASSERT_EQUAL(string("DFW-AA-DEN//CHI-UA-NYC    M- AA/UA  O- NW/NW\n"
                                "CXR: NW - OPERATING    FEE: NUC50.60\n \n"),
                         _diag->str());
  }

  void testPrintPieceNotFound()
  {
    std::vector<TravelSeg*> tvlSeg;
    createSeg(tvlSeg, _LOC_DFW, _LOC_DEN, "AA", "NW");
    createArunkSeg(tvlSeg, _LOC_DEN, _LOC_CHI);
    createSeg(tvlSeg, _LOC_CHI, _LOC_NYC, "UA", "NW");
    TvlSegPair route(tvlSeg.begin(), tvlSeg.end());

    _diag->printPieceNotFound(route);

    CPPUNIT_ASSERT_EQUAL(string("DFW-AA-DEN//CHI-UA-NYC\n"
                                "NOT FOUND\n \n"),
                         _diag->str());
  }

  void testPrintHardMatchFound()
  {
    std::vector<TravelSeg*> tvlSeg;
    createSeg(tvlSeg, _LOC_DFW, _LOC_DEN, "AA", "NW");
    createSeg(tvlSeg, _LOC_DEN, _LOC_NYC, "UA", "NW");
    TvlSegPair route(tvlSeg.begin(), tvlSeg.end());

    _diag->printHardMatchFound(route);

    CPPUNIT_ASSERT_EQUAL(string("DFW-AA-DEN-UA-NYC\n"
                                "HARD MATCH FOUND\n \n"),
                         _diag->str());
  }

  void testPrintSoftMatchFound()
  {
    std::vector<TravelSeg*> tvlSeg;
    createSeg(tvlSeg, _LOC_DFW, _LOC_DEN, "AA", "NW");
    createSeg(tvlSeg, _LOC_DEN, _LOC_NYC, "UA", "NW");
    createArunkSeg(tvlSeg, _LOC_NYC, _LOC_CHI);
    createSeg(tvlSeg, _LOC_CHI, _LOC_DFW, "UA", "NW");
    TvlSegPair route(tvlSeg.begin(), tvlSeg.end());

    _diag->printSoftMatchFound(route);

    CPPUNIT_ASSERT_EQUAL(string("DFW-AA-DEN-UA-NYC//CHI-UA-DFW\n"
                                "SOFT MATCH FOUND\n \n"),
                         _diag->str());
  }

  void testPrintResultWhenWinning()
  {
    Money feeMoney(50.6, "NUC");
    bool isWinning = true;
    DateTime tktDate;

    _diag->printResult(feeMoney, isWinning, tktDate);

    CPPUNIT_ASSERT_EQUAL(
        string("TOTAL: NUC50.60\n"
               "STATUS: PASS - LOWEST FEE/MOST FLIGHT SEGMENTS SELECTED\n"
               "***************************************************************\n"),
        _diag->str());
  }

  void testPrintResultWhenNotWinning()
  {
    Money feeMoney(60.5, "NUC");
    bool isWinning = false;
    DateTime tktDate;

    _diag->printResult(feeMoney, isWinning, tktDate);

    CPPUNIT_ASSERT_EQUAL(
        string("TOTAL: NUC60.50\n"
               "STATUS: SOFT PASS - NOT LOWEST FEE OR TOO FEW FLIGHT SEGMENTS\n"
               "***************************************************************\n"),
        _diag->str());
  }

  void testPrintResultFail()
  {
    Money feeMoney(60.5, "NUC");
    DateTime tktDate;

    _diag->printResultFail(feeMoney, tktDate);

    CPPUNIT_ASSERT_EQUAL(
        string("TOTAL: NUC60.50\n"
               "STATUS: SOFT PASS - NOT ALL PORTION MATCHED\n"
               "***************************************************************\n"),
        _diag->str());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag880CollectorTest);
} // tse
