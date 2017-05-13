//----------------------------------------------------------------------------
//  File:        ESVPQ.cpp
//  Created:     2009-05-25
//
//  Description: ESV priority queue
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

#include "Pricing/ESVPQ.h"

#include "DataModel/InterlineTicketCarrierData.h"
#include "Diagnostic/Diag956Collector.h"
#include "Pricing/ESVPQItem.h"

namespace tse
{
ESVPQ::ESVPQ(ShoppingTrx* trx, std::string title)
  : _title(title),
    _trx(trx),
    _logger(EstimatedSeatValue::_logger),
    _paxType(trx->paxType()[0]),
    _pqItemPQ(_pqItemComp)
{
  trx->dataHandle().get(_interlineTicketCarrier);
}

void
ESVPQ::init(const std::vector<ShoppingTrx::SchedulingOption*>& outSopVec,
            const std::vector<ShoppingTrx::SchedulingOption*>& inSopVec,
            QueueType qType,
            const CarrierCode& carrier,
            const std::string& diversityOption)
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::init())");

  clearInternalData();

  _carrier = carrier;
  _diversityOption = diversityOption;
  _queueType = qType;

  setupAddPenaltyFlag();

  initLegVec(outSopVec, inSopVec);

  pushFirstItemIntoPQ();
}

void
ESVPQ::init(const std::vector<ShoppingTrx::SchedulingOption*>& outSopVec,
            const std::vector<ESVSopWrapper*>& inSopWrapVec,
            QueueType qType,
            const CarrierCode& carrier,
            const std::string& diversityOption)
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::init())");

  clearInternalData();

  _carrier = carrier;
  _diversityOption = diversityOption;
  _queueType = qType;

  setupAddPenaltyFlag();

  initLegVec(outSopVec, inSopWrapVec);

  pushFirstItemIntoPQ();
}

void
ESVPQ::init(const std::vector<ESVSopWrapper*>& outSopWrapVec,
            const std::vector<ShoppingTrx::SchedulingOption*>& inSopVec,
            QueueType qType,
            const CarrierCode& carrier,
            const std::string& diversityOption)
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::init())");

  clearInternalData();

  _carrier = carrier;
  _diversityOption = diversityOption;
  _queueType = qType;

  setupAddPenaltyFlag();

  initLegVec(outSopWrapVec, inSopVec);

  pushFirstItemIntoPQ();
}

void
ESVPQ::init(const std::vector<ESVSopWrapper*>& outSopWrapVec,
            const std::vector<ESVSopWrapper*>& inSopWrapVec,
            QueueType qType,
            const CarrierCode& carrier,
            const std::string& diversityOption)
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::init())");

  clearInternalData();

  _carrier = carrier;
  _diversityOption = diversityOption;
  _queueType = qType;

  setupAddPenaltyFlag();

  initLegVec(outSopWrapVec, inSopWrapVec);

  pushFirstItemIntoPQ();
}

void
ESVPQ::setupAddPenaltyFlag()
{
  if ((_queueType == LFSOnline) || (_queueType == LFSOnlineByCarrier) ||
      (_queueType == LFSRemaining))
  {
    _addPenalty = false;
  }
  else
  {
    _addPenalty = true;
  }
}

ESVPQItem*
ESVPQ::getNextItem(Diag956Collector* diag956Collector, const bool oneItemOnly)
{
  ESVPQItemWrapper* pqItemWrapper = nullptr;
  bool pqItemValid = false;
  std::string* addInfoPtr = nullptr;

  if (diag956Collector && diag956Collector->isActive())
  {
    _addInfo = "";
    addInfoPtr = &_addInfo;
  }

  while ((!_pqItemPQ.empty()) && (!pqItemValid))
  {
    pqItemWrapper = _pqItemPQ.top();
    _pqItemPQ.pop();

    if (nullptr != pqItemWrapper->esvPqItem())
    {
      pqItemValid = validPU(pqItemWrapper->esvPqItem(), addInfoPtr);

      if (addInfoPtr)
      {
        diag956Collector->displayESVPQItem(pqItemWrapper->esvPqItem(), _addInfo, true, pqItemValid);
        diag956Collector->flushMsg();
        _addInfo = "";
      }
    }
    else
    {
      pqItemValid = false;
    }

    if (!oneItemOnly || !pqItemValid)
      generateNextPQItemSet(pqItemWrapper);
  }

  return pqItemValid ? pqItemWrapper->esvPqItem() : nullptr;
}

