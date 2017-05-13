#include "Common/BaggageTripType.h"
#include "DBAccess/Mileage.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/SurfaceSeg.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FreeBagService/BaggageItinAnalyzer.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/include/CppUnitHelperMacros.h"

#include <boost/assign/std/vector.hpp>

namespace tse
{
using boost::assign::operator+=;

// MOCKS
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

  const std::vector<Mileage*>& getMileage(const LocCode& origin,
                                          const LocCode& dest,
                                          const DateTime& date,
                                          Indicator mileageType)
  {
    std::vector<Mileage*>* ret = _memHandle.create<std::vector<Mileage*> >();
    Mileage* mil = _memHandle.create<Mileage>();

    mil->orig() = origin;
    mil->dest() = dest;
    mil->mileageType() = mileageType;

    if (origin == "MOW" && dest == "NCE")
    {
      mil->mileage() = 2000;
      ret->push_back(mil);
    }
    else if (origin == "MOW" && dest == "NYC")
    {
      mil->mileage() = 1000;
      ret->push_back(mil);
    }
    else if (origin == "MPM" && mileageType == MPM)
    {
      mil->mileage() = 120;
      ret->push_back(mil);
    }
    else if (origin == "TPM" && mileageType == TPM)
    {
      mil->mileage() = 200;
      ret->push_back(mil);
    }
    else if (origin != "CIR")
      return DataHandleMock::getMileage(origin, dest, date, mileageType);

    return *ret;
  }
};
}

class BaggageItinAnalyzerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageItinAnalyzerTest);

  CPPUNIT_TEST(testGetStopOverLength_TripOnlyInUS);
  CPPUNIT_TEST(testGetStopOverLength_TripOnlyInCanada);
  CPPUNIT_TEST(testGetStopOverLength_TripInUSAndCanada);
  CPPUNIT_TEST(testGetStopOverLength_TripNotInUSOrCanada);

  CPPUNIT_TEST(testIsCAorPanama_CA);
  CPPUNIT_TEST(testIsCAorPanama_Panama);
  CPPUNIT_TEST(testIsCAorPanama_CAandPanama);
  CPPUNIT_TEST(testIsCAorPanama_Other);

  CPPUNIT_TEST(testIsWholeTravelInCAOrPanama_BothPanama);
  CPPUNIT_TEST(testIsWholeTravelInCAOrPanama_OnePanamaOneCA);
  CPPUNIT_TEST(testIsWholeTravelInCAOrPanama_NonePanamaOrCA);

  CPPUNIT_TEST(testDetermineBaggageTravelsForUsDot_NoCheckedPoints);
  CPPUNIT_TEST(testDetermineBaggageTravelsForUsDot_OneBaggageTravel);
  CPPUNIT_TEST(testDetermineBaggageTravelsForUsDot_TwoBaggageTravels);
  CPPUNIT_TEST(testDetermineBaggageTravelsForUsDot_AdjustOutbound);
  CPPUNIT_TEST(testDetermineBaggageTravelsForUsDot_AdjustInbound);
  CPPUNIT_TEST(testDetermineBaggageTravelsForUsDot_furthestCheckedPointAtOrigin);
  CPPUNIT_TEST(testDetermineBaggageTravelsForUsDot_AdjustOutboundFPAO);
  CPPUNIT_TEST(testDetermineBaggageTravelsForUsDot_AdjustInboundFPAO);
  CPPUNIT_TEST(testDetermineBaggageTravelsForUsDot_AdjustInboundLastArunk);

  CPPUNIT_TEST(testDetermineBaggageTravelsForNonUsDot_NoCheckedPoints);
  CPPUNIT_TEST(testDetermineBaggageTravelsForNonUsDot_OneCheckedPoint);

  CPPUNIT_TEST(testInsertFurthestTicketedPointAsCP);

  CPPUNIT_TEST(testCloneBaggageTravelsForAllFarePaths_EmptyFarePathsAndBaggageTravels);
  CPPUNIT_TEST(testCloneBaggageTravelsForAllFarePaths_EmptyFarePathsOneBaggageTravel);
  CPPUNIT_TEST(testCloneBaggageTravelsForAllFarePaths_OneFarePathEmptyBaggageTravels);
  CPPUNIT_TEST(testCloneBaggageTravelsForAllFarePaths_OneFarePathOneBaggageTravel);
  CPPUNIT_TEST(testCloneBaggageTravelsForAllFarePaths_MultipleFarePathsEmptyBaggageTravels);
  CPPUNIT_TEST(testCloneBaggageTravelsForAllFarePaths_MultipleFarePathsOneBaggageTravel);
  CPPUNIT_TEST(testCloneBaggageTravelsForAllFarePaths_MultipleFarePathsMultipleBaggageTravel);

  CPPUNIT_TEST(testSetUsDot_WholeTravelInUS_NewMethod);
  CPPUNIT_TEST(testSetUsDot_WholeTravelInUS_OldMethod);
  CPPUNIT_TEST(testSetUsDot_FirstPointInUS);
  CPPUNIT_TEST(testSetUsDot_LastPointInUS);

  CPPUNIT_TEST(testSetBaggageTripType_WhollyInUS);
  CPPUNIT_TEST(testSetBaggageTripType_WhollyInCA);
  CPPUNIT_TEST(testSetBaggageTripType_BetweenUSCA);
  CPPUNIT_TEST(testSetBaggageTripType_ToFromUS);
  CPPUNIT_TEST(testSetBaggageTripType_ToFromCA_StopoverUS);
  CPPUNIT_TEST(testSetBaggageTripType_ToFromCA);
  CPPUNIT_TEST(testSetBaggageTripType_Other);

  CPPUNIT_TEST(testDetermineFurthestCheckedPoint_OneTravelSegment);
  CPPUNIT_TEST(testDetermineFurthestCheckedPoint_OneCheckedPoint);
  CPPUNIT_TEST(testDetermineFurthestCheckedPoint_TwoCheckedPoints);
  CPPUNIT_TEST(testDetermineFurthestCheckedPoint_Retransit);

  CPPUNIT_TEST(testDetermineMss_SingleSegmentSameCountry);
  CPPUNIT_TEST(testDetermineMss_AirAndSurfaceSegments);
  CPPUNIT_TEST(testDetermineMss_AreaChanged);
  CPPUNIT_TEST(testDetermineMss_AreaChanged312);
  CPPUNIT_TEST(testDetermineMss_AreaChanged321);
  CPPUNIT_TEST(testDetermineMss_AreaChanged123);
  CPPUNIT_TEST(testDetermineMss_SubAreaChanged);
  CPPUNIT_TEST(testDetermineMss_NationChanged);
  CPPUNIT_TEST(testDetermineMss_SameCountry);

  CPPUNIT_TEST(testDetermineMssForNonUsDot_OneBaggageTravel);
  CPPUNIT_TEST(testDetermineMssForNonUsDot_MultipleBaggageTravels);

  CPPUNIT_TEST(testDetermineMssForUsDot_OneBaggageTravel);
  CPPUNIT_TEST(testDetermineMssForUsDot_OneBaggageTravel_WhollyWithinUs);
  CPPUNIT_TEST(testDetermineMssForUsDot_TwoBaggageTravels);
  CPPUNIT_TEST(testDetermineMssForUsDot_TwoBaggageTravels_WhollyWithinUs);
  CPPUNIT_TEST(testDetermineMssForUsDot_TwoBaggageTravels_WhollyWithinCa);

  CPPUNIT_TEST(testIsDestinationOnlyCheckedPoint);
  CPPUNIT_TEST(testIsDestinationOnlyCheckedPoint_NotOnly);
  CPPUNIT_TEST(testIsDestinationOnlyCheckedPoint_NotOnlyButSame);

  CPPUNIT_TEST(testGetMileage_Mpm);
  CPPUNIT_TEST(testGetMileage_Tpm);
  CPPUNIT_TEST(testGetMileage_Circle);

  CPPUNIT_TEST(testPrintMileage);
  CPPUNIT_TEST(testPrintItinAnalysisResults);
  CPPUNIT_TEST(testPrintBaggageTravels);

  CPPUNIT_TEST(testRemoveDummyBaggageTravels);
  CPPUNIT_TEST(testRemoveDummyBaggageTravels_NoOriginBasedRT);
  CPPUNIT_TEST(testRemoveDummyBaggageTravels_NoMip);
  CPPUNIT_TEST(testRemoveDummyBaggageTravels_NoDummy);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _itin = _memHandle.create<Itin>();
    _itin->farePath().push_back(_memHandle.create<FarePath>());
    _req = _memHandle.create<PricingRequest>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_req);
    _trx->itin().push_back(_itin);
    _baggageItinAnalyzer = _memHandle.insert(new BaggageItinAnalyzer(*_trx, *_itin));
  }

  void tearDown() { _memHandle.clear(); }

