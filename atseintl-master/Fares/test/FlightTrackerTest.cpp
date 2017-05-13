#include <vector>

#include "Common/DateTime.h"
#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/FltTrkCntryGrp.h"
#include "DBAccess/FltTrkCntryGrp.h"
#include "DBAccess/Loc.h"
#include "Fares/FlightTracker.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"

namespace tse
{

class MockFlightTracker : public FlightTracker
{
public:
  MockFlightTracker(const PricingTrx& trx) : FlightTracker(trx) {}
  virtual ~MockFlightTracker() {}
  const FltTrkCntryGrp* getData(const CarrierCode& carrier, const std::vector<TravelSeg*>& tvlSegs)
  {
    FltTrkCntryGrp* ftcg = new FltTrkCntryGrp;
    ftcg->carrier() = "AA";
    ftcg->flttrkApplInd() = 'B';
    ftcg->nations().push_back("US");
    ftcg->nations().push_back("CA");
    ftcg->nations().push_back("AU");
    return ftcg;
  }
  DateTime getDate(const std::vector<TravelSeg*>& tvlSegs) { return DateTime::localTime(); }
};
namespace
{
class MyDataHandle : public DataHandleMock
{
  FltTrkCntryGrp _retAA;

public:
  MyDataHandle()
  {
    _retAA.carrier() = "AA";
    _retAA.flttrkApplInd() = 'B';
    _retAA.nations().push_back("US");
    _retAA.nations().push_back("CA");
  }

  const FltTrkCntryGrp* getFltTrkCntryGrp(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "AA")
      return &_retAA;
    else if (carrier == "ZZ")
      return 0;
    return DataHandleMock::getFltTrkCntryGrp(carrier, date);
  }
};
}

class FlightTrackerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FlightTrackerTest);
  CPPUNIT_TEST(testProcess1);
  CPPUNIT_TEST(testProcess2);
  CPPUNIT_TEST(testValidateFltTracking1);
  CPPUNIT_TEST(testValidateFltTracking2);
  CPPUNIT_TEST(testValidateFltTracking3);
  CPPUNIT_TEST(testValidateFltTrackingWithNoData);
  CPPUNIT_TEST(testgetFlightTrackingInfo);
  CPPUNIT_SKIP_TEST(testErrorCondition1);
  CPPUNIT_SKIP_TEST(testErrorCondition2);
  CPPUNIT_TEST(testProcess3);
  CPPUNIT_TEST(testProcessToFail);
  CPPUNIT_TEST(testgetData);
  CPPUNIT_TEST_SUITE_END();