bool
ESVPQ::validPU(ESVPQItem* pqItem, std::string* addInfoPtr)
{
  if (alreadyCreated(pqItem->sopCombinationId(), addInfoPtr))
    return false;

  if (!checkMinimumConnectTime(pqItem))
    return false;

  if (!checkCat10(pqItem))
    return false;

  addToCreated(pqItem->sopCombinationId(), addInfoPtr);

  return true;
}

bool
ESVPQ::checkMinimumConnectTime(const ESVPQItem* pqItem) const
{
  const ESVSopWrapper* outSopWrapper = pqItem->outSopWrapper();
  const ESVSopWrapper* inSopWrapper = pqItem->inSopWrapper();

  if (!inSopWrapper)
    return true;

  const TravelSeg* lastOutSeg = outSopWrapper->sop()->itin()->travelSeg().back();
  const TravelSeg* firstInSeg = inSopWrapper->sop()->itin()->travelSeg().front();

  if (lastOutSeg->arrivalDT() > firstInSeg->departureDT())
    return false;

  if (_trx->getOptions()->getMinConnectionTimeDomestic() > 0)
  {
    return ShoppingUtil::checkMinConnectionTime(
        lastOutSeg, firstInSeg, _trx->getOptions()->getMinConnectionTimeDomestic());
  }
  else
  {
    return true;
  }
}

bool
ESVPQ::checkCat10(ESVPQItem* pqItem)
{
  _sopWprapperOut = pqItem->outSopWrapper();
  _sopWprapperIn = pqItem->inSopWrapper();

  if (!isOnline())
  {
    if (haveCat10Restrictions())
      return false;

    if (_sopWprapperIn == nullptr)
    {
      if (!EstimatedSeatValue::isValidInterline(
              *_trx, this->interlineTicketCarrier(), _sopWprapperOut->sop()->itin()))
        return false;
    }
    else
    {
      if (!EstimatedSeatValue::isValidInterline(*_trx,
                                                this->interlineTicketCarrier(),
                                                _sopWprapperOut->sop()->itin(),
                                                _sopWprapperIn->sop()->itin()))
        return false;
    }
  }

  switch (_sopWprapperOut->getSopFareListType())
  {
  case ESVSopWrapper::OW:

    if (_sopWprapperIn != nullptr)
    {
      return checkONEWAY();
    }
    else
    {
      return true;
    }

    break;

  case ESVSopWrapper::RT:

    if (_sopWprapperIn != nullptr)
    {
      return checkROUNDTRIP();
    }

    break;

  case ESVSopWrapper::CT:

    if (_sopWprapperIn != nullptr)
    {
      return checkCIRCLETRIP();
    }

    break;

  case ESVSopWrapper::OJ:

    if (_sopWprapperIn != nullptr)
    {
      return checkOPENJAW();
    }

    break;

  default:
    return false;
  }

  return false;
}

bool
ESVPQ::haveCat10Restrictions()
{
  // Check if outbound or inbound part have cat 10 restrictions
  return (_sopWprapperOut->haveCat10Restrictions() ||
          ((_sopWprapperIn != nullptr) && _sopWprapperIn->haveCat10Restrictions()));
}

bool
ESVPQ::isOnline()
{
  // Check if outbound is online flight
  if ("" == _sopWprapperOut->sop()->itin()->onlineCarrier())
  {
    return false;
  }

  if (_sopWprapperIn != nullptr)
  {
    // Check if outbound and inbound have the same online carriers
    return (_sopWprapperOut->sop()->itin()->onlineCarrier() ==
            _sopWprapperIn->sop()->itin()->onlineCarrier());
  }

  return true;
}

bool
ESVPQ::checkONEWAY()
{
  // Check sop fare list type in inbound part
  return (ESVSopWrapper::OW == _sopWprapperIn->getSopFareListType());
}

