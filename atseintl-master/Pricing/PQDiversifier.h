//----------------------------------------------------------------------------
//  Copyright Sabre 2005
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

#include "Common/Global.h"
#include "Common/Hasher.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"


#include <cstring>
#include <map>
#include <set>
#include <vector>

namespace tse
{
class Itin;

// Diversity formula 1:  X = [#options * (1 + percentage)] / (#online queues + adjuster)
// Where X is the number of options each online PQ should produce
// Diversity formula 2:  Y = X * adjuster
// Where Y is the number of options the interline PQ should produce

const double PQ_DIVERSITY_ADJUST_MIN_CXRS = 1.00;
const double PQ_DIVERSITY_ADJUST_MAX_CXRS = 8.00;
const double PQ_DIVERSITY_ADJUST_MULTIPLIER = 0.50;

class ShoppingTrx;

class PQDiversifier
{
  friend class PQDiversifierTest;

  double _pqDiversityPercentage;
  uint32_t _interlineWeightFactor;
  uint32_t _interlineLessWeightFactor;

  void calculateDiversityNumbers(ShoppingTrx& trx) const;
  bool isOnlyOnlinesPossible(ShoppingTrx& trx) const;

  // For each online carrier, calculate
  // how many sops has the carrier on each leg
  void countCxrSOP(ShoppingTrx& trx) const;

  void calculateMaxCombinationCount(ShoppingTrx::PQDiversifierResult& results) const;

  void countOnlineSolutions(ShoppingTrx& trx,
                            const double numRequestOptions,
                            const double numberInterlineOptions) const;
  void allocateExtraOptions(ShoppingTrx::PQDiversifierResult& results, uint16_t extraOptions) const;

  std::vector<CarrierCode> collectOnlineCarriers(ShoppingTrx& trx) const;
  uint32_t
  calculateNumOfSolutionNeededForDatePair(const PricingOptions* options,
                                          ShoppingTrx::PQDiversifierResult& diversityResults) const;

public:
  PQDiversifier();
  void process(ShoppingTrx& trx) const;
  void init();
};

} // namespace tse

