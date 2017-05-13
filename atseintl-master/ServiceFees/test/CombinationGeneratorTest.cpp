#include "test/include/CppUnitHelperMacros.h"
#include "ServiceFees/CombinationGenerator.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/FareMarket.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "Common/TseUtil.h"

namespace tse
{

class CombinationGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CombinationGeneratorTest);

  CPPUNIT_TEST(testGenerateWhen2Segments);
  CPPUNIT_TEST(testGenerateWhen3Segments);
  CPPUNIT_TEST(testGenerateWhen4SegmentsAndSecondIsSurfaceSegment);
  CPPUNIT_TEST(testGenerateFor6SegmentsWhenSecondThirdAndFifthAreSurfaceSegments);
  CPPUNIT_TEST(testGenerateFor8SegmentsWithMaskWhenThirdAndSeventhAreSurfaceSegments);

  CPPUNIT_TEST(testCollectFareMarketsWhenFoundAll4FMs);
  CPPUNIT_TEST(testCollectFareMarketsWhen4thNotFound);
  CPPUNIT_TEST(testCollectFareMarketsWhen4thEmpty);
  CPPUNIT_TEST(testCollectFareMarketsWhen3thNotFound);
  CPPUNIT_TEST(testCollectFareMarketsWhen3thEmpty);
  CPPUNIT_TEST(testCollectFareMarketsWhenSecondNotFound);
  CPPUNIT_TEST(testCollectFareMarketsWhenSecondEmpty);
  CPPUNIT_TEST(testCollectFareMarketsWhenFirstNotFound);
  CPPUNIT_TEST(testGetFareMarketsWhenCombIs0);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10000000XX);
  CPPUNIT_TEST(testGetFareMarketsWhenComb100000XXXX);
  CPPUNIT_TEST(testGetFareMarketsWhenComb1000XXXXXX);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10XXXXXXXX);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10);
  CPPUNIT_TEST(testGetFareMarketsWhenComb1000);
  CPPUNIT_TEST(testGetFareMarketsWhenComb1010);
  CPPUNIT_TEST(testGetFareMarketsWhenComb1011);
  CPPUNIT_TEST(testGetFareMarketsWhenComb100000);
  CPPUNIT_TEST(testGetFareMarketsWhenComb100010);
  CPPUNIT_TEST(testGetFareMarketsWhenComb100011);
  CPPUNIT_TEST(testGetFareMarketsWhenComb101000);
  CPPUNIT_TEST(testGetFareMarketsWhenComb101010);
  CPPUNIT_TEST(testGetFareMarketsWhenComb101011);
  CPPUNIT_TEST(testGetFareMarketsWhenComb101110);
  CPPUNIT_TEST(testGetFareMarketsWhenComb101111);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10000000);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10000010);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10000011);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10001000);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10001010);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10001011);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10001110);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10001111);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10100000);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10100010);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10100011);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10101000);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10101010);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10101011);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10101110);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10101111);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10111000);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10111010);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10111011);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10111110);
  CPPUNIT_TEST(testGetFareMarketsWhenComb10111111);
  CPPUNIT_TEST(testGetFareMarketsWhenFMNotFound);

  CPPUNIT_TEST_SUITE_END();

protected:
  typedef std::pair<std::vector<TravelSeg*>::const_iterator,
                    std::vector<TravelSeg*>::const_iterator> TvlSegPair;
  TestMemHandle _memHandle;
  std::vector<TvlSegPair>* _routes;
  TseUtil::SolutionSet* _solutions;
  CombinationGenerator* _generator;
  TvlSegPair* _route1;
  TvlSegPair* _route12;
  TvlSegPair* _route13;
  TvlSegPair* _route14;
  TvlSegPair* _route2;
  TvlSegPair* _route23;
  TvlSegPair* _route24;
  TvlSegPair* _route3;
  TvlSegPair* _route34;
  TvlSegPair* _route4;
  AirSeg* _seg1;
  AirSeg* _seg2;
  ArunkSeg* _surfSeg2;
  AirSeg* _seg3;
  ArunkSeg* _surfSeg3;
  AirSeg* _seg4;
  AirSeg* _seg5;
  ArunkSeg* _surfSeg5;
  AirSeg* _seg6;
  ArunkSeg* _surfSeg7;
  AirSeg* _seg8;
  uint8_t* _combNo;
  int* _startShift;
  int* _endShift;
  std::vector<TvlSegPair*>* _route;
  std::vector<TravelSeg*>* _noArunkTravel;
  std::vector<TravelSeg*>* _oneArunkTravel;
  std::vector<TravelSeg*>* _twoArunksTravel;
  std::vector<TravelSeg*>* _threeArunksTravel;

