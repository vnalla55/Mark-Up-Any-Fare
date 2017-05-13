//----------------------------------------------------------------------------
//
//  File   :  BCETuning.h
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//-----------------------------------------------------------------------

#pragma once
#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class AirSeg;

class BCESegmentProcessed
{
public:
  enum SegmentNoMatchReason
  { DEFAULT_VALUE = 1,
    CARRIER,
    PRIMARY_SECONDARY,
    FLIGHT,
    EQUIPMENT,
    PORTION_OF_TRAVEL,
    TSI,
    LOCATION,
    POINT_OF_SALE,
    SOLD_TAG,
    DATE_TIME_DOW };

  const AirSeg* _airSeg = nullptr;
  uint16_t _segmentNumber = 0;
  char _ifTag = '0';
  bool _noMatchSegment = false;
  bool _segmentProcessedOnce = false;
  FareClassCode _fareProcessedSegment;
  SegmentNoMatchReason _reasonNoMatch = SegmentNoMatchReason::DEFAULT_VALUE;
};

class BCESequenceProcessed
{
public:
  uint32_t _sequenceNumber = 0;
  bool _noMatchSequence = false;
  bool _seqProcessedOnce = false;
  FareClassCode _fareProcessedSeq;
  std::vector<BCESegmentProcessed> _segmentsProcessed;
};

class BCETuning
{
public:
  uint32_t _itemNo = 0;
  Indicator _convNum = '0';
  const AirSeg* _airSeg = nullptr;
  FareClassCode _fareClass;
  uint16_t _numRepeat = 1;
  std::vector<BCESequenceProcessed> _sequencesProcessed;
};
}