protected:

  Mileage* createMileage(GlobalDirection gd, uint32_t mileage)
  {
    Mileage* mil = _memHandle(new Mileage);
    mil->mileage() = mileage;
    mil->globaldir() = gd;
    return mil;
  }

  void setMileage(std::initializer_list<Mileage*> mileages)
  {
    _mdh->set_getMileageLocCodeLocCodeDateTimeIndicator(
        _memHandle(new std::vector<Mileage*>(mileages)));
  }

  void addAirSegsToItin(int asCount, int surfaceSegNo = -1)
  {
    for (int i = 0; i < asCount; ++i)
    {
      TravelSeg* travelSeg = 0;
      if (i == surfaceSegNo)
        travelSeg = _memHandle.create<SurfaceSeg>();
      else
        travelSeg = _memHandle.create<AirSeg>();

      travelSeg->origin() = _memHandle.create<Loc>();
      travelSeg->destination() = _memHandle.create<Loc>();
      travelSeg->departureDT() = DateTime::localTime();
      travelSeg->arrivalDT() = DateTime::localTime();
      _itin->travelSeg() += travelSeg;
    }
  }

  void addBaggageTravel()
  {
    addAirSegsToItin(1);
    TravelSegPtrVecCI lastItinSegment = _itin->travelSeg().end() - 1;
    addBaggageTravel(lastItinSegment, lastItinSegment);
  }

  void addBaggageTravel(TravelSegPtrVecCI beginIter,
                        TravelSegPtrVecCI endIter,
                        BaggageItinAnalyzer* itinAnalyzer = 0)
  {
    BaggageTravel* baggageTravel = BaggageTravelBuilder(&_memHandle)
                                       .withFarePath(_itin->farePath().front())
                                       .withTrx(_trx)
                                       .withTravelSeg(beginIter, endIter)
                                       .build();

    baggageTravel->_MSS = beginIter;

    if (itinAnalyzer)
      itinAnalyzer->_baggageTravels += baggageTravel;
    else
      _baggageItinAnalyzer->_baggageTravels += baggageTravel;
  }

  void addFarePathToItin() { _itin->farePath().push_back(_memHandle.create<FarePath>()); }

  void verifyUsDotBaggageTravel(const std::vector<BaggageTravel*>& baggageTravels,
                                const TravelSegPtrVecCI firstBegin,
                                const TravelSegPtrVecCI firstEnd,
                                const TravelSegPtrVecCI secondBegin,
                                const TravelSegPtrVecCI secondEnd,
                                const size_t count = 2)
  {
    CPPUNIT_ASSERT(count == 1 || count == 2);
    CPPUNIT_ASSERT_EQUAL(count, baggageTravels.size());
    CPPUNIT_ASSERT(firstBegin == baggageTravels.front()->getTravelSegBegin());
    CPPUNIT_ASSERT(firstEnd == baggageTravels.front()->getTravelSegEnd());
    if (count == 2)
    {
      CPPUNIT_ASSERT(secondBegin == baggageTravels.back()->getTravelSegBegin());
      CPPUNIT_ASSERT(secondEnd == baggageTravels.back()->getTravelSegEnd());
    }
  }

  void verifyClonedBaggageTravels(const std::vector<BaggageTravel*>& baggageTravels,
                                  const std::vector<FarePath*>& farePaths)
  {
    std::vector<BaggageTravel*>::const_iterator baggageTravelsOriginalEnd =
        baggageTravels.begin() + (baggageTravels.size() / farePaths.size());
    std::vector<BaggageTravel*>::const_iterator allBaggageTravelsIter = baggageTravelsOriginalEnd;

    std::vector<FarePath*>::const_iterator farePathsIter = farePaths.begin() + 1;
    std::vector<FarePath*>::const_iterator farePathsEnd = farePaths.end();
    for (; farePathsIter != farePathsEnd; ++farePathsIter)
    {
      std::vector<BaggageTravel*>::const_iterator baggageTravelsOriginalIter =
          baggageTravels.begin();
      for (; baggageTravelsOriginalIter != baggageTravelsOriginalEnd;
           ++baggageTravelsOriginalIter, ++allBaggageTravelsIter)
      {
        CPPUNIT_ASSERT((*allBaggageTravelsIter)->farePath() == *farePathsIter);
        CPPUNIT_ASSERT((*allBaggageTravelsIter)->_trx == (*baggageTravelsOriginalIter)->_trx);
        CPPUNIT_ASSERT((*allBaggageTravelsIter)->getTravelSegBegin() ==
                       (*baggageTravelsOriginalIter)->getTravelSegBegin());
        CPPUNIT_ASSERT((*allBaggageTravelsIter)->getTravelSegEnd() ==
                       (*baggageTravelsOriginalIter)->getTravelSegEnd());
        CPPUNIT_ASSERT((*allBaggageTravelsIter)->_MSS == (*baggageTravelsOriginalIter)->_MSS);
        CPPUNIT_ASSERT((*allBaggageTravelsIter)->_MSSJourney ==
                       (*baggageTravelsOriginalIter)->_MSSJourney);
        CPPUNIT_ASSERT((*allBaggageTravelsIter)->_allowance ==
                       (*baggageTravelsOriginalIter)->_allowance);
      }
    }
  }

  void createDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic852;
    _rootDiag = _memHandle.create<Diagnostic>();
    _baggageItinAnalyzer->_diag852 = _memHandle.insert(new Diag852Collector(*_rootDiag));
    _baggageItinAnalyzer->_diag852->useThreadDiag() = true;
    _baggageItinAnalyzer->_diag852->trx() = _trx;
    _rootDiag->activate();
  }

  DataHandleMock* _mdh;
  TestMemHandle _memHandle;
  PricingRequest* _req;
  PricingTrx* _trx;
  Itin* _itin;
  BaggageItinAnalyzer* _baggageItinAnalyzer;
  Diagnostic* _rootDiag;