public:
  void setUp()
  {
    _routes = _memHandle.create<std::vector<TvlSegPair> >();
    _solutions = _memHandle.create<TseUtil::SolutionSet>();
    _generator = _memHandle.insert(new CombinationGenerator(4, *_routes, _route1->first));
    _seg1 = _memHandle.create<AirSeg>();
    _seg1->segmentOrder() = 1;
    _seg2 = _memHandle.create<AirSeg>();
    _seg2->segmentOrder() = 2;
    _surfSeg2 = _memHandle.create<ArunkSeg>();
    _surfSeg2->segmentOrder() = 2;
    _seg3 = _memHandle.create<AirSeg>();
    _seg3->segmentOrder() = 3;
    _surfSeg3 = _memHandle.create<ArunkSeg>();
    _surfSeg3->segmentOrder() = 3;
    _seg4 = _memHandle.create<AirSeg>();
    _seg4->segmentOrder() = 4;
    _seg5 = _memHandle.create<AirSeg>();
    _seg5->segmentOrder() = 5;
    _surfSeg5 = _memHandle.create<ArunkSeg>();
    _surfSeg5->segmentOrder() = 5;
    _seg6 = _memHandle.create<AirSeg>();
    _seg6->segmentOrder() = 6;
    _surfSeg7 = _memHandle.create<ArunkSeg>();
    _surfSeg7->segmentOrder() = 7;
    _seg8 = _memHandle.create<AirSeg>();
    _seg8->segmentOrder() = 8;
    _combNo = _memHandle.create<uint8_t>();
    _startShift = _memHandle.create<int>();
    _endShift = _memHandle.create<int>();
    _route = _memHandle.create<std::vector<TvlSegPair*> >();
    _noArunkTravel = _memHandle.create<std::vector<TravelSeg*> >();
    _oneArunkTravel = _memHandle.create<std::vector<TravelSeg*> >();
    _twoArunksTravel = _memHandle.create<std::vector<TravelSeg*> >();
    _threeArunksTravel = _memHandle.create<std::vector<TravelSeg*> >();

    _noArunkTravel->push_back(_seg1);
    _noArunkTravel->push_back(_seg2);
    _noArunkTravel->push_back(_seg3);
    _noArunkTravel->push_back(_seg4);
    _noArunkTravel->push_back(_seg6);

    _oneArunkTravel->push_back(_seg1);
    _oneArunkTravel->push_back(_surfSeg2);
    _oneArunkTravel->push_back(_seg3);
    _oneArunkTravel->push_back(_seg4);
    _oneArunkTravel->push_back(_seg6);

    _twoArunksTravel->push_back(_seg1);
    _twoArunksTravel->push_back(_seg2);
    _twoArunksTravel->push_back(_surfSeg3);
    _twoArunksTravel->push_back(_seg4);
    _twoArunksTravel->push_back(_seg5);
    _twoArunksTravel->push_back(_seg6);
    _twoArunksTravel->push_back(_surfSeg7);
    _twoArunksTravel->push_back(_seg8);
    _twoArunksTravel->push_back(_seg1);

    _threeArunksTravel->push_back(_seg1);
    _threeArunksTravel->push_back(_surfSeg2);
    _threeArunksTravel->push_back(_surfSeg3);
    _threeArunksTravel->push_back(_seg4);
    _threeArunksTravel->push_back(_surfSeg5);
    _threeArunksTravel->push_back(_seg6);
    _threeArunksTravel->push_back(_seg1);

    _route1 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin(), _noArunkTravel->begin() + 1)));
    _route12 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin(), _noArunkTravel->begin() + 2)));
    _route13 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin(), _noArunkTravel->begin() + 3)));
    _route14 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin(), _noArunkTravel->begin() + 4)));
    _route2 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin() + 1, _noArunkTravel->begin() + 2)));
    _route23 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin() + 1, _noArunkTravel->begin() + 3)));
    _route24 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin() + 1, _noArunkTravel->begin() + 4)));
    _route3 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin() + 2, _noArunkTravel->begin() + 3)));
    _route34 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin() + 2, _noArunkTravel->begin() + 4)));
    _route4 = _memHandle.insert(
        new TvlSegPair(std::make_pair(_noArunkTravel->begin() + 3, _noArunkTravel->begin() + 4)));
  }

  void tearDown() { _memHandle.clear(); }

  void testGenerateWhen2Segments()
  {
    _routes->push_back(*_route1);
    _routes->push_back(*_route2);
    _generator = _memHandle.insert(new CombinationGenerator(2, *_routes, _route1->first));

    _generator->generate(*_solutions, 100000);
    _generator->generate(*_solutions, 1, 100000);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _solutions->size());
    TseUtil::SolutionSet::nth_index<0>::type::const_iterator pos = _solutions->get<0>().begin();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[0]);
  }

  void testGenerateWhen3Segments()
  {
    _routes->push_back(*_route1);
    _routes->push_back(*_route2);
    _routes->push_back(*_route3);
    _routes->push_back(*_route12);
    _routes->push_back(*_route23);
    _generator = _memHandle.insert(new CombinationGenerator(3, *_routes, _route1->first));

    _generator->generate(*_solutions, 100000);
    _generator->generate(*_solutions, 1, 100000);
    _generator->generate(*_solutions, 2, 100000);

    CPPUNIT_ASSERT_EQUAL(size_t(11), _solutions->size());
    TseUtil::SolutionSet::nth_index<0>::type::const_iterator pos = _solutions->get<0>().begin();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[0]);
  }

  void testGenerateWhen4SegmentsAndSecondIsSurfaceSegment()
  {
    TvlSegPair route1 = std::make_pair(_oneArunkTravel->begin(), _oneArunkTravel->begin() + 1);
    TvlSegPair route3 = std::make_pair(_oneArunkTravel->begin() + 2, _oneArunkTravel->begin() + 3);
    TvlSegPair route4 = std::make_pair(_oneArunkTravel->begin() + 3, _oneArunkTravel->begin() + 4);
    TvlSegPair route13 = std::make_pair(_oneArunkTravel->begin(), _oneArunkTravel->begin() + 3);
    TvlSegPair route34 = std::make_pair(_oneArunkTravel->begin() + 2, _oneArunkTravel->begin() + 4);
    _routes->push_back(route1);
    _routes->push_back(route3);
    _routes->push_back(route4);
    _routes->push_back(route13);
    _routes->push_back(route34);
    _generator = _memHandle.insert(new CombinationGenerator(4, *_routes, route1.first));

    _generator->generate(*_solutions, 100000);
    _generator->generate(*_solutions, 1, 100000);
    _generator->generate(*_solutions, 2, 100000);
    _generator->generate(*_solutions, 3, 100000);

    CPPUNIT_ASSERT_EQUAL(size_t(11), _solutions->size());
    TseUtil::SolutionSet::nth_index<0>::type::const_iterator pos = _solutions->get<0>().begin();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[0]);
  }

  void testGenerateFor6SegmentsWhenSecondThirdAndFifthAreSurfaceSegments()
  {
    TvlSegPair route1 =
        std::make_pair(_threeArunksTravel->begin(), _threeArunksTravel->begin() + 1);
    TvlSegPair route4 =
        std::make_pair(_threeArunksTravel->begin() + 3, _threeArunksTravel->begin() + 4);
    TvlSegPair route6 =
        std::make_pair(_threeArunksTravel->begin() + 5, _threeArunksTravel->begin() + 6);
    TvlSegPair route14 =
        std::make_pair(_threeArunksTravel->begin(), _threeArunksTravel->begin() + 4);
    TvlSegPair route46 =
        std::make_pair(_threeArunksTravel->begin() + 3, _threeArunksTravel->begin() + 6);
    _routes->push_back(route1);
    _routes->push_back(route4);
    _routes->push_back(route6);
    _routes->push_back(route14);
    _routes->push_back(route46);
    _generator = _memHandle.insert(new CombinationGenerator(6, *_routes, route1.first));

    _generator->generate(*_solutions, 100000);
    _generator->generate(*_solutions, 1, 100000);
    _generator->generate(*_solutions, 2, 100000);
    _generator->generate(*_solutions, 3, 100000);
    _generator->generate(*_solutions, 4, 100000);
    _generator->generate(*_solutions, 5, 100000);

    CPPUNIT_ASSERT_EQUAL(size_t(11), _solutions->size());
    TseUtil::SolutionSet::nth_index<0>::type::const_iterator pos = _solutions->get<0>().begin();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(4), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[0]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(1), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(5), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[0]);
  }

  void testGenerateFor8SegmentsWithMaskWhenThirdAndSeventhAreSurfaceSegments()
  {
    TvlSegPair route1 = std::make_pair(_twoArunksTravel->begin(), _twoArunksTravel->begin() + 1);
    TvlSegPair route2 =
        std::make_pair(_twoArunksTravel->begin() + 1, _twoArunksTravel->begin() + 2);
    TvlSegPair route4 =
        std::make_pair(_twoArunksTravel->begin() + 3, _twoArunksTravel->begin() + 4);
    TvlSegPair route5 =
        std::make_pair(_twoArunksTravel->begin() + 4, _twoArunksTravel->begin() + 5);
    TvlSegPair route6 =
        std::make_pair(_twoArunksTravel->begin() + 5, _twoArunksTravel->begin() + 6);
    TvlSegPair route8 =
        std::make_pair(_twoArunksTravel->begin() + 7, _twoArunksTravel->begin() + 8);
    TvlSegPair route12 = std::make_pair(_twoArunksTravel->begin(), _twoArunksTravel->begin() + 2);
    TvlSegPair route14 = std::make_pair(_twoArunksTravel->begin(), _twoArunksTravel->begin() + 4);
    TvlSegPair route15 = std::make_pair(_twoArunksTravel->begin(), _twoArunksTravel->begin() + 5);
    TvlSegPair route16 = std::make_pair(_twoArunksTravel->begin(), _twoArunksTravel->begin() + 6);
    TvlSegPair route18 = std::make_pair(_twoArunksTravel->begin(), _twoArunksTravel->begin() + 8);
    TvlSegPair route24 =
        std::make_pair(_twoArunksTravel->begin() + 1, _twoArunksTravel->begin() + 4);
    TvlSegPair route25 =
        std::make_pair(_twoArunksTravel->begin() + 1, _twoArunksTravel->begin() + 5);
    TvlSegPair route26 =
        std::make_pair(_twoArunksTravel->begin() + 1, _twoArunksTravel->begin() + 6);
    TvlSegPair route28 =
        std::make_pair(_twoArunksTravel->begin() + 1, _twoArunksTravel->begin() + 8);
    TvlSegPair route45 =
        std::make_pair(_twoArunksTravel->begin() + 3, _twoArunksTravel->begin() + 5);
    TvlSegPair route46 =
        std::make_pair(_twoArunksTravel->begin() + 3, _twoArunksTravel->begin() + 6);
    TvlSegPair route48 =
        std::make_pair(_twoArunksTravel->begin() + 3, _twoArunksTravel->begin() + 8);
    TvlSegPair route56 =
        std::make_pair(_twoArunksTravel->begin() + 4, _twoArunksTravel->begin() + 6);
    TvlSegPair route58 =
        std::make_pair(_twoArunksTravel->begin() + 4, _twoArunksTravel->begin() + 8);
    TvlSegPair route68 =
        std::make_pair(_twoArunksTravel->begin() + 5, _twoArunksTravel->begin() + 8);
    _routes->push_back(route1);
    _routes->push_back(route2);
    _routes->push_back(route4);
    _routes->push_back(route5);
    _routes->push_back(route6);
    _routes->push_back(route8);
    _routes->push_back(route12);
    _routes->push_back(route14);
    _routes->push_back(route15);
    _routes->push_back(route16);
    _routes->push_back(route18);
    _routes->push_back(route24);
    _routes->push_back(route25);
    _routes->push_back(route26);
    _routes->push_back(route28);
    _routes->push_back(route45);
    _routes->push_back(route46);
    _routes->push_back(route48);
    _routes->push_back(route56);
    _routes->push_back(route58);
    _routes->push_back(route68);
    _generator = _memHandle.insert(new CombinationGenerator(8, *_routes, route1.first));

    uint64_t arunkSegsMask = 0x1010; // 1 and 7 seg = 01 00 00 00 01 00 00
    _generator->generate(*_solutions, 1, arunkSegsMask, 100000);
    _generator->generate(*_solutions, 2, arunkSegsMask, 100000);

    CPPUNIT_ASSERT_EQUAL(size_t(24), _solutions->size());
    TseUtil::SolutionSet::nth_index<0>::type::const_iterator pos = _solutions->get<0>().begin();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[9], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(2), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[6], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[17], pos->_routes[1]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[17], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[6], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[19], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[6], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[15], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[20], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[13], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[7], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[18], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[8], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(4), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[19], pos->_routes[3]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(4), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[15], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[20], pos->_routes[3]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(4), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[6], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[20], pos->_routes[3]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(4), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[11], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[18], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[3]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(4), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[12], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[3]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(4), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[7], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[3]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(5), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[3]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[20], pos->_routes[4]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(5), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(1), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[11], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[3]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[4]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(3), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[6], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[16], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[2]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(4), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[16], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[3]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(4), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[6], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[18], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[3]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(4), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[6], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[15], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[3]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(5), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[18], pos->_routes[3]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[4]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(5), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[15], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[3]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[4]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(5), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[6], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[3]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[4]);
    ++pos;
    CPPUNIT_ASSERT_EQUAL(size_t(6), pos->_routes.size());
    CPPUNIT_ASSERT_EQUAL(uint16_t(2), pos->_skipped);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[0], pos->_routes[0]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[1], pos->_routes[1]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[2], pos->_routes[2]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[3], pos->_routes[3]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[4], pos->_routes[4]);
    CPPUNIT_ASSERT_EQUAL(&(*_routes)[5], pos->_routes[5]);
  }

  void testCollectFareMarketsWhenFoundAll4FMs()
  {
    int fms[4] = { 0 };
    fms[0] = 1;
    fms[1] = 2;
    fms[2] = 3;
    fms[3] = 4;
    _generator->_mappedRoute.insert(std::make_pair(fms[0], _route1));
    _generator->_mappedRoute.insert(std::make_pair(fms[1], _route2));
    _generator->_mappedRoute.insert(std::make_pair(fms[2], _route3));
    _generator->_mappedRoute.insert(std::make_pair(fms[3], _route4));

    CPPUNIT_ASSERT(_generator->collectFareMarkets(fms, *_route));
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[2]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[3]);
  }
  void testCollectFareMarketsWhen4thNotFound()
  {
    int fms[4] = { 0 };
    fms[0] = 1;
    fms[1] = 2;
    fms[2] = 3;
    fms[3] = 5;
    _generator->_mappedRoute.insert(std::make_pair(fms[0], _route1));
    _generator->_mappedRoute.insert(std::make_pair(fms[1], _route2));
    _generator->_mappedRoute.insert(std::make_pair(fms[2], _route3));
    _generator->_mappedRoute.insert(std::make_pair(4, _route4));

    CPPUNIT_ASSERT(!_generator->collectFareMarkets(fms, *_route));
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[2]);
  }
  void testCollectFareMarketsWhen4thEmpty()
  {
    int fms[4] = { 0 };
    fms[0] = 1;
    fms[1] = 2;
    fms[2] = 3;
    fms[3] = 0;
    _generator->_mappedRoute.insert(std::make_pair(fms[0], _route1));
    _generator->_mappedRoute.insert(std::make_pair(fms[1], _route2));
    _generator->_mappedRoute.insert(std::make_pair(fms[2], _route3));
    _generator->_mappedRoute.insert(std::make_pair(4, _route4));

    CPPUNIT_ASSERT(_generator->collectFareMarkets(fms, *_route));
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[2]);
  }
  void testCollectFareMarketsWhen3thNotFound()
  {
    int fms[4] = { 0 };
    fms[0] = 1;
    fms[1] = 2;
    fms[2] = 5;
    fms[3] = 4;
    _generator->_mappedRoute.insert(std::make_pair(fms[0], _route1));
    _generator->_mappedRoute.insert(std::make_pair(fms[1], _route2));
    _generator->_mappedRoute.insert(std::make_pair(3, _route3));
    _generator->_mappedRoute.insert(std::make_pair(fms[3], _route4));

    CPPUNIT_ASSERT(!_generator->collectFareMarkets(fms, *_route));
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
  }
  void testCollectFareMarketsWhen3thEmpty()
  {
    int fms[4] = { 0 };
    fms[0] = 1;
    fms[1] = 2;
    fms[2] = 0;
    fms[3] = 0;
    _generator->_mappedRoute.insert(std::make_pair(fms[0], _route1));
    _generator->_mappedRoute.insert(std::make_pair(fms[1], _route2));
    _generator->_mappedRoute.insert(std::make_pair(3, _route3));
    _generator->_mappedRoute.insert(std::make_pair(4, _route4));

    CPPUNIT_ASSERT(_generator->collectFareMarkets(fms, *_route));
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
  }
  void testCollectFareMarketsWhenSecondNotFound()
  {
    int fms[4] = { 0 };
    fms[0] = 1;
    fms[1] = 5;
    fms[2] = 3;
    fms[3] = 4;
    _generator->_mappedRoute.insert(std::make_pair(fms[0], _route1));
    _generator->_mappedRoute.insert(std::make_pair(2, _route2));
    _generator->_mappedRoute.insert(std::make_pair(fms[2], _route3));
    _generator->_mappedRoute.insert(std::make_pair(fms[3], _route4));

    CPPUNIT_ASSERT(!_generator->collectFareMarkets(fms, *_route));
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
  }
  void testCollectFareMarketsWhenSecondEmpty()
  {
    int fms[4] = { 0 };
    fms[0] = 1;
    fms[1] = 0;
    fms[2] = 0;
    fms[3] = 0;
    _generator->_mappedRoute.insert(std::make_pair(fms[0], _route1));
    _generator->_mappedRoute.insert(std::make_pair(2, _route2));
    _generator->_mappedRoute.insert(std::make_pair(3, _route3));
    _generator->_mappedRoute.insert(std::make_pair(4, _route4));

    CPPUNIT_ASSERT(_generator->collectFareMarkets(fms, *_route));
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
  }
  void testCollectFareMarketsWhenFirstNotFound()
  {
    int fms[4] = { 0 };
    fms[0] = 5;
    fms[1] = 2;
    fms[2] = 3;
    fms[3] = 4;
    _generator->_mappedRoute.insert(std::make_pair(1, _route1));
    _generator->_mappedRoute.insert(std::make_pair(fms[1], _route2));
    _generator->_mappedRoute.insert(std::make_pair(fms[2], _route3));
    _generator->_mappedRoute.insert(std::make_pair(fms[3], _route4));

    CPPUNIT_ASSERT(!_generator->collectFareMarkets(fms, *_route));
  }
  void testGetFareMarketsWhenCombIs0()
  {
    *_combNo = 0;
    *_startShift = 4;

    CPPUNIT_ASSERT_EQUAL(*_startShift,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
  }
  void testGetFareMarketsWhenComb10000000XX() // x1 2-5
  {
    *_combNo = 3;
    *_endShift = 0;

    CPPUNIT_ASSERT_EQUAL(1,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
  }
  void testGetFareMarketsWhenComb100000XXXX() // x1 x2 3-5
  {
    *_combNo = 15;
    *_endShift = 0;

    CPPUNIT_ASSERT_EQUAL(2,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
  }
  void testGetFareMarketsWhenComb1000XXXXXX() // x1 x2 x3 4-5
  {
    *_combNo = 63;
    *_endShift = 0;

    CPPUNIT_ASSERT_EQUAL(3,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
  }
  void testGetFareMarketsWhenComb10XXXXXXXX() // x1 x2 x3 x4
  {
    *_combNo = 255;
    *_endShift = 0;

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
  }
  void setupMappedFMs()
  {
    _generator->_mappedRoute.insert(std::make_pair(5, _route1));
    _generator->_mappedRoute.insert(std::make_pair(6, _route12));
    _generator->_mappedRoute.insert(std::make_pair(7, _route13));
    _generator->_mappedRoute.insert(std::make_pair(8, _route14));
    _generator->_mappedRoute.insert(std::make_pair(10, _route2));
    _generator->_mappedRoute.insert(std::make_pair(11, _route23));
    _generator->_mappedRoute.insert(std::make_pair(12, _route24));
    _generator->_mappedRoute.insert(std::make_pair(15, _route3));
    _generator->_mappedRoute.insert(std::make_pair(16, _route34));
    _generator->_mappedRoute.insert(std::make_pair(20, _route4));
  }
  void testGetFareMarketsWhenComb10() // 1-1
  {
    *_combNo = 2;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(1,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
  }
  void testGetFareMarketsWhenComb1000() // 1-2
  {
    *_combNo = 8;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(2,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route12, (*_route)[0]);
  }
  void testGetFareMarketsWhenComb1010() // 1-1 2-2
  {
    *_combNo = 10;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(2,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb1011() // x1 2-2
  {
    *_combNo = 11;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(2,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[0]);
  }
  void testGetFareMarketsWhenComb100000() // 1-3
  {
    *_combNo = 32;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(3,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route13, (*_route)[0]);
  }
  void testGetFareMarketsWhenComb100010() // 1-1 2-3
  {
    *_combNo = 34;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(3,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route23, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb100011() // x1 2-3
  {
    *_combNo = 35;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(3,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route23, (*_route)[0]);
  }
  void testGetFareMarketsWhenComb101000() // 1-2 3-3
  {
    *_combNo = 40;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(3,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route12, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb101010() // 1-1 2-2 3-3
  {
    *_combNo = 42;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(3,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(3), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[2]);
  }
  void testGetFareMarketsWhenComb101011() // x1 2-2 3-3
  {
    *_combNo = 43;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(3,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb101110() // 1-1 x2 3-3
  {
    *_combNo = 46;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(3,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb101111() // x1 x2 3-3
  {
    *_combNo = 47;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(3,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[0]);
  }
  void testGetFareMarketsWhenComb10000000() // 1-4
  {
    *_combNo = 128;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route14, (*_route)[0]);
  }
  void testGetFareMarketsWhenComb10000010() // 1-1 2-4
  {
    *_combNo = 130;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route24, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10000011() // x1 2-4
  {
    *_combNo = 131;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route24, (*_route)[0]);
  }
  void testGetFareMarketsWhenComb10001000() // 1-2 3-4
  {
    *_combNo = 136;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route12, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route34, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10001010() // 1-1 2-2 3-4
  {
    *_combNo = 138;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(3), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route34, (*_route)[2]);
  }
  void testGetFareMarketsWhenComb10001011() // x1 2-2 3-4
  {
    *_combNo = 139;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route34, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10001110() // 1-1 x2 3-4
  {
    *_combNo = 142;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route34, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10001111() // x1 x2 3-4
  {
    *_combNo = 143;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route34, (*_route)[0]);
  }
  void testGetFareMarketsWhenComb10100000() // 1-3 4-4
  {
    *_combNo = 160;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route13, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10100010() // 1-1 2-3 4-4
  {
    *_combNo = 162;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(3), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route23, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[2]);
  }
  void testGetFareMarketsWhenComb10100011() // x1 2-3 4-4
  {
    *_combNo = 163;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route23, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10101000() // 1-2 3-3 4-4
  {
    *_combNo = 168;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(3), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route12, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[2]);
  }
  void testGetFareMarketsWhenComb10101010() // 1-1 2-2 3-3 4-4
  {
    *_combNo = 170;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(4), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[2]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[3]);
  }
  void testGetFareMarketsWhenComb10101011() // x1 2-2 3-3 4-4
  {
    *_combNo = 171;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(3), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[2]);
  }
  void testGetFareMarketsWhenComb10101110() // 1-1 x2 3-3 4-4
  {
    *_combNo = 174;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(3), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[2]);
  }
  void testGetFareMarketsWhenComb10101111() // x1 x2 3-3 4-4
  {
    *_combNo = 175;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route3, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10111000() // 1-2 x3 4-4
  {
    *_combNo = 184;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route12, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10111010() // 1-1 2-2 x3 4-4
  {
    *_combNo = 186;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(3), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[1]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[2]);
  }
  void testGetFareMarketsWhenComb10111011() // x1 2-2 x3 4-4
  {
    *_combNo = 187;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route2, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10111110() // 1-1 x2 x3 4-4
  {
    *_combNo = 190;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(2), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route1, (*_route)[0]);
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[1]);
  }
  void testGetFareMarketsWhenComb10111111() // x1 x2 x3 4-4
  {
    *_combNo = 191;
    *_endShift = 0;
    setupMappedFMs();

    CPPUNIT_ASSERT_EQUAL(4,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _route->size());
    CPPUNIT_ASSERT_EQUAL(_route4, (*_route)[0]);
  }
  void testGetFareMarketsWhenFMNotFound()
  {
    *_combNo = 191;
    *_endShift = 0;

    CPPUNIT_ASSERT_EQUAL(-1,
                         _generator->getFareMarkets(*_combNo, *_startShift, *_endShift, *_route));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationGeneratorTest);
}
