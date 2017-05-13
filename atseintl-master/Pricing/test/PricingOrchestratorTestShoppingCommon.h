#pragma once

#include "DataModel/AirSeg.h"
#include "DataModel/ShoppingTrx.h"
#include "Fares/BitmapOpOrderer.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/ShoppingPQ.h"

#include "test/include/MockTseServer.h"
#include "test/include/RefDataHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestArunkSegFactory.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestFarePathFactory.h"
#include "test/testdata/TestItinFactory.h"
#include "test/testdata/TestShoppingTrxFactory.h"
#include "test/testdata/TestTravelSegFactory.h"

namespace tse
{
struct EqualMatrixRater
{
  int operator()(const std::vector<int>& v) const { return 1; }
};

class PricingOrchestratorTestShoppingCommon;

class PricingOrchestratorDerived : public PricingOrchestrator
{
  friend class PricingOrchestratorTestShoppingCommon;
  friend class PricingOrchestrator_RemoveExcessiveAltDateOptionsTest;
  friend class PricingOrchestrator_MoveEstimatedSolutionsToFlightMatrixTest;
  friend class ShoppingPQTest;
  friend class PricingOrchestrator_CustomSolutionBuilderTest;

public:
  PricingOrchestratorDerived(TseServer& srv) : PricingOrchestrator(srv) {}
};

class ShoppingPQDerived : public ShoppingPQ
{
  friend class PricingOrchestratorTestShoppingCommon;
  friend class ShoppingPQTest;
  friend class PricingOrchestrator_CustomSolutionBuilderTest;

public:
  ShoppingPQDerived(PricingOrchestrator& po,
                    ShoppingTrx& trx,
                    uint32_t nopt,
                    int estopt,
                    CarrierCode* cxr,
                    BitmapOpOrderer& orderer)
    : ShoppingPQ(po, trx, NULL, nopt, estopt, cxr, orderer, true)
  {
  }
};
class PricingOrchestratorTestShoppingCommon
{
public:
  static FareInfo* buildFareInfo(DataHandle& dataHandle,
                                 std::string vendor,
                                 std::string carrier,
                                 std::string market1,
                                 std::string market2,
                                 GlobalDirection& globalDirection,
                                 TSEDateInterval range = TSEDateInterval())
  {
    FareInfo* fareInfo;
    dataHandle.get(fareInfo);

    fareInfo->_vendor = vendor;
    fareInfo->_carrier = carrier;
    fareInfo->_market1 = market1;
    fareInfo->_market2 = market2;
    fareInfo->_effInterval = range;
    fareInfo->_directionality = FROM;
    fareInfo->_globalDirection = globalDirection; // ZZ - turn off global direct validation

    return fareInfo;
  }

  static PaxTypeFare* buildFakePaxTypeFare(DataHandle& dataHandle)
  {
    PaxTypeFare* paxTypeFare;
    dataHandle.get(paxTypeFare);
    FareMarket* fM;
    dataHandle.get(fM);
    Fare* fare;
    dataHandle.get(fare);
    FareInfo* fareInfo;
    dataHandle.get(fareInfo);
    fareInfo->fareClass() = "TESTFARECLASS";
    fare->initialize(Fare::FS_ConstructedFare, fareInfo, *fM, 0, 0);
    paxTypeFare->initialize(fare, 0, fM);

    return paxTypeFare;
  }
  static GroupFarePath* buildGroupFarePath(DataHandle& dataHandle, MoneyAmount amt = 1000000)
  {
    GroupFarePath* gfp = NULL;
    dataHandle.get(gfp);

    gfp->setTotalNUCAmount(amt);

    return gfp;
  }

  static FarePath* buildFarePath(DataHandle& dataHandle, std::string filename)
  {
    FarePath* fp = TestFarePathFactory::create(filename);

    return fp;
  }

  static AirSeg* buildSegment(DataHandle& dataHandle,
                              std::string origin,
                              std::string destination,
                              std::string carrier,
                              DateTime depDT = DateTime::localTime(),
                              uint32_t noOfDays = 0)
  {
    AirSeg* airSeg = NULL;
    dataHandle.get(airSeg);

    airSeg->geoTravelType() = GeoTravelType::International;
    airSeg->origAirport() = origin;
    airSeg->origin() = dataHandle.getLoc(airSeg->origAirport(), airSeg->departureDT());
    airSeg->destAirport() = destination;
    airSeg->departureDT() = depDT;
    airSeg->arrivalDT() = depDT.addDays(noOfDays);
    airSeg->destination() = dataHandle.getLoc(airSeg->destAirport(), airSeg->arrivalDT());
    airSeg->boardMultiCity() = origin;
    airSeg->offMultiCity() = destination;
    airSeg->carrier() = carrier;

    return airSeg;
  }

