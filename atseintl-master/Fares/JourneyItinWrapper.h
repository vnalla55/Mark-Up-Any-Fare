//-------------------------------------------------------------------
//
//  File:        JourneyItinWrapper.h
//  Created:     Nov 02, 2011
//
//  Description: Wrapper class for journey itin used in FVO

//  Copyright Sabre 2011
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

#include "Common/TseEnums.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{

class FareMarket;
class Itin;
class TravelSeg;

class JourneyItinWrapper
{
public:
  JourneyItinWrapper(ShoppingTrx& trx,
                     FareMarket* fareMarket,
                     ShoppingTrx::Leg& leg,
                     const uint32_t legId,
                     const uint32_t carrierKey,
                     Itin* journeyItin = nullptr);

  ~JourneyItinWrapper();

  bool applySegments();
  bool applySegments(const Itin* itin, const uint32_t bitIndex);
  void eraseSegments();

  Itin& getJourneyItin() { return _journeyItin; }
  FareMarket* getFareMarket() { return _fareMarket; }
  uint32_t getCarrierKey() const { return _carrierKey; }
  uint32_t getLegId() const { return _legId; }
  ShoppingTrx::Leg& getLeg() { return _leg; }

  uint64_t getDuration() const { return _duration; }

private:
  bool initializeSol(const std::vector<TravelSeg*>& sourceSegments);
  bool initializeNormal();

  bool applySegmentsSol(const Itin* itin, const uint32_t bitIndex);
  bool applySegmentsNormal(const Itin* itin);
  TravelSeg* createOpenSegment(const TravelSeg* orig, const TravelSeg* dest);
  void setupTravelSegmentCarrier();

  TravelSeg* createOriginOpenSegment(const TravelSeg* orig, const TravelSeg* dest);
  TravelSeg* createDestinationOpenSegment(const TravelSeg* orig, const TravelSeg* dest);

private:
  bool _initialized = false;
  bool _solProcessingEnabled;
  Itin _journeyItin;
  FareMarket* _fareMarket;
  uint32_t _legId;
  uint32_t _carrierKey;
  uint32_t _segmentLegBeginIndex;
  uint32_t _segmentLegEndIndex;
  uint32_t _segmentInsertIndexStart;
  uint32_t _insertedSegmentsCount = 0;

  ShoppingTrx* _trx;
  ShoppingTrx::Leg& _leg;
  DataHandle& _dataHandle;

  std::vector<std::vector<ClassOfService*>*> _fmBackupClassOfService;
  std::vector<TravelSeg*> _fmBackupSegments;
  GlobalDirection _fmBackupDirection = GlobalDirection::ZZ;
  uint64_t _duration = 0;
};
}

