//----------------------------------------------------------------------------
//  File:        ESVSolution.h
//  Created:     2008-04-16
//
//  Description: This file contain data structures used to build output of ESV
//
//  Updates:
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/CabinType.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingUnit.h"

#include <vector>

namespace tse
{
class TravelSeg;

enum QueueType
{
  // MUST PRICE
  MPNonstopOnline, // pass 1
  MPOutNonstopOnline, // pass 1.5
  MPNonstopInterline, // pass 2
  MPOutNonstopInterline, // pass 2.5
  MPSingleStopOnline, // pass 3
  MPRemainingOnline, // pass 4
  MPRemaining, // pass 5
  // LOW FARE SEARCH
  LFSOnline, // pass 1
  LFSOnlineByCarrier, // pass 2
  LFSRemaining, // pass 3
  UnknownQueueType
};

class ESVSegmentInfo final
{
public:
  TravelSeg*& travelSeg() { return _travelSeg; }
  const TravelSeg* travelSeg() const { return _travelSeg; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  CabinType& bookedCabin() { return _bookedCabin; }
  const CabinType& bookedCabin() const { return _bookedCabin; }

private:
  TravelSeg* _travelSeg = nullptr;
  BookingCode _bookingCode = " ";
  CabinType _bookedCabin;
};

class ESVFareComponent final
{
public:
  PaxTypeFare*& paxTypeFare() { return _paxTypeFare; }
  const PaxTypeFare* paxTypeFare() const { return _paxTypeFare; }

  std::string& fareBasisCode() { return _fareBasisCode; }
  const std::string& fareBasisCode() const { return _fareBasisCode; }

  std::vector<ESVSegmentInfo*>& esvSegmentInfoVec() { return _esvSegmentInfoVec; }

  const std::vector<ESVSegmentInfo*>& esvSegmentInfoVec() const { return _esvSegmentInfoVec; }

protected:
  PaxTypeFare* _paxTypeFare = nullptr;
  std::string _fareBasisCode = " ";
  std::vector<ESVSegmentInfo*> _esvSegmentInfoVec;
};

class ESVOption final
{

public:
  int& sopId() { return _sopId; }
  const int& sopId() const { return _sopId; }

  std::vector<ESVFareComponent*>& esvFareComponentsVec() { return _esvFareComponentsVec; }

  const std::vector<ESVFareComponent*>& esvFareComponentsVec() const
  {
    return _esvFareComponentsVec;
  }

private:
  int _sopId = -1;
  std::vector<ESVFareComponent*> _esvFareComponentsVec;
};

class ESVSolution final
{
public:
  ESVOption& outboundOption() { return _outboundOption; }
  const ESVOption& outboundOption() const { return _outboundOption; }

  ESVOption& inboundOption() { return _inboundOption; }
  const ESVOption& inboundOption() const { return _inboundOption; }

  int& groupId() { return _groupId; }
  const int& groupId() const { return _groupId; }

  int& carrierGroupId() { return _carrierGroupId; }
  const int& carrierGroupId() const { return _carrierGroupId; }

  MoneyAmount& totalAmt() { return _totalAmt; }
  const MoneyAmount& totalAmt() const { return _totalAmt; }

  MoneyAmount& totalPrice() { return _totalPrice; }
  const MoneyAmount& totalPrice() const { return _totalPrice; }

  MoneyAmount& totalPenalty() { return _totalPenalty; }
  const MoneyAmount& totalPenalty() const { return _totalPenalty; }

  bool& isJcb() { return _isJcb; }
  const bool& isJcb() const { return _isJcb; }

  PricingUnit::Type& pricingUnitType() { return _pricingUnitType; }
  PricingUnit::Type pricingUnitType() const { return _pricingUnitType; }

  QueueType& queueType() { return _queueType; }
  const QueueType& queueType() const { return _queueType; }

  int& numberOfSegments() { return _numberOfSegments; }
  const int& numberOfSegments() const { return _numberOfSegments; }

private:
  ESVOption _outboundOption;
  ESVOption _inboundOption;
  int _groupId = -1;
  int _carrierGroupId = -1;
  MoneyAmount _totalAmt = 0;
  MoneyAmount _totalPrice = 0;
  MoneyAmount _totalPenalty = 0;
  bool _isJcb = false;
  PricingUnit::Type _pricingUnitType = PricingUnit::Type::UNKNOWN;
  QueueType _queueType = QueueType::UnknownQueueType;
  int _numberOfSegments = -1;
};
} // End namespace tse