bool
ESVPQ::checkROUNDTRIP()
{
  // Check sop fare list type in inbound part
  if (_sopWprapperIn->getSopFareListType() != ESVSopWrapper::RT)
  {
    return false;
  }

  // Check if outbound and inbound itin have the same number of fares
  uint32_t paxTypeFaresCount = _sopWprapperOut->paxTypeFareVec()->size();

  if (paxTypeFaresCount != _sopWprapperIn->paxTypeFareVec()->size())
  {
    return false;
  }

  // Check if we've got the same connection cities on each fare break point
  bool result = true;
  uint32_t i = 0;

  while (result && (i < paxTypeFaresCount))
  {
    PaxTypeFare* paxTypeFareOut = _sopWprapperOut->paxTypeFareVec()->operator[](i);
    PaxTypeFare* paxTypeFareIn =
        _sopWprapperIn->paxTypeFareVec()->operator[](paxTypeFaresCount - 1 - i);

    result = ((paxTypeFareOut->fareMarket()->origin()->loc() ==
               paxTypeFareIn->fareMarket()->destination()->loc()) &&
              (paxTypeFareOut->fareMarket()->destination()->loc() ==
               paxTypeFareIn->fareMarket()->origin()->loc()));

    ++i;
  }

  return result;
}

bool
ESVPQ::checkCIRCLETRIP()
{
  // Check sop fare list type in inbound part
  if (_sopWprapperIn->getSopFareListType() != ESVSopWrapper::CT)
  {
    return false;
  }

  int faresCount =
      _sopWprapperOut->paxTypeFareVec()->size() + _sopWprapperIn->paxTypeFareVec()->size();

  if (faresCount < 3)
  {
    return false;
  }

  if (4 == faresCount)
  {
    if (_sopWprapperOut->paxTypeFareVec()->operator[](0)->fareMarket()->destination()->loc() ==
        _sopWprapperIn->paxTypeFareVec()->operator[](0)->fareMarket()->destination()->loc())
    {
      return false;
    }
  }

  return true;
}

bool
ESVPQ::checkOPENJAW()
{
  // Check sop fare list type in inbound part
  if (_sopWprapperIn->getSopFareListType() != ESVSopWrapper::OJ)
  {
    return false;
  }

  // fares num, mileage, fare type combination
  const int faresCount =
      _sopWprapperOut->paxTypeFareVec()->size() + _sopWprapperIn->paxTypeFareVec()->size();

  if (3 == faresCount)
  {
    return check3componentOJ(*(_sopWprapperOut->paxTypeFareVec()),
                             *(_sopWprapperIn->paxTypeFareVec()),
                             _sopWprapperOut->getCombinationType(),
                             _sopWprapperIn->getCombinationType());
  }

  if (4 == faresCount)
  {
    return check4componentOJ(*(_sopWprapperOut->paxTypeFareVec()),
                             *(_sopWprapperIn->paxTypeFareVec()),
                             _sopWprapperOut->getCombinationType(),
                             _sopWprapperIn->getCombinationType());
  }

  return false;
}

