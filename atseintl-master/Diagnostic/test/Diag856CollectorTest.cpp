#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include "test/include/CppUnitHelperMacros.h"
#define protected public
#include "DataModel/NoPNRPricingTrx.h"
#undef protected
#include "Diagnostic/Diag856Collector.h"

#include "DBAccess/NoPNROptions.h"
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;
namespace tse
{
class Diag856CollectorTest : public Diag856Collector, public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag856CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(test_getSegmentsInfo);
  CPPUNIT_TEST(test_getCommonDiagBodyPart);
  CPPUNIT_TEST(test_process);
  CPPUNIT_TEST_SUITE_END();

public:
  virtual void setUp() {}
  virtual void tearDown() {}

  Diag856CollectorTest() { _active = true; }

  void testConstructor()
  {
    // we are here - therefore Diag856Collector has been constructed
    CPPUNIT_ASSERT(true);
  }
  void test_getSegmentsInfo() {}

  void test_getCommonDiagBodyPart() {}

  void test_process()
  {
    NoPNRPricingTrx* tstTrx = new NoPNRPricingTrx;
    tstTrx->_noPNROptions = getTestOptions();
    //  NoPNROptions** tooverwrite = (&(tTrx->_noPNROptions)) ;
    //  *tooverwrite = getTestOptions() ;
    process(*tstTrx);
    // cout << "----------- TEST RESPONSE ----------\n";
    /*cout <<*/ this->str();
    // cout << "----------- END TEST RESPONSE ------\n";
    CPPUNIT_ASSERT(true); // survived !
  }

protected:
  NoPNROptions* getTestOptions()
  {
    // return some mock data
    NoPNROptions* opt = new NoPNROptions;

    opt->userApplType() = 'C';
    /*UserApplCode*/ opt->userAppl() = "RIBA";
    /*LocKey*/ opt->loc1().locType() = 'N';
    opt->loc1().loc() = "PL";
    opt->wqNotPermitted() = 'A';
    opt->maxNoOptions() = 17;
    opt->wqSort() = 'B';
    opt->wqDuplicateAmounts() = 'C';
    opt->fareLineHeaderFormat() = 'D';
    opt->passengerDetailLineFormat() = 'E';
    opt->fareLinePTC() = 'F';
    opt->primePTCRefNo() = 'G';
    opt->secondaryPTCRefNo() = 'H';
    opt->fareLinePTCBreak() = 'I';
    opt->passengerDetailPTCBreak() = 'J';
    opt->negPassengerTypeMapping() = 'K';
    opt->noMatchOptionsDisplay() = 'L';
    opt->allMatchTrailerMessage() = 'M';
    opt->matchIntegratedTrailer() = 'N';
    opt->accompaniedTvlTrailerMsg() = 'O';
    opt->rbdMatchTrailerMsg() = 'P';
    opt->rbdNoMatchTrailerMsg() = 'Q';
    opt->rbdNoMatchTrailerMsg2() = 'R';
    opt->displayFareRuleWarningMsg() = 'S';
    opt->displayFareRuleWarningMsg2() = 'T';
    opt->displayFinalWarningMsg() = 'U';
    opt->displayFinalWarningMsg2() = 'V';
    opt->displayTaxWarningMsg() = 'W';
    opt->displayTaxWarningMsg2() = 'Y';
    opt->displayPrivateFareInd() = 'X';
    opt->displayNonCOCCurrencyInd() = 'Z';
    opt->displayTruePTCInFareLine() = '1';
    opt->applyROInFareDisplay() = '2';
    opt->alwaysMapToADTPsgrType() = '3';
    opt->noMatchRBDMessage() = '4';
    opt->noMatchNoFaresErrorMsg() = '5';
    opt->totalNoOptions() = 15;

    NoPNROptionsSeg* seg1 = new NoPNROptionsSeg;
    NoPNROptionsSeg* seg2 = new NoPNROptionsSeg;
    NoPNROptionsSeg* seg3 = new NoPNROptionsSeg;

    seg1->seqNo() = 0;
    seg1->noDisplayOptions() = 4;
    seg1->fareTypeGroup() = 6;

    seg2->seqNo() = 33;
    seg2->noDisplayOptions() = 5;
    seg2->fareTypeGroup() = 2;

    seg3->seqNo() = 92;
    seg3->noDisplayOptions() = 6;
    seg3->fareTypeGroup() = 3;

    opt->segs().push_back(seg1);
    opt->segs().push_back(seg2);
    opt->segs().push_back(seg3);

    return opt;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag856CollectorTest);
}
