//-------------------------------------------------------------------
//
//  File:        PenaltyEstimator.h
//  Created:     July, 2014
//  Authors:     Grzegorz Szczurek
//
//  Updates:
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <set>
#include <vector>

namespace tse
{

class Logger;
class ProcessTagPermutation;
class ProcessTagInfo;
class PricingTrx;

class PenaltyEstimator
{
public:
  PenaltyEstimator(const std::vector<uint32_t>& unmatchedFCs,
                   const CurrencyCode& paymentCurrency,
                   PricingTrx& trx):
    _unmatchedFCs(unmatchedFCs.begin(), unmatchedFCs.end()),
    _paymentCurrency(paymentCurrency), _trx(trx)
  {
  }

  void operator()(ProcessTagPermutation* perm);

private:
  Indicator determineFeeApplication(const ProcessTagPermutation* permutation) const;

  MoneyAmount collectFee(const ProcessTagPermutation* permutation, Indicator feeApplication) const;

  bool hasDifferentFeeApplication(const ProcessTagPermutation* permutation) const;

  bool hasFeeApplication(const ProcessTagPermutation* permutation, Indicator feeAppl) const;

  MoneyAmount highestOfAllFCs(const ProcessTagPermutation* permutation) const;

  MoneyAmount highestOfChangedFCs(const ProcessTagPermutation* permutation) const;

  MoneyAmount eachOfChangedFCs(const ProcessTagPermutation* permutation) const;

  MoneyAmount getPenaltyAmount(const ProcessTagInfo* pti) const;

  MoneyAmount getMinAmount(const ProcessTagInfo* pti) const;

  static constexpr Indicator HIGHEST_OF_CHANGED_FC = '1';
  static constexpr Indicator HIGHEST_OF_ALL_FC = '2';
  static constexpr Indicator EACH_OF_CHANGED_FC = '3';
  static constexpr Indicator HIGHEST_FROM_CHANGED_PU = '4';
  static constexpr Indicator HIGHEST_FROM_CHANGED_PU_ADDS = '5';

  const std::set<uint32_t> _unmatchedFCs;
  const CurrencyCode _paymentCurrency;

  PricingTrx& _trx;

  static Logger _logger;
};

} //tse