bool
ESVPQ::check3componentOJ(const std::vector<PaxTypeFare*>& outPaxTypeVec,
                         const std::vector<PaxTypeFare*>& inPaxTypeVec,
                         const SOPFarePath::CombinationType outCombType,
                         const SOPFarePath::CombinationType inCombType)
{
  // 3 fares (oj0, oj1, conn)

  // R+XX (out0, in0, in1) or (out0, in1, in0)
  // R+RX (out0, in0, in1)
  // R+XR (out0, in1, in0)
  // X+RX (out0, in0, in1)
  // X+XR (out0, in1, in0)

  // XX+R (out0, in0, out1) or (out1, in0, out0)
  // RX+R (out0, in0, out1)
  // XR+R (out1, in0, out0)
  // RX+X (out0, in0, out1)
  // XR+X (out1, in0, out0)

  if (outCombType == SOPFarePath::R && inCombType == SOPFarePath::XX)
  {
    if (!checkMileage(outPaxTypeVec[0], inPaxTypeVec[0], inPaxTypeVec[1]))
      if (!checkMileage(outPaxTypeVec[0], inPaxTypeVec[1], inPaxTypeVec[0]))
        return false;
  }
  else if (outCombType == SOPFarePath::R && inCombType == SOPFarePath::RX)
  {
    if (!checkMileage(outPaxTypeVec[0], inPaxTypeVec[0], inPaxTypeVec[1]))
      return false;
  }
  else if (outCombType == SOPFarePath::R && inCombType == SOPFarePath::XR)
  {
    if (!checkMileage(outPaxTypeVec[0], inPaxTypeVec[1], inPaxTypeVec[0]))
      return false;
  }
  else if (outCombType == SOPFarePath::X && inCombType == SOPFarePath::RX)
  {
    if (!checkMileage(outPaxTypeVec[0], inPaxTypeVec[0], inPaxTypeVec[1]))
      return false;
  }
  else if (outCombType == SOPFarePath::X && inCombType == SOPFarePath::XR)
  {
    if (!checkMileage(outPaxTypeVec[0], inPaxTypeVec[1], inPaxTypeVec[0]))
      return false;
  }
  else if (outCombType == SOPFarePath::XX && inCombType == SOPFarePath::R)
  {
    if (!checkMileage(outPaxTypeVec[0], inPaxTypeVec[0], outPaxTypeVec[1]))
      if (!checkMileage(outPaxTypeVec[1], inPaxTypeVec[0], outPaxTypeVec[0]))
        return false;
  }
  else if (outCombType == SOPFarePath::RX && inCombType == SOPFarePath::R)
  {
    if (!checkMileage(outPaxTypeVec[0], inPaxTypeVec[0], outPaxTypeVec[1]))
      return false;
  }
  else if (outCombType == SOPFarePath::XR && inCombType == SOPFarePath::R)
  {
    if (!checkMileage(outPaxTypeVec[1], inPaxTypeVec[0], outPaxTypeVec[0]))
      return false;
  }
  else if (outCombType == SOPFarePath::RX && inCombType == SOPFarePath::X)
  {
    if (!checkMileage(outPaxTypeVec[0], inPaxTypeVec[0], outPaxTypeVec[1]))
      return false;
  }
  else if (outCombType == SOPFarePath::XR && inCombType == SOPFarePath::X)
  {
    if (!checkMileage(outPaxTypeVec[1], inPaxTypeVec[0], outPaxTypeVec[0]))
      return false;
  }
  else // not supported combination
    return false;

  return true;
}

bool
ESVPQ::check4componentOJ(const std::vector<PaxTypeFare*>& outPaxTypeVec,
                         const std::vector<PaxTypeFare*>& inPaxTypeVec,
                         const SOPFarePath::CombinationType outCombType,
                         const SOPFarePath::CombinationType inCombType)
{
  // RR+whatever
  // whatever+RR
  // RX+RX
  // XR+XR
  const Loc& connPoint1 = (*outPaxTypeVec[0]->fareMarket()->destination());
  const Loc& connPoint2 = (*inPaxTypeVec[0]->fareMarket()->destination());
  if (connPoint1.loc() == connPoint2.loc())
  {
    return false;
  }

  if (outCombType == SOPFarePath::RR || inCombType == SOPFarePath::RR ||
      (outCombType == SOPFarePath::RX && inCombType == SOPFarePath::RX) ||
      (outCombType == SOPFarePath::XR && inCombType == SOPFarePath::XR))
  {
    const uint16_t connMileage =
        LocUtil::getTPM(connPoint1,
                        connPoint2,
                        outPaxTypeVec[0]->fareMarket()->getGlobalDirection(),
                        outPaxTypeVec[0]->fareMarket()->travelDate(),
                        _trx->dataHandle());
    if (!(checkMileage(outPaxTypeVec[0], inPaxTypeVec[1], connMileage) &&
          checkMileage(outPaxTypeVec[1], inPaxTypeVec[0], connMileage)))
      return false;
  }

  return true;
}

bool
ESVPQ::alreadyCreated(uint32_t& sopCombinationId, std::string* addInfoPtr)
{
  if (_alreadyCreatedSOPCombinations.find(sopCombinationId) != _alreadyCreatedSOPCombinations.end())
  {
    if (addInfoPtr)
    {
      *addInfoPtr += "G"; // Generated
    }

    return true;
  }
  else
  {
    return false;
  }
}

bool
ESVPQ::checkMileage(const PaxTypeFare* oj1, const PaxTypeFare* oj2, const PaxTypeFare* connFare)
{
  return checkMileage(oj1, oj2, mileage(connFare));
}

bool
ESVPQ::checkMileage(const PaxTypeFare* oj1, const PaxTypeFare* oj2, const uint16_t connMileage)
{
  if (mileage(oj1) > connMileage || mileage(oj2) > connMileage)
  {
    return true;
  }

  return false;
}

