#include "Common/ErrorResponseException.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "ItinAnalyzer/SkipFareMarketByMileageSOL.h"
#include "ItinAnalyzer/test/TravelSegmentTestUtil.h"
#include "test/include/MockGlobal.h"

#include <map>

#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include "Common/Config/ConfigMan.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using namespace boost::assign;

typedef std::pair<LocCode, LocCode> LocPair;

class SkipFareMarketByMileageSolMock : public SkipFareMarketByMileageSOL
{
public:
  typedef std::map<LocPair, Mileage> FixtureMap;

  SkipFareMarketByMileageSolMock(FixtureMap& fixture) : _fixture(&fixture) {}
  SkipFareMarketByMileageSolMock() // using this ctor we don't expect getMileage() will be called
      : _fixture(0)
  {
  }

  void initializeThruMilage(Mileage mileage) { _thruMileage = mileage; }

protected:
  /**
   * @override
   */
  virtual Mileage getMileage(const Loc& loc1, const Loc& loc2) const
  {
    if (!_fixture)
    {
      CPPUNIT_FAIL("getMileage() was not expected to be called");
    }

    FixtureMap::const_iterator mileageForLocPair = _fixture->find(LocPair(loc1.loc(), loc2.loc()));
    if (mileageForLocPair == _fixture->end())
    {
      boost::format msg("getMileage() was called with unexpected arguments: "
                        "loc1 = '%1%', loc2 = '%2%'");
      CPPUNIT_FAIL(boost::str(msg % loc1.loc() % loc2.loc()));
    }
    return mileageForLocPair->second;
  }

private:
  const FixtureMap* _fixture;
};

class SkipFareMarketByMileageSolTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SkipFareMarketByMileageSolTest);
  CPPUNIT_TEST(test_skipByConnectingCityMileage_emptyFareMarket);
  CPPUNIT_TEST(test_skipByConnectingCityMileage_cfgFareMarketMileageLimitPercentage_not_set);
  CPPUNIT_TEST(test_skipByConnectingCityMileage_mileage_no_skip);
  CPPUNIT_TEST(test_skipByConnectingCityMileage_mileage_skip);
  CPPUNIT_TEST(test_skipByConnectingCityMileage_foreignDomesticFM);
  CPPUNIT_TEST_SUITE_END();

public:
  /**
   * @override
   */
  void setUp()
  {
    _fareMarket.geoTravelType() = GeoTravelType::UnknownGeoTravelType;
    _dataHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _dataHandle.clear(); }
  void test_skipByConnectingCityMileage_emptyFareMarket()
  {
    std::vector<TravelSeg*> emptyVec;

    SkipFareMarketByMileageSOL inst;
    CPPUNIT_ASSERT(
        !inst.skipByConnectingCityMileage(_fareMarket, emptyVec, emptyVec.begin(), false).first &&
        "expect fare market is not skipped");
  }

  void test_skipByConnectingCityMileage_cfgFareMarketMileageLimitPercentage_not_set()
  {
    const std::vector<TravelSeg*> dummyVec(3, (TravelSeg*)0);

    SkipFareMarketByMileageSOL inst;
    bool skip =
        inst.skipByConnectingCityMileage(_fareMarket, dummyVec, dummyVec.begin() + 1, false).first;
    CPPUNIT_ASSERT(!skip && "expect fare market is not skipped");
  }

  void test_skipByConnectingCityMileage_mileage_no_skip()
  {
    std::vector<TravelSeg*>* vec = setupTravelSegmentVec();
    std::vector<TravelSeg*>::iterator connXpoint = ++++vec->begin();

    SkipFareMarketByMileageSolMock::FixtureMap fixture;
    fixture[LocPair("DFW", "KRK")] = 1000;
    fixture[LocPair("DFW", "MUC")] = 700;
    fixture[LocPair("MUC", "KRK")] = 400;

    setupCfgFareMarketMileageLimitPercentage(150);
    SkipFareMarketByMileageSolMock inst(fixture);
    inst.initializeThruMilage(1500);

    // 110% < 150%
    CPPUNIT_ASSERT(!inst.skipByConnectingCityMileage(_fareMarket, *vec, connXpoint, false).first &&
                   "expect fare market is not skipped");
  }

  void test_skipByConnectingCityMileage_mileage_skip()
  {
    std::vector<TravelSeg*>* vec = setupTravelSegmentVec();
    std::vector<TravelSeg*>::iterator connXpoint = ++++vec->begin();

    SkipFareMarketByMileageSolMock::FixtureMap fixture;
    fixture[LocPair("DFW", "KRK")] = 1000;
    fixture[LocPair("DFW", "MUC")] = 700;
    fixture[LocPair("MUC", "KRK")] = 400;

    setupCfgFareMarketMileageLimitPercentage(105);
    SkipFareMarketByMileageSolMock inst(fixture);
    inst.initializeThruMilage(500);

    // !110% < 105%
    CPPUNIT_ASSERT(inst.skipByConnectingCityMileage(_fareMarket, *vec, connXpoint, false).first);
  }

  void test_skipByConnectingCityMileage_foreignDomesticFM()
  {
    _fareMarket.geoTravelType() = GeoTravelType::ForeignDomestic;

    std::vector<TravelSeg*>* vec = setupTravelSegmentVec();
    std::vector<TravelSeg*>::iterator connXpoint = ++++vec->begin();

    // setup as arbitrary non-zero to involve further check path
    setupCfgFareMarketMileageLimitPercentage(105);

    SkipFareMarketByMileageSolMock inst;
    CPPUNIT_ASSERT(!inst.skipByConnectingCityMileage(_fareMarket, *vec, connXpoint, false).first &&
                   "do not skip foreign-domestic FM");
  }

private:
  DataHandle _dataHandle;
  FareMarket _fareMarket;

  void setupCfgFareMarketMileageLimitPercentage(int value)
  {
    TestConfigInitializer::setValue(
        "FAREMARKET_MILEAGE_LIMIT_PERCENTAGE", value, "SHOPPING_DIVERSITY");
  }

  std::vector<TravelSeg*>* setupTravelSegmentVec()
  {
    std::vector<TravelSeg*>* vec;
    _dataHandle.get(vec);

    *vec += createSegment("DFW", "LON"), createSegment("LON", "MUC"), createSegment("MUC", "WAR"),
        createSegment("WAR", "KRK");

    CPPUNIT_ASSERT_MESSAGE("setup fixture failed", vec->size() == 4);

    return vec;
  }

  TravelSeg* createSegment(const std::string& origin, const std::string& destination)
  {
    const GeoTravelType geo = GeoTravelType::UnknownGeoTravelType;
    const NationCode nation = EUROPE; // arbitrary one satisfies the mock

    TravelSeg* seg = createAirSegment(origin, nation, destination, nation, geo);
    _dataHandle.deleteList().adopt(seg);

    return seg;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SkipFareMarketByMileageSolTest);

} // tse
