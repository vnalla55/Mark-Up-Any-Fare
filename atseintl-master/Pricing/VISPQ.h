//----------------------------------------------------------------------------
//  File:        VISPQ.h
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

#pragma once

#include "Pricing/EstimatedSeatValue.h"
#include "Pricing/ESVPQ.h"

namespace tse
{

class ESVPQItem;

class UtilityLFSComparator
{
public:
  bool operator()(ESVPQItem* lhs, ESVPQItem* rhs);
};

class VISPQ : public ESVPQ
{
public:
  VISPQ(ShoppingTrx* trx,
        const std::vector<double>* parmBetaOut,
        const std::vector<double>* parmBetaIn,
        std::string title = "Solutions Priority Queue");

  ESVPQItem* getNextItemVIS(Diag956Collector* diag956Collector);

  void computeUtilityValuesForPQItem(ESVPQItem* pqItem);

private:
  VISPQ();

  void computeUtilityValues();

  void calculateFlightUtility(ESVPQItem* esvPQItem,
                              const int legIdx,
                              const std::vector<double>* paramBeta,
                              const bool business,
                              const CarrierCode& outboundCarrier);

  std::vector<ESVPQItem*> _pqItemsVec;
  ESVPQItem* _pqItemWithNewPrice = nullptr;
  MoneyAmount _currentAmount = 0.0;
  uint32_t _lastItemInVectorId = 0;
  UtilityLFSComparator _utilityLFSComparator;
  const std::vector<double>* _parmBetaOut = nullptr;
  const std::vector<double>* _parmBetaIn = nullptr;
};

} // End namespace tse

