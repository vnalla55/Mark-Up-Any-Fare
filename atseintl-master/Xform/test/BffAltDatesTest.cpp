
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"

#include <iostream>
#include "Fares/FareValidatorOrchestrator.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FlightFinderTrx.h"
#include "Common/Global.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/AirSeg.h"
#include "Xform/XMLShoppingHandler.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestMemHandle.h"

using namespace std;
using namespace tse;

namespace tse
{

class BffAltDatesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BffAltDatesTest);
  CPPUNIT_TEST(noAltDatesCreated);
  CPPUNIT_TEST(oneWayAltDatesCreated);
  CPPUNIT_TEST(roundTripAltDatesCreated);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _handler = _memHandle.insert(new XMLShoppingHandler(_dataHandle));
    _ffTrx = _handler->_bffParser.ffinderTrx();
    _dataHandle.get(_ffTrx->journeyItin());
    //_resources.push_back(_dataHandle.get(_ffTrx->journeyItin()));
  }

  void tearDown() { _memHandle.clear(); }

  AirSeg* buildSegment(string origin,
                       string destination,
                       string carrier,
                       DateTime departure = DateTime::localTime(),
                       DateTime arrival = DateTime::localTime())
  {
    AirSeg* airSeg;
    _dataHandle.get(airSeg);

    airSeg->departureDT() = departure;
    airSeg->arrivalDT() = arrival;

    Loc* locorig, *locdest;
    _dataHandle.get(locorig);
    _dataHandle.get(locdest);
    locorig->loc() = origin;
    locdest->loc() = destination;

    airSeg->origAirport() = origin;
    airSeg->origin() = locorig;
    airSeg->destAirport() = destination;
    airSeg->destination() = locdest;
    airSeg->carrier() = carrier;

    return airSeg;
  }

  void outputAltDates(const ShoppingTrx& trx)
  {
    typedef ShoppingTrx::AltDateInfo AltDateInfo;

    if (trx.isAltDates())
    {
      std::cout << "\n";

      for (std::map<DatePair, AltDateInfo*>::const_iterator i = trx.altDatePairs().begin();
           i != trx.altDatePairs().end();
           i++)
      {
        DatePair myPair = i->first;

        std::cout << myPair.first.dateToString(DDMMYYYY, "-") << "   "
                  << myPair.second.dateToString(DDMMYYYY, "-") << endl;
      }
    }
    else
      std::cout << "NO ALTERNATE DATES" << endl;

    std::cout << "------------------------------------------" << std::endl;
  }

  void buildRoundTrip(std::vector<TravelSeg*>& travelSeg,
                      const std::string& orig,
                      const std::string& dest,
                      const std::string& carrier,
                      const DateTime& depTimeOut,
                      const DateTime& depTimeIn)
  {
    travelSeg.push_back(buildSegment(orig, dest, carrier, depTimeOut));

    if (!depTimeIn.isEmptyDate())
    {
      travelSeg.push_back(buildSegment(dest, orig, carrier, depTimeIn));
    }
  }

  bool isDateEqual(const DateTime& d1, const DateTime& d2)
  {
    return d1.year() == d2.year() && d1.month() == d2.month() && d1.day() == d2.day();
  }

protected:
  void noAltDatesCreated()
  {
    // set first segment only
    buildRoundTrip(_handler->_bffParser.ffinderTrx()->journeyItin()->travelSeg(),
                   "AAA",
                   "BBB",
                   "AA",
                   DateTime::emptyDate(),
                   DateTime::emptyDate());

    int numDaysFwd = 0;

    _ffTrx->departureDT() = DateTime(2008, Dec, 10);
    _ffTrx->numDaysFwd() = numDaysFwd;
    _ffTrx->bffStep() = FlightFinderTrx::STEP_1;
    _ffTrx->owrt() = "O";

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->altDatePairs().size());

    _handler->_bffParser.createBffAltDatesAndJourneyItin();

    CPPUNIT_ASSERT_EQUAL(numDaysFwd, (int)_ffTrx->altDatePairs().size());

    for (std::map<DatePair, ShoppingTrx::AltDateInfo*>::const_iterator i =
             _ffTrx->altDatePairs().begin();
         i != _ffTrx->altDatePairs().end();
         i++)
    {
      DatePair myPair = i->first;
      DateTime secondDate = myPair.second;
      CPPUNIT_ASSERT_EQUAL(true, isDateEqual(DateTime::emptyDate(), secondDate));
    }
  }

  void oneWayAltDatesCreated()
  {
    // set first segment only
    buildRoundTrip(_handler->_bffParser.ffinderTrx()->journeyItin()->travelSeg(),
                   "AAA",
                   "BBB",
                   "AA",
                   DateTime::emptyDate(),
                   DateTime::emptyDate());

    int numDaysFwd = 3;

    _ffTrx->departureDT() = DateTime(2008, Dec, 10);
    _ffTrx->numDaysFwd() = numDaysFwd;
    _ffTrx->bffStep() = FlightFinderTrx::STEP_1;
    _ffTrx->owrt() = "O";

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->altDatePairs().size());

    _handler->_bffParser.createBffAltDatesAndJourneyItin();

    CPPUNIT_ASSERT_EQUAL(numDaysFwd, (int)_ffTrx->altDatePairs().size());

    for (std::map<DatePair, ShoppingTrx::AltDateInfo*>::const_iterator i =
             _ffTrx->altDatePairs().begin();
         i != _ffTrx->altDatePairs().end();
         i++)
    {
      DatePair myPair = i->first;
      DateTime secondDate = myPair.second;
      CPPUNIT_ASSERT_EQUAL(true, isDateEqual(DateTime::emptyDate(), secondDate));
    }
  }

  void roundTripAltDatesCreated()
  {
    // set first segment only
    buildRoundTrip(_handler->_bffParser.ffinderTrx()->journeyItin()->travelSeg(),
                   "AAA",
                   "BBB",
                   "AA",
                   DateTime::emptyDate(),
                   DateTime::emptyDate());

    int numDaysFwd = 5;

    _ffTrx->departureDT() = DateTime(2008, Dec, 10);
    _ffTrx->numDaysFwd() = numDaysFwd;
    _ffTrx->bffStep() = FlightFinderTrx::STEP_1;
    _ffTrx->owrt() = "R";

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->altDatePairs().size());

    _handler->_bffParser.createBffAltDatesAndJourneyItin();

    CPPUNIT_ASSERT_EQUAL(numDaysFwd, (int)_ffTrx->altDatePairs().size());

    for (std::map<DatePair, ShoppingTrx::AltDateInfo*>::const_iterator i =
             _ffTrx->altDatePairs().begin();
         i != _ffTrx->altDatePairs().end();
         i++)
    {
      DatePair myPair = i->first;
      DateTime secondDate = myPair.second;
      CPPUNIT_ASSERT_EQUAL(true, isDateEqual(DateTime::emptyDate(), secondDate));
    }
  }

private:
  XMLShoppingHandler* _handler;
  FlightFinderTrx* _ffTrx;
  DataHandle _dataHandle;
  std::vector<size_t> _resources;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BffAltDatesTest);

} // tse
