//-------------------------------------------------------------------
//
//  File:        FlightFinderJourneyValidator.h
//  Created:     April 18, 2008
//  Authors:     Miroslaw Bartyna
//
//  Description: Journey Validator for Flight Finder
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DataModel/FlightFinderTrx.h"
#include "DataModel/PricingUnit.h"

namespace tse
{

class FlightFinderJourneyValidator
{
  friend class MockFlightFinderJourneyValidator;

public:
  struct FarePathType
  {
    FarePath* farePath;
    uint8_t* farePathStatus;
    std::vector<TravelSeg*> travelSegVVect;
    size_t* invalidatedCategory;
    GlobalDirection globalDirection;
    std::string* errorMsg;
  };

  enum FarePathStatus
  {
    FP_VALID = 0,
    FP_NOT_VALID = 1,
    FP_SKIPPED = 2
  };

  struct FarePathDataType
  {
    // vector contains up to two pairs, each for one leg
    // one pair contain data need to construct farePath
    std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> > farePathData;
    Itin* itin;
    FlightFinderTrx::FlightBitInfo* fBInfo;
    const DatePair* datePair;
    size_t invalidatedCategory;
    uint16_t outboundFarePosition;
    uint16_t inboundFarePosition;
    uint8_t farePathStatus;
    std::string errorMsg;
  };

  struct SOPElementData
  {
    // const OptionElementType* optionElement;
    const std::pair<const DateTime, FlightFinderTrx::OutBoundDateInfo*>* outboundData;
    const std::pair<const DateTime, FlightFinderTrx::FlightDataInfo*>* inboundData;
    uint16_t outboundSopPosition;
    uint16_t inboundSopPosition;
    uint16_t farePosition;
  };

  struct FarePathSOPDataType
  {
    SOPElementData sopElementData;
    std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> > farePathData;
    size_t invalidatedCategory;
    GlobalDirection globalDirection;
    uint8_t farePathStatus;
    bool thisIsOutbound;
    std::string errorMsg;
  };

  typedef std::vector<FarePathSOPDataType> FarePathSOPDataVectType;
  typedef std::vector<FarePathDataType> FarePathDataVectType;
  typedef std::vector<FarePathType*> FarePathVectType;

public:
  enum Step
  {
    STEP_ONE_OR_THREE,
    STEP_TWO,
  };

  FlightFinderJourneyValidator(FlightFinderTrx* fFTrx)
    : _fFTrx(fFTrx), _dataPairsMap(nullptr), _whichStep(STEP_ONE_OR_THREE) {};
  FlightFinderJourneyValidator(FlightFinderTrx* fFTrx,
                               std::map<DatePair, FlightFinderTrx::FlightBitInfo>* dataPairsMap)
    : _fFTrx(fFTrx), _dataPairsMap(dataPairsMap), _whichStep(STEP_TWO) {};
  virtual ~FlightFinderJourneyValidator() {};

  void validate();

private:
  bool isPUValidForFareRetailerCode(PricingUnit* pu,
                                    FarePathType* farePathType,
                                    uint8_t& farePathStatus,
                                    bool isDiag965);

  bool isFPValidForFareRetailerCode(FarePath& farePath,
                                    FarePathType* farePathType,
                                    uint8_t& farePathStatus,
                                    bool isDiag965);

  void prepareJourneyItin(const FarePathType& farePathData);
  void prepareSOPDataVect(FarePathSOPDataVectType& farePathSOPDataVect);
  PricingUnit*
  createPricingUnit(std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& PUData);
  void setTravelSegVect(std::vector<TravelSeg*>& travelSegVVect,
                        std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& PUData);
  void getTravelSeg(std::vector<TravelSeg*>& travelSeg,
                    const uint32_t legId,
                    const uint32_t& originalSopIndex);
  GlobalDirection getGlobalDirection(const uint32_t legId, const uint32_t sopIndex);
  void prepareFarePathDataVect(FarePathSOPDataVectType& farePathSOPDataVect,
                               FarePathVectType& farePathVect);
  void validateFarePathVect(FarePathVectType& farePathVect);
  FlightFinderJourneyValidator::FarePathType*
  buildFarePath(std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& farePathData,
                uint8_t* farePathStatus,
                size_t* invalidatedCategory,
                std::string* errorMsg,
                Itin* itin,
                const GlobalDirection globalDirection);
  void addOutbound(const uint16_t& farePosition,
                   const uint16_t& outboundSopPosition,
                   const DateTime& outboundDepartureDT,
                   const FlightFinderTrx::FlightDataInfo* flightDataInfo);
  void addInbound(const uint16_t& farePosition,
                  const uint16_t& inboundSopPosition,
                  const DateTime& outboundDepartureDT,
                  const DateTime& inboundDepartureDT,
                  const FlightFinderTrx::FlightDataInfo* flightDataInfo);
  void updateOutboundDateflightMap(FarePathSOPDataVectType& optionElementVect);
  void checkFarePathValidity(const FarePathSOPDataVectType& optionElementVect);
  bool alreadyAddedToFlightList(const std::vector<FlightFinderTrx::SopInfo*>& flightList,
                                const FlightFinderTrx::SopInfo* sopInfo);
  MoneyAmount calculateFarePathNUCAmount(
      const std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& farePathData);
  size_t getInvalidatingCategory(PricingUnit* pu);
  void addFarePathSOPData(
      FarePathSOPDataVectType& farePathSOPDataVect,
      const std::pair<const DateTime, FlightFinderTrx::OutBoundDateInfo*>* outboundData,
      const std::pair<const DateTime, FlightFinderTrx::FlightDataInfo*>* inboundData,
      uint16_t sopPosition,
      uint32_t sopIndex,
      bool thisIsOutbound);
  PaxType*
  getActualPaxType(std::vector<std::pair<std::vector<TravelSeg*>, PaxTypeFare*> >& farePathData);

  void showDiag965(FarePathDataVectType& farePathDataVect);
  void showDiag965(FarePathSOPDataVectType& farePathSOPDataVect);
  std::string getTravelSegmentsInfo(FarePathSOPDataType& farePathSOPData);
  std::string getSOPInfo(std::vector<TravelSeg*>& tvlSegmentVect);
  void prepareFMTvlSegs(const PricingUnit& pricingUnit,
                        std::vector<std::vector<TravelSeg*> >& backupFMTravelSegVect);
  void restoreFMTvlSegs(const PricingUnit& pricingUnit,
                        std::vector<std::vector<TravelSeg*> >& backupFMTravelSegVect);
  FlightFinderTrx::SopInfo* buildSopInfo(const uint16_t& farePosition,
                                         FlightFinderTrx::SopInfo* sopInfo,
                                         bool thisIsOutbound);
  void removeOutboundsWithoutInbounds();
  bool skipFarePath(const FarePathType& farePathData, std::set<DateTime>& passedDates);
  void
  storeFarePathStatusForSkip(const FarePathType& farePathData, std::set<DateTime>& passedDates);
  void showDiag966Header();
  void showDiag966Footer();
  void showDiag966(uint16_t farePathCounter);

private:
  FlightFinderTrx* _fFTrx;
  std::map<DatePair, FlightFinderTrx::FlightBitInfo>* _dataPairsMap;
  Step _whichStep;
  FlightFinderTrx::OutBoundDateFlightMap _outboundDateflightMap;
  std::vector<TravelSeg*> _fareMarketSegBackup;
  const std::string frcPUErrorMsg = "FARE RETAILER CODE MISMATCH";
  const std::string frcFPErrorMsg = "FARE RETAILER CODE SOURCE PCC MISMATCH";
};

} // tse

