#include "test/include/CppUnitHelperMacros.h"
#include "test/testdata/TestFactoryManager.h"
#include "Common/TseCodeTypes.h"
#include "RexPricing/FareBreakProcessor.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/AirSeg.h"
#include <iostream>

using namespace tse;
using namespace std;

namespace tse
{

class FareBreakProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareBreakProcessorTest);
  CPPUNIT_TEST(testIsFareBreakValid12Keep24Hist);
  CPPUNIT_TEST(testIsFareBreakValid13Keep34Hist);
  CPPUNIT_TEST(testIsFareBreakValid13Keep34Keep);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    TestFixture::setUp();

    buildTravelPoints();
    buildTravelSegs();
    buildAllTestFareMarkets();
    buildItins();
  }

  void tearDown()
  {
    TestFixture::tearDown();
    TestFactoryManager::instance()->destroyAll();
  }

  void buildTravelPoints()
  {
    _loc1 = "ABC";
    _loc2 = "BCD";
    _loc3 = "CDE";
    _loc4 = "DEF";
    _loc5 = "EFG";
  }

  void buildTravelSegs()
  {
    _tvlSeg12 = new AirSeg();
    _tvlSeg12->pnrSegment() = 1;
    _tvlSeg12->origAirport() = _loc1;
    _tvlSeg12->destAirport() = _loc2;

    _tvlSeg23 = new AirSeg();
    _tvlSeg23->pnrSegment() = 2;
    _tvlSeg23->origAirport() = _loc2;
    _tvlSeg23->destAirport() = _loc3;

    _tvlSeg34 = new AirSeg();
    _tvlSeg34->pnrSegment() = 3;
    _tvlSeg34->origAirport() = _loc3;
    _tvlSeg34->destAirport() = _loc4;

    _tvlSeg35 = new AirSeg();
    _tvlSeg35->pnrSegment() = 3;
    _tvlSeg35->origAirport() = _loc3;
    _tvlSeg35->destAirport() = _loc5;
  }

  void buildAllTestFareMarkets()
  {
    _dataHandle.get(_fm12);
    _fm12->travelSeg().push_back(_tvlSeg12);

    _dataHandle.get(_fm13);
    _fm13->travelSeg().push_back(_tvlSeg12);
    _fm13->travelSeg().push_back(_tvlSeg23);

    _dataHandle.get(_fm14);
    _fm14->travelSeg().push_back(_tvlSeg12);
    _fm14->travelSeg().push_back(_tvlSeg23);
    _fm14->travelSeg().push_back(_tvlSeg34);

    _dataHandle.get(_fm23);
    _fm23->travelSeg().push_back(_tvlSeg23);

    _dataHandle.get(_fm24);
    _fm24->travelSeg().push_back(_tvlSeg23);
    _fm24->travelSeg().push_back(_tvlSeg34);

    _dataHandle.get(_fm25);
    _fm25->travelSeg().push_back(_tvlSeg23);
    _fm25->travelSeg().push_back(_tvlSeg35);

    _dataHandle.get(_fm34);
    _fm34->travelSeg().push_back(_tvlSeg34);

    _dataHandle.get(_fm35);
    _fm35->travelSeg().push_back(_tvlSeg35);
  }

  void buildItins()
  {
    _excItin.travelSeg().push_back(_tvlSeg12);
    _excItin.travelSeg().push_back(_tvlSeg23);
    _excItin.travelSeg().push_back(_tvlSeg34);

    _newItin.travelSeg().push_back(_tvlSeg12);
    _newItin.travelSeg().push_back(_tvlSeg23);
    _newItin.travelSeg().push_back(_tvlSeg35);
  }

  void testIsFareBreakValid12Keep24Hist()
  {
    _processTags.clear();
    addProcessTag(*_fm12, KEEP_THE_FARES);
    addProcessTag(*_fm24, GUARANTEED_AIR_FARE);

    FareBreakProcessor fbp;
    fbp.setup(_excItin, _newItin, _processTags);

    CPPUNIT_ASSERT(true == fbp.isFareBreakValid(*_fm12));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm13));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm14));
    CPPUNIT_ASSERT(true == fbp.isFareBreakValid(*_fm23));
    CPPUNIT_ASSERT(true == fbp.isFareBreakValid(*_fm25));
    CPPUNIT_ASSERT(true == fbp.isFareBreakValid(*_fm35));
  }

  void testIsFareBreakValid13Keep34Hist()
  {
    _processTags.clear();
    addProcessTag(*_fm13, KEEP_THE_FARES);
    addProcessTag(*_fm34, GUARANTEED_AIR_FARE);

    FareBreakProcessor fbp;
    CPPUNIT_ASSERT(true == fbp.setup(_excItin, _newItin, _processTags));

    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm12));
    CPPUNIT_ASSERT(true == fbp.isFareBreakValid(*_fm13));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm14));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm23));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm25));
    CPPUNIT_ASSERT(true == fbp.isFareBreakValid(*_fm35));
  }

  void testIsFareBreakValid13Keep34Keep()
  {
    _processTags.clear();
    addProcessTag(*_fm13, KEEP_THE_FARES);
    addProcessTag(*_fm34, KEEP_THE_FARES);

    FareBreakProcessor fbp;
    CPPUNIT_ASSERT(false == fbp.setup(_excItin, _newItin, _processTags));

    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm12));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm13));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm14));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm23));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm25));
    CPPUNIT_ASSERT(false == fbp.isFareBreakValid(*_fm35));
  }

  void addProcessTag(FareMarket& fm, const ProcessTag& processTag)
  {
    ProcessTagInfo* pInfo = 0;
    _dataHandle.get(pInfo);

    ReissueSequence* rs = 0;
    _dataHandle.get(rs);
    pInfo->reissueSequence()->orig() = rs;
    rs->processingInd() = (int)processTag;
    FareCompInfo* fcInfo = 0;
    _dataHandle.get(fcInfo);
    fcInfo->fareMarket() = &fm;
    pInfo->fareCompInfo() = fcInfo;

    _processTags.push_back(pInfo);
  }

protected:
private:
  DataHandle _dataHandle;
  LocCode _loc1, _loc2, _loc3, _loc4, _loc5;
  TravelSeg* _tvlSeg12, *_tvlSeg23, *_tvlSeg34, *_tvlSeg35;
  Itin _excItin, _newItin;
  std::vector<ProcessTagInfo*> _processTags;

  FareMarket* _fm12, *_fm13, *_fm14, *_fm23, *_fm24, *_fm25, *_fm34, *_fm35;
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(FareBreakProcessorTest);
