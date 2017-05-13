#ifndef FARE_VALIDATOR_ORCHESTRATOR_TEST_COMMON_H
#define FARE_VALIDATOR_ORCHESTRATOR_TEST_COMMON_H

#include "Fares/FareValidatorOrchestrator.h"
#include "test/include/MockTseServer.h"
#include "test/include/RefDataHandle.h"
#include "DataModel/AirSeg.h"

namespace tse
{

class FareValidatorOrchestratorDerived : public FareValidatorOrchestrator
{
  friend class FareValidatorOrchestratorTest;
  friend class FareValidatorOrchestrator_AddOutboundToFlightTest;
  friend class FareValidatorOrchestrator_AddInboundToFlightTest;
  friend class FareValidatorOrchestrator_AddAltDateToFlightListTest;
  friend class FareValidatorOrchestrator_CombineAltDateStatusTest;
  friend class FareValidatorOrchestrator_CombineDurationsFlightBitmapsForEachPaxTypeFareTest;
  friend class FareValidatorOrchestrator_CombineFlightBitmapsForAllPaxTypeFareTest;
  friend class FareValidatorOrchestrator_ProcessFFCarrierTest;
  friend class FareValidatorOrchestrator_CombineAltDateStatusPerLegTest;
  friend class FareValidatorOrchestrator_CombineCombinedFlightBitmapPerPaxTypeTest;
  friend class FareValidatorOrchestrator_CombineCombinedDurationFlightBitmapPerPaxTypeTest;
  friend class FareValidatorOrchestrator_dynamicDuration;

public:
  DateTime validatedFareDate;

  FareValidatorOrchestratorDerived(const std::string& name, TseServer& server)
    : FareValidatorOrchestrator(name, server),
      validatedFareDate(DateTime::emptyDate()),
      validatedFare(0)
  {
  }

  virtual ~FareValidatorOrchestratorDerived() {}

  virtual void primaryValidationForNonKeepFare(RexPricingTrx& trx)
  {
    validatedFareDate = trx.fareApplicationDT();
  }

  virtual void
  validateKeepFare(RexPricingTrx& trx, Itin& itin, PaxTypeFare& keepFare, DateTime& keepFareDate)
  {
    validatedFare = &keepFare;
    validatedFareDate = keepFareDate;
  }

  virtual void duplicatedFareCheckDerived( PricingTrx& trx,
                                           Itin& itin,
                                           FareMarket& fareMarket)
  {
    duplicatedFareCheck(trx,itin,fareMarket);
  }

  PaxTypeFare* validatedFare;
};

class MockConstructedFareInfo : public ConstructedFareInfo
{
public:
  using ConstructedFareInfo::_constructionType;
};

class FareValidatorOrchestratorTestCommon
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

  static TariffCrossRefInfo*
  buildTariffCrossRefInfo(DataHandle& dataHandle, std::string vendor, std::string carrier)
  {
    TariffCrossRefInfo* tariffCrossRefInfo;
    dataHandle.get(tariffCrossRefInfo);
    tariffCrossRefInfo->_vendor = vendor;
    tariffCrossRefInfo->_carrier = carrier;
    tariffCrossRefInfo->_globalDirection = GlobalDirection::WH;

    return tariffCrossRefInfo;
  }

  static MockConstructedFareInfo* buildConstructedFareInfo(DataHandle& dataHandle)
  {
    MockConstructedFareInfo* constructedFareInfo;
    dataHandle.get(constructedFareInfo);
    constructedFareInfo->_constructionType = ConstructedFareInfo::DOUBLE_ENDED;

    return constructedFareInfo;
  }

  static PaxTypeFare* buildPaxTypeFare(DataHandle& dataHandle,
                                       FareMarket* fM,
                                       FareInfo* fareInfo = 0,
                                       TariffCrossRefInfo* tariffCrossRefInfo = 0,
                                       ConstructedFareInfo* constructedFareInfo = 0)
  {
    PaxTypeFare* paxTypeFare;
    dataHandle.get(paxTypeFare);

    // create paxType
    PaxType* paxType = 0;
    dataHandle.get(paxType);

    // create tariffCrossRefInfo
    if (0 == tariffCrossRefInfo)
    {
      tariffCrossRefInfo =
          FareValidatorOrchestratorTestCommon::buildTariffCrossRefInfo(dataHandle, "ATP", "AA");
    }

    // create constructedFareInfo
    if (0 == constructedFareInfo)
    {
      constructedFareInfo =
          FareValidatorOrchestratorTestCommon::buildConstructedFareInfo(dataHandle);
    }

    // create fare
    Fare* fare = 0;
    dataHandle.get(fare);
    fare->initialize(
        Fare::FS_ConstructedFare, fareInfo, *fM, tariffCrossRefInfo, constructedFareInfo);
    fare->setGlobalDirectionValid(true);

    PricingTrx dummyTrx;
    paxTypeFare->initialize(fare, paxType, fM, dummyTrx);

    return paxTypeFare;
  }

