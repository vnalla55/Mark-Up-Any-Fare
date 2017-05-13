#include <vector>
#include <set>

#include "test/include/CppUnitHelperMacros.h"

#include "Common/ClassOfService.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/ASOCandidateChecker.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace shpq
{

class ASOCandidateCheckerDerived : public ASOCandidateChecker
{
public:
  ASOCandidateCheckerDerived(ShoppingTrx& trx) : ASOCandidateChecker(trx)
  {
    _asoCandidateMileage = 4000;
  }

  std::set<LocCode> collectIntermediatePoints(std::vector<TravelSeg*>& segments) const
  {
    return ASOCandidateChecker::collectIntermediatePoints(segments);
  }
  bool isDiamond(std::vector<int> sops) const { return ASOCandidateChecker::isDiamond(sops); }
  bool checkDistance(const Loc& loc1, const Loc& loc2) const
  {
    return ASOCandidateChecker::checkDistance(loc1, loc2);
  }
  bool checkLegsOverlap(std::vector<int> sops) const
  {
    return ASOCandidateChecker::checkLegsOverlap(sops);
  }
};

const uint16_t LocNumber = 5;
const uint16_t CxrNumber = 2;

class ASOCandidateCheckerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ASOCandidateCheckerTest);
  CPPUNIT_TEST(checkDistanceSmall);
  CPPUNIT_TEST(checkDistanceBig);
  CPPUNIT_TEST(checkIsDiamondFalse);
  CPPUNIT_TEST(checkIsDiamondTrue);
  CPPUNIT_TEST(checkMatchOneLeg);
  CPPUNIT_TEST(checkMatchDiamond);
  CPPUNIT_TEST(checkNoMatchDiamond);
  CPPUNIT_TEST(checkIntermediatePointsCollection);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string _loc[LocNumber];
  CarrierCode _cxr[CxrNumber];

  TestMemHandle _dataHandle;
  ShoppingTrx* _trx;

  ASOCandidateCheckerDerived* _asoCheck;

  ShoppingTrx* createTrx(const std::string& filename)
  {
    ShoppingTrx* trx = TestShoppingTrxFactory::create(filename);
    trx->setAltDates(false);
    return trx;
  }

