#include "test/include/CppUnitHelperMacros.h"
#include "Common/FxCnException.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestLocFactory.h"

using namespace tse;
using namespace std;

namespace tse
{

class FxCnExceptionTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(FxCnExceptionTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testCheckThruMarketEmptyTravelSeg);
  CPPUNIT_TEST(testCheckThruMarketFirstSegmentOriginateChina);
  CPPUNIT_TEST(testCheckThruMarketFirstSegmentLocMFM);
  CPPUNIT_TEST(testCheckThruMarketFirstSegmentLocHKG);
  CPPUNIT_TEST(testCheckThruMarketFirstSegmentIsLastSegment);
  CPPUNIT_TEST(testCheckThruMarketEndMfmOnAA);
  CPPUNIT_TEST(testCheckThruMarket);
  CPPUNIT_TEST(testOperatorFalseForValidatingCarrierEmpty);
  CPPUNIT_TEST(testOperatorFalseForValidatingCarrierNotCX_GE_KA_NX);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor() { CPPUNIT_ASSERT_NO_THROW(FxCnException(*_trx, _itin)); }

  void testCheckThruMarketEmptyTravelSeg()
  {
    FxCnException exception(*_trx, _itin);
    std::vector<TravelSeg*> tvlSeg;
    CPPUNIT_ASSERT(!(exception.checkThruMarket(tvlSeg)));
  }

  void testCheckThruMarketFirstSegmentOriginateChina()
  {
    FxCnException exception(*_trx, _itin);
    std::vector<TravelSeg*> tvlSeg;
    AirSeg* seg = createAirSeg();
    seg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPVG.xml");
    tvlSeg.push_back(seg);
    CPPUNIT_ASSERT(!(exception.checkThruMarket(tvlSeg)));
    delete seg;
  }

  void testCheckThruMarketFirstSegmentLocMFM()
  {
    FxCnException exception(*_trx, _itin);
    std::vector<TravelSeg*> tvlSeg;
    AirSeg* seg = createAirSeg();
    seg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMFM.xml");
    tvlSeg.push_back(seg);
    CPPUNIT_ASSERT(!(exception.checkThruMarket(tvlSeg)));
    delete seg;
  }

  void testCheckThruMarketFirstSegmentLocHKG()
  {
    FxCnException exception(*_trx, _itin);
    std::vector<TravelSeg*> tvlSeg;
    AirSeg* seg = createAirSeg();
    seg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    tvlSeg.push_back(seg);
    CPPUNIT_ASSERT(!(exception.checkThruMarket(tvlSeg)));
    delete seg;
  }

  void testCheckThruMarketFirstSegmentIsLastSegment()
  {
    FxCnException exception(*_trx, _itin);
    std::vector<TravelSeg*> tvlSeg;
    AirSeg* seg = createAirSeg();
    tvlSeg.push_back(seg);
    CPPUNIT_ASSERT(!(exception.checkThruMarket(tvlSeg)));
    delete seg;
  }

  void testCheckThruMarketEndMfmOnAA()
  {
    FxCnException exception(*_trx, _itin);
    std::vector<TravelSeg*> tvlSeg;
    AirSeg* seg1 = createAirSeg();
    AirSeg* seg2 = createAirSegToMFM();
    tvlSeg.push_back(seg1);
    tvlSeg.push_back(seg2);
    CPPUNIT_ASSERT(!(exception.checkThruMarket(tvlSeg)));
    delete seg1;
    delete seg2;
  }

  void testCheckThruMarket()
  {
    FxCnException exception(*_trx, _itin);
    std::vector<TravelSeg*> tvlSeg;
    AirSeg* seg1 = createAirSeg();
    AirSeg* seg2 = createAirSegToHKG();
    AirSeg* seg3 = createAirSegToPVG();
    _itin.validatingCarrier() = "CX";
    tvlSeg.push_back(seg1);
    tvlSeg.push_back(seg2);
    tvlSeg.push_back(seg3);
    CPPUNIT_ASSERT(exception.checkThruMarket(tvlSeg));
    delete seg1;
    delete seg2;
    delete seg3;
  }

  void testOperatorFalseForValidatingCarrierEmpty()
  {
    FxCnException exception(*_trx, _itin);
    CPPUNIT_ASSERT(!(exception()));
  }

  void testOperatorFalseForValidatingCarrierNotCX_GE_KA_NX()
  {
    FxCnException exception(*_trx, _itin);
    _itin.validatingCarrier() = "AA";
    CPPUNIT_ASSERT(!(exception()));
  }

  AirSeg* createAirSeg()
  {
    AirSeg* seg = new AirSeg;
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    seg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    seg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");
    seg->stopOver() = false;
    seg->carrier() = "AA";
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();
    return seg;
  }

  AirSeg* createAirSegToHKG()
  {
    AirSeg* seg = new AirSeg;
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 2;
    seg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");
    seg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    seg->stopOver() = false;
    seg->carrier() = "AA";
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();
    return seg;
  }

  AirSeg* createAirSegToMFM()
  {
    AirSeg* seg = new AirSeg;
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 2;
    seg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");
    seg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMFM.xml");
    seg->stopOver() = false;
    seg->carrier() = "AA";
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();
    return seg;
  }

  AirSeg* createAirSegToPVG()
  {
    AirSeg* seg = new AirSeg;
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 3;
    seg->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    seg->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPVG.xml");
    seg->stopOver() = false;
    seg->carrier() = "AA";
    seg->departureDT() = DateTime::localTime();
    seg->arrivalDT() = DateTime::localTime();
    return seg;
  }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin _itin;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FxCnExceptionTest);
}
