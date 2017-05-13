//----------------------------------------------------------------------------
//  File:        Diag956Collector.C
//
//  Copyright Sabre 2004
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

#include "Diagnostic/Diag956Collector.h"

#include "Common/Code.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ESVSolution.h"
#include "DataModel/PaxTypeFare.h"
#include "Pricing/EstimatedSeatValue.h"
#include "Pricing/ESVPQ.h"
#include "Pricing/ESVPQItem.h"

#include <iterator>

namespace tse
{
void
Diag956Collector::displayHeader(const ESVPQ* esvpq)
{
  *this << "DIVC PASSED, CHOSEN\n"
        << "DIV1 SOLUTION NOT CHOSEN: ALREADY IN PICKED SOLUTIONS\n"
        << "DIV2 SOLUTION NOT CHOSEN: IMPROPER NUMBER OF STOPS\n"
        << "DIV3 SOLUTION NOT CHOSEN: IT IS NOT INTERLINE\n"
        << "DIV4 SOLUTION NOT CHOSEN: IT IS NOT ONLINE\n"
        << "DIV5 ONLINE ONLY RESTRICTED CARRIER\n"
        << "DIV6 INTERLINE LIMITED CARRIER\n"
        << "DIV7 SOLUTION NOT CHOSEN: DIVERSITY LIMITS EXCEEDED\n"
        << "DIV8 SOLUTION NOT CHOSEN: UPPER BOUND FACTOR LIMIT EXCEEDED\n"
        << "F FAILED CAT10\n"
        << "FG SOP COMBINATION ALREADY GENERATED\n\n";
  *this << "DIAG 956 PQ TITLE: " << esvpq->title() << "\n";
  displayPQInfo(esvpq);
}

Diag956Collector&
Diag956Collector::displayESVPQ(ESVPQ* esvpq)
{
  if ((false == _active) || (esvpq == nullptr))
  {
    return (*this);
  }

  std::vector<std::vector<ShoppingTrx::SchedulingOption*> > legSopVec;
  std::vector<const std::vector<ESVSopWrapper*>*>::const_iterator legVecIter;

  for (legVecIter = esvpq->legVec().begin(); legVecIter != esvpq->legVec().end(); ++legVecIter)
  {
    const std::vector<ESVSopWrapper*>& wrappVec = **legVecIter;
    std::vector<ESVSopWrapper*>::const_iterator wrappIter;
    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    std::vector<ShoppingTrx::SchedulingOption*>::iterator sopIter;

    for (wrappIter = wrappVec.begin(); wrappIter != wrappVec.end(); ++wrappIter)
    {
      if (*wrappIter == nullptr)
      {
        continue;
      }

      bool sopAlreadyInVec = false;
      for (sopIter = sopVec.begin(); sopIter != sopVec.end(); sopIter++)
      {
        if ((*wrappIter)->sop() == *sopIter)
        {
          sopAlreadyInVec = true;
          break;
        }
      }

      if (!sopAlreadyInVec)
      {
        sopVec.push_back((*wrappIter)->sop());
      }
    }
    legSopVec.push_back(sopVec);
  }

  displayDiagLegsContent(esvpq, legSopVec);
  flushMsg();

  return (*this);
}

Diag956Collector&
Diag956Collector::displayDiagLegsContent(
    const ESVPQ* esvpq, std::vector<std::vector<ShoppingTrx::SchedulingOption*> >& legVec)
{
  if ((false == _active) || (!supportedPQType(esvpq->carrier(), esvpq->diversityOption())))
  {
    return (*this);
  }

  displayHeader(esvpq);
  displayDiagLegsContent(legVec);
  return *this;
}

void
Diag956Collector::displayDiagLegsContent(
    const std::vector<std::vector<ShoppingTrx::SchedulingOption*> >& legVec)
{
  std::vector<std::vector<ShoppingTrx::SchedulingOption*> >::const_iterator legIter =
      legVec.begin();
  std::vector<std::vector<ShoppingTrx::SchedulingOption*> >::const_iterator legIterEnd =
      legVec.end();
  int legIndex = 0;
  for (; legIter != legIterEnd; ++legIter)
  {
    const std::vector<ShoppingTrx::SchedulingOption*>& sopVec = *legIter;
    displayDiagSOPVec(sopVec, legIndex);
    legIndex++;
  }

  *this << "\n";
}

void
Diag956Collector::displayDiagSOPVec(const std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                    int legIndex,
                                    bool fullOutputFlag)
{
  if (false == _active)
  {
    return;
  }

  char oiInd = 'X';
  if (legIndex == 0)
    oiInd = 'O';
  else
    oiInd = 'I';

  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator sopIter = sopVec.begin();
  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator sopIterEnd = sopVec.end();
  for (; sopIter != sopIterEnd; ++sopIter)
  {
    *this << oiInd << ":" << (*sopIter)->originalSopId() << ",";
  }

  *this << "\n";
}

Diag956Collector&
Diag956Collector::displayESVPQItem(ESVPQItem* pqItem,
                                   std::string addInfo,
                                   bool cat10enabled,
                                   bool cat10passed)
{
  if ((false == _active) || (pqItem == nullptr))
  {
    return (*this);
  }

  DiagCollector& dc(*this);
  dc << "\n";
  dc << pqItem->outSopWrapper()->sop()->originalSopId();
  displayCarriers(pqItem->outSopWrapper()->sop()->itin());

  if (pqItem->inSopWrapper() != nullptr)
  {
    dc << "," << pqItem->inSopWrapper()->sop()->originalSopId();
    displayCarriers(pqItem->inSopWrapper()->sop()->itin());
  }

  dc << "\n";
  displayPQInfo(pqItem->pq());

  displayFarePathInfo(pqItem, 0);

  if (pqItem->inSopWrapper() != nullptr)
  {
    displayFarePathInfo(pqItem, 1);
  }

  displayAmounts(pqItem);

  if (cat10passed)
    dc << "P\n";
  else
  {
    dc << "F";
    dc << addInfo;
    dc << "\n";
  }

  return *this;
}

void
Diag956Collector::displayCarriers(const Itin* itin)
{
  DiagCollector& dc(*this);

  std::vector<TravelSeg*>::const_iterator iter = itin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = itin->travelSeg().end();

  for (; iter != iterEnd; ++iter)
  {
    const AirSeg* airSegment = dynamic_cast<const AirSeg*>(*iter);

    if (airSegment != nullptr)
    {
      dc << airSegment->carrier();

      if (iterEnd - iter > 1)
      {
        dc << ",";
      }
    }
  }
}

void
Diag956Collector::displayPQInfo(const ESVPQ* pq)
{
  if (pq)
  {
    DiagCollector& dc(*this);
    switch (pq->queueType())
    {
    case MPNonstopOnline:
      dc << "MPNonstopOnline " << pq->carrier() << "\n";
      break;
    case MPOutNonstopOnline:
      dc << "MPOutNonstopOnline " << pq->carrier() << "\n";
      break;
    case MPNonstopInterline:
      dc << "MPNonstopInterline " << pq->carrier() << "\n";
      break;
    case MPOutNonstopInterline:
      dc << "MPOutNonstopInterline " << pq->carrier() << "\n";
      break;
    case MPSingleStopOnline:
      dc << "MPSingleStopOnline " << pq->carrier() << "\n";
      break;
    case MPRemainingOnline:
      dc << "MPRemainingOnline " << pq->carrier() << "\n";
      break;
    case MPRemaining:
      dc << "MPRemaining " << pq->carrier() << "\n";
      break;
    case LFSOnline:
      dc << "LFSOnline " << pq->carrier() << "\n";
      break;
    case LFSOnlineByCarrier:
      dc << "LFSOnlineByCarrier " << pq->carrier() << "\n";
      break;
    case LFSRemaining:
      dc << "LFSRemaining " << pq->carrier() << "\n";
      break;
    case UnknownQueueType:
      dc << "UnknownQueueType " << pq->carrier() << "\n";
      break;
    default:
      dc << "ERROR: NOT SUPPORTED " << pq->carrier() << "\n";
      break;
    }
  }
}

void
Diag956Collector::displayFarePathInfo(const ESVPQItem* pqItem, const int ledIdx)
{
  const ESVSopWrapper* sopWrapper =
      (0 == ledIdx) ? pqItem->outSopWrapper() : pqItem->inSopWrapper();

  switch (sopWrapper->getSopFareListType())
  {
  case ESVSopWrapper::OW:
    *this << "OW";
    break;
  case ESVSopWrapper::RT:
    *this << "RT";
    break;
  case ESVSopWrapper::OJ:
    *this << "OJ(";
    displayCombinationType(sopWrapper->getCombinationType());
    *this << ")";
    break;
  case ESVSopWrapper::CT:
    *this << "CT";
    break;
  default:
    *this << "OTHER:" << sopWrapper->getSopFareListType();
    break;
  }

  std::vector<PaxTypeFare*>::const_iterator it = sopWrapper->paxTypeFareVec()->begin();
  std::vector<PaxTypeFare*>::const_iterator itEnd = sopWrapper->paxTypeFareVec()->end();
  while (it != itEnd)
  {
    *this << ",";
    PaxTypeFare* paxTypeFare = *it;
    if (paxTypeFare == nullptr)
      *this << "ERROR NULL";
    displayFBC(*paxTypeFare);
    ++it;
  }

  *this << "\n";
}

void
Diag956Collector::displayFBC(const PaxTypeFare& paxTypeFare)
{
  // Print fare basis code
  std::string fareBasis = paxTypeFare.createFareBasis((tse::PricingTrx&)*_trx, false);

  *this << fareBasis;
}

void
Diag956Collector::displayCombinationType(const SOPFarePath::CombinationType ct)
{
  switch (ct)
  {
  case SOPFarePath::NOT_INITIALIZED:
    *this << "NI";
    break;
  case SOPFarePath::R:
    *this << "R";
    break;
  case SOPFarePath::X:
    *this << "X";
    break;
  case SOPFarePath::RR:
    *this << "RR";
    break;
  case SOPFarePath::RX:
    *this << "RX";
    break;
  case SOPFarePath::XR:
    *this << "XR";
    break;
  case SOPFarePath::XX:
    *this << "XX";
    break;
  default:
    *this << "OTHER";
    break;
  }
}

Diag956Collector&
Diag956Collector::displayAddInfo(std::string addInfo)
{
  if (false == _active)
  {
    return (*this);
  }

  *this << addInfo << "\n";
  flushMsg();

  return *this;
}

Diag956Collector&
Diag956Collector::displayAmounts(ESVPQItem* pqItem)
{
  if ((false == _active) || (pqItem == nullptr))
  {
    return (*this);
  }

  DiagCollector& dc(*this);

  dc << pqItem->totalAmt() << "," << pqItem->totalPrice() << "," << pqItem->totalPenalty() << ",";

  return *this;
}

bool
Diag956Collector::supportedPQType(CarrierCode carrier, std::string diversityOption)
{
  if (false == _active)
  {
    return false;
  }

  return true;
}
}