  static AirSeg* buildSegment(std::string filename)
  {
    return TestAirSegFactory::create(filename, true);
  }

  static ArunkSeg* buildArunkSegment(std::string filename)
  {
    return TestArunkSegFactory::create(filename, true);
  }

  static Itin* buildItin(std::string filename) { return TestItinFactory::create(filename, true); }

  static ShoppingTrx* createTrx(DataHandle& dataHandle, std::string filename, bool unique = false)
  {
    ShoppingTrx* trx = TestShoppingTrxFactory::create(filename, unique);
    ShoppingTrx::Leg* pLeg1 = 0;
    ShoppingTrx::Leg* pLeg2 = 0;
    dataHandle.get(pLeg1);
    dataHandle.get(pLeg2);
    trx->legs().clear();
    trx->legs().push_back(*pLeg1);
    trx->legs().push_back(*pLeg2);
    PaxType* paxType = 0;
    dataHandle.get(paxType);
    trx->paxType().clear();
    trx->paxType().push_back(paxType);
    pLeg1->sop().reserve(10);
    pLeg2->sop().reserve(10);
    trx->flightMatrix().clear();

    return trx;
  }

  static ShoppingTrx* createOwTrx(DataHandle& dataHandle, std::string filename, bool unique = false)
  {
    ShoppingTrx* trx = TestShoppingTrxFactory::create(filename, unique);
    ShoppingTrx::Leg* pLeg1 = 0;
    dataHandle.get(pLeg1);
    trx->legs().clear();
    trx->legs().push_back(*pLeg1);
    PaxType* paxType = 0;
    dataHandle.get(paxType);
    trx->paxType().clear();
    trx->paxType().push_back(paxType);
    pLeg1->sop().reserve(10);
    trx->flightMatrix().clear();

    return trx;
  }

  static BitmapOpOrderer* createBitmapOpOrderer()
  {
    tse::ConfigMan* configMan = new tse::ConfigMan();
    BitmapOpOrderer* orderer = new BitmapOpOrderer(*configMan);
    return orderer;
  }

  static void createSOP(DataHandle& dataHandle,
                        ShoppingTrx* trx,
                        ShoppingTrx::Leg& leg,
                        int idx,
                        const std::string& carrier,
                        MoneyAmount amt = 0,
                        bool isCustomSop = false,
                        bool isLongConx = false)
  {
    // create itinCellInfo
    ItinIndex::ItinCellInfo* itinCellInfo;
    dataHandle.get(itinCellInfo);

    // create sopItinerary)
    Itin* sopItinerary;
    dataHandle.get(sopItinerary);

    // onlineCarrier
    sopItinerary->onlineCarrier() = carrier;

    // AirSeg* as = 0;
    // dataHandle.get(as);
    // sopItinerary->travelSeg().push_back(as);

    ShoppingTrx::SchedulingOption sop(sopItinerary, idx, true);
    sop.governingCarrier() = carrier;
    sop.setCustomSop(isCustomSop);
    if (isCustomSop)
    {
      leg.setCustomLeg(true);
    }
    if (isLongConx)
      sop.isLngCnxSop() = true;

    // generate crx key
    ItinIndex::Key cxrKey;
    ShoppingUtil::createCxrKey(carrier, cxrKey);

    // generate schedule key
    ItinIndex::Key scheduleKey;
    ShoppingUtil::createScheduleKey(scheduleKey);

    leg.carrierIndex().addItinCell(sopItinerary, *itinCellInfo, cxrKey, scheduleKey);
    leg.sop().push_back(sop);

    if (amt != 0)
    {
      SOPFarePath* sopFarePath = 0;
      dataHandle.get(sopFarePath);
      sopFarePath->totalAmount() = amt;
      sopItinerary->paxTypeSOPFareListMap()[trx->paxType()[0]].owSopFarePaths().push_back(
          sopFarePath);
    }
  }

  static void addSegmentToItinerary(DataHandle& dataHandle,
                                    ShoppingTrx::Leg& leg,
                                    std::string origin,
                                    std::string destination,
                                    std::string carrier,
                                    DateTime depDT = DateTime::localTime(),
                                    uint32_t noOfDays = 0)
  {
    ShoppingTrx::SchedulingOption& sop = leg.sop().back();
    sop.itin()->travelSeg().push_back(
        buildSegment(dataHandle, origin, destination, carrier, depDT, noOfDays));
  }
};
} // tse