public:
  void setUp()
  {
    _dataHandle.create<TestConfigInitializer>();
    _trx = createTrx("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");

    _loc[0] = "DFW";
    _loc[1] = "JFK";
    _loc[2] = "LAX";
    _loc[3] = "SFA";
    _loc[4] = "IST";

    _cxr[0] = "DL";
    _cxr[1] = "LH";


    _asoCheck = _dataHandle.create<ASOCandidateCheckerDerived>(*_trx);
  }

  void tearDown()
  {
    _trx->legs().clear();
    _trx->flightMatrix().clear();
    _dataHandle.clear();
  }

  void checkDistanceSmall()
  {
    setTransactionData();

    std::vector<int> sops(2);
    sops[0] = 0;
    sops[1] = 0;

    std::vector<TravelSeg*>& tvlSegsOut = _trx->legs()[0].sop()[sops[0]].itin()->travelSeg();

    setGeoLocation('N', 70, 0, 0, 'W', 70, 0, 0, const_cast<Loc*>(tvlSegsOut.front()->origin()));
    setGeoLocation(
        'N', 70, 0, 0, 'W', 75, 0, 0, const_cast<Loc*>(tvlSegsOut.front()->destination()));

    CPPUNIT_ASSERT(_asoCheck->checkDistance(*tvlSegsOut.front()->origin(),
                                            *tvlSegsOut.front()->destination()) == false);
  }

  void checkDistanceBig()
  {
    setTransactionData();

    std::vector<int> sops(2);
    sops[0] = 0;
    sops[1] = 0;

    std::vector<TravelSeg*>& tvlSegsOut = _trx->legs()[0].sop()[sops[0]].itin()->travelSeg();

    setGeoLocation('N', 70, 0, 0, 'W', 70, 0, 0, const_cast<Loc*>(tvlSegsOut.front()->origin()));
    setGeoLocation(
        'S', 70, 0, 0, 'E', 75, 0, 0, const_cast<Loc*>(tvlSegsOut.front()->destination()));

    CPPUNIT_ASSERT(_asoCheck->checkDistance(*tvlSegsOut.front()->origin(),
                                            *tvlSegsOut.front()->destination()) == true);
  }

  void checkIntermediatePointsCollection()
  {
    setTransactionData();

    std::vector<int> sops(2);
    sops[0] = 1;
    sops[1] = 1;

    std::vector<TravelSeg*>& tvlSegsOut = _trx->legs()[0].sop()[sops[0]].itin()->travelSeg();

    std::set<LocCode> intermediatePoints = _asoCheck->collectIntermediatePoints(tvlSegsOut);
    CPPUNIT_ASSERT(intermediatePoints.size() == 2);
    CPPUNIT_ASSERT(intermediatePoints.find(tvlSegsOut.front()->destAirport()) !=
                   intermediatePoints.end());
    CPPUNIT_ASSERT(intermediatePoints.find(tvlSegsOut.back()->origAirport()) !=
                   intermediatePoints.end());

    std::vector<TravelSeg*>& tvlSegsInb = _trx->legs()[1].sop()[sops[1]].itin()->travelSeg();

    std::set<LocCode> intermediatePoints1 = _asoCheck->collectIntermediatePoints(tvlSegsInb);
    CPPUNIT_ASSERT(intermediatePoints1.size() == 2);
    CPPUNIT_ASSERT(intermediatePoints1.find(tvlSegsInb.front()->destAirport()) !=
                   intermediatePoints1.end());
    CPPUNIT_ASSERT(intermediatePoints1.find(tvlSegsInb.back()->origAirport()) !=
                   intermediatePoints1.end());
  }

  void checkIsDiamondFalse()
  {
    setTransactionData();

    std::vector<int> sops(2);
    sops[0] = 1;
    sops[1] = 1;

    CPPUNIT_ASSERT(!_asoCheck->isDiamond(sops));
  }

  void checkIsDiamondTrue()
  {
    setTransactionData();

    std::vector<int> sops(2);
    sops[0] = 2;
    sops[1] = 2;

    CPPUNIT_ASSERT(_asoCheck->isDiamond(sops));
  }

  void checkMatchOneLeg()
  {
    setTransactionData();

    _trx->legs().pop_back();
    std::vector<int> sops(1);
    sops[0] = 2;

    CPPUNIT_ASSERT(!_asoCheck->match(sops));
  }

  void checkMatchDiamond()
  {
    setTransactionData();

    std::vector<int> sops(2);
    sops[0] = 2;
    sops[1] = 2;

    std::vector<TravelSeg*>& tvlSegsOut = _trx->legs()[0].sop()[sops[0]].itin()->travelSeg();

    setGeoLocation('N', 70, 0, 0, 'W', 70, 0, 0, const_cast<Loc*>(tvlSegsOut.front()->origin()));
    setGeoLocation(
        'S', 70, 0, 0, 'E', 75, 0, 0, const_cast<Loc*>(tvlSegsOut.front()->destination()));

    CPPUNIT_ASSERT(_asoCheck->match(sops));
  }

  void checkNoMatchDiamond()
  {
    setTransactionData();

    std::vector<int> sops(2);
    sops[0] = 1;
    sops[1] = 1;

    std::vector<TravelSeg*>& tvlSegsOut = _trx->legs()[0].sop()[sops[0]].itin()->travelSeg();

    setGeoLocation('N', 70, 0, 0, 'W', 70, 0, 0, const_cast<Loc*>(tvlSegsOut.front()->origin()));
    setGeoLocation(
        'S', 70, 0, 0, 'E', 75, 0, 0, const_cast<Loc*>(tvlSegsOut.front()->destination()));

    CPPUNIT_ASSERT(!_asoCheck->match(sops));
  }

