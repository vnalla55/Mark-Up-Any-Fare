
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include <iostream>
#include "Fares/FareValidatorOrchestrator.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/AirSeg.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using namespace std;

namespace FFind
{
class FakedFVO : public FareValidatorOrchestrator
{
public:
  using FareValidatorOrchestrator::createFFinderAltDates;

  FakedFVO(const std::string& name, TseServer& server) : FareValidatorOrchestrator(name, server) {}
  virtual ~FakedFVO() {}

  void setupItinerarySegment(Trx& trx,
                             AirSeg*& segment,
                             DateTime outboundDate,
                             LocCode boardCity,
                             LocCode offCity,
                             CarrierCode cxrCode,
                             int16_t pnrSegment)
  {
  }
};
}

class FFinderAltDates : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FFinderAltDates);
  CPPUNIT_TEST(altdatesForOneSegment);
  CPPUNIT_TEST(altDatesForTwoSegDifferentOutboundInboundDate);
  CPPUNIT_TEST(altDatesOutboundInboundDateTheSame);
  CPPUNIT_TEST(outboundHigherBoundGreaterThanInboundLowerBound);
  CPPUNIT_TEST_SUITE_END();

private:
  FFind::FakedFVO* _fvo;
  FlightFinderTrx* _ffTrx;
  TestMemHandle _memH;

public:
  void setUp()
  {
    const auto server = _memH.create<MockTseServer>();
    _ffTrx = _memH.insert(new FlightFinderTrx);
    _ffTrx->journeyItin() = _memH.insert(new Itin);

    _fvo = _memH.insert(new FFind::FakedFVO("FVO", *server));
  }

  void tearDown() { _memH.clear(); }

  AirSeg* buildSegment(string origin,
                       string destination,
                       string carrier,
                       DateTime departure = DateTime::localTime(),
                       DateTime arrival = DateTime::localTime())
  {
    AirSeg* airSeg = _memH.insert(new AirSeg);

    airSeg->departureDT() = departure;
    airSeg->arrivalDT() = arrival;

    Loc* locorig = _memH.insert(new Loc);
    Loc* locdest = _memH.insert(new Loc);
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
    {
      std::cout << "NO ALTERNATE DATES" << endl;
    }
    std::cout << "------------------------------------------" << std::endl;
  }

  void buildRoundTrip(std::vector<TravelSeg*>& travelseg,
                      const std::string& orig,
                      const std::string& dest,
                      const std::string& carrier,
                      const DateTime& depTimeOut,
                      const DateTime& depTimeIn)
  {
    (*_ffTrx).journeyItin()->travelSeg().push_back(buildSegment(orig, dest, carrier, depTimeOut));

    if (!depTimeIn.isEmptyDate())
    {
      (*_ffTrx).journeyItin()->travelSeg().push_back(buildSegment(dest, orig, carrier, depTimeIn));
    }
  }

protected:
  void altDatesForTwoSegDifferentOutboundInboundDate()
  {
    buildRoundTrip((*_ffTrx).journeyItin()->travelSeg(),
                   "AAA",
                   "BBB",
                   "AA",
                   DateTime(2007, Dec, 10),
                   DateTime(2007, Dec, 25));

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->altDatePairs().size());

    _fvo->createFFinderAltDates(*_ffTrx);

    // Should exclude all date pair (out,in)
    // if in < out,  and original outbound inbound
    // n - number of dates in scope [L,R]=[date-7,date+7]
    // 224= n*n -1  where  n=15 = R-L+1
    int numOfDatePairsCombinationInScope = 224;
    CPPUNIT_ASSERT_EQUAL(numOfDatePairsCombinationInScope, (int)_ffTrx->altDatePairs().size());
  }

  void altdatesForOneSegment()
  {
    buildRoundTrip((*_ffTrx).journeyItin()->travelSeg(),
                   "AAA",
                   "BBB",
                   "AA",
                   DateTime(2007, Dec, 10),
                   DateTime::emptyDate());

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->altDatePairs().size());

    _fvo->createFFinderAltDates(*_ffTrx);

    /* Should create only dates in scope [L,R]= [out-7,out+7] except out */
    int numOfDatesInScope = 14; // = R-L
    CPPUNIT_ASSERT_EQUAL(numOfDatesInScope, (int)_ffTrx->altDatePairs().size());

    /* second date is empty date  */
    for (std::map<DatePair, ShoppingTrx::AltDateInfo*>::const_iterator i =
             _ffTrx->altDatePairs().begin();
         i != _ffTrx->altDatePairs().end();
         i++)
    {
      DatePair myPair = i->first;
      DateTime secondDate = myPair.second;
      CPPUNIT_ASSERT_EQUAL((int)DateTime::emptyDate().year(), (int)secondDate.year());
      CPPUNIT_ASSERT_EQUAL((int)DateTime::emptyDate().month(), (int)secondDate.month());
      CPPUNIT_ASSERT_EQUAL((int)DateTime::emptyDate().day(), (int)secondDate.day());
    }
  }

  void altDatesOutboundInboundDateTheSame()
  {
    buildRoundTrip((*_ffTrx).journeyItin()->travelSeg(),
                   "AAA",
                   "BBB",
                   "AA",
                   DateTime(2007, Dec, 10),
                   DateTime(2007, Dec, 10));

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->altDatePairs().size());

    _fvo->createFFinderAltDates(*_ffTrx);

    // Should exclude all date pair (out,in)
    // if in < out,  and original outbound inbound
    // n - number of dates in scope [L,R]=[date-7,date+7]
    // n*n -1 -1, where n=15 = R-L+1
    // 119 = n*n - (n-1)*n/2-1 , where n=15, because we take +/- 7 day
    int numOfDatePairsCombinationInScope = 119;
    CPPUNIT_ASSERT_EQUAL(numOfDatePairsCombinationInScope, (int)_ffTrx->altDatePairs().size());
  }

  void outboundHigherBoundGreaterThanInboundLowerBound()
  {
    buildRoundTrip((*_ffTrx).journeyItin()->travelSeg(),
                   "AAA",
                   "BBB",
                   "AA",
                   DateTime(2007, Dec, 10),
                   DateTime(2007, Dec, 23));

    CPPUNIT_ASSERT_EQUAL(0, (int)_ffTrx->altDatePairs().size());

    _fvo->createFFinderAltDates(*_ffTrx);

    // outputAltDates(*_ffTrx);

    // Should exclude all date pair (out,in)
    // if in < out,  and original outbound inbound
    // n - number of dates in scope [L,R]=[date-7,date+7]
    // n*n -1 -1, where n=15 = R-L+1
    // In this case ((17,Dec,2007)(16,Dec,2007))

    int numOfDatePairsCombinationInScope = 223;
    CPPUNIT_ASSERT_EQUAL(numOfDatePairsCombinationInScope, (int)_ffTrx->altDatePairs().size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FFinderAltDates);
}
