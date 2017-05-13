//----------------------------------------------------------------------------
//  File:        ESVPQItem.cpp
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

#include "Pricing/ESVPQItem.h"

#include "Pricing/ESVPQ.h"

namespace tse
{
bool
ESVPQItem::
operator==(const ESVPQItem& pqItem)
{
  return (_sopCombinationId == pqItem.sopCombinationId());
}

void
ESVPQItem::init(ESVPQ* pq, ESVPQItemWrapper* esvPqItemWrapper)
{
  _pq = pq;
  _qType = _pq->queueType();
  countSopData(esvPqItemWrapper);
}

void
ESVPQItem::getNumberOfSegs(int& outboundSegs, int& inboundSegs) const
{
  outboundSegs = _outSopWrapper->sop()->itin()->travelSeg().size();
  inboundSegs = 0;

  if (_inSopWrapper != nullptr)
  {
    inboundSegs = _inSopWrapper->sop()->itin()->travelSeg().size();
  }
}

int
ESVPQItem::getNumberOfSegs() const
{
  int outSegsCount = 0;
  int inSegsCount = 0;

  getNumberOfSegs(outSegsCount, inSegsCount);

  return (outSegsCount + inSegsCount);
}

bool
ESVPQItem::checkDiversityLimits(ESVPQDiversifier* esvDiv,
                                QueueType qType,
                                CarrierCode& carrier,
                                std::string& addInfo)
{
  bool result = true;
  carrier = "";
  std::vector<ShoppingTrx::SchedulingOption*> sopVec;

  sopVec.push_back(_outSopWrapper->sop());

  if (_inSopWrapper != nullptr)
  {
    sopVec.push_back(_inSopWrapper->sop());
  }

  if ((qType == MPRemainingOnline) || (qType == MPRemaining))
  {
    result = esvDiv->processItinMustPriceFlightsAndCarriersLimits(sopVec, this->isOnline());
  }

  if ((qType == LFSOnline) || (qType == LFSOnlineByCarrier))
  {
    result = esvDiv->processItinLFSFlightsAndCarriersOnlineLimits(sopVec, carrier);
  }

  if ((qType == LFSRemaining))
  {
    result = esvDiv->processItinLFSFlightsAndCarriersLimits(sopVec, carrier);
  }

  if (!result)
  {
    addInfo = "DIV7";
  }

  return result;
}

bool
ESVPQItem::checkUpperBoundLimits(ESVPQDiversifier* esvDiv,
                                 MoneyAmount minFareValue,
                                 QueueType qType,
                                 std::string& addInfo)
{
  bool result = true;

  if ((qType == MPNonstopOnline) || (qType == MPOutNonstopOnline) ||
      (qType == MPNonstopInterline) || (qType == MPOutNonstopInterline))
  {
    result = (this->totalAmt() < esvDiv->calcMustPriceUpperBound(minFareValue, true));
  }

  if ((qType == MPSingleStopOnline) || (qType == MPRemainingOnline) || (qType == MPRemaining))
  {
    result = (this->totalAmt() < esvDiv->calcMustPriceUpperBound(minFareValue, false));
  }

  if ((qType == LFSOnline) || (qType == LFSOnlineByCarrier) || (qType == LFSRemaining))
  {
    result = (this->totalAmt() < esvDiv->calcLFSUpperBound(minFareValue));
  }

  if (!result)
  {
    addInfo = "DIV8";
  }

  return result;
}

const bool
ESVPQItem::isJcb() const
{
  bool isJcb = _outSopWrapper->sop()->itin()->isJcb();

  if (_inSopWrapper != nullptr)
  {
    isJcb = isJcb && _inSopWrapper->sop()->itin()->isJcb();
  }

  return isJcb;
}

const bool
ESVPQItem::isOnline() const
{
  bool isOnline = (!_outSopWrapper->sop()->itin()->onlineCarrier().empty());

  if (_inSopWrapper != nullptr)
  {
    isOnline = isOnline && (_outSopWrapper->sop()->itin()->onlineCarrier() ==
                            _inSopWrapper->sop()->itin()->onlineCarrier());
  }

  return isOnline;
}

void
ESVPQItem::countSopData(ESVPQItemWrapper* esvPqItemWrapper)
{
  int colNum = _pq->_legVec.size();

  for (int col = 0; col < colNum; ++col)
  {
    const std::vector<ESVSopWrapper*>& sopWrapperVec = *(_pq->_legVec[col]);
    int sopIdx = esvPqItemWrapper->indices()[col];
    ESVSopWrapper* sopWrapper = sopWrapperVec[sopIdx];

    if (0 == col)
    {
      _outSopWrapper = sopWrapper;
      _sopCombinationId += sopWrapper->sop()->originalSopId();
    }
    else
    {
      _inSopWrapper = sopWrapper;
      _sopCombinationId += 1000 * sopWrapper->sop()->originalSopId();
    }

    _totalPrice += sopWrapper->getTotalAmt();
    _totalPenalty += sopWrapper->sop()->itin()->totalPenalty();
  }

  _totalAmt = esvPqItemWrapper->totalAmt();
}

std::pair<ESVSopWrapper::SOPFareListType, ESVSopWrapper::SOPFareListType>
ESVPQItem::getTypes() const
{
  std::pair<ESVSopWrapper::SOPFareListType, ESVSopWrapper::SOPFareListType> p;

  p.first = _outSopWrapper->getSopFareListType();

  if (nullptr != _inSopWrapper)
  {
    p.second = _inSopWrapper->getSopFareListType();
  }
  else
  {
    p.second = _outSopWrapper->getSopFareListType();
  }

  return p;
}

int
ESVPQItem::getNumStops(const int legIdx) const
{
  ShoppingTrx::SchedulingOption* sop = (0 == legIdx) ? _outSopWrapper->sop() : _inSopWrapper->sop();
  return sop->itin()->travelSeg().size() - 1;
}

int
ESVPQItem::getFlightTimeMinutes(const int legIdx) const
{
  const Itin* const itin =
      (0 == legIdx) ? _outSopWrapper->sop()->itin() : _inSopWrapper->sop()->itin();
  return itin->getFlightTimeMinutes();
}

DateTime
ESVPQItem::getDepartTime(const int legIdx) const
{
  return (0 == legIdx) ? _outSopWrapper->sop()->itin()->getDepartTime()
                       : _inSopWrapper->sop()->itin()->getDepartTime();
}

DateTime
ESVPQItem::getArrivalTime(const int legIdx) const
{
  return (0 == legIdx) ? _outSopWrapper->sop()->itin()->getArrivalTime()
                       : _inSopWrapper->sop()->itin()->getArrivalTime();
}

CarrierCode
ESVPQItem::getFirstCarrier(const int legIdx) const
{
  const ShoppingTrx::SchedulingOption* sop =
      (0 == legIdx) ? _outSopWrapper->sop() : _inSopWrapper->sop();
  const AirSeg* as = dynamic_cast<const AirSeg*>(sop->itin()->firstTravelSeg());
  return as->carrier();
}

CarrierCode
ESVPQItem::getLastCarrier(const int legIdx) const
{
  ShoppingTrx::SchedulingOption* sop = (0 == legIdx) ? _outSopWrapper->sop() : _inSopWrapper->sop();
  const AirSeg* as = dynamic_cast<const AirSeg*>(sop->itin()->lastTravelSeg());
  return as->carrier();
}

} // tse