public:
  // TESTS

  void testGetStopOverLength_TripOnlyInUS()
  {
    _baggageItinAnalyzer->_wholeTravelInUS = true;
    CPPUNIT_ASSERT_EQUAL((int64_t)(4 * 3600), _baggageItinAnalyzer->getStopOverLength());
  }

  void testGetStopOverLength_TripOnlyInCanada()
  {
    _itin->tripCharacteristics().set(Itin::CanadaOnly);
    CPPUNIT_ASSERT_EQUAL((int64_t)(4 * 3600), _baggageItinAnalyzer->getStopOverLength());
  }

  void testGetStopOverLength_TripInUSAndCanada()
  {
    _itin->geoTravelType() = GeoTravelType::Transborder;
    CPPUNIT_ASSERT_EQUAL((int64_t)(4 * 3600), _baggageItinAnalyzer->getStopOverLength());
  }

  void testGetStopOverLength_TripNotInUSOrCanada()
  {
    _itin->geoTravelType() = GeoTravelType::International;
    _itin->tripCharacteristics().setNull();
    CPPUNIT_ASSERT_EQUAL((int64_t)(24 * 3600), _baggageItinAnalyzer->getStopOverLength());
  }

  void testIsCAorPanama_CA()
  {
    Loc loc;

    loc.subarea() = IATA_SUB_AREA_13();
    CPPUNIT_ASSERT_EQUAL(true, _baggageItinAnalyzer->isCAorPanama(&loc));
  }

  void testIsCAorPanama_Panama()
  {
    Loc loc;

    loc.nation() = NATION_PANAMA;
    CPPUNIT_ASSERT_EQUAL(true, _baggageItinAnalyzer->isCAorPanama(&loc));
  }

  void testIsCAorPanama_CAandPanama()
  {
    Loc loc;

    loc.subarea() = IATA_SUB_AREA_13();
    loc.nation() = NATION_PANAMA;
    CPPUNIT_ASSERT_EQUAL(true, _baggageItinAnalyzer->isCAorPanama(&loc));
  }

  void testIsCAorPanama_Other()
  {
    Loc loc;

    loc.subarea() = IATA_SUB_AREA_12();
    loc.nation() = NATION_FRANCE;
    CPPUNIT_ASSERT_EQUAL(false, _baggageItinAnalyzer->isCAorPanama(&loc));
  }

  void testIsWholeTravelInCAOrPanama_BothPanama()
  {
    std::vector<TravelSeg*> travelSegs;

    TravelSeg* ts1 = AirSegBuilder(&_memHandle).withNations("PA", "PA").build();
    TravelSeg* ts2 = AirSegBuilder(&_memHandle).withNations("PA", "PA").build();

    travelSegs += ts1, ts2;

    CPPUNIT_ASSERT_EQUAL(true, _baggageItinAnalyzer->isWholeTravelInCAOrPanama(travelSegs));
  }

  void testIsWholeTravelInCAOrPanama_OnePanamaOneCA()
  {
    std::vector<TravelSeg*> travelSegs;

    TravelSeg* ts1 = AirSegBuilder(&_memHandle).withNations("PA", "PA").build();
    TravelSeg* ts2 = AirSegBuilder(&_memHandle).withNations("US", "US").build();

    travelSegs += ts1, ts2;

    CPPUNIT_ASSERT_EQUAL(false, _baggageItinAnalyzer->isWholeTravelInCAOrPanama(travelSegs));
  }

  void testIsWholeTravelInCAOrPanama_NonePanamaOrCA()
  {
    std::vector<TravelSeg*> travelSegs;

    TravelSeg* ts1 = AirSegBuilder(&_memHandle).withNations("US", "US").build();
    TravelSeg* ts2 = AirSegBuilder(&_memHandle).withNations("US", "US").build();

    travelSegs += ts1, ts2;

    CPPUNIT_ASSERT_EQUAL(false, _baggageItinAnalyzer->isWholeTravelInCAOrPanama(travelSegs));
  }

  void testDetermineBaggageTravelsForUsDot_NoCheckedPoints()
  {
    _baggageItinAnalyzer->_checkedPoints.clear();

    _baggageItinAnalyzer->determineBaggageTravelsForUsDot();
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().empty());
  }

  void testDetermineBaggageTravelsForUsDot_OneBaggageTravel()
  {
    addAirSegsToItin(2);
    _baggageItinAnalyzer->_checkedPoints.push_back(
        CheckedPoint(_itin->travelSeg().end() - 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_furthestCheckedPoint.first = _itin->travelSeg().end() - 1;
    _baggageItinAnalyzer->_furthestCheckedPoint.second = CP_AT_DESTINATION;

    _baggageItinAnalyzer->determineBaggageTravelsForUsDot();
    TravelSegPtrVecCI dummyIter;
    verifyUsDotBaggageTravel(_baggageItinAnalyzer->baggageTravels(),
                             _itin->travelSeg().begin(),
                             _itin->travelSeg().end(),
                             dummyIter,
                             dummyIter,
                             1);
  }

  void testDetermineBaggageTravelsForUsDot_TwoBaggageTravels()
  {
    addAirSegsToItin(3);
    _baggageItinAnalyzer->_checkedPoints.push_back(
        CheckedPoint(_itin->travelSeg().begin() + 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_furthestCheckedPoint.first = _itin->travelSeg().begin() + 1;
    _baggageItinAnalyzer->_furthestCheckedPoint.second = CP_AT_DESTINATION;

    _baggageItinAnalyzer->determineBaggageTravelsForUsDot();
    verifyUsDotBaggageTravel(_baggageItinAnalyzer->baggageTravels(),
                             _itin->travelSeg().begin(),
                             (_itin->travelSeg().begin() + 2),
                             (_itin->travelSeg().begin() + 2),
                             _itin->travelSeg().end());
  }

  void testDetermineBaggageTravelsForUsDot_AdjustOutbound()
  {
    addAirSegsToItin(4, 2);
    _baggageItinAnalyzer->_checkedPoints.push_back(
        CheckedPoint(_itin->travelSeg().begin() + 2, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_furthestCheckedPoint.first = _itin->travelSeg().begin() + 2;
    _baggageItinAnalyzer->_furthestCheckedPoint.second = CP_AT_DESTINATION;

    _baggageItinAnalyzer->determineBaggageTravelsForUsDot();
    verifyUsDotBaggageTravel(_baggageItinAnalyzer->baggageTravels(),
                             _itin->travelSeg().begin(),
                             (_itin->travelSeg().begin() + 2),
                             (_itin->travelSeg().begin() + 3),
                             _itin->travelSeg().end());
  }

  void testDetermineBaggageTravelsForUsDot_AdjustInbound()
  {
    addAirSegsToItin(4, 2);
    _baggageItinAnalyzer->_checkedPoints.push_back(
        CheckedPoint(_itin->travelSeg().begin() + 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_furthestCheckedPoint.first = _itin->travelSeg().begin() + 1;
    _baggageItinAnalyzer->_furthestCheckedPoint.second = CP_AT_DESTINATION;

    _baggageItinAnalyzer->determineBaggageTravelsForUsDot();
    verifyUsDotBaggageTravel(_baggageItinAnalyzer->baggageTravels(),
                             _itin->travelSeg().begin(),
                             (_itin->travelSeg().begin() + 2),
                             (_itin->travelSeg().begin() + 3),
                             _itin->travelSeg().end());
  }

  void testDetermineBaggageTravelsForUsDot_furthestCheckedPointAtOrigin()
  {
    addAirSegsToItin(3);
    _baggageItinAnalyzer->_checkedPoints.push_back(
        CheckedPoint(_itin->travelSeg().begin() + 2, CP_AT_ORIGIN));
    _baggageItinAnalyzer->_furthestCheckedPoint.first = _itin->travelSeg().begin() + 1;
    _baggageItinAnalyzer->_furthestCheckedPoint.second = CP_AT_ORIGIN;

    _baggageItinAnalyzer->determineBaggageTravelsForUsDot();
    verifyUsDotBaggageTravel(_baggageItinAnalyzer->baggageTravels(),
                             _itin->travelSeg().begin(),
                             _itin->travelSeg().begin() + 1,
                             (_itin->travelSeg().begin() + 1),
                             _itin->travelSeg().end());
  }

  void testDetermineBaggageTravelsForUsDot_AdjustOutboundFPAO()
  {
    addAirSegsToItin(4, 1);
    _baggageItinAnalyzer->_checkedPoints.push_back(
        CheckedPoint(_itin->travelSeg().begin() + 2, CP_AT_ORIGIN));
    _baggageItinAnalyzer->_furthestCheckedPoint.first = _itin->travelSeg().begin() + 2;
    _baggageItinAnalyzer->_furthestCheckedPoint.second = CP_AT_ORIGIN;

    _baggageItinAnalyzer->determineBaggageTravelsForUsDot();
    verifyUsDotBaggageTravel(_baggageItinAnalyzer->baggageTravels(),
                             _itin->travelSeg().begin(),
                             _itin->travelSeg().begin() + 1,
                             (_itin->travelSeg().begin() + 2),
                             _itin->travelSeg().end());
  }

  void testDetermineBaggageTravelsForUsDot_AdjustInboundFPAO()
  {
    addAirSegsToItin(4, 2);
    _baggageItinAnalyzer->_checkedPoints.push_back(
        CheckedPoint(_itin->travelSeg().begin() + 1, CP_AT_ORIGIN));
    _baggageItinAnalyzer->_furthestCheckedPoint.first = _itin->travelSeg().begin() + 2;
    _baggageItinAnalyzer->_furthestCheckedPoint.second = CP_AT_ORIGIN;

    _baggageItinAnalyzer->determineBaggageTravelsForUsDot();
    verifyUsDotBaggageTravel(_baggageItinAnalyzer->baggageTravels(),
                             _itin->travelSeg().begin(),
                             (_itin->travelSeg().begin() + 2),
                             (_itin->travelSeg().end() - 1),
                             _itin->travelSeg().end());
  }

  void testDetermineBaggageTravelsForUsDot_AdjustInboundLastArunk()
  {
    addAirSegsToItin(5, 4);
    CheckedPoint fcp(_itin->travelSeg().begin() + 2, CP_AT_ORIGIN);
    _baggageItinAnalyzer->_furthestCheckedPoint = fcp;
    _baggageItinAnalyzer->_checkedPoints.push_back(fcp);

    _baggageItinAnalyzer->determineBaggageTravelsForUsDot();
    verifyUsDotBaggageTravel(_baggageItinAnalyzer->baggageTravels(),
                             _itin->travelSeg().begin(),
                             _itin->travelSeg().begin() + 2,
                             _itin->travelSeg().begin() + 2,
                             _itin->travelSeg().end() - 1);
  }

  void testDetermineBaggageTravelsForNonUsDot_NoCheckedPoints()
  {
    _baggageItinAnalyzer->determineBaggageTravelsForNonUsDot();
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().empty());
  }

  void testDetermineBaggageTravelsForNonUsDot_OneCheckedPoint()
  {
    _itin->geoTravelType() = GeoTravelType::International;
    std::vector<TravelSeg*>& segs = _itin->travelSeg();
    segs += AirSegBuilder(&_memHandle).withNations("N1", "N2").build(),
        AirSegBuilder(&_memHandle).withNations("N2", "N3").build();
    _baggageItinAnalyzer->_stopOverLength = 3600 * 24;
    _baggageItinAnalyzer->determineBaggageTravelsForNonUsDot();
    CPPUNIT_ASSERT_EQUAL((size_t)1, _baggageItinAnalyzer->baggageTravels().size());
  }

  void testInsertFurthestTicketedPointAsCP()
  {
    addAirSegsToItin(3);
    CheckedPointVector& cpvec = _baggageItinAnalyzer->_checkedPoints;
    const std::vector<TravelSeg*> segs = _itin->travelSeg();
    _baggageItinAnalyzer->_checkedPoints +=
        CheckedPoint(segs.begin(), CP_AT_DESTINATION),
        CheckedPoint(segs.begin() + 1, CP_AT_ORIGIN),
        CheckedPoint(segs.begin() + 2, CP_AT_ORIGIN);
    _baggageItinAnalyzer->_furthestTicketedPointIter = segs.begin() + 1;
    _baggageItinAnalyzer->insertFurthestTicketedPointAsCheckedPoint();
    CPPUNIT_ASSERT_EQUAL(size_t(4), cpvec.size());
    CPPUNIT_ASSERT(segs.begin() + 1 == cpvec[2].first);
    CPPUNIT_ASSERT_EQUAL(CP_AT_DESTINATION, cpvec[2].second);
  }

  void testCloneBaggageTravelsForAllFarePaths_EmptyFarePathsAndBaggageTravels()
  {
    _itin->farePath().clear();

    _baggageItinAnalyzer->cloneBaggageTravelsForAllFarePaths();
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().empty());
  }

  void testCloneBaggageTravelsForAllFarePaths_EmptyFarePathsOneBaggageTravel()
  {
    _itin->farePath().clear();
    addBaggageTravel();

    _baggageItinAnalyzer->cloneBaggageTravelsForAllFarePaths();
    CPPUNIT_ASSERT_EQUAL((size_t)1, _baggageItinAnalyzer->baggageTravels().size());
  }

  void testCloneBaggageTravelsForAllFarePaths_OneFarePathEmptyBaggageTravels()
  {
    _baggageItinAnalyzer->cloneBaggageTravelsForAllFarePaths();
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().empty());
  }

  void testCloneBaggageTravelsForAllFarePaths_OneFarePathOneBaggageTravel()
  {
    addBaggageTravel();

    _baggageItinAnalyzer->cloneBaggageTravelsForAllFarePaths();
    CPPUNIT_ASSERT_EQUAL((size_t)1, _baggageItinAnalyzer->baggageTravels().size());
  }

  void testCloneBaggageTravelsForAllFarePaths_MultipleFarePathsEmptyBaggageTravels()
  {
    addFarePathToItin();
    addFarePathToItin();

    _baggageItinAnalyzer->cloneBaggageTravelsForAllFarePaths();
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().empty());
  }

  void testCloneBaggageTravelsForAllFarePaths_MultipleFarePathsOneBaggageTravel()
  {
    addFarePathToItin();
    addFarePathToItin();
    addBaggageTravel();
    size_t expectedVecSize =
        _baggageItinAnalyzer->baggageTravels().size() * _itin->farePath().size();

    _baggageItinAnalyzer->cloneBaggageTravelsForAllFarePaths();
    CPPUNIT_ASSERT_EQUAL(expectedVecSize, _baggageItinAnalyzer->baggageTravels().size());
    verifyClonedBaggageTravels(_baggageItinAnalyzer->baggageTravels(), _itin->farePath());
  }

  void testCloneBaggageTravelsForAllFarePaths_MultipleFarePathsMultipleBaggageTravel()
  {
    addFarePathToItin();
    addFarePathToItin();
    addFarePathToItin();
    addBaggageTravel();
    addBaggageTravel();
    addBaggageTravel();
    size_t expectedVecSize =
        _baggageItinAnalyzer->baggageTravels().size() * _itin->farePath().size();

    _baggageItinAnalyzer->cloneBaggageTravelsForAllFarePaths();
    CPPUNIT_ASSERT_EQUAL(expectedVecSize, _baggageItinAnalyzer->baggageTravels().size());
    verifyClonedBaggageTravels(_baggageItinAnalyzer->baggageTravels(), _itin->farePath());
  }

  void testSetUsDot_WholeTravelInUS_NewMethod()
  {
    _trx->ticketingDate() = DateTime(2051, 1, 1);

    addAirSegsToItin(2);
    _baggageItinAnalyzer->_wholeTravelInUS = true;
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().begin(), CP_AT_ORIGIN));
    _baggageItinAnalyzer->_furthestCheckedPoint = *(_baggageItinAnalyzer->_checkedPoints.begin());
    _baggageItinAnalyzer->setUsDot();
    CPPUNIT_ASSERT_EQUAL(true, _itin->getBaggageTripType().isUsDot());
  }

  void testSetUsDot_WholeTravelInUS_OldMethod()
  {
    addAirSegsToItin(2);
    _baggageItinAnalyzer->_wholeTravelInUS = true;
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().begin(), CP_AT_ORIGIN));
    _baggageItinAnalyzer->_furthestCheckedPoint = *(_baggageItinAnalyzer->_checkedPoints.begin());
    _baggageItinAnalyzer->setUsDot();
    CPPUNIT_ASSERT_EQUAL(false, _itin->getBaggageTripType().isUsDot());
  }

  void testSetUsDot_FirstPointInUS()
  {
    TravelSeg* ts1 = AirSegBuilder(&_memHandle).withNations("US", "PL").build();
    TravelSeg* ts2 = AirSegBuilder(&_memHandle).withNations("PL", "PL").build();

    _itin->travelSeg() += ts1, ts2;
    _baggageItinAnalyzer->_wholeTravelInUS = false;
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().begin(), CP_AT_ORIGIN));
    _baggageItinAnalyzer->_furthestCheckedPoint = *(_baggageItinAnalyzer->_checkedPoints.begin());
    _baggageItinAnalyzer->setUsDot();
    CPPUNIT_ASSERT_EQUAL(true, _itin->getBaggageTripType().isUsDot());
  }

  void testSetUsDot_LastPointInUS()
  {
    TravelSeg* ts1 = AirSegBuilder(&_memHandle).withNations("PL", "PL").build();
    TravelSeg* ts2 = AirSegBuilder(&_memHandle).withNations("PL", "US").build();

    _itin->travelSeg() += ts1, ts2;
    _baggageItinAnalyzer->_wholeTravelInUS = false;
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().begin(), CP_AT_ORIGIN));
    _baggageItinAnalyzer->_furthestCheckedPoint = *(_baggageItinAnalyzer->_checkedPoints.begin());
    _baggageItinAnalyzer->setUsDot();
    CPPUNIT_ASSERT_EQUAL(true, _itin->getBaggageTripType().isUsDot());
  }

  void testSetBaggageTripType_WhollyInUS()
  {
    _baggageItinAnalyzer->_wholeTravelInUS = true;

    _baggageItinAnalyzer->setBaggageTripType();
    CPPUNIT_ASSERT_EQUAL(BaggageTripType::WHOLLY_WITHIN_US,
                         _itin->getBaggageTripType().getRawValue());
  }

  void testSetBaggageTripType_WhollyInCA()
  {
    _itin->tripCharacteristics().set(Itin::CanadaOnly);

    _baggageItinAnalyzer->setBaggageTripType();
    CPPUNIT_ASSERT_EQUAL(BaggageTripType::WHOLLY_WITHIN_CA,
                         _itin->getBaggageTripType().getRawValue());
  }

  void testSetBaggageTripType_BetweenUSCA()
  {
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("US", "CA").build();
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("CA", "PL").build();
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().begin(), CP_AT_DESTINATION));
    _baggageItinAnalyzer->_furthestCheckedPoint = _baggageItinAnalyzer->_checkedPoints.front();

    _baggageItinAnalyzer->setBaggageTripType();
    CPPUNIT_ASSERT_EQUAL(BaggageTripType::BETWEEN_US_CA, _itin->getBaggageTripType().getRawValue());
  }

  void testSetBaggageTripType_ToFromUS()
  {
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("PL", "CA").build();
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("CA", "AS").build();
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("AS", "FR").build();
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().begin() + 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_furthestCheckedPoint = _baggageItinAnalyzer->_checkedPoints.front();

    _baggageItinAnalyzer->setBaggageTripType();
    CPPUNIT_ASSERT_EQUAL(BaggageTripType::TO_FROM_US, _itin->getBaggageTripType().getRawValue());
  }

  void testSetBaggageTripType_ToFromCA_StopoverUS()
  {
    DateTime date = DateTime(2015, 1, 1);
    DateTime future = date.addDays(2);
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("PL", "US").
        withDepartureDT(date).withArrivalDT(date).build();
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("US", "FR").
        withDepartureDT(future).withArrivalDT(future).build();
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("FR", "CA").
        withDepartureDT(future).withArrivalDT(future).build();
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().begin() + 1, CP_AT_DESTINATION));

    _baggageItinAnalyzer->_furthestCheckedPoint = _baggageItinAnalyzer->_checkedPoints.front();

    _baggageItinAnalyzer->setBaggageTripType();
    CPPUNIT_ASSERT_EQUAL(BaggageTripType::BETWEEN_US_CA, _itin->getBaggageTripType().getRawValue());
  }

  void testSetBaggageTripType_ToFromCA()
  {
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("PL", "US").build();
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("US", "FR").build();
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("FR", "CA").build();
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().begin() + 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_furthestCheckedPoint = _baggageItinAnalyzer->_checkedPoints.front();

    _baggageItinAnalyzer->setBaggageTripType();
    CPPUNIT_ASSERT_EQUAL(BaggageTripType::TO_FROM_CA, _itin->getBaggageTripType().getRawValue());
  }

  void testSetBaggageTripType_Other()
  {
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("PL", "FR").build();
    _itin->travelSeg() += AirSegBuilder(&_memHandle).withNations("FR", "UK").build();
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().begin(), CP_AT_DESTINATION));
    _baggageItinAnalyzer->_furthestCheckedPoint = _baggageItinAnalyzer->_checkedPoints.front();

    _baggageItinAnalyzer->setBaggageTripType();
    CPPUNIT_ASSERT_EQUAL(BaggageTripType::OTHER, _itin->getBaggageTripType().getRawValue());
  }

  void testDetermineFurthestCheckedPoint_OneTravelSegment()
  {
    TravelSeg* ts1 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_SVOCDG.xml");

    CPPUNIT_ASSERT(ts1);

    _itin->travelSeg() += ts1;

    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->determineFurthestCheckedPoint();

    CPPUNIT_ASSERT(_baggageItinAnalyzer->_itin.travelSeg().end() - 1 ==
                   _baggageItinAnalyzer->_furthestCheckedPoint.first);
  }

  void testDetermineFurthestCheckedPoint_OneCheckedPoint()
  {
    TravelSeg* ts1 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_SVOCDG.xml");
    TravelSeg* ts2 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_ORYNCE.xml");

    CPPUNIT_ASSERT(ts1);
    CPPUNIT_ASSERT(ts2);

    _itin->travelSeg() += ts1, ts2;

    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->determineFurthestCheckedPoint();

    CPPUNIT_ASSERT(_baggageItinAnalyzer->_itin.travelSeg().end() - 1 ==
                   _baggageItinAnalyzer->_furthestCheckedPoint.first);
  }

  void testDetermineFurthestCheckedPoint_TwoCheckedPoints()
  {
    TravelSeg* ts1 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_SVOCDG.xml");
    TravelSeg* ts2 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_ORYNCE.xml");
    TravelSeg* ts3 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_NCEJFK.xml");

    CPPUNIT_ASSERT(ts1);
    CPPUNIT_ASSERT(ts2);
    CPPUNIT_ASSERT(ts3);

    _itin->travelSeg() += ts1, ts2, ts3;

    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 2, CP_AT_DESTINATION));
    _baggageItinAnalyzer->determineFurthestCheckedPoint();

    CPPUNIT_ASSERT(_baggageItinAnalyzer->_itin.travelSeg().end() - 2 ==
                   _baggageItinAnalyzer->_furthestCheckedPoint.first);
  }

  void testDetermineFurthestCheckedPoint_Retransit()
  {
    TravelSeg* ts1 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_SVOCDG.xml");
    TravelSeg* ts2 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_CDGSVO.xml");
    TravelSeg* ts3 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_SVOCDG.xml");

    _itin->travelSeg() += ts1, ts2, ts3;
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 2, CP_AT_DESTINATION));
    _baggageItinAnalyzer->determineFurthestCheckedPoint();

    CPPUNIT_ASSERT(_baggageItinAnalyzer->_itin.travelSeg().end() - 1 ==
                   _baggageItinAnalyzer->_furthestCheckedPoint.first);
  }

  void testDetermineMss_SingleSegmentSameCountry()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, UNITED_STATES)
                      .build();

    CPPUNIT_ASSERT(travelSegs.begin() ==
                   _baggageItinAnalyzer->determineMss(travelSegs.begin(), travelSegs.end()));
  }

  void testDetermineMss_AirAndSurfaceSegments()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, UNITED_STATES)
                      .build();

    travelSegs += _memHandle.create<SurfaceSeg>();

    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, CANADA)
                      .build();

    CPPUNIT_ASSERT(travelSegs.end() - 1 ==
                   _baggageItinAnalyzer->determineMss(travelSegs.begin(), travelSegs.end()));
  }

  void testDetermineMss_AreaChanged()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_14())
                      .withNations(UNITED_STATES, COLUMBIA)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA3)
                      .withSubArea(IATA_SUB_AREA_14(), IATA_SUB_ARE_33())
                      .withNations(COLUMBIA, JAPAN)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA3, IATA_AREA2)
                      .withSubArea(IATA_SUB_ARE_33(), IATA_SUB_AREA_21())
                      .withNations(JAPAN, FRANCE)
                      .build();

    CPPUNIT_ASSERT(travelSegs.begin() + 1 ==
                   _baggageItinAnalyzer->determineMss(travelSegs.begin(), travelSegs.end()));
  }

  void testDetermineMss_AreaChanged312()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA3, IATA_AREA1)
                      .withSubArea(IATA_SUB_ARE_33(), IATA_SUB_AREA_11())
                      .withNations(JAPAN, UNITED_STATES)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_21())
                      .withNations(UNITED_STATES, FRANCE)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA3)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_ARE_33())
                      .withNations(FRANCE, JAPAN)
                      .build();

    CPPUNIT_ASSERT(travelSegs.begin() ==
                   _baggageItinAnalyzer->determineMss(travelSegs.begin(), travelSegs.end()));
  }

  void testDetermineMss_AreaChanged321()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA3, IATA_AREA2)
                      .withSubArea(IATA_SUB_ARE_33(), IATA_SUB_AREA_21())
                      .withNations(JAPAN, FRANCE)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_11())
                      .withNations(FRANCE, UNITED_STATES)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA3)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_ARE_33())
                      .withNations(UNITED_STATES, JAPAN)
                      .build();

    CPPUNIT_ASSERT(travelSegs.begin() + 1 ==
                   _baggageItinAnalyzer->determineMss(travelSegs.begin(), travelSegs.end()));
  }

  void testDetermineMss_AreaChanged123()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_21())
                      .withNations(UNITED_STATES, FRANCE)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA3)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_ARE_33())
                      .withNations(FRANCE, JAPAN)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA3, IATA_AREA1)
                      .withSubArea(IATA_SUB_ARE_33(), IATA_SUB_AREA_11())
                      .withNations(JAPAN, UNITED_STATES)
                      .build();

    CPPUNIT_ASSERT(travelSegs.begin() ==
                   _baggageItinAnalyzer->determineMss(travelSegs.begin(), travelSegs.end()));
  }

  void testDetermineMss_SubAreaChanged()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, UNITED_STATES)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_14())
                      .withNations(UNITED_STATES, COLUMBIA)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_14(), IATA_SUB_AREA_13())
                      .withNations(COLUMBIA, MEXICO)
                      .build();

    CPPUNIT_ASSERT(travelSegs.begin() + 1 ==
                   _baggageItinAnalyzer->determineMss(travelSegs.begin(), travelSegs.end()));
  }

  void testDetermineMss_NationChanged()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(GERMANY, GERMANY)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(GERMANY, FRANCE)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(FRANCE, AUSTRIA)
                      .build();

    CPPUNIT_ASSERT(travelSegs.begin() + 1 ==
                   _baggageItinAnalyzer->determineMss(travelSegs.begin(), travelSegs.end()));
  }

  void testDetermineMss_SameCountry()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(GERMANY, GERMANY)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(GERMANY, GERMANY)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(GERMANY, GERMANY)
                      .build();

    CPPUNIT_ASSERT(travelSegs.begin() ==
                   _baggageItinAnalyzer->determineMss(travelSegs.begin(), travelSegs.end()));
  }

  void testDetermineMssForNonUsDot_OneBaggageTravel()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(GERMANY, GERMANY)
                      .build();
    addBaggageTravel(travelSegs.begin(), travelSegs.end());

    _baggageItinAnalyzer->determineMssForNonUsDot();
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().front()->_MSS == travelSegs.begin());
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().front()->_MSSJourney ==
                   travelSegs.begin());
  }

  void testDetermineMssForNonUsDot_MultipleBaggageTravels()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(GERMANY, SPAIN)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(SPAIN, FRANCE)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA2, IATA_AREA2)
                      .withSubArea(IATA_SUB_AREA_21(), IATA_SUB_AREA_21())
                      .withNations(FRANCE, AUSTRIA)
                      .build();

    addBaggageTravel(travelSegs.begin(), travelSegs.begin() + 1);
    addBaggageTravel(travelSegs.begin() + 1, travelSegs.begin() + 2);
    addBaggageTravel(travelSegs.begin() + 2, travelSegs.end());

    _baggageItinAnalyzer->determineMssForNonUsDot();
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().at(0)->_MSS == travelSegs.begin());
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().at(0)->_MSSJourney == travelSegs.begin());
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().at(1)->_MSS == travelSegs.begin() + 1);
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().at(1)->_MSSJourney ==
                   travelSegs.begin() + 1);
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().at(2)->_MSS == travelSegs.begin() + 2);
    CPPUNIT_ASSERT(_baggageItinAnalyzer->baggageTravels().at(2)->_MSSJourney ==
                   travelSegs.begin() + 2);
  }

  void testDetermineMssForUsDot_OneBaggageTravel()
  {
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, UNITED_STATES)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, CANADA)
                      .build();

    BaggageItinAnalyzer baggageItinAnalyzer(*_trx, *_itin);
    addBaggageTravel(travelSegs.begin(), travelSegs.end(), &baggageItinAnalyzer);

    baggageItinAnalyzer.determineMssForUsDot();
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().front()->_MSS == travelSegs.begin() + 1);
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().front()->_MSSJourney ==
                   travelSegs.begin() + 1);
  }

  void testDetermineMssForUsDot_OneBaggageTravel_WhollyWithinUs()
  {
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, UNITED_STATES)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, GUAM)
                      .build();

    _itin->setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_US);

    BaggageItinAnalyzer baggageItinAnalyzer(*_trx, *_itin);
    addBaggageTravel(travelSegs.begin(), travelSegs.end(), &baggageItinAnalyzer);

    baggageItinAnalyzer.determineMssForUsDot();
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().front()->_MSS == travelSegs.begin() + 1);
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().front()->_MSSJourney == travelSegs.begin());
  }

  void testDetermineMssForUsDot_TwoBaggageTravels()
  {
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, UNITED_STATES)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, CANADA)
                      .build();

    BaggageItinAnalyzer baggageItinAnalyzer(*_trx, *_itin);
    addBaggageTravel(travelSegs.begin(), travelSegs.begin() + 1, &baggageItinAnalyzer);
    addBaggageTravel(travelSegs.begin() + 1, travelSegs.end(), &baggageItinAnalyzer);

    baggageItinAnalyzer.determineMssForUsDot();
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(0)->_MSS == travelSegs.begin());
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(0)->_MSSJourney ==
                   travelSegs.begin() + 1);
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(1)->_MSS == travelSegs.begin() + 1);
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(1)->_MSSJourney ==
                   travelSegs.begin() + 1);
  }

  void testDetermineMssForUsDot_TwoBaggageTravels_WhollyWithinUs()
  {
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, UNITED_STATES)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(UNITED_STATES, GUAM)
                      .build();

    BaggageItinAnalyzer baggageItinAnalyzer(*_trx, *_itin);
    addBaggageTravel(travelSegs.begin(), travelSegs.begin() + 1, &baggageItinAnalyzer);
    addBaggageTravel(travelSegs.begin() + 1, travelSegs.end(), &baggageItinAnalyzer);

    _itin->setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_US);

    baggageItinAnalyzer.determineMssForUsDot();
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(0)->_MSS == travelSegs.begin());
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(0)->_MSSJourney == travelSegs.begin());
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(1)->_MSS == travelSegs.begin() + 1);
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(1)->_MSSJourney == travelSegs.begin());
  }

  void testDetermineMssForUsDot_TwoBaggageTravels_WhollyWithinCa()
  {
    std::vector<TravelSeg*>& travelSegs = _itin->travelSeg();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(CANADA, CANADA)
                      .build();
    travelSegs += AirSegBuilder(&_memHandle)
                      .withArea(IATA_AREA1, IATA_AREA1)
                      .withSubArea(IATA_SUB_AREA_11(), IATA_SUB_AREA_11())
                      .withNations(CANADA, CANADA)
                      .build();

    BaggageItinAnalyzer baggageItinAnalyzer(*_trx, *_itin);
    addBaggageTravel(travelSegs.begin(), travelSegs.begin() + 1, &baggageItinAnalyzer);
    addBaggageTravel(travelSegs.begin() + 1, travelSegs.end(), &baggageItinAnalyzer);

    _itin->setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_CA);

    baggageItinAnalyzer.determineMssForUsDot();
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(0)->_MSS == travelSegs.begin());
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(0)->_MSSJourney == travelSegs.begin());
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(1)->_MSS == travelSegs.begin() + 1);
    CPPUNIT_ASSERT(baggageItinAnalyzer.baggageTravels().at(1)->_MSSJourney == travelSegs.begin());
  }

  void testIsDestinationOnlyCheckedPoint()
  {
    TravelSeg* ts1 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_SVOCDG.xml");
    TravelSeg* ts2 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_ORYNCE.xml");

    CPPUNIT_ASSERT(ts1);
    CPPUNIT_ASSERT(ts2);

    _itin->travelSeg() += ts1, ts2;

    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 1, CP_AT_DESTINATION));

    CPPUNIT_ASSERT_EQUAL(true, _baggageItinAnalyzer->isDestinationOnlyCheckedPoint());
  }

  void testIsDestinationOnlyCheckedPoint_NotOnly()
  {
    TravelSeg* ts1 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_SVOCDG.xml");
    TravelSeg* ts2 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_ORYNCE.xml");

    CPPUNIT_ASSERT(ts1);
    CPPUNIT_ASSERT(ts2);

    _itin->travelSeg() += ts1, ts2;

    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 2, CP_AT_DESTINATION));

    CPPUNIT_ASSERT_EQUAL(false, _baggageItinAnalyzer->isDestinationOnlyCheckedPoint());
  }

  void testIsDestinationOnlyCheckedPoint_NotOnlyButSame()
  {
    TravelSeg* ts1 = TestAirSegFactory::create(std::string(UT_DATA_PATH) + "AirSeg_SVOCDG.xml");

    CPPUNIT_ASSERT(ts1);

    _itin->travelSeg() += ts1;

    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 1, CP_AT_DESTINATION));
    _baggageItinAnalyzer->_checkedPoints.push_back(
        std::make_pair(_itin->travelSeg().end() - 1, CP_AT_DESTINATION));

    CPPUNIT_ASSERT_EQUAL(true, _baggageItinAnalyzer->isDestinationOnlyCheckedPoint());
  }

  void testGetMileage_Mpm()
  {
    Loc loc1, loc2;
    loc1.loc() = "MPM";
    CPPUNIT_ASSERT_EQUAL(100u, _baggageItinAnalyzer->getMileage(loc1, loc2, {GlobalDirection::ZZ}));
  }

  void testGetMileage_Tpm()
  {
    Loc loc1, loc2;
    loc1.loc() = "TPM";
    CPPUNIT_ASSERT_EQUAL(200u, _baggageItinAnalyzer->getMileage(loc1, loc2, {GlobalDirection::ZZ}));
  }

  void testGetMileage_Circle()
  {
    Loc loc1, loc2;
    loc1.loc() = "CIR";
    loc1.lathem() = 'N';
    loc1.latdeg() = 70;
    loc1.lnghem() = 'W';
    loc1.lngdeg() = 70;
    loc2.lathem() = 'N';
    loc2.latdeg() = 71;
    loc2.lnghem() = 'W';
    loc2.lngdeg() = 82;
    CPPUNIT_ASSERT_EQUAL(285u, _baggageItinAnalyzer->getMileage(loc1, loc2, {GlobalDirection::ZZ}));
  }

  void testGetMileage_SingleMatchGD()
  {
    setMileage({createMileage(GlobalDirection::PA, 100), createMileage(GlobalDirection::AT, 200)});
    CPPUNIT_ASSERT_EQUAL(100u,
                         _baggageItinAnalyzer->getMileage("AAA", "BBB", {GlobalDirection::PA}, TPM));
  }

  void testGetMileage_NoMatchGD()
  {
    setMileage({createMileage(GlobalDirection::PA, 100), createMileage(GlobalDirection::AT, 200)});
    CPPUNIT_ASSERT_EQUAL(200u,
                         _baggageItinAnalyzer->getMileage("AAA", "BBB", {GlobalDirection::ZZ}, TPM));
  }

  void testGetMileage_ManyMatchesGD()
  {
    setMileage({createMileage(GlobalDirection::PA, 100),
                createMileage(GlobalDirection::AT, 200),
                createMileage(GlobalDirection::EH, 300)});
    CPPUNIT_ASSERT_EQUAL(200u,
                         _baggageItinAnalyzer->getMileage(
                             "AAA", "BBB", {GlobalDirection::PA, GlobalDirection::AT}, TPM));
  }

  void testPrintMileage()
  {
    createDiagnostic();
    const std::vector<Mileage*> vec;
    _trx->diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DISPLAY_DETAIL, "GI"));
    _baggageItinAnalyzer->printMileage("AAA", "BBB", vec, {GlobalDirection::ZZ}, 'M');

    CPPUNIT_ASSERT(_baggageItinAnalyzer->_diag852->str().find(
                       "MPM ANALYSIS FOR CITY PAIR AAA BBB") != std::string::npos);
  }

  void testPrintItinAnalysisResults()
  {
    createDiagnostic();
    _baggageItinAnalyzer->printItinAnalysisResults();

    CPPUNIT_ASSERT(_baggageItinAnalyzer->_diag852->rootDiag()->toString().find(
                       "ITINERARY ANALYSIS") != std::string::npos);
  }

  void testPrintBaggageTravels()
  {
    addBaggageTravel();
    _itin->farePath().front()->baggageTravels().push_back(
        _baggageItinAnalyzer->_baggageTravels.front());
    createDiagnostic();

    _baggageItinAnalyzer->printBaggageTravels();
    CPPUNIT_ASSERT(_baggageItinAnalyzer->_diag852->rootDiag()->toString().find("CHECKED PORTION") !=
                   std::string::npos);
  }

  void setupRemoveBaggageTravels()
  {
    _trx->setTrxType(PricingTrx::MIP_TRX);
    _req->originBasedRTPricing() = true;
    addAirSegsToItin(2);
    addBaggageTravel(_itin->travelSeg().begin(), _itin->travelSeg().begin() + 1);
    addBaggageTravel(_itin->travelSeg().begin() + 1, _itin->travelSeg().end());
  }

  void testRemoveDummyBaggageTravels()
  {
    setupRemoveBaggageTravels();
    _itin->travelSeg().front()->toAirSeg()->makeFake();
    _baggageItinAnalyzer->removeDummyBaggageTravels();
    CPPUNIT_ASSERT_EQUAL(size_t(1u), _baggageItinAnalyzer->baggageTravels().size());
  }

  void testRemoveDummyBaggageTravels_NoOriginBasedRT()
  {
    setupRemoveBaggageTravels();
    _itin->travelSeg().front()->toAirSeg()->makeFake();
    _req->originBasedRTPricing() = false;
    _baggageItinAnalyzer->removeDummyBaggageTravels();
    CPPUNIT_ASSERT_EQUAL(size_t(2u), _baggageItinAnalyzer->baggageTravels().size());
  }

  void testRemoveDummyBaggageTravels_NoMip()
  {
    setupRemoveBaggageTravels();
    _itin->travelSeg().front()->toAirSeg()->makeFake();
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    _baggageItinAnalyzer->removeDummyBaggageTravels();
    CPPUNIT_ASSERT_EQUAL(size_t(2u), _baggageItinAnalyzer->baggageTravels().size());
  }

  void testRemoveDummyBaggageTravels_NoDummy()
  {
    setupRemoveBaggageTravels();
    _baggageItinAnalyzer->removeDummyBaggageTravels();
    CPPUNIT_ASSERT_EQUAL(size_t(2u), _baggageItinAnalyzer->baggageTravels().size());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BaggageItinAnalyzerTest);
} // tse