uint16_t
ESVPQ::mileage(const PaxTypeFare* fare)
{
  const FareMarket& fm = *(fare->fareMarket());
  const DateTime& travelDate = fm.travelDate();
  const Loc& orig = *fm.origin();
  const Loc& dest = *fm.destination();

  return LocUtil::getTPM(orig, dest, fm.getGlobalDirection(), travelDate, _trx->dataHandle());
}

void
ESVPQ::addToCreated(uint32_t& sopCombinationId, std::string* addInfoPtr)
{
  std::pair<std::set<uint32_t>::iterator, bool> insRes =
      _alreadyCreatedSOPCombinations.insert(sopCombinationId);

  if (insRes.second == false)
  {
    if (addInfoPtr)
    {
      *addInfoPtr += "ERROR: SOP COMBINATION IS ALREADY CREATED";
    }
  }
}

void
ESVPQ::generateNextPQItemSet(ESVPQItemWrapper* pqItemWrapper)
{
  std::vector<int>& indices = pqItemWrapper->indices();
  int colNum = _legVec.size();

  for (int col = pqItemWrapper->expandColumn(); col < colNum; ++col)
  {
    if (indices[col] + 1 >= int(_legVec[col]->size())) // it is last elem in SOP list
    {
      continue;
    }

    ESVPQItemWrapper* newPQItemWrapper = _trx->dataHandle().create<ESVPQItemWrapper>();
    newPQItemWrapper->init(col, indices);
    setTotalAmt(newPQItemWrapper);

    if (preValidateCat10(newPQItemWrapper))
    {
      ESVPQItem* newPQItem = _trx->dataHandle().create<ESVPQItem>();
      newPQItem->init(this, newPQItemWrapper);

      newPQItemWrapper->esvPqItem() = newPQItem;
    }

    _pqItemPQ.push(newPQItemWrapper);
  }
}

bool
ESVPQ::preValidateCat10(ESVPQItemWrapper* pqItemWrapper)
{
  ESVSopWrapper* outSopWrapper = nullptr;
  ESVSopWrapper* inSopWrapper = nullptr;

  int outSopIdx = pqItemWrapper->indices()[0];
  outSopWrapper = (_legVec[0])->at(outSopIdx);

  if (_legVec.size() > 1)
  {
    int inSopIdx = pqItemWrapper->indices()[1];
    inSopWrapper = (_legVec[1])->at(inSopIdx);

    if (outSopWrapper->getSopFareListType() != inSopWrapper->getSopFareListType())
    {
      return false;
    }
  }
  else
  {
    if (outSopWrapper->getSopFareListType() != ESVSopWrapper::OW)
    {
      return false;
    }
  }

  return true;
}

void
ESVPQ::setTotalAmt(ESVPQItemWrapper* pqItemWrapper)
{
  ESVSopWrapper* outSopWrapper = nullptr;
  ESVSopWrapper* inSopWrapper = nullptr;

  int outSopIdx = pqItemWrapper->indices()[0];
  outSopWrapper = (_legVec[0])->at(outSopIdx);

  pqItemWrapper->totalAmt() += outSopWrapper->getTotalAmt();

  if (_addPenalty)
  {
    pqItemWrapper->totalAmt() += outSopWrapper->sop()->itin()->totalPenalty();
  }

  if (_legVec.size() > 1)
  {
    int inSopIdx = pqItemWrapper->indices()[1];
    inSopWrapper = (_legVec[1])->at(inSopIdx);

    pqItemWrapper->totalAmt() += inSopWrapper->getTotalAmt();

    if (_addPenalty)
    {
      pqItemWrapper->totalAmt() += inSopWrapper->sop()->itin()->totalPenalty();
    }
  }
}

void
ESVPQ::initLegVec(const std::vector<ShoppingTrx::SchedulingOption*>& outSopVec,
                  const std::vector<ShoppingTrx::SchedulingOption*>& inSopVec)
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::initLegVec()");

  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator sopIter;
  std::vector<ShoppingTrx::SchedulingOption*> chosenOutSopVec;
  std::vector<ShoppingTrx::SchedulingOption*> chosenInSopVec;

  for (sopIter = outSopVec.begin(); sopIter != outSopVec.end(); ++sopIter)
  {
    if ((*sopIter)->getDummy())
    {
      continue;
    }

    chosenOutSopVec.push_back(*sopIter);
  }

  for (sopIter = inSopVec.begin(); sopIter != inSopVec.end(); ++sopIter)
  {
    if ((*sopIter)->getDummy())
    {
      continue;
    }

    chosenInSopVec.push_back(*sopIter);
  }

  fillInLegVec(chosenOutSopVec, chosenInSopVec);
}

