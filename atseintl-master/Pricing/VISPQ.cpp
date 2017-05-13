//----------------------------------------------------------------------------
//  File:        VISPQ.cpp
//  Created:     2009-04-27
//
//  Description: VIS priority queue
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

#include "Pricing/VISPQ.h"

#include "Pricing/ESVPQItem.h"

namespace tse
{
inline bool
UtilityLFSComparator::
operator()(ESVPQItem* lhs, ESVPQItem* rhs)
{
  return lhs->itinUtility() > rhs->itinUtility();
}

VISPQ::VISPQ(ShoppingTrx* trx,
             const std::vector<double>* parmBetaOut,
             const std::vector<double>* parmBetaIn,
             std::string title)
  : ESVPQ(trx, title),
    _parmBetaOut(parmBetaOut),
    _parmBetaIn(parmBetaIn)
{
}

ESVPQItem*
VISPQ::getNextItemVIS(Diag956Collector* diag956Collector)
{
  ESVPQItem* pqItem = nullptr;

  // First check if we've got already generated set of PQ Items in vector.
  // If so return next item.
  if ((!_pqItemsVec.empty()) && (_lastItemInVectorId < _pqItemsVec.size()))
  {
    pqItem = _pqItemsVec[_lastItemInVectorId];
    ++_lastItemInVectorId;

    return pqItem;
  }

  // If we've already get all items from vector clean it
  if ((!_pqItemsVec.empty()) && (_lastItemInVectorId == _pqItemsVec.size()))
  {
    _lastItemInVectorId = 0;
    _pqItemsVec.clear();
  }

  // If our vector is empty we need to generate new PQ Items set (with the
  // same money amount)
  if (_pqItemsVec.empty())
  {
    if (_pqItemWithNewPrice != nullptr)
    {
      pqItem = _pqItemWithNewPrice;
      _currentAmount = pqItem->totalAmt();
    }
    else
    {
      pqItem = ESVPQ::getNextItem(diag956Collector);

      if (pqItem != nullptr)
      {
        _currentAmount = pqItem->totalAmt();
      }
      else
      {
        return nullptr;
      }
    }

    // Add all PQ Items with the same money amount to vector
    while ((pqItem != nullptr) && (std::abs(pqItem->totalAmt() - _currentAmount) < EPSILON))
    {
      _pqItemsVec.push_back(pqItem);

      pqItem = ESVPQ::getNextItem(diag956Collector);
    }

    if (nullptr != pqItem)
    {
      _pqItemWithNewPrice = pqItem;
    }
    else
    {
      _pqItemWithNewPrice = nullptr;
    }

    if (!_pqItemsVec.empty())
    {
      // Calculate Utility value for all PQ Items from vector
      computeUtilityValues();

      // Sort vector with PQ Items
      std::sort(_pqItemsVec.begin(), _pqItemsVec.end(), _utilityLFSComparator);

      // Return first item from vector
      pqItem = _pqItemsVec[_lastItemInVectorId];
      ++_lastItemInVectorId;

      return pqItem;
    }
  }

  return nullptr;
}

void
VISPQ::computeUtilityValues()
{
  std::vector<ESVPQItem*>::iterator pqItemIter;

  for (pqItemIter = _pqItemsVec.begin(); pqItemIter != _pqItemsVec.end(); ++pqItemIter)
  {
    ESVPQItem* pqItem = (*pqItemIter);
    computeUtilityValuesForPQItem(pqItem);
  }
}

void
VISPQ::computeUtilityValuesForPQItem(ESVPQItem* pqItem)
{
  // Calculate utility values for outbound part only if it wasn't
  // generated before
  if (0.0 == pqItem->getUtility(0))
  {
    calculateFlightUtility(pqItem, 0, _parmBetaOut, false, ""); // emptyCarrier
  }

  // If we've got two legs
  if (pqItem->inSopWrapper() != nullptr)
  {
    calculateFlightUtility(
        pqItem, 1, _parmBetaIn, false, pqItem->outSopWrapper()->sop()->itin()->onlineCarrier());
  }

  // Update Itin Utility
  pqItem->itinUtility() = pqItem->utility(0);

  if (pqItem->inSopWrapper() != nullptr)
  {
    pqItem->itinUtility() += pqItem->utility(1);
  }
}

void
VISPQ::calculateFlightUtility(ESVPQItem* esvPQItem,
                              const int legIdx,
                              const std::vector<double>* paramBeta,
                              const bool business,
                              const CarrierCode& outboundCarrier)
{
  if (nullptr == paramBeta)
  {
    return;
  }

  MoneyAmount simpleInterlinePenalty = 10.0;
  MoneyAmount complexInterlinePenalty = 100.0;

  double u_DeptTime_min = 0;
  double b_ElapsedTime = 0;
  double b_ElapsedTime_0 = (*paramBeta)[0];
  double b_ElapsedTime_1 = (*paramBeta)[1];
  double b_ElapsedTime_2 = (*paramBeta)[2];
  double b_LGFare = (*paramBeta)[3];
  // double b_PaxShare = (*paramBeta)[4];
  double b_Sin2 = (*paramBeta)[5];
  double b_Sin4 = (*paramBeta)[6];
  double b_Sin6 = (*paramBeta)[7];
  double b_Cos2 = (*paramBeta)[8];
  double b_Cos4 = (*paramBeta)[9];
  double b_Cos6 = (*paramBeta)[10];

  if (esvPQItem->getNumStops(legIdx) == 0)
  {
    b_ElapsedTime = b_ElapsedTime_0;
  }
  else if (esvPQItem->getNumStops(legIdx) == 1)
  {
    b_ElapsedTime = b_ElapsedTime_1;
  }
  else if (esvPQItem->getNumStops(legIdx) >= 2)
  {
    b_ElapsedTime = b_ElapsedTime_2;
  }

  double deptMinutesRelation = ((double)esvPQItem->getDepartTime(legIdx).totalSeconds()) /
                               SECONDS_PER_MINUTE / MINUTES_PER_DAY;

  u_DeptTime_min =
      b_Sin2 * sin(2 * M_PI * deptMinutesRelation) + b_Sin4 * sin(4 * M_PI * deptMinutesRelation) +
      b_Sin6 * sin(6 * M_PI * deptMinutesRelation) + b_Cos2 * cos(2 * M_PI * deptMinutesRelation) +
      b_Cos4 * cos(4 * M_PI * deptMinutesRelation) + b_Cos6 * cos(6 * M_PI * deptMinutesRelation);

  ESVSopWrapper* sopWrapper =
      (0 == legIdx) ? esvPQItem->outSopWrapper() : esvPQItem->inSopWrapper();

  MoneyAmount totalEsvAmt = sopWrapper->getTotalAmt();

  // We will penalize all directional portion that is interline
  if (!business)
  {
    if ("" == sopWrapper->sop()->itin()->onlineCarrier())
    {
      totalEsvAmt += complexInterlinePenalty;
    }
    else
    {
      // Calculating simple interline penalty
      if (legIdx == inLegIdx)
      {
        const CarrierCode& inboundCarrier = sopWrapper->sop()->itin()->onlineCarrier();

        if ((!outboundCarrier.empty()) && (outboundCarrier != inboundCarrier))
        {
          totalEsvAmt += simpleInterlinePenalty;
        }
      }
    }
  }

  esvPQItem->utility(legIdx) = u_DeptTime_min +
                               b_ElapsedTime * esvPQItem->getFlightTimeMinutes(legIdx) +
                               b_LGFare * log(totalEsvAmt / 100.0);
  //+ b_PaxShare * esvPQItem->paxShare(); // paxShare was always set to 0
}
}
