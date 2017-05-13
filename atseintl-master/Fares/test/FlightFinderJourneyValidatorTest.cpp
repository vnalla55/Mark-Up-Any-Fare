#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "Fares/FlightFinderJourneyValidator.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class MockFlightFinderJourneyValidator : public FlightFinderJourneyValidator
{
public:
  MockFlightFinderJourneyValidator() : FlightFinderJourneyValidator(0) {};
  virtual ~MockFlightFinderJourneyValidator() {};

  void initialize(FlightFinderTrx* fFTrx) { _fFTrx = fFTrx; }
  FlightFinderTrx* fFTrx() { return _fFTrx; }

  using FlightFinderJourneyValidator::createPricingUnit;
  using FlightFinderJourneyValidator::prepareSOPDataVect;
  std::map<DatePair, FlightFinderTrx::FlightBitInfo>*& dataPairsMap() { return _dataPairsMap; }
  FlightFinderTrx::OutBoundDateFlightMap& outboundDateflightMap() { return _outboundDateflightMap; }
  std::vector<TravelSeg*>& fareMarketSegBackup() { return _fareMarketSegBackup; }
  using FlightFinderJourneyValidator::skipFarePath;
  using FlightFinderJourneyValidator::storeFarePathStatusForSkip;
};

class FlightFinderJourneyValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FlightFinderJourneyValidatorTest);
  CPPUNIT_TEST(testCreatePricingUnit_RT);
  CPPUNIT_TEST(testCreatePricingUnit_OW);
  CPPUNIT_TEST(testPrepareSOPDataVect_EmptyOutboundDateFlightMap);
  CPPUNIT_TEST(testPrepareSOPDataVect_OneWayOutboundDateFlightMap);
  CPPUNIT_TEST(testPrepareSOPDataVect_RoundTripOutboundDateFlightMap);
  CPPUNIT_TEST(testPrepareSOPDataVect_OneWay_CustomGlobalDirection);
  CPPUNIT_TEST(testPrepareSOPDataVect_RoundTrip_CustomGlobalDirection);
  CPPUNIT_TEST(skipFarePath_Skip_step2);
  CPPUNIT_TEST(skipFarePath_DoNotSkip_step2);
  CPPUNIT_TEST(skipFarePath_Skip_step4);
  CPPUNIT_TEST(skipFarePath_DoNotSkip_step4);
  CPPUNIT_TEST(storeFarePathStatusForSkip_step2);
  CPPUNIT_TEST(storeFarePathStatusForSkip_step4);
  CPPUNIT_TEST(storeFarePathStatusForSkip_InboundIsOpen_step4);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  typedef std::map<DateTime, std::vector<uint8_t> > FlightListMapInboundDataType;
  typedef struct
  {
    FlightListMapInboundDataType flightListInboundMapData;
    std::vector<uint8_t> flightList;
  } FlightListMapOutboundElemDataType;
  typedef std::map<DateTime, FlightListMapOutboundElemDataType> FlightListMapOutboundDataType;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _paxTypeFare = FareValidatorOrchestratorTestCommon::buildFakePaxTypeFare(_dataHandle);
    _flightFinderJourneyValidator = _dataHandle.create<MockFlightFinderJourneyValidator>();
    initialize();
  }

  void tearDown()
  {
    _dataHandle.clear();
    _memHandle.clear();
  }

  void testCreatePricingUnit_RT()
  {
    std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> > PUData;
    AirSeg* airSegOut =
        FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "KRK", "FRA", "AA");
    std::vector<TravelSeg*> tvlSegOutVect;
    tvlSegOutVect.push_back(airSegOut);

    PUData += std::make_pair(tvlSegOutVect, _paxTypeFare);

    PricingUnit* pricingUnit = _flightFinderJourneyValidator->createPricingUnit(PUData);

    CPPUNIT_ASSERT_EQUAL(pricingUnit->fareUsage().size(), size_t(1));
    CPPUNIT_ASSERT_EQUAL(pricingUnit->travelSeg().size(), size_t(1));
    CPPUNIT_ASSERT_EQUAL(pricingUnit->puType(), PricingUnit::Type::ONEWAY);
  }

  void testCreatePricingUnit_OW()
  {
    std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> > PUData;
    AirSeg* airSegOut =
        FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "KRK", "FRA", "AA");
    AirSeg* airSegIn =
        FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "FRA", "KRK", "AA");
    std::vector<TravelSeg*> tvlSegOutVect;
    tvlSegOutVect.push_back(airSegOut);
    std::vector<TravelSeg*> tvlSegInVect;
    tvlSegInVect.push_back(airSegIn);

    PUData += std::make_pair(tvlSegOutVect, _paxTypeFare);
    PUData += std::make_pair(tvlSegInVect, _paxTypeFare);

    PricingUnit* pricingUnit = _flightFinderJourneyValidator->createPricingUnit(PUData);

    CPPUNIT_ASSERT_EQUAL(pricingUnit->fareUsage().size(), size_t(2));
    CPPUNIT_ASSERT_EQUAL(pricingUnit->travelSeg().size(), size_t(2));
    CPPUNIT_ASSERT_EQUAL(pricingUnit->puType(), PricingUnit::Type::ROUNDTRIP);
  }

  void testPrepareSOPDataVect_EmptyOutboundDateFlightMap()
  {
    FlightFinderJourneyValidator::FarePathSOPDataVectType farePathSOPDataVect;
    _flightFinderJourneyValidator->prepareSOPDataVect(farePathSOPDataVect);

    CPPUNIT_ASSERT_EQUAL(farePathSOPDataVect.empty(), true);
  }

  void testPrepareSOPDataVect_OneWayOutboundDateFlightMap()
  {
    //---------- OW ----------
    // 19 - 1,2,3
    // 20 - 4,5

    // 5 options
    size_t numberOfOptions = 5;

    FlightListMapOutboundDataType flightListMapOutboundData;
    flightListMapOutboundData[DateTime(2008, 9, 19)].flightList += 1, 2, 3;
    flightListMapOutboundData[DateTime(2008, 9, 20)].flightList += 4, 5;

    fillOutboundDateFlightMap(_flightFinderJourneyValidator->outboundDateflightMap(),
                              flightListMapOutboundData);
    createSchedulingOptionVect(flightListMapOutboundData, false);

    FlightFinderJourneyValidator::FarePathSOPDataVectType farePathSOPDataVect;
    _flightFinderJourneyValidator->prepareSOPDataVect(farePathSOPDataVect);

    CPPUNIT_ASSERT_EQUAL(farePathSOPDataVect.size(), numberOfOptions);
  }

  void testPrepareSOPDataVect_RoundTripOutboundDateFlightMap()
  {
    //---------- RT ----------
    // 19 - 1,2,3
    //	23 - 11,12
    //	24 - 13
    // 20 - 4,5
    //	23 - 11

    // 9 options (<1,open>,<2,open>,<3,open>,<4,open>,<5,open>,<open,11>,...)
    size_t numberOfOptions = 9;

    FlightListMapOutboundDataType flightListMapOutboundData;
    flightListMapOutboundData[DateTime(2008, 9, 19)].flightList += 1, 2, 3;
    flightListMapOutboundData[DateTime(2008, 9, 19)]
        .flightListInboundMapData[DateTime(2008, 9, 23)] += 11,
        12;
    flightListMapOutboundData[DateTime(2008, 9, 19)]
        .flightListInboundMapData[DateTime(2008, 9, 24)] += 13;
    flightListMapOutboundData[DateTime(2008, 9, 20)].flightList += 4, 5;
    flightListMapOutboundData[DateTime(2008, 9, 20)]
        .flightListInboundMapData[DateTime(2008, 9, 23)] += 11;

    fillOutboundDateFlightMap(_flightFinderJourneyValidator->outboundDateflightMap(),
                              flightListMapOutboundData);
    createSchedulingOptionVect(flightListMapOutboundData, true);
    fillFareMarketSegBackup();

    FlightFinderJourneyValidator::FarePathSOPDataVectType farePathSOPDataVect;
    _flightFinderJourneyValidator->prepareSOPDataVect(farePathSOPDataVect);

    CPPUNIT_ASSERT_EQUAL(farePathSOPDataVect.size(), numberOfOptions);
  }

  void testPrepareSOPDataVect_OneWay_CustomGlobalDirection()
  {
    FlightListMapOutboundDataType flightListMapOutboundData;
    flightListMapOutboundData[DateTime(2008, 9, 19)].flightList += 1, 2, 3;
    flightListMapOutboundData[DateTime(2008, 9, 20)].flightList += 4, 5;

    fillOutboundDateFlightMap(_flightFinderJourneyValidator->outboundDateflightMap(),
                              flightListMapOutboundData);
    createSchedulingOptionVect(flightListMapOutboundData, false, GlobalDirection::PA);

    FlightFinderJourneyValidator::FarePathSOPDataVectType farePathSOPDataVect;
    _flightFinderJourneyValidator->prepareSOPDataVect(farePathSOPDataVect);

    for (const FlightFinderJourneyValidator::FarePathSOPDataType& fp : farePathSOPDataVect)
    {
      CPPUNIT_ASSERT_EQUAL(GlobalDirection::PA, fp.globalDirection);
    }
  }

  void testPrepareSOPDataVect_RoundTrip_CustomGlobalDirection()
  {
    FlightListMapOutboundDataType flightListMapOutboundData;
    flightListMapOutboundData[DateTime(2008, 9, 19)].flightList += 1, 2, 3;
    flightListMapOutboundData[DateTime(2008, 9, 19)]
        .flightListInboundMapData[DateTime(2008, 9, 23)] += 11,
        12;
    flightListMapOutboundData[DateTime(2008, 9, 19)]
        .flightListInboundMapData[DateTime(2008, 9, 24)] += 13;
    flightListMapOutboundData[DateTime(2008, 9, 20)].flightList += 4, 5;
    flightListMapOutboundData[DateTime(2008, 9, 20)]
        .flightListInboundMapData[DateTime(2008, 9, 23)] += 11;

    fillOutboundDateFlightMap(_flightFinderJourneyValidator->outboundDateflightMap(),
                              flightListMapOutboundData);
    createSchedulingOptionVect(flightListMapOutboundData, true, GlobalDirection::PN);
    fillFareMarketSegBackup();

    FlightFinderJourneyValidator::FarePathSOPDataVectType farePathSOPDataVect;
    _flightFinderJourneyValidator->prepareSOPDataVect(farePathSOPDataVect);

    for (const FlightFinderJourneyValidator::FarePathSOPDataType& fp : farePathSOPDataVect)
    {
      CPPUNIT_ASSERT_EQUAL(GlobalDirection::PN, fp.globalDirection);
    }
  }

  void skipFarePath_Skip_step2()
  {
    _flightFinderJourneyValidator->fFTrx()->bffStep() = FlightFinderTrx::STEP_2;

    FlightFinderJourneyValidator::FarePathType* farePathData = 0;
    farePathData = buildFarePathData(DateTime(2008, 9, 19), DateTime::emptyDate());

    std::set<DateTime> passedDates;
    passedDates.insert(DateTime(2008, 9, 19).date());
    bool makeSkip = _flightFinderJourneyValidator->skipFarePath(*farePathData, passedDates);

    CPPUNIT_ASSERT(makeSkip);
  }

  void skipFarePath_DoNotSkip_step2()
  {
    _flightFinderJourneyValidator->fFTrx()->bffStep() = FlightFinderTrx::STEP_2;

    FlightFinderJourneyValidator::FarePathType* farePathData = 0;
    farePathData = buildFarePathData(DateTime(2008, 9, 19), DateTime::emptyDate());

    std::set<DateTime> passedDates;
    passedDates.insert(DateTime(2008, 9, 20).date());
    bool makeSkip = _flightFinderJourneyValidator->skipFarePath(*farePathData, passedDates);

    CPPUNIT_ASSERT(!makeSkip);
  }

  void skipFarePath_Skip_step4()
  {
    _flightFinderJourneyValidator->fFTrx()->bffStep() = FlightFinderTrx::STEP_4;

    FlightFinderJourneyValidator::FarePathType* farePathData = 0;
    farePathData = buildFarePathData(DateTime(2008, 9, 19), DateTime(2008, 9, 19));

    std::set<DateTime> passedDates;
    passedDates.insert(DateTime(2008, 9, 19).date());
    bool makeSkip = _flightFinderJourneyValidator->skipFarePath(*farePathData, passedDates);

    CPPUNIT_ASSERT(makeSkip);
  }

  void skipFarePath_DoNotSkip_step4()
  {
    _flightFinderJourneyValidator->fFTrx()->bffStep() = FlightFinderTrx::STEP_4;

    FlightFinderJourneyValidator::FarePathType* farePathData = 0;
    farePathData = buildFarePathData(DateTime(2008, 9, 19), DateTime(2008, 9, 21));

    std::set<DateTime> passedDates;
    passedDates.insert(DateTime(2008, 9, 20).date());
    bool makeSkip = _flightFinderJourneyValidator->skipFarePath(*farePathData, passedDates);

    CPPUNIT_ASSERT(!makeSkip);
  }

  void storeFarePathStatusForSkip_step2()
  {
    size_t expectedPassedDateSize = 1;
    _flightFinderJourneyValidator->fFTrx()->bffStep() = FlightFinderTrx::STEP_2;

    FlightFinderJourneyValidator::FarePathType* farePathData = 0;
    farePathData = buildFarePathData(DateTime(2008, 9, 19), DateTime::emptyDate());

    std::set<DateTime> passedDates;
    _flightFinderJourneyValidator->storeFarePathStatusForSkip(*farePathData, passedDates);

    CPPUNIT_ASSERT_EQUAL(passedDates.size(), expectedPassedDateSize);
  }

  void storeFarePathStatusForSkip_step4()
  {
    size_t expectedPassedDateSize = 1;
    _flightFinderJourneyValidator->fFTrx()->bffStep() = FlightFinderTrx::STEP_4;

    FlightFinderJourneyValidator::FarePathType* farePathData = 0;
    farePathData = buildFarePathData(DateTime(2008, 9, 19), DateTime(2008, 9, 20));

    std::set<DateTime> passedDates;
    _flightFinderJourneyValidator->storeFarePathStatusForSkip(*farePathData, passedDates);

    CPPUNIT_ASSERT_EQUAL(passedDates.size(), expectedPassedDateSize);
  }

  void storeFarePathStatusForSkip_InboundIsOpen_step4()
  {
    size_t expectedPassedDateSize = 0;
    _flightFinderJourneyValidator->fFTrx()->bffStep() = FlightFinderTrx::STEP_4;

    FlightFinderJourneyValidator::FarePathType* farePathData = 0;
    farePathData = buildFarePathData(DateTime(2008, 9, 19), DateTime::emptyDate());

    std::set<DateTime> passedDates;
    _flightFinderJourneyValidator->storeFarePathStatusForSkip(*farePathData, passedDates);

    CPPUNIT_ASSERT_EQUAL(passedDates.size(), expectedPassedDateSize);
  }

