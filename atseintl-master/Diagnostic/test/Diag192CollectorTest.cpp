//-----------------------------------------------------------------------------
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "Diagnostic/Diag192Collector.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

namespace
{
const std::string expectedSegDescRow = " 3 AA 123X 06JUN06 KRKDFW AREAS: FROM 1 TO 2 TRANSATL\n";
const std::string defaultCxrs = "   CARRIER: AA OPERATING:  MARKETING: AA\n";
const std::string header = "**************** ITIN ANALYZER DIAGNOSTICS 192 ****************\n";
const std::string line = "***************************************************************\n";
const std::string tc = "TRIP CHARACTERISTIC:\n";
const std::string vc = "VALIDATING CARRIER: \n";
const std::string turnaroundTitle = "\nTURNAROUND/FURTHEST POINT DETERMINATION\n";
const std::string turnaroundErr = "\nERROR: TURNAROUND/FURTHEST POINT NOT SET!\n";
const std::string turnaroundZero = " 3 KRKDFW GREAT CIRCLE MILES: 0\n";
}
class Diag192CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag192CollectorTest);
  CPPUNIT_TEST(testArunk);
  CPPUNIT_TEST(testOpen);
  CPPUNIT_TEST(testOpenEmpty);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _diag192Collector = _memHandle(new Diag192Collector);
    _diag192Collector->activate();

    _airSeg = _memHandle(new AirSeg);
    _airSeg->carrier() = "AA";
    _airSeg->flightNumber() = 123;
    _airSeg->setBookingCode(BookingCode('X'));
    _airSeg->departureDT() = DateTime(2006, 6, 6);
    _airSeg->pssDepartureDate() = "2007-07-07";
    _airSeg->origAirport() = "KRK";
    _airSeg->destAirport() = "DFW";
    _airSeg->pnrSegment() = 3;
    _airSeg->unflown() = false;
    addLocs(*_airSeg);
    PricingTrx* trx = _memHandle(new PricingTrx);
    trx->setOptions(_memHandle(new PricingOptions));
    _diag192Collector->trx() = trx;

    _itin = _memHandle(new Itin);
    _itin->travelSeg().push_back(_airSeg);
  }

  void tearDown() { _memHandle.clear(); }

  void addLocs(TravelSeg& ts)
  {
    ts.origin() = _memHandle(new Loc);
    const_cast<Loc*>(ts.origin())->area() = IATA_AREA1;
    ts.destination() = _memHandle(new Loc);
    const_cast<Loc*>(ts.destination())->area() = IATA_AREA2;
  }

  void testArunk()
  {
    std::string expected = expectedSegDescRow + defaultCxrs +
                           " 0 ARNK        N/A     AREAS: FROM 1 TO 2 TRANSATL\n" + line + tc + vc +
                           line + turnaroundTitle + turnaroundZero + turnaroundErr + line;
    ArunkSeg as;
    addLocs(as);
    _itin->travelSeg().push_back(&as);
    _diag192Collector->printItin(*_itin);

    CPPUNIT_ASSERT_EQUAL(expected, _diag192Collector->str());
  }

  void testOpen()
  {
    std::string expected = " 3 AAOPENX 07JUL07 KRKDFW AREAS: FROM 1 TO 2 TRANSATL\n" +
                           defaultCxrs + line + tc + vc + line + turnaroundTitle + turnaroundErr +
                           line;

    _airSeg->segmentType() = Open;
    _diag192Collector->printItin(*_itin);

    CPPUNIT_ASSERT_EQUAL(expected, _diag192Collector->str());
  }

  void testOpenEmpty()
  {
    std::string expected = " 3 AAOPENX    NONE KRKDFW AREAS: FROM 1 TO 2 TRANSATL\n" +
                           defaultCxrs + line + tc + vc + line + turnaroundTitle + turnaroundErr +
                           line;

    _airSeg->segmentType() = Open;
    _airSeg->pssDepartureDate().clear();
    _diag192Collector->printItin(*_itin);

    CPPUNIT_ASSERT_EQUAL(expected, _diag192Collector->str());
  }

private:
  Diag192Collector* _diag192Collector;
  Itin* _itin;
  AirSeg* _airSeg;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag192CollectorTest);
}