void
ESVPQ::initLegVec(const std::vector<ShoppingTrx::SchedulingOption*>& outSopVec,
                  const std::vector<ESVSopWrapper*>& inSopWrapVec)
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::initLegVec()");

  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator sopIter;
  std::vector<ShoppingTrx::SchedulingOption*> chosenOutSopVec;

  for (sopIter = outSopVec.begin(); sopIter != outSopVec.end(); ++sopIter)
  {
    if ((*sopIter)->getDummy())
    {
      continue;
    }

    chosenOutSopVec.push_back(*sopIter);
  }

  expandVecInLegVec(chosenOutSopVec, 0);
  _legVec.push_back(&inSopWrapVec);
}

void
ESVPQ::initLegVec(const std::vector<ESVSopWrapper*>& outSopWrapVec,
                  const std::vector<ShoppingTrx::SchedulingOption*>& inbSopVec)
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::initLegVec()");

  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator sopIter;
  std::vector<ShoppingTrx::SchedulingOption*> chosenInbSopVec;

  for (sopIter = inbSopVec.begin(); sopIter != inbSopVec.end(); ++sopIter)
  {
    if ((*sopIter)->getDummy())
    {
      continue;
    }

    chosenInbSopVec.push_back(*sopIter);
  }

  _legVec.push_back(&outSopWrapVec);
  expandVecInLegVec(chosenInbSopVec, 1);
}

void
ESVPQ::initLegVec(const std::vector<ESVSopWrapper*>& outSopWrapVec,
                  const std::vector<ESVSopWrapper*>& inbSopWrapVec)
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::initLegVec()");

  _legVec.push_back(&outSopWrapVec);
  _legVec.push_back(&inbSopWrapVec);
}

void
ESVPQ::fillInLegVec(std::vector<ShoppingTrx::SchedulingOption*>& outVec,
                    std::vector<ShoppingTrx::SchedulingOption*>& inVec)
{
  expandVecInLegVec(outVec, 0);

  if (_trx->legs().size() > 1)
  {
    expandVecInLegVec(inVec, 1);
  }
}

void
ESVPQ::expandVecInLegVec(std::vector<ShoppingTrx::SchedulingOption*>& sopVec, const int legIdx)
{
  std::vector<ESVSopWrapper*> allSopWrapperVec;
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopVecIt = sopVec.begin();
  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopVecItEnd = sopVec.end();

  for (; sopVecIt != sopVecItEnd; ++sopVecIt)
  {
    expandSOP(allSopWrapperVec, *sopVecIt);
  }

  sortSopWrapperVec(allSopWrapperVec);

  if (legIdx == 0) // outbound
  {
    _legVecOut.swap(allSopWrapperVec);
    _legVec.push_back(&_legVecOut);
  }
  else // inbound
  {
    _legVecIn.swap(allSopWrapperVec);
    _legVec.push_back(&_legVecIn);
  }
}

void
ESVPQ::expandSOP(std::vector<ESVSopWrapper*>& allSopWrapperVec, ShoppingTrx::SchedulingOption* sop)
{
  SOPFareList& sopFareList = sop->itin()->paxTypeSOPFareListMap()[_paxType];
  std::vector<ESVSopWrapper*>& sopWrapperVec = sop->itin()->sopWrapperVec();
  if (sopWrapperVec.size() == 0)
  {
    expandSopFarePaths(sopWrapperVec, sop, sopFareList.owSopFarePaths(), ESVSopWrapper::OW);
    expandSopFarePaths(sopWrapperVec, sop, sopFareList.rtSopFarePaths(), ESVSopWrapper::RT);
    expandSopFarePaths(sopWrapperVec, sop, sopFareList.ctSopFarePaths(), ESVSopWrapper::CT);
    expandSopFarePaths(sopWrapperVec, sop, sopFareList.ojSopFarePaths(), ESVSopWrapper::OJ);
    SOPWrapperCompare sopWrapperComp;
    std::sort(sopWrapperVec.begin(), sopWrapperVec.end(), sopWrapperComp);
  }
  std::copy(sopWrapperVec.begin(), sopWrapperVec.end(), std::back_inserter(allSopWrapperVec));
}