protected:
  FlightFinderJourneyValidator::FarePathType*
  buildFarePathData(DateTime outboundDateTime, DateTime inboundDateTime)
  {
    // FarePathType->FarePath->PricingUnit->FareUsage->AirSeg
    FlightFinderJourneyValidator::FarePathType* farePathData = 0;
    _dataHandle.get(farePathData);
    FarePath* farePath = 0;
    _dataHandle.get(farePath);
    farePathData->farePath = farePath;
    PricingUnit* pricingUnit = 0;
    _dataHandle.get(pricingUnit);
    farePath->pricingUnit().push_back(pricingUnit);
    FareUsage* fareUsage = 0;
    _dataHandle.get(fareUsage);
    pricingUnit->fareUsage().push_back(fareUsage);
    AirSeg* tvlSeg =
        FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "KTW", "DFW", "LH");
    tvlSeg->segmentType() = Open;
    tvlSeg->departureDT() = outboundDateTime;
    fareUsage->travelSeg().push_back(tvlSeg);

    FareUsage* fareUsageIn = 0;
    _dataHandle.get(fareUsageIn);
    pricingUnit->fareUsage().push_back(fareUsageIn);
    AirSeg* tvlSegIn =
        FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "KTW", "DFW", "LH");
    if (inboundDateTime.isEmptyDate())
    {
      tvlSegIn->segmentType() = Open;
    }
    tvlSegIn->departureDT() = inboundDateTime;
    fareUsageIn->travelSeg().push_back(tvlSegIn);

    return farePathData;
  }

  void fillOutboundDateFlightMap(FlightFinderTrx::OutBoundDateFlightMap& outBoundDateFlightMap,
                                 const FlightListMapOutboundDataType& flightListMapOutboundData)
  {
    FlightListMapOutboundDataType::const_iterator outIter = flightListMapOutboundData.begin();
    for (; outIter != flightListMapOutboundData.end(); ++outIter)
    {
      std::vector<uint8_t>::const_iterator flightIter = outIter->second.flightList.begin();
      for (; flightIter != outIter->second.flightList.end(); ++flightIter)
      {
        FareValidatorOrchestratorTestCommon::addOutboundToFlightListMap(
            outBoundDateFlightMap, _dataHandle, outIter->first, *flightIter, _paxTypeFare);
      }
      FlightListMapInboundDataType::const_iterator inIter =
          outIter->second.flightListInboundMapData.begin();
      for (; inIter != outIter->second.flightListInboundMapData.end(); ++inIter)
      {
        std::vector<uint8_t>::const_iterator flightIter = inIter->second.begin();
        for (; flightIter != inIter->second.end(); ++flightIter)
        {
          FareValidatorOrchestratorTestCommon::addInboundToFlightListMap(outBoundDateFlightMap,
                                                                         _dataHandle,
                                                                         outIter->first,
                                                                         inIter->first,
                                                                         *flightIter,
                                                                         _paxTypeFare);
        }
      }
    }
  }

  void createSchedulingOptionVect(const FlightListMapOutboundDataType& flightListMapOutboundData,
                                  bool roundTrip,
                                  const GlobalDirection GD = GlobalDirection::ZZ)
  {
    // create sopItinerary
    Itin* sopItineraryOut = 0;
    Itin* sopItineraryIn = 0;
    _dataHandle.get(sopItineraryOut);
    if (roundTrip)
    {
      _dataHandle.get(sopItineraryIn);

      AirSeg* airSegIn =
          FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "FRA", "KRK", "AA");
      // add airSeg to travelSeg in sopItinerary
      sopItineraryIn->travelSeg().push_back(airSegIn);

      _flightFinderJourneyValidator->fFTrx()->legs().push_back(ShoppingTrx::Leg());
    }

    AirSeg* airSegOut =
        FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "KRK", "FRA", "AA");
    // add airSeg to travelSeg in sopItinerary
    sopItineraryOut->travelSeg().push_back(airSegOut);

    _flightFinderJourneyValidator->fFTrx()->legs().push_back(ShoppingTrx::Leg());

    FlightListMapOutboundDataType::const_iterator outIter = flightListMapOutboundData.begin();
    for (; outIter != flightListMapOutboundData.end(); ++outIter)
    {
      std::vector<uint8_t>::const_iterator flightIter = outIter->second.flightList.begin();
      for (; flightIter != outIter->second.flightList.end(); ++flightIter)
      {
        ShoppingTrx::SchedulingOption sop(sopItineraryOut, *flightIter, true);
        sop.globalDirection() = GD;
        _flightFinderJourneyValidator->fFTrx()->legs().front().sop().resize(*flightIter + 1, sop);
        _flightFinderJourneyValidator->fFTrx()->legs().front().sop()[*flightIter] = sop;
      }
      FlightListMapInboundDataType::const_iterator inIter =
          outIter->second.flightListInboundMapData.begin();
      for (; inIter != outIter->second.flightListInboundMapData.end(); ++inIter)
      {
        std::vector<uint8_t>::const_iterator flightIter = inIter->second.begin();
        for (; flightIter != inIter->second.end(); ++flightIter)
        {
          ShoppingTrx::SchedulingOption sop(sopItineraryIn, *flightIter, true);
          sop.globalDirection() = GD;
          _flightFinderJourneyValidator->fFTrx()->legs().back().sop().resize(*flightIter + 1, sop);
          _flightFinderJourneyValidator->fFTrx()->legs().back().sop()[*flightIter] = sop;
        }
      }
    }
  }

  void createAltDatePairs(PricingTrx::AltDatePairs& altDatePairs,
                          const FlightListMapOutboundDataType& flightListMapOutboundData)
  {
    FlightListMapOutboundDataType::const_iterator outIter = flightListMapOutboundData.begin();
    for (; outIter != flightListMapOutboundData.end(); ++outIter)
    {
      if (outIter->second.flightListInboundMapData.size() == 0) // ow
      {
        altDatePairs[std::make_pair(outIter->first, DateTime::emptyDate())] =
            buildAltDateInfo(false);
      }
      else // rt
      {
        FlightListMapInboundDataType::const_iterator inIter =
            outIter->second.flightListInboundMapData.begin();
        for (; inIter != outIter->second.flightListInboundMapData.end(); ++inIter)
        {
          altDatePairs[std::make_pair(outIter->first, inIter->first)] = buildAltDateInfo(true);
        }
      }
    }
  }

  void fillFareMarketSegBackup()
  {
    AirSeg* airSegOut =
        FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "KRK", "FRA", "AA");
    AirSeg* airSegIn =
        FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "FRA", "KRK", "AA");

    _flightFinderJourneyValidator->fareMarketSegBackup().push_back(airSegOut);
    _flightFinderJourneyValidator->fareMarketSegBackup().push_back(airSegIn);
  }

  void createDataPairsMap(const FlightListMapOutboundDataType& flightListMapOutboundData)
  {
    FlightFinderTrx::FlightBitInfo* flightBitInfo = buildEmptyFlightBitInfo();
    _dataHandle.get(_flightFinderJourneyValidator->dataPairsMap());
    std::map<DatePair, FlightFinderTrx::FlightBitInfo>* dataPairsMap =
        _flightFinderJourneyValidator->dataPairsMap();

    FlightListMapOutboundDataType::const_iterator outIter = flightListMapOutboundData.begin();
    for (; outIter != flightListMapOutboundData.end(); ++outIter)
    {
      if (outIter->second.flightListInboundMapData.size() == 0) // ow
      {
        (*dataPairsMap)[std::make_pair(outIter->first, DateTime::emptyDate())] = *flightBitInfo;
      }
      else // rt
      {
        FlightListMapInboundDataType::const_iterator inIter =
            outIter->second.flightListInboundMapData.begin();
        for (; inIter != outIter->second.flightListInboundMapData.end(); ++inIter)
        {
          (*dataPairsMap)[std::make_pair(outIter->first, inIter->first)] = *flightBitInfo;
        }
      }
    }
  }

  PricingTrx::AltDateInfo* buildAltDateInfo(bool roundTrip)
  {
    PricingTrx::AltDateInfo* altDateInfo = 0;
    _dataHandle.get(altDateInfo);

    Itin* journeyItinerary;
    _dataHandle.get(journeyItinerary);

    AirSeg* airSegOut =
        FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "KRK", "FRA", "AA");
    journeyItinerary->travelSeg().push_back(airSegOut);
    if (roundTrip)
    {
      AirSeg* airSegIn =
          FareValidatorOrchestratorTestCommon::buildSegment(_dataHandle, "FRA", "KRK", "AA");
      journeyItinerary->travelSeg().push_back(airSegIn);
    }

    altDateInfo->journeyItin = journeyItinerary;

    return altDateInfo;
  }

  FlightFinderTrx::FlightBitInfo* buildEmptyFlightBitInfo()
  {
    FlightFinderTrx::FlightBitInfo* flightBitInfo = 0;
    _dataHandle.get(flightBitInfo);
    PaxTypeFare* paxTypeFare = 0;
    _dataHandle.get(paxTypeFare);

    flightBitInfo->flightBitStatus = 0;
    flightBitInfo->paxTypeFareVect.push_back(paxTypeFare);
    flightBitInfo->inboundPaxTypeFareVect.push_back(paxTypeFare);

    return flightBitInfo;
  }

  void clearTrxData()
  {
    _flightFinderJourneyValidator->fFTrx()->outboundDateflightMap().clear();
    _flightFinderJourneyValidator->fFTrx()->altDatePairs().clear();
  }

protected:
  void initialize()
  {
    FlightFinderTrx* fFTrx;
    _dataHandle.get(fFTrx);
    _flightFinderJourneyValidator->initialize(fFTrx);
  }

protected:
  RefDataHandle _dataHandle;
  PaxTypeFare* _paxTypeFare;
  MockFlightFinderJourneyValidator* _flightFinderJourneyValidator;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FlightFinderJourneyValidatorTest);

} // tse
