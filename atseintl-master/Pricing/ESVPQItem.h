//----------------------------------------------------------------------------
//  File:        ESVPQItem.h
//  Created:     2009-06-26
//
//  Description: ESV priority queue item class
//
//  Updates:
//
//  Copyright Sabre 2009
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

#include "Pricing/EstimatedSeatValue.h"

namespace tse
{
class ESVSopWrapper;

class ESVPQItem final
{
  friend class ESVPQ;

public:
  bool
  checkDiversityLimits(ESVPQDiversifier* esvDiv, QueueType, CarrierCode& carrier, std::string&);

  bool
  checkUpperBoundLimits(ESVPQDiversifier* esvDiv, MoneyAmount minFare, QueueType, std::string&);

  bool operator==(const ESVPQItem& pqItem);

  const bool isOnline() const;

  const bool isJcb() const;

  std::pair<ESVSopWrapper::SOPFareListType, ESVSopWrapper::SOPFareListType> getTypes() const;

  ESVSopWrapper*& outSopWrapper() { return _outSopWrapper; }
  const ESVSopWrapper* outSopWrapper() const { return _outSopWrapper; }

  ESVSopWrapper*& inSopWrapper() { return _inSopWrapper; }
  const ESVSopWrapper* inSopWrapper() const { return _inSopWrapper; }

  MoneyAmount& totalAmt() { return _totalAmt; }
  const MoneyAmount& totalAmt() const { return _totalAmt; }

  MoneyAmount& totalPrice() { return _totalPrice; }
  const MoneyAmount& totalPrice() const { return _totalPrice; }

  MoneyAmount& totalPenalty() { return _totalPenalty; }
  const MoneyAmount& totalPenalty() const { return _totalPenalty; }

  bool& isPrimarySolution() { return _isPrimarySolution; }
  const bool& isPrimarySolution() const { return _isPrimarySolution; }

  int& groupId() { return _groupId; }
  const int& groupId() const { return _groupId; }

  int& carrierGroupId() { return _carrierGroupId; }
  const int& carrierGroupId() const { return _carrierGroupId; }

  QueueType& queueType() { return _qType; }
  const QueueType& queueType() const { return _qType; }

  CarrierCode getFirstCarrier(const int legIdx) const;
  CarrierCode getLastCarrier(const int legIdx) const;

  DateTime getDepartTime(const int legIdx) const;
  DateTime getArrivalTime(const int legIdx) const;

  int getFlightTimeMinutes(const int legIdx) const;

  int getNumStops(const int legIdx) const;

  bool& isSelected() { return _isSelected; }
  const bool& isSelected() const { return _isSelected; }

  std::string& selectionSource() { return _selectionSource; }
  const std::string& selectionSource() const { return _selectionSource; }

  int& priority() { return _priority; }
  int priority() const { return _priority; }

  double& utility(uint32_t id = 0)
  {
    return (0 == id) ? _outSopWrapper->outboundUtility() : _inboundUtility;
  }

  const double getUtility(uint32_t id = 0) const
  {
    return (0 == id) ? _outSopWrapper->getOutboundUtility() : _inboundUtility;
  }

  double& itinUtility() { return _itinUtility; }
  const double& itinUtility() const { return _itinUtility; }

  const int nestID(uint32_t id = 0) const
  {
    return (0 == id) ? _outSopWrapper->nestID() : _inSopWrapper->nestID();
  }

  double& IV() { return _IV; }
  const double& IV() const { return _IV; }

  int& selectionOrder() { return _selectionOrder; }
  const int& selectionOrder() const { return _selectionOrder; }

  int& screenID() { return _screenID; }
  const int& screenID() const { return _screenID; }

  uint32_t& sopCombinationId() { return _sopCombinationId; }
  const uint32_t& sopCombinationId() const { return _sopCombinationId; }

  ESVPQ* pq() const { return _pq; }

  void getNumberOfSegs(int& outboundSegs, int& inboundSegs) const;

  int getNumberOfSegs() const;

private:
  void countSopData(ESVPQItemWrapper* esvPqItemWrapper);

  void init(ESVPQ* pq, ESVPQItemWrapper* esvPqItemWrapper);

  ESVPQ* _pq = nullptr;
  ESVSopWrapper* _outSopWrapper = nullptr;
  ESVSopWrapper* _inSopWrapper = nullptr;
  MoneyAmount _totalAmt = 0.0;
  MoneyAmount _totalPrice = 0.0;
  MoneyAmount _totalPenalty = 0.0;
  bool _isPrimarySolution = true;
  int _groupId = -1;
  int _carrierGroupId = 0;
  QueueType _qType = QueueType::UnknownQueueType;
  bool _isSelected = false;
  std::string _selectionSource;
  int _priority = 99;
  double _inboundUtility = 0.0;
  double _itinUtility = 0.0;
  double _IV = 0.0;
  int _selectionOrder = 0;
  int _screenID = 0;
  uint32_t _sopCombinationId = 0; // Created on the base of original sop ids
};
} // End namespace tse