  static void addOutboundToFlightListMap(
      std::map<DateTime, FlightFinderTrx::OutBoundDateInfo*>& outBoundDateFlightMap,
      DataHandle& dataHandle,
      const DateTime& outboundDepartureDT,
      const uint32_t originalSopIndex,
      PaxTypeFare* paxTypeFare)
  {
    if (outBoundDateFlightMap.count(outboundDepartureDT) == 0)
    {
      FlightFinderTrx::OutBoundDateInfo* outBoundDateInfoPtr = 0;
      dataHandle.get(outBoundDateInfoPtr);
      FlightFinderTrx::SopInfo* sopInfo;
      dataHandle.get(sopInfo);

      sopInfo->sopIndex = originalSopIndex;
      // setSopInfo(sopInfo->bkgCodes, segStatus);
      sopInfo->paxTypeFareVect.push_back(paxTypeFare);
      outBoundDateInfoPtr->flightInfo.flightList.push_back(sopInfo);
      outBoundDateFlightMap[outboundDepartureDT] = outBoundDateInfoPtr;
    }
    else
    {
      // if exist add next sopIndex to this date
      if (outBoundDateFlightMap[outboundDepartureDT] != 0)
      {
        FlightFinderTrx::SopInfo* sopInfo;
        dataHandle.get(sopInfo);

        sopInfo->sopIndex = originalSopIndex;
        sopInfo->paxTypeFareVect.push_back(paxTypeFare);
        // setSopInfo(sopInfo->bkgCodes, segStatus);
        outBoundDateFlightMap[outboundDepartureDT]->flightInfo.flightList.push_back(sopInfo);
      }
    }
  }

  static void addInboundToFlightListMap(
      std::map<DateTime, FlightFinderTrx::OutBoundDateInfo*>& outBoundDateFlightMap,
      DataHandle& dataHandle,
      const DateTime& outboundDepartureDT,
      const DateTime& inboundDepartureDT,
      const uint32_t originalSopIndex,
      PaxTypeFare* paxTypeFare)
  {
    FlightFinderTrx::OutBoundDateFlightMap::const_iterator it =
        outBoundDateFlightMap.find(outboundDepartureDT);
    if (it != outBoundDateFlightMap.end())
    {
      if (it->second->iBDateFlightMap.count(inboundDepartureDT) == 0)
      {
        FlightFinderTrx::FlightDataInfo* flightInfo = 0;
        dataHandle.get(flightInfo);
        FlightFinderTrx::SopInfo* sopInfo;
        dataHandle.get(sopInfo);

        sopInfo->sopIndex = originalSopIndex;
        sopInfo->paxTypeFareVect.push_back(paxTypeFare);
        // setSopInfo(sopInfo->bkgCodes, segStatus);
        flightInfo->flightList.push_back(sopInfo);
        it->second->iBDateFlightMap[inboundDepartureDT] = flightInfo;
      }
      else
      {
        // add to existing one
        FlightFinderTrx::SopInfo* sopInfo;
        dataHandle.get(sopInfo);

        sopInfo->sopIndex = originalSopIndex;
        sopInfo->paxTypeFareVect.push_back(paxTypeFare);
        // setSopInfo(sopInfo->bkgCodes, segStatus);
        it->second->iBDateFlightMap[inboundDepartureDT]->flightList.push_back(sopInfo);
      }
    }
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
    PricingTrx dummyTrx;
    paxTypeFare->initialize(fare, 0, fM, dummyTrx);

    return paxTypeFare;
  }

  static AirSeg* buildSegment(DataHandle& dataHandle,
                              std::string origin,
                              std::string destination,
                              std::string carrier)
  {
    AirSeg* airSeg;
    dataHandle.get(airSeg);

    airSeg->geoTravelType() = GeoTravelType::International;
    airSeg->origAirport() = origin;
    airSeg->departureDT() = DateTime::localTime();
    airSeg->origin() = dataHandle.getLoc(airSeg->origAirport(), airSeg->departureDT());
    airSeg->destAirport() = destination;
    airSeg->arrivalDT() = DateTime::localTime();
    airSeg->destination() = dataHandle.getLoc(airSeg->destAirport(), airSeg->arrivalDT());
    airSeg->boardMultiCity() = origin;
    airSeg->offMultiCity() = destination;
    airSeg->carrier() = carrier;

    return airSeg;
  }
};
}
#endif // FARE_VALIDATOR_ORCHESTRATOR_TEST_COMMON_H