public:
  void testProcess1()
  {
    FareMarket fareMarket1, fareMarket2;
    Loc origin;
    Loc destination;
    origin.nation() = "US";
    destination.nation() = "US";
    AirSeg airSeg1;
    airSeg1.origin() = &origin;
    airSeg1.destination() = &origin;
    fareMarket1.travelSeg().push_back(&airSeg1);
    fareMarket1.origin() = &origin;
    fareMarket1.destination() = &destination;
    fareMarket1.governingCarrier() = "AA";

    fareMarket1.geoTravelType() = GeoTravelType::Domestic;
    MockFlightTracker ft(_trx);

    bool rc = ft.process(fareMarket1);
    CPPUNIT_ASSERT(rc);
  }

  void testProcess2()
  {
    FareMarket fareMarket;

    fareMarket.geoTravelType() = GeoTravelType::International;
    MockFlightTracker ft(_trx);

    bool rc = ft.process(fareMarket);
    CPPUNIT_ASSERT(rc);
  }

  void testValidateFltTracking1()
  {
    NationCode origin = "US";
    NationCode destination = "US";

    AirSeg airSeg1;
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&airSeg1);
    CarrierCode carrier = "AA";

    MockFlightTracker ft(_trx);

    bool rc = ft.validateFltTracking(origin, destination, carrier, tvlSegs);
    CPPUNIT_ASSERT(rc);
  }

  void testValidateFltTracking3()
  {
    NationCode origin = "US";
    NationCode destination = "CA";

    AirSeg airSeg1;
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&airSeg1);
    CarrierCode carrier = "AA";

    MockFlightTracker ft(_trx);

    bool rc = ft.validateFltTracking(origin, destination, carrier, tvlSegs);
    CPPUNIT_ASSERT(rc);
  }

  void testValidateFltTracking2()
  {
    NationCode origin = "US";
    NationCode destination = "XX";

    AirSeg airSeg1;
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&airSeg1);
    CarrierCode carrier = "AA";

    MockFlightTracker ft(_trx);

    bool rc = ft.validateFltTracking(origin, destination, carrier, tvlSegs);
    CPPUNIT_ASSERT(!rc);
  }

  void testgetFlightTrackingInfo()
  {
    AirSeg airSeg1;
    Loc loc1, loc2;
    loc1.nation() = "US";
    loc2.nation() = "CA";
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&airSeg1);
    CarrierCode govCxr = "AA";

    MockFlightTracker ft(_trx);

    bool rc = ft.getFltTrackingInfo(tvlSegs, govCxr);
    CPPUNIT_ASSERT(rc);
  }

  void testErrorCondition1()
  {

    NationCode origin = "US";
    NationCode destination = "CA";
    AirSeg airSeg1;
    airSeg1.departureDT() = DateTime::localTime();
    Loc loc1, loc2;
    loc1.nation() = "US";
    loc2.nation() = "CA";
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&airSeg1);
    CarrierCode carrier = "AA";
    FlightTracker ft(_trx);
    bool rc = ft.validateFltTracking(origin, destination, carrier, tvlSegs);
    CPPUNIT_ASSERT(!rc);
  }

  void testErrorCondition2()
  {
    AirSeg airSeg1;
    airSeg1.departureDT() = DateTime::localTime();
    Loc loc1, loc2;
    loc1.nation() = "US";
    loc2.nation() = "CA";
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&airSeg1);
    FlightTracker ft(_trx);
  }

  void testProcess3()
  {
    FareMarket fareMarket;
    AirSeg airSeg1;
    Loc loc1, loc2;
    loc1.nation() = "US";
    loc2.nation() = "CA";
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    fareMarket.origin() = &loc1;

    fareMarket.destination() = &loc2;
    fareMarket.travelSeg().push_back(&airSeg1);
    fareMarket.governingCarrier() = "AA";
    fareMarket.geoTravelType() = GeoTravelType::Transborder;
    MockFlightTracker ft(_trx);

    bool rc = ft.process(fareMarket);
    CPPUNIT_ASSERT(rc);
  }

  void testgetData()
  {
    MyDataHandle mdh;
    CarrierCode carrier = "AA";
    AirSeg airSeg1;
    airSeg1.departureDT() = DateTime::localTime();
    FareMarket fm;
    fm.travelSeg().push_back(&airSeg1);
    FlightTracker ft(_trx);

    const FltTrkCntryGrp* rc = ft.getData(carrier, fm.travelSeg());
    CPPUNIT_ASSERT(rc);
    carrier = "ZZ";

    const FltTrkCntryGrp* rc1 = ft.getData(carrier, fm.travelSeg());
    CPPUNIT_ASSERT(!rc1);
  }

  void testValidateFltTrackingWithNoData()
  {
    MyDataHandle mdh;
    CarrierCode carrier = "AA";
    const NationCode& origin = "UK";
    const NationCode& destination = "FR";
    AirSeg airSeg1;
    airSeg1.departureDT() = DateTime::localTime();
    FareMarket fm;
    fm.travelSeg().push_back(&airSeg1);
    FlightTracker ft(_trx);

    bool rc1 = ft.validateFltTracking(origin, destination, carrier, fm.travelSeg());
    CPPUNIT_ASSERT(!rc1);
  }

  void testProcessToFail()
  {
    MyDataHandle mdh;
    Loc loc1, loc2;
    loc1.nation() = "FR";
    loc2.nation() = "FR";

    AirSeg airSeg1;
    airSeg1.departureDT() = DateTime::localTime();
    airSeg1.origin() = &loc1;
    airSeg1.destination() = &loc2;
    airSeg1.carrier() = "AA";

    FareMarket fm;
    fm.governingCarrier() = "AA";
    fm.travelSeg().push_back(&airSeg1);
    fm.setFltTrkIndicator(true);
    fm.origin() = &loc1;
    fm.destination() = &loc1;
    fm.geoTravelType() = GeoTravelType::ForeignDomestic;

    FlightTracker ft(_trx);

    ft.process(fm);
    CPPUNIT_ASSERT(fm.fltTrkIndicator() == false);
  }
  PricingTrx _trx;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FlightTrackerTest);
}
