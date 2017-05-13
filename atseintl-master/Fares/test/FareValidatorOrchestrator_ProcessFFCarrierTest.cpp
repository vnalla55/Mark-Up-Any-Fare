#include "Common/ShoppingUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "Fares/test/FareValidatorOrchestratorTestCommon.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class FareValidatorOrchestrator_ProcessFFCarrierTest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
    std::vector<LimitationFare*> _ret;

  public:
    const std::vector<LimitationFare*>& getFCLimitation(const DateTime& date) { return _ret; }
  };
  CPPUNIT_TEST_SUITE(FareValidatorOrchestrator_ProcessFFCarrierTest);
  CPPUNIT_TEST(testFareValidatorOrchestrator_ProcessFFCarrierTest_GLOBALDIR_FAIL);
  CPPUNIT_TEST(testFareValidatorOrchestrator_ProcessFFCarrierTest_PrimarySector);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _mockTseServer = _memHandle.create<MockTseServer>();
  }

  void tearDown()
  {
    _memHandle.clear();
    _dataHandle.clear();
  }

  void testFareValidatorOrchestrator_ProcessFFCarrierTest_GLOBALDIR_FAIL()
  {
    MyDataHandle mdh;
    FareValidatorOrchestratorDerived fvo("FVO", *_mockTseServer);
    ItinIndex::Key cxrKey;
    FareMarket* fM;
    GlobalDirection globalDirection = GlobalDirection::WH;
    // create trx wirh fare market with fares with WH direction
    FlightFinderTrx* flightFinderTrx = createFilledFlightFinderTrx(cxrKey, fM, globalDirection);
    ShoppingTrx::FareMarketRuleMap& fmrm = flightFinderTrx->fareMarketRuleMap();
    std::deque<ShoppingTrx::AltDatePairs> altDateJourneyItins;
    altDateJourneyItins.push_back(ShoppingTrx::AltDatePairs());
    // create empty flightBitMap for PTFs
    createFlightBitMapForPTFsInMarket(flightFinderTrx, fM, cxrKey);
    ShpBitValidationCollector::FMValidationSharedData* sharedData(
        flightFinderTrx->getBitValidationCollector().getFMSharedData(cxrKey, fM));
    fvo.processFFCarrier(*flightFinderTrx,
                         cxrKey,
                         (unsigned int)0,
                         flightFinderTrx->legs().front(),
                         flightFinderTrx->journeyItin(),
                         altDateJourneyItins.front(),
                         flightFinderTrx->fareMarketRuleController(),
                         flightFinderTrx->cat4RuleController(),
                         fM,
                         fmrm[fM]._shoppingBCETuningData,
                         sharedData);
    CPPUNIT_ASSERT(statusOfPaxTypeFare(fM, 'G'));
  }

  void testFareValidatorOrchestrator_ProcessFFCarrierTest_PrimarySector()
  {
    FareValidatorOrchestratorDerived fvo("FVO", *_mockTseServer);
    ItinIndex::Key cxrKey;
    FareMarket* fM;
    FareValidatorOrchestrator::FMBackup fmb;
    GlobalDirection globalDirection = GlobalDirection::WH;
    const uint32_t beginSeg = 0;
    uint32_t endSeg = 1;

    FlightFinderTrx* trx = createFilledFlightFinderTrx(cxrKey, fM, globalDirection);
    ItinIndex::ItinCell& itinCell = *(trx->legs().front().carrierIndex().beginRow(cxrKey));
    itinCell.first.setPrimarySector(itinCell.second->travelSeg()[1]);

    TravelSeg* primarySectorCopy = fM->primarySector();

    fvo.prepareShoppingValidation(*trx,
                                  trx->journeyItin(),
                                  itinCell,
                                  fM,
                                  fmb,
                                  beginSeg,
                                  endSeg);

    CPPUNIT_ASSERT_EQUAL(fM->travelSeg()[1], fM->primarySector());

    fvo.cleanupAfterShoppingValidation(*trx,
                                       trx->journeyItin(),
                                       fM,
                                       fmb,
                                       beginSeg,
                                       endSeg);

    CPPUNIT_ASSERT_EQUAL(fmb.primarySector, primarySectorCopy);
  }