void
ESVPQ::expandSopFarePaths(std::vector<ESVSopWrapper*>& sopWrapperVec,
                          ShoppingTrx::SchedulingOption* sop,
                          std::vector<SOPFarePath*>& sopFarePathVec,
                          ESVSopWrapper::SOPFareListType sopFareListType)
{
  int sopFarePathsNo = sopFarePathVec.size();

  for (int i = 0; i < sopFarePathsNo; ++i)
  {
    ESVSopWrapper* sopWrapper = _trx->dataHandle().create<ESVSopWrapper>();
    SOPFarePath* sopFarePath = sopFarePathVec[i];
    sopWrapper->init(
        _paxType, sop, sopFareListType, i, &sopFarePath->paxTypeFareVec(), sopFarePath);
    sopWrapperVec.push_back(sopWrapper);
  }
}

void
ESVPQ::sortSopWrapperVec(std::vector<ESVSopWrapper*>& sopWrapperVec)
{
  SOPWrapperCompare sopWrapperComp;
  std::sort(sopWrapperVec.begin(), sopWrapperVec.end(), sopWrapperComp);
}

void
ESVPQ::pushFirstItemIntoPQ()
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::pushFirstItemIntoPQ()");

  if (allLegsNotEmpty())
  {
    _pqItemPQ.push(generateFirstItem());
  }
}

bool
ESVPQ::allLegsNotEmpty()
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::allLegsNotEmpty()");

  int legSize = _legVec.size();
  bool result = (legSize != 0);
  int i = 0;

  while (result && (i < legSize))
  {
    if (_legVec[i]->empty())
    {
      result = false;
    }
    else
    {
      ++i;
    }
  }

  return result;
}

ESVPQItemWrapper*
ESVPQ::generateFirstItem()
{
  LOG4CXX_DEBUG(_logger, "ESVPQ::generateFirstItem()");

  int legSize = _trx->legs().size();
  std::vector<int> indices(legSize, 0);

  ESVPQItemWrapper* pqItemWrapper = _trx->dataHandle().create<ESVPQItemWrapper>();
  pqItemWrapper->expandColumn() = 0;
  pqItemWrapper->indices() = indices;
  setTotalAmt(pqItemWrapper);

  ESVPQItem* pqItem = _trx->dataHandle().create<ESVPQItem>();
  pqItem->init(this, pqItemWrapper);

  pqItemWrapper->esvPqItem() = pqItem;

  return pqItemWrapper;
}

std::string
ESVPQ::getQueueTitle(QueueType qT)
{
  std::string title;

  switch (qT)
  {
  case MPNonstopOnline:
    title = "MUST PRICE NONSTOP ONLINE SOLUTIONS";
    break;

  case MPOutNonstopOnline:
    title = "MUST PRICE OUT-NONSTOP INB-SINGLE STOP ONLINE SOLUTIONS";
    break;

  case MPNonstopInterline:
    title = "MUST PRICE NONSTOP INTERLINE SOLUTIONS";
    break;

  case MPOutNonstopInterline:
    title = "MUST PRICE OUT-NONSTOP INB-SINGLE STOP INTERLINE SOLUTIONS";
    break;

  case MPSingleStopOnline:
    title = "MUST PRICE SINGLE STOP ONLINE SOLUTIONS";
    break;

  case MPRemainingOnline:
    title = "MUST PRICE REMAINING ONLINE SOLUTIONS";
    break;

  case MPRemaining:
    title = "MUST PRICE REMAINING SOLUTIONS";
    break;

  case LFSOnline:
    title = "LFS ONLINE SOLUTIONS";
    break;

  case LFSOnlineByCarrier:
    title = "LFS ONLINE BY CARRIER SOLUTIONS";
    break;

  case LFSRemaining:
    title = "LFS REMAINING SOLUTIONS";
    break;

  case UnknownQueueType:
    title = "UNKNOWN QUEUE TYPE";
    break;
  }

  return title;
}

void
ESVPQ::clearInternalData()
{
  while (!_pqItemPQ.empty())
  {
    _pqItemPQ.pop();
  }

  _legVecOut.clear();
  _legVecIn.clear();
  _legVec.clear();
  _alreadyCreatedSOPCombinations.clear();
}
}