private:
  ClassOfService* buildCOS(const BookingCode& bkc, uint16_t numOfSeats = 9)
  {
    ClassOfService* cos(0);
    _dataHandle.get(cos);
    cos->numSeats() = numOfSeats;
    cos->cabin().setEconomyClass();
    cos->bookingCode() = bkc;

    return cos;
  };

  void setGeoLocation(char lathem,
                      LatLong latdeg,
                      LatLong latmin,
                      LatLong latsec,
                      char lnghem,
                      LatLong lngdeg,
                      LatLong lngmin,
                      LatLong lngsec,
                      Loc* location)
  {
    location->lathem() = lathem;
    location->latdeg() = latdeg;
    location->latmin() = latmin;
    location->latsec() = latsec;
    location->lnghem() = lnghem;
    location->lngdeg() = lngdeg;
    location->lngmin() = lngmin;
    location->lngsec() = lngsec;
  }

  AirSeg* buildSegment(std::string origin,
                       std::string destination,
                       std::string carrier,
                       DateTime departure = DateTime::localTime(),
                       DateTime arrival = DateTime::localTime(),
                       bool stopover = false)
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
    airSeg->setOperatingCarrierCode(carrier);
    airSeg->boardMultiCity() = locorig->loc();
    airSeg->offMultiCity() = locdest->loc();
    airSeg->stopOver() = stopover;

    ClassOfService* oneEconomyClass = buildCOS("Y");
    airSeg->classOfService().push_back(oneEconomyClass);

    return airSeg;
  };

  void addOneSegmentToItin(Itin* itin,
                           std::string origin,
                           std::string dest,
                           std::string carrier,
                           DateTime departure = DateTime::localTime(),
                           DateTime arrival = DateTime::localTime())
  {
    AirSeg* travelSeg = buildSegment(origin, dest, carrier, departure, arrival);
    itin->travelSeg().push_back(travelSeg);
  }

  void setTransactionData()
  {
    Itin* itineraryOb1(0);
    Itin* itineraryOb2(0);
    Itin* itineraryOb3(0);
    Itin* itineraryOb4(0);
    Itin* itineraryOb5(0);
    Itin* itineraryOb6(0);
    Itin* itineraryOb7(0);
    Itin* itineraryOb8(0);
    _dataHandle.get(itineraryOb1);
    _dataHandle.get(itineraryOb2);
    _dataHandle.get(itineraryOb3);
    _dataHandle.get(itineraryOb4);
    _dataHandle.get(itineraryOb5);
    _dataHandle.get(itineraryOb6);
    _dataHandle.get(itineraryOb7);
    _dataHandle.get(itineraryOb8);

    addOneSegmentToItin(itineraryOb1, _loc[0], _loc[1], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb1, _loc[1], _loc[2], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb1, _loc[2], _loc[3], _cxr[0], DateTime(2008, 5, 15));

    addOneSegmentToItin(itineraryOb2, _loc[3], _loc[2], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb2, _loc[2], _loc[1], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb2, _loc[1], _loc[0], _cxr[0], DateTime(2008, 5, 15));

    addOneSegmentToItin(itineraryOb3, _loc[0], _loc[1], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb3, _loc[1], _loc[2], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb3, _loc[2], _loc[4], _cxr[0], DateTime(2008, 5, 15));

    addOneSegmentToItin(itineraryOb4, _loc[4], _loc[2], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb4, _loc[2], _loc[1], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb4, _loc[1], _loc[0], _cxr[0], DateTime(2008, 5, 15));

    addOneSegmentToItin(itineraryOb5, _loc[0], _loc[1], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb5, _loc[1], _loc[3], _cxr[0], DateTime(2008, 5, 15));

    addOneSegmentToItin(itineraryOb6, _loc[3], _loc[2], _cxr[0], DateTime(2008, 5, 15));
    addOneSegmentToItin(itineraryOb6, _loc[2], _loc[0], _cxr[0], DateTime(2008, 5, 15));

    _trx->legs().clear();

    _trx->legs().push_back(ShoppingTrx::Leg());
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb1, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb3, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb5, 1, true));

    _trx->legs().push_back(ShoppingTrx::Leg());
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb2, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb4, 1, true));
    _trx->legs().back().sop().push_back(ShoppingTrx::SchedulingOption(itineraryOb6, 1, true));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ASOCandidateCheckerTest);
}
} // namespace tse::shqp