protected:
  void createFlightBitMapForPTFsInMarket(FlightFinderTrx* flightFinderTrx,
                                         FareMarket* fM,
                                         const ItinIndex::Key& cxrKey)
  {
    std::vector<PaxTypeFare*>::iterator pTFIter = fM->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator pTFEndIter = fM->allPaxTypeFare().end();
    uint32_t flightSize;

    for (; pTFIter != pTFEndIter; ++pTFIter)
    {
      PaxTypeFare*& curFare = static_cast<PaxTypeFare*&>(*pTFIter);

      if (curFare == NULL)
      {
        continue;
      }

      flightSize = 0;
      flightSize = flightFinderTrx->legs().front().getFlightBitmapSize(*flightFinderTrx, cxrKey);
      curFare->setFlightBitmapSize(flightSize);
    }
  }

  AirSeg* buildSegment(std::string origin, std::string destination, std::string carrier)
  {
    AirSeg* airSeg;
    _dataHandle.get(airSeg);
    airSeg->geoTravelType() = GeoTravelType::International;
    airSeg->origAirport() = origin;
    airSeg->departureDT() = DateTime::localTime();
    airSeg->origin() = _dataHandle.getLoc(airSeg->origAirport(), airSeg->departureDT());
    // allocData.push_back((unsigned int)airSeg->origin());
    airSeg->destAirport() = destination;
    airSeg->arrivalDT() = DateTime::localTime();
    airSeg->destination() = _dataHandle.getLoc(airSeg->destAirport(), airSeg->arrivalDT());
    // allocData.push_back((unsigned int)airSeg->destination());
    airSeg->boardMultiCity() = origin;
    airSeg->offMultiCity() = destination;
    airSeg->carrier() = carrier;
    return airSeg;
  }

  FlightFinderTrx* createFilledFlightFinderTrx(ItinIndex::Key& cxrKey,
                                               FareMarket*& fM,
                                               GlobalDirection& globalDirection)
  {
    // create flightFinderTrx
    FlightFinderTrx* flightFinderTrx;
    _dataHandle.get(flightFinderTrx);
    PricingRequest* request;
    _dataHandle.get(request);
    flightFinderTrx->setRequest(request);
    PricingOptions* option;
    _dataHandle.get(option);
    flightFinderTrx->setOptions(option);
    // create airSegs
    AirSeg* airSeg1 = buildSegment("KRK", "FRA", "AA");
    AirSeg* airSeg2 = buildSegment("FRA", "DFW", "AA");
    AirSeg* airSeg3 = buildSegment("DFW", "KRK", "AA");
    // create sopItinerary
    Itin* sopItinerary;
    _dataHandle.get(sopItinerary);
    // create journeyItinerary
    Itin* journeyItinerary;
    _dataHandle.get(journeyItinerary);
    // add airSegs to travelSeg in sopItinerary
    sopItinerary->travelSeg().push_back(airSeg1);
    sopItinerary->travelSeg().push_back(airSeg2);
    // add airSegs to travelSeg in journeyItinerary
    journeyItinerary->travelSeg().push_back(airSeg1);
    journeyItinerary->travelSeg().push_back(airSeg2);
    journeyItinerary->travelSeg().push_back(airSeg3);
    // add travelSeg to journayItin
    flightFinderTrx->journeyItin() = journeyItinerary;
    // add leg to flightFinderTrx
    flightFinderTrx->legs().push_back(ShoppingTrx::Leg());
    ShoppingTrx::SchedulingOption sop(sopItinerary, 1, true);
    sop.governingCarrier() = "AA";
    // add sop to leg
    flightFinderTrx->legs().front().sop().push_back(sop);
    // create itinCellInfo
    ItinIndex::ItinCellInfo* itinCellInfo;
    _dataHandle.get(itinCellInfo);
    // create fm
    _dataHandle.get(fM);

    // create paxTypeFare1
    PaxTypeFare* paxTypeFare1 = FareValidatorOrchestratorTestCommon::buildPaxTypeFare(
        _dataHandle,
        fM,
        FareValidatorOrchestratorTestCommon::buildFareInfo(
            _dataHandle, "ATP", "AA", "KRK", "DFW", globalDirection));
    // create paxTypeFare2
    PaxTypeFare* paxTypeFare2 = FareValidatorOrchestratorTestCommon::buildPaxTypeFare(
        _dataHandle,
        fM,
        FareValidatorOrchestratorTestCommon::buildFareInfo(
            _dataHandle, "ATP", "AA", "KRK", "DFW", globalDirection));
    // add fares to fm
    fM->allPaxTypeFare().push_back(paxTypeFare1);
    fM->allPaxTypeFare().push_back(paxTypeFare2);
    // add fm to itin
    sopItinerary->fareMarket().push_back(fM);
    // generate crx key
    ShoppingUtil::createCxrKey("AA", cxrKey);
    // generate schedule key
    ItinIndex::Key scheduleKey;
    ShoppingUtil::createScheduleKey(scheduleKey);
    // add itinCell to matrix
    flightFinderTrx->legs().front().carrierIndex().addItinCell(
        sopItinerary, *itinCellInfo, cxrKey, scheduleKey);
    return flightFinderTrx;
  }

  bool statusOfPaxTypeFare(FareMarket* fM, const uint8_t status)
  {
    bool everyPaxTypeFareHasDesiredStatus = false;
    std::vector<PaxTypeFare*>::iterator pTFIter = fM->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator pTFEndIter = fM->allPaxTypeFare().end();

    for (; pTFIter != pTFEndIter; ++pTFIter)
    {
      PaxTypeFare*& curFare = static_cast<PaxTypeFare*&>(*pTFIter);

      if (curFare == NULL)
      {
        continue;
      }

      for (uint32_t i = 0; i < curFare->getFlightBitmapSize(); ++i)
      {
        if (status != *(curFare->getFlightBit(i)))
        {
          everyPaxTypeFareHasDesiredStatus = false;
          break;
        }
        else
        {
          everyPaxTypeFareHasDesiredStatus = true;
        }
      }

      if (!everyPaxTypeFareHasDesiredStatus)
      {
        break;
      }
    }

    return everyPaxTypeFareHasDesiredStatus;
  }

protected:
  RefDataHandle _dataHandle;
  MockTseServer* _mockTseServer;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareValidatorOrchestrator_ProcessFFCarrierTest);

} // tse
